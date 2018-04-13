#ifndef Util_TypeDef_h
#define Util_TypeDef_h

#include <stdint.h>
namespace value
{
typedef uint8_t 				u8;
typedef uint16_t				u16;
typedef uint32_t				u32;
typedef uint64_t				u64;

typedef int8_t 					i8;
typedef int16_t					i16;
typedef int32_t					i32;
typedef int64_t					i64;

typedef float					d32;
typedef double					d64;

#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif

#ifndef LLONG_MIN
#define LLONG_MIN    (-LLONG_MAX - 1LL)
#endif

#ifndef ULLONG_MAX
#define ULLONG_MAX   18446744073709551615ULL
#endif
}

#endif

