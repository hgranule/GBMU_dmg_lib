/**
 * @file GB_wram.h
 * @brief ...
 */

#ifndef DEVICE_GB_WRAM_H_
# define DEVICE_GB_WRAM_H_

# include "GB_config.h"

# include "common/GB_types.h"
# include "common/GB_macro.h"

# include "memory/GB_vaddr.h"


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

 protected:
    inline u32 __get_bank_idx_bits() const;
    inline u32 __get_current_bank_idx() const;
    inline u32 __calc_phys_addr(word_t inner_vaddr) const;

 public:
    WRAM(): __memory(MAX_SIZE), __regs() {}

    inline byte_t get_SVBK_reg() const;
    inline void set_SVBK_reg(byte_t value);

    inline byte_t read_inner_vaddr(word_t inner_vaddr) const;
    inline void write_inner_vaddr(word_t inner_vaddr, byte_t data);

    inline byte_t read_phys_addr(word_t paddr) const;
    inline void write_phys_addr(word_t paddr, byte_t data);

};

inline u32
WRAM::__get_bank_idx_bits() const {
    return ::bit_slice(2, 0, __regs.SVBK);
}

inline u32
WRAM::__get_current_bank_idx() const {
    const u32 bank_bits = __get_bank_idx_bits();
    return (bank_bits == 0x0) ? 0x1 : bank_bits;
}

inline u32
WRAM::__calc_phys_addr(word_t inner_vaddr) const {
    const u32 bank_idx = (inner_vaddr >= memory::WRAMX_BASE_VADDR)
                            ? __get_current_bank_idx() : 0x0;
    const u32 bank_offset = inner_vaddr % BANK_SIZE;
    return (bank_idx * BANK_SIZE) + bank_offset;
}

inline byte_t
WRAM::get_SVBK_reg() const {
    return __regs.SVBK | Registers::RESERVED_BITS;
}

inline void
WRAM::set_SVBK_reg(byte_t value) {
    __regs.SVBK = value;
}

inline byte_t
WRAM::read_inner_vaddr(word_t inner_vaddr) const {
    return read_phys_addr(__calc_phys_addr(inner_vaddr));
}

inline void
WRAM::write_inner_vaddr(word_t inner_vaddr, byte_t data) {
    write_phys_addr(__calc_phys_addr(inner_vaddr), data);
}

inline byte_t
WRAM::read_phys_addr(word_t phys_addr) const {
    return __memory[phys_addr];
}

inline void
WRAM::write_phys_addr(word_t phys_addr, byte_t data) {
    __memory[phys_addr] = data;
}

}  // namespace GB::device

#endif  // DEVICE_GB_WRAM_H_
