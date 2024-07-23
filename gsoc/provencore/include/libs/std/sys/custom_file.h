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
 * @brief Custom file io definitions.
 * @author Henri Chataing
 * @date September 29th, 2017 (creation)
 * @copyright (c) 2017-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBSTD_SYS_CUSTOM_FILE_H_INCLUDED_
#define _LIBSTD_SYS_CUSTOM_FILE_H_INCLUDED_

/**
 * This file defines the type __FILE for the newlib library.
 * __FILE is usually defined in <sys/reent.h>, but defining
 * __CUSTOM_FILE_IO__ the definition from this header is used instead.
 */

struct _cFILE {
    int id;
};

typedef struct _cFILE __FILE;

#endif /* _LIBSTD_SYS_CUSTOM_FILE_H_INCLUDED_ */
