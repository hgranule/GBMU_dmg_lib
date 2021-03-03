/**
 * @file GB_macro.h
 * @brief File consist of macroses, constexpr functions and inline functions.
 */

#ifndef GB_MACRO_H_
# define GB_MACRO_H_

# include <cstdint>

/**
 * @brief Forces some expression to be compile time calculated
 * @param[in] _cexpr constant expression
 * @return result of constant expression
 */
# define force_constexpr(_cexpr) [&]() \
            { constexpr auto X = (_cexpr); return (X); }()

/**
 * @brief Computes bitmask for bitrange [_msbi:_lsbi]
 * @param[in] msbi index of most significant bit in a bitmask
 * @param[in] lsbi index of least significant bit in a bitmask
 * @return Bitmask [_msbi:_lsbi]
 */
constexpr inline uint32_t
bit_mask(uint32_t msbi, uint32_t lsbi)
{
    constexpr uint32_t bitsize = 32u;
    constexpr uint32_t filled32 = -1u;
    const uint32_t masksize = (msbi - lsbi) + 1;

    return (filled32 >> (bitsize - masksize)) << (lsbi);
}

/**
 * @brief Computes slice at number
 * @param[in] msbi index of most significant bit of slice
 * @param[in] lsbi index of least significant bit of slice
 * @param[in] bitset 32-bit number that represents a bitset
 * @return Bits at range [msbi:lsbi] from bitset
 */
constexpr inline uint32_t
bit_slice(uint32_t msbi, uint32_t lsbi, uint32_t bitset) \
{
    return (bit_mask(msbi, lsbi) & bitset) >> lsbi;
}

/**
 * @brief Inserts bits from some value into dest's ranged bits
 * @param[in] msbi index of most significant bit of injection range
 * @param[in] lsbi index of least significant bit of injection range
 * @param[in] dst destionation value for injection
 * @param[in] val value which will be injected into range [msbi:lsbi]
 *
 * @return injected dst value
 */
constexpr inline uint32_t
bit_slice_inject(uint32_t msbi, uint32_t lsbi, uint32_t dst, uint32_t val)
{
    const uint32_t bitmask = bit_mask(msbi , lsbi);
    const uint32_t bitslice = (val << lsbi) & bitmask;

    return bitslice | (dst & ~bitmask);
}

/**
 * @brief Get value of N-th bit
 * @param[in] n index of bit
 * @param[in] bitset 32-bit number that represents a bitset
 * @return true if bit is set
 */
constexpr inline bool
bit_n(uint32_t n, uint32_t bitset)
{
    return bool(bitset & (1u << n));
}

/**
 * @brief Set value of N-th bit
 * @param[in] n index of bit
 * @param[in] bitset 32-bit number that represents a bitset
 * @return bitset with activated bit
 */
constexpr inline uint32_t
bit_n_set(uint32_t n, uint32_t bitset)
{
    return bitset | (1u << n);
}

/**
 * @brief Reset value of N-th bit
 * @param[in] n index of bit which
 * @param[in] bitset 32-bit number that represents a bitset
 * @return bitset with disactivated bit
 */
constexpr inline uint32_t
bit_n_reset(uint32_t n, uint32_t bitset)
{
    return bitset & ~(1u << n);
}


namespace __internals_fast_lsb {
    /*  0100 1101 0111 1000 = 0x4D78 -> magick cycle sequence (MCS)
     *
     *          N     2 1 0
     *    X X X 1 ... 0 0 0 -> N least significant bit mask (LSBM)
     *
     *    N - least significant bit index (LSBI)
     *
     *    MCS * LSBM <==> MCS << (LSBI)
     *
     *    LSBI: (MCS * LSBM)
     *    ==========================
     *    0  : 0000 100110101111000
     *    1  : 0001 001101011110000
     *    2  : 0010 011010111100000
     *    3  : 0100 110101111000000
     *    4  : 1001 101011110000000
     *    5  : 0011 010111100000000
     *    6  : 0110 101111000000000
     *    7  : 1101 011110000000000
     *    8  : 1010 111100000000000
     *    9  : 0101 111000000000000
     *    10 : 1011 110000000000000
     *    11 : 0111 100000000000000
     *    12 : 1111 000000000000000
     *    13 : 1110 000000000000000
     *    14 : 1100 000000000000000
     *    15 : 1000 000000000000000
     *         \__/ \_____________/
     *         KEY      15 bits
     *
     *    X & -X                    => LSBM
     *    (LSBM*MCS >> 15) & 0xF    => KEY
     *    TABLE[KEY]                => LSBI
     */

    constexpr uint32_t   LSB_KEY_MASK = 0xF;
    constexpr uint32_t   LSB_KEY_SHIFT = 15u;
    constexpr uint16_t   MAGICK_CYCLE_NUM = 0x4D78;

    constexpr int LSB_CYCLE_TABLE[16] = {
         0,  1,  2,  5,
         3,  9,  6, 11,
        15,  4,  8, 10,
        14,  7, 13, 12
    };
}


/**
 * @brief Get index of most significant bit
 * @param[in] bitset 16-bit number that represents a bitset
 * @return return index of most significant bit or 32 if there is no set bits 
 */
constexpr inline int
bit_lsb(uint16_t bitset)
{
    using namespace __internals_fast_lsb;

    const uint16_t LSB_MASK = (bitset & -bitset);
    const uint32_t LSB_CYCLE_KEY = ((LSB_MASK * MAGICK_CYCLE_NUM) >> LSB_KEY_SHIFT) & LSB_KEY_MASK;

    return LSB_CYCLE_TABLE[LSB_CYCLE_KEY];
}

#endif
