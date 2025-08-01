#ifndef LIBW32_UNISTD_H_INCLUDED
#define LIBW32_UNISTD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libw32_unistd_h,"$Id: unistd.h,v 1.78 2025/06/28 14:11:52 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/*
 * win32 <unistd.h> header (_MSC_VER, __WATCOMC__ and __MINGW32__)
 *
 * Copyright (c) 1998 - 2025, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * This project is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * license for more details.
 * ==end==
 */

#if defined(_MSC_VER)
#if !defined(__MAKEDEPEND__)
#if (_MSC_VER != 1200)                          /* MSVC 6 */
#if (_MSC_VER != 1400)                          /* MSVC 8/2005 */
#if (_MSC_VER != 1500)                          /* MSVC 9/2008 */
#if (_MSC_VER != 1600)                          /* MSVC 10/2010 */
#if (_MSC_VER != 1900)                          /* MSVC 19/2015 */
#if (_MSC_VER <  1910 || _MSC_VER > 1916)       /* MSVC 2017: 19.10 .. 16 */
#if (_MSC_VER > 1929)                           /* MSVC 2019: 19.20 .. 29 */
#if (_MSC_VER > 1944)                           /* MSVC 2022: 19.30 .. 44 */
#error unistd.h: untested MSVC Version (2005 -- 2022 19.44)
	//see: https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B
#endif //2022
#endif //2019
#endif //2017
#endif //2015
#endif //2010
#endif //2008
#endif //2005
#endif //MS6
#endif //__MAKEDEPEND__

#pragma warning(disable:4115)

#if defined(_POSIX_)
#error unistd.h: _POSIX_ enabled
#endif

#if !defined(_CRT_SECURE_NO_DEPRECATE)
#define _CRT_SECURE_NO_DEPRECATE                /* disable deprecate warnings */
#endif

#elif defined(__WATCOMC__)
#if (__WATCOMC__ < 1200)
#error unistd.h: old WATCOM Version, upgrade to OpenWatcom ...
#endif
#ifndef __MAKEDEPEND__
#if (__WATCOMC__ != 1290)                       /* 1.9 */
#if (__WATCOMC__ != 1300)                       /* 2.0 */
#error unistd.h: untested OpenWatcom Version (1.9 -- 2.0) only ...
        //see: https://sourceforge.net/p/predef/wiki/Compilers/
#endif
#endif
#endif

#elif defined(__MINGW32__)

#else
#error unistd.h: unsupported compiler
#endif

#if !defined(_WIN32)                            /* _WIN32 requirement */
#error _WIN32 not defined; correct toolchain?
#endif

#if !defined(_WIN32_WINCE)                      /* require winsock2.h */
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x400                      /* entry level */
#elif (_WIN32_WINNT) < 0x400
//  Minimum system required Minimum value for _WIN32_WINNT and WINVER
//  Windows 7                                           (0x0601)
//  Windows Server 2008                                 (0x0600)
//  Windows Vista                                       (0x0600)
//  Windows Server 2003 with SP1, Windows XP with SP2   (0x0502)
//  Windows Server 2003, Windows XP _WIN32_WINNT_WINXP  (0x0501)
//
#pragma message("unistd: _WIN32_WINNT < 0400")
#endif
#endif   /*_WIN32_WINCE*/

/*
 *  avoid importing <win32_include.h>
 *      which among others includes <ctype.h>
 */
#include <win32_errno.h>
#include <win32_time.h>

#include <sys/cdefs.h>                          /* __BEGIN_DECLS, __PDECL */
#include <sys/utypes.h>
#include <sys/stat.h>
#if defined(HAVE_SYS_STATFS_H)
#include <sys/statfs.h>
#endif
#if defined(HAVE_SYS_UTIME_H)
#include <sys/utime.h>
#endif
#include <time.h>                               /* required to replace strfime() */
#include <stddef.h>                             /* offsetof() */
#if defined(USE_NATIVE_DIRECT)
#include <direct.h>
#else
#include <dirent.h>                             /* MAXPATHLENGTH, MAXNAMELENGTH */
#endif
#include <limits.h>                             /* _MAX_PATH */
#include <process.h>                            /* getpid, _beginthread */

