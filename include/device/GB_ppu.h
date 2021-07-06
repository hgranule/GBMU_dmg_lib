#ifndef DEVICE_GB_PPU_H_
# define DEVICE_GB_PPU_H_

#include <vector>
#include "common/GB_types.h"
#include "common/GB_clock.h"
#include "common/GB_macro.h"
#include "GB_oram.h"
#include "GB_vram.h"

namespace GB::device {

// TODO(dolovnyak) implement correct on/off screen behavior

class PPU {
 public:
    GBModeFlag  TEMPORARY_GB_MODE_FLAG;  // TODO(dolovnyak, hgranule) I created it to be able to test.

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
        byte_t pos_x;
        byte_t pos_y;
        byte_t sprite_index;
        byte_t attributes;
    };

    // TODO(dolobnyak, hgranule) need to remake on dots
    constexpr static clk_cycle_t ORAM_SEARCH_TIME = 80_CLKCycles;
    constexpr static clk_cycle_t OBJECT_SEARCH_TIME = 2_CLKCycles;
    constexpr static clk_cycle_t SCANLINE_TIME = 456_CLKCycles;
    constexpr static clk_cycle_t ENDLINE_TIME = 4_CLKCycles;

    // TODO(dolovnyak, hgranule) maybe rename prettier
    constexpr static unsigned MAX_INTERSECTED_OBJECTS = 10;
    constexpr static unsigned OBJ_X_INDENT = 8;
    constexpr static unsigned OBJ_Y_INDENT = 16;
    constexpr static unsigned NORMAL_OBJECT_HEIGHT = 8;
    constexpr static unsigned LARGE_OBJECT_HEIGHT = 16;

    constexpr static u8 STAT_MASK = 0b10000000;

    PPU() = delete;
    PPU(ORAM *oram, VRAM *vram);
    PPU(const PPU& other) = default;
    PPU(PPU&& other) = default;
    ~PPU() = default;

    PPU& operator=(const PPU& other) = default;
    PPU& operator=(PPU&& other) = default;

    [[nodiscard]] inline byte_t get_LCDC_reg() const;
    inline void set_LCDC_reg(byte_t value);

    [[nodiscard]] inline byte_t get_STAT_reg() const;
    inline void set_STAT_reg(byte_t value);

    [[nodiscard]] inline byte_t get_LY_reg() const;

    [[nodiscard]] inline byte_t get_LYC_reg() const;
    inline void set_LYC_reg(byte_t value);

    void step();

 protected:
    ORAM*                   __oram;
    VRAM*                   __vram;

    State                   __current_state;

    clk_cycle_t             __render_time;

    ::devsync::counter_t    __counter;

    std::vector<Object>     __intersected_objects;
    u8                      __next_object_index;

    bool                    __stat_interrupt_requested;

    // LCDC
    bool                    __bg_window_enable;
    bool                    __obj_enable;
    bool                    __obj_high;
    bool                    __bg_tile_map_memory_shifted;
    bool                    __bg_window_tiles_memory_shifted;
    bool                    __window_enable;
    bool                    __window_tile_map_memory_shifted;
    bool                    __lcd_enable;

    // STAT
    bool                    __ly_equal_to_lyc_flag;
    bool                    __hblank_interrupt_enable;
    bool                    __vblank_interrupt_enable;
    bool                    __oram_interrupt_enable;
    bool                    __ly_interrupt_enable;
    STAT_Mode               __stat_mode;

    // LY LYC
    byte_t                  __current_line;  // TODO(dolovnayk) when screen of - it fixed at 0x0.
    byte_t                  __line_to_compare;

    void add_oram_object(int current_object_index);
    void ly_equal_lyc_interrupt_handle();
};

inline byte_t PPU::get_LCDC_reg() const {
    return  __bg_window_enable << 0
            | __obj_enable << 1
            | __obj_high << 2
            | __bg_tile_map_memory_shifted << 3
            | __bg_window_tiles_memory_shifted << 4
            | __window_enable << 5
            | __window_tile_map_memory_shifted << 6
            | __lcd_enable << 7;
}

inline void PPU::set_LCDC_reg(byte_t value) {
    __bg_window_enable                  = ::bit_n(0, value);
    __obj_enable                        = ::bit_n(1, value);
    __obj_high                          = ::bit_n(2, value);
    __bg_tile_map_memory_shifted        = ::bit_n(3, value);
    __bg_window_tiles_memory_shifted    = ::bit_n(4, value);
    __window_enable                     = ::bit_n(5, value);
    __window_tile_map_memory_shifted    = ::bit_n(6, value);
    __lcd_enable                        = ::bit_n(7, value);
}

/**
 * @details:    Bit 7 unused and always set in 1
 *              https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf (8.5)
 */
inline byte_t PPU::get_STAT_reg() const {
    return  __stat_mode
            | __ly_equal_to_lyc_flag << 2
            | __hblank_interrupt_enable << 3
            | __vblank_interrupt_enable << 4
            | __oram_interrupt_enable << 5
            | __ly_interrupt_enable << 6
            | STAT_MASK;
}

inline void PPU::set_STAT_reg(byte_t value) {
    __ly_equal_to_lyc_flag      = ::bit_n(2, value);
    __hblank_interrupt_enable   = ::bit_n(3, value);
    __vblank_interrupt_enable   = ::bit_n(4, value);
    __oram_interrupt_enable     = ::bit_n(5, value);
    __ly_interrupt_enable       = ::bit_n(6, value);
}

inline byte_t PPU::get_LY_reg() const {
    return __current_line;
}

inline byte_t PPU::get_LYC_reg() const {
    return __line_to_compare;
}

inline void PPU::set_LYC_reg(byte_t value) {
    __line_to_compare = value;
}

}  // namespace GB::device

#endif  // DEVICE_GB_PPU_H_
