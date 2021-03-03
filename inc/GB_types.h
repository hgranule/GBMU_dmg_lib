/**
 * @file GB_types.h
 * @brief File consist of common type definitions
 */

#ifndef GB_TYPES_H_
# define GB_TYPES_H_

# include <cstddef>
# include <cstdint>

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

using CLKCycle = uint64_t; ///< type for representing clock cycles 

#endif
