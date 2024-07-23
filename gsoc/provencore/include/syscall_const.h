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
 * @brief Defines constants related to kernel system calls.
 * @author Henri Chataing
 * @date February 13th, 2019 (creation)
 * @copyright (c) 2019-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 *
 * /!\ This file is shared between the kernel and applications.
 */

#ifndef _SYSCALL_CONST_H_INCLUDED_
#define _SYSCALL_CONST_H_INCLUDED_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

/*
 * Kernel system calls.
 * There must be no gap between two numbers.
 *
 * /!\
 * Do not forget to update NR_CALLS if a new syscall is added.
 * /!\
 */
#define FORK                    0
#define EXIT                    1
#define EXEC                    2
#define COPY                    3
#define BREAK                   4
#define GETINFO                 5
#define SETAUTH                 6
#define REVOKE                  7
#define CHANGE_TARGET           8
#define CHANGE_EFFECTOR         9
#define CHANGE_REVOKER          10
#define IRQCTL                  11
#define KILL                    12
#define IOMAP                   13
#define IOUNMAP                 14
#define SHM_ALLOC               15
#define SHM_REGISTER            16
#define SHM_UNREGISTER          17
#define SHM_TRANSFER            18
#define SMC                     19
#define LOAD_EXEC               20
#define HYPERCALL               21
#define PLATCTL                 22

#define NR_CALLS                23

/**
 * @brief This type enumerates the various requests handled by \c GETINFO.
 *
 * @details
 *   \c INFO_DUMP and \c INFO_DEBUGGER are only optionally implemented
 *   and can lead to the kernel call returning \c ENOSYS.
 *
 *   \c INFO_UNKNOWN and \c NR_INFO_REQUESTS are not valid request values
 *   <em>per se</em> and are defined for convenience as the smallest
 *   non-negative invalid request value.
 *
 * @warn
 *   When a new type of request is added, think of extending the CC
 *   tests accordingly.
 */
typedef enum {
    INFO_PID = 0,
    INFO_DOMAIN,
    INFO_TIMES,
    INFO_UPTIME,
    INFO_QUOTA,
    INFO_DATA,
    INFO_MEMCHECK,
    INFO_METADATA_APP,
    INFO_METADATA_PID,
    INFO_SHMTOKEN,
    INFO_PHYS_BUFF,
    INFO_DUMP,
    INFO_DEBUGGER,
    INFO_SYSPROC_PID_NAME,
    INFO_CAPABILITIES,
    INFO_IPC,
    INFO_UNKNOWN,
    NR_INFO_REQUESTS = INFO_UNKNOWN
} info_request_t;

/* INFO_MEMCHECK flags values. */
#define MEMCHECK_READ           1U
#define MEMCHECK_WRITE          2U
#define MEMCHECK_ANY_OWNER      4U

/* INFO_CAPABILITIES and INFO_IPC flag values. */
#define CAPABILITIES_TRAP_SEND     1U
#define CAPABILITIES_TRAP_RECEIVE  2U
#define CAPABILITIES_TRAP_SENDREC  4U
#define CAPABILITIES_TRAP_NOTIFY   8U
#define CAPABILITIES_TRAP_SLEEP   16U

#define CAPABILITIES_PREEMPTIBLE   1U
#define CAPABILITIES_SYSPROC       2U
#define CAPABILITIES_RESTART       4U
#define CAPABILITIES_DEBUGGER      8U
#define CAPABILITIES_DEBUGGEE     16U
#define CAPABILITIES_KILLABLE     32U
#define CAPABILITIES_AUDITOR      64U

/* IRQCTL commands. */
#define IRQ_SETPOLICY           1
#define IRQ_RMPOLICY            2
#define IRQ_STATUS              3

/* IRQCTL status values. */
#define IRQ_ENABLED             1
#define IRQ_DISABLED            2
#define IRQ_RELEASED            3

/* IRQCTL flag values. */
#define IRQ_REENABLE_NO         0
#define IRQ_REENABLE_YES        1

/* Memory authorization constants. */
#define NO_ACCESS               0
#define READ_ACCESS             1
#define WRITE_ACCESS            2
#define RW_ACCESS               (READ_ACCESS | WRITE_ACCESS)

/*
 * Constant representing an invalid authorization.
 *  (Even if KCONFIG_NR_AUTH == UINT_MAX, auth number UINT_MAX is invalid)
 */
#define NO_AUTH                 UINT_MAX

/* Quota requests for the EXEC system call. */
_Static_assert(sizeof(size_t) >= sizeof(uint32_t),
               "size_t should be at least as large as uint32_t");
#define EXEC_QUOTA_ALL          SIZE_C(0xFFFFFFFF)
#define EXEC_QUOTA_ANY          SIZE_C(0)
#define EXEC_QUOTA_MAX          SIZE_C(0xEFFFFFFF)

/* Exit options. */
#define EXIT_SHUTDOWN           0
#define EXIT_RESTART            1

/* Kill signals. *Not* compatible with POSIX signals. */
#define KILLSIG_TERMINATE       UINT32_C(0)
#define KILLSIG_PAUSE           UINT32_C(1)
#define KILLSIG_RESUME          UINT32_C(2)

/* Debugger commands */
#define DEBUGGER_READ_REG       UINT32_C(0x40)
#define DEBUGGER_WRITE_REG      UINT32_C(0x41)
#define DEBUGGER_READ_MEM       UINT32_C(0x42)
#define DEBUGGER_WRITE_MEM      UINT32_C(0x43)
#define DEBUGGER_STATUS         UINT32_C(0x44)

/*
 * ARG_MAX is usually defined in <limits.h> but not available on some non
 * GNU compilers, so we define our own
 */
#define EXEC_ARG_MAX            4096U

#endif /* _SYSCALL_CONST_H_INCLUDED_ */
