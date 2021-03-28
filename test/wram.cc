#include <utility>
#include <algorithm>

#include "gtest/gtest.h"

#include "GB_config.h"
#include "device/GB_wram.h"

namespace {

    inline Byte SVBKReservedBits(Byte value) {
        return value | GB::WRAM::Registers::RESERVED_BITS;
    }

    TEST(WorkingRAM, SVBK_ReadWrite) {
        GB::WRAM    wram;

        EXPECT_EQ(SVBKReservedBits(GB::SVBK_INIT_VALUE), wram.GetSVBK());

        wram.SetSVBK(0xAB);
        EXPECT_EQ(SVBKReservedBits(0xAB), wram.GetSVBK());

        wram.SetSVBK(0x0);
        EXPECT_EQ(SVBKReservedBits(0x0), wram.GetSVBK());

        wram.SetSVBK(0x1);
        EXPECT_EQ(SVBKReservedBits(0x1), wram.GetSVBK());
    }

    TEST(WorkingRAM, SVBK_BankChoose) {
        GB::WRAM    wram;

        wram.SetSVBK(0x0);
        EXPECT_EQ(1, wram.__CurrentBankIdx());
        wram.SetSVBK(0x1);
        EXPECT_EQ(1, wram.__CurrentBankIdx());
        wram.SetSVBK(0x2);
        EXPECT_EQ(2, wram.__CurrentBankIdx());
        wram.SetSVBK(0x3);
        EXPECT_EQ(3, wram.__CurrentBankIdx());
        wram.SetSVBK(0x4);
        EXPECT_EQ(4, wram.__CurrentBankIdx());
        wram.SetSVBK(0x5);
        EXPECT_EQ(5, wram.__CurrentBankIdx());
        wram.SetSVBK(0x6);
        EXPECT_EQ(6, wram.__CurrentBankIdx());
        wram.SetSVBK(0x7);
        EXPECT_EQ(7, wram.__CurrentBankIdx());
        wram.SetSVBK(0x8);
        EXPECT_EQ(1, wram.__CurrentBankIdx());
        wram.SetSVBK(0x9);
        EXPECT_EQ(1, wram.__CurrentBankIdx());
        wram.SetSVBK(0xA);
        EXPECT_EQ(2, wram.__CurrentBankIdx());
        wram.SetSVBK(0xB);
        EXPECT_EQ(3, wram.__CurrentBankIdx());
        wram.SetSVBK(0xC);
        EXPECT_EQ(4, wram.__CurrentBankIdx());
        wram.SetSVBK(0xD);
        EXPECT_EQ(5, wram.__CurrentBankIdx());
        wram.SetSVBK(0xE);
        EXPECT_EQ(6, wram.__CurrentBankIdx());
        wram.SetSVBK(0xF);
        EXPECT_EQ(7, wram.__CurrentBankIdx());
    }

    using VA = GB::memory::VirtualAddress;

    constexpr unsigned BS = GB::WRAM::BANK_SIZE;

    inline unsigned DirectAddr(unsigned vAddr, unsigned svbkBank, unsigned minAddr, unsigned maxAddr) {
        EXPECT_TRUE(vAddr <= maxAddr);
        EXPECT_TRUE(vAddr >= minAddr);
        std::clamp(svbkBank, 1u, 7u);

        const unsigned striptedAddr = vAddr - minAddr;
        return (striptedAddr < BS)
               ? striptedAddr
               : striptedAddr%BS + svbkBank*BS;
    }

    inline unsigned DirectWRAMAddr(unsigned vAddr, unsigned svbkBank) {
        return DirectAddr(vAddr, svbkBank, GB::memory::WRAM0_BASE_VADDR, GB::memory::WRAMX_LAST_VADDR);
    }

    template <typename _Callable>
    void AddrRangePatternCall(
        std::pair<Word, Word> range,
        std::pair<Byte, Byte> pattern,
        _Callable func
    ) {
        unsigned val = pattern.first;
        for (unsigned i = range.first; i < range.second; ++i) {
            func(i, val);
            val += pattern.second;
        }
    }

    TEST(WorkingRam, MemBusBank0) {
        GB::memory::UMBus   membus;
        GB::WRAM            wram;

        wram.MapToMemory(membus);

        wram.SetSVBK(0);
        AddrRangePatternCall({VA::WRAM0_BASE_VADDR, BS}, {1, 3}, [&membus, &wram](Word addr, Byte data){
            membus.ImmWrite(addr, data);
            EXPECT_EQ(data, wram.DirectRead(DirectWRAMAddr(addr, 0)));
        });

        wram.SetSVBK(4);
        AddrRangePatternCall({VA::WRAM0_BASE_VADDR, BS}, {1, 3}, [&membus, &wram](Word addr, Byte data){
            membus.ImmWrite(addr, data);
            EXPECT_EQ(data, wram.DirectRead(DirectWRAMAddr(addr, 0)));
        });
    }

