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
 * @brief
 * @author Henri Chataing
 * @date September 28th, 2017 (creation)
 * @copyright (c) 2017-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBSTD_SETJMP_H_INCLUDED_
#define _LIBSTD_SETJMP_H_INCLUDED_

#if defined(CONFIG_ARCH_HOST)

    #include_next <setjmp.h>

#else

    #include <common.h>

    #if defined(CONFIG_ARCH_ARMV7A)
        #include "arch/armv7a/setjmp_features.h"
    #elif defined(CONFIG_ARCH_ARMV8)
        #include "arch/armv8/setjmp_features.h"
    #elif defined(CONFIG_ARCH_RISCV64)
        #include "arch/riscv64/setjmp_features.h"
    #else
        #error "Unsupported architecture"
    #endif /* CONFIG_ARCH_* */

    typedef _JBTYPE jmp_buf[_JBLEN];

    int _setjmp(jmp_buf env);
    _Noreturn void _longjmp(jmp_buf env, int ret);

    #define setjmp(env)             _setjmp(env)
    #define longjmp(env, ret)       _longjmp(env, ret)

#endif

#endif /* _LIBSTD_SETJMP_H_INCLUDED_ */
