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

    enum GBModeFlag : u16 {
        DMG_MODE = 0b000001,
        MGB_MODE = 0b000010,
        CGB_MODE = 0b001000,
    };

};

#endif
