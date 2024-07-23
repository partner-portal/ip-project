//! Safe Rust wrapper around the ETH IPC's
//!
//! Represent a connection to a process implementing the ETH IPC API
//! Some text here
//!
//! Overview
//! ========
//!
//! Underlying IPC interface
//! ------------------------
//!
//! This library is a client library abstracting Remote Procedure Call over
//! ProvenCore's IPC. The driver is implemented by another process.
//!
//! Performance consideration
//! -------------------------
//!
//! The following element were considered during the desgin of this API.  The
//! following factors are known to be performance critical:
//!
//! ### Reducing context switching
//!
//! The API is designed to enable a client to send and receive several network
//! frame per IPC calls to the ETH driver.
//!
//! On a common Ethernet/IP/UDP or TCP stack, the maximal size of a frame is
//! 1518 bytes (1522 if 801.2 VLAN tagging is used).  This translates to
//! maximal UDP payload of 1448 per frame. Reaching a throughput of 1Gbit/s of
//! user data means handling 86325 frames per seconds.  On a typical filter
//! implementation, each frame might traverse several processes (i.e. being
//! either copied or at least observed by that many processes). A typical
//! figure is 4 processes traversal per frame.  At that rate, the dominent
//! performance factor is the overhead induced by context switchwes. Hence the
//! need to armotize that cost on as many frame as possible
//!
//! ### Merging sequencial vircopy calls
//!
//! The [`pnc::AuthHandle`] shall be set on the whole client FIFO.  This is
//! because de driver merges copies of contiguous [`AlignedFrameBuffer`] in a
//! single [`pnc::vircopy`].  This has been measured to be more efficient than
//! making 2 vircopy calls, even though this copies more data altogether (as
//! 2048 bytes are copied per frames).
//!
//! ### Aligning data frames
//!
//! All network data and metadata are stored in [`AlignedFrameBuffer`]
//! buffer.  This data type guarantees that no physical page boundary fall
//! into a Frame and reduces the number authorisation of authorization check
//! made by the kernel to at most 1 per frame
//!
//! IPC API
//! =======
//!
//! #### SENDRECV
//!
//! ##### Description
//!
//! This function does 2 actions in one IPC call:
//!
//! -   send TX packets from the application to the network. The IPC
//!     arguments specify which network packets from the pool need to be
//!     sent.
//! -   write RX packets from the network to the RX buffer pool. The IPC
//!     arguments specify which slots inside the client RX pool are
//!     available for writing.
//!
//! The preconditions for successful execution are:
//!
//! -   The calling process shall have granted a read authorization to the
//!     Ethernet Driver to the data to be sent over the network.
//! -   The calling process shall have granted a write authorization to the
//!     Ethernet Driver to the data to be copied from the network.
//! -   Parameter m3 must within range:\
//!     start\_of\_tx\_pool ≤ m3 ≤ start\_of\_tx\_pool  + size\_of\_tx\_pool
//! -   Parameter m4 must within range:\
//!     start\_of\_rx\_pool  ≤ m4 ≤  start\_of\_rx\_pool  +
//!     size\_of\_tx\_pool
//!
//! ##### IPC message format
//!
//! | IPC requests field   | Description                                                                            |
//! | -------------------- | -------------------------------------------------------------------------------------- |
//! | m\_type              | 0x10                                                                                   |
//! | m3                   | Address of the first packet to transmit in the client TX pool                          |
//! | m4                   |     Address of the first free packet in the client RX pool, that the
//!                          ||  driver can write to   |
//! | m5                   | Number of TX packets to transmit.                                                      |
//! | m6                   | Number of RX packets to transmit.                                                      |
//!
//!
//! Data Description
//! ================
//!
//! Frame Buffers Queues
//! --------------------
//!
//! Buffers are grouped in a circular buffer called Frame Buffer Queue.
//!
//! ```text
//!        0       1       2       3       4       5       6       7       8
//!        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//!        |       |       |       |       |       |       |       |       |
//!        |       |       |       |       |       |       |       |       |
//!        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//!        ^       ^
//!        |       |<-------------------------------------->|
//!        |       |      Number of frames to transmit
//!        |       |
//!        |       First frame to transmit
//!        |
//!        |<----------------------------------------------------->|
//!        |             Number of frames in the FIFO
//!        |
//!        | Start of FIFO
//! ```
#![no_std]

