/*
 * Copyright (c) 2018-2023 ProvenRun S.A.S
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
 * @brief ARMv8 setjump implementation
 * @author Hadrien Barral
 * @date June 7th, 2019 (creation)
 * @copyright (c) 2018-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBSTD_ARCH_ARMV8_SETJMP_H_INCLUDED_
#define _LIBSTD_ARCH_ARMV8_SETJMP_H_INCLUDED_

#include <common.h>

/*
 * All callee preserved registers:
 * x19-x30,sp
 * Floating-point registers are *not* included
 */
#define _JBLEN 13

typedef uint64_t _JBTYPE;

#endif /* _LIBSTD_ARCH_ARMV8_SETJMP_H_INCLUDED_ */
