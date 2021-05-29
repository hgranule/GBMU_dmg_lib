
/**
 * @file GB_sound_channel_3.h
 * 
 * @brief Describes Sound Channel Three controller class
 */

#ifndef DEVICE_GB_SOUND_CHANNEL_3_H_
# define DEVICE_GB_SOUND_CHANNEL_3_H_

# include "GB_config.h"

# include "common/GB_types.h"
# include "common/GB_macro.h"

namespace GB::device {
    /**
     * @brief Implementation of Sound Channel Three from GameBoy
     */

class SoundChannel3 {
    public:

    /**
     * @brief Sound channnel 3 register's
     */

    struct Registers {

        constexpr static unsigned REG_RESERVED_NR30_BIT_MASK     = ::bit_mask(7, 7);
        constexpr static unsigned REG_RESERVED_NR32_BIT_MASK     = ::bit_mask(6, 5);
        constexpr static unsigned REG_RESERVED_NR34_BIT_MASK     = ::bit_mask(6, 6);

        /**
         * @brief NR30 register (R/W)
         * 
         * @details Bit 7 - Sound Channel 3 Off  (0 = Stop, 1 = Playback)  (Read/Write)
         */
        byte_t NR30;

        /**
         * @brief NR31 register (R/W)
         * 
         * @details Bit 7-0 - Sound length (t1: 0 - 255)
         */
        byte_t NR31;

        /**
         * @brief NR32 register (R/W)
         * 
         * @details Bits 6-5 - Select output level (Read/Write)
         */
        byte_t NR32;

        /**
         * @brief NR33 register (W)
         * 
         * @details Lower 8 bits of an 11 bit frequency (x).
         */
        byte_t NR33;

        /**
         * @brief NR34 register (R/W)
         * 
         * @details Bit 7   - Initial (1 = Restart Sound)   (Write Only)
         *          Bit 6   - Counter/consecutive selection (Read/Write) (1 = Stop output when length in NR31 expires)`
         *          Bit 2-0 - Frequency's higher 3 bits (x) (Write Only)
         * 
         * @note Frequency = 4194304/(64*(2048-x)) Hz = 65536/(2048-x) Hz
         */
        byte_t NR34;
    };

    protected:
    Registers _registers;

    public: 
    /** Set NR30 register value */
    void set_NR30_reg(byte_t value);

    /** Get NR30 register value */
    byte_t get_NR30_reg() const;

    /** Set NR31 register value */
    void set_NR31_reg(byte_t value);

    /** Get NR31 register value */
    byte_t get_NR31_reg() const;

    /** Set NR32 register value */
    void set_NR32_reg(byte_t value);

    /** Get NR32 register value */
    byte_t get_NR32_reg() const;

    /** Set NR33 register value */
    void set_NRe3_reg(byte_t value);

    /** Get NR33 register value */
    byte_t get_NR33_reg() const;

    /** Set NR34 register value */
    void set_NR34_reg(byte_t value);

    /** Get NR34 register value */
    byte_t get_NR34_reg() const;
};

inline byte_t
SoundChannel3::get_NR30_reg() const {
    return _registers.NR30 | Registers::REG_RESERVED_NR30_BIT_MASK;
}

inline byte_t
SoundChannel3::get_NR31_reg() const {
    return _registers.NR31; // TODO(godflight) Length counter unit implementation 
}

inline byte_t
SoundChannel3::get_NR32_reg() const {
    return _registers.NR32 | Registers::REG_RESERVED_NR32_BIT_MASK;
}

inline byte_t
SoundChannel3::get_NR33_reg() const {
    return 0xFF;
}

inline byte_t
SoundChannel3::get_NR34_reg() const {
    return _registers.NR34 | Registers::REG_RESERVED_NR34_BIT_MASK;
}

}

#endif //DEVICE_GB_SOUND_CHANNEL_3_H_