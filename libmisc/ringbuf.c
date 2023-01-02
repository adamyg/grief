#include <edidentifier.h>
__CIDENT_RCSID(gr_ringbuf_c,"$Id: ringbuf.c,v 1.9 2022/12/03 16:33:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ringbuf.c,v 1.9 2022/12/03 16:33:05 cvsuser Exp $
 * Simple ringbuffer
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

#include <editor.h>
#include <edtypes.h>
#include <edringbuf.h>
#include <assert.h>

#define RBMAGIC             MKMAGIC('e','D','r','B')
#define RBMIN(__a,__b)      (__a < __b ? __a : __b)

void 
ringbuf_init(struct ringbuf *rb, size_t size)
{
    assert(rb);
    assert(size >= 16);
    assert(RBMAGIC != rb->rb_magic);
    rb->rb_magic = RBMAGIC;
    rb->rb_size = size;
    rb->rb_cursor = 0;
    rb->rb_available = 0;
}


struct ringbuf *
ringbuf_new(size_t size)
{
    struct ringbuf *rb;

    assert(size >= 16);
    rb = malloc(sizeof(struct ringbuf) + size);
    ringbuf_init(rb, size);
    return rb;
}


void
ringbuf_delete(struct ringbuf *rb)
{
    assert(RBMAGIC == rb->rb_magic);
    rb->rb_magic = 0;
    free(rb);
}


void 
ringbuf_clear(struct ringbuf *rb)
{
    assert(RBMAGIC == rb->rb_magic);
    rb->rb_available   = 0;
    rb->rb_cursor = 0;
}


//  Remaining space within the rinfbuffer, in bytes.
size_t 
ringbuf_remaining(struct ringbuf *rb)
{
    if (!rb) return 0;
    assert(RBMAGIC == rb->rb_magic);
    return (rb->rb_size - rb->rb_available);
}


//  Available bytes within the ringbuf
size_t 
ringbuf_available(struct ringbuf *rb)
{
    if (!rb) return 0;
    assert(RBMAGIC == rb->rb_magic);
    return rb->rb_available;
}


size_t 
ringbuf_write(struct ringbuf *rb, const void *data, size_t len)
{
    size_t cursor, tailsz, written;

    if (!rb) return 0;
    assert(RBMAGIC == rb->rb_magic);

    cursor  = (rb->rb_cursor + rb->rb_available) % rb->rb_size;
    tailsz = ringbuf_remaining(rb);
    tailsz = RBMIN(len, tailsz);
    if (tailsz < 1) return 0;

    len = rb->rb_size - cursor;
    len = RBMIN(len, tailsz);
    memcpy(rb->rb_data + cursor, (const char *)data, len);
    rb->rb_available += len;
    written = len;
    if (len < tailsz) {
        tailsz -= len;
        len = RBMIN(tailsz, ringbuf_remaining(rb));
        memcpy(rb->rb_data, ((const char *)data) + written, len);
        rb->rb_available += len;
        written += len;
    }
    assert(rb->rb_available <= rb->rb_size);
    return written;
}


size_t 
ringbuf_read(struct ringbuf *rb, void *data, size_t len)
{
    size_t cursor, headsz, got;

    if (!rb) return 0;
    assert(RBMAGIC == rb->rb_magic);

    cursor = rb->rb_cursor;
    headsz = ringbuf_available(rb);
    headsz = RBMIN(len, headsz);
    if (headsz < 1) return 0;

    len = rb->rb_size - cursor;
    len = RBMIN(len, headsz);
    memcpy(data, ((const char *)rb->rb_data) + cursor, len);
    rb->rb_available -= len;
    rb->rb_cursor += len;
    got = len;
    if (len < headsz) {
        headsz -= len;
        len = RBMIN(headsz, ringbuf_available(rb));
        memcpy(((char *)data) + got, (const char *)rb->rb_data, len);
        rb->rb_cursor = len;
        rb->rb_available -= len;
        got += len;
    }
    assert(rb->rb_available <= rb->rb_size);
    return got;
}


#ifdef TEST_RB
int 
main(void)
{
    struct _myrb {
        struct ringbuf rb;
        char buff[10];
    } myrb;
    char buffer[10];
    char obuffer[10];
    int got, tow;

    ringbuf_init(&myrb.rb, 10);
    while ((got = read(0, buffer, 10))) {
        int len, olen;
        len = ringbuf_write(&myrb.rb, buffer, got);
        olen = ringbuf_read(&myrb.rb, obuffer, 10);
        write(1, obuffer, olen);
    }
    return 1;
}
#endif
/*end*/
