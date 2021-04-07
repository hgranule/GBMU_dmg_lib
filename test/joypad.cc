#include "gtest/gtest.h"

#include "common/GB_macro.h"
#include "device/GB_joypad.h"
#include "memory/GB_bus.h"
#include "memory/GB_vaddr.h"

namespace {

    using JoyPad = GB::device::JoyPad;
    using IntController = GB::device::InterruptController;
    
    class JoyPadMock: public JoyPad {
    public:
        JoyPadMock(IntController* icPtr) : JoyPad(icPtr)
        {
        }

        Byte __GetPressedKeys() const {
            return this->__pressedKeySet;
        }
    };

    // Reg8 ReservedP1Bits(Reg8 reg) {
    //     return JoyPad::P1_RESERVED_BITS | reg;
    // }

    TEST(Joypad, KeyManagment) {
        IntController   ic;
        JoyPadMock      jp(&ic);
    }

    TEST(Joypad, MatrixLineP1) {

    }

    TEST(Joypad, Interrupts) {

    }

    TEST(Joypad, MemoryMapping) {
    }
    
}