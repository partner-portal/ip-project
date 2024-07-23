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

use super::libc;
use core::{
    alloc::{GlobalAlloc, Layout},
    cmp, mem, ptr,
};

#[cfg(all(any(target_arch = "arm",)))]
pub const MIN_ALIGN: usize = 8;
#[cfg(all(any(
    target_arch = "x86_64",
    target_arch = "aarch64",
    target_arch = "riscv64"
)))]
pub const MIN_ALIGN: usize = 16;

struct PncAllocator;

unsafe impl GlobalAlloc for PncAllocator {
    #[inline]
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        if layout.align() <= MIN_ALIGN && layout.align() <= layout.size() {
            libc::malloc(layout.size()) as *mut u8
        } else {
            let mut out = ptr::null_mut();
            // posix_memalign requires that the alignment be a multiple of `sizeof(void*)`.
            // Since these are all powers of 2, we can just use max.
            let align = layout.align().max(mem::size_of::<usize>());
            let ret = libc::posix_memalign(&mut out, align, layout.size());
            if ret != 0 {
                ptr::null_mut()
            } else {
                out as *mut u8
            }
        }
    }

    #[inline]
    unsafe fn alloc_zeroed(&self, layout: Layout) -> *mut u8 {
        if layout.align() <= MIN_ALIGN && layout.align() <= layout.size() {
            libc::calloc(layout.size(), 1) as *mut u8
        } else {
            let ptr = self.alloc(layout);
            if !ptr.is_null() {
                ptr::write_bytes(ptr, 0, layout.size());
            }
            ptr
        }
    }

    #[inline]
    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        libc::free(ptr as *mut libc::c_void)
    }

    #[inline]
    unsafe fn realloc(&self, ptr: *mut u8, layout: Layout, new_size: usize) -> *mut u8 {
        if layout.align() <= MIN_ALIGN && layout.align() <= new_size {
            libc::realloc(ptr as *mut libc::c_void, new_size) as *mut u8
        } else {
            let new_layout = Layout::from_size_align_unchecked(new_size, layout.align());

            let new_ptr = GlobalAlloc::alloc(self, new_layout);
            if !new_ptr.is_null() {
                let size = cmp::min(layout.size(), new_size);
                ptr::copy_nonoverlapping(ptr, new_ptr, size);
                GlobalAlloc::dealloc(self, ptr, layout);
            }
            new_ptr
        }
    }
}

#[global_allocator]
static A: PncAllocator = PncAllocator;

#[alloc_error_handler]
fn alloc_error_handler(layout: Layout) -> ! {
    panic!("allocation error: {:?}", layout)
}
