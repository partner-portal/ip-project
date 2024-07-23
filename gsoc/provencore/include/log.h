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
 * @brief ProvenCore log helper macros
 * @author Vincent Siles - Henri Chataing
 * @date July 27th, 2016 (creation)
 * @copyright (c) 2016-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef LOG_H_INCLUDED_
#define LOG_H_INCLUDED_

#include "log_levels.h"
#include <compiler.h>
#include <stdarg.h>
#include <stdint.h>

#ifndef __PROVENCORE__
#  include <stdio.h>
#  include <syscall_internal.h>
#endif

struct log_feature {
    const char *feature_name;
    unsigned int feature_log_level; /* Value ranging from 0 to 31.
                                     * Messages of level lower or equal to this will be printed,
                                     * messages of level greater than this will be mute.
                                     */
    uint32_t location_loglevel_bitmap; /* Bitmap of log levels requiring to print location */
    uint32_t uptime_loglevel_bitmap; /* Bitmap of log levels requiring to print uptime */
};

/* There is a default_log_feature object statically defined.
 *
 * This object is used for pr_msg(), pr_warn(), ... functions.
 *
 * The bitmap used by this object are initiated from CONFIG_DEFAULT_LOG_UPTIME
 * and CONFIG_DEFAULT_LOG_LOCATION which can be defined through config.mk.
 *
 * By default, uptime is printed in application messages, but not locations.
 *
 * If CONFIG_DEFAULT_LOG_UPTIME is set, then this value is used as a bitmap for uptime.
 * Otherwise, if CONFIG_LOG_NO_UPTIME is set, then 0 is used instead,
 * otherwise, uptime is always printed.
 *
 * On the application level, this behavior can be dynamically changed by modifying these
 * fields in the default_log_feature.
 *
 * Remark: modifying default_log_feature in an application is supported,
 *         but since it is a static object, modifications will not affect other
 *         translation units.
 *         If you want to dynamically change the default features application wide,
 *         this is not possible, but you can create your own feature for that.
 */
#ifndef CONFIG_DEFAULT_LOG_UPTIME
#  ifdef CONFIG_LOG_NO_UPTIME
#    define CONFIG_DEFAULT_LOG_UPTIME UINT32_C(0)
#  else
#    define CONFIG_DEFAULT_LOG_UPTIME UINT32_MAX
#  endif
#endif

#ifndef CONFIG_DEFAULT_LOG_LOCATION
#  define CONFIG_DEFAULT_LOG_LOCATION UINT32_C(0)
#endif

/*
 * Support for legacy logging (single log feature at a time),
 * log levels are statically defined.
 *
 *
 * Undef all the necessary symbols
 */
#undef __CONFIG_APP_VERBOSE
#undef __CONFIG_APP_VERBOSE1
#undef __HAS_CONFIG_APP_VERBOSE
#undef __HAS_CONFIG_APP_VERBOSE1

#ifndef CONFIG_VERBOSE
#  define CONFIG_VERBOSE PR_WARN_VERBOSE
#endif


/**
 * The following variables must be externally defined:
 *    + APP_NAME_LO "[app]"
 *    + APP_NAME_UP [APP]
 *
 * Some macro definitions rely on GCC / Clang preprocessor extensions:
 *    + `##__VA_ARGS__` which clears the trailing comma for empty variadic
 *      argument lists.
 *    + evaluation of 'defined' operator in macro expansion
 */

#define CONFIG_APP_VERBOSE__(app) CONFIG_APP_VERBOSE1__(app)
#define CONFIG_APP_VERBOSE1__(app) CONFIG_##app##_VERBOSE

#ifndef APP_NAME_UP
#  define APP_NAME_UP DEF_LOG
#endif

/*
 * GCC7 added '-Wexpansion-to-defined'. Unfortunately, it is implemented
 *  in a non-useful way.
 * See:
 *  - https://bugs.webkit.org/show_bug.cgi?id=167643#c13
 *  - https://lists.llvm.org/pipermail/cfe-commits/Week-of-Mon-20160118/147239.html
 * It is a pain to disable this warning in the build system as GCC6 and below
 * will complain about '-Wno-expansion-to-defined'. Thus, we do it directly in
 * the code.
 */
#define HAS_CONFIG_APP_VERBOSE__(app) HAS_CONFIG_APP_VERBOSE1__(app)
#define HAS_CONFIG_APP_VERBOSE1__(app) defined(CONFIG_##app##_VERBOSE)

#if __GNUC__ >= 7
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wexpansion-to-defined"
#endif

#if HAS_CONFIG_APP_VERBOSE__(APP_NAME_UP)
#  undef CONFIG_VERBOSE
#  define CONFIG_VERBOSE CONFIG_APP_VERBOSE__(APP_NAME_UP)
#endif