#include <stdio.h>                              /* FILE */
#include <stdlib.h>
#include <malloc.h>
#include <string.h>                             /* memset, memmove ... */
#include <fcntl.h>
#include <io.h>                                 /* write, read ... */

#if defined(HAVE_CONFIG_H)                      /* stand alone? */
#include <getopt.h>                             /* getopt */
#endif

#ifndef _countof
#define _countof(__type) (sizeof(__type)/sizeof(__type[0]))
#endif

#if defined(LIBW32_UNISTD_MAP) && !defined(WIN32_UNISTD_MAP)
#define WIN32_UNISTD_MAP 1
#endif

#if defined(_LARGEFILE64_SOURCE)
#if !defined(OFF64_T)
#define OFF64_T 1
#define FPOS64_T 1
#if defined(__WATCOMC__)
typedef long long off64_t;
typedef long long fpos64_t;
#define stat64 _stati64     // Note: sizeof(st_size)==8, sizeof(st_atime)==4
#else
typedef __int64 off64_t;
typedef __int64 fpos64_t;
#define stat64 _stat64      // Note: sizeof(st_size)==8, sizeof(time_t/st_atime)==8
#endif
#endif // OFF64_T
#endif //_LARGEFILE64_SOURCE

#define EMODEINIT()         UINT __errmode = 0;
    // 0 - system default, displays all error dialog boxes.
#define EMODESUPPRESS()     __errmode = SetErrorMode (SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    // SEM_FAILCRITICALERRORS - system does not display the critical-error handler message box.
    // SEM_NOGPFAULTERRORBOX - system does not invoke Windows Error Reporting.
#define EMODERESTORE()      SetErrorMode (__errmode);

__BEGIN_DECLS

/*limits*/
//  Starting in Windows 10, version 1607, MAX_PATH(255) limitations have been removed from 
//  common Win32 file and directory functions. However, you must opt-in to the new behavior.
//
//  To enable the new long path behavior, both of the following conditions must be met:
//
//       Computer\HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\FileSystem\LongPathsEnabled
//
//  The (Type: REG_DWORD) registry key (above) must exist and be set to 1. The key's value 
//  will be cached by the system (per process) after the first call to an affected Win32 file
//  or directory function (see below for the list of functions). The registry key will not be 
//  reloaded during the lifetime of the process. In order for all apps on the system to recognize
//  the value of the key, a reboot might be required because some processes may have started 
//  before the key was set.
//
//  Note: The application manifest must also include the longPathAware element.
//
#define WIN32_PATH_MAX  1024
#define WIN32_LINK_DEPTH 8

/*fcntl.h*/
#define F_OK            0                       /* 00 Existence only */
#define W_OK            2                       /* 02 Write permission */
#define R_OK            4                       /* 04 Read permission */
#if !defined(X_OK)
#define X_OK            4                       /* 04 Execute permission, not available unless emulated */
#endif
#define A_OK            6                       /* 06 (access) Read and write permission */

#if !defined(O_ACCMODE)                         /* <fcntl.h>, Mask for file access modes */
#define O_ACCMODE       (O_RDONLY|O_WRONLY|O_RDWR)
#endif

#define O_NDELAY        0


/* <stat.h> */
                                                /* de facto standard definitions */
#if defined(S_IFFMT)                            /* type mask */
#if S_IFFMT != 0170000
#error  S_IFFMT redefinition error ...
#endif
#else
#define S_IFFMT         0170000
#endif  /*S_IFFMT*/

#if defined(__WATCOMC__)                        /* note, defined as 0 */
#undef  S_IFSOCK
#undef  S_ISSOCK
#undef  S_IFLNK
#undef  S_ISLNK
#undef  S_ISBLK
#undef  S_ISFIFO
#if (__WATCOMC__ >= 1300)
#undef  S_IFBLK         /*open-watcom 2.0*/
#endif
#endif  /*__WATCOMC__*/

#if defined(__MINGW32__)
#undef  S_IFBLK
#undef  S_ISBLK
#endif  /*__MINGW32__*/

