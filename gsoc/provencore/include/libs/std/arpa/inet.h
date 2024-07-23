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
 * @brief Internet host address helpers
 * @author Henri Chataing
 * @date June 7th, 2016 (creation)
 * @copyright (c) 2016-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBSTD_ARPA_INET_H_INCLUDED_
#define _LIBSTD_ARPA_INET_H_INCLUDED_

#if 1
#include <stdint.h>

#include "byte_order.h"

static inline uint16_t ntohs(uint16_t x)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (uint16_t)(uswap_16(x));
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return x;
#else
#error Unrecognized byte order
#endif
}

static inline uint32_t ntohl(uint32_t x)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return (uint32_t)(uswap_32(x));
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    return x;
#else
#error Unrecognized byte order
#endif
}

static inline uint16_t htons(uint16_t x)
{
    return ntohs(x);
}

static inline uint32_t htonl(uint32_t x)
{
    return ntohl(x);
}

#endif
#endif /* _LIBSTD_ARPA_INET_H_INCLUDED_ */
