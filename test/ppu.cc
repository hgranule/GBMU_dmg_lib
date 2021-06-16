#include "gtest/gtest.h"

#include "GB_test.h"

#include "device/GB_ppu.h"
#include "GB_config.h"

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
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000000);

    ppu.__stat_mode = PPU::STAT_Mode::STAT_Render;
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000011);

    ppu.__stat_mode = PPU::STAT_Mode::STAT_Hblank;
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000000);
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000000);

    ppu.__stat_mode = PPU::STAT_Mode::STAT_Vblank;
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000001);
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000001);

    ppu.__stat_mode = PPU::STAT_Mode::STAT_SearchingOAM;
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000010);
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000010);

    ppu.__stat_mode = PPU::STAT_Mode::STAT_Render;
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000011);
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000011);
}

TEST(PPU, add_oram_object) {
    ORAM oram;
    VRAM vram;
    PPU ppu(&oram, &vram);

    ppu.TEMPORARY_GB_MODE_FLAG = GB::GBModeFlag::CGB_MODE;
    PPU::Object objects[40];
    memset(objects, 0, sizeof(objects));
    for (int i = 0; i < 40; ++i) {
        ppu.add_oram_object(i);
    }
    EXPECT_EQ(ppu.__intersected_objects.size(), 0);
}

TEST(PPU, oram_search_pipeline) {
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
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000000 | PPU::STAT_SearchingOAM);
    // TODO(dolovnyak, hgranule) interrupts
    ppu.step();
    EXPECT_EQ(ppu.__counter.__counter, 0_CLKCycles);
    EXPECT_EQ(ppu.__next_object_index - 1, current_object_index);
    EXPECT_EQ(ppu.__current_state, PPU::State::SearchOam);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000000 | PPU::STAT_SearchingOAM);

    // oram search -> oram search
    while (ppu.__next_object_index < 39) {
        ++current_object_index;
        ppu.step();
        EXPECT_EQ(ppu.__counter.__counter, PPU::OBJECT_SEARCH_TIME - 1_CLKCycles);
        EXPECT_EQ(ppu.__next_object_index - 1, current_object_index);
        EXPECT_EQ(ppu.__current_state, PPU::State::SearchOam);
        EXPECT_EQ(ppu.get_STAT_reg(), 0b10000000 | PPU::STAT_SearchingOAM);
        ppu.step();
        EXPECT_EQ(ppu.__counter.__counter, 0_CLKCycles);
        EXPECT_EQ(ppu.__next_object_index - 1, current_object_index);
        EXPECT_EQ(ppu.__current_state, PPU::State::SearchOam);
        EXPECT_EQ(ppu.get_STAT_reg(), 0b10000000 | PPU::STAT_SearchingOAM);
    }

    // last oram search -> render
    ppu.step();
    EXPECT_EQ(ppu.__counter.__counter, PPU::OBJECT_SEARCH_TIME - 1_CLKCycles);
    EXPECT_EQ(ppu.__next_object_index, 40);
    EXPECT_EQ(ppu.__current_state, PPU::State::Render);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000000 | PPU::STAT_SearchingOAM);
    ppu.step();
    EXPECT_EQ(ppu.__counter.__counter, 0_CLKCycles);
    EXPECT_EQ(ppu.__next_object_index, 40);
    EXPECT_EQ(ppu.__current_state, PPU::State::Render);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000000 | PPU::STAT_SearchingOAM);
}

}