/*
 * Copyright (c) 2014-2023 ProvenRun S.A.S
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
 * @brief Error code listing
 * @author Vincent Siles
 * @date April 14th, 2014 (creation)
 * @copyright (c) 2014-2023, ProvenRun S.A.S and/or its affiliates.
 *   All rights reserved.
 */

#ifndef _ERRNO_H_INCLUDED_
#define _ERRNO_H_INCLUDED_

#include <compiler.h>

/*
 * These error numbers are _unsigned 32 bit integers_.
 * By convention, the value ZERO is reserved for [OK], the unique error code
 * denoting "success". Other error codes' actual values are unspecified, only
 * that they must differ from [OK].
 */
#define OK            0

#ifndef __PROVENCORE__
__BEGIN_DECLS
#endif

extern int errno;
int *__errno (void);

#ifndef __PROVENCORE__
__END_DECLS
#endif

/*
 * Most of the following error codes are POSIX codes.
 * We list here the few exceptions:
 * - ESHUTDOWN is not POSIX, but defined by GNU libc
 *   See https://www.gnu.org/software/libc/manual/html_node/Error-Codes.html
 */
#define EPERM           (  1)   /* Operation not permitted */
#define ENOENT          (  2)   /* No such file or directory */
#define ESRCH           (  3)   /* No such process */
#define EINTR           (  4)   /* Interrupted function call */
#define EIO             (  5)   /* Input/output error */
#define ENXIO           (  6)   /* No such device or address */
#define E2BIG           (  7)   /* Argument list too long */
#define ENOEXEC         (  8)   /* Exec format error */
#define EBADF           (  9)   /* Bad file descriptor */
#define ECHILD          ( 10)   /* No child processes */
#define EAGAIN          ( 11)   /* Resource temporarily unavailable */
#define EWOULDBLOCK     EAGAIN  /* Possible outcome might be blocked */
#define ENOMEM          ( 12)   /* Not enough space */
#define EACCES          ( 13)   /* Permission denied */
#define EFAULT          ( 14)   /* Bad address */
#define EBUSY           ( 16)   /* Device or resource busy */
#define EEXIST          ( 17)   /* File exists */
#define EXDEV           ( 18)   /* Improper link */
#define ENODEV          ( 19)   /* No such device */
#define ENOTDIR         ( 20)   /* Not a directory */
#define EISDIR          ( 21)   /* Is a directory */
#define EINVAL          ( 22)   /* Invalid argument */
#define ENFILE          ( 23)   /* Too many open files in system */
#define EMFILE          ( 24)   /* Too many open files */
#define ENOTTY          ( 25)   /* Inappropriate I/O control operation */
#define ETXTBSY         ( 26)   /* Text file busy */
#define EFBIG           ( 27)   /* File too large */
#define ENOSPC          ( 28)   /* No space left on device */
#define ESPIPE          ( 29)   /* Invalid seek */
#define EROFS           ( 30)   /* Read-only filesystem */
#define EMLINK          ( 31)   /* Too many links */
#define EPIPE           ( 32)   /* Broken pipe */
#define EDOM            ( 33)   /* Mathematics argument out of domain of function (C99) */
#define ERANGE          ( 34)   /* Result too large */
#define EDEADLK         ( 35)   /* Resource deadlock avoided */
#define ENAMETOOLONG    ( 36)   /* Filename too long */
#define ENOLCK          ( 37)   /* No locks available */
#define ENOSYS          ( 38)   /* Function not implemented */
#define ENOTEMPTY       ( 39)   /* Directory not empty */
#define ELOOP           ( 40)   /* Too many levels of symbolic links */
#define ENOMSG          ( 42)   /* No message of the desired type */
#define EIDRM           ( 43)   /* Identifier removed */
#define ENOSTR          ( 60)   /* Not a STREAM */
#define ENODATA         ( 61)   /* No message is available on the STREAM head read queue */
#define ETIME           ( 62)   /* Timer expired */
#define ENOSR           ( 63)   /* No STREAM resources */
#define ENOLINK         ( 67)   /* Link has been severed */
#define EPROTO          ( 71)   /* Protocol error */
#define EMULTIHOP       ( 72)   /* Multihop attempted */
#define EBADMSG         ( 74)   /* Bad message */
#define EOVERFLOW       ( 75)   /* Value too large to be stored in data type */
#define EILSEQ          ( 84)   /* Illegal byte sequence */
#define ENOTSOCK        ( 88)   /* Not a socket */
#define EDESTADDRREQ    ( 89)   /* Destination address required */
#define EMSGSIZE        ( 90)   /* Message too long */
#define EPROTOTYPE      ( 91)   /* Protocol wrong type for socket */
#define ENOPROTOOPT     ( 92)   /* Protocol not available */
#define EPROTONOSUPPORT ( 93)   /* Protocol not supported */
#define ENOTSUP         ( 95)   /* Operation not supported */
/*
 * ENOTSUP and EOPNOTSUPP have the same value on Linux,
 * but according to POSIX.1 these error values should be distinct.
 *
 * However, POSIX.1-2017 allows them to be equal.
 * http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/errno.h.html#tag_13_10
 */
