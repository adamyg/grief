/*	$OpenBSD: fread.c,v 1.6 2005/08/08 08:05:36 espie Exp $ */
/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <bstdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "local.h"

static int
lflush(BFILE *fp)
{
    if ((fp->_flags & (__BSLBF|__BSWR)) == (__BSLBF|__BSWR))
        return (__sflush_locked(fp));
    return (0);
}

size_t
bfread(void *buf, size_t size, size_t count, BFILE *fp)
{
    size_t resid;
    char *p;
    int r;
    size_t total;

    /*
     * The ANSI standard requires a return value of 0 for a count
     * or a size of 0.  Peculiarily, it imposes no such requirements
     * on fwrite; it only requires fread to be broken.
     */
    if ((resid = count * size) == 0)
        return (0);
    FLOCKFILE(fp);
    if (fp->_r < 0)
        fp->_r = 0;
    total = resid;
    p = buf;

    /* optimize unbuffered reads */
    if (fp->_flags & __BSNBF && fp->_ur == 0)
    {
        /* the following comes mainly from __srefill(), with slight
         * modifications
         */

        /* make sure stdio is set up */
        if (!__sdidinit)
            __sinit();

        fp->_r = 0;     /* largely a convenience for callers */

        /* SysV does not make this test; take it out for compatibility */
        if (fp->_flags & __BSEOF) {
            FUNLOCKFILE(fp);
            return (EOF);
        }

        /* if not already reading, have to be reading and writing */
        if ((fp->_flags & __BSRD) == 0) {
            if ((fp->_flags & __BSRW) == 0) {
                fp->_flags |= __BSERR;
                FUNLOCKFILE(fp);
                errno = EBADF;
                return (EOF);
            }
            /* switch to reading */
            if (fp->_flags & __BSWR) {
                if (__sflush(fp)) {
                    FUNLOCKFILE(fp);
                    return (EOF);
                }
                fp->_flags &= ~__BSWR;
                fp->_w = 0;
                fp->_lbfsize = 0;
            }
            fp->_flags |= __BSRD;
        } else {
            /*
             * We were reading.  If there is an ungetc buffer,
             * we must have been reading from that.  Drop it,
             * restoring the previous buffer (if any).  If there
             * is anything in that buffer, return.
             */
            if (HASUB(fp)) {
                FREEUB(fp);
            }
        }

        /*
         * Before reading from a line buffered or unbuffered file,
         * flush all line buffered output files, per the ANSI C
         * standard.
         */

        if (fp->_flags & (__BSLBF|__BSNBF)) {
            /* Ignore this file in _fwalk to deadlock. */
            fp->_flags |= __BSIGN;
            (void) _fwalk(lflush);
            fp->_flags &= ~__BSIGN;

            /* Now flush this file without locking it. */
            if ((fp->_flags & (__BSLBF|__BSWR)) == (__BSLBF|__BSWR))
                __sflush(fp);
        }

        while (resid > 0) {
            int   len = (*fp->_read)(fp->_cookie, p, resid );
            fp->_flags &= ~__BSMOD;
            if (len <= 0) {
                if (len == 0)
                    fp->_flags |= __BSEOF;
                else {
                    fp->_flags |= __BSERR;
                }
                FUNLOCKFILE(fp);
                return ((total - resid) / size);
            }
            p     += len;
            resid -= len;
        }
        FUNLOCKFILE(fp);
        return (count);
    }
    else
    {
        while (resid > (size_t)(r = fp->_r)) {
            (void)memcpy((void *)p, (void *)fp->_p, (size_t)r);
            fp->_p += r;
            /* fp->_r = 0 ... done in __srefill */
            p += r;
            resid -= r;
            if (__srefill(fp)) {
                /* no more input: return partial result */
                FUNLOCKFILE(fp);
                return ((total - resid) / size);
            }
        }
    }

    (void)memcpy((void *)p, (void *)fp->_p, resid);
    fp->_r -= resid;
    fp->_p += resid;
    FUNLOCKFILE(fp);
    return (count);
}
