/*
 * Copyright (c) 2019-2023 ProvenRun S.A.S
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
 * @brief Arch-dependent low-level syscall helpers
 * @author Hadrien Barral
 * @date April 10th, 2019 (creation)
 * @copyright (c) 2019-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBPNC_ARCH_ARMV7A_SYSCALL_LOW_H_INCLUDED_
#define _LIBPNC_ARCH_ARMV7A_SYSCALL_LOW_H_INCLUDED_

#include <errno.h>

typedef union {
    struct {
        uint32_t s0;
        uint32_t s1;
        uint32_t s2;
        uint32_t s3;
        uint32_t s4;
        uint32_t s5;
        uint32_t s6;
        uint32_t s7;
        uint32_t s8;
        uint32_t s9;
    };
    struct {
        uint64_t s0_s1;
        uint64_t s2_s3;
        uint64_t s4_s5;
        uint64_t s6_s7;
        uint64_t s8_s9;
    };
} pnc_kcall_args_t;

/* Make sure both 'views' (structs in the union) are the same */
_Static_assert(offsetof(pnc_kcall_args_t, s8) ==
               offsetof(pnc_kcall_args_t, s8_s9), "Invalid view");

/*
 * We macro define the syscalls helpers.
 * The template below gives an idea of the generated functions body.
 *
 * The "memory" clobber is intentional. Not only do some kernel calls access
 * memory, but some are also used as a synchronization mechanism, which requires
 * them to be acquire and release operations. It is therefore necessary to
 * prevent the compiler from reordering the assembly statements with memory
 * operations, which the "memory" clobber takes care of.
 *
 * Warning: using explicit registers variables is tricky.
 * Do read the doc before modifying the template.
 * https://gcc.gnu.org/onlinedocs/gcc/Local-Register-Variables.html
 */

#if 0 /* Template */
static inline
int arch_pnc_fast_kernel_call_iX+1_oY+1 (int kcall, const pnc_kcall_args_t *regs)
{
    uint32_t ipc_type_tmp = FAST_KERNEL_CALL;
    uint32_t kcall_nr_tmp = (uint32_t) kcall;
    uint32_t arg_0_tmp = regs->s0;
    ...
    uint32_t arg_X_tmp = regs->sX;

    register uint32_t ipc_type  asm ("r0")      = ipc_type_tmp;
    register uint32_t kcall_nr  asm ("r1")      = kcall_nr_tmp;
    register uint32_t arg_0     asm ("r2")      = arg_0_tmp;
    ...
    register uint32_t arg_X     asm ("rX+2")    = arg_X_tmp;

    register int32_t ret_code   asm ("r0");
    register uint32_t ret_0     asm ("r1");
    ...
    register uint32_t ret_Y     asm ("rY+1");

    asm volatile ("svc     #0" :
                  "=r" (ret_code), "=r" (ret_0), ..., "=r" (ret_Y) :
                  "r" (ipc_type), "r" (kcall_nr),
                  "r" (arg_0), ..., "r" (arg_X) :
                  "memory");

    int32_t ret_code_tmp = ret_code;
    uint32_t ret_0_tmp = ret0;
    ...
    uint32_t ret_Y_tmp = ret_Y;

    regs->s0 = ret_0_tmp;
    ...
    regs->sY = ret_Y_tmp;

    return (int) ret_code_tmp;
}
#endif /* End of Template */

#define __FKCALL_ITER_IN_0(X)
#define __FKCALL_ITER_IN_1(X)  __FKCALL_ITER_IN_0(X) X(0,  "r2")
#define __FKCALL_ITER_IN_2(X)  __FKCALL_ITER_IN_1(X) X(1,  "r3")
#define __FKCALL_ITER_IN_3(X)  __FKCALL_ITER_IN_2(X) X(2,  "r4")
#define __FKCALL_ITER_IN_4(X)  __FKCALL_ITER_IN_3(X) X(3,  "r5")
#define __FKCALL_ITER_IN_5(X)  __FKCALL_ITER_IN_4(X) X(4,  "r6")
#define __FKCALL_ITER_IN_6(X)  __FKCALL_ITER_IN_5(X) X(5,  "r7")
#define __FKCALL_ITER_IN_7(X)  __FKCALL_ITER_IN_6(X) X(6,  "r8")
#define __FKCALL_ITER_IN_8(X)  __FKCALL_ITER_IN_7(X) X(7,  "r9")
#define __FKCALL_ITER_IN_9(X)  __FKCALL_ITER_IN_8(X) X(8, "r10")

#define __FKCALL_DECL_ARG_TMP(nr, reg) \
    uint32_t arg_ ## nr ## _tmp = regs->s ## nr;
#define __FKCALL_DECL_ARG_REG(nr, reg) \
    register uint32_t arg_ ## nr asm (reg) = arg_ ## nr ## _tmp;
