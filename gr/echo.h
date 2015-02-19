#ifndef GR_ECHO_H_INCLUDED
#define GR_ECHO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_echo_h,"$Id: echo.h,v 1.24 2014/10/22 02:32:57 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: echo.h,v 1.24 2014/10/22 02:32:57 ayoung Exp $
 * Command/echo line implementation/interface.
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

#include <edtypes.h>

__CBEGIN_DECLS

/*--export--defines--*/
/*
 *  echo line flags.
 */
#define E_LINE                  0x0001          /* Line: ..                     */
#define E_COL                   0x0002          /* Col: ..                      */
#define E_PERCENT               0x0004          /* nn%                          */
#define E_TIME                  0x0008          /* hh:mm a/pm                   */
#define E_REMEMBER              0x0010          /* RE / PA String.              */
#define E_CURSOR                0x0020          /* IN / OV cursor type.         */
#define E_FROZEN                0x0040          /* Echo line is frozen, ie not updated. */
#define E_VIRTUAL               0x0080          /* Virtual character indicator  */
#define E_CHARVALUE             0x0100          /* Character value              */
#define E_TIME24                0x1000          /* Time in 24hour form (HH:MM)  */
#define E_FORMAT                0x8000          /* Format override active       */
/*--end--*/

/* elinecol() flags */
#define LC_FORCE                0x0001
#define LC_DONTENABLE           0x0002

/* echo line buffer size */
#define EBUFSIZ                 1024            /* must be >= MAX_PATH and MAX_CMDLINE */

extern int                  eyorn(const char *msg);
extern int                  eyesno(const char *msg);
extern void                 ecursor(int imode);
extern int                  ereply(const char *prompt, char *buf, int nbuf);
extern int                  ereply1(const char *prompt, char *buf, int nbuf);
extern int                  equestion(const char *prompt, char *buf, int nbuf);
extern int                  egetparm(const char *prompt, const char *defstr, char *buf, int nbuf, int one);

extern void                 ewprintf(const char *str, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern void                 ewprintx(const char *str, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern void                 eeprintf(const char *str, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern void                 eeprintx(const char *str, ...) __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern void                 eeputs(const char *msg);
extern void                 ewputs(const char *msg);
extern void                 eclear(void);
extern void                 elinecol(int flags);

extern void                 eredraw(void);

extern void                 infof_truncated(const char *fmt, const char *fname);
extern void                 infof(const char *str, ...)  __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern void                 infos(const char *str);
extern void                 errorf(const char *str, ...)  __ATTRIBUTE_FORMAT__((printf, 1, 2));
extern void                 errorfx(const char *str, ...)  __ATTRIBUTE_FORMAT__((printf, 1, 2));

extern void                 set_echo_format(const char *fmt);

extern int                  xf_echoflags;
extern int                  x_prompting;
extern int                  x_pause_on_error;
extern int                  x_pause_on_message;

__CEND_DECLS

#endif /*GR_ECHO_H_INCLUDED*/
