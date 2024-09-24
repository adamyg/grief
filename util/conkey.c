/* -*- mode: c; indent-width: 4; -*- */
/* $Id: conkey.c,v 1.8 2024/09/20 14:51:42 cvsuser Exp $
 * console key util, support.
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

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if defined(_MSC_VER) && (_MSC_VER <= 1600 /*2010*/)
#define snprintf _snprintf
#endif

#if defined(_WIN32)
#define  WINDOWS_MEAN_AND_LEAN
#include <windows.h>
#else
#include "conkeywin.h"
#endif

#include "conkey.h"

static unsigned XTermModifiers(unsigned n);


//
//  Mouse event description
//
const char *
mouse_description(const MOUSE_EVENT_RECORD *me)
{
        static char t_buffer[200];              // local description buffer

        const DWORD dwControlKeyState = me->dwControlKeyState,
                dwEventFlags = me->dwEventFlags, dwButtonState = me->dwButtonState;
        const BOOL release = (dwEventFlags & (MOUSE_RELEASED|MOUSE_RELEASE_ALL)) ? 1 : 0;
        char* cursor = t_buffer, * end = cursor + sizeof(t_buffer);

        // controls
        if (dwControlKeyState & ALT_PRESSED)
                cursor += snprintf(cursor, end - cursor, "Alt-");
        if (dwControlKeyState & CTRL_PRESSED)
                cursor += snprintf(cursor, end - cursor, "Ctrl-");
        if (dwControlKeyState & CTRL_PRESSED)
                cursor += snprintf(cursor, end - cursor, "Shift-");

        // buttons
        if (dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED)
                cursor += snprintf(cursor, end - cursor, "Button1");
        else if (dwButtonState & RIGHTMOST_BUTTON_PRESSED)
                cursor += snprintf(cursor, end - cursor, "Button2");
        else if (dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED)
                cursor += snprintf(cursor, end - cursor, "Button3");
    //  else if (dwButtonState & FROM_LEFT_3RD_BUTTON_PRESSED)
    //          cursor += snprintf(cursor, end - cursor, "Button4");
    //  else if (dwButtonState & FROM_LEFT_4TH_BUTTON_PRESSED)
    //          cursor += snprintf(cursor, end - cursor, "Button5");

        // actions
        if (release) {
                cursor += snprintf(cursor, end - cursor, (cursor > t_buffer ? "-Release" : "Release"));
        } else {
                if (cursor > t_buffer)
                        *cursor++ = '-';

                if (dwEventFlags & MOUSE_WHEELED) {
                        cursor += snprintf(cursor, end - cursor, "Wheel-%s",
                                    (0xFF000000 & me->dwButtonState ? "Down" : "Up"));

                } else if (dwEventFlags & MOUSE_HWHEELED) {
                        cursor += snprintf(cursor, end - cursor, "Wheel-%s",
                                    (0xFF000000 & me->dwButtonState ? "Right" : "Left"));

                } else if (dwEventFlags & (MOUSE_WHEELED | MOUSE_HWHEELED)) {
                        cursor += snprintf(cursor, end - cursor, "Wheel");

                } else if (dwEventFlags & MOUSE_MOVED) {
                        cursor += snprintf(cursor, end - cursor, "Motion");

                } else if (dwEventFlags & DOUBLE_CLICK) {
                        cursor += snprintf(cursor, end - cursor, "Double");

                } else if (0 == dwEventFlags || (dwEventFlags & MOUSE_PRESSED)) {
                        cursor += snprintf(cursor, end - cursor, "Press");
                }
        }

        // position
        switch (dwEventFlags)
        {
        case MOUSE_WHEELED:  // vertical
        case MOUSE_HWHEELED: // horizontal
                break;
        default:
                cursor += snprintf(cursor, end - cursor, ", X:%u, Y:%u",
                            (unsigned)me->dwMousePosition.X, (unsigned)me->dwMousePosition.Y);
                break;
        }

        return (cursor > t_buffer ? t_buffer : NULL);
}


