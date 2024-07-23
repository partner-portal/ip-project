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
 * @brief List of ProvenCore kernel call signatures
 * @author Vincent Siles
 * @date April 14th, 2014 (creation)
 * @copyright (c) 2014-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBPNC_SYSCALL_INTERNAL_H_INCLUDED_
#define _LIBPNC_SYSCALL_INTERNAL_H_INCLUDED_

#include <arch_const.h>         /* paddr_t */
#include <compiler.h>
#include <ipc.h>
#include <pnc_metadata.h>
#include <smc.h>
#include <stdbool.h>
#include <stddef.h>             /* size_t */
#include <stdint.h>
#include <sys/cdefs.h>          /* __BEGIN_DECLS */
#include <syscall_const.h>

#if defined(CONFIG_ARCH_ARMV7A) || defined(CONFIG_ARCH_ARMV8)
#include <hvc.h>
#endif /* defined(CONFIG_ARCH_ARMV7A) || defined(CONFIG_ARCH_ARMV8) */

__BEGIN_DECLS

typedef struct smc32_params smc32_params_t;
typedef struct smc64_params smc64_params_t;
typedef struct mcall_params mcall_params_t;
typedef struct hvc32_params hvc32_params_t;
typedef struct hvc64_params hvc64_params_t;

/**
 * @brief Internal method.
 * @uses SENDREC
 */
int pnc_slow_kernel_call(int callnr, message *payload);

/**
 * @brief Duplicate the current process, spawning a new process with the same
 *  state and almost the same resources.
 *
 * @note It is not advised to call this function directly, one should use
 *  \ref s_fork instead, which updates internal library settings.
 *  In particular, the implementation of \ref printf relies on a permanent
 *  authorization set on a print buffer for the UARTOUT process. This
 *  authorization needs to be reset in the child process in order for subsequent
 *  calls to \ref printf to execute normally.
 *
 *  - The child process gets a new and unique ID.
 *  - The child process is created in the same domain as the parent process.
 *  - The child process automatically registers to any shared memory region
 *      registered by the current process. They are all set up without any
 *      access, even if the current process has read/write access to the
 *      region. See \ref pnc_shm_alloc for more information about shared memory.
 *  - The child process does not inherit any region of device memory that
 *      the current process might have registered to using \ref pnc_iomap.
 *  - The child process does not inherit the physical buffer that the current
 *      may have requested at boot time.
 *  - The child process does not inherit any region shared with the non-secure
 *      World that the current process might have registered to using
 *      \ref pnc_iomap.
 *  - The child process starts with no authorization at all, they are not
 *      inherited from the current process. See \ref pnc_setauth for more
 *      information about authorizations.
 *  - The current quantum of time is split between both processes.
 *
 *  If the process was privileged, the new process loses the
 *  privileges and becomes a "user" process. As a side effect, this will also
 *  clear any pending interrupt or notification <a>for the child process</a>:
 *  as a user process sharing the same privilege with all user processes,
 *  it will no longer be able to receive any notification, it will not even
 *  be able to use \ref pnc_notify or \ref pnc_irqctl.
 *  All user processes share the same privileges (which can be considered
 *  as the "default" privileges of the system).
 *
 * @param is_parent         Pointer to a buffer written to `1` for the
 *                            parent process, and `0` for the child process.
 * @param child_pid         Pointer to a buffer where to store the identifier
 *                            of the created child process.
 * @return
 * - `EAGAIN`
 *      If there is no free slot in the kernel's process table.
 * - `ENOMEM`
 *      If the system ran out of physical memory while trying to
 *      create a new process.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the FORK kernel call.
 * - `OK` on success.
 * @uses SENDREC FORK
 */
int pnc_fork(_Bool *is_parent, s_pid_t *child_pid);

/**
 * @brief Control the execution of another process.
 *
 *  It can be used to \a pause, \a resume or \a terminate the execution of
 *  another process. Since there is no support for POSIX signals, it is the
 *  equivalent of `kill SIGSTOP`, `kill SIGCONT` and
 *  `kill SIGKILL` in most Unix-based distributions.
 *
 *  - It is always forbidden to use \ref pnc_kill on kernel tasks.
 *  - A privileged process can use \ref pnc_kill on any user process
 *      if they are in the same domain. If a privileged
 *      process is domain 0, it can use \ref pnc_kill on any user process.
 *  - A user process can only use \ref pnc_kill on another user
 *      process, if they are in the same domain. If a user process is in
 *      domain 0, it can use \ref pnc_kill on any user process.
 *  - A process cannot pause or resume itself.
 *  - There is a debug mechanism allowing some privileged processes to pause
 *      or resume other privileged processes, but it is not available in
 *      client releases. Please contact the ProvenCore team
 *      for more information.
 *
 * @param target        Identifier of the process we want to control.
 * @param signal        Control operation to be performed. This value should
 *                        be one of \ref KILLSIG_TERMINATE, \ref KILLSIG_PAUSE
 *                        or \ref KILLSIG_RESUME.
 * @return
 * - `EINVAL`
 *      If \p target does not correspond to an active process.
 * - `EBADMSG`
 *      If \p signal is not valid.
 * - `EPERM`
 *      If \p target is a kernel task, if the current process and
 *      \p target are not in compatible domains, or if \p target
 *      is a privileged process.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the KILL kernel call.
 * - `OK` on success.
 * @uses SENDREC KILL
 */
int pnc_kill(s_pid_t target, uint32_t signal);

/**
 * @brief Replace the code and data of the current process
 *  with the ones from a new secure executable pointed to by
 *  \p path.
 *
 *  The initial stack frame of the new process is built on the user side
 *  (in the library) before being sent to the kernel.
 *
 * @warn \ref pnc_exec _can_ terminate the current process on failure.
 *  This will happen if the kernel cannot allocate the necessary pages
 *  to setup the new ELF.
 *
 * @param path      Name of the new process to run. It can only be chosen
 *                    from the static list of embedded applications that are
 *                    shipped with the kernel.
 * @param argv      This array holds the initial arguments of the new process.
 *                    It is a \a NULL terminated array, which is used to build
 *                    the initial frame of the new process.
 * @param envp      This array holds the initial environment of the new process.
 *                    It is a \a NULL terminated array, which is used to
 *                    build the initial frame of the new process.
 * @param quota     Requested number of memory pages that will be
 *                    used for quota value to limit the available memory of
 *                    the new process. A quota value of \ref EXEC_QUOTA_ANY means
 *                    that no particular quota limit is requested, and that the
 *                    process will use the maximum allowed quota. A quota value
 *                    of \ref EXEC_QUOTA_ALL means that the process must not
 *                    have a quota restriction at all. Other values higher than
 *                    \ref EXEC_QUOTA_MAX are invalid.
 * @return
 * - `ENOMEM`
 *      If the requested stack data is too large for the system.
 * - `EINVAL`
 *      If the \p path, \p argv, or \p envp point to invalid ranges of
 *      addresses in the caller's address space, or if \p quota takes an invalid
 *      value.
 * - `ESRCH`
 *      If the requested executable name cannot be found
 *      in the store of all available executables.
 * - `E2BIG`
 *      If the requested quota exceeds the limitation of the system
 *      configuration or the quota of the current process. This can
 *      never happen if \p quota is \ref EXEC_QUOTA_ANY.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the EXEC kernel call.
 * - On success, \ref pnc_exec is `_Noreturn`
 * @uses SENDREC EXEC BREAK
 */
