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
 * @brief ProvenCore low level ABI (format of messages and IPC definitions)
 * @author Vincent Siles
 * @date April 9th, 2014 (creation)
 * @copyright (c) 2014-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBPNC_IPC_H_INCLUDED_
#define _LIBPNC_IPC_H_INCLUDED_

#include <ipc_const.h>
#include <stddef.h>
#include <sys/cdefs.h>  /* __BEGIN_DECLS */

__BEGIN_DECLS

/*
 * The compact_message type is part of the kernel/userspace interface;
 * both have to agree on its size, alignment, and layout. More specifically, in
 * IPC calls, the kernel expects (and checks) that userspace will provide
 * a virtual address:
 *
 *  1. suitably aligned for compact_message;
 *  2. starting an interval with size sizeof(compact_message);
 *  3. such that this interval is included in the caller's data or stack region;
 *  4. such that this interval does not cross a 4 KiB page boundary.
 *
 * We can have conditions 1 and 2 for free (but not really condition 3) in the
 * user library by simply requiring a pointer to compact_message.
 * Condition 4 is more problematic, though. Our solution is to use an
 * overaligned duplicate of compact_message, message. We make sure that we
 * satisfy conditions 1 and 2, and agree with the kernel on the layout of
 * the message by statically asserting that:
 *
 *  1. message is at least as strictly aligned as compact_message;
 *  2. message is at least as long as compact_message;
 *  3. each field of message has the same offset inside the structure as the
 *     corresponding field in compact_message.
 *
 * Corresponding static checks are implemented in lib/pnc/syscall.c
 */

typedef struct {
    _Alignas(64)
    uint64_t m_source;
    uint64_t uptime; /**< in µs */
    uint32_t m_type;
    uint32_t m1;
    uint64_t m2;
    uint32_t m3;
    uint32_t m4;
    uint64_t m5;
    uint64_t m6;
} message;

#define MSOURCE(m)              ((s_pid_t) ((m)->m_source & (MSOURCE_MASK)))

#define is_ipc_notify(m)        (((m)->m_source & MSOURCE_NOTIFY_FLAG) != 0)
#define is_ipc_with_receive(m)  (((m)->m_source & MSOURCE_RECEIVE_FLAG) != 0)
#define is_ipc_sendrec(m)       (!is_ipc_notify(m) && is_ipc_with_receive(m))
#define is_ipc_send(m)          (!is_ipc_notify(m) && !is_ipc_with_receive(m))
#define ipc_notify_src(m)       (MSOURCE(m))
#define ipc_notify_arg(m)       ((m)->m1)

/*
 * `send`, `send_nonblock`, `receive`, `receive_nonblock`, `sendrec`,
 * `sendrec_nonblock`, `notify` and `pnc_sleep` are defined as
 * static inline in ipc_low.h, but we keep the prototypes here anyway as:
 * - it is easier for users to read their doc
 * - these functions are arch-dependent, so we make sure the prototypes stay
 *    unified
 */

/**
 * @brief Simple check to get the kind of a process.
 * The pid of user processes always has the bit 54 set.
 */
#define is_sysproc_pid(pid)     (((pid) & (UINT64_C(1) << 54)) == 0)

/**
 * @brief Send a message to \p dest.
 *
 * The current process will be blocked until \p dest provides a message.
 *
 * @param dest Identity of the destination
 * @param mptr Pointer to a \ref message structure to send to \p dest
 * @return Status of the IPC (see ProvenCore's API documentation)
 * @uses SEND
 */
static int send(s_pid_t dest, const message *mptr);
static int send_nonblock(s_pid_t dest, const message *mptr);

/**
 * @brief Receive a message from \p src
 *
 * @param src  Identity of the source
 * @param mptr Pointer to a \ref message structure where to store the
 *             received message from \p src
 * @param timeout Time limit to wait for the message (in µs). Use the
 *                special value \ref NO_TIMEOUT if the call should wait
 *                forever.
 * @return Status of the IPC (see ProvenCore's API documentation)
 * @uses RECEIVE
 */
static int receive(s_pid_t src, message *mptr, uint64_t timeout);
static int receive_nonblock(s_pid_t src, message *mptr);

