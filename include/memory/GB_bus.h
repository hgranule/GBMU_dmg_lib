/**
 * @file GB_bus.h
 * @brief File describes united memory bus.
 */

#ifndef GB_MEMORY_BUS_H_
# define GB_MEMORY_BUS_H_

# include "common/GB_clock.h"
# include "common/GB_types.h"
# include <array>

namespace GB::memory {

    /**
     *  @brief United Memory Bus
     */
    class BusInterface {
    public:
        static constexpr size_t     VIRTUAL_MEMORY_SIZE = 0x10000ul;
        static constexpr clk_cycle_t   MEMORY_ACCESS_PRICE_CLK = 4_CLKCycles;

        using ConnectorPtr  = void*;
        using WriteCmd      = void (*)(ConnectorPtr memCtx, word_t addr, byte_t data);
        using ReadCmd       = byte_t (*)(ConnectorPtr memCtx, word_t addr);

        struct MemMapEntry {
            ReadCmd         readCmd; ///< read command function
            WriteCmd        writeCmd; ///< write command function
            ConnectorPtr    connector; ///< pointer to an instance which is "connected" to a memory bus

            MemMapEntry(ReadCmd rFunc = nullptr, WriteCmd wFunc = nullptr, ConnectorPtr con = nullptr)
            : readCmd(rFunc), writeCmd(wFunc), connector(con) {
            }
        };

        using FastDirectMMap  = std::array<MemMapEntry, VIRTUAL_MEMORY_SIZE>;

        using MemoryMap = FastDirectMMap;

    protected:
        devsync::Action<MEMORY_ACCESS_PRICE_CLK>    __memAccessSyncDecorator;
        MemoryMap                                   __map;

    public:

        inline void MapVAddr(word_t vAddr1, word_t vAddr2, ReadCmd rFunc, WriteCmd wFunc, ConnectorPtr connection = nullptr) {
            for (
                unsigned currentAddr = vAddr1, lastAddr = vAddr2;
                currentAddr <= lastAddr;
                ++currentAddr
            ) {
                MapVAddr(currentAddr, rFunc, wFunc, connection);
            }
        }

        inline void MapVAddr(word_t vAddr, ReadCmd rFunc, WriteCmd wFunc, ConnectorPtr connection = nullptr) {
            __map[vAddr] = MemMapEntry(rFunc, wFunc, connection);
        }

        inline void
        Write(clk_cycle_t& clock, word_t vAddr, byte_t data) {
            __memAccessSyncDecorator(clock, &BusInterface::ImmWrite, this, vAddr, data);
        }

        inline byte_t
        Read(clk_cycle_t& clock, word_t vAddr) {
            return __memAccessSyncDecorator(clock, &BusInterface::ImmRead, this, vAddr);
        }

        inline void
        ImmWrite(word_t vAddr, byte_t data) {
            auto [_, wCmd, connected] = __map[vAddr];
            wCmd(connected, vAddr, data);
        }

        inline byte_t
        ImmRead(word_t vAddr) {
            auto [rCmd, _, connected] = __map[vAddr];
            return rCmd(connected, vAddr);
        }
    };

}

#endif
