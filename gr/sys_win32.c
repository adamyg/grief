#include <edidentifier.h>
__CIDENT_RCSID(gr_sys_win32_c,"$Id: sys_win32.c,v 1.85 2025/06/30 10:17:08 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sys_win32.c,v 1.85 2025/06/30 10:17:08 cvsuser Exp $
 * WIN32 system support.
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

#ifdef WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#undef WINVER
#define WINVER _WIN32_WINNT
#endif

#include <editor.h>
#include <edfileio.h>
#include <edenv.h>                              /* gputenv(), ggetenv() */
#include <edalt.h>

#include "accum.h"
#include "buffer.h"
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "keyboard.h"
#include "m_pty.h"
#include "main.h"
#if !defined(HAVE_MOUSE)
#define  HAVE_MOUSE                             /* enable prototypes */
#endif
#include "mouse.h"
#include "system.h"
#include "tty.h"
#include "window.h"

#if defined(WIN32)                              /* module WIN32 specific */
#include <../libw32/win32_io.h>
#include "vio.h"

static DWORD                dwVersion, dwMajorVersion, dwMinorVersion, dwBuild;
static DWORD                consoleMode = (DWORD)-1;

static BOOL                 CtrlHandler(DWORD fdwCtrlType);
static void CALLBACK        HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
                                LONG idObject, LONG idChild,  DWORD dwEventThread, DWORD dwmsEventTime);

static int                  Resize(int winch);
static int                  ResizeCheck(unsigned *checks);

static void                 OutputDebugPrintA(const char* fmt, ...);

#if defined(__WATCOMC__)
#if (__WATCOMC__ >= 1300)   /*XXX, still supported?*/
volatile char               __WD_Present = 0;
#else
extern volatile char        __WD_Present;
#endif
#if defined(_M_IX86)
extern void                 EnterDebugger(void);
#pragma aux EnterDebugger = "int 3"
#define CheckEnterDebugger() \
    if (__WD_Present) EnterDebugger()
#endif
#endif


/*  Function:           sys_initialise
 *      Open the console
 *
 *  Notes:
 *      The following table summarizes the most recent operating system version numbers.
 *
 *          Operating system            Version number (major.minor)
 *          --------------------------------------------------------------
 *          Windows 11                  10.0
 *          Windows 10                  10.0
 *          Windows 8                   6.2
 *          Windows 7                   6.1
 *          Windows Server 2008 R2      6.1
 *          Windows Server 2008         6.0
 *          Windows Vista               6.0
 *          Windows Server 2003 R2      5.2
 *          Windows Server 2003         5.2
 *          Windows XP 64-Bit Edition   5.2
 *          Windows XP                  5.1
 *          Windows 2000                5.0
 */
void
sys_initialise(void)
{
    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);

    /*
     *  Windows version.
     */
    dwVersion = GetVersion();
    dwMajorVersion = (DWORD)(LOBYTE(LOWORD(dwVersion)));
    dwMinorVersion = (DWORD)(HIBYTE(LOWORD(dwVersion)));

    if (dwVersion < 0x80000000) {               /* Windows NT/2000 and greater */
        dwBuild = (DWORD)(HIWORD(dwVersion));
    } else if (dwMajorVersion < 4) {            /* Win32s */
        dwBuild = (DWORD)(HIWORD(dwVersion) & ~0x8000);
    } else {                                    /* Windows 95/98 -- No build number */
        dwBuild = 0;
    }

    /*
     *  Console mode, required for correct Ctrl-C processing.
     */
    if (hConsole) {
        if ((DWORD)-1 == consoleMode) {         /* save */
            GetConsoleMode(hConsole, &consoleMode);
        }

        if (xf_mouse) {                         /* mouse enabled */
            if (! SetConsoleMode(hConsole, ENABLE_EXTENDED_FLAGS|\
                        ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT|ENABLE_PROCESSED_INPUT)) {
                    // Note: Stating ENABLE_EXTENDED_FLAGS disables ENABLE_INSERT_MODE and/or ENABLE_QUICK_EDIT_MODE.
                    //  required for correct mouse operation; restored within sys_shutdown().
                SetConsoleMode(hConsole, ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT|ENABLE_PROCESSED_INPUT);
                    // No extended support/XP.
            }
        } else {
            SetConsoleMode(hConsole, ENABLE_WINDOW_INPUT|ENABLE_PROCESSED_INPUT);
        }
    }

    SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler, TRUE);

    Resize(FALSE);
    x_display_ctrl |= DC_SHADOW_SHOWTHRU;       /* for non-color mode */
}


/*  Function:           sys_shutdown
 *      Close the console.
 *
 */
