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
bit_n(uint32_t bitset, uint32_t n)
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

#endif
