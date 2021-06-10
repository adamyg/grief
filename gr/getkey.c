#include <edidentifier.h>
__CIDENT_RCSID(gr_getkey_c,"$Id: getkey.c,v 1.45 2021/06/10 06:13:02 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: getkey.c,v 1.45 2021/06/10 06:13:02 cvsuser Exp $
 * Low level input, both keyboard and mouse.
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

#define ED_LEVEL 1
#define ED_ASSERT

#include <editor.h>
#include <edtermio.h>
#include <eddebug.h>
#include <edassert.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */

#if defined(HAVE_SYS_TIME_H)
#include <sys/time.h>                           /* select/timeval */
#endif
#include <edalt.h>
#if defined(HAVE_POLL)
#include <poll.h>
#endif
#if defined(__CYGWIN__)
#include <windows.h>
#endif

#include "accum.h"                              /* acc_...() */
#include "builtin.h"
#include "display.h"
#include "echo.h"
#include "eval.h"                               /* get_...()/isa_...() */
#include "getkey.h"
#include "keyboard.h"
#include "m_pty.h"
#include "main.h"
#include "mouse.h"
#include "playback.h"
#include "procspawn.h"
#include "register.h"
#include "symbol.h"
#include "system.h"
#include "tty.h"

#if defined(__OS2__)
#define INCL_BASE
#define INCL_NOPM
#include "os2.h"
#endif

#define DEF_TIMEOUT     30                      /* default timeout in seconds. */
#define UPDATE_DELAY    3                       /* update delay (in seconds). */

#define STATE_1ST       0x0002                  /* wait for first key pressed. */
#define STATE_2ND       0x0004                  /* wait for subsequent key of a function key. */
#define STATE_RAW       0x0008                  /* wait for the set timeout (exclusive). */
#define STATE_KEYS      (STATE_1ST | STATE_2ND | STATE_RAW)
#define STATE_IDLE      0x0010                  /* idle timer active. */
#define STATE_PTY       0x0020                  /* PTY active - need to sample at 1sec interval. */

#define WAIT_IDLE       0x0100                  /* idle timer active. */
#define WAIT_PTY        0x0200                  /* PTY active - need to sample at 1sec interval. */
#define WAIT_MINUTE     0x0400                  /* minute passed. */
#define WAIT_TIMEDOUT   0x0800                  /* timeout past to getkey() expired. */
#define WAIT_2ND        0x1000                  /* second character timeout. */

enum {
    ESCDELAY_DEFAULT = 0,
    ESCDELAY_ENV,
    ESCDELAY_SET,
    ESCDELAY_CMDLINE
};

typedef struct {
    uint64_t            value;
} IOTimer_t;

static int              io_wait(int state, struct IOEvent *evt, accint_t tmo);
static void             io_escimport(void);

static int              x_waitstate = 0;

static int              x_escdelay = -1;        /* ESC delay */
static int              x_escsource;

static int              x_kbdq;                 /* pending key event */

static KEY *            x_pushback = NULL;      /* local push back */
static KEY              x_pushbuffer[32];

#if defined(HAVE_SELECT) && \
        !defined(__OS2__) && !defined(__MSDOS__)
static fd_set           x_fdset;                /* input file descriptors */
static int              x_fdmax;                /* upper range */
#endif
#if defined(HAVE_POLL)
static int              x_pollcnt;              /* file descriptors */
static struct pollfd    x_pollfds[16];
#endif

#if !defined(HAVE_SELECT) || \
        defined(__OS2__) || defined(__MSDOS__) || \
       (defined(WIN32) && !defined(__CYGWIN__))
#define HAVE_X_POLL_TIMER
static time_t           x_poll_timer;
#endif

static time_t           x_minute_timer;         /* one minute timer */
static IOTimer_t        x_update_timer;         /* update timer */
static IOTimer_t        x_idle_timer;           /* idle timer */

static uint64_t         x_iotimer;              /* timer reference */
static time_t           x_ionow;                /* current time */


static void
iot_current(void)
    {
        int ms = 0;
        x_ionow = sys_time(&ms);
        x_iotimer = ((uint64_t)x_ionow * 1000) + ms;
    }


static void
iot_start(IOTimer_t *tmr, int tmo)
    {
        if (tmo > 0) tmr->value = x_iotimer + tmo;
        else tmr->value = 0;
    }


static void
iot_starts(IOTimer_t *tmr, int tmo)
    {
        if (tmo > 0) tmr->value = x_iotimer + ((uint64_t)tmo * 1000);
        else tmr->value = 0;
    }


static void
iot_stop(IOTimer_t *tmr)
    { tmr->value = 0; }


static int
iot_expired(const IOTimer_t *tmr)
    { return (tmr->value && x_iotimer >= tmr->value); }


static int
iot_active(const IOTimer_t *tmr)
    { return (tmr->value ? 1 : 0); }


static int
iot_remaining(const IOTimer_t *tmr)
    {
        const uint64_t value = tmr->value;

        if (value > 0) {
            if (x_iotimer >= value) {           /* expired */
                return 0;
            }
            return (int)(value - x_iotimer);
        }
        return -1;
    }


/*  Function:           io_wait
 *      Low-level keyboard input wait, invoked by <io_check> to
 *      actually read from the keyboard if io_check could not satisfy
 *      the request from the various internal buffers.
 *
 *      'state' indicates what sort of thing we are waiting for, and
 *      buf receives the data retrieved.
 *
 *      tmo indicates a timeout. If zero, then the calling function
 *      only wants to timeout on the designated event. Otherwise, tmo
 *      states the upper timeout in milliseconds.
 *
 *  Parameters:
 *      state - Current i/o status.
 *      buf - Reply buffer.
 *      utmo - Timeout in milliseconds.
 *
 *  Returns:
 *      Zero (0) on success, otherwise one the manifest WAIT constants.
 */
static int
io_wait(int state, struct IOEvent *evt, accint_t utmo)
{
    int tmo = EVT_SECOND(DEF_TIMEOUT);          /* default */
    int ret, event = 0;

#if defined(HAVE_SELECT) && \
        !defined(__OS2__) && !defined(__MSDOS__)
    static struct timeval tv = {0};
    fd_set fds;
    int nfds;
#endif

    evt->type = EVT_NONE;                       /* default return */
    evt->code = 0;

    /*
     *  setup timer/
     *      STATE_RAW       Absolute timout.
     *
     *  or  default         30 seconds.
     *      STATE_1ST       One minute.
     *      STATE_IDLE      Next idle event (1 .. 30 seconds).
     *      STATE_PTY       One second.
     *      STATE_2ND       ESC delay (~750 ms).
     *      utmo            If callers/user specified timeout is shorter.
     */
    if (STATE_RAW & state) {                    /* 'utmo' is absolute */
        event = WAIT_TIMEDOUT;
        tmo = utmo;

    } else if (STATE_2ND & state) {             /* 2nd character, apply esc-delay */
        if (-1 == x_escdelay ) {
            io_escimport();
        }
        event = WAIT_2ND;
        tmo = x_escdelay;

    } else {
        if (STATE_1ST & state) {
            event = WAIT_MINUTE;                /* wait timer */
        }

        if (STATE_IDLE & state) {
            int remaining;

            if ((remaining = iot_remaining(&x_update_timer)) > 0 ||
                    (remaining = iot_remaining(&x_idle_timer)) > 0) {
                if (remaining < tmo) {
                    event = WAIT_IDLE;
                    tmo = remaining;
                }
            }
        }

        if ((STATE_PTY & state) && tmo > EVT_SECOND(1)) {
            event = WAIT_PTY;                   /* pty, max timer 1 second */
            tmo = EVT_SECOND(1);
        }

        if (utmo > 0 && utmo < tmo) {
            event = WAIT_TIMEDOUT;
            tmo = utmo;
        }
    }

    /*
     *  Await for the next i/o event or timeout
     */
    if (x_scrfn.scr_event) {                    /* x11/win32 */
        if (0 == (ret = (*x_scrfn.scr_event)(evt, (accint_t)tmo))) {
            assert(EVT_NONE != evt->type);
            return 0;
        }
        return (ret == -2 ? WAIT_PTY : event);
    }

#if defined(HAVE_MOUSE) && \
        (!defined(HAVE_SELECT) || defined(__OS2__))
    if (0 == (STATE_RAW & state)) {
        if (mouse_active()) {
            if (tmo > EVT_MILLISECOND(250)) {
                tmo = EVT_MILLISECOND(250);     /* mouse, max timer 250ms */
            }
        }
    }
#endif  /*HAVE_MOUSE*/

#if defined(HAVE_SELECT) && \
        !defined(__OS2__) && !defined(__MSDOS__)

#if defined(__CYGWIN__)
    if (tmo > 0 && ((STATE_RAW|STATE_2ND) & state)) {
        do {
            if (sys_iocheck(evt)) {             /* poll issues, 1.7/windows 7 */
                assert(EVT_NONE != evt->type);
                return 0;
            }
            Sleep(250);
        } while ((tmo -= 250) > 0);
        return event;
    }
#endif  /*CYGWIN*/

    fds = x_fdset;                              /* TODO - poll()/select() */
    tv.tv_sec = tmo / 1000;
    tv.tv_usec = (tmo % 1000) * 1000;

    if (! x_scrfn.scr_select) {
        x_scrfn.scr_select = select;
    }

    if ((nfds = (*x_scrfn.scr_select)(x_fdmax + 1, &fds, NULL, NULL, &tv)) < 0) {
#if defined(SYSV) && !defined(SOLARIS2)
        if (EINTR != errno) {
             if (1 == sys_getchar(TTY_INFD, &ret, tmo)) {
                evt->type = EVT_KEYRAW;
                evt->code = ret;
                return 0;
            }
        }
#endif  /*SYSV*/
        return event;
    }

#if defined(HAVE_MOUSE)
    if (mouse_poll(&fds)) {                     /* mouse input */
        return 0;
    }
#endif  /*HAVE_MOUSE*/

    if (FD_ISSET(TTY_INFD, &fds)) {             /* primary input */
        if (x_scrfn.scr_read) {
            ret = (*x_scrfn.scr_read)();
        } else {
            unsigned char uch;

            ret = -1;
            if (1 == read(TTY_INFD, (void *)&uch, 1)) {
                ret = (int)uch;
            }
        }

        if (ret > 0) {
            evt->type = EVT_KEYRAW;
            evt->code = ret;
            return 0;
        }
    }

    if (nfds > 0) {
        /*
         *  Neither keyboard nor mouse input available, then assume pty related.
         *
         *  Note, if an event occurred on a pty and we are waiting for subsequent
         *  function key character, then ignore the timeout on the function key
         *  for now.
         */
        return WAIT_PTY;
    }

#elif defined(__OS2__)
    if (1 == main_thread_read_kbd(tmo)) {
        return 0;
    }
#if defined(HAVE_MOUSE)
    if (mouse_poll(NULL)) {
        return 0;
    }
#endif

#elif defined(_VMS)
    sys_timeout(STATE_2 & state);
    if (1 == sys_getchar(TTY_INFD, &ret, 0)) {
        evt->type = EVT_KEYDOWN;
        evt->code = ret;
        return 0;
    }

#else   /*MSDOS/WIN32 etc*/
    if (1 == sys_getchar(TTY_INFD, &ret, (accint_t)tmo)) {
        evt->type = EVT_KEYDOWN;
        evt->code = ret;
        return 0;
    }
#if defined(HAVE_MOUSE)
    if (mouse_poll(NULL)) {
        return 0;
    }
#endif
#endif

    return event;
}


/*  Function:           io_escimport
 *      Import the ESCDELAY environment variable otherwise default.
 *
 *  Description:
 *      The ESCDELAY environment variable sets the length of time to
 *      wait before timing out and treating ESC keystrokes as the
 *      Escape character rather then combining other characters in the
 *      input-stream to create ket sequence.
 *
 *      ESCDELAY value is generally measured in milliseconds, with the
 *      default being 750. Under AIX this value is measured in fifths
 *      of a milliseconds.
 *
 *      The most common instance where you may wish to change this
 *      value is to work with slow hosts, e.g., running on a network.
 *      If the host cannot read characters rapidly enough, it will
 *      have the same effect as if the terminal did not send
 *      characters rapidly enough.
 *
 *      Note that xterm mouse events are built up from character
 *      sequences received from the xterm. If your application makes
 *      heavy use of multiple-clicking, you may wish to lengthen this
 *      default value because the timeout applies to the composed
 *      multi-click event as well as the individual clicks.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static void
io_escimport(void)
{
    const char *cp;

    if (xf_escdelay > 0) {
        x_escsource = ESCDELAY_CMDLINE;
        x_escdelay = xf_escdelay;               /* user override */

    } else if ((cp = ggetenv("ESCDELAY")) != (const char *)NULL &&
                    (x_escdelay = atoi(cp)) > 0) {
#if defined(_AIX)
        if ((x_escdelay /= 5) == 0) {           /* convert to milliseconds */
            x_escdelay = 750;
        }
#endif
        x_escsource = ESCDELAY_ENV;

    } else {
        x_escsource = ESCDELAY_DEFAULT;
        x_escdelay = 750;                       /* default */
    }
}


