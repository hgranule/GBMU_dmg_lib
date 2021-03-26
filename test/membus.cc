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

    TEST(MemoryBus, simple) {
        GB::memory::UMBus       memBus;
        SimpleMemMappedDevice   dev;

        // fill memory tables
        memBus.MapVAddr(0x0000, 0x7FFF,
                        GB::memory::UMBus::ReadCmd(SimpleMemMappedDevice::ReadA),
                        GB::memory::UMBus::WriteCmd(SimpleMemMappedDevice::Write),
                        &dev);

        memBus.MapVAddr(0x8000, 0xEFFF,
                        GB::memory::UMBus::ReadCmd(SimpleMemMappedDevice::ReadB),
                        GB::memory::UMBus::WriteCmd(SimpleMemMappedDevice::Write),
                        &dev);
        
        memBus.MapVAddr(0xF000, 0xFFFF,
                        GB::memory::UMBus::ReadCmd(SimpleMemMappedDevice::UndefinedRead),
                        GB::memory::UMBus::WriteCmd(SimpleMemMappedDevice::UndefinedWrite));

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