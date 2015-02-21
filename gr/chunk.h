#ifndef GR_CHUNK_H_INCLUDED
#define GR_CHUNK_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_chunk_h,"$Id: chunk.h,v 1.10 2015/02/21 22:47:27 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: chunk.h,v 1.10 2015/02/21 22:47:27 ayoung Exp $
 * File memory management.
 *
 *
 * Copyright (c) 1998 - 2015, Adam Young.
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

extern void                 chunk_attach(BUFFER_t *bp);
extern void                 chunk_detach(BUFFER_t *bp);
extern size_t               chunk_size(size_t size);

extern void *               chunk_new(BUFFER_t *bp, size_t size, void **chunk);
extern void                 chunk_delete(BUFFER_t *bp, void *chunkp);
extern void                 chunk_protect(BUFFER_t *bp, void *chunk, uint32_t refs);
extern void *               chunk_ref(void *chunk, BUFFER_t *bp);
extern uint32_t             chunk_unref(void *chunk, BUFFER_t *bp);

extern void                 chunk_zap(BUFFER_t *bp);

__CEND_DECLS

#endif /*GR_CHUNK_H_INCLUDED*/
