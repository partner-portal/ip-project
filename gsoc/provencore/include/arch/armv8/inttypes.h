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
 * @brief ProvenCore 'inttypes.h'
 * @author Vincent Siles
 * @date December 19th, 2016 (creation)
 * @copyright (c) 2016-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

/*
 * This file is here to fill a hole in the build process when compiling with
 * a 'none-eabi' flavored GCC:
 * 1) we don't rely on the usual libc so we build our kernel *and* our
 *    apps in freestanding mode.
 * 2) therefore, the stdint.h file used by gcc is its own file stdint-gcc.h
 * 3) since we are in freestanding mode, gcc doesn't care about inttypes.h
 * => Format string are not synchronized
 *
 * E.g uint64_t is defined as unsigned long long int but PRIx64 is defined as x
 *
 * This file aims at making things right so that format constant are correctly
 * defined, until we build our own toolchain
 */
#ifndef _ARCH_ARMV8_INTTYPES_H_INCLUDED_
#define _ARCH_ARMV8_INTTYPES_H_INCLUDED_

#include <stdint.h>

#if defined(__GNUC__)
#ifndef __linux__

_Static_assert(sizeof(uint32_t) == sizeof(unsigned int),
               "uint32_t size mismatch");
_Static_assert(sizeof(uint64_t) == sizeof(unsigned long int),
               "uint64_t size mismatch");
#define __have_long64 1

#ifdef __INT64_TYPE__
#define __int64_t_defined 1
#endif

#endif /* __linux__ */

#include_next <inttypes.h>

#endif /* defined(__GNUC__) */

/* Unknow compiler. Include the next one directly */
#ifndef __GNUC__
#include_next <inttypes.h>
#endif

#endif /* _ARCH_ARMV8_INTTYPES_H_INCLUDED_ */
