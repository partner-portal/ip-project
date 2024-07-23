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

#![no_std]

//! An inter-process channel crate for provencore
//!
//! `pnc_channel` provides an agnostic interface to multiple methods of doing IPC in provencore and
//! transparent serialization with multiple transport formats.
//!
//! For example:
//! ```
//! use pnc_channel::message;
//!
//! let (tx, rx) = message::bytes_channel();
//! // ...
//!
//! // In the sender
//! let data = b"hello";
//! let tx = tx(me, other);
//! tx.send(&data)?;
//!
//! // In the receiver
//! let rx = rx(me, other);
//! let received: Vec<u8> = rx.recv()?;
//! ```
//! ```
//! use pnc_channel::vircopy;
//!
//! let (tx, rx) = vircopy::json_channel::<Vec<u64>>();
//! // ...
//!
//! // In the sender
//! let data = vec![1, 2, 3, 4, 5];
//! let tx = tx(me, other);
//! tx.send(&data)?;
//!
//! // In the receiver
//! let rx = rx(me, other);
//! let received: Vec<u64> = rx.recv()?;
//! ```
extern crate alloc;

mod channel;
mod transport;

pub use channel::*;

use alloc::vec::Vec;
use pnc::{error::Result, s_pid_t};

/// Builder type for channel handles
///
/// It takes your process identifier and that of the process on the other side of the channel.
pub type PidBuilder<T> = fn(me: s_pid_t, other: s_pid_t) -> T;

/// Channel handle to send bytes
pub trait BytesSender {
    fn builder() -> PidBuilder<Self>;
    fn send(&self, buf: &[u8]) -> Result<()>;
}

/// Channel handle to receive bytes
pub trait BytesReceiver {
    fn builder() -> PidBuilder<Self>;
    fn recv(&self) -> Result<Vec<u8>>;
}

/// Channel handle to send a serializable type
#[cfg(all(feature = "serde"))]
pub trait Sender<T>
where
    T: serde::Serialize,
{
    fn builder() -> PidBuilder<Self>;
    fn send(&self, buf: &T) -> Result<()>;
}

/// Channel handle to receive a serializable type
#[cfg(all(feature = "serde"))]
pub trait Receiver<T>
where
    T: for<'de> serde::Deserialize<'de>,
{
    fn builder() -> PidBuilder<Self>;
    fn recv(&self) -> Result<T>;
}

#[doc(hidden)]
#[macro_export]
macro_rules! channel_builder {
    (
        $(#[$meta:meta])*
        $name:ident, $sender_transport:path, $receiver_transport:path) => {
        $(#[$meta])*
        pub fn $name<T>() -> (
            PidBuilder<impl $crate::Sender<T>>,
            PidBuilder<impl $crate::Receiver<T>>,
        )
        where
            T: serde::Serialize + for<'de> serde::Deserialize<'de>,
        {
            (
                <$sender_transport as $crate::Sender<T>>::builder(),
                <$receiver_transport as $crate::Receiver<T>>::builder(),
            )
        }
    };
}
