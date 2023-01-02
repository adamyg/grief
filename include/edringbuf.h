#ifndef GR_EDRINGBUF_H_INCLUDED
#define GR_EDRINGBUF_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edringbuf_h,"$Id: edringbuf.h,v 1.9 2023/01/01 11:26:59 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edringbuf.h,v 1.9 2023/01/01 11:26:59 cvsuser Exp $
 * Basic binary ringbuffer ...
 *
 *
 *
 * Copyright (c) 1998 - 2023, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <edtypes.h>

__CBEGIN_DECLS

struct ringbuf {
    uint32_t        rb_magic;
    size_t          rb_size;
    size_t          rb_available;
    size_t          rb_cursor;
    uint8_t         rb_data[1];
};

void                ringbuf_init(struct ringbuf *rb, size_t size);
struct ringbuf *    ringbuf_new(size_t size);
void                ringbuf_delete(struct ringbuf *rb);
void                ringbuf_clear(struct ringbuf *rb);
size_t              ringbuf_remaining(struct ringbuf *rb);
size_t              ringbuf_available(struct ringbuf *rb);
size_t              ringbuf_write(struct ringbuf *rb, const void *data, size_t len);
size_t              ringbuf_read(struct ringbuf *rb, void *data, size_t len);

__CEND_DECLS

#endif /*GR_EDRINGBUF_H_INCLUDED*/
