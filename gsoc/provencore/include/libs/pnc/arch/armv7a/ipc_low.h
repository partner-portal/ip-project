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
 * @brief Arch-dependent low-level IPC helpers
 * @author Hadrien Barral
 * @date April 10th, 2019 (creation)
 * @copyright (c) 2019-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBPNC_ARCH_ARMV7A_IPC_LOW_H_INCLUDED_
#define _LIBPNC_ARCH_ARMV7A_IPC_LOW_H_INCLUDED_

#include <asm.h>
#include <ipc_const.h>

/*
 * The wrappers around the assembly instructions in this file are carefully
 * written. Please pay attention to the following points.
 *
 * The "memory" clobber is intentional. Not only do some IPCs access other
 * memory locations than the message (kernel message-based calls), but IPCs in
 * general are also used as a synchronization mechanism, which requires them to
 * be acquire and release operations. It is therefore necessary to prevent the
 * compiler from reordering the assembly statements with memory operations,
 * which the "memory" clobber takes care of.
 *
 * Moreover, we use GCC's local register variables, which have important
 * limitations outlined in
 * https://gcc.gnu.org/onlinedocs/gcc/Local-Register-Variables.html
 */

/**
 * @brief Full helper for performing an IPC
 */
static inline
int _pnc_do_ipc(uint32_t ipc_type, s_pid_t target, message *mptr,
                uint64_t timeout)
{
    uint32_t mptr_u32 = (uintptr_t) mptr;
    uint32_t target_lo = (uint32_t)  target;
    uint32_t target_hi = (uint32_t) (target >> 32);
    uint32_t timeout_lo = (uint32_t)  timeout;
    uint32_t timeout_hi = (uint32_t) (timeout >> 32);

    register uint32_t r0  asm ("r0")  = ipc_type;
    register uint32_t r1  asm ("r1")  = mptr_u32;
    register uint32_t r2  asm ("r2")  = target_lo;
    register uint32_t r3  asm ("r3")  = target_hi;
    register uint32_t r4  asm ("r4")  = timeout_lo;
    register uint32_t r5  asm ("r5")  = timeout_hi;

    register int32_t r0_out  asm ("r0");

    asm volatile ("svc     #0" :
                  "=r" (r0_out) :
                  "r" (r0), "r" (r1), "r" (r2), "r" (r3), "r" (r4), "r" (r5) :
                  "memory");

    int32_t ret_code = r0_out;

    return (int) ret_code;
}

/**
 * @brief Helper for performing an IPC when the timeout input is not needed
 */
static inline
int _pnc_do_ipc_no_timeout(uint32_t ipc_type, s_pid_t target, message *mptr)
{
    uint32_t mptr_u32 = (uintptr_t) mptr;
    uint32_t target_lo = (uint32_t)  target;
    uint32_t target_hi = (uint32_t) (target >> 32);

    register uint32_t r0  asm ("r0")  = ipc_type;
    register uint32_t r1  asm ("r1")  = mptr_u32;
    register uint32_t r2  asm ("r2")  = target_lo;
    register uint32_t r3  asm ("r3")  = target_hi;
    /* Timeout (r4/r5) is unused here */

    register int32_t r0_out  asm ("r0");

    asm volatile ("svc     #0" :
                  "=r" (r0_out) :
                  "r" (r0), "r" (r1), "r" (r2), "r" (r3) :
                  "memory");

    int32_t ret_code = r0_out;

    return (int) ret_code;
}

/**
 * @brief Helper for performing an IPC when the timeout input is not needed
 *         and the input message is read-only
 */
