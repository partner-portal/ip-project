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

#include "static-wrappers.h"

inline int pnc_rs_send(s_pid_t dest, const message *mptr) { return send(dest, mptr); }
inline int pnc_rs_receive(s_pid_t src, message *mptr, uint64_t timeout) { return receive(src, mptr, timeout); }
inline int pnc_rs_send_nonblock(s_pid_t dest, const message *mptr) { return send_nonblock(dest, mptr); }
inline int pnc_rs_receive_nonblock(s_pid_t src, message *mptr) { return receive_nonblock(src, mptr); }
inline int pnc_rs_receive_any(message *mptr, uint64_t timeout) { return receive_any(mptr, timeout); }
inline int pnc_rs_receive_any_nonblock(message *mptr) { return receive_any_nonblock(mptr); }
inline int pnc_rs_receive_hw(message *mptr, uint64_t timeout) { return receive_hw(mptr, timeout); }
inline int pnc_rs_receive_hw_nonblock(message *mptr) { return receive_hw_nonblock(mptr); }
inline int pnc_rs_sendrec(s_pid_t src_dst, message *mptr) { return sendrec(src_dst, mptr); }
inline int pnc_rs_sendrec_nonblock(s_pid_t src_dst, message *mptr) { return sendrec_nonblock(src_dst, mptr); }
inline int pnc_rs_notify(s_pid_t dest) { return notify(dest); }
inline int pnc_rs_sleep(uint64_t timeout) { return pnc_sleep(timeout); }

inline int pnc_rs_irqsetpolicy(
		unsigned int irq,
		unsigned int policy,
		unsigned int notify_id,
		unsigned int status,
		unsigned int *hook_id) {
	return pnc_irqsetpolicy(irq, policy, notify_id, status, hook_id);
}
inline int pnc_rs_irqrmpolicy(unsigned int hook_id) { return pnc_irqrmpolicy(hook_id); }
inline int pnc_rs_irqstatus(unsigned int hook_id, unsigned int status) { return pnc_irqstatus(hook_id, status); }
inline int pnc_rs_irqdisable(unsigned int hook_id) { return pnc_irqdisable(hook_id); }
inline int pnc_rs_irqenable(unsigned int hook_id) { return pnc_irqenable(hook_id); }
inline int pnc_rs_irqrelease(unsigned int hook_id) { return pnc_irqrelease(hook_id); }
