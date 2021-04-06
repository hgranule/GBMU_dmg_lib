#include "device/GB_interrupt.h"

namespace GB::device {

    using IC    = InterruptController;
    using Mbus  = ::GB::memory::BusInterface;

    // Memory mapping

    static void MBusSetIF(IC* ic, Word, Byte val) {
        ic->SetIF(val);
    }

    static void MBusGetIF(IC* ic, Word) {
        ic->GetIF();
    }

    static void MBusSetIE(IC* ic, Word, Byte val) {
        ic->SetIE(val);
    }

    static void MBusGetIE(IC* ic, Word) {
        ic->GetIE();
    }

    void IC::MapToMemory(Mbus& memoryBus) {
        memoryBus.MapVAddr(memory::IF_VADDR, Mbus::ReadCmd(MBusGetIF), Mbus::WriteCmd(MBusSetIF), this);
        memoryBus.MapVAddr(memory::IE_VADDR, Mbus::ReadCmd(MBusGetIE), Mbus::WriteCmd(MBusSetIE), this);
    }

}