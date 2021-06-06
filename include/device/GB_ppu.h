#ifndef GB_PPU_H_
# define GB_PPU_H_

#include <vector>
#include "common/GB_types.h"
#include "common/GB_clock.h"
#include "common/GB_macro.h"
#include "GB_oram.h"
#include "GB_vram.h"

/**
 * NOTE:
 *      Now there is state machine with rather long transition between state - Render takes 168-291 clocks.
 *
 *      It means that there is an incorrect behavior:
 *          * Changing color palette register during Render will take effect only on the next Render state
 *            (it means - what in this case color of a part of pixels in current scanline will differ from original).
 *          * Changing LCDC register also will take effect only on the next Render.
 *          * If dma-transfer will started or ended during Render - part of pixels also will be differ from original.
 *
 *      Way to fix this incorrect behavior.
 *          Instead Render state - implement FETCHING_STATE_MACHINE (or just added states to current machine).
 *
 *          Fetching sate machine states are something like:
 *          FETCHER_GET_TILE, 8 clocks
 *          FETCHER_GET_TILE_DATA_LOWER, 8 clocks
 *          FETCHER_GET_TILE_DATA_HIGH, 8 clocks
 *          FETCHER_PUSH, 8 clocks
 *          ....
 */

/**
 * Current state machine:
 *      OBJECT_SEARCH_TIME = ORAM_SEARCH_TIME / ORAM_OBJECTS_NUM = 2, SCANLINE_TIME = 456, ENDLINE_TIME = 4,
 *      __render_time = calculated depending on founded objects and SCX register
 *
 *      FirstOamSearch(CurObj = 0) -> SearchOam	{                                                        }
 *      SearchOam (cur obj <= 38) -> SearchOam	{ [Step = OBJECT_SEARCH_TIME] ++current_searching_object }
 *      SearchOam (cur obj = 39) -> Render      {                                                        }
 *      Render -> Hblank                      	[Step = __render_time]
 *      Hblank -> EndLine                       [Step = SCANLINE_TIME - ORAM_SEARCH_TIME - __render_time - ENDLINE_TIME]
 *      EndLine (0 <= LY <= 143) -> SearchOam   {                              }
 *      EndLine (144 <= LY <= 152) -> Vblank    { [Step = ENDLINE_TIME] (++LY) }
 *      EndLine (LY = 153) -> LastVblank        {                              }
 *      Vblank -> EndLine                       [Step = SCANLINE_TIME - ENDLINE_TIME]
 *      LastVblank -> LastEndLine               [Step = SCANLINE_TIME - ENDLINE_TIME] (++LY)
 *      LastEndLine -> FirstOamSearch           [Step = ENDLINE_TIME]
 */
namespace GB::device {

// TODO(dolovnyak) implement correct on/off screen behavior

class PPU {
 public:
    struct Registers {
        Reg8 LCDC;
        Reg8 STAT;
        Reg8 SCY;
        Reg8 SCX;
        Reg8 LY;
        Reg8 LYC;
        Reg8 BGP;
        Reg8 OBP0;
        Reg8 OBP1;
        Reg8 WY;
        Reg8 WX;

        //CGB registers
        Reg8 BCPS;
        Reg8 BCPD;
        Reg8 OCPS;
        Reg8 OCPD;

        explicit
        Registers(
                Reg8 lcdc_init_value = LCDC_INIT_VALUE,
                Reg8 stat_init_value = STAT_INIT_VALUE)
        : LCDC(lcdc_init_value), STAT(stat_init_value)
        {}
    };

    enum STAT_Mode {
        STAT_Hblank = 0,
        STAT_Vblank = 1,
        STAT_SearchingOAM = 2,
        STAT_Render = 3
    };

    enum State {
        FirstOamSearch = 0,
        SearchOam = 1,
        Render = 3,
        HBlank = 4,
        VBlank = 5,
        LastVblank = 6,
        EndLine = 7,
        LastEndLine = 8
    };

