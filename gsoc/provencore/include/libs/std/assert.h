/*
 * Copyright (c) 2014-2023 ProvenRun S.A.S
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
 * @brief Macros to ease the use of assert in applications
 * @author Vincent Siles
 * @date April 14th, 2014 (creation)
 * @copyright (c) 2014-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#undef assert
#undef __assert

#ifdef NDEBUG
#define assert(e)       ((void)0)
#else

#define assert(e)    ((e) ? (void)0 : __assert(__func__, __FILE__, \
                                               __LINE__, ""))

#ifndef _LIBSTD_ASSERT_H_INCLUDED_
#define _LIBSTD_ASSERT_H_INCLUDED_
void __assert(const char *function, const char *file, unsigned int line,
              const char *assertion);
#endif /* _LIBSTD_ASSERT_H_INCLUDED_ */
#endif /* NDEBUG */
