#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyutil_c,"$Id: ttyutil.c,v 1.3 2024/07/23 12:52:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyutil.c,v 1.3 2024/07/23 12:52:20 cvsuser Exp $
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

#if !defined(USE_VIO_BUFFER) && !defined(DJGPP)

#include <edtermio.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */

#include "ttyutil.h"

#include "debug.h"                              /* trace ... */
#include "system.h"
#include "tty.h"

static int isterm(const char *term, const char *name);

/*  Function:           tty_defaultscheme
 *      Determine the terminals default luminance, based on legacy TERM/environment logic.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      0 if light, otherwise 1 if dark.
 */
int
tty_defaultscheme(void)
{
    const char *fgbg, *term = ggetenv("TERM");
    int isdark = 0;

    if (term) {
        if (isterm(term, "linux") == 0
            || isterm(term, "screen.linux") == 0
            || isterm(term, "cygwin") == 0
            || isterm(term, "putty") == 0
            || isterm(term, "ms-terminal") == 0
                || ggetenv("WT_SESSION")
            || (x_pt.pt_vtdatype == 'M') // mintty
            || ((fgbg = ggetenv("COLORFGBG")) != NULL && // rxvt, COLORFGBG='0;default;15'
                    (fgbg = strrchr(fgbg, ';')) != NULL && ((fgbg[1] >= '0' && fgbg[1] <= '6') || fgbg[1] == '8') && fgbg[2] == '\0')) {
            isdark = 1;
        }
    } else {
#if defined(WIN32) || defined(__CYGWIN__)
        isdark = 1;                             /* generally dark */
#endif
    }
    return isdark;
}


static int
isterm(const char *term, const char *name)
{
    const size_t tlen = (size_t)strlen(term);
    const size_t nlen = (size_t)strlen(name);

    if (tlen >= nlen) {                         /* xxxx[\0.-] */
        if (0 == memcmp(term, name, nlen)) {    /* dot, allow <screen.xterm> */
            return (term[nlen] == '\0' || term[nlen] == '-' || term[nlen] == '.');
        }
    }
    return 0;
}


/*
 *  tty_identification ---
 *      Request the terminal version/DA2 details.
 *
 *      DA2 - Secondary Device Attributes
 *          In this DA exchange, the host requests the terminal's identification code, firmware version level, and hardware options.
 *
 *          Host Request:
 *              The host uses the following sequence to send this request.
 *
 *              CSI     >       c       or      CSI     >       0       c
 *              9/11    3/14    6/3             9/11    3/14    3/0     6/3
 *
 *          Terminal Response:
 *              The terminal with a VT keyboard uses the following sequence to respond.
 *
 *              CSI     >       6       1       ;       Pv      ;       0       c
 *              9/11    3/14    3/6     3/1     3/11    3/n     3/11    3/0     6/3     DA2R for terminal with STD keyboard.
 *
 *              CSI     >       6       1       ;       Pv      ;       1       c
 *              9/11    3/14    3/6     3/1     3/11    3/n     3/11    3/1     6/3     DA2R for terminal with PC keyboard.
 *
 *      Example:
 *          \033[>82;20710;0c
 *                ^type
 *                   ^version
 *
 *          Terminal                    Type        Version         Example
 *          ------------------------------------------------------------------
 *          Gnome-terminal (legacy)     1           >= 1115         1;3801;0
 *          PuTTY                       0           136             0;136;0
 *          MinTTY                      77(=M)                      77;20005;0c
 *          rxvt                        82(=R)
 *          screen                      83(=S)                      83;40500;0
 *          urxvt                       85(=U)
 *          xterm                       -2(a)       123             XTERM_VERSION=123
 *
 *  Parameters:
 *      RV - Optional request-command string; default applied otherwise.
 *      timeoutoutms - Timeout in millseconds.
 *
 *  Returns:
 *      0 if successful, otherwise -1.
 *
 *      x_pt.pt_vtdatype/vtdaversion and vtdaoptions upon details being available.
 */
