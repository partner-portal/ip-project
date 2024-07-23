/*
 * Copyright (c) 2020-2023 Prove & Run S.A.S
 * All Rights Reserved.
 *
 * This software is the confidential and proprietary information of
 * Prove & Run S.A.S ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with Prove & Run S.A.S
 *
 * PROVE & RUN S.A.S MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. PROVE & RUN S.A.S SHALL
 * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING,
 * MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */

//! `pnc` is a crate of safe low level bindings for the ProvenCore kernel

#![no_std]
#![feature(panic_info_message, alloc_error_handler, never_type)]
#![warn(missing_docs)]

extern crate alloc;

mod allocator;
mod auth;
/// Devices
pub mod device;
pub mod error;
mod ipc;
mod irq;
pub mod libc;
pub mod libpnc;
mod panic;
/// REES
pub mod rees;
mod shm;
/// System Process ID
pub mod sys_procs_pid;

pub use alloc::format;
use alloc::vec::Vec;
use core::convert::TryInto;
use core::ops::{BitAnd, BitOr};

pub use auth::*;
use cstr_core::CString;
use error::{Error, Result};
pub use ipc::*;
use libc::{c_char, c_uint, c_void};
pub use libpnc::s_pid_t;
use libpnc::{errno, hvc32_params_t, mcall_params_t, paddr_t, smc32_params_t, smc64_params_t};
pub use shm::*;

/// Print string with newline
pub fn puts(string: &str) {
    let string = CString::new(string).expect("Couldn't create CString");
    unsafe {
        libc::puts(string.as_ptr());
    }
}

/// `no_std` alternative to std's `println!` using `puts` and a heap allocated `String`
#[macro_export]
macro_rules! println {
    ($($arg:tt)*) => ($crate::puts(&$crate::format!($($arg)*)));
}

/// Return the identity of the current process.
///
/// # Errors
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the INFO kernel call
pub fn get_pid() -> Result<s_pid_t> {
    let mut pid = 0;
    match unsafe { libpnc::pnc_get_pid(&mut pid) } {
        errno::OK => Ok(pid),
        err => Err(Error::from(err).context("get_pid failed")),
    }
}

/// Duplicate the current process, spawning a new process with the same
/// state and almost the same resources.
///
///  - The child process gets a new and unique ID.
///  - The child process is created in the same domain as the parent process.
///  - The child process automatically registers to any shared memory region
///    registered by the current process. They are all set up without any
///    access, even if the current process has read/write access to the region.
///  - The child process does not inherit any region of device memory that the current process
///    might have registered to using `iomap`.
///  - The child process does not inherit the physical buffer that the current
///    may have requested at boot time.
///  - The child process does not inherit any region shared with the non-secure
///    World that the current process might have registered to using `iomap`.
///  - The child process starts with no authorization at all, they are not
///    inherited from the current process.
///  - The current quantum of time is split between both processes.
///
/// If the process was privileged, the new process loses the
/// privileges and becomes a "user" process. As a side effect, this will also
/// clear any pending interrupt or notification for the child process:
/// as a user process sharing the same privilege with all user processes,
/// it will no longer be able to receive any notification, it will not even
/// be able to use `notify` or `irqctl`.
/// All user processes share the same privileges (which can be considered
/// as the "default" privileges of the system).
///
/// In case of success, the return value is 0 in the child and the process
/// identifier of the child in the parent.
///
/// # Errors
///
/// - `EAGAIN`
///      If there is no free slot in the kernel's process table.
/// - `ENOMEM`
///      If the system ran out of physical memory while trying to
///      create a new process.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the FORK kernel call.
pub fn s_fork() -> Result<s_pid_t> {
    let mut pid = 0;
    match unsafe { libpnc::s_fork(&mut pid) } {
        errno::OK => Ok(pid),
        err => Err(Error::from(err).context("fork failed")),
    }
}

/// A process signal to be used with kill
///
/// PnC has no support for POSIX signals so these are the equivalents of `SIGSTOP`, `SIGCONT` and
/// `SIGKILL`
pub enum Signal {
    /// Terminate the process, equivalent to `SIGKILL`
    Terminate = 0,
    /// Pause the process, equivalent to `SIGSTOP`
    Pause = 1,
    /// Resume the process, equivalent to `SIGCONT`
    Resume = 2,
}

/// Control the execution of another process.
///
///  It can be used to pause, resume or terminate the execution of
///  another process. Since there is no support for POSIX signals, it is the
///  equivalent of `kill SIGSTOP`, `kill SIGCONT` and
///  `kill SIGKILL` in most Unix-based distributions.
///
///  - It is always forbidden to use kill on kernel tasks.
///  - A privileged process can use kill on any user process
///      if they are in the same domain. If a privileged
///      process is domain 0, it can use kill on any user process.
///  - A user process can only use kill on another user
///      process, if they are in the same domain. If a user process is in
///      domain 0, it can use kill on any user process.
///  - A process cannot pause or resume itself.
///  - There is a debug mechanism allowing some privileged processes to pause
///      or resume other privileged processes, but it is not available in
///      client releases. Please contact the ProvenCore team
///      for more information.
///
/// # Arguments
///
/// * `target`        Identifier of the process we want to control.
/// * `signal`        Control operation to be performed.
///
/// # Errors
///
/// - `EINVAL`
///      If `target` does not correspond to an active process.
/// - `EBADMSG`
///      If `signal` is not valid.
/// - `EPERM`
///      If `target` is a kernel task, if the current process and
///      `target` are not in compatible domains, or if `target`
///      is a privileged process.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the KILL kernel call.
pub fn kill(target: s_pid_t, signal: Signal) -> Result<()> {
    match unsafe { libpnc::pnc_kill(target, signal as u32) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("kill failed")),
    }
}