#if __GNUC__ >= 7
#  pragma GCC diagnostic pop
#endif

#ifndef APP_NAME_LO
#  define APP_NAME_LO__(app_name_lo) #app_name_lo
#  define APP_NAME_LO APP_NAME_LO__(APP_NAME_UP)
#endif


/* PR_*_ENABLED macros are useful when you need to do an operation
 * in case debug is enabled only.
 * Example: int count = count_something(); pr_info("%d", count);
 *          ... no use of 'count' below
 * If the compiler cannot remove the call to 'count_something', you can
 * make it conditional on the PR_*_ENABLED macro.
 */

#if CONFIG_VERBOSE >= PR_FATAL_VERBOSE
    #define PR_FATAL_ENABLED
#endif

#if CONFIG_VERBOSE >= PR_ERR_VERBOSE
    #define PR_ERR_ENABLED
#endif

#if CONFIG_VERBOSE >= PR_WARN_VERBOSE
    #define PR_WARN_ENABLED
#endif

#if CONFIG_VERBOSE >= PR_INFO_VERBOSE
    #define PR_INFO_ENABLED
#endif

#if CONFIG_VERBOSE >= PR_DEBUG_VERBOSE
    #define PR_DEBUG_ENABLED
#endif


__maybe_unused
static struct log_feature default_log_feature = {
    .feature_name = APP_NAME_LO,
    .feature_log_level = CONFIG_VERBOSE,
    .location_loglevel_bitmap = CONFIG_DEFAULT_LOG_LOCATION,
    .uptime_loglevel_bitmap = CONFIG_DEFAULT_LOG_UPTIME,
};

#ifndef __PROVENCORE__
#include <sys/cdefs.h> /* __BEGIN_DECLS */
__BEGIN_DECLS
#endif

void vpr_raw(const struct log_feature *lf, const struct log_level *ll,
             const char *file, const char *func, unsigned int line,
             const char *format, va_list vl)
__printflike(6, 0);

#ifndef __PROVENCORE__
__END_DECLS
#endif


__maybe_unused __printflike(6, 7)
static void pr_raw(const struct log_feature *lf, const struct log_level *ll,
                   const char *file, const char *func, unsigned int line,
                   const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vpr_raw(lf, ll, file, func, line, format, ap);
    va_end(ap);
}

/******************************************************************************
 * PR_DEFINITIONS(NAME,VERB)
 *
 * generates two static functions, one with variadic arguments,
 * the other with va_list arguments.
 *
 * Their prototypes, with exception of the last argument is:
 *
 * - const struct log_feature *lf
 *       This is the log_feature used to print the message.
 *
 * - const char *file
 * - const char *func
 * - unsigned int line
 *       These arguments are for when location is to be printed.
 *
 * - const char *format
 *       To describe the following arguments (va_list or ellipsis).
 *
 * - returned type is void
 *
 * For instance, PR_DEFINITIONS(foo, PR_FOO_VERBOSE) will generate
 * - vpr_feat_foo_extra(): va_list variant to print messages at log level foo
 * - pr_feat_foo_extra(): variadic variant to print messages at log level foo
 */

