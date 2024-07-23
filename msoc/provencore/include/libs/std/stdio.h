/*
 * Copyright (c) 2017-2023 ProvenRun S.A.S
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
 * @brief Standard IO function and type definitions
 * @author Henri Chataing
 * @date September 29th, 2017 (creation)
 * @copyright (c) 2017-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _LIBSTD_STDIO_H_INCLUDED_
#define _LIBSTD_STDIO_H_INCLUDED_

#include <stdarg.h>
#include <stddef.h>
#include <sys/cdefs.h>          /* __BEGIN_DECLS, __printflike */
#include <sys/custom_file.h>    /* __FILE */

#ifndef __printflike
#define __printflike(n, m) __attribute__ ((format (printf, (n), (m))))
#endif

#define EOF             (-1)
#define BUFSIZ          (256)
#define FILENAME_MAX    (256)
#define FOPEN_MAX       (0)
#define L_tmpnam        (8)
#define TMP_MAX         (0)

#ifndef SEEK_CUR
#define SEEK_SET        (0)
#define SEEK_CUR        (1)
#define SEEK_END        (2)
#endif

#define stdin           (_stdin)
#define stdout          (_stdout)
#define stderr          (_stderr)

typedef __FILE FILE;
typedef long fpos_t;

typedef size_t rsize_t; // C11

__BEGIN_DECLS

extern FILE * const _stdin;
extern FILE * const _stdout;
extern FILE * const _stderr;

/** Operation on files. */
int remove(const char *filename);
int rename(const char *oldname, const char *newname);
FILE *tmpfile(void);
char *tmpnam(char *s);

/** File access. */
int fclose(FILE *stream);
int fflush(FILE *stream);
FILE *fopen(const char *__restrict filename, const char *__restrict mode);
FILE *freopen(const char *__restrict filename, const char *__restrict mode,
              FILE *__restrict stream);
void setbuf(FILE *__restrict stream, char *__restrict buf);
int setvbuf(FILE *__restrict stream, char *__restrict buf,
            int mode, size_t size);

/** Formatted input/output. */

/**
 * @uses SENDREC GETINFO SETAUTH
 */
int fprintf(FILE *__restrict stream, const char *__restrict format, ...);

int fscanf(FILE *__restrict stream, const char *__restrict format, ...);

/**
 * @uses SENDREC GETINFO SETAUTH
 */
int printf(const char *__restrict format, ...) __printflike(1, 2);

int scanf(const char *__restrict format, ...);

int snprintf(char *__restrict s, size_t n, const char *__restrict format, ...)
__printflike(3, 4); // C99
int sprintf(char *__restrict s, const char *__restrict format, ...)
__printflike(2, 3);
int sscanf(const char *__restrict s, const char *__restrict format, ...);

/**
 * @uses SENDREC GETINFO SETAUTH
 */
int vfprintf(FILE *__restrict stream, const char *__restrict format,
             va_list arg);
int vfscanf(FILE *__restrict stream, const char *__restrict format,
            va_list arg); // C99

/**
 * @uses SENDREC GETINFO SETAUTH
 */
int vprintf(const char *__restrict format, va_list arg) __printflike(1, 0);
int vscanf(const char *__restrict format, va_list arg); // C99

int vsnprintf(char *__restrict s, size_t n, const char *__restrict format,
              va_list arg) __printflike(3, 0);  // C99
int vsprintf(char *__restrict s, const char *__restrict format, va_list arg)
__printflike(2, 0);
int vsscanf(const char *__restrict s, const char *__restrict format,
            va_list arg); // C99

/** Character input/output. */
/**
 * @uses SENDREC GETINFO
 */
int fgetc(FILE *stream);
char *fgets(char *__restrict s, int n, FILE *__restrict stream);
/**
 * @uses SENDREC GETINFO SETAUTH
 */
int fputc(int c, FILE *stream);
/**
 * @uses SENDREC GETINFO SETAUTH REVOKE
 */
int fputs(const char *__restrict s, FILE *__restrict stream);
/**
 * @uses SENDREC GETINFO
 */
int getc(FILE *stream);
int getchar(void);
/**
 * @uses SENDREC GETINFO
 */
int putc(int c, FILE *stream);
/**
 * @uses SENDREC GETINFO SETAUTH
 */
int putchar(int c);
/**
 * @uses SENDREC GETINFO SETAUTH REVOKE
 */
__attribute__((deprecated("Please use puts_s instead")))
int puts(const char *s);
int ungetc(int c, FILE *stream);

char *gets_s(char *str, rsize_t n); // C11
/**
 * @uses SENDREC GETINFO SETAUTH REVOKE
 */
int puts_s(const char *str, rsize_t n); // Non standard !

/** Direct input/output. */
size_t fread(void *__restrict ptr, size_t size, size_t nmemb,
             FILE *__restrict stream);
size_t fwrite(const void *__restrict ptr, size_t size, size_t nmemb,
              FILE *__restrict stream);

/** File positioning. */
int fgetpos(FILE *__restrict stream, fpos_t *__restrict pos);
int fseek(FILE *stream, long offset, int whence);
int fsetpos(FILE *stream, const fpos_t *pos);
long ftell(FILE *stream);
void rewind(FILE *stream);

/** Error handling. */
void clearerr(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);
void perror(const char *s);

__END_DECLS

#endif /* _LIBSTD_STDIO_H_INCLUDED_ */
