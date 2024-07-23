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
 * @brief ProvenCore log levels definitions
 * @author Hadrien Barral
 * @date May 7th, 2019 (creation)
 * @copyright (c) 2019-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef LOG_LEVELS_H_INCLUDED_
#define LOG_LEVELS_H_INCLUDED_

struct log_level {
    unsigned int internal_log_level;
    const char *prefix; /* Prefix of the message, used to set the text color for instance */
    const char *infix; /* Pattern to provide a hint on the kind of message, such as !!! for errors */
    const char *suffix; /* Suffix of the message, used to reset the text color for instance */
};

extern const struct log_level msg_log_level;
extern const struct log_level fatal_log_level;
extern const struct log_level err_log_level;
extern const struct log_level warn_log_level;
extern const struct log_level info_log_level;
extern const struct log_level debug_log_level;

#define PR_MSG_VERBOSE    0
#define PR_FATAL_VERBOSE  0
#define PR_ERR_VERBOSE    2
#define PR_WARN_VERBOSE   5
#define PR_INFO_VERBOSE  10
#define PR_DEBUG_VERBOSE 20

#endif /* LOG_LEVELS_H_INCLUDED_ */
