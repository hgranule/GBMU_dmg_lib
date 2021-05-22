#include "gtest/gtest.h"

#include "GB_test.h"

#include "GB_config.h"
#include "device/GB_wram.h"

namespace {

using WRAM = GB::device::WRAM;

inline byte_t set_SVBK_reserved_bits(byte_t value) {
    return value | WRAM::Registers::RESERVED_BITS;
}

TEST(Work_RAM, Initialization) {
    WRAM ram;

    EXPECT_EQ(WRAM::MAX_SIZE, GB::WRAM_CGB_SIZE);  // 32 KB
    EXPECT_EQ(WRAM::BANK_SIZE, GB::WRAM_CGB_BANK_SIZE);  // 4 KB

    EXPECT_EQ(ram.__memory.length(), WRAM::MAX_SIZE);
    EXPECT_EQ(ram.__regs.SVBK, GB::SVBK_INIT_VALUE);
}

TEST(Work_RAM, Copy_Move_Constructor) {
    WRAM ram_src;

    ram_src.write_phys_addr(0x1000, 0x1);
    ram_src.write_phys_addr(0x2000, 0x2);
    ram_src.write_phys_addr(0x3000, 0x3);

    /// Copy constructor
    WRAM ram_copy(ram_src);

    EXPECT_EQ(ram_copy.read_phys_addr(0x1000), 0x1);
    EXPECT_EQ(ram_copy.read_phys_addr(0x2000), 0x2);
    EXPECT_EQ(ram_copy.read_phys_addr(0x3000), 0x3);

    ram_copy.write_phys_addr(0x3000, 0xDE);

    EXPECT_NE(ram_src.read_phys_addr(0x3000), ram_copy.read_phys_addr(0x3000));

    /// Move constructor
    WRAM ram_mvd(std::move(ram_copy));

    EXPECT_EQ(ram_mvd.read_phys_addr(0x1000), 0x1);
    EXPECT_EQ(ram_mvd.read_phys_addr(0x2000), 0x2);
    EXPECT_EQ(ram_mvd.read_phys_addr(0x3000), 0xDE);

    EXPECT_EQ(ram_copy.__memory.__data, nullptr);
    EXPECT_EQ(ram_copy.__memory.__len, 0x0);
}

TEST(Work_RAM, Phys_Addr_Read_Write) {
    WRAM ram;

    /// Bank 0
    ram.write_phys_addr(0x10, 0x42);
    ram.write_phys_addr(0x11, 0x21);
    ram.write_phys_addr(0x12, 0x1);

    EXPECT_EQ(ram.read_phys_addr(0x10), 0x42);
    EXPECT_EQ(ram.read_phys_addr(0x11), 0x21);
    EXPECT_EQ(ram.read_phys_addr(0x12), 0x1);

    /// Bank 1
    ram.write_phys_addr(0x1000 + 0xA, 0x1);
    ram.write_phys_addr(0x1000 + 0xB, 0x2);
    ram.write_phys_addr(0x1000 + 0xC, 0x3);

    EXPECT_EQ(ram.read_phys_addr(0x1000 + 0xA), 0x1);
    EXPECT_EQ(ram.read_phys_addr(0x1000 + 0xB), 0x2);
    EXPECT_EQ(ram.read_phys_addr(0x1000 + 0xC), 0x3);

    /// Bank 4
    ram.write_phys_addr(0x4000 + 0x404, 0xAA);
    ram.write_phys_addr(0x4000 + 0x405, 0xBB);
    ram.write_phys_addr(0x4000 + 0x406, 0xCC);

    EXPECT_EQ(ram.read_phys_addr(0x4000 + 0x404), 0xAA);
    EXPECT_EQ(ram.read_phys_addr(0x4000 + 0x405), 0xBB);
    EXPECT_EQ(ram.read_phys_addr(0x4000 + 0x406), 0xCC);

}

TEST(Work_RAM, SVBK_Set_Get) {
    WRAM    ram;

    EXPECT_EQ(set_SVBK_reserved_bits(GB::SVBK_INIT_VALUE), ram.get_SVBK_reg());

    ram.set_SVBK_reg(0xAB);
    EXPECT_EQ(set_SVBK_reserved_bits(0xAB), ram.get_SVBK_reg());

    ram.set_SVBK_reg(0x0);
    EXPECT_EQ(set_SVBK_reserved_bits(0x0), ram.get_SVBK_reg());

    ram.set_SVBK_reg(0x1);
    EXPECT_EQ(set_SVBK_reserved_bits(0x1), ram.get_SVBK_reg());
}

TEST(Work_RAM, SVBK_Bank_Choose) {
    WRAM    ram;

    ram.set_SVBK_reg(0x0);
    EXPECT_EQ(1, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0x1);
    EXPECT_EQ(1, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0x2);
    EXPECT_EQ(2, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0x3);
    EXPECT_EQ(3, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0x4);
    EXPECT_EQ(4, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0x5);
    EXPECT_EQ(5, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0x6);
    EXPECT_EQ(6, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0x7);
    EXPECT_EQ(7, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0x8);
    EXPECT_EQ(1, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0x9);
    EXPECT_EQ(1, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0xA);
    EXPECT_EQ(2, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0xB);
    EXPECT_EQ(3, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0xC);
    EXPECT_EQ(4, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0xD);
    EXPECT_EQ(5, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0xE);
    EXPECT_EQ(6, ram.__get_current_bank_idx());
    ram.set_SVBK_reg(0xF);
    EXPECT_EQ(7, ram.__get_current_bank_idx());
}

}  // namespace