use core::ops::{Deref, DerefMut};

use pnc::error::{Error, Result};
use pnc::libpnc::errno::Errno;
use pnc::libpnc::errno::*;
use pnc::libpnc::*;
use pnc::*;

//Rewrite from services/applications/ethernet/enet.h
pub const RECV: u32 = 0;
pub const SEND: u32 = 1;
pub const SETP: u32 = 2;
pub const HALT: u32 = 3; // stop driver
pub const START: u32 = 4; // restart driver
pub const GETM: u32 = 5; // get MAC address
pub const CHANGEM: u32 = 6; // change MAC address
pub const OPEN: u32 = 7;
pub const CLOSE: u32 = 8;
pub const IOCTL: u32 = 9;
pub const SENDRECV: u32 = 16;
pub const INIT_WITH_BUFFERS: u32 = 18;
pub const IOCTL_PROMISC_ON: u32 = 0x91;
pub const IOCTL_PROMISC_OFF: u32 = 0x92;

pub const REPLY: u32 = 42;

// FIXME: maximum ethernet specification in enet.h is 1518+16
pub const MAX_ETHERNET_FRAME_SIZE: usize = 1522;

pub struct EthernetPort {
    pub driver_pid: s_pid_t,
    pub channel: u32,
}

/// An array of `u8` array, suitable for holding an Ethernet Frame, with
/// alignment chosen to maximize data copy performance
///
/// The buffer is aligned on 2048 bytes to optimize the following:
/// - When performing a `vircopy` from/to another `AlignedFrameBuffer`, no
/// page boundary falls into the buffer of either source or dest.  This
/// reduces the number of authorizations check that `vircopy` need to perform
/// to at most 1 per frame.
/// - The compiler is able to generate more efficient load/store because it
/// knows the alignment of the buffer.  The compiler (rustc 1.64.0-nightly)
/// the able to exploit it. E.g. on AArch64, with strict alignment enforced,
/// loading a `u128` at the beginning of the buffer would assemble to a single
/// LDP (load pair) instruction, compared to
/// 16 LDB (load byte) instructions for an arbitrary array of bytes with no
///    assumption on the alignment.
///
/// The buffer is 2048 long so that it is large enough to hold an ethernet
/// frame.
///
/// Frame Buffer Format
/// -------------------
///
/// The frame buffer format is described in the figure below
///
/// | Offset (bytes) | length (bytes) | Description                                                                    |
/// | --:            | --:            | :--                                                                            |
/// | 0x00           | 2              | Length of the Ethernet frame, without FCS, contained in this Frame Buffer (in network order). This value cannot be less than 60 bytes |
/// | 0x02           | 14             | Unused, shall be set to 0 when writing Rx frame, ignored when reading Tx Frames|
/// | 0x10           | N              | Ethernet Frame content                                                         |
/// | N              | 2048-16-N      | Padding. Unspecified value                                                     |
///
//  ```none
//          0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f
//          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   0x00   | Eth frame len |                                               |
//          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   0x10   | Dest MAC              | Source MAC            | Type  |802.1Q |
//          |                       |                       |       |VLAN   |
//          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   0x20   |802.1Q |Ver|Typ| Total | Ident | Frag  |TTL|Pro|Check  |Src    |
//          |VLAN   |   |Srv|Length |       |       |   |to |   sum |Addr   |
//          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   0x30   |Src    | Destination   |UDP src|UDP dst|  UDP  |Check  |       |
//          |Addr   | Address       | port  | port  |  Len  |sum UDP|       |
//          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   0x40   |                                                               |
//          |                       UDP Payload                             |
//    ...   |                                                               |
//          |                                                               |
//   0x1F0  |                                                               |
//          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//
//                              Sample Ethernet Frame buffer
//                              showing a UDP packet.
//  ```
///
///
#[derive(Debug)]
#[repr(C, align(2048))]
pub struct AlignedFrameBuffer {
    array: [u8; 2048],
}

