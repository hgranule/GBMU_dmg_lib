
/**
 * @file GB_sound_length_counter.h
 * 
 * @brief Describes Sweep Unit controller class
 */

#ifndef DEVICE_GB_SOUND_SWEEP_UNIT
# define DEVICE_GB_SOUND_SWEEP_UNIT

# include "GB_config.h"

# include "common/GB_types.h"
# include "common/GB_macro.h"

constexpr unsigned int FREQUENCY_PEAK = 2047;

namespace GB::device {
    /**
     * @brief Implementation of sound Sweep Unit from GameBoy
     * 
     * @details Changes the sound frequency with a specified period and step
     * 
     * @note Sweep Time:
     *              000	sweep off - no freq change
     *              001	7.8 ms (1/128Hz)
     *              010	15.6 ms (2/128Hz)
     *              011	23.4 ms (3/128Hz)
     *              100	31.3 ms (4/128Hz)
     *              101	39.1 ms (5/128Hz)
     *              110	46.9 ms (6/128Hz)
     *              111	54.7 ms (7/128Hz)
     */

class SweepUnit {
public:

    explicit
    SweepUnit()
    {
        _sweep_counter = 0;
        _internal_enabled_flag_reg = false;
    }

    void Step(const int frame_sequencer_step);

    byte_t get_NRX0_reg() const;

    void set_NRX0_reg(const byte_t value);

    byte_t get_NRX4_reg() const;

    void set_NRX4_reg(const byte_t value);

private:
/**
 * @note Next frequency value at the next reading: F(t) = F(t – 1) ± F(t – 1) / 2^n
 *       F (t) - next frequency value
 *       F (t - 1) - current frequency value
 *       n - step value from NRX0 register
 */
    int CalculateFrequency();

    byte_t _NRX0;
    byte_t _NRX3;
    byte_t _NRX4;

    int _current_frame_sequencer_step;
    int _sweep_counter;

    /**
     * @note Also sweep period
     */
    int _sweep_time;
    int _sweep_mode;
    int _sweep_shift;
    /**
     * @brief activity flag
     */
    bool _internal_enabled_flag_reg;

    /**
     * @brief register-buffer for frequency
     */
    int _frequency_shadow_reg;

    byte_t _mask_NRX0_time;
    byte_t _mask_NRX0_mode;
    byte_t _mask_NRX0_shift;
    byte_t _mask_NRX4_restart_sound;
    byte_t _mask_NRX4_higher_bits;

    byte_t _status_bit;

};

inline void
SweepUnit::Step(const int frame_sequencer_step) {
    _current_frame_sequencer_step = frame_sequencer_step;

    if (_current_frame_sequencer_step != 0 && _current_frame_sequencer_step % 4 != 0) {
        return;
    }

    _sweep_counter--;

    if (_sweep_time <= 0) {
        if (_sweep_counter <= 0) {
            _sweep_counter = _sweep_time;

            int current_frequency = CalculateFrequency(); // F(t - 1)

            if (current_frequency <= FREQUENCY_PEAK && _sweep_shift) {
                _NRX3 = ::bit_slice(8, 0, _NRX3);
                _NRX4 = (_NRX4 & ~REG_NRX4_SWEEP_HIGHER_BITS_BIT_MASK) | ::bit_slice(2, 0, current_frequency);
                _frequency_shadow_reg = current_frequency;
                CalculateFrequency();
            }
        }
    }
}

inline void
SweepUnit::set_NRX0_reg(const byte_t value) {
    _sweep_mode = value & _mask_NRX0_mode;
    _sweep_time = value & _mask_NRX0_time;
    _sweep_shift = value & _mask_NRX0_shift;
    _NRX0 = value;
}

inline void
SweepUnit::set_NRX4_reg(const byte_t value) {
    if (value & _mask_NRX4_restart_sound) {
        _frequency_shadow_reg = ((_NRX4 & _mask_NRX4_higher_bits) << 8) | _NRX3;

        _internal_enabled_flag_reg = _sweep_shift || _sweep_time;

        if (_sweep_shift) {
            CalculateFrequency();
        }
    }
}

inline int
SweepUnit::CalculateFrequency() {
    int new_frequency = _frequency_shadow_reg;
    int freqeucny_shift = _frequency_shadow_reg >> _sweep_shift; // F(t - 1) / 2^n
    
    if (!_sweep_mode) { // Addition mode
        new_frequency = _frequency_shadow_reg + freqeucny_shift;
    }
    else if (freqeucny_shift < _frequency_shadow_reg) { // Subtraction mode
        new_frequency = _frequency_shadow_reg - freqeucny_shift;
    }

    if (new_frequency > FREQUENCY_PEAK) {
        _status_bit = 0;
    }

    return new_frequency;
}

}

#endif //DEVICE_GB_SOUND_SWEEP_UNIT