int pnc_exec(const char *path, char *const *argv,
             char *const *envp, uint64_t quota);

/**
 * @brief Terminate the current process.
 *
 * @note It is not advised to call this function directly, one should use
 *  the standard \ref exit function instead, which calls functions tagged
 *  `__attribute__((destructor))` and other callbacks registered through
 *  \ref atexit.
 *
 *  It also frees all the resources used by the current process, and removes
 *  it from any queues (IPC, authorization, scheduling, IRQ hooks...)
 *  where it might still be referenced. Any authorization that mentioned
 *  this process is automatically released.
 *  In case of success, \ref pnc_exit does not return.
 *  It might return without changing anything if the configuration prevents
 *  calling it (which is not advised).
 *
 * @param code      Exit value of the process, ignored at the moment.
 * @return
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the BREAK kernel call.
 * @uses SENDREC EXIT
 */
int pnc_exit(uint32_t code);

/**
 * @brief Request for more heap memory from the kernel.
 *
 * @note It is not advised to call it directly, one should rely on the
 *  \ref malloc / \ref free interface instead.
 *
 *  The purpose of this function is to allocate more memory to the current
 *  process by extending its heap to the limit \p vaddr.
 *
 *  The memory allocated through \ref pnc_brk is the sole property of
 *  the current process and cannot be shared implicitly with
 *  another process. The new limit of the heap, also known as
 *  \a break value, is returned in \p new_vaddr.
 *
 * @param vaddr     New address for the end of the heap.
 *                    At the moment, the data segment can only grow, and
 *                    can never shrink. Also, there is an upper hard limit
 *                    which prevents this address from increasing above
 *                    `0x20000000` (about 500Mib).
 * @param new_vaddr Pointer to a buffer which will be filled on success
 *                    with the new \a break value.
 *
 * @return
 * - `EDOM`
 *      If the required limit is below the current data region.
 * - `ENOSYS`
 *      If the required limit is below the current limit, i.e. the
 *      reduction of the data region is not yet supported.
 * - `E2BIG`
 *      If the required limit would exceed the caller's memory quota.
 * - `ENOMEM`
 *      If there is not enough physical memory or not enough
 *      virtual space to extend the caller's data region as required.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the BREAK kernel call.
 * - `OK` on success.
 * @uses BREAK
 */
int pnc_brk(const void *vaddr, void **new_vaddr);

/**
 * @brief Map external resources in the current process' virtual address space.
 *
 *  \ref pnc_iomap is the mean for an application to ask the kernel for access
 *  to \a external resources, like \a{memory mapped} devices, or
 *  \a{non-secure memory} managed by Linux.
 *  This enables an application to gain control over a peripheral, or
 *  exchange information with an entity in the non-secure world.
 *
 *  @warning
 *      Be reminded that such buffer is freely accessible from the non-secure
 *      side. Any data, in particular secret or private data, that are copied
 *      in these buffers could be accessible to unauthorized third-parties in
 *      the non-secure world.
 *
 *  The physical layout of each platform is declared in the platform \a *.dtsi
 *  FDT file; in the \a rees and \a devices sub-sections.
 *  Please refer to the ProvenCore Configuration Reference Manual for
 *  more details about the contents of this file. The main goal of
 *  \ref pnc_iomap is to request the mapping of a sub-region of one of the
 *  \a device and \a ree entries of this file.
 *
 *  \ref pnc_brk requests have alignment and size constraints:
 *  - For \a device requests, \p paddr must be 4Kib aligned, and \p length must be
 *      a multiple of 4Kib.
 *  - For \a ree requests, \p paddr must be 2Mib aligned,
 *      and \p length must be a multiple of 2Mib.
 *
 *  @warning
 *      By design, regions requested by \ref pnc_iomap are mapped
 *      \a consecutively and \a contiguously in the process external region
 *      starting at \a 0x30000000. For \ref pnc_iomap to succeed the current
 *      index in the external region must respect the alignment constraint
 *      (4Kib, resp. 2Mib) for the \a device (resp. \ree) being mapped.
 *      It is advised to map the necessary \a ree regions before devices
 *      to make sure the alignment remains valid throughout.
 *
 * @param device_id     Identifier which uniquely identifies a
 *                        \a device or \a ree region on the board.
 *                        Correct values are listed in the \a device.h file.
 * @param paddr         Physical address of the beginning of the region the
 *                        application wants to map. Must be inside the
 *                        memory description of the requested device.
 * @param length        Length of the region the application wants
 *                        to map. The whole region described by \p paddr
 *                        and \p length must be inside the memory description
 *                        of the requested device.  If \p length is set to
 *                        0, \p paddr is ignored and the whole
 *                        device is mapped.
 * @param vaddr         Pointer to a buffer filled on success with the virtual
 *                        address where the process can access the device
 *                        memory.
 * @return
 * - `EPERM`
 *      If the current process is not allowed to access this device.
 * - `EINVAL`
 *      If the requested device does not exist, if the required range
 *      description is overflowing, or if the required range
 *      is not inside the requested device layout.
 * - `EDOM`
 *      If the requested physical address is not correctly aligned, or if
 *      the requested length does not fulfill the device constraints.
 * - `ENOMEM`
 *      If there is not enough physical memory or space in the
 *      current process' address space to map the required region.
 * - `EFAULT`
 *      If the virtual buffer is not correctly aligned.
 *      This is due to a current limitation of the kernel (see related note).
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the IOMAP kernel call.
 * - `OK` on success.
 * @uses IOMAP
 */
int pnc_iomap(unsigned int device_id, paddr_t paddr, size_t length,
              void **vaddr);

/**
 * @brief Unmap external resources in the current process' virtual
 *  address space.
 *
 *  \ref pnc_iounmap is the mean for an application to ask the kernel to remove
 *  the bindings to \a external resources that were installed using
 *  \ref pnc_iomap.
 *
 *  Please note that it is possible to use \ref pnc_iounmap to remove the
 *  mapping of a \a physical buffer allocated at boot time by the kernel.
 *  However, once such a buffer is unmapped, there is no way to regain
 *  access to it. We do not advise to perform such an operation.
 *
 * @param vaddr         Virtual address of the region to unmap.
 *                        This address has been returned by the call to
 *                        \ref pnc_iomap.
 * @return
 * - `ENOENT`
 *      If the requested region does not match any external region in the
 *      current process memory map.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the IOUNMAP kernel call.
 * - `OK` on success.
 * @uses IOUNMAP
 */
int pnc_iounmap(void *vaddr);