/// Terminate the current process.
///
/// It also frees all the resources used by the current process, and removes
/// it from any queues (IPC, authorization, scheduling, IRQ hooks...)
/// where it might still be referenced. Any authorization that mentioned
/// this process is automatically released.
/// In case of success, `exit` does not return.
/// It might return without changing anything if the configuration prevents
/// calling it (which is not advised).
///
/// # Arguments
///
/// * `code` Exit value of the process.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the BREAK kernel call.
pub fn exit(code: u32) -> Result<!> {
    Err(Error::from(unsafe { libpnc::pnc_exit(code) }).context("exit failed"))
}

/// Constant representing an infinite quota.
pub const EXEC_QUOTA_ALL: usize = 0xffffffff;
/// Constant representing any available quota.
pub const EXEC_QUOTA_ANY: usize = 0;
/// Constant representing the maximum quota value
pub const EXEC_QUOTA_MAX: usize = 0xefffffff;

/// Replace the code and data of the current process with the ones from a new secure executable
/// pointed to by `path`.
///
///  The initial stack frame of the new process is built on the user side
///  (in the library) before being sent to the kernel.
///
///  `exec` **can** terminate the current process on failure.
///  This will happen if the kernel cannot allocate the necessary pages
///  to setup the new ELF.
///
/// # Arguments
///
/// * `path`      Name of the new process to run. It can only be chosen
///               from the static list of embedded applications that are
///               shipped with the kernel.
/// * `argv`      This slice holds the initial arguments of the new process.
///               Used to build the initial frame of the new process.
/// * `envp`      This array holds the initial environment of the new process.
///               Used to build the initial frame of the new process.
/// * `quota`     Requested number of memory pages that will be used for quota value to limit the
///               available memory of the new process. `None` or means that no quota limit
///               is requested.
/// # Errors
///
/// - `ENOMEM`
///      If the requested stack data is too large for the system.
/// - `EINVAL`
///      If the `path`, `argv`, or `envp` point to invalid ranges of
///      addresses in the caller's address space, or if `quota` takes an invalid
///      value.
/// - `ESRCH`
///      If the requested executable name cannot be found
///      in the store of all available executables.
/// - `E2BIG`
///      If the requested quota exceeds the limitation of the system
///      configuration or the quota of the current process. This can never
///      happen if `quota` is `EXEC_QUOTA_ANY`.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the EXEC kernel call.
pub fn exec(path: &str, argv: &[&str], envp: &[&str], quota: Option<usize>) -> Result<!> {
    let path_str = CString::new(path).expect("Couldn't create CString");
    let path = path_str.as_ptr();

    let argv_strs = argv
        .iter()
        .map(|&s| CString::new(s).expect("Couldn't create CString"))
        .collect::<Vec<CString>>();
    let mut argv = argv_strs
        .iter()
        .map(|s| s.as_ptr())
        .collect::<Vec<*const c_char>>();
    argv.push(core::ptr::null());
    let argv = argv.as_mut_ptr();

    let envp_strs = envp
        .iter()
        .map(|&s| CString::new(s).expect("Couldn't create CString"))
        .collect::<Vec<CString>>();
    let mut envp = envp_strs
        .iter()
        .map(|s| s.as_ptr())
        .collect::<Vec<*const c_char>>();
    envp.push(core::ptr::null());
    let envp = envp.as_mut_ptr();

    let quota = quota.unwrap_or(EXEC_QUOTA_ANY);

    Err(
        Error::from(unsafe { libpnc::pnc_exec(path, argv, envp, quota.try_into().unwrap()) })
            .context("exec failed"),
    )
}

/// Copy memory from a process' virtual address space to another process'
///  virtual address space.
///
///  The copy is valid only when matching authorizations have been
///  set up by both endpoints. See `setauth` for more information
///  about authorizations. In particular, copying from a process to itself
///  will always fail, because it is impossible to create an authorization
///  from a process to itself.
///
///  # Parameters
///
/// * `src`           Source process of the memory copy.
/// * `src_vaddr`     Virtual address in the source to copy from.
/// * `dst`           Destination process of the memory copy.
/// * `dst_vaddr`     Virtual address in the destination to copy to.
/// * `size`          Number of bytes to copy.
///
/// # Errors
///
/// - `EDOM`
///      If the number of bytes to copy goes beyond the range of
///      representable addresses in either `src` or `dst`.
/// - `EINVAL`
///      If `src` and/or `dst` are not active processes.
/// - `EPERM`
///      If the current memory authorizations do not allow the
///      requested copy.
/// - `EFAULT`
///      If the source (resp. destination) range of addresses is not
///      correctly mapped in a data region of the `src` process
///      (resp. `dst` process).
/// - `ENOMEM`
///      If performing the copy required allocating some physical pages
///      and the system ran out of memory doing so (this condition can be
///      triggered when the destination range is shared *via* copy-on-right with
///      a third process).
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the COPY kernel call.
pub fn vircopy(
    src: s_pid_t,
    src_vaddr: *const u8,
    dst: s_pid_t,
    dst_vaddr: *mut u8,
    size: usize,
) -> Result<()> {
    match unsafe {
        libpnc::pnc_vircopy(
            src,
            src_vaddr as *const c_void,
            dst,
            dst_vaddr as *mut c_void,
            size,
        )
    } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("vircopy failed")),
    }
}

