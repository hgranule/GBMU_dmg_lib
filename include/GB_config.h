/**
 * @file GB_config.h
 * @brief ...
 */

#ifndef GB_CONFIG_H_
# define GB_CONFIG_H_

# include "common/GB_types.h"

namespace GB {

constexpr unsigned WRAM_CGB_BANK_SIZE = 4_KBytes;
constexpr unsigned WRAM_CGB_SIZE = 32_KBytes;
constexpr unsigned WRAM_NON_CGB_SIZE = 8_KBytes;

constexpr unsigned SVBK_INIT_VALUE = 0x0;
constexpr unsigned VBK_INIT_VALUE = 0x0;

constexpr unsigned INTC_IF_INIT_VALUE = 0x0;
constexpr unsigned INTC_IE_INIT_VALUE = 0x0;
constexpr bool INTC_IME_INIT_VALUE = true;

constexpr unsigned ORAM_OBJECTS_NUM = 40;
constexpr unsigned ORAM_SIZE = ORAM_OBJECTS_NUM * 4_Bytes;

constexpr unsigned VRAM_BANK_SIZE = 8_KBytes;
constexpr unsigned VRAM_NON_CGB_SIZE = VRAM_BANK_SIZE;
constexpr unsigned VRAM_CGB_SIZE = VRAM_BANK_SIZE * 2;
constexpr unsigned VRAM_MAX_SIZE = VRAM_CGB_SIZE;

constexpr unsigned SND_LN_CNTR_INIT_VALUE = 0x0;


enum GBModeFlag : u16 {
    DMG_MODE = 0b000001,
    MGB_MODE = 0b000010,
    CGB_DMG_MODE = 0b000100,
    CGB_MODE = 0b001000
};

}  // namespace GB

#endif  // GB_CONFIG_H_