/*  Function:           do_set_char_timeout
 *      set_char_timeout primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_char_timeout - Set the escape delay.

        void
        set_char_timeout([int timeout])

    Macro Description:
        The 'set_char_timeout()' primitive sets the length of time to
        wait before timing out and treating the ESC keystroke as the
        ESC character rather than combining it with other characters
        in the buffer to create a key sequence.

        The most common instance where you may wish to change this
        value is to work with slow hosts, e.g., running on a network.
        If the host cannot read characters rapidly enough, it will
        have the same effect as if the terminal did not send
        characters rapidly enough. The library will still see a
        timeout.

        The escape-delay value defaults to 750 milliseconds, but this
        shall be overridden by the ESCDELAY environment variable or
        the '--escdelay' command line option.

        The *ESCDELAY* environment is defined specifies the default
        timeout interval, which is generally measured in millisecond.
        If ESCDELAY is 0, the system immediately composes the ESCAPE
        response without waiting for more information from the
        buffer. The user may choose any value between 0 and 9999,
        inclusive.

        Note!:
        The 'ESCDELAY' value is generally measured in milliseconds
        under most unix environments, but under AIX this value is
        measured in fifths of a milliseconds (200 microseconds) to be
        compatible with 'libcursor' implementations; the default
        value is 500, or 0.1 second

    Macro Parameters:
        timeout - Optional character timeout is milliseconds,
            otherwise the current *ESCDELAY* shall be reapplied.

    Macro Returns:
        The 'set_char_timeout()' primitive returns the previous
        character timeout.

    Macro Portability:
        n/a

    Macro See Also:
        inq_char_timeout
 */