///
/// Request for more heap memory from the kernel.
///
///  The purpose of this function is to allocate more memory to the current
///  process by extending its heap to the limit `vaddr`.
///
///  The memory allocated through `brk` is the sole property of
///  the current process and cannot be shared implicitly with
///  another process. The new limit of the heap, also known as
///  *break* value, is returned.
///
///  # Arguments
///
/// * `vaddr`     New address for the end of the heap.
///               At the moment, the data segment can only grow, and
///               can never shrink. Also, there is an upper hard limit
///               which prevents this address from increasing above
///               `0x20000000` (about 500Mib).
///
/// # Errors
///
/// - `EDOM`
///      If the required limit is below the current data region.
/// - `ENOSYS`
///      If the required limit is below the current limit, i.e. the
///      reduction of the data region is not yet supported.
/// - `E2BIG`
///      If the required limit would exceed the caller's memory quota.
/// - `ENOMEM`
///      If there is not enough physical memory or not enough
///      virtual space to extend the caller's data region as required.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the BREAK kernel call.

// TODO test
pub fn brk(vaddr: *const c_void) -> Result<*const c_void> {
    let mut new_vaddr = core::ptr::null_mut();
    match unsafe { libpnc::pnc_brk(vaddr as *const _, &mut new_vaddr) } {
        errno::OK => Ok(new_vaddr),
        err => Err(Error::from(err).context("brk failed")),
    }
}

/// Return the domain of the process identified by object.
///
/// If the process identified by object is not the current application, then the
/// caller must be configured as an auditor.
///
/// # Arguments
///
/// - `object`    Process identifier of the process whose information is
///               requested.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the INFO kernel call
/// - `EINVAL`
///      If the specified process identifier is invalid.
/// - `EPERM`
///      If the specified object process is not the current application,
///      and the current application is not configured as an auditor.

// TODO test
pub fn get_domain_of(object: s_pid_t) -> Result<c_uint> {
    let mut dom = 0;
    match unsafe { libpnc::pnc_get_domain(object, &mut dom) } {
        errno::OK => Ok(dom),
        err => Err(Error::from(err).context("get_domain failed")),
    }
}

/// Return the domain of the current process
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the INFO kernel call

pub fn get_domain() -> Result<c_uint> {
    get_domain_of(get_pid()?)
}

/// Return the amount of time, in microseconds, used so far by the process
/// identified by object.
///
/// It does not take into account the time spent in the kernel for the execution
/// of kernel calls.
///
/// If the process identified by object is not the current application, then the
/// caller must be configured as an auditor.
///
///  # Arguments
///
/// - `object`    Process identifier of the process whose information is
///               requested.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the INFO kernel call
/// - `EINVAL`
///      If the specified process identifier is invalid.
/// - `EPERM`
///      If the specified object process is not the current application,
///      and the current application is not configured as an auditor.

// TODO test
pub fn get_times_of(object: s_pid_t) -> Result<u64> {
    let mut times = 0;
    match unsafe { libpnc::pnc_get_times(object, &mut times) } {
        errno::OK => Ok(times),
        err => Err(Error::from(err).context("get_times failed")),
    }
}

/// Return the amount of time, in microseconds, used so far by the current
/// process.
///
/// It does not take into account the time spent in the kernel for the execution
/// of kernel calls.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the INFO kernel call

pub fn get_times() -> Result<u64> {
    get_times_of(get_pid()?)
}

/// Return the current uptime of the system, in microseconds.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the INFO kernel call

// TODO test
pub fn get_uptime() -> Result<u64> {
    let mut uptime = 0;
    match unsafe { libpnc::pnc_get_uptime(&mut uptime) } {
        errno::OK => Ok(uptime),
        err => Err(Error::from(err).context("get_uptime failed")),
    }
}

/// Returns the number of 4Kib memory pages the process identified by object is
/// using, along with the maximum allowed quota.
///
/// If the process identified by object is not the current application, then the
/// caller must be configured as an auditor.
///
/// # Arguments
///
/// - `object`    Process identifier of the process whose information is
///               requested.
/// - `quota`     Mutable reference to a buffer where to store the number of
///               4Kib memory pages the object  process is using for its data
///               consumption.
/// - `max_quota` Mutable reference to a buffer where to store the maximum
///               number of 4Kib memory pages available to the object process
///               for its data consumption.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current process from performing
///      the INFO kernel call.
/// - `EINVAL`
///      If the specified process identifier is invalid.
/// - `EPERM`
///      If the specified object process is not the current application,
///      and the current application is not configured as an auditor.

// TODO test
pub fn get_quota_of(object: s_pid_t, quota: &mut c_uint, max_quota: &mut c_uint) -> Result<()> {
    match unsafe { libpnc::pnc_get_quota(object, quota, max_quota) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("get_quota failed")),
    }
}

/// Returns the number of 4Kib memory pages the current process is using, along
/// with the maximum allowed quota.
///
/// # Arguments
///
/// - `quota`     Mutable reference to a buffer where to store the number of
///               4Kib memory pages the object  process is using for its data
///               consumption.
/// - `max_quota` Mutable reference to a buffer where to store the maximum
///               number of 4Kib memory pages available to the object process
///               for its data consumption.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current process from performing
///      the INFO kernel call.

