#include "gtest/gtest.h"

#define protected public    // TODO(dolovnyak): Use #include "GB_test.h" now

#include "device/GB_oram.h"

namespace {

using ORAM = GB::device::ORAM;

TEST(ObjectsRAM, Constructors) {
    ORAM oram;
    EXPECT_EQ(oram.get_memory_buffer_ref().size(), 160);

    oram.write_phys_addr(0, 100);
    oram.write_phys_addr(10, 101);
    oram.write_phys_addr(159, 102);

    ORAM oram_test_move_constr(std::move(oram));
    EXPECT_EQ(oram.__memory.__data, nullptr);
    EXPECT_EQ(oram.__memory.__len, 0);
    EXPECT_EQ(oram_test_move_constr.get_memory_buffer_ref().size(), 160);
    EXPECT_EQ(oram_test_move_constr.read_phys_addr(0), 100);
    EXPECT_EQ(oram_test_move_constr.read_phys_addr(10), 101);
    EXPECT_EQ(oram_test_move_constr.read_phys_addr(159), 102);

    ORAM oram_test_copy_constr(oram_test_move_constr);
    EXPECT_EQ(oram_test_copy_constr.get_memory_buffer_ref().size(), 160);
    EXPECT_EQ(oram_test_copy_constr.read_phys_addr(0), 100);
    EXPECT_EQ(oram_test_copy_constr.read_phys_addr(10), 101);
    EXPECT_EQ(oram_test_copy_constr.read_phys_addr(159), 102);

    ORAM oram_test_copy_assignment;
    oram_test_copy_assignment = oram_test_copy_constr;
    EXPECT_EQ(oram_test_copy_assignment.get_memory_buffer_ref().size(), 160);
    EXPECT_EQ(oram_test_copy_assignment.read_phys_addr(0), 100);
    EXPECT_EQ(oram_test_copy_assignment.read_phys_addr(10), 101);
    EXPECT_EQ(oram_test_copy_assignment.read_phys_addr(159), 102);

    ORAM oram_test_move_assignment;
    oram_test_move_assignment = std::move(oram_test_copy_constr);
    EXPECT_EQ(oram_test_copy_constr.__memory.__data, nullptr);
    EXPECT_EQ(oram_test_copy_constr.__memory.__len, 0);
    EXPECT_EQ(oram_test_move_assignment.get_memory_buffer_ref().size(), 160);
    EXPECT_EQ(oram_test_move_assignment.read_phys_addr(0), 100);
    EXPECT_EQ(oram_test_move_assignment.read_phys_addr(10), 101);
    EXPECT_EQ(oram_test_move_assignment.read_phys_addr(159), 102);
}

TEST(ObjectsRAM, Phys_Read_Write_And_Correct_Raw_Memory) {
    ORAM oram;

    oram.write_phys_addr(0, 100);
    oram.write_phys_addr(10, 101);
    oram.write_phys_addr(159, 102);

    EXPECT_EQ(oram.get_memory_buffer_ref()[0], 100);
    EXPECT_EQ(oram.get_memory_buffer_ref()[10], 101);
    EXPECT_EQ(oram.get_memory_buffer_ref()[159], 102);

    EXPECT_EQ(oram.read_phys_addr(0), 100);
    EXPECT_EQ(oram.read_phys_addr(10), 101);
    EXPECT_EQ(oram.read_phys_addr(159), 102);
}

// TODO(dolovnyak) make unit-tests for read/write with memory_bus when it will be done

}   // namespace
