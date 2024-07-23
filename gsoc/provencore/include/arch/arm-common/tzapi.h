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
 * @brief Definition of ProvenCore TrustZone API
 * @author Henri Chataing
 * @date December 14th, 2017 (creation)
 * @copyright (c) 2017-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _ARCH_ARM_COMMON_TZAPI_H_INCLUDED_
#define _ARCH_ARM_COMMON_TZAPI_H_INCLUDED_

#include "smc.h"

/* At the moment, PL310 related, so only for Cortex-a9 based config */
#define SMC_L2CACHE_ENABLE \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_CPU, 0)
#define SMC_L2CACHE_DISABLE \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_CPU, 1)

/* ProvenCore uses '63' */
#define SMC_OWNER_PNC (SMC_OWNER_TRUSTED_OS + 13)

/* PnC specific: TRUSTED_OS, range from 0 to 0x1000 */
#define SMC_YIELD \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_PNC, 0)
#define SMC_ACTION_FROM_S \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_PNC, 1)
#define SMC_GET_SHAREDMEM \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_PNC, 2)
#define SMC_CONFIG_SHAREDMEM \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_PNC, 3)
#define SMC_ACTION_FROM_NS \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_PNC, 4)
#define SMC_SIMD \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_PNC, 5)

/*
 * TRUSTED OS functionality, but platform dependent, so we use distinct
 * subspace of SMC_OWNER_TRUSTED_OS
 *
 * SMC_GET_RPMB_KEY: Calls rpmb_key_get to store 16 bytes of
 *                   the RPMB key in the SMC registers (r0..r3).
 *                   This part of the key starts at offset 4*r1 in bytes.
 *                   0 is returned in registers r0..r3 of the SMC in case of error:
 *                   - less than 16 bytes are available from the provided offset,
 *                   - the key has already been read during the same power cycle.
 * SMC_GET_RNG_VECTOR: Calls rng_vector_get to store 16 random bytes
 *                     managed by the monitor in the SMC registers (r0..r3).
 *                     - less than 16 bytes of entropy are available.
 * SMC_SECURE_BOOT_NS: NS world requires secure boot operations
 * SMC_SECURE_BOOT_S: S world indicate end of secure boot operation
 */
#define SMC_GET_RPMB_KEY \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_PNC, 0x1000 + 0)
#define SMC_GET_RNG_VECTOR \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_PNC, 0x1000 + 1)
#define SMC_SEMIHOSTING \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_PNC, 0x1000 + 2)
#define SMC_SECURE_BOOT_NS \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_PNC, 0x1000 + 3)
#define SMC_SECURE_BOOT_S \
    SMC_FUNC_ID(SMC_32BIT, SMC_FASTCALL, SMC_OWNER_PNC, 0x1000 + 4)

/* fast call return value */
#define SMC_RETURN_YIELD        0
#define SMC_RETURN_OK           1
#define SMC_RETURN_ERROR        2
#define SMC32_RETURN_UNKNOWN_ID UINT32_C(0xffffffff)
#define SMC64_RETURN_UNKNOWN_ID UINT64_C(0xffffffffffffffff)

/* Entity tag for configuration shared memory */
#define LINUX_SHARED_MEM_TAG    0xcafe

#endif /* _ARCH_ARM_COMMON_TZAPI_H_INCLUDED_ */