// TODO test
pub fn get_quota(quota: &mut c_uint, max_quota: &mut c_uint) -> Result<()> {
    get_quota_of(get_pid()?, quota, max_quota)
}

/// Map external resources in the current process' virtual address space.
///
///  `iomap` is the mean for an application to ask the kernel for access
///  to *external* resources, like *memory mapped devices*, or
///  *non-secure memory* managed by Linux.
///  This enables an application to gain control over a peripheral, or
///  exchange information with an entity in the non-secure world.
///
///  > Be reminded that such buffer is freely accessible from the non-secure
///  > side. Any data, in particular secret or private data, that are copied
///  > in these buffers could be accessible to unauthorized third-parties in
///  > the non-secure world.
///
///  The physical layout of each platform is declared in the platform `*.dtsi`
///  FDT file; in the *rees* and *devices* sub-sections.
///  Please refer to the ProvenCore Configuration Reference Manual for
///  more details about the contents of this file. The main goal of
///  `iomap` is to request the mapping of a sub-region of one of the
///  *device* and *ree* entries of this file.
///
///  `brk` requests have alignment and size constraints:
///  - For *device* requests, `paddr` must be 4Kib aligned, and `length` must be
///      a multiple of 4Kib.
///  - For *ree* requests, `paddr` must be 2Mib aligned,
///      and `length` must be a multiple of 2Mib.
///
/// > By design, regions requested by `iomap` are mapped
/// > *consecutively* and *contiguously* in the process external region
/// > starting at `0x30000000`. For `iomap` to succeed the current
/// > index in the external region must respect the alignment constraint
/// > (4Kib, resp. 2Mib) for the *device* (resp. *ree*) being mapped.
/// > It is advised to map the necessary *ree* regions before devices
/// > to make sure the alignment remains valid throughout.
///
/// `iomap` returns a pointer to a buffer filled on success with the virtual address where the process
/// can access the device memory.
///
///
/// # Arguments
///
/// - `device_id`     Identifier which uniquely identifies a
///                        *device* or *ree* region on the board.
///                        Correct values are listed in the `device.h` file.
/// - `paddr`         Physical address of the beginning of the region the
///                        application wants to map. Must be inside the
///                        memory description of the requested device.
/// - `length`        Length of the region the application wants
///                        to map. The whole region described by `paddr`
///                        and `length` must be inside the memory description
///                        of the requested device.  If `length` is set to
///                        0, `paddr` is ignored and the whole
///                        device is mapped.
/// # Errors
///
/// - `EPERM`
///      If the current process is not allowed to access this device.
/// - `EINVAL`
///      If the requested device does not exist, if the required range
///      description is overflowing, or if the required range
///      is not inside the requested device layout.
/// - `EDOM`
///      If the requested physical address is not correctly aligned, or if
///      the requested length does not fulfill the device constraints.
/// - `ENOMEM`
///      If there is not enough physical memory or space in the
///      current process' address space to map the required region.
/// - `EFAULT`
///      If the virtual buffer is not correctly aligned.
///      This is due to a current limitation of the kernel (see related note).
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the IOMAP kernel call.

// TODO test
pub fn iomap(device_id: c_uint, paddr: paddr_t, length: usize) -> Result<*mut c_void> {
    let mut vaddr = core::ptr::null_mut();
    match unsafe { libpnc::pnc_iomap(device_id, paddr, length, &mut vaddr) } {
        errno::OK => Ok(vaddr),
        err => Err(Error::from(err).context("iomap failed")),
    }
}

/// Unmap external resources in the current process' virtual
///  address space.
///
///  `iounmap` is the mean for an application to ask the kernel to remove
///  the bindings to *external* resources that were installed using
///  `iomap`.
///
///  Please note that it is possible to use `iounmap` to remove the
///  mapping of a *physical* buffer allocated at boot time by the kernel.
///  However, once such a buffer is unmapped, there is no way to regain
///  access to it. We do not advise to perform such an operation.
///
/// # Arguments
///
/// - `vaddr`         Virtual address of the region to unmap.
///                        This address has been returned by the call to `iomap`.
/// # Errors
/// - `ENOENT`
///      If the requested region does not match any external region in the
///      current process memory map.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the IOUNMAP kernel call.

// TODO test
pub fn iounmap(vaddr: *mut c_void) -> Result<()> {
    match unsafe { libpnc::pnc_iounmap(vaddr as *mut _) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("iounmap failed")),
    }
}

