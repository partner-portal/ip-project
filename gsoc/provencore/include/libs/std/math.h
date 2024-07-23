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
 * @brief Math builtin definitions
 * @author Henri Chataing
 * @date February 08th, 2018 (creation)
 * @copyright (c) 2018-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBSTD_MATH_H_INCLUDED_
#define _LIBSTD_MATH_H_INCLUDED_

/*
 * Note: the header is missing some macro defines implementing generic
 * versions of float / double / long double math methods.
 * Unless necessary, they will not be implemented.
 */

#include <stdlib.h>             /* float_t, double_t */
#include <sys/cdefs.h>          /* __BEGIN_DECLS */

#define HUGE_VAL        (__builtin_huge_val())
#define HUGE_VALF       (__builtin_huge_valf())
#define HUGE_VALL       (__builtin_huge_vall())
#define INFINITY        (__builtin_inff())
/* #define NAN */

#define FP_NAN 0
#define FP_INFINITE 1
#define FP_NORMAL 2
#define FP_SUBNORMAL 3
#define FP_ZERO 4

/* Standard macros */
#define fpclassify(...) (__builtin_fpclassify(FP_NAN, FP_INFINITE, FP_NORMAL, \
                                              FP_SUBNORMAL, FP_ZERO, \
                                              __VA_ARGS__))
#define isfinite(...)   (__builtin_isfinite(__VA_ARGS__))
#define isgreater(...)  (__builtin_isgreater(__VA_ARGS__))
#define isgreaterequal(...)     (__builtin_isgreaterequal(__VA_ARGS__))
#define isinf(...)      (__builtin_isinf(__VA_ARGS__))
#define isless(...)     (__builtin_isless(__VA_ARGS__))
#define islessequal(...)        (__builtin_islessequal(__VA_ARGS__))
#define islessgreater(...)      (__builtin_islessgreater(__VA_ARGS__))
#define isnan(...)      (__builtin_isnan(__VA_ARGS__))
#define isnormal(...)   (__builtin_isnormal(__VA_ARGS__))
#define isunordered(...)        (__builtin_isunordered(__VA_ARGS__))
#define signbit(...)    (__builtin_signbit(__VA_ARGS__))

__BEGIN_DECLS

/* Standard functions */
static inline double acos(double arg)
{
    return __builtin_acos(arg);
}

static inline float acosf(float arg)
{
    return __builtin_acosf(arg);
}

static inline long double acosl(long double arg)
{
    return __builtin_acosl(arg);
}

static inline double acosh(double arg)
{
    return __builtin_acosh(arg);
}

static inline float acoshf(float arg)
{
    return __builtin_acoshf(arg);
}

static inline long double acoshl(long double arg)
{
    return __builtin_acoshl(arg);
}

static inline double asin(double arg)
{
    return __builtin_asin(arg);
}

static inline float asinf(float arg)
{
    return __builtin_asinf(arg);
}

static inline long double asinl(long double arg)
{
    return __builtin_asinl(arg);
}

static inline double asinh(double arg)
{
    return __builtin_asinh(arg);
}

static inline float asinhf(float arg)
{
    return __builtin_asinhf(arg);
}

static inline long double asinhl(long double arg)
{
    return __builtin_asinhl(arg);
}

static inline double atan(double arg)
{
    return __builtin_atan(arg);
}

static inline float atanf(float arg)
{
    return __builtin_atanf(arg);
}

static inline long double atanl(long double arg)
{
    return __builtin_atanl(arg);
}

static inline double atan2(double x, double y)
{
    return __builtin_atan2(x, y);
}

static inline float atan2f(float x, float y)
{
    return __builtin_atan2f(x, y);
}

static inline long double atan2l(long double x, long double y)
{
    return __builtin_atan2l(x, y);
}

static inline double atanh(double arg)
{
    return __builtin_atanh(arg);
}

static inline float atanhf(float arg)
{
    return __builtin_atanhf(arg);
}

static inline long double atanhl(long double arg)
{
    return __builtin_atanhl(arg);
}

static inline double cbrt(double arg)
{
    return __builtin_cbrt(arg);
}

static inline float cbrtf(float arg)
{
    return __builtin_cbrtf(arg);
}

static inline long double cbrtl(long double arg)
{
    return __builtin_cbrtl(arg);
}

static inline double ceil(double arg)
{
    return __builtin_ceil(arg);
}

static inline float ceilf(float arg)
{
    return __builtin_ceilf(arg);
}

static inline long double ceill(long double arg)
{
    return __builtin_ceill(arg);
}

