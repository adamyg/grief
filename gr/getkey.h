#ifndef GR_GETKEY_H_INCLUDED
#define GR_GETKEY_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_getkey_h,"$Id: getkey.h,v 1.20 2025/06/30 10:18:18 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: getkey.h,v 1.20 2025/06/30 10:18:18 cvsuser Exp $
 * Keyboard event interface.
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

#include <edsym.h>
#if defined(HAVE_POLL) && defined(HAVE_POLL_H)
#include <poll.h>
#endif
#include <time.h>

__CBEGIN_DECLS

struct IOMouse {
    int x, y;                                   /* coordinates */
    int win;                                    /* associated window */
    int where;                                  /* window location */
    accint_t when;                              /* timestamp, in milliseconds */
};

typedef unsigned char KEYCHAR;

struct IOSequence {
    unsigned len;                               /* sequence length, in bytes */
#define IOSEQUENCE_LENGTH   64
    KEYCHAR data[IOSEQUENCE_LENGTH];            /* raw key sequence data (63 + NUL) */
};

struct IOEvent {
    int type;                                   /* event type */
#define EVT_TIMEOUT         -1                  /* read timeout */
#define EVT_NONE            0                   /* no reportable event */
#define EVT_KEYDOWN         1                   /* key-down event */
#define EVT_MOUSE           2                   /* mouse event */
#define EVT_KEYRAW          99                  /* raw key-code */
    int code;                                   /* key-code, see edalt.h and KEY */
    unsigned modifiers;                         /* raw modifiers */
    struct IOMouse mouse;                       /* associated mouse details; type=EVT_MOUSE */
    struct IOSequence sequence;                 /* underlying key data */
};

#define EVT_SECOND(_s)  ((_s) * 1000L)
#define EVT_MILLISECOND(_m) (_m)

extern int                  io_next(struct IOEvent *evt, accint_t tmo);
extern int                  io_get_event(struct IOEvent *evt, accint_t tmo);
extern int                  io_get_key(accint_t tmo);
extern int                  io_get_raw(accint_t tmo);
extern int                  io_typeahead(void);
extern int                  io_pending(struct IOEvent *evt);
extern int                  io_escdelay(void);
extern time_t               io_time(void);
extern void                 io_pty_state(int state);
extern void                 io_reset_timers(void);
extern void                 io_device_add(int fd);
extern void                 io_device_remove(int fd);
#if defined(HAVE_SELECT)
extern fd_set *             io_device_fds(int *fdmax);
#endif
#if defined(HAVE_POLL)
extern struct pollfd *      io_device_pollfds(int *count);
#endif

extern void                 do_set_char_timeout(void);
extern void                 do_keyboard_flush(void);
extern void                 do_read_char(void);
extern void                 inq_char_timeout(void);
extern void                 inq_kbd_char(void);
extern void                 inq_kbd_flags(void);

__CEND_DECLS

#endif /*GR_GETKEY_H_INCLUDED*/
