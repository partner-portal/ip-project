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
use crate::libpnc::{self, errno, s_pid_t};

// TODO SL -> JS: Don't we need to export the constant
//  values for HARDWARE, PM_ID and the source flags as well?

/// Identity of the HARDWARE task which is the source of userland
/// interrupt notifications.
pub const HARDWARE: s_pid_t = 16;

/// Constant PIDS for kernel tasks
pub const PM_ID: s_pid_t = 8;

/// Constant to execute an IPC call without any timeout
pub const NO_TIMEOUT: u64 = core::u64::MAX;

/// Safe IPC message interface
#[derive(Debug)]
pub struct Message {
    msg: libpnc::message,
}

impl Message {
    /// Create a new message filled with zeroes
    pub fn empty() -> Self {
        Self {
            msg: libpnc::message {
                m_source: 0,
                uptime: 0,
                m_type: 0,
                m1: 0,
                m2: 0,
                m3: 0,
                m4: 0,
                m5: 0,
                m6: 0,
            },
        }
    }
    /// Create a new message from field values
    pub fn new(m_type: u32, m1: u32, m2: u64, m3: u32, m4: u32, m5: u64, m6: u64) -> Self {
        Self {
            msg: libpnc::message {
                m_source: 0,
                uptime: 0,
                m_type,
                m1,
                m2,
                m3,
                m4,
                m5,
                m6,
            },
        }
    }
    /// Wrap a new message from an existing underlying `libpnc::message` structure
    pub fn from_raw(msg: libpnc::message) -> Self {
        Self { msg }
    }

    /// Send a message and wait for the reception of the anwser.
    ///
    /// Note that no timeout on the receive part of the IPC is available.
    ///
    /// # Arguments
    ///
    /// * `target` Identify of the process/task to contact
    ///
    /// # Errors
    ///
    /// - `ETRAPDENIED` If the current process is not allowed by the configuration to perform the IPC.
    /// - `EBADSRCDST` If target is not a valid process identifier.
    /// - `EDSTDIED` If target has unexpectedly stopped working
    ///   (e.g. if it exits with some pending send requests) during the SEND part of the call.
    /// - `ESRCDIED` If target has unexpectedly stopped working
    ///   (e.g. if it exits with some pending receive requests) during the RECEIVE part of the call.
    /// - `EDSTDENIED` If target is neither in domain 0 nor in the same domain as the caller,
    ///   or if the configuration is preventing the caller from sending messages to target.
    /// - `ELOCKED` If the action would lead to a deadlock situation,
    ///   for example if target is currently sending to the current process.
    /// - `EMSGBUF` If the message pointed by msg is not valid.
    /// - `ENOMEM` If the current message is stored in a chunk of Copy on Write memory,
    ///   and there is not enough memory left to resolve the situation.
    /// - `ECALLDENIED` In case of a kernel call, if the configuration is preventing
    ///   the current process from performing the requested kernel call.
    /// - `EBADREQUEST` In case of a kernel call, if the kernel call requested by the current
    ///   process is an invalid one.
    pub fn sendrec(&mut self, target: s_pid_t) -> Result<()> {
        match unsafe { libpnc::pnc_rs_sendrec(target, &mut self.msg) } {
            errno::OK => Ok(()),
            err => Err(Error::from(err).context("sendrec failed")),
        }
    }
    /// Send a message immediately and wait for the reception of the anwser.
    ///
    /// Returns `Ok(true)` if the message was successfully sent and `Ok(false)` if the message
    /// couldn't be sent immediately because the target wasn't ready to acknowledge
    ///
    /// # Arguments
    ///
    /// * `target` Identity of the process/task to contact
    ///
    /// # Errors
    ///
    /// - `ETRAPDENIED` If the current process is not allowed by the configuration to perform the IPC.
    /// - `EBADSRCDST` If target is not a valid process identifier.
    /// - `EDSTDIED` If target has unexpectedly stopped working
    ///   (e.g. if it exits with some pending send requests) during the SEND part of the call.
    /// - `ESRCDIED` If target has unexpectedly stopped working
    ///   (e.g. if it exits with some pending receive requests) during the RECEIVE part of the call.
    /// - `EDSTDENIED` If target is neither in domain 0 nor in the same domain as the caller,
    ///   or if the configuration is preventing the caller from sending messages to target.
    /// - `ELOCKED` If the action would lead to a deadlock situation,
    ///   for example if target is currently sending to the current process.
    /// - `EMSGBUF` If the message pointed by msg is not valid.
    /// - `ENOMEM` If the current message is stored in a chunk of Copy on Write memory,
    ///   and there is not enough memory left to resolve the situation.
    /// - `ECALLDENIED` In case of a kernel call, if the configuration is preventing
    ///   the current process from performing the requested kernel call.
    /// - `EBADREQUEST` In case of a kernel call, if the kernel call requested by the current
    ///   process is an invalid one.
    pub fn sendrec_nonblock(&mut self, target: s_pid_t) -> Result<bool> {
        match unsafe { libpnc::pnc_rs_sendrec_nonblock(target, &mut self.msg) } {
            errno::OK => Ok(true),
            errno::ENOTREADY => Ok(false),
            err => Err(Error::from(err).context("sendrec_nonblock failed")),
        }
    }

