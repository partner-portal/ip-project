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
use crate::get_pid;
use crate::libc::{c_int, c_uint, c_void};
use crate::libpnc::{self, errno, s_pid_t};

/// Guard that will automatically revoke memory authorizations when it drops out of scope
///
/// Wraps over an `AuthHandle` to give it RAII semantics.
/// When the `AuthGuard` drops the authorization is revoked. This will panic if the authorization
/// fails.
pub struct AuthGuard {
    handle: AuthHandle,
}

impl AuthGuard {
    ///
    /// Configure a memory authorization guard.
    ///
    ///
    ///  `set` sets up a new authorization which grants the process
    ///  `grantee` access to the current process' data with rights specified by
    ///  `mode`. The authorization is effective in the address space region represented by `obj`.
    ///  The process `effector` is the one that is allowed to trigger the
    ///  action of moving data between the current process and `grantee`.
    ///
    ///  **The current process is the only process that can remove this authorization.**
    ///  It will be automatically removed when the `AuthGuard` drops.
    ///
    ///  On success, a valid handle is created which can be used in further calls to `vircopy` or
    ///  manipulated with `AuthGuard`'s methods.
    ///
    /// # Arguments
    ///
    /// - `mode`  The authorization mode.

    /// - `grantee`       Process to which (resp. from which) a copy
    ///                        is being authorized in read mode (resp. write mode).
    ///                        It cannot be the current process, as copying to
    ///                        itself is forbidden.
    /// - `obj`         A reference to a byte buffer in the current process'
    ///                        address space that is impacted by the new
    ///                        authorization.
    /// - `effector`      Process which is allowed to perform the transfer of
    ///                        memory between the current process and grantee.
    ///
    /// # Errors
    ///
    /// - `EDOM`
    ///      If the range of addresses described overflows, or the length is not
    ///      strictly positive.
    /// - `EINVAL`
    ///      If the required permission is set to `NO_ACCESS`, or
    ///      if one of grantee, revoker or effector is not an active
    ///      process, or if grantee is the current process.
    /// - `EFAULT`
    ///      If the described range of addresses is not correctly mapped in
    ///      the data region of the current process.
    /// - `ENOMEM`
    ///      If the current process has reached its maximum number of active
    ///      memory permissions.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the SETAUTH kernel call.
    pub fn set(mode: AuthMode, grantee: s_pid_t, obj: &[u8], effector: s_pid_t) -> Result<Self> {
        let revoker = get_pid()?;
        let handle = AuthHandle::set(mode, grantee, obj, effector, revoker)?;
        Ok(Self { handle })
    }
    /// Create a new `AuthGuard` out of raw handle and owner
    ///
    /// This is usually to be used with barebones IPC, so that you can create an authorization in
    /// one process, send the raw parts over to another process and reconstruct the underlying `AuthHandle` in
    /// that other process.
    pub fn from_raw(handle: c_uint, owner: s_pid_t) -> Self {
        let handle = AuthHandle::from_raw(handle, owner);
        Self { handle }
    }
    /// Revoke the underlying authorization
    ///
    /// This consumes the guard and drops it. Will panic if the revocation fails.
    pub fn revoke(self) {}
    /// Change the process that is allowed to access the data subject to
    ///  the authorization.
    ///
    ///  If the current process is the current grantee of this authorization but
    ///  is not the final endpoint of a transaction, it can rely on this call
    ///  to give up its right to the authorization, and give them to another process.
    ///  Processes other than the current grantee of the authorization are not
    ///  allowed it to perform this change.
    ///
    ///  Note that it is forbidden to transfer an authorization to the process
    ///  identified by `owner` to prevent that process to use `pnc_copy` to copy
    ///  data to itself.
    ///
    ///  # Arguments
    ///
    /// - `new_grantee`   Identity of the new process which will
    ///                        be granted the right to access the owner's
    ///                        address space as defined by the authorization.
    /// # Errors
    ///
    /// - `EINVAL`
    ///      If `owner` or `new_grantee` are not active processes,
    ///      or if `handle` is not a valid handle on an authorization set by
    ///      `owner`, or if `new_grantee` is `owner`.
    /// - `EPERM`
    ///      If the current process is not the designated grantee of
    ///      this authorization.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the CHANGE_GRANTEE kernel call.
    pub fn change_grantee(&mut self, new_grantee: s_pid_t) -> Result<()> {
        self.handle.change_grantee(new_grantee)
    }
    /// Transfer to a new process the right to execute a successful copy
    ///  between two endpoints.
    ///
    ///  If the current process is not the relevant process to perform
    ///  a copy, it can rely on this call to give up its right and give them
    ///  to another process.
    ///
    ///  This is only allowed if the current process is the configured effector
    ///  for this authorization.
    ///
    /// # Arguments
    ///
    /// - `new_effector`  Identity of the new process which will
    ///                        be granted the right to perform the copy.
    /// # Errors
    /// - `EINVAL`
    ///      If `owner` or `new_effector` are not active processes,
    ///      or if `auth_handle` is not a valid handle on an authorization set by
    ///      `owner`.
    /// - `EPERM`
    ///      If the current process is not the designated effector of
    ///      this authorization.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the CHANGE_EFFECTOR kernel call.
    pub fn change_effector(&mut self, new_effector: s_pid_t) -> Result<()> {
        self.handle.change_effector(new_effector)
    }
    /// Return the underlying raw handle of the authorization
    pub fn raw_handle(&self) -> c_uint {
        self.handle.raw_handle()
    }
    /// Return the owner of the authorization we are wrapping around
    pub fn owner(&self) -> s_pid_t {
        self.handle.owner()
    }
}