void
do_set_char_timeout(void)       /* int ([int timeout]) */
{
    int value;

    if (-1 == x_escdelay) {
        io_escimport();
    }

    acc_assign_int(x_escdelay);

    if (isa_integer(1) &&
            (value = get_xinteger(1, 0)) > 0) {
        x_escsource = ESCDELAY_SET;
        x_escdelay = value;
    }
}


/*  Function:           inq_char_timeout
 *      inq_char_timeout primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_char_timeout - Get the escape delay.

        int
        inq_char_timeout()

    Macro Description:
        The 'inq_char_timeout()' primitive retrieves the current value
        of the terminal escape delay.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_char_timeout()' primitive returns the current
        character timeout.

    Macro Portability:
        A Grief extension

    Macro See Also:
        inq_kbd_char, read_char
 */
void
inq_char_timeout(void)          /* int ([source]) */
{
    argv_assign_int(1, x_escsource);
    acc_assign_int(x_escdelay);
}


/*  Function:           do_keyboard_flush
 *      do_keyboard_flush primitive, flush any keyboard input.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: keyboard_flush - Flush the keyboard buffer.

        void
        keyboard_flush()

    Macro Description:
        The 'keyboard_flush()' primitive flushs the keyboard input
        buffer, consuming any pending input, permanently removing the
        keystrokes.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        inq_kbd_char, read_char
 */
