#include <edidentifier.h>
__CIDENT_RCSID(gr_argrc_c,"$Id: argrc.c,v 1.5 2024/12/09 14:48:39 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: argrc.c,v 1.5 2024/12/09 14:48:39 cvsuser Exp $
 * Command line argument resource functionality.
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
#include <errno.h>
#include <assert.h>

#include <edfileio.h>
#include <chkalloc.h>
#include <libstr.h>

#include "argrc.h"

static int          rcparse(const char *argv0, struct argrc *rc);
static char *       rccompress(char *cursor, unsigned *argc);


int
argrc_load(const char *file, struct argrc *rc)
{
    return argrc_load2(NULL, file, rc);
}


int
argrc_load2(const char *argv0, const char *file, struct argrc *rc)
{
    const char* progname = (argv0 ? argv0 : "argrc");
    struct stat st = { 0 };
    char *data;
    int fd, ret;

    assert(file && rc);    
    (void)memset(rc, 0, sizeof(*rc));
    if (NULL == file)
        return 0;

    // import
#if !defined(O_BINARY)
#define O_BINARY 0
#endif

    if ((fd = fileio_open(file, O_RDONLY | O_BINARY, 0)) < 0 || fstat(fd, &st) == -1) {
        fprintf(stderr, "%s: cannot open <%s>\n", progname, file);
        if (fd >= 0) {
            fileio_close(fd);
        }
        return -1;
    }

    if (0 == st.st_size) {                      // empty
        fileio_close(fd);
        return 0;
    }

    if (NULL == (data = (char *) chk_alloc(st.st_size + 2)) ||
            fileio_read(fd, data, st.st_size) != st.st_size) {
        const int xerrno = errno;

        fprintf(stderr, "%s: failed to read <%s> : %s (%d)\n", 
            progname, file, strerror(xerrno), xerrno);
        chk_free(data);
        fileio_close(fd);
        return -1;
    }

    data[st.st_size] = '\0';                    // terminate \0\0
    data[st.st_size + 1] = '\0';
    fileio_close(fd);

    rc->data = data;                            // working buffer
    if ((ret = rcparse(argv0, rc)) <= 0) {
        if (-1 == ret) {
            const int xerrno = errno;

            fprintf(stderr, "%s: failed to process <%s> : %s (%d)\n",
                progname, file, strerror(xerrno), xerrno);
        }
        argrc_free(rc);
    }
    return ret;
}


int
argrc_parse(const char* buffer, struct argrc* rc)
{
    return argrc_parse2(NULL, buffer, rc);
}


int
argrc_parse2(const char* argv0, const char *buffer, struct argrc* rc)
{
    size_t size;
    char *data;
    int ret;

    assert(buffer && rc);
    (void)memset(rc, 0, sizeof(*rc));
    if (NULL == buffer)
        return 0;

    size = strlen(buffer);
    if (NULL == (data = (char *) chk_alloc(size + 2))) {
        return -1;
    }

    (void) memcpy(data, buffer, size);
    data[size] = 0;                             // terminate \0\0
    data[size + 1] = 0;

    rc->data = data;                            // working buffer
    if ((ret = rcparse(argv0, rc)) <= 0) {
        argrc_free(rc);
    }
    return ret;
}


void
argrc_free(struct argrc *rc)
{
    if (NULL == rc) return;
    chk_free((void *)rc->argv);
    chk_free((void *)rc->data);
    (void) memset(rc, 0, sizeof(*rc));
}


static int
rcparse(const char *argv0, struct argrc *rc)
{
    char *end, *data = rc->data;
    const char **argv;
    unsigned n, argc = 0;

    // compress, removing whitespace and comments
    if ((end = rccompress(data, &argc)) == data || 0 == argc) {
        return 0;                               // empty
    }

    if (argv0) {                                // argv0, clone
        const size_t argsz = ((argc + 2 /*argv0 + NULL*/) * sizeof(char*)),
            progsz = strlen(argv0) + 1 /*NUL*/;
        char* progname;

        if (NULL == (argv = (const char**)chk_alloc(argsz + progsz))) {
            return -1;
        }

        progname = ((char *)(argv)) + argsz;
        memcpy(progname, argv0, progsz);
        rc->argc = argc + 1;
        rc->argv = argv;
        *argv++ = progname;

    } else {
        const size_t argsz = ((argc + 1 /*NULL*/) * sizeof(char*));

        if (NULL == (argv = (const char**)chk_alloc(argsz))) {
            return -1;
        }

        rc->argc = argc;
        rc->argv = argv;
    }

    // export
    for (n = 0; n < argc && data < end; data += strlen(data) + 1) {
        assert(data < end);
        argv[n++] = data;
    }
    argv[n] = NULL;

    assert(argc == n);
    assert(data == end);

    return argc; // loaded count, not including argv0, if any
}


static int
is_white(char ch)
{                                               // white-space, avoid locale
    return (ch == ' '|| ch == '\t' || ch == '\f' || ch == '\v' || ch == '\n' || ch == '\r');
}


static char *
rccompress(char *cursor, unsigned *argc)
{
    char *out = cursor;
    unsigned cnt = 0;

    // compress
    for (;;) {

        // leading
        while (is_white(*cursor)) {
            ++cursor;                           // white-space
        }

        if (0 == *cursor)
            break;

        if (';' == *cursor || '#' == *cursor) { // end-of-line comment
            while (*cursor) {
                if ('\n' == *cursor++) {
                    if ('\r' == *cursor) {
                        ++cursor;
                    }
                    break;
                }
            }
            continue;
        }

        // argument
        if ('\"' == *cursor || '\'' == *cursor) {
            // quoted argument
            char quote = *cursor++;

            for (;;) {
                const char ch = *cursor++;

                if (ch == 0 || ch == '\n' || ch == '\r' || ch == quote)
                    break;

                if ('\\' == ch)  {              // escape     
                    const char qch = *cursor;
                    if (qch == '\"' || qch == '\'' || qch == '\\') {
                        *out++ = qch;
                        ++cursor;
                        continue;
                    }
                }

                *out++ = ch;
            }

        } else {
            for (;;) {
                const char ch = *cursor++;

                if (ch == 0 || is_white(ch))
                    break;

                if ('\\' == ch)  {              // escape     
                    const char qch = *cursor;
                    if (qch == '\"' || qch == '\'' || qch == '\\') {
                        *out++ = qch;
                        ++cursor;
                        continue;
                    }
                }

                *out++ = ch;
            }
        }

        // Terminate
        assert(cursor > out);
        if (out < cursor)
            *out++ = 0;
        ++cnt;
    }

    *argc = cnt;
    return out;
}

//end