    /// Send a message and block until it's acknowledged.
    ///
    /// # Arguments
    ///
    /// * `target` Identity of the process/task to contact
    ///
    /// # Errors
    ///
    /// - `ETRAPDENIED`
    /// If the current process is not allowed by the configuration to perform the IPC.
    /// - `EBADSRCDST`
    /// If target is not a valid process identifier.
    /// - `EDSTDIED`
    /// If target has unexpectedly stopped working
    /// (e.g. if it exits with some pending send requests).
    /// - `EDSTDENIED`
    /// If target is neither in domain 0 nor in the same domain as the caller, or if the
    /// configuration is preventing the caller from sending messages to target.
    /// - `ELOCKED`
    /// If the action would lead to a deadlock situation, for example if target is currently
    /// sending to the current process.
    /// - `EMSGBUF`
    /// If the message pointed by msg is not valid.
    /// - `ENOMEM`
    /// If the current message is stored in a chunk of Copy on Write memory, and there is not
    /// enough memory left to resolve the situation.
    pub fn send(&self, target: s_pid_t) -> Result<()> {
        match unsafe { libpnc::pnc_rs_send(target, &self.msg) } {
            errno::OK => Ok(()),
            err => Err(Error::from(err).context("send failed")),
        }
    }
    /// Send a message immediately without blocking.
    ///
    /// Returns `Ok(true)` if the message was successfully sent and `Ok(false)` if the message
    /// couldn't be sent immediately because the target wasn't ready to acknowledge
    ///
    /// # Arguments
    ///
    /// * `target` Identity of the process/task to contact
    ///
    /// # Errors
    ///
    /// - `ETRAPDENIED`
    /// If the current process is not allowed by the configuration to perform the IPC.
    /// - `EBADSRCDST`
    /// If target is not a valid process identifier.
    /// - `EDSTDIED`
    /// If target has unexpectedly stopped working
    /// (e.g. if it exits with some pending send requests).
    /// - `EDSTDENIED`
    /// If target is neither in domain 0 nor in the same domain as the caller, or if the
    /// configuration is preventing the caller from sending messages to target.
    /// - `ELOCKED`
    /// If the action would lead to a deadlock situation, for example if target is currently
    /// sending to the current process.
    /// - `EMSGBUF`
    /// If the message pointed by msg is not valid.
    /// - `ENOMEM`
    /// If the current message is stored in a chunk of Copy on Write memory, and there is not
    /// enough memory left to resolve the situation.
    pub fn send_nonblock(&self, target: s_pid_t) -> Result<bool> {
        match unsafe { libpnc::pnc_rs_send_nonblock(target, &self.msg) } {
            errno::OK => Ok(true),
            errno::ENOTREADY => Ok(false),
            err => Err(Error::from(err).context("send_nonblock failed")),
        }
    }
    /// Receive a message from a specific source while blocking with possible timeout.
    ///
    /// # Arguments
    ///
    /// * `source` Identity of the process from whom a message is expected.
    /// * `timeout` Time to wait for the message in microseconds of `NO_TIMEOUT` to wait forever
    ///
    /// # Errors
    ///
    /// - `ETRAPDENIED`
    /// If the current process is not allowed by the configuration to perform the IPC.
    /// - `EBADSRCDST`
    /// If target is not a valid process identifier.
    /// - `ESRCDIED`
    /// If target has unexpectedly stopped working
    /// (e.g. if it exits with some pending receive requests).
    /// - `EDSTDENIED`
    /// If target is neither in domain 0 nor in the same domain as the caller.
    /// - `ELOCKED`
    /// If the action would lead to a deadlock situation, for example if target is currently
    /// waiting to receive from the current process.
    /// - `EMSGBUF`
    /// If the message pointed by msg is not valid.
    /// - `ENOMEM`
    /// If the current message is stored in a chunk of Copy on Write memory,
    /// and there is not enough memory left to resolve the situation.
    /// - `ETIMEDOUT`
    /// If timeout was not `NO_TIMEOUT` and it has expired.
    pub fn receive(source: s_pid_t, timeout: u64) -> Result<Message> {
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
        match unsafe { libpnc::pnc_rs_receive(source, &mut msg, timeout) } {
            errno::OK => Ok(Message { msg }),
            err => Err(Error::from(err).context("receive failed")),
        }
    }
    /// Receive a message from any source while blocking with possible timeout.
    ///
    /// # Arguments
    ///
    /// * `timeout` Time to wait for the message in microseconds of `NO_TIMEOUT` to wait forever
    ///
    /// # Errors
    ///
    /// - `ETRAPDENIED`
    /// If the current process is not allowed by the configuration to perform the IPC.
    /// - `EBADSRCDST`
    /// If target is not a valid process identifier.
    /// - `ESRCDIED`
    /// If target has unexpectedly stopped working
    /// (e.g. if it exits with some pending receive requests).
    /// - `EDSTDENIED`
    /// If target is neither in domain 0 nor in the same domain as the caller.
    /// - `ELOCKED`
    /// If the action would lead to a deadlock situation, for example if target is currently
    /// waiting to receive from the current process.
    /// - `EMSGBUF`
    /// If the message pointed by msg is not valid.
    /// - `ENOMEM`
    /// If the current message is stored in a chunk of Copy on Write memory,
    /// and there is not enough memory left to resolve the situation.
    /// - `ETIMEDOUT`
    /// If timeout was not `NO_TIMEOUT` and it has expired.
    pub fn receive_any(timeout: u64) -> Result<Message> {
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
        match unsafe { libpnc::pnc_rs_receive_any(&mut msg, timeout) } {
            errno::OK => Ok(Message { msg }),
            err => Err(Error::from(err).context("receive_any failed")),
        }
    }
    /// Receive a message from `HARDWARE` while blocking with possible timeout.
    ///
    /// # Arguments
    ///
    /// * `timeout` Time to wait for the message in microseconds of `NO_TIMEOUT` to wait forever
    ///
    /// # Errors
    ///
    /// - `ETRAPDENIED`
    /// If the current process is not allowed by the configuration to perform the IPC.
    /// - `EBADSRCDST`
    /// If target is not a valid process identifier.
    /// - `ESRCDIED`
    /// If target has unexpectedly stopped working
    /// (e.g. if it exits with some pending receive requests).
    /// - `EDSTDENIED`
    /// If target is neither in domain 0 nor in the same domain as the caller.
    /// - `ELOCKED`
    /// If the action would lead to a deadlock situation, for example if target is currently
    /// waiting to receive from the current process.
    /// - `EMSGBUF`
    /// If the message pointed by msg is not valid.
    /// - `ENOMEM`
    /// If the current message is stored in a chunk of Copy on Write memory,
    /// and there is not enough memory left to resolve the situation.
    /// - `ETIMEDOUT`
    /// If timeout was not `NO_TIMEOUT` and it has expired.
    pub fn receive_hw(timeout: u64) -> Result<Message> {
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
        match unsafe { libpnc::pnc_rs_receive_hw(&mut msg, timeout) } {
            errno::OK => Ok(Message { msg }),
            err => Err(Error::from(err).context("receive_hw failed")),
        }
    }
    /// Receive a message from a specific source immediately without blocking.
    ///
    /// Will return `Ok(None)` is no message is immediately available,
    ///
    /// # Arguments
    ///
    /// * `source` Identity of the process from whom a message is expected.
    ///
    /// # Errors
    ///
    /// - `ETRAPDENIED`
    /// If the current process is not allowed by the configuration to perform the IPC.
    /// - `EBADSRCDST`
    /// If target is not a valid process identifier.
    /// - `ESRCDIED`
    /// If target has unexpectedly stopped working
    /// (e.g. if it exits with some pending receive requests).
    /// - `EDSTDENIED`
    /// If target is neither in domain 0 nor in the same domain as the caller.
    /// - `ELOCKED`
    /// If the action would lead to a deadlock situation, for example if target is currently
    /// waiting to receive from the current process.
    /// - `EMSGBUF`
    /// If the message pointed by msg is not valid.
    /// - `ENOMEM`
    /// If the current message is stored in a chunk of Copy on Write memory,
    /// and there is not enough memory left to resolve the situation.
    pub fn receive_nonblock(source: s_pid_t) -> Result<Option<Message>> {
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
        match unsafe { libpnc::pnc_rs_receive_nonblock(source, &mut msg) } {
            errno::OK => Ok(Some(Message { msg })),
            errno::ENOTREADY => Ok(None),
            err => Err(Error::from(err).context("receive_nonblock failed")),
        }
    }
    /// Receive a message from any source immediately without blocking.
    ///
    /// Will return `Ok(None)` is no message is immediately available.
    ///
    /// # Arguments
    ///
    /// * `source` Identity of the process from whom a message is expected.
    ///
    /// # Errors
    ///
    /// - `ETRAPDENIED`
    /// If the current process is not allowed by the configuration to perform the IPC.
    /// - `EBADSRCDST`
    /// If target is not a valid process identifier.
    /// - `ESRCDIED`
    /// If target has unexpectedly stopped working
    /// (e.g. if it exits with some pending receive requests).
    /// - `EDSTDENIED`
    /// If target is neither in domain 0 nor in the same domain as the caller.
    /// - `ELOCKED`
    /// If the action would lead to a deadlock situation, for example if target is currently
    /// waiting to receive from the current process.
    /// - `EMSGBUF`
    /// If the message pointed by msg is not valid.
    /// - `ENOMEM`
    /// If the current message is stored in a chunk of Copy on Write memory,
    /// and there is not enough memory left to resolve the situation.
    pub fn receive_any_nonblock() -> Result<Option<Message>> {
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
        match unsafe { libpnc::pnc_rs_receive_any_nonblock(&mut msg) } {
            errno::OK => Ok(Some(Message { msg })),
            errno::ENOTREADY => Ok(None),
            err => Err(Error::from(err).context("receive_any_nonblock failed")),
        }
    }
    /// Receive a message from the `HARDWARE` source immediately without blocking.
    ///
    /// Will return `Ok(None)` is no message is immediately available.
    ///
    /// # Arguments
    ///
    /// * `source` Identity of the process from whom a message is expected.
    ///
    /// # Errors
    ///
    /// - `ETRAPDENIED`
    /// If the current process is not allowed by the configuration to perform the IPC.
    /// - `EBADSRCDST`
    /// If target is not a valid process identifier.
    /// - `ESRCDIED`
    /// If target has unexpectedly stopped working
    /// (e.g. if it exits with some pending receive requests).
    /// - `EDSTDENIED`
    /// If target is neither in domain 0 nor in the same domain as the caller.
    /// - `ELOCKED`
    /// If the action would lead to a deadlock situation, for example if target is currently
    /// waiting to receive from the current process.
    /// - `EMSGBUF`
    /// If the message pointed by msg is not valid.
    /// - `ENOMEM`
    /// If the current message is stored in a chunk of Copy on Write memory,
    /// and there is not enough memory left to resolve the situation.
    pub fn receive_hw_nonblock() -> Result<Option<Message>> {
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
        match unsafe { libpnc::pnc_rs_receive_hw_nonblock(&mut msg) } {
            errno::OK => Ok(Some(Message { msg })),
            errno::ENOTREADY => Ok(None),
            err => Err(Error::from(err).context("receive_hw_nonblock failed")),
        }
    }

