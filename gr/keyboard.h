#ifndef GR_KEYBOARD_H_INCLUDED
#define GR_KEYBOARD_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_keyboard_h,"$Id: keyboard.h,v 1.26 2024/08/27 12:44:33 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: keyboard.h,v 1.26 2024/08/27 12:44:33 cvsuser Exp $
 * Key maps and binding management.
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

struct IOEvent;

#define MAX_KEYBUF      64                      /* key description buffer length. */

typedef struct keyseq {
    KEY ks_code;                                /* internal keycode */
    char ks_buf[1];                             /* array of characters null terminated */
} keyseq_t;

typedef struct mouseevt {
    const char* seq;
    int x, y;
    int win;
    int where;
} mouseevt_t;

extern void                 key_init(void);
extern void                 key_shutdown(void);
extern void                 key_typeables(void);
extern int                  key_define_key_seq(int key, const char *str);
extern void *               key_get_seq_list(int *num_syms);

extern int                  key_string2code(const char *cp, char *buf, int bufsize);
extern const KEY *          key_string2seq(const char *cp, int *lenp);
extern const char *         key_code2name(int key);
extern int                  key_name2code(const char *name, int *lenp);

extern void                 key_cache_key(ref_t *pp, int ch, int front);
extern void                 key_cache_mouse(ref_t *pp, int ch, int front, const mouseevt_t *evt);
extern int                  key_cache_test(ref_t *pp);
extern int                  key_cache_pop(ref_t *pp, struct IOEvent *evt);

extern void                 key_execute(int c, const char *seq);

extern int                  key_check(const char *buf, unsigned buflen, int *multi, int no_ambig);
extern int                  key_mapwin32(unsigned, unsigned, unsigned);

extern void                 key_local_detach(BUFFER_t *bp);

extern void                 do_assign_to_key(void);
extern void                 do_copy_keyboard(void);
extern void                 do_get_mouse_pos(void);
extern void                 do_int_to_key(void);
extern void                 do_key_list(void);
extern void                 do_key_to_int(void);
extern void                 do_keyboard_pop(void);
extern void                 do_keyboard_push(void);
extern void                 do_keyboard_typeables(void);
extern void                 do_push_back(void);
extern void                 do_set_kbd_name(void);
extern void                 do_set_macro_history(void);
extern void                 do_use_local_keyboard(void);
extern void                 inq_assignment(void);
extern void                 inq_command(void);
extern void                 inq_kbd_name(void);
extern void                 inq_keyboard(void);
extern void                 inq_local_keyboard(void);
extern void                 inq_macro_history(void);

extern ref_t *              x_push_ref;         /* Pointer to keyboard push back buffer. */

extern int32_t              x_character;        /* Current character typed. */

__CEND_DECLS

#endif /*GR_KEYBOARD_H_INCLUDED*/

