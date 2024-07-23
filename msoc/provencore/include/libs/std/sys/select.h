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
 * @date June 12th, 2019 (creation)
 * @copyright (c) 2019-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBSTD_SYS_SELECT_H_INCLUDED_
#define _LIBSTD_SYS_SELECT_H_INCLUDED_

#include <limits.h>
#if 0
#include <sys/time.h>
#include <sys/types.h>
#endif

#define LONG_BIT    ((int)sizeof(unsigned long) * CHAR_BIT)
#ifdef CONFIG_FD_SETSIZE
#define FD_SETSIZE  CONFIG_FD_SETSIZE
#else
#define FD_SETSIZE  64
#endif


//typedef struct {
//    unsigned long fds_bits[FD_SETSIZE / LONG_BIT];
//} fd_set;


/** void FD_CLR(int fd, fd_set *set); */
#define FD_CLR(fd, set) \
    ({ \
        int __fd = (fd); \
        int __offset = __fd / LONG_BIT; \
        int __shift = __fd % LONG_BIT; \
        if (__fd >= 0 && __fd < FD_SETSIZE) { \
            (set)->fds_bits[__offset] &= ~(1UL << __shift); \
        } \
    })

/** void FD_SET(int fd, fd_set *set); */
#define FD_SET(fd, set) \
    ({ \
        int __fd = (fd); \
        int __offset = __fd / LONG_BIT; \
        int __shift = __fd % LONG_BIT; \
        if (__fd >= 0 && __fd < FD_SETSIZE) { \
            (set)->fds_bits[__offset] |= 1UL << __shift; \
        } \
    })

/** int FD_ISSET(int fd, fd_set *set); */
#define FD_ISSET(fd, set) \
    ({ \
        int __fd = (fd); \
        int __offset = __fd / LONG_BIT; \
        int __shift = __fd % LONG_BIT; \
        (__fd >= 0 && __fd < FD_SETSIZE) && \
        (((set)->fds_bits[__offset] & (1UL << __shift)) != 0); \
    })

/** void FD_ZERO(fd_set *set); */
#define FD_ZERO(set) \
    ({ \
        for (unsigned int nr = 0; nr < FD_SETSIZE / LONG_BIT; nr++) { \
            (set)->fds_bits[nr] = 0; \
        } \
    })


#if 0
/**
 * These functions defined in POSIX.1-2001 / POSIX.1g / BSD4.4 standards
 * are currently not implemented.
 */

int select(int nfds, fd_set *readfds, fd_set *writefds,
           fd_set *exceptfds, struct timeval *timeout);

int pselect(int nfds, fd_set *readfds, fd_set *writefds,
            fd_set *exceptfds, const struct timespec *timeout,
            const sigset_t *sigmask);
#endif

#endif /* _LIBSTD_SYS_SELECT_H_INCLUDED_ */