impl Drop for AuthGuard {
    fn drop(&mut self) {
        match unsafe { libpnc::pnc_revoke(self.handle.owner(), self.handle.raw_handle()) } {
            errno::OK | errno::EINVAL => {} // Auth is no longer valid because either it was dropped by someone else or the process no longer exists
            err => panic!("{}", Error::from(err).context("revoke failed")),
        }
    }
}

/// Memory authorization modes
pub enum AuthMode {
    /// Memory read access authorization
    Read = 1,
    /// Memory write access authorization
    Write = 2,
    /// Memory read and write access authorization
    ReadWrite = 1 | 2,
}

/// Simple wrapper for memory authorizations
///
/// To be able to copy data between two processes, each endpoint
/// must have set up sufficient authorizations. The kernel implements a strong
/// access control policy, which makes it possible to guarantee a strong formal
/// isolation property between user processes.
pub struct AuthHandle {
    handle: c_uint,
    owner: s_pid_t,
}

impl AuthHandle {
    ///
    /// Configure a memory authorization.
    ///
    ///
    ///  `set` sets up a new authorization which grants the process
    ///  `grantee` access to the current process' data with rights specified by
    ///  `mode`. The authorization is effective in the address space region represented by `obj`.
    ///  The process `effector` is the one that is allowed to trigger the
    ///  action of moving data between the current process and `grantee`.
    ///  The process `revoker` is the process that can remove this authorization.
    ///
    ///  On success, a valid handle is created which can be used in further calls to `vircopy` or
    ///  manipulated with `AuthHandle`'s methods.
    ///
    /// # Arguments
    ///
    /// - `mode`  The authorization mode.

    /// - `grantee`       Process to which (resp. from which) a copy
    ///                        is being authorized in read mode (resp. write mode).
    ///                        It cannot be the current process, as copying to
    ///                        itself is forbidden.
    /// - `obj`         A reference to a byte buffer in the current process'
    ///                        address space that is impacted by the new
    ///                        authorization.
    /// - `effector`      Process which is allowed to perform the transfer of
    ///                        memory between the current process and grantee.
    /// - `revoker`       Process which is allowed to remove this authorization
    ///                        from the current process' active authorizations.
    ///
    /// # Errors
    ///
    /// - `EDOM`
    ///      If the range of addresses described overflows, or the length is not
    ///      strictly positive.
    /// - `EINVAL`
    ///      If the required permission is set to `NO_ACCESS`, or
    ///      if one of grantee, revoker or effector is not an active
    ///      process, or if grantee is the current process.
    /// - `EFAULT`
    ///      If the described range of addresses is not correctly mapped in
    ///      the data region of the current process.
    /// - `ENOMEM`
    ///      If the current process has reached its maximum number of active
    ///      memory permissions.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the SETAUTH kernel call.
    pub fn set(
        mode: AuthMode,
        grantee: s_pid_t,
        obj: &[u8],
        effector: s_pid_t,
        revoker: s_pid_t,
    ) -> Result<Self> {
        let owner = get_pid()?;
        let size = core::mem::size_of_val(obj);
        let mut handle: c_uint = 0;
        match unsafe {
            libpnc::pnc_setauth(
                mode as c_int,
                grantee,
                obj as *const _ as *const c_void,
                size,
                effector,
                revoker,
                &mut handle,
            )
        } {
            errno::OK => Ok(Self { handle, owner }),
            err => Err(Error::from(err).context("setauth failed")),
        }
    }
    /// Create a new `AuthHandle` out of raw handle and owner
    ///
    /// This is usually to be used with barebones IPC, so that you can create an authorization in
    /// one process, send the raw parts over to another process and reconstruct the `AuthHandle` in
    /// that other process.
    pub fn from_raw(handle: c_uint, owner: s_pid_t) -> Self {
        Self { handle, owner }
    }

