#include "device/GB_ppu.h"

namespace GB::device {

PPU::PPU(ORAM *oram, VRAM *vram)
: __oram(oram), __vram(vram), __regs(), __current_state(FirstOamSearch)
{}

// TODO(dolovnyak)  there is strange clock behavior depending on SCX register
//                  https://gbdev.io/pandocs/STAT.html (In section properties STAT mode)
//                  https://habr.com/ru/post/155323/ (In section LCDMODE_LYXX_OAM)
void PPU::add_oram_object() {
    // TODO(dolovnyak)  if (__dma.running)
    //                      return;
    if (__intersected_objects.size() == MAX_INTERSECTED_OBJECTS)
        return;

    u8 object_height = ::bit_n(2, __regs.LCDC) ? LARGE_OBJECT_HEIGHT : NORMAL_OBJECT_HEIGHT;

    // TODO(dolovnyak) ugly way to get object
    const PPU::Object& current_object = reinterpret_cast<PPU::Object*>
            (__oram->get_memory_buffer_ref().get_data_addr())[__next_object_index - 1];

    if (current_object.pos_y - OBJ_Y_INDENT <= __regs.LY &&
        current_object.pos_y - OBJ_Y_INDENT + object_height > __regs.LY) {
        if (TEMPORARY_GB_MODE_FLAG == CGB_MODE) {
            __intersected_objects.push_back(current_object);
        }
        else if (TEMPORARY_GB_MODE_FLAG == DMG_MODE || TEMPORARY_GB_MODE_FLAG == CGB_DMG_MODE) {
            // TODO sort by x coordinate
        }


        // TODO(dolovnyak) add object considering priority
    }
}

void PPU::step() {

    if (!__counter.is_ready()) {
        __counter.pay(1_CLKCycles);   //TODO(hgranule) what I need to pass in pay() parameter?
        return;
    }

    switch (__current_state) {

        case State::FirstOamSearch:
            set_STAT_mode(STAT_SearchingOAM);

            //TODO(dolovnyak, hgranule) throw OAM_SEARCHING interrupt if it's possible.
            //TODO(dolovnyak, hgranule) throw LY=LYC interrupt if it's needed and possible.

            __next_object_index = 1;
            __intersected_objects.clear();
            add_oram_object();

            __current_state = State::SearchOam;
            __counter.step(OBJECT_SEARCH_TIME);
            break;


        case State::SearchOam:
            ++__next_object_index;
            add_oram_object();

            if (__next_object_index == ORAM_OBJECTS_NUM) {
                __current_state = State::Render;
            }
            __counter.step(OBJECT_SEARCH_TIME);
            break;


        case State::Render:
            set_STAT_mode(STAT_Render);
            // render line
            // put it into lcd screen

            __current_state = State::HBlank;
            __counter.step(__render_time);
            break;


        case State::HBlank:
            set_STAT_mode(STAT_Hblank);

            __current_state = State::EndLine;
            __counter.step(SCANLINE_TIME - ORAM_SEARCH_TIME - __render_time - ENDLINE_TIME);
            break;


        case State::VBlank:
            set_STAT_mode(STAT_Vblank);

            __current_state = State::EndLine;
            __counter.step(SCANLINE_TIME - ENDLINE_TIME);
            break;


        case State::LastVblank:
            set_STAT_mode(STAT_Vblank);
            __regs.LY = 0;

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
            ++__regs.LY;

            if (__regs.LY <= 143) {
                __current_state = State::SearchOam;
            }
            else if (__regs.LY <= 152) {
                __current_state = State::VBlank;
            }
            else if (__regs.LY == 153) {
                __current_state = State::LastVblank;
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