const char *
key_description(const KEY_EVENT_RECORD *ke)
{
#if defined(META_PRESSED)
#define CTRLSTATUSMASK  (META_PRESSED|ALT_PRESSED|CTRL_PRESSED||SHIFT_PRESSED)
#else
#define CTRLSTATUSMASK  (ALT_PRESSED|CTRL_PRESSED||SHIFT_PRESSED)
#endif

        static char t_buffer[200];              // local description buffer

        const DWORD dwControlKeyState = ke->dwControlKeyState;
//      const WORD wVirtualKeyCode = ke->wVirtualKeyCode;
        char* cursor = t_buffer, * end = cursor + sizeof(t_buffer);

        // Control states
#if defined(META_PRESSED)
        if (dwControlKeyState & META_PRESSED)
            cursor += snprintf(cursor, end - cursor, "Meta-");
#endif

        if (dwControlKeyState & ALT_PRESSED)
            cursor += snprintf(cursor, end - cursor, "Alt-");

        if (dwControlKeyState & CTRL_PRESSED)
            cursor += snprintf(cursor, end - cursor, "Ctrl-");

        if (dwControlKeyState & SHIFT_PRESSED)
            cursor += snprintf(cursor, end - cursor, "Shift-");

        // Key code
        if (ke->uChar.UnicodeChar) {
                const WORD uc = ke->uChar.UnicodeChar;
                const char *desc = NULL;

                switch (uc) {
                case '\a': desc = "Alert"; break;
                case '\b': desc = "Backspace"; break;
                case '\n': desc = "Newline"; break;
                case '\r': desc = "Enter"; break;
                case '\t': desc = "Tab"; break;
                case '\v': desc = "Vtab"; break;
                case '\f': desc = "Formfeed"; break;
                case 0x1b: desc = "Esc"; break;
                case ' ':
                        if (dwControlKeyState & CTRLSTATUSMASK) {
                                desc = "Space";
                        }
                        break;
                }

            if (desc) {
                    cursor += snprintf(cursor, end - cursor, "%s", desc);
            } else {
                    if (uc > ' ' && uc < 0xff) {
                            // ASCII
                            if ((dwControlKeyState & CTRLSTATUSMASK) && uc >= 'a' && uc <= 'z') {
                                *cursor++ = (char)(uc - 'a' + 'A'); // upper-case meta controls.
                            } else {
                                *cursor++ = (char)uc;
                            }
                            *cursor++ = 0;

                    } else if (uc < ' ' && (dwControlKeyState & CTRL_PRESSED)) {
                            // Controls
                            *cursor++ = (char)(('A' - 1) + uc);
                            *cursor++ = 0;

                    } else {
                            // Unicode
                            cursor += snprintf(cursor, end - cursor, "#%u", uc);
                    }
            }
        }

        return (cursor > t_buffer ? t_buffer :  NULL);
}


//
//  Decode a cygwin specific key escape sequence into our internal key-code.
//

