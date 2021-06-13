#include "gtest/gtest.h"

#include "GB_test.h"

#include "device/GB_ppu.h"

namespace {

using PPU = GB::device::PPU;
using ORAM = GB::device::ORAM;
using VRAM = GB::device::VRAM;

TEST(PPU, Constructors) {
    // TODO
}

TEST(PPU, STAT_Register) {
    ORAM oram;
    VRAM vram;
    PPU ppu(&oram, &vram);

    ppu.set_STAT_reg(0b11111111);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b11111100);
    ppu.set_STAT_reg(0b10110011);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10110000);
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0x0);

    ppu.__regs.STAT = 0xFF;
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b00000011);

    ppu.set_STAT_mode(PPU::STAT_Hblank);
    EXPECT_EQ(ppu.get_STAT_reg(), 0x0);
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0x0);

    ppu.set_STAT_mode(PPU::STAT_Vblank);
    EXPECT_EQ(ppu.get_STAT_reg(), 0x1);
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0x1);

    ppu.set_STAT_mode(PPU::STAT_SearchingOAM);
    EXPECT_EQ(ppu.get_STAT_reg(), 0x2);
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0x2);

    ppu.set_STAT_mode(PPU::STAT_Render);
    EXPECT_EQ(ppu.get_STAT_reg(), 0x3);
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0x3);
}

/*
 * NOTE:    In GameBoy coordinates of objects are specified for the lower right corner 8x16 object.
 *
 *          Top left corner calculated using: newPosX = posX - 8, newPosY = posY - 16.
 *
 *          Y=0 hides 8x8 and 8x16 both objects.
 *          Y=2 hides an 8x8 object, but display last two rows of an 8x16 object.
 *          Y=16 displays both 8x8 8x16 objects at the top of the screen.
 *          Y=144 displays 8x16 object aligned with the bottom of the screen and 8x8 on line 136.
 */
TEST(PPU, add_oram_object) {
    ORAM oram;
    VRAM vram;

    PPU::Object objects[40];

    objects[0].pos_y = 0;

    PPU ppu(&oram, &vram);
}

TEST(PPU, oam_search_pipeline) {
    ORAM oram;
    VRAM vram;

    PPU ppu(&oram, &vram);
    EXPECT_EQ(ppu.__current_state, PPU::State::FirstOamSearch);

    // first oram search -> oram search
    int current_object_index = 0;
    ppu.step();
    EXPECT_EQ(ppu.__counter.__counter, PPU::OBJECT_SEARCH_TIME - 1_CLKCycles);
    EXPECT_EQ(ppu.__next_object_index - 1, current_object_index);
    EXPECT_EQ(ppu.__current_state, PPU::State::SearchOam);
    EXPECT_EQ(ppu.get_STAT_reg(), PPU::STAT_SearchingOAM);
    // TODO(dolovnyak, hgranule) interrupts
    ppu.step();
    EXPECT_EQ(ppu.__counter.__counter, 0_CLKCycles);
    EXPECT_EQ(ppu.__next_object_index - 1, current_object_index);
    EXPECT_EQ(ppu.__current_state, PPU::State::SearchOam);
    EXPECT_EQ(ppu.get_STAT_reg(), PPU::STAT_SearchingOAM);

    // oram search -> oram search
    while (ppu.__next_object_index < 39) {
        ++current_object_index;
        ppu.step();
        EXPECT_EQ(ppu.__counter.__counter, PPU::OBJECT_SEARCH_TIME - 1_CLKCycles);
        EXPECT_EQ(ppu.__next_object_index - 1, current_object_index);
        EXPECT_EQ(ppu.__current_state, PPU::State::SearchOam);
        EXPECT_EQ(ppu.get_STAT_reg(), PPU::STAT_SearchingOAM);
        ppu.step();
        EXPECT_EQ(ppu.__counter.__counter, 0_CLKCycles);
        EXPECT_EQ(ppu.__next_object_index - 1, current_object_index);
        EXPECT_EQ(ppu.__current_state, PPU::State::SearchOam);
        EXPECT_EQ(ppu.get_STAT_reg(), PPU::STAT_SearchingOAM);
    }

    // last oram search -> render
    ppu.step();
    EXPECT_EQ(ppu.__counter.__counter, PPU::OBJECT_SEARCH_TIME - 1_CLKCycles);
    EXPECT_EQ(ppu.__next_object_index, 40);
    EXPECT_EQ(ppu.__current_state, PPU::State::Render);
    EXPECT_EQ(ppu.get_STAT_reg(), PPU::STAT_SearchingOAM);
    ppu.step();
    EXPECT_EQ(ppu.__counter.__counter, 0_CLKCycles);
    EXPECT_EQ(ppu.__next_object_index, 40);
    EXPECT_EQ(ppu.__current_state, PPU::State::Render);
    EXPECT_EQ(ppu.get_STAT_reg(), PPU::STAT_SearchingOAM);
}

}