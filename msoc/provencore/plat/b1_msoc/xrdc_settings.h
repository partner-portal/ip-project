/*
 * Copyright (c) 2023-2024 ProvenRun S.A.S
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
 * @file  : xrdc_config.h
 * @brief : Contains all the XRDC settings specific to this board NXP S32G-VNP-RDB3
 * @author Aymeric PLOTON
 * @date December 5th, 2023 (creation)
 * @copyright (c) 2023-2024, Prove & Run and/or its affiliates. All rights reserved.
 */


#ifndef _XRDC_SETTINGS_H_INCLUDED_
#define _XRDC_SETTINGS_H_INCLUDED_

/*******************************************************************************
 * common utils macros and defines                                             *
 ******************************************************************************/

/* Defines -------------------------------------------------------------------*/

/*
 * Enable or disable XRDC security driver
 */

#define ENABLE_XRDC_SECURITY_DRIVER

/*
 * If the XRDC check does not match the required configuration,
 * the failure causes ProvenCore-A to crash.
 */
/* #define XRDC_FAIL_TO_PANIC */

/*
 * If the XRDC check presents security vulnerabilities,
 * the failure causes ProvenCore-A to crash.
 */
/* #define XRDC_BREACH_TO_PANIC */

/*
 * For debug purpose like print register and other information
 */
#define XRDC_DEBUG

/* Macros -------------------------------------------------------------------*/

/* MRC special requirement */
#define MRC_ADDR_REQ(addr)      UINT32_C(((addr) >> 4) | 0x1)


/******************************************************************************
 * S32G-vnp-rdb3 XRDC config                                                  *
 *****************************************************************************/

/* Protected area mapping ---------------------------------------------------*/

struct mrc_t {
    paddr_t start_pa;    /* Start physical address of the mrc memory mapped */
    paddr_t end_pa;    /* Start physical address of the mrc memory mapped */
};

/* All mrc to be check */
#define NR_MRC_PROTECTED        2
static const struct mrc_t mrcs0[] = {
    [0] = {.start_pa = MRC_ADDR_REQ(0x80000000), .end_pa = MRC_ADDR_REQ(0x82600000)},	/* cf platform.h */
};

/* General config -----------------------------------------------------------*/

#define XRDC0_BASE              0x401A4000
#define XRDC0_SIZE              0x00005000      /* page align */


/* Control Register(CR) config -------------------------------------------------
 *Diagram :
 * +-+-+---------------------+-+-+--+----+-+
 * | | |                     | | |  |    |G|
 * | |L|                     |V|M|  |  H |V|
 * | |K|                     |A|R|  |  R |L|
 * | |1|                     |W|F|  |  L |D|
 * +-+-+---------------------+-+-+--+----+-+
 */
#define XRDC0_CR_OFFSET         0x00000000

/* Memory Region General Descriptor (MRGD) config ------------------------------
 * Particular offset please see RMS32G3.pdf - page 614
 * #define MRGD_W0_(n * 16 + m)         0x(2000h + (n × 200h) + (m × 20h))
 * with :
 * +--------------+---------+
 * | Index n      | Index m |
 * +--------------+---------+
 * | 0, 2-10 , 13 |   0-15  |
 * |      11      |   0-11  |
 * |      12      |   0-03  |
 * +--------------+---------+
 * diagram : Specifies bits 35–5 of the 0-modulo 32-byte start address of the memory region.
 * 31                             1|0|
 * +-------------------------------+-+
 * |            SRTADDR            |X|
 * +-------------------------------+-+
 *
 * Same for WORD 1 but with a base offset of 0x2004
 * MRGD_W1_(n * 16 + m) 0x(2004h + (n × 200h) + (m × 20h))
 * diagram : Specifies bits 35–5 of the 31-modulo 32-byte end address of memory region r.
 * 31                             1|0|
 * +-------------------------------+-+
 * |            ENDADDR            |X|
 * +-------------------------------+-+
 *
 * Same for WORD 2 but with a base offset of 0x2008
 * MRGD_W2_(n * 16 + m) 0x(2008h + (n × 200h) + (m × 20h))
 * diagram : Specifies the ACP for the associated domain
 * 31                                      |2 0|
 * +-+-+--+----+---+---+---+---+---+---+---+---+
 * |X| |XX|  S | D | D | D | D | D | D | D | D |
 * |X|S|XX|  N | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * |X|E|XX|  U | A | A | A | A | A | A | A | A |
 * |X| |XX|  M | C | C | C | C | C | C | C | C |
 * |X| |XX|    | P | P | P | P | P | P | P | P |
 * +-+-+--+----+---+---+---+---+---+---+---+---+
 *
 * Same for WORD 3 but with a base offset of 0x200C
 * MRGD_W3_(n * 16 + m) 0x(200Ch + (n × 200h) + (m × 20h))
 * diagram : Specifies the ACP for the associated domain
 * 31                                     |2 0|
 * +-+--+-----+---+---+---+---+---+---+---+---+
 * | |  |XXXXX| D | D | D | D | D | D | D | D |
 * |V| L|XXXXX| 1 | 1 | 1 | 1 | 1 | 1 | 9 | 8 |
 * |L| K|XXXXX| 5 | 4 | 3 | 2 | 1 | 0 | A | A |
 * |D| 2|XXXXX| A | A | A | A | A | A | C | C |
 * | |  |XXXXX| C | C | C | C | C | C | P | P |
 * | |  |XXXXX| P | P | P | P | P | P |   |   |
 * +-+--+-----+---+---+---+---+---+---+---+---+
 */
#define XRDC0_MRGD_OFFSET               0x00002000                      /*0x(2000h + (0 × 200h) + (0 × 20h))*/
/* Offset of word 0 */
#define XRDC0_MRGD_W0_OFFSET            XRDC0_MRGD_OFFSET
/* Offset of word 1 */
#define XRDC0_MRGD_W1_OFFSET            XRDC0_MRGD_OFFSET + 0x00000004
/* Offset of word 2 */
#define XRDC0_MRGD_W2_OFFSET            XRDC0_MRGD_OFFSET + 0x00000008
/* Offset of word 3 */
#define XRDC0_MRGD_W3_OFFSET            XRDC0_MRGD_OFFSET + 0x0000000C


#endif /* _XRDC_SETTINGS_H_INCLUDED_ */

