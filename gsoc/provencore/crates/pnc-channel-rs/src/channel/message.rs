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

//! Message based channel that transfers bytes by packing them into a series of IPC messages
//!
//!```
//! | source  |               | target  |
//! |---------|               |---------|
//! |         |               |         |
//! |         | +----len----> |         |
//! |         |               |         |
//! |         | +-----------> |         |
//! |         | +-----------> |         |
//! |         | +-----------> |         |
//! |         |      ...      |         |
//! |         | +-----------> |         |
//!```

use crate::channel_builder;
use crate::{BytesReceiver, BytesSender, PidBuilder};
use alloc::vec::Vec;
use pnc::{error::Result, s_pid_t, Message, NO_TIMEOUT};

/// Create a message based bytes channel
pub fn bytes_channel() -> (PidBuilder<impl BytesSender>, PidBuilder<impl BytesReceiver>) {
    (
        <MsgSender as BytesSender>::builder(),
        <MsgReceiver as BytesReceiver>::builder(),
    )
}

#[cfg(all(feature = "serde", feature = "postcard"))]
channel_builder!(
    /// Create a message based Postcard channel
    postcard_channel,
    crate::transport::postcard::PostcardSender<MsgSender>,
    crate::transport::postcard::PostcardReceiver<MsgReceiver>
);

#[cfg(all(feature = "serde", feature = "serde_json"))]
channel_builder!(
    /// Create a message based JSON channel
    json_channel,
    crate::transport::json::JsonSender<MsgSender>,
    crate::transport::json::JsonReceiver<MsgReceiver>
);

#[cfg(all(feature = "serde", feature = "serde_cbor"))]
channel_builder!(
    /// Create a message based CBOR channel
    cbor_channel,
    crate::transport::cbor::CborSender<MsgSender>,
    crate::transport::cbor::CborReceiver<MsgReceiver>
);

/// `M_TYPE` field value of the messages used by the channel interface
pub const M_TYPE: u32 = 9002;

/// Amount of bytes we expect to transport in a single `Message`
const MESSAGE_LEN: usize = 36;
/// Number of messages we will need to transport a buffer or size `len`
const fn message_count(len: usize) -> usize {
    // ceiling division
    (len + MESSAGE_LEN - 1) / MESSAGE_LEN
}

struct MsgSender {
    target: s_pid_t,
}
impl MsgSender {
    fn buf_to_message(buf: [u8; MESSAGE_LEN]) -> Message {
        let mut m1 = [0u8; 4];
        m1.copy_from_slice(&buf[0..4]);
        let mut m2 = [0u8; 8];
        m2.copy_from_slice(&buf[4..12]);
        let mut m3 = [0u8; 4];
        m3.copy_from_slice(&buf[12..16]);
        let mut m4 = [0u8; 4];
        m4.copy_from_slice(&buf[16..20]);
        let mut m5 = [0u8; 8];
        m5.copy_from_slice(&buf[20..28]);
        let mut m6 = [0u8; 8];
        m6.copy_from_slice(&buf[28..36]);
        Message::new(
            M_TYPE,
            u32::from_be_bytes(m1),
            u64::from_be_bytes(m2),
            u32::from_be_bytes(m3),
            u32::from_be_bytes(m4),
            u64::from_be_bytes(m5),
            u64::from_be_bytes(m6),
        )
    }
}
impl BytesSender for MsgSender {
    fn builder() -> fn(me: s_pid_t, other: s_pid_t) -> Self {
        |_me, other| Self { target: other }
    }
    fn send(&self, val: &[u8]) -> Result<()> {
        Message::new(M_TYPE, 0, val.len() as u64, 0, 0, 0, 0).send(self.target)?;

        let mut chunks: Vec<&[u8]> = val.chunks(MESSAGE_LEN).collect();

        // pad last chunk
        let last = chunks.last_mut().unwrap();
        let mut buf = Vec::from(*last);
        buf.resize(MESSAGE_LEN, 0u8);
        *last = buf.as_slice();

        for chunk in chunks {
            let mut buf = [0u8; 36];
            buf.copy_from_slice(chunk);
            let msg = MsgSender::buf_to_message(buf);
            msg.send(self.target)?
        }

        Ok(())
    }
}

struct MsgReceiver {
    source: s_pid_t,
}
impl MsgReceiver {
    fn message_to_buf(msg: Message) -> Vec<u8> {
        let fields: [&[u8]; 6] = [
            &msg.m1().to_be_bytes(),
            &msg.m2().to_be_bytes(),
            &msg.m3().to_be_bytes(),
            &msg.m4().to_be_bytes(),
            &msg.m5().to_be_bytes(),
            &msg.m6().to_be_bytes(),
        ];
        fields.concat()
    }
}

impl BytesReceiver for MsgReceiver {
    fn builder() -> fn(me: s_pid_t, other: s_pid_t) -> Self {
        |_me, other| Self { source: other }
    }
    fn recv(&self) -> Result<Vec<u8>> {
        let len = Message::receive(self.source, NO_TIMEOUT)?.m2() as usize;
        let msg_count = message_count(len);

        let mut messages = Vec::with_capacity(msg_count);
        for _ in 0..msg_count {
            messages.push(Message::receive(self.source, NO_TIMEOUT)?);
        }

        let mut buf = messages
            .into_iter()
            .map(MsgReceiver::message_to_buf)
            .collect::<Vec<_>>()
            .concat();
        buf.truncate(len);

        Ok(buf)
    }
}