void
do_keyboard_flush(void)         /* void () */
{
    struct IOEvent evt = {0};

    while (io_pending(&evt) > 0) {
        /*consume*/;
    }
}


/*  Function:           inq_kbd_char
 *      inq_kbd_char primitive, keyboard status.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_kbd_char - Peek at the keyboard.

        int
        inq_kbd_char()

    Macro Description:
        The 'inq_kbd_char()' primitive determines whether a character
        is available to be retrieved from the keyboard using
        'read_char'.

        Note!:
        This primitive only tests the internal look ahead buffer not
        the external terminal and/or keyboard stream.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_kbd_char()' primitive returns non-zero if one or
        more characters are available, otherwise 0 if the keyboard
        buffer is empty.

    Macro Portability:
        n/a

    Macro See Also:
        read_char
 */
void
inq_kbd_char(void)              /* int () */
{
    acc_assign_int((accint_t) io_typeahead());
}


/*  Function:           inq_kbd_flags
 *      inq_kbd_flags primitive, keyboard flags.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_kbd_flags - Get keyboard key flags.

        int
        inq_kbd_flags()

    Macro Description:
        The 'inq_kbd_flags()' primitive returns the state of the
        special keyboard keys.

(start table,format=nd)
        [ Bit       [Definition             ]
      ! 0x01        Right Shift
      ! 0x02        Left Shift
      ! 0x04        Ctrl
      ! 0x08        Alt
      ! 0x10        Scroll
      ! 0x20        Num Lock
      ! 0x40        Caps Lock
(end table)

    Macro Parameters:
        none

    Macro Returns:
        Always returns 0.

    Macro Portability:
        Provided only for BRIEF compatibility.

    Macro See Also:
        inq_kbd_char
 */
void
inq_kbd_flags(void)             /* int () */
{
    acc_assign_int(0);
}


/*  Function:           do_read_char
 *      read_char primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: read_char - Read next key from the keyboard.

        void
        read_char([int timeout = 0], [int mode = 0])

    Macro Description:
        The 'read_char()' primitive attempts to retrieve the next
        character from the keyboard.

        The 'timeout' argument specifies the handling of non key
        conditions, is other words when no keys are available. If a
        positive value the 'timeout' argument specifies the interval
        in milliseconds 'read_char()' should block waiting for a key
        to become ready. If the time limit expires before any key
        events read_char returns -1. If omitted or zero 'read_char'
        shall block unconditionally until a key is available. A
        negative (-1) timeout shall not block returning immediately
        effectively polling the keyboard.

        The 'mode' parameter controls the type of keyboard event to
        be returned. Supported modes are as follows.

(start table,format=nd)
         [Mode      [Description                    ]
       ! 0          Normal.
       ! >0         Raw mode.
       ! <0         Extended, returning additional
                    keys codes including mouse.
(end table)

     Macro Parameters:
        timeout - Optional timeout in milliseconds, if omitted or zero
            'read_char' blocks until a key-code is available,
            otherwise -1 shall simply poll for the next key.

        mode - Optional request mode, see above.

    Macro Returns:
        The 'read_char()' primitive returns the key-code, otherwise -1
        on a timeout.

    Macro Portability:
        Use of a '-'1 timeout value is required to emulate BRIEF.

    Macro See Also:
        int_to_key, inq_kbd_char, inq_kbd_flags
 */