/// Configure the security of master and slave peripherals.
///
///  - For a slave device, the application can control which World
///    (Normal/Secure) can access the peripheral using the memory-mapped
///    registers. It can also control which privilege level (kernel/userland)
///    can access the registers, and the associated rights
///    (read-only/read-write) for the accesses.
///  - For a master, the application can control if the peripheral is
///    allowed to read/write to secure memory or not.
///  - This call is reserved to privileged process only.
///
///  `platctl` returns a `Message` handle that can be used to
///                        pass request-specific parameters and retrieve
///                        request-specific outputs. This behavior is different
///                        from other kernel calls where the message is hidden
///                        in the library because this call is platform
///                        dependent, and the layout of the message cannot be
///                        fixed like any other calls. Please refer to the
///                        `platctl.h` header specific to your platform to
///                        have a description of the message layout for a
///                        particular specific request.
///
/// # Arguments
///
/// - `device_id`     Identifier which uniquely identifies a device on the
///                        board. Correct values are listed in the generated
///                        `device.h` header file.
/// - `request`       Requested action on the device security configuration.
///                        This parameter is platform dependent. All platform
///                        must at least support the following values:
///                         PLATCTL_MASTER_GET,  PLATCTL_MASTER_SET,
///                         PLATCTL_SLAVE_GET,  PLATCTL_SLAVE_SET.
///
/// # Errors
///
/// - `EPERM`
///      If the calling process is not a *privileged* process.
/// - `EINVAL`
///      If the requested device identifier is not valid, or if the requested
///      operation is not supported, or if a platform dependent error
///      occurred.
/// - `ENOSYS`
///      If the current platform does support the configuration of
///      device security.
/// - `ELOCKED`
///      If a modification on a locked configuration is attempted.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the PLATCTL kernel call.

// TODO test
pub fn platctl(device_id: c_uint, request: c_uint) -> Result<Message> {
    let mut msg = libpnc::message {
        m_source: 0,
        uptime: 0,
        m_type: 0,
        m1: 0,
        m2: 0,
        m3: 0,
        m4: 0,
        m5: 0,
        m6: 0,
    };
    match unsafe { libpnc::pnc_platctl(device_id, request, &mut msg) } {
        errno::OK => Ok(Message::from_raw(msg)),
        err => Err(Error::from(err).context("platctl failed")),
    }
}

/// Perform a remote function call to the secure monitor.
///
/// This method is specific to ARM based architectures. It is optionally
/// implemented, and will always return `ENOSYS` if the current platform
/// disables the use of this syscall.
///
/// Following ARM's convention for Monitor calls, the command is encoded
/// in `r0`, inputs parameters are stored in the fields `r1` to `r7` and
/// output values are stored in the fields `r0` to `r3`.
///
/// # Arguments
///
/// * `params` Mutable reference to a structure storing the eight `u32` input values
///            of the call, and updated on success with the four return values.
/// # Errors
///
/// - `ENOSYS`
///      If the current architecture does not support monitor calls.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the SMC kernel call.

// TODO test
pub fn smc(params: &mut smc32_params_t) -> Result<()> {
    match unsafe { libpnc::pnc_smc(params) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("smc failed")),
    }
}

/// Perform a remote function call to the secure monitor, based on the
/// `params.a0` value and using the SMC64 calling convention.
///
/// This method is specific to the armv8 architecture. It is optionally
/// implemented, and will always return `ENOSYS` if the current platform
/// disables the use of this syscall.
///
/// Following ARM's SMC64 convention for Monitor calls, the command is encoded
/// in `w0`, inputs parameters are stored in the fields `x1` to `x17` and
/// output values are stored in the fields `x0` to `x17`.
///
/// # Arguments
///
/// - `params`      Mutable reference to a structure storing the eighteen u64
///                 input values of the call, and updated on success with the
///                 return values.
///
/// # Errors
///
/// - `ENOSYS`
///      If the current architecture does not support this monitor call.
/// - `ECALLDENIED`
///      If the configuration is preventing the current process from performing
///      the SMC kernel call.
/// - `EINVAL`
///      On Aarch64, if the calling convention used is SMC32 (detected based on
///      the value of `x0`)

// TODO test
pub fn smc64(params: &mut smc64_params_t) -> Result<()> {
    match unsafe { libpnc::pnc_smc64(params) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("smc64 failed")),
    }
}

/// Perform a remote function call to the M-mode firmware, based on the
/// `params.a7` and `params.a6` values and using the RISC-V Supervisor Binary
/// Interface calling convention.
///
/// This method is specific to the riscv64 architecture. It is optionally
/// implemented, and will always return `ENOSYS` if the current platform
/// disables the use of this syscall.
///
/// Following the SBI specification, input parameters are stored in the fields
/// `a0` to `a7` and output values are stored in the fields `a0` and `a1`.
///
/// # Arguments
///
/// - `params`      Pointer to a structure storing the eight u64 input values of
///                 the call, and updated on success with the return values.
///
/// # Errors
///
/// - `ENOSYS`
///      If the current architecture does not support this monitor call.
/// - `ECALLDENIED`
///      If the configuration is preventing the current process from performing
///      the SMC kernel call.

// TODO test
pub fn mcall(params: &mut mcall_params_t) -> Result<()> {
    match unsafe { libpnc::pnc_mcall(params) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("mcall failed")),
    }
}

/// Perform a remote function call to the hypervisor
///
/// This method is specific to ARM based architectures. It is optionally
/// implemented, and will always return `ENOSYS` if the current platform
/// disables the use of this syscall.
///
/// Following ARM's convention for Hypervisor calls, the command is encoded
/// in `r0`, inputs parameters are stored in the fields `r1` to `r7` and
/// output values are stored in the fields `r0` to `r3`.
///
/// # Arguments
///
/// * `params`        Pointer to a structure storing the eight `u32` input values of the call,
///                   and updated on success with the four return values.
/// # Errors
///
/// - `ENOSYS`
///      If the current architecture does not support hypervisor calls.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the HVC kernel call.

// TODO test
pub fn hvc(params: &mut hvc32_params_t) -> Result<()> {
    match unsafe { libpnc::pnc_hvc(params) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("hvc failed")),
    }
}

