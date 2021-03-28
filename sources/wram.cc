#include "device/GB_wram.h"

#include "memory/GB_vaddr.h"


namespace GB {

    WRAM::WRAM()
    : __memory(MAX_SIZE)
    , __regs()
    {
    }

    WRAM::WRAM(const WRAM& other)
    : __memory(other.__memory)
    , __regs(other.__regs)
    {
    }

    WRAM::WRAM(WRAM&& other)
    : __memory(std::move(other.__memory))
    , __regs(other.__regs)
    {
    }

    WRAM::~WRAM()
    {
    }

    WRAM& WRAM::operator=(const WRAM& other) {
        if (&other != this) {
            __memory = other.__memory;
            __regs = other.__regs;
        }

        return *this;
    }

    WRAM& WRAM::operator=(WRAM&& other) {
        if (&other != this) {
            __memory = std::move(other.__memory);
            __regs = other.__regs;
        }

        return *this;
    }

    static Byte __MemReadSVBK(WRAM* dev, Word) {
        return dev->GetSVBK();
    }

    static void __MemWriteSVBK(WRAM* dev, Word, Byte value) {
        dev->SetSVBK(value);
    }

    static Byte __MemBankRead(WRAM* dev, Word vAddr) {
        const Word innerAddr = vAddr - memory::WRAM0_BASE_VADDR;
        return dev->Read(innerAddr);
    }

    static void __MemBankWrite(WRAM* dev, Word vAddr, Byte data) {
        const Word innerAddr = vAddr - memory::WRAM0_BASE_VADDR;
        dev->Write(innerAddr, data);
    }

    static Byte __MemBankEchoRead(WRAM* dev, Word vAddr) {
        const Word innerAddr = vAddr - memory::WRAM0_ECHO_BASE_VADDR;
        return dev->Read(innerAddr);
    }

    static void __MemBankEchoWrite(WRAM* dev, Word vAddr, Byte data) {
        const Word innerAddr = vAddr - memory::WRAM0_ECHO_BASE_VADDR;
        dev->Write(innerAddr, data);
    }

    void WRAM::MapToMemory(memory::UMBus& memoryBus) {
        memoryBus.MapVAddr( memory::SVBK_VADDR
                          , memory::UMBus::ReadCmd(__MemReadSVBK)
                          , memory::UMBus::WriteCmd(__MemWriteSVBK)
                          , this);
        memoryBus.MapVAddr( memory::WRAM0_BASE_VADDR, memory::WRAMX_LAST_VADDR
                          , memory::UMBus::ReadCmd(__MemBankRead)
                          , memory::UMBus::WriteCmd(__MemBankWrite)
                          , this);
        memoryBus.MapVAddr( memory::WRAM0_ECHO_BASE_VADDR, memory::WRAMX_ECHO_LAST_VADDR
                          , memory::UMBus::ReadCmd(__MemBankEchoRead)
                          , memory::UMBus::WriteCmd(__MemBankEchoWrite)
                          , this);
    }

}