    /// Bit mask for notification messages
    pub const MSOURCE_NOTIFY_FLAG: u64 = 1 << 63;
    /// Bit mask for messages that expect an answer
    pub const MSOURCE_RECEIVE_FLAG: u64 = 1 << 62;
    /// Mask to get process id from an m_source
    pub const MSOURCE_MASK: u64 = !(Message::MSOURCE_NOTIFY_FLAG | Message::MSOURCE_RECEIVE_FLAG);

    /// Returns true if the message is a notification
    pub fn is_ipc_notify(&self) -> bool {
        (self.msg.m_source & Message::MSOURCE_NOTIFY_FLAG) != 0
    }

    /// Returns true if the message expects an answer
    pub fn is_ipc_with_receive(&self) -> bool {
        (self.msg.m_source & Message::MSOURCE_RECEIVE_FLAG) != 0
    }

    /// Returns true if the message comes from a `SENDREC`
    pub fn is_ipc_sendrec(&self) -> bool {
        !self.is_ipc_notify() && self.is_ipc_with_receive()
    }

    /// Returns true if the message comes from a `SEND`
    pub fn is_ipc_send(&self) -> bool {
        !self.is_ipc_notify() && !self.is_ipc_with_receive()
    }

    /// Get the id of the process who sent this message and control bits
    pub fn m_source(&self) -> u64 {
        self.msg.m_source
    }