/// Returns the base address and size of the data region the of the process
/// identified by the object process identifier.
///
/// If the process identified by object is not the current application, then the
/// caller must be configured as an auditor.
///
/// # Arguments
///
/// - `object`      Process identifier of the process whose information is
///                 requested.
/// - `pdata_base`  Mutable reference to a buffer where to store the virtual
///                 base address of the object process' data region.
/// - `pdata_size`  Mutable reference  to a buffer where to store the size (in
///                 bytes) of the object process' data region.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current process from performing
///      the INFO kernel call.
/// - `EINVAL`
///      If the specified process identifier is invalid.
/// - `EPERM`
///      If the specified object process is not the current application,
///      and the current application is not configured as an auditor.

// TODO test
pub fn get_data_of(
    object: s_pid_t,
    pdata_base: &mut *mut c_void,
    pdata_size: &mut c_uint,
) -> Result<()> {
    match unsafe { libpnc::pnc_get_data(object, pdata_base, pdata_size) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("get_data failed")),
    }
}

/// Returns the base address and size of the data region the of the current
/// process.
///
/// # Arguments
///
/// - `pdata_base`  Mutable reference to a buffer where to store the virtual
///                 base address of the current process' data region.
/// - `pdata_size`  Mutable reference  to a buffer where to store the size (in
///                 bytes) of the current process' data region.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current process from performing
///      the INFO kernel call.

// TODO test
pub fn get_data(pdata_base: &mut *mut c_void, pdata_size: &mut c_uint) -> Result<()> {
    get_data_of(get_pid()?, pdata_base, pdata_size)
}

/// Return the kernel related metadata for any ELF application,
///  identified by its name (the same one you would use for the `exec` call).
///
///  As a result, the output message will store the maximum quota
///  limitation (in 4Kib pages), the physical buffer information
///  (size in 4Kib pages and type), some ELF related status information
///  (like the ELF identity). Three macros `METADATA_PHYSINFO_CACHED`,
///  `METADATA_PHYSINFO_NRPAGES` and `METADATA_CACHE_CRITICAL`
///  are provided to extract the relevant parts from the metadata information.
///
/// # Arguments
///
/// * `elf_name`  Reference to a string describing the name of the application to target.
/// * `data_size` Mutable reference to a buffer where to store the maximum quota limitation,
///               in pages.
/// * `minfo`     Mutable reference to a buffer where to store the metadata.
/// * `elf_nr`    Mutable reference to a buffer where to store the ELF identifier.
///
/// # Errors
///
/// - `EDOM`
///      If the address range given for the ELF name is invalid.
/// - `EINVAL`
///      If the given address range is not mapped in the caller.
/// - `ESRCH`
///      If the given ELF name cannot be found.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the INFO kernel call.

// TODO test
pub fn get_metadata_by_app(
    elf_name: &str,
    data_size: &mut u32,
    minfo: &mut u32,
    elf_nr: &mut u32,
) -> Result<()> {
    let elf_name_str = CString::new(elf_name).expect("Couldn't create CString");
    let elf_name_len = elf_name_str.as_bytes().len();
    match unsafe {
        libpnc::pnc_get_metadata_by_app(
            elf_name_str.as_ptr(),
            elf_name_len,
            data_size,
            minfo,
            elf_nr,
        )
    } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("get_metadata_by_app failed")),
    }
}

/// Return the kernel related metadata for any application, identified by its PID.
///
/// As a result, the output message will store the maximum quota
/// limitation (in 4Kib pages), the physical buffer information
/// (size in 4Kib pages and type), some ELF related status information
/// (like the ELF identity). Three macros `METADATA_PHYSINFO_CACHED`,
/// `METADATA_PHYSINFO_NRPAGES` and `METADATA_CACHE_CRITICAL`
/// are provided to extract the relevant parts from the metadata information.
///
/// # Arguments
///
/// - `pid`       Identifier of the process one wants metadata information from.
/// - `data_size` Mutable reference to a buffer where to store the maximum quota
///               limitation, in pages.
/// - `minfo`     Mutable reference to a buffer where to store the metadata.
/// - `elf_nr`    Mutable reference to a buffer where to store the ELF
///               identifier.
///
/// # Errors
///
/// - `EINVAL`
///      If the requested process identifier is invalid.
/// - `EPERM`
///      If the requested process identifier corresponds to a kernel task.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the INFO kernel call.

// TODO test
pub fn get_metadata_by_pid(
    pid: s_pid_t,
    data_size: &mut u32,
    minfo: &mut u32,
    elf_nr: &mut u32,
) -> Result<()> {
    match unsafe { libpnc::pnc_get_metadata_by_pid(pid, data_size, minfo, elf_nr) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("get_metadata_by_pid failed")),
    }
}

/// Return the PID of a system process identified by its name in the
/// kernel's configuration
///
/// # Arguments
///
/// * `sysproc_name`  A string describing the name of the application to target.
///
/// # Errors
/// - `EDOM`
///      If the address range given for the system process name is invalid.
/// - `EINVAL`
///      If the given address range is not mapped in the caller.
/// - `ESRCH`
///      If the given system process name cannot be found.
/// - `EDSTDIED`
///      If the given system process has terminated
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the INFO kernel call.

