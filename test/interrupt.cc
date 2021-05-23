#include "gtest/gtest.h"

#include "GB_test.h"
#include "GB_config.h"

#include "common/GB_macro.h"
#include "device/GB_interrupt.h"
#include "memory/GB_vaddr.h"

namespace {

    using InterruptController = GB::device::InterruptController;

    Reg8 int_ctrl_reserved_bits(Reg8 reg) {
        return InterruptController::Registers::REG_RESERVED_BITS | reg;
    }

    TEST(Interrupt_Controller, Initialization) {
        InterruptController     int_ctrl;

        EXPECT_EQ(GB::INTC_IME_INIT_VALUE, int_ctrl.__registers.IME);
        EXPECT_EQ(GB::INTC_IF_INIT_VALUE, int_ctrl.__registers.IF);
        EXPECT_EQ(GB::INTC_IE_INIT_VALUE, int_ctrl.__registers.IE);
    }

    TEST(Interrupt_Controller, IE_Set_Get) {
        InterruptController     int_ctrl(InterruptController::Registers(0x3, 0x0, false));

        // check initial values
        EXPECT_EQ(int_ctrl.get_IE_reg(), int_ctrl_reserved_bits(0x3));

        int_ctrl.set_IE_reg(0x0);
        EXPECT_EQ(int_ctrl.get_IE_reg(), int_ctrl_reserved_bits(0x0));

        int_ctrl.set_IE_reg(0xF);
        EXPECT_EQ(int_ctrl.get_IE_reg(), int_ctrl_reserved_bits(0xF));
    }

    TEST(InterruptController, IF_Set_Get) {
        InterruptController     int_ctrl(InterruptController::Registers(0x0, 0x5, false));

        // check initial values
        EXPECT_EQ(int_ctrl.get_IF_reg(), int_ctrl_reserved_bits(0x5));

        int_ctrl.set_IF_reg(0x7);
        EXPECT_EQ(int_ctrl.get_IF_reg(), int_ctrl_reserved_bits(0x7));

        int_ctrl.set_IF_reg(0xAA);
        EXPECT_EQ(int_ctrl.get_IF_reg(), int_ctrl_reserved_bits(0xAA));
    }

    TEST(Interrupt_Controller, IME_Set_Get) {
        InterruptController     int_ctrl(InterruptController::Registers(0x0, 0x0, true));

        // check initial values
        EXPECT_TRUE(int_ctrl.get_IME_reg());

        int_ctrl.set_IME_reg(true);
        EXPECT_TRUE(int_ctrl.get_IME_reg());

        int_ctrl.set_IME_reg(false);
        EXPECT_FALSE(int_ctrl.get_IME_reg());

        int_ctrl.set_IME_reg(true);
        EXPECT_TRUE(int_ctrl.get_IME_reg());

        int_ctrl.set_IME_reg(false);
        EXPECT_FALSE(int_ctrl.get_IME_reg());
    }

    TEST(Interrupt_Controller, Request_Reset_Interrupt) {
        using IntController = InterruptController;
        using IntIdx = IntController::InterruptIdx;

        IntController int_ctrl(IntController::Registers(0x0, 0x0, true));

        int_ctrl.request_interrupt(IntIdx::JOYPAD_INT);
        int_ctrl.request_interrupt(IntIdx::VBLANK_INT);
        int_ctrl.request_interrupt(IntIdx::SERIO_INT);

        EXPECT_EQ(int_ctrl.get_IF_reg(),
            int_ctrl_reserved_bits(IntController::interrupts<IntIdx::JOYPAD_INT, IntIdx::VBLANK_INT,
                                                             IntIdx::SERIO_INT>));

        int_ctrl.request_interrupt(IntIdx::LCDSTAT_INT);
        int_ctrl.request_interrupt(IntIdx::TIMOVER_INT);

        EXPECT_EQ(int_ctrl.get_IF_reg(),
            int_ctrl_reserved_bits(IntController::interrupts<IntIdx::JOYPAD_INT, IntIdx::VBLANK_INT,
                                                             IntIdx::SERIO_INT, IntIdx::LCDSTAT_INT,
                                                             IntIdx::TIMOVER_INT>));

        int_ctrl.reset_interrupt(IntIdx::VBLANK_INT);
        int_ctrl.reset_interrupt(IntIdx::JOYPAD_INT);
        int_ctrl.reset_interrupt(IntIdx::TIMOVER_INT);

        EXPECT_EQ(int_ctrl.get_IF_reg(),
            int_ctrl_reserved_bits(IntController::interrupts<IntIdx::SERIO_INT, IntIdx::LCDSTAT_INT>));

        int_ctrl.reset_interrupt(IntIdx::SERIO_INT);
        int_ctrl.reset_interrupt(IntIdx::LCDSTAT_INT);

        EXPECT_EQ(int_ctrl.get_IF_reg(),
            int_ctrl_reserved_bits(0x0));
    }

    TEST(Interrupt_Controller, Highest_Priority_Interrupt) {
        using IntController = InterruptController;
        using IntIdx = IntController::InterruptIdx;

        constexpr byte_t ALL_INTS = IntController::interrupts<
                                        IntIdx::VBLANK_INT,
                                        IntIdx::JOYPAD_INT,
                                        IntIdx::SERIO_INT,
                                        IntIdx::LCDSTAT_INT,
                                        IntIdx::TIMOVER_INT>;

        IntController int_ctrl(IntController::Registers(0x0, 0x0, true));

        EXPECT_EQ(IntIdx::NO_INTERRUPT, int_ctrl.get_highest_priority_interrupt());
        int_ctrl.request_interrupt(IntIdx::VBLANK_INT);
        int_ctrl.request_interrupt(IntIdx::JOYPAD_INT);
        int_ctrl.request_interrupt(IntIdx::SERIO_INT);
        int_ctrl.request_interrupt(IntIdx::LCDSTAT_INT);
        int_ctrl.request_interrupt(IntIdx::TIMOVER_INT);
        EXPECT_EQ(IntIdx::NO_INTERRUPT, int_ctrl.get_highest_priority_interrupt());

        int_ctrl.set_IE_reg(ALL_INTS);

        EXPECT_EQ(IntIdx::VBLANK_INT, int_ctrl.get_highest_priority_interrupt());
        int_ctrl.reset_interrupt(IntIdx::VBLANK_INT);

        EXPECT_EQ(IntIdx::LCDSTAT_INT, int_ctrl.get_highest_priority_interrupt());
        int_ctrl.reset_interrupt(IntIdx::LCDSTAT_INT);

        EXPECT_EQ(IntIdx::TIMOVER_INT, int_ctrl.get_highest_priority_interrupt());
        int_ctrl.reset_interrupt(IntIdx::TIMOVER_INT);

        EXPECT_EQ(IntIdx::SERIO_INT, int_ctrl.get_highest_priority_interrupt());
        int_ctrl.reset_interrupt(IntIdx::SERIO_INT);

        EXPECT_EQ(IntIdx::JOYPAD_INT, int_ctrl.get_highest_priority_interrupt());
        int_ctrl.reset_interrupt(IntIdx::JOYPAD_INT);

        EXPECT_EQ(IntIdx::NO_INTERRUPT, int_ctrl.get_highest_priority_interrupt());
    }

}  // namespace
