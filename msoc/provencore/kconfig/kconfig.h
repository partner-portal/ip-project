/*
 * Copyright (c) 2018-2023 ProvenRun S.A.S
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
 * @brief
 * @author Henri Chataing
 * @date March 22nd, 2018 (creation)
 * @copyright (c) 2018-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _KCONFIG_H_INCLUDED_
#define _KCONFIG_H_INCLUDED_

#include <stddef.h>
#include <stdint.h>

#define PF_PREEMPTIBLE      (UINT8_C(1) << 0)
#define PF_SYS_PROC         (UINT8_C(1) << 1)
#define PF_RESTART          (UINT8_C(1) << 2)
#define PF_DEBUGGER         (UINT8_C(1) << 3)
#define PF_DEBUGGEE         (UINT8_C(1) << 4)
#define PF_KILLABLE         (UINT8_C(1) << 5)
#define PF_AUDITOR          (UINT8_C(1) << 6)

#define TM_SEND             (UINT8_C(1) << 0)
#define TM_RECEIVE          (UINT8_C(1) << 1)
#define TM_SENDREC          (UINT8_C(1) << 2)
#define TM_NOTIFY           (UINT8_C(1) << 3)
#define TM_SLEEP            (UINT8_C(1) << 4)

/** Number of kernel tasks */
#define NR_TASKS            3U

#define SEC_ENTRY_TAG_SHIFT 28
#define SF_MASTER           (UINT32_C(1) << 31)
#define SF_SLAVE            (UINT32_C(1) << 30)

/**
 * @brief The types of memory supported by Provencore
 *
 * @warn Avoid using 0 as a valid enum constant to make sure
 *   cases where device fields are left uninitialized are
 *   more easily caught.
 */
typedef enum {
    MEM_KIND_NORMAL = 1,  /**< Normal cached memory   */
    MEM_KIND_UNCACHED,    /**< Normal uncached memory */
    MEM_KIND_DEVICE       /**< Device memory          */
} mem_kind_t;

/**
 * @brief The security domains supported by Provencore
 *
 * @warn Not all architectures or platforms need support
 *  security domains other than \p PROVENCORE.
 *
 * @warn Avoid using 0 as a valid enum constant to make sure
 *   cases where device fields are left uninitialized are
 *   more easily caught.
 */
typedef enum {
    SECURITY_DOMAIN_PROVENCORE = 1,  /**< Provencore's security level */
    SECURITY_DOMAIN_REE /**< Security level for rich execution environments */
} security_domain_t;

typedef struct {
    uint64_t pa;
    uint64_t size;
    uint32_t slave_id; // value of the slave entry for this device
    uint32_t master_id; // value of the master entry for this device
    uintptr_t irqs;
    security_domain_t security;
    mem_kind_t mem_kind;
} device_t;

typedef struct {
    uint32_t entry; // plat + device dependent
    uint32_t config; // device dependent
} sec_entry_t;


/** Maximum number of arguments for a system process */
#define PROC_ARGS_MAX_COUNT   64U
/** Maximum total size of a system process' arguments */
#define PROC_ARGS_MAX_SIZE  2048U
/** Maximum size of a system process name */
#define PROC_NAME_LEN       32U

typedef struct {
    uint8_t proc_nr;
    uint8_t flags;
    uint16_t dom_nr;
    uint32_t elf_nr;
    uint32_t priority;
    uint32_t min_priority;
    uint32_t quantum;
    uint32_t args_size;
    uint64_t call_mask;
    uintptr_t target_mask;
    uintptr_t device_mask;
    uintptr_t args;
    uint8_t trap_mask;
    char name[PROC_NAME_LEN];
} boot_image_t;

/* These static asserts are here to make sure the compiler
 *  compiling the config has the same view of the structure as
 *  the compiler of the kernel.
 * This is only a compiler sanity-check.
 */
#ifdef __PROVENCORE__
_Static_assert(offsetof(device_t, pa) == 0, "bad pa offset");
_Static_assert(offsetof(device_t, size) == 8, "bad size offset");
_Static_assert(offsetof(device_t, slave_id) == 16, "bad slave_id offset");
_Static_assert(offsetof(device_t, master_id) == 20, "bad master_id offset");
_Static_assert(offsetof(device_t, irqs) == 24, "bad irq offset");