static inline double copysign(double x, double y)
{
    return __builtin_copysign(x, y);
}

static inline float copysignf(float x, float y)
{
    return __builtin_copysignf(x, y);
}

static inline long double copysignl(long double x, long double y)
{
    return __builtin_copysignl(x, y);
}

static inline double cos(double arg)
{
    return __builtin_cos(arg);
}

static inline float cosf(float arg)
{
    return __builtin_cosf(arg);
}

static inline long double cosl(long double arg)
{
    return __builtin_cosl(arg);
}

static inline double cosh(double arg)
{
    return __builtin_cosh(arg);
}

static inline float coshf(float arg)
{
    return __builtin_coshf(arg);
}

static inline long double coshl(long double arg)
{
    return __builtin_coshl(arg);
}

static inline double erf(double arg)
{
    return __builtin_erf(arg);
}

static inline float erff(float arg)
{
    return __builtin_erff(arg);
}

static inline long double erfl(long double arg)
{
    return __builtin_erfl(arg);
}

static inline double erfc(double arg)
{
    return __builtin_erfc(arg);
}

static inline float erfcf(float arg)
{
    return __builtin_erfcf(arg);
}

static inline long double erfcl(long double arg)
{
    return __builtin_erfcl(arg);
}

static inline double exp(double arg)
{
    return __builtin_exp(arg);
}

static inline float expf(float arg)
{
    return __builtin_expf(arg);
}

static inline long double expl(long double arg)
{
    return __builtin_expl(arg);
}

static inline double exp2(double arg)
{
    return __builtin_exp2(arg);
}

static inline float exp2f(float arg)
{
    return __builtin_exp2f(arg);
}

static inline long double exp2l(long double arg)
{
    return __builtin_exp2l(arg);
}

static inline double expm1(double arg)
{
    return __builtin_expm1(arg);
}

static inline float expm1f(float arg)
{
    return __builtin_expm1f(arg);
}

static inline long double expm1l(long double arg)
{
    return __builtin_expm1l(arg);
}

static inline double fabs(double arg)
{
    return __builtin_fabs(arg);
}

static inline float fabsf(float arg)
{
    return __builtin_fabsf(arg);
}

static inline long double fabsl(long double arg)
{
    return __builtin_fabsl(arg);
}

static inline double fdim(double x, double y)
{
    return __builtin_fdim(x, y);
}

static inline float fdimf(float x, float y)
{
    return __builtin_fdimf(x, y);
}

static inline long double fdiml(long double x, long double y)
{
    return __builtin_fdiml(x, y);
}

static inline double floor(double arg)
{
    return __builtin_floor(arg);
}

static inline float floorf(float arg)
{
    return __builtin_floorf(arg);
}

static inline long double floorl(long double arg)
{
    return __builtin_floorl(arg);
}

static inline double fma(double x, double y, double z)
{
    return __builtin_fma(x, y, z);
}

static inline float fmaf(float x, float y, float z)
{
    return __builtin_fmaf(x, y, z);
}

static inline long double fmal(long double x, long double y, long double z)
{
    return __builtin_fmal(x, y, z);
}

static inline double fmax(double x, double y)
{
    return __builtin_fmax(x, y);
}

static inline float fmaxf(float x, float y)
{
    return __builtin_fmaxf(x, y);
}

static inline long double fmaxl(long double x, long double y)
{
    return __builtin_fmaxl(x, y);
}

static inline double fmin(double x, double y)
{
    return __builtin_fmin(x, y);
}

static inline float fminf(float x, float y)
{
    return __builtin_fminf(x, y);
}

static inline long double fminl(long double x, long double y)
{
    return __builtin_fminl(x, y);
}

static inline double fmod(double x, double y)
{
    return __builtin_fmod(x, y);
}

static inline float fmodf(float x, float y)
{
    return __builtin_fmodf(x, y);
}

static inline long double fmodl(long double x, long double y)
{
    return __builtin_fmodl(x, y);
}

static inline double frexp(double arg, int *exp)
{
    return __builtin_frexp(arg, exp);
}

static inline float frexpf(float arg, int *exp)
{
    return __builtin_frexpf(arg, exp);
}

static inline long double frexpl(long double arg, int *exp)
{
    return __builtin_frexpl(arg, exp);
}

static inline double hypot(double x, double y)
{
    return __builtin_hypot(x, y);
}

static inline float hypotf(float x, float y)
{
    return __builtin_hypotf(x, y);
}

static inline long double hypotl(long double x, long double y)
{
    return __builtin_hypotl(x, y);
}

static inline int ilogb(double arg)
{
    return __builtin_ilogb(arg);
}

