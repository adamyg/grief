#include <edidentifier.h>
__CIDENT_RCSID(gr_edbt_linux_c,"$Id: edbt_linux.c,v 1.15 2024/06/14 19:06:23 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edbt_linux.c,v 1.15 2024/06/14 19:06:23 cvsuser Exp $
 * linux backtrace implementation
 *
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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

#if defined(linux)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#define __USE_GNU                               /* REG_xxx */

#include <edstacktrace.h>
#include <libstr.h>

#include <stdlib.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <memory.h>
#include <ucontext.h>
#if defined(HAVE_BFD_H)
#define EDBT_DEMANGLE
#include <bfd.h>                                /* libbfd, binutils-dev */
#endif
#if defined(HAVE_LIBIBERTY)
#if defined(HAVE_CPLUS_DEMANGLE)
#define EDBT_DEMANGLE
#endif
#if defined(HAVE_LIBIBERTY_H)
#include <libiberty.h>
#endif
#if defined(HAVE_DEMANGLE_H)
#include <demangle.h>                           /* cplus_demangle */
#endif
#endif
//  #if !defined(NO_CPP_DEMANGLE)
//  #include <cxxabi.h>
//  #endif

#if defined(REG_RIP)
#define STACK_IA64
#elif defined(REG_EIP)
#define STACK_X86
#else
#define STACK_GENERIC
#endif

#if defined(STACK_X86) || defined(STACK_IA64)
#define EDBT_BACKTRACE
#endif
#if defined(EDBT_BACKTRACE)
#if defined(HAVE_BFD_H)
//  Currently, the function name and offset only be obtained on systems that use the ELF binary
//  format for programs and libraries. On other systems, only the hexadecimal return address will be
//  present. Also, you need to pass additional flags to the linker to make the function names
//  available to the program. For example, on systems using GNU ld, you must pass (-rdynamic)
//  instructing the linker to add all symbols, not only used ones, to the dynamic symbol table.
//
#define EDBT_SOURCEINFO
#endif
#endif

#define MAX_SYMBOL_LENGTH 512
#define MAX_FRAMES 64

#if defined(EDBT_SOURCEINFO)
static const char *         x_progname;
static bfd *                x_bfd;
static asymbol **           x_syms;

typedef struct {
    bfd_vma                 si_pc;
    bfd_boolean             si_found;
    const char *            si_filename;
    const char *            si_function;
    unsigned int            si_line;
} search_info; 


static void
section_search(bfd *bfd, asection *section, void *data)
{
    search_info *psi = (search_info *)data; 
    bfd_size_type size;
    bfd_vma vma;

    if (TRUE == psi->si_found) {
        return;
    }

#if defined(HAVE_BFD_SECTION_FLAGS)
    if (0 == (bfd_section_flags(bfd, section) & SEC_ALLOC)) {
        return;
    }
#else
    if (0 == (bfd_get_section_flags(bfd, section) & SEC_ALLOC)) {
        return;
    }
#endif

#if defined(HAVE_BFD_SECTION_VMA)
    vma = bfd_section_vma(bfd, section);
#else
    vma = bfd_get_section_vma(bfd, section);
#endif
    if (psi->si_pc < vma) {
        return;
    }

#if defined(HAVE_BFD_SECTION_SIZE)
    size = bfd_section_size(section);
#else
    size = bfd_get_section_size(section);
#endif
    if (psi->si_pc >= vma + size) {
        return;
    }

    psi->si_found = bfd_find_nearest_line(bfd, section, x_syms,
                        psi->si_pc - vma, &psi->si_filename, &psi->si_function, &psi->si_line);
}


static const char *
remove_path(const char *path, int level)
{
    size_t len = strlen(path) - 1, stop = 0;
    const char *fname = NULL;
    char c;

    while (0 != (c = path[len])) {
        if ('\\' == c || '/' == c) {
            fname = path + len + 1;
            if (++stop >= level) {
                break;
            }
        }
        --len;
    }
    if (!fname) fname = path;
    return fname;
}


static int
sourceinfo(FILE *out, const char *modname, const char *address,
        char *filename, int flen, unsigned int *line)
{
    static unsigned once = 0;
    const char *progname;

    if (NULL == x_bfd && 1 == ++once) {
        bfd_init();                         // runtime initialisatio
    }

    if (NULL == modname || 0 == *modname) {
        modname = "/proc/self/exe";
    }

    if (modname) {
        if (NULL != (x_bfd = bfd_openr(modname, NULL))) {
            if (0 == bfd_check_format(x_bfd, bfd_archive)) {
                char **matching = NULL;

                if (bfd_check_format_matches(x_bfd, bfd_object, &matching) > 0) {
                    if (HAS_SYMS == (bfd_get_file_flags(x_bfd) & HAS_SYMS)) {
                        unsigned int size = 0;
                        long symcount =     // static
                            bfd_read_minisymbols(x_bfd, FALSE, (void **)(&x_syms), &size);

                        if (symcount <= 0)  // dynamic
                            bfd_read_minisymbols(x_bfd, TRUE, (void **)(&x_syms), &size);
                    }
                }
            } else if (out) {
                fprintf(out, "bfd(%s) : cannot get addresses from archive\n", modname);
            }
        } else if (out) {
            fprintf(out, "bfd_openr(%s) : %s\n", modname, bfd_errmsg (bfd_get_error ()));
        }
    }

    if (x_bfd) {
        search_info si = {0};

        si.si_pc = bfd_scan_vma(address, NULL, 16);
        bfd_map_over_sections(x_bfd, section_search, (void *)&si);
        if (si.si_found) {
            const char *t_filename = si.si_filename;

            if (t_filename && *t_filename) {
                strxcpy(filename, remove_path(t_filename, 2), flen);
            }
            *line = si.si_line;
            return 1;
        }
        if (out) fprintf(out, "bfd(%s/%p): address not found\n", modname, address);
    }
    return -1;
}
#endif  //EDBT_SOURCEINFO


void
edbt_init(const char *progname, int options, FILE *out)
{
#if defined(EDBT_SOURCEINFO)
    if (progname) {
        x_progname = strdup(progname);
    }
#endif
}


void
edbt_auto(void)
{
}


void
edbt_stackdump(FILE *out, int level)
{
    ucontext_t *ucontext = malloc(sizeof(ucontext_t));
    int count = 0;

#if defined(EDBT_BACKTRACE)
#if defined(EDBT_SOURCEINFO)
    char filename[MAX_SYMBOL_LENGTH];
    unsigned int line;
#endif
    Dl_info dlinfo;
    void **bp = 0;
    void *ip = 0;
#else
    void *bt[MAX_FRAMES] = {0};
    char **strings;
    size_t sz;
    long i;
#endif

    getcontext(ucontext);

#if defined(EDBT_BACKTRACE)
#if defined(STACK_IA64)                         // 64 bit system
    ip = (void *)ucontext->uc_mcontext.gregs[REG_RIP];
    bp = (void **)ucontext->uc_mcontext.gregs[REG_RBP];
#elif defined(STACK_X86)                        // 32 bit system
    ip = (void *)ucontext->uc_mcontext.gregs[REG_EIP];
    bp = (void **)ucontext->uc_mcontext.gregs[REG_EBP];
#endif

    while (bp && ip)  {
        const char *cxx_symbol = NULL, *symbol;

        memset(&dlinfo, 0, sizeof(dlinfo));
        if (! dladdr(ip, &dlinfo)) {
             break;
        }

        if (NULL != (symbol = dlinfo.dli_sname)) {
            int status = 0;

#if defined(EDBT_DEMANGLE)
#if defined(HAVE_LIBBFD)
#define DEMANGLE_PARAMS     (1 << 0)            // + function arguments
#define DEMANGLE_ANSI       (1 << 1)            // + const, volatile, etc
            if (x_bfd) {
                cxx_symbol = bfd_demangle(x_bfd, symbol, DEMANGLE_ANSI | DEMANGLE_PARAMS);
            }
#elif defined(HAVE_CPLUS_DEMANGLE)
            cxx_symbol = cplus_demangle(symbol, DMGL_ANSI | DMGL_PARAMS);
#else
//          cxx_symbol = abi::__cxa_demangle(symbol, NULL, NULL, &status);
            cxx_symbol = __cxa_demangle(symbol, NULL, NULL, &status);
#endif
#endif
            if (0 == status && cxx_symbol) {
                symbol = cxx_symbol;
            }
        } else {
            symbol = "n.a";
        }

        if (count >= level) {
            if (count == level) {
                fprintf(out, "\nCall trace:\n");
            } else {
                fprintf(out, "  called by ");
            }

#if defined(EDBT_SOURCEINFO)
            if (sourceinfo(out, x_progname ? x_progname : dlinfo.dli_fname, (void *)((char *)ip - 1), 
                    filename, MAX_SYMBOL_LENGTH, &line) > 0) {
                fprintf(out, "%s (+0x%lu) [%s, line %u]\n", 
                    symbol, (unsigned long)(ip - dlinfo.dli_saddr), filename, line);
            
            } else 
#endif
                if (dlinfo.dli_fname && *dlinfo.dli_fname) {
                    fprintf(out, "%s (+0x%lu) [%s]\n", 
                        symbol, (unsigned long)(ip - dlinfo.dli_saddr), dlinfo.dli_fname);

                } else {
                    fprintf(out, "%s (+0x%lu)\n", 
                        symbol, (unsigned long)(ip - dlinfo.dli_saddr));
                }
        }

#if defined(EDBT_DEMANGLE)
        free((void *)cxx_symbol);
#endif
        if (dlinfo.dli_sname && 0 == strcmp(dlinfo.dli_sname, "main")) {
            fprintf(out, "  in program %s\n\n", dlinfo.dli_fname);
            break;
        }
        ip = (void *)bp[1];
        bp = (void **)bp[0];
        ++count;
    }

#else   /*!(STACK_X86 || STACK_IA64)*/
    //
    //  Example:
    //      Call trace (18 levels):
    //      /home/user/work/cr/src/bin/cr() [0x80b2818]
    //          called by /home/user/work/cr/src/bin/cr() [0x80533fa]()
    //          called by /home/user/work/cr/src/bin/cr() [0x8052a33]()
    //          called by /home/user/work/cr/src/bin/cr() [0x8052857]()
    //                              :
    //          called by /home/user/work/cr/src/bin/cr() [0x808236c]()
    //          called by /lib/tls/i686/cmov/libc.so.6(__libc_start_main+0xe6) [0x340bd6]()
    //          called by /home/user/work/cr/src/bin/cr() [0x804c101]()
    //
    sz = backtrace(bt, MAX_FRAMES);
    strings = backtrace_symbols(bt, sz);

    fprintf(out, "\nCall trace (%ld levels):\n", (unsigned long)(sz - level));
    fprintf(out, "%s\n", strings[level]);
    for (i = level + 1; i < sz; ++i) {
	fprintf(out, "  called by %s\n", strings[i]);
    }
    fprintf(out, "\n");
    free(strings);
#endif

    free(ucontext);
}
#endif  /*linux*/

/*quiet archivers*/
extern int edbt_linux(void);
int
edbt_linux(void)
{
    return 0;
}

/*end*/