/**
 * @brief Low-level function for configuring hardware interrupt hooks.
 *  See \ref pnc_irqsetpolicy, \ref pnc_irqrmpolicy, \ref pnc_irqstatus,
 *  \ref pnc_irqenable, \ref pnc_irqdisable, \ref pnc_irqrelease.
 * @uses IRQCTL
 */
int pnc_irqctl(unsigned int request, unsigned int irq, unsigned int policy,
               unsigned int id, unsigned int status, unsigned int *hook_id);

/**
 * @brief Configure a new interrupt hook.
 *
 *  \ref pnc_irqsetpolicy requests an handle to the IRQ line \p irq.
 *  The current process will receive interrupt notifications from \ref HARDWARE
 *  with the bit \p notify_id set in the notification mask.
 *
 *  The interrupt hook can be in multiple states:
 *      - \ref IRQ_ENABLED the interrupt is enabled and the process will
 *          receive notifications for the IRQ \p irq.
 *      - \ref IRQ_DISABLED the interrupt is disable and the \a current process
 *          will not receive notifications for the IRQ \p irq. Other processes
 *          which requests the same interrupt can still be notified.
 *      - \ref IRQ_RELEASED for platform with TrustZone implemented,
 *          the process signals that the interrupt can be released to the
 *          non-secure world, whereas the interrupt is strictly controlled by
 *          ProvenCore otherwise. The interrupt is \a actually released when
 *          all processes who created hooks on the IRQ \p irq configured
 *          the interrupt as \ref IRQ_RELEASED.
 *
 *  Depending on the policy, the interrupt will be enabled or disabled
 *  before a notification is sent to the process:
 *      - \ref IRQ_REENABLE_YES the interrupt will be in state \ref IRQ_ENABLED
 *          on reception on the notification.
 *      - \ref IRQ_REENABLE_NO the interrupt will be in state \ref IRQ_DISABLED
 *          on reception on the notification, and the process will have to
 *          re-enable the interrupt with \ref pnc_irqenable once it has finished
 *          processing the interrupt.
 *  This distinction is particularly important when dealing with level-sensitive
 *  interrupts (as opposed to edge-triggered). As the interrupt remains pending
 *  until the source of the interrupt has been cleared (e.g. by clearing an
 *  \a INTR in a device control register), the application will be immediately
 *  preempted if the policy is \ref IRQ_REENABLE_YES.
 *
 * @param irq           The identifier of the line to which the hook will
 *                        be registered. This value is provided by the
 *                        documentation of your hardware, and is usually
 *                        defined in the generated \a device.h file.
 * @param policy        Interrupt hook configuration. Must be one of
 *                        \ref IRQ_REENABLE_YES, \ref IRQ_REENABLE_NO
 * @param notify_id     Each IRQ line needs to be setup with a unique
 *                        \a notification identifier, whose value is
 *                        restricted to the range [0, 31]. These \a ids are
 *                        used by the kernel when sending the bitmap
 *                        of pending IRQs during the reception
 *                        of a notification.
 * @param status        Initial state of the interrupt hook. Must be one of
 *                        \ref IRQ_ENABLED, \ref IRQ_DISABLED,
 *                        \ref IRQ_RELEASED.
 * @param hook_id       Pointer to a buffer which will be filled on success
 *                        with the handle to the created interrupt hook.
 * @return
 * - `EINVAL`
 *      If the given IRQ handle is not a valid identifier, if the requested
 *      policy or status is not supported.
 * - `EPERM`
 *      If the current process is not a privileged process, or if the current
 *      process is not allowed to configure the requested IRQ (meaning the
 *      current process is not granted control over the device which
 *      generates this interrupt).
 * - `EBUSY`
 *      If the required identifier \p notify_id is already in use by the
 *      current process.
 * - `ENOSPC`
 *      If there were not enough free hooks for this process
 *      to be able to register a new one, or if the required IRQ line was
 *      already full.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the IRQCTL kernel call.
 * - `OK` on success.
 * @uses IRQCTL
 */
static inline int pnc_irqsetpolicy(unsigned int irq, unsigned int policy,
                                   unsigned int notify_id, unsigned int status,
                                   unsigned int *hook_id)
{
    return pnc_irqctl(IRQ_SETPOLICY, irq, policy, notify_id, status, hook_id);
}

/**
 * @brief Remove an existing interrupt hook.
 *
 *  Delete an existing hook, identified by an IRQ handle \p hook_id. The
 *  interrupt hook is automatically moved to the state \ref IRQ_RELEASED.
 *
 * @param hook_id       Handle to the interrupt hook to remove.
 * @return
 * - `EPERM`
 *      If the current process is not a privileged process, or if the current
 *      process is trying to access a hook of another process.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the IRQCTL kernel call.
 * - `OK` on success.
 * @uses IRQCTL
 */
static inline int pnc_irqrmpolicy(unsigned int hook_id)
{
    return pnc_irqctl(IRQ_RMPOLICY, 0, 0, hook_id, 0, NULL);
}

/**
 * @brief Update the configuration of an existing IRQ hook.
 *
 * @param hook_id       Handle to the interrupt hook to update.
 * @param status        Select the new configuration for the interrupt hook
 *                        \p hook_id. Must be one of \ref IRQ_ENABLED,
 *                        \ref IRQ_DISABLED, \ref IRQ_RELEASED. See
 *                        \ref pnc_irqsetpolicy for more information.
 * @return
 * - `EINVAL`
 *      If the requested status is not supported.
 * - `EPERM`
 *      If the current process is not a privileged process, or if the current
 *      process is trying to access a hook of another process.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the IRQCTL kernel call.
 * - `OK` on success.
 * @uses IRQCTL
 */
static inline int pnc_irqstatus(unsigned int hook_id, unsigned int status)
{
    return pnc_irqctl(IRQ_STATUS, 0, 0, hook_id, status, NULL);
}

/**
 * @brief Reconfigure an existing IRQ hook as \ref IRQ_ENABLED.
 *
 * @param hook_id       Handle to the interrupt hook to update.
 * @return
 * - `EPERM`
 *      If the current process is not a privileged process, or if the current
 *      process is trying to access a hook of another process.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the IRQCTL kernel call.
 * - `OK` on success.
 * @uses IRQCTL
 */
static inline int pnc_irqdisable(unsigned int hook_id)
{
    return pnc_irqctl(IRQ_STATUS, 0, 0, hook_id, IRQ_DISABLED, NULL);
}

/**
 * @brief Reconfigure an existing IRQ hook as \ref IRQ_ENABLED.
 *
 * @param hook_id       Handle to the interrupt hook to update.
 * @return
 * - `EPERM`
 *      If the current process is not a privileged process, or if the current
 *      process is trying to access a hook of another process.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the IRQCTL kernel call.
 * - `OK` on success.
 * @uses IRQCTL
 */
static inline int pnc_irqenable(unsigned int hook_id)
{
    return pnc_irqctl(IRQ_STATUS, 0, 0, hook_id, IRQ_ENABLED, NULL);
}

