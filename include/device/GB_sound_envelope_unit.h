
/**
 * @file GB_sound_envelope_unit.h
 * 
 * @brief Describes Envelope Unit controller class
 */

#ifndef DEVICE_GB_SOUND_ENVELOPE_UNIT
# define DEVICE_GB_SOUND_ENVELOPE_UNIT

# include "GB_config.h"

# include "common/GB_types.h"
# include "common/GB_macro.h"

namespace GB::device {
    /**
     * @brief Implementation of sound Envelope Unit from GameBoy
     * 
     * @details This component controls the volume of the sound,
     *          decreasing or increasing it in constant steps with a certain period.
     *          Loudness in this case means the amplitude of the generated signal.
     *          This component is controlled by register NRX2
     *
     *
     *                 ------
     *                |      | ------
     *    trigger     |      |       | ------
     *           \    |      |       |       | ------
     *            \   |      |       |       |       | ------
     *             -> |      |       |       |       |       |
     *               ===========================================
     *                |      |       |       |       |       |
     *                |      |       |       |       |       |
     *                <------><------><------><------><------>
     *                 78.1ms  78.1ms  78.1ms  78.1ms  78.1ms
     * 
     *      The figure above shows the graph of the signal amplitude
     *      change when the Envelope unit is operating with the NRX2 value equal to 0x55.
     *      The initial value of the amplitude is set when
     *      the channel is restarted (trigger) - in this case it is equal to 5.
     *      During operation, it is no longer used or modified.
     *      Further, with each period, the amplitude decreases by one until it reaches zero.
     * 
     * 
     */

class EnvelopeUnit {
public:

    explicit
    EnvelopeUnit(byte_t mask_NRX2_initial_volume, byte_t mask_NRX2_mode, byte_t mask_NRX2_sweep_number, byte_t mask_NRX4_restart_sound)
    : _mask_NRX2_initial_volume(mask_NRX2_initial_volume)
    , _mask_NRX2_mode(mask_NRX2_mode)
    , _mask_NRX2_sweep_number(mask_NRX2_sweep_number)
    , _mask_NRX4_restart_sound(mask_NRX4_restart_sound)
    {
        _sweep_number = 0;
        _mode = 0;
        _initial_volume = 0;
        _current_voulume = 0;
    }

    void Step(const int frame_sequencer_step) {
        _current_frame_sequencer_step = frame_sequencer_step;
        
        if (_current_frame_sequencer_step != 1) {
            return;
        }

        _counter--;

        if (_sweep_number) {
            if (_mode && _current_voulume < 0xF) { // Additional mode
                _current_voulume++;
            }
            else if (!_mode && _current_voulume > 0) { // Subtraction mode 
                _current_voulume--;
            }
        }
        
        if (_counter <= 0) {
            _counter = 8;
        }

    }

    byte_t get_NRX2_reg() {
        return _NRX2;
    }

    void set_NRX2_reg(const byte_t value) {
        if (_sweep_number == 0 && _counter != 0) {
            _current_voulume++;
        }
        else if (!_mode) {
            _current_voulume += 2;
        }

        if ((value ^ _NRX2) & _mask_NRX2_mode) {
            _current_voulume = 16 - _current_voulume;
        }

        _current_voulume &= 0xF;

        _sweep_number = value & _mask_NRX2_sweep_number;
        _mode = value & _mask_NRX2_mode;
        _initial_volume = value & _mask_NRX2_initial_volume;
    }

    void set_NRX4_reg(const byte_t value) {
        
        if (_counter == 0) {
            _counter = 8;
        }

        if (value & _mask_NRX4_restart_sound) {

            if (_current_frame_sequencer_step == 0) {
                _counter++;
            }
        }
    }

private:

    int _current_frame_sequencer_step;
    int _initial_volume;
    int _mode;
    int _sweep_number;
    int _current_voulume; // min - 0, max 0xF
    byte_t _NRX2;

    byte_t _counter_mode;
    
    byte_t _mask_NRX2_initial_volume;
    byte_t _mask_NRX2_mode;
    byte_t _mask_NRX2_sweep_number;
    byte_t _mask_NRX4_restart_sound;

    int _counter;

};

}

#endif //DEVICE_GB_SOUND_ENVELOPE_UNIT