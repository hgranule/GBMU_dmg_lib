#ifndef DEVICE_GB_VRAM_H_
#define DEVICE_GB_VRAM_H_

#include "common/GB_macro.h"
#include "GB_config.h"

namespace GB::device {

class VRAM {
 public:
    struct Registers {
        constexpr static u32 RESERVED_BITS = ::bit_mask(7, 1);

        Reg8 VBK;

        explicit
        Registers(Reg8 vbk_init_value = VBK_INIT_VALUE) : VBK(vbk_init_value) {}
    };

    VRAM() : __memory(VRAM_MAX_SIZE), __regs() {}
    VRAM(const VRAM& other) = default;
    VRAM(VRAM&& other) = default;
    ~VRAM() = default;

    VRAM& operator=(const VRAM& other) = default;
    VRAM& operator=(VRAM&& other) = default;

    inline byte_t get_VBK_reg() const;
    inline void set_VBK_reg(byte_t value);

    inline byte_t read_inner_vaddr(word_t inner_vaddr) const;
    inline void write_inner_vaddr(word_t inner_vaddr, byte_t value);

    inline byte_t read_phys_addr(word_t phys_addr) const;
    inline void write_phys_addr(word_t phys_addr, byte_t value);

 protected:
    dbuffer_t   __memory;
    Registers __regs;

    inline unsigned __calc_phys_addr(word_t inner_vaddr) const;
};

/**
 * @brief   Return VBK register value (only zero bit readable).
 *
 * @details Only zero bit matter and can return different values, other always set in '1' 0b1111111{ZERO_BIT_VALUE}
 */
// TODO(hgranule, dolovnyak):   DMG/CGB_DMG_MODE conflicts.
//                              Behavior which described above correct for CGB/CGB_DMG_MODE behavior.
//                              In DMG_MODE this function must return 0xFF (0xFA now).
//                              I think we will define the correct behavior after or during
//                              the implementation of the membus mapping.
inline byte_t
VRAM::get_VBK_reg() const {
    return __regs.VBK | Registers::RESERVED_BITS;
}

/**
 * @brief   Write value to VBK register (only zero bit writable).
 *
 * @details Only zero bit will rewrite.
 */
inline void
VRAM::set_VBK_reg(byte_t value) {
    __regs.VBK = ::bit_n(0, value);
}

inline byte_t
VRAM::read_inner_vaddr(word_t inner_vaddr) const {
    return read_phys_addr(__calc_phys_addr(inner_vaddr));
}

inline void
VRAM::write_inner_vaddr(word_t inner_vaddr, byte_t value) {
    write_phys_addr(__calc_phys_addr(inner_vaddr), value);
}

inline byte_t
VRAM::read_phys_addr(word_t phys_addr) const {
    return __memory[phys_addr];
}

inline void
VRAM::write_phys_addr(word_t phys_addr, byte_t value) {
    __memory[phys_addr] = value;
}

inline unsigned
VRAM::__calc_phys_addr(word_t inner_vaddr) const {
    const u32 bank_offset = inner_vaddr % VRAM_BANK_SIZE;
    const u32 bank_base = (::bit_n(0, __regs.VBK)) ? VRAM_BANK_SIZE : 0;

    return bank_base + bank_offset;
}

}  // namespace GB::device

#endif  // DEVICE_GB_VRAM_H_

