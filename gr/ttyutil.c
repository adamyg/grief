#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyutil_c,"$Id: ttyutil.c,v 1.12 2024/09/24 12:54:15 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyutil.c,v 1.12 2024/09/24 12:54:15 cvsuser Exp $
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
#include <edalt.h>

#include "debug.h"                              /* trace ... */
#include "mouse.h"
#include "system.h"
#include "tty.h"


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
    trace_ilog("\tXTERM: \x1b[M%s\n", cursor + 3);

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
                me->type = ((button & 64) ? MOUSEEVENT_TMOTION : 0); // 1002 mode
                switch (button & 0x3) {         // 0000 00xx
                case 0: // button-1
                    me->b1 = 1;
                    break;
                case 1: // button-2
                    me->b2 = 1;
                    break;
                case 2: // button-3
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
            me->x = (int)(arguments[1] - 32); // + 1;
            me->y = (int)(arguments[2] - 32); // + 1;
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
    trace_ilog("\tSGR: \x1b[<%s\n", cursor + 3);

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
                me->type = ((button & 64) ? MOUSEEVENT_TMOTION : 0); // 1002 mode
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
 *      params - Parameters, [0]=final, [1]=opening and [2]=intermediate (only one supported).
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

        end = cursor;
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

    assert(cursor == end);                      // terminating, command byte
    params[0] = *cursor++;

    return (cursor - buffer);                   // number of bytes consumed
}


/*
 *  UnitTest's
 */

#if defined(LOCAL_MAIN)
#if defined(NDEBUG)
#undef NDEBUG
#endif

#include <assert.h>

void
trace_ilog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}


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
#endif

#else

int
tty_mouse_xterm(struct MouseEvent* me, const void* seq)
{
    __CUNUSED(me);
    __CUNUSED(seq);
    return -1;
}


int
tty_mouse_sgr(struct MouseEvent* me, const void* seq)
{
    __CUNUSED(me);
    __CUNUSED(seq);
    return -1;
}

#endif /*!USE_VIO_BUFFER && !DJGPP*/

/*end*/
