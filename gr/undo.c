#include <edidentifier.h>
__CIDENT_RCSID(gr_undo_c,"$Id: undo.c,v 1.46 2014/11/27 18:56:53 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: undo.c,v 1.46 2014/11/27 18:56:53 ayoung Exp $
 * undo and redo facilities.
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
#include <edfileio.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>                             /* str_...()/sxprintf() */

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

#include "undo.h"                               /* public header */

#include "anchor.h"                             /* anchor_...() */
#include "accum.h"
#include "builtin.h"
#include "cmap.h"
#include "debug.h"
#include "echo.h"
#include "eval.h"                               /* get_...() */
#include "file.h"
#include "keyboard.h"
#include "kill.h"
#include "line.h"
#include "mac1.h"
#include "main.h"
#include "map.h"
#include "playback.h"
#include "register.h"
#include "sysinfo.h"
#include "system.h"
#include "window.h"


/*
 *  undo op-codes
 */
enum undo_opcodes {
    UO_GOTO             = 0,                    /* Cursor modification. */
    UO_INSERT           = 1,
    UO_DELETE           = 2,
    UO_RAISE            = 3,
    UO_DROP             = 4,
    UO_ANCHOR           = 5,                    /* Anchor modification. */
    UO_SCRAP            = 6,                    /* Scrap modification. */
    UO_REPLACE          = 7,                    /* INSERT+DELETE, used for translate code. */
    UO_START            = 8,                    /* Beginning of an undo sequence. */
    UO_SOFTSTART        = 9,                    /* Marker for undoing translates. */
};


static const char *opcodes[] = {                /* See enum above */
    "GOTO",
    "INSERT",
    "DELETE",
    "RAISE",
    "DROP",
    "ANCHOR",
    "SCRAP",
    "REPLACE",
    "START",
    "SOFTSTART"
    };


/*
 *  undo record
 */
typedef struct {
    enum undo_opcodes   u_opcode;               /* Operation code. */
    LINENO              u_line;                 /* Line number. */
    LINENO              u_col;                  /* Column number. */
    LINENO              u_dot;                  /* Physical line offset. */
    FSIZE_t             u_length;               /* Modification length, in bytes. */
    FSIZE_t             u_last;                 /* Tell() position of previous undo. */
    int                 u_chain;                /* Non-zero if part of a single undo. */
    const cmap_t *      u_cmap;                 /* Character map. */
} undo_t;


/*
 *  File cache, org'ed into slices
 */
#define SLICE_SZE       1024                    /* Slice cach size */

#define SLICE_NUMBER    16                      /* Total slice number (^2 value needed) */

#define SLICE_OFF(off)  (off & ~(SLICE_SZE-1))  /* Round offset to slice boundary */

typedef struct slice {
    MAGIC_t             sl_magic;               /* Structure magic */
#define SLICE_MAGIC         MKMAGIC('N','n','D','o')
    int                 sl_ident;               /* block identifier */
    int                 sl_active;              /* TRUE when buffer holds a slice. */
    FSIZE_t             sl_offset;              /* offset into file. */
    char                sl_buf[SLICE_SZE];      /* slice buffer */
} undoslice_t;

static undoslice_t      u_slices[SLICE_NUMBER] = {0};


/*
 *  File information.
 */
static char             u_fname[MAX_PATH];      /* file image */

static int              u_fh = -1;              /* file handle */
static FILE *           u_fp = NULL;            /* file descriptor */

static FSIZE_t          u_offset = 4;           /* file cursor */

static int              u_need_seek = TRUE;     /* read has repositioned file, seek on next write */

static FSIZE_t          u_end_of_file = 4;      /* current end-of-file */


/*
 *  Engine status.
 */
static int              x_undo_scrap = FALSE;   /* quiet undo processing */

static enum undo_state {
    US_NORMAL,
    US_REDO,
    US_UNDO
} x_undo_state = US_NORMAL;


static int              undo_check(void);
static void             undo_debug(const undo_t *undo, const char *str);
static int              undo_command(undo_t *undo, int redo, UNDO_t *up, int pastwrite);
static void             undo_goto(const undo_t *undo);

static void             uwrite_op(undo_t *undo);
static void             uwrite_data(FSIZE_t offset, const char *buf, int len);
static void             uwrite_ctrl(FSIZE_t offset, undo_t *undo);

static int              slice_read_last(undo_t *undo, UNDO_t *up);
static int              slice_read(char *buf, int len, FSIZE_t offset);
static int              slice_write(const char *buf, int len, FSIZE_t offset);
static int              slice_rdwr(char *buf, int len, FSIZE_t offset, const int rdwr);
static undoslice_t *    slice_find(register FSIZE_t offset, const int rdwr);

static int              upwrite(const char *buf, int len, FSIZE_t offset);
static int              upread(char *buf, int len, FSIZE_t offset);


void
undo_init(void)
{
    undoslice_t *sp;
    const char *bundo;
    int sn, fd;

    for (sn = 0, sp = u_slices; sn < SLICE_NUMBER; ++sn, ++sp) {
        sp->sl_magic  = SLICE_MAGIC;
        sp->sl_ident  = sn;
        sp->sl_active = 0;
    }

    if (NULL == (bundo = ggetenv("GRUNDO"))) {  /* path override */
        bundo = sysinfo_tmpdir();
    }
    sxprintf(u_fname, sizeof(u_fname), "%s%sgrunXXXXXX", bundo, sys_delim());
#if defined(__WATCOMC__) || defined(_MSC_VER)   /* MSDOS */
#define FDMODE "w+b"
#else
#define FDMODE "w+"
#endif
    if (-1 == (fd = sys_mkstemp(u_fname)) ||
                (FILE *)NULL == (u_fp = fileio_fdopen(fd, FDMODE))) {
        printf("Cannot create undo file (%d):\n", fd);
        perror(u_fname);
        gr_exit(0);
    }
    u_fh = fileio_fileno(u_fp);
#if !defined(NO_UNLINK_OPEN)
    unlink(u_fname);
    u_fname[0] = 0;
#endif
    sys_noinherit(u_fh);
    fwrite("Undo", 4, 1, u_fp);                 /* ensure ftell(u_fp) != 0) */
}


void
undo_close(void)
{
    if (u_fp) {
        fclose(u_fp);
        if (u_fname[0]) {
            fileio_unlink(u_fname);
        }
        u_fname[0] = 0;
        u_fp = NULL;
    }
}


void
u_replace(const char *str, FSIZE_t del, FSIZE_t ins)
{
    undo_t undo = {0};

    if ((0 == del && 0 == ins) || undo_check()) {
        return;
    }

    ED_TRACEX(DB_UNDO, ("u_replace(ins:%d) = %d [", (int)ins, (int)del))
    ED_DATAX(DB_UNDO, (str, del, "]\n"))
    if (US_NORMAL == x_undo_state) {
        ++curbp->b_nummod;
    }
    undo.u_opcode = UO_REPLACE;
    undo.u_length = del;
    uwrite_op(&undo);
    uwrite_data(0, (const char *)&ins, sizeof(ins));
    uwrite_data(0, str, del);
}


FSIZE_t
u_insert(FSIZE_t length, int dot)
{
    undo_t undo = {0};
    const LINENO numlines = curbp->b_numlines;
    register LINENO cline = *cur_line;
    register LINE_t *lp;
    FSIZE_t n = length;

    if (0 == n || undo_check()) {
        return n;
    }
    ED_TRACEX(DB_UNDO, ("u_insert(size:%d, line:%d, offset:%d)\n", (int)n, (int)cline, (int)dot))
    assert(n > 0);

    if (US_NORMAL == x_undo_state) {
        ++curbp->b_nummod;
    }

    undo.u_opcode = UO_INSERT;
    undo.u_length = n;
    uwrite_op(&undo);

    lp = vm_lock_line(cline);
    while (n > 0 && cline <= numlines) {        /* NEWLINE */
        FSIZE_t x, w;

        x = llength(lp) - dot;
        w = x;
        if (x >= n) {
            w = n;
        }

        assert(dot <= (int)llength(lp));
        assert(dot + w <= (int)llength(lp));

        uwrite_data(0, (const char *)ltext(lp) + dot, (int)w);
        if ((n -= w) != 0) {
            uwrite_data(0, "\n", 1);
            --n;
        }
        dot = 0;

        ++cline;
        if (NULL == (lp = vm_lock_next(lp, cline))) {
            break;
        }
    }
    vm_unlock(cline);

    ED_TRACEX(DB_UNDO, ("==> length:%d\n", (int)(length - n)))
    return (length - n);
}


void
u_delete(FSIZE_t n)
{
    undo_t t_undo = {0}, undo = {0};
    UNDO_t *up;

    if (0 == n || undo_check()) {
        return;
    }
    ED_TRACEX(DB_UNDO, ("u_delete(line:%d col:%d size:%d)\n", (int)*cur_line, (int)*cur_col, (int)n))
    assert(n > 0);

    up = (x_undo_state == US_UNDO ? &curbp->b_redo : &curbp->b_undo);

    if (x_selfinsert && up->u_last) {
        /*
         *  compress undo actions of self_insert() into words
         */
        if (slice_read_last(&t_undo, up)) {
            goto normal;
        }

        if (UO_DELETE == t_undo.u_opcode) {
            const int32_t prevchar = curbp->b_uchar;

            ED_TRACEX(DB_UNDO, ("undo(COLLAPSING, %d + %d => %d)\n", \
                (int)t_undo.u_length, (int)n, (int)(t_undo.u_length + n)))

            if ((isalnum(prevchar) && isalnum(x_character)) ||
                (isspace(prevchar) && isspace(x_character)) ||
                    (0x1b == prevchar || 0x1b == x_character) ||
                (ispunct(prevchar) && ispunct(x_character)))
            {                                   /* MCHAR??? */
                ED_TRACEX(DB_UNDO, ("--> collapsed\n"))
                t_undo.u_length += n;
                uwrite_ctrl(up->u_last, &t_undo);
                return;
            }
        }
    }

normal:
    if (US_NORMAL == x_undo_state) {
        ++curbp->b_nummod;
    }
    undo.u_opcode = UO_DELETE;
    undo.u_length = n;
    uwrite_op(&undo);
}


void
u_chain(void)
{
    if (curbp) {
        if (PLAYBACK_PLAYING != playback_status()) {
            curbp->b_undo.u_chain = 0;
            curbp->b_redo.u_chain = 0;
        }
    }
}


void
u_terminate(void)
{
    undo_t undo = {0};

    if (undo_check()) {
        return;
    }
    ED_TRACEX(DB_UNDO, ("u_terminator()\n"))
    undo.u_opcode = UO_START;
    uwrite_op(&undo);
}


void
u_soft_start(void)
{
    undo_t undo = {0};

    if (undo_check()) {
        return;
    }
    ED_TRACEX(DB_UNDO, ("u_soft_start()\n"))
    undo.u_opcode = UO_SOFTSTART;
    uwrite_op(&undo);
}


void
u_dot(void)
{
    undo_t undo = {0};

    if (undo_check()) {
        return;
    }
    ED_TRACEX(DB_UNDO, ("u_dot(line:%d col:%d)\n", (int)*cur_line, (int)*cur_col))
    undo.u_opcode = UO_GOTO;
    uwrite_op(&undo);
}


void
u_raise(void)
{
    undo_t undo = {0};

    if (undo_check()) {
        return;
    }
    ED_TRACEX(DB_UNDO, ("u_raise(line:%d col:%d)\n", (int)*cur_line, (int)*cur_col))
    undo.u_opcode = UO_RAISE;
    uwrite_op(&undo);
}


void
u_drop(void)
{
    undo_t undo = {0};
    ANCHOR_t anchor = {0};

    if (undo_check() || NULL == curbp) {
        return;
    }
    ED_TRACEX(DB_UNDO, ("u_drop(line:%d col:%d)\n", (int)*cur_line, (int)*cur_col))
    anchor_read(curbp, &anchor);
    undo.u_opcode = UO_DROP;
    uwrite_op(&undo);
    uwrite_data(0, (const char *)&anchor, sizeof(ANCHOR_t));
}


void
u_anchor(void)
{
    undo_t undo = {0};
    ANCHOR_t anchor = {0};

    if (undo_check() || NULL == curbp || NULL == curbp->b_anchor) {
        return;
    }
    ED_TRACEX(DB_UNDO, ("u_anchor(line:%d col:%d)\n", (int)*cur_line, (int)*cur_col))
    anchor_read(curbp, &anchor);
    undo.u_opcode = UO_ANCHOR;
    uwrite_op(&undo);
    uwrite_data(0, (const char *)&anchor, sizeof(ANCHOR_t));
}


void
u_scrap(void)
{
    undo_t undo = {0};

    if (undo_check()) {
        return;
    }
    ED_TRACEX(DB_UNDO, ("u_scrap(line:%d col:%d)\n", (int)*cur_line, (int)*cur_col))
    undo.u_opcode = UO_SCRAP;
    uwrite_op(&undo);
}


/*
 *  Check to see whether we should be doing undo for this buffer
 */
static int
undo_check(void)
{
    if (BFTST(curbp, BF_NO_UNDO) || NULL == u_fp) {
        return TRUE;
    }

    /*
     *  If user isn't undoing an undo then
     *      terminate the redo chain so we don't confuse the user.
     */
    if (US_NORMAL == x_undo_state) {
        curbp->b_redo.u_last = 0;
    }
    return FALSE;
}


static void
undo_debug(const undo_t *undo, const char *str)
{
//#if defined(ED_TRACE) & (ED_TRACE >= 1)
    ED_TRACEX(DB_UNDO, ("%s(op:%d/%s, line:%d, col:%d, len:%ld, last=%08lx, chain:%d)\n", \
        str, undo->u_opcode, opcodes[(int) undo->u_opcode], \
        undo->u_line, undo->u_col, (long)undo->u_length, (long)undo->u_last, \
        undo->u_chain))
//#endif
    __CUNUSED(opcodes)
    __CUNUSED(undo)
    __CUNUSED(str)
}


/*  Function:           undo_command
 *      Undo the specified command.
 *
 *  Parameters:
 *      undo - Undo object;
 *      pastwrite - Past write_buffer marker logic.
 *
 *  Returns:
 *      0 on success otherwise -1 if nothing to undo.
 */
static int
undo_command(undo_t *undo, int redo, UNDO_t *up, int pastwrite)
{
    FSIZE_t saved_last, pos;
    char buf[64];
    int n;

    if (0 == up->u_last) {
        ewprintf("Nothing to %s.", redo ? "redo" : "undo");
        if (BFTST(curbp, BF_CHANGED)) {
            BFCLR(curbp, BF_CHANGED);           /* change attribute */
            trigger(REG_BUFFER_MOD);            /* buffer has been unmodified */
        }
        return -1;
    }

    if (slice_read_last(undo, up)) {
        return -1;
    }

    pos = up->u_last + sizeof(*undo);
    undo_debug(undo, "UNDOING");
    saved_last = up->u_last;

    up->u_last = undo->u_last;

    /*
     *  Restore the character map, otherwise converting to and from column positions may
     *  cause problems if we're now viewing the buffer in a different mode.
     */
    cur_cmap = undo->u_cmap;

    switch (undo->u_opcode) {
    case UO_REPLACE: {
            FSIZE_t l;

            slice_read((void *)&l, sizeof(l), pos);
            pos += sizeof(l);
            undo_goto(undo);
            ldelete(l);
        }
        /*FALLTHRU*/

    case UO_INSERT:
        if (curbp->b_nummod) {
            --curbp->b_nummod;
        }
        undo_goto(undo);
        while (undo->u_length > 0) {
            n = undo->u_length;
            if ((size_t) n > sizeof(buf)) {
                n = sizeof(buf);
            }
            slice_read(buf, n, pos);
            linserts(buf, n);
            pos += n;
            undo->u_length -= n;
        }
        if (US_UNDO == x_undo_state) {
            undo_goto(undo);
        }
        break;

    case UO_GOTO:
        undo_goto(undo);
        break;

    case UO_DELETE:
        if (curbp->b_nummod) {
            --curbp->b_nummod;
        }
        undo_goto(undo);
        ldelete(undo->u_length);
        break;

    case UO_RAISE:
        anchor_raise();
        break;

    case UO_DROP: {
            ANCHOR_t t_anchor;

            anchor_drop(MK_NORMAL);
            slice_read((void *)&t_anchor, sizeof(t_anchor), pos);
            anchor_write(curbp, &t_anchor);
        }
        break;

    case UO_ANCHOR: {
            ANCHOR_t t_anchor;

            slice_read((void *)&t_anchor, sizeof(t_anchor), pos);
            anchor_write(curbp, &t_anchor);
        }
        break;

    case UO_SCRAP:
        x_undo_scrap = TRUE;
        k_undo();
        x_undo_scrap = FALSE;
        break;

    case UO_START:
        if (0 == pastwrite ||                   /* disabled */
                (pastwrite < 0 && !eyorn("Undo past saved file mark"))) {
            up->u_last = saved_last;
            return -1;
        }
        break;

    case UO_SOFTSTART:
        /*
         *  Do nothing -- this is just a marker for the translate/undo option.
         */
        break;

    default:
        ewprintf("undo: opcode error");
    }
    return 0;
}


static void
undo_goto(const undo_t *undo)
{
    ED_TRACEX(DB_UNDO, ("\tundo_goto(line:%d, col:%d)\n", undo->u_line, undo->u_col))
    win_modify(WFMOVE);
    *cur_line = undo->u_line;
    *cur_col  = undo->u_col;
    win_modify(WFMOVE);
}


/*  Function:           uwrite_op
 *      Writes out an undo atom, setting up the backward link for the
 *      current undo item so we can find the previous item in the chain.
 *      Also stores the current line and column number (common to all
 *      undo operations).
 *
 *  Parameters:
 *      undo - undo object.
 *
 *  Returns:
 *      nothing.
 */
static void
uwrite_op(undo_t *undo)
{
    UNDO_t *up;

    if (US_UNDO == x_undo_state) {
        up = &curbp->b_redo;
    } else {
        up = &curbp->b_undo;
    }

    assert(*cur_line >= 1);
    assert(*cur_col  >= 1);

    undo->u_line  = *cur_line;
    undo->u_col	  = *cur_col;
    undo->u_last  = up->u_last;
    undo->u_chain = up->u_chain++;
    undo->u_cmap  = cur_cmap;			/*XXX - cmap identifier??*/
    up->u_last	  = u_end_of_file;

    undo_debug(undo, US_NORMAL == x_undo_state ? "normal" :
        (US_UNDO == x_undo_state ? "undo" : "redo"));
    uwrite_ctrl(0, undo);
}


/*  Function:           uwrite_ctrl
 *      Write a control buffer to the undo file. We try and cache the
 *      undo control buffers in memory because we may need to refer to
 *      them again.
 *
 *  Parameters:
 *      offset - Undo offset.
 *      undo - undo object.
 *
 *  Returns:
 *      nothing.
 */
static void
uwrite_ctrl(FSIZE_t offset, undo_t *undo)
{
    uwrite_data(offset, (const char *)undo, sizeof(*undo));
}


/*  Function:           uwrite_data
 *      Write arbitrary text data to undo file. Used when we delete text
 *      from buffer and we need to save the deleted text for an undo
 *      operation.
 *
 *  Parameters:
 *      offset - Undo offset.
 *      buf - Text buffer address.
 *      len - Size of the text buffer in bytes.
 *
 *  Returns:
 *      nothing.
 */
static void
uwrite_data(FSIZE_t offset, const char *buf, int len)
{
    if (0 == offset) {
        offset = u_end_of_file;
    }
    slice_write(buf, len, offset);
    if (offset == u_end_of_file) {
        u_end_of_file += len;
    }
}


/*  Function:           slice_read_last
 *      Read from the undo file, caching were possible; used for undo 
 *      compaction code.
 *
 *  Parameters:
 *      undo - undo object.
 *      up - undo definition.
 *
 *  Returns:
 *      0 on success, otherwise -1.
 */
static int
slice_read_last(undo_t *undo, UNDO_t *up)
{
    if (slice_read((void *)undo, sizeof(*undo), up->u_last) != sizeof(*undo)) {
        ewprintf("undo: I/O error");
        return -1;
    }
    return 0;
}


/*  Function:           slice_read
 *      Read from the undo file, caching were possible.
 *
 *  Parameters:
 *      buf - Buffer address.
 *      len - Length of the buffer, in bytes.
 *      offset - File offset.
 *
 *  Returns:
 *      0 on success, otherwise -1.
 */
static int
slice_read(char *buf, int len, FSIZE_t offset)
{
    return slice_rdwr(buf, len, offset, TRUE);
}


/*  Function:           slice_write
 *      Write to the undo file, caching were possible.
 *
 *  Parameters:
 *      buf - Buffer address.
 *      len - Length of the buffer, in bytes.
 *      offset - File offset.
 *
 *  Returns:
 *      0 on success, otherwise -1.
 */
static int
slice_write(const char *buf, int len, FSIZE_t offset)
{
    return slice_rdwr((char *)buf, len, offset, FALSE);
}


/*  Function:           slice_rdwr
 *      Common code for slice reading and writing.
 *
 *  Parameters:
 *      buf - Buffer address.
 *      len - Length of the buffer, in bytes.
 *      offset - File offset.
 *      rdwr - Direction (read = TRUE, write = FALSE)
 *
 *  Returns:
 *      0 on success, otherwise -1.
 */
static int
slice_rdwr(char *buf, int len, FSIZE_t offset, const int rdwr)
{
    register undoslice_t *sp;
    int n, start_len = len;
    char *cp;

    while (len > 0) {
        sp = slice_find(offset, rdwr);
        if (NULL == sp) {
            return -1;
        }

        cp = &sp->sl_buf[offset - sp->sl_offset];
        n = &sp->sl_buf[SLICE_SZE] - cp;
        if (n > len) {
            n  = len;
        }

        if (rdwr) {
            memcpy(buf, cp, n);
        } else {
            memcpy(cp, buf, n);
        }

        len -= n;
        buf += n;
        offset += n;
    }
    return start_len;
}


/*  Function:           slice_find
 *      Search the slice cache and return the associated buffer.
 *
 *      If not in memory, allocate the first free slice, otherwise flush
 *      an old slice to disk; for read requests re-populate the buffer
 *      with the disk image.
 *
 *  Parameters:
 *      offset - File offset.
 *      rdwr - Direction (read = TRUE, write = FALSE).
 *
 *  Returns:
 *      Slice address, otherwise NULL on error.
 */
static undoslice_t *
slice_find(FSIZE_t offset, const int rdwr)
{
    static undoslice_t *last_sp = u_slices;
    static undoslice_t *last_slice = u_slices;
    undoslice_t *sp;
    int sn;

    for (sn = 0, sp = last_sp; sn < SLICE_NUMBER; ++sn) {
        assert(SLICE_MAGIC == sp->sl_magic);

        if (sp->sl_active &&
                offset >= sp->sl_offset && offset < sp->sl_offset + SLICE_SZE) {
            last_sp = sp;
            return sp;
        }

        if (++sp >= &u_slices[SLICE_NUMBER]) {
            sp = u_slices;
        }
    }

    if (last_slice >= &u_slices[SLICE_NUMBER]) {
        last_slice = u_slices;
    }

    if (last_slice->sl_active) {
        if (upwrite(last_slice->sl_buf, SLICE_SZE, last_slice->sl_offset)) {
            return NULL;
        }
    }

    last_slice->sl_offset = SLICE_OFF(offset);
    last_slice->sl_active = TRUE;

    if (rdwr) {                                 /* repopulate buffer */
        upread(last_slice->sl_buf, SLICE_SZE, last_slice->sl_offset);
    }
    return last_slice++;
}


/*  Function:           upwrite
 *      Write data to undo file at a specified offset, seeking offset 
 *      if required.
 *
 *  Parameters:
 *      buf - Buffer address.
 *      len - Length of the buffer, in bytes.
 *      offset - File offset.
 *
 *  Returns:
 *      0 on success, otherwise -1.
 */
static int
upwrite(const char *buf, int len, FSIZE_t offset)
{
    if (u_offset != offset || u_need_seek) {
        (void) fseek(u_fp, offset, 0);
    }
    if (fwrite(buf, len, 1, u_fp) != 1) {
        return -1;
    }
    u_offset = offset + len;
    u_need_seek = FALSE;
    return 0;
}


/*  Function:           upread
 *      Read data from the undo file at a specified offset, seeking offset
 *      if required.
 *
 *  Parameters:
 *      buf - Buffer address.
 *      len - Length of the buffer, in bytes.
 *      offset - File offset.
 *
 *  Returns:
 *      0 on success, otherwise -1.
 */
static int
upread(char *buf, int len, FSIZE_t offset)
{
    int ret;

    if (offset != u_offset) {
        (void) fseek(u_fp, offset, 0);
        u_offset = offset;
    }
    ret = (int) fread(buf, len, 1, u_fp);
    u_offset += len;
    u_need_seek = TRUE;
    return ret;
}


/*  Function:           do_undo
 *      undo/redo primitive.
 *
 *  Parameters:
 *      mode - Undo mode (-1 = standard; 0 = scrap undo; 1 = translate)
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: undo - Undo previous edit operations.

        void
        undo([int move], [int pastwrite = -1], [int redo = FALSE])

    Macro Description:
        The 'undo()' primitive undoes buffer modifications on the current
        buffer.
        
        Executing without arguments undoes the last operation performed
        on the buffer, including cursor movement, any text modification
        and marked region modification. If the previous operation on the
        buffer was a macro, or a complex operation, e.g. global
        translate, then all the buffer modifications performed are
        undone in one step.

        The 'move' option limits the undo operation to the last
        buffer modification, restoring the cursor to the its location
        where the buffer was actually modified.

        Each buffer maintains their own undo stack, unless disabled 
        (see set_buffer_flags). Under BRIEF the undo stack for a
        particular buffer was cleared when the buffer is written to
        disk, as such it was not possible to undo any operations
        performed on the buffer before the last 'write_buffer'.

        Under Grief the undo stack is retained for the duration of the
        editor lifetime. If 'pastwrite' is specified as positive value, 
        the 'undo' shall perform undo's beyond the last write_buffer; by
        default the user is prompted as follows

>           Undo past saved file mark?

    Note!:
        Unlike BRIEF, it is possible to call 'undo' from within a
        macro, but this use is highly dubious. It is normally called
        by the user directly from one of the key assignments.

    Macro Parameters:
        move - Optional boolean value, if true then all buffer
            operations up the last buffer modification are undone.

        pastwrite - Option integer stating the write_buffer undo logic.
            Zero (0) disables undo's past the last write, a negative
            value (-1) prompts the user whether to continue otherwise
            and a postive value (1) permits undo's without prompt.

        redo - Optional boolean value, if true the previous undo action
            shall be undone.

    Macro Returns:
        The 'undo()' primitive returns the number of operations undo;
        this is Grief extension.

    Macro Portability:
        n/a

    Macro See Also:
        redo
 */
void
do_undo(int mode)              /* (int move, [int pastwrite = -1]. [int redo = FALSE]) */
{
    const int move = (-1 == mode ? get_xinteger(1, 0) : 0);
    const int past = (-1 == mode ? get_xinteger(2, -1) : -1);
    const int redo = (-1 == mode ? get_xinteger(3, FALSE) : FALSE);
    int ret = 0;
    
    enum undo_state s_undo_state = x_undo_state;

    UNDO_t *up = (redo ? &curbp->b_redo : &curbp->b_undo);
    undo_t  undo = {0};
    FSIZE_t num = -1;

    ED_TRACEX(DB_UNDO, (redo ? "doing redo\n" : "doing undo\n"))

    x_undo_state = (redo ? US_REDO : US_UNDO);
    do {
        (void) memset(&undo, 0, sizeof(undo));
        if (-1 == undo_command(&undo, redo, up, past)) {
            x_undo_state = US_NORMAL;
            if (-1 == mode) {
                acc_assign_int(ret);
            }
            return;
        }

        ++ret;

        if (num < 0) {
            num = undo.u_chain;
        } else if (num && undo.u_chain) {
            percentage(PERCENTAGE_BYTES, num - undo.u_chain, num, "Undoing", "command");
        }

        if (move &&
             (UO_INSERT != undo.u_opcode &&
              UO_DELETE != undo.u_opcode)) {
            undo.u_chain = 1;
        }

        if (mode > 0 && UO_SOFTSTART == undo.u_opcode) {
            break;                              /* soft terminate, e.g. interactive translate */
        }

    } while (undo.u_chain);

    x_undo_state = s_undo_state;

    if (mode <= 0 && !x_undo_scrap) {
        ewprintf(redo ? "Redone." : "Undone.");
    }

    if (-1 == mode) {
        acc_assign_int(ret);
    }
}


/*  Function:           do_redo
 *      redo primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>> [scrap]
    Macro: redo - Redo an undo operation.

        void
        redo()

    Macro Description:
        The 'redo()' primitive redoes the last undone operation.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        undo
 */
void
do_redo(void)                   /* () */
{
    /*see macro implementation*/
}   
/*end*/