void
sys_shutdown(void)
{
    HANDLE hConsole = GetStdHandle(STD_INPUT_HANDLE);

    if (hConsole) {
        if ((DWORD)-1 != consoleMode) {
            SetConsoleMode(hConsole, consoleMode);
        }
    }
    SetConsoleCtrlHandler(NULL, TRUE);
}


/*  Function:           sys_cleanup
 *      System specific cleanup.
 *
 */
void
sys_cleanup(void)
{
}


/*  Function:           CtrlHandler
 *      Console control handler.
 *
 *   Description:
 *      When a CTRL_CLOSE_EVENT signal is received, the control handler returns TRUE, causing the
 *      system to display a dialog box that gives the user the choice of terminating the process
 *      and closing the console or allowing the process to continue execution. If the user
 *      chooses not to terminate the process, the system closes the console when the process
 *      finally terminates.
 *
 *      When a CTRL+BREAK, CTRL_LOGOFF_EVENT, or CTRL_SHUTDOWN_EVENT signal is received, the
 *      control handler returns FALSE. Doing this causes the signal to be passed to the next
 *      control handler function. If no other control handlers have been registered or none of
 *      the registered handlers returns TRUE, the default handler will be used, resulting in the
 *      process being terminated
 *
 */
static BOOL
CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType) {
    case CTRL_CLOSE_EVENT:
        Beep(600, 200); //TODO: KEY_SHUTDOWN/trigger
        if (buf_anycb() == TRUE) {
            return FALSE;
        }
        gr_exit(0);
        return TRUE;

    case CTRL_C_EVENT:
        execute_event_ctrlc();
        return TRUE;

    case CTRL_BREAK_EVENT:
#if defined(CheckEnterDebugger)
        CheckEnterDebugger();
#endif
        return TRUE;

    case CTRL_SHUTDOWN_EVENT:
    case CTRL_LOGOFF_EVENT:
        check_exit();
        return FALSE;

    default:
        break;
    }
    return FALSE;
}


/*  Function:           sys_mouseinit
 *      Open the mouse device.
 *
 */
int
sys_mouseinit(const char *dev)
{
    (void)dev;
    return TRUE;
}


/*  Function:           sys_mouseclose
 *      Close the mouse device.
 *
 */
void
sys_mouseclose(void)
{
}


/*  Function:           sys_mousepointer
 *      Control the status of the mouse pointer
 *
 */
void
sys_mousepointer(int on)
{
    (void)on;
}


/*  Function:           sys_doubleclickms
 *      System double-click time.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Returns double-click time is milliseconds.
 */
unsigned
sys_doubleclickms(void)
{
    return GetDoubleClickTime();                /* default=500ms */
}


/*  Function:           sys_noinherit
 *      Make sure we don't inherit the specified file descriptor when we create a new process.
 *
 */
void
sys_noinherit(int fd)
{
    HANDLE handle;

    if ((handle = (HANDLE) _get_osfhandle(fd)) != INVALID_HANDLE_VALUE) {
        SetHandleInformation(handle, HANDLE_FLAG_INHERIT, 0);
    }
}


/*  Function:           DiffTicks
 *      Determine the period since 'sticks' and the current system tick.
 *
 */
static DWORD
DiffTicks(DWORD stick)                          /* start tick */
{
    DWORD etick;                                /* end tick */

    etick = GetTickCount();
    if (etick >= stick) {                       /* normal case */
        return (etick - stick) + 1;
    }
    return (0xffffffff - stick) + 1 + etick;    /* ticks have wrapped */
}


/*  Function:           Resize
 *     Determine whether a resize is required and invoke as a result ttwinch().
 *
 */
static int
Resize(int winch)
{
    static int  orows = 0, ocols = 0;
    static char ofont[80] = {0};
    static RECT orect = {0};

    VIOMODEINFO mi = { sizeof(VIOMODEINFO) };
    int nrows, ncols, resize = 0;

    VioGetMode(&mi, 0);
    ncols = (int)mi.col;
    nrows = (int)mi.row;

    if (2 == winch) {                           /* possible font change */
        RECT rect;

        if (ofont[0]) {
            char nfont[sizeof(ofont)] = {0};
            VioGetFont(nfont, sizeof(nfont));
            resize = (0 != memcmp(nfont, ofont, sizeof(ofont)));
            if (resize) {
                trace_log("resize: font change <%s to %s>\n", ofont, nfont);
                memcpy(ofont, (const char *)nfont, sizeof(ofont));
            }
        } else {
            VioGetFont(ofont, sizeof(ofont));
        }

        GetWindowRect(GetConsoleWindow(), &rect);
        if ((orect.right - orect.left) != (rect.right - rect.left) ||
                (orect.bottom - orect.top) != (rect.bottom - rect.top)) {
            if (orect.bottom) {                 /* previous? */
                trace_log("resize: frame change <%u by %u>\n",
                    (unsigned)(rect.right - rect.left), (unsigned)(rect.bottom - rect.top));
                resize = 1;
            }
        }
        orect = rect;
    }

    if (orows != nrows || ocols != ncols) {
        trace_log("resize: size change <%d/%d to %d/%d>\n", orows, ocols, nrows, ncols);
        orows = nrows;                          /* change; cache current */
        ocols = ncols;
        resize = 1;
    }

    if (resize) {
        if (winch) {
            ++tty_needresize;
        }
        return 1;
    }
    return 0;
}


