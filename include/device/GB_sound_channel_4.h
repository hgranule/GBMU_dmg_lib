
/**
 * @file GB_sound_channel_3.h
 * 
 * @brief Describes Sound Channel Four controller class
 */

#ifndef DEVICE_GB_SOUND_CHANNEL_4_H_
# define DEVICE_GB_SOUND_CHANNEL_4_H_

# include "GB_config.h"

# include "common/GB_types.h"
# include "common/GB_macro.h"

namespace GB::device {
    /**
     * @brief Implementation of Sound Channel Four from GameBoy
     */

class SoundChannel3 {
    public:

    /**
     * @brief Sound channnel 4 register's
     */

    struct Registers {

        constexpr static unsigned REG_RESERVED_NR44_BIT_MASK     = ::bit_mask(6, 6);

        /**
         * @brief NR41 register (R/W)
         * 
         * @details Bit 5-0 - Sound length data (t1: 0-63)
         */
        byte_t NR41;

        /**
         * @brief NR42 register (R/W)
         * 
         * @details  Bit 7-4 - Initial Volume of envelope (0-0Fh) (0 = No Sound)
         *           Bit 3   - Envelope Direction (0 = Decrease, 1 = Increase)
         *           Bit 2-0 - Number of envelope sweep (n: 0-7) (If zero, stop envelope operation.)
         */
        byte_t NR42;

        /**
         * @brief NR43 register (R/W)
         * 
         * @details Bit 7-4 - Shift Clock Frequency (s)
         *          Bit 3   - Counter Step/Width (0 = 15 bits, 1 = 7 bits)
         *          Bit 2-0 - Dividing Ratio of Frequencies (r)
         */
        byte_t NR43;

        /**
         * @brief NR44 register (R/W)
         * 
         * @details Bit 7   - Initial (1=Restart Sound)     (Write Only)
                    Bit 6   - Counter/consecutive selection (Read/Write) ( 1 =Stop output when length in NR41 expires)
         */
        byte_t NR44;
    };

    protected:
    Registers _registers;

    public: 

    /** Set NR41 register value */
    void set_NR41_reg(byte_t value);

    /** Get NR41 register value */
    byte_t get_NR41_reg() const;

    /** Set NR42 register value */
    void set_NR42_reg(byte_t value);

    /** Get NR42 register value */
    byte_t get_NR42_reg() const;

    /** Set NR43 register value */
    void set_NRe3_reg(byte_t value);

    /** Get NR43 register value */
    byte_t get_NR43_reg() const;

    /** Set NR44 register value */
    void set_NR44_reg(byte_t value);

    /** Get NR44 register value */
    byte_t get_NR44_reg() const;
};

inline byte_t
SoundChannel3::get_NR41_reg() const {
    return _registers.NR41; // TODO(godflight) Length counter unit implementation 
}

inline byte_t
SoundChannel3::get_NR42_reg() const {
    return _registers.NR42; // TODO(godflight) Envelope unit implementation
}

inline byte_t
SoundChannel3::get_NR43_reg() const {
    return _registers.NR43; // TODO(godflight) Polynomial counter implementation 
}

inline byte_t
SoundChannel3::get_NR44_reg() const {
    return _registers.NR44 | Registers::REG_RESERVED_NR44_BIT_MASK;
}

}

#endif //DEVICE_GB_SOUND_CHANNEL_4_H_