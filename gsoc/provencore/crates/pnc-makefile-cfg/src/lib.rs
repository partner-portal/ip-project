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
pub fn setup() {
    let target = std::env::var("CONFIG_VARIABLES").unwrap();
    let vars = target.split_whitespace().collect::<Vec<_>>();
    for makefile_var in vars {
        if makefile_var
            .chars()
            .all(|c| c.is_alphanumeric() || c == '_')
        {
            println!("cargo:rustc-cfg=CONFIG_{}", makefile_var);
        }
        else if makefile_var
            .chars()
            .any(|c| c == '=')
        {
            let key_val: Vec<&str> = makefile_var.split('=').collect();
            println!("cargo:rustc-env=CONFIG_{}={}", key_val[0], key_val[1]);
            println!("cargo:rustc-cfg=CONFIG_{}", key_val[0]);
        }
    }
}
