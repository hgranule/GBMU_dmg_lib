#include "gtest/gtest.h"
#include "common/GB_macro.h"
#include "device/GB_interrupt.h"
#include "memory/GB_bus.h"
#include "memory/GB_vaddr.h"

namespace {

    using InterruptController = GB::device::InterruptController;

    Reg8 ReservedBits(Reg8 reg) {
        return InterruptController::Registers::REG_RESERVED_BITS | reg;
    }

    TEST(InterruptController, IE_SetGet) {
        InterruptController     interruptCTRL(InterruptController::Registers(0x3, 0x0, false));

        // check initial values
        EXPECT_EQ(interruptCTRL.GetIE(), ReservedBits(0x3));

        interruptCTRL.SetIE(0x0);
        EXPECT_EQ(interruptCTRL.GetIE(), ReservedBits(0x0));

        interruptCTRL.SetIE(0xF);
        EXPECT_EQ(interruptCTRL.GetIE(), ReservedBits(0xF));
    }

    TEST(InterruptController, IF_SetGet) {
        InterruptController     interruptCTRL(InterruptController::Registers(0x0, 0x5, false));

        // check initial values
        EXPECT_EQ(interruptCTRL.GetIF(), ReservedBits(0x5));

        interruptCTRL.SetIF(0x7);
        EXPECT_EQ(interruptCTRL.GetIF(), ReservedBits(0x7));

        interruptCTRL.SetIF(0xAA);
        EXPECT_EQ(interruptCTRL.GetIF(), ReservedBits(0xAA));
    }

    TEST(InterruptController, IME_SetGet) {
        InterruptController     interruptCTRL(InterruptController::Registers(0x0, 0x0, true));

        // check initial values
        EXPECT_TRUE(interruptCTRL.GetIME());

        interruptCTRL.SetIME(true);
        EXPECT_TRUE(interruptCTRL.GetIME());

        interruptCTRL.SetIME(false);
        EXPECT_FALSE(interruptCTRL.GetIME());

        interruptCTRL.SetIME(true);
        EXPECT_TRUE(interruptCTRL.GetIME());

        interruptCTRL.SetIME(false);
        EXPECT_FALSE(interruptCTRL.GetIME());
    }

    TEST(InterruptController, RequestResetInterrupt) {
        using GBIC = InterruptController;
        using GBINT = GBIC::InterruptIdx;

        GBIC interruptCTRL(GBIC::Registers(0x0, 0x0, true));
        
        interruptCTRL.RequestInt(GBINT::JOYPAD_INT);
        interruptCTRL.RequestInt(GBINT::VBLANK_INT);
        interruptCTRL.RequestInt(GBINT::SERIO_INT);

        EXPECT_EQ(interruptCTRL.GetIF(),
            ReservedBits(GBIC::interrupts<GBINT::JOYPAD_INT, GBINT::VBLANK_INT, GBINT::SERIO_INT>)
        );

        interruptCTRL.RequestInt(GBINT::LCDSTAT_INT);
        interruptCTRL.RequestInt(GBINT::TIMOVER_INT);

        EXPECT_EQ(interruptCTRL.GetIF(),
            ReservedBits(GBIC::interrupts<GBINT::JOYPAD_INT, GBINT::VBLANK_INT, GBINT::SERIO_INT, GBINT::LCDSTAT_INT, GBINT::TIMOVER_INT>)
        );

        interruptCTRL.ResetInt(GBINT::VBLANK_INT);
        interruptCTRL.ResetInt(GBINT::JOYPAD_INT);
        interruptCTRL.ResetInt(GBINT::TIMOVER_INT);

        EXPECT_EQ(interruptCTRL.GetIF(),
            ReservedBits(GBIC::interrupts<GBINT::SERIO_INT, GBINT::LCDSTAT_INT>)
        );

        interruptCTRL.ResetInt(GBINT::SERIO_INT);
        interruptCTRL.ResetInt(GBINT::LCDSTAT_INT);

        EXPECT_EQ(interruptCTRL.GetIF(),
            ReservedBits(0x0)
        );
    }

