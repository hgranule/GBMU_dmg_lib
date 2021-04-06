/**
 * @file GB_wram.h
 * @brief ...
 */

#ifndef GB_WRAM_H_
# define GB_WRAM_H_

# include "common/GB_types.h"
# include "common/GB_macro.h"

# include "memory/GB_vaddr.h"
# include "memory/GB_bus.h"

# include "GB_config.h"

namespace GB::device {

    class WRAM {
    public:
        struct Registers {
            constexpr static unsigned RESERVED_BITS = ::bits_set(6);

            Reg8    SVBK;

            Registers(Reg8 svbkInitValue = SVBK_INIT_VALUE) : SVBK(svbkInitValue) {}
        };

        constexpr static unsigned MAX_SIZE = WRAM_CGB_SIZE;
        constexpr static unsigned BANK_SIZE = WRAM_CGB_BANK_SIZE;

    private:
        DBuff       __memory;
        Registers   __regs;

    public:
        WRAM();
        WRAM(const WRAM& other);
        WRAM(WRAM&& other);
        ~WRAM();

        WRAM& operator=(const WRAM& other);
        WRAM& operator=(WRAM&& other);

        void MapToMemory(memory::BusInterface& memoryBus);

        inline unsigned __BankIdxBits() const {
            return ::bit_slice(2, 0, __regs.SVBK);
        }

        inline unsigned __CurrentBankIdx() const {
            const unsigned bankBits = __BankIdxBits();
            return (bankBits == 0x0) ? 0x1 : bankBits;
        }

        inline unsigned __DirectAddress(Word innerAddr) const {
            const unsigned bankIdx = (innerAddr >= memory::WRAMX_BASE_VADDR)
                                   ? __CurrentBankIdx() : 0x0;
            const unsigned bankOffset = innerAddr % BANK_SIZE;
            return (bankIdx * BANK_SIZE) + bankOffset;
        }

        inline Byte GetSVBK() const;
        inline void SetSVBK(Byte value);

        inline Byte Read(Word innerAddr) const;
        inline void Write(Word innerAddr, Byte data);

        inline Byte DirectRead(Word directAddr) const;
        inline void DirectWrite(Word directAddr, Byte data);

    };

    inline Byte
    WRAM::GetSVBK() const {
        return __regs.SVBK | Registers::RESERVED_BITS;
    }

    inline void
    WRAM::SetSVBK(Byte value) {
        __regs.SVBK = value | Registers::RESERVED_BITS;
    }

    inline Byte
    WRAM::Read(Word inputAddr) const {
        return DirectRead(__DirectAddress(inputAddr));
    }

    inline void
    WRAM::Write(Word inputAddr, Byte data) {
        DirectWrite(__DirectAddress(inputAddr), data);
    }

    inline Byte
    WRAM::DirectRead(Word directAddr) const {
        return __memory[directAddr];
    }

    inline void
    WRAM::DirectWrite(Word directAddr, Byte data) {
        __memory[directAddr] = data;
    }

}

#endif