    TEST(WorkingRam, MemBusBank1) {
        GB::memory::UMBus   membus;
        GB::WRAM            wram;

        wram.MapToMemory(membus);

        Word addr = VA::WRAMX_BASE_VADDR;
        Word dist = BS;

        Byte val = random();
        Byte step = random();

        wram.SetSVBK(1);
        AddrRangePatternCall({addr, dist}, {val, step}, [&membus](Word _address, Byte _data){
            membus.ImmWrite(_address, _data);
        });

        wram.SetSVBK(1);
        AddrRangePatternCall({addr, dist}, {val, step}, [&wram](Word _address, Byte _data){
            EXPECT_EQ(_data, wram.DirectRead(DirectWRAMAddr(_address, wram.__CurrentBankIdx())));
        });

        wram.SetSVBK(0);
        AddrRangePatternCall({addr, dist}, {val, step}, [&wram](Word _address, Byte _data){
            EXPECT_EQ(_data, wram.DirectRead(DirectWRAMAddr(_address, wram.__CurrentBankIdx())));
        });

        val = random();
        step = random();

        wram.SetSVBK(1);
        AddrRangePatternCall({addr, dist}, {val, step}, [&membus, &wram](Word _address, Byte _data){
            wram.DirectWrite(DirectWRAMAddr(_address, wram.__CurrentBankIdx()), _data);
            EXPECT_EQ(_data, membus.ImmRead(_address));
        });

        val = random();
        step = random();

        wram.SetSVBK(1);
        AddrRangePatternCall({addr, dist}, {val, step}, [&membus, &wram](Word _address, Byte _data){
            membus.ImmWrite(_address, _data);
            EXPECT_EQ(_data, wram.DirectRead(DirectWRAMAddr(_address, wram.__CurrentBankIdx())));
        });
    }

    void TestMemBank(GB::memory::UMBus& mbus, GB::WRAM& wram, unsigned bank, Word address, Word dist) {
        Word addr = address;
        Word distance = dist;
        Byte val = random();
        Byte step = random();

        wram.SetSVBK(bank);

        AddrRangePatternCall({addr, distance}, {val, step}, [&mbus, &wram, bank](Word _address, Byte _data){
            mbus.ImmWrite(_address, _data);
            EXPECT_EQ(_data, wram.DirectRead(DirectWRAMAddr(_address, bank)));
        });

        AddrRangePatternCall({addr, distance}, {val, step}, [&mbus, &wram, bank](Word _address, Byte _data){
            wram.DirectWrite(DirectWRAMAddr(_address, bank), _data);
            EXPECT_EQ(_data, mbus.ImmRead(_address));
        });
    }

    TEST(WorkingRam, MemBusBank2) {
        GB::memory::UMBus   membus;
        GB::WRAM            wram;

        wram.MapToMemory(membus);
        TestMemBank(membus, wram, 2, VA::WRAMX_BASE_VADDR, BS);
    }

    TEST(WorkingRam, MemBusBank3) {
        GB::memory::UMBus   membus;
        GB::WRAM            wram;

        wram.MapToMemory(membus);
        TestMemBank(membus, wram, 3, VA::WRAMX_BASE_VADDR, BS);
    }

    TEST(WorkingRam, MemBusBank4) {
        GB::memory::UMBus   membus;
        GB::WRAM            wram;

        wram.MapToMemory(membus);
        TestMemBank(membus, wram, 4, VA::WRAMX_BASE_VADDR, BS);
    }

    TEST(WorkingRam, MemBusBank5) {
        GB::memory::UMBus   membus;
        GB::WRAM            wram;

        wram.MapToMemory(membus);
        TestMemBank(membus, wram, 5, VA::WRAMX_BASE_VADDR, BS);
    }

    TEST(WorkingRam, MemBusBank6) {
        GB::memory::UMBus   membus;
        GB::WRAM            wram;

        wram.MapToMemory(membus);
        TestMemBank(membus, wram, 6, VA::WRAMX_BASE_VADDR, BS);
    }

    TEST(WorkingRam, MemBusBank7) {
        GB::memory::UMBus   membus;
        GB::WRAM            wram;

        wram.MapToMemory(membus);
        TestMemBank(membus, wram, 7, VA::WRAMX_BASE_VADDR, BS);
    }

