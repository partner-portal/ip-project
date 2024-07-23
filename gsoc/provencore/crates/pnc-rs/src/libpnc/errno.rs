/*
 * Copyright (c) 2020-2020 Prove & Run S.A.S
 * All Rights Reserved.
 *
 * This software is the confidential and proprietary information of
 * Prove & Run S.A.S ("Confidential Information"). You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered
 * into with Prove & Run S.A.S
 *
 * PROVE & RUN S.A.S MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE
 * SUITABILITY OF THE SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT. PROVE & RUN S.A.S SHALL
 * NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING,
 * MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS DERIVATIVES.
 */

//! Standard kernel error codes

/// The error code type
pub type Errno = crate::libc::c_int;

use crate::error::Error;

impl From<Errno> for Error {
    fn from(errno: Errno) -> Error {
        Error::msg(match errno {
            OK => "OK",
            EPERM => "EPERM",
            ENOENT => "ENOENT",
            ESRCH => "ESRCH",
            EINTR => "EINTR",
            EIO => "EIO",
            ENXIO => "ENXIO",
            E2BIG => "E2BIG",
            ENOEXEC => "ENOEXEC",
            EBADF => "EBADF",
            ECHILD => "ECHILD",
            EAGAIN_EWOULDBLOCK => "EAGAIN or EWOULDBLOCK",
            ENOMEM => "ENOMEM",
            EACCES => "EACCES",
            EFAULT => "EFAULT",
            EBUSY => "EBUSY",
            EEXIST => "EEXIST",
            EXDEV => "EXDEV",
            ENODEV => "ENODEV",
            ENOTDIR => "ENOTDIR",
            EISDIR => "EISDIR",
            EINVAL => "EINVAL",
            ENFILE => "ENFILE",
            EMFILE => "EMFILE",
            ENOTTY => "ENOTTY",
            ETXTBSY => "ETXTBSY",
            EFBIG => "EFBIG",
            ENOSPC => "ENOSPC",
            ESPIPE => "ESPIPE",
            EROFS => "EROFS",
            EMLINK => "EMLINK",
            EPIPE => "EPIPE",
            EDOM => "EDOM",
            ERANGE => "ERANGE",
            EDEADLK => "EDEADLK",
            ENAMETOOLONG => "ENAMETOOLONG",
            ENOLCK => "ENOLCK",
            ENOSYS => "ENOSYS",
            ENOTEMPTY => "ENOTEMPTY",
            ELOOP => "ELOOP",
            ENOMSG => "ENOMSG",
            EIDRM => "EIDRM",
            ENOSTR => "ENOSTR",
            ENODATA => "ENODATA",
            ETIME => "ETIME",
            ENOSR => "ENOSR",
            ENOLINK => "ENOLINK",
            EPROTO => "EPROTO",
            EMULTIHOP => "EMULTIHOP",
            EBADMSG => "EBADMSG",
            EOVERFLOW => "EOVERFLOW",
            EILSEQ => "EILSEQ",
            ENOTSOCK => "ENOTSOCK",
            EDESTADDRREQ => "EDESTADDRREQ",
            EMSGSIZE => "EMSGSIZE",
            EPROTOTYPE => "EPROTOTYPE",
            ENOPROTOOPT => "ENOPROTOOPT",
            EPROTONOSUPPORT => "EPROTONOSUPPORT",
            ENOTSUP_EOPNOTSUPP => "ENOTSUP or EOPNOTSUPP",
            EAFNOSUPPORT => "EAFNOSUPPORT",
            EADDRINUSE => "EADDRINUSE",
            EADDRNOTAVAIL => "EADDRNOTAVAIL",
            ENETDOWN => "ENETDOWN",
            ENETUNREACH => "ENETUNREACH",
            ENETRESET => "ENETRESET",
            ECONNABORTED => "ECONNABORTED",
            ECONNRESET => "ECONNRESET",
            ENOBUFS => "ENOBUFS",
            EISCONN => "EISCONN",
            ENOTCONN => "ENOTCONN",
            ESHUTDOWN => "ESHUTDOWN",
            ETIMEDOUT => "ETIMEDOUT",
            ECONNREFUSED => "ECONNREFUSED",
            EHOSTUNREACH => "EHOSTUNREACH",
            EALREADY => "EALREADY",
            EINPROGRESS => "EINPROGRESS",
            ESTALE => "ESTALE",
            EDQUOT => "EDQUOT",
            ECANCELED => "ECANCELED",
            EOWNERDEAD => "EOWNERDEAD",
            ENOTRECOVERABLE => "ENOTRECOVERABLE",
            EGENERIC => "EGENERIC",
            EPACKSIZE => "EPACKSIZE",
            EOUTOFBUFS => "EOUTOFBUFS",
            EBADIOCTL => "EBADIOCTL",
            EBADMODE => "EBADMODE",
            EBADDEST => "EBADDEST",
            EDSTNOTRCH => "EDSTNOTRCH",
            EURG => "EURG",
            ENOURG => "ENOURG",
            ENOCONN => "ENOCONN",
            ELOCKED => "ELOCKED",
            EBADCALL => "EBADCALL",
            EBADSRCDST => "EBADSRCDST",
            ECALLDENIED => "ECALLDENIED",
            ENOTREADY => "ENOTREADY",
            EBADREQUEST => "EBADREQUEST",
            ESRCDIED => "ESRCDIED",
            EDSTDIED => "EDSTDIED",
            ETRAPDENIED => "ETRAPDENIED",
            EMSGBUF => "EMSGBUF",
            EDSTDENIED => "EDSTDENIED",
            EDONTREPLY => "EDONTREPLY",
            SUSPEND => "SUSPEND",
            __ELASTERROR => "__ELASTERROR",
            _ => "Unknown Error code",
        })
    }
}

