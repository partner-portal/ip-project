/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2020-2023, ProvenRun S.A.S
 */

/**
 * @file smc.h
 * @brief Internal provencore driver definitions for SMC handling
 *
 * This file is supposed to be shared between all provencore driver files only.
 *
 * @author Alexandre Berdery
 * @date October 6th, 2020 (creation)
 * @copyright (c) 2020-2023, Prove & Run and/or its affiliates.
 *   All rights reserved.
 */

#ifndef PNC_SMC_H_INCLUDED
#define PNC_SMC_H_INCLUDED

#include <linux/version.h>

#include <linux/arm-smccc.h>

/* Structure to package SMC params */
struct pnc_smc_params {
	uint32_t a0;
	uint32_t a1;
	uint32_t a2;
	uint32_t a3;
	uint32_t a4;
	uint32_t a5;
	uint32_t a6;
	uint32_t a7;
};

/* TZSW owned SMCs: Provencore uses '63' */
#define ARM_SMCCC_OWNER_PNC (ARM_SMCCC_OWNER_TRUSTED_OS + 13)

/* Below are constants used by REE driver for SMC handling */

#define LINUX_SHARED_MEM_TAG	0xcafe

#define SMC_ACTION_FROM_NS	\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, ARM_SMCCC_SMC_32, \
		ARM_SMCCC_OWNER_PNC, 4)

#define SMC_CONFIG_SHAREDMEM	\
	ARM_SMCCC_CALL_VAL(ARM_SMCCC_FAST_CALL, ARM_SMCCC_SMC_32, \
		ARM_SMCCC_OWNER_PNC, 3)

/**
 * @brief Schedule SMC execution the CPU 0.
 *
 * If the calling process is executed on CPU != 0, the function schedules SMC
 * work on CPU#0. Otherwise, it directly executes SMC.
 *
 * @param params        Parameters to the SMC call
 */
void pnc_sched_smc(struct pnc_smc_params *params);

/**
 * @brief Init REE driver's SMC framework.
 *
 * Only called during module init.
 */
int pnc_smc_init(void);

/**
 * @brief Release REE driver's SMC framework.
 *
 * Only called during module exit.
 */
void pnc_smc_exit(void);

#endif /* PNC_SMC_H_INCLUDED */
