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

//! CBOR Transport using `serde_cbor` for serialization

use crate::{BytesReceiver, BytesSender, Receiver, Sender};
use pnc::{
    error::{Error, Result},
    format, s_pid_t,
};

pub struct CborSender<S: BytesSender> {
    inner: S,
}

impl<T, S> Sender<T> for CborSender<S>
where
    T: serde::Serialize,
    S: BytesSender,
{
    fn builder() -> fn(me: s_pid_t, other: s_pid_t) -> Self {
        |me, other| CborSender {
            inner: S::builder()(me, other),
        }
    }
    fn send(&self, val: &T) -> Result<()> {
        let buf = serde_json::to_vec(val)
            .map_err(|e| Error::msg(&format!("CBOR Serialization failed: {}", e)))?;
        self.inner.send(&buf)
    }
}

pub struct CborReceiver<R: BytesReceiver> {
    inner: R,
}

impl<T, R> Receiver<T> for CborReceiver<R>
where
    T: for<'de> serde::Deserialize<'de>,
    R: BytesReceiver,
{
    fn builder() -> fn(me: s_pid_t, other: s_pid_t) -> Self {
        |me, other| CborReceiver {
            inner: R::builder()(me, other),
        }
    }
    fn recv(&self) -> Result<T> {
        let buf = self.inner.recv()?;
        serde_json::from_slice(&buf)
            .map_err(|e| Error::msg(&format!("CBOR Deserialization failed: {}", e)))
    }
}