#if defined(S_IFSOCK)
#if S_IFSOCK != 0140000
#error  S_IFSOCK redefinition error ...
#endif
#else
#define S_IFSOCK        0140000                 /* socket */
#endif  /*S_IFSOCK*/

#if defined(S_IFLNK)
#if S_IFLNK != 0120000
#error  S_IFLNK redefinition error ...
#endif
#else
#define S_IFLNK         0120000                 /* symbolic link */
#endif  /*S_IFLNK*/

#if defined(S_IFREG)                            /* regular file */
#if (S_IFREG != 0100000)
#error  S_IFREG redefinition error ...
#endif
#else
#define S_IFREG         0100000
#endif  /*S_IFREG*/

#if defined(S_IFBLK)                            /* block device */
#if (S_IFBLK != 0060000) && (S_IFBLK != 060000)
#error  S_IFBLK redefinition error ...
#endif
#else
#define S_IFBLK         0060000
#endif  /*S_IFBLK*/

#if defined(S_IFDIR)                            /* regular file */
#if (S_IFDIR != 0040000)
#error  S_IFDIR redefinition error ...
#endif
#else
#define S_IFDIR         0040000
#endif  /*S_IFDIR*/

#if defined(S_IFCHR)                            /* character special device */
#if (S_IFCHR != 0020000)
#error  S_IFCHR redefinition error ...
#endif
#else
#define S_IFCHR         0020000
#endif  /*S_IFCHR*/

#if defined(S_IFIFO)                            /* fifo */
#if (S_IFIFO != 0010000)
#error  S_IFIFO redefinition error ...
#endif
#else
#define S_IFIFO         0010000
#endif  /*S_IFIFO*/

#if defined(S_IFFIFO)                           /* fifo??? */
#error  S_IFFIFO is defined ??? ...
#endif

/* de facto standard definitions */
#if defined(S_ISUID)
#if (S_ISUID != 0004000)
#error  S_ISUID redefinition error ...
#endif
#else
#define S_ISUID         0004000                 /* set user id on execution */
#endif

#if defined(S_ISGID)
#if (S_ISGID != 0002000)
#error  S_ISGID redefinition error ...
#endif
#else
#define S_ISGID         0002000                 /* set group id on execution */
#endif

#ifndef _POSIX_SOURCE
#if defined(S_ISTXT)
#if (S_ISTXT != 0001000)
#endif
#else
#define S_ISTXT         0001000                 /* sticky bit */
#endif /*S_ISTXT*/
#endif /*_POSIX_SOURCE*/

#ifndef S_ISVTX
#define S_ISVTX         0                       /* on directories, restricted deletion flag; not supported */
#endif

#if defined(S_IRWXU)
#if (S_IRWXU != 0000700) && (S_IRWXU != 000700)
#error  S_IRWXU redefinition error ...
#endif
#else
#define S_IRWXU         0000700                 /* read, write, execute: owner */
#endif  /*S_IRWXU*/

#if defined(S_IRUSR)
#if (S_IRUSR != 0000400) && (S_IRUSR != 000400)
#error  S_IRUSR redefinition error ...
#endif
#else
#define S_IRUSR         0000400                 /* read permission: owner */
#define S_IWUSR         0000200                 /* write permission: owner */
#define S_IXUSR         0000100                 /* execute permission: owner */
#endif  /*S_IRUSR*/

#ifdef  _S_IREAD        /*verify environment*/
#if (_S_IREAD  != 0000400)
#error  _S_IREAD definition error ...
#endif
#if (_S_IWRITE != 0000200)
#error  _S_IWRITE definition error ...
#endif
#if (_S_IEXEC  != 0000100)
#error  _S_IEXEC definition error ...
#endif
#endif /*_S_IREAD*/

#ifndef _POSIX_SOURCE
#ifndef S_IREAD
#define S_IREAD         S_IRUSR
#define S_IWRITE        S_IWUSR
#define S_IEXEC         S_IXUSR
#endif /*S_IREAD*/
#endif /*_POSIX_SOURCE*/