/// Deref trait implementation that give access to the underlying array object
impl Deref for AlignedFrameBuffer {
    type Target = [u8; 2048];

    fn deref(&self) -> &Self::Target {
        &self.array
    }
}

/// Deref trait implementation that gives access to the underlying array object
impl DerefMut for AlignedFrameBuffer {
    fn deref_mut(&mut self) -> &mut Self::Target {
        &mut self.array
    }
}

/// Default implementation that yields an empty (zero'ed) AlignedFrameBuffer
impl Default for AlignedFrameBuffer {
    fn default() -> Self {
        AlignedFrameBuffer { array: [0u8; 2048] }
    }
}

impl EthernetPort {
    /// Create a new EthernetPort object.
    ///
    /// This constructor makes a synchronous `SETP` IPC call to the ethernet
    /// driver in order to registers the caller's PID as process that must be
    /// notified whenever Ethernet frames coming from the network becomes
    /// available.
    ///
    /// The calling process may be notified of available packets as soon as
    /// this function returns.
    ///
    /// # Arguments
    ///
    /// - `driverpid` The PID of the ethernet driver.
    pub fn new(driverpid: s_pid_t, channel: u32) -> Self {
        Self {
            driver_pid: driverpid,
            channel: channel,
        }
    }

    pub fn setp(&self) -> Result<()> {
        // Fill new message with flag SETP:m_type=2 SLOT:m1=0/1
        let mut msg_out = Message::new(SETP, self.channel, 0, 0, 0, 0, 0);

        // Sendrec message to driver ETH
        msg_out.sendrec(self.driver_pid)?;

        if msg_out.m_type() != REPLY {
            return Err(Error::from(EGENERIC).context("Unexpected m_type in ETH response"));
        }
        if msg_out.m1() != 0 {
            return Err(Error::from(msg_out.m1() as Errno).context("ETH SETP error"));
        }
        Ok(())
    }

    pub fn set_promisc_on(&self, on: bool) -> Result<()> {
        // Fill new message with flag IOCTL:m_type=9 SLOT:m2=0/1
        let mut msg_out = match on {
            true => Message::new(IOCTL, IOCTL_PROMISC_ON, self.channel as u64, 0, 0, 0, 0),
            false => Message::new(IOCTL, IOCTL_PROMISC_OFF, self.channel as u64, 0, 0, 0, 0),
        };

        // Sendrec message to driver ETH
        msg_out.sendrec(self.driver_pid)
    }

    // TODO This function shall accept 2 queues and no `count_elt`.  The rational is put the burden
    // of computing the address of RX/TX buffer on the lib instead of on the client.
    pub fn init_with_buffers(
        &self,
        rx_buffer_ptr: *mut u32,
        tx_buffer_ptr: *mut u32,
        count_elt: u64,
    ) -> Result<()> {
        // Fill new message with flag INIT_WITH_BUFFERS
        let mut msg_out = Message::new(
            INIT_WITH_BUFFERS,
            self.channel,
            count_elt as u64,
            rx_buffer_ptr as u32,
            tx_buffer_ptr as u32,
            count_elt as u64,
            0,
        );

        // Sendrec message to driver ETH
        msg_out.sendrec(self.driver_pid)
    }