    constexpr unsigned EBS = VA::WRAMX_ECHO_LAST_VADDR - VA::WRAMX_ECHO_BASE_VADDR + 1;
    inline unsigned DirectECHOAddr(unsigned vAddr, unsigned svbkBank) {
        return DirectAddr(vAddr, svbkBank, GB::memory::WRAM0_ECHO_BASE_VADDR, GB::memory::WRAMX_ECHO_LAST_VADDR);
    }

    TEST(WorkingRam, MemBusEchoBank0) {
        GB::memory::UMBus   membus;
        GB::WRAM            wram;

        wram.MapToMemory(membus);

        wram.SetSVBK(0);
        AddrRangePatternCall({VA::WRAM0_BASE_VADDR, BS}, {56, 3}, [&membus](Word addr, Byte data){
            membus.ImmWrite(addr, data);
        });

        AddrRangePatternCall({VA::WRAM0_ECHO_BASE_VADDR, BS}, {56, 3}, [&wram](Word addr, Byte data){
            EXPECT_EQ(data, wram.DirectRead(DirectECHOAddr(addr, wram.__CurrentBankIdx())));
        });

        AddrRangePatternCall({VA::WRAM0_ECHO_BASE_VADDR, BS}, {47, 31}, [&membus](Word addr, Byte data){
            membus.ImmWrite(addr, data);
        });

        AddrRangePatternCall({VA::WRAM0_BASE_VADDR, BS}, {47, 31}, [&wram](Word addr, Byte data){
            EXPECT_EQ(data, wram.DirectRead(DirectWRAMAddr(addr, wram.__CurrentBankIdx())));
        });

        wram.SetSVBK(5);
        AddrRangePatternCall({VA::WRAM0_ECHO_BASE_VADDR, BS}, {56, 3}, [&membus](Word addr, Byte data){
            membus.ImmWrite(addr, data);
        });

        AddrRangePatternCall({VA::WRAM0_BASE_VADDR, BS}, {56, 3}, [&wram](Word addr, Byte data){
            EXPECT_EQ(data, wram.DirectRead(DirectWRAMAddr(addr, wram.__CurrentBankIdx())));
        });

        AddrRangePatternCall({VA::WRAM0_BASE_VADDR, BS}, {47, 31}, [&membus](Word addr, Byte data){
            membus.ImmWrite(addr, data);
        });

        AddrRangePatternCall({VA::WRAM0_ECHO_BASE_VADDR, BS}, {47, 31}, [&wram](Word addr, Byte data){
            EXPECT_EQ(data, wram.DirectRead(DirectECHOAddr(addr, wram.__CurrentBankIdx())));
        });

    }

    TEST(WorkingRam, MemBusEchoBankX) {
        GB::memory::UMBus   membus;
        GB::WRAM            wram;

        wram.MapToMemory(membus);

        wram.SetSVBK(4);
        AddrRangePatternCall({VA::WRAMX_BASE_VADDR, BS}, {59, 2}, [&membus](Word addr, Byte data){
            membus.ImmWrite(addr, data);
        });

        AddrRangePatternCall({VA::WRAMX_ECHO_BASE_VADDR, EBS}, {59, 2}, [&wram](Word addr, Byte data){
            EXPECT_EQ(data, wram.DirectRead(DirectECHOAddr(addr, wram.__CurrentBankIdx())));
        });

        AddrRangePatternCall({VA::WRAMX_ECHO_BASE_VADDR, EBS}, {41, 1}, [&membus](Word addr, Byte data){
            membus.ImmWrite(addr, data);
        });

        AddrRangePatternCall({VA::WRAMX_BASE_VADDR, EBS}, {41, 1}, [&wram](Word addr, Byte data){
            EXPECT_EQ(data, wram.DirectRead(DirectWRAMAddr(addr, wram.__CurrentBankIdx())));
        });

        wram.SetSVBK(0);
        AddrRangePatternCall({VA::WRAMX_ECHO_BASE_VADDR, EBS}, {5, 13}, [&membus](Word addr, Byte data){
            membus.ImmWrite(addr, data);
        });

        AddrRangePatternCall({VA::WRAMX_BASE_VADDR, EBS}, {5, 13}, [&wram](Word addr, Byte data){
            EXPECT_EQ(data, wram.DirectRead(DirectWRAMAddr(addr, wram.__CurrentBankIdx())));
        });

        AddrRangePatternCall({VA::WRAMX_BASE_VADDR, BS}, {41, 1}, [&membus](Word addr, Byte data){
            membus.ImmWrite(addr, data);
        });

        AddrRangePatternCall({VA::WRAMX_ECHO_BASE_VADDR, EBS}, {41, 1}, [&wram](Word addr, Byte data){
            EXPECT_EQ(data, wram.DirectRead(DirectECHOAddr(addr, wram.__CurrentBankIdx())));
        });
    }
}