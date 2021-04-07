#include "device/GB_joypad.h"

#include "memory/GB_vaddr.h"

namespace GB::device {

    static Byte MemBusGetP1(JoyPad* jp, Word) {
        return jp->GetP1();
    }

    static void MemBusSetP1(JoyPad* jp, Word, Byte data) {
        jp->SetP1(data);
    }

    void JoyPad::MapToMemory(::GB::memory::BusInterface& memBus) {
        memBus.MapVAddr( ::GB::memory::VirtualAddress::P1_VADDR
                       , ::GB::memory::BusInterface::ReadCmd(&MemBusGetP1)
                       , ::GB::memory::BusInterface::WriteCmd(&MemBusSetP1)
                       , this);
    }

}