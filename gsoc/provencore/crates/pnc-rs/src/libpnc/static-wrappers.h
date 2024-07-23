/*
 * Copyright (c) 2020-2020 Prove & Run S.A.S
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

#ifndef STATIC_WRAPPERS_H_ZJFNBSCX
#define STATIC_WRAPPERS_H_ZJFNBSCX

#include <ipc.h>
#include <syscall_internal.h>

int pnc_rs_send(s_pid_t dest, const message *mptr);
int pnc_rs_receive(s_pid_t src, message *mptr, uint64_t timeout);
int pnc_rs_send_nonblock(s_pid_t dest, const message *mptr);
int pnc_rs_receive_nonblock(s_pid_t src, message *mptr);
int pnc_rs_receive_any(message *mptr, uint64_t timeout);
int pnc_rs_receive_any_nonblock(message *mptr);
int pnc_rs_receive_hw(message *mptr, uint64_t timeout);
int pnc_rs_receive_hw_nonblock(message *mptr);
int pnc_rs_sendrec(s_pid_t src_dst, message *mptr);
int pnc_rs_sendrec_nonblock(s_pid_t src_dst, message *mptr);
int pnc_rs_notify(s_pid_t dest);
int pnc_rs_sleep(uint64_t timeout);

int pnc_rs_irqsetpolicy(
		unsigned int irq,
		unsigned int policy,
		unsigned int notify_id,
		unsigned int status,
		unsigned int *hook_id);
int pnc_rs_irqrmpolicy(unsigned int hook_id);
int pnc_rs_irqstatus(unsigned int hook_id, unsigned int status);
int pnc_rs_irqdisable(unsigned int hook_id);
int pnc_rs_irqenable(unsigned int hook_id);
int pnc_rs_irqrelease(unsigned int hook_id);

#endif /* end of include guard:  */
