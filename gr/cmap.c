#include <edidentifier.h>
__CIDENT_RCSID(gr_cmap_c,"$Id: cmap.c,v 1.33 2021/06/10 06:13:01 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: cmap.c,v 1.33 2021/06/10 06:13:01 cvsuser Exp $
 * Character map management/primitives.
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

#include <editor.h>
#include <edalt.h>
#include <edhandles.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"                              /* acc_...() */
#include "asciidefs.h"                          /* ASCIIDEF_... */
#include "buffer.h"                             /* buf_...() */
#include "builtin.h"
#include "cmap.h"
#include "debug.h"
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "lisp.h"
#include "main.h"
#include "mchar.h"                              /* mchar_...() */
#include "symbol.h"
#include "window.h"
#include "word.h"

const cmap_t *          x_base_cmap = NULL;     /* System base character-map */
const cmap_t *          x_binary_cmap = NULL;   /* Default for BINARY buffers */
const cmap_t *          x_default_cmap = NULL;  /* Default character-map when non explicitly defined */

const cmap_t *          cur_cmap = NULL;        /* Current char map for the current buffer/window */

static TAILQ_HEAD(CMAPList, _cmap)
                        x_cmapq;                /* Queue head */

static int              x_cmapid = 0;           /* Character map */

static vbyte_t          x_graphicspecials[] = {
    CH_HORIZONTAL,
    CH_VERTICAL,
    CH_TOP_LEFT,
    CH_TOP_RIGHT,
    CH_BOT_LEFT,
    CH_BOT_RIGHT,
    CH_TOP_JOIN,
    CH_BOT_JOIN,
    CH_LEFT_JOIN,
    CH_RIGHT_JOIN,
    CH_CROSS,
    CH_VSCROLL,
    CH_VTHUMB,
    CH_HSCROLL,
    CH_HTHUMB,
    0
    };

static void             cmap_build(cmap_t *base);
static cmap_t *         cmap_alloc(const char *name, unsigned size);
static cmap_t *         cmap_find_id(int id);
static cmap_t *         cmap_find_name(const char *name);

static __CINLINE cmapchr_t *cmapchr_get(cmap_t *cmap, unsigned ch);

static void             cmapchr_copy(cmapchr_t *dst, const cmapchr_t *src);
static int              cmapchr_set(cmap_t *cmap, unsigned ch, const char *sval, unsigned val);
static void             cmapchr_str(cmapchr_t *mc, const char *str);
static void             cmapchr_int(cmapchr_t *mc, unsigned val);


/*  Function:           cmap_init
 *      Initialise the default character-map's.
 *
 *  Notes:
 *      Control characters plus DEL are printed as ^x.
 *
 *      Characters with the top bit set get printed depending on whether
 *      the terminal can support printing the 8-bit character directly or not.
 *
 *      If it cannot, we print these characters in hex notation.
 *
 *  Parameters:
 *      ch -                Character value to be mapped.
 *
 *  Returns:
 *      Special character index, otherwise -1.
 */
void
cmap_init(void)
{
    const size_t map_size =                     /* map storage requirements */
            (32 * 3) + (95 * 2) + (1 * 3) + (128 * 5) + 1;
    cmap_t *cmap;

    /* "normal" character map */
    if (NULL == (cmap = (cmap_t *)x_base_cmap)) {
        cmap = cmap_alloc("normal", map_size);
        x_default_cmap = x_base_cmap = cmap;
    }

    cmap_build(cmap);
    cmap->cm_chars[ASCIIDEF_HT].mc_class  = CMAP_TAB;
    cmap->cm_chars[ASCIIDEF_ESC].mc_class = CMAP_ESCAPE;
    cmap->cm_chars[ASCIIDEF_BS].mc_class  = CMAP_BACKSPACE;

    /* "binary" character map */
    if (NULL == (cmap = (cmap_t *)x_binary_cmap)) {
        cmap = cmap_alloc("binary", map_size);
        x_binary_cmap = cmap;
    }
    cmap_build(cmap);
}


const cmapchr_t *
cmapchr_lookup(const cmap_t *cmap, vbyte_t ch)
{
    return cmapchr_get((cmap_t *)cmap, ch);
}


int
cmapchr_class(const cmap_t *cmap, vbyte_t ch)
{
    const cmapchr_t *mc = cmapchr_get((cmap_t *)cmap, ch);
    if (mc) return mc->mc_class;
    return 0;
}


int
cmapchr_width(const cmap_t *cmap, vbyte_t ch)
{
    const cmapchr_t *mc = cmapchr_get((cmap_t *)cmap, ch);
    if (mc) return mc->mc_width;
    return 1;
}


void
cmap_shutdown(void)
{
    if (x_cmapid) {
        cmap_t *cmap;

        while (NULL != (cmap = TAILQ_FIRST(&x_cmapq))) {
            assert(CMAP_MAGIC == cmap->cm_magic);
            assert(CMAP_MAGIC == cmap->cm_magic2);

            if (cmap->cm_buffer) {              /* single buffer */
                chk_free((void *)cmap->cm_buffer);

            } else {
                cmapchr_t *mc;
                unsigned idx;

                for (idx = 0, mc = cmap->cm_chars; idx < CMAP_CHARMAX; ++idx, ++mc) {
                    chk_free((void *)mc->mc_str);
                }

                for (idx = 0, mc = cmap->cm_specials; idx < CMAP_SPECIALS; ++idx, ++mc) {
                    chk_free((void *)mc->mc_str);
                }
            }

            TAILQ_REMOVE(&x_cmapq, cmap, cm_node);
            cmap->cm_magic = ~CMAP_MAGIC;
            cmap->cm_magic2 = ~CMAP_MAGIC;
            chk_free(cmap);
        }
    }
    x_base_cmap = NULL;
    x_binary_cmap = NULL;
    x_default_cmap = NULL;
}


static void
cmap_build(cmap_t *cmap)
{
    const int isutf8 = vtisutf8();              /* UTF8 support */
    const int is8bit = vtis8bit();              /* 8-bit support */
    const int unicode = (isutf8 && 0 == (DC_ASCIIONLY & x_display_ctrl));
    cmapchr_t *mc = NULL;
    unsigned char *cp;
    unsigned i;

    trace_log("cmap_build(ident:%d, is8bit:%d, unicode:%d=%d)\n", cmap->cm_ident, is8bit, isutf8, unicode);

    assert(cmap);
    assert(CMAP_MAGIC == cmap->cm_magic);
    assert(CMAP_MAGIC == cmap->cm_magic2);

    /*
     *  (re)initialise
     *
     *    o control-characters - ^[A-Z] in ASCII, otherwise use of
     *            UNICODE 0x2400-0x241F C0 character range.
     *    o DEL - ^? otherwise UNICODE 0x2421.
     *    o printable - all characters below 0x80, otherwise characters
     *            above if 8bit and not 0xff or UNICODE.
     *    o non-printable - other characters.
     */
    cp = (unsigned char *)cmap->cm_buffer;
    memset(cp, 0, cmap->cm_size);

    for (mc = cmap->cm_chars, i = 0; i < CMAP_CHARMAX; ++i, ++mc) {
        if (i < ' ') {                          /* NUL/Control characters */
            if (unicode) {
                cmapchr_int(mc, 0x2400 + i);    /* C0 characters */

            } else {
#if defined(DOSISH)         /* cmap */
                if (0 == i) {                   /* NUL, TODO - should be terminal/display specific */
#endif
                    *cp++ = '^';
                    *cp++ = (unsigned char)('@' + i);
                    *cp++ = 0;
                    cmapchr_str(mc, (const char *)(cp - 3));
#if defined(DOSISH)         /* cmap */
                }
#endif
            }

        } else if (0x7f == i) {                 /* DEL */
            if (unicode) {
                cmapchr_int(mc, 0x2421);        /* C0 Character */
            } else {
                *cp++ = '^';
                *cp++ = '?';
                *cp++ = 0;
                cmapchr_str(mc, (const char *)(cp - 3));
            }

        } else if (i < 0x80) {                  /* printable ascii */
            *cp++ = (unsigned char)i;
            *cp++ = 0;
            cmapchr_str(mc, (const char *)(cp - 2));

        } else if (isutf8 && i <= 0x9f) {       /* C1 - unicode */
            if (unicode) {
                cmapchr_int(mc, 0xfffd);        /* replacement character */
            } else {
                *cp++ = '?';
                *cp++ = 0;
                cmapchr_str(mc, (const char *)(cp - 2));
            }
                                                /* printable non-ascii/C1 characters */
        } else if (isutf8 || (is8bit && i < 0xff)) {
            *cp++ = (unsigned char)i;
            *cp++ = 0;
            cmapchr_str(mc, (const char *)(cp - 2));

        } else {                                /* non-printable */
            static const char hex_digits[] = "0123456789ABCDEF";

            *cp++ = '\\';
            *cp++ = 'x';
            *cp++ = hex_digits[(i >> 4) & 0x0f];
            *cp++ = hex_digits[ i       & 0x0f];
            *cp++ = 0;
            cmapchr_str(mc, (const char *)(cp - 5));
        }
    }

    /*
     *  specials
     */
    cmapchr_set(cmap, CMAP_TABSTART, NULL, ' ');
    cmapchr_set(cmap, CMAP_TABVIRTUAL, NULL, ' ');
    cmapchr_set(cmap, CMAP_TABEND, NULL, ' ');
    cmapchr_set(cmap, CMAP_EOL, NULL, ' ');
    cmapchr_set(cmap, CMAP_EOF, NULL, 0);

    /*
     *  line drawing/graphics and control characters
     */
    if (2 == sizeof(vbyte_t)) {                 /* small model only */
        for (i = 0; x_graphicspecials[i]; ++i) {
            const unsigned ch = x_graphicspecials[i];

            if (ch <= 0xff) {
                cmapchr_int(cmap->cm_chars + ch, ch);
            }
        }
    }
}


/*  Function:           cmap_specunicode
 *      Remap the special character to a unicode suitable value.
 *
 *      See the UNICODE standards documentation for additional details.
 *
 *  Parameters:
 *      ch -                Character value to be mapped.
 *
 *  Returns:
 *      Unicode value, otherwise 0.
 *
 *  Notes:
 *      0x256D          BOX DRAWING LIGHT ARC DOWN AND RIGHT.
 *      0x256E          BOX DRAWING LIGHT ARC DOWN AND LEFT.
 *      0x256F          BOX DRAWING LIGHT ARC UP AND LEFT.
 *      0x2570          BOX DRAWING LIGHT ARC UP AND RIGHT.
 *
 *      0x25EF  ( )     LARGE CIRCLE.
 *      0x25C9  (O)     FISH EYE.
 *
 *      0x25A1  [ ]     WHITE SQUARE.
 *      0x25A3  [x]     WHITE SQUARE CONTAINING SMALL BLACK SQUARE.
 *
 *  Other possible characters:
 *
 *      0x2297  (x)     CIRCLED TIMES.
 *      0x229B  (*)     CIRCLED ASTERISK.
 *      0x1F518 (o)     RADIO BUTTON.
 *      0x25CE  (O)     BULL EYE.
 *
 *      0x2610  [ ]     BALLOT BOX.
 *      0x2611  [v]     BALLOT BOX WITH CHECK/TICK.
 *      0x2612  [x]     BALLOT BOX WITH X.
 */
vbyte_t
cmap_specunicode(vbyte_t ch)
{
    ch &= ~VBYTE_ATTR_MASK;
    switch (ch) {
    case CH_VSCROLL:    ch = 0x2591; break;
    case CH_VTHUMB:     ch = 0x2593; break;
    case CH_HSCROLL:    ch = 0x2591; break;
    case CH_HTHUMB:     ch = 0x2593; break;
    case CH_HORIZONTAL: ch = 0x2500; break;
    case CH_VERTICAL:   ch = 0x2502; break;
    case CH_TOP_LEFT:   ch = 0x250c; break;
    case CH_TOP_RIGHT:  ch = 0x2510; break;
    case CH_TOP_JOIN:   ch = 0x252c; break;
    case CH_BOT_LEFT:   ch = 0x2514; break;
    case CH_BOT_RIGHT:  ch = 0x2518; break;
    case CH_BOT_JOIN:   ch = 0x2534; break;
    case CH_LEFT_JOIN:  ch = 0x2524; break;
    case CH_RIGHT_JOIN: ch = 0x251c; break;
    case CH_CROSS:      ch = 0x253c; break;
        break;

#if defined(WIN32) || defined(__CYGWIN__) || defined(linux)
    case CH_TOP_LEFT2:  ch = 0x250c; break;
    case CH_TOP_RIGHT2: ch = 0x2510; break;
    case CH_BOT_LEFT2:  ch = 0x2514; break;
    case CH_BOT_RIGHT2: ch = 0x2518; break;
#else
    case CH_TOP_LEFT2:  ch = 0x256D; break;
    case CH_TOP_RIGHT2: ch = 0x256E; break;
    case CH_BOT_LEFT2:  ch = 0x2570; break;
    case CH_BOT_RIGHT2: ch = 0x256F; break;
#endif

    case CH_RADIO_OFF:  ch = 0x25CB; break;
    case CH_RADIO_ON:   ch = 0x25C9; break;
    case CH_CHECK_OFF:  ch = 0x25A1; break;
    case CH_CHECK_ON:   ch = 0x25A3; break;
    case CH_CHECK_TRI:  ch = 0x2611; break;

    case CH_PADDING:    break;
    default:
        return 0;
    }

    return ch;
}


static cmap_t *
cmap_alloc(const char *name, unsigned size)
{
    cmap_t *cmap = (cmap_t *) chk_calloc(sizeof(cmap_t),1);

    if (cmap) {
        if (0 == x_cmapid) {
            TAILQ_INIT(&x_cmapq);
	    x_cmapid = GRBASE_CMAP;
        }
        memset((void *)cmap, 0, sizeof(cmap_t));
        cmap->cm_magic  = CMAP_MAGIC;
        cmap->cm_ident  = ++x_cmapid;
        if (name) {
            strncpy(cmap->cm_name, name, sizeof(cmap->cm_name));
        }
        cmap->cm_buffer = NULL;
        cmap->cm_cursor = NULL;
        if ((cmap->cm_size = size) > 0) {
            cmap->cm_buffer = chk_calloc(size, 1);
            cmap->cm_cursor = cmap->cm_buffer;
        }
        cmap->cm_lower  = 0;
        cmap->cm_upper  = 0xff;
        cmap->cm_range  = 0xff;
        cmap->cm_magic2 = CMAP_MAGIC;
        TAILQ_INSERT_TAIL(&x_cmapq, cmap, cm_node);
    }
    return cmap;
}


/*  Function:           cmap_find_id
 *      Function to find a character map given the id.
 *
 *  Parameters:
 *      id -                Identifier.
 *
 *  Returns:
 *      nothing.
 */
static cmap_t *
cmap_find_id(int id)
{
    if (x_cmapid && id >= 0) {
        cmap_t *cmap;

        for (cmap = TAILQ_FIRST(&x_cmapq); cmap; cmap = TAILQ_NEXT(cmap, cm_node)) {
            assert(CMAP_MAGIC == cmap->cm_magic);
            assert(CMAP_MAGIC == cmap->cm_magic2);
            if (cmap->cm_ident == id) {
                return cmap;
            }
        }
    }
    return NULL;
}


/*  Function:           cmap_find_name
 *      Function to find a character map given the name.
 *
 *  Parameters:
 *      name -              Character-map label.
 *
 *  Returns:
 *      nothing.
 */
static cmap_t *
cmap_find_name(const char *name)
{
    if (x_cmapid && name && *name) {
        cmap_t *cmap;

        for (cmap = TAILQ_FIRST(&x_cmapq); cmap; cmap = TAILQ_NEXT(cmap, cm_node)) {
            assert(CMAP_MAGIC == cmap->cm_magic);
            assert(CMAP_MAGIC == cmap->cm_magic2);
            if (cmap->cm_name[0] &&             /* XXX - case */
                    0 == strcmp(cmap->cm_name, name)) {
                return cmap;
            }
        }
    }
    return NULL;
}


static __CINLINE cmapchr_t *
cmapchr_get(cmap_t *cmap, unsigned ch)
{
    uint32_t lower;

    if (cmap) {     /*NULL for anon*/
        if (ch >= (lower = cmap->cm_lower) && ch <= cmap->cm_upper) {
            return cmap->cm_chars + (ch - lower);

        } else if (ch >= CMAP_SPECIALMIN && ch <= CMAP_SPECIALMAX) {
            return cmap->cm_specials + (ch - CMAP_SPECIALMIN);
        }
    }
    return NULL;
}


static void
cmapchr_copy(cmapchr_t *dst, const cmapchr_t *src)
{
    dst->mc_class   = src->mc_class;
    dst->mc_literal = src->mc_literal;
    dst->mc_length  = src->mc_length;
    dst->mc_width   = src->mc_width;
    if (NULL != (dst->mc_str = src->mc_str)) {
        dst->mc_str = chk_salloc(dst->mc_str);
    }
    dst->mc_chr     = src->mc_chr;
}


static int
cmapchr_set(cmap_t *cmap, unsigned ch, const char *sval, unsigned ival)
{
    cmapchr_t *mc = cmapchr_get(cmap, ch);

    if (mc) {
        if (sval) {
            cmapchr_str(mc, sval);
        } else {
            cmapchr_int(mc, ival);
        }
        return 0;
    }
    return -1;
}


static void
cmapchr_str(cmapchr_t *mc, const char *str)
{
    assert(strlen(str) < 255);
    mc->mc_class    = 0;
    mc->mc_literal  = 1;
    mc->mc_length   = (uint8_t)strlen(str);
    mc->mc_width    = mc->mc_length;
    mc->mc_str      = str;
    mc->mc_chr      = 0;
}


static void
cmapchr_int(cmapchr_t *mc, unsigned val)
{
    mc->mc_class    = 0;
    mc->mc_literal  = 1;
    mc->mc_length   = 0;
    mc->mc_width    = 1;
    mc->mc_str      = NULL;
    mc->mc_chr      = val;
}


/*  Function:           do_create_char_map
 *      create_char_map primitive, create a character-map for mapping the way
 *      characters appear on output.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: create_char_map - Create a display character-map.

        int
        create_char_map([int mapid|string name],
                [int start = 0], [list chars],
                [list flags], [string name])

    Macro Description:
        The 'create_char_map()' primitive either creates or updates
        an existing character-map. Character-map's determine the
        mapping between the buffer encoding, special purpose
        characters and their display representation.

        A map represents each of the 256 possible byte values which
        may be contained within any given 8-byte encoded buffer. In
        addition to the 256 base characters a number of special
        markers for End-Of-Line, Tabs and End-Of-File may also be
        defined.

        The identifier 'mapid' or the alias 'name' either references
        an existing map or if omitted a new map shall be generated.
        On creation a character-map is derived from the system
        "normal" map.

        Upon the existing or created map the specified 'chars' list
        is then applied starting at the character 'start', updating
        each character in sequence until the end of the list is
        encountered. Optionally the pairs of integer values contained
        within the 'flags' list are then applied in the same manor.
        If specified, the character-maps alias is changed to 'name'.

      *Character-map Names*

        By default two system predefined character-map's are
        available known by the names "normal" and "binary", these are
        in addition to the number managed by the view's package; care
        should be taken not to overwrite these resources prior to
        understanding the impact.

      *Special Characters*

        The follow special character indexes have importance when
        displaying buffer content and can be controlled using
        create_char_map for example.

>           create_char_map("literal", CMAP_EOF, quote_list("[EOF]"));

(start table, format=basic)
        [Constant       [Value      [Description                ]
        CMAP_TABSTART   -           Beginning of a Tab
        CMAP_TABVIRTUAL -           Tab body
        CMAP_TABEND     -           Tab end
        CMAP_EOL        -           End-of-line marker
        CMAP_EOF        -           End-of-file marker
(end table)

        By default, all specials are simple spaces as such invisible.

        Note, for CRiSPEdit compatibility the 'char' list may
        reference 257 characters with the 257th entry being
        equivalent to CMAP_EOL.

      *Character Flags*

        The 'flags' argument should be given as a set of one or more
        integer pairs:

>           { <character>, <flag> }

        The character flags available include, with one

(start table, format=basic)
        [Constant       [Value      [Description                ]
        CMAP_DEFAULT    0           Default
        CMAP_TAB        1           Horizontal tab
        CMAP_BACKSPACE  2           Backspace
        CMAP_ESCAPE     3           ESCAPE character
(end table)

      *Normal Character Map*

        The "normal" or default character-map is built using the rules
        below.

            o control-characters - '^[A-Z]' in ASCII, otherwise use of
                UNICODE 0x2400-0x241F C0 character range.

            o DEL - '^?' otherwise UNICODE 0x2421.

            o printable - all characters below 0x80 are themselves,
                otherwise characters above if 8bit and not 0xff or
                UNICODE.

            o non-printable - hexadecimal representation of the form
                '0x##'.

    Macro Parameters:
        mapid, name - Character-map reference, being either an
            integer map identifier or the associated map name as a
            string. If omitted a unique identifier shall be allocated.

        start - Optional integer, stating the first character index.

        chars - Character definition list, being a list of integer
            and/or string values for each character in the sequence
            starting at the offset 'start'.

        flags - Character flag list, being one or more pairs of integer
            values. Within each pair, The first element is the
            integer character position, and the second is the integer
            flag value from the set "Character Flags".

        name - Optional string, being the unique name by which the
            character-map may be referenced.

    Macro Returns:
        The 'create_char_map()' primitive returns the identifier of
        the resolved or created character-map otherwise -1 if the
        specified character map does not exist.

    Macro Example:
        The following enables an ASCII representation for control
        characters '0' to '31'.

>       int map =
>           create_char_map(0, NULL,
>                   quote_list(
>                   "<NUL>", "<SOH>", "<STX>", "<ETX>",
>                   "<EOT>", "<ENQ>", "<ACK>", "<BEL>",
>                   "<BS>",  "<HT>",  "<NL>",  "<VT>",
>                   "<FF>",  "<CR>",  "<SO>",  "<SI>",
>                   "<DLE>", "<DC1>", "<DC2>", "<DC3>",
>                   "<DC4>", "<NAK>", "<SYN>", "<ETB>",
>                   "<CAN>", "<EM>",  "<SUB>", "<ESC>",
>                   "<FS>",  "<GS>",  "<RS>",  "<US>"),
>               quote_list('\t', CMAP_TAB));
>       set_buffer_cmap(map);

    Macro Portability:
        n/a

    Macro See Also:
        inq_char_map
 */
void
do_create_char_map(void)        /* (int mapid, [int start = 0], [list chars], [list flags], [string name]) */
{
    const accint_t start = get_xinteger(2, 0);
    const LIST *lp;
    cmapchr_t *mc;
    cmap_t *cmap;

    /*
     *  Specified character-map, otherwise allocate a new image.
     */
    if (isa_undef(1)) {
        const char *name = get_xstr(5);

        if (name && *name) {
            if (NULL == (cmap = cmap_find_name(name))) {
                cmap = cmap_alloc(name, 0);
            }
        } else {
            cmap = cmap_alloc(name, 0);
        }

    } else {
        if (NULL == (cmap = cmap_find_id(get_xinteger(1, 0)))) {
            acc_assign_int(-1);
            return;
        }
    }

    /*
     *  Optional character list.
     */
    if (NULL != (lp = get_xlist(3))) {
        const LIST *nextlp;
        const char *sval = NULL;
        unsigned idx = (start >= 0 ? start : 0);
        accint_t ival;

        while ((nextlp = atom_next(lp)) != lp) {

            if (NULL != (mc = cmapchr_get(cmap, idx)) ||
                    (256 == idx && NULL != (mc = cmapchr_get(cmap, CMAP_EOL)))) {

                if (atom_xint(lp, &ival)) {
                    if (ival > 0) {
                        cmapchr_int(mc, ival);
                    }

                } else if (NULL != (sval = atom_xstr(lp))) {
                    if (0 == *sval) {           /* <space> */
                        cmapchr_int(mc, ' ');
                    } else {
                        cmapchr_str(mc, chk_salloc(sval));
                    }
                }

                trace_log("\tchar:%d/0x%x = %u/%s\n", /*ACCINT*/
                    idx, idx, (unsigned) ival, (sval ? sval : ""));
            }

            lp = nextlp;
            ++idx;
        }

    } else if (! isa_undef(3)) {                /* undocumented */
        const unsigned idx = (start >= 0 ? start : 0);

        if (NULL != (mc = cmapchr_get(cmap, idx))) {
            if (isa_integer(3)) {
                const int ival = get_integer(3);

                if (ival > 0) {
                    cmapchr_int(mc, ival);
                }

            } else if (isa_string(3)) {
                const char *sval = get_xstr(3);

                if (sval) {
                    if (0 == *sval) {           /* <space> */
                        cmapchr_int(mc, ' ');
                    } else {
                        cmapchr_str(mc, chk_salloc(sval));
                    }
                }
            }
        }
    }

    if (x_base_cmap) {                          /* inherit from base character-map */
        if (x_base_cmap != cmap) {
            const cmapchr_t *basemc;
            unsigned idx;
                                                /* character-set */
            for (idx = 0, mc = cmap->cm_chars, basemc = x_base_cmap->cm_chars;
                    idx < CMAP_CHARMAX; ++idx, ++mc, ++basemc) {
                if (NULL == mc->mc_str && 0 == mc->mc_chr) {
                    cmapchr_copy(mc, basemc);
                }
            }
                                                /* special-characters */
            for (idx = 0, mc = cmap->cm_specials, basemc = x_base_cmap->cm_specials;
                    idx < CMAP_SPECIALS; ++idx, ++mc, ++basemc) {
                if (NULL == mc->mc_str && 0 == mc->mc_chr) {
                    cmapchr_copy(mc, basemc);
                }
            }
        }
    }

    /*
     *  Optional flaglist, flags are a pair of numbers --
     *      the first is the character position, and the second is the flag value
     *
     *      CMAP_DEFAULT    =0,
     *      CMAP_TAB        =1,
     *      CMAP_BACKSPACE  =2,
     *      CMAP_ESCAPE     =3
     */
    if (NULL != (lp = get_xlist(4))) {
        const LIST *nextlp;

        while ((nextlp = atom_next(lp)) != lp) {
            accint_t ival;
            unsigned idx;

            if (! atom_xint(lp, &ival)) {       /* character position */
                ewprintf("%s: invalid flag index.", execute_name());
                break;
            }

            lp = nextlp;
            idx = (int) ival;
            if ((nextlp = atom_next(lp)) != lp) {
                if (!atom_xint(lp, &ival)) {    /* flag value */
                    ival = 0;
                }

                if (ival >= CMAP_DEFAULT && ival <= CMAP_ESCAPE) {
                    if (idx < CMAP_CHARMAX) {
                        cmap->cm_chars[idx].mc_class = (unsigned char)ival;
                    }
                }

                trace_log("\tflag:%d/0x%x = 0x%x/%x\n", idx, idx, (unsigned)ival, (unsigned)ival);
                lp = nextlp;
            }
        }
    }

    acc_assign_int((accint_t) cmap->cm_ident);   /* resolved/create character-map identifier */
}


/*  Function:           set_window_cmap
 *      set_window_cmap primitive, which attachs a character map to a window so
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_window_cmap - Set a windows character-map.

        int
        set_window_cmap([int mapid|string name], [int winnum])

    Macro Description:
        The 'set_window_cmap()' primitive attachs the specified
        character-map to a given window. A single character-map can
        be attached to any number of windows.

        By default two system predefined character-map's are
        available known by the names "normal" and "binary", these are
        in addition to the number managed by the view's package.

        Note that any buffer level association <set_buffer_cmap>
        shall have precedence over the windows view of a buffer.

    Macro Parameters:
        mapid, name - Character-map reference, being either an
            integer map identifier or the associated map name as a
            string. If omitted the default character-map shall be
            attached.

        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

    Macro Returns:
        The 'set_window_cmap()' primitive returns the identifier of the
        resolved character-map otherwise -1 if the specified character
        map does not exist.

    Macro Portability:
        n/a

    Macro See Also:
        create_char_map, set_buffer_cmap, inq_char_map
 */
void
do_set_window_cmap(void)        /* ([int mapid|string name], [int winnum]) */
{
    const cmap_t *cmap = x_default_cmap;        /* default */
    WINDOW_t *wp;

    if (!isa_undef(1)) {
        const char *name = get_xstr(1);

        if (name) {                             /* extension */
            if (NULL == (cmap = cmap_find_name(name))) {
                acc_assign_int(-1);
                return;
            }
        } else {
            if (NULL == (cmap = cmap_find_id(get_xinteger(1, 0)))) {
                acc_assign_int(-1);
                return;
            }
        }
    }

    if (isa_undef(2)) {                         /* no window, set 'default' character-map */
        x_default_cmap = cmap;
    } else {
        if (NULL != (wp = window_lookup(get_xinteger(2, 0)))) {
            wp->w_cmap = cmap;
            window_modify(wp, WFHARD);
        }
    }

    acc_assign_int((accint_t) (cmap ? cmap->cm_ident : 0));
}


/*  Function:           set_buffer_cmap
 *      set_buffer_cmap primitive, attach a character map to the specified buffer.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_buffer_cmap - Set a buffers character-map.

        int
        set_buffer_cmap([int mapid|string name], [int bufnum])

    Macro Description:
        The 'set_buffer_cmap()' primitive attachs the specified
        character-map to a given buffer. A single character-map can
        be attached to any number of buffers.

        Note that this association shall have precedence over the
        windows view of a buffer <set_window_cmap>.

    Macro Parameters:
        mapid, name - Character-map reference, being either an
            integer map identifier or the associated map name as a
            string. If omitted the default character-map shall be
            attached.
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        The 'set_buffer_cmap()' primitive returns the identifier of the
        resolved character-map otherwise -1 if the specified character
        map does not exist.

    Macro Portability:
        n/a

    Macro See Also:
        create_char_map, set_window_cmap, inq_char_map
 */
void
do_set_buffer_cmap(void)        /* (int mapid|string name, [int bufnum]) */
{
    BUFFER_t *bp = buf_argument(2);
    cmap_t *cmap = NULL;

    if (!isa_undef(1)) {
        const char *name = get_xstr(1);

        if (name) {                             /* extension */
            if (NULL == (cmap = cmap_find_name(name))) {
                acc_assign_int(-1);
                return;
            }

        } else {
            if (NULL == (cmap = cmap_find_id(get_xinteger(1, 0)))) {
                acc_assign_int(-1);
                return;
            }
        }
    }

    if (bp) {
        bp->b_cmap = cmap;
        acc_assign_int((accint_t)(cmap ? cmap->cm_ident : 0));
        window_harden();
        set_hooked();
    }
}


/*  Function:           inq_char_map
 *      inq_char_map primitive, return current character map for specified
 *      window otherwise the current window.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_char_map - Retrieve the character-map.

        int
        inq_char_map([int winnum], [string &name])

    Macro Description:
        The 'inq_char_map()' primitive retrieves the current
        character-map identifier of the underlying buffer, otherwise
        if none is associated with the specified window. If the
        window identifier is omitted, the current window shall be
        queried.

    Macro Parameters:
        winnum - Optional window identifier, if omitted the current
            window shall be referenced.

        name - Optional string referenced, if specified shall be
            populated with the assigned character-map name.

    Macro Returns:
        The 'inq_char_map()' primitive returns the associated
        character-mapid otherwise -1 if one is not assigned.

    Macro Portability:
        The 'name' parameter is a GRIEF extension.

    Macro See Also:
        create_char_map, set_window_cmap, set_buffer_cmap
 */
void
inq_char_map(void)              /* ([int winnum], [string &~name] */
{
    const cmap_t *cmap = NULL;
    WINDOW_t *wp;

    if (isa_undef(1)) {
        wp = curwp;                             /* current window */
    } else {
        wp = window_lookup(get_xinteger(1, -1));
    }

    if (wp) {                                   /* associated buffer, otherwise window */
        cmap = (wp->w_bufp && wp->w_bufp->b_cmap ?
                    wp->w_bufp->b_cmap : wp->w_cmap);
    } else {
        cmap = curbp->b_cmap;                   /* current buffer */
    }

    if (cmap) {
        argv_assign_str(2, cmap->cm_name);      /* extension */
        acc_assign_int(cmap->cm_ident);
    } else {
        argv_assign_str(2, "");
        acc_assign_int(-1);
    }
}


#if (TODO)
/*<<GRIEF-TODO>>
    Macro: get_char_map - Retrieve a character map

        list
        get_char_map(int mapid|string name)

    Macro Description:
        The 'get_char_map()' primitive retrieves the definition of the
        specified character map.

    Macro Parameters:
        mapid, name - Either the integer identifier or name of the
            character name to be returned.

    Macro Returns:
        Returns a list contains the referenced character map definition
        , being a list of string+integer pairs one for each character
        definition.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        get_char_map
 */
void
do_get_char_map(void)           /* list (int mapid|string name) */
{
    const cmap_t *cmap = x_default_cmap;        /* default */
    LIST *newlp = NULL;
    int length = -1;

    /*reference character-map */
    if (!isa_undef(1)) {
        const char *name = get_xstr(1);
        if (name) {                             /* extension */
            cmap = cmap_find_name(name);
        } else {
            cmap = cmap_find_id(get_xinteger(1, 0));
        }
    }

    /*build*/
    if (cmap) {
        int count = CMAP_CHARMAX + CMAP_SPECIALS;

        length = (count * (sizeof_atoms[F_INT] + sizeof_atoms[F_STR] + sizeof_atoms[F_INT])) +
                    sizeof_atoms[F_HALT];

        if (NULL != (newlp = lst_alloc(length, count))) {
            LIST *lp = newlp;
            const cmapchr_t *mc;
            unsigned idx;

            for (idx = 0, mc = cmap->cm_chars; idx < CMAP_CHARMAX; ++idx, ++mc) {
                lp = atom_push_int(lp, idx);
                if (mc->mc_str) {
                    lp = atom_push_str(lp, mc->mc_str);
                } else {
                    lp = atom_push_int(lp, mc->mc_chr);
                }
                lp = atom_push_int(lp, mc->mc_class);
                --count;
            }

            for (idx = 0, mc = cmap->cm_specials; idx < CMAP_SPECIALS; ++idx, ++mc) {
                lp = atom_push_int(lp, idx + CMAP_SPECIALMIN);
                if (mc->mc_str) {
                    lp = atom_push_str(lp, mc->mc_str);
                } else {
                    lp = atom_push_int(lp, mc->mc_chr);
                }
                lp = atom_push_int(lp, mc->mc_class);
                --count;
            }

            assert(0 == count);
        }
    }

    /*donate result*/
    if (newlp && length > 0) {
        acc_donate_list(newlp, length);
    } else {
        acc_assign_null();
    }
}


/*<<GRIEF-TODO>>
    Macro: get_char_map - Retrieve a buffers character map

        int
        set_char_map(int mapid|string name, list definition)

    Macro: get_char_map - Retrieve a character map

        list
        get_char_map(int mapid|string name)

    Macro Description:
        The 'get_char_map()' primitive retrieves the definition of the
        specified character map.

    Macro Parameters:
        mapid, name - Either the integer identifier or name of the
            character name to be returned.
        definition - Character map definition, being a list of
            string+integer pairs one for each character definition.

    Macro Returns:
        Returns 0 on success, otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        get_char_map
 */
void
do_set_char_map(void)           /* int (int mapid|string name, list definition) */
{
    cmap_t *cmap = NULL;
    const LIST *lp = get_xlist(2);

    /*reference character-map */
    if (!isa_undef(1)) {
        const char *name = get_xstr(1);
        if (name) {                             /* extension */
            cmap = cmap_find_name(name);
        } else {
            cmap = cmap_find_id(get_xinteger(1, 0));
        }
    }

    /*import*/
    if (cmap && lp) {
        cmapchr_t *mc;
        const LIST *nextlp;
        const char *sval;
        accint_t ival;

        while ((nextlp = atom_next(lp)) != lp) {

            if (atom_xint(lp, &ival)&&
                    NULL != (mc = cmapchr_get(cmap, ival))) {

                if ((nextlp = atom_next(lp = nextlp)) != lp) {

                    /* string or integer */
                    if (atom_xint(lp, &ival)) {
                        cmapchr_int(mc, ival);
                    } else if ((sval = atom_xstr(lp))) {
                        cmapchr_str(mc, sval);
                    } else {
                        break;
                    }

                    /*class*/
                    if ((nextlp = atom_next(lp = nextlp)) != lp &&
                            atom_xint(lp, &ival)) {
                        mc->mc_class = ival;
                        continue;
                    }
                }
            }
            break;  /*error*/
        }
    }
}
#endif

/*eof*/