#define EOPNOTSUPP      ( 95)   /* Operation not supported on socket */
#define EAFNOSUPPORT    ( 97)   /* Address family not supported */
#define EADDRINUSE      ( 98)   /* Address already in use */
#define EADDRNOTAVAIL   ( 99)   /* Address not available */
#define ENETDOWN        (100)   /* Network is down */
#define ENETUNREACH     (101)   /* Network unreachable */
#define ENETRESET       (102)   /* Connection aborted by network */
#define ECONNABORTED    (103)   /* Connection aborted */
#define ECONNRESET      (104)   /* Connection reset */
#define ENOBUFS         (105)   /* No buffer space available */
#define EISCONN         (106)   /* Socket is connected */
#define ENOTCONN        (107)   /* The socket is not connected */
#define ESHUTDOWN       (108)   /* Content send after transport endpoint shutdown */
#define ETIMEDOUT       (110)   /* Connection timed out */
#define ECONNREFUSED    (111)   /* Connection refused */
#define EHOSTUNREACH    (113)   /* Host is unreachable */
#define EALREADY        (114)   /* Connection already in progress */
#define EINPROGRESS     (115)   /* Operation in progress */
#define ESTALE          (116)   /* Stale file handle */
#define EDQUOT          (122)   /* Disk quota exceeded */
#define ECANCELED       (125)   /* Operation canceled */
#define EOWNERDEAD      (130)   /* Previous owner died */
#define ENOTRECOVERABLE (131)   /* State not recoverable */

/*
 * The rest are non-standard, inherited from Minix (and patched to avoid dups),
 * or used by ProvenCore answers.
 *
 * It seems standard errno are less than 129 (by experiment), so we decided to
 * put these one about 200
 */
#define EGENERIC        (200)   /* Generic error */
#define EPACKSIZE       (201)   /* Invalid packet size for some protocol */
#define EOUTOFBUFS      (202)   /* Not enough buffers left */
#define EBADIOCTL       (203)   /* Illegal ioctl for device */
#define EBADMODE        (204)   /* Badmode in ioctl */
#define EBADDEST        (205)   /* Not a valid destination address */
#define EDSTNOTRCH      (206)   /* Destination not reachable */
#define EURG            (207)   /* Urgent data present */
#define ENOURG          (208)   /* No urgent data present */
#define ENOCONN         (209)   /* No such connection */
/* The following errors relate to ProvenCore. */
#define ELOCKED         (210)   /* Can't send message due to deadlock */
#define EBADCALL        (211)   /* Illegal system call number */
#define EBADSRCDST      (212)   /* Bad source or destination process */
#define ECALLDENIED     (213)   /* No permission for system call */
#define ENOTREADY       (214)   /* Source or destination is not ready */
#define EBADREQUEST     (215)   /* Destination cannot handle request */
#define ESRCDIED        (216)   /* Source just died */
#define EDSTDIED        (217)   /* Destination just died */
#define ETRAPDENIED     (218)   /* IPC trap not allowed */
#define EMSGBUF         (219)   /* IPC message not correctly aligned/mapped */
#define EDSTDENIED      (220)   /* Configuration do not authorize IPC to destination */
#define EDONTREPLY      (221)   /* Pseudo-code: don't send a reply */
#define SUSPEND         (222)   /* Don't answer, suspend execution */

#define __ELASTERROR    (255)   /* Users can add values starting here */
#endif /* _ERRNO_H_INCLUDED_ */