void
do_read_char(void)              /* int ([int ms = 0], [int mode = 0]) */
{
    const accint_t tmo = get_xaccint(1, 0);     /* timeout, default=0 (block) */
    const accint_t mode = get_xaccint(2, 0);    /* >0=raw, <0=full, 0=normal */
    int ret;

    vtupdate();                                 /* echo-line update */

    if (mode > 0) {
        /*
         *  raw keystroke request
         */
        ret = io_get_raw(tmo);
        acc_assign_int(ret > 0 ? (accint_t) ret : (accint_t) -1);
        x_character = (int32_t) ret;

    } else if (mode < 0) {
        /*
         *  full, including mouse (extension)
         */
        ret = io_next(tmo);
        acc_assign_int(ret > 0 ? (accint_t) ret : (accint_t) -1);

    } else {
        /*
         *  normal
         */
        if (-1 != (ret = io_get_key(xf_wait ? tmo : -1))) {
            x_character = (int32_t) ret;        /* self_insert() support */
        }
        acc_assign_int((accint_t) ret);
    }
}


/*  Function:           io_cook
 *      Convert the key sequence into a keycode. This function will handle when a
 *      mouse button is hit from an Xterm when the mouse option is enabled in the
 *      window.
 *
 *      Upon an xterm style mouse, the trailing control characters shall be
 *      retrieved and cooked thru the mouse_process() interface. Note that the
 *      mouse action is then indirectly returned to the caller thru the typeahead
 *      buffer.
 *
 *  Parameters:
 *      buf -               Current i/o status.
 *      multikey -          Reply buffer.
 *      noambig -           Timeout in milliseconds.
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
 */
static int
io_cook(const KEY *buf, int noambig, struct IOEvent *evt, int *multikey)
{
    int key;

    if ((key = key_check(buf, multikey, noambig)) > 0) {
        if (MOUSE_KEY == key) {
            /*
             *  mouse events ...
             */
            struct IOEvent t_evt = {0};
            int button, ch[3] = {0};
            unsigned i;

            for (i = 0; i < 3; ++i) {
                if (io_wait(STATE_2ND, &t_evt, 0) ||
                        t_evt.type != EVT_KEYRAW) {
                    evt->type = EVT_TIMEOUT;
                    return (evt->code = -1);
                }
                ch[i] = t_evt.code;
            }

            /* wheel, convert to cursor and page up/down events. */
            button = ch[0];
            if (0x60 == (0x60 & button)) {
                switch (button & 0x1f) {
                case    0:  /* 00000 - '`'        */
                    key = KEY_UP;
                    break;

                case    1:  /* 00001 - 'a'        */
                    key = KEY_DOWN;
                    break;

                case  4|0:  /* 00100 - 'd', shift */
                case  8|0:  /* 01000 - 'h', meta  */
                case 16|0:  /* 10000 - 'p', ctrl  */
                    key = KEY_PAGEUP;
                    break;

                case  4|1:  /* 00101 - 'e', shift */
                case  8|1:  /* 01000 - 'i', meta  */
                case 16|1:  /* 10001 - 'q', ctrl  */
                    key = KEY_PAGEDOWN;
                    break;

                default:
                    evt->type = EVT_NONE;
                    return (evt->code = KEY_VOID);
                }

                evt->type = EVT_KEYDOWN;
                return (evt->code = key);
            }

            /* others, click and movement */
            mouse_process((ch[1] - ' ' - 1), (ch[2] - ' ' - 1),
                (' ' == button), ('!' == button), ('"' == button), -1);

            evt->type = EVT_NONE;
            return (evt->code = KEY_VOID);
        }

        evt->type = EVT_KEYDOWN;
        return (evt->code = key);
    }

    evt->type = EVT_NONE;
    return (evt->code = key);
}


/*  Function:           io_check
 *      Determine the next input event.
 *
 *      If tmo == 0, then wait forever for a key. If tmo == -1, then poll, i.e.
 *      dont wait for a key stroke. Otherwise tmo is time in milliseconds to wait
 *      for a keystroke.
 *
 *      We also do a lot of things inside here like check to see if we can update
 *      the clock on the status line, or poll the process buffers and update their
 *      windows. Also we handle any pushed-backed characters and other keystrokes
 *      buffered up.
 *
 *  Parameteres:
 *      evt -               Input event.
 *      mousemode -         Whether mouse events should be returned.
 *      tmo -               Timeout, in milliseconds.
 *
 *  Returns:
 *      -1 -                Timeout.
 *      >= 0 -              Key code.
 *
 */
