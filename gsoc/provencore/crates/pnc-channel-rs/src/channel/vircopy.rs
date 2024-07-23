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

//! Copy based channel that transfers bytes using `vircopy`
//!
//!```
//! | source  |               | target  |
//! |---------|               |---------|
//! |         |               |         |
//! |         | +----len----> |         |
//! |         |               |         |
//! |         |               | setauth |
//! |         |               |         |
//! |         | <----ptr----+ |         |
//! |         |               |         |
//! | setauth |               |         |
//! |         |               |         |
//! | vircopy |               |         |
//! |         |               |         |
//! |         | +-----------> |         |
//!```

use crate::channel_builder;
use crate::{BytesReceiver, BytesSender, PidBuilder};
use alloc::vec;
use alloc::vec::Vec;
use pnc::{error::Result, s_pid_t, vircopy, AuthGuard, AuthMode, Message, NO_TIMEOUT};

/// Create a copy based bytes channel
pub fn bytes_channel() -> (PidBuilder<impl BytesSender>, PidBuilder<impl BytesReceiver>) {
    (
        <VircopySender as BytesSender>::builder(),
        <VircopyReceiver as BytesReceiver>::builder(),
    )
}

#[cfg(all(feature = "serde", feature = "postcard"))]
channel_builder!(
    /// Create a copy based Postcard channel
    postcard_channel,
    crate::transport::postcard::PostcardSender<VircopySender>,
    crate::transport::postcard::PostcardReceiver<VircopyReceiver>
);

#[cfg(all(feature = "serde", feature = "serde_json"))]
channel_builder!(
    /// Create a copy based JSON channel
    json_channel,
    crate::transport::json::JsonSender<VircopySender>,
    crate::transport::json::JsonReceiver<VircopyReceiver>
);

#[cfg(all(feature = "serde", feature = "serde_cbor"))]
channel_builder!(
    /// Create a copy based CBOR channel
    cbor_channel,
    crate::transport::cbor::CborSender<VircopySender>,
    crate::transport::cbor::CborReceiver<VircopyReceiver>
);

/// `M_TYPE` field value of the messages used by the channel interface
pub const M_TYPE: u32 = 9001;

struct VircopySender {
    source: s_pid_t,
    target: s_pid_t,
}
impl BytesSender for VircopySender {
    fn builder() -> fn(me: s_pid_t, other: s_pid_t) -> Self {
        |me, other| Self {
            source: me,
            target: other,
        }
    }
    fn send(&self, val: &[u8]) -> Result<()> {
        let _auth = AuthGuard::set(AuthMode::Read, self.target, val, self.source)?;

        let mut msg = Message::new(M_TYPE, 0, val.len() as u64, 0, 0, 0, 0);
        msg.sendrec(self.target)?;
        let dst_vaddr = msg.m2() as *mut u8;
        let src_vaddr = val.as_ptr();
        let size = val.len();

        vircopy(self.source, src_vaddr, self.target, dst_vaddr, size)?;

        Message::new(M_TYPE, 0, 0, 0, 0, 0, 0).send(self.target)?;

        Ok(())
    }
}

struct VircopyReceiver {
    source: s_pid_t,
}
impl BytesReceiver for VircopyReceiver {
    fn builder() -> fn(me: s_pid_t, other: s_pid_t) -> Self {
        |_me, other| Self { source: other }
    }
    fn recv(&self) -> Result<Vec<u8>> {
        let len = Message::receive(self.source, NO_TIMEOUT)?.m2() as usize;

        let mut dst = vec![0; len];
        let dst_vaddr = dst.as_mut_ptr();

        let _auth = AuthGuard::set(AuthMode::Write, self.source, dst.as_slice(), self.source)?;

        Message::new(M_TYPE, 0, dst_vaddr as u64, 0, 0, 0, 0).sendrec(self.source)?;

        Ok(dst)
    }
}
