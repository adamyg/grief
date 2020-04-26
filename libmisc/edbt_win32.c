#include <edidentifier.h>
__CIDENT_RCSID(gr_edbt_win32_c,"$Id: edbt_win32.c,v 1.21 2020/04/13 21:06:07 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edbt_win32.c,v 1.21 2020/04/13 21:06:07 cvsuser Exp $
 * win32 (include cygwin) backtrace implementation.
 *
 *
 *
 * Copyright (c) 1998 - 2017, Adam Young.
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
#include <edstacktrace.h>                       /* public interface */
#include <edassert.h>
#include <eddebug.h>
#include <stdlib.h>
#include <libstr.h>

#if defined(WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)
#ifndef  WIN32_LEAN_AND_MEAN
#define  WIN32_LEAN_AND_MEAN
#endif
#undef   u_char
#include <windows.h>

#if defined(__WATCOMC__) || defined(__CYGWIN__) || defined(__MINGW32__)
/* WATCOMC 1.9, 64bit issues */
/* CYGWIN 1.7, no 64 support */
#include <imagehlp.h>
#else
#if defined(WIN64) || defined(_WIN64)
#define EDBT_64BIT                              /* windows sdk */
#elif defined(_MSC_VER) && \
        (defined(_M_X64) || defined(__amd64__))
#define EDBT_64BIT
#endif
#include <dbghelp.h>
#endif

#if !defined(EDBT_64BIT)
#define EDBT_32BIT
#endif

#if defined(__CYGWIN__)
#include <sys/cygwin.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <io.h>
#if defined(__CYGWIN__)
#include <unistd.h>
#endif
#include <tailqueue.h>
#include <rbtree.h>

#pragma comment(lib, "dbghelp.lib")

typedef DWORD   (__stdcall *SymSetOptions_t)(DWORD SymOptions);

typedef BOOL    (__stdcall *SymInitialize_t)(HANDLE hProcess, PSTR UserSearchPath, BOOL fInvadeProcess);

typedef DWORD   (__stdcall *SymLoadModule_t)(HANDLE hProcess, HANDLE hFile, PSTR ImageName,
                                PSTR ModuleName, DWORD BaseOfDll, DWORD SizeOfDll);

typedef USHORT  (__stdcall *CaptureStackBackTrace_t)(ULONG FramesToSkip, ULONG FramesToCapture,
                                PVOID *BackTrace, PULONG BackTraceHash);

#if defined(__CYGWIN__) || defined(__MINGW32__)
#define PCTSTR const char *
#define PTSTR char *
#endif
typedef DWORD   (__stdcall *UnDecorateSymbolName_t)(PCTSTR DecoratedName, PTSTR UnDecoratedName,
                                DWORD UndecoratedLength, DWORD Flags);

#if defined(EDBT_64BIT)
typedef BOOL    (__stdcall *StackWalk_t)(DWORD MachineType, HANDLE hProcess, HANDLE hThread,
                                LPSTACKFRAME64 StackFrame, PVOID ContextRecord,
                                PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
                                PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
                                PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
                                PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
typedef PVOID   (__stdcall *SymFunctionTableAccess_t)(HANDLE hProcess, DWORD64 AddrBase);
typedef DWORD64 (__stdcall *SymGetModuleBase_t)(HANDLE hProcess, DWORD64 dwAddr);
typedef BOOL    (__stdcall *SymGetSymFromAddr_t)(
                                HANDLE hProcess, DWORD64 dwAddr, PDWORD64 pdwDisplacement, PIMAGEHLP_SYMBOL64 Symbol);
typedef BOOL    (__stdcall *SymGetLineFromAddr_t)(
                                HANDLE hProcess, DWORD64 dwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line);
#else
typedef BOOL    (__stdcall *StackWalk_t)(DWORD MachineType, HANDLE hProcess, HANDLE hThread,
                                LPSTACKFRAME StackFrame, PVOID ContextRecord,
                                PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
                                PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
                                PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
                                PTRANSLATE_ADDRESS_ROUTINE TranslateAddress);
typedef PVOID   (__stdcall *SymFunctionTableAccess_t)(HANDLE hProcess, DWORD AddrBase);
typedef DWORD   (__stdcall *SymGetModuleBase_t)(HANDLE hProcess, DWORD dwAddr);
typedef BOOL    (__stdcall *SymGetSymFromAddr_t)(HANDLE hProcess, DWORD dwAddr, PDWORD pdwDisplacement,
                                PIMAGEHLP_SYMBOL Symbol);
typedef BOOL    (__stdcall *SymGetLineFromAddr_t)(HANDLE hProcess, DWORD dwAddr,
                                PDWORD pdwDisplacement, PIMAGEHLP_LINE Line);
#endif

LONG WINAPI                     __edbt_exceptionfilter(EXCEPTION_POINTERS *rec);

static HINSTANCE                hDbghelpDll = NULL;
static HINSTANCE                hKernel32Dll = NULL;
static SymSetOptions_t          fSymSetOptions;
static SymInitialize_t          fSymInitialize;
static SymLoadModule_t          fSymLoadModule;
static StackWalk_t              fStackWalk;
static CaptureStackBackTrace_t  fCaptureStackBackTrace;
static SymFunctionTableAccess_t fSymFunctionTableAccess;
static SymGetModuleBase_t       fSymGetModuleBase;
static SymGetSymFromAddr_t      fSymGetSymFromAddr;
static SymGetLineFromAddr_t     fSymGetLineFromAddr;
static UnDecorateSymbolName_t   fUnDecorateSymbolName;

#define MAX_SYMBOL 512
#define MAX_FRAMES 64

#if defined(_X86_) && defined(EDBT_32BIT)
#define ADDR_T DWORD
#else
#define ADDR_T DWORD64
#endif

static ADDR_T                   frames[MAX_FRAMES] = {0};

static const char *             LastError(void);


/*
 *  symbol name demangler
 */
#if defined(__WATCOMC__)
extern size_t __cdecl __demangle_l(             // DEMANGLE A C++ NAME
    char const *input,                          // - mangled C++ name
    size_t len,                                 // - length of mangled name
    char *output,                               // - for demangled C++ name
    size_t size);                               // - size of output buffer

extern size_t __cdecl __demangle_r(             // DEMANGLE A C++ NAME
    char const *input,                          // - mangled C++ name
    size_t len,                                 // - length of mangled name
    char **output,                              // - for demangled C++ name
    size_t size,                                // - size of output buffer
    char * (*realloc)(char *, size_t));         // - size adjuster for output

static const char *
watcom_demangle(const char *funcname)
{
    static char t_name[MAX_SYMBOL];

#if (0)                                         // work in progress
    int len = __demangle_l(funcname, strlen(funcname), t_name, MAX_SYMBOL-1);
    t_name[len] = 0;
    return t_name;

#else
    const char *aux = strstr(funcname, "W?");   // standard Watcom prefix name is "W?"

    if (aux) {
        char *name = t_name;
        size_t len = 0;

        aux += 2;
        while (*aux && *aux != '$') {           // copy characters until the end of string or '$' character
            if (len > (MAX_SYMBOL - 2)) {
                break;
            }
            name[len++] = *aux++;
        }
        name[len] = '\0';
        return t_name;
    }
    return funcname;
#endif
}


#elif defined(_MSC_VER) || defined(__CYGWIN__)
static const char *
msc_demangle(const char *funcname)
{
    static char t_name[MAX_SYMBOL];

    if ('?' == funcname[0]) {
        if (fUnDecorateSymbolName) {
            fUnDecorateSymbolName(funcname, t_name, sizeof(t_name), 0);
            return t_name;
        }
    }
    return funcname;
}
#endif


/*
 *  map loader.
 */
#if defined(__WATCOMC__) || defined(_MSC_VER) || \
            defined(__CYGWIN__)
typedef TAILQ_HEAD(modules, module)
                        modulelist_t;           // name global module list

typedef struct module {
    TAILQ_ENTRY(module) m_node;
    unsigned            m_count;
    void *              m_lower;
    void *              m_upper;
    char                m_name[1];
} module_t;

typedef RB_HEAD(symboltree, symbol)
                        symboltree_t;

typedef struct symbol {
    RB_ENTRY(symbol)    s_node;                 // tree node
    void *              s_address;
    const module_t *    s_module;
    char                s_flag;
    char                s_name[1];
} symbol_t;

static int              map_compare(symbol_t *a, symbol_t *b);

RB_PROTOTYPE_STATIC(symboltree, symbol, s_node, map_compare);
RB_GENERATE_STATIC(symboltree, symbol, s_node, map_compare);

static modulelist_t     x_modules;
static symboltree_t     x_symbols;


static module_t *
map_module(const char *mname, size_t mlen)
{
    modulelist_t *modules = &x_modules;
    module_t *module;

    if (NULL != (module = malloc(sizeof(module_t) + mlen))) {
        memset(module, 0, sizeof(module_t));
        TAILQ_INSERT_TAIL(modules, module, m_node);
        memcpy(module->m_name, mname, mlen);
        module->m_name[mlen] = 0;
    }
    return module;
}


static symbol_t *
map_symbol(module_t *module, const char *sname, size_t slen, void *address)
{
    symboltree_t *symbols = &x_symbols;
    symbol_t *symbol = NULL;

    if (module) {
        if (NULL != (symbol = malloc(sizeof(symbol_t) + slen))) {
            memset(symbol, 0, sizeof(symbol_t));
            symbol->s_module = module;
            symbol->s_address = address;
            memcpy(symbol->s_name, sname, slen);
            symbol->s_name[slen] = 0;
            if (RB_INSERT(symboltree, symbols, symbol)) {
                free(symbol);
                symbol = NULL; /*non-unqiue*/
            }
        }

        if (1 == ++module->m_count) {
            module->m_lower = address;
            module->m_upper = address;
        } else {
            if (address < module->m_lower) module->m_lower = address;
            if (address > module->m_upper) module->m_upper = address;
        }
    }
    return symbol;
}


static void
map_load(const char *progname)
{
    char line[2048] = {0}, *name = line, *end;
    FILE *file = NULL;

    // derive map from program name
    if (progname && *progname) {
        strxcpy(name, progname, sizeof(line)-5);

    } else {
        int namelen =                           // module name (note 1/2 buffer)
                GetModuleFileName(GetModuleHandle(NULL), line, (sizeof(line)/2)-5);

        if (namelen > 0) {
            line[namelen] = 0;

#if defined(__CYGWIN__)                         // convert to a POSIX name
#if defined(CCP_WIN_A_TO_POSIX)
            namelen = cygwin_conv_path(CCP_WIN_A_TO_POSIX|CCP_ABSOLUTE, line, NULL, 0);
            if (namelen > 0 && namelen < ((sizeof(line)/2) - 5)) {
                name = line + sizeof(line)/2;   // use upper half of buffer
                cygwin_conv_path(CCP_WIN_A_TO_POSIX|CCP_ABSOLUTE, line, name, namelen);
            }
#endif
#endif  /*__CYGWIN__*/
        }
    }

    if (NULL == (end = (char *)strrchr(name, '.')) ||
            str_icmp(end, ".exe")) {
        end = name + strlen(name);
    }
    strcpy(end, ".map");
    file = fopen(name, "rt");
    ED_TRACE(("map<%s>=%d\n", name, (file ? 1 : 0)))

    // parse map symbols
    if (file) {
        module_t *module = NULL;
        unsigned lineno = 1;

        RB_INIT(&x_symbols);
        TAILQ_INIT(&x_modules);
        for (;;) {
            size_t len;

            if (NULL == fgets(line, sizeof(line), file) ||
                    (len = strlen(line)) < 1 || line[len - 1] != '\n') {
                break;
            }
            line[--len] = 0;
            if ('\r' == line[len - 1]) {
                line[--len] = 0;
            }

#if defined(__WATCOMC__)
            if (1 == ++lineno) {                // map signature
                if (0 == strncmp(line, "Open Watcom", 11)) {
                    continue;
                }
                break;
            }

            if ('M' == *line) {
                if (0 == strncmp(line, "Module: ", 8)) {
                    const char *s, *e;          // module

                    if (NULL != (s = strchr(line + 8, '(')) &&
                            NULL != (e = strrchr(++s, ')'))) {
                        module = map_module(s, e - s);
                    }
                }

            } else if (module && isdigit(*((unsigned char *)line))) {
                const char *sname;              // symbol
                void *address = 0;
                char flag;

                if (2 == sscanf(line, "%p%c", &address, &flag) && address &&
                        NULL != (sname = strrchr(line + 8, ' ')) && *++sname) {
                    const size_t slen = strlen(sname);
                    symbol_t *symbol;

                    ED_TRACE(("module:%s, symbol:%.*s, address:%p\n", module->m_name, slen, sname, address))
                    if (NULL != (symbol = map_symbol(module, sname, slen, address))) {
                        symbol->s_flag = flag;
                    }
                }
            }

#elif defined(_MSC_VER)
            if (' ' == *line && isdigit((unsigned char)line[1])) {
                const char *sname = NULL, *mname = NULL;
                char *cursor = line + 1;
                size_t slen = 0, mlen = 0;
                void *address = 0;
                int token = 0;

                do {
                    switch (++token) {
                    case 1:         /* 0001:<address> */
                        if (1 == sscanf(cursor, "0001:%p", &address)) {
                            cursor += 5;
                            while (*cursor && isxdigit(*((unsigned char *)cursor))) ++cursor;
                        } else {
                            token = 99;
                        }
                        break;
                    case 2:         /* <symbol> */
                        sname = cursor;
                        while (*cursor && ' ' != *cursor) ++cursor;
                        if ((slen = cursor - sname) > 0) {
                            *cursor++ = 0;
                        } else {
                            token = 99;
                        }
                        break;
                    case 3:         /* rva+base */
                        if (1 == sscanf(cursor, "%p", &address)) {
                            while (*cursor && isxdigit(*((unsigned char *)cursor))) ++cursor;
                        } else {
                            token = 99;
                        }
                        break;
                    case 4:         /* <types ..> */
                        switch (*cursor++) {
                        case 'f':       /* f [i ] */
                            if (0 == strncmp(cursor, " i", 2)) cursor += 2;
                            break;
                        default:
                            token = 99;
                            break;
                        }
                        break;
                    case 5:         /* <Lib:Object> */
                        mname = cursor;
                        while (*cursor && ' ' != *cursor) ++cursor;
                        if ((mlen = cursor - mname) > 0) {
                            const char *mangled = msc_demangle(sname);

                            ED_TRACE(("module:%.*s, symbol:%.*s/%s, address:%p\n",
                                mlen, mname, slen, sname, (sname == mangled ? "" : mangled), address))
                            if (NULL == module || 0 != strcmp(mname, module->m_name)) {
                                module = map_module(mname, mlen);
                            }
                            map_symbol(module, sname, slen, address);
                        }
                        break;
                    }

                    while (*cursor && ' ' == *cursor) {
                        ++cursor;
                    }
                } while (*cursor && token < 5);
            }

#elif defined(__CYGWIN__)
            // .text          0x00401000       0x90 /usr/lib/gcc/i686-pc-cygwin/4.5.3/../../../crt0.o
            //       0x00401000                _WinMainCRTStartup
            //       0x00401000                _mainCRTStartup
            //
            if (0 == strncmp(line, " .text", 6)) {
                void *address = 0;
                unsigned size = 0;
                int cursor = -1;
                                                // %n = byte count
                if (sscanf(line + 6, " 0x%p %x %n", &address, &size, &cursor) >= 2 &&
                        address && size && cursor > 0) {
                    ED_TRACE(("module:%s, base:%p, size:%x\n", line + cursor + 6, address, size))
                    module = map_module(line + cursor, strlen(line + cursor));
                }

            } else if (module) {
                const char *symbol;
                void *address = 0;
                int cursor = -1;

                if (sscanf(line, " 0x%p %n", &address, &cursor) >= 1 && address &&
                        (symbol = line + cursor) > line && *symbol) {
                    ED_TRACE(("symbol:%s, base:%p\n", symbol, address))
                    map_symbol(module, symbol, strlen(symbol), address);
                } else {
                    module = NULL;
                }
            }

#else
#error map_load: unknown/supported map file format
#endif
        }

#if defined(ED_LEVEL) && (ED_LEVEL >= 1)
        {   /*dump*/
            symboltree_t *symbols = &x_symbols;
            const char *module = "";
            symbol_t *symbol;

            RB_FOREACH_REVERSE(symbol, symboltree, symbols) {
                if (0 != strcmp(module, symbol->s_module->m_name)) {
                    module = symbol->s_module->m_name;
                    ED_TRACE(("%s\n", module))
                }
                ED_TRACE((" 0x%08p%c <%s>\n", symbol->s_address, ' ', symbol->s_name))
            }
        }
#endif
        fclose(file);
    }
}


static int
#if defined(EDBT_64BIT)
map_lookup(void *address, PIMAGEHLP_SYMBOL64 psymbol, PIMAGEHLP_LINE64 pline)
#else
map_lookup(void *address, PIMAGEHLP_SYMBOL psymbol, PIMAGEHLP_LINE pline)
#endif
{
    symboltree_t *symbols = &x_symbols;
    const symbol_t *symbol;
    symbol_t key = {0};

    key.s_address = address;
    if (NULL != (symbol = RB_NFIND(symboltree, symbols, &key))) {
        if (symbol && address >= symbol->s_address) {
            strncpy(psymbol->Name, symbol->s_name, psymbol->MaxNameLength);
            psymbol->Address = (ADDR_T)symbol->s_address;
            if (pline) {
                pline->FileName = (char *)symbol->s_module->m_name;
                pline->LineNumber = 0;
            }
            return TRUE;
        }
    }
    return FALSE;
}


static int
map_compare(symbol_t *a, symbol_t *b)
{
    if (a->s_address < b->s_address) {          // highest to lowest
        return 1;
    } else if (a->s_address > b->s_address) {
        return -1;
    }
    return 0;
}
#endif  /*__WATCOMC__ || _MSC_VER*/


static const char *
remove_path(char *path, size_t level)
{
    size_t stop = 0, i = strlen(path) - 1;
    char c, *fname = NULL;

    while (0 != (c = path[i])) {
        if ('\\' == c || '/' == c) {
            fname = path + i + 1;
            if (++stop >= level) {
                break;
            }
        }
        --i;
    }
    if (!fname) fname = path;
    return fname;
}


static LPTOP_LEVEL_EXCEPTION_FILTER x_org_filter = NULL;

static int
prt(char *buffer, const char *fmt, ...)
{
    static const char hex[] = "0123456789abcdef";
    char ch, *cursor = buffer;
    va_list ap;

    va_start(ap, fmt);
    while (0 != (ch = *fmt++)) {
        if ('%' != ch) {
            *cursor++ = ch;
            continue;
        }
        switch (*fmt++) {
        case 's': {         /* %s */
                const char *s = va_arg(ap, const char *);
                if (s) {
                    while (0 != (ch = *s++)) {
                        *cursor++ = ch;
                    }
                }
            }
            break;
        case 'p': {         /* %p */
                ADDR_T value = (ADDR_T)va_arg(ap, void *);
                char *stack;
                int i;

                *cursor++ = '0';
                *cursor++ = 'x';
                for (i = 0; i < (SIZEOF_VOID_P * 2); ++i) {
                    *cursor++ = '0';
                }
                stack = cursor;
                while (value) {
                    *--stack = hex[value & 0xf];
                    value >>= 4;
                }
            }
            break;
        case 'u': {         /* %u */
                unsigned value = (ADDR_T)va_arg(ap, unsigned);
                char t_buffer[16], *end = t_buffer + sizeof(t_buffer),
                        *stack = end;

                do {
                    *--stack = hex[value % 10];
                    value /= 10;
                } while (value);
                while (stack < end) {
                    *cursor++ = *stack++;
                }
            }
            break;
        }
    }
    va_end(ap);
    return (cursor - buffer);
}


static const char *
exception_description(const EXCEPTION_RECORD *ex)
{
    static char unknown[32];
    const char *desc = "";

    switch (ex->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:      desc = "ACCESS VIOLATION"; break;
      // EXCEPTION_BREAKPOINT
      // EXCEPTION_SINGLE_STEP
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: desc = "ARRY BOUNDS EXCEEDED"; break;
    case EXCEPTION_DATATYPE_MISALIGNMENT: desc = "DATA MISALIGNMENT"; break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:  desc = "DENORMAL OPERAND"; break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:    desc = "DIVIDE BY ZERO"; break;
    case EXCEPTION_FLT_INEXACT_RESULT:    desc = "INEXACT RESULT"; break;
    case EXCEPTION_FLT_INVALID_OPERATION: desc = "INVALID OPERATION"; break;
    case EXCEPTION_FLT_OVERFLOW:          desc = "OVERFLOW"; break;
    case EXCEPTION_FLT_STACK_CHECK:       desc = "FLOAT STACK CHECK"; break;
    case EXCEPTION_FLT_UNDERFLOW:         desc = "FLOAT UNDERFLOW"; break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:   desc = "ILLEGAL INSTRUCTION"; break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:    desc = "DIVIDE BY ZERO"; break;
    case EXCEPTION_INT_OVERFLOW:          desc = "INTEGER OVERFLOW"; break;
    case EXCEPTION_PRIV_INSTRUCTION:      desc = "PRIVILEGED INSTRUCTION"; break;
    case EXCEPTION_STACK_OVERFLOW:        desc = "STACK OVERFLOW"; break;
    default:
        sxprintf(unknown, sizeof(unknown), "UNKNOWN(%u)", (unsigned)ex->ExceptionCode);
        return unknown;
    }
    return desc;
}


/*
 *  Example:
 *      Exception ACCESS VIOLATION at 0x0046bf30 (do_regress_op_(+0x7b)):
 *
 *      Call trace:
 *      __edbt_exceptionfilter(+0xb0) - 004a16d9 [libmisc/edbt_win32.c, line 580]
 *        called by GetProfileStringW(+0x12aa3) - 75a6003f
 *        called by __ExceptionFilter(+0x6c) - 004c284c
 *        called by LdrRemoveLoadAsDataTable(+0xd22) - 771fb42b
 *        called by KiUserExceptionDispatcher(+0xf) - 771b0133
 *              <exception>
 *        called by execute_xmacro_(+0xb60) - 00408aa4 [gr\builtin.c, line 719]
 *        called by execute_xmacro_(+0x22c) - 00408170 [gr\builtin.c, line 356]
 *        called by execute_macro_(+0xdb) - 00407f33 [gr\builtin.c, line 278]
 *        called by execute_nmacro_(+0x43) - 00407dc5 [gr\builtin.c, line 211]
 *        called by execute_str_(+0x30c) - 00407ce2 [gr\builtin.c, line 176]
 *        called by do_execute_macro_(+0x56) - 004350c2 [gr\mac1.c, line 328]
 *        called by execute_xmacro_(+0xb60) - 00408aa4 [gr\builtin.c, line 719]
 *        called by execute_xmacro_(+0x22c) - 00408170 [gr\builtin.c, line 356]
 *        called by execute_macro_(+0xdb) - 00407f33 [gr\builtin.c, line 278]
 *        called by execute_nmacro_(+0x43) - 00407dc5 [gr\builtin.c, line 211]
 *        called by execute_str_(+0x30c) - 00407ce2 [gr\builtin.c, line 176]
 *        called by key_execute_(+0x17e) - 00428622 [gr\keyboard.c, line 1692]
 *        called by do_process_(+0x3c) - 0046b722 [gr\m_main.c, line 66]
 *        called by main_loop_(+0x3a) - 0046b6a3 [gr\m_main.c, line 31]
 *        called by main_(+0x363) - 0043a547 [gr\main.c, line 527]
 *
 *      The write instruction at 0x0046bf05 referenced memory at 0x00000000
 *      EAX=0x00000000 EBX=0x000cd580 ECX=0x00000380 EDX=0x000cd580
 *      ESI=0x00000000 EDI=0x00000000 BP =0x000cd544 ESP=0x000cd514
 *      EIP=0x0046bf05 EFL=0x00010246 CS =0x00000023 SS =0x0000002b
 *      DS =0x0000002b ES =0x0000002b FS =0x00000053 GS =0x0000002b
 */
LONG WINAPI
__edbt_exceptionfilter(EXCEPTION_POINTERS *rec)
{
    static int edbt_nesting = 0;                // guard recursive entries
    const EXCEPTION_RECORD *ex = rec->ExceptionRecord;
    const CONTEXT *context = rec->ContextRecord;
    const int fd = fileno(stderr);
    char buf[256];
    int len = 0;

    if (1 == ++edbt_nesting) {                  // backtrace
        const char *message;
        char symbol[MAX_SYMBOL];

        symbol[0] = 0;
        (void) edbt_symbol(ex->ExceptionAddress, symbol, sizeof(symbol));
        len = prt(buf, "Exception %s at %p (%s):\n",
                    exception_description(ex), ex->ExceptionAddress, symbol);
        if (NULL != (message = edAssertMsg())) {
            fputs(message, stderr);
        }
        fflush(stderr);
#if defined(__CYGWIN__)
        (void) write(fd, buf, len);
#else
        (void) _write(fd, buf, len);
#endif
        edbt_stackdump(stderr, 1);
    } else {
        len = prt(buf, "Exception %s:\n", exception_description(ex));
    }

    --edbt_nesting;

    if (x_org_filter) {                         // system implementation
        return (*x_org_filter)(rec);
    }

    len = 0;
    switch (ex->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
        // The thread tried to read from or write to a virtual address for which it does not
        // have the appropriate access.
        len += prt(buf+len, "The %s instruction at %p referenced memory at %p\n",
                    (0 == ex->ExceptionInformation[0] ? "read" : "write"),
                    ex->ExceptionAddress, (void *)(ex->ExceptionInformation[1]));
        break;

    case EXCEPTION_DATATYPE_MISALIGNMENT:
        // The thread tried to read or write data that is misaligned on hardware that does not
        // provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries;
        // 32-bit values on 4-byte boundaries, and so on.
        len += prt(buf+len, "The instruction at %p tried to reference unaligned data at %p.\n",
                    ex->ExceptionAddress, (void *)(ex->ExceptionInformation[2]));
        break;

    case EXCEPTION_FLT_DENORMAL_OPERAND:
        // One of the operands in a floating-point operation is denormal.
        // A denormal value is one that is too small to represent as a standard floating-point value.
        len += prt(buf+len, "The instruction at %p caused a denormal operand floating point exception.\n",
                    ex->ExceptionAddress);
        break;

    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        // The thread tried to divide a floating-point value by a floating-point divisor of zero.
        len += prt(buf+len, "The instruction at %p caused a division by zero floating point exception.\n",
                    ex->ExceptionAddress);
        break;

    case EXCEPTION_FLT_INEXACT_RESULT:
        // The result of a floating-point operation cannot be represented exactly as a decimal fraction.
        len += prt(buf+len, "The instruction at %p caused an inexact value floating point exception.\n",
                    ex->ExceptionAddress);
        break;

    case EXCEPTION_FLT_INVALID_OPERATION:
        // This exception represents any floating-point exception not included in this list.
        len += prt(buf+len, "The instruction at %p caused an invalid operation floating point exception.\n",
                    ex->ExceptionAddress);
        break;

    case EXCEPTION_FLT_OVERFLOW:
        // The exponent of a floating-point operation is greater than the magnitude allowed by
        // the corresponding type.
        len += prt(buf+len, "The instruction at %p caused an overflow floating point exception.\n",
                    ex->ExceptionAddress);
        break;

    case EXCEPTION_FLT_STACK_CHECK:
        // The stack overflowed or underflowed as the result of a floating-point operation.
#if defined(SW_C1)
        len += prt(buf+len, "The instruction at %p caused a stack %s floating point exception.\n",
                    ex->ExceptionAddress, ((context->FloatSave.StatusWord & SW_C1) ? "overflow": " underflow"));
#else
        len += prt(buf+len, "The instruction at %p caused a stack %u floating point exception.\n",
                    ex->ExceptionAddress, (unsigned)context->FloatSave.StatusWord);
#endif
        break;

    case EXCEPTION_FLT_UNDERFLOW:
        // The exponent of a floating-point operation is less than the magnitude allowed by
        // the corresponding type.
        len += prt(buf+len, "The instruction at %p caused an underflow floating point exception.\n",
                    ex->ExceptionAddress);
        break;

    case EXCEPTION_ILLEGAL_INSTRUCTION:
        // The thread tried to execute an invalid instruction.
        len += prt(buf+len, "An illegal instruction was executed at address %p.\n",
                    ex->ExceptionAddress);
        break;

    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        // The thread tried to divide an integer value by an integer divisor of zero.
        len += prt(buf+len, "An integer divide by zero was encountered at address %p.\n",
                    ex->ExceptionAddress);
        break;

    case EXCEPTION_INT_OVERFLOW:
        // The result of an integer operation caused a carry out of the most significant bit
        // of the result.
        len += prt(buf+len, "An integer overflow was encountered at address %p.\n",
                    ex->ExceptionAddress);
        break;

    case EXCEPTION_PRIV_INSTRUCTION:
        // The thread tried to execute an instruction whose operation is not allowed in the
        // current machine mode.
        len += prt(buf+len, "A privileged instruction was executed at address %p.\n",
                    ex->ExceptionAddress);
        break;

    case EXCEPTION_STACK_OVERFLOW:
        // The thread used up its stack.
        len += prt(buf+len, "A stack overflow was encountered at address %p.\n",
                    ex->ExceptionAddress);
        break;

    default:
        len += prt(buf+len, "The program encountered exception %u at address %p.\n",
                    (unsigned)ex->ExceptionCode, ex->ExceptionAddress);
        break;
    }
#if defined(__CYGWIN__)
    (void) write(fd, buf, len);
#else
    (void) _write(fd, buf, len);
#endif

#if defined(_M_IX86)
    len = 0;
    if (CONTEXT_INTEGER & context->ContextFlags) {
        len += prt(buf + len,
                "EAX=%p EBX=%p ECX=%p EDX=%p\n"
                "ESI=%p EDI=%p ",
                (void *)context->Eax, (void *)context->Ebx,
                    (void *)context->Ecx, (void *)context->Edx,
                (void *)context->Edi, (void *)context->Esi);
    }
    if (CONTEXT_CONTROL & context->ContextFlags) {
        len += prt(buf + len,
                "EBP=%p ESP=%p\n"
                "EIP=%p EFL=%p CS =%p SS =%p\n",
                (void *)context->Ebp, (void *)context->Esp,
                (void *)context->Eip, (void *)context->EFlags,
                    (void *)context->SegCs, (void *)context->SegSs);
    }
    if (CONTEXT_SEGMENTS & context->ContextFlags) {
        len += prt(buf + len,
                "DS =%p ES =%p FS =%p GS =%p\n",
                (void *)context->SegDs,
                (void *)context->SegEs,
                (void *)context->SegFs,
                (void *)context->SegGs);
    }
#if defined(__CYGWIN__)
    (void) write(fd, buf, len);
#else
    (void) _write(fd, buf, len);
#endif
#endif /*_M_IX86*/

    return EXCEPTION_EXECUTE_HANDLER;
}


#if defined(__CYGWIN__)
static ADDR_T
getframeaddress(const unsigned int level)
{
    ADDR_T pc = 0;

    switch (level) {
    case  0: if (__builtin_frame_address(0))  pc = (ADDR_T)__builtin_return_address(0);  break;
    case  1: if (__builtin_frame_address(1))  pc = (ADDR_T)__builtin_return_address(1);  break;
    case  2: if (__builtin_frame_address(2))  pc = (ADDR_T)__builtin_return_address(2);  break;
    case  3: if (__builtin_frame_address(3))  pc = (ADDR_T)__builtin_return_address(3);  break;
    case  4: if (__builtin_frame_address(4))  pc = (ADDR_T)__builtin_return_address(4);  break;
    case  5: if (__builtin_frame_address(5))  pc = (ADDR_T)__builtin_return_address(5);  break;
    case  6: if (__builtin_frame_address(6))  pc = (ADDR_T)__builtin_return_address(6);  break;
    case  7: if (__builtin_frame_address(7))  pc = (ADDR_T)__builtin_return_address(7);  break;
    case  8: if (__builtin_frame_address(8))  pc = (ADDR_T)__builtin_return_address(8);  break;
    case  9: if (__builtin_frame_address(9))  pc = (ADDR_T)__builtin_return_address(9);  break;
    case 10: if (__builtin_frame_address(10)) pc = (ADDR_T)__builtin_return_address(10); break;
    case 11: if (__builtin_frame_address(11)) pc = (ADDR_T)__builtin_return_address(11); break;
    case 12: if (__builtin_frame_address(12)) pc = (ADDR_T)__builtin_return_address(12); break;
    case 13: if (__builtin_frame_address(13)) pc = (ADDR_T)__builtin_return_address(13); break;
    case 14: if (__builtin_frame_address(14)) pc = (ADDR_T)__builtin_return_address(14); break;
    case 15: if (__builtin_frame_address(15)) pc = (ADDR_T)__builtin_return_address(15); break;
    case 16: if (__builtin_frame_address(16)) pc = (ADDR_T)__builtin_return_address(16); break;
    case 17: if (__builtin_frame_address(17)) pc = (ADDR_T)__builtin_return_address(17); break;
    case 18: if (__builtin_frame_address(18)) pc = (ADDR_T)__builtin_return_address(18); break;
    case 19: if (__builtin_frame_address(19)) pc = (ADDR_T)__builtin_return_address(19); break;
    case 20: if (__builtin_frame_address(20)) pc = (ADDR_T)__builtin_return_address(20); break;
    case 21: if (__builtin_frame_address(21)) pc = (ADDR_T)__builtin_return_address(21); break;
    case 22: if (__builtin_frame_address(22)) pc = (ADDR_T)__builtin_return_address(22); break;
    case 23: if (__builtin_frame_address(23)) pc = (ADDR_T)__builtin_return_address(23); break;
    case 24: if (__builtin_frame_address(24)) pc = (ADDR_T)__builtin_return_address(24); break;
    case 25: if (__builtin_frame_address(25)) pc = (ADDR_T)__builtin_return_address(25); break;
    case 26: if (__builtin_frame_address(26)) pc = (ADDR_T)__builtin_return_address(26); break;
    case 27: if (__builtin_frame_address(27)) pc = (ADDR_T)__builtin_return_address(27); break;
    case 28: if (__builtin_frame_address(28)) pc = (ADDR_T)__builtin_return_address(28); break;
    case 29: if (__builtin_frame_address(29)) pc = (ADDR_T)__builtin_return_address(29); break;
    case 30: if (__builtin_frame_address(30)) pc = (ADDR_T)__builtin_return_address(30); break;
    case 31: if (__builtin_frame_address(31)) pc = (ADDR_T)__builtin_return_address(31); break;
    case 32: if (__builtin_frame_address(32)) pc = (ADDR_T)__builtin_return_address(32); break;
    case 33: if (__builtin_frame_address(33)) pc = (ADDR_T)__builtin_return_address(33); break;
    case 34: if (__builtin_frame_address(34)) pc = (ADDR_T)__builtin_return_address(34); break;
    case 35: if (__builtin_frame_address(35)) pc = (ADDR_T)__builtin_return_address(35); break;
    case 36: if (__builtin_frame_address(36)) pc = (ADDR_T)__builtin_return_address(36); break;
    case 37: if (__builtin_frame_address(37)) pc = (ADDR_T)__builtin_return_address(37); break;
    case 38: if (__builtin_frame_address(38)) pc = (ADDR_T)__builtin_return_address(38); break;
    case 39: if (__builtin_frame_address(39)) pc = (ADDR_T)__builtin_return_address(39); break;
    case 40: if (__builtin_frame_address(40)) pc = (ADDR_T)__builtin_return_address(40); break;
    case 41: if (__builtin_frame_address(41)) pc = (ADDR_T)__builtin_return_address(41); break;
    case 42: if (__builtin_frame_address(42)) pc = (ADDR_T)__builtin_return_address(42); break;
    case 43: if (__builtin_frame_address(43)) pc = (ADDR_T)__builtin_return_address(43); break;
    case 44: if (__builtin_frame_address(44)) pc = (ADDR_T)__builtin_return_address(44); break;
    case 45: if (__builtin_frame_address(45)) pc = (ADDR_T)__builtin_return_address(45); break;
    case 46: if (__builtin_frame_address(46)) pc = (ADDR_T)__builtin_return_address(46); break;
    case 47: if (__builtin_frame_address(47)) pc = (ADDR_T)__builtin_return_address(47); break;
    case 48: if (__builtin_frame_address(48)) pc = (ADDR_T)__builtin_return_address(48); break;
    case 49: if (__builtin_frame_address(49)) pc = (ADDR_T)__builtin_return_address(49); break;
    }
    return pc;
}
#endif /*__CYGWIN__*/


void
edbt_init(const char *progname, int options, FILE *out)
{
    static unsigned once = 0;

    __CUNUSED(progname)
    __CUNUSED(options)

    ED_TRACE(("edbt_init:%lx\n", (unsigned long)hDbghelpDll))

    // dynamically bind
    if (! hDbghelpDll) {
        HANDLE hProcess;

        if (++once > 1) return;

#if defined(__WATCOMC__) || defined(_MSC_VER) || \
            defined(__CYGWIN__)
        map_load(progname);
#endif

        if (NULL == (hDbghelpDll = LoadLibraryA("dbghelp.dll"))) {
            if (out) fprintf(out, "\n\nError: Cannot load 'dbghelp.dll',\n stack trace cannot be performed\n");
            return;
        }

        if (NULL == (hKernel32Dll = LoadLibraryA("kernel32.dll"))) {
            if (out) fprintf(out, "\n\nError: Cannot load 'kernel32.dll',\n stack trace cannot be performed\n");
            return;
        }

        fSymInitialize = (SymInitialize_t) GetProcAddress(hDbghelpDll, "SymInitialize");
        fSymSetOptions = (SymSetOptions_t) GetProcAddress(hDbghelpDll, "SymSetOptions");
        fSymLoadModule = (SymLoadModule_t) GetProcAddress(hDbghelpDll, "SymLoadModule");
        fUnDecorateSymbolName = (UnDecorateSymbolName_t) GetProcAddress(hDbghelpDll, "UnDecorateSymbolName");

        fCaptureStackBackTrace = (CaptureStackBackTrace_t) GetProcAddress(hKernel32Dll, "CaptureStackBackTrace");
        if (NULL == fCaptureStackBackTrace) {   // Vista+
            fCaptureStackBackTrace = (CaptureStackBackTrace_t) GetProcAddress(hKernel32Dll, "RtlCaptureStackBackTrace");
        }

#if defined(EDT_64BIT)
        fStackWalk = (StackWalk_t) GetProcAddress(hDbghelpDll, "StackWalk64");
        fSymFunctionTableAccess = (SymFunctionTableAccess_t) GetProcAddress(hDbghelpDll, "SymFunctionTableAccess64");
        fSymGetModuleBase = (SymGetModuleBase_t) GetProcAddress(hDbghelpDll, "SymGetModuleBase64");
        fSymGetSymFromAddr = (SymGetSymFromAddr_t) GetProcAddress(hDbghelpDll, "SymGetSymFromAddr64");
        fSymGetLineFromAddr = (SymGetLineFromAddr_t) GetProcAddress(hDbghelpDll, "SymGetLineFromAddr64");
#else
        fStackWalk = (StackWalk_t) GetProcAddress(hDbghelpDll, "StackWalk");
        fSymFunctionTableAccess = (SymFunctionTableAccess_t) GetProcAddress(hDbghelpDll, "SymFunctionTableAccess");
        fSymGetModuleBase = (SymGetModuleBase_t) GetProcAddress(hDbghelpDll, "SymGetModuleBase");
        fSymGetSymFromAddr = (SymGetSymFromAddr_t) GetProcAddress(hDbghelpDll, "SymGetSymFromAddr");
        fSymGetLineFromAddr = (SymGetLineFromAddr_t) GetProcAddress(hDbghelpDll, "SymGetLineFromAddr");
#endif

        if ((fSymInitialize == NULL) || 
                (fCaptureStackBackTrace == NULL) ||
                (fStackWalk == NULL) || 
                (fSymGetModuleBase == NULL) || (fSymGetSymFromAddr == NULL) || 
                (fSymGetLineFromAddr == NULL) || (fSymLoadModule == NULL)) {
            if (out) {
                fprintf(out, "Error: required function not found in dbghelp.dll and/or kernel32.dll\n" );
                if (NULL == fSymInitialize        ) fprintf(out, " missing SymInitialize\n");
                if (NULL == fCaptureStackBackTrace) fprintf(out, " missing CaptureStackBackTrace\n");
                if (NULL == fStackWalk            ) fprintf(out, " missing StackWalk\n");
                if (NULL == fSymGetModuleBase     ) fprintf(out, " missing SymGetModuleBase\n");
                if (NULL == fSymGetSymFromAddr    ) fprintf(out, " missing SymGetSymFromAddr\n");
                if (NULL == fSymGetLineFromAddr   ) fprintf(out, " missing SymGetLineFromAddr\n");
                if (NULL == fSymLoadModule        ) fprintf(out, " missing SymLoadModule\n");
            }
            FreeLibrary(hDbghelpDll);
            hDbghelpDll = 0;
            FreeLibrary(hKernel32Dll);
            hKernel32Dll = 0;
            return;
        }

        hProcess = GetCurrentProcess();
        fSymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);
        if (! fSymInitialize(hProcess, NULL, TRUE)) {
            if (out) fprintf(out, "Warning: debugging symbols cannot be initialized %s\n", LastError());
        }

#if (0)     //basic stacktrace
        if (out) {
            void *stack[ 64 ] = {0};
            SYMBOL_INFO *symbol;
        //  HANDLE hProcess;
            int frames, i;

        //  hProcess = GetCurrentProcess();
        //  SymInitialize(process, NULL, TRUE);
            frames = (int)CaptureStackBackTrace(0, 63 /*MAX XP*/, stack, NULL);
            symbol = (SYMBOL_INFO *) calloc(sizeof( SYMBOL_INFO ) + 256 * sizeof( char ), 1 );
            symbol->MaxNameLen = 255;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

            fprintf(out, "StackTrace\n");
            for (i = frames - 1; i >= 0; --i) {
                SymFromAddr(hProcess, (DWORD64)(stack[i]), 0, symbol);
                fprintf(out, " %i: %s - 0x%0X\n", frames - i - 1, symbol->Name, symbol->Address );
            }
            free(symbol);
        }
#endif
    }
}


void
edbt_auto(void)
{
    x_org_filter = SetUnhandledExceptionFilter(__edbt_exceptionfilter);
}


void
edbt_stackdump(FILE *out, int level)
{
#define MAX_INSTRUCTION_LENGTH 100
#define MAX_FUNCTION_PROLOG 64

    int count = 0, depth = 0;
    HANDLE hProcess = GetCurrentProcess();
#if defined(_MSC_VER)
    HANDLE hThread  = GetCurrentThread();
#endif
    CONTEXT context = {0};
#if defined(EDBT_64BIT)
    STACKFRAME64 sf = {0};
#else
    STACKFRAME sf = {0};
#endif

    edbt_init(NULL, 0, out);
    if (level < 0) level = 0;

    //  Decode stackframes
    //
#if defined(__CYGWIN__)
    {
        //  Walk stack frames ...
        //
        ADDR_T pc = (ADDR_T)__builtin_return_address(0);
        int rdepth = 0;

        while (pc >= 0x400000) {
            if (count++ >= (unsigned)level) {
                if (depth >= MAX_FRAMES) {
                    break;
                }
                frames[depth++] = pc;
            }
            pc = getframeaddress(++rdepth + 1);
        }
    }

#else /*!__CYGWIN__*/
        //  win32 - get current context of processor registers
        //
        //      Prior to XP there is no reliable way for obtaining of the actual context
        //      for the current thread which is portable on the different versions of Windows
        //
        //      Instruction pointer cannot be captured reliably but we can use the address
        //      of function increased so that the pointer refers somewhere into the code of
        //      function generated by source codes and not to code of function prolog
        //      generated by compiler. The length of the function prolog is defined by
        //      MAX_FUNCTION_PROLOG.
        //
#if (_WIN32_WINNT >= 0x501)
        RtlCaptureContext(&context);
#else
        do {
#if defined(_X86_)
            DWORD rebp = 0, resp = 0;

#if defined(__GNUC__)           /*MINGW32*/
	        asm ("movl %%ebp, %0" : "=r" (rebp) );
	        asm ("movl %%esp, %0" : "=r" (resp) );
#else
            __asm {
                mov rebp, ebp
                mov resp, esp
            };
#endif

            context.Ebp = rebp;
            context.Esp = resp;
            context.Eip = (unsigned int)(&edbt_stackdump) + MAX_FUNCTION_PROLOG;
#elif defined(_IA64_) || defined(_AMD64_)
            DWORD64 rebp, resp;
            __asm {
                mov rebp, rbp
                mov resp, rsp
                };
            context.Rbp = rebp;
            context.Rsp = resp;
            context.Rip = (unsigned int)(&edbt_stackdump) + MAX_FUNCTION_PROLOG;
#endif
            context.ContextFlags = CONTEXT_FULL;
        } while(0);
#endif

        // Initialize the STACKFRAME structure for the first call.
        // This is only necessary for Intel CPUs, and isn't mentioned in the documentation.
        sf.AddrPC.Mode      = AddrModeFlat;
        sf.AddrStack.Mode   = AddrModeFlat;
        sf.AddrFrame.Mode   = AddrModeFlat;

#if defined(_X86_)
        sf.AddrPC.Offset    = context.Eip;
        sf.AddrStack.Offset = context.Esp;
        sf.AddrFrame.Offset = context.Ebp;
#elif defined(_IA64_) || defined(_AMD64_)
        sf.AddrPC.Offset    = context.Rip;
        sf.AddrStack.Offset = context.Rsp;
        sf.AddrFrame.Offset = context.Rbp;
#endif

        // Walk callstack
        {
#if defined(_MSC_VER)
#if defined(_X86_)
            const DWORD machine_type = IMAGE_FILE_MACHINE_I386;
#elif defined(_IA64_) || defined(_AMD64_)
            const DWORD machine_type = IMAGE_FILE_MACHINE_AMD64;
#endif

            do {
                if (count++ >= level) {
                    if (depth < MAX_FRAMES) {
                        frames[depth++] = sf.AddrPC.Offset;
                    }
                }
            } while (fStackWalk(machine_type, hProcess, hThread, &sf, &context, NULL,
                        fSymFunctionTableAccess, fSymGetModuleBase, NULL) && sf.AddrFrame.Offset);

#else /*!_MSC_VER*/
            ADDR_T reip = 0;

            while (sf.AddrFrame.Offset && (reip = sf.AddrPC.Offset) >= 0x400000) {
                if (count++ >= level) {
                    if (depth < MAX_FRAMES) {
                        frames[depth++] = reip;
                    }
                }

                // return point is stored before ebp address, the content of EBP address
                // is base pointer for the upper stack level. the stack pointer address ESP
                // can be set to arbitrary value above the base pointer EBP.
#if defined(_X86_)
                sf.AddrPC.Offset    = *((DWORD *)(sf.AddrFrame.Offset + sizeof(void *)));
                sf.AddrFrame.Offset = *((DWORD *)(sf.AddrFrame.Offset));
#elif defined(_IA64_) || defined(_AMD64_)
                sf.AddrPC.Offset    = *((DWORD64 *)(sf.AddrFrame.Offset + sizeof(void *)));
                sf.AddrFrame.Offset = *((DWORD64 *)(sf.AddrFrame.Offset));
#endif
                sf.AddrStack.Offset = sf.AddrFrame.Offset - sizeof(void *);

                                                // loop/corrupt
                if ((sf.AddrPC.Offset == sf.AddrReturn.Offset) || (reip == sf.AddrPC.Offset)) {
                    break;
                }
            }
#endif
        }
#endif  /*__CYGWIN__*/

    // Dump results
    //
    for (count = 0; count < depth; ++count) {
        //  IMAGEHLP is wacky, and requires you to pass in a pointer to an IMAGEHLP_SYMBOL structure.
        //  The problem is that this structure is variable length; that is, you determine how big the
        //  structure is at runtime. This means that you can't use sizeof(struct), instead make a
        //  buffer that's big enough, and make a pointer to the buffer.
        //
        //  We also need to initialize not one, but TWO members of the structure before it can be used.
        //
        const ADDR_T frame = frames[count];     // frame address
        const char *filename = NULL, *funcname = NULL;

#if defined(EDBT_64BIT)
        BYTE symbol_buffer[sizeof(IMAGEHLP_SYMBOL64) + MAX_SYMBOL];
        PIMAGEHLP_SYMBOL64 pSymbol = (PIMAGEHLP_SYMBOL64) symbol_buffer;
        IMAGEHLP_LINE64 line = {0};
        DWORD64 displacement = 0;
#else
        BYTE symbol_buffer[sizeof(IMAGEHLP_SYMBOL) + MAX_SYMBOL];
        PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL) symbol_buffer;
        IMAGEHLP_LINE line = {0};
        DWORD displacement = 0;
#endif
        DWORD offset = 0;

        memset(symbol_buffer, 0, sizeof(symbol_buffer));
        pSymbol->SizeOfStruct = sizeof(symbol_buffer);
        pSymbol->MaxNameLength = MAX_SYMBOL;

        line.SizeOfStruct = sizeof(line);

        // symbol name and line number info
        if (NULL == fSymGetSymFromAddr ||
                FALSE == fSymGetSymFromAddr(hProcess, frame, &displacement, pSymbol)) {
#if defined(__WATCOMC__) || defined(_MSC_VER) || \
        defined(__CYGWIN__)
            if (map_lookup((void *)frame, pSymbol, &line)) {
                filename = remove_path(line.FileName, 2);
            } else
#endif
            {
                fprintf(out, "\nCannot resolve symbols %s\n", LastError());
            }
        } else if (fSymGetLineFromAddr) {
            //  SymGetLineFromAddr was loaded
            //
            //  Watcom
            //      Supports only CodeView format version 4 which/
            //      a)  Is stored in the separated file progname.dbg, where progname is the name of the
            //          executed program.
            //
            //          The 'dbg' file can be created by rebase.exe utility from the Windows SDK (1999)
            //          which extracts the debug information stored within the exe file.
            //
            //      b)  Embedded within the application.
            //
            //      In the case of CV4 format, the line numbers and source file names can be detected
            //      only for the instruction address of the line beginning and not for arbitrary
            //      addresses between the beginning of the call instruction and the return point.
            //
            //      Thus we have to test addresses starting one byte before return point until the
            //      line beginning of the function call is captured.
            //
            //  Visual Studio
            //      Utilises pdb symbol database which accepts arbitrary instruction address in the range
            //      belonging to the call instruction of the given function. In such the case, the loop
            //      finishes after the first pass.
            //
            unsigned range = 1;                 // starting range from the address of return point

            while ((fSymGetLineFromAddr(hProcess, frame - range, &offset, &line) == FALSE) &&
                        (range < MAX_INSTRUCTION_LENGTH)) {
                ++range;                        // increase range
            }
                                                // filename, omit leading directories
            if (fSymGetLineFromAddr(hProcess, frame - range, &offset, &line) != FALSE) {
                filename = remove_path(line.FileName, 2);
            }
        }

        // symbol information
        funcname = pSymbol->Name;
#if defined(__WATCOMC__)
        funcname = watcom_demangle(funcname);
#endif
        if (0 == count) {
            fprintf(out, "\nCall trace:\n");
        } else {
            fprintf(out, "  called by ");
        }

        if (funcname && *funcname) {
            fprintf(out, "%s(+0x%lx) - %p",
                funcname, (unsigned long)(frame - pSymbol->Address), (void *)frame);
        } else {
            fprintf(out, "address %p", (void *)frame);
        }

        if (filename && *filename) {
            if (line.LineNumber > 0) {
                fprintf(out, " [%s, line %u]", filename, (unsigned)line.LineNumber);
            } else {
                fprintf(out, " [%s]", filename);
            }
        }
        fprintf(out, "\n");

        // terminate at main(...)
        if (funcname) {
            if ('m' == *funcname) {
                if (0 == strcmp(funcname, "main") || 0 == strcmp(funcname, "main_")) {
                    break;
                }
            } else if ('_' == *funcname && 'm' == funcname[1]) {
                if (0 == strcmp(funcname, "_main")) {
                    break;
                }
            }
        }
    }
    fprintf(out, "\n");
    fflush(out);
}


