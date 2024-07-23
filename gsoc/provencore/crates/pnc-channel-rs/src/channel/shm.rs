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

//! Shared memory based channel
//!
//!```
//!    | source          |               | target            |
//!    |-----------------|               |-------------------|
//!    |                 |               |                   |
//!    | alloc           |               |                   |
//!    |                 |               |                   |
//!    |                 | +--id, len--> |                   |
//!    |                 |               |                   |
//!    |                 |               | register          |
//!    |                 |               |                   |
//!    |                 |               | set_receiver_perm |
//!    |                 |               |                   |
//!    |                 | <-----------+ |                   |
//!    |                 |               |                   |
//!    | set_sender_perm |               |                   |
//!    |                 |               |                   |
//!    | transfer        |               |                   |
//!    |                 |               |                   |
//!    |                 | +-----------> |                   |
//!```

use crate::channel_builder;
use crate::{BytesReceiver, BytesSender, PidBuilder};
use alloc::vec::Vec;
use pnc::{error::Result, s_pid_t, Message, SharedMem, NO_TIMEOUT};

/// Create a shared memory based bytes channel
pub fn bytes_channel() -> (PidBuilder<impl BytesSender>, PidBuilder<impl BytesReceiver>) {
    (
        <ShmSender as BytesSender>::builder(),
        <ShmReceiver as BytesReceiver>::builder(),
    )
}

#[cfg(all(feature = "serde", feature = "postcard"))]
channel_builder!(
    /// Create a shared memory based Postcard channel
    postcard_channel,
    crate::transport::postcard::PostcardSender<ShmSender>,
    crate::transport::postcard::PostcardReceiver<ShmReceiver>
);

#[cfg(all(feature = "serde", feature = "serde_json"))]
channel_builder!(
    /// Create a shared memory based JSON channel
    json_channel,
    crate::transport::json::JsonSender<ShmSender>,
    crate::transport::json::JsonReceiver<ShmReceiver>
);

#[cfg(all(feature = "serde", feature = "serde_cbor"))]
channel_builder!(
    /// Create a shared memory based CBOR channel
    cbor_channel,
    crate::transport::cbor::CborSender<ShmSender>,
    crate::transport::cbor::CborReceiver<ShmReceiver>
);

/// `M_TYPE` field value of the messages used by the channel interface
pub const M_TYPE: u32 = 9003;

/// Number of pages we will need to store a buffer or size `len`
const fn page_count(len: usize) -> usize {
    // ceiling division
    (len + 4096 - 1) / 4096
}

struct ShmSender {
    target: s_pid_t,
}
impl BytesSender for ShmSender {
    fn builder() -> fn(me: s_pid_t, other: s_pid_t) -> Self {
        |_me, other| Self { target: other }
    }
    fn send(&self, val: &[u8]) -> Result<()> {
        let mut shm = SharedMem::alloc(page_count(val.len()) as u32)?;

        let shm_buf = shm.as_buf_mut();
        shm_buf[..val.len()].copy_from_slice(val);

        Message::new(M_TYPE, shm.id(), val.len() as u64, 0, 0, 0, 0).sendrec(self.target)?;

        shm.set_sender_perm(self.target)?;
        shm.transfer(self.target)?;

        Message::new(M_TYPE, 0, 0, 0, 0, 0, 0).send(self.target)?;

        Ok(())
    }
}

struct ShmReceiver {
    source: s_pid_t,
}
impl BytesReceiver for ShmReceiver {
    fn builder() -> fn(me: s_pid_t, other: s_pid_t) -> Self {
        |_me, other| Self { source: other }
    }
    fn recv(&self) -> Result<Vec<u8>> {
        let msg = Message::receive(self.source, NO_TIMEOUT)?;
        let shm_id = msg.m1();
        let len = msg.m2() as usize;

        let mut shm = SharedMem::register(shm_id)?;
        shm.set_receiver_perm(self.source)?;

        Message::new(M_TYPE, 0, 0, 0, 0, 0, 0).sendrec(self.source)?;

        Ok(shm.as_buf()[..len].to_vec())
    }
}
