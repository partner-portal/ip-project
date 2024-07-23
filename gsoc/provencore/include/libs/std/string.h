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
 * @brief "Standard" string.h header file
 * @author Vincent Siles
 * @date February 22th, 2019 (creation)
 * @copyright (c) 2019-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _STRING_H_INCLUDED_
#define _STRING_H_INCLUDED_

#include <stddef.h>
#include <sys/cdefs.h>          /* _BEGIN_DECLS */

#ifndef __pure
#define __pure __attribute__ ((pure))
#endif

__BEGIN_DECLS
/* c/c++ standard functions */
void *memchr(const void *ptr, int ch, size_t count);
int memcmp(const void *lhs, const void *rhs, size_t count);
void *memcpy(void *__restrict dest, const void *__restrict src, size_t count);
void *memmove(void *dest, const void *src, size_t count);
void *memset(void *dest, int ch, size_t count);

char *strncpy(char *__restrict dest, const char *__restrict src, size_t count);
char *strncat(char *__restrict dest, const char *__restrict src, size_t count);
int strcmp(const char *lhs, const char *rhs) __pure;
int strncmp(const char *lhs, const char *rhs, size_t count) __pure;
char *strchr(const char *str, int ch) __pure;
size_t strcspn(const char *dest, const char *src) __pure;
size_t strspn(const char *dest, const char *src) __pure;
char *strrchr(const char *str, int ch) __pure;
char *strerror(int errnum);
size_t strlen(const char *str) __pure;
size_t strnlen(const char *str, size_t maxlen) __pure;

/* posix standard functions */
char *strdup(const char *s);
char *strndup(const char *s, size_t size);

/* 4.4BSD */
char *strsep(char **stringp, const char *delim);

#if 0
/*
 * These functions may be implemented in the future.
 */
char *strpbrk(const char *dest, const char *breakset) __pure;
char *strstr(const char *str, const char *substr) __pure;
char *strtok(char *__restrict str, const char *__restrict delim);
size_t strxfrm(char *__restrict dest, const char *__restrict src, size_t count);

/*
 * These functions rely on the locale, which is not implemented in this
 * libc.
 */
int strcoll(const char *lhs, const char *rhs) __pure;

/*
 * These functions are not implemented on purpose.
 * Use the sized alternative for each one.
 */

char *strcpy(char *__restrict dest, const char *__restrict src);
char *strcat(char *__restrict dest, const char *__restrict src);
#endif

/*
 * Aliases to standard functions, which cannot be optimized by
 * the 'hosted' mode of compilers.
 */
void *__memcpy(void *__restrict dest, const void *__restrict src, size_t count);
void *__memset(void *dest, int ch, size_t count);
void *__memmove(void *dest, const void *src, size_t count);

__END_DECLS

#endif /* _STRING_H_INCLUDED_ */
