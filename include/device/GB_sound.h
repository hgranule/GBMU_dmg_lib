
/**
 * @file GB_sound.h
 * 
 * @brief Describes Sound controller class
 */

#ifndef DEVICE_GB_SOUND_H_
# define DEVICE_GB_SOUND_H_

# include "GB_config.h"

# include "common/GB_types.h"
# include "common/GB_macro.h"

# include "GB_sound_channel_1.h"

namespace GB::device {
    /**
     * @brief Implementation of sound controller from GameBoy
     * 
     * @note NRXY - sound register's where X - number of channel(1-4), Y - number of register of the channel
     */

class SoundController {

    public:

    /**
     * @brief Sound 1 unit register's
     */

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

    /**
     * @brief Sound 2 unit register's
     */

    /** Set NR20 register value */
    void set_NR20_reg(byte_t value);

    /** Get NR20 register value */
    byte_t get_NR20_reg() const;

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

    /**
     * @brief Sound 3 unit register's
     */

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
    void set_NR33_reg(byte_t value);

    /** Get NR33 register value */
    byte_t get_NR33_reg() const;

    /** Set NR34 register value */
    void set_NR34_reg(byte_t value);

    /** Get NR34 register value */
    byte_t get_NR34_reg() const;

    /**
     * @brief Sound 4 unit register's
     */

    /** Set NR40 register value */
    void set_NR40_reg(byte_t value);

    /** Get NR40 register value */
    byte_t get_NR40_reg() const;

    /** Set NR41 register value */
    void set_NR41_reg(byte_t value);

    /** Get NR41 register value */
    byte_t get_NR41_reg() const;

    /** Set NR42 register value */
    void set_NR42_reg(byte_t value);

    /** Get NR42 register value */
    byte_t get_NR42_reg() const;

    /** Set NR43 register value */
    void set_NR43_reg(byte_t value);

    /** Get NR43 register value */
    byte_t get_NR43_reg() const;

    /** Set NR44 register value */
    void set_NR44_reg(byte_t value);

    /** Get NR44 register value */
    byte_t get_NR44_reg() const;

    /**
     * @brief General register's
     * 
     * @details NR50 and NR51 are general registers.
     *          In addition to them, there is a general register NR52, which contains a flag
     *          for mute all sound, as well as bits showing the status of audio channels.
     *          Only the mute flag can be changed. The status bits are read-only and are constantly updated.
     * 
     * @note If the sound in register NR52 is muted, then the following happens:
     * 
     *       1)All registers are cleared except for the "Length counter".
     *       This means that only the bits related to the "Duty cycle" need to be cleared.
     *       2)Writing to all registers except NRX1 is prohibited. 
     *       Moreover, writing can be carried out only in those bits
     *       that relate to the Length counter.
     */

    /** Set NR50 register value */
    void set_NR50_reg(byte_t value);

    /** Get NR50 register value */
    byte_t get_NR50_reg() const;

    /** Set NR51 register value */
    void set_NR51_reg(byte_t value);

    /** Get NR51 register value */
    byte_t get_NR51_reg() const;

    private:
        SoundChannel1 sh1;
    };

inline byte_t
SoundController::get_NR10_reg() const {
    return sh1.get_NR10_reg();
}

inline byte_t
SoundController::get_NR11_reg() const {
    return sh1.get_NR11_reg();
}

inline byte_t
SoundController::get_NR12_reg() const {
    return sh1.get_NR12_reg();
}

inline byte_t
SoundController::get_NR13_reg() const {
    return sh1.get_NR13_reg();
}

inline byte_t
SoundController::get_NR14_reg() const {
    return sh1.get_NR14_reg();
}

} // namespace GB::device



# endif // DEVICE_GB_SOUND_H_