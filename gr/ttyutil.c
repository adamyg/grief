#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyutil_c,"$Id: ttyutil.c,v 1.7 2024/08/31 08:13:38 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyutil.c,v 1.7 2024/08/31 08:13:38 cvsuser Exp $
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

#if !defined(USE_VIO_BUFFER) && !defined(DJGPP)

#include <edtermio.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <edalt.h>

#include "debug.h"                              /* trace ... */
#include "mouse.h"
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
tty_write(const void *buffer, int length)
{
    return sys_write(TTY_OUTFD, buffer, length);
}


/*
 *  tty_read ---
 *      Terminal low-level read.
 */
int
tty_read(void *ibuffer, int length, int timeoutms)
{
    char *buffer = ((char *)ibuffer);
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


/*
 *  tty_mouse_xterm ---
 *      Parse a xterm (X10 compatibility mode) mouse encoded event.
 *
 *  Parameters:
 *      me - Optional event structure if be populated; assumed zero initialised.
 *      seq - Mouse message sequence.
 *
 *  Returns:
 *      Returns the positive value MOUSE_XTERM_KEY on success and populates 'me',
 *      0 when additional data is required, otherwise -1 if not a recognised message.
 *
 *  Notes:
 *
 *      Normal tracking mode sends an escape sequence on both button press and
 *      release. Modifier key (shift, ctrl, meta) information is also sent. It
 *      is enabled by specifying parameter 1000 to DECSET.
 *
 *      On button press or release, xterm sends CSI M Cb Cx Cy .
 *
 *      o The low two bits of C b encode button information:
 *
 *              0=MB1 pressed,
 *              1=MB2 pressed,
 *              2=MB3 pressed,
 *              3=release.
 *
 *      o The next three bits encode the modifiers which were down when the
 *        button was pressed and are added together:
 *
 *              4=Shift, 8=Meta, 16=Control.
 *
 *        Note however that the shift and control bits are normally unavailable
 *        because xterm uses the control modifier with mouse for popup menus, and
 *        the shift modifier is used in the default translations for button events.
 *        The Meta modifier recognized by xterm is the mod1 mask, and is not
 *        necessarily the "Meta" key (see xmodmap).
 *
 *      o C x and C y are the x and y coordinates of the mouse event, encoded as
 *        in X10 mode. x = (Cx - 33), y = (Cy - 33)
 *
 *      Wheel mice may return buttons 4 and 5. Those buttons are represented by
 *      the same event codes as buttons 1 and 2 respectively, except that 64 is
 *      added to the event code. Release events for the wheel buttons are not
 *      reported. By default, the wheel mouse events are translated to scroll-back
 *      and scroll-forw actions. Those actions normally scroll the whole window,
 *      as if the scrollbar was used. However if Alternate Scroll mode is set,
 *      then cursor up/down controls are sent when the terminal is displaying the
 *      alternate screen.
 *
 *  Notes:
 *      mintty for example may return x/y values of 224, which is (224 + 32) == 256/0x100
 *      yet as this value as an 8-bit 0x00 causes issues as NUL's cannot be represented
 *      within the buffer.
 */
int
tty_mouse_xterm(struct MouseEvent *me, const void *seq)
{
    const unsigned char *cursor = (unsigned char *)seq;
    unsigned arguments[3] = {0}, nargs = 0;

    //
    //  \x1B[Mabc", where:
    //     a:  Button number plus 32.
    //     b:  Column number (one-based) plus 32.
    //     c:  Row number (one-based) plus 32.
    //
    if (cursor[0] != 0x1b || cursor[1] != '[' || cursor[2] != 'M') {
        return -1;                              // CSI
    }
    trace_ilog("\tXTERM: \x1b[<%d\n", cursor + 3);

    // parameters
    for (cursor += 3; *cursor && *cursor >= ' ' && nargs < 3;) {
        arguments[nargs++] = *cursor++;
    }

    // decode
    if (3 == nargs) {
        if (NULL != me) {                       // key_to_int usage
            const unsigned button = arguments[0];

            if (0x60 == (0x60 & button)) {      // 0110 000x
                me->type = MOUSEEVENT_TWHEELED;
                me->b1 = (button & 0x1) ? 0 : 1;

            } else {
                switch (button & 0x3) {         // 0000 00xx
                case 0: // button-1
                    me->type = ((button & 32) ? MOUSEEVENT_TMOTION : 0);
                    me->b1 = 1;
                    break;
                case 1: // button-2
                    me->type = ((button & 32) ? MOUSEEVENT_TMOTION : 0);
                    me->b2 = 1;
                    break;
                case 2: // button-3
                    me->type = ((button & 32) ? MOUSEEVENT_TMOTION : 0);
                    me->b3 = 1;
                    break;
                case 3: // release-all
                    me->type = MOUSEEVENT_TRELEASE_ALL;
                    break;
                }
                me->multi = -1;
            }

            me->ctrl =                          // 000x xx00
                (-!!(button & 16) & MOUSEEVENT_CCTRL) |
                (-!!(button &  8) & MOUSEEVENT_CMETA) |
                (-!!(button &  4) & MOUSEEVENT_CSHIFT);
            me->x = (int)(arguments[1] - 32) + 1;
            me->y = (int)(arguments[2] - 32) + 1;
        }
        return MOUSE_XTERM_KEY;
    }
    return 0;
}


/*
 *  tty_mouse_sgr ---
 *      Parse a SGR mouse encoded event.
 *
 *  Parameters:
 *      me - Optional event structure if be populated; assumed zero initialised.
 *      seq - Mouse message sequence.
 *
 *  Returns:
 *      Returns the positive value MOUSE_SGR_KEY on success and populates 'me',
 *      0 when additional data is required, otherwise -1 if not a recognised message.
 */
int
tty_mouse_sgr(struct MouseEvent *me, const void *seq)
{
    const unsigned char *cursor = (unsigned char *)seq;
    unsigned arguments[3] = {0}, args = 0, digits = 0;
    int function = 0;

    //
    //  \x1B[<B;Px;PyM":
    //     B:  Button event.
    //     Px: Column number.
    //     Py: Row number.
    //     M:  M=press or m=release.
    //
    if (cursor[0] != 0x1b || cursor[1] != '[' || cursor[2] != '<') {
        return -1;                              // CSI
    }
    trace_ilog("\tSGR: \x1b[<%d\n", cursor + 3);

    // parameters
    for (cursor += 3; *cursor;) {
        const unsigned char c = *cursor++;

        if (c >= '0' && c <= '9') {
            if (0 == digits++) {
                if (args > 3) {
                    break;                      // overflow
                }
                arguments[args] = (c - '0');
            } else {
                arguments[args] = (arguments[args] * 10) + (c - '0');
            }
        } else if (c == ';' || c == 'M' || c == 'm') {
            trace_ilog("\tsgr[%d]=%u\n", args, arguments[args]);
            digits = 0;
            ++args;
            if (c != ';') {
                function = c;
                break;                          // terminator
            }
        }
    }

    // decode
    if (args >= 3) {
        if (NULL != me) {                       // key_to_int usage
            const unsigned button = arguments[0];

            if (button & 64) {
                me->type = MOUSEEVENT_TWHEELED;
                me->b1 = (button & 0x1) ? 0 : 1;

            } else {
                me->type = ((button & 32) ? MOUSEEVENT_TMOTION : 0);
                me->type |= (function == 'M' ? MOUSEEVENT_TPRESS : MOUSEEVENT_TRELEASE);
                switch (button & 0x3) {
                case 0: // button-1
                    me->b1 = 1;
                    break;
                case 1: // button-2
                    me->b2 = 1;
                    break;
                case 2: // button-3
                    me->b3 = 1;
                    break;
                case 3:
                    assert(0);
                    break;
                }
                me->multi = -1;
            }
            me->ctrl =
                (-!!(button & 16) & MOUSEEVENT_CCTRL) |
                (-!!(button &  8) & MOUSEEVENT_CMETA) |
                (-!!(button &  4) & MOUSEEVENT_CSHIFT);
            me->x = (int)(arguments[1]);
            me->y = (int)(arguments[2]);
        }
        return MOUSE_SGR_KEY;
    }
    return 0;
}



/*
 *  tty_isterm ---
 *      Determine whether the terminal-name has the specified prefix.
 *
 *  Parameters:
 *      term - TERM or BTERM value.
 *      name - Terminal name/prefix.
 */
int
tty_isterm(const char *term, const char *name)
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



int
tty_hasfeature(const char *term, const char *what)
{
    const size_t wlen = (size_t)strlen(what);
    const char *elm;

    for (elm = strchr(term, '-'); elm; elm = strchr(elm, '-')) {
        ++elm;
        if (0 == strncmp(elm, what, wlen)) {    /* -xxxx[\0-] */
            if (elm[wlen] == '\0' || elm[wlen] == '-') {
                return 1;
            }
        }
    }
    return 0;
}



/*
 *  tty_csi_parse ---
 *      Control Sequence Introducer (CSI) parser.
 *
 *  Parameters:
 *      buffer - Control string buffer.
 *      buflen - Length of the control buffer.
 *      maxargs - Maximum arguments; system limit is 16.
 *      arguments - Numeric arguments.
 *      params - Parameters, [0]=final, [1]=opening and [2]=
 *      pnargs - Optional value, populated with numeric arguments.
 *
 *  Returns:
 *      Number of bytes consumed, otherwise 0 if additional characters are required or -1 on error.
 */
int
tty_csi_parse(const char *buffer, size_t buflen,
        unsigned maxargs, unsigned arguments[], char params[3], unsigned *pnargs)
{
    const char *start = buffer, *end = buffer + buflen,
        *cursor = start;

    /*  Control Sequence Introducer (CSI) uses two or more bytes to define a specific control function.
     *
     *      CSI P...P I...I F
     *
     *           CSI:    Control Sequence Introducer (CSI)
     *
     *           P:      Zero or more parameters characters received after CSI; in the range 0x30 -- 0x3F (0-9:;<=>?).
     *
     *               These characters are in the 3/0 to 3/15 range in the code table. Parameter characters modify the action
     *               or interpretation of the seqence. You can use up to 16 parameters per sequence.
     *               You must use the ; (3/11) character to separate parameters.
     *
     *               All parameters are unsigned, positive decimal integers, with the most significant digit sent first.
     *               Any parameter greater than 9999 (decimal) is set to 9999 (decimal). If you do not specify a value,
     *               a 0 value is assumed. A 0 value or omitted parameter indicates a default value for the sequence.
     *               For most sequences, the default value is 1.
     *
     *           I:      Intermediate (zero or more characters); in the range 0x20 -- 0x2F (' ' and !"#$%&'()*+,-./.).
     *
     *           F:      Final (one character); in the range 0x40 -- 0x7F (@A-Z[\]^_`a-z{\}~).
     *
     *  If the first character in a parameter string is the "?") character, it indicates that VT parameters follow.
     *  All common sequences just use the parameters as a series of semicolon-separated numbers such as 1;2;3.
     *
     *  Note: Parser limits the intermediate characters to zero or two; one leading and one as a parameter.
     */

    // CSI
    if (start < end && *start == 0x1b) {        // "\x1b["
        if (++start < end && *start == '[') {
            ++start;
        }
    }

    for (cursor = start; cursor < end; ++cursor) {
        if (*cursor >= 0x40 && *cursor < 0x80) {
            break;                              // final byte; in the range 0x40-0x7E (@A-Z[\]^_`a-z{\}~)
        }
    }

    if (cursor >= end) {
        return 0;                               // no terminator; more needed
    }

    params[0] = params[1] = params[2] = 0;

    // arguments
    {
        unsigned digits = 0, nargs = 0;

        cursor = start;
        if (*cursor >= '<' && *cursor <= '?') {
            params[1] = *cursor++;              // "<,=,>,?", initial command byte
                // eg: "0x1b[?1004h"
        }

        while (cursor < end) {
            const unsigned char c = *((unsigned char *)cursor);

            if (c >= '0' && c <= '9') {         // "0..9", numeric arguments
                if (0 == digits++) {
                    arguments[nargs] = (c - '0');
                } else {
                    arguments[nargs] = (arguments[nargs] * 10) + (c - '0');
                }

            } else if (c == ';') {              // terminator
                digits = 0;
                if (++nargs > 16 /*limit*/ || nargs > maxargs) {
                    break;
                }

            } else if (c >= 0x20 && c <= 0x2f) {
                if (0 == params[2]) {
                    params[2] = c;              // intermediate byte
                } else {
                    return -1;
                }

            } else {
                return -1;                      // unexpected
            }
            ++cursor;
        }

        if (digits)
            ++nargs;

        if (pnargs)
            *pnargs = nargs;
    }

    if (cursor == end) {                        // terminating, command byte
        assert(*cursor >= 0x40 && *cursor < 0x80);
        params[0] = *cursor++;
    }

    return (cursor - end);                      // number of bytes consumed
}


#if defined(LOCAL_MAIN)
#include <assert.h>

void
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
}
#endif

#else

int
tty_mouse_xterm(struct MouseEvent* me, const void* seq)
{
    return -1;
}


int
tty_mouse_sgr(struct MouseEvent* me, const void* seq)
{
    return -1;
}

#endif /*!USE_VIO_BUFFER && !DJGPP*/

/*end*/

