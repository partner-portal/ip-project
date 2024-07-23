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
 * @brief Macro to help write assembly code
 * @author Vincent Siles
 * @date January 6th, 2016 (creation)
 * @copyright (c) 2016-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _ARCH_ARMV8_ASM_H_INCLUDED_
#define _ARCH_ARMV8_ASM_H_INCLUDED_

#define ASM_NL ;

/* *INDENT-OFF* */
#define SYMBOL_NAME_STR(X)      #X
#define SYMBOL_NAME(X)          X
#define SYMBOL_NAME_LABEL(X)    X:
/* *INDENT-ON* */

#ifdef __ASSEMBLER__

#define STT_FUNC    %function
#define ASM_ALIGN   .p2align 4

#define LENTRY(name)        \
    ASM_ALIGN ASM_NL        \
    SYMBOL_NAME_LABEL(name)

#define ENTRY(name) \
    .global SYMBOL_NAME(name) ASM_NL \
    LENTRY(name)

#define WEAK(name) \
    .weak SYMBOL_NAME(name) ASM_NL \
    LENTRY(name)

#define END(name) \
    .size name, .-name

#define ENDPROC(name) \
    .type name,STT_FUNC ASM_NL \
    END(name)

/* *INDENT-OFF* */
// Alias for x30 - link register
lr      .req    x30

.macro VECTOR funct
    .p2align 7
    b \funct
.endm

.macro VECTORM macro
    .p2align 7
    VECTORM__\@__start:
    \macro
    /* FIXME:LLVM: The clang assembler does not handle
     *             `.if` with symbolic expressions. */
    #ifndef __clang__
    .if (. - VECTORM__\@__start) > 128
        .error "Vector payload is too long (max 2^7 bytes)"
    .endif
    #endif
.endm
/* *INDENT-ON* */
/* For applications, always include unwinding info. C++ needs it. */
/* FIXME: unwinding info has been disabled with clang due to lld issues */
#if !defined(__PROVENCORE__) || (DEBUG > 0 && !defined(__clang__))
#define UNWIND(...) __VA_ARGS__
#else
#define UNWIND(...)
#endif
#endif /* __ASSEMBLER__ */
#endif /* _ARCH_ARMV8_ASM_H_INCLUDED_ */
