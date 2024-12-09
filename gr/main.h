#ifndef GR_MAIN_H_INCLUDED
#define GR_MAIN_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_main_h,"$Id: main.h,v 1.37 2024/12/09 14:13:08 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: main.h,v 1.37 2024/12/09 14:13:08 cvsuser Exp $
 * Globals and main process primitives.
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

#include <time.h>
#include <edsym.h>

__CBEGIN_DECLS

/*
 *  Global variables declared in main.c
 */

extern const char *         x_progname;         /* arg0 or better */

extern const char *         x_resource;         /* active resource-file, in any. */

extern const char *         xf_mouse;           /* mouse mode; NULL disabled. */

extern int                  xf_compat;          /* TRUE normal stdout I/O otherwise optimised */

extern int                  xf_usevmin;         /* TRUE if we can use VMIN */

extern int                  xf_wait;            /* FALSE if read_char should return -1 on no key pressed */

extern int                  xf_readonly;        /* TRUE if -R/--readonly specified. */

extern int                  xf_ega43;           /* TRUE if in 43 line mode. */

extern int                  xf_backups;         /* TRUE/FALSE, file backups. */

extern int                  xf_autosave;        /* TRUE/FALSE, autosave. */

extern int                  xf_spell;           /* TRUE/FALSE/-1 enable spell. */

extern int                  xf_scrollregions;   /* TRUE/FALSE, enable scroll regions. */

#define COLORMODE_AUTO          -1
#define COLORMODE_NONE          0
#define COLORMODE_8             8
#define COLORMODE_16            16
#define COLORMODE_88            88
#define COLORMODE_256           256
#define COLORMODE_TRUECOLOR     1000
#define COLORMODE_DIRECT        1001

extern int                  xf_color;           /* color-mode override, default=COLORMODE_AUTO. */

extern int                  xf_graph;           /* TRUE/FALSE, user specified graphic mode. */

extern int                  xf_visbell;         /* TRUE/FALSE, visual bell. */

extern int                  xf_kbprotocol;      /* Keyboard protocol mode. */

extern const char *         xf_kbconfig;        /* Optional keyboard configuration. */

#define UNDERSTYLE_LINE         0x0001          /* underline */
#define UNDERSTYLE_EXTENDED     0x0002          /* undercurl and others */
#define UNDERSTYLE_BLINK        0x0004          /* b/w blink */

extern int                  xf_understyle;      /* Active understyle's. */

extern int                  xf_title;           /* TRUE/FALSE, user specified console title mode. */

extern int                  xf_noinit;          /* TRUE, disable tty init/reinit */

extern int                  xf_nokeypad;        /* TRUE, disable tty keypad init/reinit */

extern int                  xf_profile;         /* TRUE if profiling on */

extern int                  xf_lazyvt;          /* !zero if limits vt updates. */

extern int                  xf_syntax_flags;    /* 0x1=Syntax hiliting,0x02=Bracket matching. */

extern int                  xf_strictlock;      /* TRUE/FALSE, if strict file-locking. */

extern int                  xf_warnings;        /* TRUE/FALSE, enable warnings. */

extern int                  xf_restrict;        /* TRUE/FALSE, restrict search paths. */

extern int                  xf_utf8;            /* UTF8 options. */

extern int                  xf_escdelay;        /* ESC delay. */

extern const char *         x_encoding_guess;   /* Text encoding guess configuration. */

extern int                  x_bftype_default;   /* Default buffer-type */

extern const char *         x_encoding_default; /* Default file encoding. */

extern int                  x_display_enabled;  /* TRUE when we do a set_term_characters. */

extern time_t               x_startup_time;     /* Timestamp of application startup. */

extern int                  xf_sigtrap;         /* TRUE/FALSE, control signal traps. */

extern int                  xf_dumpcore;        /* 1=dumpcore, 2=stack-dump (if available). */

extern int                  x_mflag;            /* TRUE whilst processing -m strings to avoid messages being printed */

extern int                  x_applevel;         /* Value of GRLEVEL. */

extern mode_t               x_umask;            /* Current umask value for creating files. */

extern int                  x_plevel;           /* Process level */

extern int                  x_panycb;           /* any change buffer action */

extern int                  x_msglevel;         /* Message level */

extern int                  x_ctrlc;            /* TRUE when SIGINT occured */

extern uint32_t             xf_test;            /* TRUE enables test code --- internal use only --- */

#define XF_FLAG(__flag)         (1 << (__flag - 1))
#define XF_TEST(__test)         (xf_test & XF_FLAG(__test))

extern BUFFER_t *           curbp;              /* Current buffer */
extern WINDOW_t *           curwp;              /* Current window */

extern void                 set_curbp(BUFFER_t *bp);
extern void                 set_curwp(WINDOW_t *wp);
extern void                 set_curwpbp(WINDOW_t *wp, BUFFER_t *bp);

extern void                 panic(const char *msg, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern void                 gr_exit(int);
extern void                 gr_shutdown(int);

extern void                 main_loop(void);
extern void                 check_exit(void);

extern void                 do_abort(void);
extern void                 do_exit(void);
extern void                 do_process(void);
extern void                 do_suspend(void);

__CEND_DECLS

#endif /*GR_MAIN_H_INCLUDED*/

