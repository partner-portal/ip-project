/*
 * Copyright (c) 2018-2018 Prove & Run S.A.S
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
/**
 * @file
 * @brief
 * @author Henri Chataing
 * @date December 07th, 2018 (creation)
 * @copyright (c) 2018-2018, Prove & Run S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _RANDOM_API_H_INCLUDED_
#define _RANDOM_API_H_INCLUDED_

enum random_cmd {
    GET_ENTROPY = 0,
    GET_RANDOM = 1,
};

/**
 * Maximum number of entropy bytes to be generated from one call
 * to \ref get_entropy.
 */
#define ENTROPY_MAX_LEN          128U

/**
 * Maximum number of random bytes to be generated from one call
 * to \ref get_random.
 */
#define RANDOM_MAX_LEN           65536U

#endif /* _RANDOM_API_H_INCLUDED_ */