#define PR_DEFINITIONS(NAME,VERB)                                         \
                                                                          \
    __printflike(5, 0)                                                    \
    static inline void vpr_feat_##NAME##_extra(                           \
        const struct log_feature *lf,                                     \
        const char *file, const char *func, unsigned int line,            \
        const char *format, va_list vl)                                   \
    {                                                                     \
        if (CONFIG_VERBOSE >= VERB) {                                     \
            vpr_raw(lf, &NAME##_log_level, file, func, line, format, vl); \
        }                                                                 \
    }                                                                     \
                                                                          \
    __maybe_unused __printflike(5, 6)                                     \
    static void pr_feat_##NAME##_extra(                                   \
        const struct log_feature *lf,                                     \
        const char *file, const char *func, unsigned int line,            \
        const char *format, ...)                                          \
    {                                                                     \
        if (CONFIG_VERBOSE >= VERB) {                                     \
            va_list ap;                                                   \
            va_start(ap, format);                                         \
            vpr_raw(lf, &NAME##_log_level, file, func, line, format, ap); \
            va_end(ap);                                                   \
        }                                                                 \
    }

/******************************************************************************
 * MSG
 *
 * Macro defined functions:
 *
 * void vpr_feat_msg_extra(const struct log_feature *lf,
 *                         const char *file, const char *func, unsigned int line,
 *                         const char *format, va_list vl);
 *
 * void vpr_feat_msg_extra(const struct log_feature *lf,
 *                         const char *file, const char *func, unsigned int line,
 *                         const char *format, ...);
 */
PR_DEFINITIONS(msg, PR_MSG_VERBOSE)

#define vpr_feat_msg(lf, fmt, ...)                   vpr_feat_msg_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define  pr_feat_msg(lf, fmt, ...)                    pr_feat_msg_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define          vpr_msg(fmt, ...) vpr_feat_msg_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define           pr_msg(fmt, ...)  pr_feat_msg_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)


/******************************************************************************
 * FATAL
 *
 * Macro defined functions:
 *
 * void vpr_feat_fatal_extra(const struct log_feature *lf,
 *                           const char *file, const char *func, unsigned int line,
 *                           const char *format, va_list vl);
 *
 * void vpr_feat_fatal_extra(const struct log_feature *lf,
 *                           const char *file, const char *func, unsigned int line,
 *                           const char *format, ...);
 */
PR_DEFINITIONS(fatal, PR_FATAL_VERBOSE)

#define vpr_feat_fatal(lf, fmt, ...)                   vpr_feat_fatal_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define  pr_feat_fatal(lf, fmt, ...)                    pr_feat_fatal_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define          vpr_fatal(fmt, ...) vpr_feat_fatal_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define           pr_fatal(fmt, ...)  pr_feat_fatal_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)


/******************************************************************************
 * ERR
 *
 * Macro defined functions:
 *
 * void vpr_feat_err_extra(const struct log_feature *lf,
 *                         const char *file, const char *func, unsigned int line,
 *                         const char *format, va_list vl);
 *
 * void vpr_feat_err_extra(const struct log_feature *lf,
 *                         const char *file, const char *func, unsigned int line,
 *                         const char *format, ...);
 */
PR_DEFINITIONS(err, PR_ERR_VERBOSE)

#define vpr_feat_err(lf, fmt, ...)                   vpr_feat_err_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define  pr_feat_err(lf, fmt, ...)                    pr_feat_err_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define          vpr_err(fmt, ...) vpr_feat_err_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define           pr_err(fmt, ...)  pr_feat_err_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)


/******************************************************************************
 * WARN
 *
 * Macro defined functions:
 *
 * void vpr_feat_warn_extra(const struct log_feature *lf,
 *                          const char *file, const char *func, unsigned int line,
 *                          const char *format, va_list vl);
 *
 * void vpr_feat_warn_extra(const struct log_feature *lf,
 *                          const char *file, const char *func, unsigned int line,
 *                          const char *format, ...);
 */
PR_DEFINITIONS(warn, PR_WARN_VERBOSE)

#define vpr_feat_warn(lf, fmt, ...)                   vpr_feat_warn_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define  pr_feat_warn(lf, fmt, ...)                    pr_feat_warn_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define          vpr_warn(fmt, ...) vpr_feat_warn_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define           pr_warn(fmt, ...)  pr_feat_warn_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)


/******************************************************************************
 * INFO
 *
 * Macro defined functions:
 *
 * void vpr_feat_info_extra(const struct log_feature *lf,
 *                          const char *file, const char *func, unsigned int line,
 *                          const char *format, va_list vl);
 *
 * void vpr_feat_info_extra(const struct log_feature *lf,
 *                          const char *file, const char *func, unsigned int line,
 *                          const char *format, ...);
 */
PR_DEFINITIONS(info, PR_INFO_VERBOSE)

#define vpr_feat_info(lf, fmt, ...)                   vpr_feat_info_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define  pr_feat_info(lf, fmt, ...)                    pr_feat_info_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define          vpr_info(fmt, ...) vpr_feat_info_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define           pr_info(fmt, ...)  pr_feat_info_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)


/******************************************************************************
 * DEBUG
 *
 * Macro defined functions:
 *
 * void vpr_feat_debug_extra(const struct log_feature *lf,
 *                           const char *file, const char *func, unsigned int line,
 *                           const char *format, va_list vl);
 *
 * void vpr_feat_debug_extra(const struct log_feature *lf,
 *                           const char *file, const char *func, unsigned int line,
 *                           const char *format, ...);
 */
PR_DEFINITIONS(debug, PR_DEBUG_VERBOSE)

#define vpr_feat_debug(lf, fmt, ...)                   vpr_feat_debug_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define  pr_feat_debug(lf, fmt, ...)                    pr_feat_debug_extra(lf, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define          vpr_debug(fmt, ...) vpr_feat_debug_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)
#define           pr_debug(fmt, ...)  pr_feat_debug_extra(&default_log_feature, __FILE__, __func__, __LINE__, fmt, ##__VA_ARGS__)


#endif /* LOG_H_INCLUDED_ */
