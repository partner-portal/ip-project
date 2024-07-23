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
 * @brief Description of ARM HVC Calling Convention (from the SMC Calling
 * Convention Documentation).
 * @author Alexandre Thevenet
 * @date June 8th, 2018 (creation)
 * @copyright (c) 2016-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _ARCH_ARM_COMMON_HVC_H_INCLUDED_
#define _ARCH_ARM_COMMON_HVC_H_INCLUDED_

/**
 * Definitions from ARM DEN 0028B - SMC Calling Convention
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

#define HVC_32BIT               UINT32_C(0)
#define HVC_64BIT               UINT32_C(0x40000000)
#define HVC_STDCALL             UINT32_C(0)
#define HVC_FASTCALL            UINT32_C(0x80000000)

#define HVC32_PARAMS_SIZE       0x20
#define HVC64_PARAMS_SIZE       0x90

#ifndef __ASSEMBLER__

/**
 * @brief Structure describing the values of the generic purpose register
 * before and after the hypercall. This interface assumes that the hypercall
 * respects the SMCCC, and thus :
 * - r0 contains the function identifier and its 30th bit must be cleared.
 * - the immediate value "0" will be used by ProvenCore when doing the
 *   hypercall.
 * - r7 may end up being rewritten to store a client ID instead of the provided
 *   value.
 * - Only r0-r7 registers are available to pass argument or recover HVC result.
 *   Provencore will put arbitrary values in the other registers and won't pass
 *   their value back to the application.
 */
struct hvc32_params {
    uint32_t r0;    /**< Function identifier */
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
};

/**
 * @brief Structure describing the values of the generic purpose register
 * before and after the hypercall. This interface assumes that the hypercall
 * respects the SMCCC, and thus :
 * - x0 contains the function identifier, its 30th bit must be set and the
 *   bits [32, 63] must be cleared. 
 * - the immediate value "0" will be used by ProvenCore when doing the
 *   hypercall.
 * - x7 may end up being rewritten to store a client ID instead of the provided
 *   value.
 * - Only x0-x17 registers are available to pass argument or recover HVC
 *   result. Provencore will put arbitrary values in the other registers and
 *   won't pass their value back to the application.
 */
struct hvc64_params {
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

_Static_assert(sizeof(struct hvc32_params) == HVC32_PARAMS_SIZE,
               "The hvc32_params structure has an unexpected layout");
_Static_assert(sizeof(struct hvc64_params) == HVC64_PARAMS_SIZE,
               "The hvc64_params structure has an unexpected layout");

typedef struct hvc32_params hvc32_params_t;
typedef struct hvc64_params hvc64_params_t;

static inline _Bool hvc_is_64bit(uint32_t r0)
{
    return (r0 & HVC_64BIT) != 0;
}

#endif /* __ASSEMBLER__ */
#endif /* _ARCH_ARM_COMMON_HVC_H_INCLUDED_ */