#if defined(S_IRWXG)
#if (S_IRWXG != 0000070) && (S_IRWXG != 000070)
#error  S_IRWXG redefinition error ...
#endif
#else
#define S_IRWXG         0000070                 /* read, write, execute: group */
#define S_IRGRP         0000040                 /* read permission: group */
#define S_IWGRP         0000020                 /* write permission: group */
#define S_IXGRP         0000010                 /* execute permission: group */
#endif

#if defined(S_IRWXO)
#if (S_IRWXO != 0000007) && (S_IRWXO != 000007)
#error  S_IRWXO redefinition error ...
#endif
#else
#define S_IRWXO         0000007                 /* read, write, execute: other */
#define S_IROTH         0000004                 /* read permission: other */
#define S_IWOTH         0000002                 /* write permission: other */
#define S_IXOTH         0000001                 /* execute permission: other */
#endif

#define __S_ISTYPE(__mode,__mask) \
                        (((__mode) & S_IFFMT) == __mask)

#ifndef S_ISBLK                                 /* test for a block special */
#define S_ISBLK(m)      __S_ISTYPE(m, S_IFBLK)
#endif
#if !defined(__MINGW32__)
#ifndef S_ISFIFO                                /* test for a pipe or FIFO special file */
#define S_ISFIFO(m)     __S_ISTYPE(m, S_IFIFO)
#endif
#endif /*__MINGW32__*/
#ifndef S_ISDIR                                 /* test for a directory */
#define S_ISDIR(m)      __S_ISTYPE(m, S_IFDIR)
#endif
#ifndef S_ISCHR
#define S_ISCHR(m)      __S_ISTYPE(m, S_IFCHR)
#endif
#ifndef S_ISREG                                 /* test for a regular file */
#define S_ISREG(m)      __S_ISTYPE(m, S_IFREG)
#endif
#ifndef S_ISLNK                                 /* test for a symbolic link */
#define S_ISLNK(m)      __S_ISTYPE(m, S_IFLNK)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(m)     __S_ISTYPE(m, S_IFSOCK) /* test for a socket */
#endif

#ifndef _POSIX_SOURCE
#define ACCESSPERMS     (S_IRWXU|S_IRWXG|S_IRWXO) /* 0777 */
#define ALLPERMS        (S_ISUID|S_ISGID|S_ISTXT|S_IRWXU|S_IRWXG|S_IRWXO) /* 7777 */
#define DEFFILEMODE     (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) /* 0666 */
#endif

/*stdio.h*/
#if !defined(STDIN_FILENO)
#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2
#endif

/*signal.h*/
#define SIGCHLD         -101
#define SIGWINCH        -102
#define SIGPIPE         -103

#if !defined(__MINGW32__) || defined(__MINGW64_VERSION_MAJOR)
typedef struct {
    unsigned            junk;
} sigset_t;

typedef struct {
    unsigned            junk;
} siginfo_t;

struct sigaction {
    void              (*sa_handler)(int);
    void              (*sa_sigaction)(int, siginfo_t *, void *);
#define SA_RESTART                  0x01
    unsigned            sa_flags;
    sigset_t            sa_mask;
};

LIBW32_API int          sigemptyset (sigset_t *);
LIBW32_API int          sigaction (int, struct sigaction *, struct sigaction *);
#endif /*__MINGW32__*/

/*shell support*/
#if !defined(WNOHANG)
#define WNOHANG         1
#endif

LIBW32_API int          w32_waitpid (int, int *, int);
LIBW32_API int          w32_kill (int pid, int sig);

#if defined(WIN32_UNISTD_MAP)
#define                 kill(__pid, __val) \
                w32_kill(__pid, __val)
#endif /*WIN32_UNISTD_MAP*/

#if !defined(WEXITSTATUS)
LIBW32_API int          WEXITSTATUS (int status);
LIBW32_API int          WIFEXITED (int status);
LIBW32_API int          WIFSIGNALED (int status);
LIBW32_API int          WTERMSIG (int status);
LIBW32_API int          WCOREDUMP (int status);
LIBW32_API int          WIFSTOPPED (int status);
#endif

/* <stdlib.h> */
LIBW32_VAR char         *suboptarg;

