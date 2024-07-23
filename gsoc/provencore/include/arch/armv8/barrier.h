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
 * @brief ARM memory barrier wrappers
 * @author Vincent Siles
 * @date October 26th, 2016 (creation)
 * @copyright (c) 2016-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _ARCH_ARMV8_BARRIER_H_INCLUDED_
#define _ARCH_ARMV8_BARRIER_H_INCLUDED_

#include <compiler.h>

/* Data memory barrier */
inline __always_inline
static void dmb(void)
{
/* *INDENT-OFF* */
    asm volatile ("dmb sy" : : : "memory");
/* *INDENT-ON* */
}

inline __always_inline
static void dmb_ish(void)
{
/* *INDENT-OFF* */
    asm volatile ("dmb ish" : : : "memory");
/* *INDENT-ON* */
}

/* Data synchronization barrier */
inline __always_inline
static void dsb(void)
{
/* *INDENT-OFF* */
    asm volatile ("dsb sy" : : : "memory");
/* *INDENT-ON* */
}

/* Instruction synchronization barrier */
inline __always_inline
static void isb(void)
{
/* *INDENT-OFF* */
    asm volatile ("isb" : : : "memory");
/* *INDENT-ON* */
}

#endif /* _ARCH_ARMV8_BARRIER_H_INCLUDED_ */
