
/**
 * @file GB_sound_length_counter.h
 * 
 * @brief Describes Wave Generator controller class
 */

#ifndef DEVICE_GB_SOUND_WAVE_GENERATOR
# define DEVICE_GB_SOUND_WAVE_GENERATOR

# include "GB_config.h"

# include "common/GB_types.h"
# include "common/GB_macro.h"


namespace GB::device {
    /**
     * @brief Implementation of sound Wave Generator from GameBoy
     */

class WaveGenerator {
public:

    explicit
    WaveGenerator(byte_t mask_restart_sound, byte_t mask_NRX4_higher_bits)
    : _mask_restart_sound(mask_restart_sound)
    , _mask_NRX4_higher_bits(mask_NRX4_higher_bits)
    {}

    void Step(const int delta); //sync with cpu

    void set_NRX1_reg(const byte_t value);
    void set_NRX3_reg(const byte_t value);
    void set_NRX4_reg(const byte_t value);

private:

    /**
     * @note Formula:  4 * (2048 – X)
     *       X - frequency from registers NRX3 and NRX4, F - sound frequency
     */
    void CalculatePeriod();

    /** @note Duty Cycle | Waveform of one period of a signal
     *         0.125     |  00000001
     *         0.25      |  10000001
     *         0.5       |  10000111
     *         0.75      |  01111110
     */
    const byte_t _duty_cycle[4][8]
    {
        {0, 0, 0, 0, 0, 0, 0, 1}, // 0.125
        {1, 0, 0, 0, 0, 0, 0, 1}, // 0.25
        {1, 0, 0, 0, 0, 1, 1, 1}, // 0.5
        {0, 1, 1, 1, 1, 1, 1, 0}  // 0.75
    };
    
    int _waveform_index; // _duty_cycle[_waveform_index][y]
    int _waveform_value; // _duty_cycle[x][_waveform_value]
    int _generator_counter;
    int _period;
    
    byte_t _NRX1;
    byte_t _NRX3;
    byte_t _NRX4;

    byte_t _mask_restart_sound;
    byte_t _mask_NRX4_higher_bits;
};

inline void
WaveGenerator::set_NRX1_reg(const byte_t value) {
    _NRX1 = value;
}

inline void
WaveGenerator::set_NRX3_reg(const byte_t value) {
    _NRX3 = value;
}

inline void
WaveGenerator::set_NRX4_reg(const byte_t value) {
    _NRX4 = value;
}

inline void
WaveGenerator::Step(const int delta) {
    _generator_counter += delta;

    // some logic that i dont know
}

inline void
WaveGenerator::CalculatePeriod() {
    _period = 4 * (2048 - ((_NRX4 & _mask_NRX4_higher_bits) | _NRX3));
}

}

#endif //DEVICE_GB_SOUND_WAVE_GENERATOR