    TEST(InterruptController, HighestPriorityInt) {
        using GBIC = InterruptController;
        using GBINT = GBIC::InterruptIdx;

        constexpr Byte ALL_INTS = GBIC::interrupts<
                                    GBINT::VBLANK_INT,
                                    GBINT::JOYPAD_INT,
                                    GBINT::SERIO_INT,
                                    GBINT::LCDSTAT_INT,
                                    GBINT::TIMOVER_INT>;

        GBIC interruptCTRL(GBIC::Registers(0x0, 0x0, true));
        
        EXPECT_EQ(GBINT::NO_INTERRUPT, interruptCTRL.GetHighestPossibleInt());
        interruptCTRL.RequestInt(GBINT::VBLANK_INT);
        interruptCTRL.RequestInt(GBINT::JOYPAD_INT);
        interruptCTRL.RequestInt(GBINT::SERIO_INT);
        interruptCTRL.RequestInt(GBINT::LCDSTAT_INT);
        interruptCTRL.RequestInt(GBINT::TIMOVER_INT);
        EXPECT_EQ(GBINT::NO_INTERRUPT, interruptCTRL.GetHighestPossibleInt());

        interruptCTRL.SetIE(ALL_INTS);

        EXPECT_EQ(GBINT::VBLANK_INT, interruptCTRL.GetHighestPossibleInt());
        interruptCTRL.ResetInt(GBINT::VBLANK_INT);

        EXPECT_EQ(GBINT::LCDSTAT_INT, interruptCTRL.GetHighestPossibleInt());
        interruptCTRL.ResetInt(GBINT::LCDSTAT_INT);

        EXPECT_EQ(GBINT::TIMOVER_INT, interruptCTRL.GetHighestPossibleInt());
        interruptCTRL.ResetInt(GBINT::TIMOVER_INT);

        EXPECT_EQ(GBINT::SERIO_INT, interruptCTRL.GetHighestPossibleInt());
        interruptCTRL.ResetInt(GBINT::SERIO_INT);

        EXPECT_EQ(GBINT::JOYPAD_INT, interruptCTRL.GetHighestPossibleInt());
        interruptCTRL.ResetInt(GBINT::JOYPAD_INT);

        EXPECT_EQ(GBINT::NO_INTERRUPT, interruptCTRL.GetHighestPossibleInt());
    }

    TEST(InterruptController, MemBus) {
        GB::memory::BusInterface  membus;
        InterruptController   interruptCTRL;
        
        interruptCTRL.MapToMemory(membus);

        membus.ImmWrite(GB::memory::VirtualAddress::IF_VADDR, 0x4);
        EXPECT_EQ(ReservedBits(0x4), membus.ImmRead(GB::memory::VirtualAddress::IF_VADDR));
        membus.ImmWrite(GB::memory::VirtualAddress::IF_VADDR, 0x1);
        EXPECT_EQ(ReservedBits(0x1), membus.ImmRead(GB::memory::VirtualAddress::IF_VADDR));
        membus.ImmWrite(GB::memory::VirtualAddress::IF_VADDR, 0x7);
        EXPECT_EQ(ReservedBits(0x7), membus.ImmRead(GB::memory::VirtualAddress::IF_VADDR));
        membus.ImmWrite(GB::memory::VirtualAddress::IF_VADDR, 0x0);
        EXPECT_EQ(ReservedBits(0x0), membus.ImmRead(GB::memory::VirtualAddress::IF_VADDR));

        membus.ImmWrite(GB::memory::VirtualAddress::IE_VADDR, 0x4);
        EXPECT_EQ(ReservedBits(0x4), membus.ImmRead(GB::memory::VirtualAddress::IE_VADDR));
        membus.ImmWrite(GB::memory::VirtualAddress::IE_VADDR, 0x1);
        EXPECT_EQ(ReservedBits(0x1), membus.ImmRead(GB::memory::VirtualAddress::IE_VADDR));
        membus.ImmWrite(GB::memory::VirtualAddress::IE_VADDR, 0x7);
        EXPECT_EQ(ReservedBits(0x7), membus.ImmRead(GB::memory::VirtualAddress::IE_VADDR));
        membus.ImmWrite(GB::memory::VirtualAddress::IE_VADDR, 0x0);
        EXPECT_EQ(ReservedBits(0x0), membus.ImmRead(GB::memory::VirtualAddress::IE_VADDR));
    }
}