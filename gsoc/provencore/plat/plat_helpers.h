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
 * @brief Header specifically for inclusion in platform.h
 * @author Henri Chataing
 * @date February 15th, 2019 (creation)
 * @copyright (c) 2019-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _PLAT_HELPERS_H_INCLUDED_
#define _PLAT_HELPERS_H_INCLUDED_

#include <arch_const.h>

#ifndef __ASSEMBLER__
#include <stdint.h>

#ifdef __PROVENCORE__
#include <vm_types.h>
#else
/*
 * Kernel definitions should not be used by user applications; generate
 * a syntax error if used (no other solution ?).
 */
#define KADDR_C(x)              0_
#endif /* __PROVENCORE__ */

#else /* __ASSEMBLER__ */
#define KADDR_C(x)              x
#endif /* __ASSEMBLER__ */

#define PLAT_UNSAFE_ALIGN(n, bound)  (((n) + (bound) - 1) & ~((bound) - 1))

#endif /* _PLAT_HELPERS_H_INCLUDED_ */
