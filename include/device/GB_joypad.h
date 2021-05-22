/**
 * @file GB_joypad.h
 * 
 * @brief Describes joypad model
 */

#ifndef DEVICE_GB_JOYPAD_H_
# define DEVICE_GB_JOYPAD_H_

# include "common/GB_types.h"
# include "common/GB_macro.h"

# include "device/GB_interrupt.h"

namespace GB::device {

class JoyPad {
 public:
    enum KeyIdx: u16 {
        A_KEY = 0,      // P10 [P15=0]
        B_KEY = 1,      // P11 [P15=0]
        SC_KEY = 2,     // P12 [P15=0]
        ST_KEY = 3,     // P13 [P15=0]
        RT_KEY = 4,     // P10 [P14=0]
        LF_KEY = 5,     // P11 [P14=0]
        UP_KEY = 6,     // P12 [P14=0]
        DW_KEY = 7      // P13 [P14=0]
    };

    enum PortBitIdx: u16 {
        P10 = 0,
        P11 = 1,
        P12 = 2,
        P13 = 3,
        P14 = 4,
        P15 = 5
    };

    constexpr static unsigned P1_RESERVED_BITS = Reg8(~(::bits_set(P10, P11, P12, P13, P14, P15)));

 protected:
    InterruptController*    __interrupt_link;
    byte_t                  __previous_step_key_set;
    byte_t                  __pressed_key_set;
    bool                    __p14;
    bool                    __p15;

 protected:
    void __raise_interrupt();

 public:

    explicit
    JoyPad(InterruptController* ic_link = nullptr)  : __interrupt_link(ic_link)
                                        , __previous_step_key_set(0)
                                        , __pressed_key_set(0)
                                        , __p14(true)
                                        , __p15(false) {}

    void unpress_key(KeyIdx keyIdx);
    void press_key(KeyIdx keyIdx);

    void set_P1_reg(byte_t value);
    byte_t get_P1_reg() const;

    void step();

};

inline void
JoyPad::step() {
    /* NOTE:
        * If a matrix line makes a HIGH->LOW voltage change, then it must raise an interrupt.
        * In other words, Joypad must raise the interrupt when a key makes UNPRESSED->PRESSED state change.
        * 
        * ~prevPressed => Mask for all previously unpressed keys
        *  & & & & & & => bitand
        *  currPressed => Mask for all currently pressed keys
        * =================
        * UNPRESSED->PRESSED mask
        */
    const byte_t previously_unpressed = byte_t(~__previous_step_key_set);
    if ((previously_unpressed & __pressed_key_set) != 0) {
        __raise_interrupt();
    }
    __previous_step_key_set = __pressed_key_set;
}

inline void
JoyPad::__raise_interrupt() {
    __interrupt_link->request_interrupt(GB::device::InterruptController::JOYPAD_INT);
}

inline void
JoyPad::press_key(KeyIdx key_idx) {
    __pressed_key_set = ::bit_n_set(key_idx, __pressed_key_set);
}

inline void
JoyPad::unpress_key(KeyIdx key_idx) {
    __pressed_key_set = ::bit_n_reset(key_idx, __pressed_key_set);
}

inline void
JoyPad::set_P1_reg(byte_t value) {
    __p14 = ::bit_n(P14, value);
    __p15 = ::bit_n(P15, value);
}

inline byte_t
JoyPad::get_P1_reg() const {
    /* NOTE:
        *  Lines: UUU UUU P15 P14 P13 P12 P11 P10
        * Values: X   X   0   0   X   X   X   X    ==> Undefined (I think that we must return all bits set to 1)
        * Values: X   X   0   1   X   X   X   X    ==> action keys output
        * Values: X   X   1   0   X   X   X   X    ==> arrow keys output
        * Values: X   X   1   1   X   X   X   X    ==> Undefined (I think that we must return all bits set to 1)
        * 
        * For faster output, I construct uint32 value, which:
        *     1111 1111 1111 1111 1111  ARRS  ACTN  1111 _____________
        *     / Bad values area      |  |  |  |  \  | Bad values area \
        *           ___________________/   |  |   \________________
        *          / Direction values area |  | Action values area \
        * 
        * So we can have a 4bit wide value getter window, [N:N+3] where N could have the next values:
        * N=0 when P15=0 and P14=0, so we want to get an undefined value
        * N=1 when P15=0 and P14=1, so we want to get the action buttons value
        * N=2 when P15=1 and P14=0, so we want to get the direction buttons value
        * N=3 when P15=1 and P15=3, so we want to get an undefined value
        * 
        * As we can see from the above, we can make a formula responds to all our needs in calculation that N:
        *          N = (P15 << 1) + P14
        * 
        * Also we need to invert __pressedKeySet, because if keys is pressed, then corresponding
        * lines (P1X) have value which equals to 0.
        */

    // zero expand pressedKeys, shift it and then invert it
    const u32 search_field = ~(u32(__pressed_key_set) << 4);
    const u32 search_window_lsb = ((u32(__p15) << 1) | u32(__p14)) * 4;
    return ::bit_slice(search_window_lsb + 3, search_window_lsb, search_field)
            | P1_RESERVED_BITS
            | u32(__p14) << P14
            | u32(__p15) << P15;
}

}  // namespace GB::device

#endif  // DEVICE_GB_JOYPAD_H_
