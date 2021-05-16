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
    explicit
    JoyPadMock(IntController* ic_link)
    : JoyPad(ic_link) {
    }

    byte_t __get_pressed_keys() const {
        return this->__pressed_key_set;
    }

    void __reset_keys() {
        this->__pressed_key_set = 0;
    }
};

Reg8 set_P1_reserved_bits(Reg8 reg) {
    return JoyPad::P1_RESERVED_BITS | reg;
}

TEST(Joypad, KeyManagment) {
    JoyPadMock      jp(NULL);
    byte_t            key_set;

    jp.press_key(JoyPad::A_KEY);
    jp.press_key(JoyPad::B_KEY);
    jp.press_key(JoyPad::UP_KEY);
    jp.press_key(JoyPad::LF_KEY);

    key_set = ::bits_set(JoyPad::A_KEY, JoyPad::B_KEY, JoyPad::UP_KEY, JoyPad::LF_KEY);
    EXPECT_EQ(key_set, jp.__get_pressed_keys());

    jp.press_key(JoyPad::A_KEY);
    jp.press_key(JoyPad::DW_KEY);
    jp.press_key(JoyPad::SC_KEY);

    key_set |= ::bits_set(JoyPad::A_KEY, JoyPad::DW_KEY, JoyPad::SC_KEY);
    EXPECT_EQ(key_set, jp.__get_pressed_keys());

    jp.unpress_key(JoyPad::SC_KEY);
    jp.unpress_key(JoyPad::ST_KEY);
    jp.unpress_key(JoyPad::B_KEY);
    jp.unpress_key(JoyPad::LF_KEY);

    key_set = key_set & ~(::bits_set(JoyPad::SC_KEY, JoyPad::ST_KEY, JoyPad::B_KEY, JoyPad::LF_KEY));
    EXPECT_EQ(key_set, jp.__get_pressed_keys());

    jp.__reset_keys();

    key_set = 0;
    EXPECT_EQ(key_set, jp.__get_pressed_keys());

    jp.press_key(JoyPad::RT_KEY);
    jp.press_key(JoyPad::ST_KEY);

    key_set = ::bits_set(JoyPad::RT_KEY, JoyPad::ST_KEY);
    EXPECT_EQ(key_set, jp.__get_pressed_keys());

}

TEST(Joypad, MatrixLineP1) {
    JoyPadMock      jp(NULL);

    jp.__reset_keys();
    jp.set_P1_reg(0b100000);
    EXPECT_EQ(set_P1_reserved_bits(0b101111), jp.get_P1_reg());

    jp.press_key(JoyPad::RT_KEY);
    jp.press_key(JoyPad::LF_KEY);
    EXPECT_EQ(set_P1_reserved_bits(0b101100), jp.get_P1_reg());

    jp.set_P1_reg(0b010111);
    EXPECT_EQ(set_P1_reserved_bits(0b011111), jp.get_P1_reg());

    jp.press_key(JoyPad::A_KEY);
    jp.press_key(JoyPad::ST_KEY);
    EXPECT_EQ(set_P1_reserved_bits(0b010110), jp.get_P1_reg());

    jp.press_key(JoyPad::UP_KEY);
    EXPECT_EQ(set_P1_reserved_bits(0b010110), jp.get_P1_reg());

    jp.set_P1_reg(0b100000);
    EXPECT_EQ(set_P1_reserved_bits(0b101000), jp.get_P1_reg());

    jp.unpress_key(JoyPad::RT_KEY);
    jp.unpress_key(JoyPad::UP_KEY);
    EXPECT_EQ(set_P1_reserved_bits(0b101101), jp.get_P1_reg());

    jp.set_P1_reg(0b110000);
    EXPECT_EQ(set_P1_reserved_bits(0b111111), jp.get_P1_reg());

    jp.set_P1_reg(0b000000);
    EXPECT_EQ(set_P1_reserved_bits(0b001111), jp.get_P1_reg());

    jp.__reset_keys();
    jp.press_key(JoyPad::A_KEY);
    jp.set_P1_reg(0b010000);
    EXPECT_EQ(set_P1_reserved_bits(0b011110), jp.get_P1_reg());
}

static constexpr unsigned NO_JP_IF_VAL      = IntController::Registers::REG_RESERVED_BITS;
static constexpr unsigned RAISED_JP_IF_VAL  = ::bits_set(IntController::JOYPAD_INT)
                                            | NO_JP_IF_VAL;

TEST(Joypad, Interrupts) {
    IntController   int_ctrl;
    JoyPadMock      jp(&int_ctrl);

    jp.press_key(JoyPad::A_KEY);
    jp.step();
    EXPECT_EQ(RAISED_JP_IF_VAL, int_ctrl.get_IF_reg());


    int_ctrl.reset_interrupt(IntController::JOYPAD_INT);
    jp.press_key(JoyPad::A_KEY);
    jp.step();
    EXPECT_EQ(NO_JP_IF_VAL, int_ctrl.get_IF_reg());

    jp.press_key(JoyPad::B_KEY);
    jp.step();
    EXPECT_EQ(RAISED_JP_IF_VAL, int_ctrl.get_IF_reg());
    jp.step();
    int_ctrl.reset_interrupt(IntController::JOYPAD_INT);
    EXPECT_EQ(NO_JP_IF_VAL, int_ctrl.get_IF_reg());

    jp.unpress_key(JoyPad::B_KEY);
    jp.step();
    EXPECT_EQ(NO_JP_IF_VAL, int_ctrl.get_IF_reg());

    jp.press_key(JoyPad::A_KEY);
    jp.step();
    EXPECT_EQ(NO_JP_IF_VAL, int_ctrl.get_IF_reg());

    jp.press_key(JoyPad::ST_KEY);
    jp.step();
    EXPECT_EQ(RAISED_JP_IF_VAL, int_ctrl.get_IF_reg());
}

TEST(Joypad, MemoryMapping) {
    MemBus      mbus;
    JoyPadMock  jp(NULL);

    jp.map_to_memory(mbus);
    jp.press_key(JoyPad::A_KEY);
    jp.press_key(JoyPad::B_KEY);
    jp.press_key(JoyPad::SC_KEY);
    mbus.ImmWrite(Vaddr::P1_VADDR, 0b010000);
    EXPECT_EQ(set_P1_reserved_bits(0b011000), mbus.ImmRead(Vaddr::P1_VADDR));
    jp.unpress_key(JoyPad::A_KEY);
    EXPECT_EQ(set_P1_reserved_bits(0b011001), mbus.ImmRead(Vaddr::P1_VADDR));

    jp.press_key(JoyPad::UP_KEY);
    jp.press_key(JoyPad::DW_KEY);
    jp.press_key(JoyPad::LF_KEY);
    jp.press_key(JoyPad::RT_KEY);
    mbus.ImmWrite(Vaddr::P1_VADDR, 0b100000);
    EXPECT_EQ(set_P1_reserved_bits(0b100000), mbus.ImmRead(Vaddr::P1_VADDR));
    jp.unpress_key(JoyPad::RT_KEY);
    EXPECT_EQ(set_P1_reserved_bits(0b100001), mbus.ImmRead(Vaddr::P1_VADDR));
    jp.unpress_key(JoyPad::LF_KEY);
    EXPECT_EQ(set_P1_reserved_bits(0b100011), mbus.ImmRead(Vaddr::P1_VADDR));
}

}  // namespace
