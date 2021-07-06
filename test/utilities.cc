#include "gtest/gtest.h"

#include "GB_test.h"

#include "common/GB_utilities.h"

namespace {

TEST(Vector, insert_sorting) {
    std::vector<int> vec {1, 2, 5, 7};

    auto comparator = [](const int& new_elem, const int& cur_elem){ return new_elem < cur_elem; };

    std::vector<int>::iterator iter = insert_sorting(vec, 20, comparator);
    EXPECT_EQ(iter - vec.begin(), 4);
    EXPECT_EQ(vec.size(), 5);

    iter = insert_sorting(vec, 3, comparator);
    EXPECT_EQ(iter - vec.begin(), 2);
    EXPECT_EQ(vec.size(), 6);

    iter = insert_sorting(vec, 0, comparator);
    EXPECT_EQ(iter - vec.begin(), 0);
    EXPECT_EQ(vec.size(), 7);

    EXPECT_EQ(vec[0], 0);
    EXPECT_EQ(vec[1], 1);
    EXPECT_EQ(vec[2], 2);
    EXPECT_EQ(vec[3], 3);
    EXPECT_EQ(vec[4], 5);
    EXPECT_EQ(vec[5], 7);
    EXPECT_EQ(vec[6], 20);
}

}   // namespace
