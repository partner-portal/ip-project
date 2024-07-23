/*
 * Copyright (c) 2020-2023 Prove & Run S.A.S
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
#![allow(bad_style)]
#![allow(missing_docs)]

//! FFI bindings for PNC's libpnc

pub mod errno;

use super::libc::{c_char, c_int, c_uint, c_void};

/// Secure PID type
pub type s_pid_t = u64;

pub type paddr_t = u64;

#[derive(Debug, Clone)]
#[repr(C, align(64))]
pub struct message {
    pub m_source: u64,
    pub uptime: u64, // in µs
    pub m_type: u32,
    pub m1: u32,
    pub m2: u64,
    pub m3: u32,
    pub m4: u32,
    pub m5: u64,
    pub m6: u64,
}

#[repr(C)]
pub struct compact_message {
    pub m_source: u64,
    pub uptime: u64, // in µs
    pub m_type: u32,
    pub m1: u32,
    pub m2: u64,
    pub m3: u32,
    pub m4: u32,
    pub m5: u64,
    pub m6: u64,
}

#[repr(C)]
pub struct smc32_params_t {
    pub r0: u32, // function identifier
    pub r1: u32,
    pub r2: u32,
    pub r3: u32,
    pub r4: u32,
    pub r5: u32,
    pub r6: u32,
    pub r7: u32,
}

#[repr(C)]
pub struct smc64_params_t {
    pub x0: u64, // function identifier
    pub x1: u64,
    pub x2: u64,
    pub x3: u64,
    pub x4: u64,
    pub x5: u64,
    pub x6: u64,
    pub x7: u64,
    pub x8: u64,
    pub x9: u64,
    pub x10: u64,
    pub x11: u64,
    pub x12: u64,
    pub x13: u64,
    pub x14: u64,
    pub x15: u64,
    pub x16: u64,
    pub x17: u64,
}

#[repr(C)]
pub struct mcall_params_t {
    pub a0: u64, // function identifier
    pub a1: u64,
    pub a2: u64,
    pub a3: u64,
    pub a4: u64,
    pub a5: u64,
    pub a6: u64,
    pub a7: u64,
}

#[repr(C)]
pub struct hvc32_params_t {
    pub r0: u32, // function identifier
    pub r1: u32,
    pub r2: u32,
    pub r3: u32,
    pub r4: u32,
    pub r5: u32,
    pub r6: u32,
    pub r7: u32,
}

// syscall.h
extern "C" {
    pub fn udelay(microseconds: u64) -> c_int;
    pub fn get_pid(pid: *mut s_pid_t) -> c_int;
    pub fn s_kill(target: s_pid_t, signal: u32);
    pub fn s_execve(
        path: *const c_char,
        argv: *mut *const c_char,
        envp: *mut *const c_char,
        quota: usize,
    ) -> c_int;
    pub fn s_fork(pid: *mut s_pid_t) -> c_int;
    // TODO
}

// syscall_internal.h
extern "C" {
    pub fn pnc_slow_kernel_call(callnr: c_int, payload: *mut message) -> c_int;
    pub fn pnc_exec(
        path: *const c_char,
        argv: *mut *const c_char,
        envp: *mut *const c_char,
        quota: u64,
    ) -> c_int;
    pub fn pnc_fork(is_parent: *mut bool, child_pid: *mut s_pid_t) -> c_int;
    pub fn pnc_kill(target: s_pid_t, signal: u32) -> c_int;
    pub fn pnc_exit(code: u32) -> c_int;
    pub fn pnc_brk(vaddr: *const c_void, new_vaddr: *mut *mut c_void) -> c_int;
    pub fn pnc_iomap(
        device_id: c_uint,
        paddr: paddr_t,
        length: usize,
        vaddr: *mut *mut c_void,
    ) -> c_int;
    pub fn pnc_iounmap(vaddr: *mut c_void) -> c_int;
    pub fn pnc_irqctl(
        request: c_uint,
        irq: c_uint,
        polcy: c_uint,
        id: c_uint,
        status: c_uint,
        hook_id: *mut c_uint,
    ) -> c_int;
    pub fn pnc_platctl(device_id: c_uint, request: c_uint, payload: *mut message) -> c_int;
    pub fn pnc_smc(params: *mut smc32_params_t) -> c_int;
    pub fn pnc_smc64(params: *mut smc64_params_t) -> c_int;
    pub fn pnc_mcall(params: *mut mcall_params_t) -> c_int;
    pub fn pnc_hvc(params: *mut hvc32_params_t) -> c_int;
    pub fn pnc_setauth(
        mode: c_int,
        grantee: s_pid_t,
        vaddr: *const c_void,
        length: usize,
        effector: s_pid_t,
        revoker: s_pid_t,
        auth_handle: *mut c_uint,
    ) -> c_int;
    pub fn pnc_vircopy(
        src: s_pid_t,
        src_vaddr: *const c_void,
        dst: s_pid_t,
        dst_vaddr: *mut c_void,
        length: usize,
    ) -> c_int;
    pub fn pnc_revoke(owner: s_pid_t, auth_handle: c_uint) -> c_int;
    pub fn pnc_change_grantee(owner: s_pid_t, auth_handle: c_uint, new_grantee: s_pid_t) -> c_int;
    pub fn pnc_change_effector(owner: s_pid_t, auth_handle: c_uint, new_effector: s_pid_t)
        -> c_int;
    pub fn pnc_change_revoker(owner: s_pid_t, auth_handle: c_uint, new_revoker: s_pid_t) -> c_int;
    pub fn pnc_shm_alloc(nr_pages: c_uint, id: *mut c_uint, vaddr: *mut *mut c_void) -> c_int;
    pub fn pnc_shm_register(id: c_uint, nr_pages: *mut c_uint, vaddr: *mut *mut c_void) -> c_int;
    pub fn pnc_shm_unregister(id: c_uint) -> c_int;
    pub fn pnc_shm_transfer(id: c_uint, target: s_pid_t) -> c_int;
    pub fn pnc_get_pid(pid: *mut s_pid_t) -> c_int;
    pub fn pnc_get_domain(object: s_pid_t, dom: *mut c_uint) -> c_int;
    pub fn pnc_get_times(object: s_pid_t, times: *mut u64) -> c_int;
    pub fn pnc_get_uptime(uptime: *mut u64) -> c_int;
    pub fn pnc_get_quota(object: s_pid_t, quota: *mut c_uint, max_quota: *mut c_uint) -> c_int;
    pub fn pnc_get_data(
        object: s_pid_t,
        pdata_base: *mut *mut c_void,
        pdata_size: *mut c_uint,
    ) -> c_int;
    pub fn pnc_get_metadata_by_app(
        elf_name: *const c_char,
        length: usize,
        data_size: *mut u32,
        minfo: *mut u32,
        elf_nr: *mut u32,
    ) -> c_int;
    pub fn pnc_get_metadata_by_pid(
        pid: s_pid_t,
        data_size: *mut u32,
        minfo: *mut u32,
        elf_nr: *mut u32,
    ) -> c_int;
    pub fn pnc_get_shm_token_owner(id: c_uint, owner: *mut s_pid_t) -> c_int;
    pub fn pnc_get_phys_buff(
        vaddr: *mut *mut c_void,
        paddr: *mut paddr_t,
        nr_pages: *mut c_uint,
    ) -> c_int;
    pub fn pnc_get_sysproc_pid_by_name(
        sysproc_name: *const c_char,
        length: usize,
        pid: *mut s_pid_t,
    ) -> c_int;
    pub fn pnc_get_capabilities(
        object: s_pid_t,
        priv_flags: *mut u32,
        trap_mask: *mut u32,
        call_mask: *mut u64,
    ) -> c_int;
    pub fn pnc_get_allowed_ipc(object: s_pid_t, target: s_pid_t, trap_mask: *mut u32) -> c_int;
}

// static-wrappers.h
extern "C" {
    pub fn pnc_rs_send(target: s_pid_t, mgs: *const message) -> c_int;
    pub fn pnc_rs_send_nonblock(target: s_pid_t, msg: *const message) -> c_int;
    pub fn pnc_rs_receive(target: s_pid_t, msg: *mut message, timeout: u64) -> c_int;
    pub fn pnc_rs_receive_nonblock(target: s_pid_t, msg: *mut message) -> c_int;
    pub fn pnc_rs_receive_any(msg: *mut message, timeout: u64) -> c_int;
    pub fn pnc_rs_receive_any_nonblock(msg: *mut message) -> c_int;
    pub fn pnc_rs_receive_hw(msg: *mut message, timeout: u64) -> c_int;
    pub fn pnc_rs_receive_hw_nonblock(msg: *mut message) -> c_int;
    pub fn pnc_rs_sendrec(target: s_pid_t, mst: *mut message) -> c_int;
    pub fn pnc_rs_sendrec_nonblock(target: s_pid_t, msg: *mut message) -> c_int;
    pub fn pnc_rs_notify(target: s_pid_t) -> c_int;
    pub fn pnc_rs_sleep(timeout: u64) -> c_int;
    // pub fn ptr_to_u32_message_field(ptr: *const c_void) -> u32;
    // pub fn ptr_to_u64_message_field(ptr: *const c_void) -> u64;
    // pub fn u32_message_field_to_ptr(mx: u32) -> *mut c_void;
    // pub fn u64_message_field_to_ptr(mx: u64) -> *mut c_void;
    // pub fn size_to_u32_message_field(size: usize) -> u32;
    // pub fn size_to_u64_message_field(size: usize) -> u64;
    // pub fn u32_message_field_to_size(mx: u32) -> usize;
    // pub fn u64_message_field_to_size(mx: u64) -> usize;
    // pub fn copy_compact_message_to_compact_message(
    //     dest: *mut compact_message,
    //     src: *const compact_message,
    // );
    // pub fn copy_compact_message_to_message(dest: *mut message, src: *const compact_message);
    // pub fn copy_message_to_compact_message(dest: *mut compact_message, src: *const message);
    // pub fn copy_message_to_message(dest: *mut message, src: *const message);

    pub fn pnc_rs_irqsetpolicy(
        irq: c_uint,
        polcy: c_uint,
        notify_id: c_uint,
        status: c_uint,
        hook_id: *mut c_uint,
    ) -> c_int;
    pub fn pnc_rs_irqrmpolicy(hook_id: c_uint) -> c_int;
    pub fn pnc_rs_irqstatus(hook_id: c_uint, status: c_uint) -> c_int;
    pub fn pnc_rs_irqdisable(hook_id: c_uint) -> c_int;
    pub fn pnc_rs_irqenable(hook_id: c_uint) -> c_int;
    pub fn pnc_rs_irqrelease(hook_id: c_uint) -> c_int;
}
