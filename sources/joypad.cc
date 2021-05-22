#include "device/GB_joypad.h"
#include "memory/GB_vaddr.h"

namespace GB::device {

    // static byte_t read_P1_reg(JoyPad* jp, word_t) {
    //     return jp->get_P1_reg();
    // }

    // static void write_P1_reg(JoyPad* jp, word_t, byte_t data) {
    //     jp->set_P1_reg(data);
    // }

    // void JoyPad::map_to_memory(::GB::memory::BusInterface& mem_bus) {
    //     mem_bus.MapVAddr(::GB::memory::VirtualAddress::P1_VADDR
    //                    , ::GB::memory::BusInterface::ReadCmd(&read_P1_reg)
    //                    , ::GB::memory::BusInterface::WriteCmd(&write_P1_reg)
    //                    , this);
    // }

}  // namespace GB::device
