/*
 * Copyright (c) 2018-2023 ProvenRun S.A.S
 * All Rights Reserved.
 *
 * This software is the confidential and proprietary information of
 * ProvenRun S.A.S ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with ProvenRun S.A.S
 *
 * PROVENRUN S.A.S MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. PROVENRUN S.A.S SHALL
 * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING,
 * MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */
/**
 * @file
 * @brief Public headers type (kernel/codes) used by ProvenCore
 * @author Vincent Siles
 * @date February 1st, 2018 (creation)
 * @copyright (c) 2018-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _PNC_HEADERS_H_INCLUDED_
#define _PNC_HEADERS_H_INCLUDED_

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Binary Header structure of the codes.bin archive
 *
 * We define a 16-bytes header.
 * It currently only holds the size of the code archive (without
 * this header).
 * The header is arch-dependent.
 *
 * header_version: Version of the header. Version bump might not be
 *                  backward-compatible. Must be equal to
 *                  header_t::header_version.
 * code_size:       Full code archive size (excluding the present header)
 */
typedef struct {
    uint64_t header_version;
    uint32_t code_size;
} app_header_t;

_Static_assert(sizeof(app_header_t) == 16, "Invalid application header size");

/**
 * @brief Binary Header structure of ProvenCore's kernel
 *
 * We define a 32-bytes header.
 * It currently only holds the full kernel size.
 * The header is arch-dependent.
 *
 * header_version: Version of the header. Version bump might not be
 *                  backward-compatible.
 * kernel_size:    Full kernel size. (including bss and stack, but
 *                  not applications)
 *                 It has type 'uintptr_t' because of build limitations.
 *                 This is the kernel size, not a pointer to the kernel size.
 * reserved(0):    Reserved for future use
 */

typedef struct {
    uint32_t jump_to_entry_point_instruction;
    uint32_t reserved0;
    uint64_t header_version;
#if defined(CONFIG_KARCH_BITS_32)
    uintptr_t kernel_size;
    uint32_t reserved1;
#elif defined(CONFIG_KARCH_BITS_64)
    uintptr_t kernel_size;
#else
#error "Unsupported architecture"
#endif
    uint64_t reserved2;
} header_t;

/* Jump_to_entry_point_instruction values rely on this, as well as kernel integrity */
_Static_assert(sizeof(header_t) ==  32, "Invalid kernel header size");
/* Some loaders (u-boot, qemu launcher) rely on this */
_Static_assert(offsetof(header_t, header_version) == 8,
               "Invalid kernel header offset: header_version");
/* Some loaders (u-boot, qemu launcher) rely on this */
_Static_assert(offsetof(header_t, kernel_size) == 16,
               "Invalid kernel header offset: kernel_size");
/* Let us make sure that the kernel-size slot is 64bits */
_Static_assert(offsetof(header_t, reserved2) == 24,
               "Invalid kernel header offset: reserved2");

#endif /* _PNC_HEADERS_H_INCLUDED_ */
