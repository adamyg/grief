#include <edidentifier.h>
__CIDENT_RCSID(gr_sys_win32_c,"$Id: sys_win32.c,v 1.57 2020/04/13 01:22:18 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sys_win32.c,v 1.57 2020/04/13 01:22:18 cvsuser Exp $
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

#ifdef  WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif
#define WINVER 0x0601
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
static void                 ResizeCheck(unsigned *checks);

#if defined(__WATCOMC__)
extern char volatile        __WD_Present;
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
    GetConsoleMode(hConsole, &consoleMode);
    SetConsoleMode(hConsole, ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT|ENABLE_PROCESSED_INPUT);
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

    if ((DWORD)-1 != consoleMode) {
        SetConsoleMode(hConsole, consoleMode);
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
        Beep(600, 200);
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
    static int  orows, ocols;
    static char ofont[80];
    static RECT orect;

    VIOMODEINFO mi = { sizeof(VIOMODEINFO) };
    int nrows, ncols, nfont = -1;
    char font[sizeof(ofont)];
    RECT rect;

    VioGetMode(&mi, 0);
    ncols = (int)mi.col;
    nrows = (int)mi.row;
    if (2 == winch) {                           /* possible font change */
        VioGetFont(font, sizeof(font));
        nfont = (0 != memcmp(font, ofont, sizeof(ofont)));
        GetWindowRect(GetConsoleWindow(), &rect);
        if ((orect.right - orect.left) != (rect.right - rect.left) ||
                (orect.bottom - orect.top) != (rect.bottom - rect.top)) {
            nfont = 1;
        }
    }

    if (orows != nrows || ocols != ncols || 1 == nfont) {
        orows = nrows;
        ocols = ncols;
        if (-1 == nfont) {
            VioGetFont(font, sizeof(font));
            GetWindowRect(GetConsoleWindow(), &rect);
        }
        memcpy(ofont, font, sizeof(ofont));
        orect = rect;
        if (winch) {
            tty_needresize = TRUE;
        }
        return 1;
    }
    return 0;
}


static void
ResizeCheck(unsigned *checks)
{
    unsigned t_checks = *checks;
    if (t_checks) {
        if (Resize(2)) t_checks = 0;
        else --t_checks;
    } else {
        Resize(1);
    }
    *checks = t_checks;
}


static int
Modifiers(const DWORD dwControlKeyState)
{
    int modifiers = 0;

    if (dwControlKeyState & (LEFT_ALT_PRESSED|RIGHT_ALT_PRESSED))
        modifiers |= MOD_META;
    if (dwControlKeyState & (LEFT_CTRL_PRESSED|RIGHT_CTRL_PRESSED))
        modifiers |= MOD_CTRL;
    if (dwControlKeyState & SHIFT_PRESSED)
        modifiers |= MOD_SHIFT;
    return modifiers;
}


static int
MouseEvent(const DWORD dwEventFlags, const DWORD dwButtonState)
{
    const int move  = (dwEventFlags & MOUSE_MOVED  ? 1 : 0);
    const int multi = (dwEventFlags & DOUBLE_CLICK ? 1 : 0);

    if (dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) {
        //button1
        if (move) return BUTTON1_MOTION;
        return (multi ? BUTTON1_DOUBLE : BUTTON1_DOWN);

    } else if (dwButtonState & RIGHTMOST_BUTTON_PRESSED) {
        //button2
        if (move) return BUTTON2_MOTION;
        return (multi ? BUTTON2_DOUBLE : BUTTON2_DOWN);

    } else if (dwButtonState & FROM_LEFT_2ND_BUTTON_PRESSED) {
        //button3
        if (move) return BUTTON3_MOTION;
        return (multi ? BUTTON3_DOUBLE : BUTTON3_DOWN);
    }
    return 0;
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
sys_getevent(struct IOEvent *evt, int tmo)
{
    unsigned checks = 1;
    HANDLE hKbd = GetStdHandle(STD_INPUT_HANDLE);
    DWORD ticks, tmticks;
    INPUT_RECORD k;
    DWORD count, rc;

    for (;;) {

        if ((tmticks = tmo) == 0) {
            tmticks = INFINITE;                 /* block forever */
        } else if (tmo < 0) {
            tmticks = 0;                        /* no time-out */
        }

        ticks = GetTickCount();                 /* ticks (ms) as start */
        rc = WaitForSingleObject(hKbd, tmticks);
        ticks = DiffTicks(ticks);               /* ticks (ms) as end */

        if (rc == WAIT_OBJECT_0 &&
                ReadConsoleInput(hKbd, &k, 1, &count)) {

            switch (k.EventType) {
            case KEY_EVENT:
                if (k.Event.KeyEvent.bKeyDown) {
                    const KEY_EVENT_RECORD *ke = &k.Event.KeyEvent;
                    int code;
                                                /* see kbd.c */
                    if ((code = key_mapwin32((unsigned) ke->dwControlKeyState,
                                    ke->wVirtualKeyCode, ke->uChar.AsciiChar)) != -1) {
                        evt->type = EVT_KEYDOWN;
                        evt->code = code;
                        evt->modifiers = Modifiers(ke->dwControlKeyState);
                        assert(code > 0 && code < KEY_VOID);
                        return 0;
                    }
                } else {
                    ResizeCheck(&checks);
                }
                break;

            case MOUSE_EVENT: {
                    const MOUSE_EVENT_RECORD *me = &k.Event.MouseEvent;

#ifndef MOUSE_WHEELED
#define MOUSE_WHEELED   4                       /* Not available within NT or 95/98 SDK */
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

                    if (0 == me->dwEventFlags ||
                            ((me->dwEventFlags & (MOUSE_MOVED|DOUBLE_CLICK)) && me->dwButtonState)) {
                        evt->type = EVT_MOUSE;
                        evt->code = MouseEvent(me->dwEventFlags, me->dwButtonState);
                        evt->modifiers = Modifiers(me->dwControlKeyState);
                        evt->mouse.x = me->dwMousePosition.X + 1;
                        evt->mouse.y = me->dwMousePosition.Y + 1;
                        return 0;
                    }
                }
                ResizeCheck(&checks);
                break;

            case FOCUS_EVENT:
                VioSetFocus(k.Event.FocusEvent.bSetFocus);
                Resize(2);
                break;

            case WINDOW_BUFFER_SIZE_EVENT:
                Resize(2);
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

    if (OpenClipboard(NULL)) {
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
 *      paste -             Paste callback.
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
#define DISABLE_HARD_ERRORS     SetErrorMode (0)
#define ENABLE_HARD_ERRORS      SetErrorMode (SEM_FAILCRITICALERRORS | \
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
        (void) fileio_chmod(dst, perms);        /* FIXME: return */
#ifdef HAVE_CHOWN
        chown(dst, owner, group);
#endif
        return TRUE;
    }
    return -1;                                  /* use default */
}


/*  Function:           sys_realpath
 *      Retrieve the real/absolute for the speified path.
 *
 */
int
sys_realpath(const char *name, char *buf, int size)
{
    return (NULL == _fullpath(buf, name, size) ? -1 : 0);
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
time_t
sys_time(int *msec)
{
    SYSTEMTIME stm;
    time_t t;

    t = time(NULL);
    if (msec) {
        GetSystemTime(&stm);
        *msec = (int) stm.wMilliseconds;
    }
    return (t);
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
 *
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
//  DWORD rc;
    if (GetProcessVersion((DWORD)pid) != 0) {
        return 1;
    }
//  rc = GetLastError();
    return 0;
}
#endif  /*WIN32*/

