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
 * @brief Helper macro to deal with byte ordering (bit/little endian)
 * @author Vincent Siles
 * @date March 21st, 2016 (creation)
 * @copyright (c) 2016-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _BYTE_ORDER_H_INCLUDED_
#define _BYTE_ORDER_H_INCLUDED_

#define uswap_16(x)      __builtin_bswap16(x)
#define uswap_32(x)      __builtin_bswap32(x)
#define uswap_64(x)      __builtin_bswap64(x)

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define cpu_to_be16(x)  uswap_16(x)
#define cpu_to_be32(x)  uswap_32(x)
#define cpu_to_be64(x)  uswap_64(x)
#define be16_to_cpu(x)  uswap_16(x)
#define be32_to_cpu(x)  uswap_32(x)
#define be64_to_cpu(x)  uswap_64(x)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define cpu_to_be16(x)  (x)
#define cpu_to_be32(x)  (x)
#define cpu_to_be64(x)  (x)
#define be16_to_cpu(x)  (x)
#define be32_to_cpu(x)  (x)
#define be64_to_cpu(x)  (x)
#else
#error Unrecognized byte order
#endif

#endif /* _BYTE_ORDER_H_INCLUDED_ */
