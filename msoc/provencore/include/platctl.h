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
 * @brief Define the opcodes for the PLATCTL kernel call.
 * @author Henri Chataing
 * @date March 29th, 2018 (creation)
 * @copyright (c) 2018-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _PLATCTL_H_INCLUDED_
#define _PLATCTL_H_INCLUDED_

/**
 * @brief Retrieve the master configuration of the selected device.
 *
 * The (platform dependent) configuration is returned in the field `m3` of
 * the request message.
 */
#define PLATCTL_MASTER_GET      0

/**
 * @brief Configure the master security of the selected device.
 *
 * The (platform dependent) configuration is read from the field `m3` of
 * the request message.
 */
#define PLATCTL_MASTER_SET      1

/**
 * @brief Retrieve the slave configuration of the selected device.
 *
 * The (platform dependent) configuration is returned in the field `m3` of
 * the request message.
 */
#define PLATCTL_SLAVE_GET       2

/**
 * @brief Configure the slave security of the selected device.
 *
 * The (platform dependent) configuration is read from the field `m3` of
 * the request message.
 */
#define PLATCTL_SLAVE_SET       3

/**
 * @brief Configure the master for DMA accesses.
 *
 * The master is configured to have access to the phys buff of the calling
 * application only.
 */
#define PLATCTL_IOMMU_CONFIG    4

#endif /* _PLATCTL_H_INCLUDED_ */