/**
 * @brief Reconfigure an existing IRQ hook as \ref IRQ_RELEASED.
 *
 * @param hook_id       Handle to the interrupt hook to update.
 * @return
 * - `EPERM`
 *      If the current process is not a privileged process, or if the current
 *      process is trying to access a hook of another process.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the IRQCTL kernel call.
 * - `OK` on success.
 * @uses IRQCTL
 */
static inline int pnc_irqrelease(unsigned int hook_id)
{
    return pnc_irqctl(IRQ_STATUS, 0, 0, hook_id, IRQ_RELEASED, NULL);
}

/**
 * @brief Configure the security of master and slave peripherals.
 *
 *  - For a slave device, the application can control which World
 *    (Normal/Secure) can access the peripheral using the memory-mapped
 *    registers. It can also control which privilege level (kernel/userland)
 *    can access the registers, and the associated rights
 *    (read-only/read-write) for the accesses.
 *  - For a master, the application can control if the peripheral is
 *    allowed to read/write to secure memory or not.
 *  - This call is reserved to privileged process only.
 *
 * @param device_id     Identifier which uniquely identifies a device on the
 *                        board. Correct values are listed in the generated
 *                        `device.h` header file.
 * @param request       Requested action on the device security configuration.
 *                        This parameter is platform dependent. All platform
 *                        must at least support the following values:
 *                        \ref PLATCTL_MASTER_GET, \ref PLATCTL_MASTER_SET,
 *                        \ref PLATCTL_SLAVE_GET, \ref PLATCTL_SLAVE_SET.
 * @param payload       Pointer to a message buffer that used to
 *                        pass request-specific parameters and retrieve
 *                        request-specific outputs. This behavior is different
 *                        from other kernel calls where the message is hidden
 *                        in the library because this call is platform
 *                        dependent, and the layout of the message cannot be
 *                        fixed like any other calls. Please refer to the
 *                        `platctl.h` header specific to your platform to
 *                        have a description of the message layout for a
 *                        particular specific request.
 * @return
 * - `EPERM`
 *      If the calling process is not a \a privileged process.
 * - `EINVAL`
 *      If the requested device identifier is not valid, or if the requested
 *      operation is not supported, or if a platform dependent error
 *      occurred.
 * - `ENOSYS`
 *      If the current platform does support the configuration of
 *      device security.
 * - `ELOCKED`
 *      If a modification on a locked configuration is attempted.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the PLATCTL kernel call.
 * - `OK` on success.
 * @uses PLATCTL
 */
int pnc_platctl(unsigned int device_id, unsigned int request, message *payload);

/**
 * @brief Perform a remote function call to the secure monitor, based on
 *  the `params->a0` value and using the SMC32 calling convention.
 *
 *  This method is specific to ARM based architectures. It is optionally
 *  implemented, and will always return `ENOSYS` if the current platform
 *  disables the use of this syscall.
 *
 *  Following ARM's SMC32 convention for Monitor calls, the command is encoded
 *  in `r0`, inputs parameters are stored in the fields `r1` to `r7` and
 *  output values are stored in the fields `r0` to `r7`.
 *
 * @param params        Pointer to a structure storing the eight \a uint32_t
 *                        input values of the call, and updated on success
 *                        with the four return values.
 * @return
 * - `ENOSYS`
 *      If the current architecture does not support this monitor call.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the SMC kernel call.
 * - `EINVAL`
 *      On Aarch64, if the calling convention used is SMC64 (detected based on
 *      the value of `r0`)
 * - `OK` on success.
 * @uses SMC
 */
int pnc_smc(smc32_params_t *params);

/**
 * @brief Perform a remote function call to the secure monitor, based on
 *  the `params->a0` value and using the SMC64 calling convention.
 *
 *  This method is specific to the armv8 architecture. It is optionally
 *  implemented, and will always return `ENOSYS` if the current platform
 *  disables the use of this syscall.
 *
 *  Following ARM's SMC64 convention for Monitor calls, the command is encoded
 *  in `w0`, inputs parameters are stored in the fields `x1` to `x17` and
 *  output values are stored in the fields `x0` to `x17`.
 *
 * @param params        Pointer to a structure storing the eighteen \a uint64_t
 *                        input values of the call, and updated on success
 *                        with the return values.
 * @return
 * - `ENOSYS`
 *      If the current architecture does not support this monitor call.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the SMC kernel call.
 * - `EINVAL`
 *      On Aarch64, if the calling convention used is SMC32 (detected based on
 *      the value of `x0`)
 * - `OK` on success.
 * @uses SMC
 */
int pnc_smc64(smc64_params_t *params);

/**
 * @brief Perform a remote function call to the M-mode firmware, based on
 *  the `params->a7` and `params->a6` values and using the RISC-V Supervisor
 *  Binary Interface calling convention.
 *
 *  This method is specific to the riscv64 architecture. It is optionally
 *  implemented, and will always return `ENOSYS` if the current platform
 *  disables the use of this syscall.
 *
 *  Following the SBI specification, input parameters are stored in the fields
 *  `a0` to `a7` and output values are stored in the fields `a0` and `a1`.
 *
 * @param params        Pointer to a structure storing the eight \a uint64_t
 *                        input values of the call, and updated on success
 *                        with the return values.
 * @return
 * - `ENOSYS`
 *      If the current architecture does not support this monitor call.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the SMC kernel call.
 * - `OK` on success.
 * @uses SMC
 */
int pnc_mcall(mcall_params_t *params);

/**
 * @brief Perform a remote function call to the hypervisor, based on
 *  the `params->a0` value and using the HVC32 calling convention.
 *
 *  This method is specific to ARM based architectures. It is optionally
 *  implemented, and will always return `ENOSYS` if the current platform
 *  disables the use of this syscall.
 *
 *  Following ARM's HVC32 convention for Hypervisor calls, the command is
 *  encoded in `r0`, inputs parameters are stored in the fields `r1` to `r7`
 *  and output values are stored in the fields `r0` to `r7`.
 *
 * @param params        Pointer to a structure storing the eight \a uint32_t
 *                        input values of the call, and updated on success
 *                        with the eight return values.
 * @return
 * - `ENOSYS`
 *      If the current architecture does not support hypervisor calls.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the HVC kernel call.
 * - `EINVAL`
 *      On Aarch64, if the calling convention used is HVC64 (detected based on
 *      the value of `r0`)
 * - `OK` on success.
 */
int pnc_hvc(hvc32_params_t *params);

/**
 * @brief Perform a remote function call to the hypervisor, based on
 *  the `params->a0` value and using the HVC64 calling convention.
 *
 *  This method is specific to the armv8 architecture. It is optionally
 *  implemented, and will always return `ENOSYS` if the current platform
 *  disables the use of this syscall.
 *
 *  Following ARM's HVC64 convention for Hypervisor calls, the command is
 *  encoded in `w0`, inputs parameters are stored in the fields `x1` to `x17`
 *  and output values are stored in the fields `x0` to `x17`.
 *
 * @param params        Pointer to a structure storing the eighteen \a uint64_t
 *                        input values of the call, and updated on success
 *                        with the return values.
 * @return
 * - `ENOSYS`
 *      If the current architecture does not support this monitor call.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the SMC kernel call.
 * - `EINVAL`
 *      On Aarch64, if the calling convention used is HVC32 (detected based on
 *      the value of `x0`)
 * - `OK` on success.
 */
