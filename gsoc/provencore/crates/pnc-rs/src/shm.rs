/*
 * Copyright (c) 2020-2020 Prove & Run S.A.S
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

use crate::error::{Error, Result};
use crate::libc::{c_uint, c_void};
use crate::libpnc::{self, errno, s_pid_t};
use crate::{get_pid, AuthGuard, AuthMode};

/// A safe wrapper over ProvenCore's shared memory mechanism
pub struct SharedMem {
    id: c_uint,
    vaddr: *mut c_void, // marked as null if we unregister manually
    nr_pages: c_uint,   // size of the allocation in number of 4KiB pages
    _auth: Option<AuthGuard>,
}

#[allow(clippy::len_without_is_empty)]
impl SharedMem {
    /// Create a region of shared memory with read and  write access to
    /// the current process.
    ///
    /// On success, it returns a fresh handle to a new shared region.
    /// The new handle should be explicitly advertised to any other party involved
    /// in the sharing, for example by performing a process-to-process copy of
    /// `id`, or sending it by the means of IPCs. Any shared memory buffer
    /// allocation is subject to quota restriction, like `brk`.
    ///
    /// # Arguments
    ///
    /// * `nr_pages`  Size of the memory region to create, as a number of 4KiB pages.
    /// * `id `       Pointer to a buffer which will hold a handle to the memory region.
    /// * `vaddr`     Pointer to a buffer which will hold the virtual address at which the memory
    ///               region has been mapped.
    ///
    /// # Errors
    ///
    /// This function may fail with the following error codes:
    ///
    /// - `EINVAL`
    ///      If the number of pages is zero.
    /// - `ENOSPC`
    ///      If there is no available region descriptor to
    ///      map this new shared memory, or if the current process
    ///      is already registered to its maximum number of shared
    ///      memories, or if there is not sufficient space in the
    ///      current process' virtual address space to map this
    ///      new region.
    /// - `E2BIG`
    ///      If the additional allocated memory would exceed the
    ///      quota restriction.
    /// - `ENOMEM`
    ///      If there are not enough physical pages available
    ///      to allocate and map a memory region of the required size.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the SHM_ALLOC kernel call.
    ///
    pub fn alloc(nr_pages: c_uint) -> Result<SharedMem> {
        let mut id = 0;
        let mut vaddr = core::ptr::null_mut();

        match unsafe { libpnc::pnc_shm_alloc(nr_pages, &mut id, &mut vaddr) } {
            errno::OK => Ok(Self {
                id,
                vaddr,
                nr_pages,
                _auth: None,
            }),
            err => Err(Error::from(err).context("shm_alloc failed")),
        }
    }

    /// Create the mapping to access the shared region identified by the
    /// handle `id`.
    ///
    /// On success, a region of `nr_pages` 4KiB pages will be mapped starting
    /// at some address written to `vaddr`, with **no rights at all**.
    /// At this point, access to this region will trigger a fault which will
    /// terminate the process. Registration to a shared memory buffer is considered
    /// to be a memory allocation, and is therefore subject to quota restriction,
    /// like `brk`.
    ///
    /// # Arguments
    ///
    /// * `id` Handle of the shm region to register to.
    ///
    /// # Errors
    ///
    /// This function may fail with the following error codes:
    ///
    /// - `EINVAL`
    ///      If the given shared memory ID does not correspond
    ///      to an existing active shared memory.
    /// - `ENOSPC`
    ///      If there is no available region descriptor to map
    ///      this new shared memory, or if the current process is
    ///      already registered to its maximum number of shared
    ///      memories, or if there is not sufficient space in the
    ///      current process' virtual address space to map this
    ///      new region.
    /// - `E2BIG`
    ///      If the additional allocated memory would exceed the
    ///      quota restriction.
    /// - `ENOMEM`
    ///      If there are not enough physical pages available
    ///      to create the bindings to map this shared memory region.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the SHM_REGISTER kernel call.
    pub fn register(id: c_uint) -> Result<SharedMem> {
        let mut vaddr = core::ptr::null_mut();
        let mut nr_pages = 0;
        match unsafe { libpnc::pnc_shm_register(id, &mut nr_pages, &mut vaddr) } {
            errno::OK => Ok(Self {
                id,
                vaddr,
                nr_pages,
                _auth: None,
            }),
            err => Err(Error::from(err).context("shm_register failed")),
        }
    }

    /// Returns the identity of the current owner of a shared memory token,
    ///  provided that the current process is correctly registered to the
    ///  requested shared memory.
    ///
    /// # Errors
    /// - `EINVAL`
    ///      If the given shared memory region id is invalid.
    /// - `EPERM`
    ///      If the caller is not registered to the given shared memory region.
    /// - `ESRCH`
    ///      If the given shared memory region has no owner.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the INFO kernel call.
    pub fn owner(&self) -> Result<s_pid_t> {
        let mut owner = 0;
        match unsafe { libpnc::pnc_get_shm_token_owner(self.id, &mut owner) } {
            errno::OK => Ok(owner),
            err => Err(Error::from(err).context("owner failed")),
        }
    }

    /// Give up a current focus on some shared memory region, and transfer
    /// it to another process.
    ///
    /// If the current process is the current owner of the shared memory, it will
    /// transfer the control of the region to process `target`, and lose all its
    /// rights to access the shared buffer. The region is still alive, but
    /// mapped without any access (neither read, nor write access) in the
    /// current process' address space. The target, however, now owns full
    /// rights on the newly transferred buffer.
    ///
    /// Note that for `transfer` to be successful, correct authorizations
    /// must have been configured: the current process has to allow reading from
    /// its shared memory buffer, and `target` has to allow writing to its
    /// shared memory buffer.
    ///
    /// This can be setup either manually by using `setauth` yourself or more simply
    /// by using the `set_sender_perm` and `set_receiver_perm` methods in the respective
    /// processes before calling `transfer`.
    ///
    /// # Arguments
    ///
    /// * `target`    Identifier of the process that will gain focus over the shared region.
    ///
    /// # Errors
    ///
    /// This function may fail with the following error codes:
    ///
    /// - `EINVAL`
    ///      If the given process ID does not correspond to
    ///      an active process, or if the given process ID corresponds
    ///      to a kernel task, or if the given shared memory ID does
    ///      not correspond to an existing shared memory region that
    ///      the current process is registered to, or if the given
    ///      process ID does not correspond to a process registered
    ///      to the requested shared memory region.
    /// - `EPERM`
    ///      If the current process is not the current focus of
    ///      that shared memory region, or if the authorization
    ///      on the shared memory region is not set up correctly on
    ///      both sides.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the SHM_TRANSFER kernel call.
    pub fn transfer(&self, target: s_pid_t) -> Result<()> {
        match unsafe { libpnc::pnc_shm_transfer(self.id, target) } {
            errno::OK => Ok(()),
            err => Err(Error::from(err).context("shm_transfer failed")),
        }
    }

    /// Borrow the shared memory as an immutable byte buffer.
    ///
    /// You do not need to own the shared memory at the time you make the borrow, but you will need
    /// to own it to read buffer values.
    pub fn as_buf(&self) -> &[u8] {
        unsafe {
            core::slice::from_raw_parts(self.vaddr as *mut u8, self.nr_pages as usize * 0x1000)
        }
    }

    /// Borrow the shared memory as a mutable byte buffer.
    ///
    /// You do not need to own the shared memory at the time you make the borrow, but you will need
    /// to own it to read or write buffer values.
    pub fn as_buf_mut(&mut self) -> &mut [u8] {
        unsafe {
            core::slice::from_raw_parts_mut(self.vaddr as *mut u8, self.nr_pages as usize * 0x1000)
        }
    }

    /// Setup necessary permissions to transfer the shared memory on the side of the sender.
    ///
    /// This function should be called before `transfer` if you have not setup the permissions
    /// yourself.
    ///
    /// Permissions setup are guaranteed to be revoked once the `SharedMem` is destroyed.
    ///
    /// # Arguments
    ///
    /// * `receiver` The process that the shared memory will be transferred to.
    ///
    /// # Errors
    ///
    /// This will forwared the underlying errors of `setauth` and `get_pid`
    pub fn set_sender_perm(&mut self, receiver: s_pid_t) -> Result<()> {
        let sender = get_pid()?;
        let obj = self.as_buf();
        self._auth = Some(AuthGuard::set(AuthMode::Read, receiver, obj, sender)?);
        Ok(())
    }

    /// Setup necessary permissions to transfer the shared memory on the side of the receiver.
    ///
    /// This function should be called before `transfer` is called by the sender if you have not
    /// setup the permissions yourself.
    ///
    /// Permissions setup are guaranteed to be revoked once the `SharedMem` is destroyed.
    ///
    /// # Arguments
    ///
    /// * `sender` The process that the shared memory will be transferred from.
    ///
    /// # Errors
    ///
    /// This will forwared the underlying errors of `setauth`
    pub fn set_receiver_perm(&mut self, sender: s_pid_t) -> Result<()> {
        let obj = self.as_buf();
        self._auth = Some(AuthGuard::set(AuthMode::Write, sender, obj, sender)?);
        Ok(())
    }

    /// Force revokation of the permissions setup through `set_sender_perm` or `set_receiver_perm`
    pub fn clear_perm(&mut self) {
        self._auth = None;
    }

    /// Drop without unregistering.
    ///
    /// # Safety
    ///
    /// The shared memory handle will unregister the shared memory when dropped by default, but in
    /// case you may want to handle the unregistration yourself (at it can fail) you may use this
    /// function to drop the shared memory handle without triggering `shm_unregister`.
    ///
    /// If you use this function and do not handle the unregistration you will leak a shared memory
    /// handle.
    pub unsafe fn was_unregistered(mut self) {
        self.vaddr = core::ptr::null_mut();
    }

    /// Get a pointer to the top of the underlying shared memory block
    pub fn vaddr(&self) -> *mut c_void {
        self.vaddr
    }

    /// Get the handle id of the shared memory block
    pub fn id(&self) -> c_uint {
        self.id
    }

    /// Get the number of 4KiB pages this shared memory block is composed of
    pub fn nr_pages(&self) -> c_uint {
        self.nr_pages
    }

    /// Get the total adressable size of this shared memory block in bytes
    pub fn len(&self) -> usize {
        self.nr_pages() as usize * 4096
    }
}

impl Drop for SharedMem {
    fn drop(&mut self) {
        self.clear_perm();

        if !self.vaddr.is_null() {
            match unsafe { libpnc::pnc_shm_unregister(self.id) } {
                errno::OK => {
                    self.vaddr = core::ptr::null_mut();
                }
                err => panic!("{}", Error::from(err).context("shm_unregister failed")),
            }
        }
    }
}
