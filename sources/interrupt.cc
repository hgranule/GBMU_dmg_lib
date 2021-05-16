#include "device/GB_interrupt.h"

namespace GB::device {

    using IC    = InterruptController;
    using Mbus  = ::GB::memory::BusInterface;

    // Memory mapping

    static void write_IF_reg(IC* ic, word_t, byte_t val) {
        ic->set_IF_reg(val);
    }

    static void read_IF_reg(IC* ic, word_t) {
        ic->get_IF_reg();
    }

    static void write_IE_reg(IC* ic, word_t, byte_t val) {
        ic->set_IE_reg(val);
    }

    static void read_IE_reg(IC* ic, word_t) {
        ic->get_IE_reg();
    }

    void IC::map_to_memory(Mbus& mem_bus) {
        mem_bus.MapVAddr(memory::IF_VADDR, Mbus::ReadCmd(read_IF_reg), Mbus::WriteCmd(write_IF_reg), this);
        mem_bus.MapVAddr(memory::IE_VADDR, Mbus::ReadCmd(read_IE_reg), Mbus::WriteCmd(write_IE_reg), this);
    }

}  // namespace GB::device
