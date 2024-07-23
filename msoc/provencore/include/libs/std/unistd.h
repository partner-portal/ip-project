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
 * @brief
 * @author Henri Chataing
 * @date April 05th, 2019 (creation)
 * @copyright (c) 2019-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBSTD_UNISTD_H_INCLUDED_
#define _LIBSTD_UNISTD_H_INCLUDED_

#include <stdint.h>
// #include <sys/types.h>

/**
 * @uses SENDREC EXEC BREAK
 */
int execve(const char *path, char *const argv[], char *const envp[]);

/**
 * @uses SLEEP
 */
unsigned int sleep(unsigned int seconds);

/**
 * @uses BREAK
 */
int brk(void *addr);

/**
 * @uses BREAK
 */
void *sbrk(intptr_t incr);

/* Defined for compatibility reasons, should not be used. */
int getpid(void);

#if 0
/* SVID issue 1 */
/* _Noreturn void _exit(int status); */
#endif

#endif /* _LIBSTD_UNISTD_H_INCLUDED_ */