static int
ResizeCheck(unsigned *checks)
{
    unsigned t_checks = *checks;
    int resize;

    if (t_checks) {
        if ((resize = Resize(2)) != 0) t_checks = 0;
        else --t_checks;
    } else {
        resize = Resize(1);
    }
    *checks = t_checks;
    return resize;
}


static int
Modifiers(const DWORD dwControlKeyState)
{
#define CTRLSTATUSMASK  (LEFT_ALT_PRESSED|LEFT_CTRL_PRESSED|RIGHT_ALT_PRESSED|RIGHT_CTRL_PRESSED|SHIFT_PRESSED)
#define ALT_PRESSED     (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED)
#define CTRL_PRESSED    (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED)
    int modifiers = 0;

    if (dwControlKeyState & ALT_PRESSED)
        modifiers |= MOD_META;
    if (dwControlKeyState & CTRL_PRESSED)
        modifiers |= MOD_CTRL;
    if (dwControlKeyState & SHIFT_PRESSED)
        modifiers |= MOD_SHIFT;
    return modifiers;
}


//  MouseProcess ---
//      Process a console mouse event.
//
//  Notes:
//    o Mouse events are placed in the input buffer when the console is in mouse mode (ENABLE_MOUSE_INPUT).
//    o Mouse events are generated whenever the user moves the mouse, or presses or releases one of the mouse buttons.
//    o Mouse events are placed in a console's input buffer only when the console group has the keyboard focus
//      and the cursor is within the borders of the console's window. 
//    o Focus is retained if button are active and the mouse cursor is moved outside the window borders, 
//      with reported positions clipped to the borders.
//    o Coordinate system (0, 0) is at the top, left cell of the buffer.
//
static int
MouseProcess(const MOUSE_EVENT_RECORD *mer)
{
    struct MouseEvent me = { 0 };
    const DWORD dwButtonState = mer->dwButtonState;
    const DWORD dwControlKeyState = mer->dwControlKeyState;
    const DWORD dwEventFlags = mer->dwEventFlags;

    // meta data
    me.x  = mer->dwMousePosition.X + 1;
    me.y  = mer->dwMousePosition.Y + 1;
    me.b1 = (dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) ? 1 : 0;
    me.b2 = (dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED) ? 1 : 0;
    me.b3 = (dwButtonState & RIGHTMOST_BUTTON_PRESSED) ? 1 : 0;
    if (dwControlKeyState & ALT_PRESSED)
        me.ctrl |= MOUSEEVENT_CMETA;
    if (dwControlKeyState & CTRL_PRESSED)
        me.ctrl |= MOUSEEVENT_CCTRL;
    if (dwControlKeyState & SHIFT_PRESSED)
        me.ctrl |= MOUSEEVENT_CSHIFT;
    me.multi = 0;

    if (dwEventFlags == MOUSE_MOVED) {
        // motion events
        if (me.b1 | me.b2 | me.b3) {
            me.type = MOUSEEVENT_TMOTION;
            mouse_process(&me, "");
            return 1;
        }
    } else {
        // button events
        assert(dwEventFlags == 0 || (dwEventFlags & DOUBLE_CLICK));
        if (dwEventFlags & DOUBLE_CLICK) //note: MOUSE_MOVED|DOUBLE_CLICK possible
            me.multi = 1;
        me.type = MOUSEEVENT_TPRESSRELEASE;
        mouse_process(&me, "");
        return 1;
    }
    return 0;
}


