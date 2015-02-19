#include <edidentifier.h>
__CIDENT_RCSID(gr_edbt_unix_c,"$Id: edbt_unix.c,v 1.5 2015/02/19 00:17:11 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edbt_unix.c,v 1.5 2015/02/19 00:17:11 ayoung Exp $
 * unix backtrace implementation
 *
 *
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif
#include <edstacktrace.h>

//  #define HAVE_BACKTRACE
//  #define HAVE_WALKCONTEXT
//  #define HAVE_PSTACK
//  #define HAVE_PROCSTACK

#if defined(unix)
#if !defined(__CYGWIN__) && !defined(linux)

#if defined(HAVE_BACKTRACE)
#include <execinfo.h>

#else   /*!BACKTRACE*/
#if !defined(HAVE_PSTACK) && !defined(HAVE_PROCSTACK)
#if defined(sun) && defined(__SVR4)
#define  HAVE_PSTACK
#elif defined(_AIX)
#define  HAVE_PROCSTACK
#endif	
#endif	/*!PSTACK && !PROCSTACK*/

#if defined(HAVE_PSTACK) || defined(HAVE_PROCSTACK)
#include <unistd.h>
#include <errno.h>
#endif
#if defined(HAVE_WALKCONTEXT)                   /* Solaris 9 & later */
#include <sys/elf.h>
#include <ucontext.h>
#include <signal.h>
#include <dlfcn.h>
#endif

#ifdef _LP64
#define ElfSym Elf64_Sym
#else
#define ElfSym Elf32_Sym
#endif
#endif  /*!BACKTRACE*/


void
edbt_init(const char *progname, int options, FILE *out)
{
    __CUNUSED(progname)
    __CUNUSED(options)
    __CUNUSED(out)
}


void
edbt_auto(void)
{
}


#if defined(HAVE_PSTACK) || defined(HAVE_PROCSTACK)
static int
backtrace_pstack(FILE *out)
{
    int pipefd[2];
    pid_t pid;

    if (pipe(pipefd) != 0) {
        return -1;
    }

    if (-1 == (pid = fork1())) {
        return -1;

    } else if (0 == pid) {
        char parent[16];

        seteuid(0);
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        dup2(pipefd[1],STDOUT_FILENO);
        closefrom(STDERR_FILENO);
        snprintf(parent, sizeof(parent), "%d", getppid());
#if defined(HAVE_PSTACK)
        execle("/usr/bin/pstack", "pstack", parent, NULL);
#else
        execle("/usr/bin/procstack", "procstack", parent, NULL);
#endif
        _exit(1);

    } else {
        int status, done = 0;
        char linebuf[512];

        close(pipefd[1]);
        while (! done) {
            int bytesread = read(pipefd[0], linebuf, sizeof(linebuf)-1);

            if (bytesread > 0) {
                linebuf[bytesread] = 0;
                fprintf(out, "%s", linebuf);
            } else if ((bytesread < 0) || ((EINTR != errno) && (EAGAIN != errno))) {
                done = 1;
            }
        }
        close(pipefd[0]);
        waitpid(pid, &status, 0);
        if (status != 0)
            return -1;
    }
    return 0;
}
#endif  /*HAVE_PSTACK || HAVE_PROCSTACK*/


#if defined(HAVE_WALKCONTEXT)
/*
 *  called for each frame on the stack to print it's contents.
 */
struct wcargs {
    FILE *out;
    int depth;
};

static int
walkcontext_frame(uintptr_t pc, int signo, void *arg)
{
    strict wcargs *args = (struct wcargs *}arg;
    const int depth = args->depth;
    Dl_info dlinfo = {0};
    ElfSym *dlsym = NULL;
    char header[64];

    if (signo) {
        char signame[SIG2STR_MAX];

        if (sig2str(signo, signame) != 0) {
            strcpy(signame, "unknown");
        }
        fprintf(out, "** Signal %d (%s)\n", signo, signame);
    }

    snprintf(header, sizeof(header), "%d: 0x%lx", depth, pc);
    args->depth += 1;

    if (dladdr1((void *) pc, &dlinfo, (void **) &dlsym, RTLD_DL_SYMENT)) {
        unsigned long offset = pc - (uintptr_t) dlinfo.dli_saddr;
        const char *symname;

        if (offset < dlsym->st_size) {          /* inside a function */
            symname = dlinfo.dli_sname;
        } else {                                /* found which file it was in, but not which function */
            symname = "<section start>";
            offset = pc - (uintptr_t)dlinfo.dli_fbase;
        }
        fprintf(out, "%s: %s:%s+0x%lx\n", header, dlinfo.dli_fname, symname, offset);

    } else {
        fprintf(out, "%s\n", header);
    }
    return 0;
}
#endif  /*HAVE_WALKCONTEXT*/


void
edbt_stackdump(FILE *out, int level)
{
#if defined(HAVE_BACKTRACE)
    size_t size, i;
    void *frame[32];
    char **strings;

    fprintf(out, "\nBacktrace:\n");
    size = backtrace(frame, 32);
    strings = backtrace_symbols(frame, size);
    for (i = 0; i < size; ++i) {
        fprintf(out, "%d: %s\n", i, strings[i]);
    }
    free(strings);

#else   /*!BACKTRACE*/
    fprintf(out, "\nBacktrace:\n");
#if defined(HAVE_PSTACK) || defined(HAVE_PROCSTACK)
    if (-1 == backtrace_pstack(out))
#endif
    {
#if defined(HAVE_WALKCONTEXT)
        struct wcargs args = {out, 1};
        ucontext_t u = {0};

        if (0 == getcontext(&u)) {
            walkcontext(&u, walkcontext_frame, &args);
        } else
#endif
        {
            fprintf(out, "Failed to get backtrace info");
        }
    }
    fprintf(out, "\n");
#endif  /*!BACKTRACE*/
}

#endif  /*!__CYGWIN__ && !linux */
#endif  /*unix*/

/*quiet archivers*/
extern int edbt_unix(void);

int
edbt_unix(void)
{
    return 0;
}
/*end*/
