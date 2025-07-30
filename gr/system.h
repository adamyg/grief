#ifndef GR_SYSTEM_H_INCLUDED
#define GR_SYSTEM_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_system_h,"$Id: system.h,v 1.41 2025/02/07 03:03:22 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: system.h,v 1.41 2025/02/07 03:03:22 cvsuser Exp $
 * System interface.
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

#include <edsym.h>
#include <time.h>

__CBEGIN_DECLS

struct IOEvent;

extern void                 sys_initialise(void);
extern void                 sys_shutdown(void);
extern void                 sys_cleanup(void);

extern int                  sys_iocheck(struct IOEvent *evt);
extern int                  sys_getevent(struct IOEvent *evt, accint_t tmo);
extern int                  sys_getchar(int fd, int *buf, accint_t tmo);
extern void                 sys_tty_delay(int fd, int state /*0=off, 1=on*/);

extern const char *         sys_delim(void);
extern const char *         sys_getshell(void);
extern int                  sys_enable_char(int ch, int enable);
extern void                 sys_timeout(int yes);
extern time_t               sys_time(int *msec);

extern int                  sys_stat(const char *path, struct stat *sb);
extern int                  sys_lstat(const char *path, struct stat *sb);
extern int                  sys_readlink(const char *path, char *buf, int maxlen);
extern int                  sys_symlink(const char *name1, const char *name2);
extern int                  sys_unlink(const char *fname);

extern const char *         sys_basename(const char *fname);
extern const char *         sys_pathend(const char *path);
extern int                  sys_isabspath(const char *path);
extern const char *         sys_pathdelimiter(void);
extern const char *         sys_pathseparator(void);

extern int                  sys_mkdir(const char *path, int amode);
extern int                  sys_access(const char *path, int amode);
extern int                  sys_chmod(const char *path, int mode);
extern int                  sys_realpath(const char *path, char *real, int size);
extern const char *         sys_cwd(char *cwd, int size);
extern int                  sys_read(int fd, void *buf, int size);
extern int                  sys_read_timed(int fd, void *buf, int size, unsigned timeoutms, unsigned *remainingms);
extern int                  sys_write(int fd, const void *buf, int size);
extern int                  sys_copy(const char *src, const char *dst, int perms, int owner, int group);
extern void                 sys_noinherit(int fd);

extern void                 sys_deamonize(void);
extern int                  sys_shell(const char *cmd, const char *macro,
                                const char *fstdin, const char *fstdout, const char *fstderr, int mode);
extern int                  sys_waitpid(int pid, int *status, int nowait);
extern int                  sys_kill(int pid, int value);
extern void                 sys_pty_mode(int fd);

extern int                  sys_cut(int total, int append, void (*copy)(char *buf, int len));
extern int                  sys_paste(void (*paste)(const char *buf, int len));

extern int                  sys_mkstemp(char *pattern);

extern int                  sys_getuid(void);
extern int                  sys_geteuid(void);
extern int                  sys_getpid(void);
extern int                  sys_core(const char *msg, const char *path, const char *fname);
extern void                 sys_abort(void);

FILE *                      sys_popen(const char *cmd, const char *mode);
int                         sys_pclose(FILE *file);

typedef void (*signal_handler_t)(int);
extern signal_handler_t     sys_signal(int sig, signal_handler_t func);

extern int                  sys_running(int pid);

#if defined(DOSISH)                             /* DOSISH specific functions */
extern void                 sys_cwdd(int drv, char *cwd, int size);
extern int                  sys_drive_get(void);
extern int                  sys_drive_set(int);
#endif

#if (defined(_WIN32) || defined(WIN32)) && !defined(__CYGWIN__)
#define mkdir(__p,__m)      _mkdir(__p)         /* name mangling */
#endif /*WIN32*/

#if defined(_WIN32) || defined(WIN32)
extern int                  sys_fstype(const char *path);
#endif

#if defined(__OS2__)
extern int                  kbhit(void);
extern int                  main_thread_read_kbd(long tmo);
extern void                 getkey_alarm_handler(int sig);
extern int                  sys_fstype(const char *path);
#endif /*__OS2__*/

#if defined(_VMS)
extern char *               sys_fname_unix_to_vms(const char *src, char *dst, int size);
extern char *               vms_filename_canon(char *buf);
extern int                  system(const char *cmd);
#endif /*_VMS*/

__CEND_DECLS

#endif /*GR_SYSTEM_H_INCLUDED*/