LIBW32_API int          getsubopt (char **optionp, char * const *tokens, char **valuep);

/* <string.h> */
//#if (0) //libcompat
#if defined(_MSC_VER) || defined(__WATCOMC__)
#define NEED_STRCASECMP                         /*see: w32_string.c*/
#endif
#if defined(NEED_STRCASECMP)
LIBW32_API int          strcasecmp (const char *s1, const char *s2);
LIBW32_API int          strncasecmp (const char *s1, const char *s2, size_t len);
#endif /*NEED_STRCASECMP*/

#if (defined(_MSC_VER) && (_MSC_VER < 1400)) || \
            defined(__WATCOMC__) || \
            defined(__MINGW32__)
#define NEED_STRNLEN                            /*see: w32_string.c*/
#endif
#if defined(NEED_STRNLEN)
LIBW32_API size_t       strnlen(const char *s, size_t maxlen);
#endif /*NEED_STRNLEN*/
//#endif //libcompat

LIBW32_API int          w32_gethostname (char *name, size_t namelen);
LIBW32_API int          w32_getdomainname (char *name, size_t namelen);

#if defined(WIN32_UNISTD_MAP)
#if (defined(_WINSOCKAPI_) || defined(_WINSOCK2API_))
#if !defined(gethostname)
#define gethostname(__name,__namelen) \
                        w32_gethostname(__name,__namelen)
#endif //gethostname
#if !defined(getdomainname)
#define getdomainname(__name,__namelen) \
                        w32_getdomainname(__name,__namelen)
#endif //getdomainname
#endif
#endif /*WIN32_UNISTD_MAP*/

LIBW32_API const char * getlogin (void);
LIBW32_API int          getlogin_r (char *name, size_t namesize);

LIBW32_API void         setprogname (const char *name);
LIBW32_API void         setprognameW (const wchar_t *name);
LIBW32_API const char * getprogname (void);
LIBW32_API const char * getprognameA (void);
LIBW32_API const wchar_t * getprognameW (void);

LIBW32_API int          issetugid (void);

LIBW32_API int          w32_getuid (void);
LIBW32_API int          w32_geteuid (void);
LIBW32_API int          w32_getgid (void);
LIBW32_API int          w32_getegid (void);
LIBW32_API int          w32_getgpid (void);

#if defined(WIN32_UNISTD_MAP)
#define getuid()        w32_getuid()
#define geteuid()       w32_geteuid()
#define getgid()        w32_getgid()
#define getegid()       w32_getegid()
#define getgpid()       w32_getgpid()
#endif /*WIN32_UNISTD_MAP*/

LIBW32_API int          getgroups (int gidsetsize, gid_t grouplist[]);
LIBW32_API int          setgroups (size_t size, const gid_t *gidset);

/* time.h */
LIBW32_API unsigned int sleep (unsigned int secs);
LIBW32_API unsigned int w32_sleep (unsigned int secs);
LIBW32_API size_t       w32_strftime (char *buf, size_t buflen, const char *fmt, const struct tm *tm);

#if defined(WIN32_UNISTD_MAP)
#define strftime(a,b,c,d) \
                w32_strftime (a, b, c, d)
#endif /*WIN32_UNISTD_MAP*/

/* i/o */
LIBW32_API int          w32_utf8filenames_enable (void);

LIBW32_API int          w32_open (const char *path, int, ...);
LIBW32_API int          w32_openA (const char *path, int, int);
LIBW32_API int          w32_openW (const wchar_t *path, int, int);

LIBW32_API int          w32_stat (const char *path, struct stat *sb);
LIBW32_API int          w32_statA (const char *path, struct stat *sb);
LIBW32_API int          w32_statW (const wchar_t *path, struct stat *sb);
LIBW32_API int          w32_lstat (const char *path, struct stat *sb);
LIBW32_API int          w32_lstatA (const char *path, struct stat *sb);
LIBW32_API int          w32_lstatW (const wchar_t *path, struct stat *sb);
LIBW32_API int          w32_fstat (int fildes, struct stat *sb);
LIBW32_API int          w32_fstatA (int fildes, struct stat *sb);
LIBW32_API int          w32_fstatW (int fildes, struct stat *sb);

