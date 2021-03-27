
/**
 * @file GB_interrupt.h
 * 
 * @brief Describes Interrupt controller class
 */

#ifndef GB_INTERRUPT_H_
# define GB_INTERRUPT_H_

# include "common/GB_types.h"
# include "common/GB_macro.h"

# include "memory/GB_vaddr.h"
# include "memory/GB_bus.h"

namespace GB {

    /**
     * @brief Implementation of interrupt controller from GameBoy
     */
    class InterruptController {
    public:

        using Reg8 = Byte;

        /** Interrupt bit offsets for IF and IE registers */
        enum InterruptIdx : U8 {
            VBLANK_INT = 0,     ///< Vertical blanking period
            LCDSTAT_INT = 1,    ///< LCD event
            TIMOVER_INT = 2,    ///< Timer overflow
            SERIO_INT = 3,      ///< Serial I/O transferred 1 byte
            JOYPAD_INT = 4,     ///< Joypad button press (1 -> 0)

            NO_INTERRUPT = 5,   ///< Index of first reserved bits, see REG_RESERVED_BIT_MASK reference for details
        };

        /** Interrupt handlers virtual addresses */
        enum InterruptAddr : U16 {
            VBLANK_JMP = memory::VirtualAddress::VBLANK_JMP_VADDR,
            LCDSTAT_JMP = memory::VirtualAddress::LCDSTAT_JMP_VADDR,
            TIMOVER_JMP = memory::VirtualAddress::TIMOVER_JMP_VADDR,
            SERIO_JMP = memory::VirtualAddress::SERIO_JMP_VADDR,
            JOYPAD_JMP = memory::VirtualAddress::JOYPAD_JMP_VADDR
        };

        /** Maps interrupt bit index to interrupt handler virtual address */
        constexpr static InterruptAddr   INTERRUPTS_ADDR[] = {
            VBLANK_JMP, LCDSTAT_JMP,
            TIMOVER_JMP, SERIO_JMP,
            JOYPAD_JMP
        }; 

        /** Create an interrupt gourp: interrupts<JOYPAD_INT, SERIO_INT, LCDSTAT_INT> => value for registers IE/IF */
        template <InterruptIdx int_idx, InterruptIdx... other_int_idxies>
        constexpr static unsigned interrupts = interrupts<int_idx> | interrupts<other_int_idxies...>;

        template <InterruptIdx int_idx>
        constexpr static unsigned interrupts<int_idx> = 1 << int_idx;

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
            constexpr static unsigned REG_RESERVED_BIT_MASK     = bit_mask(7,5);
            constexpr static unsigned REG_MEANINGFUL_BIT_MASK   = ~REG_RESERVED_BIT_MASK; /** @todo if i use uint8, compiler gives a warning */ 
            constexpr static unsigned REG_RESERVED_BITS         = -1u & REG_RESERVED_BIT_MASK;

            Reg8    IE; ///< Interrupt enable register (all reserved bits must be 1)
            Reg8    IF; ///< Interrupt request register (all reserved bits must be 1)
            bool    IME; ///< Interrupt Main Enable flag register


            Registers(Reg8 valIE=0x00, Reg8 valIF=0x00, bool valIME=true)
            : IE(valIE), IF(valIF), IME(valIME)
            {
            }

        };

    private:
        Registers           __registers;

    public:

        InterruptController()
        : __registers()
        {
        }

        InterruptController(const Registers& regs)
        : __registers(regs)
        {
        }

        /**
         * @brief Request some interrupt
         * @param[in] interrupt some interrupt bit index
         * 
         * @details Sets corresponding bit at IF
         */
        void RequestInt(InterruptIdx interrupt);

        /**
         * @brief Cancel request of some interrupt
         * @param[in] interrupt some interrupt bit index
         * 
         * @details Resets corresponding bit at IF
         */
        void ResetInt(InterruptIdx interrupt);

        /**
         * @brief Returns the highest priority interrupt index to be handled by CPU
         * @returns Interrupt index if has any enabled and requested one, or InterruptIdx::NO_INTERRUPTS if
         *          nothing to handle.
         * @details Interrupts handled by index bit priority. The lowest bit index has the highest
         *          priority of handling. 
         */
        InterruptIdx GetHighestPossibleInt() const;

        /**  Set IE register */
        void SetIE(Byte value);

        /** Get IE register's value */
        Byte GetIE() const;

        /** Set IF register */
        void SetIF(Byte value);

        /** Get IF register's value */
        Byte GetIF() const;

        /** Set IME register */
        void SetIME(bool value);

        /** Get IME register */
        bool GetIME() const;

        /** Maps interrupt contoller registers IE and IF inside memory bus table */
        void MapToMemory(memory::UMBus& memoryBus);

    };

    inline void
    InterruptController::RequestInt(InterruptController::InterruptIdx interrupt) {
        __registers.IF = bit_n_set(interrupt, __registers.IF);
    }

    inline void
    InterruptController::ResetInt(InterruptController::InterruptIdx interrupt) {
        __registers.IF = bit_n_reset(interrupt, __registers.IF);
    }

    inline InterruptController::InterruptIdx
    InterruptController::GetHighestPossibleInt() const {
        const U8 interruptsToHandle = (__registers.IF & __registers.IE) | Registers::REG_RESERVED_BITS;
        return InterruptIdx(bit_lsb(U16(interruptsToHandle)));
    }

    inline void
    InterruptController::SetIE(Byte value) {
        __registers.IE = value | Registers::REG_RESERVED_BITS;
    }

    inline Byte
    InterruptController::GetIE() const {
        return __registers.IE | Registers::REG_RESERVED_BITS;
    }

    inline void
    InterruptController::SetIF(Byte value) {
        __registers.IF = value | Registers::REG_RESERVED_BITS;
    }

    inline Byte
    InterruptController::GetIF() const {
        return __registers.IF | Registers::REG_RESERVED_BITS;
    }

    inline void
    InterruptController::SetIME(bool value) {
        __registers.IME = value;
    }

    inline bool
    InterruptController::GetIME() const {
        return __registers.IME;
    }

}

#endif