/**
 * @brief Receive a message from the special HARDWARE source.
 *
 * @param mptr Pointer to a \ref message structure where to store the
 *             received notification message.
 * @param timeout Time limit to wait for the message (in µs). Use the
 *                special value \ref NO_TIMEOUT if the call should wait
 *                forever.
 * @return Status of the IPC (see ProvenCore's API documentation)
 * @uses RECEIVE
 */
static inline int receive_hw(message *mptr, uint64_t timeout)
{
    return receive(HARDWARE, mptr, timeout);
}

static inline int receive_hw_nonblock(message *mptr)
{
    return receive_nonblock(HARDWARE, mptr);
}

/**
 * @brief Receive a message from any source
 *
 * @param mptr Pointer to a \ref message structure where to store the
 *             received message from \p src
 * @param timeout Time limit to wait for the message (in µs). Use the
 *                special value \ref NO_TIMEOUT if the call should wait
 *                forever.
 * @return Status of the IPC (see ProvenCore's API documentation)
 * @uses RECEIVE
 */
static inline int receive_any(message *mptr, uint64_t timeout)
{
    return receive(TARGET_ANY, mptr, timeout);
}

static inline int receive_any_nonblock(message *mptr)
{
    return receive_nonblock(TARGET_ANY, mptr);
}

/**
 * @brief Send a message and wait for the reception of the anwser.
 *
 * Note that no timeout on the receive part of the IPC is available.
 *
 * @param src_dst Identify of the process/task to contact
 * @param mptr Pointer to a \ref message structure to be send to \p src_dst. Will
 * be updated by the answer from \p src_dst
 * @return Status of the IPC (see ProvenCore's API documentation)
 * @uses SENDREC
 */
static int sendrec(s_pid_t src_dst, message *mptr);
static int sendrec_nonblock(s_pid_t src_dst, message *mptr);

/**
 * @brief Send a notification to \p dst
 *
 * This call is non-blocking: the current process will resume execution even
 * if \p dst is not receiving at the moment.
 *
 * @param dst  Identity of the destination
 * @return Status of the IPC (see ProvenCore's API documentation)
 * @uses NOTIFY
 */
static int notify(s_pid_t dest);

/**
 * @brief Stop the execution for some time.
 *
 * @param timeout Time limit to wait in µs. Must be different from \ref NO_TIMEOUT
 * @return Status of the IPC (see ProvenCore's API documentation)
 * @uses SLEEP
 */
static int pnc_sleep(uint64_t timeout);

/**
 * @brief Casts an application virtual address in order to store in 32-bit
 *         message fields.
 *        Note that it works even for 64-bit applications, because valid
 *         addresses are in the first 1GiB range, so they fit in 32 bits.
 * @param ptr Pointer to an application virtual address
 * @return A value representing \p ptr suitable to store in m1/m3/m4
 */
static inline uint32_t ptr_to_u32_message_field(const void *ptr)
{
    return (uint32_t)(uintptr_t)ptr;
}

/**
 * @brief Casts an application virtual address in order to store in 64-bit
 *         message fields.
 * @param ptr Pointer to an application virtual address
 * @return A value representing \p ptr suitable to store in m2/m5/m6
 */
static inline uint64_t ptr_to_u64_message_field(const void *ptr)
{
    return (uint64_t)(uintptr_t)ptr;
}


/**
 * @brief Casts a 32-bit message fields into a void pointer
 *        See `ptr_to_u32_message_field`
 * @param mx 32-bit Message field
 * @return An application virtual address representing \p mx
 */
static inline void *u32_message_field_to_ptr(uint32_t mx)
{
    return (void *)(uintptr_t)mx;
}

/**
 * @brief Casts a 64-bit message fields into a void pointer
 * @param mx 64-bit Message field
 * @return An application virtual address representing \p mx
 */
static inline void *u64_message_field_to_ptr(uint64_t mx)
{
    return (void *)(uintptr_t)mx;
}

/**
 * @brief Casts an application object size in order to store in 32-bit
 *         message fields.
 *        Note that it works even for 64-bit applications, because valid
 *         addresses are in the first 1GiB range, so they fit in 32 bits.
 * @param size An application object size
 * @return A value representing \p ptr suitable to store in m1/m3/m4
 */
static inline uint32_t size_to_u32_message_field(const size_t size)
{
    return (uint32_t)size;
}

