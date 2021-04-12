/**
 * @file GB_joypad.h
 * 
 * @brief Describes joypad model
 */

#ifndef GB_JOYPAD_H_
# define GB_JOYPAD_H_

# include <atomic>

# include "common/GB_types.h"
# include "common/GB_macro.h"

# include "memory/GB_bus.h"

# include "device/GB_interrupt.h"

namespace GB::device {

    class JoyPad {
    public:
        enum KeyIdx: U16 {
            A_KEY = 0,      // P10 [P15=0]
            B_KEY = 1,      // P11 [P15=0]
            SC_KEY = 2,     // P12 [P15=0]
            ST_KEY = 3,     // P13 [P15=0]
            RT_KEY = 4,     // P10 [P14=0]
            LF_KEY = 5,     // P11 [P14=0]
            UP_KEY = 6,     // P12 [P14=0]
            DW_KEY = 7      // P13 [P14=0]
        };

        enum PortBitIdx: U16 {
            P10 = 0,
            P11 = 1,
            P12 = 2,
            P13 = 3,
            P14 = 4,
            P15 = 5
        };

        constexpr static unsigned P1_RESERVED_BITS = Reg8(~(::bits_set(P10, P11, P12, P13, P14, P15)));

    protected:
        InterruptController*    __interruptLink;
        Byte                    __previousStepPKEYS;
        Byte                    __pressedKeySet;
        bool                    __p14;
        bool                    __p15;

    public:

        JoyPad(InterruptController* icPtr)  : __interruptLink(icPtr)
                                            , __pressedKeySet(0)
                                            , __p14(true)
                                            , __p15(false) {}

        void __RaiseInterrupt();

    public:
        void UnpressKey(KeyIdx keyIdx);
        void PressKey(KeyIdx keyIdx);

        void SetP1(Byte value);
        Byte GetP1() const;

        void Step(CLKCycle clockDelta);

        void MapToMemory(::GB::memory::BusInterface& memBus);

    };

    inline void
    JoyPad::Step(CLKCycle) {
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
        const Byte previouslyUnpressed = Byte(~__previousStepPKEYS);
        if ((previouslyUnpressed & __pressedKeySet) != 0) {
            __RaiseInterrupt();
        }
        __previousStepPKEYS = __pressedKeySet;
    }

    inline void
    JoyPad::__RaiseInterrupt() {
        __interruptLink->RequestInt(GB::device::InterruptController::JOYPAD_INT);
    }

    inline void
    JoyPad::PressKey(KeyIdx keyIdx) {
        __pressedKeySet = ::bit_n_set(keyIdx, __pressedKeySet);
    }

    inline void
    JoyPad::UnpressKey(KeyIdx keyIdx) {
        __pressedKeySet = ::bit_n_reset(keyIdx, __pressedKeySet);
    }

    inline void
    JoyPad::SetP1(Byte value) {
        __p14 = ::bit_n(P14, value);
        __p15 = ::bit_n(P15, value);
    }

    inline Byte
    JoyPad::GetP1() const {
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
        const unsigned searchField = ~(static_cast<unsigned>(__pressedKeySet) << 4); // zero expand pressedKeys, shift it and then invert it
        const unsigned searchWindowLsb = ((unsigned(__p15) << 1) | unsigned(__p14)) * 4;
        return ::bit_slice(searchWindowLsb + 3, searchWindowLsb, searchField)
               | P1_RESERVED_BITS
               | unsigned(__p14) << P14
               | unsigned(__p15) << P15;
    }

}

#endif
