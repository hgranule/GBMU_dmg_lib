#include "device/GB_ppu.h"

namespace GB::device {

PPU::PPU(ORAM *oram, VRAM *vram)
: __oram(oram), __vram(vram), __regs(), __current_state(FirstOamSearch)
{}

void PPU::last_endline_to_search_transition() {
    __current_state = State::SearchOam;
    __counter.step(ENDLINE_TIME);
}

// TODO(dolovnyak)  In the future need to check with test roms (if it will possible) is LY update timings correct.
//                  Also need to check is it possible for developers handle LYC = 0.
void PPU::endline_transition() {
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
}

void PPU::last_vblank_to_last_endline_transition() {
    set_STAT_mode(STAT_Vblank);
    __regs.LY = 0;

    __current_state = State::LastEndLine;
    __counter.step(SCANLINE_TIME - ENDLINE_TIME);
}

void PPU::vblank_to_endline_transition() {
    set_STAT_mode(STAT_Vblank);

    __current_state = State::EndLine;
    __counter.step(SCANLINE_TIME - ENDLINE_TIME);
}

void PPU::hblank_to_endline_transition() {
    set_STAT_mode(STAT_Hblank);

    __current_state = State::EndLine;
    __counter.step(SCANLINE_TIME - ORAM_SEARCH_TIME - __render_time - ENDLINE_TIME);
}

void PPU::render_to_hblank_transition() {
    set_STAT_mode(STAT_Render);
    // render line
    // put it into lcd screen

    __current_state = State::HBlank;
    __counter.step(__render_time);
}

// TODO(dolovnyak)  there are strange timings with different behavior on CGB/DMG, will need to check on tests roms
//                  https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf (8.8)

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
        __intersected_objects.push_back(current_object);
        // TODO(dolovnyak) add object considering priority
    }
}

void PPU::first_search_transition() {
    set_STAT_mode(STAT_SearchingOAM);

    //TODO(dolovnyak, hgranule) throw OAM_SEARCHING interrupt if it's possible.
    //TODO(dolovnyak, hgranule) throw LY=LYC interrupt if it's needed and possible.

    __next_object_index = 1;
    __intersected_objects.clear();
    add_oram_object();

    __current_state = State::SearchOam;
    __counter.step(OBJECT_SEARCH_TIME);
}

void PPU::search_to_search_transition() {
    ++__next_object_index;
    add_oram_object();

    __counter.step(OBJECT_SEARCH_TIME);
}

void PPU::search_to_render_transition() {
    ++__next_object_index;
    add_oram_object();

    __current_state = State::Render;
    __counter.step(OBJECT_SEARCH_TIME);
}

void PPU::step() {
    if (!__counter.is_ready()) {
        __counter.pay(1_CLKCycles);   //TODO(hgranule) what I need to pass in pay() parameter?
        return;
    }
    switch (__current_state) {
        case State::FirstOamSearch:
            first_search_transition();
            break;
        case State::SearchOam:
            __next_object_index == ORAM_OBJECTS_NUM - 1 ?
            search_to_render_transition() : search_to_search_transition();
            break;
        case State::Render:
            render_to_hblank_transition();
            break;
        case State::HBlank:
            hblank_to_endline_transition();
            break;
        case State::VBlank:
            vblank_to_endline_transition();
            break;
        case State::LastVblank:
            last_vblank_to_last_endline_transition();
            break;
        case State::EndLine:
            endline_transition();
            break;
        case State::LastEndLine:
            last_endline_to_search_transition();
            break;
    }
    __counter.pay(1_CLKCycles);
}

}