//  AltPlusEnabled, AltPlusEvent ---
// 
//      Alt+<key-code> event handler
//
//      Alt+KeyCode works and behaves well when character only input is required, by simply
//      reporting any down or up key events which populate the 'UnicodeChar' value. Whereas
//      when extended keystroke handling is required, for example arrow and numpad keys,
//      additional effort is needed.
//
//      Alt+Keycodes are only reported within the 'UnicodeChar' value of up event on a "ALT" key
//      post the valid entry of one-or-more hex characters. During KeyCode entry the API unfortunately
//      does not publicly indicate this state plus continues to return the associated virtual keys,
//      including the leading 'keypad-plus' and any associated key-code elements, wherefore we need
//      to filter.  Furthermore, if during the key-code entry an invalid non-hex key combination is
//      given, the key-code is invalidated and UnicodeChar=0 is returned on the ALT release.
//
//  Notes:
//    o To enable requires the registry REG_SZ value "EnableHexNumpad" under
//       "HKEY_Current_User/Control Panel/Input Method" to be "1".
//
//    o Hex-value overflow goes unreported, limiting input to a 16-bit Unicode result.
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
AltPlusEvent(const KEY_EVENT_RECORD *ke, struct IOEvent *evt)
{
#define ISXDIGIT(_uc) \
            ((_uc >= '0' && _uc <= '9') || (_uc >= 'a' && _uc <= 'f') || (_uc >= 'A' && _uc <= 'F') ? 1 : 0)

    static unsigned alt_code = 0;               // >0=active
    static DWORD alt_control = 0;

    const unsigned wVirtualKeyCode = ke->wVirtualKeyCode,
        dwControlKeyState = (CTRLSTATUSMASK & ke->dwControlKeyState),
        dwEnhanced = (ENHANCED_KEY & ke->dwControlKeyState);

#if (DO_KEYTRACE)
    OutputDebugPrintA("Key: %s-%s%s%s%s%s%sVK=0x%02x/%u, UC=0x%04x/%u/%c, SC=0x%x/%u\n",
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
#endif

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
                alt_code = 1;
            }
            return 1;                           // consume
        }

        if (alt_code) {
            if (alt_control != dwControlKeyState ||
                    (ke->uChar.UnicodeChar && 0 == ISXDIGIT(ke->uChar.UnicodeChar))) {
                // new control status or non-hex, emit "Alt-Plus" and reset state
                evt->type = EVT_KEYDOWN;
                evt->code = KEYPAD_PLUS;
                evt->modifiers = MOD_ALT;
                alt_code = 0;
                return 0;
            }
            ++alt_code;                         // associated key count
            return 1;                           // consume
        }

    } else if (alt_code) {                      // up event
        if (VK_MENU == wVirtualKeyCode &&
                (0 == (ke->dwControlKeyState & ALT_PRESSED))) {
            // Alt completion
            const WCHAR UnicodeChar = ke->uChar.UnicodeChar;
            const int alt_old = alt_code;

            alt_code = 0;
            if (1 == alt_old && 0 == UnicodeChar) {
                // Alt only, emit.
                evt->type = EVT_KEYDOWN;
                evt->code = KEYPAD_PLUS;
                evt->modifiers = MOD_ALT;
                return 0;

            } else if (UnicodeChar) {
                // Alt keycode, return keycode.
                evt->type = EVT_KEYDOWN;
                evt->code = UnicodeChar;
                evt->modifiers = 0;
                return 0;
            }
        }
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

    trace_log("AltGr: 0x%04lx = 0x%04lx", (unsigned long)key->dwControlKeyState, (unsigned long)state);
    return state;
}


/*  Function:           sys_getevent
 *      Retrieve the input event from the status keyboard stream, within
 *      the specified timeout 'tmo'.
 *
 *  Parameters:
 *      fd - File descriptor.
 *      evt - Event buffer.
 *      tmo - Timeout, in milliseconds.
 *
 *  Returns:
 *      On success (0), unless a timeout (-1).
 */
