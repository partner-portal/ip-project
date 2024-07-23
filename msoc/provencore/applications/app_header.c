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
 * @brief Applications header section
 * @author Vincent Siles
 * @date February 1st, 2018 (creation)
 * @copyright (c) 2018-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#include <pnc_headers.h>
#include <stdint.h>

#include "version.h"

#ifndef __KERNEL_VERSION__
#error "Missing __KERNEL_VERSION__"
#endif

#ifndef STATIC_ARCHIVES_SIZE
#error "Missing STATIC_ARCHIVES_SIZE"
#endif

#define CHECK_UL(X)                                                  \
    (__builtin_choose_expr                                           \
         (__builtin_types_compatible_p(typeof(X), unsigned long), 0, \
         1))

_Static_assert(CHECK_UL(__KERNEL_VERSION__) == 0,
               "Invalid __KERNEL_VERSION__");
_Static_assert(CHECK_UL(STATIC_ARCHIVES_SIZE) == 0,
               "Invalid STATIC_ARCHIVES_SIZE");

__attribute__((section(".app.header")))
app_header_t app_header = {
    .header_version = __KERNEL_VERSION__,
    .code_size = STATIC_ARCHIVES_SIZE
};
