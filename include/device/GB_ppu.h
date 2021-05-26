#ifndef GB_PPU_H_
# define GB_PPU_H_

#include "common/GB_types.h"
#include "common/GB_clock.h"
#include "common/GB_macro.h"
#include "GB_oram.h"
#include "GB_vram.h"

namespace GB::device {

// TODO(dolovnyak) implement correct on/off screen behavior

class PPU {
 public:
    constexpr static clk_cycle_t OAM_SEARCH_TIME = 80_CLKCycles;
    constexpr static clk_cycle_t SCANLINE_TIME = 456_CLKCycles;
    constexpr static clk_cycle_t ENDLINE_TIME = 4_CLKCycles;  // TODO(dolovnyak) need to check is it correct on test roms

    constexpr static unsigned LARGE_OBJECT_HEIGHT = 16;     // TODO(dolovnyak) del if not will be used
    constexpr static unsigned NORMAL_OBJECT_HEIGHT = 8;     // TODO(dolovnyak) del if not will be used

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
    };

    enum STAT_Mode {
        STAT_Hblank = 0,
        STAT_Vblank = 1,
        STAT_SearchingOAM = 2,
        STAT_Render = 3
    };

    enum State {
        Search = 0,
        Render = 1,
        HBlank = 2,
        VBlank = 3,
        LastVblank = 4,
        EndLine = 5,
        LastEndLine = 6
    };

    PPU() = delete;
    PPU(ORAM *oram, VRAM *vram);
    PPU(const PPU& other) = default;
    PPU(PPU&& other) = default;
    ~PPU() = default;

    PPU& operator=(const PPU& other) = default;
    PPU& operator=(PPU&& other) = default;


    inline byte_t get_LCDC_reg() const;
    inline void set_LCDC_reg(byte_t value);

    inline byte_t get_STAT_reg() const;
    inline void set_STAT_reg(byte_t value);

    inline byte_t get_LY_reg() const;

    inline byte_t get_LYC_reg() const;
    inline void set_LYC_reg(byte_t value);

    inline void set_STAT_mode(STAT_Mode state);


    /**
     * OAM_SEARCH_TIME = 80, SCANLINE_TIME = 456, ENDLINE_TIME = 4, __render_time = calculated during search
     *
     * Search -> Render                     [Step = OAM_SEARCH_TIME]
     * Render -> Hblank                     [Step = __render_time]
     * Hblank -> EndLine                    [Step = SCANLINE_TIME - OAM_SEARCH_TIME - __render_time - ENDLINE_TIME]
     * EndLine (0 <= LY <= 143) -> Search   {                              }
     * EndLine (144 <= LY <= 152) -> Vblank { [Step = ENDLINE_TIME] (++LY) }
     * EndLine (LY = 153) -> LastVblank     {                              }
     * Vblank -> EndLine                    [Step = SCANLINE_TIME - ENDLINE_TIME]
     * LastVblank -> FinalEndLine           [Step = SCANLINE_TIME - ENDLINE_TIME] (++LY)
     * FinalEndLine -> Search               [Step = ENDLINE_TIME]
     */
    void search_to_render_transition();
    void render_to_hblank_transition();
    void hblank_to_endline_transition();
    void vblank_to_endline_transition();
    void last_vblank_to_last_endline_transition();
    void endline_transition();
    void last_endline_to_search_transition();

    void step();

 protected:
    ORAM*                   __oram;
    VRAM*                   __vram;
    Registers               __regs;
    State                   __current_state;
    clk_cycle_t             __render_time;
    ::devsync::counter_t    __counter;

    struct Object {
        u8 pos_x;
        u8 pos_y;
        u8 sprite_index;
        u8 attributes;
    };
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
    __regs.STAT |= ::bit_slice(7, 2, value);
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