int pnc_hvc64(hvc64_params_t *params);

/**
 * @brief Configure a memory authorization.
 *
 *  To be able to copy data between two processes, each endpoint
 *  must have set up sufficient authorizations. The kernel implements a strong
 *  access control policy, which makes it possible to guarantee a strong formal
 *  isolation property between user processes.
 *
 *  \ref pnc_setauth sets up a new authorization which grants the process
 *  \p grantee access to the current process' data with rights specified by
 *  \p mode. The authorization is effective in the address space region between
 *  [\p vaddr, \p vaddr + \p length ).
 *  The process \p effector is the one that is allowed to trigger the
 *  action of moving data between the current process and \p grantee.
 *  The process \p revoker is the process that can remove this authorization.
 *
 *  On success, \ref pnc_setauth writes to \p auth_handle a valid handle
 *  which can be used in further calls to \ref pnc_vircopy, \ref pnc_revoke
 *  \ref pnc_change_grantee, \ref pnc_change_effector, \ref pnc_change_revoker
 *  to reference the created authorization.
 *
 * @param mode
 *  Defined authorization modes are:
 *      - \ref READ_ACCESS
 *      - \ref WRITE_ACCESS
 *      - \ref RW_ACCESS
 *      - \ref NO_ACCESS
 *  Note that \ref NO_ACCESS cannot be used in this context.
 * @param grantee       Process to which (resp. from which) a copy
 *                        is being authorized in read mode (resp. write mode).
 *                        It cannot be the current process, as copying to
 *                        itself is forbidden. (see \ref pnc_change_grantee).
 * @param vaddr         Base of the range of addresses in the current process'
 *                        address space that are impacted by the new
 *                        authorization.
 * @param length        Size of the range of addresses that are impacted by
 *                        the authorization.
 * @param effector      Process which is allowed to perform the transfer of
 *                        memory between the current process and \p grantee
 *                        (see \ref pnc_vircopy, \ref pnc_change_effector).
 * @param revoker       Process which is allowed to remove this authorization
 *                        from the current process' active authorizations
 *                        (see \ref pnc_revoke, \ref pnc_change_revoker).
 * @param auth_handle   Pointer to a buffer which will be filled on success
 *                        with a valid authorization handle.
 * @return
 * - `EDOM`
 *      If the range of addresses described overflows, or the length is not
 *      strictly positive.
 * - `EINVAL`
 *      If the required permission is set to \ref NO_ACCESS, or
 *      if one of \p grantee, \p revoker or \p effector is not an active
 *      process, or if \p grantee is the current process.
 * - `EFAULT`
 *      If the described range of addresses is not correctly mapped in
 *      the data region of the current process.
 * - `ENOMEM`
 *      If the current process has reached its maximum number of active
 *      memory permissions.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the SETAUTH kernel call.
 * @uses SETAUTH
 */
__access_none(3)
int pnc_setauth(int mode, s_pid_t grantee, void const *vaddr,
                size_t length, s_pid_t effector, s_pid_t revoker,
                unsigned int *auth_handle);

/**
 * @brief Copy memory from a process' virtual address space to another process'
 *  virtual address space.
 *
 *  The copy is valid only when matching authorizations have been
 *  set up by both endpoints. See \ref pnc_setauth for more information
 *  about authorizations. In particular, copying from a process to itself
 *  will always fail, because it is impossible to create an authorization
 *  from a process to itself.
 *
 * @param src           Source process of the memory copy.
 * @param src_vaddr     Virtual address in \p src to copy from.
 * @param dst           Destination process of the memory copy.
 * @param dst_vaddr     Virtual address in \p dst to copy to.
 * @param length        Number of bytes to copy from \p src to \p dst.
 * @return
 * - `EDOM`
 *      If the number of bytes to copy goes beyond the range of
 *      representable addresses in either \p src or \p dst.
 * - `EINVAL`
 *      If src and/or dst are not active processes.
 * - `EPERM`
 *      If the current memory authorizations do not allow the
 *      requested copy.
 * - `EFAULT`
 *      If the source (resp. destination) range of addresses is not
 *      correctly mapped in a data region of the \p src process
 *      (resp. \p dst process).
 * - `ENOMEM`
 *      If performing the copy required allocating some physical pages
 *      and the system ran out of memory doing so (this condition can be
 *      triggered when the destination range is shared *via* copy-on-right with
 *      a third process).
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the COPY kernel call.
 * - `OK` on success.
 * @uses VIRCOPY
 */
int pnc_vircopy(s_pid_t src, void const *src_vaddr, s_pid_t dst,
                void *dst_vaddr, size_t length);

/**
 * @brief Remove an authorization set using \ref pnc_setauth.
 *
 *  It removes the authorization identified by the handle \p auth_handle,
 *  in the process \p owner. The current process must have been configured
 *  as the current revoker of the authorization, either directly in
 *  \ref pnc_setauth or later by a call to \ref pnc_change_revoker.
 *
 * @param owner         Identity of the process which created the authorization
 *                        we want to remove.
 * @param auth_handle   Handle returned by \ref pnc_setauth of the authorization
 *                        we want to remove.
 * @return
 * - `EINVAL`
 *      If \p owner is not an active process or if \p auth_handle is not
 *      a valid handle on an authorization set by \p owner.
 * - `EPERM`
 *      If the current process is not the designated revoker for
 *      this authorization.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the REVOKE kernel call.
 * - `OK` on success.
 * @uses REVOKE
 */
int pnc_revoke(s_pid_t owner, unsigned int auth_handle);

/**
 * @brief Change the process that is allowed to access the data subject to
 *  the authorization \p auth_handle in process \p owner.
 *
 *  If the current process is the current grantee of this authorization but
 *  is not the final endpoint of a transaction, it can rely on this call
 *  to give up its right to the authorization, and give them to another process.
 *  Processes other than the current grantee of the authorization are not
 *  allowed it to perform this change.
 *
 *  Note that it is forbidden to transfer an authorization to the process
 *  identified by \p owner to prevent that process to use \p pnc_copy to copy
 *  data to itself.

 * @param owner         Identity of the process which holds the
 *                        address space targeted by the authorization.
 * @param auth_handle   Handle of the authorization we want to transfer.
 * @param new_grantee   Identity of the new process which will
 *                        be granted the right to access the \p owner's
 *                        address space as defined by the authorization
 *                        \p auth_handle.
 * @return
 * - `EINVAL`
 *      If \p owner or \p new_grantee are not active processes,
 *      or if \p auth_handle is not a valid handle on an authorization set by
 *      \p owner, or if \p new_grantee is \p owner.
 * - `EPERM`
 *      If the current process is not the designated grantee of
 *      this authorization.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the CHANGE_GRANTEE kernel call.
 * - `OK` on success.
 * @uses CHANGE_TARGET
 */
