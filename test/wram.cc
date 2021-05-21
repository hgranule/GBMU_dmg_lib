#include "gtest/gtest.h"

#include "GB_config.h"
#include "device/GB_wram.h"

namespace {

using WRAM = GB::device::WRAM;

inline byte_t set_SVBK_reserved_bits(byte_t value) {
    return value | WRAM::Registers::RESERVED_BITS;
}

TEST(WorkingRAM, SVBK_ReadWrite) {
    WRAM    wram;

    EXPECT_EQ(set_SVBK_reserved_bits(GB::SVBK_INIT_VALUE), wram.get_SVBK_reg());

    wram.set_SVBK_reg(0xAB);
    EXPECT_EQ(set_SVBK_reserved_bits(0xAB), wram.get_SVBK_reg());

    wram.set_SVBK_reg(0x0);
    EXPECT_EQ(set_SVBK_reserved_bits(0x0), wram.get_SVBK_reg());

    wram.set_SVBK_reg(0x1);
    EXPECT_EQ(set_SVBK_reserved_bits(0x1), wram.get_SVBK_reg());
}

TEST(WorkingRAM, SVBK_BankChoose) {
    WRAM    wram;

    wram.set_SVBK_reg(0x0);
    EXPECT_EQ(1, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0x1);
    EXPECT_EQ(1, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0x2);
    EXPECT_EQ(2, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0x3);
    EXPECT_EQ(3, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0x4);
    EXPECT_EQ(4, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0x5);
    EXPECT_EQ(5, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0x6);
    EXPECT_EQ(6, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0x7);
    EXPECT_EQ(7, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0x8);
    EXPECT_EQ(1, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0x9);
    EXPECT_EQ(1, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0xA);
    EXPECT_EQ(2, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0xB);
    EXPECT_EQ(3, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0xC);
    EXPECT_EQ(4, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0xD);
    EXPECT_EQ(5, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0xE);
    EXPECT_EQ(6, wram.__get_current_bank_idx());
    wram.set_SVBK_reg(0xF);
    EXPECT_EQ(7, wram.__get_current_bank_idx());
}

// TODO(hgranule): make unit-tests for read/write with and without memory_bus

}  // namespace
