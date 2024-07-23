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

use core::mem::size_of;
use core::mem::MaybeUninit;
use core::sync::atomic::AtomicU32;

use libpnc::paddr_t;
use pnc::*;

use crate::uni_ring::{UrShrdCons, UrShrdProd};

/// A unidirectionnal ring buffer
pub struct UniRing<T, const CAPACITY: usize> {
    pub prod_notif: AtomicU32,
    pub cons_notif: AtomicU32,
    pub shrd_prod: UrShrdProd,
    pub shrd_cons: UrShrdCons,
    pub msgs: [T; CAPACITY],
}

/// A [`UniRing`] with storage allocated in given shared memory
///
pub struct SharedUniRing<'a, T, const CAPACITY: usize> {
    pub shm: &'a mut UniRing<T, CAPACITY>,
}

impl<'a, T, const CAPACITY: usize> SharedUniRing<'a, T, CAPACITY> {
    pub fn new(
        devid: u32,
        paddr: paddr_t,
    ) -> Result<SharedUniRing<'a, T, CAPACITY>, pnc::error::Error> {
        /* Let's make a broad assumption here: the alignment is OK */
        let sz = size_of::<UniRing<T, CAPACITY>>();
        let mapping_len = match sz % 0x200000 {
            0 => sz,
            _ => sz + (0x200000 - sz % 0x200000),
        };

        let shm = pnc::iomap(devid, paddr, mapping_len)?;

        /*
         * Transform a the SHM into a proper structure
         */
        let ptr = shm as *mut u8;

        let p_shm = ptr as *mut UniRing<T, CAPACITY>;

        let _slice_shm_uninit: &mut [MaybeUninit<UniRing<T, CAPACITY>>] = unsafe {
            core::slice::from_raw_parts_mut(shm as *mut MaybeUninit<UniRing<T, CAPACITY>>, 1)
        };

        /*
         * Now let's leave the unsafe world
         */
        let xxx = unsafe { p_shm.as_mut().unwrap() };
        Ok(SharedUniRing { shm: xxx })
    }
}
