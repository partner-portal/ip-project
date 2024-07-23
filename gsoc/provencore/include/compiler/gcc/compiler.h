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
 * @brief Compiler-specific definitions
 * @author Hadrien Barral
 * @date May 11th, 2018 (creation)
 * @copyright (c) 2018-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _COMPILER_H_INCLUDED_
#define _COMPILER_H_INCLUDED_

#ifndef __PROVENCORE__
/*
 * sys/cdefs.h defines some of the symbols below
 * and including it after this header would cause preprocessing errors.
 */
#include <sys/cdefs.h>
#endif /* __PROVENCORE__ */

#ifndef __pure
#define __pure              __attribute__((pure))
#endif

#ifndef __printflike
#define __printflike(n, m)  __attribute__((format (printf, (n), (m))))
#endif

/* Attribute 'unused' really is 'might be unused' */
#define __maybe_unused      __attribute__((unused))
#define __warn_unused       __attribute__((warn_unused_result))
#define __no_optimize       __attribute__((optimize("O0")))

/*
 * Short-hand for functions that *must* be inlined, in order
 *  not to break the kernel (by generating function calls where they cannot
 *  be handled).
 * The compiler library already defines '__always_inline', so we
 *  force our definition to be sure it is in line with our needs.
 */
#undef __always_inline
#define __always_inline     __attribute__((always_inline))

/* This is to be put on functions called from assembly only.
 * We do not want to force having a prototype in such cases here.
 */
#define __called_from_assembly_only \
    _Pragma("GCC diagnostic ignored \"-Wmissing-prototypes\"")

/*
 * This is used for void const * parameters which are never dereferenced by the
 * callee (i.e. only the address is used) to prevent -Wmaybe-uninitialized
 * warnings.
 */
#if __GNUC__ >= 11
#define __access_none(n)    __attribute__((access (none, (n))))
#else
#define __access_none(n)
#endif

#endif /*  _COMPILER_H_INCLUDED_ */
