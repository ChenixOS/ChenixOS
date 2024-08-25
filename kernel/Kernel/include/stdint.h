#ifndef STDINT_H_
#define STDINT_H_


#include "limits.h"

/**
 * @name Exact-width integer types.
 * The typedef name int N _t designates a signed integer type with width N, no
 * padding bits, and a two's-complement representation.
 * The typedef name uint N _t designates an unsigned integer type with width N.
 * @{
 */

typedef signed char             int8_t;
typedef signed short            int16_t;
typedef signed long             int32_t;
typedef signed long long        int64_t;

typedef unsigned char           uint8_t;
typedef unsigned short          uint16_t;
typedef unsigned long           uint32_t;
typedef unsigned long long      uint64_t;

#define INT8_MIN                (-0x80)
#define INT16_MIN               (-0x8000)
#define INT32_MIN               (-0x80000000)
#define INT64_MIN               (-0x8000000000000000LL)

#define INT8_MAX                0x7F
#define INT16_MAX               0x7FFF
#define INT32_MAX               0x7FFFFFFF
#define INT64_MAX               0x7FFFFFFFFFFFFFFFLL

#define UINT8_MAX               0xFF
#define UINT16_MAX              0xFFFF
#define UINT32_MAX              0xFFFFFFFFU
#define UINT64_MAX              0xFFFFFFFFFFFFFFFFULL

/** @} */

/**
 * @name Minimum-width integer types.
 * The typedef name int_least N _t designates a signed integer type with a
 * width of at least N, such that no signed integer type with lesser size has
 * at least the specified width. Thus, int_least32_t denotes a signed integer
 * type with a width of at least 32 bits.
 * @{
 */

typedef signed char             int_least8_t;
typedef signed short            int_least16_t;
typedef signed long             int_least32_t;
typedef signed long long        int_least64_t;

typedef unsigned char           uint_least8_t;
typedef unsigned short          uint_least16_t;
typedef unsigned long           uint_least32_t;
typedef unsigned long long      uint_least64_t;

#define INT_LEAST8_MIN          (-0x80)
#define INT_LEAST16_MIN         (-0x8000)
#define INT_LEAST32_MIN         (-0x80000000)
#define INT_LEAST64_MIN         (-0x8000000000000000LL)

#define INT_LEAST8_MAX          0x7F
#define INT_LEAST16_MAX         0x7FFF
#define INT_LEAST32_MAX         0x7FFFFFFF
#define INT_LEAST64_MAX         0x7FFFFFFFFFFFFFFFLL

#define UINT_LEAST8_MAX         0xFF
#define UINT_LEAST16_MAX        0xFFFF
#define UINT_LEAST32_MAX        0xFFFFFFFFU
#define UINT_LEAST64_MAX        0xFFFFFFFFFFFFFFFFULL

/** @} */

/**
 * @name Fastest minimum-width integer types.
 * Each of the following types designates an integer type that is usually
 * fastest to operate with among all integer types that have at least the
 * specified width.
 * The designated type is not guaranteed to be fastest for all purposes; if the
 * implementation has no clear grounds for choosing one type over another, it
 * will simply pick some integer type satisfying the signedness and width
 * requirements.
 * The typedef name int_fast N _t designates the fastest signed integer type
 * with a width of at least N. The typedef name uint_fast N _t designates the
 * fastest unsigned integer type with a width of at least N.
 * @{
 */

typedef signed char             int_fast8_t;
typedef signed int              int_fast16_t;
typedef signed long             int_fast32_t;
typedef signed long long        int_fast64_t;

typedef unsigned char           uint_fast8_t;
typedef unsigned int            uint_fast16_t;
typedef unsigned long           uint_fast32_t;
typedef unsigned long long      uint_fast64_t;

#define INT_FAST8_MIN           (-0x80)
#define INT_FAST16_MIN          (-0x80000000)
#define INT_FAST32_MIN          (-0x80000000)
#define INT_FAST64_MIN          (-0x8000000000000000LL)

#define INT_FAST8_MAX           0x7F
#define INT_FAST16_MAX          0x7FFFFFFF
#define INT_FAST32_MAX          0x7FFFFFFF
#define INT_FAST64_MAX          0x7FFFFFFFFFFFFFFFLL

#define UINT_FAST8_MAX          0xFF
#define UINT_FAST16_MAX         0xFFFFFFFFU
#define UINT_FAST32_MAX         0xFFFFFFFFU
#define UINT_FAST64_MAX         0xFFFFFFFFFFFFFFFFULL

/** @} */


/**
 * @name Integer types capable of holding object pointers.
 * The following type designates a signed integer type with the property that
 * any valid pointer to void can be converted to this type, then converted back
 * to a pointer to void, and the result will compare equal to the original
 * pointer: intptr_t.
 * The following type designates an unsigned integer type with the property that
 * any valid pointer to void can be converted to this type, then converted back
 * to a pointer to void, and the result will compare equal to the original
 * pointer: uintptr_t.
 * @{
 */

typedef signed long int         intptr_t;
typedef unsigned long int       uintptr_t;

#define INTPTR_MIN              (-0x80000000)
#define INTPTR_MAX              0x7FFFFFFF
#define UINTPTR_MAX             0xFFFFFFFFU

/** @} */

/**
 * @name Greatest-width integer types
 * The following type designates a signed integer type capable of representing
 * any value of any signed integer type: intmax_t.
 * The following type designates an unsigned integer type capable of
 * representing any value of any unsigned integer type: uintmax_t.
 */

typedef long long               intmax_t;
typedef unsigned long long      uintmax_t;

#define INTMAX_MIN              (-INTMAX_C(0x8000000000000000))
#define INTMAX_MAX              (INTMAX_C(0x7FFFFFFFFFFFFFFF))
#define UINTMAX_MAX             (UINTMAX_C(0xFFFFFFFFFFFFFFFF))

/** @} */

#include "global/types.h"

#endif /* STDINT_H_ */
