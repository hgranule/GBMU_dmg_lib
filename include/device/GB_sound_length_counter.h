
/**
 * @file GB_sound_length_counter.h
 * 
 * @brief Describes Length Counter controller class
 */

#ifndef DEVICE_GB_SOUND_LENGTH_COUNTER_H_
# define DEVICE_GB_SOUND_LENGTH_COUNTER_H_

# include "GB_config.h"

# include "common/GB_types.h"
# include "common/GB_macro.h"

namespace GB::device {
    /**
     * @brief Implementation of sound Length Counter from GameBoy
     *
     * @details Controls the duration of the audio output
     */

class LengthCounter {
public:

    explicit
    LengthCounter(byte_t mask_NRX1, byte_t mask_NRX4, byte_t restart_sound_mask) 
    : _mask_NRX1(mask_NRX1)
    , _mask_NRX4(mask_NRX4)
    , _restart_sound_mask(restart_sound_mask)
    {
        Reset();
    }

    void Step(const int frame_sequencer_step);

    void Reset();

    byte_t get_NRX1_reg() const;

    void set_NRX1_reg(const byte_t value);

    byte_t get_NRX4_reg() const;

    void set_NRX4_reg(const byte_t value);


private:

    void CalculateCounter();
    void ResetCounter();

    byte_t _NRX1;
    byte_t _NRX4;
    
    /**
     * @brief Bit 6 of NRX4 register
     */
    byte_t _counter_mode;

    /**
     * @brief Mask with which only the duration value is retrieved from the register
     */
    byte_t _mask_NRX1;
    byte_t _mask_NRX4;
    /**
     * @brief Bit 7 of NRX4 register
     */
    byte_t _restart_sound_mask;
    int _counter;
    int _current_frame_sequencer_step;
};

inline void
LengthCounter::Step(const int frame_sequencer_step) {
    _current_frame_sequencer_step = frame_sequencer_step;
    
    if (_current_frame_sequencer_step % 2 == 0) {
        return;
    }

    if (_counter_mode) {
        CalculateCounter();
    }
}

inline void
LengthCounter::Reset() {
    _counter = 0;
    _NRX1 = 0;
    _counter_mode = 0;
    _current_frame_sequencer_step = 0;
}

inline byte_t
LengthCounter::get_NRX1_reg() const {
        return _NRX1 | _mask_NRX1;
}

inline void
LengthCounter::set_NRX1_reg(const byte_t value) {
    _NRX1 = value;

    ResetCounter();   
}

inline void
LengthCounter::set_NRX4_reg(const byte_t value) {
    bool frame_sequencer_is_counting = _current_frame_sequencer_step % 2;

    /**
     * @note If the component was turned off and is now turning on,
     *       the counter has not reached zero, and the current state of the Frame Sequencer is counting the component,
     *       then we immediately count. This can lead to the counter reaching zero and the channel will be disconnected,
     *       but one more condition must be taken into account here - the channel is disconnected here only
     *       if the channel is not restarted, i.e. restart bit is zero.
     */
    if (_counter && _counter_mode && (value & _mask_NRX4)
        && frame_sequencer_is_counting) {
        CalculateCounter();
    }

    /**
     * @note If a restart is carried out and the counter reaches zero,
     *       then the maximum possible duration value is written to the NRX1 register,
     *       i.e. all duration bits are cleared. If, at the same time,
     *       the component is turned on and the current state of the Frame Sequencer is counting the component,
     *       then we immediately count it.
     */
    if ((value & _restart_sound_mask) && !_counter) {
        set_NRX1_reg(~_NRX1 & _mask_NRX1);

        if ((value & _mask_NRX4) && !frame_sequencer_is_counting) {
            CalculateCounter();
        }
    }

    _NRX4 = value;
    _counter_mode = value & _mask_NRX4;
}

inline byte_t
LengthCounter::get_NRX4_reg() const {
    return _NRX4 | _mask_NRX4;
}

inline void
LengthCounter::ResetCounter() {
    _counter =  (~_NRX1 & _mask_NRX1) + 1;
}

inline void
LengthCounter::CalculateCounter() {
    _counter--;

    if (_counter_mode <= 0) {
        _counter = 0;
    }
    // (64 - t1) * (1/256)
    _NRX1 = (_NRX1 & (~_mask_NRX1)) | (~(_counter - 1) & _mask_NRX1);
}


}

#endif //DEVICE_GB_SOUND_LENGTH_COUNTER_H_