#define __FKCALL_PASS_ARG_REG(nr, reg) \
    , "r" (arg_ ## nr)

#define __FKCALL_ITER_OUT_0(X)
#define __FKCALL_ITER_OUT_1(X)  __FKCALL_ITER_OUT_0(X) X(0, "r1")
#define __FKCALL_ITER_OUT_2(X)  __FKCALL_ITER_OUT_1(X) X(1, "r2")
#define __FKCALL_ITER_OUT_3(X)  __FKCALL_ITER_OUT_2(X) X(2, "r3")
#define __FKCALL_ITER_OUT_4(X)  __FKCALL_ITER_OUT_3(X) X(3, "r4")
#define __FKCALL_ITER_OUT_5(X)  __FKCALL_ITER_OUT_4(X) X(4, "r5")
#define __FKCALL_ITER_OUT_6(X)  __FKCALL_ITER_OUT_5(X) X(5, "r6")
#define __FKCALL_ITER_OUT_7(X)  __FKCALL_ITER_OUT_6(X) X(6, "r7")
#define __FKCALL_ITER_OUT_8(X)  __FKCALL_ITER_OUT_7(X) X(7, "r8")

#define __FKCALL_DECL_RET_REG(nr, reg) \
    register uint32_t ret_ ## nr asm (reg);
#define __FKCALL_PASS_RET_REG(nr, reg) \
    , "=r" (ret_ ## nr)
#define __FKCALL_DECL_RET_TMP(nr, reg) \
    uint32_t ret_ ## nr ## _tmp = ret_ ## nr;
#define __FKCALL_WRITE_RET_TMP(nr, reg) \
    regs->s ## nr = ret_ ## nr ## _tmp;

#define __FKCALL_DEFINE(in, out) \
    static inline \
    int arch_pnc_fast_kernel_call_i ## in ## _o ## out \
        (int kcall, pnc_kcall_args_t *regs) { \
        uint32_t ipc_type_tmp = FAST_KERNEL_CALL; \
        uint32_t kcall_nr_tmp = (uint32_t) kcall; \
        __FKCALL_ITER_IN_ ## in (__FKCALL_DECL_ARG_TMP) \
        register uint32_t ipc_type asm ("r0") = ipc_type_tmp; \
        register uint32_t kcall_nr asm ("r1") = kcall_nr_tmp; \
        __FKCALL_ITER_IN_ ## in (__FKCALL_DECL_ARG_REG) \
        register int32_t ret_code asm ("r0"); \
        __FKCALL_ITER_OUT_ ## out(__FKCALL_DECL_RET_REG) \
        asm volatile ("svc     #0" : \
                      "=r" (ret_code) \
                      __FKCALL_ITER_OUT_ ## out(__FKCALL_PASS_RET_REG) : \
                      "r" (ipc_type), "r" (kcall_nr) \
                      __FKCALL_ITER_IN_ ## in(__FKCALL_PASS_ARG_REG) : \
                      "memory"); \
        int32_t ret_code_tmp = ret_code; \
        __FKCALL_ITER_OUT_ ## out(__FKCALL_DECL_RET_TMP) \
        __FKCALL_ITER_OUT_ ## out(__FKCALL_WRITE_RET_TMP) \
        return (int) ret_code_tmp; \
    }

/* Macro-defines `arch_pnc_fast_kernel_call_i1_o0` */
__FKCALL_DEFINE(1, 0);

/* Macro-defines `arch_pnc_fast_kernel_call_i1_o1` */
__FKCALL_DEFINE(1, 1);

/* Macro-defines `arch_pnc_fast_kernel_call_i1_o2` */
__FKCALL_DEFINE(1, 2);

/* Macro-defines `arch_pnc_fast_kernel_call_i1_o4` */
__FKCALL_DEFINE(1, 4);

/* Macro-defines `arch_pnc_fast_kernel_call_i2_o0` */
__FKCALL_DEFINE(2, 0);

/* Macro-defines `arch_pnc_fast_kernel_call_i2_o1` */
__FKCALL_DEFINE(2, 1);

/* Macro-defines `arch_pnc_fast_kernel_call_i2_o2` */
__FKCALL_DEFINE(2, 2);

/* Macro-defines `arch_pnc_fast_kernel_call_i3_o0` */
__FKCALL_DEFINE(3, 0);

/* Macro-defines `arch_pnc_fast_kernel_call_i3_o1` */
__FKCALL_DEFINE(3, 1);

/* Macro-defines `arch_pnc_fast_kernel_call_i3_o3` */
__FKCALL_DEFINE(3, 3);

/* Macro-defines `arch_pnc_fast_kernel_call_i4_o0` */
__FKCALL_DEFINE(4, 0);

/* Macro-defines `arch_pnc_fast_kernel_call_i4_o1` */
__FKCALL_DEFINE(4, 1);

/* Macro-defines `arch_pnc_fast_kernel_call_i4_o2` */
__FKCALL_DEFINE(4, 2);

/* Macro-defines `arch_pnc_fast_kernel_call_i4_o3` */
__FKCALL_DEFINE(4, 3);

/* Macro-defines `arch_pnc_fast_kernel_call_i4_o4` */
__FKCALL_DEFINE(4, 4);

/* Macro-defines `arch_pnc_fast_kernel_call_i5_o0` */
__FKCALL_DEFINE(5, 0);

/* Macro-defines `arch_pnc_fast_kernel_call_i5_o1` */
__FKCALL_DEFINE(5, 1);

/* Macro-defines `arch_pnc_fast_kernel_call_i5_o2` */
__FKCALL_DEFINE(5, 2);

/* Macro-defines `arch_pnc_fast_kernel_call_i6_o1` */
__FKCALL_DEFINE(6, 1);

/* Macro-defines `arch_pnc_fast_kernel_call_i7_o0` */
__FKCALL_DEFINE(7, 0);

/* Macro-defines `arch_pnc_fast_kernel_call_i8_o2` */
__FKCALL_DEFINE(8, 2);

/* Macro-defines `arch_pnc_fast_kernel_call_i8_o4` */
__FKCALL_DEFINE(8, 4);

/* Macro-defines `arch_pnc_fast_kernel_call_i8_o6` */
__FKCALL_DEFINE(8, 6);

/* Macro-defines `arch_pnc_fast_kernel_call_i8_o8` */
__FKCALL_DEFINE(8, 8);

/* Macro-defines `arch_pnc_fast_kernel_call_i9_o1` */
__FKCALL_DEFINE(9, 1);

#endif /* _LIBPNC_ARCH_ARMV7A_SYSCALL_LOW_H_INCLUDED_ */
