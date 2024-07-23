/*
 * Copyright (c) 2016-2023 ProvenRun S.A.S
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
 * @brief Description of ARM SMC32 and SMC64 Calling Conventions.
 * @author Vincent Siles
 * @author Henri Chataing
 * @author Hadrien Barral
 * @date April 12th, 2016 (creation)
 * @copyright (c) 2016-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _ARCH_ARM_COMMON_SMC_H_INCLUDED_
#define _ARCH_ARM_COMMON_SMC_H_INCLUDED_

/**
 * Definitions from ARM DEN 0028B - SMC Calling Convention
 *              and ARM DEN 0070A - Firmware interfaces for mitigating
 *                                  CVE-2017-5715
 */

#ifdef __ASSEMBLER__
#ifndef UINT32_C
#define UINT32_C(c)     (c)
#endif
#ifndef UINT64_C
#define UINT64_C(c)     (c)
#endif
#else /*__ASSEMBLER__ */
#ifdef __KERNEL__
#include <linux/types.h>
#ifndef UINT32_C
#define UINT32_C(c)     c ## U
#endif
#ifndef UINT64_C
#define UINT64_C(c)     c ## UL
#endif
#else /* __KERNEL__ */
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#endif /* __KERNEL__ */
#endif /* __ASSEMBLER__ */

#define SMCCC_MAJOR_VERSION     UINT16_C(1)
#define SMCCC_MINOR_VERSION     UINT16_C(2)

/** Generic SMCC answer */
#define SMCCC_SUCCESS           INT32_C(0)
#define SMCCC_NOT_SUPPORTED     INT32_C(-1)
#define SMCCC_NOT_REQUIRED      INT32_C(-2)
#define SMCCC_INVALID_PARAMETER INT32_C(-3)

/** SMCCC_ARCH_FEATURES specific success answer for SMCCC_ARCH_WORKAROUND_2 feature*/
#define SMCCC_SUCCESS_ARCH_WORKAROUND_2_DYNAMIC_REQUIRED     INT32_C(0)
#define SMCCC_SUCCESS_ARCH_WORKAROUND_2_DYNAMIC_NOT_REQUIRED INT32_C(1)

#define SMC_32BIT               UINT32_C(0)
#define SMC_64BIT               UINT32_C(0x40000000)
#define SMC_STDCALL             UINT32_C(0)
#define SMC_FASTCALL            UINT32_C(0x80000000)
#define SMC_OWNER_MASK          UINT32_C(0x3f)
#define SMC_OWNER_SHIFT         24
#define SMC_OWNER(owner)        (((owner) & SMC_OWNER_MASK) << SMC_OWNER_SHIFT)
#define SMC_FUNC_MASK           UINT32_C(0xffff)
#define SMC_FUNC(func)          ((func) & SMC_FUNC_MASK)

#define SMC_FUNC_ID(arch, type, owner, func) \
    ((arch) | (type) | SMC_OWNER(owner) | SMC_FUNC(func))

#define SMC_OWNER_ARCH          UINT32_C(0)
#define SMC_OWNER_CPU           UINT32_C(1)
#define SMC_OWNER_SIP           UINT32_C(2)
#define SMC_OWNER_OEM           UINT32_C(3)
#define SMC_OWNER_STD_SEC       UINT32_C(4)
#define SMC_OWNER_STD_HYP       UINT32_C(5)
#define SMC_OWNER_VENDOR_HYP    UINT32_C(6)

/** Range 48-49 */
#define SMC_OWNER_TRUSTED_APP   UINT32_C(48)

/** Range 50-63 */
#define SMC_OWNER_TRUSTED_OS        UINT32_C(50)
#define SMC_OWNER_TRUSTED_OS_END    UINT32_C(63)

/** Arm Architecture Service range function IDs */
#define SMCCC_VERSION           UINT32_C(0x80000000)
#define SMCCC_ARCH_FEATURES     UINT32_C(0x80000001)
#define SMCCC_ARCH_SOC_ID       UINT32_C(0x80000002)
#define SMCCC_ARCH_WORKAROUND_1 UINT32_C(0x80008000)
#define SMCCC_ARCH_WORKAROUND_2 UINT32_C(0x80007FFF)

/** Arm Architecture Service range helpers */
#define SMCCC_VERSION_CONSTANT(major, minor) \
    ((UINT32_C(major)<<16) + UINT32_C(minor))

#define SMC32_PARAMS_SIZE       0x20
#define SMC64_PARAMS_SIZE       0x90

#ifndef __ASSEMBLER__

struct smc32_params {
    uint32_t r0;    /**< Function identifier */
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
};

struct smc64_params {
    uint64_t x0;    /**< Function identifier (w0 only) */
    uint64_t x1;
    uint64_t x2;
    uint64_t x3;
    uint64_t x4;
    uint64_t x5;
    uint64_t x6;
    uint64_t x7;
    uint64_t x8;
    uint64_t x9;
    uint64_t x10;
    uint64_t x11;
    uint64_t x12;
    uint64_t x13;
    uint64_t x14;
    uint64_t x15;
    uint64_t x16;
    uint64_t x17;
};

_Static_assert(sizeof(struct smc32_params) == SMC32_PARAMS_SIZE,
               "The smc32_params structure has an unexpected layout");
_Static_assert(sizeof(struct smc64_params) == SMC64_PARAMS_SIZE,
               "The smc64_params structure has an unexpected layout");

typedef struct smc32_params smc32_params_t;
typedef struct smc64_params smc64_params_t;

static inline _Bool smc_is_64bit(uint32_t r0)
{
    return (r0 & SMC_64BIT) != 0;
}

static inline _Bool smc_is_fastcall(uint32_t r0)
{
    return (r0 & SMC_FASTCALL) != 0;
}

static inline uint16_t smc_function_number(uint32_t smc_function_ID)
{
    return smc_function_ID & SMC_FUNC_MASK;
}

_Static_assert(sizeof(smc32_params_t) == SMC32_PARAMS_SIZE,
               "smc32_params_t size mismatch");
_Static_assert(sizeof(smc64_params_t) == SMC64_PARAMS_SIZE,
               "smc64_params_t size mismatch");

#endif /* __ASSEMBLER__ */
#endif /* _ARCH_ARM_COMMON_SMC_H_INCLUDED_ */
