
/**
 * @file GB_interrupt.h
 * 
 * @brief Describes Interrupt controller class
 */

#ifndef DEVICE_GB_INTERRUPT_H_
# define DEVICE_GB_INTERRUPT_H_

# include "common/GB_types.h"
# include "common/GB_macro.h"

# include "memory/GB_vaddr.h"
# include "memory/GB_bus.h"

namespace GB::device {

/**
 * @brief Implementation of interrupt controller from GameBoy
 */
class InterruptController {
 public:

    /** Interrupt bit offsets for IF and IE registers */
    enum InterruptIdx : u8 {
        VBLANK_INT = 0,     ///< Vertical blanking period
        LCDSTAT_INT = 1,    ///< LCD event
        TIMOVER_INT = 2,    ///< Timer overflow
        SERIO_INT = 3,      ///< Serial I/O transferred 1 byte
        JOYPAD_INT = 4,     ///< Joypad button press (1 -> 0)

        NO_INTERRUPT = 5,   ///< Index of first reserved bits, see REG_RESERVED_BIT_MASK reference for details
    };

    /** Interrupt handlers virtual addresses */
    enum InterruptAddr : u16 {
        VBLANK_JMP  = ::GB::memory::VirtualAddress::VBLANK_JMP_VADDR,
        LCDSTAT_JMP = ::GB::memory::VirtualAddress::LCDSTAT_JMP_VADDR,
        TIMOVER_JMP = ::GB::memory::VirtualAddress::TIMOVER_JMP_VADDR,
        SERIO_JMP   = ::GB::memory::VirtualAddress::SERIO_JMP_VADDR,
        JOYPAD_JMP  = ::GB::memory::VirtualAddress::JOYPAD_JMP_VADDR
    };

    /** Maps interrupt bit index to interrupt handler virtual address */
    constexpr static InterruptAddr   INTERRUPTS_ADDR[] = {
        VBLANK_JMP, LCDSTAT_JMP,
        TIMOVER_JMP, SERIO_JMP,
        JOYPAD_JMP
    };

    /** Create an interrupt gourp: interrupts<JOYPAD_INT, SERIO_INT, LCDSTAT_INT> => value for registers IE/IF */
    template <unsigned... interrupt_indexies>
    constexpr static unsigned interrupts = ::bits_set(interrupt_indexies...);

 public:

    /**
     * @brief Interrupt controller registers
     */
    struct Registers {
        /**
         * @details Reserved bits at interrupt enable and interrupt request registers
         *          must be set to 1, for a LSB index search, which is used for prioritize
         *          Interrupts.
         * 
         * @note The LSB index search at the expression (IE & IF) returns bit index of highest priority interrupt
         *       that must be handled. If we set reserved bits to value 0 and there are no interrupts enabled or requested, 
         *       the LSB search returns undefined value, because there is no LSB, because there are not set bits in (IF & IE).
         *       But if we set reserved bits, the LSB search returns index of first bit at reserved range [7:5] -> 5, that can
         *       be interpreted as NO_INTERRUPTS_TO_HADNLE.
         */
        constexpr static unsigned REG_RESERVED_BIT_MASK     = ::bit_mask(7, 5);
        /** @todo if i use uint8, compiler gives a warning */
        constexpr static unsigned REG_MEANINGFUL_BIT_MASK   = ~REG_RESERVED_BIT_MASK;
        constexpr static unsigned REG_RESERVED_BITS         = -1u & REG_RESERVED_BIT_MASK;

        Reg8    IE;     ///< Interrupt enable register (all reserved bits must be 1)
        Reg8    IF;     ///< Interrupt request register (all reserved bits must be 1)
        bool    IME;    ///< Interrupt Main Enable flag register


        explicit
        Registers(Reg8 val_IE = 0x00, Reg8 val_IF = 0x00, bool val_IME = true)
        : IE(val_IE), IF(val_IF), IME(val_IME) {
        }

    };

 protected:
    Registers           __registers;

 public:

    InterruptController()
    : __registers() {
    }

    explicit
    InterruptController(const Registers& regs)
    : __registers(regs) {
    }

    /**
     * @brief Request some interrupt
     * @param[in] interrupt some interrupt bit index
     * 
     * @details Sets corresponding bit at IF
     */
    void request_interrupt(InterruptIdx interrupt);

    /**
     * @brief Cancel request of some interrupt
     * @param[in] interrupt some interrupt bit index
     * 
     * @details Resets corresponding bit at IF
     */
    void reset_interrupt(InterruptIdx interrupt);

    /**
     * @brief Returns the highest priority interrupt index to be handled by CPU
     * @returns Interrupt index if has any enabled and requested one, or InterruptIdx::NO_INTERRUPTS if
     *          nothing to handle.
     * @details Interrupts handled by index bit priority. The lowest bit index has the highest
     *          priority of handling. 
     */
    InterruptIdx get_highest_priority_interrupt() const;

    /**  Set IE register */
    void set_IE_reg(byte_t value);

    /** Get IE register's value */
    byte_t get_IE_reg() const;

    /** Set IF register */
    void set_IF_reg(byte_t value);

    /** Get IF register's value */
    byte_t get_IF_reg() const;

    /** Set IME register */
    void set_IME_reg(bool value);

    /** Get IME register */
    bool get_IME_reg() const;

    /** Maps interrupt contoller registers IE and IF inside memory bus table */
    void map_to_memory(::GB::memory::BusInterface& mem_bus);

};

inline void
InterruptController::request_interrupt(InterruptController::InterruptIdx interrupt) {
    __registers.IF = bit_n_set(interrupt, __registers.IF);
}

inline void
InterruptController::reset_interrupt(InterruptController::InterruptIdx interrupt) {
    __registers.IF = bit_n_reset(interrupt, __registers.IF);
}

inline InterruptController::InterruptIdx
InterruptController::get_highest_priority_interrupt() const {
    const u8 interrupts_to_handle = (__registers.IF & __registers.IE) | Registers::REG_RESERVED_BITS;
    return InterruptIdx(bit_lsb(u16(interrupts_to_handle)));
}

inline void
InterruptController::set_IE_reg(byte_t value) {
    __registers.IE = value | Registers::REG_RESERVED_BITS;
}

inline byte_t
InterruptController::get_IE_reg() const {
    return __registers.IE | Registers::REG_RESERVED_BITS;
}

inline void
InterruptController::set_IF_reg(byte_t value) {
    __registers.IF = value | Registers::REG_RESERVED_BITS;
}

inline byte_t
InterruptController::get_IF_reg() const {
    return __registers.IF | Registers::REG_RESERVED_BITS;
}

inline void
InterruptController::set_IME_reg(bool value) {
    __registers.IME = value;
}

inline bool
InterruptController::get_IME_reg() const {
    return __registers.IME;
}

}  // namespace GB::device

#endif  // DEVICE_GB_INTERRUPT_H_