#if defined(_LARGEFILE64_SOURCE)
LIBW32_API int          w32_stat64 (const char *path, struct stat64 *sb);
LIBW32_API int          w32_stat64A (const char *path, struct stat64 *sb);
LIBW32_API int          w32_stat64W (const wchar_t *path, struct stat64 *sb);
LIBW32_API int          w32_lstat64 (const char *path, struct stat64 *sb);
LIBW32_API int          w32_lstat64A (const char *path, struct stat64 *sb);
LIBW32_API int          w32_lstat64W (const wchar_t *path, struct stat64 *sb);
LIBW32_API int          w32_fstat64 (int fildes, struct stat64 *sb);
LIBW32_API int          w32_fstat64A (int fildes, struct stat64 *sb);
LIBW32_API int          w32_fstat64W (int fildes, struct stat64 *sb);
#endif

LIBW32_API int          w32_read (int fildes, void *buffer, size_t cnt);
LIBW32_API int          w32_write (int fildes, const void *buffer, size_t cnt);
LIBW32_API int          w32_close (int fildes);
LIBW32_API const char * w32_strerror (int errnum);

LIBW32_API int          w32_link (const char *from, const char *to);
LIBW32_API int          w32_linkA (const char *from, const char *to);
LIBW32_API int          w32_linkW (const wchar_t *from, const wchar_t *to);

LIBW32_API int          w32_unlink (const char *fname);
LIBW32_API int          w32_unlinkA (const char *fname);
LIBW32_API int          w32_unlinkW (const wchar_t *fname);

LIBW32_API FILE *       w32_fopen (const char *path, const char *mode);
LIBW32_API FILE *       w32_fopenA (const char *path, const char *mode);
LIBW32_API FILE *       w32_fopenW (const wchar_t *path, const wchar_t *mode);

LIBW32_API int          w32_access (const char *fname, int mode);
LIBW32_API int          w32_accessA (const char *fname, int mode);
LIBW32_API int          w32_accessW (const wchar_t *fname, int mode);

LIBW32_API int          w32_rename (const char *ofile, const char *nfile);
LIBW32_API int          w32_renameA (const char *ofile, const char *nfile);
LIBW32_API int          w32_renameW (const wchar_t *ofile, const wchar_t *nfile);

LIBW32_API ssize_t      pread (int fildes, void *buf, size_t nbyte, off_t offset);
LIBW32_API ssize_t      pwrite (int fildes, const void *buf, size_t nbyte, off_t offset);

#if defined(_LARGEFILE64_SOURCE)
LIBW32_API off64_t      w32_lseek64 (int fildes, off64_t offset, int whence);
LIBW32_API off64_t      w32_tell64 (int fildes);
LIBW32_API off64_t      w32_filelength64 (int fildes);

LIBW32_API off64_t      w32_fseeko64 (FILE *stream, off64_t offset, int whence);
LIBW32_API off64_t      w32_ftello64 (FILE *stream);
LIBW32_API int          w32_fgetpos64 (FILE *stream, fpos64_t *pos);

LIBW32_API ssize_t      pread64 (int fildes, void *buf, size_t nbyte, off64_t offset);
LIBW32_API ssize_t      pwrite64 (int fildes, const void *buf, size_t nbyte, off64_t offset);
#endif

LIBW32_API int          w32_pipe (int fildes[2]);

#if defined(WIN32_UNISTD_MAP)
#define open            w32_open
#if defined(_LARGEFILE64_SOURCE)
#define stat(a,b)       w32_stat64 (a, b)
#define lstat(a,b)      w32_lstat64 (a, b)
#define fstat(a,b)      w32_fstat64 (a, b)
#define lseek(a,b,c)    w32_lseek64 (a, b, c)
#define tell(a)         w32_tell64 (a)
#define filelength(a)   w32_filelength64 (a)
#else
#define stat(a,b)       w32_stat (a, b)
#define lstat(a,b)      w32_lstat (a, b)
#define fstat(a,b)      w32_fstat (a, b)
#endif
#define read(a,b,c)     w32_read (a, b, c)
#define write(a,b,c)    w32_write (a, b, c)
#define close(a)        w32_close (a)
#define link(f,t)       w32_link (f,t)
#define unlink(p)       w32_unlink (p)
#define access(p,m)     w32_access (p, m)
#define rename(a,b)     w32_rename (a,b)
#define pipe(__f)       w32_pipe (__f)
#endif /*WIN32_UNISTD_MAP*/