int pnc_change_grantee(s_pid_t owner, unsigned int auth_handle,
                       s_pid_t new_grantee);

/**
 * @brief Transfer to a new process the right to execute a successful copy
 *  between two endpoints.
 *
 *  If the current process is not the relevant process to perform
 *  a copy, it can rely on this call to give up its right and give them
 *  to another process.
 *
 *  This is only allowed if the current process is the configured effector
 *  for this authorization, as set initially by \ref pnc_setauth
 *  or by later calls to \ref pnc_change_effector.
 *
 * @param owner         Identity of the process which holds the
 *                        authorization whose effector right we want to transfer.
 * @param auth_handle   Handle of the authorization whose effector right we
 *                        want to transfer. It is the value returned by
 *                        \ref pnc_setauth.
 * @param new_effector  Identity of the new process which will
 *                        be granted the right to perform the copy.
 * @return
 * - `EINVAL`
 *      If \p owner or \p new_effector are not active processes,
 *      or if \p auth_handle is not a valid handle on an authorization set by
 *      \p owner.
 * - `EPERM`
 *      If the current process is not the designated effector of
 *      this authorization.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the CHANGE_EFFECTOR kernel call.
 * - `OK` on success.
 * @uses CHANGE_EFFECTOR
 */
int pnc_change_effector(s_pid_t owner, unsigned int auth_handle,
                        s_pid_t new_effector);

/**
 * @brief Transfer the right to revoke an authorization to a new process.
 *
 *  If the current process is not the relevant process to clean up an
 *  authorization, it can rely on this call to give up its right to revoke,
 *  and give it to another process.
 *
 *  This is only allowed if the current process is the configured revoker
 *  for this authorization, as set initially by \ref pnc_setauth
 *  or by later calls to \ref pnc_change_revoker.
 *
 * @param owner         Identity of the process which holds the
 *                        authorization whose revoker right we want to transfer.
 * @param auth_handle   Handle of the authorization whose revoker right we
 *                        want to transfer. It is the value returned by
 *                        \ref pnc_setauth.
 * @param new_revoker   Identity of the new process which will
 *                        be granted the right to revoke the authorization.
 * @return
 * - `EINVAL`
 *      If \p owner or \p new_revoker are not active processes,
 *      or if \p auth_handle is not a valid handle on an authorization set by
 *      \p owner.
 * - `EPERM`
 *      If the current process is not the designated revoker of
 *      this authorization.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the CHANGE_REVOKER kernel call.
 * - `OK` on success.
 * @uses CHANGE_REVOKER
 */
int pnc_change_revoker(s_pid_t owner, unsigned int auth_handle,
                       s_pid_t new_revoker);

/**
 * @brief Create a region of shared memory with read and  write access to
 *  the current process.
 *
 *  On success, it overwrites \p id with a fresh handle to a new shared region,
 *  and \p addr with the virtual address of the beginning of the new region.
 *  The new handle should be explicitly advertised to any other party involved
 *  in the sharing, for example by performing a process-to-process copy of
 *  \p id, or sending it by the means of IPCs. Any shared memory buffer
 *  allocation is subject to quota restriction, like \ref pnc_brk.
 *
 * @param nr_pages  Size of the memory region to create, as a
 *                    number of 4Kib pages.
 * @param id        Pointer to a buffer which will hold a handle
 *                    to the memory region.
 * @param vaddr     Pointer to a buffer which will hold the virtual address
 *                    at which the memory region has been mapped.
 * @return
 * - `EINVAL`
 *      If the number of pages is zero.
 * - `ENOSPC`
 *      If there is no available region descriptor to
 *      map this new shared memory, or if the current process
 *      is already registered to its maximum number of shared
 *      memories, or if there is not sufficient space in the
 *      current process' virtual address space to map this
 *      new region.
 * - `E2BIG`
 *      If the additional allocated memory would exceed the
 *      quota restriction.
 * - `ENOMEM`
 *      If there are not enough physical pages available
 *      to allocate and map a memory region of the required size.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the SHM_ALLOC kernel call.
 * - `OK` on success.
 * @uses SHM_ALLOC
 */
int pnc_shm_alloc(unsigned int nr_pages, unsigned int *id, void **vaddr);

/**
 * @brief Create the mapping to access the shared region identified by the
 *  handle \p id.
 *
 *  On success, a region of *\p nr_pages 4Kib pages will be mapped starting
 *  at some address written to *\p vaddr, with _no rights at all_.
 *  At this point, access to this region will trigger a fault which will
 *  terminate the process. Registration to a shared memory buffer is considered
 *  to be a memory allocation, and is therefore subject to quota restriction,
 *  like \ref pnc_brk.
 *
 * @param id        Handle of the shm region to register to. It is
 *                    the value returned by \ref pnc_shm_alloc.
 * @param nr_pages  Pointer to a buffer which will be filled with the size
 *                    of the shared region, as 4Kib pages.
 * @param vaddr     Pointer to a buffer which will be filled with the virtual
 *                    address of the shared region, in the current process
 *                    address' space.
 * @return
 * - `EINVAL`
 *      If the given shared memory ID does not correspond
 *      to an existing active shared memory.
 * - `ENOSPC`
 *      If there is no available region descriptor to map
 *      this new shared memory, or if the current process is
 *      already registered to its maximum number of shared
 *      memories, or if there is not sufficient space in the
 *      current process' virtual address space to map this
 *      new region.
 * - `E2BIG`
 *      If the additional allocated memory would exceed the
 *      quota restriction.
 * - `ENOMEM`
 *      If there are not enough physical pages available
 *      to create the bindings to map this shared memory region.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the SHM_REGISTER kernel call.
 * - `OK` on success.
 * @uses SHM_REGISTER
 */
int pnc_shm_register(unsigned int id, unsigned int *nr_pages, void **vaddr);

/**
 * @brief Release a claim to a shared region identified by the handle \p id.
 *
 *  All further accesses to the region will trigger a fault, and no transfer
 *  to the current process can be performed anymore. If all claims are
 *  lifted, kernel resources are automatically released. Be advised that if
 *  the current owner of the handle unregistered, no more transfers could
 *  be performed, and every other process should unregister too.
 *
 * @param id        Identifier of the shared memory region we want to
 *                    unregister from.
 * @return
 * - `EINVAL`
 *      If the given shared memory ID does not correspond
 *      to an existing shared memory that the current process is
 *      registered to.
 * - `EBUSY`
 *      If there are active memory authorizations in the
 *      specified shared memory zone.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the SHM_UNREGISTER kernel call.
 * - `OK` on success.
 * @uses SHM_UNREGISTER
 */
int pnc_shm_unregister(unsigned int id);

