#include <edidentifier.h>
__CIDENT_RCSID(gr_sys_os2_c,"$Id: sys_os2.c,v 1.27 2018/10/04 15:39:29 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sys_os2.c,v 1.27 2018/10/04 15:39:29 cvsuser Exp $
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
#include <edalt.h>
#include <edtermio.h>

#include "accum.h"
#include "builtin.h"
#include "echo.h"
#include "main.h"
#include "m_pty.h"
#include "tty.h"
#include "system.h"

#if defined(__OS2__)

#define INCL_BASE
#define INCL_NOPM
#define INCL_VIO
#define INCL_MOU
#include <os2.h>

#define CENTISECONDS            5               /* x0.1 seconds */

static USHORT           fansi;                  /* Saved ANSI state. */
#if defined(__FLAT__)
#define THREAD_STACK_SIZE       8192            /* Smallest possible anyway */
#else
#define THREAD_STACK_SIZE       2048
static BYTE             kbd_thread_stack[THREAD_STACK_SIZE];
#endif

static TID              tid;
static int              timed_out = FALSE;
static KBDKEYINFO       keyinfo;
static DISPLAY_t       *global_dp;              /* Global variable needed for pty thread. */

#if defined(__FLAT__)
static HEV              main_sem  = 0;
static HEV              kbd_sem   = 0;
static HMTX             pty_sem   = 0;          /* Thread to synchronise between main and pty threads. */
#else
static ULONG            main_sem  = 0;
static ULONG            kbd_sem   = 0;
static ULONG            pty_sem   = 0;          /* Thread to synchronise between main and pty threads. */
#endif

static void             scr_beep(void);
static void             scr_cursor(int insmode, int v_space);
static void             scr_windowsize(int * nrow, int * ncol);

#if !defined(SEM_INFINITE_WAIT)
#define SEM_INFINITE_WAIT       (-1)
#endif

static const KEY        dos_keys[] = {
    /* 0x00*/
    0x00,       0x00,       0x00,       0x00,
    0x00,       0x00,       0x00,       0x00,
    0x00,       0x00,       0x00,       0x00,
    0x00,       0x00,       0x00,       BACK_TAB,
    /* 0x10 */
    ALT_Q,      ALT_W,      ALT_E,      ALT_R,
    ALT_T,      ALT_Y,      ALT_U,      ALT_I,
    ALT_O,      ALT_P,      0x00,       0x00,
    0x00,       0x00,       ALT_A,      ALT_S,
    /* 0x20 */
    ALT_D,      ALT_F,      ALT_G,      ALT_H,
    ALT_J,      ALT_K,      ALT_L,      0x00,
    0x00,       0x00,       0x00,       0x00,
    ALT_Z,      ALT_X,      ALT_C,      ALT_V,
    /* 0x30 */
    ALT_B,      ALT_N,      ALT_M,      0x00,
    0x00,       0x00,       0x00,       0x00,
    0x00,       0x00,       0x00,       F(1),
    F(2),       F(3),       F(4),       F(5),
    /* 0x40 */
    F(6),       F(7),       F(8),       F(9),
    F(10),      0x00,       0x00,       209,
    210,        211,        0x00,       206,
    0x00,       208,        0x00,       203,
    /* 0x50 */
    204,        205,        202,        212,
    SF(1),      SF(2),      SF(3),      SF(4),
    SF(5),      SF(6),      SF(7),      SF(8),
    SF(9),      SF(10),     CF(1),      CF(2),
    /* 0x60 */
    CF(3),      CF(4),      CF(5),      CF(6),
    CF(7),      CF(8),      CF(9),      CF(10),
    AF(1),      AF(2),      AF(3),      AF(4),
    AF(5),      AF(6),      AF(7),      AF(8),
    /* 0x70 */
    AF(9),      AF(10),     0,          220,
    222,        0xd9,       0xdb,       0xdf,
    ALT_1,      ALT_2,      ALT_3,      ALT_4,
    ALT_5,      ALT_6,      ALT_7,      ALT_8,
    /* 0x80 */
    ALT_9,      ALT_0,      ALT_MINUS,  ALT_EQUALS, 0xe1,
    0x00,       0x00,       0x00,
    };


static int
xlat_key(KBDKEYINFO *ky)
{
    int ch = ky->chChar;

    /* Handle keypad +, -, * in-line. They are annoying */
    if (ky->chChar == 43 && ky->chScan == 78)
        return KEY_COPY;
    if (ky->chChar == 45 && ky->chScan == 74)
        return KEY_CUT;
    if (ky->chChar == 42 && ky->chScan == 55)
        return KEY_UNDO;
    if (ky->chChar == 0  && ky->chScan == 76)
        return SHIFT_KEYPAD_5;
    if (ky->chChar == 56 && ky->chScan == 72)
        return SHIFT_KEYPAD_8;
    if (ky->chChar == 52 && ky->chScan == 75)
        return SHIFT_KEYPAD_4;
    if (ky->chChar == 54 && ky->chScan == 77)
        return SHIFT_KEYPAD_6;
    if (ky->chChar == 50 && ky->chScan == 80)
        return SHIFT_KEYPAD_2;
    if (!ch || ch == 0xe0)
        ch = dos_keys[ky->chScan];
    return ch;
}


#if defined(__FLAT__)

/* Code for thread handling keyboard input
 */
static void
kbd_thread(ULONG arg)
{
    static KBDINFO kinfo = {16};

    (void)arg;
    KbdGetStatus(&kinfo, TTY_FD);
    if ((kinfo.fsMask & KEYBOARD_BINARY_MODE) == 0) {
        kinfo.fsMask = KEYBOARD_BINARY_MODE;
        KbdSetStatus(&kinfo, TTY_FD);
    }

    while (1) {
        ULONG pCount;

        DosWaitEventSem(kbd_sem, SEM_INDEFINITE_WAIT);
        KbdCharIn(&keyinfo, IO_WAIT, 0);        /* Get char */
        DosResetEventSem(kbd_sem, &pCount);
        DosPostEventSem(main_sem);
    }
}


void
sys_initialise(void)
{
    VioGetAnsi(&fansi, 0);

    x_scrfn.scr_cursor  = scr_cursor;
    x_scrfn.scr_window_size = scr_windowsize;
    x_scrfn.scr_beep    = scr_beep;
    x_scrfn.scr_print   = vio_putstring;

    DosCreateEventSem(NULL, &main_sem, 0L, 0);
    DosCreateEventSem(NULL, &kbd_sem, 0L, 0);
    if (DosCreateThread(&tid, &kbd_thread, 0L, 1L, THREAD_STACK_SIZE) != 0) {
        panic("Cannot create keyboard thread.");
    }
    DosResumeThread(tid);
}


int
main_thread_read_kbd(long tmo)
{
    ULONG pCount;

    if (timed_out) {
        timed_out = FALSE;
        if (DosQueryEventSem(main_sem, &pCount) == 0 && pCount > 0) {
            goto rd_char;
        }
        DosResumeThread(tid);
    } else  {
        DosPostEventSem(kbd_sem);
    }

    if (DosWaitEventSem(main_sem, tmo) == 0) {
rd_char:
        DosResetEventSem(main_sem, &pCount);
        return xlat_key(&keyinfo);
    }
    timed_out = TRUE;
    DosSuspendThread(tid);
    return -1;
}


#else   /*!__FLAT__*/

void
sys_intialise(void)
{
    void kbd_thread();

    VioGetAnsi(&fansi, 0);

    x_scrfn.scr_cursor = scr_cursor;
    x_scrfn.scr_window_size = scr_windowsize;
    x_scrfn.scr_beep = scr_beep;

    DosSemSet(&kbd_sem);
    DosSemSet(&main_sem);
    if (DosCreateThread(kbd_thread, &tid, kbd_thread_stack) != 0) {
        panic("Cannot create keyboard thread.");
    }
}


int
main_thread_read_kbd(long tmo)
{
    if (timed_out) {
        timed_out = FALSE;
        if (main_sem == 0)
            return xlat_keys(&keyinfo);
        DosResumeThread(tid);
    } else {
        DosSemClear(&kbd_sem);
    }

    if (DosSemWait(&main_sem, tmo) == 0) {
        DosSemSet(&main_sem);
        return xlat_keys(&keyinfo);
    }

    timed_out = TRUE;
    DosSuspendThread(tid);
    return -1;
}


/* Code for thread handling keyboard input
 */
void
kbd_thread(void)
{
    static KBDINFO kinfo = {16};

    KbdGetStatus(&kinfo, TTY_FD);
    if ((kinfo.fsMask & KEYBOARD_BINARY_MODE) == 0) {
        kinfo.fsMask = KEYBOARD_BINARY_MODE;
        KbdSetStatus(&kinfo, TTY_FD);
    }

    while (1) {
        DosSemWait(&kbd_sem, SEM_INDEFINITE_WAIT);
        DosSemSet(&kbd_sem);
        KbdCharIn(&keyinfo, IO_WAIT, 0);
        DosSemClear(&main_sem);
    }
}

#endif  /*!__FLAT__*/


/* Make sure we don't inherit the specified file descriptor when
 * we create a new process
 */
void
sys_noinherit(int fd)
{
#if defined(__FLAT__)
    DosSetFHState(fd, OPEN_FLAGS_NOINHERIT);
#else
    DosSetFHandState(fd, OPEN_FLAGS_NOINHERIT);
#endif
}


/* System specific copy, under OS/2's DosCopy is faster and may 
 * be able to preserve EA's
 */
int
sys_copy(
    const char *src, const char *dst, int perms, int owner, int group )
{
    int r;

    r = DosCopy(src, dst, DCPY_FAILEAS);
    if (r == ERROR_EAS_NOT_SUPPORTED) {         /* Can't copy with EA's */
        r = DosCopy(src, dst, 0);
    }
    if (r == 0) {
        chmod(dst, perms);
#ifdef HAVE_CHOWN
        chown(dst, owner, group);
#endif
        return TRUE;
    }
    return -1;
}


void
sys_shutdown(void)
{
    VioSetAnsi(fansi, 0);
}


static int
dos_write(char * buf, int len)
{
    return VioWrtTTY(buf, len, 0);
}


int
kbhit(void)
{
    KBDKEYINFO ky = {0};

    if (KbdPeek(&ky, 0) != 0 || !(ky.fbStatus & KBDTRF_FINAL_CHAR_IN)) {
        return 0;
    }
    return 1;
}



/*  Function:           sys_getchar
 *      Retrieve the character from the status keyboard stream, within 
 *      the specified timeout 'tmo'.
 *
 *  Parameters:
 *      fd -                File descriptor.
 *      buf -               Output buffer.
 *      tmo -               Timeout, in milliseconds.
 *
 *  Returns:
 *      On success (1), otherwise (0) unless a timeout (-1).
 */
int
sys_getchar(int fd, int *buf, accint_t tmo)
{                                      
    KBDKEYINFO ky;

    (void)fd; (void)(tmo); (void)(mt);
    if (KbdCharIn(&ky, IO_WAIT, 0) != 0) {
        return -1;
    }
    buf[0] = xlat_key(&ky);
    return 1;
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
    int buf;

    if (! timed_out) {
        if (kbhit() && sys_getchar(0, &buf, 0, NULL) > 0) {
            evt->type = EVT_KEYDOWN;            /* cooked key-code */
            evt->code = buf;
            return TRUE;
        }
    }
    return FALSE;
}


const char *
sys_delim(void)
{
    return "/";
}


static void
scr_beep(void)
{
    DosBeep(2048, 50);
}


static void
scr_windowsize(int * nrow, int * ncol)
{
    VIOMODEINFO vm = {sizeof(VIOMODEINFO)};

    VioGetMode(&vm, 0);
    *ncol = (int)vm.col;
    *nrow = (int)vm.row;
}


static void
scr_cursor(int insmode, int v_space)
{
    VIOCURSORINFO cursorinfo = { sizeof(VIOCURSORINFO) };

    VioGetCurType(&cursorinfo, 0);
    if (insmode) {
        if (v_space) {
            cursorinfo.yStart = -50;
            cursorinfo.cEnd = -100;
        } else {
            cursorinfo.yStart = -90;
            cursorinfo.cEnd = -100;
        }
    } else {
        if (v_space) {
            cursorinfo.yStart = 0;
            cursorinfo.cEnd = -50;
        } else {
            cursorinfo.yStart = 0;
            cursorinfo.cEnd = -100;
        }
    }

    cursorinfo.cx = 1;
    cursorinfo.attr = 0;
    VioSetCurType(&cursorinfo, 0);
}


#if !defined(__EMX__)
void
pause(void)
{
}
#endif


#if !defined(__FLAT__)
void
fork(void)
{
    return -1;
}
#endif


void
sys_cleanup(void)
{
}


#if !defined(__EMX__)
int
kill(int pid, int sig)
{
    if (pid < 0) {
        return DosKillProcess(0, -pid);
    }
    return DosKillProcess(1, pid);
}


int
pipe(int * p)
{
    return -1;
}
#endif


#if defined(__EMX__)
char *
getcwd(char *buffer, size_t size)
{
    return _getcwd2(buffer, size);
}
#endif


/* 
 *  sys_getcwd ---
 */
void
sys_cwdd(int drv, char *path, int size)
{
    (void)size;
    _getcwd1(cwdd, (char) drv);
}


/* 
 *  Get current drive ---
 */
int
sys_drive_get(void)
{
    ULONG drives;

#if defined(__FLAT__)
    ULONG cur_drive;

    DosQueryCurrentDisk(&cur_drive, &drives);
#else
    USHORT cur_drive;

    DosQCurDisk(&cur_drive, &drives);
#endif
    return cur_drive + 'A' - 1;
}


int
sys_fstype(const char *path)
{
    int fst = 0;
    int drv = sys_drive_get();

    union {
        FSQBUFFER2 q;
        char b[512];
    } q;
    long ql = sizeof(q);
    char buf[5];

    buf[0] = (char) drv;
    buf[1] = ':';
    buf[2] = 0;
    if (isa_integer(1)) {
        buf[0] = (char) toupper(get_xinteger(1, 0));
    }

    if (DosQueryFSAttach(buf, 1, FSAIL_QUERYNAME, (PFSQBUFFER2) &q, &ql) == 0) {
        const char *ptr = q.q.szName + q.q.cbName + 1;

        fst = *ptr;
    }
    sys_drive_set(drv);
    return fst;
}


/* 
 *  Set the current drive ---
 */
int
sys_drive_set(int ch)
{
    int ret;

    if (islower(ch))
        ch = toupper(ch);
#if defined(__FLAT__)
    ret = DosSetDefaultDisk(ch - 'A' + 1);
#else
    ret = DosSelectDisk(ch - 'A' + 1);
#endif
    return ret;
}


int
sys_enable_char(int ch, int enable)
{
    (void)ch;
    (void)enable;
    return 1;
}


/* 
 *  Routines which are machine independent but are useful for
 *  making the rest of the code portable, especially to VMS.
 */
int
sys_read(int fd, char * buf, int size)
{
    int n, osize = size;

    do {
        n = read(fd, buf, size);
        size -= n;
        buf += n;
    } while (n > 0);
    return osize - size;
}


int
sys_write(int fd, char * buf, int size)
{
    if (!xf_compat) {
        dos_write(buf, size);
        return size;
    }
    write(fd, buf, size);
    return size;
}


/*  -------------------------------------------------------------------- */

#if defined(HAVE_MOUSE)
static char *       mouse_dev = "POINTER$";   /* Device name of mouse.     */
static HMOU         mouse_fd = -1;
static int          mouse_is_active = 0;
static int          mouse_is_visible = 0;


int
sys_mouseinit(const char *dev)
{
    VIOMODEINFO mi = { sizeof(VIOMODEINFO) };
    USHORT evt = MOUSE_BN1_DOWN | MOUSE_MOTION_WITH_BN1_DOWN | \
                    MOUSE_BN2_DOWN | MOUSE_MOTION_WITH_BN2_DOWN | \
                    MOUSE_BN3_DOWN | MOUSE_MOTION_WITH_BN3_DOWN;

    (void)dev;
    VioGetMode(&mi, 0);
    if (MouOpen(mouse_dev, &mouse_fd) != 0)
        return;
    MouSetEventMask(&evt, mouse_fd);
    mouse_is_active = 1;
    mouse_is_visible = 0;
}


void
sys_mouseclose(void)
{
    if (mouse_is_active) {
        mouse_pointer(0);
        MouClose(0);
        mouse_fd = -1;
        mouse_is_active = 0;
    }
}


int
sys_mousepoll(fd_set *fds, struct MouseEvent *m)
{
    MOUEVENTINFO me;
    USHORT p = 0;
    MOUQUEINFO qi;

    (void)fds;
    if (!mouse_is_active) {
        return FALSE;
    }

    if (MouGetNumQueEl(&qi, mouse_fd) != 0 || 
            !qi.cEvents || MouReadEventQue(&me, &p, mouse_fd) != 0) {
        return FALSE;
    }

    m->x = me.col;
    m->y = me.row;
    m->b1 = (me.fs & MOUSE_BN1_DOWN) != 0;
    m->b2 = (me.fs & MOUSE_BN2_DOWN) != 0;
    m->b3 = (me.fs & MOUSE_BN3_DOWN) != 0;
    m->mutli = 0;
    return TRUE;
}


void
sys_mousepointer(int state)
{
    if (mouse_is_active) {
        if (state) {
            if (!mouse_is_visible) {
                MouDrawPtr(mouse_fd);
            }
            mouse_is_visible = 1;
        
        } else if (mouse_is_visible) {
            NOPTRRECT pr;

            pr.row = 0;
            pr.col = 0;
            pr.cRow = ttrows() - 1;
            pr.cCol = ttcols() - 1;
            MouRemovePtr(&pr, mouse_fd);
            mouse_is_visible = 0;
        }
    }
}
#endif  /*HAVE_MOUSE*/

#endif  /*__OS2__*/

