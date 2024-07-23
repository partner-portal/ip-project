/*
 * Copyright (c) 2017-2023 ProvenRun S.A.S
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
 * @brief Operation on bit (wrapper around compiler builtin)
 * @author Olivier Delande
 * @date April 3rd, 2017 (creation)
 * @copyright (c) 2017-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _BITOPS_H_INCLUDED_
#define _BITOPS_H_INCLUDED_

#include <stdint.h>

/**
 * @brief Finds the number of trailing 0s in a 32-bit unsigned integer.
 * @param x a 32-bit unsigned integer
 * @return the largest \c n <= 32 such that the \c n least significant bits of
 * \p x are 0s
 */
static inline uint_fast8_t count_trailing_zeros_u32(uint32_t x)
{
    return (x == 0) ? 32U : ((uint_fast8_t)__builtin_ctzll(x));
}

/**
 * @brief Finds the number of trailing 0s in a 64-bit unsigned integer.
 * @param x a 64-bit unsigned integer
 * @return the largest \c n <= 64 such that the \c n least significant bits of
 * \p x are 0s
 */
static inline uint_fast8_t count_trailing_zeros_u64(uint64_t x)
{
    return (x == 0) ? 64U : ((uint_fast8_t)__builtin_ctzll(x));
}

/**
 * @brief Finds the number of trailing 1s in a 64-bit unsigned integer.
 * @param x a 64-bit unsigned integer
 * @return the largest \c n <= 64 such that the \c n least significant bits of
 * \p x are 1s
 */
static inline uint_fast8_t count_trailing_ones_u64(uint64_t x)
{
    return count_trailing_zeros_u64(~x);
}

/**
 * @brief Finds the number of leading 0s in a 32-bit unsigned integer.
 * @param x a 32-bit unsigned integer
 * @return the largest \c n <= 32 such that the \c n most significant bits of
 * \p x are 0s
 */
static inline uint_fast8_t count_leading_zeros_u32(uint32_t x)
{
    return (x == 0) ? 32U : ((uint_fast8_t)__builtin_clzll(x));
}

/**
 * @brief Finds the number of leading 0s in a 64-bit unsigned integer.
 * @param x a 64-bit unsigned integer
 * @return the largest \c n <= 64 such that the \c n most significant bits of
 * \p x are 0s
 */
static inline uint_fast8_t count_leading_zeros_u64(uint64_t x)
{
    return (x == 0) ? 64U : ((uint_fast8_t)__builtin_clzll(x));
}

/**
 * @brief Finds the number of leading 1s in a 64-bit unsigned integer.
 * @param x a 64-bit unsigned integer
 * @return the largest \c n <= 64 such that the \c n most significant bits of
 * \p x are 1s
 */
static inline uint_fast8_t count_leading_ones_u64(uint64_t x)
{
    return count_leading_zeros_u64(~x);
}

/**
 * @brief Sign-extends an 8-bit integer to 64 bits
 * @param x the 8-bit integer
 * @return the 64-bit integer
 */
static inline uint64_t sign_extend_8_64(uint8_t x)
{
    // GCC's implementation-defined behavior (see https://gcc.gnu.org/onlinedocs/gcc/Integers-implementation.html)
    return (uint64_t)(int64_t)(int8_t)x;
}

/**
 * @brief Sign-extends a 32-bit integer to 64 bits
 * @param x the 32-bit integer
 * @return the 64-bit integer
 */
static inline uint64_t sign_extend_32_64(uint32_t x)
{
    // GCC's implementation-defined behavior (see https://gcc.gnu.org/onlinedocs/gcc/Integers-implementation.html)
    return (uint64_t)(int64_t)(int32_t)x;
}

/**
 * Transform an int31_t value (encoded in the lower bits of an uint32_t) into
 * an int32_t offset.
 */
static inline int32_t prel31_to_i32(uint32_t pre)
{
    /*
     * See GCC for undefined behaviours
     *  https://gcc.gnu.org/onlinedocs/gcc/Integers-implementation.html
     *
     * Namely:
     *   + GCC supports only two’s complement integer types
     *   + For conversion to a type of width N, the value is reduced
     *     modulo 2^N to be within range of the type; no signal is raised.
     *   + Signed ‘>>’ acts on negative numbers by sign extension.
     */

    /*
     * NB: the cast from uint32_t to int32_t is performed after the bit shift
     * to remove an undefined behaviour: if E1 has a signed type and
     * nonnegative value, and E1 × 2^E2 is representable in the result type,
     * then that is the resulting value; otherwise, the behavior is undefined.
     */
    int32_t res = (int32_t)(uint32_t)(pre << 1);
    return res >> 1;
}

/**
 * Transform an int32_t value to the correspondig two’s complement uint32_t
 *
 * GCC supports only two’s complement integer types so the function body is
 *  minimal.
 *  ( see https://gcc.gnu.org/onlinedocs/gcc/Integers-implementation.html )
 */
static inline uint32_t i32_to_u32_2comp(int32_t i)
{
    return (uint32_t) i;
}

/**
 * Transform an uint64_t value to the correspondig two’s complement int64_t
 *
 * GCC supports only two’s complement integer types so the function body is
 *  minimal.
 *  ( see https://gcc.gnu.org/onlinedocs/gcc/Integers-implementation.html )
 */
static inline int64_t u64_to_i64_2comp(uint64_t i)
{
    return (int64_t) i;
}

/**
 * Transform an int value to the correspondig two’s complement unsigned int
 *
 * GCC supports only two’s complement integer types so the function body is
 *  minimal.
 *  ( see https://gcc.gnu.org/onlinedocs/gcc/Integers-implementation.html )
 */
static inline unsigned int int_to_uint_2comp(int i)
{
    return (unsigned int) i;
}

/**
 * Transform an unsigned int value to the correspondig two’s complement int
 *
 * GCC supports only two’s complement integer types so the function body is
 *  minimal.
 *  ( see https://gcc.gnu.org/onlinedocs/gcc/Integers-implementation.html )
 */
static inline int uint_to_int_2comp(unsigned int i)
{
    return (int) i;
}
#endif /* _BITOPS_H_INCLUDED_ */