int
sys_getevent(struct IOEvent *evt, accint_t tmo)
{
    unsigned checks = 1;
    HANDLE hKbd = GetStdHandle(STD_INPUT_HANDLE);
    DWORD ticks, tmticks;
    INPUT_RECORD k;
    DWORD count, rc;
    int resize = 0;                             /* new resize event */

    for (;;) {

        if ((tmticks = (DWORD)tmo) == 0) {
            tmticks = INFINITE;                 /* block forever */
        } else if (tmo < 0) {
            tmticks = 0;                        /* no time-out */
        }

#if defined(_MSC_VER)
#pragma warning(suppress:28159)
#endif
        ticks = GetTickCount();                 /* ticks (ms) as start */
        rc = WaitForSingleObject(hKbd, tmticks);
        ticks = DiffTicks(ticks);               /* ticks (ms) as end */

        if (rc == WAIT_OBJECT_0 &&
                ReadConsoleInputW(hKbd, &k, 1, &count)) {

            switch (k.EventType) {
            case KEY_EVENT: {
                    const KEY_EVENT_RECORD *ke = &k.Event.KeyEvent;

                    {                           /* Alt+KeyCode (experimental) */
                        const int altstate = AltPlusEvent(ke, evt);
                        if (altstate == 0) return 0;
                        if (altstate == 1) break;
                    }

                    if (ke->bKeyDown) {
                        const DWORD dwControlKeyState = AltGrEvent(ke);
                        int code;
                                                /* see kbd.c */
                        if ((code = key_mapwin32((unsigned) dwControlKeyState,
                                        ke->wVirtualKeyCode, ke->uChar.UnicodeChar)) != -1) {

                            evt->type = EVT_KEYDOWN;
                            evt->code = code;
                            evt->modifiers = Modifiers(ke->dwControlKeyState);
                            assert(code > 0 && code <= (MOD_MASK|RANGE_MASK|KEY_MASK) && code != KEY_VOID);
                            return 0;
                        }
                    } else {
                        resize = ResizeCheck(&checks);
                    }
                }
                break;

            case MOUSE_EVENT: {
                    const MOUSE_EVENT_RECORD *me = &k.Event.MouseEvent;

#ifndef MOUSE_WHEELED
#define MOUSE_WHEELED 0x0004 /* Not available within NT or 95/98 SDK */
#endif
#ifndef MOUSE_HWHEELED
#define MOUSE_HWHEELED 0x0008
#endif

                    if (MOUSE_WHEELED & me->dwEventFlags) {
                        const int down = (0xFF000000 & me->dwButtonState ? TRUE : FALSE);

                        evt->type = EVT_KEYDOWN;
                        if (SHIFT_PRESSED & me->dwButtonState) {
                            evt->code = (down ? KEY_PAGEDOWN : KEY_PAGEUP);
                        } else {
                            evt->code = (down ? WHEEL_DOWN : WHEEL_UP);
                        }
                        return 0;
                    }

                    if (MOUSE_HWHEELED & me->dwEventFlags) {
                        const int down = (0xFF000000 & me->dwButtonState ? TRUE : FALSE);

                        evt->type = EVT_KEYDOWN;
                        evt->code = (down ? WHEEL_RIGHT : WHEEL_LEFT);
                        return 0;
                    }

                    if (MouseProcess(me)) {
                        evt->type = EVT_MOUSE;
                        evt->code = KEY_VOID;
                        return 0;
                    }
                }
                ResizeCheck(&checks);
                break;

            case FOCUS_EVENT:
                if (tty_open) {
                    VioSetFocus(k.Event.FocusEvent.bSetFocus);
                    resize = Resize(2);
                }
                break;

            case WINDOW_BUFFER_SIZE_EVENT:
                resize = Resize(2);
                break;

            case MENU_EVENT:
                /*
                 *  font changes wont be reported, *unless* the window/buffer size is modified as a result,
                 *      as such force font checks for the number of input iterations.
                 */
                checks = 10;
                break;

            default:
                break;
            }
        }

        if (resize) {                           /* resize event */
            evt->type = EVT_KEYDOWN;
            evt->code = KEY_WINCH;
            evt->modifiers = 0;
            return 0;
        }

        if (tmo < 0 || (tmo > 0 && (tmo -= ticks) <= 0)) {
            break;                              /* poll/timeout */
        }
    }
    return -1;
}


/*  Function:           sys_getchar
 *      Retrieve the character from the status keyboard stream, within
 *      the specified timeout 'tmo'.
 *
 *  Parameters:
 *      fd - File descriptor (ignored).
 *      buf - Output buffer.
 *      tmo - Timeout, in milliseconds.
 *
 *  Returns:
 *      On success (1), otherwise (0) unless a timeout (-1).
 */
int
sys_getchar(int fd, int *buf, accint_t tmo)
{
    struct IOEvent evt = {0};

    __CUNUSED(fd)
    if (0 == sys_getevent(&evt, tmo)) {
        if (EVT_KEYDOWN == evt.type) {
            if (buf) *buf = evt.code;
            return 1;
        }
    }
    return (tmo >= 0 ? -1 : 0);
}


/*  Function:           sys_iocheck
 *      Check for an event input event.
 *
 *  Parameters:
 *      None
 *
 *  Returns:
 *      *true* or *false*.
 */
int
sys_iocheck(struct IOEvent *evt)
{
    return (0 == sys_getevent(evt, /*NOWAIT*/-1) ? TRUE : FALSE);
}


/*  Function:           sys_cut
 *      Cut the current marked region to the 'system' clipboard/scrap.
 *
 */
