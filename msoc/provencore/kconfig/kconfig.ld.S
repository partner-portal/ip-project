/*
 * Copyright (c) 2015-2023 ProvenRun S.A.S
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
 * @brief Linker script for the secure kernel configuration file
 * @author Vincent Siles
 * @date September 15th, 2015 (creation)
 * @copyright (c) 2015-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#if (!defined(CONFIG_LINKER_ARCH)) || (!defined(CONFIG_LINKER_FORMAT))
    #error CONFIG_LINKER_ARCH and CONFIG_LINKER_FORMAT must be defined
#endif

OUTPUT_ARCH(CONFIG_LINKER_ARCH)
OUTPUT_FORMAT(CONFIG_LINKER_FORMAT)

SECTIONS
{
    /*
     * Start at 0, all virtual addresses will be offsets from the start of
     * the kernel config binary.
     */
    . = 0;
    _start = .;

    .data :
    {
        *(.kconfig.header)
        *(.kconfig.irq_masks)
        *(.kconfig.device_masks)
        *(.kconfig.target_masks)
        *(.kconfig.devices)
        *(.kconfig.images)
        *(.kconfig.sec_entries)
        *(.kconfig.args_pool)
    }

    /* LLD complains if we remove these sections.
     * Note: I tried adding an assert that these are indeed empty,
     *  but it turns out to be false even if there are no such sections
     *  in the output file.
     *   This is not too much of an issue since we only extract .data in
     *  later stages.
     */
    .shstrtab : { *(.shstrtab) }
    .rel.plt : { *(.rel.plt) }
    .rela.plt : { *(.rela.plt) }

    /DISCARD/ :
    {
        *(.*)
    }
}