static inline
int _pnc_do_ipc_no_timeout_const_message(uint32_t ipc_type, s_pid_t target,
                                         const message *mptr)
{
    uint32_t mptr_u32 = (uintptr_t) mptr;
    uint32_t target_lo = (uint32_t)  target;
    uint32_t target_hi = (uint32_t) (target >> 32);

    register uint32_t r0  asm ("r0")  = ipc_type;
    register uint32_t r1  asm ("r1")  = mptr_u32;
    register uint32_t r2  asm ("r2")  = target_lo;
    register uint32_t r3  asm ("r3")  = target_hi;
    /* Timeout (r4/r5) is unused here */

    register int32_t r0_out  asm ("r0");

    asm volatile ("svc     #0" :
                  "=r" (r0_out) :
                  "r" (r0), "r" (r1), "r" (r2), "r" (r3) :
                  "memory");

    int32_t ret_code = r0_out;

    return (int) ret_code;
}

/**
 * @brief Helper for performing an IPC when the only inputs are the ipc type
 *         and the target
 */
static inline
int _pnc_do_ipc_pid_only(uint32_t ipc_type, s_pid_t target)
{
    uint32_t target_lo = (uint32_t)  target;
    uint32_t target_hi = (uint32_t) (target >> 32);

    register uint32_t r0  asm ("r0")  = ipc_type;
    /* Message (r1) is unused here */
    register uint32_t r2  asm ("r2")  = target_lo;
    register uint32_t r3  asm ("r3")  = target_hi;
    /* Timeout (r4/r5) is unused here */

    register int32_t r0_out  asm ("r0");

    asm volatile ("svc     #0" :
                  "=r" (r0_out) :
                  "r" (r0), "r" (r2), "r" (r3) :
                  "memory");

    int32_t ret_code = r0_out;

    return (int) ret_code;
}

/**
 * @brief Helper for performing an IPC when the only inputs are the ipc type
 *         and the timeout
 */
static inline
int _pnc_do_ipc_timeout_only(uint32_t ipc_type, uint64_t timeout)
{
    uint32_t timeout_lo = (uint32_t)  timeout;
    uint32_t timeout_hi = (uint32_t) (timeout >> 32);

    register uint32_t r0  asm ("r0")  = ipc_type;
    /* Message (r1) is unused here */
    /* Target (r2/r3) is unused here */
    register uint32_t r4  asm ("r4")  = timeout_lo;
    register uint32_t r5  asm ("r5")  = timeout_hi;

    register int32_t r0_out  asm ("r0");

    asm volatile ("svc     #0" :
                  "=r" (r0_out) :
                  "r" (r0), "r" (r4), "r" (r5) :
                  "memory");

    int32_t ret_code = r0_out;

    return (int) ret_code;
}


/* See doc in header */
static inline int send(s_pid_t dest, const message *mptr)
{
    return _pnc_do_ipc_no_timeout_const_message(SEND, dest, mptr);
}

/* See doc in header */
static inline int send_nonblock(s_pid_t dest, const message *mptr)
{
    return _pnc_do_ipc_no_timeout_const_message(SEND_NONBLOCK, dest, mptr);
}

/* See doc in header */
static inline int receive(s_pid_t src, message *mptr, uint64_t timeout)
{
    return _pnc_do_ipc(RECEIVE, src, mptr, timeout);
}

/* See doc in header */
static inline int receive_nonblock(s_pid_t src, message *mptr)
{
    return _pnc_do_ipc_no_timeout(RECEIVE_NONBLOCK, src, mptr);
}

/* See doc in header */
static inline int sendrec(s_pid_t dest, message *mptr)
{
    return _pnc_do_ipc_no_timeout(SENDREC, dest, mptr);
}

/* See doc in header */
static inline int sendrec_nonblock(s_pid_t dest, message *mptr)
{
    return _pnc_do_ipc_no_timeout(SENDREC_NONBLOCK, dest, mptr);
}

/* See doc in header */
static inline int notify(s_pid_t dest)
{
    return _pnc_do_ipc_pid_only(NOTIFY, dest);
}

/* See doc in header */
static inline int pnc_sleep(uint64_t timeout)
{
    return _pnc_do_ipc_timeout_only(SLEEP, timeout);
}

#endif /* _LIBPNC_ARCH_ARMV7A_IPC_LOW_H_INCLUDED_ */
