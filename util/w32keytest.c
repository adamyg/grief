/* -*- mode: c; indent-width: 4; -*- */
/* $Id: w32keytest.c,v 1.34 2024/10/02 11:51:00 cvsuser Exp $
 * console key-test -- win32
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

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <wincon.h>

#include <stdio.h>
#include <assert.h>

#include "getopt.h"
#include "conkey.h"

#define APP_PRESSED             0x0200      /* APPS enabled, extension */

#if defined(__WATCOMC__) || defined(__MINGW32__)
#if !defined(_countof)
#define _countof(array) (sizeof(array) / (unsigned)sizeof(array[0]))
#endif
#endif

#define CTRLSTATUSMASK          (LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED|SHIFT_PRESSED|APP_PRESSED)

static const struct w32key {
    WORD                wVirtualKeyCode;    /* windows virtual key code */

#define VKMOD_ANY               (0x10000000)
#define VKMOD_META              (0x20000000)
#define VKMOD_CTRL              (0x30000000)
#define VKMOD_SHIFT             (0x40000000)
#define VKMOD_NONSHIFT          (0x50000000)
#define VKMOD_ENHANCED          (0x60000000)
#define VKMOD_NOTENHANCED       (0x70000000)

    DWORD               modifiers;          /* modifiers */
    const wchar_t *     desc;               /* description */

} w32Keys[] = {
    // Only reportsd as an up event, down redirected to event handler.
//  { VK_CANCEL,        MOD_CTRL,           L"Ctrl-Break"       },

    { VK_BACK,          0,                  L"Back"             },
    { VK_TAB,           0,                  L"Tab"              },
    { VK_BACK,          VKMOD_SHIFT,        L"Back"             },
    { VK_TAB,           VKMOD_SHIFT,        L"Tab"              },
    { VK_BACK,          VKMOD_CTRL,         L"Back"             },
    { VK_TAB,           VKMOD_CTRL,         L"Tab"              },
    { VK_BACK,          VKMOD_META,         L"Back"             },
    { VK_TAB,           VKMOD_META,         L"Tab"              },
    { VK_ESCAPE,        VKMOD_ANY,          L"Esc"              },
    { VK_RETURN,        VKMOD_ANY,          L"Return"           },
    { VK_RETURN,        VKMOD_ENHANCED,     L"Keypad-Return"    },
    { VK_PAUSE,         VKMOD_ANY,          L"Keypad-Pause"     },
    { VK_PRIOR,         VKMOD_ANY,          L"PgUp"             },
    { VK_NEXT,          VKMOD_ANY,          L"PgDn"             },
    { VK_END,           VKMOD_ANY,          L"End"              },
    { VK_HOME,          VKMOD_ANY,          L"Home"             },
    { VK_LEFT,          VKMOD_ANY,          L"Left"             },
    { VK_UP,            VKMOD_ANY,          L"Up"               },
    { VK_RIGHT,         VKMOD_ANY,          L"Right"            },
    { VK_DOWN,          VKMOD_ANY,          L"Down"             },
    { VK_INSERT,        VKMOD_ANY,          L"Ins"              },
    { VK_DELETE,        VKMOD_ANY,          L"Delete"           },

    { VK_HELP,          VKMOD_ANY,          L"Help"             },
#if defined(VK_ICO_HELP)
    { VK_ICO_HELP,      VKMOD_ANY,          L"Help"             },
#endif

    { VK_PRIOR,         VKMOD_NOTENHANCED,  L"Keypad-PgUp"      },
    { VK_NEXT,          VKMOD_NOTENHANCED,  L"Keypad-PgDn"      },
    { VK_END,           VKMOD_NOTENHANCED,  L"Keypad-End"       },
    { VK_HOME,          VKMOD_NOTENHANCED,  L"Keypad-Home"      },
    { VK_LEFT,          VKMOD_NOTENHANCED,  L"Keypad-Left"      },
    { VK_CLEAR,         VKMOD_NOTENHANCED,  L"Keypad-5"         },
    { VK_UP,            VKMOD_NOTENHANCED,  L"Keypad-Up"        },
    { VK_RIGHT,         VKMOD_NOTENHANCED,  L"Keypad-Right"     },
    { VK_DOWN,          VKMOD_NOTENHANCED,  L"Keypad-Down"      },
    { VK_INSERT,        VKMOD_NOTENHANCED,  L"Keypad-Ins"       },
    { VK_DELETE,        VKMOD_NOTENHANCED,  L"Keypad-Delete"    },
    { VK_HELP,          VKMOD_NOTENHANCED,  L"Keypad-Help"      },
    { VK_SUBTRACT,      VKMOD_ANY,          L"Keypad-Minus"     },
    { VK_MULTIPLY,      VKMOD_ANY,          L"Keypad-Star"      },
    { VK_ADD,           VKMOD_ANY,          L"Keypad-Plus"      },
    { VK_DIVIDE,        VKMOD_ANY,          L"Keypad-Divide"    },

    /* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
    /* VK_A - VK_Z are the same as ASCII 'A'- 'Z' (0x41 - 0x5A) */

    { 0x30,             VKMOD_CTRL,         L"0"                },
    { 0x31,             VKMOD_CTRL,         L"1"                },
    { 0x32,             VKMOD_CTRL,         L"2"                },
    { 0x33,             VKMOD_CTRL,         L"3"                },
    { 0x34,             VKMOD_CTRL,         L"4"                },
    { 0x35,             VKMOD_CTRL,         L"5"                },
    { 0x36,             VKMOD_CTRL,         L"6"                },
    { 0x37,             VKMOD_CTRL,         L"7"                },
    { 0x38,             VKMOD_CTRL,         L"8"                },
    { 0x39,             VKMOD_CTRL,         L"9"                },

    { VK_F1,            VKMOD_ANY,          L"F1"               },
    { VK_F2,            VKMOD_ANY,          L"F2"               },
    { VK_F3,            VKMOD_ANY,          L"F3"               },
    { VK_F4,            VKMOD_ANY,          L"F4"               },
    { VK_F5,            VKMOD_ANY,          L"F5"               },
    { VK_F6,            VKMOD_ANY,          L"F6"               },
    { VK_F7,            VKMOD_ANY,          L"F7"               },
    { VK_F8,            VKMOD_ANY,          L"F8"               },
    { VK_F9,            VKMOD_ANY,          L"F9"               },
    { VK_F10,           VKMOD_ANY,          L"F10"              },
    { VK_F11,           VKMOD_ANY,          L"F11"              },
    { VK_F12,           VKMOD_ANY,          L"F12"              },
    { VK_F13,           VKMOD_ANY,          L"F13"              },
    { VK_F14,           VKMOD_ANY,          L"F14"              },
    { VK_F15,           VKMOD_ANY,          L"F15"              },
    { VK_F16,           VKMOD_ANY,          L"F16"              },
    { VK_F17,           VKMOD_ANY,          L"F17"              },
    { VK_F18,           VKMOD_ANY,          L"F18"              },
    { VK_F19,           VKMOD_ANY,          L"F19"              },
    { VK_F20,           VKMOD_ANY,          L"F20"              },

    { VK_NUMLOCK,       VKMOD_ANY,          L"Keypad-Numlock"   },
    { VK_SCROLL,        VKMOD_ANY,          L"Keypad-Scroll"    },

    { VK_OEM_PLUS,      VKMOD_NONSHIFT,     L"+"                },
    { VK_OEM_COMMA,     VKMOD_NONSHIFT,     L","                },
    { VK_OEM_MINUS,     VKMOD_NONSHIFT,     L"-"                },
    { VK_OEM_PERIOD,    VKMOD_NONSHIFT,     L"."                },
    { VK_OEM_3,         VKMOD_NONSHIFT,     L"~"                },
    };

