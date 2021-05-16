#include "gtest/gtest.h"
#include "memory/GB_bus.h"
#include "common/GB_macro.h"

namespace {

    struct SimpleMemMappedDevice {
        u8     registerA;
        u8     registerB;

        static byte_t ReadA(SimpleMemMappedDevice* dev, word_t) {
            return dev->registerA;
        }

        static byte_t ReadB(SimpleMemMappedDevice* dev, word_t) {
            return dev->registerB;
        }

        static void Write(SimpleMemMappedDevice* dev, word_t addr, byte_t data) {
            byte_t& devRegister = (addr < 0x8000)
                                ? dev->registerA : dev->registerB;
            devRegister = data;
        }

        static byte_t UndefinedRead(SimpleMemMappedDevice*, word_t) {
            return 0x42;
        }

        static void UndefinedWrite(SimpleMemMappedDevice*, word_t, byte_t) {
        }
    };

    TEST(MemoryBus, SimpleMemoryMapDevice) {
        GB::memory::BusInterface       memBus;
        SimpleMemMappedDevice   dev;

        // fill memory tables
        memBus.MapVAddr(0x0000, 0x7FFF,
                        GB::memory::BusInterface::ReadCmd(SimpleMemMappedDevice::ReadA),
                        GB::memory::BusInterface::WriteCmd(SimpleMemMappedDevice::Write),
                        &dev);

        memBus.MapVAddr(0x8000, 0xEFFF,
                        GB::memory::BusInterface::ReadCmd(SimpleMemMappedDevice::ReadB),
                        GB::memory::BusInterface::WriteCmd(SimpleMemMappedDevice::Write),
                        &dev);
        
        memBus.MapVAddr(0xF000, 0xFFFF,
                        GB::memory::BusInterface::ReadCmd(SimpleMemMappedDevice::UndefinedRead),
                        GB::memory::BusInterface::WriteCmd(SimpleMemMappedDevice::UndefinedWrite));

        // try to write and read the A register address range
        memBus.ImmWrite(0x0013, 0xAB);
        EXPECT_EQ(memBus.ImmRead(0x0000), 0xAB);
        EXPECT_EQ(dev.registerA, 0xAB);
        memBus.ImmWrite(0x0000, 0xAC);
        EXPECT_EQ(memBus.ImmRead(0x1000), 0xAC);
        EXPECT_EQ(dev.registerA, 0xAC);
        memBus.ImmWrite(0x7FFF, 0xAE);
        EXPECT_EQ(memBus.ImmRead(0x7FFF), 0xAE);
        EXPECT_EQ(dev.registerA, 0xAE);

        // try to write and read the B register address range
        memBus.ImmWrite(0x8000, 0xBB);
        EXPECT_EQ(memBus.ImmRead(0xEFFF), 0xBB);
        EXPECT_EQ(dev.registerB, 0xBB);
        memBus.ImmWrite(0xEFFF, 0xBC);
        EXPECT_EQ(memBus.ImmRead(0xABCD), 0xBC);
        EXPECT_EQ(dev.registerB, 0xBC);
        memBus.ImmWrite(0xABCD, 0xBD);
        EXPECT_EQ(memBus.ImmRead(0x8000), 0xBD);
        EXPECT_EQ(dev.registerB, 0xBD);

        // try to write and read to the undefined range
        memBus.ImmWrite(0xF000, 0x01);
        EXPECT_EQ(memBus.ImmRead(0xFFFF), 0x42);
        memBus.ImmWrite(0xFFFF, 0x02);
        EXPECT_EQ(memBus.ImmRead(0xF000), 0x42);
        memBus.ImmWrite(0xFF01, 0x03);
        EXPECT_EQ(memBus.ImmRead(0xFF01), 0x42);

        // check the last A and B registers values
        EXPECT_EQ(memBus.ImmRead(0xBCDE), 0xBD);
        EXPECT_EQ(dev.registerB, 0xBD);
        EXPECT_EQ(memBus.ImmRead(0x1234), 0xAE);
        EXPECT_EQ(dev.registerA, 0xAE);
    }
}