_Static_assert(offsetof(boot_image_t, proc_nr) == 0, "bad proc_nr offset");
_Static_assert(offsetof(boot_image_t, flags) == 1, "bad flags offset");
_Static_assert(offsetof(boot_image_t, dom_nr) == 2, "bad dom_nr offset");
_Static_assert(offsetof(boot_image_t, elf_nr) == 4, "bad elf_nr offset");
_Static_assert(offsetof(boot_image_t, priority) == 8, "bad priority offset");
_Static_assert(offsetof(boot_image_t, min_priority) == 12, "bad min_priority offset");
_Static_assert(offsetof(boot_image_t, quantum) == 16, "bad quantum offset");
_Static_assert(offsetof(boot_image_t, args_size) == 20, "bad args_size offset");
_Static_assert(offsetof(boot_image_t, call_mask) == 24, "bad call_mask offset");
_Static_assert(offsetof(boot_image_t, target_mask) == 32, "bad target_mask offset");

#if defined(CONFIG_KARCH_BITS_32)
_Static_assert(offsetof(boot_image_t, device_mask) == 36, "bad device_mask offset");
_Static_assert(offsetof(boot_image_t, args) == 40, "bad args offset");
_Static_assert(offsetof(boot_image_t, trap_mask) == 44, "bad trap_mask offset");
_Static_assert(offsetof(boot_image_t, name) == 45, "bad name offset");
#elif defined(CONFIG_KARCH_BITS_64)
_Static_assert(offsetof(boot_image_t, device_mask) == 40, "bad device_mask offset");
_Static_assert(offsetof(boot_image_t, args) == 48, "bad args offset");
_Static_assert(offsetof(boot_image_t, trap_mask) == 56, "bad trap_mask offset");
_Static_assert(offsetof(boot_image_t, name) == 57, "bad name offset");
#else
#error "Unsupported architecture"
#endif
#endif /* __PROVENCORE__ */

#define KCONFIG_VERSION        UINT32_C(3)
#define KCONFIG_MAGIC_ARMV7    UINT32_C(0xcafecafe)
#define KCONFIG_MAGIC_ARMV8    UINT32_C(0xfee1beef)
#define KCONFIG_MAGIC_RISCV64  UINT32_C(0x215c5641)

typedef struct {
    /* Version */
    uint32_t version;
    uint32_t magic;

    /* Size config */
    uint32_t nr_irq_hooks;
    uint32_t nr_sched_queues;
    uint32_t nr_procs;
    uint32_t nr_regions;
    uint32_t nr_proc_auths;
    uint32_t nr_proc_shms;
    uint32_t nr_proc_stack_pages;
    uint32_t nr_shms;
    uint32_t nr_dma_pages;
    uint32_t nr_dyn_elfs;
    uint32_t nr_dyn_elf_pages;
    uint32_t nr_irq_vectors;
    uint32_t initial_process;

    /* Boot image */
    uint32_t nr_devices;
    uint32_t nr_rees;
    uint32_t nr_images;
    uint32_t nr_irq_masks;
    uint32_t nr_device_masks;
    uint32_t nr_target_masks;
    uint32_t nr_sec_entries;
    uint32_t args_pool_size;
    uintptr_t devices;
    uintptr_t images;
    uintptr_t irq_masks;
    uintptr_t device_masks;
    uintptr_t target_masks;
    uintptr_t sec_entries;
    uintptr_t args_pool;
} kconfig_t;

#if defined(CONFIG_KARCH_ARMV7A)
#define KCONFIG_MAGIC KCONFIG_MAGIC_ARMV7
#elif defined(CONFIG_KARCH_ARMV8)
#define KCONFIG_MAGIC KCONFIG_MAGIC_ARMV8
#elif defined(CONFIG_KARCH_RISCV64)
#define KCONFIG_MAGIC KCONFIG_MAGIC_RISCV64
#else
#error "kconfig: Unsupported architecture"
#endif

#endif /* _KCONFIG_H_INCLUDED_ */
