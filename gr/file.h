#ifndef GR_FILE_H_INCLUDED
#define GR_FILE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_file_h,"$Id: file.h,v 1.30 2024/05/11 16:38:28 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: file.h,v 1.30 2024/05/11 16:38:28 cvsuser Exp $
 * File ulitities.
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

enum {
    PERCENTAGE_BYTES,
    PERCENTAGE_LINES,
    PERCENTAGE_FILE
};

extern int                  file_terminator_set(BUFFER_t *bp, const void *buffer, int length, int termtype);
extern int                  file_terminator_get(BUFFER_t *bp, void *buffer, int length, int *termtype);

extern int                  file_edit(const char *fname, const int32_t flags, const char *encoding);
extern int                  file_load(const char *fname, const int32_t flags, const char *encoding);
extern int                  file_write(const char *fname, const int32_t flags);
extern int                  file_readin(BUFFER_t *bp, const char *fname, const int32_t flags, const char *encoding);

extern int                  file_rdonly(BUFFER_t *bp, const char *who);
extern int                  rdonly(void);

extern void                 file_attach(BUFFER_t *bp);
extern void                 file_cleanup(BUFFER_t *bp);
extern int                  file_cmp(const char *f1, const char *f2);
extern int                  file_ncmp(const char *f1, const char *f2, int len);
extern char *               file_case(char *str);
extern char *               file_slashes(char *str);
extern char *               file_tilder(const char *file, char *path, int len);
extern char *               file_getenv(char *path, int len);
extern char *               file_expand(const char *file, char *path, int len);
extern int                  file_chdir(const char *dir);
extern char *               file_cwd(char *cwd, unsigned length);
extern char *               file_cwdd(int drv, char *cwd, unsigned length);
extern char *               file_canonicalize(const char *file, char *path, int len);
extern char *               file_modedesc(mode_t mode, const char *source, int type, char *buffer, int len);

extern int                  second_passed(void);
extern void                 percentage(int what, accuint_t a, accuint_t b, const char *str, const char *str1);

extern void                 do_edit_file(int version);
extern void                 do_output_file(void);
extern void                 do_read_file(void);
extern void                 do_reload_buffer(void);
extern void                 do_set_binary_size(void);
extern void                 do_set_file_magic(void);
extern void                 do_set_terminator(void);
extern void                 do_write_buffer(void);
extern void                 inq_byte_pos(void);
extern void                 inq_file_change(void);
extern void                 inq_file_magic(void);
extern void                 inq_terminator(void);

__CEND_DECLS

#endif /*GR_FILE_H_INCLUDED*/
