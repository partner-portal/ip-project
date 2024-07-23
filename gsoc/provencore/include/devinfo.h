/*
 * Copyright (c) 2017-2023 ProvenRun S.A.S
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
 * @brief Platform description related structures
 * @author Vincent Siles
 * @date April 24th, 2017 (creation)
 * @copyright (c) 2017-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _DEVINFO_H_INCLUDED_
#define _DEVINFO_H_INCLUDED_

#include <arch_const.h>     // paddr_t
#include <platctl.h>        // platctl opcodes
#include <stddef.h>
#include <stdint.h>

typedef struct {
    unsigned int irq;
    const char *name;
} irq_line_t;

typedef struct {
    uint8_t id;
    uint8_t cfg;            // control bits
    _Bool field : 1;        // 1=[23:16], 0=[7:0]
    _Bool lock  : 1;
} slave_entry_t;

typedef struct {
    uint8_t id;
    _Bool mode  : 1;        // s=1, ns=0
    _Bool lock  : 1;
} master_s_entry_t;

typedef struct {
    uint8_t id;
    _Bool mode   : 1;       // supervisor=1, user=0
    _Bool lock   : 1;
    _Bool enable : 1;       // enable priv config
} master_p_entry_t;

typedef struct {
    paddr_t pa;
    size_t size;
    size_t nr_lines;
    irq_line_t const *lines;
    uint32_t slave_id;
    uint32_t master_id;
} plat_entry_t;

#endif /* _DEVINFO_H_INCLUDED_ */
