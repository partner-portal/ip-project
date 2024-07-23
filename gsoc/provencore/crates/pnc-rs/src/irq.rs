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
use crate::libc::c_uint;
use crate::libpnc::{self, errno};

/// IRQ reenabling policy to be used with `irq_set_policy`
///
///  This distinction is particularly important when dealing with level-sensitive
///  interrupts (as opposed to edge-triggered). As the interrupt remains pending
///  until the source of the interrupt has been cleared (e.g. by clearing an
///  `INTR` in a device control register), the application will be immediately
///  preempted if the policy is `IRQ_REENABLE_YES`.
pub enum IrqPolicy {
    /// `IRQ_REENABLE_YES` the interrupt will be in state `IRQ_ENABLED`
    /// on reception on the notification.
    ReenableYes = 0,
    /// `IRQ_REENABLE_NO` the interrupt will be in state `IRQ_DISABLED` on reception on the
    /// notification, and the process will have to re-enable the interrupt with `irq_enable`
    /// once it has finished processing the interrupt.
    ReenableNo = 1,
}

/// IRQ states to be used with `irq_set_policy`
pub enum IrqStatus {
    /// `IRQ_ENABLED` the interrupt is enabled and the process will receive notifications
    /// for the IRQ `irq`.
    Enabled = 1,
    ///`IRQ_DISABLED` the interrupt is disabled and the *current* process will not receive
    /// notifications for the IRQ `irq`. Other processes which requests the same interrupt
    /// can still be notified.
    Disabled = 2,
    /// `IRQ_RELEASED` for platform with TrustZone implemented, the process signals that the
    /// interrupt can be released to the non-secure world, whereas the interrupt is strictly
    /// controlled by ProvenCore otherwise. The interrupt is *actually* released when all
    /// processes who created hooks on the IRQ `irq` configured the interrupt as `IRQ_RELEASED`.
    Released = 3,
}

/// Configure a new interrupt hook.
///
///
///  It requests an handle to the IRQ line `irq` and
///  returns the handle to the created interrupt hook.
///
///  The current process will receive interrupt notifications from `HARDWARE`
///  with the bit `notify_id` set in the notification mask.
///
///  The interrupt hook can be in multiple states, see the documentation for `IrqStatus`
///
///  Depending on the policy, the interrupt will be enabled or disabled
///  before a notification is sent to the process, see the documentation for `IrqPolicy`.
///
///
/// * `irq`           The identifier of the line to which the hook will
///                   be registered. This value is provided by the
///                   documentation of your hardware, and is usually
///                   defined in the generated `device.h` file.
/// * `policy`        Interrupt hook configuration.
/// * `notify_id`     Each IRQ line needs to be setup with a unique notification identifier,
///                   whose value is restricted to the range [0, 31].
///                   These ids are used by the kernel when sending the bitmap of pending IRQs
///                   during the reception of a notification.
/// * `status`        Initial state of the interrupt hook.
///
/// # Errors
/// - `EINVAL`
///      If the given IRQ handle is not a valid identifier, if the requested
///      policy or status is not supported.
/// - `EPERM`
///      If the current process is not a privileged process, or if the current
///      process is not allowed to configure the requested IRQ (meaning the
///      current process is not granted control over the device which
///      generates this interrupt).
/// - `EBUSY`
///      If the required identifier `notify_id` is already in use by the
///      current process.
/// - `ENOSPC`
///      If there were not enough free hooks for this process
///      to be able to register a new one, or if the required IRQ line was
///      already full.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the IRQCTL kernel call.

// TODO: test
pub fn irq_set_policy(
    irq: c_uint,
    policy: IrqPolicy,
    notify_id: c_uint,
    status: IrqStatus,
) -> Result<c_uint> {
    let mut hook_id = 0;
    match unsafe {
        libpnc::pnc_rs_irqsetpolicy(
            irq,
            policy as c_uint,
            notify_id,
            status as c_uint,
            &mut hook_id,
        )
    } {
        errno::OK => Ok(hook_id),
        err => Err(Error::from(err).context("irq_set_policy failed")),
    }
}

/// Remove an existing interrupt hook.
///
///  Delete an existing hook, identified by an IRQ handle `hook_id`. The
///  interrupt hook is automatically moved to the state `IRQ_RELEASED`.
///
/// # Arguments
///
/// * `hook_id`       Handle to the interrupt hook to remove.
///
/// # Errors
/// - `EPERM`
///      If the current process is not a privileged process, or if the current
///      process is trying to access a hook of another process.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the IRQCTL kernel call.

// TODO: test
pub fn irq_rm_policy(hook_id: c_uint) -> Result<()> {
    match unsafe { libpnc::pnc_rs_irqrmpolicy(hook_id) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("irq_rm_policy failed")),
    }
}

/// Update the configuration of an existing IRQ hook.
///
/// # Arguments
///
/// * `hook_id`       Handle to the interrupt hook to update.
/// * `status`        Select the new configuration for the interrupt hook `hook_id`.
///
/// # Errors
/// - `EINVAL`
///      If the requested status is not supported.
/// - `EPERM`
///      If the current process is not a privileged process, or if the current
///      process is trying to access a hook of another process.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the IRQCTL kernel call.

// TODO: test
pub fn irq_status(hook_id: c_uint, status: IrqStatus) -> Result<()> {
    match unsafe { libpnc::pnc_rs_irqstatus(hook_id, status as c_uint) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("irq_status failed")),
    }
}

/// Reconfigure an existing IRQ hook as `IRQ_ENABLED`.
///
/// # Arguments
///
/// * `hook_id`       Handle to the interrupt hook to update.
///
/// # Errors
///
/// - `EPERM`
///      If the current process is not a privileged process, or if the current
///      process is trying to access a hook of another process.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the IRQCTL kernel call.

// TODO: test
pub fn irq_disable(hook_id: c_uint) -> Result<()> {
    match unsafe { libpnc::pnc_rs_irqdisable(hook_id) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("irq_disable failed")),
    }
}

/// Reconfigure an existing IRQ hook as `IRQ_ENABLED`.
///
/// # Arguments
///
/// * `hook_id`       Handle to the interrupt hook to update.
///
/// # Errors
/// - `EPERM`
///      If the current process is not a privileged process, or if the current
///      process is trying to access a hook of another process.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the IRQCTL kernel call.

// TODO: test
pub fn irq_enable(hook_id: c_uint) -> Result<()> {
    match unsafe { libpnc::pnc_rs_irqenable(hook_id) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("irq_enable failed")),
    }
}

/// Reconfigure an existing IRQ hook as `IRQ_RELEASED`.
///
/// # Arguments
///
/// * `hook_id`       Handle to the interrupt hook to update.
///
/// # Errors
///
/// - `EPERM`
///      If the current process is not a privileged process, or if the current
///      process is trying to access a hook of another process.
/// - `ECALLDENIED`
///      If the configuration is preventing the current
///      process from performing the IRQCTL kernel call.

// TODO: test
pub fn irq_release(hook_id: c_uint) -> Result<()> {
    match unsafe { libpnc::pnc_rs_irqrelease(hook_id) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("irq_release failed")),
    }
}
