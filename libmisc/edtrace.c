#include <edidentifier.h>
__CIDENT_RCSID(gr_edtrace_c,"$Id: edtrace.c,v 1.48 2024/04/27 15:22:32 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edtrace.c,v 1.48 2024/04/27 15:22:32 cvsuser Exp $
 * Simple diagnostic trace.
 *
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

#include <editor.h>

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>
#endif
#if !defined(HAVE_SYS_TIME_H) || defined(TIME_WITH_SYS_TIME)
#include <time.h>
#endif
#include <stdarg.h>
#include <assert.h>

#include <edconfig.h>
#include <edtrace.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>

static int                  debug_open(void);
static void                 debug_pad(const char *prefix, int depth);

static unsigned             x_debug_flags = 0;  /* EDDEBUG_Fxxx */
static const char *         x_debug_log   = NULL;
static FILE *               x_debug_file  = NULL;


int
trace_filename(const char *name)
{
#if (defined(_WIN32) || defined(WIN32)) && defined(_MSC_VER)
    if (x_debug_log) {
        free((void *)x_debug_log);
    }
    x_debug_log = (name ? _strdup(name) : NULL);
#else
    if (x_debug_log) {
        free((void *)x_debug_log);
    }
    x_debug_log = (name ? strdup(name) : NULL);
#endif
    if (x_debug_file) {
        fclose(x_debug_file);
        x_debug_file = NULL;
    }
    return debug_open();
}


void
trace_active(unsigned flags)
{
    x_debug_flags = flags;
}


int
trace_isactive(void)
{
    return (x_debug_flags && x_debug_file);
}


FILE *
trace_stream(void)
{
    return x_debug_file;
}


void
trace_flush(void)
{
    if (x_debug_file) {
        fflush(x_debug_file);
    }
}


static void
hex(const unsigned char *data, int length, int offsets, const char *term)
{
    const unsigned char *end = data + length;
    char buf[12 + (16 * 7) + 1];                /* 16 characters + NUL */
    int offset = 0, len = 0;

    if (0 == x_debug_flags) {
        return;
    }

    buf[0] = 0;
    while (data < end) {
        const unsigned ch = *data++;

        len += sprintf(buf + len, "%02x %c  ",  /* "xx c  " */
                  ch, (ch >= ' ' && ch < 0x7f ? ch : '.'));
        if (offset++ && 0 == (offset % 16)) {
            if (offsets) {
                trace_log("%04x: %s\n", offset - 1, buf);
            } else {
                trace_log("%s\n", buf);
            }
            buf[len = 0] = 0;
        }
    }

    if (buf[0]) {
        if (offsets) {
            trace_log("%04x: %s%s", offset, buf, term);
        } else {
            trace_log("%s%s", buf, term);
        }
    }
}


static int
debug_open(void)
{
    if (NULL == x_debug_file) {
        const char *name;

        if (NULL == (name = ggetenv("GRIEF_LOG"))) {
            name = x_debug_log;
        }
        x_debug_file = fopen((name ? name : GRLOG_FILE), "w");
#if !defined(DOSISH)
        if (NULL == x_debug_file) {
#if defined(__OS2__)
            x_debug_file = fopen(GRLOG_FILE, "w");
#else
            x_debug_file = fopen("/tmp/" GRLOG_FILE, "w");
#endif
        }
#endif  /*DOSISH*/
    }
    return (x_debug_file != NULL);
}


static void
debug_pad(const char *prefix, int depth)
{
    char buf[1024], *p;                         /* MAGIC */
    int len, i;

    strxcpy(buf, (prefix ? prefix : ""), sizeof(buf));
    len = (int)strlen(buf);

    if ((i = depth) > 0) {
        if (depth > len) {
            depth = len;
        }
        p = buf + len;
        while (i-- > 0) {
            *p++ = '.';
        }
        *p = '\0';
    }
    trace_str(buf);
}


int
trace_log(const char *str, ...)
{
    va_list ap;
    int ret;

    va_start(ap, str);
    ret = trace_logv(str, ap);
    va_end(ap);
    return ret;
}


int
trace_logv(const char *fmt, va_list ap)
{
    int ret = 0;

    if (x_debug_flags) {
        if (debug_open()) {
            ret = vfprintf(x_debug_file, fmt, ap);
            if (EDTRACE_FFLUSH & x_debug_flags) {
                fflush(x_debug_file);
            }
        }
    }
    return ret;
}


void
trace_logx(int level, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    trace_logxv(level, fmt, ap);
    va_end(ap);
}


static const struct tm *
my_localtime(void)
{
    const time_t ctm = time(NULL);
    static struct tm x_ltm;
    static time_t x_ctm;

    if (ctm != x_ctm) {
        if (ctm == (x_ctm + 1)) {
            if (++x_ltm.tm_sec >= 60) {
                x_ltm.tm_sec = 0;
                if (++x_ltm.tm_min >= 60) {
                    x_ltm.tm_min = 0;
                    if (++x_ltm.tm_hour >= 24) {
#if defined(HAVE_LOCALTIME_R)
                        (void) localtime_r(&ctm, &x_ltm);
#else
                        x_ltm = *localtime(&ctm);
#endif
                    }
                }
            }
        } else {
#if defined(HAVE_LOCALTIME_R)
            (void) localtime_r(&ctm, &x_ltm);
#else
            x_ltm = *localtime(&ctm);
#endif
        }
        x_ctm = ctm;
    }
    return &x_ltm;
}


void
trace_logxv(int level, const char *fmt, va_list ap)
{
    static int old_level = -1;
    char buf[32];

    if (x_debug_flags) {
#if (TODO)
        if (EDDEBUG_TIME & x_debug_flags) {
            const struct tm *tp = tp = my_localtime();
            
            sprintf(buf, "%02d:%02d:%02d.%d:",
                tp->tm_year + 1900, tp->tm_min, tp->tm_sec, subsec);
        }
#endif

        if (old_level == level) {
            strcpy(buf, "\t");
        } else {
            sprintf(buf, "%02d:..", level);
        }
        debug_pad(buf, level);
        old_level = level;
        if (debug_open()) {
            vfprintf(x_debug_file, fmt, ap);
            if (EDTRACE_FFLUSH & x_debug_flags) {
                fflush(x_debug_file);
            }
        }
    }
}


void
trace_hex(const void *data, int length)
{
    hex(data, length, TRUE, "\n");
}


void
trace_data(const void *data, int length, const char *term)
{
    hex(data, length, FALSE, (term ? term : "\n"));
}


int
trace_str(const char *str)
{
    int ret = 0;

    if (x_debug_flags) {
        if (str && debug_open()) {
            ret = fputs(str, x_debug_file);
            if (EDTRACE_FFLUSH & x_debug_flags) {
                fflush(x_debug_file);
            }
        }
    }
    return ret;
}


/*
 *  MT-LEVEL:
 *      Not MT safe since the returned address shall be a reference to a local
 *      static buffer.
 */
const char *
c_string(const char *str)
{
    static char buf[4 * 1024];                  /* MAGIC */
    const char *endbp = (buf + (sizeof(buf) - 10));
    register char *bp = buf;
    char c;

    if (0 == x_debug_flags) {
        return str;
    }

    while (0 != (c = *str++)) {
        switch (c) {                            /* escapes */
        case '\a': *bp++ = '\\', *bp++ = 'a';  break;
        case '\b': *bp++ = '\\', *bp++ = 'b';  break;
        case '\f': *bp++ = '\\', *bp++ = 'f';  break;
        case '\n': *bp++ = '\\', *bp++ = 'n';  break;
        case '\r': *bp++ = '\\', *bp++ = 'r';  break;
        case '\t': *bp++ = '\\', *bp++ = 't';  break;
        case '\v': *bp++ = '\\', *bp++ = 'v';  break;
        case '\\': *bp++ = '\\', *bp++ = '\\'; break;
        case 0x1b: *bp++ = '^',  *bp++ = '[';  break;
        default:
            if (c < ' ') {  /* oct */
                sprintf(bp, "\\%03o", (unsigned char)c);
                bp += 4;
            } else {        /* otherwise direct */
                *bp++ = c;
            }
            break;
        }

        if (bp > endbp) {                       /* truncate */
            *bp++ = '.';
            *bp++ = '.';
            *bp++ = '.';
            break;
        }
    }
    *bp = '\0';                                 /* terminate */
    return buf;
}
/*end*/