// TODO test
pub fn get_sysproc_pid(sysproc_name: &str) -> Result<s_pid_t> {
    let sysproc_name_str = CString::new(sysproc_name).expect("Couldn't create CString");
    let sysproc_name_len = sysproc_name_str.as_bytes().len();
    let mut pid = 0;
    match unsafe {
        libpnc::pnc_get_sysproc_pid_by_name(sysproc_name_str.as_ptr(), sysproc_name_len, &mut pid)
    } {
        errno::OK => Ok(pid),
        err => Err(Error::from(err).context("get_sysproc_pid failed")),
    }
}

const INVALID_PID_CODE_WIDTH: usize = 32;
const INVALID_PID_CODE_SHIFT: usize = 16;
const INVALID_PID_LEFT: s_pid_t = 0x003f;
const INVALID_PID_RIGHT: s_pid_t = 0x0000;

/// Generate a guaranteed invalid PID
///
/// Special PIDs guaranteed not to collide we live ones.
/// To build them, we choose static PIDs which are of the form
/// ```
/// 0x00xyyyyyyyyyyyyz where x = {0, 1, 2, 3}, y = hex numbers, z = {0, 8}
/// ```
/// Out of them, only the first 128 are valid.
/// Thus we could set a 32 bits range of invalid PIDs of the form:
/// ```
/// 0x003fyyyyyyyy0000
/// ```
///
/// Note that there are pids which cannot be generated by the kernel,
/// and for which `is_invalid_pid(pid)` still does not hold.
///
/// `invalid_pid(code)` is really to allow functions returning either a valid pid,
/// or a special value (usually an error code).
///
/// This can also be used to initialize variables to values which are guaranteed
/// to not collide with an existing pid.
pub const fn invalid_pid(code: u32) -> s_pid_t {
    ((((code as u64) & ((1u64 << INVALID_PID_CODE_WIDTH) - 1))
        | (INVALID_PID_LEFT << INVALID_PID_CODE_WIDTH))
        << INVALID_PID_CODE_SHIFT)
        | INVALID_PID_RIGHT
}

/// Check that a PID was generated by `invalid_pid`
///
/// `is_invalid_pid(invalid_pid(code))` is guaranteed to be true.
///
/// Note that there are pids which cannot be generated by the kernel,
/// and for which `is_invalid_pid(pid)` still does not hold.
pub const fn is_invalid_pid(pid: s_pid_t) -> bool {
    (((pid) >> (INVALID_PID_CODE_SHIFT + INVALID_PID_CODE_WIDTH)) == INVALID_PID_LEFT)
        && (((pid) & (((1u64) << INVALID_PID_CODE_SHIFT) - 1)) == INVALID_PID_RIGHT)
}

/// Get the code inside a invalid PID generated with `invalid_pid`
///
/// Assuming that is_invalid_pid(pid) holds, `INVALID_PID(invalid_pid_code(pid)) == pid` is
/// guaranteed to be true.
pub const fn invalid_pid_code(pid: s_pid_t) -> u32 {
    (((pid) >> INVALID_PID_CODE_SHIFT) & (((1u64) << INVALID_PID_CODE_WIDTH) - 1)) as u32
}

/// Return the capabilities of the application specified by the object process
/// identifier, as described by its privilege structure.
///
/// As a result, three different bitmasks are returned:
///  - `priv_flags` contains various privileged attributes which can be accessed
///    using the flags `PrivFlag::Preemptible`, `PrivFlag::Sysproc`,
///    `PrivFlag::Restart`, `PrivFlag::Debugger`, `PrivFlag::Debuggee`,
///    `PrivFlag::Killable` `PrivFlag::Auditor`
///  - `trap_mask` describes which IPC traps are allowed; the flags
///    `TrapCapability::Send`, `TrapCapability::Receive`,
///    `TrapCapability::Sendrec`, `TrapCapability::Notify` and
///    `TrapCapability::Sleep` allow to test each individual kind of trap.
///  - `call_mask` describes which kernel calls are allowed; each kernel call is
///    denoted by the bit corresponding to its kernel call number.
///
/// If the process identified by object is not the current application, then the
/// caller must be configured as an auditor.
///
/// # Arguments
///
/// - `object`      Process identifier of the process whose information is
///                 requested.
/// - `priv_flags`  When successful, contains a bitmask describing some
///                 privileged attributes of the object process.
/// - `trap_mask`   When successful, contains a bitmask describing which IPC
///                 traps are allowed for the object process.
/// - `call_mask`   When successful, contains a bitmask describing which kernel
///                 calls are allowed for the object  process.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current process from performing
///      the INFO kernel call.
/// - `EINVAL`
///      If the specified process identifier is invalid.
/// - `EPERM`
///      If the specified object process is not the current application,
///      and the current application is not configured as an auditor.

// TODO test
pub fn get_capabilities_of(
    object: s_pid_t,
    priv_flags: &mut u32,
    trap_mask: &mut u32,
    call_mask: &mut u64,
) -> Result<()> {
    match unsafe { libpnc::pnc_get_capabilities(object, priv_flags, trap_mask, call_mask) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("get_capabilities failed")),
    }
}

/// Return the capabilities of the current application, as described by its
/// privilege structure.
///
/// See `get_capabilities_of` for more details.
///
/// # Arguments
///
/// - `priv_flags`  When successful, contains a bitmask describing some
///                 privileged attributes of the current process.
/// - `trap_mask`   When successful, contains a bitmask describing which IPC
///                 traps are allowed for the current process.
/// - `call_mask`   When successful, contains a bitmask describing which kernel
///                 calls are allowed for the current  process.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current process from performing
///      the INFO kernel call.

