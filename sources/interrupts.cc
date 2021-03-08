#include "GB_interrupt.h"

namespace GB {

    using IC    = InterruptController;
    using Mbus  = memory::UMBus;

    inline void
    IC::RequestInt(InterruptIdx interrupt) {
        __registers.IF = bit_n_set(interrupt, __registers.IF);
    }

    inline void
    IC::ResetInt(InterruptIdx interrupt) {
        __registers.IF = bit_n_reset(interrupt, __registers.IF);
    }

    inline IC::InterruptIdx
    IC::GetHighestPossibleInt() const {
        const U8 interruptsToHandle = (__registers.IF & __registers.IE);
        return InterruptIdx(bit_lsb(U16(interruptsToHandle)));
    }

    inline void
    IC::SetIE(Byte value) {
        __registers.IE = value | Registers::REG_RESERVED_BITS;
    }

    inline Byte
    IC::GetIE() const {
        return __registers.IE | Registers::REG_RESERVED_BITS;
    }

    inline void
    IC::SetIF(Byte value) {
        __registers.IF = value | Registers::REG_RESERVED_BITS;
    }

    inline Byte
    IC::GetIF() const {
        return __registers.IF | Registers::REG_RESERVED_BITS;
    }

    inline void
    IC::SetIME(bool value) {
        __registers.IME = value;
    }

    inline bool
    IC::GetIME() const {
        return __registers.IME;
    }

    // Memory mapping

    static void MBusSetIF(IC* ic, Word addr, Byte val) {
        ic->SetIF(val);
    }

    static void MBusGetIF(IC* ic, Word addr) {
        ic->GetIF();
    }

    static void MBusSetIE(IC* ic, Word addr, Byte val) {
        ic->SetIE(val);
    }

    static void MBusGetIE(IC* ic, Word addr) {
        ic->GetIE();
    }

    inline void
    IC::MapToMemory(Mbus& memoryBus) {
        memoryBus.MapVAddr(memory::IF_VADDR, Mbus::ReadCmd(MBusGetIF), Mbus::WriteCmd(MBusSetIF), this);
        memoryBus.MapVAddr(memory::IE_VADDR, Mbus::ReadCmd(MBusGetIE), Mbus::WriteCmd(MBusSetIE), this);
    }

}