static inline int ilogbf(float arg)
{
    return __builtin_ilogbf(arg);
}

static inline int ilogbl(long double arg)
{
    return __builtin_ilogbl(arg);
}

static inline double ldexp(double arg, int exp)
{
    return __builtin_ldexp(arg, exp);
}

static inline float ldexpf(float arg, int exp)
{
    return __builtin_ldexpf(arg, exp);
}

static inline long double ldexpl(long double arg, int exp)
{
    return __builtin_ldexpl(arg, exp);
}

static inline double lgamma(double arg)
{
    return __builtin_lgamma(arg);
}

static inline float lgammaf(float arg)
{
    return __builtin_lgammaf(arg);
}

static inline long double lgammal(long double arg)
{
    return __builtin_lgammal(arg);
}

static inline long long llrint(double arg)
{
    return __builtin_llrint(arg);
}

static inline long long llrintf(float arg)
{
    return __builtin_llrintf(arg);
}

static inline long long llrintl(long double arg)
{
    return __builtin_llrintl(arg);
}

static inline long long llround(double arg)
{
    return __builtin_llround(arg);
}

static inline long long llroundf(float arg)
{
    return __builtin_llroundf(arg);
}

static inline long long llroundl(long double arg)
{
    return __builtin_llroundl(arg);
}

static inline double log(double arg)
{
    return __builtin_log(arg);
}

static inline float logf(float arg)
{
    return __builtin_logf(arg);
}

static inline long double logl(long double arg)
{
    return __builtin_logl(arg);
}

static inline double log10(double arg)
{
    return __builtin_log10(arg);
}

static inline float log10f(float arg)
{
    return __builtin_log10f(arg);
}

static inline long double log10l(long double arg)
{
    return __builtin_log10l(arg);
}

static inline double log1p(double arg)
{
    return __builtin_log1p(arg);
}

static inline float log1pf(float arg)
{
    return __builtin_log1pf(arg);
}

static inline long double log1pl(long double arg)
{
    return __builtin_log1pl(arg);
}

static inline double log2(double arg)
{
    return __builtin_log2(arg);
}

static inline float log2f(float arg)
{
    return __builtin_log2f(arg);
}

static inline long double log2l(long double arg)
{
    return __builtin_log2l(arg);
}

static inline double logb(double arg)
{
    return __builtin_logb(arg);
}

static inline float logbf(float arg)
{
    return __builtin_logbf(arg);
}

static inline long double logbl(long double arg)
{
    return __builtin_logbl(arg);
}

static inline long lrint(double arg)
{
    return __builtin_lrint(arg);
}

static inline long lrintf(float arg)
{
    return __builtin_lrintf(arg);
}

static inline long lrintl(long double arg)
{
    return __builtin_lrintl(arg);
}

static inline long lround(double arg)
{
    return __builtin_lround(arg);
}

static inline long lroundf(float arg)
{
    return __builtin_lroundf(arg);
}

static inline long lroundl(long double arg)
{
    return __builtin_lroundl(arg);
}

static inline float modf(double arg, double *iptr)
{
    return __builtin_modf(arg, iptr);
}

static inline float modff(float arg, float *iptr)
{
    return __builtin_modff(arg, iptr);
}

static inline long double modfl(long double arg, long double *iptr)
{
    return __builtin_modfl(arg, iptr);
}

static inline double nan(const char *arg)
{
    return __builtin_nan(arg);
}

static inline float nanf(const char *arg)
{
    return __builtin_nanf(arg);
}

static inline long double nanl(const char *arg)
{
    return __builtin_nanl(arg);
}

static inline double nearbyint(double arg)
{
    return __builtin_nearbyint(arg);
}

static inline float nearbyintf(float arg)
{
    return __builtin_nearbyintf(arg);
}

static inline long double nearbyintl(long double arg)
{
    return __builtin_nearbyintl(arg);
}

static inline double nextafter(double from, double to)
{
    return __builtin_nextafter(from, to);
}

static inline float nextafterf(float from, float to)
{
    return __builtin_nextafterf(from, to);
}

static inline long double nextafterl(long double from, long double to)
{
    return __builtin_nextafterl(from, to);
}

static inline double nexttoward(double from, long double to)
{
    return __builtin_nexttoward(from, to);
}

static inline float nexttowardf(float from, long double to)
{
    return __builtin_nexttowardf(from, to);
}

static inline long double nexttowardl(long double from, long double to)
{
    return __builtin_nexttowardl(from, to);
}

static inline double pow(double base, double exponent)
{
    return __builtin_pow(base, exponent);
}