static int
io_check(struct IOEvent *evt, int mousemode, accint_t tmo)
{
    IOTimer_t timer;
    KEY seq[32], seqidx;
    int state, event, multikey;
    int ch16 = 0, key, ret;

    iot_current();

    if (tmo <= -1) {                            /* poll, only process typeahead */
        if (!x_pushback && !io_typeahead()) {
            evt->type = EVT_TIMEOUT;
            return (evt->code = -1);
        }
    }
#define IO_RESET() {                            /* initial state WAIT1 and IDLE */ \
        state = (x_waitstate & ~STATE_KEYS) | (STATE_1ST | STATE_IDLE); \
        seq[seqidx = 0] = 0; \
        ch16 = 0; \
    }

    iot_starts(&x_idle_timer, xf_interval);     /* interval, in seconds */
    iot_start(&x_update_timer,
        (xf_interval <= 0 || xf_interval > UPDATE_DELAY) ? UPDATE_DELAY : xf_interval);
    iot_start(&timer, tmo);

    IO_RESET()

    for (;;) {

        /* Retrieve next character */
        while (1) {
            proc_check();                       /* sub-process checks */

            /* push-back queues */
            if (x_pushback) {                   /* local push-back */
                ch16 = (int) *x_pushback;
                if (0 == *++x_pushback) {
                    x_pushback = NULL;          /* empty */
                }
                assert(ch16 > 0);
                break;
            }

            if ((ch16 = io_pending(evt)) > 0) {
                if (EVT_KEYRAW != evt->type) {
                    if (EVT_MOUSE == evt->type && !mousemode) {
                        continue;               /* consume */
                    }
                    return ch16;
                }
                break;  //KEYRAW
            }

            /* timers */
            if (0 == (STATE_2ND & state)) {
                if (iot_expired(&x_idle_timer)) {
                    /*
                     *  Execute REG_IDLE and loop reparse push back buffers, etc.
                     */
                    if (0 == x_trigger_level) {
                        trigger_idle();
                        vtupdate_cursor();
                    }
                    iot_starts(&x_idle_timer, (accint_t)xf_interval);
                    continue;
                }

#if defined(HAVE_X_POLL_TIMER)
                if (x_ionow >= (x_poll_timer + 1)) {
                    x_poll_timer = x_ionow;     /* no select, poll the pty's once a second. */
                    pty_poll();
                    continue;
                }
#endif

                if (iot_expired(&x_update_timer)) {
                    if (! xf_interval) {
                        state &= ~STATE_IDLE;   /* disable idle timer */
                    }
                    vtupdate_idle();            /* update display */
                    iot_stop(&x_update_timer);  /* one-shot */
                }
            }

            /*
             *  Call REG_KEYBOARD registered macro; if an underlying macro was
             *  executed, reparse push back buffers, etc.
             */
            ret = -1;
            if (0 == x_trigger_level && (STATE_1ST & state) &&
                    (ret = trigger(REG_KEYBOARD)) > 0) {
                continue;
            }

            if (0 == ret && (ch16 = io_pending(evt)) > 0) {
                if (EVT_KEYRAW != evt->type) {
                    if (EVT_MOUSE == evt->type && !mousemode) {
                        continue;               /* consume */
                    }
                    return ch16;
                }
                break;  //KEYRAW
            }

            if (0 == (event = io_wait(state, evt, tmo))) {
                ch16 = evt->code;
                if (EVT_KEYRAW != evt->type) {
                    if (EVT_MOUSE == evt->type && !mousemode) {
                        continue;               /* consume */
                    }
                    return ch16;
                }
                break;  //KEYRAW
            }

            /* timers */
            iot_current();

            if (WAIT_TIMEDOUT & event) {
                evt->type = EVT_TIMEOUT;
                return (evt->code = -1);
            }

            if ((WAIT_PTY | WAIT_MINUTE | WAIT_IDLE) & event) {
                if (WAIT_PTY & event) {
                    pty_poll();                 /* connected pty's (if any) */
                }
                                                /* status line */
                if (x_ionow >= (x_minute_timer + 60)) {
                    x_minute_timer = x_ionow;
                    elinecol(0);
                    vtupdate_cursor();
                }

                if (iot_expired(&timer)) {      /* user timeout */
                    evt->type = EVT_TIMEOUT;
                    return (evt->code = -1);
                }

                continue;
            }

            if (WAIT_2ND & event) {
                /*
                 *  Awaiting additional character(s) and previous sequence was an
                 *  ambiguous sequence, eg ABC & ABCD; now check not ambiguous.
                 */
                if ((key = io_cook(seq, TRUE, evt, &multikey)) > 0) {
                    if (EVT_MOUSE == evt->type && !mousemode) {
                        if (tmo <= -1 || iot_expired(&timer)) {
                            evt->type = EVT_TIMEOUT;
                            return (evt->code = -1);
                        }
                        IO_RESET()
                        continue;

                    } else if (KEY_VOID != key) {
                        playback_store(key);
                    }
                    return key;
                }
                goto timeout;
            }

        } //while (ch16 <= 0);

        /*
         *  Append to sequence and cook ...
         *      determine whether the current sequence (seq) is a complete key.
         */
        assert(ch16 > 0);
        assert(ch16 <= KEY_VOID);
        assert(0 == (ch16 & ~0xff));            /* 8-bit only */

        iot_current();                          /* update timer */

        seq[seqidx] = (KEY) ch16;
        seq[++seqidx] = 0;

        if ((key = io_cook(seq, STATE_2ND & state, evt, &multikey)) <= 0) {
            if (0 == key) {
                break;                          /* no matches */
            }

        } else if (KEY_VOID == key ||
                (EVT_MOUSE == evt->type && !mousemode)) {
            if (tmo <= -1 || iot_expired(&timer)) {
                evt->type = EVT_TIMEOUT;
                return (evt->code = -1);
            }
            IO_RESET()
            continue;

        } else {
            playback_store(key);
            return key;
        }

        if (! multikey) {                       /* !multikey, secondary characters */
            state &= ~STATE_KEYS;
            state |= STATE_2ND;
        }
    }