namespace {

    using MemorySpace = std::array<byte_t, GB::memory::BusInterface::VIRTUAL_MEMORY_SIZE>;

    struct VAddressSpace {
        MemorySpace& memoryLink;

        VAddressSpace(MemorySpace& memoryOrigin)
        : memoryLink(memoryOrigin)
        {
        }

        static byte_t Read(VAddressSpace *dev, word_t vAddr) {
            return dev->memoryLink[vAddr];
        }

        static void Write(VAddressSpace *dev, word_t vAddr, byte_t data) {
            dev->memoryLink[vAddr] = data;
        }

    };

    void RandomVAGen(MemorySpace& space) {
        for (auto& byte : space)
            byte = static_cast<u8>(::bit_slice(7, 0, ::random()));
    }

    template <typename _Func>
    void ForAllVirtualAddress(_Func func) {
        for (unsigned vAddr = 0; vAddr < GB::memory::BusInterface::VIRTUAL_MEMORY_SIZE; ++vAddr)
            func(vAddr);
    }

    TEST(MemoryBus, FullAddressSpaceTest) {
        GB::memory::BusInterface       memBus;
        MemorySpace             memorySpace;
        VAddressSpace           vAddrSpace(memorySpace);

        RandomVAGen(memorySpace);
        memBus.MapVAddr(0x0000, 0xFFFF
                        , GB::memory::BusInterface::ReadCmd(VAddressSpace::Read)
                        , GB::memory::BusInterface::WriteCmd(VAddressSpace::Write)
                        , &vAddrSpace);
        
        ForAllVirtualAddress([&](word_t vAddr){
            EXPECT_EQ(memorySpace[vAddr], memBus.ImmRead(vAddr));
        });
        
        ForAllVirtualAddress([&](word_t vAddr){
            memBus.ImmWrite(vAddr, static_cast<byte_t>(::bit_slice(7, 0, vAddr)) );
        });

        ForAllVirtualAddress([&](word_t vAddr){
            EXPECT_EQ(memorySpace[vAddr], static_cast<byte_t>(::bit_slice(7, 0, vAddr)) );
        });
    }
}

namespace {

    void DummyWrite(void*, word_t, byte_t) {
    }

    byte_t DummyRead(void*, word_t) {
        return 0x42;
    }

    TEST(MemoryBus, DeviceSync) {
        clk_cycle_t            clockCounter;
        GB::memory::BusInterface   memBus;

        memBus.MapVAddr(0x0000,0xFFFF, DummyRead, DummyWrite);

        memBus.Read(clockCounter, 0x0000); // -4 ticks
        devsync::step(clockCounter, GB::memory::BusInterface::MEMORY_ACCESS_PRICE_CLK); // +4 ticks
        EXPECT_TRUE(devsync::is_ready(clockCounter));

        memBus.Write(clockCounter, 0x0001, 0x42); // -4 ticks
        memBus.Write(clockCounter, 0x0002, 0x42); // -4 ticks
        devsync::step(clockCounter, GB::memory::BusInterface::MEMORY_ACCESS_PRICE_CLK); // +4 ticks
        devsync::step(clockCounter, GB::memory::BusInterface::MEMORY_ACCESS_PRICE_CLK); // +4 ticks
        EXPECT_TRUE(devsync::is_ready(clockCounter));

        memBus.Write(clockCounter, 0x0003, 0x42); // -4 ticks
        EXPECT_FALSE(devsync::is_ready(clockCounter));
        memBus.Read(clockCounter, 0x0004); // -4 ticks
        EXPECT_FALSE(devsync::is_ready(clockCounter));

        devsync::step(clockCounter, GB::memory::BusInterface::MEMORY_ACCESS_PRICE_CLK);
        devsync::step(clockCounter, GB::memory::BusInterface::MEMORY_ACCESS_PRICE_CLK);
        EXPECT_TRUE(devsync::is_ready(clockCounter));
    }

}