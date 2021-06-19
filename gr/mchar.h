#ifndef GR_MCHAR_H_INCLUDED
#define GR_MCHAR_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_mchar_h,"$Id: mchar.h,v 1.16 2021/06/13 16:29:54 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: mchar.h,v 1.16 2021/06/13 16:29:54 cvsuser Exp $
 * Multibyte character support.
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

#define MCHAR_MAX_ENCODING      48
#define MCHAR_MAX_TERMBUF       16
#define MCHAR_MAX_LENGTH        6

#define MCHAR_ISUTF8(_ch)       ((_ch) >= 0x80)

#define MCHAR_UTF1              "utf-1"
#define MCHAR_UTF7              "utf-7"
#define MCHAR_UTF8              "utf-8"
#define MCHAR_UTF16             "utf-16"
#define MCHAR_UTF32             "utf-32"
#define MCHAR_UTF16BE           "utf-16be"
#define MCHAR_UTF16LE           "utf-16le"
#define MCHAR_UTF32BE           "utf-32be"
#define MCHAR_UTF32LE           "utf-32le"
#define MCHAR_UTF32XX           "utf-32xx"
#define MCHAR_UTFEBCDIC         "utf-ebcdic"
#define MCHAR_BOCU1             "bocu-1"
#define MCHAR_SCSU              "scsu"
#define MCHAR_GB18030           "gb18030"

typedef int32_t MCHAR_t;                        /* wide/multiple character value */

typedef struct mcharguessinfo {
    BUFTYPE_t               fi_type;            /* buffer type */
    int                     fi_endian;          /* big-endian(1), little-endian(0), otherwise(-1) */
    unsigned                fi_flags;
#define MCHAR_FI_CRLF           0x0001

    const char *            fi_desc;            /* description, if any */
    char                    fi_encoding[MCHAR_MAX_ENCODING];

    const char *            fi_bomdes;
    unsigned                fi_bomlen;
    unsigned char           fi_bombuf[4];

    unsigned                fi_termtype;
    unsigned                fi_termlen;
    unsigned char           fi_termbuf[MCHAR_MAX_TERMBUF];
} mcharguessinfo_t;


typedef struct mcharcharsetinfo_t {
    int                     cs_type;            /* buffer type */
    uint32_t                cs_flags;           /* type specific flags */
    const char *            cs_name;
    const char *            cs_desc;
    int                     cs_codepage;
} mcharcharsetinfo_t;


typedef struct {                /* i/o streaming */
    MAGIC_t                 is_magic;           /* structure magic */
#define MCHAR_ISTREAM_MAGIC     MKMAGIC('M','i','S','m')

    const char *            is_filename;        /* filename */
    int                     is_filehandle;      /* physical file descriptor/handle */
    int                     is_direction;       /* conversion direction */
    ssize_t                 is_iobytes;         /* underlying bytes read/written of last operation */
    const char *            is_buffer;          /* local push-back buffer */
    size_t                  is_buflen;          /* and associated size */
    struct mchar_iconv *    is_iconv;           /* owner */
    void *                  is_istream;         /* conversion stream */
    void *                  is_handle;          /* istream specific handle, if any */
} mchar_istream_t;


typedef struct {                /* line cursor */
    MAGIC_t                 ic_magic;           /* structure magic */
#define MCHAR_ICURSOR_MAGIC     MKMAGIC('M','i','L','c')

    const void *            ic_cursor;
    void *                  ic_buffer;
    int32_t                 ic_buflen;
    struct mchar_iconv *    ic_iconv;
} mchar_cursor_t;