// TODO test
pub fn get_capabilities(
    priv_flags: &mut u32,
    trap_mask: &mut u32,
    call_mask: &mut u64,
) -> Result<()> {
    get_capabilities_of(get_pid()?, priv_flags, trap_mask, call_mask)
}

/// Properties associated to a privilege structure in the kernel configuration
pub enum PrivFlag {
    /// Bit in bitmask expressing that a process is preemptible
    Preemptible = 0x1,
    /// Bit in bitmask expressing that a process is a sysproc
    Sysproc = 0x2,
    /// Bit in bitmask expressing that a process is automatically restarted
    Restart = 0x4,
    /// Bit in bitmask expressing that a process is a debugger
    Debugger = 0x8,
    /// Bit in bitmask expressing that a process is a debuggee
    Debuggee = 0x10,
    /// Bit in bitmask expressing that a process is killable
    Killable = 0x20,
    /// Bit in bitmask expressing that a process is an auditor
    Auditor = 0x40,
}

impl BitOr for PrivFlag {
    type Output = u32;

    fn bitor(self, rhs: Self) -> Self::Output {
        self as u32 | rhs as u32
    }
}

impl BitOr<u32> for PrivFlag {
    type Output = u32;

    fn bitor(self, rhs: u32) -> Self::Output {
        self as u32 | rhs
    }
}

impl BitOr<PrivFlag> for u32 {
    type Output = u32;

    fn bitor(self, rhs: PrivFlag) -> Self::Output {
        self | rhs as u32
    }
}

impl BitAnd<u32> for PrivFlag {
    type Output = u32;

    fn bitand(self, rhs: u32) -> Self::Output {
        self as u32 & rhs
    }
}

impl BitAnd<PrivFlag> for u32 {
    type Output = u32;

    fn bitand(self, rhs: PrivFlag) -> Self::Output {
        self & rhs as u32
    }
}

/// Traps allowed to a privilege structure in the kernel configuration
pub enum TrapCapability {
    /// Bit in bitmask representing the ability to perform the send IPC
    Send = 0x1,
    /// Bit in bitmask representing the ability to perform the receive IPC
    Receive = 0x2,
    /// Bit in bitmask representing the ability to perform the sendrec IPC
    Sendrec = 0x4,
    /// Bit in bitmask representing the ability to perform the notify IPC
    Notify = 0x8,
    /// Bit in bitmask representing the ability to perform the sleep IPC
    Sleep = 0x10,
}

impl BitOr for TrapCapability {
    type Output = u32;

    fn bitor(self, rhs: Self) -> Self::Output {
        self as u32 | rhs as u32
    }
}

impl BitOr<u32> for TrapCapability {
    type Output = u32;

    fn bitor(self, rhs: u32) -> Self::Output {
        self as u32 | rhs
    }
}

impl BitOr<TrapCapability> for u32 {
    type Output = u32;

    fn bitor(self, rhs: TrapCapability) -> Self::Output {
        self | rhs as u32
    }
}

impl BitAnd<u32> for TrapCapability {
    type Output = u32;

    fn bitand(self, rhs: u32) -> Self::Output {
        self as u32 & rhs
    }
}

impl BitAnd<TrapCapability> for u32 {
    type Output = u32;

    fn bitand(self, rhs: TrapCapability) -> Self::Output {
        self & rhs as u32
    }
}

/// Return the authorized IPC traps for the object application with the
/// application identified by the PID target.
///
/// As a result, the allowed IPC traps will be returned as a bitmask. The flags
/// `TrapCapability::Send`, `TrapCapability::Receive`,
/// `TrapCapability::Sendrec`, `TrapCapability::Notify` and
/// `TrapCapability::Sleep` allow to test each individual kind of trap. The
/// returned information is based on the processes' privileges and domains, i.e.
/// static information. The current particular status of target, regarding
/// deadlocks for instance, is ignored. Also note, that the status of the
/// `TrapCapability::Sleep` is independent of the actual target.
///
/// If the process identified by object is not the current application, then the
/// caller must be configured as an auditor.
///
/// # Arguments
///
/// - `object`      Process identifier of the process whose information is
///                 requested.
/// - `pid`         Identifier of the process one wants to communicate with.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current process from performing
///      the INFO kernel call.
/// - `EINVAL`
///      If the object or target process identifiers do not correspond to valid
///      non-kernel tasks.
/// - `EPERM`
///      If the specified object process is not the current application, and
///      the current application is not configured as an auditor.

// TODO test
pub fn get_allowed_ipc_of(object: s_pid_t, target: s_pid_t) -> Result<u32> {
    let mut trap_mask = 0;
    match unsafe { libpnc::pnc_get_allowed_ipc(object, target, &mut trap_mask) } {
        errno::OK => Ok(trap_mask),
        err => Err(Error::from(err).context("get_allowed_ipc failed")),
    }
}
/// Return the authorized IPC traps for the current application with the
/// application identified by the PID target.
///
/// See `get_allowed_ipc_of` for more details.
///
/// # Arguments
///
/// - `pid`         Identifier of the process one wants to communicate with.
///
/// # Errors
///
/// - `ECALLDENIED`
///      If the configuration is preventing the current process from performing
///      the INFO kernel call.

// TODO test
pub fn get_allowed_ipc(target: s_pid_t) -> Result<u32> {
    get_allowed_ipc_of(get_pid()?, target)
}

// TODO: get_phys_buff
