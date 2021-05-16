/**
 * @file GB_types.h
 * @brief File consist of common type definitions
 */

#ifndef COMMON_GB_TYPES_H_
# define COMMON_GB_TYPES_H_

# include <cstddef>
# include <cstdint>
# include <memory>
# include <string>

# include "common/GB_dbuffer.h"

using u64   = uint64_t;
using i64   = int64_t;
using u32   = uint32_t;
using i32   = int32_t;
using u16   = uint16_t;
using i16   = int16_t;
using u8    = uint8_t;
using i8    = int8_t;

using word_t  = u16;  ///< 16-bit word type
using byte_t  = u8;   ///< 8-bit byte type

using Reg8 = byte_t;

constexpr static auto K_BYTE_MULTI = 0x400ull;
constexpr static auto M_BYTE_MULTI = K_BYTE_MULTI * K_BYTE_MULTI;
constexpr static auto G_BYTE_MULTI = M_BYTE_MULTI * K_BYTE_MULTI;

constexpr inline u64
operator "" _Bytes(u64 bytes) { return bytes; }

constexpr inline u64
operator "" _KBytes(u64 k_bytes) { return k_bytes * K_BYTE_MULTI; }

constexpr inline u64
operator "" _MBytes(u64 m_bytes) { return m_bytes * M_BYTE_MULTI; }

constexpr inline u64
operator "" _GBytes(u64 g_bytes) { return g_bytes * G_BYTE_MULTI; }

#endif  // COMMON_GB_TYPES_H_