timeout:;
    ch16 = seq[0];
    assert(seqidx > 0);
    assert(ch16);
    playback_store(ch16);

    if (seq[seqidx]) {
        KEY tkey;

        x_pushback = x_pushbuffer;
        for (ret = 1; 0 != (tkey = seq[ret]); ++ret) {
            *x_pushback++ = tkey;
        }
        *x_pushback = 0;
    } else {
        x_pushback = NULL;
    }

    evt->type = EVT_KEYDOWN;
    return (evt->code = ch16);
}


/*  Function:           io_next
 *      Retrieve the next input event. If the case of mouse events, the global
 *      mouse execution profile shall be updated, see mouse_execute().
 *
 *  Notes:
 *      Whilst awaiting the next event, timer, process-buffers, display-timers
 *      and the echo-line are serviced.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Key code, otherwise -1 on a timeout.
 */
int
io_next(accint_t tmo)
{
    struct IOEvent evt = {0};
    int key;

    if ((key = io_check(&evt, TRUE, tmo)) > 0) {
        x_time_last_key = x_ionow;
        if (EVT_MOUSE == evt.type) {
            mouse_execute(&evt);
        }
    }
    ED_TRACE(("io_next() = [%d/0x%x]\n", key, key))
    return key;
}


/*  Function:           io_get_event
 *      Retrieve the next key or mouse input event.
 *
 *  Notes:
 *      Whilst awaiting the next event, timer, process-buffers,
 *      display-timers and the echo-line are serviced.
 *
 *  Parameters:
 *      evt -               Input event.
 *      tmo -               Timeout, in milliseconds.
 *
 *  Returns:
 *      EVT_TIMEOUT, EVT_MOUSE or EVT_KEYDOWN.
 */
int
io_get_event(struct IOEvent *evt, accint_t tmo)
{
    if (io_check(evt, TRUE, tmo) <= -1) {
        evt->code = -1;
        return (evt->type = EVT_TIMEOUT);
    }
    x_time_last_key = x_ionow;
    return evt->type;
}


/*  Function:           io_get_key
 *      Retrieve the next key event, using the specified timeout.
 *
 *      If tmo == 0, then wait forever for a key.
 *
 *      If tmo == -1, then poll, i.e. dont wait for a key stroke.
 *
 *      Otherwise tmo is time in milliseconds to wait for a keystroke.
 *
 *  Notes:
 *      Whilst awaiting the next event, timer, process-buffers,
 *      update-timers and the echo-line are serviced.
 *
 *  Parameters:
 *      tmo -               Timeout, in milliseconds.
 *
 *  Returns:
 *      Key code, otherwise -1 on a timeout.
 */
int
io_get_key(accint_t tmo)
{
    struct IOEvent evt = {0};
    int key = io_check(&evt, FALSE, tmo);

    ED_TRACE(("io_getkey(tmo:%d) = [%d/0x%x]\n", (int)tmo, key, key))
    if (key > 0) {
	x_time_last_key = x_ionow;
    }
    return key;
}


/*  Function:           io_get_raw
 *      Retrieve the next raw keystroke from the keyboard.
 *
 *  Parameters:
 *      tmo -               Timeout, in milliseconds.
 *
 *  Returns:
 *      Key-code, otherwise 0.
 */
int
io_get_raw(accint_t tmo)
{
    struct IOEvent evt = {0};
    int ch16;

    if ((ch16 = io_pending(&evt)) >= 0) {       /* pending */
        return ch16;
    }

    do {                                        /* input streams */
        if (0 == io_wait(STATE_RAW, &evt, tmo)) {
            return evt.code;
        }

        if ((ch16 = io_pending(&evt)) >= 0) {   /* re-check pending */
            return ch16;
        }

    } while (0 == tmo);

    return 0;           /*timeout*/
}


/*  Function:           io_typeahead
 *      Determine returns whether there are additional io-events pending
 *      available from the various input streams.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      *true* or *false*.
 */
