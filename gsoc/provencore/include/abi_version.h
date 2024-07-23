/*
 * Copyright (c) 2022-2023 ProvenRun S.A.S
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
 * @brief ABI version definition
 * @author Florian Lugou
 * @date September 19th, 2022 (creation)
 * @copyright (c) 2022-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _ABI_VERSION_H_INCLUDED_
#define _ABI_VERSION_H_INCLUDED_

/* Vendor string used in PT_NOTE descriptors */
#define ELF_VEND_ABI_VERSION        "ProvenRun"
/* Size of ELF_VEND_ABI_VERSION */
#define ELF_VEND_ABI_VERSION_LEN    10
/* Type of the PT_NOTE descriptor for the ABI version */
#define ELF_DESC_ABI_VERSION        1
/* Current major version number */
#define ABI_VERSION_MAJOR           11
/* Current minor version number */
#define ABI_VERSION_MINOR           1

#endif /* _ABI_VERSION_H_INCLUDED_ */
