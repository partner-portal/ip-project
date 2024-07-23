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
 * @brief sys/types.h redirect.
 * @author Hadrien Barral
 * @date January 26th, 2018 (creation)
 * @copyright (c) 2018-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

/* Clang is really freestanding. Unlike gcc, it does not
 * have "sys/types.h" in freestanding mode.
 */

#ifndef __PROVENCORE__

/* Call the lib one */
#include_next <sys/types.h>

/* Sometimes, useconds_t is not defined */
#if (!defined(__useconds_t_defined)) && (!defined(_USECONDS_T_DECLARED))
#include <stdint.h>
typedef uint64_t useconds_t;
#define __useconds_t_defined
#define _USECONDS_T_DECLARED
#endif /* (!defined(__useconds_t_defined)) && ... */

#endif /* __PROVENCORE__ */
