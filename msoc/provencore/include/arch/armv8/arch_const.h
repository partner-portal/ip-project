/*
 * Copyright (c) 2019-2023 ProvenRun S.A.S
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
 * @brief Defines constants and types related to the current user architecture.
 * @author Henri Chataing
 * @date February 14th, 2019 (creation)
 * @copyright (c) 2019-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 *
 * /!\ This file must be synchronized with the kernel's definitions.
 */

#ifndef _ARCH_ARMV8_ARCH_CONST_H_INCLUDED_
#define _ARCH_ARMV8_ARCH_CONST_H_INCLUDED_

#ifndef __ASSEMBLER__

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#define SIZE_C(x)           ((size_t)(UINTMAX_C(x)))
#define UINTPTR_C(x)        ((uintptr_t)(UINTMAX_C(x)))

#else

#define SIZE_C(x)           x
#define UINT32_C(x)         x
#define UINT64_C(x)         x
#define UINTMAX_C(x)        x

#endif /* __ASSEMBLER__ */

#ifndef __ASSEMBLER__
typedef uint64_t paddr_t;
#endif

#define PADDR_C             UINT64_C
#define PADDR_MAX           UINT64_MAX
#define PRIxPA              PRIx64
#define PRIuPA              PRIu64

#define PAGE_SHIFT          12
#define PAGE_MASK           UINT64_C(0xfffffffff000)
#define PAGE_SIZE           SIZE_C(0x1000)

#define SECTION_SHIFT       21
#define SECTION_MASK        UINT64_C(0xffffffe00000)
#define SECTION_SIZE        SIZE_C(0x200000)

#endif /* _ARCH_ARMV8_ARCH_CONST_H_INCLUDED_ */
