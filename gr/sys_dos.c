#include <edidentifier.h>
__CIDENT_RCSID(gr_sys_dos_c,"$Id: sys_dos.c,v 1.38 2023/09/10 16:35:52 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: sys_dos.c,v 1.38 2023/09/10 16:35:52 cvsuser Exp $
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
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <edalt.h>
#include <edtermio.h>

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "m_pty.h"
#include "main.h"
#include "mouse.h"
#include "system.h"
#include "tty.h"

#if defined(__MSDOS__) && !defined(WIN32)
#if defined(DJGPP)
#include <dpmi.h>
#endif
#include "vio.h"
#include "dos.h"
#include "bios.h"

/* codes for msdos mouse event */
#define MSDOS_MOUSE_LEFT        0x01            /* button 1 */
#define MSDOS_MOUSE_RIGHT       0x02            /* button 2 */
#define MSDOS_MOUSE_MIDDLE      0x04            /* button 3 */

/* mouse status */
#define MOUSE_LEFT              0x0001
#define MOUSE_RIGHT             0x0010
#define MOUSE_MIDDLE            0x0100
#define MOUSE_DOUBLE            0x1000
#define MOUSE_DRAG              0x2000
#define MOUSE_RELEASE           0x4000

static int      p_mouset = 300;                 /* double tick timeout (ms) */

static unsigned mouse_avail = FALSE;            /* mouse present */
static int      mouse_click = 0;                /* mouse status */

#if defined(HAVE_MOUSE)
static int      mouse_isactive;                 /* mouse enabled */
static int      mouse_ishidden;                 /* mouse not shown */
static int      mouse_last_click = -1;          /* previous status at click */
static int      mouse_x = -1;                   /* mouse x coodinate */
static int      mouse_y = -1;                   /* mouse y coodinate */
static long     mouse_click_time = 0;           /* biostime() of last click */
static int      mouse_click_count = 0;          /* count for multi-clicks */
static int      mouse_click_x = 0;              /* x of previous mouse click */
static int      mouse_click_y = 0;              /* y of previous mouse click */
static int      mouse_x_div = 8;                /* column = x coord / mouse_x_div */
static int      mouse_y_div = 8;                /* line   = y coord / mouse_y_div */
#endif /*HAVE_MOUSE*/

static struct {
    int         viokey;
    KEY         key;
} vio_keytbl[] =
    {
        { VIOKEY_CTRLA,         CTRL_A },
        { VIOKEY_CTRLB,         CTRL_B },
        { VIOKEY_CTRLC,         CTRL_C },
        { VIOKEY_CTRLD,         CTRL_D },
        { VIOKEY_CTRLE,         CTRL_E },
        { VIOKEY_CTRLF,         CTRL_F },
        { VIOKEY_CTRLG,         CTRL_G },
        { VIOKEY_CTRLH,         CTRL_H },
        { VIOKEY_CTRLI,         CTRL_I },
        { VIOKEY_CTRLJ,         CTRL_J },
        { VIOKEY_CTRLK,         CTRL_K },
        { VIOKEY_CTRLL,         CTRL_L },
        { VIOKEY_CTRLM,         CTRL_M },
        { VIOKEY_CTRLN,         CTRL_N },
        { VIOKEY_CTRLO,         CTRL_O },
        { VIOKEY_CTRLP,         CTRL_P },
        { VIOKEY_CTRLQ,         CTRL_Q },
        { VIOKEY_CTRLR,         CTRL_R },
        { VIOKEY_CTRLS,         CTRL_S },
        { VIOKEY_CTRLT,         CTRL_T },
        { VIOKEY_CTRLU,         CTRL_U },
        { VIOKEY_CTRLV,         CTRL_V },
        { VIOKEY_CTRLW,         CTRL_W },
        { VIOKEY_CTRLX,         CTRL_X },
        { VIOKEY_CTRLY,         CTRL_Y },
        { VIOKEY_CTRLZ,         CTRL_Z },
        { VIOKEY_CTRLTAB,       CTRL_TAB },
//      { VIOKEY_CTRLINS,       },                      /* Ctrl Insert          */
//      { VIOKEY_CTRLOPENB,     },                      /* Ctrl Open bracket    */
//      { VIOKEY_CTRLCLOSEB,    },                      /* Ctrl Close bracket   */
//      { VIOKEY_CTRLSTAR,      },                      /* Ctrl *               */
//      { VIOKEY_CTRLSLASH,     },                      /* Ctrl /               */
//      { VIOKEY_CTRLBSLASH,    },                      /* Ctrl \               */
//      { VIOKEY_CTRLMINUS,     },                      /* Ctrl -               */
        { VIOKEY_CTRLBS,        CTRL_BACKSPACE },       /* Ctrl Backspace       */

        { VIOKEY_F1,            F(1) },
        { VIOKEY_F2,            F(2) },
        { VIOKEY_F3,            F(3) },
        { VIOKEY_F4,            F(4) },
        { VIOKEY_F5,            F(5) },
        { VIOKEY_F6,            F(6) },
        { VIOKEY_F7,            F(7) },
        { VIOKEY_F8,            F(8) },
        { VIOKEY_F9,            F(9) },
        { VIOKEY_F10,           F(10) },
        { VIOKEY_F11,           F(11) },
        { VIOKEY_F12,           F(12) },
        { VIOKEY_SF1,           SF(1) },
        { VIOKEY_SF2,           SF(2) },
        { VIOKEY_SF3,           SF(3) },
        { VIOKEY_SF4,           SF(4) },
        { VIOKEY_SF5,           SF(5) },
        { VIOKEY_SF6,           SF(6) },
        { VIOKEY_SF7,           SF(7) },
        { VIOKEY_SF8,           SF(8) },
        { VIOKEY_SF9,           SF(9) },
        { VIOKEY_SF10,          SF(10) },
        { VIOKEY_SF11,          SF(11) },
        { VIOKEY_SF12,          SF(12) },
        { VIOKEY_CF1,           CF(1) },
        { VIOKEY_CF2,           CF(2) },
        { VIOKEY_CF3,           CF(3) },
        { VIOKEY_CF4,           CF(4) },
        { VIOKEY_CF5,           CF(5) },
        { VIOKEY_CF6,           CF(6) },
        { VIOKEY_CF7,           CF(7) },
        { VIOKEY_CF8,           CF(8) },
        { VIOKEY_CF9,           CF(9) },
        { VIOKEY_CF10,          CF(10) },
        { VIOKEY_CF11,          CF(11) },
        { VIOKEY_CF12,          CF(12) },
        { VIOKEY_AF1,           AF(1) },
        { VIOKEY_AF2,           AF(2) },
        { VIOKEY_AF3,           AF(3) },
        { VIOKEY_AF4,           AF(4) },
        { VIOKEY_AF5,           AF(5) },
        { VIOKEY_AF6,           AF(6) },
        { VIOKEY_AF7,           AF(7) },
        { VIOKEY_AF8,           AF(8) },
        { VIOKEY_AF9,           AF(9) },
        { VIOKEY_AF10,          AF(10) },
        { VIOKEY_AF11,          AF(11) },
        { VIOKEY_AF12,          AF(12) },

        { VIOKEY_ALT1,          ALT_0 },
        { VIOKEY_ALT2,          ALT_1 },
        { VIOKEY_ALT3,          ALT_2 },
        { VIOKEY_ALT4,          ALT_3 },
        { VIOKEY_ALT5,          ALT_4 },
        { VIOKEY_ALT6,          ALT_5 },
        { VIOKEY_ALT7,          ALT_6 },
        { VIOKEY_ALT8,          ALT_7 },
        { VIOKEY_ALT9,          ALT_8 },
        { VIOKEY_ALT0,          ALT_9 },
        { VIOKEY_ALTA,          ALT_A },
        { VIOKEY_ALTB,          ALT_B },
        { VIOKEY_ALTC,          ALT_C },
        { VIOKEY_ALTD,          ALT_D },
        { VIOKEY_ALTE,          ALT_E },
        { VIOKEY_ALTF,          ALT_F },
        { VIOKEY_ALTG,          ALT_G },
        { VIOKEY_ALTH,          ALT_H },
        { VIOKEY_ALTI,          ALT_I },
        { VIOKEY_ALTJ,          ALT_J },
        { VIOKEY_ALTK,          ALT_K },
        { VIOKEY_ALTL,          ALT_L },
        { VIOKEY_ALTM,          ALT_M },
        { VIOKEY_ALTN,          ALT_N },
        { VIOKEY_ALTO,          ALT_O },
        { VIOKEY_ALTP,          ALT_P },
        { VIOKEY_ALTQ,          ALT_Q },
        { VIOKEY_ALTR,          ALT_R },
        { VIOKEY_ALTS,          ALT_S },
        { VIOKEY_ALTT,          ALT_T },
        { VIOKEY_ALTU,          ALT_U },
        { VIOKEY_ALTV,          ALT_V },
        { VIOKEY_ALTW,          ALT_W },
        { VIOKEY_ALTX,          ALT_X },
        { VIOKEY_ALTY,          ALT_Y },
        { VIOKEY_ALTZ,          ALT_Z },

        { VIOKEY_ALTMINUS,      __ALT('-') },           /* Alt -                */
        { VIOKEY_ALTEQUAL,      __ALT('=') },           /* Alt =                */
        { VIOKEY_ALTBSLASH,     __ALT('\\') },          /* Alt \                */
        { VIOKEY_ALTOPENB,      __ALT('{') },           /* Alt open bracket     */
        { VIOKEY_ALTCLOSEB,     __ALT('}') },           /* Alt close bracket    */
        { VIOKEY_ALTCOMMA,      __ALT(',') },           /* Alt ,                */
        { VIOKEY_ALTPERIOD,     __ALT('.') },           /* Alt .                */
        { VIOKEY_ALTSLASH,      __ALT('/') },           /* Alt /                */
        { VIOKEY_ALTACCENT,     __ALT('`') },           /* Alt `                */
        { VIOKEY_ALTAPOST,      __ALT('\'') },          /* Alt ' Apostrophe     */
        { VIOKEY_ALTSEMI,       __ALT(';') },           /* Alt Semicolon        */
        { VIOKEY_ALTTAB,        ALT_TAB },              /* Alt Tab              */
        { VIOKEY_ALTBS,         ALT_BACKSPACE },        /* Alt Backspace        */
//      { VIOKEY_ALTESC,        },                      /* Alt Esc              */
//      { VIOKEY_ALTENTER,      },                      /* Alt enter            */

        { VIOKEY_INS,           KEY_INS },              /* 0  Insert            */
        { VIOKEY_END,           KEY_END },              /* 1  End               */
        { VIOKEY_DOWN,          KEY_DOWN },             /* 2  Cursor down       */
        { VIOKEY_PGDN,          KEY_PAGEDOWN },         /* 3  Page down         */
        { VIOKEY_LEFT,          KEY_LEFT },             /* 4  Cursor left       */
        { VIOKEY_CNTR,          KEYPAD_5 },             /* 5                    */
        { VIOKEY_RIGHT,         KEY_RIGHT },            /* 6  Right             */
        { VIOKEY_HOME,          KEY_HOME },             /* 7  Home              */
        { VIOKEY_UP,            KEY_UP },               /* 8  Up                */
        { VIOKEY_PGUP,          KEY_PAGEUP },           /* 9  Page up           */
        { VIOKEY_DEL,           KEY_DEL },              /* 10 Delete            */
        { VIOKEY_KPPLUS,        KEYPAD_PLUS },          /* 11 Keypad +          */
        { VIOKEY_KPMINUS,       KEYPAD_MINUS },         /* 12 Keypad -          */
        { VIOKEY_KPSTAR,        KEYPAD_STAR },          /* 13 *                 */
        { VIOKEY_KPSLASH,       KEYPAD_DIV },           /* 14 /                 */
//      { VIOKEY_KPEQUAL,       KEYPAD_EQUAL },         /* 15                   */
        { VIOKEY_KPENTER,       KEYPAD_ENTER },         /* 16                   */
//      {                       KEYPAD_PAUSE },         /* 17                   */
//      {                       KEYPAD_PRTSC },         /* 18                   */
//      {                       KEYPAD_SCROLL },        /* 19                   */
//      {                       KEYPAD_NUMLOCK },       /* 20                   */

        { VIOKEY_STAB,          BACK_TAB },             /* Shift tab            */

        { VIOKEY_SHIFTINS,      SHIFT_KEYPAD_0 },       /* 0  Shift Insert      */
        { VIOKEY_SHIFTEND,      SHIFT_KEYPAD_1 },       /* 1  Shift End         */
        { VIOKEY_SHIFTDOWN,     SHIFT_KEYPAD_2 },       /* 2  Shift Cursor down */
        { VIOKEY_SHIFTPGDN,     SHIFT_KEYPAD_3 },       /* 3  Shift Page down   */
        { VIOKEY_SHIFTLEFT,     SHIFT_KEYPAD_4 },       /* 4  Shift Cursor left */
        { VIOKEY_SHIFTRIGHT,    SHIFT_KEYPAD_6 },       /* 6  Shift Right       */
        { VIOKEY_SHIFTHOME,     SHIFT_KEYPAD_7 },       /* 7  Shift Home        */
        { VIOKEY_SHIFTUP,       SHIFT_KEYPAD_8 },       /* 8  Shift Up          */
        { VIOKEY_SHIFTPGUP,     SHIFT_KEYPAD_9 },       /* 9  Shift Page up     */
        { VIOKEY_SHIFTDEL,      SHIFT_KEYPAD_DEL },     /* 10 Shift Delete      */
        { VIOKEY_SHIFTKPPLUS,   SHIFT_KEYPAD_PLUS },    /* 11 Shift Keypad +    */
        { VIOKEY_SHIFTKPMINUS,  SHIFT_KEYPAD_MINUS },   /* 12 Shift Keypad -    */
        { VIOKEY_SHIFTKPSTAR,   SHIFT_KEYPAD_STAR },    /* 13 Shift Keypad *    */
        { VIOKEY_SHIFTKPSLASH,  SHIFT_KEYPAD_DIV },     /* 14 Shift Keypad /    */
//      {                       SHIFT_KEYPAD_EQUAL },   /* 15                   */
        { VIOKEY_SHIFTKPENTER,  SHIFT_KEYPAD_ENTER },   /* 16                   */
//      {                       SHIFT_KEYPAD_PAUSE },   /* 17                   */
//      {                       SHIFT_KEYPAD_PRTSC },   /* 18                   */
//      {                       SHIFT_KEYPAD_SCROLL },  /* 19                   */
//      {                       SHIFT_KEYPAD_NUMLOCK }, /* 20                   */

        { VIOKEY_CTRLINS,       CTRL_KEYPAD_0 },        /* 0  Ctrl Insert       */
        { VIOKEY_CTRLEND,       CTRL_KEYPAD_1 },        /* 1  Ctrl End          */
        { VIOKEY_CTRLDOWN,      CTRL_KEYPAD_2 },        /* 2  Ctrl Cursor down  */
        { VIOKEY_CTRLPGDN,      CTRL_KEYPAD_3 },        /* 3  Ctrl PgDn         */
        { VIOKEY_CTRLLEFT,      CTRL_KEYPAD_4 },        /* 4  Ctrl Cursor left  */
        { VIOKEY_CTRLCNTR,      CTRL_KEYPAD_5 },        /* 5  Ctrl Center       */
        { VIOKEY_CTRLRIGHT,     CTRL_KEYPAD_6 },        /* 6  Ctrl Cursor right */
        { VIOKEY_CTRLHOME,      CTRL_KEYPAD_7 },        /* 7  Ctrl Home         */
        { VIOKEY_CTRLUP,        CTRL_KEYPAD_8 },        /* 8  Ctrl Cursor up    */
        { VIOKEY_CTRLPGUP,      CTRL_KEYPAD_9 },        /* 9  Ctrl PgUp         */
        { VIOKEY_CTRLDEL,       CTRL_KEYPAD_DEL },      /* 10 Ctrl Delete       */
        { VIOKEY_CTRLKPPLUS,    CTRL_KEYPAD_PLUS },     /* 11 Ctrl Keypad +     */
        { VIOKEY_CTRLKPMINUS,   CTRL_KEYPAD_MINUS },    /* 12 Ctrl Keypad -     */
        { VIOKEY_CTRLKPSTAR,    CTRL_KEYPAD_STAR },     /* 13 Ctrl Keypad *     */
        { VIOKEY_CTRLKPSLASH,   CTRL_KEYPAD_DIV },      /* 14 Ctrl Keypad /     */
//      {                       CTRL_KEYPAD_EQUAL },    /* 15                   */
        { VIOKEY_CTRLKPENTER,   CTRL_KEYPAD_ENTER },    /* 16                   */
//      {                       CTRL_KEYPAD_PAUSE },    /* 17                   */
        { VIOKEY_CTRLPRTSC,     CTRL_KEYPAD_PRTSC },    /* 18 Ctrl Print Screen */
//      {                       CTRL_KEYPAD_SCROLL },   /* 19                   */
//      {                       CTRL_KEYPAD_NUMLOCK },  /* 20                   */

        { VIOKEY_ALTINS,        __ALT_KEYPAD(0)},       /* 0  Alt Ins           */
        { VIOKEY_ALTEND,        ALT_KEYPAD_END },       /* 1  Alt End           */
        { VIOKEY_ALTDOWN,       __ALT_KEYPAD(2) },      /* 2  Alt Cursor down   */
        { VIOKEY_ALTPGDN,       __ALT_KEYPAD(3) },      /* 3  Alt PgDn          */
        { VIOKEY_ALTLEFT,       __ALT_KEYPAD(4) },      /* 4  Alt Cursor left   */
//      { VIOKEY_ALTCNTR,       __ALT_KEYPAD(5) },      /* 5  Alt Center        */
        { VIOKEY_ALTRIGHT,      __ALT_KEYPAD(6) },      /* 6  Alt Cursor right  */
        { VIOKEY_ALTHOME,       ALT_KEYPAD_HOME },      /* 7  Alt Home          */
        { VIOKEY_ALTUP,         __ALT_KEYPAD(8) },      /* 8  Alt Cursor up     */
        { VIOKEY_ALTPGUP,       __ALT_KEYPAD(9) },      /* 9  Alt PgUp          */
        { VIOKEY_ALTDEL,        __ALT_KEYPAD(10) },     /* 10 Alt Del           */
        { VIOKEY_ALTKPPLUS,     __ALT_KEYPAD(11) },     /* 11 Alt Keypad +      */
        { VIOKEY_ALTKPMINUS,    ALT_KEYPAD_MINUS },     /* 12 Alt Keypad -      */
//      { VIOKEY_ALTKPSTAR,     __ALT_KEYPAD(13) },     /* 13 Alt Keypad *      */
        { VIOKEY_ALTKPSLASH,    __ALT_KEYPAD(14) },     /* 14 Alt keypad /      */
//      {                       __ALT_KEYPAD(15) },     /* 15                   */
        { VIOKEY_ALTKPENTER,    ALT_KEYPAD_ENTER },     /* 16 Alt keypad enter  */
//      {                       __ALT_KEYPAD(17) },     /* 17                   */
//      {                       __ALT_KEYPAD(18) },     /* 18 Alt Print Screen  */
//      {                       __ALT_KEYPAD(19) },     /* 19                   */
//      {                       __ALT_KEYPAD(20) },     /* 20                   */

        { VIOKEY_STAB,          BACK_TAB },             /* Shift tab            */
    };


/*
 *  sys_initialise/shutdown ---
 *      System speific initialisation and shutdown.
 */
void
sys_initialise(void)
{
}


void
sys_shutdown(void)
{
}


void
sys_cleanup(void)
{
}


/*
 *  sys_noinherit ---
 *      Make sure we don't inherit the specified file descriptor when
 *      we create a new process.
 */
void
sys_noinherit(int fd)
{
#if defined(FIOCLEX)
    fcntl(fd, FIOCLEX, 0);

#elif defined(DJGPPP)
    fcntl(fd, FD_SETFD, FD_COLEXEC);

#else
    (void)fd;
#endif
}


/*
 *  sys_setup_pty ---
 *      Function called from PTY code to set up terminal modes when
 *      we create a new shell buffer.
 */
void
sys_pty_mode(int fd)
{
    (void) fd;
}


#if defined(HAVE_MOUSE)
/*
 *  sys_mouseinit ---
 *      Open the mouse device.
 */
int
sys_mouseinit(const char *dev)
{
    union REGS regs;

    (void) dev;

    /* find out if a MS compatible mouse is available */
    regs.x.ax = 0;
    (void)int86(0x33, &regs, &regs);
    if ((mouse_avail = regs.x.ax) != 0) {
        int rows, columns;

        ttgetsize(&rows, &columns);
        if (columns <= 40) {
            mouse_x_div = 16;
        }
        if (rows == 30) {
            mouse_y_div = 16;
        }
    }

    trace_ilog("mouse_init(%s) : %d\n", dev, mouse_avail);
    mouse_isactive = TRUE;

    return (mouse_avail);
}


/*
 *  sys_mouseclose ---
 *      Close the mouse device.
 */
void
sys_mouseclose(void)
{
}


/*
 *  mouse_area ---
 *      Set area where mouse can be moved to (ie the whole screen).
 */
static void
mouse_area(void)
{
    union REGS regs;

    if (mouse_avail) {
        int rows, columns;

        ttgetsize(&rows, &columns);
        regs.x.cx = 0;                  /* mouse visible between cx and dx */
        regs.x.dx = columns * mouse_x_div - 1;
        regs.x.ax = 7;
        (void)int86(0x33, &regs, &regs);

        regs.x.cx = 0;                  /* mouse visible between cx and dx */
        regs.x.dx = rows * mouse_y_div - 1;
        regs.x.ax = 8;
        (void)int86(0x33, &regs, &regs);
    }
}


int
sys_mousepoll(fd_set *fds, struct MouseEvent *m)
{
    (void) fds;

    if (!mouse_avail || !mouse_click) {
        return FALSE;
    }

    m->b1    = (mouse_click & MOUSE_LEFT);
    m->b2    = (mouse_click & MOUSE_RIGHT);
    m->b3    = (mouse_click & MOUSE_MIDDLE);
    m->multi = (mouse_click & MOUSE_DOUBLE);
    if (mouse_click & MOUSE_DRAG) {
        m->x = mouse_x;
        m->y = mouse_y;
    } else {
        m->x = mouse_click_x;
        m->y = mouse_click_y;
    }
    mouse_click = 0;
    return TRUE;
}


void
sys_mousepointer(int on)
{
    static int was_on = FALSE;
    union REGS regs;

    if (mouse_avail) {
        if (!mouse_isactive || mouse_ishidden)
            on = FALSE;

        if ((on && !was_on) || (!on && was_on)) {
            was_on = on;
            regs.x.ax = on ? 1 : 2;
            int86(0x33, &regs, &regs);          /* show mouse */
            if (on)
                mouse_area();
        }
    }
}
#endif  /*HAVE_MOUSE*/


#define BIOSTICK     55                         /* biostime() about every 55 msec */

static KEY
vio_keyxlat(int key)
{
   register int i;

   for (i = 0; i<(int)(sizeof(vio_keytbl)/sizeof(vio_keytbl[0])); i++)
      if (key == vio_keytbl[i].viokey)
         return vio_keytbl[i].key;              /* xlate key code */
   return ((KEY)key);                           /* unknown, just return */
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
#if defined(HAVE_MOUSE)
    union REGS regs;
    int x, y;
#endif
    long starttime, curtime;
    int ret = -1;

    (void) fd;

    starttime = biostime(0, 0L);
    if (tmo <= 0)
        tmo = 0;
    else if ((tmo = tmo / BIOSTICK) < 1)        /* convert to ticks */
        tmo = 1;

    for (;;) {
#if defined(HAVE_MOUSE)
        long clicktime;
        static int last_status = 0;

        if (mouse_avail && mouse_isactive && !mouse_click) {
            regs.x.ax = 3;
            int86(0x33, &regs, &regs);          /* check mouse status */

            x = regs.x.cx / mouse_x_div;
            y = regs.x.dx / mouse_y_div;

            if ((last_status == 0) != (regs.x.bx == 0)) {

                if (last_status) {              /* button up */
                    mouse_click = MOUSE_RELEASE;

                } else {                        /* button down */
                    /* Translate mouse events */
                    if (regs.x.bx & MSDOS_MOUSE_LEFT) {
                        mouse_click  = MOUSE_LEFT;
                    } else if (regs.x.bx & MSDOS_MOUSE_RIGHT) {
                        mouse_click  = MOUSE_RIGHT;
                    } else if (regs.x.bx & MSDOS_MOUSE_MIDDLE) {
                        mouse_click  = MOUSE_MIDDLE;
                    }

                    /* Find out if this is a multi-click */
                    clicktime = biostime(0, 0L);
                    if (mouse_click_x == x && mouse_click_y == y &&
                            mouse_click_count != 4 &&
                            mouse_click == mouse_last_click &&
                            clicktime < mouse_click_time + p_mouset / BIOSTICK) {
                        mouse_click_count++;
                    } else {
                        mouse_click_count = 1;
                    }

                    mouse_click_time = clicktime;
                    mouse_last_click = mouse_click;
                    mouse_click_x = x;
                    mouse_click_y = y;
                    if (mouse_click_count > 1)
                        mouse_click |= MOUSE_DOUBLE;
                }
            } else if (last_status && (x != mouse_x || y != mouse_y)) {
                mouse_click = MOUSE_DRAG;
            }

            last_status = regs.x.bx;

            if (mouse_ishidden &&
                        mouse_x >= 0 && (mouse_x != x || mouse_y != y)) {
                mouse_ishidden = FALSE;
                sys_mousepointer(TRUE);
            }
            mouse_x = x;
            mouse_y = y;

            return 0;
        }
#endif  /*HAVE_MOUSE*/

        if (VioKeyHit()) {                      /* priority ? */
            ret = vio_keyxlat(VioKeyGet());     /* blocked read */
            break;
        }

        curtime = biostime(0, 0L);
        if (tmo != 0 &&                         /* spin ? */
                (curtime < starttime || curtime > starttime + tmo)) {
            break;
        }

#if defined(DJGPP)
        __dpmi_yield();                         /* keep windows happy */
#endif
    }
    if (ret == -1)
        return (-1);                            /* ctrl-break/timeout */
    buf[0] = ret;
    return (1);
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

    if (VioKeyHit() && sys_getchar(0, &buf, 0) > 0) {
        evt->type = EVT_KEYDOWN;                /* cooked key-code */
        evt->code = buf;
        return TRUE;
    }
    return FALSE;
}


const char *
sys_delim(void)
{
#if defined(DJGPP)
    return "/";
#else
    return "\\";
#endif
}


const char *
sys_getshell(void)
{
    const char *shname;

    shname = ggetenv("SHELL");
    if (shname == NULL)
        shname = ggetenv("COMSPEC");
#if defined(DJGPP)
    if (shname == NULL)
        shname = "sh";
#else
    if (shname == NULL)
        shname = "command";
#endif
    return shname;
}



/*
 *  sys_getcwd ---
 *      Retrieve the current working directory for the specified drive.
 */
void
sys_cwdd(int drv, char *path, int size)
{
    int odrv;

    if ((odrv = sys_drive_get()) != drv) {
        sys_drive_set(drv);
    }
    getcwd(path, size);
    if (odrv != drv) {
        sys_drive_set(odrv);
    }
}


/*
 *  sys_drive_get ---
 *      Get the current drive
 */
int
sys_drive_get(void)
{
    int ret;
    (void)_dos_getdrive(&ret);
    return (ret - 1 + 'A');
}


/*
 *  sys_drive_set ---
 *      Set the current drive
 */
int
sys_drive_set(int ch)
{
    int ret;
    _dos_setdrive(toupper(ch)-'A'+1, &ret);     /* 1=A, 2=B .... */
    return (0);
}


/* Routine called by input_mode() primitive to enable certain
 * keys to be seen by Grief, e.g. XON/XOFF, ^Z.
 */
int
sys_enable_char(int ch, int state)
{
    (void) ch;
    (void) state;
    return 1;
}


/*
 *  Copy a file -- returns -1, force use of default
 */
int
sys_copy(
    const char *src, const char *dst, int perms, int owner, int group )
{
    (void)src;
    (void)dst;
    (void)perms;
    (void)owner;
    (void)group;

    return -1;                                  /* default */
}


int
sys_read(int fd, void *buf, int size)
{
    void *data = (char *)buf;
    int n, osize = size;

    while (size > 0) {
        if ((n = read(fd, data, size)) <= 0) {
            if (0 == n) {
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
sys_write(int fd, void *buf, int size)
{
    return write(fd, buf, size);
}


int
sys_realpath(const char *name, char *buf)
{
    register const char *i = name;
    register char *o = buf;
    char sep;

    sep = strchr(name, '/') ? '/' : DIRSEP;     /* handle UNIX, default MSDOS */

   /*   Takes as input an arbitrary path.  Fixes up the path by:
    *     1. Convert relative path to absolute
    *     2. Removing consecutive slashes
    *     3. Removing trailing slashes
    *     4. Making the path absolute if it wasn't already
    *     5. Removing "." in the path
    *     6. Removing ".." entries in the path (and the directory above them)
    */

    /* Convert relative path to absolute */
    {
        int drive;

        if (*(i+1) == ':' &&
                ((*i >= 'a' && *i <= 'z') || (*i >= 'A' && *i <= 'Z'))) {
            *o++ = toupper(*i++);
            *o++ = *i++;
        } else {
            *o++ = sys_drive_get();
            *o++ = ':';
        }

        if (! is_slash(*i)) {                   /* <cwd>/fn */
            drive = *buf - 'A' + 1;
            *o = '\0';
            sys_cwdd(drive, o, MAX_PATH);
            if (*o) {
                if (':' == o[1]) {              /* remove drive, if any */
                    memmove(o, (const char *)(o + 2), MAX_PATH - 2);
                }
                while (*o) {
                    *o++;
                }
            }
        }
    }

    /* Step through the input path */
    while (*i != '\0') {
        if (is_slash(*i)) {                     /* skip input slashes */
            i++;
            continue;
        }

        if (*i == '.' && is_term(*(i + 1))) {   /* skip "." and output nothing */
            i++; continue;
        }

                                                /* skip ".." and remove previous output directory */
        if (*i == '.' && *(i + 1) == '.' && is_term(*(i + 2))) {
            /* Don't back up over drive spec or consume ../.. */
            if (o > out+2 && strncmp(o-2, "..", 2) != 0) {
                /* Consume .. and parent directory */
                i += 2;
                while (--o > out+2 && !is_slash(*o)) {
                    ;
                }
                continue;
            }
        }
        *o++ = sep;                             /* copy path component from in to p */

        while (! is_term(*i)) {
            *o++ = *i++;                        /* copy next filename/directory */
        }
    }
    *o = '\0';
    return buf;
}

#endif  /*__MSDOS__*/
