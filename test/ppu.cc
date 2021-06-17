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

TEST(PPU, LCDC_Register_get_set) {
    ORAM oram;
    VRAM vram;
    PPU ppu(&oram, &vram);

    EXPECT_EQ(ppu.get_LCDC_reg(), 0x0);

    ppu.set_LCDC_reg(0xFF);
    EXPECT_EQ(ppu.get_LCDC_reg(), 0xFF);

    ppu.set_LCDC_reg(0x0);
    EXPECT_EQ(ppu.get_LCDC_reg(), 0x0);

    ppu.set_LCDC_reg(0b11010101);
    EXPECT_EQ(ppu.get_LCDC_reg(), 0b11010101);
}

TEST(PPU, STAT_Register_get_set) {
    ORAM oram;
    VRAM vram;
    PPU ppu(&oram, &vram);

    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000000);

    ppu.set_STAT_reg(0b11111111);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b11111100);
    ppu.set_STAT_reg(0b10110011);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10110000);
    ppu.set_STAT_reg(0x0);
    EXPECT_EQ(ppu.get_STAT_reg(), 0b10000000);

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

TEST(PPU, LY_Register_get) {
    ORAM oram;
    VRAM vram;
    PPU ppu(&oram, &vram);

    EXPECT_EQ(ppu.get_LY_reg(), 0x0);

    /// first frame
    /// run 153 lines.
    for (int i = 0; i < 152; ++i) {
        for (int j = 0; j < 456; ++j)
            ppu.step();
        EXPECT_EQ(ppu.get_LY_reg(), i + 1);
    }
    /// on last 154 line LY becomes to zero during first few ticks.
    for (int j = 0; j < 456; ++j)
        ppu.step();
    EXPECT_EQ(ppu.get_LY_reg(), 0);

    /// second frame
    for (int i = 0; i < 152; ++i) {
        for (int j = 0; j < 456; ++j)
            ppu.step();
        EXPECT_EQ(ppu.get_LY_reg(), i + 1);
    }
    for (int j = 0; j < 456; ++j)
        ppu.step();
    EXPECT_EQ(ppu.get_LY_reg(), 0);

    /// third frame
    for (int i = 0; i < 152; ++i) {
        for (int j = 0; j < 456; ++j)
            ppu.step();
        EXPECT_EQ(ppu.get_LY_reg(), i + 1);
    }
    for (int j = 0; j < 456; ++j)
        ppu.step();
    EXPECT_EQ(ppu.get_LY_reg(), 0);
}

TEST(PPU, LYC_Register_get_set) {
    ORAM oram;
    VRAM vram;
    PPU ppu(&oram, &vram);

    EXPECT_EQ(ppu.get_LYC_reg(), 0x0);

    ppu.set_LYC_reg(0xFF);
    EXPECT_EQ(ppu.get_LYC_reg(), 0xFF);

    ppu.set_LYC_reg(0x0);
    EXPECT_EQ(ppu.get_LYC_reg(), 0x0);

    ppu.set_LYC_reg(0b11010101);
    EXPECT_EQ(ppu.get_LYC_reg(), 0b11010101);
}