    struct Object {
        u8 pos_x;
        u8 pos_y;
        u8 sprite_index;
        u8 attributes;
    };

    constexpr static clk_cycle_t ORAM_SEARCH_TIME = 80_CLKCycles;
    // TODO(dolovnyak) need to find accurate object search timings.
    constexpr static clk_cycle_t OBJECT_SEARCH_TIME = 2_CLKCycles;
    constexpr static clk_cycle_t SCANLINE_TIME = 456_CLKCycles;
    constexpr static clk_cycle_t ENDLINE_TIME = 4_CLKCycles;  // TODO(dolovnyak) need to check timings on test roms

    // TODO(dolovnyak, hgranule) rename prettier
    constexpr static unsigned MAX_INTERSECTED_OBJECTS = 10;
    constexpr static unsigned LARGE_OBJECT_HEIGHT = 16;
    constexpr static unsigned NORMAL_OBJECT_HEIGHT = 8;
    constexpr static unsigned OBJ_Y_INDENT = 16;

    PPU() = delete;
    PPU(ORAM *oram, VRAM *vram);
    PPU(const PPU& other) = default;
    PPU(PPU&& other) = default;
    ~PPU() = default;

    PPU& operator=(const PPU& other) = default;
    PPU& operator=(PPU&& other) = default;

    /**
     * @details:
     *  0 bit - DMG/DMG_MODE - BG and Window become white and only sprites could be visible.
     *          CGB_MODE - BG and Window lose their priority, sprites will be always displayed on top of BG and window,
     *                     independently of the priority flags in OAM and BG Map attributes.
     *  1 bit - 0 - sprites disable, 1 - sprites enable
     *  2 bit - Object size 0 - 8x8, 1 - 8x16
     *  3 bit -
     *  4 bit -
     *  5 bit -
     *  6 bit -
     *  7 bit -
     */
    inline byte_t get_LCDC_reg() const;
    inline void set_LCDC_reg(byte_t value);

    inline byte_t get_STAT_reg() const;
    inline void set_STAT_reg(byte_t value);

    inline byte_t get_LY_reg() const;

    inline byte_t get_LYC_reg() const;
    inline void set_LYC_reg(byte_t value);

    inline void set_STAT_mode(STAT_Mode state);

    void step();

 protected:
    ORAM*                   __oram;
    VRAM*                   __vram;
    Registers               __regs;
    State                   __current_state;
    clk_cycle_t             __render_time;
    ::devsync::counter_t    __counter;
    std::vector<Object>     __intersected_objects;
    u8                      __next_object_index;

    void add_oram_object();

    void first_search_transition();
    void search_to_search_transition();
    void search_to_render_transition();
    void render_to_hblank_transition();
    void hblank_to_endline_transition();
    void vblank_to_endline_transition();
    void last_vblank_to_last_endline_transition();
    void endline_transition();
    void last_endline_to_search_transition();
};

inline byte_t PPU::get_LCDC_reg() const {
    return __regs.LCDC;
}

inline void PPU::set_LCDC_reg(byte_t value) {
    __regs.LCDC = value;
}

inline byte_t PPU::get_STAT_reg() const {
    return __regs.STAT;
}

inline void PPU::set_STAT_reg(byte_t value) {
    __regs.STAT = (value & ::bit_mask(7, 2)) | ::bit_slice(1, 0, __regs.STAT);
}

inline byte_t PPU::get_LY_reg() const {
    return __regs.LY;
}

inline byte_t PPU::get_LYC_reg() const {
    return __regs.LYC;
}

inline void PPU::set_LYC_reg(byte_t value) {
    __regs.LYC = value;
}

inline void PPU::set_STAT_mode(PPU::STAT_Mode stat_mode) {
    __regs.STAT = ::bit_slice(7, 2, __regs.STAT) | stat_mode;
}

}

#endif
