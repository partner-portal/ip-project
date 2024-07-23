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

#ifndef _LIBPNC_ARCH_RISCV64_IPC_LOW_H_INCLUDED_
#define _LIBPNC_ARCH_RISCV64_IPC_LOW_H_INCLUDED_

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
    uint64_t ipc_type_u64 = ipc_type;
    uint64_t mptr_u64 = (uintptr_t) mptr;
    uint64_t target_u64 = target;
    register uint64_t a0  asm ("a0")  = ipc_type_u64;
    register uint64_t a1  asm ("a1")  = mptr_u64;
    register uint64_t a2  asm ("a2")  = target_u64;
    register uint64_t a3  asm ("a3")  = timeout;

    register int64_t a0_out asm ("a0");

    asm volatile ("ecall" :
                  "=r" (a0_out) :
                  "r" (a0), "r" (a1), "r" (a2), "r" (a3) :
                  "memory");

    int64_t ret_code = a0_out;

    return (int) ret_code;
}

/**
 * @brief Helper for performing an IPC when the timeout input is not needed
 */
static inline
int _pnc_do_ipc_no_timeout(uint32_t ipc_type, s_pid_t target, message *mptr)
{
    uint64_t ipc_type_u64 = ipc_type;
    uint64_t mptr_u64 = (uintptr_t) mptr;
    uint64_t target_u64 = target;
    register uint64_t a0  asm ("a0")  = ipc_type_u64;
    register uint64_t a1  asm ("a1")  = mptr_u64;
    register uint64_t a2  asm ("a2")  = target_u64;
    /* Timeout (a3) is unused here */

    register int64_t a0_out asm ("a0");

    asm volatile ("ecall" :
                  "=r" (a0_out) :
                  "r" (a0), "r" (a1), "r" (a2) :
                  "memory");

    int64_t ret_code = a0_out;

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
    uint64_t ipc_type_u64 = ipc_type;
    uint64_t mptr_u64 = (uintptr_t) mptr;
    uint64_t target_u64 = target;
    register uint64_t a0  asm ("a0")  = ipc_type_u64;
    register uint64_t a1  asm ("a1")  = mptr_u64;
    register uint64_t a2  asm ("a2")  = target_u64;
    /* Timeout (a3) is unused here */

    register int64_t a0_out asm ("a0");

    asm volatile ("ecall" :
                  "=r" (a0_out) :
                  "r" (a0), "r" (a1), "r" (a2) :
                  "memory");

    int64_t ret_code = a0_out;

    return (int) ret_code;
}

/**
 * @brief Helper for performing an IPC when the only inputs are the ipc type
 *         and the target
 */
static inline
int _pnc_do_ipc_pid_only(uint32_t ipc_type, s_pid_t target)
{
    uint64_t ipc_type_u64 = ipc_type;
    uint64_t target_u64 = target;
    register uint64_t a0  asm ("a0")  = ipc_type_u64;
    /* Message (a1) is unused here */
    register uint64_t a2  asm ("a2")  = target_u64;
    /* Timeout (a3) is unused here */

    register int64_t a0_out asm ("a0");

    asm volatile ("ecall" :
                  "=r" (a0_out) :
                  "r" (a0), "r" (a2) :
                  "memory");

    int64_t ret_code = a0_out;

    return (int) ret_code;
}

/**
 * @brief Helper for performing an IPC when the only inputs are the ipc type
 *         and the timeout
 */
static inline
int _pnc_do_ipc_timeout_only(uint32_t ipc_type, uint64_t timeout)
{
    uint64_t ipc_type_u64 = ipc_type;
    register uint64_t a0  asm ("a0")  = ipc_type_u64;
    /* Message (a1) is unused here */
    /* Target (a2) is unused here */
    register uint64_t a3  asm ("a3")  = timeout;

    register int64_t a0_out asm ("a0");

    asm volatile ("ecall" :
                  "=r" (a0_out) :
                  "r" (a0), "r" (a3) :
                  "memory");

    int64_t ret_code = a0_out;

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

#endif /* _LIBPNC_ARCH_RISCV64_IPC_LOW_H_INCLUDED_ */
