/**
 * @file GB_bus.h
 * @brief File describes united memory bus.
 */

#ifndef GB_MEMORY_BUS_H_
# define GB_MEMORY_BUS_H_

# include "common/GB_types.h"
# include <array>

namespace GB::memory {

    /**
     *  @brief United Memory Bus
     */
    class UMBus {
    public:
        static constexpr size_t     VIRTUAL_MEMORY_SIZE = 0x10000ul;
        static constexpr CLKCycle   MEMORY_ACCESS_PRICE_CLK = 4_CLKCycles;

        using ConnectorPtr  = void*;
        using WriteCmd      = void (*)(ConnectorPtr memCtx, Word addr, Byte data);
        using ReadCmd       = Byte (*)(ConnectorPtr memCtx, Word addr);

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

    private:
        sync::Action<MEMORY_ACCESS_PRICE_CLK>   __memAccessSyncDecorator;
        MemoryMap                               __map;

    public:

        inline void MapVAddr(Word vAddr, ReadCmd rFunc, WriteCmd wFunc, ConnectorPtr connection) {
            __map[vAddr] = MemMapEntry(rFunc, wFunc, connection);
        }

        inline void
        Write(CLKCycle& clock, Word vAddr, Byte data) {
            __memAccessSyncDecorator(clock, &UMBus::ImmWrite, this, vAddr, data);
        }

        inline Byte
        Read(CLKCycle& clock, Word vAddr) {
            return __memAccessSyncDecorator(clock, &UMBus::ImmRead, this, vAddr);
        }

        inline void
        ImmWrite(Word vAddr, Byte data) {
            auto [_, wCmd, connected] = __map[vAddr];
            wCmd(connected, vAddr, data);
        }

        inline Byte
        ImmRead(Word vAddr) {
            auto [rCmd, _, connected] = __map[vAddr];
            return rCmd(connected, vAddr);
        }
    };

}

#endif
