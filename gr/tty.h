#ifndef GR_TTY_H_INCLUDED
#define GR_TTY_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_tty_h,"$Id: tty.h,v 1.36 2014/10/22 02:33:22 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: tty.h,v 1.36 2014/10/22 02:33:22 ayoung Exp $
 * TTY interface.
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
#include <edfeatures.h>
#include "getkey.h"                             /* IOEvent */

__CBEGIN_DECLS

/*
 *  Display driver interface
 */
struct timeval;

struct _VCELL;
struct IOEvent;

typedef struct _scrprof {
    int     sp_lastsafe;
    int     sp_rows;
    int     sp_cols;
    int     sp_colors;
} scrprofile_t;

typedef struct _scrfn {
    /*
     *  initialise and configuration.
     */
                                                /* initialisation */
    void    (*scr_init)(int *pargc, char **argv);

    void    (*scr_open)(scrprofile_t *profile); /* open terminal */

                                                /* ready the terminal, run-time (re)initialisation */
    void    (*scr_ready)(int repaint, scrprofile_t *profile);

    void    (*scr_display)(void);               /* enable display */

                                                /* feature change event */
    void    (*scr_feature)(int ident, scrprofile_t *profile);

#define SCR_CTRL_NORMAL     10                  /* normal color */
#define SCR_CTRL_GARBLED    20
#define SCR_CTRL_SAVE       30
#define SCR_CTRL_RESTORE    40
#define SCR_CTRL_COLORS     50

                                                /* general process control */
    int     (*scr_control)(int action, int param, ...);

    void    (*scr_close)(void);                 /* close terminal */

    /*
     *  cursor control.
     */
    int     (*scr_cursor)(int, int, int);       /* cursor visible/insert/overwrite control */

    void    (*scr_move)(int, int);              /* cursor position */

    void    (*scr_winch)(int *, int *);         /* winch event */

    /*
     *  i/o control.
     */
    void    (*scr_clear)(void);                 /* clear screen image */

                                                /* video cell print */
    void    (*scr_print)(int, int, int, const struct _VCELL *);

    void    (*scr_putc)(vbyte_t);               /* character write */

    void    (*scr_flush)(void);                 /* flush output to window */

                                                /* feep or flash window */
    void    (*scr_beep)(int freq, int duration);

                                                /* set window title and icon name */
    int     (*scr_names)(const char *tile, const char *icon);

                                                /* window font */
    int     (*scr_font)(int setlen, char *fontname);

    /*
     *  output hooks.
     */
                                                /* insert one or more lines */
    int     (*scr_insl)(int, int, int, vbyte_t);

                                                /* delete one or more lines */
    int     (*scr_dell)(int, int, int, vbyte_t);

    int     (*scr_eeol)(void);                  /* erase to current end-of-line */

    void    (*scr_repeat)(int, vbyte_t, int);   /* repeat specified character */

    void    (*scr_push)(const char *buf);       /* push characters into the output stream */

    /*
     *  input hooks
     */ 
                                                /* pending event hook */
    int     (*scr_event)(struct IOEvent *, int tmo);

    int     (*scr_read)(void);                  /* replace read() */

#if !defined(__MSDOS__)                         /* replace select() */
#if !defined(__MINGW32__)
    int     (*scr_select)(int, fd_set *, fd_set *, fd_set *, struct timeval *);
#endif
#endif
} scrfn_t;

    /* High level terminal control */

extern void                 ttcurses(void);
extern void                 ttx11(void);

extern void                 ttinit(void);
extern void                 ttopen(void);
extern void                 ttsizeinit(void);
extern void                 ttready(int repaint);
extern void                 ttkeys(void);
extern void                 ttdisplay(void);
extern void                 ttclose(void);
extern void                 ttfeature(int ident);

extern void                 ttmove(int row, int col);
extern int                  ttcursor(int visible, int imode, int vspace);

extern int                  ttcolordepth(void);
extern void                 ttcolornormal(void);
extern const char *         ttdefaultscheme(void);

extern void                 ttwinch(void);
extern int                  ttresize(void);
extern int                  ttwinched(int rows, int cols);

extern void                 tttitle(const char *title);
extern void                 ttbeep(void);
extern void                 ttclear(void);
extern void                 ttflush(void);

    /* Cursor status */

extern int                  ttrows(void);
extern int                  ttcols(void);
extern int                  ttatrow(void);
extern int                  ttatcol(void);
extern void                 ttposget(int *row, int *col);
extern void                 ttposset(int row, int col);
extern void                 ttposinvalid(void);

    /* Low level terminal control */

extern int                  ttinsl(int row, int bot, int nlines, vbyte_t fillcolor);
extern int                  ttdell(int row, int bot, int nlines, vbyte_t fillcolor);
extern int                  tteeol(void);

#define WHERE_END       1
#define WHERE_START     2
#define WHERE_DONTCARE  3

extern int                  ttrepeat(int cnt, vbyte_t fill, int where);
extern int                  ttlastsafe(void);
extern void                 ttpush(const char *cp);

    /* Misc */

extern char *               ttstringcopy(char *dp, int len, const char *bp, int delim);

    /* Configuration */

extern int                  ttbandw(int attr, int underline, int italic, int blink);
extern void                 ttdefaults(void);
extern void                 ttboxcharacters(int force);
extern const char *         ttspecchar(vbyte_t ch);

extern int                  ttisetstr(const char *tag, int teglen, const char *value);
extern int                  ttisetnum(const char *tag, int taglen, const char *value);
extern int                  ttisetflag(const char *tag, int taglen, const char *value);

    /* Globals */

extern int                  tty_open;
extern int                  tty_needresize;
extern int                  tty_egaflag;
extern int                  tty_tceeol;
extern int                  tty_tcinsl;
extern int                  tty_tcdell;

extern features_t           x_pt;               /* terminal configuration/features */
extern scrfn_t              x_scrfn;            /* terminal descriptor */

    /* Commands */

extern void                 do_copy_screen(void);
extern void                 do_ega(void);
extern void                 do_view_screen(void);

__CEND_DECLS

#endif /*GR_TTY_H_INCLUDED*/
