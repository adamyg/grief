#include <edidentifier.h>
__CIDENT_RCSID(gr_chunk_c,"$Id: chunk.c,v 1.26 2025/02/07 03:03:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: chunk.c,v 1.26 2025/02/07 03:03:20 cvsuser Exp $
 * Buffer chunk implementation.
 *
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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

#ifndef ED_LEVEL
#define ED_LEVEL 1
#endif

#include <editor.h>

#if defined(HAVE_MMAP)
#if defined(HAVE_SYS_MMAN_H)
#include <sys/mman.h>
#endif
#if !defined(MAP_ANONYMOUS) && defined(MAP_ANON)
#define MAP_ANONYMOUS   MAP_ANON
#endif
#if !defined(MAP_FILE)
#define MAP_FILE        0
#endif
#endif

/*
 *  File chunk size, in bytes.
 *
 *  Chunk and the underlying line size should not exceed 16,777,215 bytes (2^24) being the
 *  current under limit on lines (see LINE_t definition).
 */
#if (2 == SIZEOF_INT)
#define CHUNK_MMAP      (32 * 1024)
#define CHUNK_SIZE      (32 * 1024)

#else
#if defined(HAVE_MMAP)
#define CHUNK_SIZE      (128 * 1024)
#else
#define CHUNK_SIZE      (64 * 1024)
#endif
#define CHUNK_MMAP      (64 * 1024)
#endif

#include "debug.h"
#include "chunk.h"
#include "main.h"

#if defined(HAVE_MMAP)
static int              chunkmmap(BUFFER_t *bp, size_t size);
#endif
static void             chunkfree(BUFFERCHUNKLIST_t *chunklist, BUFFERCHUNK_t *chunk);

#if defined(HAVE_MMAP)
static int              x_zerofd = -1;
#endif


/*  Function:           chunk_attach
 *      Attach the chunk cache to the specified buffer.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing
 */
void
chunk_attach(BUFFER_t *bp)
{
    BUFFERCHUNKLIST_t *chunklist = &bp->b_chunk_list;

    ED_TRACE(("chunk_attach(buffer:%p)\n", bp))
    bp->b_chunk_ident = 0;
    TAILQ_INIT(chunklist);
}


/*  Function:           chunk_detach
 *      Detach the chunk cache for the specified buffer.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing
 */
void
chunk_detach(BUFFER_t *bp)
{
    ED_TRACE(("chunk_detach(buffer:%p)\n", bp))

    bp->b_chunk_ident = 0;
    chunk_zap(bp);
}


/*  Function:           chunk_size
 *      Determine the desired size of the next file chunk.
 *
 *  Parameters:
 *      size - Required size, in bytes.
 *
 *  Returns:
 *      Chunk size, in bytes (<= size).
 */
size_t
chunk_size(size_t size)
{
    return (size > CHUNK_SIZE ? CHUNK_SIZE : size);
}


/*  Function:           chunk_new
 *      Allocate a new buffer specific chunk.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      size - Chunk size, in bytes.
 *      chunk - Chunk object reference (handle).
 *
 *  Returns:
 *      Buffer address and optional hunk address assignment, other NULL.
 */
void *
chunk_new(BUFFER_t *bp, size_t size, void **chunkp)
{
    BUFFERCHUNKLIST_t *chunklist = &bp->b_chunk_list;
    BUFFERCHUNK_t *chunk = NULL;
    void *buffer = NULL;

#if defined(HAVE_MMAP)
    if (chunkmmap(bp, size) && size >= 1024) {  /* mmap chunk? */

        if (NULL != (chunk = chk_calloc(sizeof(BUFFERCHUNK_t), 1))) {
#if !defined(MAP_ANONYMOUS)
            if (x_zerofd < 0) {
                x_zerofd = open("/dev/zero", O_RDWR);
            }
#define MAP_ANONYMOUS       0
#endif

            if (MAP_FAILED != (buffer =         /* MMAP */
                    (void *) mmap((void *) NULL, size, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, x_zerofd, 0))) {
                chunk->c_vmaddr = buffer;
                chunk->c_vmsize = size;
                ED_TRACE(("chunk_vmnew(vmaddr:%p,vmsize:%lu)\n", buffer, (unsigned long)size))

            } else {
                ED_TRACE(("chunk_vmnew(vmsize:%lu) = %d (error)\n", (unsigned long)size, errno))
                chk_free(chunk);
                chunk = NULL;
            }
        }
    }
#endif  /*HAME_MMAP*/

    if (NULL == chunk) {
        if (NULL == (chunk = chk_calloc(sizeof(BUFFERCHUNK_t) + (size | 0xf), 1))) {
            return NULL;
        }
        chunk->c_vmaddr = NULL;
        buffer = (void *)(chunk + 1);
        ED_TRACE(("chunk_new(addr:%p,size:%lu)\n", buffer, (unsigned long)size))
    }

    chunk->c_magic  = BUFFERCHUNK_MAGIC;
    chunk->c_magic2 = BUFFERCHUNK_MAGIC;
    chunk->c_ident  = ++bp->b_chunk_ident;
    chunk->c_size   = (uint32_t)size;
    chunk->c_refers = 0;
    chunk->c_buffer = bp;

    TAILQ_INSERT_TAIL(chunklist, chunk, c_node);

    if (chunkp) {
        *chunkp = chunk;
    }
    return buffer;
}


void
chunk_delete(BUFFER_t *bp, void *chunk)
{
    ED_TRACE(("chunk_delete(buffer:%p,chunk:%p)\n", bp, chunk))

    if (bp && chunk) {
        BUFFERCHUNK_t *t_chunk = (BUFFERCHUNK_t *)chunk;

        assert(BUFFERCHUNK_MAGIC == t_chunk->c_magic);
        assert(BUFFERCHUNK_MAGIC == t_chunk->c_magic2);
        if (NULL == bp) {                       /* implied chunk onwer */
            bp = t_chunk->c_buffer;
        }
        assert(bp == t_chunk->c_buffer);
        if (bp) {
            assert(0 == t_chunk->c_refers);
            chunkfree(&bp->b_chunk_list, chunk);
        }
    }
}


/*  Function:           chunk_zap
 *      Zap the chunk cache for the specified buffer.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing.
 */
void
chunk_zap(BUFFER_t *bp)
{
    BUFFERCHUNKLIST_t *chunklist = &bp->b_chunk_list;
    BUFFERCHUNK_t *chunk;

    ED_TRACE(("chunk_zap(buffer:%p)\n", bp))

    while (NULL != (chunk = TAILQ_FIRST(chunklist))) {
        assert(BUFFERCHUNK_MAGIC == chunk->c_magic);
        assert(BUFFERCHUNK_MAGIC == chunk->c_magic2);
        chunkfree(chunklist, chunk);
    }
}


/*  Function:           chunk_protect
 *      Protect the specified chunk against write operations.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *      chunk - Chunk size, in bytes.
 *      refs - Reference count.
 *
 *  Returns:
 *      nothing.
 */
void
chunk_protect(BUFFER_t *bp, void *chunk, uint32_t refs)
{
    ED_TRACE(("chunk_protect(buffer:%p,chunk:%p,refs:%u)\n", bp, chunk, refs))

    if (bp && chunk) {
        BUFFERCHUNK_t *t_chunk = (BUFFERCHUNK_t *)chunk;

        assert(BUFFERCHUNK_MAGIC == t_chunk->c_magic);
        assert(BUFFERCHUNK_MAGIC == t_chunk->c_magic2);
        assert(bp == t_chunk->c_buffer);
        assert(refs);

        if (bp == t_chunk->c_buffer) {
#if defined(HAVE_MMAP)
#if defined(HAVE_MPROTECT)
            if (t_chunk->c_vmaddr) {            /* protect page, read-only */
                errno = 0;
                mprotect(t_chunk->c_vmaddr, t_chunk->c_vmsize, PROT_READ);
                ED_TRACE(("==> mprotect(read-only) : %d\n", errno))
            }
#endif
#endif
            t_chunk->c_refers += refs;
        }
    }
}


/*  Function:           chunk_ref
 *      Increment the chunk reference.
 *
 *  Parameters:
 *      chunk - Chunk address.
 *      bp - Buffer object address.
 *
 *  Returns:
 *      Chunk address.
 */
void *
chunk_ref(void *chunk, BUFFER_t *bp)
{
    uint32_t refs = 0;

    ED_TRACE(("chunk_ref(buffer:%p,chunk:%p)", bp, chunk))

    if (chunk) {
        BUFFERCHUNK_t *t_chunk = (BUFFERCHUNK_t *)chunk;

        assert(BUFFERCHUNK_MAGIC == t_chunk->c_magic);
        assert(BUFFERCHUNK_MAGIC == t_chunk->c_magic2);
        if (NULL == bp) {                       /* implied chunk onwer */
            bp = t_chunk->c_buffer;
        }
        assert(bp == t_chunk->c_buffer);

        if (bp) {
            assert(t_chunk->c_refers);
            refs = ++t_chunk->c_refers;
        }
    }

    ED_TRACE((" : refs=%u\n", (unsigned)refs))
    return chunk;
}


/*  Function:           chunk_unref
 *      Decrement the chunk reference.
 *
 *  Parameters:
 *      chunk - Chunk address.
 *      bp - Buffer object address.
 *
 *  Returns:
 *      Chunk address.
 */
uint32_t
chunk_unref(void *chunk, BUFFER_t *bp)
{
    uint32_t refs = 0;

    ED_TRACE(("chunk_unref(buffer:%p,chunk:%p)", bp, chunk))

    if (chunk) {
        BUFFERCHUNK_t *t_chunk = (BUFFERCHUNK_t *)chunk;

        assert(BUFFERCHUNK_MAGIC == t_chunk->c_magic);
        assert(BUFFERCHUNK_MAGIC == t_chunk->c_magic2);
        if (NULL == bp) {                       /* implied chunk onwer */
            bp = t_chunk->c_buffer;
        }
        assert(bp == t_chunk->c_buffer);

        if (bp) {
            assert(t_chunk->c_refers);
            if (0 == (refs = --t_chunk->c_refers)) {
                chunkfree(&bp->b_chunk_list, chunk);
            }
        }
    }

    ED_TRACE((" : refs=%u\n", (unsigned)refs))
    return refs;
}


#if defined(HAVE_MMAP)
static int
chunkmmap(BUFFER_t *bp, size_t size)
{
    BUFFERCHUNKLIST_t *chunklist = &bp->b_chunk_list;
    BUFFERCHUNK_t *chunk;

    if (! BFTST(bp, BF_SYSBUF)) {               /* not system-buffers */
        if (size >= CHUNK_MMAP ||               /* size or secondary mmap chunk */
                ((NULL != (chunk = TAILQ_FIRST(chunklist))) && chunk->c_vmaddr)) {
            return TRUE;
        }
    }
    return FALSE;
}
#endif


static void
chunkfree(BUFFERCHUNKLIST_t *chunklist, BUFFERCHUNK_t *chunk)
{
#if defined(HAVE_MMAP)
    if (chunk->c_vmaddr) {                      /* release page */
        errno = 0;
        (void) munmap(chunk->c_vmaddr, chunk->c_vmsize);
        ED_TRACE(("==> munmap(%p) : %d\n", chunk->c_vmaddr, errno))
        chunk->c_vmaddr = NULL;
    }
#endif
    TAILQ_REMOVE(chunklist, chunk, c_node);
    memset(chunk, 9, sizeof(*chunk));
    chunk->c_magic = 0xDEADBEEF;
    chunk->c_magic2 = 0xDEADBEEF;
    chk_free(chunk);
}

/*end*/
