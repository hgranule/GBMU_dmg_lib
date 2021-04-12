#include "gtest/gtest.h"

#include "common/GB_macro.h"
#include "device/GB_joypad.h"
#include "memory/GB_bus.h"
#include "memory/GB_vaddr.h"

namespace {

    using JoyPad = GB::device::JoyPad;
    using IntController = GB::device::InterruptController;
    using MemBus = GB::memory::BusInterface;
    using Vaddr = GB::memory::VirtualAddress;
    
    class JoyPadMock: public JoyPad {
    public:
        JoyPadMock(IntController* icPtr) : JoyPad(icPtr)
        {
        }

        Byte __GetPressedKeys() const {
            return this->__pressedKeySet;
        }

        void __ResetKeys() {
            this->__pressedKeySet = 0;
        }
    };

    Reg8 ReservedP1Bits(Reg8 reg) {
        return JoyPad::P1_RESERVED_BITS | reg;
    }

    TEST(Joypad, KeyManagment) {
        JoyPadMock      jp(NULL);
        Byte            keySet;

        jp.PressKey(JoyPad::A_KEY);
        jp.PressKey(JoyPad::B_KEY);
        jp.PressKey(JoyPad::UP_KEY);
        jp.PressKey(JoyPad::LF_KEY);

        keySet = ::bits_set(JoyPad::A_KEY, JoyPad::B_KEY, JoyPad::UP_KEY, JoyPad::LF_KEY);
        EXPECT_EQ(keySet, jp.__GetPressedKeys());

        jp.PressKey(JoyPad::A_KEY);
        jp.PressKey(JoyPad::DW_KEY);
        jp.PressKey(JoyPad::SC_KEY);

        keySet |= ::bits_set(JoyPad::A_KEY, JoyPad::DW_KEY, JoyPad::SC_KEY);
        EXPECT_EQ(keySet, jp.__GetPressedKeys());

        jp.UnpressKey(JoyPad::SC_KEY);
        jp.UnpressKey(JoyPad::ST_KEY);
        jp.UnpressKey(JoyPad::B_KEY);
        jp.UnpressKey(JoyPad::LF_KEY);

        keySet = keySet & ~(::bits_set(JoyPad::SC_KEY, JoyPad::ST_KEY, JoyPad::B_KEY, JoyPad::LF_KEY));
        EXPECT_EQ(keySet, jp.__GetPressedKeys());

        jp.__ResetKeys();

        keySet = 0;
        EXPECT_EQ(keySet, jp.__GetPressedKeys());

        jp.PressKey(JoyPad::RT_KEY);
        jp.PressKey(JoyPad::ST_KEY);

        keySet = ::bits_set(JoyPad::RT_KEY, JoyPad::ST_KEY);
        EXPECT_EQ(keySet, jp.__GetPressedKeys());

    }

    TEST(Joypad, MatrixLineP1) {
        JoyPadMock      jp(NULL);

        jp.__ResetKeys();
        jp.SetP1(0b100000);
        EXPECT_EQ(ReservedP1Bits(0b101111), jp.GetP1());

        jp.PressKey(JoyPad::RT_KEY);
        jp.PressKey(JoyPad::LF_KEY);
        EXPECT_EQ(ReservedP1Bits(0b101100), jp.GetP1());

        jp.SetP1(0b010111);
        EXPECT_EQ(ReservedP1Bits(0b011111), jp.GetP1());

        jp.PressKey(JoyPad::A_KEY);
        jp.PressKey(JoyPad::ST_KEY);
        EXPECT_EQ(ReservedP1Bits(0b010110), jp.GetP1());

        jp.PressKey(JoyPad::UP_KEY);
        EXPECT_EQ(ReservedP1Bits(0b010110), jp.GetP1());

        jp.SetP1(0b100000);
        EXPECT_EQ(ReservedP1Bits(0b101000), jp.GetP1());

        jp.UnpressKey(JoyPad::RT_KEY);
        jp.UnpressKey(JoyPad::UP_KEY);
        EXPECT_EQ(ReservedP1Bits(0b101101), jp.GetP1());

        jp.SetP1(0b110000);
        EXPECT_EQ(ReservedP1Bits(0b111111), jp.GetP1());

        jp.SetP1(0b000000);
        EXPECT_EQ(ReservedP1Bits(0b001111), jp.GetP1());

        jp.__ResetKeys();
        jp.PressKey(JoyPad::A_KEY);
        jp.SetP1(0b010000);
        EXPECT_EQ(ReservedP1Bits(0b011110), jp.GetP1());
    }

