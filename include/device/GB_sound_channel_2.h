
/**
 * @file GB_sound_channel_2.h
 * 
 * @brief Describes Sound Channel Two controller class
 */

#ifndef DEVICE_GB_SOUND_CHANNEL_2_H_
# define DEVICE_GB_SOUND_CHANNEL_2_H_

# include "GB_config.h"

# include "common/GB_types.h"
# include "common/GB_macro.h"

namespace GB::device {
    /**
     * @brief Implementation of Sound Channel Two from GameBoy
     */

class SoundChannel2 {
    public:

    /**
     * @brief Sound channnel 2 register's
     */

    struct Registers {

        constexpr static unsigned REG_RESERVED_NR24_BIT_MASK     = ::bit_mask(6, 6);

        /**
         * @brief NR21 register (R/W)
         * 
         * @details Bit 7-6 - Wave Pattern Duty (Read/Write)
         *          Bit 5-0 - Sound length data (Write Only) (t1: 0-63)
         */
        byte_t NR21;

        /**
         * @brief NR22 register (R/W)
         * 
         * @details Bit 7-4 - Initial Volume of envelope (0-0Fh) (0 = No Sound)
         *          Bit 3   - Envelope Direction (0 = Decrease, 1 = Increase)
         *          Bit 2-0 - Number of envelope sweep (n:§ 0-7) (If zero, stop envelope operation.)
         */
        byte_t NR22;

        /**
         * @brief NR23 register (W)
         * 
         * @details Frequency’s lower 8 bits of 11 bit data (x). Next 3 bits are in NR24 ($FF19)
         */
        byte_t NR23;

        /**
         * @brief NR24 register (R/W)
         * 
         * @details Bit 7   - Initial (1 = Restart Sound)   (Write Only)
         *          Bit 6   - Counter/consecutive selection (Read/Write) (1 = Stop output when length in NR21 expires)
         *          Bit 2-0 - Frequency's higher 3 bits (x) (Write Only)
         */
        byte_t NR24;
    };

    protected:
    Registers _registers;

    public: 

    /** Set NR21 register value */
    void set_NR21_reg(byte_t value);

    /** Get NR21 register value */
    byte_t get_NR21_reg() const;

    /** Set NR22 register value */
    void set_NR22_reg(byte_t value);

    /** Get NR22 register value */
    byte_t get_NR22_reg() const;

    /** Set NR23 register value */
    void set_NR23_reg(byte_t value);

    /** Get NR23 register value */
    byte_t get_NR23_reg() const;

    /** Set NR24 register value */
    void set_NR24_reg(byte_t value);

    /** Get NR24 register value */
    byte_t get_NR24_reg() const;
};

inline byte_t
SoundChannel2::get_NR21_reg() const {
    return _registers.NR21; // TODO(godflight) Length counter unit implementation 
}

inline byte_t
SoundChannel2::get_NR22_reg() const {
    return _registers.NR22; // TODO(godflight) Envelope unit implementation
}

inline byte_t
SoundChannel2::get_NR23_reg() const {
    return 0xFF;
}

inline byte_t
SoundChannel2::get_NR24_reg() const {
    return _registers.NR24 | Registers::REG_RESERVED_NR24_BIT_MASK;
}

}

#endif //DEVICE_GB_SOUND_CHANNEL_2_H_