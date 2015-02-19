#ifndef GR_PLAYBACK_H_INCLUDED
#define GR_PLAYBACK_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_playback_h,"$Id: playback.h,v 1.17 2014/10/22 02:33:14 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: playback.h,v 1.17 2014/10/22 02:33:14 ayoung Exp $
 * Macro playback.
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

enum playbackstate {     
    PLAYBACK_NONE,
    PLAYBACK_RECORDING,
    PLAYBACK_PLAYING
};

extern void                 playback_init(void);
extern enum playbackstate   playback_status(void);
extern void                 playback_shutdown(void);

extern void                 playback_store(int ch);
extern void                 playback_macro(const char *cp);
extern int                  playback_grab(int get_it);

extern void                 do_load_keystroke_macro(void);
extern void                 do_pause(void);
extern void                 do_playback(void);
extern void                 do_remember(void);
extern void                 do_save_keystroke_macro(void);
extern void                 inq_keystroke_macro(void);
extern void                 inq_keystroke_status(void);
extern void                 inq_remember_buffer(void);

extern const char *         x_rem_string;

__CEND_DECLS

#endif /*GR_PLAYBACK_H_INCLUDED*/
