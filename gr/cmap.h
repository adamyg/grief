#ifndef GR_CMAP_H_INCLUDED
#define GR_CMAP_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_cmap_h,"$Id: cmap.h,v 1.6 2014/10/22 02:32:54 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: cmap.h,v 1.6 2014/10/22 02:32:54 ayoung Exp $
 * Character map primitives.
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

extern void                 cmap_init(void);
extern void                 cmap_shutdown(void);

extern const cmapchr_t *    cmapchr_lookup(const cmap_t *cmap, vbyte_t ch);
extern int                  cmapchr_class(const cmap_t *cmap, vbyte_t ch);
extern int                  cmapchr_width(const cmap_t *cmap, vbyte_t ch);

extern vbyte_t              cmap_specunicode(vbyte_t ch);

extern void                 do_create_char_map(void);
extern void                 do_set_buffer_cmap(void);
extern void                 do_set_window_cmap(void);
extern void                 inq_char_map(void);

extern const cmap_t *       x_base_cmap;
extern const cmap_t *       x_binary_cmap;
extern const cmap_t *       x_default_cmap;
extern const cmap_t *       cur_cmap;

__CEND_DECLS

#endif /*GR_CMAP_H_INCLUDED*/
