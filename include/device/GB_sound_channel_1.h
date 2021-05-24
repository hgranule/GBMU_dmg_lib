
/**
 * @file GB_sound_channel_1.h
 * 
 * @brief Describes Sound Channel One controller class
 */

#ifndef DEVICE_GB_SOUND_CHANNEL_1_H_
# define DEVICE_GB_SOUND_CHANNEL_1_H_

# include "GB_config.h"

# include "common/GB_types.h"
# include "common/GB_macro.h"

namespace GB::device {
    /**
     * @brief Implementation of Sound Channel One from GameBoy
     */

class SoundChannel1 {
    public:

    /**
     * @brief Sound channnel 1 register's
     */

    struct Registers {

        constexpr static unsigned REG_RESERVED_NR11_BIT_MASK     = ::bit_mask(7, 6);
        constexpr static unsigned REG_RESERVED_NR14_BIT_MASK     = ::bit_mask(6, 6);

        /**
         * @brief NR10 register (R/W)
         * 
         * @details Bit 6-4 - Sweep Time
         *          Bit 3   - Sweep Increase/Decrease
         *                    0: Addition    (frequency increases)
         *                    1: Subtraction (frequency decreases)
         *          Bit 2-0 - Number of sweep shift (n: 0-7)
         */
        byte_t NR10;

        /**
         * @brief NR11 register (R/W)
         * 
         * @details Bit 7-6 - Wave Pattern Duty (Read/Write)
         *          Bit 5-0 - Sound length data (Write Only) (t1: 0-63)
         */
        byte_t NR11;

        /**
         * @brief NR12 register (R/W)
         * 
         * @details Bit 7-4 - Initial Volume of envelope (0-0Fh) (0=No Sound)
         *          Bit 3   - Envelope Direction (0=Decrease, 1=Increase)
         *          Bit 2-0 - Number of envelope sweep (n: 0-7) (If zero, stop envelope operation.)
         *          Bit 5-0 - Sound length data (Write Only) (t1: 0-63)
         */
        byte_t NR12;

        /**
         * @brief NR13 register (W)
         * 
         * @details   Lower 8 bits of 11 bit frequency (x). Next 3 bit are in NR14 ($FF14)
         */
        byte_t NR13;

        /**
         * @brief NR14 register (R/W)
         * 
         * @details Bit 7   - Initial (1=Restart Sound)     (Write Only)
         *          Bit 6   - Counter/consecutive selection (Read/Write). (1 = Stop output when length in NR11 expires)`
         *          Bit 2-0 - Frequency's higher 3 bits (x) (Write Only)
         */
        byte_t NR14;
    };

    protected:
    Registers _registers;

    public: 
    /** Set NR10 register value */
    void set_NR10_reg(byte_t value);

    /** Get NR10 register value */
    byte_t get_NR10_reg() const;

    /** Set NR11 register value */
    void set_NR11_reg(byte_t value);

    /** Get NR11 register value */
    byte_t get_NR11_reg() const;

    /** Set NR12 register value */
    void set_NR12_reg(byte_t value);

    /** Get NR12 register value */
    byte_t get_NR12_reg() const;

    /** Set NR13 register value */
    void set_NR13_reg(byte_t value);

    /** Get NR13 register value */
    byte_t get_NR13_reg() const;

    /** Set NR14 register value */
    void set_NR14_reg(byte_t value);

    /** Get NR14 register value */
    byte_t get_NR14_reg() const;
};

inline byte_t
SoundChannel1::get_NR10_reg() const {
    return _registers.NR10; // TODO(godflight) Sweep unit implementation
}

inline byte_t
SoundChannel1::get_NR11_reg() const {
    return _registers.NR11 | Registers::REG_RESERVED_NR11_BIT_MASK;
}

inline byte_t
SoundChannel1::get_NR12_reg() const {
    return _registers.NR12; // TODO(godflight) Envelope unit implementation
}

inline byte_t
SoundChannel1::get_NR13_reg() const {
    return 0xFF;
}

inline byte_t
SoundChannel1::get_NR14_reg() const {
    return _registers.NR14 | Registers::REG_RESERVED_NR14_BIT_MASK;
}

}

#endif //DEVICE_GB_SOUND_CHANNEL_1_H_