typedef struct mchar_iconv {    /* conversion descriptor */
    MAGIC_t                 ic_magic;           /* structure magic */
#define MCHAR_ICONV_MAGIC       MKMAGIC('M','i','C','v')

    const char *            ic_encoding;        /* encoding/driver name */
    uint32_t                ic_unit;            /* base character unit size, 1, 2 or 4 */
    const void *          (*ic_decode)(struct mchar_iconv *ic, const void *src, const void *cpend, int32_t *cooked, int32_t *raw);
    const void *          (*ic_decode_safe)(struct mchar_iconv *ic, const void *src, const void *cpend, int32_t *cooked);
    int                   (*ic_encode)(struct mchar_iconv *ic, const int32_t ch, void *buffer);
    int                   (*ic_length)(struct mchar_iconv *ic, const int32_t ch);

//TODO
//  #define MCHAR_ISSPACE
//  #define MCHAR_ISDIGIT  
//  #define MCHAR_ISWORD
//  #define MCHAR_ISCSYM
//  #define MCHAR_ISLOWER
//  #define MCHAR_ISUPPER
//  int32_t               (*ic_isa)(struct mchar_iconv *ic, const int32_t ch, const int flags);
//
//  #define MCHAR_TOUPPER
//  #define MCHAR_TOLOWER
//  int32_t               (*ic_toa)(struct mchar_iconv *ic, const int32_t ch, const int flags);

    void                  (*ic_close)(struct mchar_iconv *ic);
    mchar_istream_t *     (*ic_stream_open)(struct mchar_iconv *ic, int fd, const char *filename, int direction);
    void                  (*ic_stream_close)(struct mchar_iconv *ic, mchar_istream_t *is);

    const char *            ic_internal;        /* internal encoding, if different to encoding */
    mcharcharsetinfo_t      ic_info;
    void *                  ic_xhandle;
    void *                  ic_ihandle;
    void *                  ic_ohandle;
    mchar_istream_t *       ic_istream;         /* active stream */
    uint32_t                ic_uflags;
    MAGIC_t                 ic_magic2;
} mchar_iconv_t;

extern int                  mchar_init(void);
extern void                 mchar_shutdown(void);

extern int                  mchar_info_init(void);
extern void                 mchar_info_shutdown(void);
extern int                  mchar_info_load(const char *infofile);

extern const char *         mchar_vim_strip(const char *encoding);

extern int                  mchar_locale_utf8(const char *encoding);

extern int                  mchar_alias_load(int type, const char *aliasfile);

extern void                 mchar_guess_init(void);
extern void                 mchar_guess_shutdown(void);
extern int                  mchar_guess_endian(const char *encoding);
extern int                  mchar_guess(const char *specification, unsigned flags,
                                   const void *buffer, unsigned length, struct mcharguessinfo *fileinfo);
extern char *               mchar_guess_default(void);

extern const mcharcharsetinfo_t *
                            mchar_info(mcharcharsetinfo_t *info, const char *name, int namelen);
       
extern void                 mchar_iconv_init(void);
extern mchar_iconv_t *      mchar_iconv_open(const char *encoding);
extern void                 mchar_iconv_close(mchar_iconv_t *iconv);
#define                     mchar_encoding(__ic) \
                                (__ic ? __ic->ic_encoding : "n/a")
#define                     mchar_internal_encoding(__ic) \
                                (__ic && __ic->ic_internal ? __ic->ic_internal : mchar_encoding(__ic))
#define                     mchar_decode(__ic, __src, __cpend, __cooked, __raw) \
                                ((* (__ic)->ic_decode)(__ic, __src, __cpend, __cooked, __raw))
#define                     mchar_decode_safe(__ic, __src, __cpend, __cooked) \
                                ((* (__ic)->ic_decode_safe)(__ic, __src, __cpend, __cooked))
#define                     mchar_encode(__ic, __ch, __buffer) \
                                ((* (__ic)->ic_encode)(__ic, __ch, __buffer))
#define                     mchar_length(__ic, __ch) \
                                ((* (__ic)->ic_length)(__ic, __ch))

extern int                  mchar_ucs_width(int32_t ch, int bad);
extern int                  mchar_ucs_encode(int32_t ch, char *buffer);

extern mchar_istream_t *    mchar_stream_open(mchar_iconv_t *ic, int handle, const char *filename, const char *mode);
extern void                 mchar_stream_push(mchar_istream_t *is, const char *buffer, size_t buflen);
extern size_t               mchar_stream_read(mchar_istream_t *is, char *buffer, size_t buflen, size_t *inbytes);
extern size_t               mchar_stream_write(mchar_istream_t *is, const char *buffer, size_t buflen, size_t *outbytes);
extern void                 mchar_stream_close(mchar_istream_t *is);

extern const char *         sys_get_locale(int isterminal);
extern int                  sys_unicode_locale(int isterminal);

__CEND_DECLS

#endif /*GR_MCHAR_H_INCLUDED*/
