#include <edidentifier.h>
__CIDENT_RCSID(gr_chkalloc_c,"$Id: chkalloc.c,v 1.31 2020/06/05 17:37:37 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: chkalloc.c,v 1.31 2020/06/05 17:37:37 cvsuser Exp $
 * Memory allocation front end.
 *
 *
 *
 * Copyright (c) 1998 - 2020, Adam Young.
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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <chkalloc.h>
#if defined(_MSC_VER)
#include <malloc.h>                             /* _expand */
#include <crtdbg.h>
#endif
#include <edtrace.h>

#if defined(HAVE_MALLOC_STATS)
#if !defined(__CYGWIN__) /* cygwin, struct malinfo redefinition/dlmalloc */
#include <malloc.h>                             /* malloc_stats */
#if defined(__linux__) || defined(__linux) /*redef warnings*/
#undef M_TRIM_THRESHOLD
#undef M_MMAP_THRESHOLD
#endif
#else
extern void malloc_stats (void);
#endif
#endif/*HAVE_MALLOC_STATS*/

#if !defined(HAVE_LIBDLMALLOC) && \
        (defined(_MSC_VER) || defined(__WATCOMC__))
#define HAVE_LIBDLMALLOC                        /* implied */
#endif

#if defined(HAVE_LIBDLMALLOC)
#ifndef USE_DL_PREFIX
#define USE_DL_PREFIX                           /* dlxxxx assumed */
#endif
#include "../libmalloc/dlmalloc.h"

#define MALLOC(_sz)         dlmalloc(_sz)
#define CALLOC(_ne,_sz)     dlcalloc(_ne,_sz)
#define REALLOC(_bf,_sz)    dlrealloc(_bf,_sz)
#define FREE(_bf)           dlfree(_bf)
#define HAVE_NATIVE_ALLOC

#elif defined(_MSC_VER) && (0)
#define MALLOC(_sz)         _malloc(_sz)
#define CALLOC(_ne,_sz)     _calloc(_ne,_sz)
#define REALLOC(_bf,_sz)    _realloc(_bf,_sz)
#define FREE(_bf)           _free(_bf)

#else
#define MALLOC(_sz)         malloc(_sz)
#define CALLOC(_ne,_sz)     calloc(_ne,_sz)
#define REALLOC(_bf,_sz)    realloc(_bf,_sz)
#define FREE(_bf)           free(_bf)
#endif

/*
 *  check_xxx() interface
 *
 *  Block structure
 *
 *      <header>
 *          magic           MAGIC_ALLOCED or MAGIC_ALLOCED_WHERE.
 *          length
 *      <user-arena>
 *          0 .. length in size
 *      <tail>
 *          magic           MAGIC_TAIL
 *          [filename:linenumber]
 */

#include <tailqueue.h>

#ifndef MKMAGIC
#define MKMAGIC(a,b,c,d)    (((unsigned long)(a))<<24|((unsigned long)(b))<<16|(c)<<8|(d))
#endif

#define MAGIC_ALLOCED       MKMAGIC('M','b','a','L')
#define MAGIC_ALLOCED_WHERE MKMAGIC('M','b','a','W')
#define MAGIC_TAIL          MKMAGIC('M','b','t','L')
#define MAGIC_FREED         MKMAGIC('m','B','f','R')

typedef TAILQ_HEAD(_MemList, _MemBlock)
                            MEMLIST_t;
static MEMLIST_t            x_memtail;
static unsigned             x_memflags;
            // = CHKALLOC_TAIL | CHKALLOC_FILL | CHKALLOC_ZERO | CHKALLOC_WHERE | CHKALLOC_UNINIT;
static unsigned             x_totalallocs = 0;
static unsigned             x_allocs = 0;
static uint32_t             x_tail = MAGIC_TAIL;

struct _MemBlock {
    union {
        struct _MemInternal {
            uint32_t        magic;
            size_t          length;
            TAILQ_ENTRY(_MemBlock) node;
        } block;
#define ALIGNMENT   16
        char                alignment[ALIGNMENT];
            /*  Assure if allocation succeeds, 
             *  we return a pointer that is suitably aligned for any object type with fundamental alignment.
             */
    };
};

static int                  x_native;
static unsigned             x_inuse;
static unsigned             x_leak;


static void
checkfailed(const char *msg, const char *filename, unsigned line)
{
    fprintf(stderr, "%s(%u): %s\r\n", (filename ? filename : "?"), line, msg);
    fflush(stderr);
    abort();
}


/*
 *  Walk down the allocated blocks, checking for buffer corruption.
 */
static void
checktail(const char *filename, unsigned line)
{
    if (x_totalallocs) {
        const MEMLIST_t *tq = &x_memtail;
        const struct _MemBlock *mp;
        size_t size;

        TAILQ_FOREACH(mp, tq, block.node) {
            const struct _MemInternal *ip = &mp->block;

            if (MAGIC_ALLOCED != ip->magic) {
                if (MAGIC_FREED == ip->magic) {
                    checkfailed("check(), block already freed memory.", filename, line);
                } else if (MAGIC_ALLOCED_WHERE != ip->magic) {
                    checkfailed("check(), non-alloced memory.", filename, line);
                }
            }

            if (0 == (size = ip->length)) {
                checkfailed("check(), overwritten block length.", filename, line);
            }

            if (0 != memcmp((const char *)&x_tail, ((const char *)(ip + 1)) + size, sizeof(x_tail))) {
                checkfailed("check(), overwritten arena magic.", filename, line);
            }
        }
    }
}


static void *
checkalloc(size_t size, const char *filename, unsigned line)
{
    void *result = NULL;
    size_t fnsize = 0;

    if (0 == size) {
        return NULL;
    }

    if (CHKALLOC_TAIL & x_memflags) {
        checktail(filename, line);
    }

    if ((CHKALLOC_WHERE & x_memflags) && filename && *filename) {
        fnsize = strlen(filename) + 15 /*line*/;
        fnsize = ALIGNU64(fnsize);
    }

    /*
     *  Structure:
     *    < Block >
     *    < ... memory of length bytes ... >
     *    < Tail >
     *    [ filename:line\0 ]
     */
    if (NULL != (result =
            MALLOC(sizeof(struct _MemBlock) + size + sizeof(x_tail) + fnsize))) {
        struct _MemBlock *mp = (struct _MemBlock *) result;
        struct _MemInternal *ip = &mp->block;
        char *end;

        if (0 == x_totalallocs++) {
            assert(sizeof(struct _MemBlock) == ALIGNMENT);
            TAILQ_INIT(&x_memtail);
        }
        ++x_allocs;

        ip->magic = MAGIC_ALLOCED;
        ip->length = size;

        result = (void *)(mp + 1);              /* trailing data block */
        end = ((char *)result) + size;

        TAILQ_INSERT_TAIL(&x_memtail, mp, block.node);

        memcpy(end, (const char *)&x_tail, sizeof(x_tail));

        if (CHKALLOC_WHERE & x_memflags) {
            (void) sprintf(end + sizeof(x_tail), "%s:%u", filename, line);
            ip->magic = MAGIC_ALLOCED_WHERE;
        }
    }
    return result;
}


unsigned
check_configure(unsigned flags)
{
    const unsigned oflags = x_memflags;

    if ((unsigned)-1 != flags) x_memflags |= flags;
    return oflags;
}


void *
check_alloc(size_t size, const char *filename, unsigned line)
{
    void *result = NULL;

    if (size) {
        if (NULL != (result = checkalloc(size, filename, line))) {
            if (CHKALLOC_FILL & x_memflags) {
                memset(result, 'A', size);

            } else if (CHKALLOC_ZERO & x_memflags) {
                memset(result, 0, size);
            }
        }
    }
    assert(ISALIGNTO(result, ALIGNMENT));
    return result;
}


void *
check_realloc(void *p, size_t size, const char *filename, unsigned line)
{
    void *result = NULL;

    if (! p) {
        result = check_alloc(size, filename, line);

    } else {
        struct _MemBlock *mp = ((struct _MemBlock *) p) - 1;
        struct _MemInternal *ip = &mp->block;
        size_t oldsize, fnsize = 0;
        const char *oldend;
        char *end;

        if (MAGIC_ALLOCED != ip->magic) {
            if (MAGIC_FREED == ip->magic) {
                checkfailed("realloc(), block already freed memory.", filename, line);
            } else if (MAGIC_ALLOCED_WHERE != ip->magic) {
                checkfailed("realloc(), non-alloced memory.", filename, line);
            }
        }

        if (0 == (oldsize = ip->length)) {
            checkfailed("realloc(), overwritten block length.", filename, line);
        }

        oldend = ((char *)p) + oldsize;

        if (0 != memcmp((const char *)&x_tail, oldend, sizeof(x_tail))) {
            checkfailed("realloc(), overwritten arena magic.", filename, line);
        }

        if (size <= oldsize) {
            return p;                           /* dont shrink, return original */
        }

        TAILQ_REMOVE(&x_memtail, mp, block.node);

        if (CHKALLOC_TAIL & x_memflags) {
            checktail(filename, line);
        }

        if (MAGIC_ALLOCED_WHERE == ip->magic) {
            const char *fileinfo = oldend + sizeof(x_tail);
            fnsize = strlen(fileinfo) + 1;      /* trailing file information */
        }

        if (NULL == (result =
                REALLOC((void *)mp, sizeof(struct _MemBlock) + size + sizeof(x_tail) + ALIGNU64(fnsize)))) {
            TAILQ_INSERT_TAIL(&x_memtail, mp, block.node);

        } else {
            mp = (struct _MemBlock *) result;
            ip = &mp->block;

            ip->magic = (fnsize ? MAGIC_ALLOCED_WHERE : MAGIC_ALLOCED);
            ip->length = size;
            result = (void *)(mp + 1);
            end = ((char *)result) + size;
            oldend = ((const char *)result + oldsize);

            TAILQ_INSERT_TAIL(&x_memtail, mp, block.node);

            assert(0 == memcmp((const char *)&x_tail, oldend, sizeof(x_tail)));
            memcpy(end, (const char *)&x_tail, sizeof(x_tail));
            if (fnsize) {                       /* fileinfo */
                memmove(end + sizeof(x_tail), oldend + sizeof(x_tail), fnsize);
            }
        }
    }
    return result;
}


void *
check_recalloc(void *p, size_t osize, size_t nsize, const char *filename, unsigned line)
{
    assert(nsize > osize);                      /* assume expand */
    if (NULL == p) {
        p = check_calloc(1, nsize, filename, line);
    } else if (NULL != (p = check_realloc(p, nsize, filename, line))) {
        if (nsize > osize) {                    /* zero new storage */
            (void) memset((char *)p + osize, 0, nsize - osize);
        }
    }
    return p;
}


size_t
check_expand(void *p, size_t size, const char *filename, unsigned line)
{
    __CUNUSED(p);
    __CUNUSED(size);
    __CUNUSED(filename);
    __CUNUSED(line);
    return 0;
}


size_t
check_shrink(void *p, size_t size, const char *filename, unsigned line)
{
    __CUNUSED(p);
    __CUNUSED(size);
    __CUNUSED(filename);
    __CUNUSED(line);
    return 0;
}


void
check_free(void *p, const char * filename, unsigned line)
{
    if (p) {
        struct _MemBlock *mp = ((struct _MemBlock *) p) - 1;
        struct _MemInternal *ip = &mp->block;
        size_t length;  

        if (MAGIC_ALLOCED != ip->magic) {
            if (MAGIC_FREED == ip->magic) {
                checkfailed("free(), block already freed memory.", filename, line);
            } else if (MAGIC_ALLOCED_WHERE != ip->magic) {
                checkfailed("free(), non-alloced memory.", filename, line);
            }
        }

        if (0 == (length = ip->length)) {
            checkfailed("free(), overwritten block length.", filename, line);
        }

        if (0 != memcmp((const char *)&x_tail, (const char *)p + length, sizeof(x_tail))) {
            checkfailed("free(), overwritten arena magic.", filename, line);
        }

        TAILQ_REMOVE(&x_memtail, mp, block.node);

        if (CHKALLOC_TAIL & x_memflags) {
            checktail(filename, line);
        }

        if (CHKALLOC_UNINIT & x_memflags) {
            size_t iplength = (sizeof(struct _MemBlock) + length) / sizeof(uint32_t);
            uint32_t *ip2 = (uint32_t *)ip;
            while (iplength--) {
                *++ip2 = 0xDEADBEEF;
            }
        }
        ip->magic = MAGIC_FREED;
        --x_allocs;

        FREE(mp);
    }
}


void *
check_calloc(size_t elem, size_t elsize, const char *filename, unsigned line)
{
    const size_t size = elem * elsize;
    void *result = NULL;

    if (size) {
        if (NULL != (result = checkalloc(size, filename, line))) {
            memset(result, 0, size);
        }
    }
    return result;
}


void *
check_salloc(const char *s, const char *filename, unsigned line)
{
    void *result = NULL;

    if (s) {
        const size_t size = strlen(s) + 1 /*nul*/;
        if (NULL != (result = checkalloc(size, filename, line))) {
            memcpy(result, s, size);
        }
    }
    return result;
}


void *
check_snalloc(const char *s, size_t len, const char *filename, unsigned line)
{
    void *result = NULL;

    if (s) {
        if (NULL != (result = checkalloc(len + 1 /*nul*/, filename, line))) {
            memcpy(result, s, len);
            ((char *)result)[len] = 0;
        }
    }
    return result;
}


void
check_leak(const void *p, const char *file, unsigned line)
{
    __CUNUSED(p);
    __CUNUSED(file);
    __CUNUSED(line);
}


/*
 *  MSVC support
 */

#if defined(_MSC_VER)

static int __cdecl
mscv_allochook(int nType, void * pvData, size_t nSize, int nBlockUse,
        long lRequest, const unsigned char * szFile, int nLine)
{
    static const char *operation[] = {"", "allocating", "re-allocating", "freeing"};
    static const char *blockType[] = {"Free", "Normal", "CRT", "Ignore", "Client"};

    if (nBlockUse == _CRT_BLOCK) {              // ignore internal C runtime library allocations
        return 1;
    }

    _ASSERT((nType > 0) && (nType < 4));
    _ASSERT((nBlockUse >= 0) && (nBlockUse < 5));

    if (szFile && szFile[0]) {
        if (pvData != NULL) {
            trace_log("Memory operation in %s, line %d: %s a %d-byte '%s' block (#%ld) at %X\n",
                szFile, nLine, operation[nType], nSize, blockType[nBlockUse], lRequest, pvData);

        } else {
            trace_log("Memory operation in %s, line %d: %s a %d-byte '%s' block (#%ld)\n",
                szFile, nLine, operation[nType], nSize, blockType[nBlockUse], lRequest);
        }
    } else {
        if (pvData != NULL) {
            trace_log("Memory operation %s a %d-byte '%s' block (#%ld) at %X\n",
                operation[nType], nSize, blockType[nBlockUse], lRequest, pvData);

        } else {
            trace_log("Memory operation %s a %d-byte '%s' block (#%ld)\n",
                operation[nType], nSize, blockType[nBlockUse], lRequest);
        }
    }
    return 7;                                   // allow the memory operation to proceed
}


static int
mscv_reporthook(int nRptType, char *szMsg, int *retVal)
{
    const char *RptTypes[] = { "Warning", "Error", "Assert" };

    __CUNUSED(retVal)
    if (nRptType > 0 || strstr(szMsg, "DAMAGE")) {
        fprintf(stderr, "%s: %s", RptTypes[nRptType], szMsg);
    } else {
        trace_log("\t%s", szMsg);
    }
    return 7;                                   // allow the report to be made as usual
}


static void
mscv_diagnositics(void)
{
#if !defined(_DEBUG) || (1) /*1=disable*/
    printf("Skipping this for non-debug mode.\n");

#else
#define CRT_DUMP_MEMORY_LEAKS
//  _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) |_CRTDBG_CHECK_ALWAYS_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
//  _CrtSetAllocHook(mscv_allochook);
    _CrtSetReportHook(mscv_reporthook);
#endif
}


static void
mscv_dump(void)
{
#if defined(CRT_DUMP_MEMORY_LEAKS)
    _CrtDumpMemoryLeaks();
#endif
    trace_flush();
}
#endif  /*_MSC_VER*/


/*
 *  chk_xxx() interface
 */

#undef chk_alloc
#undef chk_calloc
#undef chk_realloc
#undef chk_recalloc
#undef chk_shrink
#undef chk_expand
#undef chk_free
#undef chk_salloc
#undef chk_snalloc
#undef chk_leak
#undef chk_isleak


/*
 *  Enable use of system native allocator, if not already the default.
 */
void
chk_native(void)
{
    x_native = 1;
}


/*
 *  Enable native run-time check, if availble
 */
void
chk_rtc(void)
{
    x_native = 2;

#if defined(_MSC_VER)
    mscv_diagnositics();
#elif defined(HAVE_MALLOPT)
#if defined(M_CHECK_ACTION)
    mallopt(M_CHECK_ACTION, 3);                 /* check and abort() */
#endif
#endif
}


void
chk_stats(void)
{
    printf("in use blocks    = %10u\n", x_inuse);
    printf("possible leaks   = %10u\n", (x_leak < x_inuse ? x_inuse - x_leak : (unsigned)-1));

#if defined(HAVE_NATIVE_ALLOC)
    if (x_native) {
#if defined(_MSC_VER)
        mscv_dump();
#elif defined(HAVE_MALLOC_STATS)
        malloc_stats();
#endif
        return;
    }
#endif /*HAVE_NATIVE_ALLOC*/

#if defined(HAVE_LIBDLMALLOC)
    dlmalloc_stats();
#elif defined(_MSC_VER)
    mscv_dump();
#endif
}


void *
chk_alloc(size_t size)
{
    void *result;

#if defined(HAVE_NATIVE_ALLOC)
    if (x_native) {
        if (NULL != (result = malloc(size))) {
            ++x_inuse;
        }
        return result;
    }
#endif /*HAVE_NATIVE_ALLOC*/

    if (NULL != (result = MALLOC(size))) {
        if (CHKALLOC_FILL & x_memflags) {
            memset(result, 'A', size);

        } else if (CHKALLOC_ZERO & x_memflags) {
            memset(result, 0, size);
        }
    }
    return result;
}


void *
chk_calloc(size_t elem, size_t elsize)
{
    void *result;

#if defined(HAVE_NATIVE_ALLOC)
    if (x_native) {
        if (NULL != (result = calloc(elem, elsize))) {
            ++x_inuse;
        }
        return result;
    }
#endif /*HAVE_NATIVE_ALLOC*/

    if (NULL != (result = CALLOC(elem, elsize)))
        ++x_inuse;
    return result;
}


void *
chk_realloc(void *p, size_t size)
{
    if (NULL == p) return chk_alloc(size);

#if defined(HAVE_NATIVE_ALLOC)
    if (x_native) {
        return realloc(p, size);
    }
#endif /*HAVE_NATIVE_ALLOC*/

    return REALLOC(p, size);
}


void *
chk_recalloc(void *p, size_t osize, size_t nsize)
{
    assert(nsize > osize);                      /* assume expand */
    if (NULL == p) {
        p = chk_calloc(1, nsize);
    } else if (NULL != (p = chk_realloc(p, nsize))) {
        if (nsize > osize) {                    /* zero new storage */
            (void) memset((char *)p + osize, 0, nsize - osize);
        }
    }
    return p;
}


size_t
chk_shrink(void *p, size_t size)
{
    if (NULL == p) return 0;

#if defined(HAVE_NATIVE_ALLOC)
    if (x_native) {
        return 0;
    }
#endif /*HAVE_NATIVE_ALLOC*/

#if defined(HAVE_NATIVE_ALLOC)
    if (p && size && NULL != dlrealloc_in_place(p, size)) {
        return size;
    }
#elif defined(_MSC_VER)
    if (p && size && size && NULL != _expand(p, size)) {
        return _msize(p);
    }
#else
    __CUNUSED(size);
#endif
    return 0;
}


size_t
chk_expand(void *p, size_t size)
{
#if defined(HAVE_NATIVE_ALLOC)
    if (x_native) {
#if defined(_MSC_VER)
        if (p && size && NULL != _expand(p, size)) {
            return _msize(p);
        }
#endif
        return 0;
    }
#endif /*HAVE_NATIVE_ALLOC*/

#if defined(HAVE_NATIVE_ALLOC)
    if (p && size && NULL != dlrealloc_in_place(p, size)) {
        return size;
    }
#elif defined(_MSC_VER)
    if (p && size && NULL != _expand(p, size)) {
        return _msize(p);
    }
#elif defined(__GNUC__) && defined(HAVE_MALLOC_USAGE_SIZE)
    if (p && size && size <= malloc_usage_size(p)) {
        void *newptr = realloc(p, size);
        assert(newptr == p);
        return size;
    }
#else
    __CUNUSED(p);
    __CUNUSED(size);
#endif
    return 0;
}


void
chk_leak(const void *p)
{
    __CUNUSED(p);
    ++x_leak;
}


int
chk_isleak(const void *p)
{
    __CUNUSED(p);
    return -1;
}


void
chk_free(void *p)
{
    if (p) {
        --x_inuse;

#if defined(HAVE_NATIVE_ALLOC)
        if (x_native) {
            free(p);
            return;
        }
#endif /*HAVE_NATIVE_ALLOC*/

        FREE(p);
    }
}


void *
chk_salloc(const char *s)
{
    void *result;

#if defined(HAVE_NATIVE_ALLOC)
    if (x_native) {
        if (NULL != (result = strdup(s)))
            ++x_inuse;
        return result;
    }
#endif /*HAVE_NATIVE_ALLOC*/

    {   const size_t len = strlen(s) + 1;

        if (NULL != (result = MALLOC(len))) {
            memcpy(result, s, len);
            ++x_inuse;
        }
    }
    return result;
}


void *
chk_snalloc(const char *s, size_t len)
{
    void *result;

#if defined(HAVE_NATIVE_ALLOC)
    if (x_native) {
        if (NULL != (result = malloc(len + 1))) {
            memcpy(result, s, len);
            ((char *) result)[len] = 0;
            ++x_inuse;
        }
        return result;
    }
#endif /*HAVE_NATIVE_ALLOC*/

    if (NULL != (result = MALLOC(len + 1))) {
        memcpy(result, s, len);
        ((char *) result)[len] = 0;
        ++x_inuse;
    }
    return result;
}

/*end*/