    /// Get the id of the process who sent this message
    pub fn source(&self) -> s_pid_t {
        self.msg.m_source & Message::MSOURCE_MASK
    }

    /// Get the uptime in microseconds at the reception of the message
    pub fn uptime(&self) -> u64 {
        self.msg.uptime
    }
    /// Get the application defined message type
    pub fn m_type(&self) -> u32 {
        self.msg.m_type
    }
    /// Get the `m1` message field
    pub fn m1(&self) -> u32 {
        self.msg.m1
    }
    /// Get the `m2` message field
    pub fn m2(&self) -> u64 {
        self.msg.m2
    }
    /// Get the `m3` message field
    pub fn m3(&self) -> u32 {
        self.msg.m3
    }
    /// Get the `m4` message field
    pub fn m4(&self) -> u32 {
        self.msg.m4
    }
    /// Get the `m5` message field
    pub fn m5(&self) -> u64 {
        self.msg.m5
    }
    /// Get the `m6` message field
    pub fn m6(&self) -> u64 {
        self.msg.m6
    }
}

/// Stop the execution for some time.
///
/// # Arguments
///
/// * `timeout` Time limit to wait in µs. Must be different from `core::u64::MAX`
///
/// # Errors
///
/// - `ETRAPDENIED` If the current process is not allowed by the configuration to perform
/// the IPC, or if timeout is set to `NO_TIMEOUT`.
pub fn sleep(timeout: u64) -> Result<()> {
    match unsafe { libpnc::pnc_rs_sleep(timeout) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("sleep failed")),
    }
}

/// Send a notification to another process
///
/// This call is non-blocking: the current process will resume execution even
/// if the target is not receiving at the moment.
///
/// Unlike a message, a notification has no payload attached so no other information can be
/// convayed to the target than the notification. While receiving from any source the special
/// source `HARDWARE` can appear, meaning that some registered interrupt have happened.
///
/// # Arguments
///
/// - `target` the process to notify
///
/// # Errors
///
/// - `ETRAPDENIED`
/// If the current process is not allowed by the configuration to perform the IPC.
///
/// - `EBADSRCDST`
/// If target is not a valid process identifier.
///
/// - `EDSTDENIED`
/// If target is neither in domain 0 nor in the same domain as the caller, or if the
/// configuration is preventing the caller from sending notifications to target.
///
pub fn notify(target: s_pid_t) -> Result<()> {
    match unsafe { libpnc::pnc_rs_notify(target) } {
        errno::OK => Ok(()),
        err => Err(Error::from(err).context("notify failed")),
    }
}
