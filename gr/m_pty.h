#ifndef GR_M_PTY_H_INCLUDED
#define GR_M_PTY_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_m_pty_h,"$Id: m_pty.h,v 1.8 2014/10/22 02:33:06 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_pty.h,v 1.8 2014/10/22 02:33:06 ayoung Exp $
 * Process control primitives.
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

#define PTY_FG_MASK         0x0f00
#define PTY_BG_MASK         0xf000

#define PTY_FG_SHIFT        8
#define PTY_BG_SHIFT        12

#define PTY_FG_SET(_x)      ((_x) << PTY_FG_SHIFT)
#define PTY_BG_SET(_x)      ((_x) << PTY_BG_SHIFT)

#define PTY_FG_GET(_x)      (((_x) & PTY_FG_MASK) >> PTY_FG_SHIFT)
#define PTY_BG_GET(_x)      (((_x) & PTY_BG_MASK) >> PTY_BG_SHIFT)

extern void                 pty_cleanup(BUFFER_t *bp);
extern void                 pty_free(DISPLAY_t *dp);
extern void                 pty_poll(void);

extern int                  pty_connect(DISPLAY_t *dp, const char *shell, const char *cwd);
extern DISPLAY_t *          pty_argument(int argi);
extern int                  pty_send_signal(int pid, int value);
extern void                 pty_send_term(int pid);
extern int                  pty_read(BUFFER_t *bp, char *buf, int count);
extern int                  pty_died(BUFFER_t *bp);

extern int                  pty_inserts(const char *buf, int len);
extern int                  pty_insertc(int ch);
extern void                 pty_write(const char *buf, int len);

extern void                 do_connect(void);
extern void                 do_disconnect(void);
extern void                 do_send_signal(void);
extern void                 do_wait(void);
extern void                 do_wait_for(void);
extern void                 inq_connection(void);

extern void                 set_process_position(void);
extern void                 inq_process_position(void);

__CEND_DECLS

#endif /*GR_M_PTY_H_INCLUDED*/
