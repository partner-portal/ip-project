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
 * @brief Defines initial-stack related types (before fixup by crt0.S)
 * @author Hadrien Barral
 * @date June 14th, 2018 (creation)
 * @copyright (c) 2018-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _ARCH_ARMV8_INIT_STACK_H_INCLUDED_
#define _ARCH_ARMV8_INIT_STACK_H_INCLUDED_

#ifdef CONFIG_ARCH_ARMV7A
/*
 * We are in the armv8 kernel case, but with armv7a userland.
 * Thus, we want to include v7 init_stack.h
 */
#include "../armv7a/init_stack.h"
#else

#include <stdint.h>

/**
 * @brief Basic block of exec's initial stack serialization format.
 */
typedef uint64_t init_stack_element_t;

/** ARM C stack must be aligned to a 16 byte boundary */
#define USER_STACK_ALIGNMENT        16

#endif /* CONFIG_ARCH_* */
#endif /* _ARCH_ARMV8_INIT_STACK_H_INCLUDED_ */
