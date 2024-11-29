#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyutil_test_c,"$Id: ttyutil_test.c,v 1.1 2024/11/18 13:42:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyutil_test.c,v 1.1 2024/11/18 13:42:22 cvsuser Exp $
 * TTY common utility functions
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

#include "ttyutil.h"
#if defined(NDEBUG)
#undef NDEBUG
#endif

#include <assert.h>

int
main()
{
    assert(tty_isterm("screen", "screen"));
    assert(tty_isterm("screen-xxx", "screen"));
    assert(tty_isterm("screen.linux", "screen"));
    assert(tty_isterm("screen.linux", "screen.linux"));
    assert(! tty_isterm("screen1", "screen"));
    assert(! tty_isterm("screen2", "screen"));

    assert(tty_hasfeature("screen-rv", "rv"));
    assert(tty_hasfeature("screen-rv-xxx", "rv"));
    assert(tty_hasfeature("screen-xxx-rv", "rv"));

    assert(tty_hasfeatureplus("xterm-direct", "direct"));
    assert(tty_hasfeatureplus("xterm-direct256", "direct"));
    assert(! tty_hasfeatureplus("xterm-direct256a", "direct"));

    { // xterm
        const char ctrltab[] = "\x1b[3;3~";     // Control-Del
        unsigned args[4] = {0}, nargs = sizeof(args)/sizeof(args[0]);
        char params[3] = {0};
        int ret;

        ret = tty_csi_parse(ctrltab, sizeof(ctrltab) - 1, nargs, args, params, &nargs);
        printf("Control-Del = %d %u;%u;%u;%u %u\n", ret, args[0], args[1], args[2], args[3], params[0]);

        assert(ret == (sizeof(ctrltab) - 1));
        assert('~' == params[0]);
        assert(2   == nargs);
        assert(3   == args[0]);
        assert(3   == args[1]);
        assert(0   == args[2]);
        assert(0   == args[3]);
    }

    { // xterm-mok2
        const char ctrltab[] = "\x1b[27;5;9~";  // Control-TAB
        unsigned args[4] = {0}, nargs = sizeof(args)/sizeof(args[0]);
        char params[3] = {0};
        int ret;

        ret = tty_csi_parse(ctrltab, sizeof(ctrltab) - 1, nargs, args, params, &nargs);
        printf("Control-TAB = %d %u;%u;%u;%u %u\n", ret, args[0], args[1], args[2], args[3], params[0]);

        assert(ret == (sizeof(ctrltab) - 1));
        assert('~' == params[0]);
        assert(3   == nargs);
        assert(27  == args[0]);
        assert(5   == args[1]);
        assert(9   == args[2]);
        assert(0   == args[3]);
    }

    { // mintty-mok2
        const char ctrlp[] = "\x1b[112;5u";     // Control-p
        unsigned args[4] = {0}, nargs = sizeof(args)/sizeof(args[0]);
        char params[3] = {0};
        int ret;

        ret = tty_csi_parse(ctrlp, sizeof(ctrlp) - 1, nargs, args, params, &nargs);
        printf("Control-p = %d %u;%u;%u;%u %u\n", ret, args[0], args[1], args[2], args[3], params[0]);

        assert(ret == (sizeof(ctrlp) - 1));
        assert('u' == params[0]);
        assert(2   == nargs);
        assert(112 == args[0]);
        assert(5   == args[1]);
        assert(0   == args[2]);
        assert(0   == args[3]);
    }

    return 0;
}


void
trace_ilog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

/*end*/
