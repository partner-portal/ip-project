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
 * @brief ProvenCore 'stdlib.h'
 * @author Cédric Auger
 * @date February 3rd, 2015 (creation)
 * @copyright (c) 2015-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBSTD_STDLIB_H_INCLUDED_
#define _LIBSTD_STDLIB_H_INCLUDED_

#include <stddef.h>             /* size_t */
#include <stdint.h>
#include <sys/cdefs.h>          /* __BEGIN_DECLS */

typedef struct {
    int quot;
    int rem;
} div_t;

typedef struct {
    long quot;
    long rem;
} ldiv_t;

typedef struct {
    long long quot;
    long long rem;
} lldiv_t;

typedef float float_t;
typedef double double_t;

__BEGIN_DECLS

#if 0
double             atof (const char *nptr);
double             strtod  (const char *restrict nptr, char **restrict endptr);
float              strtof  (const char *restrict nptr, char **restrict endptr); // C99
long double        strtold (const char *restrict nptr, char **restrict endptr); // C99

void *bsearch(const void *key, const void *base, size_t nmemb, size_t size,
              int (*compar)(const void *, const void *));
void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *));

int mblen(const char *s, size_t n);
int mbtowc(wchar_t *restrict pwc, const char *restrict s, size_t n);
int wctomb(char *s, wchar_t wchar);
size_t mbstowcs(wchar_t *restrict pwcs, const char *restrict s, size_t n);
size_t wcstombs(char *restrict s, const wchar_t *restrict pwcs, size_t n);

int at_quick_exit(void (*func)(void))                                     // C++11
void quick_exit(int status);                                              // C++11
#endif

int atoi(const char *nptr);
long atol(const char *nptr);
long long atoll(const char *nptr);
long strtol(const char *__restrict nptr, char **__restrict endptr, int base);
long long strtoll(const char *__restrict nptr, char **__restrict endptr,
                  int base); // C99
unsigned long strtoul(const char *__restrict nptr, char **__restrict endptr,
                      int base);
unsigned long long strtoull(const char *__restrict nptr,
                            char **__restrict endptr, int base); // C99

/**
 * @uses BREAK
 */
void *malloc(size_t size);
void free(void *ptr);
/**
 * @uses BREAK
 */
void *calloc(size_t nmemb, size_t size);
/**
 * @uses BREAK
 */
void *realloc(void *ptr, size_t size);
/**
 * @uses BREAK
 */
void *aligned_alloc(size_t alignment, size_t size); // C11
/**
 * @uses BREAK
 */
int posix_memalign(void **memptr, size_t alignment, size_t size);
/**
 * @uses BREAK
 */
void *aligned_realloc(void *ptr, size_t alignment, size_t size);

char *getenv(const char *name);
int putenv(char *entry);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);

_Noreturn void abort(void);
int atexit(void (*func)(void));
_Noreturn void _Exit(int status); // C99
_Noreturn void exit(int status);
_Noreturn void hang(void);

static inline int abs(int n)
{
    return (n < 0) ? -n : n;
}

static inline long labs(long n)
{
    return (n < 0) ? -n : n;
}

static inline long long llabs(long long n)
{
    return (n < 0) ? -n : n;
}

static inline div_t div(int num, int denom)
{
    div_t result;
    result.quot = num / denom;
    result.rem = num % denom;
    return result;
}

static inline ldiv_t ldiv(long num, long denom)
{
    ldiv_t result;
    result.quot = num / denom;
    result.rem = num % denom;
    return result;
}

static inline lldiv_t lldiv(long long num, long long denom)
{
    lldiv_t result;
    result.quot = num / denom;
    result.rem = num % denom;
    return result;
}

#if 0
/*
 * The library cannot provide strong enough random at the moment.
 * These functions are hidden.
 */

/* ISO C */
#define RAND_MAX 0x7fffffff
int rand(void);
void srand(unsigned seed);

/* POSIX Threads Extension (1003.1c-1995) */
int rand_r(unsigned *seed);

/* SVID issue 1 */
long jrand48(unsigned short xsubi[3]);
long mrand48(void);
long nrand48(unsigned short xsubi[3]);
long lrand48(void);
void lcong48(unsigned short param[7]);
unsigned short *seed48(unsigned short seed16v[3]);
void srand48(long seedval);
#endif

__END_DECLS

#endif /* _LIBSTD_STDLIB_H_INCLUDED_ */
