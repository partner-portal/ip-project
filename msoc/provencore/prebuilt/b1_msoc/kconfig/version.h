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
 * @brief ProvenCore Version
 * @author Vincent Siles
 * @date February 6th, 2018 (creation)
 * @copyright (c) 2018-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _VERSION_H_INCLUDED_
#define _VERSION_H_INCLUDED_

/*
 * ProvenCore version
 * Used to check the synchronization between the kernel 'core'
 * and the kernel 'codes' part (applications)
 *
 * Version 2:
 * - kernel core is provided as kernel.bin
 * - userland codes and kernel configuration are stored in a TAR archive,
 *   built in codes.bin
 * - both kernel.bin & codes.bin must be loaded by the bootloader at their
 *   correct load address
 * - version must _be equal_ in both kernel header (this value) and codes.bin's
 *   header
 *
 * Increase in VERSION number will happen when the interface between kernel.bin
 * and codes.bin is modified.
 */
#define __KERNEL_VERSION__      2UL

#endif /* _VERSION_H_INCLUDED_ */