    static constexpr unsigned NO_JP_IF_VAL      = IntController::Registers::REG_RESERVED_BITS;
    static constexpr unsigned RAISED_JP_IF_VAL  = ::bits_set(IntController::JOYPAD_INT)
                                                | NO_JP_IF_VAL;

    TEST(Joypad, Interrupts) {
        IntController   intC;
        JoyPadMock      jp(&intC);

        jp.PressKey(JoyPad::A_KEY);
        jp.Step(1_MCycles);
        EXPECT_EQ(RAISED_JP_IF_VAL, intC.GetIF());


        intC.ResetInt(IntController::JOYPAD_INT);
        jp.PressKey(JoyPad::A_KEY);
        jp.Step(1_MCycles);
        EXPECT_EQ(NO_JP_IF_VAL, intC.GetIF());

        jp.PressKey(JoyPad::B_KEY);
        jp.Step(1_MCycles);
        EXPECT_EQ(RAISED_JP_IF_VAL, intC.GetIF());
        jp.Step(1_MCycles);
        intC.ResetInt(IntController::JOYPAD_INT);
        EXPECT_EQ(NO_JP_IF_VAL, intC.GetIF());

        jp.UnpressKey(JoyPad::B_KEY);
        jp.Step(1_MCycles);
        EXPECT_EQ(NO_JP_IF_VAL, intC.GetIF());

        jp.PressKey(JoyPad::A_KEY);
        jp.Step(1_MCycles);
        EXPECT_EQ(NO_JP_IF_VAL, intC.GetIF());

        jp.PressKey(JoyPad::ST_KEY);
        jp.Step(1_MCycles);
        EXPECT_EQ(RAISED_JP_IF_VAL, intC.GetIF());
    }

    TEST(Joypad, MemoryMapping) {
        MemBus      mbus;
        JoyPadMock  jp(NULL);

        jp.MapToMemory(mbus);
        jp.PressKey(JoyPad::A_KEY);
        jp.PressKey(JoyPad::B_KEY);
        jp.PressKey(JoyPad::SC_KEY);
        mbus.ImmWrite(Vaddr::P1_VADDR, 0b010000);
        EXPECT_EQ(ReservedP1Bits(0b011000), mbus.ImmRead(Vaddr::P1_VADDR));
        jp.UnpressKey(JoyPad::A_KEY);
        EXPECT_EQ(ReservedP1Bits(0b011001), mbus.ImmRead(Vaddr::P1_VADDR));

        jp.PressKey(JoyPad::UP_KEY);
        jp.PressKey(JoyPad::DW_KEY);
        jp.PressKey(JoyPad::LF_KEY);
        jp.PressKey(JoyPad::RT_KEY);
        mbus.ImmWrite(Vaddr::P1_VADDR, 0b100000);
        EXPECT_EQ(ReservedP1Bits(0b100000), mbus.ImmRead(Vaddr::P1_VADDR));
        jp.UnpressKey(JoyPad::RT_KEY);
        EXPECT_EQ(ReservedP1Bits(0b100001), mbus.ImmRead(Vaddr::P1_VADDR));
        jp.UnpressKey(JoyPad::LF_KEY);
        EXPECT_EQ(ReservedP1Bits(0b100011), mbus.ImmRead(Vaddr::P1_VADDR));
    }
    
}