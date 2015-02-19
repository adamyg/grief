#ifndef TTY_TTY_XTERM_H_INCLUDDED
#define TTY_TTY_XTERM_H_INCLUDDED
/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: tty_xterm.h,v 1.4 2014/10/22 02:34:41 ayoung Exp $
 * xterm tty features ...
 *
 *
 */

#if defined(__PROTOTYPES__)
extern int                  xterm_util(void);
extern void                 xterm_graphic(void);
extern void                 xterm_color(void);
extern void                 xterm_mono(void);
extern void                 xterm_pccolors(void);
extern void                 xterm_colour88(void);
extern void                 xterm_color88(void);
extern void                 xterm_88colour(void);
extern void                 xterm_colour256(void);
extern void                 xterm_color256(void);
extern void                 xterm_256color(void);
extern void                 xterm_88color(void);
extern void                 xterm_standard(void);
extern void                 xterm_arrow(void);
extern void                 xterm_mouse(void);
extern void                 xterm_altmeta_keys(void);

extern void                 openwin(void);
extern void                 sunview(void);
extern void                 vga(void);
#endif

#endif /*TTY_TTY_XTERM_H_INCLUDED*/