#if defined(WIN32_UNISTD_MAP) || \
    defined(LIBW32_SOCKET_MAP_FD) || defined(WIN32_SOCKET_MAP_FD) || \
    defined(LIBW32_SOCKET_MAP_NATIVE) || defined(WIN32_SOCKET_MAP_NATIVE)
#define strerror(a)     w32_strerror(a)
	//#define g_strerror(a)   w32_strerror(a)         /* must also replace libglib version */
#endif

LIBW32_API int          w32_mkdir (const char *path, int mode);
LIBW32_API int          w32_mkdirA (const char *path, int mode);
LIBW32_API int          w32_mkdirW (const wchar_t *path, int mode);

LIBW32_API int          w32_chdir (const char *path);
LIBW32_API int          w32_chdirA (const char *path);
LIBW32_API int          w32_chdirW (const wchar_t *path);

LIBW32_API int          w32_rmdir (const char *path);
LIBW32_API int          w32_rmdirA (const char *path);
LIBW32_API int          w32_rmdirW (const wchar_t *path);

LIBW32_API char *       w32_getcwd (char *path, size_t size);
LIBW32_API char *       w32_getcwdA (char *path, size_t size);
LIBW32_API wchar_t *    w32_getcwdW (wchar_t *path, size_t size);

LIBW32_API char *       w32_getcwdd (char drive, char *path, size_t size);
LIBW32_API char *       w32_getcwddA (char drive, char *path, size_t size);
LIBW32_API wchar_t *    w32_getcwddW (char drive, wchar_t *path, size_t size);

LIBW32_API char *       w32_getdirectory (void);
LIBW32_API char *       w32_getdirectoryA (void);
LIBW32_API wchar_t *    w32_getdirectoryW (void);

LIBW32_API int          w32_getdrive (void);
LIBW32_API int          w32_getsystemdrive (void);
LIBW32_API int          w32_getlastdrive (void);

#if defined(WIN32_UNISTD_MAP)
#define mkdir(d,m)      w32_mkdir(d, m)
#define chdir(d)        w32_chdir(d)
#define rmdir(d)        w32_rmdir(d)
#define getcwd(d,s)     w32_getcwd(d,s)
#define utime(p,t)      w32_utime(p,t)

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#ifndef vsnprintf
#define vsnprintf       _vsnprintf
#endif
#ifndef snprintf
#define snprintf        _snprintf
#endif
#endif /*_MSC_VER*/
#endif /*WIN32_UNISTD_MAP*/

LIBW32_API int          w32_mkstemp (char *path);
LIBW32_API int          w32_mkstempA (char *path);
LIBW32_API int          w32_mkstempW (wchar_t *path);

LIBW32_API int          w32_mkstemps (char *path, int suffixlen);
LIBW32_API int          w32_mkstempsA (char *path, int suffixlen);
LIBW32_API int          w32_mkstempsW (wchar_t *path, int suffixlen);

LIBW32_API int          w32_mkstempx (char *path);
LIBW32_API int          w32_mkstempxA (char *path);
LIBW32_API int          w32_mkstempxW (wchar_t *path);

LIBW32_API char *       w32_mkdtemp (char *path);
LIBW32_API char *       w32_mkdtempA (char *path);
LIBW32_API wchar_t *    w32_mkdtempW (wchar_t *path);

LIBW32_API char *       w32_mkdtemps (char *path, int suffixlen);
LIBW32_API char *       w32_mkdtempsA (char *path, int suffixlen);
LIBW32_API wchar_t *    w32_mkdtempsW (wchar_t *path, int suffixlen);

