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

#![allow(bad_style)]
#![allow(missing_docs)]

//! FFI bindings for the PNC's libc

pub use cty::*;

extern "C" {
    pub fn malloc(size: size_t) -> *mut c_void;
    pub fn posix_memalign(memptr: *mut *mut c_void, alignment: size_t, size: size_t) -> c_int;
    pub fn calloc(nmemb: size_t, size: size_t) -> *mut c_void;
    pub fn free(ptr: *mut c_void);
    pub fn realloc(ptr: *mut c_void, size: size_t) -> *mut c_void;
    pub fn puts(string: *const c_char) -> c_int;
    pub fn memcpy(dest: *mut c_void, src: *const c_void, n: size_t) -> *mut c_void;
    pub fn memcmp(s1: *const c_void, s2: *const c_void, n: size_t) -> c_int;
}