const char *
DecodeCygwinKey(INPUT_RECORD *ir, const char *buf, const char *end)
{
        enum {
                cwKeyDown,
                cwRepeatCount,
                cwVirtualKeyCode,
                cwVirtualScanCode,
                cwUnicodeChar,
                cwControlKeyState,
                cwArgumentMax
        };

        if ((buf + 8) < end && buf[0] == '\033' && buf[1] == '{') {
                /*
                 *  \033[2000h - turn on raw keyboard mode,
                 *  \033[2000l - turn off raw keyboard mode.
                 *
                 *  Format:
                 *      <ESC>{Kd;Rc;Vk;Sc;Uc;CsK
                 *
                 *       Kd: the value of bKeyDown.
                 *       Rc: the value of wRepeatCount.
                 *       Vk: the value of wVirtualKeyCode.
                 *       Sc: the value of wVirtualScanCode.
                 *       Uc: the decimal value of UnicodeChar.
                 *       Cs: the value of dwControlKeyState.
                 *
                 *  Example:
                 *      <ESC>{0;1;13;28;13;0K
                 */
                unsigned args[cwArgumentMax] = { 0, 0, 0, 0, 0, 0 };
                const char *cursor;

                if (NULL != (cursor = DecodeKeyArguments(args, cwArgumentMax, 'K', buf + 2, end))) {
                        KEY_EVENT_RECORD* ke = &ir->Event.KeyEvent;

                        memset(ke, 0, sizeof(*ke));
                        ke->bKeyDown = (args[cwKeyDown] ? 1U : 0U);
                        ke->wRepeatCount = (WORD)(args[cwRepeatCount]);
                        ke->wVirtualKeyCode = (WORD)(args[cwVirtualKeyCode]);
                        ke->wVirtualScanCode = (WORD)(args[cwVirtualScanCode]);
                        ke->uChar.UnicodeChar = (WCHAR)(args[cwUnicodeChar]);
                        ke->dwControlKeyState = (DWORD)(args[cwControlKeyState]);
                        ir->EventType = KEY_EVENT;
                        return cursor;
                }
        }
        return NULL;
}


//
//  Decode a MSTerminal specific key escape sequence into our internal key-code.
//

const char *
DecodeMSTerminalKey(INPUT_RECORD *ir, const char *buf, const char *end)
{
        enum {
                msVirtualKeyCode,
                msVirtualScanCode,
                msUnicodeChar,
                msKeyDown,
                msControlKeyState,
                msRepeatCount,
                msArgumentMax
        };

        if ((buf + 8) < end && buf[0] == '\033' && buf[1] == '[') {
                /*
                 *  Terminal win32-input-mode
                 *
                 *      <ESC>[17;29;0;1;8;1_
                 *
                 *  Format:
                 *
                 *      <ESC>[Vk;Sc;Uc;Kd;Cs;Rc_
                 *
                 *      Vk: the value of wVirtualKeyCode - any number. If omitted, defaults to '0'.
                 *      Sc: the value of wVirtualScanCode - any number. If omitted, defaults to '0'.
                 *      Uc: the decimal value of UnicodeChar - for example, NUL is "0", LF is
                 *           "10", the character 'A' is "65". If omitted, defaults to '0'.
                 *      Kd: the value of bKeyDown - either a '0' or '1'. If omitted, defaults to '0'.
                 *      Cs: the value of dwControlKeyState - any number. If omitted, defaults to '0'.
                 *      Rc: the value of wRepeatCount - any number. If omitted, defaults to '1'.
                 *
                 *  Reference
                 *      https://github.com/microsoft/terminal/blob/main/doc/specs/%234999%20-%20Improved%20keyboard%20handling%20in%20Conpty.md
                 */
                unsigned args[msArgumentMax] = { 0, 0, 0, 0, 0, 1 };
                const char *cursor;

                if (NULL != (cursor = DecodeKeyArguments(args, msArgumentMax, '_', buf + 2, end))) {
                        KEY_EVENT_RECORD* ke = &ir->Event.KeyEvent;

                        memset(ke, 0, sizeof(*ke));
                        ke->bKeyDown = (args[msKeyDown] ? 1U : 0U);
                        ke->wRepeatCount = (WORD)(args[msRepeatCount]);
                        ke->wVirtualKeyCode = (WORD)(args[msVirtualKeyCode]);
                        ke->wVirtualScanCode = (WORD)(args[msVirtualScanCode]);
                        ke->uChar.UnicodeChar = (WCHAR)(args[msUnicodeChar]);
                        ke->dwControlKeyState = (DWORD)(args[msControlKeyState]);
                        ir->EventType = KEY_EVENT;
                        return cursor;
                }
        }
        ir->EventType = 0;
        return NULL;
}


