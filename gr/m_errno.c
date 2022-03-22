#include <edidentifier.h>
__CIDENT_RCSID(gr_m_errno_c,"$Id: m_errno.c,v 1.27 2021/07/05 15:01:27 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_errno.c,v 1.27 2021/07/05 15:01:27 cvsuser Exp $
 * errno symbol primitives.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <editor.h>

#include <sys/stat.h>
#if defined(WIN32)
#if defined(HAVE_SOCKET_H)
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#endif
#include <unistd.h>
#include <errno.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_errno.h"                            /* public interface */

#include "accum.h"                              /* acc_...() */
#include "builtin.h"
#include "echo.h"
#include "eval.h"                               /* get_...()/isa_...() */
#include "prntf.h"
#include "symbol.h"                             /* sym_...() */

#if !defined(E2BIG)
#define E2BIG           -1
#endif              /*2BIG*/
#if !defined(EACCES)
#define EACCES          -1
#endif              /*ACCES*/
#if !defined(EADDRINUSE)
#define EADDRINUSE      -1
#endif              /*ADDRINUSE*/
#if !defined(EADDRNOTAVAIL)
#define EADDRNOTAVAIL       -1
#endif              /*ADDRNOTAVAIL*/
#if !defined(EAFNOSUPPORT)
#define EAFNOSUPPORT    -1
#endif              /*AFNOSUPPORT*/
#if !defined(EAGAIN)
#define EAGAIN          -1
#endif              /*AGAIN*/
#if !defined(EALREADY)
#define EALREADY        -1
#endif              /*ALREADY*/
#if !defined(EBADE)
#define EBADE           -1
#endif              /*BADE*/
#if !defined(EBADF)
#define EBADF           -1
#endif              /*BADF*/
#if !defined(EBADFD)
#define EBADFD          -1
#endif              /*BADFD*/
#if !defined(EBADMSG)
#define EBADMSG         -1
#endif              /*BADMSG*/
#if !defined(EBADR)
#define EBADR           -1
#endif              /*BADR*/
#if !defined(EBADRQC)
#define EBADRQC         -1
#endif              /*BADRQC*/
#if !defined(EBADSLT)
#define EBADSLT         -1
#endif              /*BADSLT*/
#if !defined(EBUSY)
#define EBUSY           -1
#endif              /*BUSY*/
#if !defined(ECANCELED)
#define ECANCELED       -1
#endif              /*CANCELED*/
#if !defined(ECHILD)
#define ECHILD          -1
#endif              /*CHILD*/
#if !defined(ECHRNG)
#define ECHRNG          -1
#endif              /*CHRNG*/
#if !defined(ECOMM)
#define ECOMM           -1
#endif              /*COMM*/
#if !defined(ECONNABORTED)
#define ECONNABORTED    -1
#endif              /*CONNABORTED*/
#if !defined(ECONNREFUSED)
#define ECONNREFUSED    -1
#endif              /*CONNREFUSED*/
#if !defined(ECONNRESET)
#define ECONNRESET      -1
#endif              /*CONNRESET*/
#if !defined(EDEADLK)
#define EDEADLK         -1
#endif              /*DEADLK*/
#if !defined(EDEADLOCK)
#define EDEADLOCK       -1
#endif              /*DEADLOCK*/
#if !defined(EDESTADDRREQ)
#define EDESTADDRREQ    -1
#endif              /*DESTADDRREQ*/
#if !defined(EDOM)
#define EDOM            -1
#endif              /*DOM*/
#if !defined(EDQUOT)
#define EDQUOT          -1
#endif              /*DQUOT*/
#if !defined(EEXIST)
#define EEXIST          -1
#endif              /*EXIST*/
#if !defined(EFAULT)
#define EFAULT          -1
#endif              /*FAULT*/
#if !defined(EFBIG)
#define EFBIG           -1
#endif              /*FBIG*/
#if !defined(EHOSTDOWN)
#define EHOSTDOWN       -1
#endif              /*HOSTDOWN*/
#if !defined(EHOSTUNREACH)
#define EHOSTUNREACH    -1
#endif              /*HOSTUNREACH*/
#if !defined(EIDRM)
#define EIDRM           -1
#endif              /*IDRM*/
#if !defined(EILSEQ)
#define EILSEQ          -1
#endif              /*ILSEQ*/
#if !defined(EINPROGRESS)
#define EINPROGRESS     -1
#endif              /*INPROGRESS*/
#if !defined(EINTR)
#define EINTR           -1
#endif              /*INTR*/
#if !defined(EINVAL)
#define EINVAL          -1
#endif              /*INVAL*/
#if !defined(EIO)
#define EIO             -1
#endif              /*IO*/
#if !defined(EISCONN)
#define EISCONN         -1
#endif              /*ISCONN*/
#if !defined(EISDIR)
#define EISDIR          -1
#endif              /*ISDIR*/
#if !defined(EISNAM)
#define EISNAM          -1
#endif              /*ISNAM*/
#if !defined(EKEYEXPIRED)
#define EKEYEXPIRED     -1
#endif              /*KEYEXPIRED*/
#if !defined(EKEYREJECTED)
#define EKEYREJECTED    -1
#endif              /*KEYREJECTED*/
#if !defined(EKEYREVOKED)
#define EKEYREVOKED     -1
#endif              /*KEYREVOKED*/
#if !defined(EL2HLT)
#define EL2HLT          -1
#endif              /*L2HLT*/
#if !defined(EL2NSYNC)
#define EL2NSYNC        -1
#endif              /*L2NSYNC*/
#if !defined(EL3HLT)
#define EL3HLT          -1
#endif              /*L3HLT*/
#if !defined(EL3RST)
#define EL3RST          -1
#endif              /*L3RST*/
#if !defined(ELIBACC)
#define ELIBACC         -1
#endif              /*LIBACC*/
#if !defined(ELIBBAD)
#define ELIBBAD         -1
#endif              /*LIBBAD*/
#if !defined(ELIBEXEC)
#define ELIBEXEC        -1
#endif              /*LIBEXEC*/
#if !defined(ELIBMAX)
#define ELIBMAX         -1
#endif              /*LIBMAX*/
#if !defined(ELIBSCN)
#define ELIBSCN         -1
#endif              /*LIBSCN*/
#if !defined(ELOOP)
#define ELOOP           -1
#endif              /*LOOP*/
#if !defined(EMEDIUMTYPE)
#define EMEDIUMTYPE     -1
#endif              /*MEDIUMTYPE*/
#if !defined(EMFILE)
#define EMFILE          -1
#endif              /*MFILE*/
#if !defined(EMLINK)
#define EMLINK          -1
#endif              /*MLINK*/
#if !defined(EMSGSIZE)
#define EMSGSIZE        -1
#endif              /*MSGSIZE*/
#if !defined(EMULTIHOP)
#define EMULTIHOP       -1
#endif              /*MULTIHOP*/
#if !defined(ENAMETOOLONG)
#define ENAMETOOLONG    -1
#endif              /*NAMETOOLONG*/
#if !defined(ENETDOWN)
#define ENETDOWN        -1
#endif              /*NETDOWN*/
#if !defined(ENETRESET)
#define ENETRESET       -1
#endif              /*NETRESET*/
#if !defined(ENETUNREACH)
#define ENETUNREACH     -1
#endif              /*NETUNREACH*/
#if !defined(ENFILE)
#define ENFILE          -1
#endif              /*NFILE*/
#if !defined(ENOBUFS)
#define ENOBUFS         -1
#endif              /*NOBUFS*/
#if !defined(ENODATA)
#define ENODATA         -1
#endif              /*NODATA*/
#if !defined(ENODEV)
#define ENODEV          -1
#endif              /*NODEV*/
#if !defined(ENOENT)
#define ENOENT          -1
#endif              /*NOENT*/
#if !defined(ENOEXEC)
#define ENOEXEC         -1
#endif              /*NOEXEC*/
#if !defined(ENOKEY)
#define ENOKEY          -1
#endif              /*NOKEY*/
#if !defined(ENOLCK)
#define ENOLCK          -1
#endif              /*NOLCK*/
#if !defined(ENOLINK)
#define ENOLINK         -1
#endif              /*NOLINK*/
#if !defined(ENOMEDIUM)
#define ENOMEDIUM       -1
#endif              /*NOMEDIUM*/
#if !defined(ENOMEM)
#define ENOMEM          -1
#endif              /*NOMEM*/
#if !defined(ENOMSG)
#define ENOMSG          -1
#endif              /*NOMSG*/
#if !defined(ENONET)
#define ENONET          -1
#endif              /*NONET*/
#if !defined(ENOPKG)
#define ENOPKG          -1
#endif              /*NOPKG*/
#if !defined(ENOPROTOOPT)
#define ENOPROTOOPT     -1
#endif              /*NOPROTOOPT*/
#if !defined(ENOSPC)
#define ENOSPC          -1
#endif              /*NOSPC*/
#if !defined(ENOSR)
#define ENOSR           -1
#endif              /*NOSR*/
#if !defined(ENOSTR)
#define ENOSTR          -1
#endif              /*NOSTR*/
#if !defined(ENOSYS)
#define ENOSYS          -1
#endif              /*NOSYS*/
#if !defined(ENOTBLK)
#define ENOTBLK         -1
#endif              /*NOTBLK*/
#if !defined(ENOTCONN)
#define ENOTCONN        -1
#endif              /*NOTCONN*/
#if !defined(ENOTDIR)
#define ENOTDIR         -1
#endif              /*NOTDIR*/
#if !defined(ENOTEMPTY)
#define ENOTEMPTY       -1
#endif              /*NOTEMPTY*/
#if !defined(ENOTSOCK)
#define ENOTSOCK        -1
#endif              /*NOTSOCK*/
#if !defined(ENOTSUP)
#define ENOTSUP         -1
#endif              /*NOTSUP*/
#if !defined(ENOTTY)
#define ENOTTY          -1
#endif              /*NOTTY*/
#if !defined(ENOTUNIQ)
#define ENOTUNIQ        -1
#endif              /*NOTUNIQ*/
#if !defined(ENXIO)
#define ENXIO           -1
#endif              /*NXIO*/
#if !defined(EOPNOTSUPP)
#define EOPNOTSUPP      -1
#endif              /*OPNOTSUPP*/
#if !defined(EOVERFLOW)
#define EOVERFLOW       -1
#endif              /*OVERFLOW*/
#if !defined(EPERM)
#define EPERM           -1
#endif              /*PERM*/
#if !defined(EPFNOSUPPORT)
#define EPFNOSUPPORT    -1
#endif              /*PFNOSUPPORT*/
#if !defined(EPIPE)
#define EPIPE           -1
#endif              /*PIPE*/
#if !defined(EPROTO)
#define EPROTO          -1
#endif              /*PROTO*/
#if !defined(EPROTONOSUPPORT)
#define EPROTONOSUPPORT -1
#endif              /*PROTONOSUPPORT*/
#if !defined(EPROTOTYPE)
#define EPROTOTYPE      -1
#endif              /*PROTOTYPE*/
#if !defined(ERANGE)
#define ERANGE          -1
#endif              /*RANGE*/
#if !defined(EREMCHG)
#define EREMCHG         -1
#endif              /*REMCHG*/
#if !defined(EREMOTE)
#define EREMOTE         -1
#endif              /*REMOTE*/
#if !defined(EREMOTEIO)
#define EREMOTEIO       -1
#endif              /*REMOTEIO*/
#if !defined(ERESTART)
#define ERESTART        -1
#endif              /*RESTART*/
#if !defined(EROFS)
#define EROFS           -1
#endif              /*ROFS*/
#if !defined(ESHUTDOWN)
#define ESHUTDOWN       -1
#endif              /*SHUTDOWN*/
#if !defined(ESOCKTNOSUPPORT)
#define ESOCKTNOSUPPORT -1
#endif              /*SOCKTNOSUPPORT*/
#if !defined(ESPIPE)
#define ESPIPE          -1
#endif              /*SPIPE*/
#if !defined(ESRCH)
#define ESRCH           -1
#endif              /*SRCH*/
#if !defined(ESTALE)
#define ESTALE          -1
#endif              /*STALE*/
#if !defined(ESTRPIPE)
#define ESTRPIPE        -1
#endif              /*STRPIPE*/
#if !defined(ETIME)
#define ETIME           -1
#endif              /*TIME*/
#if !defined(ETIMEDOUT)
#define ETIMEDOUT       -1
#endif              /*TIMEDOUT*/
#if !defined(ETXTBSY)
#define ETXTBSY         -1
#endif              /*TXTBSY*/
#if !defined(EUCLEAN)
#define EUCLEAN         -1
#endif              /*UCLEAN*/
#if !defined(EUNATCH)
#define EUNATCH         -1
#endif              /*UNATCH*/
#if !defined(EUSERS)
#define EUSERS          -1
#endif              /*USERS*/
#if !defined(EWOULDBLOCK)
#define EWOULDBLOCK     -1
#endif              /*WOULDBLOCK*/
#if !defined(EXDEV)
#define EXDEV           -1
#endif              /*XDEV*/
#if !defined(EXFULL)
#define EXFULL          -1
#endif              /*XFULL*/


/*
 *  Below is a list of the symbolic error names that are defined on Linux.
 *
 *  Some of these are marked POSIX.1, indicating that the name is defined by
 *  POSIX.1-2001, or C99, indicating that the name is defined by C99.
 */
static const struct {
    const char *    tag;
    int             value;
} x_errno_consts[] = {
#define __E(__x)    #__x, __x           /*  No  Description */
    __E(EPERM),                         /*  1   Operation not permitted (POSIX.1) */
    __E(ENOENT),                        /*  2   No such file or directory (POSIX.1) */
    __E(ESRCH),                         /*  3   No such process (POSIX.1) */
    __E(EINTR),                         /*  4   Interrupted function call (POSIX.1); see signal(7). */
    __E(EIO),                           /*  5   Input/output error (POSIX.1) */
    __E(ENXIO),                         /*  6   No such device or address (POSIX.1) */
    __E(E2BIG),                         /*  7   Argument list too long (POSIX.1) */
    __E(ENOEXEC),                       /*  8   Exec format error (POSIX.1) */
    __E(EBADF),                         /*  9   Bad file descriptor (POSIX.1) */
    __E(ECHILD),                        /* 10   No child processes (POSIX.1) */
    __E(EAGAIN),                        /* 11   Resource temporarily unavailable (may be the same value as EWOULDBLOCK) (POSIX.1) */
    __E(EWOULDBLOCK),                   /* --   Operation would block (maybe same value as EAGAIN or 41) (POSIX.1) */
    __E(ENOMEM),                        /* 12   Not enough space (POSIX.1) */
    __E(EACCES),                        /* 13   Permission denied (POSIX.1) */
    __E(EFAULT),                        /* 14   Bad address (POSIX.1) */
    __E(ENOTBLK),                       /* 15   Block device required */
    __E(EBUSY),                         /* 16   Device or resource busy (POSIX.1) */
    __E(EEXIST),                        /* 17   File exists (POSIX.1) */
    __E(EXDEV),                         /* 18   Improper link (POSIX.1) */
    __E(ENODEV),                        /* 19   No such device (POSIX.1) */
    __E(ENOTDIR),                       /* 20   Not a directory (POSIX.1) */
    __E(EISDIR),                        /* 21   Is a directory (POSIX.1) */
    __E(EINVAL),                        /* 22   Invalid argument (POSIX.1) */
    __E(ENFILE),                        /* 23   Too many open files in system (POSIX.1) */
    __E(EMFILE),                        /* 24   Too many open files (POSIX.1) */
    __E(ENOTTY),                        /* 25   Inappropriate I/O control operation (POSIX.1) */
    __E(ETXTBSY),                       /* 26   Text file busy (POSIX.1) */
    __E(EFBIG),                         /* 27   File too large (POSIX.1) */
    __E(ENOSPC),                        /* 28   No space left on device (POSIX.1) */
    __E(ESPIPE),                        /* 29   Invalid seek (POSIX.1) */
    __E(EROFS),                         /* 30   Read-only file system (POSIX.1) */
    __E(EMLINK),                        /* 31   Too many links (POSIX.1) */
    __E(EPIPE),                         /* 32   Broken pipe (POSIX.1) */
    __E(EDOM),                          /* 33   Mathematics argument out of domain of function (POSIX.1, C99) */
    __E(ERANGE),                        /* 34   Result too large (POSIX.1, C99) */
    __E(EDEADLK),                       /* 35   Resource deadlock avoided (POSIX.1) */
    __E(EDEADLOCK),                     /* --   Synonym for EDEADLK */
    __E(ENAMETOOLONG),                  /* 36   Filename too long (POSIX.1) */
    __E(ENOLCK),                        /* 37   No locks available (POSIX.1) */
    __E(ENOSYS),                        /* 38   Function not implemented (POSIX.1) */
    __E(ENOTEMPTY),                     /* 39   Directory not empty (POSIX.1) */
    __E(ELOOP),                         /* 40   Too many levels of symbolic links (POSIX.1) */
    /*41-EWOULDBLOCK*/
    __E(ENOMSG),                        /* 42   No message of the desired type (POSIX.1) */
    __E(EIDRM),                         /* 43   Identifier removed (POSIX.1) */
    __E(ECHRNG),                        /* 44   Channel number of of range */
    __E(EL2NSYNC),                      /* 45   Level 2 not synchronized */
    __E(EL3HLT),                        /* 46   Level 3 halted */
    __E(EL3RST),                        /* 47   Level 3 halted */
#if defined(ELNRNG)
    __E(ELNRNG),                        /* 48   Link number out of range */
#endif
    __E(EUNATCH),                       /* 49   Protocol driver not attached */
#if defined(ENOCSI)
    __E(ENOCSI),                        /* 50   No CSI structure available. */
#endif
    __E(EL2HLT),                        /* 51   Level 2 halted */
    __E(EBADE),                         /* 52   Invalid exchange */
    __E(EBADR),                         /* 53   Invalid request descriptor */
    __E(EXFULL),                        /* 54   Exchange full */
#if defined(ENOANO)
    __E(ENOANO),                        /* 55   No anode */
#endif
    __E(EBADRQC),                       /* 56   Invalid request code */
    __E(EBADSLT),                       /* 57   Invalid slot */
    /*58*/
#if defined(EBFONT)
    __E(EBFONT),                        /* 59   Bad font file format */
#endif
    __E(ENOSTR),                        /* 60   Not a STREAM (POSIX.1 (XSI STREAMS option)) */
    __E(ENODATA),                       /* 61   No message is available on the STREAM head read queue (POSIX.1) */
    __E(ETIME),                         /* 62   Timer expired (POSIX.1 (XSI STREAMS option)) */
    __E(ENOSR),                         /* 63   No STREAM resources (POSIX.1 (XSI STREAMS option)) */
    __E(ENONET),                        /* 64   Machine is not on the network */
    __E(ENOPKG),                        /* 65   Package not installed */
    __E(EREMOTE),                       /* 66   Object is remote */
    __E(ENOLINK),                       /* 67   Link has been severed (POSIX.1) */
#if defined(EADV)
    __E(EADV),                          /* 68   Advertise error */
#endif
#if defined(ESRMNT)
    __E(ESRMNT),                        /* 69   Srmount error */
#endif
    __E(ECOMM),                         /* 70   Communication error on send */
    __E(EPROTO),                        /* 71   Protocol error (POSIX.1) */
    __E(EMULTIHOP),                     /* 72   Multihop attempted (POSIX.1) */
#if defined(EDOTDOT)
    __E(EDOTDOT),                       /* 73   RFS specific error */
#endif
    __E(EBADMSG),                       /* 74   Bad message (POSIX.1) */
    __E(EOVERFLOW),                     /* 75   Value too large to be stored in data type (POSIX.1) */
    __E(ENOTUNIQ),                      /* 76   Name not unique on network */
    __E(EBADFD),                        /* 77   File descriptor in bad state */
    __E(EREMCHG),                       /* 78   Remote address changed */
    __E(ELIBACC),                       /* 79   Cannot access a needed shared library */
    __E(ELIBBAD),                       /* 80   Accessing a corrupted shared library */
    __E(ELIBSCN),                       /* 81   lib section in a.out corrupted */
    __E(ELIBMAX),                       /* 82   Attempting to link in too many shared libraries */
    __E(ELIBEXEC),                      /* 83   Cannot exec a shared library directly */
    __E(EILSEQ),                        /* 84   Illegal byte sequence (POSIX.1, C99) */
    __E(ERESTART),                      /* 85   Interrupted system call should be restarted */
    __E(ESTRPIPE),                      /* 86   Streams pipe error */
    __E(EUSERS),                        /* 87   Too many users */
    __E(ENOTSOCK),                      /* 88   Not a socket (POSIX.1) */
    __E(EDESTADDRREQ),                  /* 89   Destination address required (POSIX.1) */
    __E(EMSGSIZE),                      /* 90   Message too long (POSIX.1) */
    __E(EPROTOTYPE),                    /* 91   Protocol wrong type for socket (POSIX.1) */
    __E(ENOPROTOOPT),                   /* 92   Protocol not available (POSIX.1) */
    __E(EPROTONOSUPPORT),               /* 93   Protocol not supported (POSIX.1) */
    __E(ESOCKTNOSUPPORT),               /* 94   Socket type not supported */
    __E(EOPNOTSUPP),                    /* 95   Operation not supported on socket (POSIX.1) */
    __E(ENOTSUP),                       /* --   Operation not supported (POSIX.1) */
    __E(EPFNOSUPPORT),                  /* 96   Protocol family not supported */
    __E(EAFNOSUPPORT),                  /* 97   Address family not supported (POSIX.1) */
    __E(EADDRINUSE),                    /* 98   Address already in use (POSIX.1) */
    __E(EADDRNOTAVAIL),                 /* 99   Address not available (POSIX.1) */
    __E(ENETDOWN),                      /* 100  Network is down (POSIX.1) */
    __E(ENETUNREACH),                   /* 101  Network unreachable (POSIX.1) */
    __E(ENETRESET),                     /* 102  Connection aborted by network (POSIX.1) */
    __E(ECONNABORTED),                  /* 103  Connection aborted (POSIX.1) */
    __E(ECONNRESET),                    /* 104  Connection reset (POSIX.1) */
    __E(ENOBUFS),                       /* 105  No buffer space available (POSIX.1 (XSI STREAMS option)) */
    __E(EISCONN),                       /* 106  Socket is connected (POSIX.1) */
    __E(ENOTCONN),                      /* 107  The socket is not connected (POSIX.1) */
    __E(ESHUTDOWN),                     /* 108  Cannot send after transport endpoint shutdown */
#if defined(ETOOMANYREFS)
    __E(ETOOMANYREFS),                  /* 109  Too many references; cannot splice */
#endif
    __E(ETIMEDOUT),                     /* 110  Connection timed out (POSIX.1) */
    __E(ECONNREFUSED),                  /* 111  Connection refused (POSIX.1) */
    __E(EHOSTDOWN),                     /* 112  Host is down */
    __E(EHOSTUNREACH),                  /* 113  Host is unreachable (POSIX.1) */
    __E(EALREADY),                      /* 114  Connection already in progress (POSIX.1) */
    __E(EINPROGRESS),                   /* 115  Operation in progress (POSIX.1) */
    __E(ESTALE),                        /* 116  Stale file handle (POSIX.1) */
    __E(EUCLEAN),                       /* 117  Structure needs cleaning */
#if defined(ENOTNAM)
    __E(ENOTNAM),                       /* 118  Not a Xenix names type file */
#endif
#if defined(ENAVAIL)
    __E(ENAVAIL),                       /* 119  No Xenix semaphores available */
#endif
    __E(EISNAM),                        /* 120  Is a named type file */
    __E(EREMOTEIO),                     /* 121  Remote I/O error */
    __E(EDQUOT),                        /* 122  Disk quota exceeded (POSIX.1) */

    __E(ENOMEDIUM),                     /* 123  No medium found */
    __E(EMEDIUMTYPE),                   /* 124  Wrong medium type */
    __E(ECANCELED),                     /* 125  Operation canceled (POSIX.1) */
    __E(ENOKEY),                        /* 126  Required key not available */
    __E(EKEYEXPIRED),                   /* 127  Key has expired */
    __E(EKEYREJECTED),                  /* 128  Key was rejected by service */
    __E(EKEYREVOKED),                   /* 129  Key has been revoked */

#if defined(EOWNERDEAD)
    __E(EOWNERDEAD),                    /* 130  Owner died */
#endif
#if defined(ENOTRECOVERABLE)
    __E(ENOTRECOVERABLE),               /* 131  State not recoverable */
#endif
#if defined(ERFKILL)
    __E(ERFKILL),                       /* 132  Operation not possible due to RF-kill */
#endif
#if defined(EOTHER)
    __E(EOTHER),                        /* Other (POSIX.1) */
#endif

#if defined(WIN32)                      /* Windows socket error codes */
#if defined(STRUNCATE)
    __E(STRUNCATE),                     /* 80    */
#endif
    __E(WSA_INVALID_HANDLE),            /* 6     */
    __E(WSA_NOT_ENOUGH_MEMORY),         /* 8     */
    __E(WSA_INVALID_PARAMETER),         /* 87    */
    __E(WSA_OPERATION_ABORTED),         /* 995   */
    __E(WSA_IO_INCOMPLETE),             /* 996   */
    __E(WSA_IO_PENDING),                /* 997   */

    /*these are mapped to their unix equiv*/
    __E(WSAEINTR),                      /* 10004 */
    __E(WSAEBADF),                      /* 10009 */
    __E(WSAEACCES),                     /* 10013 */
    __E(WSAEFAULT),                     /* 10014 */
    __E(WSAEINVAL),                     /* 10022 */
    __E(WSAEMFILE),                     /* 10024 */
    __E(WSAEWOULDBLOCK),                /* 10035 */
    __E(WSAEINPROGRESS),                /* 10036 */
    __E(WSAEALREADY),                   /* 10037 */
    __E(WSAENOTSOCK),                   /* 10038 */
    __E(WSAEDESTADDRREQ),               /* 10039 */
    __E(WSAEMSGSIZE),                   /* 10040 */
    __E(WSAEPROTOTYPE),                 /* 10041 */
    __E(WSAENOPROTOOPT),                /* 10042 */
    __E(WSAEPROTONOSUPPORT),            /* 10043 */
    __E(WSAESOCKTNOSUPPORT),            /* 10044 */
    __E(WSAEOPNOTSUPP),                 /* 10045 */
    __E(WSAEPFNOSUPPORT),               /* 10046 */
    __E(WSAEAFNOSUPPORT),               /* 10047 */
    __E(WSAEADDRINUSE),                 /* 10048 */
    __E(WSAEADDRNOTAVAIL),              /* 10049 */
    __E(WSAENETDOWN),                   /* 10050 */
    __E(WSAENETUNREACH),                /* 10051 */
    __E(WSAENETRESET),                  /* 10052 */
    __E(WSAECONNABORTED),               /* 10053 */
    __E(WSAECONNRESET),                 /* 10054 */
    __E(WSAENOBUFS),                    /* 10055 */
    __E(WSAEISCONN),                    /* 10056 */
    __E(WSAENOTCONN),                   /* 10057 */
    __E(WSAESHUTDOWN),                  /* 10058 */
    __E(WSAETOOMANYREFS),               /* 10059 */
    __E(WSAETIMEDOUT),                  /* 10060 */
    __E(WSAECONNREFUSED),               /* 10061 */
    __E(WSAELOOP),                      /* 10062 */
    __E(WSAENAMETOOLONG),               /* 10063 */
    __E(WSAEHOSTDOWN),                  /* 10064 */
    __E(WSAEHOSTUNREACH),               /* 10065 */
    __E(WSAENOTEMPTY),                  /* 10066 */
    __E(WSAEPROCLIM),                   /* 10067 */
    __E(WSAEUSERS),                     /* 10068 */
    __E(WSAEDQUOT),                     /* 10069 */
    __E(WSAESTALE),                     /* 10070 */
    __E(WSAEREMOTE),                    /* 10071 */

    __E(WSASYSNOTREADY),                /* 10091 */
    __E(WSAVERNOTSUPPORTED),            /* 10092 */
#if !defined(ENOTINITIALISED)
    __E(WSANOTINITIALISED),             /* 10093 */
#else
    __E(ENOTINITIALISED),
#endif

    __E(WSAEDISCON),                    /* 10101 */
    __E(WSAENOMORE),                    /* 10102 */
    __E(WSAECANCELLED),                 /* 10103 */
    __E(WSAEINVALIDPROCTABLE),          /* 10104 */
    __E(WSAEINVALIDPROVIDER),           /* 10105 */
    __E(WSAEPROVIDERFAILEDINIT),        /* 10106 */
    __E(WSASYSCALLFAILURE),             /* 10107 */
    __E(WSASERVICE_NOT_FOUND),          /* 10108 */
    __E(WSATYPE_NOT_FOUND),             /* 10109 */
    __E(WSA_E_NO_MORE),                 /* 10110 */
    __E(WSA_E_CANCELLED),               /* 10111 */
    __E(WSAEREFUSED),                   /* 10112 */
    __E(WSAHOST_NOT_FOUND),             /* 11001 */
    __E(WSATRY_AGAIN),                  /* 11002 */
    __E(WSANO_RECOVERY),                /* 11003 */
    __E(WSANO_DATA),                    /* 11004 */
    __E(WSA_QOS_RECEIVERS),             /* 11005 */
    __E(WSA_QOS_SENDERS),               /* 11006 */
    __E(WSA_QOS_NO_SENDERS),            /* 11007 */
    __E(WSA_QOS_NO_RECEIVERS),          /* 11008 */
    __E(WSA_QOS_REQUEST_CONFIRMED),     /* 11009 */
    __E(WSA_QOS_ADMISSION_FAILURE),     /* 11010 */
    __E(WSA_QOS_POLICY_FAILURE),        /* 11011 */
    __E(WSA_QOS_BAD_STYLE),             /* 11012 */
    __E(WSA_QOS_BAD_OBJECT),            /* 11013 */
    __E(WSA_QOS_TRAFFIC_CTRL_ERROR),    /* 11014 */
    __E(WSA_QOS_GENERIC_ERROR),         /* 11015 */
    __E(WSA_QOS_ESERVICETYPE),          /* 11016 */
    __E(WSA_QOS_EFLOWSPEC),             /* 11017 */
    __E(WSA_QOS_EPROVSPECBUF),          /* 11018 */
    __E(WSA_QOS_EFILTERSTYLE),          /* 11019 */
    __E(WSA_QOS_EFILTERTYPE),           /* 11020 */
    __E(WSA_QOS_EFILTERCOUNT),          /* 11021 */
    __E(WSA_QOS_EOBJLENGTH),            /* 11022 */
    __E(WSA_QOS_EFLOWCOUNT),            /* 11023 */
#if defined(WSA_QOS_EUNKNOWNPSOBJ)
    __E(WSA_QOS_EUNKNOWNPSOBJ),         /* 11024 */
#elif defined(WSA_QOS_EUNKOWNPSOBJ)
    __E(WSA_QOS_EUNKOWNPSOBJ),
#endif
    __E(WSA_QOS_EPOLICYOBJ),            /* 11025 */
    __E(WSA_QOS_EFLOWDESC),             /* 11026 */
    __E(WSA_QOS_EPSFLOWSPEC),           /* 11027 */
    __E(WSA_QOS_EPSFILTERSPEC),         /* 11028 */
    __E(WSA_QOS_ESDMODEOBJ),            /* 11029 */
    __E(WSA_QOS_ESHAPERATEOBJ),         /* 11030 */
    __E(WSA_QOS_RESERVED_PETYPE),       /* 11031 */
#endif

#if defined(EUNKNOWNERR)
    __E(EUNKNOWNERR),                   /* Unknown error */
#endif
#undef  __E
    };


/*  Function:           sym_errno_constants
 *      Create and initialise the well-known global symbols.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
sym_errno_constants(void)
{
    SYMBOL *sp;
    unsigned i;

    for (i = 0; i < (sizeof(x_errno_consts)/sizeof(x_errno_consts[0])); ++i) {
        sp = sym_push(1, x_errno_consts[i].tag, F_INT, SF_CONSTANT|SF_SYSTEM);
        sym_assign_int(sp, x_errno_consts[i].value);
    }
}


/*  Function:           do_strerror
 *      strerror primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: strerror - String error.

        string
        strerror([int errnum = errno],
            [string &manifest], [int multi = FALSE])

    Macro Description:
        The 'strerror()' primitive maps the error number in 'errnum'
        to a locale-dependent error message and returns a string
        containing the mapped condition.

    Macro Parameters:
        errnum - Integer value of the error condition to be decoded,
            if omitted the current system 'errno' is decoded.

        manifest - Optional string variable which is specified shall be
            populated with the signal manifest.

    Macro Returns:
        The 'strerror' function returns a string descripting the error
        value, otherwise an empty string if undefined.

    Macro Portability:
        The 'manifest' and 'multi' options are Grief extensions.

    Macro See Also:
        errno, perror
 */
void
do_strerror(void)               /* string ([int errnum], [string &manifest], [int multi = FALSE]) */
{
    const int xerrno = get_xinteger(1, (int) *x_errno_ptr);

    if (!isa_undef(2))  {                       /* optional, constant name */
        const int multi = get_xinteger(3, FALSE);
        const char *tag = NULL;
        char *buf = NULL;

        if (xerrno) {
            if (xerrno > 0) {
                size_t taglen, buflen = 0;
                unsigned i;
                                                /* could map, but expect low usage */
                for (i = 0; i < (sizeof(x_errno_consts)/sizeof(x_errno_consts[0])); ++i) {
                    if (xerrno == x_errno_consts[i].value) {
                        tag = x_errno_consts[i].tag;
                        if (! multi) {
                            break;
                        }
                        taglen = strlen(tag);
                        if (buf) buf[buflen++] = ',';
                        if (NULL == (buf = chk_realloc(buf, buflen + taglen + 1))) {
                            break;
                        }
                        strcpy(buf + buflen, tag);
                        buflen += taglen;
                        tag = buf;
                    }
                }
            }
            if (!tag) tag = "EUNKNOWNERR";
        }
        argv_assign_str(2, tag ? tag : "");
        chk_free(buf);
    }
    acc_assign_str(str_error(xerrno), -1);
}


/*  Function:           do_perror
 *      perror primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: perror - Print error

        string
        perror([int errnum = errno], string format, ...)

    Macro Description:
        The 'perror()' primitive shall map the error number
        specified by 'errnum' to a language-dependent error
        message, which shall be written to the standard error
        stream as follows:

>           <message> : <error - description>

    Macro Parameters:
        errnum - Integer value of the error condition to be decoded,
            if omitted the current system 'errno' is decoded.

    Macro Returns:
        The 'perror' function returns a string containing the
        formatted message with a trailing description of the the
        error value.

    Macro Portability:
        A Grief extenions.

    Macro See Also:
        errno, strerror
 */
void
do_perror(void)                 /* ([int errno], string format, ...) */
{
    int msglen = -1, errlen;
    const int xerrno = get_xinteger(1, (int) *x_errno_ptr);
    const char *msg = print_formatted(2, &msglen, NULL);
    char err[80];

    errlen = sxprintf(err, sizeof(err), ": %s (%d)", str_error(xerrno), xerrno);
    if (msg && msglen) {
        acc_assign_str2(msg, msglen, err, errlen);
    } else {
        acc_assign_str(err, errlen);
    }
    errorf("%s", acc_get_sval());
}
/*end*/
