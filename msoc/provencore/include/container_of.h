/*
 * Copyright (c) 2016-2023 ProvenRun S.A.S
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
 * @brief An implementation of the famous \c container_of macro.
 * @author Olivier Delande
 * @date December 8th, 2016 (creation)
 * @copyright (c) 2016-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _CONTAINER_OF_H_INCLUDED_
#define _CONTAINER_OF_H_INCLUDED_

/* File copied from lecore//srv/hg/libraries/c */

#include <stddef.h>

/**
 * @brief Computes the address of a structure from the address of one of its
 * members.
 * @param member_ptr a pointer to the member
 * @param container_type the type of the structure
 * @param member_name the identifier of the member
 * @return a pointer to the structure
 */
/* *INDENT-OFF* */
#define container_of(member_ptr, container_type, member_name)                  \
  ((container_type *)(void *)((unsigned char *)(1 ? (member_ptr) :             \
  &((container_type *)0)->member_name) - offsetof(container_type, member_name)))
/* *INDENT-ON* */
#endif /* _CONTAINER_OF_H_INCLUDED_ */
