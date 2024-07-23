/*
 * Copyright (c) 2021-2021 Prove & Run S.A.S
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

extern crate bindgen;
extern crate pnc_header_paths;

use std::env;
use std::path::PathBuf;

fn main() {
    let target = std::env::var("TARGET").unwrap();
    // Tell cargo to tell rustc to link the system bzip2
    // shared library.
    println!("cargo:rustc-link-lib=bz2");

    // Tell cargo to invalidate the built crate whenever the wrapper changes
    println!("cargo:rerun-if-changed=wrapper.h");

    let mut bindings = bindgen::Builder::default();

    for include_path in pnc_header_paths::include_paths_from_target(target.as_str()) {
        bindings = bindings.clang_arg(&format!("-I{}", include_path));
    }

    let bindings = bindings
        .header("wrapper.h")
        .use_core()
        .ctypes_prefix("pnc::libc")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks))
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
