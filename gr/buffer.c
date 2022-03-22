#include <edidentifier.h>
__CIDENT_RCSID(gr_buffer_c,"$Id: buffer.c,v 1.50 2021/10/18 13:16:15 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: buffer.c,v 1.50 2021/10/18 13:16:15 cvsuser Exp $
 * Buffer managment.
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

#if !defined(ED_LEVEL)
#define ED_LEVEL 1
#endif

#include <editor.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"                              /* acc_...() */
#include "anchor.h"                             /* anchor_...() */
#include "buffer.h"
#include "builtin.h"
#include "chunk.h"
#include "color.h"
#include "debug.h"
#include "display.h"                            /* vtiscolor() */
#include "echo.h"
#include "eval.h"
#include "file.h"
#include "hilite.h"                             /* hilite_...() */
#include "keyboard.h"
#include "kill.h"                               /* k_...() */
#include "line.h"
#include "main.h"
#include "map.h"
#include "mchar.h"
#include "register.h"                           /* register_...() */
#include "ruler.h"                              /* tabchar_get/set() */
#include "symbol.h"
#include "system.h"                             /* sys_...() */
#include "window.h"

#include "m_pty.h"                              /* pty_...() */

#define X_BFTYPES       (sizeof(x_bftypes)/sizeof(x_bftypes[0]))

static struct {                 /* buffer types */
    const char *        name;                   /* description */
    size_t              namelen;                /* length of description, in bytes */
    const char *        encoding;               /* implied encoding */
    BUFTYPE_t           type;                   /* internal buffer type enumeration */

} x_bftypes[] = {
#define __BFTYP(__n)    __n, sizeof(__n)-1
    { __BFTYP("UNIX"),          NULL,               BFTYP_UNIX          },  /* LF */
    { __BFTYP("DOS"),           NULL,               BFTYP_DOS           },  /* CR/LF */
    { __BFTYP("MAC"),           NULL,               BFTYP_MAC           },  /* CR */
    { __BFTYP("ANSI"),          NULL,               BFTYP_ANSI          },  /* ANSI */

    { __BFTYP("EBCDIC"),        "ebcdic",           BFTYP_EBCDIC        },  /* EBCDIC */

    { __BFTYP("BIN"),           "binary",           BFTYP_BINARY        },  /* <none> */

    { __BFTYP("UTF8"),          MCHAR_UTF8,         BFTYP_UTF8          },  /* UTF8 */
    { __BFTYP("UC16"),          MCHAR_UTF16,        BFTYP_UTF16         },  /* UTF16/USC2 */
    { __BFTYP("UC32"),          MCHAR_UTF32,        BFTYP_UTF32         },  /* UTF32/USC4 */
    { __BFTYP("UTFE"),          MCHAR_UTFEBCDIC,    BFTYP_UTFEBCDIC     },  /* UTF8/EBCDIC (rare) */
    { __BFTYP("BOCU"),          MCHAR_BOCU1,        BFTYP_BOCU1         },  /* Binary Ordered Compression for Unicode */
    { __BFTYP("SCSU"),          MCHAR_SCSU,         BFTYP_SCSU          },  /* Standard Compression Scheme for Unicode */
    { __BFTYP("UTF7"),          MCHAR_UTF7,         BFTYP_UTF7          },  /* 7-bit Unicode Transformation Format */

    { __BFTYP("GB"),            MCHAR_GB18030,      BFTYP_GB            },
    { __BFTYP("BIG5"),          "big5",             BFTYP_BIG5          },
#if defined(BFTYP_HZ)
    { __BFTYP("HZ"),            "hz",               BFTYP_HZ            },  /* RFC1843, "man hztty" for details */
#endif

    { __BFTYP("2022"),          "iso-2022",         BFTYP_ISO2022       },

    { __BFTYP("SBCS"),          NULL,               BFTYP_SBCS          },  /* Single Byte Character Set */
    { __BFTYP("DBCS"),          NULL,               BFTYP_DBCS          },  /* Double Byte */
    { __BFTYP("MBCS"),          NULL,               BFTYP_MBCS          },  /* Multi/Double Byte Character Set (non-unicode) */
    { __BFTYP("OTHER"),         NULL,               BFTYP_OTHER         },

    { __BFTYP("N/S"),           NULL,               BFTYP_UNSUPPORTED   },  /* Specials */
    { __BFTYP("N/D"),           NULL,               BFTYP_UNKNOWN       },
    { __BFTYP("N/A"),           NULL,               BFTYP_UNDEFINED     }
#undef  __BFTYP
    };

#define X_LTERMS        (sizeof(x_lterms)/sizeof(x_lterms[0]))

static struct {                 /* line terminators */
    const char *        name;
    size_t              namelen;
    int                 type;

} x_lterms[] = {
#define __LTERM(__n)    __n, sizeof(__n)-1
    { __LTERM("UNDEF"),         LTERM_UNDEFINED },
    { __LTERM("UNIX"),          LTERM_UNIX      },
    { __LTERM("DOS"),           LTERM_DOS       },
    { __LTERM("NONE"),          LTERM_NONE      },
    { __LTERM("MAC"),           LTERM_MAC       },
    { __LTERM("NEL"),           LTERM_NEL       },
    { __LTERM("UCSNL"),         LTERM_UCSNL     },
    { __LTERM("USER"),          LTERM_USER      }
#undef  __LTERM
    };

static void             sort_buffer_list(void);
static int              sort_buf_compare(void const *b1, void const *b2);

static BUFFERLIST_t     x_buffers;              /* buffer queue */

static IDENTIFIER_t     x_bufnum = 0;           /* buffer sequence number */

int                     x_imode = TRUE;         /* insert mode */


/*
 *  Function: buffer_init
 *      Runtime initialisation.
 */
void
buffer_init(void)
{
}


/*
 *  Function: buffer_shutdown
 *      Shutdown the buffer subsystem, releasing any system resources.
 */
void
buffer_shutdown(void)
{
    BUFFER_t *bp;

    while (NULL != (bp = buf_first())) {
        buf_kill(bp->b_bufnum);
    }
}


/*
 *  Function: buf_first
 *      Return first buffer within the buffer list.
 */
BUFFER_t *
buf_first(void)
{
    return TAILQ_FIRST(&x_buffers);
}


/*
 *  Function: buf_next
 *      Return next buffer from specified within the buffer list.
 */
BUFFER_t *
buf_next(BUFFER_t *bp)
{
    if (bp) {
        assert(BUFFER_MAGIC == bp->b_magic);
        assert(BUFFER_MAGIC == bp->b_magic2);
        if (NULL != (bp = TAILQ_NEXT(bp, b_node))) {
            assert(BUFFER_MAGIC == bp->b_magic);
            assert(BUFFER_MAGIC == bp->b_magic2);
            return bp;
        }
    }
    return NULL;
}


/*
 *  Function: buf_prev
 *      Return previous buffer from the specified within buffer list.
 */
BUFFER_t *
buf_prev(BUFFER_t *bp)
{
    if (bp) {
        assert(BUFFER_MAGIC == bp->b_magic);
        assert(BUFFER_MAGIC == bp->b_magic2);
        if (NULL != (bp = TAILQ_PREV(bp, _BufferList, b_node))) {
            assert(BUFFER_MAGIC == bp->b_magic);
            assert(BUFFER_MAGIC == bp->b_magic2);
            return bp;
        }
    }
    return NULL;
}


/*
 *  Function: buf_last
 *      Return last buffer wi5hin the buffer list.
 */
BUFFER_t *
buf_last(void)
{
    return TAILQ_LAST(&x_buffers, _BufferList);
}


/*
 *  Function: buf_lookup
 *      Return buffer by number.
 */
BUFFER_t *
buf_lookup(int bufnum)
{
    const BUFFER_t *bp;

    for (bp = buf_first(); bp; bp = buf_next((BUFFER_t *)bp)) {
        if (bufnum == bp->b_bufnum) {
            return (BUFFER_t *)bp;
        }
    }
    return NULL;
}


BUFFER_t *
buf_lookupx(int bufnum, const char *caller)
{
    const BUFFER_t *bp;

    for (bp = buf_first(); bp; bp = buf_next((BUFFER_t *)bp)) {
        if (bufnum == bp->b_bufnum) {
            return (BUFFER_t *)bp;
        }
    }
    if (caller) ewprintf("%s: no such buffer", caller);
    return NULL;
}


/*
 *  Function: buf_argument
 *      Retrieve the buffer associated with the specified argument.
 */
BUFFER_t *
buf_argument(int argi)
{
    assert(argi > 0);
    if (isa_integer(argi)) {
        const int bufnum = get_xinteger(argi, 0);

        if (bufnum > 0) {
            BUFFER_t *bp;

            if (NULL == (bp = buf_lookup(bufnum))) {
                ewprintf("warning: invalid buffer identifier (%d)", bufnum);
            }
            return bp;
        }
    }
    return curbp;
}


/*
 *  Function: buf_iswritable
 *      Determine whether the buffer requires to be written.
 */
int
buf_isdirty(const BUFFER_t *bp)
{
    if (*bp->b_fname && bp->b_nummod &&
            !BFTST(bp, BF_SYSBUF) && !k_isscrap(bp)) {
        return TRUE;
    }
    return FALSE;
}


/*
 *  Function: buf_anycb
 *      Walk buffers, giving the user a chance to save.
 *
 *      Return TRUE if there are any changed buffers afterwards. Buffers that
 *      don't have an associated file don't count.
 *
 *  Returns:
 *      Return FALSE if there are no changed buffers.
 */
int
buf_anycb(void)
{
    int anycb = x_panycb;
    BUFFER_t *bp, *saved_curbp = curbp;
    char prompt[MAX_CMDLINE] = {0};
    char reply[4] = {0};
    int mods = 0;

    x_panycb = 0;
    for (bp = buf_first(); bp; bp = buf_next(bp)) {
        if (buf_isdirty(bp)) {
            if (BFTST(bp, BF_AUTOWRITE)) {      /* unconditonal write */
                set_curbp(bp);
                if (file_write(NULL, 0 /*TODO - WRITE_NOTRIGGER*/) < 0) {
                    set_curbp(saved_curbp);
                    return TRUE;
                }
            } else {
                ++mods;                         /* modified buffer count */
            }
        }
    }
    set_curbp(saved_curbp);

    if (0 == mods) {
        return FALSE;
    }

    if (anycb) {
        if ('y' == anycb) {
            return FALSE;                       /* yes */
        } else if ('w' != anycb) {
            anycb = 0;                          /* not write */
        }
    }

    if (!anycb) {
        (void) sprintf(prompt, "\001%d buffer%s not been saved. Exit [^y^n^w]? ",
                    mods, (1 == mods ? " has" : "s have"));
        ereply1(prompt, reply, sizeof(reply));
        eclear();

        if ('Y' == reply[0] || 'y' == reply[0]) {
            return FALSE;                       /* yes */
        }

        if ('W' != reply[0] && 'w' != reply[0]) {
            return TRUE;                        /* no */
        }
    }

    for (bp = buf_first(); bp; bp = buf_next(bp)) {
        if (buf_isdirty(bp)) {
            set_curbp(bp);
            if (file_write(NULL, 0 /*XXX - WRITE_NOTRIGGER*/) < 0) {
                set_curbp(saved_curbp);
                return TRUE;
            }
        }
    }

    set_curbp(saved_curbp);
    return FALSE;
}


/*
 *  Function: buf_find
 *      Search for a buffer, by name.
 *
 *  Returns:
 *      Buffer pointer, otherwise NULL.
 */
BUFFER_t *
buf_find(const char *name)
{
    return buf_find2(name, FALSE, NULL);
}


/*
 *  Function: buf_find_or_create
 *      Search for a buffer, by name.
 *
 *      If not found, create a buffer and put it in the list of all buffers.
 *
 *  Returns:
 *      Buffer pointer.
 */
BUFFER_t *
buf_find_or_create(const char *name)
{
    return buf_find2(name, TRUE, NULL);
}


/*
 *  Function: buf_find2
 *      Search for a buffer, by name.
 *
 *      If not found, and the "cflag" is TRUE, create a buffer and put it in the
 *      list of all buffers.
 *
 *  Returns:
 *      Buffer pointer, otherwise NULL.
 */
BUFFER_t *
buf_find2(const char *name, int cflag, const char *encoding)
{
    if (name && name[0]) {
        register const BUFFER_t *bp;

        for (bp = buf_first(); bp; bp = buf_next((BUFFER_t *)bp)) {
            if (0 == file_cmp(name, bp->b_fname)) {
                return (BUFFER_t *)bp;
            }
        }
        if (cflag) {
            return buf_create(name, encoding, FALSE);
        }
    }
    return NULL;
}


/*
 *  Function: buf_imode
 *      Return the buffer insert mode.
 */
int
buf_imode(const BUFFER_t *bp)
{
    if (bp && bp->b_imode >= 0) {
        return bp->b_imode;                     /* buffer specific */
    }
    return x_imode;                             /* global */
}


/*
 *  Function: buf_create
 *      Creates a new buffer structure and place into the buffer list,
 *      in alphabetical order.
 */
BUFFER_t *
buf_create(const char *name, const char *encoding, int aflag)
{
    register BUFFER_t *bp;
    uint16_t i;

    __CUNUSED(encoding)                         /* TODO */

    if (0 == x_bufnum) {
        TAILQ_INIT(&x_buffers);
    }

    if (NULL == (bp = chk_calloc(sizeof(BUFFER_t),1))) {
        ewprintf("Can't create buffer");
        return NULL;
    }

    bp->b_magic = BUFFER_MAGIC;
    bp->b_magic2 = BUFFER_MAGIC;
    bp->b_bufnum = ++x_bufnum;                  /* unique buffer number (>= 1) */
    bp->b_refs = 1;                             /* create/reference count */

    BFRST(bp, 0);
    BF2RST(bp, 0);
    BF3RST(bp, 0);
    if (aflag) {
        BF2SET(bp, BF2_ATTRIBUTES);             /* character attributes */
    }

    bp->b_termtype = LTERM_UNDEFINED;           /* default line terminator */
    bp->b_type = BFTYP_UNDEFINED;               /* system default */
    buf_encoding_set(bp, encoding);             /* buffer encoding */

    bp->b_attrcurrent = ATTR_NORMAL;
    bp->b_attrnormal = ATTR_NORMAL;

    bp->b_imode = -1;                           /* non-localised insert-mode */

    bp->b_fname = file_canonicalize(name, NULL, 0);
#if defined(S_IRUSR) && !defined(__MINGW32__)
    bp->b_mode = (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) & ~x_umask;
#else
    bp->b_mode = (0666) & ~x_umask;
#endif
    bp->b_uid = (uid_t) -1;
    bp->b_gid = (gid_t) -1;

    bp->b_line = bp->b_col = 1;
    bp->b_numlines = 0;                         /* NEWLINE */

    bp->b_maxlength = 0;
    bp->b_maxlinep = NULL;

    /* ruler and tabs settings */
    if (tabchar_get()) {
        BFSET(bp, BF_TABS);
    }
    for (i = 0; i < BUFFER_NTABS && i < 3; ++i) {
        bp->b_tabs[i] = (i + 1) * 8;            /* default ruler, 9, 17 */
    }
    bp->b_tabs[i] = -1;
    bp->b_indent = 0;

    /* line list */
    TAILQ_INIT(&bp->b_lineq);                   /* NEWLINE */

    /* enqueue the buffer */
    TAILQ_INSERT_TAIL(&x_buffers, bp, b_node);  /* queue */
    sort_buffer_list();                         /* maintain buffer list in alphabetical order */

    /* buffer resources */
    chunk_attach(bp);
    file_attach(bp);
    anchor_attach(bp);
    hilite_attach(bp);
    sym_attach(bp);
    register_attach(bp);

    return bp;
}


/*
 *  Function: buf_name
 *      Name and/or rename the buffer.
 *
 *  Parameters:
 *      bp - Buffer reference.
 *      fname - New buffer name; the buffer assumes ownership.
 */
int
buf_name(BUFFER_t *bp, char *fname)
{
    assert(bp);
    assert(fname);
    if (bp && fname) {
        if (NULL == bp->b_fname || 0 != strcmp(fname, bp->b_fname)) {
            chk_free((void *)bp->b_fname);
            bp->b_fname = fname;
            sort_buffer_list();
            return TRUE;
        }
    }
    chk_free(fname);
    return FALSE;
}


/*
 *  Function: buf_type_set
 *      Set the buffer base type.
 *
 *  Parameters:
 *      bp - Buffer reference.
 *      type - Base buffer type.
 */
void
buf_type_set(BUFFER_t *bp, int type)
{
    assert(bp);
    if (bp && type != bp->b_type) {
        switch (type) {
        case BFTYP_DOS:     /* CR/LF */
        case BFTYP_MAC:     /* CR */
            file_terminator_set(bp, NULL, 0, LTERM_UNDEFINED);
            BFSET(bp, BF_CR_MODE);
            break;
        case BFTYP_UNIX:    /* LF */
        case BFTYP_ANSI:    /* NEL */
            file_terminator_set(bp, NULL, 0, LTERM_UNDEFINED);
            BFCLR(bp, BF_CR_MODE);
            break;
        case BFTYP_BINARY:  /* <none> */
            file_terminator_set(bp, NULL, 0, LTERM_NONE);
            BFCLR(bp, BF_CR_MODE);
            break;
        }
        bp->b_type = (BUFTYPE_t)type;
    }
}


/*
 *  Function: buf_encoding_set
 *      Set the buffer character encoding.
 *
 *  Parameters:
 *      bp - Buffer reference.
 *      encoding - Encoding name or alias.
 */
void
buf_encoding_set(BUFFER_t *bp, const char *encoding)
{
    assert(bp);
    if (bp) {
        BUFTYPE_t type = bp->b_type;

        if (bp->b_encoding) {                   /* replace existing */
            chk_free((void *)bp->b_encoding);
            bp->b_encoding = NULL;
            bp->b_endian = -1;
        }

        if (bp->b_iconv) {
            mchar_iconv_close(bp->b_iconv);
            bp->b_iconv = NULL;
        }

        if (encoding && *encoding) {
            mcharcharsetinfo_t info = {0};
            const char *native_encoding = NULL;

                                                /* verify buffer-type against encoding */
            if (mchar_info(&info, encoding, -1)) {
                if (BFTYP_UNDEFINED == type || BFTYP_UNKNOWN == type) {
                    if (BFTYP_SBCS == (type = (BUFTYPE_t)info.cs_type)) {
                        type = BFTYP_DEFAULT;
                    }                           /* incompatible, convert */
                } else if (buf_type_base(type) != info.cs_type) {
                    type = (BUFTYPE_t)info.cs_type;
                }
                native_encoding = info.cs_name;

            } else {
                if (BFTYP_UNDEFINED == type) {
                    type = BFTYP_UNKNOWN;
                }
            }

            encoding = bp->b_encoding =         /* assign encoding */
                (encoding && *encoding ? chk_salloc(encoding) : NULL);
            bp->b_endian = (int16_t)mchar_guess_endian(native_encoding ? native_encoding : encoding);
            bp->b_iconv = mchar_iconv_open(native_encoding ? native_encoding : encoding);

        } else {
            if (BFTYP_UNDEFINED == type) {
                type = BFTYP_DEFAULT;
            }
            bp->b_endian = -1;
            bp->b_iconv = mchar_iconv_open(NULL);
        }

        bp->b_type = type;
    }
}


/*
 *  Function: buf_type_default
 *      Apply default buffer encoding logic.
 *
 *  Parameters:
 *      bp - Buffer reference.
 */
void
buf_type_default(BUFFER_t *bp)
{
    assert(bp);
    if (bp) {
        const char *encoding = NULL;
        BUFTYPE_t type = bp->b_type;

        /*
         *  apply buffer-type, allowing terminator selection
         */
        if (BFTYP_UNDEFINED == type || BFTYP_UNKNOWN == type) {
            type = (BUFTYPE_t)x_bftype_default; /* default buffer type */
            if (BFTYP_UNDEFINED == type) {
                type = BFTYP_DEFAULT;
            }
        }
        buf_type_set(bp, type);

        /*
         *  apply encoding
         */
        if (NULL == (encoding = x_encoding_default)) {
            encoding = buf_type_encoding(type);
        }
        buf_encoding_set(bp, encoding);
    }
}


/*
 *  Function: buf_mined
 *      Update the buffer and associated window mined regions.
 *
 *  Parameters:
 *      start - Start of changed region.
 *      end - End of changed region.
 *
 *  Returns:
 *      nothing
 */
void
buf_mined(BUFFER_t *bp, LINENO start, LINENO end)
{
    if (bp) {
        if (0 == bp->b_syntax_min) {
            bp->b_syntax_min = start;
            bp->b_syntax_max = end;
        } else {
            if (start < bp->b_syntax_min) {
                bp->b_syntax_min = start;
            }
            if (end > bp->b_syntax_max) {
                bp->b_syntax_max = end;
            }
        }
    }
}


/*
 *  Function: buf_kill
 *      Delete a buffer by number.
 */
int
buf_kill(int bufnum)
{
    BUFFER_t *bp;
    WINDOW_t *wp;

    if (NULL == (bp = buf_lookup(bufnum)) ||
            --bp->b_refs > 0) {                 /* dereference the buffer */
        return FALSE;
    }

    triggerx(REG_BUFFER_DELETE, "%d", bufnum);

    pty_cleanup(bp);                            /* attached processes */
    file_cleanup(bp);                           /* file locks */
    key_local_detach(bp);                       /* local keyboards */
    buf_clear(bp);                              /* buffer text */

    TAILQ_REMOVE(&x_buffers, bp, b_node);       /* dequeue */

    if (curbp == bp || NULL == curbp) {         /* removing current, replace */
        for (curbp = buf_first(); curbp; curbp = buf_next(curbp)) {
            if (!BFTST(curbp, BF_SYSBUF)) {
                break;                          /* first non-system buffer */
            }
        }
        if (NULL == curbp) {
             curbp = buf_first();
        }
        assert(curbp != bp);
        set_hooked();
    }

    /* Unhook buffer from attached windows */
    if (bp->b_nwnd > 0) {
        for (wp = window_first(); wp && bp->b_nwnd; wp = window_next(wp)) {
            if (wp->w_bufp == bp) {
                if (NULL != (wp->w_bufp = curbp)) {
                    ++curbp->b_nwnd;
                }
                --bp->b_nwnd;
            }
        }
    }

    /* Release buffer resources */
    mchar_iconv_close(bp->b_iconv);
    chk_free((void *)bp->b_title);
    chk_free((void *)bp->b_fname);
    chk_free((void *)bp->b_ruler);
    register_detach(bp);
    anchor_detach(bp);
    hilite_detach(bp);
    sym_detach(bp);
    chunk_detach(bp);
    set_hooked();

    bp->b_magic = ~BUFFER_MAGIC;
    bp->b_magic2 = ~BUFFER_MAGIC;
    chk_free(bp);

    triggerx(REG_BUFFER_DELETE, "0");
    return TRUE;
}


/*
 *  Function: sort_buffer_list
 *      Sorts the buffer list, keeps things in alphabetical
 *      order; invoked during buffer creation or name changes.
 */
static void
sort_buffer_list(void)
{
    register BUFFER_t *bp;
    BUFFER_t **array;
    int i, num = 0;

    for (bp = buf_first(); bp; bp = buf_next(bp)) {
        ++num;
    }

    array = (BUFFER_t **) chk_alloc(sizeof(BUFFER_t *) * num);
    for (i = 0, bp = buf_first(); bp; bp = buf_next(bp)) {
        array[i++] = bp;
    }
    qsort(array, (size_t)num, sizeof(BUFFER_t *), sort_buf_compare);

    TAILQ_INIT(&x_buffers);
    for (i = 0; i < num; ++i) {
        TAILQ_INSERT_TAIL(&x_buffers, array[i], b_node);
    }

    chk_free(array);
}


/*
 *  Function: sort_buf_compare
 *      Buffer comparison, sorted according to the filename
 *      component; ignoring the directory prefix.
 *
 *  TODO - strverscmp() style ordering
 */
static int
sort_buf_compare(void const *b1, void const *b2)
{
    char const *f1 = (*(BUFFER_t const **) b1)->b_fname;
    char const *f2 = (*(BUFFER_t const **) b2)->b_fname;

    return file_cmp(sys_basename(f1), sys_basename(f2));
}


/*
 *  Function: buf_clear
 *      Clear a buffer of all contents.
 */
void
buf_clear(BUFFER_t *bp)
{
    BFCLR(bp, BF_CHANGED);
    BFCLR(bp, BF_RDONLY);
    while (bp->b_numlines > 0) {                /* NEWLINE */
        lremove(bp, 1);
    }
    bp->b_line = bp->b_col = 1;
    bp->b_numlines = 0;

    /* Clear all anchors */
    anchor_zap(bp);
    chunk_zap(bp);
}


/*
 *  Function: buf_show
 *      Display the given buffer in the given window.
 */
void
buf_show(BUFFER_t *bp, WINDOW_t *wp)
{
    WINDOW_t *owp;

    if (NULL == wp)
        return;

    if (wp->w_bufp == bp) {
        wp->w_status |= WFHARD;
        return;
    }

    detach_buffer(wp);
    wp->w_bufp = bp;
    wp->w_old_line = 1;
    wp->w_old_col = 1;
    wp->w_eol_col = 0;
    window_title(wp, sys_basename(bp->b_fname), "");

    if (0 == bp->b_nwnd++) {
        set_window_parms(wp, bp);
        return;
    }
    wp->w_status |= WFHARD;

    /* already on screen, steal values from other window */
    for (owp = window_first(); owp; owp = window_next(owp))
        if (owp->w_bufp == bp && owp != wp)  {
            wp->w_top_line = owp->w_top_line;
            wp->w_old_line = owp->w_old_line;
            if ((wp->w_line = owp->w_line) < 1) {
                wp->w_line = 1;
            }
            if ((wp->w_col = owp->w_col) < 1) {
                wp->w_col = 1;
            }
            return;
        }

    /*
     *  No other window has buffer on view so just try and keep the current
     *  line somewhere in the middle of the window
     */
    wp->w_col = 1;
    if ((wp->w_line = bp->b_line) < 1) {
        wp->w_line = 1;
    }
    window_center_line(wp, wp->w_line);
}


/*
 *  Function: buf_line_length
 *      Determine the length of the longest line within the specified buffer.
 *
 *  Parameters:
 *      bp - Buffer reference.
 *      marked - If TRUE only the marked region within the
 *          buffer is considered, otherwise the entire content.
 *
 *  Returns:
 *      Length of the line in characters.
 */
int
buf_line_length(const BUFFER_t *bp, /*__CBOOL*/ int marked)
{
    BUFFER_t *saved_curbp = curbp;
    LINENO wline = 0, woldline = 0;
    LINE_t *maxlinep = 0;
    LINENO maxcol = 0;
    ANCHOR_t a;
    LINENO line, col;

    ED_TRACE(("buf_line_length(bp:%p, marked:%d)\n", bp, marked))

    if (! marked && bp->b_maxlinep) {           /* cache */
        return bp->b_maxlength;
    }

    if (curwp) {                                /* don't reposition buffer in current window */
        wline = curwp->w_line;
        woldline = curwp->w_old_line;
    }

    set_curbp((BUFFER_t *)bp);
    if (! marked || FALSE == anchor_get(NULL, NULL, &a)) {
        a.start_line = 1;
        a.end_line = curbp->b_numlines;         /* NEWLINE */
    }

    for (line = a.start_line; line <= a.end_line; ++line) {
        if ((col = line_column_eol(line)) > maxcol) {
            maxlinep = vm_lock_line2(line);
            maxcol = col;
        }
    }

    if (! marked) {                             /* cache */
        curbp->b_maxlinep = maxlinep;
        curbp->b_maxlength = maxcol;
    }

    set_curbp(saved_curbp);

    /* Make sure buffer is repositioned where it was. */
    if (curwp) {
        curwp->w_line = wline;
        curwp->w_old_line = woldline;
    }
    return maxcol;
}


/*
 *  Function: buf_change_window
 *      change the current window.
 *
 *  Parameters:
 *      wp - Window object address.
 *
 *  Returns:
 *      nothing
 */
void
buf_change_window(WINDOW_t *wp)
{
    set_buffer_parms(curbp, curwp);
    curwp->w_status |= WFHARD;
    wp->w_status |= WFHARD;
    set_curwpbp(wp, wp->w_bufp);
}


/*
 *  Function: buf_type_desc
 *      Retrieve the buffer type description, if any, otherwise the specified default 'def'.
 */
const char *
buf_type_desc(int type, const char *def)
{
    if (type >= 0) {
        unsigned idx;

        for (idx = 0; idx < X_BFTYPES; ++idx) {
            if ((int)x_bftypes[idx].type == type) {
                return x_bftypes[idx].name;
            }
        }
    }
    return def;
}


/*
 *  Function: buf_type_encoding
 *      Return default buffer encoding, if any, otherwise NULL.
 */
const char *
buf_type_encoding(int type)
{
    if (type >= 0) {
        unsigned idx;

        for (idx = 0; idx < X_BFTYPES; ++idx) {
            if ((int)x_bftypes[idx].type == type) {
                return x_bftypes[idx].encoding;
            }
        }
    }
    return NULL;
}


/*
 *  Function: buf_type_base
 *      Determine the buffer base type.
 */
int
buf_type_base(int type)
{
    switch (type) {
    case BFTYP_UNIX:
    case BFTYP_DOS:
    case BFTYP_MAC:
    case BFTYP_ANSI:
        return BFTYP_SBCS;
    }
    return type;
}


/*
 *  Function: buf_termtype_desc
 *      Retrieve the buffer line termination type description, if any, otherwise the specified default 'def'.
 */
const char *
buf_termtype_desc(int type, const char *def)
{
    if (type >= 0) {
        unsigned idx;

        for (idx = 0; idx < X_LTERMS; ++idx) {
            if (x_lterms[idx].type == type) {
                return x_lterms[idx].name;
            }
        }
    }
    return def;
}


void
set_window_parms(WINDOW_t *wp, const BUFFER_t *bp)
{
    wp->w_status |= WFHARD;
    wp->w_top_line = bp->b_top;
    wp->w_line = bp->b_line;
    wp->w_col = bp->b_col;
    if (wp->w_line - wp->w_top_line >= wp->w_h) {
        wp->w_top_line = wp->w_line;
    }
}


void
set_buffer_parms(BUFFER_t *bp, const WINDOW_t *wp)
{
    if ((bp->b_line = wp->w_line) < 1) {
        bp->b_line = 1;
    }
    if ((bp->b_col = wp->w_col) < 1) {
        bp->b_col = 1;
    }
    bp->b_top = wp->w_top_line;
}
/*end*/