    /// Send and receive 0 or more Ethernet frames from an Ethernet channel.
    ///
    /// This function can send and receive several Ethernet frames per IPC
    /// call to the driver.
    ///
    /// TX frames are read from the TX buffer and transmitted to the network. RX frames are stored
    /// in the free slots of the RX buffer.
    ///
    /// # Parameters
    ///
    /// - `tx_index` Index of the first slot to send in the TX FIFO
    ///
    /// - `rx_index` Index of the first free slot in the RX FIFO
    ///
    /// - `tx_count` Number of element to transmit
    ///
    /// - `rx_count` Number of free slots in the RX FIFO
    ///
    /// - 'tx_buffer_ptr' Pointer to the TX FIFO buffer
    ///
    /// - 'tx_buffer_size' Size of TX FIFO buffer
    ///
    /// - 'rx_buffer_ptr' Pointer to the RX FIFO buffer
    ///
    /// - 'rx_buffer_size' Size of RX FIFO buffer
    ///
    pub fn sendrecv(
        &self,
        tx_index: u32,
        rx_index: u32,
        tx_count: u64,
        rx_count: u64,
        tx_buffer_ptr: *mut u32,
        tx_buffer_size: usize,
        rx_buffer_ptr: *mut u32,
        rx_buffer_size: usize,
        user_pid: u64,
    ) -> Result<(u32, u32)> {
        // Authenticate access to buffer rx and tx
        let whole_rx = unsafe {
            let p = rx_buffer_ptr.cast::<u8>();
            core::slice::from_raw_parts_mut(p, (rx_buffer_size * 2048) as usize)
        };
        let whole_tx = unsafe {
            let p = tx_buffer_ptr.cast::<u8>();
            core::slice::from_raw_parts_mut(p, (tx_buffer_size * 2048) as usize)
        };
        let mut rx_auth = AuthHandle::set(
            AuthMode::ReadWrite,
            self.driver_pid,
            whole_rx,
            self.driver_pid,
            user_pid,
        )?;
        let mut tx_auth = AuthHandle::set(
            AuthMode::ReadWrite,
            self.driver_pid,
            whole_tx,
            self.driver_pid,
            user_pid,
        )?;

        let mut eth_recv_msg = pnc::Message::new(
            SENDRECV,
            self.channel,
            0,
            tx_index,
            rx_index,
            tx_count as u64,
            rx_count as u64,
        );
        eth_recv_msg.sendrec(self.driver_pid)?;
        if eth_recv_msg.m_type() != REPLY {
            return Err(Error::from(EGENERIC).context("Unexpected m_type in ETH response"));
        }
        if eth_recv_msg.m1() != 0 {
            return Err(Error::from(eth_recv_msg.m1() as Errno).context("ETH SENDRECV error"));
        }

        // Revoke access to buffer rx and tx
        rx_auth.revoke()?;
        tx_auth.revoke()?;

        return Ok((eth_recv_msg.m5() as u32, eth_recv_msg.m6() as u32));
    }

    pub fn recv(&self, rx_buf: &mut [u8]) -> Result<u32> {
        let mut eth_recv = pnc::Message::new(RECV, rx_buf.as_mut_ptr() as u32, 0, 0, 0, 0, 0);
        eth_recv.sendrec(self.driver_pid)?;
        if eth_recv.m_type() != RECV {
            return Err(Error::from(eth_recv.m1() as Errno).context("ETH RECV error"));
        } else {
            if eth_recv.m4() != self.channel {
                return Err(Error::from(EGENERIC).context("Channel not matched in ETH response"));
            }
            /* Return number of bytes received */
            return Ok(eth_recv.m2() as u32);
        }
    }

    pub fn send(&self, tx_buf: &[u8]) -> Result<()> {
        let mut eth_send = pnc::Message::new(
            SEND,
            tx_buf.as_ptr() as u32,
            tx_buf.len() as u64,
            self.channel,
            0,
            0,
            0,
        );
        eth_send.sendrec(self.driver_pid)?;
        if eth_send.m_type() != REPLY {
            return Err(Error::from(EGENERIC).context("Unexpected m_type in ETH response"));
        }
        if eth_send.m1() != OK as u32 {
            return Err(Error::from(eth_send.m1() as Errno).context("ETH SEND error"));
        } else {
            Ok(())
        }
    }
}
