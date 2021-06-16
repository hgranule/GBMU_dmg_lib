#include "common/GB_utilities.h"
#include "device/GB_ppu.h"

namespace GB::device {

PPU::PPU(ORAM *oram, VRAM *vram)
: __oram(oram), __vram(vram), __current_state(State::FirstOamSearch) {

    __bg_window_enable = ::bit_n(0, LCDC_INIT_VALUE);
    __obj_enable = ::bit_n(1, LCDC_INIT_VALUE);
    __obj_high = ::bit_n(2, LCDC_INIT_VALUE);
    __bg_tile_map_memory_shifted = ::bit_n(3, LCDC_INIT_VALUE);
    __bg_window_tiles_memory_shifted = ::bit_n(4, LCDC_INIT_VALUE);
    __window_enable = ::bit_n(5, LCDC_INIT_VALUE);
    __window_tile_map_memory_shifted = ::bit_n(6, LCDC_INIT_VALUE);
    __lcd_enable = ::bit_n(7, LCDC_INIT_VALUE);

    __stat_mode = static_cast<STAT_Mode>(::bit_slice(1, 0, STAT_INIT_VALUE));
    __ly_equal_to_lyc = ::bit_n(2, STAT_INIT_VALUE);
    __hblank_interrupt_enable = ::bit_n(3, STAT_INIT_VALUE);
    __vblank_interrupt_enable = ::bit_n(4, STAT_INIT_VALUE);
    __oram_interrupt_enable = ::bit_n(5, STAT_INIT_VALUE);
    __ly_interrupt_enable = ::bit_n(6, STAT_INIT_VALUE);
}

// TODO(dolovnyak)  there is strange clock behavior depending on SCX register
//                  https://gbdev.io/pandocs/STAT.html (In section properties STAT mode)
//                  https://habr.com/ru/post/155323/ (In section LCDMODE_LYXX_OAM)
/**
 * @brief:      Adds one object by the current index if it is crossed by a current line.
 *
 * @details:    This function will add object by current index to intersected objects if this object intersected by
 *              current line and there is no dma search right now, and number of intersected objects equal or lower than
 *              ten.
 *
 *              In GameBoy - coordinates of objects are specified for the lower right corner 8x16 object.
 *              Top left corner of object calculated using: top_left_X = X - 8, top_left_Y = Y - 16.
 *              Y=0 hides 8x8 and 8x16 both objects.
 *              Y=2 hides an 8x8 object, but display last two rows of an 8x16 object.
 *              Y=16 displays both 8x8 8x16 objects at the top of the screen.
 *              Y=144 displays 8x16 object aligned with the bottom of the screen and 8x8 on line 136.
 *
 *              https://gbdev.io/pandocs/OAM.html
 */
void PPU::add_oram_object(int current_object_index) {
    // TODO(dolovnyak)  if (__dma.running)
    //                      return;
    if (__intersected_objects.size() == MAX_INTERSECTED_OBJECTS)
        return;

    u8 object_height = __obj_high ? LARGE_OBJECT_HEIGHT : NORMAL_OBJECT_HEIGHT;

    // TODO(dolovnyak, hgranule) not perfect way to get object
    const PPU::Object& current_object = reinterpret_cast<PPU::Object*>
            (__oram->get_memory_buffer_ref().get_data_addr())[current_object_index];

    if (current_object.pos_y - OBJ_Y_INDENT <= __current_line &&
        current_object.pos_y - OBJ_Y_INDENT + object_height > __current_line) {
        if (TEMPORARY_GB_MODE_FLAG == CGB_MODE) {
            __intersected_objects.push_back(current_object);
        }
        // TODO(dolovnyak) CGB_DMG_MODE - exactly correct, but need to check about DMG_MODE.
        else if (TEMPORARY_GB_MODE_FLAG == DMG_MODE || TEMPORARY_GB_MODE_FLAG == CGB_DMG_MODE) {
            auto comparator = [](const Object& new_obj, const Object& cur_obj){ return new_obj.pos_x < cur_obj.pos_x; };
            insert_sorting(__intersected_objects, current_object, comparator);
        }
    }
}

/* TODO
 *          Implement FETCHING_STATE_MACHINE (or just added states to current machine).
 *          Fetching sate machine states are something like:
 *          FETCHER_GET_TILE, 8 clocks
 *          FETCHER_GET_TILE_DATA_LOWER, 8 clocks
 *          FETCHER_GET_TILE_DATA_HIGH, 8 clocks
 *          FETCHER_PUSH, 8 clocks
 *          ................
 */
/**
 * Current state machine:
 *      OBJECT_SEARCH_TIME = ORAM_SEARCH_TIME / ORAM_OBJECTS_NUM = 2, SCANLINE_TIME = 456, ENDLINE_TIME = 4,
 *      __render_time = calculated depending on founded objects and SCX register
 *
 *      FirstOamSearch(CurObj = 0) -> SearchOam	{                                                        }
 *      SearchOam (cur obj <= 38) -> SearchOam	{ [Step = OBJECT_SEARCH_TIME] ++current_searching_object }
 *      SearchOam (cur obj = 39) -> Render      {                                                        }
 *      Render -> Hblank                      	[Step = __render_time]
 *      Hblank -> EndLine                       [Step = SCANLINE_TIME - ORAM_SEARCH_TIME - __render_time - ENDLINE_TIME]
 *      EndLine (0 <= LY <= 143) -> SearchOam   {                              }
 *      EndLine (144 <= LY <= 152) -> Vblank    { [Step = ENDLINE_TIME] (++LY) }
 *      EndLine (LY = 153) -> LastVblank        {                              }
 *      Vblank -> EndLine                       [Step = SCANLINE_TIME - ENDLINE_TIME]
 *      LastVblank -> LastEndLine               [Step = SCANLINE_TIME - ENDLINE_TIME] (++LY)
 *      LastEndLine -> FirstOamSearch           [Step = ENDLINE_TIME]
 */
void PPU::step() {

    if (!__counter.is_ready()) {
        __counter.pay(1_CLKCycles);   //TODO(hgranule) what I need to pass in pay() parameter?
        return;
    }

    switch (__current_state) {

        case State::FirstOamSearch:
            __stat_mode = STAT_Mode::STAT_SearchingOAM;

            //TODO(dolovnyak, hgranule) throw OAM_SEARCHING interrupt if it's possible.
            //TODO(dolovnyak, hgranule) throw LY=LYC interrupt if it's needed and possible.

            __next_object_index = 1;
            __intersected_objects.clear();
            add_oram_object(__next_object_index - 1);

            __current_state = State::SearchOam;
            __counter.step(OBJECT_SEARCH_TIME);
            break;


        case State::SearchOam:
            ++__next_object_index;
            add_oram_object(__next_object_index - 1);

            if (__next_object_index == ORAM_OBJECTS_NUM) {
                __current_state = State::Render;
            }
            __counter.step(OBJECT_SEARCH_TIME);
            break;


        case State::Render:
            __stat_mode = STAT_Mode::STAT_Render;

            // render line
            // put it into lcd screen

            __current_state = State::HBlank;
            __counter.step(__render_time);
            break;


        case State::HBlank:
            __stat_mode = STAT_Mode::STAT_Hblank;

            __current_state = State::EndLine;
            __counter.step(SCANLINE_TIME - ORAM_SEARCH_TIME - __render_time - ENDLINE_TIME);
            break;


        case State::VBlank:
            __stat_mode = STAT_Mode::STAT_Vblank;

            __current_state = State::EndLine;
            __counter.step(SCANLINE_TIME - ENDLINE_TIME);
            break;


        case State::LastVblank:
            __stat_mode = STAT_Mode::STAT_Vblank;
            __current_line = 0;

            __current_state = State::LastEndLine;
            __counter.step(SCANLINE_TIME - ENDLINE_TIME);
            break;


        // TODO(dolovnyak)  EndLine state was taken from https://habr.com/ru/post/155323/ but this source is not
        //                  reliable. In this source:
        //                  https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf(8.8)
        //                  there are strange, but accurate timings with different behavior on CGB/DMG,
        //                  in the future, need to check on tests roms.
        //
        // TODO(dolovnyak)  In the future need to check with test roms (if it will possible) is LY update
        //                  timings correct.
        //                  Also need to check is it possible for developers handle LYC = 0.
        case State::EndLine:
            ++__current_line;

            if (__current_line <= 143) {
                __current_state = State::SearchOam;
            }
            else {
                __current_state = __current_line == 153 ? State::LastVblank : State::VBlank;
            }
            __counter.step(ENDLINE_TIME);
            break;


        case State::LastEndLine:
            __current_state = State::SearchOam;
            __counter.step(ENDLINE_TIME);
            break;

    }
    __counter.pay(1_CLKCycles);
}

}  // namespace GB::device