#define ISHEX(_uc) \
    ((_uc >= '0' && _uc <= '9') || (_uc >= 'a' && _uc <= 'f') || (_uc >= 'A' && _uc <= 'F') ? 1 : 0)

static void Usage(void);
static void Process(HANDLE in, int rawmode);
static int ReadConsoleInputRaw(HANDLE hConsoleInput, PINPUT_RECORD ir);

static BOOL WINAPI CtrlHandler(DWORD fdwCtrlType);
static int AltPlusEnabled(void);
static int AltPlusEvent(const KEY_EVENT_RECORD *ke, int offset);
static DWORD AltGrEvent(const KEY_EVENT_RECORD* key);

static const wchar_t* wkey_description(const KEY_EVENT_RECORD* ker);
static const wchar_t *wcontrol_state(DWORD dwControlKeyState);
static const wchar_t *wvirtual_description(WORD wVirtualKeyCode);

static int cprinta(const char* fmt, ...);
static int cprintw(const wchar_t* fmt, ...);

static int xf_verbose = 0;
static int xf_rawmode = -1;
static int xf_altcode = -1;                     /* 1=enable,0=disable,-1=auto */
static int xf_mouse = 1;
static int xf_tracking = 0;
static int xf_paste = 1;
static int x_break = 0;

static const char *soptions = "vrRmMtaAh";
static struct option loptions[] = {
    {"verbose",     no_argument, NULL, 'v'},
    {"rawmode",     no_argument, NULL, 'r'},
    {"norawmode",   no_argument, NULL, 'R'},
    {"nomouse",     no_argument, NULL, 'm'},
    {"nomouse",     no_argument, NULL, 'M'},
    {"tracking",    no_argument, NULL, 't'},
    {"altcode",     no_argument, NULL, 'a'},
    {"noaltcode",   no_argument, NULL, 'A'},
    {"help",        no_argument, 0, 'h'},
    {0}
};