int
sys_cut(int total, int append, void (*copy)(char *buf, int total))
{
    int ret = 0;

    if (NULL == copy || OpenClipboard(NULL)) {
        ret = -1;
    } else {
        HGLOBAL hglb;
        LPVOID lps;

        if (NULL == (hglb = GlobalAlloc(GMEM_MOVEABLE, (total+1) * sizeof(TCHAR))) ||
                NULL == (lps = GlobalLock(hglb))) {
            ret = -1;

        } else  {
            char *cp = (char *)lps;

            (*copy)(cp, total);
            cp[ total ] = 0;
            GlobalUnlock(hglb);
            if (! append) EmptyClipboard();
            SetClipboardData(CF_TEXT, hglb);
            ret = 1;
        }
        CloseClipboard();
    }
    return ret;
}


/*  Function:           sys_paste
 *      Paste the 'system' clipboard/scrap into the current buffer.
 *
 *  Parameters:
 *      paste - Paste callback.
 *
 */
int
sys_paste(void (*paste)(const char *buf, int len))
{
    int ret = 0;

    if (! IsClipboardFormatAvailable(CF_TEXT) ||
            ! OpenClipboard(NULL)) {            /* MCHAR??? */
        ret = -1;
    } else {
        HGLOBAL hglb;
        LPVOID lps;

        if (NULL != (hglb = GetClipboardData(CF_TEXT)))
            if (NULL != (lps = GlobalLock(hglb))) {
                const char *cp = (const char *)lps;
                if (*cp) {
                    if (paste) (paste)(cp, -1);
                    ret = 1;
                }
                GlobalUnlock(hglb);
            }
        CloseClipboard();
    }
    return ret;
}


/*  Function:           sys_getshell
 *      Retrieve the default shell.
 *
 */
const char *
sys_getshell(void)
{
    const char *shname;

    shname = ggetenv("SHELL");
    if (NULL == shname) shname = ggetenv("COMSPEC");
    if (NULL == shname) shname = ggetenv("ComSpec");
    if (NULL == shname) {
        if (dwVersion < 0x80000000) {
            shname = "CMD.EXE";                 /* Windows NT/2000/XP */
        } else {
            shname = "COMMAND.EXE";             /* ... others */
        }
    }
    return shname;
}


/*  Function:           sys_delim
 *      Retrieve the default directory delimiter.
 *
 */
const char *
sys_delim(void)
{
    return "\\";
}


/*  Function:           sys_cwd
 *      Set the current working directory.
 *
 */
const char *
sys_cwd(char *buffer, int size)
{
    return w32_getcwd(buffer, size);
}



/*  Function:           sys_ccwd
 *      Get the current directory for the specified drive.
 *
 */
void
sys_cwdd(int drive, char *path, int size)
{
    w32_getcwdd((char)drive, path, size);
}


/*  Function:           sys_drive_get
 *      Get the current working drive.
 *
 */
int
sys_drive_get(void)
{
    char path[MAX_PATH];

    if (GetCurrentDirectoryA(sizeof(path), path) <= 0) {
        w32_errno_set();
        return '\0';
    }
    return path[0];
}


/*  Function:           sys_drive_set
 *      Set the current working drive.
 *
 */
int
sys_drive_set(int drive)
{
    const unsigned nDrive =
            (isalpha((unsigned char)drive) ? (toupper(drive) - 'A') : 0xff);
    char t_path[4] = {0};

    if (nDrive >= 26) {
        errno = EINVAL;
        return -1;
    }
    t_path[0] = (char)drive;
    t_path[1] = ':';
    t_path[2] = '\0';
    if (! SetCurrentDirectoryA(t_path)) {
        w32_errno_set();
        return -1;
    }
    return 0;
}


/*  Function:           sys_fstype
 *      Determine the file-system type.
 *
 *  Returns:
 *      1 if a HPFS otherwise 0; HPFS/NTFS, i.e. support for long
 *      file names and being case sensitive to some extent.
 */
int
sys_fstype(const char *directory)
{
#define DISABLE_HARD_ERRORS SetErrorMode (0)
#define ENABLE_HARD_ERRORS  SetErrorMode (SEM_FAILCRITICALERRORS | \
                                    SEM_NOOPENFILEERRORBOX)

    char            bName[4];
    DWORD           flags;
    DWORD           maxname;
    BOOL            rc;
    unsigned int    nDrive;
    char            szCurDir[MAX_PATH+1];

                                                // XXX - should cache results??
    if (directory && isalpha(directory[0]) && (directory[1] == ':')) {
        nDrive = toupper (directory[0]) - '@';
    } else {
        GetCurrentDirectoryA(MAX_PATH, szCurDir);
        nDrive = szCurDir[0] - 'A' + 1;
    }

    strcpy(bName, "x:\\");
    bName[0] = (char) (nDrive + '@');

    DISABLE_HARD_ERRORS;
    rc = GetVolumeInformationA(bName, NULL, 0,
                (LPDWORD)NULL, &maxname, &flags, NULL, 0);
    ENABLE_HARD_ERRORS;

    return ((rc) &&                             // XXX - FS_CASE_IS_PRESERVED
        (flags & (FS_CASE_SENSITIVE | FS_CASE_IS_PRESERVED))) ? TRUE : FALSE;