const char *
DecodeXTermKey(INPUT_RECORD *ir, const char *buf, const char *end)
{
        if ((buf + 6) < end && buf[0] == '\033' && buf[1] == '[') {
                /*
                 *      \e[27;<modifier>;<char>~ or     xterm
                 *      \e[<char>;<modifier>u           formatOtherKeys=1 in xterm; which is the mintty default.
                 *
                 *      https://invisible-island.net/xterm/modified-keys-gb-altgr-intl.html#other_modifiable_keycodes
                 */
                unsigned args[3] = { 0, 0, 0 };
                const char *cursor;

                if (buf[2] == '2' && buf[3] == '7') {
                        if (NULL != (cursor = DecodeKeyArguments(args, 3, '~', buf + 2, end))) {
                                if (args[0] == 27) {
                                        KEY_EVENT_RECORD* ke = &ir->Event.KeyEvent;
                                        const unsigned modifiers = XTermModifiers(args[1]);
                                        WCHAR key = (WCHAR)args[2];

                                        if ((modifiers & SHIFT_PRESSED) && key >= 'a' && key <= 'z')
                                                key += (WCHAR)('A' - 'a');

                                        memset(ke, 0, sizeof(*ke));
                                        ke->bKeyDown = 1U;
                                        ke->wRepeatCount = 1;
                                        ke->wVirtualKeyCode = (WORD)(0);
                                        ke->wVirtualScanCode = (WORD)(0);
                                        ke->uChar.UnicodeChar = (WCHAR)(key);
                                        ke->dwControlKeyState = (DWORD)(modifiers);
                                        ir->EventType = KEY_EVENT;
                                        return cursor;
                                }
                        }
                }

                if (NULL != (cursor = DecodeKeyArguments(args, 2, 'u', buf + 2, end))) {
                        KEY_EVENT_RECORD* ke = &ir->Event.KeyEvent;
                        const unsigned modifiers = XTermModifiers(args[1]);
                        WCHAR key = (WCHAR)args[0];

                        if ((modifiers & SHIFT_PRESSED) && key >= 'a' && key <= 'z')
                                key += (WCHAR)('A' - 'a');

                        memset(ke, 0, sizeof(*ke));
                        ke->bKeyDown = 1U;
                        ke->wRepeatCount = 1;
                        ke->wVirtualKeyCode = (WORD)(0);
                        ke->wVirtualScanCode = (WORD)(0);
                        ke->uChar.UnicodeChar = (WCHAR)(key);
                        ke->dwControlKeyState = (DWORD)(modifiers);
                        ir->EventType = KEY_EVENT;
                        return cursor;
                }
        }
        ir->EventType = 0;
        return NULL;
}


static unsigned
XTermModifiers(unsigned n)
{
        unsigned code = n - 1, modifiers = 0;

        if (code & 1)
            modifiers |= SHIFT_PRESSED;
        if (code & 2)
            modifiers |= ALT_PRESSED;
        if (code & 4)
            modifiers |= CTRL_PRESSED;
#if defined(META_PRESSED)
        if (code & 8)
            modifiers |= META_PRESSED;
#endif
        // others?
        return modifiers;
}



const void *
DecodeKeyArguments(unsigned arguments[], unsigned maxargs, char terminator, const void *buffer, const void *end)
{
        const char *cursor = (const char *)buffer;
        unsigned args = 0, digits = 0;

        while (cursor < (const char *)end && *cursor) {
                const unsigned char c = *cursor++;

                if (c >= '0' && c <= '9') {
                        if (0 == digits++) {
                                if (args >= maxargs) {
                                        break;  // overflow
                                }
                                arguments[args] = (c - '0');
                        } else {
                                arguments[args] = (arguments[args] * 10) + (c - '0');
                        }
                } else if (c == ';' || c == terminator) {
                        digits = 0;
                        ++args;
                        if (c == terminator) {  // terminator
                                return (args >= 1 ? cursor : NULL);
                        }

                } else {
                        args = 0;               // non-digit
                        break;
                }
        }
        return NULL;
}


