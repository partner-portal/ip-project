/*
 * Copyright (c) 2020-2022 Prove & Run S.A.S
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

use std::path::Path;

fn build_base() -> String {
    std::env::var("BUILD_BASE").unwrap()
}

fn provencore_path() -> String {
    std::env::var("BASE_DIR").unwrap()
}

fn services_path() -> String {
    std::env::var("PATH_TO_SERVICES").unwrap()
}

pub fn include_dir() -> String {
    Path::new(build_base().as_str()).join("include").into_os_string().into_string().unwrap()
}

fn is_dev_context() -> bool {
    !Path::new(format!("{}/prebuilt", provencore_path()).as_str()).exists()
}

pub enum Compiler {
    Clang,
    Gcc,
}

pub fn compiler_include_path(compiler: Compiler) -> String {
    let pathname = match compiler {
        Compiler::Clang => "clang",
        Compiler::Gcc => "gcc",
    };
    format!("{}/include/compiler/{}", provencore_path(), pathname)
}

pub fn openssl_config_include_paths() -> Vec<String> {
    let mut res = Vec::new();
    if is_dev_context() {
        res.push(format!(
            "{}/lib/build/openssl-1.1.1/build/include",
            build_base()
        ));
    } else {
        res.push(format!("{}/include/libs/openssl-1.1.1", provencore_path()));
    }
    res.push(format!("{}/include/openssl-1.1.1", build_base()));
    res
}

pub fn include_paths_from_target(target: &str) -> Vec<String> {
    let mut res = Vec::new();
    let pnc = provencore_path();
    let bbase = build_base();

    res.push(format!("{}/include", pnc));

    if is_dev_context() {
        // Dev
        res.push(format!("{}/lib/pnc/include", pnc));
    } else {
        // Release
        res.push(format!("{}/include/libs/pnc", pnc));
    }

    res.push(format!("{}/plat", bbase));

    match target {
        "armv7a-none-eabi" => {
            res.push(format!("{}/include/arch/armv7a", pnc));
            res.push(format!("{}/include/arch/arm-common", pnc));
        }
        "aarch64-unknown-none-softfloat" => {
            res.push(format!("{}/include/arch/armv8", pnc));
            res.push(format!("{}/include/arch/arm-common", pnc));
        }
        "riscv64imac-unknown-none-elf" => {
            res.push(format!("{}/include/arch/riscv64", pnc));
        }
        _ => {
            res.push(format!("{}/include/arch/armv8", pnc));
            res.push(format!("{}/include/arch/arm-common", pnc));
        }
    };
    res
}

pub fn service_lib_include_path(lib_name: &str) -> String {
    if is_dev_context() {
        format!("{}/libraries/{}/include", services_path(), lib_name)
    } else {
        format!("{}/include/libs/{}", provencore_path(), lib_name)
    }
}

pub fn include_path_from_buildbase(path: &str) -> String {
    format!("{}/{}", build_base(), path)
}