int
tty_identification(const char *RV, int timeoutms)
{
    static char xterm_da2_cmd[] = "\033[>c";
    const char *xterm_version;
    int ret = -1;

    /*
     *  XTERM_VERSION/
     *      Xterm(256) ==> 256
     */
    if (NULL != (xterm_version = ggetenv("XTERM_VERSION"))) {
        char vname[32+1] = {0};
        int vnumber = 0;
                                                /* decode and return patch/version number */
        if (2 == sscanf(xterm_version, "%32[^(](%u)", vname, &vnumber))
            if (vnumber > 0) {
                x_pt.pt_vtdatype = -2;          /* source: xterm_version */
                x_pt.pt_vtdaversion = vnumber;
                x_pt.pt_vtdaoptions = 0;
            }
        trace_ilog("XTERM_VERSION(%s) = %d (%s)\n", xterm_version, vnumber, vname);
        ret = -2;
    }

    /*
     *  DA2/Request version.
     */
    if (NULL == RV || 0 == *RV)
        RV = xterm_da2_cmd;                     /* default */

    {
        const int rvlen = strlen(RV);
        char buffer[32] = {0};
        int len = 0;

        if (rvlen == tty_write(RV, rvlen) &&
                (len = tty_read(buffer, sizeof(buffer), timeoutms)) > 1) {
            trace_ilog("term_da2(%d, %s)\n", len, buffer);

            if ('\033' == buffer[0] && '[' == buffer[1] &&
                    ('>' == buffer[2] || '?' == buffer[2])) {
                unsigned datype, daversion, daoptions = 0;

                if (sscanf(buffer + 3, "%u;%u;%uc", &datype, &daversion, &daoptions) >= 2) {
                    trace_ilog("\t==> type:%d, version:%d, options:%x\n", datype, daversion, daoptions);
                    x_pt.pt_vtdatype = datype;
                    x_pt.pt_vtdaversion = daversion;
                    x_pt.pt_vtdaoptions = daoptions;
                    return 0;
                }
            }
        }
    }
    return ret;
}


/*  Function:           tty_luminance
 *      Request the terminals background RGB color and determine the luminance.
 *
 *  Parameters:
 *      RV - Optional request-command string; default applied otherwise.
 *      timeoutoutms - Timeout in millseconds.
 *
 *  Returns:
 *      0 if light, 1 if dark, otherwise -1.
 */
int
tty_luminance(int timeoutms)
{
#define XTERM_OCS11_LEN     (sizeof(xterm_ocs11) - 1)

    static char xterm_ocs11[] = "\x1b]11;?\007";

    unsigned rgb[3] = {0,0,0}, rgbmax = 0;
    unsigned char *cp, buffer[32] = {0};
    int len = 0;

    /*
     *  Format:
     *
     *          <ESC>]rgb:xx/xx/xx<ESC|DEL>
     *      or  <ESC>]rgb:xxxx/xxxx/xxxx<ESC|DEL>
     *
     *  Example:
     *
     *      echo -ne '\e]11;?\a'; cat
     *
     *      ESC]11;rgb:0000/0000/0000
     */
    if ((XTERM_OCS11_LEN != tty_write(xterm_ocs11, XTERM_OCS11_LEN)) ||
            (len = tty_read(buffer, sizeof(buffer), timeoutms)) < 1) {
        trace_ilog("ttluminance : io (tm=%d, len=%d)\n", timeoutms, len);
        return -1;
    }

    cp = buffer;
    if (cp[0] == '\033' && cp[1] == ']') {      /* ESC] */
        cp += 2;
    } else if (cp[0] == 0x9d) {                 /* OSC */
        cp += 1;
    } else {
        cp = NULL;
    }

    if (cp && cp[0] == '1' && (cp[1] == '0' || cp[1] == '1') && cp[2] == ';') {
        /*
         *  parse RGB values
         */
        const char *spec = (const char *)(cp + 3);
        if (cp[11] == '/') {
            if (sscanf(spec, "rgb:%4x/%4x/%4x\033", rgb+0, rgb+1, rgb+2) == 3 ||
                    sscanf(spec, "rgb:%4x/%4x/%4x\007", rgb+0, rgb+1, rgb+2) == 3) {
                rgbmax = 0xffff;
            }

        } else if (cp[9] == '/') {
            if (sscanf(spec, "rgb:%2x/%2x/%2x\033", rgb+0, rgb+1, rgb+2) == 3 ||
                    sscanf(spec, "rgb:%2x/%2x/%2x\007", rgb+0, rgb+1, rgb+2) == 3) {
                rgbmax = 0xff;
            }
        }
    }

    if (rgbmax) {
        /*
         *  Luminance (perceived)
         *  Reference: https://www.w3.org/TR/AERT/#color-contrast
         */
        const double r = (double)rgb[0] / (double)rgbmax;
        const double g = (double)rgb[1] / (double)rgbmax;
        const double b = (double)rgb[2] / (double)rgbmax;
        const double l = (0.299 * r) + (0.587 * g) + (0.114 * b);

        trace_ilog("ttluminance(%d, %s) : %04x/%04x/%04x\n", len, buffer, rgb[0], rgb[1], rgb[2]);
        trace_ilog("\t==> %g [%s]\n", l, l < 0.5 ? "dark" : "light");

        if (l < 0.5) {
            return 1;                           /* dark */
        }
        return 0;                               /* otherwise light */
    }

    trace_ilog("ttluminance(%d, %s) : n/a", len, buffer);
    return -1;
}


