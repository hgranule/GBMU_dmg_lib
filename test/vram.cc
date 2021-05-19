#include "gtest/gtest.h"

#define protected public    // TODO(dolovnyak): When tests will be upgraded maybe remove it.
#include "device/GB_vram.h"

namespace {

using VRAM = GB::device::VRAM;

TEST(VideoRAM, Constructors) {
    VRAM vram;

    EXPECT_EQ(vram.__memory.size(), 16_KBytes);
    EXPECT_EQ(vram.__regs.RESERVED_BITS, 0xFE);

    vram.write_phys_addr(0, 100);
    vram.write_phys_addr(10, 101);
    vram.write_phys_addr(159, 102);

    VRAM vram_test_move_constr(std::move(vram));
    EXPECT_EQ(vram.__memory.__data, nullptr);
    EXPECT_EQ(vram.__memory.__len, 0);
    EXPECT_EQ(vram.__regs.VBK, 0);
    EXPECT_EQ(vram_test_move_constr.read_phys_addr(0), 100);
    EXPECT_EQ(vram_test_move_constr.read_phys_addr(10), 101);
    EXPECT_EQ(vram_test_move_constr.read_phys_addr(159), 102);

    VRAM vram_test_copy_constr(vram_test_move_constr);
    EXPECT_EQ(vram_test_copy_constr.read_phys_addr(0), 100);
    EXPECT_EQ(vram_test_copy_constr.read_phys_addr(10), 101);
    EXPECT_EQ(vram_test_copy_constr.read_phys_addr(159), 102);

    VRAM vram_test_copy_assignment;
    vram_test_copy_assignment = vram_test_copy_constr;
    EXPECT_EQ(vram_test_copy_assignment.read_phys_addr(0), 100);
    EXPECT_EQ(vram_test_copy_assignment.read_phys_addr(10), 101);
    EXPECT_EQ(vram_test_copy_assignment.read_phys_addr(159), 102);

    VRAM vram_test_move_assignment;
    vram_test_move_assignment = std::move(vram_test_copy_constr);
    EXPECT_EQ(vram_test_copy_constr.__memory.__data, nullptr);
    EXPECT_EQ(vram_test_copy_constr.__memory.__len, 0);
    EXPECT_EQ(vram_test_copy_constr.__regs.VBK, 0);
    EXPECT_EQ(vram_test_move_assignment.read_phys_addr(0), 100);
    EXPECT_EQ(vram_test_move_assignment.read_phys_addr(10), 101);
    EXPECT_EQ(vram_test_move_assignment.read_phys_addr(159), 102);
}

TEST(VideoRAM, VBK_Read_Write) {
    VRAM vram;

    EXPECT_EQ(vram.get_VBK_reg(), 0xFE);

    vram.set_VBK_reg(0xAA);
    EXPECT_EQ(vram.get_VBK_reg(), 0xFE);

    vram.set_VBK_reg(0xFF);
    EXPECT_EQ(vram.get_VBK_reg(), 0xFF);

    vram.set_VBK_reg(0x0);
    EXPECT_EQ(vram.get_VBK_reg(), 0xFE);
}

TEST(VideoRAM, Phys_Read_Write) {
    VRAM vram;

    vram.write_phys_addr(0x0, 100);
    vram.write_phys_addr(0x100, 101);
    vram.write_phys_addr(0x1FFF, 102);
    vram.write_phys_addr(0x3FFF, 103);

    EXPECT_EQ(vram.read_phys_addr(0x0), 100);
    EXPECT_EQ(vram.read_phys_addr(0x100), 101);
    EXPECT_EQ(vram.read_phys_addr(0x1FFF), 102);
    EXPECT_EQ(vram.read_phys_addr(0x3FFF), 103);
}

TEST(VideoRAM, Inner_Read_Write) {
    VRAM vram;

    EXPECT_EQ(vram.get_VBK_reg(), 0xFE);
    vram.write_inner_vaddr(0x0, 20);
    vram.write_inner_vaddr(0x100, 21);
    vram.write_inner_vaddr(0x1FFF, 22);
    vram.set_VBK_reg(0xFF);
    vram.write_inner_vaddr(0x0, 30);
    vram.write_inner_vaddr(0x100, 31);
    vram.write_inner_vaddr(0x1FFF, 32);

    vram.set_VBK_reg(0x0);
    EXPECT_EQ(vram.read_inner_vaddr(0x0), 20);
    EXPECT_EQ(vram.read_inner_vaddr(0x100), 21);
    EXPECT_EQ(vram.read_inner_vaddr(0x1FFF), 22);
    vram.set_VBK_reg(0xFF);
    EXPECT_EQ(vram.read_inner_vaddr(0x0), 30);
    EXPECT_EQ(vram.read_inner_vaddr(0x100), 31);
    EXPECT_EQ(vram.read_inner_vaddr(0x1FFF), 32);

    EXPECT_EQ(vram.__memory[0x0], 20);
    EXPECT_EQ(vram.__memory[0x100], 21);
    EXPECT_EQ(vram.__memory[0x1FFF], 22);
    EXPECT_EQ(vram.__memory[0x2000], 30);
    EXPECT_EQ(vram.__memory[0x2100], 31);
    EXPECT_EQ(vram.__memory[0x3FFF], 32);
}

// TODO(dolovnyak): Add tests with membus when it well be implemented

}   // namespace
