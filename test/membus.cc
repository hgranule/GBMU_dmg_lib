#include "gtest/gtest.h"
#include "memory/GB_bus.h"
#include "common/GB_macro.h"

namespace {

    struct SimpleMemMappedDevice {
        U8     registerA;
        U8     registerB;

        static Byte ReadA(SimpleMemMappedDevice* dev, Word) {
            return dev->registerA;
        }

        static Byte ReadB(SimpleMemMappedDevice* dev, Word) {
            return dev->registerB;
        }

        static void Write(SimpleMemMappedDevice* dev, Word addr, Byte data) {
            Byte& devRegister = (addr < 0x8000)
                                ? dev->registerA : dev->registerB;
            devRegister = data;
        }

        static Byte UndefinedRead(SimpleMemMappedDevice*, Word) {
            return 0x42;
        }

        static void UndefinedWrite(SimpleMemMappedDevice*, Word, Byte) {
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

    using MemorySpace = std::array<Byte, GB::memory::BusInterface::VIRTUAL_MEMORY_SIZE>;

    struct VAddressSpace {
        MemorySpace& memoryLink;

        VAddressSpace(MemorySpace& memoryOrigin)
        : memoryLink(memoryOrigin)
        {
        }

        static Byte Read(VAddressSpace *dev, Word vAddr) {
            return dev->memoryLink[vAddr];
        }

        static void Write(VAddressSpace *dev, Word vAddr, Byte data) {
            dev->memoryLink[vAddr] = data;
        }

    };

    void RandomVAGen(MemorySpace& space) {
        for (auto& byte : space)
            byte = static_cast<U8>(::bit_slice(7, 0, ::random()));
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
        
        ForAllVirtualAddress([&](Word vAddr){
            EXPECT_EQ(memorySpace[vAddr], memBus.ImmRead(vAddr));
        });
        
        ForAllVirtualAddress([&](Word vAddr){
            memBus.ImmWrite(vAddr, static_cast<Byte>(::bit_slice(7, 0, vAddr)) );
        });

        ForAllVirtualAddress([&](Word vAddr){
            EXPECT_EQ(memorySpace[vAddr], static_cast<Byte>(::bit_slice(7, 0, vAddr)) );
        });
    }
}

namespace {

    void DummyWrite(void*, Word, Byte) {
    }

    Byte DummyRead(void*, Word) {
        return 0x42;
    }

    TEST(MemoryBus, DeviceSync) {
        CLKCycle            clockCounter;
        GB::memory::BusInterface   memBus;

        memBus.MapVAddr(0x0000,0xFFFF, DummyRead, DummyWrite);

        memBus.Read(clockCounter, 0x0000); // -4 ticks
        devsync::StepDone(clockCounter, GB::memory::BusInterface::MEMORY_ACCESS_PRICE_CLK); // +4 ticks
        EXPECT_TRUE(devsync::Synced(clockCounter));

        memBus.Write(clockCounter, 0x0001, 0x42); // -4 ticks
        memBus.Write(clockCounter, 0x0002, 0x42); // -4 ticks
        devsync::StepDone(clockCounter, GB::memory::BusInterface::MEMORY_ACCESS_PRICE_CLK); // +4 ticks
        devsync::StepDone(clockCounter, GB::memory::BusInterface::MEMORY_ACCESS_PRICE_CLK); // +4 ticks
        EXPECT_TRUE(devsync::Synced(clockCounter));

        memBus.Write(clockCounter, 0x0003, 0x42); // -4 ticks
        EXPECT_FALSE(devsync::Synced(clockCounter));
        memBus.Read(clockCounter, 0x0004); // -4 ticks
        EXPECT_FALSE(devsync::Synced(clockCounter));

        devsync::StepDone(clockCounter, GB::memory::BusInterface::MEMORY_ACCESS_PRICE_CLK);
        devsync::StepDone(clockCounter, GB::memory::BusInterface::MEMORY_ACCESS_PRICE_CLK);
        EXPECT_TRUE(devsync::Synced(clockCounter));
    }

}