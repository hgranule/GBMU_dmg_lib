#ifndef GB_SOUND_MASKS_H_
# define GB_SOUND_MASKS_H_

# include "common/GB_types.h"
# include "common/GB_macro.h"

constexpr static unsigned REG_NRX0_SWEEP_TIME_BIT_MASK                   = ::bit_mask(6, 4);
constexpr static unsigned REG_NRX0_SWEEP_MODE_BIT_MASK                   = ::bit_mask(3, 3);
constexpr static unsigned REG_NRX0_SWEEP_SHIFT_BIT_MASK                  = ::bit_mask(2, 0);
constexpr static unsigned REG_NRX1_WAVE_PAATERN_DUTY_BIT_MASK            = ::bit_mask(7, 6);
constexpr static unsigned REG_NRX1_SOUND_LENGTH_BIT_MASK                 = ::bit_mask(5, 0);
constexpr static unsigned REG_NRX2_ENVELOPE_INITIAL_VALUE_BITS_BIT_MASK  = ::bit_mask(7, 4);
constexpr static unsigned REG_NRX2_ENVELOPE_MODE_BITS_BIT_MASK           = ::bit_mask(3, 3);
constexpr static unsigned REG_NRX2_ENVELOPE_SWEEP_NUMBER_BITS_BIT_MASK   = ::bit_mask(2, 0);
constexpr static unsigned REG_NRX4_RESTART_SOUND_BIT_MASK                = ::bit_mask(7, 7);
constexpr static unsigned REG_NRX4_LENGTH_COUNTER_BIT_MASK               = ::bit_mask(6, 6);
constexpr static unsigned REG_NRX4_SWEEP_HIGHER_BITS_BIT_MASK            = ::bit_mask(2, 0);

# define GET_NRX1_LENGTH_COUNTER_SOUND_LENGTH_BITS(value)   (value & REG_NRX1_SOUND_LENGTH_BIT_MASK)
# define GET_NRX2_ENVELOPE_MODE_BIT(value)                  (value & REG_NRX2_ENVELOPE_MODE_BITS_BIT_MASK)
# define GET_NRX2_ENVELOPE_SWEEP_NUMBER_BITS(value)         (value & REG_NRX2_ENVELOPE_SWEEP_NUMBER_BITS_BIT_MASK)
# define GET_NRX2_INITIAL_VALUE_BITS(value)                 ((value >> 4) & 0xF)
# define GET_NRX4_RESTART_SOUND_BIT(value)                  ((value & REG_NRX4_RESTART_SOUND_BIT_MASK) >> 7)

#endif