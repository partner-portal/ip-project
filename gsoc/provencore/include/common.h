/*
 * Copyright (c) 2013-2023 ProvenRun S.A.S
 * All Rights Reserved.
 *
 * This software is the confidential and proprietary information of
 * ProvenRun S.A.S ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with ProvenRun S.A.S
 *
 * PROVENRUN S.A.S MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. PROVENRUN S.A.S SHALL
 * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING,
 * MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */
/**
 * @file
 * @brief Common macro to be used in kernel and applications
 * @author Jérémie Corbier
 * @date November 23rd, 2013 (creation)
 * @copyright (c) 2013-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _COMMON_H_INCLUDED_
#define _COMMON_H_INCLUDED_

#include <byte_order.h>
#include <compiler.h>
#include <inttypes.h>
#include <stdalign.h>

#ifdef __cplusplus
#include <atomic>
#define _Atomic(X) std::atomic< X >
#include <stdbool.h>
#else
#include <stdatomic.h>
#endif

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
using namespace std;
#endif

struct ld_symbol;
typedef struct ld_symbol ld_symbol;

#ifdef CONFIG_ARCH_RISCV64

/*
 * On RISC-V 64, GCC appears to generate AMO instructions for
 * atomic_load_explicit and atomic_store_explicit even with
 * memory_order_relaxed. This may be an issue on some platforms which may not
 * support AMO instructions when they target peripherals.
 *
 * On RISC-V 64, using simple volatile access rather than atomic primitives
 * should not be an issue (although it relies on implementation defined
 * behavior) as there are instructions to read and write 64 bits values as a
 * whole (sd/ld).
 */

#define __ATOMIC_LOAD(n)                                        \
    inline __attribute__ ((always_inline))                      \
    static uint##n##_t reg_read##n(uintptr_t addr)              \
    {                                                           \
        uint##n##_t const volatile *p = (uint##n##_t *)addr;    \
        return *p;                                              \
    }

#define __ATOMIC_STORE(n)                                       \
    inline __attribute__ ((always_inline))                      \
    static void reg_write##n(uintptr_t addr, uint##n##_t val)   \
    {                                                           \
        uint##n##_t volatile *p = (uint##n##_t *)addr;          \
        *p = val;                                               \
    }

#else /* CONFIG_ARCH_RISCV64 */

#ifdef __clang__
/*
 * Bug in C11 standard, LLVM should be lenient here.
 * http://www.open-std.org/jtc1/sc22/wg14/www/docs/n1807.htm
 */
