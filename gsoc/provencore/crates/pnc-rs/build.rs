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

extern crate pnc_header_paths;
use std::process::Command;
use std::path::Path;
use std::fs::File;
use std::io::Write;
//use std::vec::Vec;
use regex::Regex;
use file_lock::{FileLock, FileOptions};


/* Some magic constant for generating the sys_procs_pid.
 * Found in provencore/tools/gen_sys_procs_pid.sh
 */
const NR_TASKS: usize = 3;

fn fdtget(args: &[&str]) -> String {
    let output = Command::new("fdtget").args(args).output()
        .expect("Command executed with failing error code");

    let stdout_string = String::from_utf8(output.stdout)
        .expect("Could not decode stdout as UTF-8");
    return stdout_string;
}

fn generate_sys_procs_pid(file: &mut File) {
    /*
     * Create the static list of PIDs (the equivalent of sys_procs_pid.h, but for rust
     */
    let build_base = std::env::var("BUILD_BASE")
        .expect("Could not find environment variable BUILD_BASE");

    /* DTB file is found in $BUILD_BASE/kconfig/kernel_config.dtb */
    let dtb_path = Path::new(&build_base).join("kconfig").join("kernel_config.dtb");

    let r = fdtget(&["-l", dtb_path.to_str().unwrap(), "/processes"]);
    for (procnr, name) in r.lines().enumerate() {
        /*
        writeln!(file, "/// PID of application {}", name.to_uppercase());
        writeln!(file, "pub const {}_ID: u64 = 0x{:02x};",
            name.to_uppercase(),
            8*(procnr + NR_TASKS)).unwrap();
        */
        if let Err(e) = writeln!(file, "/// PID of application {}", name.to_uppercase()) {
                println!("Writing error: {}", e.to_string());   
        }
        if let Err(e) = writeln!(file, "pub const {}_ID: u64 = 0x{:02x};",
            name.to_uppercase(), 8*(procnr + NR_TASKS)) {
                println!("Writing error: {}", e.to_string());   
        }
    }
}

fn generate_device(dev_file: &mut File, ree_file: &mut File) {
    let build_base = std::env::var("BUILD_BASE")
        .expect("Could not find environment variable BUILD_BASE");

    /* DTB file is found in $BUILD_BASE/kconfig/kernel_config.dtb */
    let dtb_path = Path::new(&build_base).join("kconfig").join("kernel_config.dtb");
    let re = Regex::new(r"@.*").unwrap();


    let devices = fdtget(&["-l", dtb_path.to_str().unwrap(), "/devices"]);
    for (devid, devname) in devices.lines().enumerate() {
        let simple_name = re.replace(devname, "");
        if let Err(e) = writeln!(dev_file, "/// DEV number of {}", 
            simple_name.to_uppercase()) {
                println!("Writing error: {}", e.to_string());   
        }
        if let Err(e) = writeln!(dev_file, "pub const {}: u64 = 0x{:02x};",
            simple_name.to_uppercase(),
            devid) {
                println!("Writing error: {}", e.to_string());   
        }
    }

    for (devid, devname) in fdtget(&["-l", dtb_path.to_str().unwrap(), "/rees"]).lines().enumerate() {
        let simple_name = re.replace(devname, "");
        if let Err(e) = writeln!(ree_file, "/// REE definition of {}", 
            simple_name.to_uppercase()) {
                println!("Writing error: {}", e.to_string());   
        }
        if let Err(e) = writeln!(ree_file, "pub const {}: u32 = 0x{:02x};",
            simple_name.to_uppercase(),
            devices.lines().count() + devid) {
                println!("Writing error: {}", e.to_string());   
        }
    }
}

fn main() {
    let should_we_block  = true;
    let options = FileOptions::new()
                        .write(true)
                        .create(true)
                        .append(true);

    let filelock = match FileLock::lock("myfile.txt", should_we_block, options) {
        Ok(lock) => lock,
        Err(err) => panic!("Error getting write lock: {}", err),
    };

    let target = std::env::var("TARGET").unwrap();
    let mut build = cc::Build::new();
    let llvm = std::env::var("PROVENCORE_LLVM_BUILD")
        .and_then(|v| v.parse::<i32>().map_err(|_| std::env::VarError::NotPresent))
        .unwrap_or(0) == 1;
    let compiler = if llvm {
        pnc_header_paths::Compiler::Clang
    } else {
        pnc_header_paths::Compiler::Gcc
    };
    build.compiler(format!("{}{}",
                           std::env::var("CROSS_COMPILE").unwrap(),
                           if llvm { "clang" } else { "gcc" }));
    build.file("src/libpnc/static-wrappers.c");

    match target.as_str() {
        "armv7a-none-eabi" => build.define("CONFIG_ARCH_ARMV7A", ""),
        "aarch64-unknown-none-softfloat" => build.define("CONFIG_ARCH_ARMV8", ""),
        "riscv64imac-unknown-none-elf" => build.define("CONFIG_ARCH_RISCV64", ""),
        _ => build.define("CONFIG_ARCH_ARMV8", ""),
    };

    for include_path in pnc_header_paths::include_paths_from_target(target.as_str()) {
        build.include(include_path);
    }
    build.include(pnc_header_paths::compiler_include_path(compiler));

    build.compile("pnc-rs-static-wrappers");

    let mut sys_procs_pid_file = File::create(Path::new("src/sys_procs_pid.rs"))
        .expect("Could not create sys_procs_pid file");
    generate_sys_procs_pid(&mut sys_procs_pid_file);

    let mut device_file = File::create(Path::new("src/device.rs"))
        .expect("Could not create device file");
    let mut ree_file = File::create(Path::new("src/rees.rs"))
        .expect("Could not create ree_file file");
    generate_device(&mut device_file, &mut ree_file);
    let _ = filelock.unlock();

}