int
main(int argc, char **argv)
{
    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode = 0;
    int optidx = 0, c;
    int rawmode = 0;

    // arguments
    xf_tracking = 1000;
    while ((c = getopt_long(argc, (const char * const *)argv, soptions, loptions, &optidx, -1)) != -1) {
        switch (c) {
        case 'v':   // -v,--verbose
            ++xf_verbose;
            break;
        case 'r':   // -r,--rawmode
            xf_rawmode = 1;
            break;
        case 'R':   // -R,--norawmode
            xf_rawmode = 0;
            break;
        case 'm':   // -m,--mouse
            xf_mouse = 1;
            break;
        case 'M':   // -M,--nomouse
            xf_mouse = 0;
            break;
        case 't':   // -t,--tracking
            // 1000 -> only listen to button press and release
            // 1002 -> listen to button press and release + mouse motion only while pressing button
            // 1003 -> listen to button press and release + mouse motion at all times
            if (1000 == xf_tracking)
                xf_tracking = 1002;
            else if (1002 == xf_tracking)
                xf_tracking = 1003;
            break;
        case 'a':   // -a,--altmode
            xf_altcode = 1;
            break;
        case 'A':   // -A,--noaltmode
            xf_altcode = 0;
            break;
        case 'h':   // -h,--help
            Usage();
            return EXIT_FAILURE;
        default:
            assert('?' == c || ':' == c);
            return EXIT_FAILURE;
        }
    }

    if (xf_rawmode == -1) {                     // running under MsTerminal?
        xf_rawmode = (getenv("WT_SESSION") != NULL);
    }

    GetConsoleMode(in, &mode);
    if (xf_rawmode) {
#if !defined(ENABLE_VIRTUAL_TERMINAL_PROCESSING)
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#if !defined(ENABLE_VIRTUAL_TERMINAL_INPUT)
#define ENABLE_VIRTUAL_TERMINAL_INPUT 0x0200
#endif
        DWORD nmode;

        nmode = mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if ((ENABLE_VIRTUAL_TERMINAL_PROCESSING & mode) ||
                    SetConsoleMode(in, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {

            SetConsoleMode(in, (nmode & ~ENABLE_LINE_INPUT) | ENABLE_VIRTUAL_TERMINAL_INPUT);
            cprinta("\x1b[4:3mVT Processing enabled:\x1b[0m\n");
            cprinta("\x1b[?9001h");             // enable win32-input-mode.

            if (xf_mouse) {
                cprinta("\x1b[?%uh", xf_tracking); // enable X11 mouse mode.
                cprinta("\x1b[?1006h");         // enable SGR extended mouse mode.
            }

            if (xf_paste) {
                cprinta("\x1B[?2004s");         // save bracketed paste.
                cprinta("\x1B[?2004h");         // enable bracketed paste.
            }
            rawmode = 1;

        } else {
            cprinta("error: Could not enable VT Processing - please update to a recent Windows 10 build\n");
            return 3;
        }

    } else {
        if (xf_mouse) {                         // mouse enabled.
            if (!SetConsoleMode(in, ENABLE_EXTENDED_FLAGS | \
                        ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT/*|ENABLE_PROCESSED_INPUT*/)) {
                    // Note: Stating ENABLE_EXTENDED_FLAGS disables ENABLE_INSERT_MODE and/or ENABLE_QUICK_EDIT_MODE.
                    //  required for correct mouse operation; restored within sys_shutdown().
                SetConsoleMode(in, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT /*|ENABLE_PROCESSED_INPUT*/);
                    // No extended support/XP.
            }
        } else {
            SetConsoleMode(in, ENABLE_WINDOW_INPUT /*|ENABLE_PROCESSED_INPUT*/);
        }
    }

        // The ENABLE_LINE_INPUT and ENABLE_ECHO_INPUT modes only affect processes that use ReadFile or ReadConsole
        // to read from the console's input buffer. Similarly, the ENABLE_PROCESSED_INPUT mode primarily affects
        // ReadFile and readReadConsole users, except that it also determines whether CTRL+C input is reported in the
        // input buffer (to be read by the ReadConsoleInput function) or is passed to a function defined by
        // the application.
    SetConsoleCtrlHandler(CtrlHandler, TRUE);

    Process(in, rawmode);

    if (rawmode) {
        if (xf_paste) {
            cprinta("\x1B[?2004l");             // disable bracketed paste.
            cprinta("\x1B[?2004r");             // restore bracketed paste.
        }

        if (xf_mouse) {
            cprinta("\x1b[?1006l");             // disable extended mouse mode.
            cprinta("\x1b[?%ul", xf_tracking);  // disable mouse tracking.
        }

        cprinta("\x1b[?9001l");                 // disable win32-input-mode.
    }
    SetConsoleMode(in, mode);

    return 0;
}


static void
Usage(void)
{
    static const char usage[] = {
        "\n" \
        "Options:\n" \
        "   -v,--verbose        increased disgnostics.\n" \
        "\n" \
        "   -r,--rawmode        enable win32-input-mode; default auto\n" \
        "   -R,--norawmode      disable win32-input-mode.\n" \
        "\n" \
        "   -m,--mouse          enable mouse events\n" \
        "   -M,--nomouse        disable mouse events.\n" \
        "   -t,--tracking       extended mouse tracking.\n" \
        "\n" \
        "   -a,--altmode        enable Alt + Numpad mode.\n" \
        "   -A,--noaltmode      disable alt-numpad mode.\n" \
        "\n" \
        "   -h,--help           command line usage.\n" \
        "\n"
        };

    printf("w32keytest [options]\n%s", usage);
}


static void
Process(HANDLE in, int rawmode)
{
    int alt_state = 0, esc_state = 0, event_count = 0;
    DWORD apps_control = 0;
    int offset = 0;

#define COLUMN1 100

    cprinta("\nConsole input test, ESC+ESC to quit.");
    if (xf_altcode < 0) {
        if (0 == xf_altcode ||(xf_altcode < 0 && ! AltPlusEnabled())) {
            cprintw(L"\nAlt-Plus-KeyCode not enabled");
            xf_altcode = 0;
        } else {
            cprintw(L"\nAlt-Plus-KeyCode enabled");
            xf_altcode = 1;
        }
    }

    cprinta("\n\n#     UC     Chr Dir  Rpt VK                SC   State\n");
        //  nnnn  U+uuuu c   1111 111 111111111111 1111 1111 xxxxxxxxxxxxxxx

    for (;;) {
        INPUT_RECORD ir[1] = {0};
        DWORD i, cEventsRead = 0;

        if (rawmode) {
            for (;;) {
                const int ret = ReadConsoleInputRaw(in, ir);
                if (x_break >= 3) {
                    cprinta("\nbye...");
                    return;
                }
                if (ret == 1) {
                    cEventsRead = 1;
                    break;
                } else if (ret == -1) {
                    cprinta("ReadConsoleInput failed!");
                    return;
                }
            }
        } else {
            if (! ReadConsoleInputW(in, ir, 1, &cEventsRead)) {
                cprinta("ReadConsoleInput failed!");
                return;
            }
            if (x_break >= 3) {
                cprinta("\nbye...");
                return;
            }
        }

        for (i = 0; i < cEventsRead; ++i, ++event_count) {
            if (ir[i].EventType & KEY_EVENT) {
                const KEY_EVENT_RECORD *ker = &ir[i].Event.KeyEvent;

                // Special filters

                alt_state = AltPlusEvent(ker, offset);
                if (ker->bKeyDown) {
                    if (VK_APPS == ker->wVirtualKeyCode) {
                        apps_control = APP_PRESSED;
                    }
                } else {
                    if (VK_APPS == ker->wVirtualKeyCode) {
                        apps_control = 0;
                    }
                }

                // Key details

                cprintw(L"\n");
                offset = cprintw(L"%4d: U+%04x %c   %s %03u %04x %-12s %04x %08x %s = %d",
                    event_count, (WORD) ker->uChar.UnicodeChar,
                        (ker->uChar.UnicodeChar > 32 ? ker->uChar.UnicodeChar : ' '),
                        (ker->bKeyDown ? L"down" : L" up "), ker->wRepeatCount,
                    ker->wVirtualKeyCode, wvirtual_description(ker->wVirtualKeyCode), ker->wVirtualScanCode,
                    ker->dwControlKeyState, wcontrol_state(ker->dwControlKeyState|apps_control),
                    alt_state);

                if (ker->bKeyDown) {
                    if (alt_state <= 0) {       // "Alt +" inactive
                        KEY_EVENT_RECORD t_ker = *ker;
                        const wchar_t *kd;

                        t_ker.dwControlKeyState = AltGrEvent(&t_ker);
                        t_ker.dwControlKeyState |= apps_control;
                        if (NULL != (kd = wkey_description(&t_ker))) {
                            cprintw(L"%*s<%s>", COLUMN1 - offset, L"", kd);
                            offset = COLUMN1;
                        }
                    }

                    if (0 == (CTRLSTATUSMASK & ker->dwControlKeyState) &&
                                0x1b == ker->uChar.UnicodeChar) {
                        if (++esc_state >= 2) { // Escape
                            cprinta("\nbye...");
                            return;
                        }
                    } else {
                        esc_state = 0;
                    }
                }

            } else if (ir[i].EventType & MOUSE_EVENT) {
                const MOUSE_EVENT_RECORD* me = &ir[i].Event.MouseEvent;

                cprinta("\n");
                offset = cprinta("%4d: %03u %04u %03u                          %s",
                    event_count, (unsigned)me->dwEventFlags,
                        (unsigned)LOWORD(me->dwButtonState), (unsigned)me->dwControlKeyState,
                    mouse_description(me));
            }
        }
    }
}


static int
ReadConsoleInputRaw(HANDLE hConsoleInput, PINPUT_RECORD ir)
{
    static BYTE cached[128] = {0};
    static unsigned cache_length = 0;

    BYTE buffer[128];
    DWORD timeout = INFINITE;
    unsigned length = 0;
    WCHAR ch = 0;

    for (;;) {
        for (;;) {
            DWORD dwRead = 0;

            memset(ir, 0, sizeof(*ir));
            if (WaitForSingleObject(hConsoleInput, timeout) != WAIT_OBJECT_0 ||
                        ! ReadConsoleInputA(hConsoleInput, ir, 1, &dwRead) || 1 != dwRead) {
                cprinta("\nerror: i/o error (timeout=%u, error=%u, read=%u)\n",
                    timeout, GetLastError(), dwRead);
                break;
            }

            if (ir->EventType == KEY_EVENT) {
                KEY_EVENT_RECORD *ker = &ir->Event.KeyEvent;

                if (xf_verbose >= 2) {
                    cprintw(L"  RAW %4d: U+%04x %c %s %03u %04x %04x %08x\n",
                        length, (WORD) ker->uChar.UnicodeChar,
                            (ker->uChar.UnicodeChar > 32 ? ker->uChar.UnicodeChar : ' '),
                        (ker->bKeyDown ? L"down" : L" up "),
                        ker->wRepeatCount, ker->wVirtualKeyCode, ker->wVirtualScanCode, ker->dwControlKeyState);
                }

                if (ker->bKeyDown) { // down event
                    ch = ker->uChar.UnicodeChar;
                    break;
                } else {
                    if (1 == length) { // true ESC, return
                        if (ker->uChar.UnicodeChar == 0x1b) {
                            ker->bKeyDown = 1;
                            return TRUE;
                        }
                    }
                }
            }
        }

        if (0 == length) { // first character
            if (ch == '\x1b') {
                buffer[length++] = 0x1b;
                timeout = 500;
                continue;
            }
            return TRUE;
        }

        if (ch < ' ' || ch > 0xff) {
            cprinta("\nerror: unexpected character 0X%x/'%c'\n", ch, isprint(ch) ? ch : ' ');
            break;
        }

        if (1 == length) { // second character
            if (ch == '[') {
                buffer[length++] = (char)ch;
                timeout = 100;
                continue;
            }
            cprinta("\nerror: expected character 0X%x/'%c'\n", ch, isprint(ch) ? ch : ' ');
            break;
        }

        if (length == (_countof(buffer) - 1)) {
            cprinta("\nerror: buffer overflow <%.*s>.\n", length, buffer);
            break;
        }

        buffer[length++] = (char)ch;
        if (ch != '_') {
            continue;
        }
        buffer[length] = 0;

        //  https://github.com/microsoft/terminal/blob/main/doc/specs/%234999%20-%20Improved%20keyboard%20handling%20in%20Conpty.md
        //
        //  ^ [[ Vk; Sc; Uc; Kd; Cs; Rc _
        //
        //      Vk : the value of wVirtualKeyCode - any number. If omitted, defaults to '0'.
        //      Sc : the value of wVirtualScanCode - any number. If omitted, defaults to '0'.
        //      Uc : the decimal value of UnicodeChar - for example, NUL is "0", LF is
        //          "10", the character 'A' is "65". If omitted, defaults to '0'.
        //      Kd : the value of bKeyDown - either a '0' or '1'. If omitted, defaults to '0'.
        //      Cs : the value of dwControlKeyState - any number. If omitted, defaults to '0'.
        //      Rc : the value of wRepeatCount - any number. If omitted, defaults to '1'.
        //
        {
            unsigned arguments[6] = { 0, 0, 0, 0, 0, 1 };

            if (NULL == DecodeKeyArguments(arguments, _countof(arguments), '_', buffer + 2, buffer + length)) {
                const BYTE *cursor, *end;

                cprinta("\nerror: buffer format, length=%u [", length);
                for (cursor = buffer, end = cursor + length; cursor != end; ++cursor) {
                    cprinta("0x%x/%c", *cursor, isprint(*cursor) ? *cursor: ' ');
                }
                cprinta("] ==> %u;%u;%u;%u;%lu;%u_\n", arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
                break;
            }

            if (0 == arguments[0] && 0 == arguments[1] && arguments[2] == 0x1b) {
                if (arguments[3]) {
                    // ESC (special VK=SC=0) + down
                    cache_length = 0;
                    cached[cache_length++] = 0x1b;
                    if (xf_verbose) {
                        cprinta("\nESC: [%s] ==> %u;%u;%u;%u;%lu;%u_",
                            buffer + 2, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
                        cprinta(": %u, <\\x1b%.*s>", cache_length, cache_length - 1, cached + 1);
                    }
                    return FALSE;
                }
            }

            if (cache_length) {
                if (arguments[2] && arguments[3]) {
                    // unicode + down
                    cached[cache_length++] = (BYTE)arguments[2];
                    if (xf_verbose) {
                        cprinta("\nADD: [%s] ==> %u;%u;%u;%u;%lu;%u_",
                            buffer + 2, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
                        cprinta(": %u, <\\x1b%.*s>", cache_length, cache_length - 1, cached + 1);
                    }
                }

                if (cache_length >= 6) {
                    if (cached[1] == '[' && cached[2] == 'M') {
                        //
                        //  \x1B[Mabc", where:
                        //     a:  Button number plus 32.
                        //     b:  Column number (one-based) plus 32.
                        //     c:  Row number (one-based) plus 32.
                        //
                        const void* end = cached + cache_length;

                        cache_length = 0;
                        return (DecodeXTermMouse(ir, cached, end) != NULL);

                    } else if (cached[1] == '[' && cached[2] == '<') {
                        //
                        //  \x1B[<B;Px;PyM":
                        //     B:  Button event.
                        //     Px: Column number.
                        //     Py: Row number.
                        //     M:  M=press or m=release.
                        //
                        if (cached[cache_length - 1] == 'M' || cached[cache_length - 1] == 'm') {
                            const void *end = cached + cache_length;

                            cache_length = 0;
                            return (DecodeSGRMouse(ir, cached, end) != NULL);
                        }

                    } else if (0 == memcmp(cached, "\x1b[200~", 6)) {
                        //
                        //  Bracketed Paste Mode
                        //      When bracketed paste mode is set, pasted text is bracketed with control sequences so that the program
                        //      can differentiate pasted text from typed - in text.When bracketed paste mode is set, the program will receive :
                        //
                        //          ESC[200~, <text>, ESC[201~
                        //
                        cache_length = 0;

                    } else if (0 == memcmp(cached, "\x1b[201~", 6)) {
                        cache_length = 0;

                    } else {
                        const BYTE *cursor, *end;

                        cprinta("\nerror: unexpected escape, length=%u [", cache_length);
                        for (cursor = cached, end = cursor + cache_length; cursor != end; ++cursor) {
                            cprinta("0x%x/%c", *cursor, isprint(*cursor) ? *cursor : ' ');
                        }
                        cprinta("]\n");
                        return FALSE;
                    }
                }

                if (0 == arguments[2] || 0 == arguments[3])
                    goto ignored;                   // non-unicode or up
                return FALSE;
            }

            if (xf_verbose) {
                cprinta("\nKEY: [%s] ==> %u;%u;%u;%u;%lu;%u_",
                    buffer + 2, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
            }

            {
                KEY_EVENT_RECORD* ke = &ir->Event.KeyEvent;
                ke->wVirtualKeyCode = (WORD)arguments[0];
                ke->wVirtualScanCode = (WORD)arguments[1];
                ke->uChar.UnicodeChar = (WCHAR)arguments[2];
                ke->bKeyDown = (arguments[3] ? TRUE : FALSE);
                ke->dwControlKeyState = (DWORD)arguments[4];
                ke->wRepeatCount = (WORD)arguments[5];
                ir->EventType = KEY_EVENT;
            }
            return TRUE;

        ignored:;
            if (xf_verbose) {
                cprinta("\nNUL: [%s] ==> %u;%u;%u;%u;%lu;%u_",
                    buffer + 2, arguments[0], arguments[1], arguments[2], arguments[3], arguments[4], arguments[5]);
            }
            return FALSE;
        }
    }
    return -1;
}


static BOOL WINAPI
CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType) {
    case CTRL_C_EVENT:      // Ctrl-C signal.
        cprinta("\nCtrl-C event\n");
        return TRUE;
    case CTRL_BREAK_EVENT:  // Ctrl-Break signal.
        cprinta("\nCtrl-Break event\n");
        ++x_break;
        return TRUE;
    case CTRL_CLOSE_EVENT:  // Ctrl-Close: confirm that the user wants to exit.
        cprinta("\nCtrl-Close event\n");
        Beep(600, 200);
        return TRUE;
    case CTRL_LOGOFF_EVENT:
        cprinta("\nCtrl-Logoff event\n");
        Beep(1000, 200);
        return FALSE;
    case CTRL_SHUTDOWN_EVENT:
        cprinta("\nCtrl-Shutdown event\n");
        Beep(750, 500);
        return FALSE;
    default:
        return FALSE;
    }
}


//  Alt+<key-code> event handler
//
//      Alt+KeyCode works and behaves well when character only input is required, by simply
//      reporting any down or up key events which populate the 'UnicodeChar' value. Whereas
//      when extended keystroke handling is required, for example arrow and numpad keys,
//      additional effort is needed.
//
//      Alt+Keycodes are only reported within the 'UnicodeChar' value of up event on a "ALT" key
//      post the valid entry of one-or-more hex characters. During KeyCode entry the API unfortunately
//      does not publiciy indicate this state plus continues to return the associated virtual keys,
//      including the leading 'keypad-plus' and any associated key-code elements, wherefore we need
//      to filter.  Furthermore, if during the key-code entry an invalid non-hex key combination is
//      given, the key-code is invalidated and UnicodeChar=0 is returned on the ALT release.
//
//      Notes:
//       o To enable requires the registry REG_SZ value "EnableHexNumpad" under
//          "HKEY_Current_User/Control Panel/Input Method" to be "1".
//
//       o Hex-value overflow goes unreported, limiting input to a 16-bit unicode result.
//

#pragma comment(lib, "Imm32.lib")

static int
AltPlusEnabled(void)
{
    static int state = -1;

    if (-1 == state) {
        HKEY hKey = 0;

        state = 0;
        if (RegOpenKeyExA(HKEY_CURRENT_USER,
                "Control Panel\\Input Method", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            char szEnableHexNumpad[100] = { 0 };
            DWORD dwSize = _countof(szEnableHexNumpad);
            if (RegQueryValueExA(hKey, "EnableHexNumpad", NULL, NULL, (LPBYTE)szEnableHexNumpad, &dwSize) == ERROR_SUCCESS) {
                if (szEnableHexNumpad[0] == '1' && szEnableHexNumpad[1] == 0) {
                    state = 1;
                }
            }
            RegCloseKey(hKey);
        }
    }
    return state;
}


static int
AltPlusEvent(const KEY_EVENT_RECORD* ke, /*struct IOEvent* evt*/ int offset)
{
#define ISXDIGIT(_uc) \
            ((_uc >= '0' && _uc <= '9') || (_uc >= 'a' && _uc <= 'f') || (_uc >= 'A' && _uc <= 'F') ? 1 : 0)

    static unsigned alt_code = 0;               // >0=active
    static DWORD alt_control = 0;

    const unsigned wVirtualKeyCode = ke->wVirtualKeyCode,
        dwControlKeyState = (CTRLSTATUSMASK & ke->dwControlKeyState),
        dwEnhanced = (ENHANCED_KEY & ke->dwControlKeyState);

    wchar_t completion[64] = { 0 };
    completion[0] = 0;

    if (xf_verbose >= 2) {
        cprinta(" Key: %s-%s%s%s%s%s%sVK=0x%02x/%u, UC=0x%04x/%u/%c, SC=0x%x/%u\n",
            (ke->bKeyDown ? "DN" : "UP"),
                (dwEnhanced ? "Enh-" : ""),
                (dwControlKeyState & LEFT_ALT_PRESSED) ? "LAlt-" : "",
                (dwControlKeyState & RIGHT_ALT_PRESSED) ? "RAlt-" : "",
                (dwControlKeyState & LEFT_CTRL_PRESSED) ? "LCtrl-" : "",
                (dwControlKeyState & RIGHT_CTRL_PRESSED) ? "RCtrl-" : "",
                (dwControlKeyState & SHIFT_PRESSED) ? "Shift-" : "",
            wVirtualKeyCode, wVirtualKeyCode,
            ke->uChar.UnicodeChar, ke->uChar.UnicodeChar,
            (ke->uChar.UnicodeChar && ke->uChar.UnicodeChar < 255 ? ke->uChar.UnicodeChar : ' '),
            ke->wVirtualScanCode, ke->wVirtualScanCode);
    }

    if (ke->bKeyDown) {                         // down event

        if ((wVirtualKeyCode >= VK_NUMPAD0 && wVirtualKeyCode <= VK_NUMPAD9) &&
                0 == dwEnhanced && (LEFT_ALT_PRESSED == dwControlKeyState || RIGHT_ALT_PRESSED == dwControlKeyState)) {
            // "Alt NumPad ..." event
            // Note:
            //  o NumLock is required to be enabled for NUMPAD key reporting.
            //  o Non-enhanced, NumPad verses 101 cursor-keys; same VK codes.
            if (0 == alt_code) {
                alt_control = dwControlKeyState;
                alt_code = 1;
            }
            return 1;                           // consume
        }

        if (VK_ADD == wVirtualKeyCode &&
                (LEFT_ALT_PRESSED == dwControlKeyState || RIGHT_ALT_PRESSED == dwControlKeyState)) {
            // "Alt + Hex" event
            if (0 == AltPlusEnabled())
                return -1;                      // enabled?
            if (0 == alt_code) {
                alt_control = dwControlKeyState;
                alt_code = 0x101;
            }
            return 1;                           // consume
        }

        if (alt_code) {
            if (alt_control != dwControlKeyState ||
                    (ke->uChar.UnicodeChar && 0 == ISXDIGIT(ke->uChar.UnicodeChar))) {
                // new control status or non-hex, emit "Alt-Plus" and reset state
                // TODO/XXX - or should these consumed
                swprintf(completion, _countof(completion), L"Alt-Plus (error)");
                alt_code = 0;

            } else {
                ++alt_code;                     // associated key count
                return 1;                       // consume
            }
        }

    } else if (alt_code) {                      // up event
        if (VK_MENU == wVirtualKeyCode &&
                (0 == (ke->dwControlKeyState & ALT_PRESSED))) {
            // Alt completion
            const WCHAR UnicodeChar = ke->uChar.UnicodeChar;
            const int alt_old = alt_code;

            alt_code = 0;
            if (1 == (alt_old & 0xff) && 0 == UnicodeChar) {
                // Alt only, emit.
                swprintf(completion, _countof(completion), L"Alt-Plus");

            } else if (UnicodeChar) {
                // Alt keycode, return keycode.
                if (alt_old & 0x100) {
                    swprintf(completion, _countof(completion), L"Alt-Plus-#0x%x", UnicodeChar);
                } else {
                    swprintf(completion, _countof(completion), L"Alt-#%d", UnicodeChar);
                }
            }
        }
    }

    if (completion[0]) {                        // completion
        cprintw(L"%*s<%s>", COLUMN1 - offset, L"", completion);
        return 0;
    }

    return -1;                                  // unhandled
}


/*  Function:           AltGrEvent
 *      Filter AtrGr events from modifiers; attempt to allow:
 *
 *          Left-Alt + AltGr,
 *          Right-Ctrl + AltGr,
 *          Left-Alt + Right-Ctrl + AltGr.
 */
static DWORD
AltGrEvent(const KEY_EVENT_RECORD* key)
{
    DWORD state = key->dwControlKeyState;

    // AltGr condition (LCtrl + RAlt)
    if (0 == (state & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)))
        return state;

    if (0 == (state & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)))
        return state;

    if (0 == key->uChar.UnicodeChar)
        return state;

    if (state & RIGHT_ALT_PRESSED) {
        // Remove Right-Alt.
        state &= ~RIGHT_ALT_PRESSED;

        // As a character was presented, Left-Ctrl is almost always set,
        // except if the user presses Right-Ctrl, then AltGr (in that specific order) for whatever reason.
        // At any rate, make sure the bit is not set.
        state &= ~LEFT_CTRL_PRESSED;

    } else if (state & LEFT_ALT_PRESSED) {
        // Remove Left-Alt.
        state &= ~LEFT_ALT_PRESSED;

        // Whichever Ctrl key is down, remove it from the state.
        // We only remove one key, to improve our chances of detecting the corner-case of Left-Ctrl + Left-Alt + Right-Ctrl.
        if ((state & LEFT_CTRL_PRESSED) != 0) {
            // Remove Left-Ctrl.
            state &= ~LEFT_CTRL_PRESSED;

        } else if ((state & RIGHT_CTRL_PRESSED) != 0) {
            // Remove Right-Ctrl.
            state &= ~RIGHT_CTRL_PRESSED;
        }
    }
    return state;
}



static const wchar_t *
wkey_description(const KEY_EVENT_RECORD *ker)
{
    static wchar_t t_buffer[200];
    wchar_t *cursor = t_buffer, *end = cursor + _countof(t_buffer);

    const DWORD dwControlKeyState = ker->dwControlKeyState;
    const WORD wVirtualKeyCode = ker->wVirtualKeyCode;
    const struct w32key *key = w32Keys + _countof(w32Keys);

    // Virtual key mapping
    while (--key >= w32Keys) {
        if (key->wVirtualKeyCode == wVirtualKeyCode) {
            const DWORD modifiers = key->modifiers;
            if (VKMOD_ANY == modifiers) {
                break;
            } else if (VKMOD_SHIFT == modifiers) {
                if (0 != (dwControlKeyState & SHIFT_PRESSED)) {
                    break;
                }
            } else if (VKMOD_NONSHIFT == modifiers) {
                if (0 == (dwControlKeyState & SHIFT_PRESSED)) {
                    break;
                }
            } else if (VKMOD_ENHANCED == modifiers) {
                if (0 != (dwControlKeyState & ENHANCED_KEY)) {
                    break;
                }
            } else if (VKMOD_NOTENHANCED == modifiers) {
                if (0 == (dwControlKeyState & ENHANCED_KEY)) {
                    break;
                }
            } else if (0 == modifiers || (dwControlKeyState & modifiers)) {
                break;
            }
        }
    }
    if (key < w32Keys) {
        if (0 == ker->uChar.UnicodeChar) {
            return NULL;
        }
        key = NULL;
    }

    // Control states
    if (dwControlKeyState & APP_PRESSED) // special
        cursor += swprintf(cursor, end - cursor, L"App-");

    if (dwControlKeyState & ALT_PRESSED)
        cursor += swprintf(cursor, end - cursor, L"Alt-");

    if (dwControlKeyState & CTRL_PRESSED)
        cursor += swprintf(cursor, end - cursor, L"Ctrl-");

    if (dwControlKeyState & SHIFT_PRESSED)
        cursor += swprintf(cursor, end - cursor, L"Shift-");

    // Key code
    if (key) {
        cursor += swprintf(cursor, end - cursor, L"%s", key->desc);

    } else if (ker->uChar.UnicodeChar) {
        const WORD uc = ker->uChar.UnicodeChar;
        const wchar_t *desc = NULL;

        switch (uc) {
        case '\b': desc = L"Backspace"; break;
        case '\r': desc = L"Enter"; break;
        case '\t': desc = L"Tab"; break;
        case 0x1b: desc = L"Esc"; break;
        case ' ':
            if (dwControlKeyState & CTRLSTATUSMASK) {
                desc = L"Space";
            }
            break;
        }

        if (desc) {
            cursor += swprintf(cursor, end - cursor, desc);
        } else {
            if (uc > ' ' && uc < 0xff) {        // ASCII
                if ((dwControlKeyState & CTRLSTATUSMASK) && uc >= 'a' && uc <= 'z') {
                    *cursor++ = uc - 'a' + 'A'; // upper-case meta controls.
                } else {
                    *cursor++ = uc;
                }
                *cursor++ = 0;

            } else if (uc < ' ' && (dwControlKeyState & CTRL_PRESSED)) {
                *cursor++ = ('A' - 1) + uc;     // controls
                *cursor++ = 0;

            } else {                            // Unicode
                cursor += swprintf(cursor, end - cursor, L"#%u", uc);
            }
        }
    }

    return (cursor > t_buffer ? t_buffer :  NULL);
}


static const wchar_t *
wcontrol_state(DWORD dwControlKeyState)
{
    static wchar_t t_buffer[200];
    wchar_t *cursor = t_buffer;

    t_buffer[0] = 0;
    if (dwControlKeyState & CAPSLOCK_ON)        wcscpy(cursor, L"CapLk,"),    cursor += wcslen(cursor);
    if (dwControlKeyState & ENHANCED_KEY)       wcscpy(cursor, L"Enhanced,"), cursor += wcslen(cursor);
    if (dwControlKeyState & LEFT_ALT_PRESSED)   wcscpy(cursor, L"LeftAlt,"),  cursor += wcslen(cursor);
    if (dwControlKeyState & LEFT_CTRL_PRESSED)  wcscpy(cursor, L"LeftCtl,"),  cursor += wcslen(cursor);
    if (dwControlKeyState & NUMLOCK_ON)         wcscpy(cursor, L"NumLck,"),   cursor += wcslen(cursor);
    if (dwControlKeyState & RIGHT_ALT_PRESSED)  wcscpy(cursor, L"RightAlt,"), cursor += wcslen(cursor);
    if (dwControlKeyState & RIGHT_CTRL_PRESSED) wcscpy(cursor, L"RightCtl,"), cursor += wcslen(cursor);
    if (dwControlKeyState & SCROLLLOCK_ON)      wcscpy(cursor, L"ScrLk,"),    cursor += wcslen(cursor);
    if (dwControlKeyState & SHIFT_PRESSED)      wcscpy(cursor, L"Shift,"),    cursor += wcslen(cursor);
    if (dwControlKeyState & APP_PRESSED)        wcscpy(cursor, L"App,"),      cursor += wcslen(cursor);

    if (cursor > t_buffer) cursor[-1] = 0;
    return t_buffer;
}


static const wchar_t *
wvirtual_description(WORD wVirtualKeyCode)
{
    switch (wVirtualKeyCode) {
    case VK_LBUTTON             :  /*0x01*/ return L"LBUTTON";
    case VK_RBUTTON             :  /*0x02*/ return L"RBUTTON";
    case VK_CANCEL              :  /*0x03*/ return L"CANCEL";
    case VK_MBUTTON             :  /*0x04*/ return L"MBUTTON";

    case VK_XBUTTON1            :  /*0x05*/ return L"XBUTTON1";
    case VK_XBUTTON2            :  /*0x06*/ return L"XBUTTON2";

    case VK_BACK                :  /*0x08*/ return L"BACK";
    case VK_TAB                 :  /*0x09*/ return L"TAB";

    case VK_CLEAR               :  /*0x0C*/ return L"CLEAR";
    case VK_RETURN              :  /*0x0D*/ return L"RETURN";

    case VK_SHIFT               :  /*0x10*/ return L"SHIFT";
    case VK_CONTROL             :  /*0x11*/ return L"CONTROL";
    case VK_MENU                :  /*0x12*/ return L"MENU";
    case VK_PAUSE               :  /*0x13*/ return L"PAUSE";
    case VK_CAPITAL             :  /*0x14*/ return L"CAPITAL";

    case VK_KANA                :  /*0x15*/ return L"KANA";
  //case VK_HANGEUL             :  /*0x15*/ return L"HANGEUL";
  //case VK_HANGUL              :  /*0x15*/ return L"HANGUL";
    case VK_JUNJA               :  /*0x17*/ return L"JUNJA";
    case VK_FINAL               :  /*0x18*/ return L"FINAL";
    case VK_HANJA               :  /*0x19*/ return L"HANJA";
  //case VK_KANJI               :  /*0x19*/ return L"KANJI";

    case VK_ESCAPE              :  /*0x1B*/ return L"ESCAPE";

    case VK_CONVERT             :  /*0x1C*/ return L"CONVERT";
    case VK_NONCONVERT          :  /*0x1D*/ return L"NONCONVERT";
    case VK_ACCEPT              :  /*0x1E*/ return L"ACCEPT";
    case VK_MODECHANGE          :  /*0x1F*/ return L"MODECHANGE";

    case VK_SPACE               :  /*0x20*/ return L"SPACE";
    case VK_PRIOR               :  /*0x21*/ return L"PRIOR";
    case VK_NEXT                :  /*0x22*/ return L"NEXT";
    case VK_END                 :  /*0x23*/ return L"END";
    case VK_HOME                :  /*0x24*/ return L"HOME";
    case VK_LEFT                :  /*0x25*/ return L"LEFT";
    case VK_UP                  :  /*0x26*/ return L"UP";
    case VK_RIGHT               :  /*0x27*/ return L"RIGHT";
    case VK_DOWN                :  /*0x28*/ return L"DOWN";
    case VK_SELECT              :  /*0x29*/ return L"SELECT";
    case VK_PRINT               :  /*0x2A*/ return L"PRINT";
    case VK_EXECUTE             :  /*0x2B*/ return L"EXECUTE";
    case VK_SNAPSHOT            :  /*0x2C*/ return L"SNAPSHOT";
    case VK_INSERT              :  /*0x2D*/ return L"INSERT";
    case VK_DELETE              :  /*0x2E*/ return L"DELETE";
    case VK_HELP                :  /*0x2F*/ return L"HELP";

    case VK_LWIN                :  /*0x5B*/ return L"LWIN";
    case VK_RWIN                :  /*0x5C*/ return L"RWIN";
    case VK_APPS                :  /*0x5D*/ return L"APPS";

    case VK_SLEEP               :  /*0x5F*/ return L"SLEEP";

    case VK_NUMPAD0             :  /*0x60*/ return L"NUMPAD0";
    case VK_NUMPAD1             :  /*0x61*/ return L"NUMPAD1";
    case VK_NUMPAD2             :  /*0x62*/ return L"NUMPAD2";
    case VK_NUMPAD3             :  /*0x63*/ return L"NUMPAD3";
    case VK_NUMPAD4             :  /*0x64*/ return L"NUMPAD4";
    case VK_NUMPAD5             :  /*0x65*/ return L"NUMPAD5";
    case VK_NUMPAD6             :  /*0x66*/ return L"NUMPAD6";
    case VK_NUMPAD7             :  /*0x67*/ return L"NUMPAD7";
    case VK_NUMPAD8             :  /*0x68*/ return L"NUMPAD8";
    case VK_NUMPAD9             :  /*0x69*/ return L"NUMPAD9";
    case VK_MULTIPLY            :  /*0x6A*/ return L"MULTIPLY";
    case VK_ADD                 :  /*0x6B*/ return L"ADD";
    case VK_SEPARATOR           :  /*0x6C*/ return L"SEPARATOR";
    case VK_SUBTRACT            :  /*0x6D*/ return L"SUBTRACT";
    case VK_DECIMAL             :  /*0x6E*/ return L"DECIMAL";
    case VK_DIVIDE              :  /*0x6F*/ return L"DIVIDE";
    case VK_F1                  :  /*0x70*/ return L"F1";
    case VK_F2                  :  /*0x71*/ return L"F2";
    case VK_F3                  :  /*0x72*/ return L"F3";
    case VK_F4                  :  /*0x73*/ return L"F4";
    case VK_F5                  :  /*0x74*/ return L"F5";
    case VK_F6                  :  /*0x75*/ return L"F6";
    case VK_F7                  :  /*0x76*/ return L"F7";
    case VK_F8                  :  /*0x77*/ return L"F8";
    case VK_F9                  :  /*0x78*/ return L"F9";
    case VK_F10                 :  /*0x79*/ return L"F10";
    case VK_F11                 :  /*0x7A*/ return L"F11";
    case VK_F12                 :  /*0x7B*/ return L"F12";
    case VK_F13                 :  /*0x7C*/ return L"F13";
    case VK_F14                 :  /*0x7D*/ return L"F14";
    case VK_F15                 :  /*0x7E*/ return L"F15";
    case VK_F16                 :  /*0x7F*/ return L"F16";
    case VK_F17                 :  /*0x80*/ return L"F17";
    case VK_F18                 :  /*0x81*/ return L"F18";
    case VK_F19                 :  /*0x82*/ return L"F19";
    case VK_F20                 :  /*0x83*/ return L"F20";
    case VK_F21                 :  /*0x84*/ return L"F21";
    case VK_F22                 :  /*0x85*/ return L"F22";
    case VK_F23                 :  /*0x86*/ return L"F23";
    case VK_F24                 :  /*0x87*/ return L"F24";

    case VK_NUMLOCK             :  /*0x90*/ return L"NUMLOCK";
    case VK_SCROLL              :  /*0x91*/ return L"SCROLL";

#if defined(VK_OEM_NEC_EQUAL)
    case VK_OEM_NEC_EQUAL       :  /*0x92*/ return L"OEM_NEC_EQUAL";
#endif

#if defined(VK_OEM_FJ_MASSHOU)
  //case VK_OEM_FJ_JISHO        :  /*0x92*/ return L"OEM_FJ_JISHO";
    case VK_OEM_FJ_MASSHOU      :  /*0x93*/ return L"OEM_FJ_MASSHOU";
    case VK_OEM_FJ_TOUROKU      :  /*0x94*/ return L"OEM_FJ_TOUROKU";
    case VK_OEM_FJ_LOYA         :  /*0x95*/ return L"OEM_FJ_LOYA";
    case VK_OEM_FJ_ROYA         :  /*0x96*/ return L"OEM_FJ_ROYA";
#endif

    case VK_LSHIFT              :  /*0xA0*/ return L"LSHIFT";
    case VK_RSHIFT              :  /*0xA1*/ return L"RSHIFT";
    case VK_LCONTROL            :  /*0xA2*/ return L"LCONTROL";
    case VK_RCONTROL            :  /*0xA3*/ return L"RCONTROL";
    case VK_LMENU               :  /*0xA4*/ return L"LMENU";
    case VK_RMENU               :  /*0xA5*/ return L"RMENU";

    case VK_BROWSER_BACK        :  /*0xA6*/ return L"BROWSER_BACK";
    case VK_BROWSER_FORWARD     :  /*0xA7*/ return L"BROWSER_FORWARD";
    case VK_BROWSER_REFRESH     :  /*0xA8*/ return L"BROWSER_REFRESH";
    case VK_BROWSER_STOP        :  /*0xA9*/ return L"BROWSER_STOP";
    case VK_BROWSER_SEARCH      :  /*0xAA*/ return L"BROWSER_SEARCH";
    case VK_BROWSER_FAVORITES   :  /*0xAB*/ return L"BROWSER_FAVORITES";
    case VK_BROWSER_HOME        :  /*0xAC*/ return L"BROWSER_HOME";

    case VK_VOLUME_MUTE         :  /*0xAD*/ return L"VOLUME_MUTE";
    case VK_VOLUME_DOWN         :  /*0xAE*/ return L"VOLUME_DOWN";
    case VK_VOLUME_UP           :  /*0xAF*/ return L"VOLUME_UP";
    case VK_MEDIA_NEXT_TRACK    :  /*0xB0*/ return L"MEDIA_NEXT_TRACK";
    case VK_MEDIA_PREV_TRACK    :  /*0xB1*/ return L"MEDIA_PREV_TRACK";
    case VK_MEDIA_STOP          :  /*0xB2*/ return L"MEDIA_STOP";
    case VK_MEDIA_PLAY_PAUSE    :  /*0xB3*/ return L"MEDIA_PLAY_PAUSE";
    case VK_LAUNCH_MAIL         :  /*0xB4*/ return L"LAUNCH_MAIL";
    case VK_LAUNCH_MEDIA_SELECT :  /*0xB5*/ return L"LAUNCH_MEDIA_SELECT";
    case VK_LAUNCH_APP1         :  /*0xB6*/ return L"LAUNCH_APP1";
    case VK_LAUNCH_APP2         :  /*0xB7*/ return L"LAUNCH_APP2";

#if defined(VK_OEM_1)
    case VK_OEM_1               :  /*0xBA*/ return L"OEM_1";
    case VK_OEM_PLUS            :  /*0xBB*/ return L"OEM_PLUS";
    case VK_OEM_COMMA           :  /*0xBC*/ return L"OEM_COMMA";
    case VK_OEM_MINUS           :  /*0xBD*/ return L"OEM_MINUS";
    case VK_OEM_PERIOD          :  /*0xBE*/ return L"OEM_PERIOD";
    case VK_OEM_2               :  /*0xBF*/ return L"OEM_2";
    case VK_OEM_3               :  /*0xC0*/ return L"OEM_3";
#endif

#if defined(VK_OEM_4)
    case VK_OEM_4               :  /*0xDB*/ return L"OEM_4";
    case VK_OEM_5               :  /*0xDC*/ return L"OEM_5";
    case VK_OEM_6               :  /*0xDD*/ return L"OEM_6";
    case VK_OEM_7               :  /*0xDE*/ return L"OEM_7";
    case VK_OEM_8               :  /*0xDF*/ return L"OEM_8";
#endif

#if defined(VK_OEM_AX)
    case VK_OEM_AX              :  /*0xE1*/ return L"OEM_AX";
    case VK_OEM_102             :  /*0xE2*/ return L"OEM_102";
    case VK_ICO_HELP            :  /*0xE3*/ return L"ICO_HELP";
    case VK_ICO_00              :  /*0xE4*/ return L"ICO_00";
#endif

    case VK_PROCESSKEY          :  /*0xE5*/ return L"PROCESSKEY";

#if defined(VK_ICO_CLEAR)
    case VK_ICO_CLEAR           :  /*0xE6*/ return L"ICO_CLEAR";
#endif

    case VK_PACKET              :  /*0xE7*/ return L"PACKET";

#if defined(VK_OEM_RESET)
    case VK_OEM_RESET           :  /*0xE9*/ return L"OEM_RESET";
    case VK_OEM_JUMP            :  /*0xEA*/ return L"OEM_JUMP";
    case VK_OEM_PA1             :  /*0xEB*/ return L"OEM_PA1";
    case VK_OEM_PA2             :  /*0xEC*/ return L"OEM_PA2";
    case VK_OEM_PA3             :  /*0xED*/ return L"OEM_PA3";
    case VK_OEM_WSCTRL          :  /*0xEE*/ return L"OEM_WSCTRL";
    case VK_OEM_CUSEL           :  /*0xEF*/ return L"OEM_CUSEL";
    case VK_OEM_ATTN            :  /*0xF0*/ return L"OEM_ATTN";
    case VK_OEM_FINISH          :  /*0xF1*/ return L"OEM_FINISH";
    case VK_OEM_COPY            :  /*0xF2*/ return L"OEM_COPY";
    case VK_OEM_AUTO            :  /*0xF3*/ return L"OEM_AUTO";
    case VK_OEM_ENLW            :  /*0xF4*/ return L"OEM_ENLW";
    case VK_OEM_BACKTAB         :  /*0xF5*/ return L"OEM_BACKTAB";
#endif

    case VK_ATTN                :  /*0xF6*/ return L"ATTN";
    case VK_CRSEL               :  /*0xF7*/ return L"CRSEL";
    case VK_EXSEL               :  /*0xF8*/ return L"EXSEL";
    case VK_EREOF               :  /*0xF9*/ return L"EREOF";
    case VK_PLAY                :  /*0xFA*/ return L"PLAY";
    case VK_ZOOM                :  /*0xFB*/ return L"ZOOM";
    case VK_NONAME              :  /*0xFC*/ return L"NONAME";
    case VK_PA1                 :  /*0xFD*/ return L"PA1";
    case VK_OEM_CLEAR           :  /*0xFE*/ return L"OEM_CLEAR";

    default:
        return L"";
    }
}


/*
 *  Console formatted output
 */
static int
cprinta(const char* fmt, ...)
{
    HANDLE cout = GetStdHandle(STD_OUTPUT_HANDLE);
    char* cursor, * nl;
    char buffer[512];
    int total, len;
    va_list ap;

    va_start(ap, fmt);
    total = len = vsnprintf(buffer, _countof(buffer), fmt, ap);
    buffer[_countof(buffer) - 1] = 0;
    va_end(ap);

    cursor = buffer;
    while ((nl = strchr(cursor, '\n')) != NULL) {
        const int part = (nl - cursor) + 1;
        WriteConsoleA(cout, cursor, part - 1, NULL, NULL);
        WriteConsoleA(cout, "\n\r", 2, NULL, NULL);
        cursor += part;
        len -= part;
    }
    WriteConsoleA(cout, cursor, len, NULL, NULL);

    return total;
}


static int
cprintw(const wchar_t* fmt, ...)
{
    HANDLE cout = GetStdHandle(STD_OUTPUT_HANDLE);
    wchar_t* cursor, * nl;
    wchar_t buffer[512];
    int total, len;
    va_list ap;

    va_start(ap, fmt);
    total = len = vswprintf(buffer, _countof(buffer)-1, fmt, ap);
    buffer[_countof(buffer) - 1] = 0;
    va_end(ap);

    cursor = buffer;
    while ((nl = wcschr(cursor, '\n')) != NULL) {
        const int part = (nl - cursor) + 1;
        WriteConsoleW(cout, cursor, part - 1, NULL, NULL);
        WriteConsoleA(cout, "\n\r", 2, NULL, NULL);
        cursor += part;
        len -= part;
    }
    WriteConsoleW(cout, cursor, len, NULL, NULL);

    return total;
}



#if (0)
#pragma comment(lib, "Imm32.lib")
static int
ImmTest(void)
{
    HWND hWnd = GetConsoleWindow();
    HWND hIME = ImmGetDefaultIMEWnd(hWnd);
    LRESULT status = SendMessage(hIME, WM_IME_CONTROL, IMC_GETOPENSTATUS, 0);
    HIMC imc = ImmCreateContext(),
        oldimc = ImmAssociateContext(hWnd, imc),
        retimc = ImmGetContext(hWnd);
    ImmSetOpenStatus(imc, TRUE);
    BOOL isopen = ImmGetOpenStatus(imc);
    Sleep(2 * 2000);
    ImmAssociateContext(hWnd, oldimc);
    ImmReleaseContext(hWnd, imc);
    Sleep(100);
}

#endif

/*end*/