#define __ATOMIC_LOAD(n)                                        \
    inline __attribute__ ((always_inline))                      \
    static uint##n##_t reg_read##n(uintptr_t addr)              \
    {                                                           \
        _Atomic(uint##n##_t) volatile *p = (_Atomic(uint##n##_t) *)addr;   \
        return (uint##n##_t)atomic_load_explicit(p, memory_order_relaxed); \
    }
#else
#define __ATOMIC_LOAD(n)                                        \
    inline __attribute__ ((always_inline))                      \
    static uint##n##_t reg_read##n(uintptr_t addr)              \
    {                                                           \
        _Atomic(uint##n##_t) const volatile *p = (_Atomic(uint##n##_t) *)addr; \
        return (uint##n##_t)atomic_load_explicit(p, memory_order_relaxed);     \
    }
#endif /* __clang__ */

#define __ATOMIC_STORE(n)                                       \
    inline __attribute__ ((always_inline))                      \
    static void reg_write##n(uintptr_t addr, uint##n##_t val)   \
    {                                                           \
        _Atomic(uint##n##_t) volatile *p = (_Atomic(uint##n##_t) *)addr;   \
        atomic_store_explicit(p, val, memory_order_relaxed);    \
    }

#endif /* !CONFIG_ARCH_RISCV64 */

#define __CLEAR(n)                                              \
    inline __attribute__ ((always_inline))                      \
    static void reg_clr##n(uintptr_t addr, uint##n##_t clr)     \
    {                                                           \
        uint##n##_t val = reg_read##n(addr);                    \
        uint##n##_t mask = (uint##n##_t) ~clr;                  \
        reg_write##n(addr, val & mask);                         \
    }

#define __SET(n)                                                \
    inline __attribute__ ((always_inline))                      \
    static void reg_set##n(uintptr_t addr, uint##n##_t set)     \
    {                                                           \
        uint##n##_t val = reg_read##n(addr);                    \
        reg_write##n(addr, val | set);                          \
    }

#define __CLEARSET(n)                                           \
    inline __attribute__ ((always_inline))                      \
    static void reg_clrset##n(uintptr_t addr, uint##n##_t clr,  \
                              uint##n##_t set)                  \
    {                                                           \
        uint##n##_t val = reg_read##n(addr);                    \
        uint##n##_t mask = (uint##n##_t) ~clr;                  \
        reg_write##n(addr, (val & mask) | set);                 \
    }

#define __REG_OPS(n)        \
    __ATOMIC_LOAD(n);       \
    __ATOMIC_STORE(n);      \
    __CLEAR(n);             \
    __SET(n);               \
    __CLEARSET(n)

/* Macro defines `reg_read8`, `reg_write8`, `reg_clr8`, `reg_set8`
 *  and `reg_clrset8`.
 */
__REG_OPS(8);
/* Macro defines `reg_read16`, `reg_write16`, `reg_clr16`, `reg_set16`
 *  and `reg_clrset16`.
 */
__REG_OPS(16);
/* Macro defines `reg_read32`, `reg_write32`, `reg_clr32`, `reg_set32`
 *  and `reg_clrset32`.
 */
__REG_OPS(32);

#if (defined(__PROVENCORE__) && defined(CONFIG_KARCH_BITS_64)) \
    || defined(CONFIG_ARCH_BITS_64)
/*
 * See https://phabricator.lan.provenrun.com/D359 for an extended discussion on
 * why we don't support this on 32 bit architectures.
 */
/* Macro defines `reg_read64`, `reg_write64`, `reg_clr64`, `reg_set64`
 *  and `reg_clrset64`.
 */
__REG_OPS(64);
#endif /* (__PROVENCORE__ && CONFIG_KARCH_BITS_64) || CONFIG_ARCH_BITS_64 */

inline __attribute__ ((always_inline))
static void reg_non_atomic_write64(uintptr_t addr, uint64_t value)
{
    *(volatile uint64_t *)addr = value;
}

#undef __REG_OPS
#undef __CLEARSET
#undef __SET
#undef __CLEAR
#undef __ATOMIC_STORE
#undef __ATOMIC_LOAD

#define reg_read32_be(addr)             uswap_32(reg_read32((addr)))
#define reg_write32_be(addr, value)     reg_write32((addr), uswap_32((value)))
#define reg_read64_be(addr)             uswap_64(reg_read64((addr)))
#define reg_write64_be(addr, value)     reg_write64((addr), uswap_64((value)))

#define reg_set32_be(addr, set)         reg_set32((addr), uswap_32((set)))
#define reg_clr32_be(addr, clr)         reg_clr32((addr), uswap_32((clr)))
#define reg_clrset32_be(addr, clr, set) \
    reg_clrset32((addr), uswap_32((clr)), uswap_32((set)))

/* max_align_t is defined in stddef.h */
#define MAX_ALIGN alignof(max_align_t)

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))
#define countof COUNT_OF

/**
 * @brief Evaluates to 1 if \p n is a power of 2.
 *  /!\ This macro is unsafe because the parameter \p n is evaluated multiple
 *  times.
 * @param n         Value to check.
 */
#define UNSAFE_IS_POWEROF2(n)      ((n) > 0 && ((n) & ((n) - 1)) == 0)

/**
 * @brief Macro generating a helper function
 *  `_Bool name_is_powerof2(typ val);` which returns whether the value
 *  `val` is a power of 2.
 *
 * @param name      Name prefix of the generated function
 * @param typ       Type of the parameter, must be an unsigned scalar type
 */
#define __IS_POWEROF2(name, typ)                                               \
    __attribute__((always_inline))                                             \
    static inline _Bool name##_is_powerof2(typ n)                              \
    {                                                                          \
        return n > 0 && (n & (n - 1)) == 0;                                    \
    }

__IS_POWEROF2(uchar, unsigned char)
__IS_POWEROF2(ushort, unsigned short)
__IS_POWEROF2(uint, unsigned int)
__IS_POWEROF2(ulong, unsigned long)
__IS_POWEROF2(ulonglong, unsigned long long)

/* *INDENT-OFF* */
/**
 * @brief Evaluates whether the value \p val is a power of 2.
 *
 *  The function is generic and can be called with parameters of any unsigned
 *  scalar type.
 *
 * @param val           Value to check
 * @return              1 if \p val is a power of 2, 0 otherwise.
 */
#define is_powerof2(val)                                                       \
    _Generic ((val),                                                           \
              unsigned char: uchar_is_powerof2,                                \
              unsigned short: ushort_is_powerof2,                              \
              unsigned int: uint_is_powerof2,                                  \
              unsigned long: ulong_is_powerof2,                                \
              unsigned long long: ulonglong_is_powerof2)                       \
             ((val))
/* *INDENT-ON* */

/**
 * @brief Align the value \p n to the next multiple of \p bound.
 *
 * /!\ This macro is unsafe for multiple reasons:
 *  1. the parameter \p bound is evaluated twice
 *  2. the result is not checked for overflow
 * If possible, you should prefer the safer alternative \ref align_overflow().
 *
 * @param n         Value to align
 * @param bound     Requested alignment, must be a power of 2
 */
#define UNSAFE_ALIGN(n, bound)  (((n) + (bound) - 1) & ~((bound) - 1))

/**
 * @brief Macro generating a helper function
 *  `_Bool name_align_overflow(typ val, typ alignment, typ *res);` which
 *  aligns the value `val` to the specified alignment value `alignment`, and
 *  stores the result in `res`, before returning whether the computation
 *  overflowed (`1`) or not (`0`).
 *
 * @param name      Name prefix of the generated function
 * @param typ       Type of the parameters, must be an unsigned scalar type
 */
#define __ALIGN_OVERFLOW(name, typ)                                            \
    __attribute__((always_inline,warn_unused_result))                          \
    static inline _Bool name##_align_overflow(typ a, typ b, typ *res)          \
    {                                                                          \
        typ tmp;                                                               \
        if (__builtin_add_overflow(a, b - 1, &tmp)) {                          \
            return 1;                                                          \
        }                                                                      \
        *res = (typ)(tmp & ~(b - 1));                                          \
        return 0;                                                              \
    }

__ALIGN_OVERFLOW(uchar, unsigned char)
__ALIGN_OVERFLOW(ushort, unsigned short)
__ALIGN_OVERFLOW(uint, unsigned int)
__ALIGN_OVERFLOW(ulong, unsigned long)
__ALIGN_OVERFLOW(ulonglong, unsigned long long)

/* *INDENT-OFF* */
/**
 * @brief Aligns the value \p val to the next \p alignment boundary and
 *  evaluates whether the computation overflows.
 *
 *  The result is stored in \p res only if the result did not overflow.
 *
 *  The function is generic and can be called with parameters of any unsigned
 *  scalar type. The type of the operands for the computation is determined
 *  by the type of \p res; the other parameters are converted to the same type.
 *
 * @param val           Value to align
 * @param alignment     Requested alignment, must be a power of 2
 * @param res           Pointer to a buffer where to store the aligned value
 *                      Must not be NULL.
 * @return              1 if the computation overflowed, 0 otherwise.
 */
#define align_overflow(val, alignment, res)                                    \
    _Generic ((res),                                                           \
              unsigned char *: uchar_align_overflow,                           \
              unsigned short *: ushort_align_overflow,                         \
              unsigned int *: uint_align_overflow,                             \
              unsigned long *: ulong_align_overflow,                           \
              unsigned long long *: ulonglong_align_overflow)                  \
             ((val), (alignment), (res))
/* *INDENT-ON* */

#endif /* _COMMON_H_INCLUDED_ */
