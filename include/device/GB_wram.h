/**
 * @file GB_wram.h
 * @brief ...
 */

#ifndef DEVICE_GB_WRAM_H_
# define DEVICE_GB_WRAM_H_

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

        explicit
        Registers(Reg8 svbk_init_val = SVBK_INIT_VALUE) : SVBK(svbk_init_val) {}
    };

    constexpr static unsigned MAX_SIZE = WRAM_CGB_SIZE;
    constexpr static unsigned BANK_SIZE = WRAM_CGB_BANK_SIZE;

 protected:
    dbuffer_t   __memory;
    Registers   __regs;

 public:
    ~WRAM();
    WRAM();
    WRAM(const WRAM& other);
    WRAM(WRAM&& other);

    WRAM& operator=(const WRAM& other);
    WRAM& operator=(WRAM&& other);

    void map_to_memory(memory::BusInterface& mem_bus);

    inline u32 __get_bank_idx_bits() const {
        return ::bit_slice(2, 0, __regs.SVBK);
    }

    inline u32 __get_current_bank_idx() const {
        const u32 bank_bits = __get_bank_idx_bits();
        return (bank_bits == 0x0) ? 0x1 : bank_bits;
    }

    // TODO(hgranule) fix to correct codestyle
    // also suggest replace out of class all inline methods implementations or, conversely, include all
    // also suggest rename to __calc_phys_addr(word_t inner_vaddr)
    inline u32 __calc_phys_address(word_t laddr) const {
        const u32 bankIdx = (laddr >= memory::WRAMX_BASE_VADDR)
                                ? __get_current_bank_idx() : 0x0;
        const u32 bankOffset = laddr % BANK_SIZE;
        return (bankIdx * BANK_SIZE) + bankOffset;
    }

    inline byte_t get_SVBK_reg() const;
    inline void set_SVBK_reg(byte_t value);

    // TODO(hgranule) rename to read/write_inner_vaddr(word_t inner_vaddr)
    inline byte_t read_logic_addr(word_t laddr_stripped) const;
    inline void write_logic_addr(word_t laddr_stripped, byte_t data);

    inline byte_t read_phys_addr(word_t phys_addr) const;
    inline void write_phys_addr(word_t phys_addr, byte_t data);

};

inline byte_t
WRAM::get_SVBK_reg() const {
    return __regs.SVBK | Registers::RESERVED_BITS;
}

inline void
WRAM::set_SVBK_reg(byte_t value) {
    __regs.SVBK = value | Registers::RESERVED_BITS;
}

inline byte_t
WRAM::read_logic_addr(word_t laddr) const {
    return read_phys_addr(__calc_phys_address(laddr));
}

inline void
WRAM::write_logic_addr(word_t laddr, byte_t data) {
    write_phys_addr(__calc_phys_address(laddr), data);
}

inline byte_t
WRAM::read_phys_addr(word_t direct_addr) const {
    return __memory[direct_addr];
}

inline void
WRAM::write_phys_addr(word_t direct_addr, byte_t data) {
    __memory[direct_addr] = data;
}

}  // namespace GB::device

#endif  // DEVICE_GB_WRAM_H_
