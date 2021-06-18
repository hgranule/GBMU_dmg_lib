#include "common/GB_utilities.h"
#include "device/GB_ppu.h"

namespace GB::device {

PPU::PPU(ORAM *oram, VRAM *vram)
: __oram(oram), __vram(vram),
__current_state(State::FirstOamSearch),
__render_time(0),
__counter(0),
__next_object_index(0),
__stat_interrupt_requested(false) {

    __intersected_objects.reserve(10);

    // LCDC
    __bg_window_enable                  = ::bit_n(0, LCDC_INIT_VALUE);
    __obj_enable                        = ::bit_n(1, LCDC_INIT_VALUE);
    __obj_high                          = ::bit_n(2, LCDC_INIT_VALUE);
    __bg_tile_map_memory_shifted        = ::bit_n(3, LCDC_INIT_VALUE);
    __bg_window_tiles_memory_shifted    = ::bit_n(4, LCDC_INIT_VALUE);
    __window_enable                     = ::bit_n(5, LCDC_INIT_VALUE);
    __window_tile_map_memory_shifted    = ::bit_n(6, LCDC_INIT_VALUE);
    __lcd_enable                        = ::bit_n(7, LCDC_INIT_VALUE);

    // STAT
    __stat_mode                 = static_cast<STAT_Mode>(::bit_slice(1, 0, STAT_INIT_VALUE));
    __ly_equal_to_lyc_flag      = ::bit_n(2, STAT_INIT_VALUE);
    __hblank_interrupt_enable   = ::bit_n(3, STAT_INIT_VALUE);
    __vblank_interrupt_enable   = ::bit_n(4, STAT_INIT_VALUE);
    __oram_interrupt_enable     = ::bit_n(5, STAT_INIT_VALUE);
    __ly_interrupt_enable       = ::bit_n(6, STAT_INIT_VALUE);

    // LY LYX
    __current_line      = LY_INIT_VALUE;
    __line_to_compare   = LYC_INIT_VALUE;
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
 *
 *              When CGB mode intersected objects sorted only by oram index
 *              In CGB_DMG_MODE/DMG_MODE (need to check about DMG_MODE) it sorted by x coordinate, then by oram index.
 */
void PPU::add_oram_object(int current_object_index) {
    // TODO(dolovnyak)  if (__dma.running)
    //                      return;
    if (__intersected_objects.size() == MAX_INTERSECTED_OBJECTS)
        return;

    u8 object_height = __obj_high ? LARGE_OBJECT_HEIGHT : NORMAL_OBJECT_HEIGHT;

    // TODO(dolovnyak, hgranule) ugly way to get object
    const PPU::Object& current_object = reinterpret_cast<const PPU::Object *>
            (__oram->get_memory_buffer_ref().get_data_addr())[current_object_index];

    if (current_object.pos_y <= __current_line + OBJ_Y_INDENT
        && current_object.pos_y + object_height > __current_line + OBJ_Y_INDENT) {
        if (TEMPORARY_GB_MODE_FLAG == CGB_MODE) {
            __intersected_objects.push_back(current_object);
        }
        // TODO(dolovnyak) CGB_DMG_MODE - exactly correct (from PanDocs), but need to check about DMG_MODE.
        else if (TEMPORARY_GB_MODE_FLAG == DMG_MODE || TEMPORARY_GB_MODE_FLAG == CGB_DMG_MODE) {
            auto comparator = [](const Object& new_obj, const Object& cur_obj){ return new_obj.pos_x < cur_obj.pos_x; };
            insert_sorting(__intersected_objects, current_object, comparator);
        }
    }
}

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

           if (!__stat_interrupt_requested && __oram_interrupt_enable) {
               // TODO(dolovnyak, hgranule) request oram interrupt
               __stat_interrupt_requested = true;
           }

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


        /* TODO
         *          Implement FETCHING_STATE_MACHINE with pixelFIFO.
         *          Fetching sate machine states are something like:
         *          FETCHER_GET_TILE, 2 clocks
         *          FETCHER_GET_TILE_DATA_LOWER, 2 clocks
         *          FETCHER_GET_TILE_DATA_HIGH, 2 clocks
         *          FETCHER_PUSH, 2 clocks
         *          ................
         */
        case State::Render:
            __stat_mode = STAT_Mode::STAT_Render;

            // render line
            // put it into lcd screen

            __current_state = State::HBlank;
            __counter.step(__render_time);
            break;


        case State::HBlank:
            __stat_mode = STAT_Mode::STAT_Hblank;

            if (!__stat_interrupt_requested && __hblank_interrupt_enable) {
                // TODO(dolovnyak, hgranule) request hblank interrupt
                __stat_interrupt_requested = true;
            }

            __current_state = State::EndLine;
            __counter.step(SCANLINE_TIME - ORAM_SEARCH_TIME - __render_time - ENDLINE_TIME);
            break;


        case State::VBlank:
            __stat_mode = STAT_Mode::STAT_Vblank;

            if (__current_line == 143)
                ; // TODO(dolovnyak, hgranule) request VBL interrupt

            if (!__stat_interrupt_requested && __vblank_interrupt_enable) {
                // TODO(dolovnyak, hgranule) request stat vblank interrupt
                __stat_interrupt_requested = true;
            }

            __current_state = State::EndLine;
            __counter.step(SCANLINE_TIME - ENDLINE_TIME);
            break;


        /**
         * NOTE:    LY (current_line) sets to 0 on line 153 during last vblank first few ticks.
         */
        case State::LastVblank:
            __stat_mode = STAT_Mode::STAT_Vblank;
            __current_line = 0;

            ly_equal_lyc_interrupt_handle();
            __current_state = State::LastEndLine;
            __counter.step(SCANLINE_TIME - ENDLINE_TIME);
            break;


        // TODO(dolovnyak)  In this source:
        //                  https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf (8.8)
        //                  there are accurate timings with different behavior on CGB/DMG, now I don't implemented
        //                  them fully completely.
        case State::EndLine:
            ++__current_line;

            /// update line and state
            if (__current_line <= 143) {
                __current_state = State::FirstOamSearch;
            }
            else {
                __current_state = __current_line == 152 ? State::LastVblank : State::VBlank;
            }

            // TODO(dolovnyak) on CGB_MODE it updates correct, but for DMG_MODE and CGB_DMG_MODE it updates
            //                 during first four ticks on next line
            //                 https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf (8.9.1, 8.9.2)
            ly_equal_lyc_interrupt_handle();

            __counter.step(ENDLINE_TIME);
            break;


        case State::LastEndLine:
            __current_state = State::FirstOamSearch;

            __counter.step(ENDLINE_TIME);
            break;

    }
    __counter.pay(1_CLKCycles);
}

void PPU::ly_equal_lyc_interrupt_handle() {
    if (__current_line == __line_to_compare) {
        if (__ly_equal_to_lyc_flag) {
            __ly_equal_to_lyc_flag = false;
        } else {
            __stat_interrupt_requested = true;
            __ly_equal_to_lyc_flag = true;
            if (__ly_interrupt_enable)
                ;  // TODO(dolovnyak, hgranule) request ly compare to lyc interrupt
        }

    }
}

}  // namespace GB::device
