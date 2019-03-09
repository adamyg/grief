#ifndef GR_LIBCHARTABLE_H_INCLUDED
#define GR_LIBCHARTABLE_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_libchartable_h,"$Id: libchartable.h,v 1.12 2018/10/01 22:10:53 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* libchartable.
 *
 *
 * Copyright (c) 2010 - 2018, Adam Young.
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

#define UNICODE_HI_SURROGATE_START 0xD800
#define UNICODE_HI_SURROGATE_END 0xDBFF

#define UNICODE_LO_SURROGATE_START 0xDC00
#define UNICODE_LO_SURROGATE_END 0xDFFF

#define UNICODE_REPLACE         0xfffd          /* replacement character */
#define UNICODE_REPLACECTRL     0x1a            /* substitute */
#define UNICODE_REPLACEFULL     0xff1f          /* full-width '?' */
#define UNICODE_MAX             0x10ffff

#define CS_NAMELEN              80

#define CS_ICONV_TRANSLIT       0x01
#define CS_ICONV_IGNORE         0x02


enum {
    /*
     *  character-set/alias format/mode
     */
    CHARSET_MODE_BASIC = 1,
    CHARSET_MODE_INI,
    CHARSET_MODE_X11,
    CHARSET_MODE_ICONV,
    CHARSET_MODE_MOZILLA,
    CHARSET_MODE_IANA
};


struct charsetmap {
    unsigned                cs_magic;
    struct charset *        cs_table;
    unsigned                cs_count;
    unsigned                cs_alloced;
};


typedef struct charset_iconv {
    /*
     *  iconv descriptor
     */
    MAGIC_t                 ic_magic;           /* structure magic */
    void *                  ic_module;
    const char *            ic_desc;
    void *                  ic_data;
    uint32_t                ic_flags;
    const void *          (*ic_decode)(struct charset_iconv *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
    int                   (*ic_encode)(struct charset_iconv *ic, const int32_t ch, void *buffer);
    size_t                (*ic_import)(struct charset_iconv *ic, const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes);
    size_t                (*ic_export)(struct charset_iconv *ic, const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes);
} charset_iconv_t;

extern void                     charset_alias_init(void);
extern void                     charset_alias_shutdown(void);

extern int                      charset_alias_open(int mode, int paths, const char **dirs, const char *aliasset);
extern int                      charset_alias_load(int mode, const char *aliasset);

extern const char *             charset_alias_lookup(const char *name, int namelen);
extern void                     charset_alias_dump(void);

extern const char *             charset_map_locale(const char *locale, char *buffer, int bufsize);

extern const char *             charset_description(int32_t ch, char *buffer, int buflen);

extern const char *             charset_terminal_encoding(void);
extern const char *             charset_text_encoding(void);
extern const char *             charset_current(const char *env, const char *def);

extern const char *             charset_canonicalize(const char *name, int namelen, char *buffer, int bufsiz);
extern int                      charset_compare(const char *primary, const char *name, int namelen);

extern void                     charset_iconv_home(const char *path);
extern void                     charset_iconv_path(const char *path);
extern struct charset_iconv *   charset_iconv_open(const char *name, int flags);
extern void                     charset_iconv_close(struct charset_iconv *iconv);
extern const void *             charset_iconv_decode(struct charset_iconv *iconv, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
extern int                      charset_iconv_encode(struct charset_iconv *iconv, const int32_t ch, void *buffer);
extern int                      charset_iconv_length(struct charset_iconv *iconv, const int32_t ch);
extern size_t                   charset_iconv_import(struct charset_iconv *ic,
                                            const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes);
extern size_t                   charset_iconv_export(struct charset_iconv *ic,
                                            const char **inbuf, size_t *inbytes, char **outbuf, size_t *outbytes);

extern int                      charset_width_ucs(int32_t ucs, int bad);
extern int                      charset_swidth_ucs(const int32_t *pwcs, size_t n);

extern int                      charset_width_cjk(int32_t ucs);
extern int                      charset_swidth_cjk(const int32_t *pwcs, size_t n);

extern int                      charset_locale_utf8(const char *encoding);
extern const void *             charset_utf8_decode(const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
extern const void *             charset_utf8_decode_safe(const void *src, const void *cpend, int32_t *cooked);
extern int                      charset_utf8_encode(const int32_t ch, void *buffer);
extern int                      charset_utf8_length(const int32_t ch);

extern const void *             charset_utf16_decode(int endian, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
extern const void *             charset_utf16_decode_safe(int endian, const void *src, const void *cpend, int32_t *cooked);
extern int                      charset_utf16_encode(int endian, const int32_t ch, void *buffer);

extern const void *             charset_utf16be_decode(const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
extern const void *             charset_utf16be_decode_safe(const void *src, const void *cpend, int32_t *cooked);
extern int                      charset_utf16be_encode(const int32_t ch, void *buffer);

extern const void *             charset_utf16le_decode(const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
extern const void *             charset_utf16le_decode_safe(const void *src, const void *cpend, int32_t *cooked);
extern int                      charset_utf16le_encode(const int32_t ch, void *buffer);

extern const void *             charset_utf32_decode(int endian, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
extern const void *             charset_utf32_decode_safe(int endian, const void *src, const void *cpend, int32_t *cooked);
extern int                      charset_utf32_encode(int endian, int32_t ch, void *buffer);

extern const void *             charset_utf32be_decode(const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
extern const void *             charset_utf32be_decode_safe(const void *src, const void *cpend, int32_t *cooked);
extern int                      charset_utf32be_encode(const int32_t ch, void *buffer);

extern const void *             charset_utf32le_decode(const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
extern const void *             charset_utf32le_decode_safe(const void *src, const void *cpend, int32_t *cooked);
extern int                      charset_utf32le_encode(const int32_t ch, void *buffer);

__CEND_DECLS

#endif /*GR_LIBCHARTABLE_H_INCLUDED*/