/**
 * @brief Give up a current focus on some shared memory region, and transfer
 *  it to another process.
 *
 *  If the current process is the current owner of the handle \p id, it will
 *  transfer the control of the region to process \p target, and lose all its
 *  rights to access the shared buffer. The region is still alive, but
 *  mapped without any access (neither read, nor write access) in the
 *  current process' address space. The target, however, now owns full
 *  rights on the newly transferred buffer.
 *
 *  Note that for \ref pnc_shm_transfer to be successful, correct authorizations
 *  must have been configured: the current process has to allow reading from
 *  its shared memory buffer, and \p target has to allow writing to its
 *  shared memory buffer.
 *
 * @param id        Identifier of the shared memory region we want
 *                    to give up control and transfer it to \p target.
 * @param target    Identifier of the process that will gain
 *                    focus over the shared region.
 * @return
 * - `EINVAL`
 *      If the given process ID does not correspond to
 *      an active process, or if the given process ID corresponds
 *      to a kernel task, or if the given shared memory ID does
 *      not correspond to an existing shared memory region that
 *      the current process is registered to, or if the given
 *      process ID does not correspond to a process registered
 *      to the requested shared memory region.
 * - `EPERM`
 *      If the current process is not the current focus of
 *      that shared memory region, or if the authorization
 *      on the shared memory region is not set up correctly on
 *      both sides.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the SHM_TRANSFER kernel call.
 * - `OK` on success.
 * @uses SHM_TRANSFER
 */
int pnc_shm_transfer(unsigned int id, s_pid_t target);

