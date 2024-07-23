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
 * @brief Uart application definitions
 * @author Hadrien Barral
 * @date April 30th, 2018 (creation)
 * @copyright (c) 2018-2018, Prove & Run and/or its affiliates. All rights reserved.
 */

#ifndef _UART_API_H_INCLUDED_
#define _UART_API_H_INCLUDED_

#include <inttypes.h>
#include <ipc.h>
#include <stdint.h>

#define UART_FLAG_NOBLOCK   (UINT32_C(1) << 0)
#define UART_FLAG_ECHO      (UINT32_C(1) << 1)
/**
 * Request from the uart to return to the caller the grantee and effector
 * rights attached to the authorization passed in the field m5 of the
 * IPC message.
 */
#define UART_FLAG_RESET_AUTH    (UINT32_C(1) << 2)

#define UART_CTRL_ENABLE    UINT32_C(0x1)
#define UART_CTRL_DISABLE   UINT32_C(0x2)

#define UART_PACKED_LEN(l)  ((uint32_t)(l) << 16)
#define UART_PACKED_MAXLEN  SIZE_C(32)

/**
 * m1 - flags : NOBLOCK, ECHO
 * m2 - owner
 * m3 - ptr
 * m4 - len
 * m5 - auth
 */
#define UART_READ           UINT32_C(0x0)

/**
 * m1 - flags : NOBLOCK
 * m2 - owner
 * m3 - ptr
 * m4 - len
 * m5 - auth
 */
#define UART_WRITE          UINT32_C(0x1)

/**
 * m1 - len, flags : NOBLOCK, ECHO
 */
#define UART_READ_PACKED    UINT32_C(0x2)

/**
 * m1 - len, flags : NOBLOCK
 */
#define UART_WRITE_PACKED   UINT32_C(0x3)

/**
 * m1 - command : ENABLE, DISABLE
 */
#define UART_CTRL           UINT32_C(0x4)

/**
 * @brief Return the pointer and maximum length of a packed buffer.
 */
static inline void uart_get_packed_ptr(message *msg, char **ptr, size_t *len)
{
    /*
     * Check that the 'message' structure has no hole. We pack the read or written
     * characters in m2-m6.
     */
    _Static_assert(offsetof(message, m3) == offsetof(message, m2) + sizeof(msg->m2),
                   "'message' structure is not packed");
    _Static_assert(offsetof(message, m4) == offsetof(message, m3) + sizeof(msg->m3),
                   "'message' structure is not packed");
    _Static_assert(offsetof(message, m5) == offsetof(message, m4) + sizeof(msg->m4),
                   "'message' structure is not packed");
    _Static_assert(offsetof(message, m6) == offsetof(message, m5) + sizeof(msg->m5),
                   "'message' structure is not packed");

    *ptr = (char *)(&msg->m2);
    *len = offsetof(message, m6) + sizeof(msg->m6) - offsetof(message, m2);
}

#endif /* _UART_API_H_INCLUDED_ */
