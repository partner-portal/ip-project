/*
 * Copyright (c) 2023-2024 ProvenRun S.A.S and/or its affiliates.
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
 * FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. PROVE & RUN S.A.S SHALL
 * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING,
 * MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */
/**
 * @file    : platform.h
 * @brief   : Linflex UART definitions
 *
 * @author  : Aymeric PLOTON aymeric.ploton@provenrun.com
 * @date    : 23/02/2024 (creation)
 * Copyright (c) 2023-2024 ProvenRun S.A.S and/or its affiliates.
 * All Rights Reserved.
 */


#ifndef _PLAT_PLATFORM_H_INCLUDED_
#define _PLAT_PLATFORM_H_INCLUDED_

#include <plat_helpers.h>

/* Provencore address : */
// DDR_BASE_PA address is mandatory to equal to this in order to compile atf to fip image.

// reserve 64 Mo (supposé suffisant) avant l'adresse de l'atf qui est en ff80_0000
#define DDR_BASE_PA             PADDR_C(0x80200000)		// The base address of the secure memory
#define DDR_SIZE                SIZE_C(0x01600000)		// The size of the secure memory
#define DDR_BASE_VA             KADDR_C(0xfffffffc00000000)	// The virtual address of the secure memory for arm64, risv64, use a 32-bit number for arm, or use 1:1 mapping
#define KERNEL_BOOT_OFFSET      0x0				// The offset from the start of the secure memory, which should be loaded the kernel.bin or provencore.bin (only for arm64)

/* Non secure world address : */
/* From ProvenCore-A to the end of the low RAM */
#define NS_DDR_BASE_PA          PADDR_C(0x81800000)		// The base address of the non secure memory
#define NS_DDR_SIZE             SIZE_C(0x7E800000)		// The size of the non secure memory (end : 0x9B000000)

/* Periphical address : */
#define PERIPHERALS_BASE_VA     (DDR_BASE_VA + PLAT_UNSAFE_ALIGN(DDR_SIZE, SECTION_SIZE)) // Trang : Can keep this virtual addres as is
#define PERIPHERALS_BASE_PA     PADDR_C(0x40000000)		// The base address of the peripheral registers
#define PERIPHERALS_SIZE        SIZE_C(0x20000000)		// The size of the peripheral registers
// It is possible to have multiple regions of peripherals

/* GIC-500 (v3) : */
#define GIC_BASE_PA             PADDR_C(0x50800000) 		// The gic version and base address of the GIC should be in the SoC's RM : RMS32G3.pdf
#define GIC_DIST_REGS_OFFSET    0x0000				// This information should be in the SoC's RM or the General GIC RM : freescale/s32g3.dtsi
#define GIC_RDIST_REGS_OFFSET   0x100000			// This information should be in the SoC's RM or the General GIC RM : freescale/s32g3.dtsi
#define NR_IRQ_VECTORS          256				// Amount of interrupt lines that managed by this GIC : RMS32G3.pdf - appendix file S32G3_interrupt_map.xlsx (271 ?)

/* Generic timer : */
#define SYSCOUNTER_INT          29				// The IRQ of the Secure Physical Timer is 29, we usually use this one for ProvenCore : DDI0516E_gic5000_r1p1_trm.pdf
#define TIMER_INT               SYSCOUNTER_INT			// Define a general macro TIMER_INT to map and enable timer interrupt.
#define SYSCOUNTER_FREQ         (5000000)			// The frequency of the Secure Physical Timer
#define SYSCOUNTER_PHYSICAL					// To use the Secure Physical Timer, another option is SYSCOUNTER_VIRTUAL, which use IRQ#30
#define EVENT_IDLE_TIMEOUT      (30000000)	/* 30 s */	// Secure Monitor switches context from NS to S after 30s without any action from Linux.
#define EVENT_LINUX_TIMEOUT     (40000)		/* 40 µs */	// FIXME : what is ?

/* UART : */
#define DEFAULT_UART            0				// Use uart0 to output message from provencore and secure applications.
#define MAX_UARTS               2U				// Max index from 0 of the uarts in this SoC: for example this platform has 3 uart => Max index in 2.
#define UART_CLOCK              1				// FIXME : Where ? (if you want to initialize the uart ourself, generally we should avoid that, it has been initialized by bootloader)

#define UART0_BASE_PA           PADDR_C(0x401C8000)		// The base address of the uart0 in this SoC, this information is in the SoC's RM : RMS32G3.pdf and appendix file S32G3_memory_map.xlsx
#define UART1_BASE_PA           PADDR_C(0x401CC000)		// The base address of the uart1 in this SoC, this information is in the SoC's RM : RMS32G3.pdf and appendix file S32G3_memory_map.xlsx
#define UART2_BASE_PA           PADDR_C(0x402BC000)		// The base address of the uart2 in this SoC, this information is in the SoC's RM : RMS32G3.pdf and appendix file S32G3_memory_map.xlsx

/* Platform memory map */
#define VM_MAPPING_REGIONS      3				// 2 memory regions in the macro MEMORY_MAP will be mapped.
#define VM_MAPPING_SECURE_RAM   0				// Region index VM_MAPPING_SECURE_RAM is the Secure Memory.

#define OTHER_BASE_PA PADDR_C(0x80000000)
#define OTHER_SIZE SIZE_C(0x200000)
// The next line declared the list of memory regions that will be mapped by MMU, without this step, ProvenCore and secure apps can not access the devices.
#define MEMORY_MAP \
    MEMORY_REGION("secure DDR", DDR_BASE_PA, DDR_BASE_VA, DDR_SIZE, VM_MAP_CACHED_FLAGS) \
    MEMORY_REGION("runtime DEVICES", PERIPHERALS_BASE_PA, PERIPHERALS_BASE_VA, PERIPHERALS_SIZE, VM_MAP_DEVICE_FLAGS) \
    MEMORY_REGION("the other part of the RAM", OTHER_BASE_PA, (PERIPHERALS_BASE_VA + PLAT_UNSAFE_ALIGN(PERIPHERALS_SIZE, SECTION_SIZE)), OTHER_SIZE, VM_MAP_CACHED_FLAGS)


#endif /* _PLAT_PLATFORM_H_INCLUDED_ */