LIBW32_API int          ftruncate (int fildes, off_t size);
LIBW32_API int          truncate (const char *path, off_t length);
LIBW32_API int          truncateA (const char *path, off_t length);
LIBW32_API int          truncateW (const wchar_t *path, off_t length);
#if defined(_LARGEFILE64_SOURCE)
LIBW32_API int          ftruncate64 (int fildes, off64_t size);
LIBW32_API int          truncate64 (const char *path, off64_t size);
LIBW32_API int          truncate64A (const char *path, off64_t length);
LIBW32_API int          truncate64W (const wchar_t *path, off64_t length);
#endif

LIBW32_API int          w32_readlink (const char *path, char *name, size_t sz);
LIBW32_API int          w32_readlinkA (const char *path, char *name, size_t sz);
LIBW32_API int          w32_readlinkW (const wchar_t *path, wchar_t *name, size_t sz);

LIBW32_API int          w32_symlink (const char *from, const char *to);
LIBW32_API int          w32_symlinkA (const char *from, const char *to);
LIBW32_API int          w32_symlinkW (const wchar_t *from, const wchar_t *to);

LIBW32_API char *       w32_realpath (const char *path, char *resolved_path /*PATH_MAX*/);
LIBW32_API char *       w32_realpath2 (const char *path, char *resolved_path, size_t maxlen);
LIBW32_API char *       w32_realpathA (const char *path, char *resolved_path, size_t maxlen);
LIBW32_API wchar_t *    w32_realpathW (const wchar_t *path, wchar_t *resolved_path, size_t maxlen);

#if defined(WIN32_UNISTD_MAP)
#define readlink(__path,__name, __sz) \
                w32_readlink (__path, __name, __sz)
#define symlink(__from,__to) \
                w32_symlink (__from, __to)
#endif

LIBW32_API int          w32_chmod (const char *, mode_t);
LIBW32_API int          w32_chmodA (const char *, mode_t);
LIBW32_API int          w32_chmodW (const wchar_t *, mode_t);

#if defined(WIN32_UNISTD_MAP)
#define chmod(__path,__mode) \
                w32_chmod (__path, __mode)
#endif

LIBW32_API int          chown (const char *path, uid_t uid, gid_t gid);
LIBW32_API int          chownA (const char *path, uid_t uid, gid_t gid);
LIBW32_API int          chownW (const wchar_t *path, uid_t uid, gid_t gid);

LIBW32_API int          mknod (const char *path, int mode, int dev);
LIBW32_API int          mknodA (const char *path, int mode, int dev);
LIBW32_API int          mknodW (const wchar_t *path, int mode, int dev);

#if !defined(F_GETFL)   /* match Linux definitions */
#define F_GETFL         3       /* get file status flags */
#define F_SETFL         4       /* set file status flags */
#endif

#if !defined(fcntl)
LIBW32_API int          fcntl (int fd, int ctrl, int);
#endif
LIBW32_API int          w32_fcntl (int fd, int ctrl, int);
LIBW32_API int          w32_fsync (int fildes);

/*string.h*/
//#if (0) //libcompat
LIBW32_API char *       strsep (char **stringp, const char *delim);
#if !defined(HAVE_STRSEP)
#define HAVE_STRSEP     1
#endif
#if defined(_MSC_VER) || defined(__MINGW32__)
#if !defined(HAVE_STRLCAT)
#define HAVE_STRLCAT    1
#define HAVE_STRLCPY    1
#endif
LIBW32_API size_t       strlcat (char *dst, const char *src, size_t siz);
LIBW32_API size_t       strlcpy (char *dst, const char *src, size_t siz);
//#endif //libcompat

#if defined(_MSC_VER) && (_MSC_VER <= 1600)
LIBW32_API unsigned long long strtoull (const char * nptr, char ** endptr, int base);
LIBW32_API long long    strtoll (const char * nptr, char ** endptr, int base);
#endif
#endif /*_MSC_VER*/

LIBW32_API void         setproctitle (const char *fmt, ...);
LIBW32_API void         setproctitle_fast (const char *fmt, ...);

__END_DECLS

#endif /*LIBW32_UNISTD_H_INCLUDED*/
