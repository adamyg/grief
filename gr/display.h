#ifndef GR_DISPLAY_H_INCLUDED
#define GR_DISPLAY_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_display_h,"$Id: display.h,v 1.28 2024/07/29 16:14:45 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: display.h,v 1.28 2024/07/29 16:14:45 cvsuser Exp $
 * Display management.
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

__CBEGIN_DECLS

typedef uint32_t VFLAG_t;                       /* line flags */

/*
 *  Display type
 */
enum {
    DISPTYPE_UNKNOWN = -1,
    DISPTYPE_7BIT,                              /* 0, 7-bit */
    DISPTYPE_8BIT,                              /* 1, 8-bit */ 
    DISPTYPE_UTF8,                              /* 2, utf-8 encoding */
    DISPTYPE_DBCS,                              /* 3, double byte (variable encoding) */
    DISPTYPE_MBCS,                              /* 4, multiple byte (fixed encoding) */
    DISPTYPE_UNICODE                            /* 5, generic unicode support */
};


/*
 *  UTF8 control flags
 */
enum {
    DISPUTF8_COMBINED   = 0x0001,
    DISPUTF8_SEPERATE   = 0x0010,

    DISPUTF8_SUBST_MASK = 0x0f00,               /* Subst */
    DISPUTF8_NCR        = 0x0100,               /* Numerical Character Reference */
    DISPUTF8_UCN        = 0x0200,               /* Universal Character Name */
    DISPUTF8_HEX        = 0x0400,               /* HEX */
    DISPUTF8_C99        = 0x0800,               /* C99 */
};


/*
 *  Video driver virtual character cell.
 */
typedef struct _VCELL {
    vbyte_t         primary;                    /* primary character */ 
#define VCOMBINED_MAX       4                   /* combined character (following ncurses limit) */
    vbyte_t *       combined;                   /* combined character(s), optional */
} VCELL_t;


extern void                 vtinit(int *argcp, char **argv);
extern int                  vtinited(void);
extern void                 vtready(void);
extern void                 vtclose(int clear);

extern int                  vtis8bit(void);
extern int                  vtisutf8(void);
extern int                  vtisunicode(void);
extern int                  vtcharwidth(int ch, int wcwidth);

extern int                  vtiscolor(void);
extern void                 vtcolorscheme(int now, const char *name);
extern const char *         vtdefaultscheme(void);
extern int                  vtcolordepth(void);
extern vbyte_t              vtmapansi(vbyte_t ansi);

extern void                 vttitle(const char *title);
extern void                 vtgarbled(void);
extern int                  vtisgarbled(void);
extern int                  vtupdating(void);
extern void                 vtwinch(int oncol, int onrow);
extern void                 vtmouseicon(int newy, int newx, int oldy, int oldx);
extern int                  vtpute(vbyte_t ch, int col);
extern void                 vtupdate(void);
extern void                 vtupdate2(int force);
extern void                 vtupdate_idle(void);
extern void                 vtupdate_bottom(int col);
extern void                 vtupdate_cursor(void);
extern void                 vtwritehl(const LINECHAR *text, int line, int col, int tabs);

extern int                  xf_disptype;
extern int                  xf_termcap;

__CEND_DECLS

#endif /*GR_DISPLAY_H_INCLUDED*/
