#include "device/GB_ppu.h"

namespace GB::device {

void PPU::last_endline_to_search_transition() {
    __current_state = State::Search;
    __counter.step(ENDLINE_TIME);
}

// TODO(dolovnyak)  In the future need to check with test roms (if it will possible) is LY update timings correct.
//                  Also need to check is it possible for developers handle LYC = 0.
void PPU::endline_transition() {
    ++__regs.LY;
    if (__regs.LY <= 143) {
        __current_state = State::Search;
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
    __counter.step(SCANLINE_TIME - OAM_SEARCH_TIME - __render_time - ENDLINE_TIME);
}

void PPU::render_to_hblank_transition() {
    set_STAT_mode(STAT_Render);
    // render line
    // put it into lcd screen

    __current_state = State::HBlank;
    __counter.step(__render_time);
}

// TODO(dolovnyak)  there is strange timings with different behavior on CGB/DMG, will need to check on tests roms
//                  https://github.com/AntonioND/giibiiadvance/blob/master/docs/TCAGBD.pdf (8.8)

// TODO(dolovnyak)  need to check is it correct that the OAM Mode interrupt is called before LY=LYC interrupt

// TODO(dolovnyak)  there is strange clock behavior depending on SCX register
//                  https://gbdev.io/pandocs/STAT.html (In section properties STAT mode)
//                  https://habr.com/ru/post/155323/ (In section LCDMODE_LYXX_OAM)

// TODO(dolovnyak)  maybe in the future replace finding objects into render_to_hblank_transition, because oam busy
//                  in both modes and it's data doesn't change. And also it will allow implement conflict with
//                  DMA search which started during OAM search.
/**
 * @details:    * Set STAT mode to searching OAM.
 *              * Request OAM_SEARCH interrupt if there is necessary STAT flag, and this is the first scanline interrupt
 *                (only one interrupt could be requested during scanline).
 *              * Check and set LY=LYC (It should be in the start of scanline).
 *              * Request LY=LYC interrupt if there is necessary STAT flag, and this is the first scanline interrupt.
 *              * Find objects (maximum first ten).
 *              * Calculate __render_time depending on found objects and SCX register.
 */
void PPU::search_to_render_transition() {
    set_STAT_mode(STAT_SearchingOAM);
    // find objects
    // calculate render_time

    __current_state = State::Render;
    __counter.step(OAM_SEARCH_TIME);
}

void PPU::step() {
    if (!__counter.is_ready()) {
        __counter.pay(1);   //TODO(hgranule) what I need to pass in pay() parameter?
        return;
    }
    switch (__current_state) {
        case State::Search:
            search_to_render_transition();
            return;
        case State::Render:
            render_to_hblank_transition();
            return;
        case State::HBlank:
            hblank_to_endline_transition();
            return;
        case State::VBlank:
            vblank_to_endline_transition();
            return;
        case State::LastVblank:
            last_vblank_to_last_endline_transition();
            return;
        case State::EndLine:
            endline_transition();
            return;
        case State::LastEndLine:
            last_endline_to_search_transition();
            return;
    }
}

}