int
edbt_symbol(void *address, char *buffer, int buflen)
{
    HANDLE hProcess = GetCurrentProcess();
#if defined(EDBT_64BIT)
    BYTE symbol_buffer[sizeof(IMAGEHLP_SYMBOL64) + MAX_SYMBOL];
    PIMAGEHLP_SYMBOL64 pSymbol = (PIMAGEHLP_SYMBOL64) symbol_buffer;
    DWORD64 displacement = 0;
#else
    BYTE symbol_buffer[sizeof(IMAGEHLP_SYMBOL) + MAX_SYMBOL];
    PIMAGEHLP_SYMBOL pSymbol = (PIMAGEHLP_SYMBOL) symbol_buffer;
    DWORD displacement = 0;
#endif
    const char *symbol;
    int symlen;

    edbt_init(NULL, 0, NULL);

    memset(symbol_buffer, 0, sizeof(symbol_buffer));
    pSymbol->SizeOfStruct = sizeof(symbol_buffer);
    pSymbol->MaxNameLength = MAX_SYMBOL;

    if (NULL == fSymGetSymFromAddr ||
            FALSE == fSymGetSymFromAddr(hProcess, (ADDR_T)address, &displacement, pSymbol)) {
#if defined(__WATCOMC__) || defined(_MSC_VER) || \
            defined(__CYGWIN__)
        map_lookup(address, pSymbol, NULL);
#endif
    }

    if (NULL == (symbol = pSymbol->Name)) {
        return -1;
    }

#if defined(__WATCOMC__)
    symbol = watcom_demangle(symbol);
#endif
    strxcpy(buffer, symbol, buflen);
    if ((symlen = (int)strlen(symbol)) + 16 < buflen) {
        symlen += sxprintf(buffer + symlen, buflen - symlen, "(+0x%lx)",
                        (unsigned long)((ADDR_T)address - pSymbol->Address));
    }
    return symlen;
}


static const char *
LastError(void)
{
    static char errbuf[180];
    char errtxt[128];
    DWORD rc = GetLastError();
    int len;

    len = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, rc, 0, errtxt, sizeof(errtxt), NULL);
    if (len > 2) errtxt[len - 2] = 0;           // remove trailing \r\n
#if defined(_MSC_VER)
    _snprintf(errbuf, sizeof(errbuf), ": %u (%s)", (unsigned)rc, errtxt);
#else
    snprintf(errbuf, sizeof(errbuf), ": %u (%s)", (unsigned)rc, errtxt);
#endif
    errbuf[sizeof(errbuf) - 1] = 0;
    return errbuf;
}
#endif  /*WIN32*/

/*quiet archivers*/
extern int edbt_win32(void);
int
edbt_win32(void)
{
    return 0;
}
/*end*/