//  DecodeXTermMouse
//      Parse a xterm (X10 compatibility mode) mouse encoded event.
//
//      Normal tracking mode sends an escape sequence on both button press and
//      release. Modifier key (shift, ctrl, meta) information is also sent. It
//      is enabled by specifying parameter 1000 to DECSET.
//
//      On button press or release, xterm sends CSI M Cb Cx Cy .
//
//      o The low two bits of C b encode button information:
//
//              0=MB1 pressed,
//              1=MB2 pressed,
//              2=MB3 pressed,
//              3=release.
//
//      o The next three bits encode the modifiers which were down when the
//        button was pressed and are added together:
//
//              4=Shift, 8=Meta, 16=Control.
//
//        Note however that the shift and control bits are normally unavailable
//        because xterm uses the control modifier with mouse for popup menus, and
//        the shift modifier is used in the default translations for button events.
//        The Meta modifier recognized by xterm is the mod1 mask, and is not
//        necessarily the "Meta" key (see xmodmap).
//
//      o C x and C y are the x and y coordinates of the mouse event, encoded as
//        in X10 mode. x = (Cx - 33), y = (Cy - 33)
//
//      Wheel mice may return buttons 4 and 5. Those buttons are represented by
//      the same event codes as buttons 1 and 2 respectively, except that 64 is
//      added to the event code. Release events for the wheel buttons are not
//      reported. By default, the wheel mouse events are translated to scroll-back
//      and scroll-forw actions. Those actions normally scroll the whole window,
//      as if the scrollbar was used. However if Alternate Scroll mode is set,
//      then cursor up/down controls are sent when the terminal is displaying the
//      alternate screen.
//
//  Notes:
//      o msterminal wont report motion beyond x or y value of 94 (127 - 33).
//      o mintty reports x/y values upto 224, which is (224 + 32) == 256/0x100, represented by a nul.
//
const void *
DecodeXTermMouse(INPUT_RECORD *ir, const void *spec, const void *end)
{
        const BYTE *cursor = (const BYTE *)spec;
        unsigned arguments[3] = {0}, nargs = 0;

        //
        //  \x1B[Mabc", where:
        //     a:  Button number plus 32.
        //     b:  Column number (one-based) plus 32.
        //     c:  Row number (one-based) plus 32.
        //
        if ((cursor + 6) > (const BYTE *)end ||
                    cursor[0] != 0x1b || cursor[1] != '[' || cursor[2] != 'M') {
                return NULL;                        // CSI
        }

        // parameters
        for (cursor += 3; cursor != (const BYTE *)end && (*cursor >= ' ' || 0 == *cursor) && nargs < 3;) {
                arguments[nargs++] = *cursor++;     // mintty, allow NUL
        }

        // decode
        if (3 == nargs) {
                MOUSE_EVENT_RECORD* me = &ir->Event.MouseEvent;
                const unsigned button = arguments[0];

                memset(me, 0, sizeof(*me));

                if (0x60 == (0x60 & button)) {      // 0110 000x
                        me->dwEventFlags = MOUSE_WHEELED;
                        me->dwButtonState =
                            (button & 0x1) ? 0xFF000000 /*down*/ : 0x00000000 /*up*/;

                } else {
                        me->dwEventFlags = ((button & 64) ? MOUSE_MOVED : 0); // 1002 mode
                        switch (button & 0x3) {     // 000x 00xx
                        case 0: // button-1
                                me->dwButtonState = FROM_LEFT_1ST_BUTTON_PRESSED;
                                break;
                        case 1: // button-2
                                me->dwButtonState = RIGHTMOST_BUTTON_PRESSED;
                                break;
                        case 2: // button-3
                                me->dwButtonState = FROM_LEFT_2ND_BUTTON_PRESSED;
                                break;
                        case 3:
                                me->dwEventFlags |= MOUSE_RELEASE_ALL;
                                break;
                        }
                }
                me->dwControlKeyState =             // 000x xx00
                    (-!!(button & 16) & CTRL_PRESSED) |
                    (-!!(button &  8) & ALT_PRESSED) |
                    (-!!(button &  4) & SHIFT_PRESSED);
                me->dwMousePosition.X = (short)(arguments[1] - 32) + 1;
                me->dwMousePosition.Y = (short)(arguments[2] - 32) + 1;
                ir->EventType = MOUSE_EVENT;
                return cursor;
        }
        ir->EventType = 0;
        return NULL;
}