    /// Revoke the underlying authorization
    ///
    /// # Errors
    ///
    /// - `EINVAL`
    ///      If `owner` is not an active process or if `handle` is not
    ///      a valid handle on an authorization set by `owner`.
    /// - `EPERM`
    ///      If the current process is not the designated revoker for
    ///      this authorization.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the REVOKE kernel call.
    pub fn revoke(&mut self) -> Result<()> {
        match unsafe { libpnc::pnc_revoke(self.owner, self.handle) } {
            errno::OK => Ok(()),
            err => Err(Error::from(err).context("revoke failed")),
        }
    }
    /// Change the process that is allowed to access the data subject to
    ///  the authorization.
    ///
    ///  If the current process is the current grantee of this authorization but
    ///  is not the final endpoint of a transaction, it can rely on this call
    ///  to give up its right to the authorization, and give them to another process.
    ///  Processes other than the current grantee of the authorization are not
    ///  allowed it to perform this change.
    ///
    ///  Note that it is forbidden to transfer an authorization to the process
    ///  identified by `owner` to prevent that process to use `pnc_copy` to copy
    ///  data to itself.
    ///
    ///  # Arguments
    ///
    /// - `new_grantee`   Identity of the new process which will
    ///                        be granted the right to access the owner's
    ///                        address space as defined by the authorization.
    /// # Errors
    ///
    /// - `EINVAL`
    ///      If `owner` or `new_grantee` are not active processes,
    ///      or if `handle` is not a valid handle on an authorization set by
    ///      `owner`, or if `new_grantee` is `owner`.
    /// - `EPERM`
    ///      If the current process is not the designated grantee of
    ///      this authorization.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the CHANGE_GRANTEE kernel call.
    pub fn change_grantee(&mut self, new_grantee: s_pid_t) -> Result<()> {
        match unsafe { libpnc::pnc_change_grantee(self.owner, self.handle, new_grantee) } {
            errno::OK => Ok(()),
            err => Err(Error::from(err).context("change_grantee failed")),
        }
    }
    /// Transfer to a new process the right to execute a successful copy
    ///  between two endpoints.
    ///
    ///  If the current process is not the relevant process to perform
    ///  a copy, it can rely on this call to give up its right and give them
    ///  to another process.
    ///
    ///  This is only allowed if the current process is the configured effector
    ///  for this authorization.
    ///
    /// # Arguments
    ///
    /// - `new_effector`  Identity of the new process which will
    ///                        be granted the right to perform the copy.
    /// # Errors
    /// - `EINVAL`
    ///      If `owner` or `new_effector` are not active processes,
    ///      or if `auth_handle` is not a valid handle on an authorization set by
    ///      `owner`.
    /// - `EPERM`
    ///      If the current process is not the designated effector of
    ///      this authorization.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the CHANGE_EFFECTOR kernel call.
    pub fn change_effector(&mut self, new_effector: s_pid_t) -> Result<()> {
        match unsafe { libpnc::pnc_change_effector(self.owner, self.handle, new_effector) } {
            errno::OK => Ok(()),
            err => Err(Error::from(err).context("change_effector failed")),
        }
    }
    /// Transfer the right to revoke an authorization to a new process.
    ///
    ///  If the current process is not the relevant process to clean up an
    ///  authorization, it can rely on this call to give up its right to revoke,
    ///  and give it to another process.
    ///
    ///  This is only allowed if the current process is the configured revoker
    ///  for this authorization.
    ///
    /// # Arguments
    ///
    /// - `new_revoker`   Identity of the new process which will
    ///                        be granted the right to revoke the authorization.
    /// # Errors
    /// - `EINVAL`
    ///      If `owner` or `new_revoker` are not active processes,
    ///      or if `auth_handle` is not a valid handle on an authorization set by
    ///      `owner`.
    /// - `EPERM`
    ///      If the current process is not the designated revoker of
    ///      this authorization.
    /// - `ECALLDENIED`
    ///      If the configuration is preventing the current
    ///      process from performing the CHANGE_REVOKER kernel call.
    pub fn change_revoker(&mut self, new_revoker: s_pid_t) -> Result<()> {
        match unsafe { libpnc::pnc_change_revoker(self.owner, self.handle, new_revoker) } {
            errno::OK => Ok(()),
            err => Err(Error::from(err).context("change_revoker failed")),
        }
    }
    /// Return the underlying raw handle of the authorization
    pub fn raw_handle(&self) -> c_uint {
        self.handle
    }
    /// Return the owner of the authorization we are wrapping around
    pub fn owner(&self) -> s_pid_t {
        self.owner
    }
}