#undef DISABLE_HARD_ERRORS
#undef ENABLE_HARD_ERRORS
}


/*  Function:           sys_enable_char
 *      Routine called by input_mode() primitive to enable certain
 *      keys to be seen by Grief, e.g. XON/XOFF, ^Z.
 */
int
sys_enable_char(int ch, int state)
{
    (void) ch;
    (void) state;
    return 1;
}


/*  Function:           sys_read
 *      Routines which are machine independent but are useful for making
 *      the rest of the code portable, especially to VMS.
 *
 */
int
sys_read(int fd, void *buf, int size)
{
    char *data = (char *)buf;
    int n, osize = size;

    while (size > 0) {
        if ((n = fileio_read(fd, data, size)) <= 0) {
            if (n < 0 && data == (char *)buf) {
                return -1;
            }
            break;
        }
        size -= n;
        data += n;
    }
    return (osize - size);
}


int
sys_write(int fd, const void *buf, int size)
{
    const char *data = (const char *)buf;
    int n, cnt = 0;

    do {                                        /* EINTR safe */
        if ((n = fileio_write(fd, data, size)) >= 0) {
            data += n;
            cnt += n;
            if ((size -= n) <= 0) {
                return cnt;                     /* success */
            }
        }
    } while (n >= 0 || (n < 0 && errno == EINTR));
    return -1;
}


/*  Function:           sys_copy
 *      Copy a file using system specific functionality, if available.
 *
 */
int
sys_copy(
    const char *src, const char *dst, int perms, int owner, int group )
{
    BOOL rc;

#ifndef HAVE_CHOWN
    __CUNUSED(group)
    __CUNUSED(owner)
#endif
    if ((rc = CopyFileA(src, dst, FALSE)) != FALSE) {
        (void) sys_chmod(dst, perms);           /* FIXME: return */
#ifdef HAVE_CHOWN
        chown(dst, owner, group);
#endif
        return TRUE;
    }
    return -1;                                  /* use default */
}


/*  Function:           sys_xxx
 *      System i/o primitives.
 */
int
sys_mkdir(const char *path, int amode)
{
    return w32_mkdir(path, amode);
}


int
sys_access(const char *path, int amode)
{
    return w32_access(path, amode);
}


int
sys_chmod(const char *path, int mode)
{
    return w32_chmod(path, (mode_t)mode);
}


int
sys_realpath(const char *name, char *buf, int size)
{
    return (NULL == w32_realpath2(name, buf, size) ? -1 : 0);
}


int
sys_stat(const char *path, struct stat *sb)
{
    return w32_stat(path, sb);
}


int
sys_lstat(const char *path, struct stat *sb)
{
    return w32_lstat(path, sb);
}


int
sys_readlink(const char *path, char *buf, int maxlen)
{
    return w32_readlink(path, buf, maxlen);
}


int
sys_symlink(const char *name1, const char *name2)
{
    return w32_symlink(name1, name2);
}


int
sys_unlink(const char *fname)
{
    return w32_unlink(fname);
}


/*  Function:           sys_time
 *      Retrieve the current system time.
 *
 */
typedef void (WINAPI *GetSystemTimePreciseAsFileTime_t)(LPFILETIME lpSystemTimeAsFileTime);

static unsigned long long
GetSystemTimeNS100(void)
{
    static GetSystemTimePreciseAsFileTime_t fGetSystemTimePreciseAsFileTime = NULL;
    FILETIME ft = {0};
    unsigned long long ns100;

    /*
     *  GetSystemTime(Precise)AsFileTime returns the number of 100-nanosecond intervals since January 1, 1601 (UTC).
     *
     *  GetSystemTimeAsFileTime has a resolution of approximately the TimerResolution (~15.6ms) on Windows XP.
     *  On Windows 7 it appears to have sub-millisecond resolution. GetSystemTimePreciseAsFileTime (available on
     *  Windows 8) has sub-microsecond resolution.
     */
    if (NULL == fGetSystemTimePreciseAsFileTime) {
        HINSTANCE hinst;

#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
        hinst = LoadLibraryA("Kernel32");
        if (NULL == (fGetSystemTimePreciseAsFileTime =
                             (GetSystemTimePreciseAsFileTime_t)GetProcAddress(hinst, "GetSystemTimePreciseAsFileTime"))) {
            fGetSystemTimePreciseAsFileTime =
                (GetSystemTimePreciseAsFileTime_t)GetProcAddress(hinst, "GetSystemTimeAsFileTime"); /*fall-back*/
        }
#if defined(GCC_VERSION) && (GCC_VERSION >= 80000)
#pragma GCC diagnostic pop
#endif
    }

    fGetSystemTimePreciseAsFileTime(&ft);

    ns100 = ft.dwHighDateTime;
    ns100 <<= 32UL;
    ns100 |= ft.dwLowDateTime;
    ns100 -= 116444736000000000LL; /* 1601->1970 epoch */

    return ns100;
}