//  SGR Mouse event
//
//      SGR(1006) The normal mouse response is altered to use "CSI <" followed by semicolon separated numbers: /
//
//        o Button value
//        o Px and Py ordinates and
//        o Final character which is M for button press and m for button release.
//
const void *
DecodeSGRMouse(INPUT_RECORD *ir, const void *spec, const void *end)
{
        const BYTE *cursor = spec;
        unsigned arguments[3] = {0}, args = 0;
        int digits = 0, press = -1;

        //
        //  \x1B[<B;Px;PyM":
        //     B:  Button event.
        //     Px: Column number.
        //     Py: Row number.
        //     M:  M=press or m=release.
        //
        if ((cursor + 8) > (const BYTE *)end ||
                    cursor[0] != 0x1b || cursor[1] != '[' || cursor[2] != '<') {
                return NULL;                        // CSI
        }

        // parameters
        for (cursor += 3; *cursor;) {
                const BYTE c = *cursor++;

                if (c >= '0' && c <= '9') {
                        if (0 == digits++) {
                                if (args > 3) {
                                        break;      // overflow
                                }
                                arguments[args] = (c- '0');
                        } else {
                                arguments[args] = (arguments[args] * 10) + (c - '0');
                        }
                } else if (c == ';' || c == 'M' || c == 'm') {
                        digits = 0;
                        ++args;
                        if (c != ';') {
                                press = (c == 'M');
                                break;              // terminator
                        }
                }
        }

        // buttons, position
        if (args == 3 && press != -1) {
                MOUSE_EVENT_RECORD* me = &ir->Event.MouseEvent;
                const unsigned button = arguments[0];

                memset(me, 0, sizeof(*me));
                if (button & 64) {
                        me->dwEventFlags = MOUSE_WHEELED;
                        me->dwButtonState =
                            (button & 0x1) ? 0xFF000000 /*down*/ : 0x00000000 /*up*/;

                } else {
                        me->dwEventFlags = ((button & 64) ? MOUSE_MOVED : 0); // 1002 mode
                        me->dwEventFlags |= (press ? MOUSE_PRESSED : MOUSE_RELEASED);
                        switch (button & 0x3) {
                        case 0: // button-1
                            me->dwButtonState = FROM_LEFT_1ST_BUTTON_PRESSED;
                            break;
                        case 1: // button-2
                            me->dwButtonState = RIGHTMOST_BUTTON_PRESSED;
                            break;
                        case 2: // button-3
                            me->dwButtonState = FROM_LEFT_2ND_BUTTON_PRESSED;
                            break;
                        case 3:
                            break;
                        }
                }
                me->dwControlKeyState =
                        (-!!(button & 16) & ALT_PRESSED) |
                        (-!!(button &  8) & CTRL_PRESSED) |
                        (-!!(button &  4) & SHIFT_PRESSED);
                me->dwMousePosition.X = (WORD)(arguments[1]);
                me->dwMousePosition.Y = (WORD)(arguments[2]);
                ir->EventType = MOUSE_EVENT;
                return cursor;
        }
        ir->EventType = 0;
        return NULL;
}

//end