#if (TODO_MCHAR_DETECT)                         /* TODO: terminal ambiguous width */
/*  Function:           tty_utf8_features
 *      Request the terminals background RGB color and determine the luminance.
 *
 *  Parameters:
 *      RV - Optional request-command string; default applied otherwise.
 *      timeoutoutms - Timeout in millseconds.
 *
 *  Returns:
 *      0 if light, 1 if dark, otherwise -1.
 */
static void
tty_utf8_features(void)
{
#define XTERM_UTF8_TEST2        (sizeof(xterm_utf8_test2)-1)

    static unsigned char xterm_utf8_test2[] = {
            '\r',                               /* UTF features */
            0xa5,
            0xc3, 0x84, 0xd9,
            0xa7,
            0xd8, 0xb8, 0xe0, 0xe0,
            0xa9,
            0xa9,
            0xb8, 0x88, 0xe5, 0xe5, 0x88,
            0xa2, 0xa2,
            0x1b, '[',  '6',  'n',              /* cursor position */
            0x00                                /* NUL */
            };

    if (XTERM_UTF8_TEST2 == tty_write(xterm_utf8_test2, XTERM_UTF8_TEST2)) {
        int row = -1, col = -1;
        char buffer[32] = {0};
        int len;

        /*
         *  check cursor column after test string, determine screen mode
         *
         *      6       -> UTF-8, no double-width, with LAM/ALEF ligature joining
         *      7       -> UTF-8, no double-width, no LAM/ALEF ligature joining
         *      8       -> UTF-8, double-width, with LAM/ALEF ligature joining
         *      9       -> UTF-8, double-width, no LAM/ALEF ligature joining
         *      11,16   -> CJK terminal (with luit)
         *      10,15   -> 8 bit terminal or CJK terminal
         *      13      -> Poderosa terminal emulator, UTF-8, or TIS620 terminal
         *      14,17   -> CJK terminal
         *      16      -> Poderosa, ISO Latin-1
         *      (17)    -> Poderosa, (JIS)
         *      18      -> CJK terminal (or 8 bit terminal, e.g. Linux console)
         */
        if ((len = tty_read(buffer, sizeof(buffer), -2)) >= 4) {
            sscanf(buffer, "\033[%d;%dR", &row, &col);
        }

        trace_ilog("\tisutf8B(%d) = col:%d\n", len, col);
    }
}
#endif  /*XXX_MCHAR_DETECT*/


/*
 *  tty_write---
 *      Terminal low-level write.
 */
int
tty_write(const char *buffer, int length)
{
    return sys_write(TTY_OUTFD, buffer, length);
}


/*
 *  tty_read ---
 *      Terminal low-level read.
 */
int
tty_read(char *buffer, int length, int timeoutms)
{
    int cnt = 0;

    if (timeoutms <= -2)
        timeoutms = 700;                        /* ms escape character wait; delay */

    assert(buffer && length);
    assert(timeoutms >= -1);
    if (NULL == buffer || 0 == length)
        return 0;

    if (length > 1) {
        int ret;

        --length;                               /* null terminator */

        if (timeoutms < 0) {                    /* blocking */
            if ((ret = sys_read(TTY_INFD, buffer + cnt, length - cnt)) > 0) {
                cnt = ret;
                while (cnt < length &&
                        (ret = sys_read_timed(TTY_INFD, buffer + cnt, length - cnt, 50, NULL)) > 0) {
                    cnt += ret;                 /* secondary characters */
                }
            }

        } else {                                /* timed */
            if ((ret = sys_read_timed(TTY_INFD, buffer, length, timeoutms, NULL)) > 0) {
                cnt = ret;
            }
        }
    }

    buffer[cnt] = '\0';
    return cnt;
}

#endif  /*!USE_VIO_BUFFER && !DJGPP */

/*end*/

