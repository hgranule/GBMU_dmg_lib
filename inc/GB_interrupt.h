
/**
 * @file GB_interrupt.h
 * 
 * @brief Describes Interrupt controller class
 */

#ifndef GB_INTERRUPT_H_
# define GB_INTERRUPT_H_

# include "GB_types.h"
# include "GB_macro.h"

namespace gameboy
{

    /**
     * @brief Implementation of interrupt controller from GameBoy
     */
    class InterruptController
    {
    public:

        using Reg8 = U8;

        /// Interrupt bit offsets for IF and IE registers
        enum InterruptIdx : U8
        {
            VBLANK_INT = 0,     ///< Vertical blanking period
            LCDSTAT_INT = 1,    ///< LCD event
            TIMOVER_INT = 2,    ///< Timer overflow
            SERIO_INT = 3,      ///< Serial I/O transferred 1 byte
            JOYPAD_INT = 4,     ///< Joypad button press (1 -> 0)

            NO_INTERRUPT = 5,   ///< Index of first reserved bits, see REG_RESERVED_BIT_MASK reference for details
        };

        /// Interrupt handlers virtual addresses
        enum InterruptAddr : U16
        {
            VBLANK_JMP  = 0x0040,
            LCDSTAT_JMP = 0x0048,
            TIMOVER_JMP = 0x0050,
            SERIO_JMP   = 0x0058,
            JOYPAD_JMP  = 0x0060
        };

        /// Maps interrupt bit index to interrupt handler virtual address
        constexpr static InterruptAddr   INTERRUPTS_ADDR[] = {
            VBLANK_JMP, LCDSTAT_JMP,
            TIMOVER_JMP, SERIO_JMP,
            JOYPAD_JMP
        }; 

    private:

        /**
         * @brief Interrupt controller registers
         */
        struct Registers
        {
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
            constexpr static Reg8 REG_RESERVED_BIT_MASK     = bit_mask(7,5);
            constexpr static Reg8 REG_MEANINGFUL_BIT_MASK   = ~REG_RESERVED_BIT_MASK;
            constexpr static Reg8 REG_RESERVED_BITS         = -1u & REG_RESERVED_BIT_MASK;

            Reg8    IE; ///< Interrupt enable register (all reserved bits must be 1)
            Reg8    IF; ///< Interrupt request register (all reserved bits must be 1)
            bool    IME; ///< Interrupt Main Enable flag register
        };


    public:

        inline bool IsEnabled(InterruptIdx interrupt)
        {
            return __registers.IME && bit_n(interrupt, __registers.IE);
        }

        inline bool IsRequested(InterruptIdx interrupt)
        {
            return bit_n(interrupt, __registers.IF);
        }

        inline void EnableInt(InterruptIdx interrupt)
        {
            __registers.IE = bit_n_set(interrupt, __registers.IE);
        }

        inline void DisableInt(InterruptIdx interrupt)
        {
            __registers.IE = bit_n_reset(interrupt, __registers.IE);
        }

        inline void RequestInt(InterruptIdx interrupt)
        {
            __registers.IF = bit_n_set(interrupt, __registers.IF);
        }

        inline void ResetInt(InterruptIdx interrupt)
        {
            __registers.IF = bit_n_reset(interrupt, __registers.IF);
        }

        inline InterruptIdx GetHighestPossibleInt()
        {
            const U8 interruptsToHandle = (__registers.IF & __registers.IE)
                                        | Registers::REG_RESERVED_BITS;

            return InterruptIdx(bit_lsb(U16(interruptsToHandle)));
        }

    private:
        Registers           __registers;

    };
}

#endif