int
io_typeahead(void)
{
    if (key_cache_test(x_push_ref)) {
        return TRUE;
    }

    if (playback_grab(FALSE)) {
        return TRUE;
    }

    if (x_kbdq) {
        return TRUE;
    }

    if (! x_scrfn.scr_event) {
        struct IOEvent evt = {0};

        if (sys_iocheck(&evt) && evt.code > 0) {
            assert(evt.type == EVT_KEYDOWN || evt.type == EVT_KEYRAW);
            x_kbdq = evt.code;
            return TRUE;
        }
    }

    return FALSE;
}


/*  Function:           io_pending
 *      Retrieve the next pending input event, if any, first from the push-back
 *      streams and secondary from the underlying keyboard stream.
 *
 *  Parameters:
 *      evt -               Input event.
 *
 *  Returns:
 *      Character code otherwise -1.
 */
int
io_pending(struct IOEvent *evt)
{
    int ch;

    if (x_push_ref) {
        if ((ch = key_cache_pop(x_push_ref, evt)) > 0) {
            return ch;
        }
    }

    if ((ch = playback_grab(TRUE)) > 0) {
        evt->code = ch;
        evt->type = EVT_KEYDOWN;
        return ch;
    }

    if ((ch = x_kbdq) > 0) {
        evt->code = ch;
        evt->type = (~0xff & ch ? EVT_KEYDOWN : EVT_KEYRAW);
        x_kbdq = 0;
        return ch;
    }

    return -1;
}


/*  Function:           io_pty_state
 *      Controls whether pty are polled (at least) every 1 second.
 *
 *  Parameters:
 *      state -             *true* or *false*.
 *
 *  Returns:
 *      nothing.
 */
void
io_pty_state(int state)
{
    if (state) {
        x_waitstate |=  STATE_PTY;
    } else {
        x_waitstate &= ~STATE_PTY;
    }
}


/*  Function:           io_escdelay
 *      Retrieve the ESC delay.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      ESC delay, in milliseconds.
 */
int
io_escdelay(void)
{
    return (x_escdelay >= 0 ? x_escdelay : 750);
}


/*  Function:           io_time
 *      Retrieve the current input time.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      Current time.
 */
time_t
io_time(void)
{
    return x_ionow;
}


/*  Function:           io_reset_timers
 *      Reset the timers avoiding an a related timer event (e.g autosave or screen blank)
 *      occurring immediately upon a return from shell mode.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing.
 */
void
io_reset_timers(void)
{
    iot_starts(&x_idle_timer, xf_interval);
}


/*  Function:           io_device_add
 *      Add a file-descriptor as a source of input Encapsulate the dirty work because
 *      we may need to tell the operating system in a funny way (e.g. X-Windows).
 *
 *  Parameters:
 *      fd -                File descriptor.
 *
 *  Returns:
 *      nothing.
 */
void
io_device_add(int fd)
{
    __CUNUSED(fd)
    assert(fd >= 0);
#if defined(HAVE_SELECT) && \
        !defined(__OS2__) && !defined(__MSDOS__)
    if (fd > x_fdmax) x_fdmax = fd;
    FD_SET(fd, &x_fdset);
#endif  /*HAVE_SELECT*/
#if defined(HAVE_POLL)
    x_pollfds[x_pollcnt].fd = fd;
    x_pollfds[x_pollcnt].events = POLLIN;
    ++x_pollcnt;
#endif  /*HAVE_POLL*/
}


/* Function:            io_device_remove
 *      Remove a file-descriptor from being a source of input. Encapsulate the
 *      dirty work because we may need to tell the operating system in a funny
 *      way (e.g. X-Windows).
 *
 *  Parameters:
 *      fd -                File descriptor.
 *
 *  Returns:
 *      nothing.
 */
void
io_device_remove(int fd)
{
    __CUNUSED(fd)
    assert(fd >= 0);
#if defined(HAVE_SELECT) && \
        !defined(__OS2__) && !defined(__MSDOS__)
    FD_CLR(fd, &x_fdset);
#endif  /*HAVE_SELECT*/
#if defined(HAVE_POLL)
    {
        int i, cnt = x_pollcnt;

        for (i = 0; i < cnt; ++i) {
            if (fd == x_pollfds[i].fd) {        /* match */
                cnt = --x_pollcnt;
                while (i < cnt) {               /* compress */
                    x_pollfds[i] = x_pollfds[i + 1];
                }
                break;
            }
        }
    }
#endif  /*HAVE_POLL*/
}


#if defined(HAVE_SELECT) && \
        !defined(__OS2__) && !defined(__MSDOS__)
fd_set *
io_device_fds(int *fdmax)
{
    *fdmax = x_fdmax;
    return &x_fdset;
}
#endif  /*HAVE_SELECT*/


#if defined(HAVE_POLL)
struct pollfd *
io_device_pollfds(int *count)
{
    if (x_pollcnt > 0) {
        int i, cnt = x_pollcnt;
        for (i = 0; i < cnt; ++i) {
            x_pollfds[i].revents = 0;
        }
        *count = x_pollcnt;
        return x_pollfds;
    }
    return NULL;
}
#endif  /*HAVE_POLL*/

/*end*/
