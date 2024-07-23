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
 * @brief ProvenCore Metadata inforation
 * @author Vincent Siles
 * @date October 30th, 2017 (creation)
 * @copyright (c) 2017-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _PNC_METADATA_H_INCLUDED_
#define _PNC_METADATA_H_INCLUDED_

/* ProvenCore metadata information */
#define MD_CACHE_CRITICAL_SHIFT     21
#define MD_PHYSINFO_CACHED_SHIFT    20
#define MD_PHYSINFO_NRPAGES_MASK    UINT32_C(0xfffff)

#define METADATA_PHYSINFO_CACHED(p)     (!!((p) & (UINT32_C(1) << MD_PHYSINFO_CACHED_SHIFT)))
#define METADATA_PHYSINFO_NRPAGES(p)    ((p) & MD_PHYSINFO_NRPAGES_MASK)
#define METADATA_CACHE_CRITICAL(p)      (!!((p) & (UINT32_C(1) << MD_CACHE_CRITICAL_SHIFT)))

#endif /* _PNC_METADATA_H_INCLUDED_ */
