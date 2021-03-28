/**
 * @file GB_types.h
 * @brief File consist of common type definitions
 */

#ifndef GB_TYPES_H_
# define GB_TYPES_H_

# include <cstddef>
# include <cstdint>
# include <memory>
# include <string>

# include "common/GB_dbuffer.h"
# include "common/GB_clock.h"

using U16   = uint16_t;
using I16   = int16_t;
using U8    = uint8_t;
using I8    = int8_t;

struct U8_8
{
    U8 h;
    U8 l;
};

using Word  = U16;  ///< 16-bit word type
using Byte  = U8;   ///< 8-bit byte type
using WordS = U8_8; ///< 16-bit word represented as 2 different bytes

using Reg8 = Byte;

using DBuff = DBuffer< std::allocator<U8> >; ///< Short name of dynamic buffer of bytes

constexpr static auto K_BYTE_MULTI = 0x400ull;
constexpr static auto M_BYTE_MULTI = K_BYTE_MULTI * K_BYTE_MULTI;
constexpr static auto G_BYTE_MULTI = M_BYTE_MULTI * K_BYTE_MULTI;

constexpr inline unsigned long long
operator "" _Bytes(unsigned long long bytes) { return bytes; }

constexpr inline unsigned long long
operator "" _KBytes(unsigned long long kBytes) { return kBytes * K_BYTE_MULTI; }

constexpr inline unsigned long long
operator "" _MBytes(unsigned long long mBytes) { return mBytes * M_BYTE_MULTI; }

constexpr inline unsigned long long
operator "" _GBytes(unsigned long long gBytes) { return gBytes * G_BYTE_MULTI; }

#endif