/**
 * @brief Return the identity of the current process.
 * @param pid   Pointer to a buffer where to store the id of the
 *                current process.
 * @return
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the INFO kernel call
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_pid(s_pid_t *pid);

/**
 * @brief Return the domain of the process identified by \p object.
 *
 * @note If the process identified by \p object is not the current
 *  application, then the caller must be configured as an auditor.
 *
 * @param dom   Pointer to a buffer where to store the domain of the
 *                object process.
 * @return
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the INFO kernel call
 * - `EINVAL`
 *      If the specified process identifier is invalid.
 * - `EPERM`
 *      If the specified object process is not the current application,
 *      and the current application is not configured as an \e auditor.
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_domain(s_pid_t object, unsigned int *dom);

/**
 * @brief Return the amount of time, in microseconds, used so far by the
 *  process identified by \p object.
 *
 *  It does not take into account the time spent in the kernel for the
 *  execution of kernel calls.
 *
 * @note If the process identified by \p object is not the current
 *  application, then the caller must be configured as an auditor.
 *
 * @param times     Pointer to a buffer where to store the amount of time,
 *                    in microseconds, used by the object process.
 * @return
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the INFO kernel call
 * - `EINVAL`
 *      If the specified process identifier is invalid.
 * - `EPERM`
 *      If the specified object process is not the current application,
 *      and the current application is not configured as an \e auditor.
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_times(s_pid_t object, uint64_t *times);

/**
 * @brief Return the current uptime of the system, in microseconds.
 * @param uptime    Pointer to a buffer where to store the current uptime,
 *                    in microseconds.
 * @return
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the INFO kernel call
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_uptime(uint64_t *uptime);

/**
 * @brief Returns the number of 4Kib memory pages the process identified
 *  by \p object is using, along with the maximum allowed quota.
 *
 * @note If the process identified by \p object is not the current
 *  application, then the caller must be configured as an auditor.
 *
 * @param quota     Pointer to a buffer where to store the number of 4Kib
 *                    memory pages the object  process is using
 *                    for its data consumption.
 * @param max_quota Pointer to a buffer where to store the maximum number
 *                    of 4Kib memory pages available to the object process
 *                    for its data consumption.
 * @return
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the INFO kernel call.
 * - `EINVAL`
 *      If the specified process identifier is invalid.
 * - `EPERM`
 *      If the specified object process is not the current application,
 *      and the current application is not configured as an \e auditor.
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_quota(s_pid_t object,
                  unsigned int *quota, unsigned int *max_quota);

/**
 * @brief Returns the base address and size of the data region the
 *  of the process identified by the \p object process identifier.
 *
 * @note If the process identified by \p object is not the current
 *  application, then the caller must be configured as an auditor.
 *
 * @param pdata_base  Pointer to a buffer where to store the virtual
 *                    base address of the object process' data region.
 * @param pdata_size  Pointer to a buffer where to store the size
 *                    (in bytes) of the object process' data region.
 *
 * @return
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the INFO kernel call.
 * - `EINVAL`
 *      If the specified process identifier is invalid.
 * - `EPERM`
 *      If the specified object process is not the current application,
 *      and the current application is not configured as an \e auditor.
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_data(s_pid_t object,
                 void **pdata_base, unsigned int *pdata_size);

/**
 * @brief Return the kernel related metadata for any ELF application,
 *  identified by its name (the same one you would use for the
 *  \ref pnc_exec call).
 *
 *  As a result, the output message will store the maximum quota
 *  limitation (in 4Kib pages), the physical buffer information
 *  (size in 4Kib pages and type), some ELF related status information
 *  (like the ELF identity). Three macros \ref METADATA_PHYSINFO_CACHED,
 *  \ref METADATA_PHYSINFO_NRPAGES and \ref METADATA_CACHE_CRITICAL
 *  are provided to extract the relevant parts from the metadata information.
 *
 * @param elf_name  Pointer to the string describing the name of the
 *                    application to target.
 * @param length    Size of the string \p elf_name in bytes.
 * @param data_size Pointer to a buffer where to store the maximum quota
 *                    limitation, in pages.
 * @param minfo     Pointer to a buffer where to store the metadata.
 * @param elf_nr    Pointer to a buffer where to store the ELF identifier.
 * @return
 * - `EDOM`
 *      If the address range given for the ELF name is invalid.
 * - `EINVAL`
 *      If the given address range is not mapped in the caller.
 * - `ESRCH`
 *      If the given ELF name cannot be found.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the INFO kernel call.
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_metadata_by_app(char const *elf_name, size_t length,
                            uint32_t *data_size, uint32_t *minfo,
                            uint32_t *elf_nr);

/**
 * @brief Return the kernel related metadata for any application,
 *  identified by its PID (\a s_pid_t).
 *
 *  As a result, the output message will store the maximum quota
 *  limitation (in 4Kib pages), the physical buffer information
 *  (size in 4Kib pages and type), some ELF related status information
 *  (like the ELF identity). Three macros \ref METADATA_PHYSINFO_CACHED,
 *  \ref METADATA_PHYSINFO_NRPAGES and \ref METADATA_CACHE_CRITICAL
 *  are provided to extract the relevant parts from the metadata information.
 *
 * @param pid       Identifier of the process one wants metadata
 *                    information from.
 * @param data_size Pointer to a buffer where to store the maximum quota
 *                    limitation, in pages.
 * @param minfo     Pointer to a buffer where to store the metadata.
 * @param elf_nr    Pointer to a buffer where to store the ELF identifier.
 * @return
 * - `EINVAL`
 *      If the requested process identifier is invalid.
 * - `EPERM`
 *      If the requested process identifier corresponds to a kernel task.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the INFO kernel call.
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_metadata_by_pid(s_pid_t pid, uint32_t *data_size,
                            uint32_t *minfo, uint32_t *elf_nr);

/**
 * @brief Returns the identity of the current owner of a shared memory token,
 *  provided that the current process is correctly registered to the
 *  requested shared memory.
 *
 * @param id        Identifier of the shared memory whose current
 *                    owner is requested.
 * @param owner     Pointer to a buffer where to store the id of the process
 *                    which currently has focus on the requested shared memory
 *                    region.
 * @return
 * - `EINVAL`
 *      If the given shared memory region id is invalid.
 * - `EPERM`
 *      If the caller is not registered to the given shared memory region.
 * - `ESRCH`
 *      If the given shared memory region has no owner.
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the INFO kernel call.
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_shm_token_owner(unsigned int id, s_pid_t *owner);

/**
 * @brief Return information about the physical buffer requested by the
 *  calling process.
 *
 *  If it is not a privileged process, it cannot have a physical
 *  buffer and \ref EPERM will be returned.
 *
 *  Otherwise, the size (in 4Kib pages) of the physical buffer of the
 *  calling process is returned, along with the physical address of the
 *  contiguous buffer available to the process, and the virtual address
 *  where the process can access the contiguous buffer.
 *
 * @param vaddr     Pointer to a buffer where to store the user virtual base
 *                    address of the requested physical buffer.
 * @param paddr     Pointer to a buffer where to store the physical base
 *                    address of the requested physical buffer.
 * @param nr_pages  Pointer to a buffer where to store the size (in 4Kib pages)
 *                    of the requested physical buffer.
 * @return
 * - `EPERM`
 *      If the caller is not a system process.
 * - `ENOENT`
 *      If the caller is not a process which was associated
 *      with a region of contiguous physical memory at start-up.
 * - `ENOMEM`
 *      If the caller required a region of contiguous
 *      physical memory at start-up but the physical pool reserved to that
 *      end in the kernel's configuration did not suffice to grant that
 *      request.
 * - `EINVAL`
 *      If the buffer of physical memory associated with
 *      the caller is invalid or corrupted (which can happen if the
 *      buffer has already been released via \ref pnc_iounmap).
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the INFO kernel call.
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_phys_buff(void **vaddr, paddr_t *paddr, unsigned int *nr_pages);

/**
 * @brief Return the PID of a system process identified by its name in the
 * kernel's configuration
 *
 * @param sysproc_name  Pointer to the string describing the name of the
 *                      application to target.
 * @param length        Size of the string \p sysproc_name in bytes.
 * @param pid           Pointer to a buffer where to store the id of the
 *                      system process.
 * @return
 * - `EDOM`
 *      If the address range given for the system process name is invalid.
 * - `EINVAL`
 *      If the given address range is not mapped in the caller.
 * - `ESRCH`
 *      If the given system process name cannot be found.
 * - `EDSTDIED`
 *      If the given system process has terminated
 * - `ECALLDENIED`
 *      If the configuration is preventing the current
 *      process from performing the INFO kernel call.
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_sysproc_pid_by_name(char const *sysproc_name, size_t length,
                                s_pid_t *pid);

/**
 * @brief Return the capabilities of the application specified by
 *  the \p object process identifier, as described by its privilege
 *  structure.
 *
 *  As a result, three different bitmasks are returned:
 *   - `priv_flags` contains various privileged attributes which can
 *      be accessed using the flags \ref CAPABILITIES_PREEMPTIBLE,
 *      \ref CAPABILITIES_SYSPROC, \ref CAPABILITIES_DEBUGGER,
 *      \ref CAPABILITIES_DEBUGGEE and \ref CAPABILITIES_KILLABLE
 *   - `trap_mask` describes which IPC traps are allowed; the flags
 *     \ref CAPABILITIES_TRAP_SEND, \ref CAPABILITIES_TRAP_RECEIVE,
 *     \ref CAPABILITIES_TRAP_SENDREC, \ref CAPABILITIES_TRAP_NOTIFY and
 *     \ref CAPABILITIES_TRAP_SLEEP allow to test each individual kind of
 *     trap.
 *   - `call_mask` describes which kernel calls are allowed; each kernel
 *     call is denoted by the bit corresponding to its kernel call number,
 *     so for instance `IOMAP` is allowed if and only if
 *     `call_mask & (UINT64_C(1) << IOMAP)` is non-zero.
 *
 * @note If the process identified by \p object is not the current
 *  application, then the caller must be configured as an auditor.
 *
 * @param priv_flags When successful, contains a bitmask describing
 *                   some privileged attributes of the object process.
 * @param trap_mask When successful, contains a bitmask describing which
 *                  IPC traps are allowed for the object process.
 * @param call_mask When successful, contains a bitmask describing which
 *                  kernel calls are allowed for the object  process.
 * @return
 * - `EINVAL`
 *      If the specified process identifier is invalid.
 * - `EPERM`
 *      If the specified object process is not the current application,
 *      and the current application is not configured as an \e auditor.
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_capabilities(s_pid_t object,
                         uint32_t *priv_flags, uint32_t *trap_mask,
                         uint64_t *call_mask);

/**
 * @brief Return the authorized IPC traps for the \p object application
 *  with the application identified by the PID \p target.
 *
 *  As a result, the allowed IPC traps will be returned as a bitmask.
 *  The flags \ref CAPABILITIES_TRAP_SEND, \ref CAPABILITIES_TRAP_RECEIVE,
 *  \ref CAPABILITIES_TRAP_SENDREC, \ref CAPABILITIES_TRAP_NOTIFY and
 *  \ref CAPABILITIES_TRAP_SLEEP allow to test each individual kind of
 *  trap. The returned information is based on the processes' privileges
 *  and domains, i.e. static information. The current particular status
 *  of \p target, regarding deadlocks for instance, is ignored.
 *  Also note, that the status of the \ref CAPABILITIES_TRAP_SLEEP is
 *  independent of the actual target.
 *
 * @note If the process identified by \p object is not the current
 *  application, then the caller must be configured as an auditor.
 *
 * @param target    Identifier of the process one wants to communicate with.
 * @param trap_mask When successful, contains a bitmask describing what
 *                  kind of IPCs are allowed between the object process
 *                  and \p target.
 * @return
 * - `EINVAL`
 *      If the \p object or \p target process identifiers do not correspond
 *      to valid non-kernel tasks.
 * - `EPERM`
 *      If the specified object process is not the current application, and
 *      the current application is not configured as an \e auditor.
 * - `OK` on success.
 * @uses GETINFO
 */
int pnc_get_allowed_ipc(s_pid_t object,
                        s_pid_t target, uint32_t *trap_mask);

int _pnc_check_memory_rights(unsigned int flags, void *vaddr, size_t length);
int _pnc_dump(unsigned int arg);
int _pnc_get_free_pages(unsigned int *val);
int _pnc_load_exec(void *elf, size_t elf_len, char const *name,
                   size_t name_len);

__END_DECLS

#endif /* _LIBPNC_SYSCALL_INTERNAL_H_INCLUDED_ */
