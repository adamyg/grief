#include <edidentifier.h>
__CIDENT_RCSID(gr_vfs_stream_c,"$Id: vfs_stream.c,v 1.13 2020/04/14 23:13:32 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vfs_stream.c,v 1.13 2020/04/14 23:13:32 cvsuser Exp $
 * Virtual file system interface - streams.
 *
 *
 * Copyright (c) 1998 - 2019, Adam Young.
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
#include <errno.h>

#include "vfs_internal.h"

static vfs_file_t *         fdstream(const char *path, int handle, int flags, int mode, unsigned bsize);


vfs_file_t *
vfs_fopen(const char *path, int flags, int mode, unsigned bsize)
{
    vfs_file_t *file = NULL;
    int handle = -1;

    if ((flags & O_WRONLY) == O_WRONLY) {
        if ((handle = vfs_open(path, flags, (mode ? mode : 0666))) >= 0) {
            file = fdstream(path, handle, flags, mode, bsize);
        }
    }

    VFS_TRACE(("vfs_fopen(%s, 0x%04x, 0x%03o, %u) : %d\n", \
        path, flags, mode, bsize, (file ? handle : -1)))
    return file;
}


vfs_file_t *
vfs_fdopen(const char *path, int handle, int flags, int mode, unsigned bsize)
{
    vfs_file_t *file = NULL;

    if ((flags & O_WRONLY) == O_WRONLY) {
        if (handle >= 0) {
            file = fdstream(path, handle, flags, mode, bsize);
        }
    }

    VFS_TRACE(("vfs_fdopen(%s, %d, 0x%04x, 0x%03o, %u) : %d\n", \
        path, handle, flags, mode, bsize, (file ? 0 : -1)))
    return file;
}


static vfs_file_t *
fdstream(const char *path, int handle, int flags, int mode, unsigned bsize)
{
    const unsigned pathlen = strlen(path);
    vfs_file_t *file;

    __CUNUSED(mode);

    /*
     *  Build file structure/
     *
     *      [ File control structure                ]
     *      [ Buffer of size 'bsize' in bytes       ]
     *      [ Path buffer of 'pathlen+1' in bytes   ]
     */
    if (bsize < 1024)  {
        bsize = 1024;                           /* default/min buffer */
    }

    if (NULL == (file = chk_alloc(sizeof(vfs_file_t) + bsize + pathlen + 1))) {
        vfs_close(handle);
        return NULL;
    }

    file->f_magic   = VFILE_MAGIC;
    file->f_handle  = handle;
    file->f_buffer  = (char *)(file + 1);
    file->f_path    = (char *)(file->f_buffer + bsize);
    strcpy((char *)file->f_path, path);
    file->f_bsize   = bsize;
    file->f_flags   = flags;
    file->f_cursor  = file->f_buffer;
    file->f_left    = file->f_bsize;
    return file;
}


int
vfs_fwrite(vfs_file_t *file, const void *buffer, unsigned size)
{
    const char *t_buffer = buffer;
    unsigned t_size = size;

    assert(VFILE_MAGIC == file->f_magic);
    VFS_TRACE2(("vfs_fwrite(%s, %u, left:%u)", file->f_path, size, file->f_left))

    /* space within cache? */
    if (file->f_left < t_size) {
        const int handle = file->f_handle;
        const unsigned bsize = file->f_bsize;
        int total = 0, ret;

        /* full cache */
        if (file->f_left) {
            unsigned left = file->f_left;

            VFS_TRACE2((" cached(%u/%u)", left, t_size))
            memcpy(file->f_cursor, t_buffer, left);
            t_buffer += left;
            t_size -= left;
            total += left;
        }

        /* flush cache */
        VFS_TRACE2((" flush"))
        if ((ret = vfs_write(handle, file->f_buffer, bsize)) != bsize) {
            size = ret;
            goto done;
        }
        file->f_cursor = file->f_buffer;
        file->f_left = bsize;

        /* write extra buffer(s) */
        while (t_size > bsize) {
            VFS_TRACE2((" write(%u/%u)", bsize, t_size))
            if ((ret = vfs_write(handle, t_buffer, bsize)) != bsize) {
                size = total;
                goto done;
            }
            t_buffer += bsize;
            t_size -= bsize;
            total += bsize;
        }
    }

    /* cache within buffer */
    if (t_size) {
        VFS_TRACE2((" cached(%u)", t_size))
        memcpy(file->f_cursor, t_buffer, t_size);
        file->f_cursor += t_size;
        file->f_left -= t_size;
    }

done:;
    VFS_TRACE2((" : %d\n", size))
    return size;
}


int
vfs_fputc(vfs_file_t *file, char ch)
{
    assert(VFILE_MAGIC == file->f_magic);
    VFS_TRACE2(("vfs_fputc(%s, left:%u)", file->f_path, file->f_left))

    if (file->f_left) {
        VFS_TRACE2((": 1\n"))
        *file->f_cursor++ = ch; --file->f_left;
        return 1;
    }
    return vfs_fwrite(file, &ch, 1);
}


int
vfs_fputs(vfs_file_t *file, const char *str)
{
    assert(VFILE_MAGIC == file->f_magic);
    return vfs_fwrite(file, str, strlen(str));
}


int
vfs_fclose(vfs_file_t *file)
{
    const unsigned final = file->f_bsize - file->f_left;
    int ret = 0;

    assert(VFILE_MAGIC == file->f_magic);
    VFS_TRACE2(("vfs_fclose(%s, left:%u, final:%u)", file->f_path, file->f_left, final))

    if (final) {
        VFS_TRACE2((" flush"))
        if (vfs_write(file->f_handle, file->f_buffer, final) != final) {
            ret = -1;
        }
    }

    vfs_close(file->f_handle);
    if (file->f_buffer != (char *)(file+1))
        chk_free(file->f_buffer);
    chk_free(file);

    VFS_TRACE2((": %d\n", ret))
    return ret;
}
/*end*/