time_t
sys_time(int *msec)
{
    const unsigned long long ns100 = GetSystemTimeNS100();
    time_t secs;

    if (msec)
        *msec = (int) ((ns100 % 10000000UL) / 10000UL);
    secs = (time_t)(ns100 / 10000000UL);
    return secs;
}


/*  Function:           sys_mkstemp
 *      Create a unique filename by modifying the template argument.
 *
 *  Parameter
 *      pattern - Filename pattern.
 *
 *  Return Value:
 *      Each of these functions returns a pointer to the modified pattern. The function returns
 *      NULL if pattern is badly formed or no more unique names can be created from the given
 *      pattern.
 */
int
sys_mkstemp(char *pattern)
{
    return w32_mkstemp(pattern);
}


/*  Function:           sys_getpid
 *      Retrieve the current process identifier.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Current process identifier;
 */
int
sys_getpid(void)
{
    return (int)GetCurrentProcessId();
}


int
sys_getuid(void)
{
    return w32_getuid();
}


int
sys_geteuid(void)
{
    return w32_getuid();
}



/*  Function:           sys_core
 *      Generate a core/system stack for the current process.
 *
 *  Parameters:
 *      XXX -
 *
 *  Returns:
 *      Returns 0 on success, otherwise -1;
 */
int
sys_core(const char *msg, const char *path, const char *fname)
{
    __CUNUSED(msg)
    __CUNUSED(path)
    __CUNUSED(fname)
    /*DrWatson ??*/
    return -1;
}


void
sys_abort(void)
{
    abort();
    /*NOTREACHED*/
}



/*  Function:           sys_signal
 *      Signal handler installation.
 *
 *  Parameters:
 *      sig - Signal number.
 *      func - Signal handler function.
 *
 *  Returns:
 *      Returns 0 on success, otherwise -1;
 */
signal_handler_t
sys_signal(int sig, signal_handler_t func)
{
    switch (sig) {
#if defined(SIGCHLD)
    case SIGCHLD:       //TODO
        errno = EIO;
        return NULL;
#endif
#if defined(SIGPIPE)
    case SIGPIPE:       //TODO
        errno = EIO;
        return NULL;
#endif
#if defined(SIGWINCH)
    case SIGWINCH:      //ignore
        errno = EIO;
        return NULL;
#endif
    default:
        return signal(sig, func);
    }
}


/*  Function:           sys_running
 *      Determine if given process is still running.
 *
 *  Parameters:
 *      pid -               Process identifier.
 *
 *  Returns:
 *      Returns 1 if the processing is still running, 0 if not otherwise -1 on an error condition.
 */
int
sys_running(int pid)
{
    if (GetProcessVersion((DWORD)pid) != 0) {
        return 1;
    }
    return 0;
}


void
OutputDebugPrintA(const char* fmt, ...)
{
    struct timeval tv;
    char out[512];
    va_list ap;
    int prefix;

    va_start(ap, fmt);
    w32_gettimeofday(&tv, NULL);
#if !defined(SIZEOF_TIME_T)
#error SIZEOF_TIME_T missing
#endif
#if (SIZEOF_TIME_T == SIZEOF_LONG_LONG)
    prefix = sprintf(out, "%lld.%03u: ", (long long)tv.tv_sec, (unsigned)(tv.tv_usec / 1000));
#elif (SIZEOF_TIME_T == SIZEOF_LONG)
    prefix = sprintf(out, "%ld.%03u: ", (long)tv.tv_sec, (unsigned)(tv.tv_usec / 1000));
#else
    prefix = sprintf(out, "%d.%03u: ", tv.tv_sec, (unsigned)(tv.tv_usec / 1000));
#endif
    vsprintf_s(out + prefix, _countof(out) - prefix, fmt, ap);
    va_end(ap);
    OutputDebugStringA(out);
}

#endif /*WIN32*/
