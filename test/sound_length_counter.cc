#include "gtest/gtest.h"

#define protected public
#define private public

#include "device/GB_sound_length_counter.h"

namespace {
    using LengthCounter = GB::device::LengthCounter;

    TEST(LengthCounter, Initialization) {
        bool sound_status = true;
        LengthCounter length_counter(sound_status);

        EXPECT_EQ(GB::SND_LN_CNTR_INIT_VALUE, length_counter._counter);
        EXPECT_EQ(GB::SND_LN_CNTR_INIT_VALUE, length_counter._NRX1);
        EXPECT_EQ(GB::SND_LN_CNTR_INIT_VALUE, length_counter._counter_mode);
        EXPECT_EQ(GB::SND_LN_CNTR_INIT_VALUE, length_counter._current_frame_sequencer_step);
    }

    TEST(LengthCounter, FrameSequencerStepsWithotCounterMode) {
        bool sound_status = true;
        LengthCounter length_counter(sound_status);

        for (int i = 0; i < 8; i++) {
            length_counter.Step(i);
            EXPECT_EQ(GB::SND_LN_CNTR_INIT_VALUE, length_counter._NRX1);
        }
    }

    TEST(LengthCounter, SetNRX1) {
        bool sound_status = true;
        LengthCounter length_counter(sound_status);

        byte_t set_exptected_value = 0b11011011;
        byte_t counter_expected_value = 0b100101;

        length_counter.set_NRX1_reg(set_exptected_value);
        EXPECT_EQ(set_exptected_value, length_counter._NRX1);
        EXPECT_EQ(counter_expected_value, length_counter._counter);
    }

    TEST(LengthCounter, GetNRX1) {
        bool sound_status = true;
        LengthCounter length_counter(sound_status);

        byte_t set_exptected_value = 0b10000110;
        byte_t final_expected_value = 0b10111111;

        length_counter._NRX1 = set_exptected_value;
        EXPECT_EQ(final_expected_value, length_counter.get_NRX1_reg());
    }

    TEST(LengthCounter, SetNRX4) {
        bool sound_status = true;
        LengthCounter length_counter(sound_status);

        byte_t set_exptected_value = 0b10000110;

        length_counter.set_NRX4_reg(set_exptected_value);
        EXPECT_EQ(set_exptected_value, length_counter._NRX4);
        EXPECT_EQ(1, length_counter._counter_mode);
    }

    TEST(LengthCounter, GetNRX4) {
        bool sound_status = true;
        LengthCounter length_counter(sound_status);

        byte_t set_exptected_value = 0b10000110;

        length_counter._NRX4 = set_exptected_value;
        EXPECT_EQ(set_exptected_value, length_counter.get_NRX4_reg());
    }

    TEST(LengthCounter, HardSetNRX4) {
        bool sound_status = true;
        LengthCounter length_counter(sound_status);

        byte_t set_NRX1_exptected_value = 0b11011011;
        byte_t start_counter_exptected_value = 0x0;
        byte_t set_NRX4_exptected_value = 0b10000110;

        length_counter._NRX4 = set_NRX4_exptected_value;
        length_counter._NRX1 = set_NRX1_exptected_value;
        length_counter._counter = 0x1;
        length_counter._counter_mode = 1;
        length_counter._current_frame_sequencer_step = 2;
        length_counter.set_NRX4_reg(set_NRX4_exptected_value);
        EXPECT_EQ(set_NRX4_exptected_value, length_counter.get_NRX4_reg());
        EXPECT_EQ(start_counter_exptected_value, length_counter._counter);
        EXPECT_EQ(false, sound_status);
        EXPECT_EQ(set_NRX4_exptected_value, length_counter.get_NRX4_reg());
    }

    TEST(LengthCounter, FullTest) {
        bool sound_status = true;
        LengthCounter length_counter(sound_status);

        byte_t set_NRX1_exptected_value = 0b11011011;
        byte_t set_NRX4_exptected_value = 0b10000110;

        length_counter.set_NRX1_reg(set_NRX1_exptected_value);
        EXPECT_EQ(set_NRX1_exptected_value, length_counter._NRX1);
        length_counter.set_NRX4_reg(set_NRX4_exptected_value);
        EXPECT_EQ(set_NRX4_exptected_value, length_counter._NRX4);
        EXPECT_EQ(1, length_counter._counter_mode);

        for (int i = 0; i < 8; i++) {
            // if (i == 4)
            if (i == 0) {
                EXPECT_EQ(0b100101, length_counter._counter);
            }
            else if (i == 1) {
                EXPECT_EQ(0b100101, length_counter._counter);
            }
            length_counter.Step(i);
            if (i == 0) {
                EXPECT_EQ(0b11011011, length_counter._NRX1);
                EXPECT_EQ(0b100101, length_counter._counter);
            }
            else if (i == 1) {
                EXPECT_EQ(0b11011100, length_counter._NRX1);
                EXPECT_EQ(0b100100, length_counter._counter);
            }
        }
    }
}