TEST(PPU, add_oram_object) {
    ORAM oram;
    VRAM vram;
    PPU ppu(&oram, &vram);

    ppu.TEMPORARY_GB_MODE_FLAG = GB::GBModeFlag::CGB_MODE;

    /// All objects have pos_y = 0 by default (because all oram memory is zero by default)
    for (int i = 0; i < 40; ++i) {
        ppu.add_oram_object(i);
    }
    EXPECT_EQ(ppu.__intersected_objects.size(), 0);

    PPU::Object* objects;
    objects = reinterpret_cast<PPU::Object *>(oram.get_memory_buffer_ref().get_data_addr());

    /// top border with 8x8 object
    ppu.__current_line = 0;
    ppu.__obj_high = false;
    objects[13].pos_y = 3;
    objects[17].pos_y = 7;
    objects[20].pos_y = 8;
    objects[25].pos_y = 9;
    objects[27].pos_y = 11;
    objects[30].pos_y = 16;
    objects[32].pos_y = 17;
    for (int i = 0; i < 40; ++i) {
        ppu.add_oram_object(i);
    }
    EXPECT_EQ(ppu.__intersected_objects.size(), 3);
    EXPECT_EQ(ppu.__intersected_objects[0].pos_y, 9);
    EXPECT_EQ(ppu.__intersected_objects[1].pos_y, 11);
    EXPECT_EQ(ppu.__intersected_objects[2].pos_y, 16);

    /// top border with 8x16 object
    ppu.__intersected_objects.clear();
    ppu.__current_line = 0;
    ppu.__obj_high = true;
    bzero(objects, GB::ORAM_SIZE);
    objects[13].pos_y = 0;
    objects[17].pos_y = 1;
    objects[20].pos_y = 2;
    objects[24].pos_y = 8;
    objects[27].pos_y = 16;
    objects[31].pos_y = 17;
    for (int i = 0; i < 40; ++i) {
        ppu.add_oram_object(i);
    }
    EXPECT_EQ(ppu.__intersected_objects.size(), 4);
    EXPECT_EQ(ppu.__intersected_objects[0].pos_y, 1);
    EXPECT_EQ(ppu.__intersected_objects[1].pos_y, 2);
    EXPECT_EQ(ppu.__intersected_objects[2].pos_y, 8);
    EXPECT_EQ(ppu.__intersected_objects[3].pos_y, 16);

    /// bottom border with 8x8
    ppu.__intersected_objects.clear();
    ppu.__current_line = 143;
    ppu.__obj_high = false;
    bzero(objects, GB::ORAM_SIZE);
    objects[13].pos_y = 151;
    objects[16].pos_y = 152;
    objects[20].pos_y = 153;
    objects[23].pos_y = 159;
    objects[26].pos_y = 160;
    for (int i = 0; i < 40; ++i) {
        ppu.add_oram_object(i);
    }
    EXPECT_EQ(ppu.__intersected_objects.size(), 3);
    EXPECT_EQ(ppu.__intersected_objects[0].pos_y, 152);
    EXPECT_EQ(ppu.__intersected_objects[1].pos_y, 153);
    EXPECT_EQ(ppu.__intersected_objects[2].pos_y, 159);

    /// bottom border with 8x16
    ppu.__intersected_objects.clear();
    ppu.__current_line = 143; // last visible line
    ppu.__obj_high = true;
    bzero(objects, GB::ORAM_SIZE);
    objects[13].pos_y = 143;
    objects[16].pos_y = 144;
    objects[20].pos_y = 145;
    objects[23].pos_y = 159;
    objects[26].pos_y = 160;
    for (int i = 0; i < 40; ++i) {
        ppu.add_oram_object(i);
    }
    EXPECT_EQ(ppu.__intersected_objects.size(), 3);
    EXPECT_EQ(ppu.__intersected_objects[0].pos_y, 144);
    EXPECT_EQ(ppu.__intersected_objects[1].pos_y, 145);
    EXPECT_EQ(ppu.__intersected_objects[2].pos_y, 159);

    /// priority in CGB_MODE
    ppu.__intersected_objects.clear();
    ppu.__current_line = 143; // last visible line
    ppu.__obj_high = true;
    bzero(objects, GB::ORAM_SIZE);

    objects[11].pos_y = 144;
    objects[11].pos_x = 170;

    objects[13].pos_y = 144;
    objects[13].pos_x = 0;

    objects[15].pos_y = 144;
    objects[15].pos_x = 20;

    objects[17].pos_y = 144;
    objects[17].pos_x = 10;
    objects[17].sprite_index = 1;

    objects[18].pos_y = 144;
    objects[18].pos_x = 10;
    objects[18].sprite_index = 5;

    for (int i = 0; i < 40; ++i) {
        ppu.add_oram_object(i);
    }
    EXPECT_EQ(ppu.__intersected_objects.size(), 5);
    EXPECT_EQ(ppu.__intersected_objects[0].pos_x, 170);
    EXPECT_EQ(ppu.__intersected_objects[1].pos_x, 0);
    EXPECT_EQ(ppu.__intersected_objects[2].pos_x, 20);
    EXPECT_EQ(ppu.__intersected_objects[3].sprite_index, 1);
    EXPECT_EQ(ppu.__intersected_objects[4].sprite_index, 5);

    /// priority in DMG_MODE/CGB_DMG_MODE
    ppu.TEMPORARY_GB_MODE_FLAG = GB::GBModeFlag::CGB_DMG_MODE;
    ppu.__intersected_objects.clear();
    ppu.__current_line = 143; // last visible line
    ppu.__obj_high = true;
    bzero(objects, GB::ORAM_SIZE);

    objects[11].pos_y = 144;
    objects[11].pos_x = 170;

    objects[13].pos_y = 144;
    objects[13].pos_x = 0;

    objects[15].pos_y = 144;
    objects[15].pos_x = 20;

    objects[17].pos_y = 144;
    objects[17].pos_x = 10;
    objects[17].sprite_index = 1;

    objects[18].pos_y = 144;
    objects[18].pos_x = 10;
    objects[18].sprite_index = 5;

    for (int i = 0; i < 40; ++i) {
        ppu.add_oram_object(i);
    }
    EXPECT_EQ(ppu.__intersected_objects.size(), 5);
    EXPECT_EQ(ppu.__intersected_objects[0].pos_x, 0);
    EXPECT_EQ(ppu.__intersected_objects[1].sprite_index, 1);
    EXPECT_EQ(ppu.__intersected_objects[2].sprite_index, 5);
    EXPECT_EQ(ppu.__intersected_objects[3].pos_x, 20);
    EXPECT_EQ(ppu.__intersected_objects[4].pos_x, 170);
}

TEST(PPU, oram_search_pipeline) {
    ORAM oram;
    VRAM vram;

    PPU ppu(&oram, &vram);
    EXPECT_EQ(ppu.__current_state, PPU::State::FirstOamSearch);

    /// first oram search -> oram search
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

    /// oram search -> oram search
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

    /// last oram search -> render
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