pub const OK: Errno = 0;
pub const EPERM: Errno = 1;
pub const ENOENT: Errno = 2;
pub const ESRCH: Errno = 3;
pub const EINTR: Errno = 4;
pub const EIO: Errno = 5;
pub const ENXIO: Errno = 6;
pub const E2BIG: Errno = 7;
pub const ENOEXEC: Errno = 8;
pub const EBADF: Errno = 9;
pub const ECHILD: Errno = 10;
const EAGAIN_EWOULDBLOCK: Errno = 11;
pub const EAGAIN: Errno = EAGAIN_EWOULDBLOCK;
pub const EWOULDBLOCK: Errno = EAGAIN_EWOULDBLOCK;
pub const ENOMEM: Errno = 12;
pub const EACCES: Errno = 13;
pub const EFAULT: Errno = 14;
pub const EBUSY: Errno = 16;
pub const EEXIST: Errno = 17;
pub const EXDEV: Errno = 18;
pub const ENODEV: Errno = 19;
pub const ENOTDIR: Errno = 20;
pub const EISDIR: Errno = 21;
pub const EINVAL: Errno = 22;
pub const ENFILE: Errno = 23;
pub const EMFILE: Errno = 24;
pub const ENOTTY: Errno = 25;
pub const ETXTBSY: Errno = 26;
pub const EFBIG: Errno = 27;
pub const ENOSPC: Errno = 28;
pub const ESPIPE: Errno = 29;
pub const EROFS: Errno = 30;
pub const EMLINK: Errno = 31;
pub const EPIPE: Errno = 32;
pub const EDOM: Errno = 33;
pub const ERANGE: Errno = 34;
pub const EDEADLK: Errno = 35;
pub const ENAMETOOLONG: Errno = 36;
pub const ENOLCK: Errno = 37;
pub const ENOSYS: Errno = 38;
pub const ENOTEMPTY: Errno = 39;
pub const ELOOP: Errno = 40;
pub const ENOMSG: Errno = 42;
pub const EIDRM: Errno = 43;
pub const ENOSTR: Errno = 60;
pub const ENODATA: Errno = 61;
pub const ETIME: Errno = 62;
pub const ENOSR: Errno = 63;
pub const ENOLINK: Errno = 67;
pub const EPROTO: Errno = 71;
pub const EMULTIHOP: Errno = 72;
pub const EBADMSG: Errno = 74;
pub const EOVERFLOW: Errno = 75;
pub const EILSEQ: Errno = 84;
pub const ENOTSOCK: Errno = 88;
pub const EDESTADDRREQ: Errno = 89;
pub const EMSGSIZE: Errno = 90;
pub const EPROTOTYPE: Errno = 91;
pub const ENOPROTOOPT: Errno = 92;
pub const EPROTONOSUPPORT: Errno = 93;
const ENOTSUP_EOPNOTSUPP: Errno = 95;
pub const ENOTSUP: Errno = ENOTSUP_EOPNOTSUPP;
pub const EOPNOTSUPP: Errno = ENOTSUP_EOPNOTSUPP;
pub const EAFNOSUPPORT: Errno = 97;
pub const EADDRINUSE: Errno = 98;
pub const EADDRNOTAVAIL: Errno = 99;
pub const ENETDOWN: Errno = 100;
pub const ENETUNREACH: Errno = 101;
pub const ENETRESET: Errno = 102;
pub const ECONNABORTED: Errno = 103;
pub const ECONNRESET: Errno = 104;
pub const ENOBUFS: Errno = 105;
pub const EISCONN: Errno = 106;
pub const ENOTCONN: Errno = 107;
pub const ESHUTDOWN: Errno = 108;
pub const ETIMEDOUT: Errno = 110;
pub const ECONNREFUSED: Errno = 111;
pub const EHOSTUNREACH: Errno = 113;
pub const EALREADY: Errno = 114;
pub const EINPROGRESS: Errno = 115;
pub const ESTALE: Errno = 116;
pub const EDQUOT: Errno = 122;
pub const ECANCELED: Errno = 125;
pub const EOWNERDEAD: Errno = 130;
pub const ENOTRECOVERABLE: Errno = 131;
pub const EGENERIC: Errno = 200;
pub const EPACKSIZE: Errno = 201;
pub const EOUTOFBUFS: Errno = 202;
pub const EBADIOCTL: Errno = 203;
pub const EBADMODE: Errno = 204;
pub const EBADDEST: Errno = 205;
pub const EDSTNOTRCH: Errno = 206;
pub const EURG: Errno = 207;
pub const ENOURG: Errno = 208;
pub const ENOCONN: Errno = 209;
pub const ELOCKED: Errno = 210;
pub const EBADCALL: Errno = 211;
pub const EBADSRCDST: Errno = 212;
pub const ECALLDENIED: Errno = 213;
pub const ENOTREADY: Errno = 214;
pub const EBADREQUEST: Errno = 215;
pub const ESRCDIED: Errno = 216;
pub const EDSTDIED: Errno = 217;
pub const ETRAPDENIED: Errno = 218;
pub const EMSGBUF: Errno = 219;
pub const EDSTDENIED: Errno = 220;
pub const EDONTREPLY: Errno = 221;
pub const SUSPEND: Errno = 222;
pub const __ELASTERROR: Errno = 255;