static inline float powf(float base, float exponent)
{
    return __builtin_powf(base, exponent);
}

static inline long double powl(long double base, long double exponent)
{
    return __builtin_powl(base, exponent);
}

static inline double remainder(double x, double y)
{
    return __builtin_remainder(x, y);
}

static inline float remainderf(float x, float y)
{
    return __builtin_remainderf(x, y);
}

static inline long double remainderl(long double x, long double y)
{
    return __builtin_remainderl(x, y);
}

static inline double remquo(double x, double y, int *quo)
{
    return __builtin_remquo(x, y, quo);
}

static inline float remquof(float x, float y, int *quo)
{
    return __builtin_remquof(x, y, quo);
}

static inline long double remquol(long double x, long double y, int *quo)
{
    return __builtin_remquol(x, y, quo);
}

static inline double rint(double arg)
{
    return __builtin_rint(arg);
}

static inline float rintf(float arg)
{
    return __builtin_rintf(arg);
}

static inline long double rintl(long double arg)
{
    return __builtin_rintl(arg);
}

static inline double round(double arg)
{
    return __builtin_round(arg);
}

static inline float roundf(float arg)
{
    return __builtin_roundf(arg);
}

static inline long double roundl(long double arg)
{
    return __builtin_roundl(arg);
}

static inline double scalbln(double arg, long exp)
{
    return __builtin_scalbln(arg, exp);
}

static inline float scalblnf(float arg, long exp)
{
    return __builtin_scalblnf(arg, exp);
}

static inline long double scalblnl(long double arg, long exp)
{
    return __builtin_scalblnl(arg, exp);
}

static inline double scalbn(double arg, int exp)
{
    return __builtin_scalbn(arg, exp);
}

static inline float scalbnf(float arg, int exp)
{
    return __builtin_scalbnf(arg, exp);
}

static inline long double scalbnl(long double arg, int exp)
{
    return __builtin_scalbnl(arg, exp);
}

static inline double sin(double arg)
{
    return __builtin_sin(arg);
}

static inline float sinf(float arg)
{
    return __builtin_sinf(arg);
}

static inline long double sinl(long double arg)
{
    return __builtin_sinl(arg);
}

static inline double sinh(double arg)
{
    return __builtin_sinh(arg);
}

static inline float sinhf(float arg)
{
    return __builtin_sinhf(arg);
}

static inline long double sinhl(long double arg)
{
    return __builtin_sinhl(arg);
}

static inline double sqrt(double arg)
{
    return __builtin_sqrt(arg);
}

static inline float sqrtf(float arg)
{
    return __builtin_sqrtf(arg);
}

static inline long double sqrtl(long double arg)
{
    return __builtin_sqrtl(arg);
}

static inline double tan(double arg)
{
    return __builtin_tan(arg);
}

static inline float tanf(float arg)
{
    return __builtin_tanf(arg);
}

static inline long double tanl(long double arg)
{
    return __builtin_tanl(arg);
}

static inline double tanh(double arg)
{
    return __builtin_tanh(arg);
}

static inline float tanhf(float arg)
{
    return __builtin_tanhf(arg);
}

static inline long double tanhl(long double arg)
{
    return __builtin_tanhl(arg);
}

static inline double tgamma(double arg)
{
    return __builtin_tgamma(arg);
}

static inline float tgammaf(float arg)
{
    return __builtin_tgammaf(arg);
}

static inline long double tgammal(long double arg)
{
    return __builtin_tgammal(arg);
}

static inline double trunc(double arg)
{
    return __builtin_trunc(arg);
}

static inline float truncf(float arg)
{
    return __builtin_truncf(arg);
}

static inline long double truncl(long double arg)
{
    return __builtin_truncl(arg);
}

__END_DECLS

/* Mathematical constants */
#define M_E             2.71828182845904523536
#define M_LOG2E         1.44269504088896340736
#define M_LOG10E        0.434294481903251827651
#define M_LN2           0.693147180559945309417
#define M_LN10          2.30258509299404568402
#define M_PI            3.14159265358979323846
#define M_PI_2          1.57079632679489661923
#define M_PI_4          0.785398163397448309616
#define M_1_PI          0.318309886183790671538
#define M_2_PI          0.636619772367581343076
#define M_1_SQRTPI      0.564189583547756286948
#define M_2_SQRTPI      1.12837916709551257390
#define M_SQRT2         1.41421356237309504880
#define M_SQRT_2        0.707106781186547524401

#endif /* _LIBSTD_MATH_H_INCLUDED_ */
