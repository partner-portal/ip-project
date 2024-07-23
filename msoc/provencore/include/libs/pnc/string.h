/*
 * Copyright (c) 2020-2023 ProvenRun S.A.S
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
 * @brief wrapper around the "standard" string.h header file
 * @author Florian Lugou
 * @date October 29th, 2020 (creation)
 * @copyright (c) 2020-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _PNC_STRING_H_INCLUDED_
#define _PNC_STRING_H_INCLUDED_

#include <sys/cdefs.h>

#include_next <string.h>

__BEGIN_DECLS

/* ProvenCore std lib */
char *errno2string(int errnum);

__END_DECLS

#endif /* _PNC_STRING_H_INCLUDED_ */