/**
 * @brief Casts an application object size in order to store in 64-bit
 *         message fields.
 * @param size An application object size
 * @return A value representing \p ptr suitable to store in m2/m5/m6
 */
static inline uint64_t size_to_u64_message_field(const size_t size)
{
    return (uint64_t)size;
}


/**
 * @brief Casts a 32-bit message field into an object size
 *        See `size_to_u32_message_field`
 * @param mx 32-bit message field
 * @return An application object size representing \p mx
 */
static inline size_t u32_message_field_to_size(uint32_t mx)
{
    return (size_t)mx;
}

/**
 * @brief Casts a 64-bit message field into an object size
 * @param mx 64-bit message field
 * @return An application object size representing \p mx
 */
static inline size_t u64_message_field_to_size(uint64_t mx)
{
    return (size_t)mx;
}

static inline void copy_compact_message_to_compact_message(compact_message *dest,
                                                           compact_message const *src)
{
    *dest = *src;
}

static inline void copy_compact_message_to_message(message *dest,
                                                   compact_message const *src)
{
    dest->m_source = src->m_source;
    dest->uptime = src->uptime;
    dest->m_type = src->m_type;
    dest->m1 = src->m1;
    dest->m2 = src->m2;
    dest->m3 = src->m3;
    dest->m4 = src->m4;
    dest->m5 = src->m5;
    dest->m6 = src->m6;
}

static inline void copy_message_to_compact_message(compact_message *dest,
                                                   message const *src)
{
    dest->m_source = src->m_source;
    dest->uptime = src->uptime;
    dest->m_type = src->m_type;
    dest->m1 = src->m1;
    dest->m2 = src->m2;
    dest->m3 = src->m3;
    dest->m4 = src->m4;
    dest->m5 = src->m5;
    dest->m6 = src->m6;
}

static inline void copy_message_to_message(message *dest, message const *src)
{
    *dest = *src;
}

/**
 * Generic macro to copy to / from compact messages and ipc messages.
 * @param dest      Pointer to the destination message _ compact or ipc
 * @param src       Pointer to the source message _ compact or ipc
 */
#define copy_message(dest, src) \
    _Generic ((dest), \
              compact_message *: \
              _Generic((src), \
                       compact_message const *: copy_compact_message_to_compact_message, \
                       compact_message *: copy_compact_message_to_compact_message, \
                       message const *: copy_message_to_compact_message, \
                       message *: copy_message_to_compact_message), \
              message *: \
              _Generic((src), \
                       compact_message const *: copy_compact_message_to_message, \
                       compact_message *: copy_compact_message_to_message, \
                       message const *: copy_message_to_message, \
                       message *: copy_message_to_message))((dest), (src))

#if defined(CONFIG_ARCH_ARMV7A)
    #include "arch/armv7a/ipc_low.h"
#elif defined(CONFIG_ARCH_ARMV8)
    #include "arch/armv8/ipc_low.h"
#elif defined(CONFIG_ARCH_RISCV64)
    #include "arch/riscv64/ipc_low.h"
#else
    #error "Unsupported architecture"
#endif /* CONFIG_ARCH_* */

/**
 * Function-like macros to test and extract error codes from invalid pids
 * generated by the INVALID_PID function-like macro.
 *
 * These macros ensure the following properties:
 * - is_invalid_pid(INVALID_PID(code)) is always true
 * - assuming that code is an unsigned invalid_pid_code_width bits integer,
 *   invalid_pid_code(INVALID_PID(code)) == code
 * - assuming that is_invalid_pid(pid) holds,
 *   INVALID_PID(invalid_pid_code(pid)) == pid
 */
#define is_invalid_pid(pid) \
    (((((uint64_t)(pid)) >> (INVALID_PID_CODE_SHIFT + INVALID_PID_CODE_WIDTH)) \
      == INVALID_PID_LEFT) && \
     ((((uint64_t)(pid)) & ((UINT64_C(1) << INVALID_PID_CODE_SHIFT) - 1)) \
      == INVALID_PID_RIGHT))
#define invalid_pid_code(pid) \
    ((((uint64_t)(pid)) >> INVALID_PID_CODE_SHIFT) & ((UINT64_C(1) << INVALID_PID_CODE_WIDTH) - 1))

__END_DECLS

#endif /* _LIBPNC_IPC_H_INCLUDED_ */
