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
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]

use core::prelude::v1::Some;
use core::slice;
use pnc::error::{Error, ErrorCode, Result, ResultCode};
use pnc::libc::{c_int, c_ulong, c_void};
use pnc::libpnc::errno::*;
use pnc::libpnc::message;
use pnc::Message;
use pnc_device::*;

include!(concat!(env!("OUT_DIR"), "/bindings.rs"));

pub const REE_MTYPE_TERM: u32 = 0xffffffff;
pub const REE_MTYPE_NOP: u32 = 0xfffffffe;
/// Ask for one available NS request
pub const REE_MTYPE_REQUEST: u32 = 0;
/// Indicate response to previous request is available
pub const REE_MTYPE_RESPONSE: u32 = 1;
/// Ask for SHM infos
pub const REE_MTYPE_INFO: u32 = 4;
/// Register a process as a service
pub const REE_MTYPE_REGISTER: u32 = 5;
/// Lookup the pid of a service
pub const REE_MTYPE_LOOKUP: u32 = 6;

/// Ask for one available NS request
pub const REE_MTYPE_FETCH_REQUEST: u32 = 0;
/// Ask to send request's response to NS
pub const REE_MTYPE_SEND_RESPONSE: u32 = 1;
/// Wait for one available NS request
pub const REE_MTYPE_WAIT_REQUEST: u32 = 7;

/// Ask for available NS response
pub const REE_MTYPE_FETCH_RESPONSE: u32 = 8;
/// Wait for one available NS response
pub const REE_MTYPE_WAIT_RESPONSE: u32 = 9;
/// Ask for available NS signal
pub const REE_MTYPE_FETCH_SIGNAL: u32 = 10;
/// Wait for available NS signal
pub const REE_MTYPE_WAIT_SIGNAL: u32 = 11;
/// Ask for available NS event
pub const REE_MTYPE_FETCH_EVENT: u32 = 12;
/// Wait for available NS event
pub const REE_MTYPE_WAIT_EVENT: u32 = 13;
/// Ask to send request to NS
pub const REE_MTYPE_SEND_REQUEST: u32 = 14;
/// Ask to send signal to NS
pub const REE_MTYPE_SEND_SIGNAL: u32 = 15;
/// Send response: only get send status if failed, otherwise,
/// continue waiting for next coming request
pub const REE_MTYPE_SEND_RESPONSE_WAIT_REQUEST: u32 = 16;
///Ask for previous A_REQUEST cancelation
pub const REE_MTYPE_CANCEL: u32 = 17;

/// Wrapper ree_map
pub fn ree_map_rs(reeid: s_pid_t) -> Result<()> {
    match unsafe { ree_map(reeid, REE_REE) } {
        OK => Ok(()),
        r => Err(Error::from(r).context("ree_map failed")),
    }
}

extern "C" fn c_call_ree_handler<State: Reev3Client + Sized>(
    mptr: *mut message,
    state: *mut c_void,
) -> c_int {
    let reev3_client_opt = unsafe { (state as *mut State).as_mut() };
    let msg_opt = unsafe { mptr.as_ref() };
    return match (reev3_client_opt, msg_opt) {
        (Some(client), Some(msg)) => {
            let ret = client.ree_handler(&Message::from_raw(msg.clone()));
            match ret {
                Ok(_) => 0,
                Err(e) => e.code(),
            }
        }
        (_, _) => ENOSYS,
    };
}

extern "C" fn c_call_pnc_handler<State: Reev3Client + Sized>(
    mptr: *mut message,
    state: *mut c_void,
) -> c_int {
    let reev3_client_opt = unsafe { (state as *mut State).as_mut() };
    let msg_opt = unsafe { mptr.as_ref() };
    return match (reev3_client_opt, msg_opt) {
        (Some(client), Some(msg)) => {
            client.pnc_handler(&Message::from_raw(msg.clone()));
            0
        }
        (_, _) => ENOSYS,
    };
}

pub trait Reev3Client {
    fn ree_handler(&mut self, _msg: &Message) -> ResultCode<()>;
    fn pnc_handler(&mut self, _msg: &Message);

    fn ree_loop(&mut self, ree_pid: s_pid_t) -> Result<()>
    where
        Self: Sized,
    {
        let state_ptr: *mut c_void = self as *mut _ as *mut c_void;
        return unsafe {
            let err = ree_loop(
                ree_pid,
                Some(c_call_ree_handler::<Self>),
                Some(c_call_pnc_handler::<Self>),
                state_ptr,
            );

            if err != OK {
                Err(Error::from(err).context("ree_loop"))
            } else {
                Ok(())
            }
        };
    }
}

pub fn ree_get_slice(msg: &Message) -> ResultCode<&'static mut [u8]> {
    if msg.m_type() == REE_MTYPE_TERM {
        return Err(ErrorCode::new(OK, "session terminated"));
    }
    let offset = msg.m1() as c_ulong;
    let size = msg.m3() as c_ulong;
    let max_offset = offset + size;
    if max_offset < offset || max_offset > unsafe { ree_size } {
        return Err(ErrorCode::new(ESPIPE, "seek"));
    }

    // Ref data from REE memory
    let addr = (unsafe { ree_vaddr }) as u64 + offset;
    let data_slice = unsafe { slice::from_raw_parts_mut(addr as *mut u8, size as usize) };
    Ok(data_slice)
}
