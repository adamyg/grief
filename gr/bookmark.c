#include <edidentifier.h>
__CIDENT_RCSID(gr_bookmark_c,"$Id: bookmark.c,v 1.32 2015/02/11 23:25:12 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: bookmark.c,v 1.32 2015/02/11 23:25:12 cvsuser Exp $
 * Bookmark implementation.
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

#include "bookmark.h"

#include "accum.h"                              /* acc_...() */
#include "basic.h"                              /* move_abs */
#include "buffer.h"
#include "builtin.h"
#include "echo.h"
#include "eval.h"
#include "lisp.h"
#include "main.h"
#include "register.h"
#include "symbol.h"
#include "window.h"
#include "word.h"

typedef TAILQ_HEAD(_BookmarkList, _bookmark)
                        BOOKMARKLIST_t;

typedef struct _bookmark {
    MAGIC_t             b_magic;                /* Structure magic */
#define BOOKMARK_MAGIC      MKMAGIC('B','k','M','k')
    TAILQ_ENTRY(_bookmark)
                        b_node;                 /* List node */
    accint_t            b_ident;                /* Bookmark number */
 // accini_t		b_local;                /* Vim style buffer localised identifier (TODO) */
    IDENTIFIER_t        b_buffer;               /* Buffer identifier */
    LINENO              b_line;                 /* Line */
    LINENO              b_col;                  /* Column */
} bookmark_t;

static bookmark_t *     bookmark_find(accint_t ident, int create);

static BOOKMARKLIST_t   x_bookmarks;            /* Bookmark list */


/*  Function:           bookmark_init
 *      Initialise the bookmark subsystem.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
bookmark_init(void)
{
    TAILQ_INIT(&x_bookmarks);
}


/*  Function:           bookmark_shutdown
 *      Shutdown the bookmark subsystem
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
bookmark_shutdown(void)
{
    BOOKMARKLIST_t *bookmarks = &x_bookmarks;
    bookmark_t *bk;

    while (NULL != (bk = TAILQ_FIRST(bookmarks))) {
        assert(BOOKMARK_MAGIC == bk->b_magic);
        TAILQ_REMOVE(bookmarks, bk, b_node);
        bk->b_magic = 0xDEADBEEF;
        chk_free((void *)bk);
    }
}


/*  Function:           do_drop_bookmark
 *      drop_anchor primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: drop_bookmark - Create or update a bookmark.

        int
        drop_bookmark([int bookid], [string yesno],
	    [int bufnum], [int line], [int column], [int local = FALSE])

    Macro Description:
        The 'drop_bookmark()' primitive either create a new or
        updates an existing bookmark. A bookmark is a named place
        holder with a buffer, representing a specific physical
        location within that buffer.

        'bookid' is the unique identifier to be associated with the
        bookmark; any valid integer may be used as the identifier.
        The bookmark shall be associated with the buffer 'bufnum',
        'line' and 'column', if any are omitted the current buffer
        and location with that shall be used.

        Upon there being an existing definition against the specified
        bookmark identifier, the user shall be prompted as follows,
        asking whether or not the bookmark should be replaced:

>           Overwrite existing bookmark [y/n]?

        The 'yesno' argument if given disables the user prompt. If
        supplied with either "y" or "yes" the bookmark shall
        automatically be replaced, otherwise the bookmark is retained
        without change with the user informed as follows:

>           Bookmark already exists.

        Upon successful completion, the user shall be informed as
        follows regardless of whether a new or updated definition
        resulted:

>           Bookmark dropped.

    Macro Parameters:
        bookid - Optional bookmark identifier, if omitted a new unique
            book mark identifier shall be generated.

        yesno - Optional string buffer containing the answer to be
            applied upon the bookmark pre-existing, if given as
            "y[es]" the bookmark shall be overridden otherwise is
            shall be related. Otherwise if omitted upon a preexisting
            bookmark the user shall be prompted.

        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

        line - Optional integer line number within the buffer, if
            omitted shall default to the top of the buffer (1).

        column - Optional integer column number within the buffer, if
            omitted shall default to the left of the buffer (1).

	local - Reserved for future use.

    Macro Returns:
        The 'drop_bookmark()' primitive returns the associated book
        mark identifier, otherwise 0 on error.

    Macro Portability:
        n/a

    Macro TODO:
        Support buffer local bookmark identifiers; improves Vim
        compatibility.

    Macro See Also:
        bookmark_list, delete_bookmark, goto_bookmark
 */
void
do_drop_bookmark(void)          /* (int bookid, [string yesno],
                                        [int bufnum], [int line], [int row], [int local = FALSE]) */
{
    const accint_t bookid = get_xinteger(1, -1);
    const char *yesno = get_xstr(2);
    bookmark_t *bk;

    acc_assign_int((accint_t) 0);

    /* create or overwrite */
    if (-1 != bookid &&
            NULL != (bk = bookmark_find(bookid, FALSE))) {
        if (NULL == yesno) {
            if (eyorn("Overwrite existing bookmark") != TRUE) {
                return;
            }
        } else if ('y' != *yesno && 'Y' != *yesno) {
            infof("Bookmark already exists.");
            return;
        }
    } else {
        if (NULL == (bk = bookmark_find(bookid, TRUE))) {
            return;
        }
    }

    if (isa_integer(3) && isa_integer(4) && isa_integer(5)) {
        /*
         *  apply all
         */
        bk->b_buffer = (uint16_t) get_xinteger(3, 0);
        bk->b_line = get_xinteger(4, 1);
        bk->b_col = get_xinteger(5, 1);

    } else {
        /*
         *  current
         */
        bk->b_buffer = curbp->b_bufnum;
        bk->b_line = *cur_line;
        bk->b_col = *cur_col;
    }

    infof("Bookmark dropped.");
    triggerx(REG_BOOKMARK, "%d", (int) bookid); /*ACCINT*/
    acc_assign_int((accint_t) bookid);
}


/*  Function:           do_delete_bookmark
 *      delete_bookmark primitive, function to destroy a bookmark.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: delete_bookmark - Delete a bookmark.

        void
        delete_bookmark(int bookid)

    Macro Description:
        The 'delete_bookmark()' primitive deletes the bookmark 'bookid'.

        Upon successful completion, the *REG_BOOKMARK* event shall be
        triggered (see register_macro).

    Macro Parameters:
        bookid - Bookmark identifier.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        bookmark_list, drop_bookmark, goto_bookmark
 */
void
do_delete_bookmark(void)        /* (int bookid) */
{
    const accint_t bookid = get_xinteger(1, -1);
    BOOKMARKLIST_t *bookmarks = &x_bookmarks;
    bookmark_t *bk = NULL;

    TAILQ_FOREACH(bk, bookmarks, b_node) {
        assert(BOOKMARK_MAGIC == bk->b_magic);
        if (bk->b_ident == bookid) {
            TAILQ_REMOVE(bookmarks, bk, b_node);
            bk->b_magic = 0xDEADBEEF;
            chk_free((void *)bk);
            triggerx(REG_BOOKMARK, "%d", (int) bookid); /*ACCINT*/
            return;
        }
    }
}


/*  Function:           do_goto_bookmark
 *      goto_bookmark primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: goto_bookmark - Seek a bookmark.

        int
        goto_bookmark(int bookid = NULL,
                [int &bufnum], [int &line], [int &column])

    Macro Description:
        The 'goto_bookmark()' primitive changes the current buffer
        and cursor location to the values associated with the named
        bookmark.

        'bookid' is the unique identifier or name associated with the
        bookmark; any valid integer may be used as the identifier. If
        omitted the user shall be prompted for the bookmark identifier.

>           Go to bookmark:

        If any of the arguments 'bufnum', 'line' or 'column' are
        specified, these are modified to contain the related bookmark
        values without effecting the current buffer or location. If
        all are omitted the bookmark is applied, updating the buffer
        and/or cursor location as required.

    Macro Parameters:
        bookid - Bookmark identifier.

        bufnum - Optional integer reference, if specified shall be
            populated with the associate buffer number.

        line - Optional integer reference, if specified shall be
            populated with the buffer line number.

        column - Optional integer reference, if specified shall be
            populated with the buffer column number.

    Macro Returns:
        The 'goto_bookmark()' primitive returns a non-zero value and
        populate any of the supplied arguments 'bufnum', 'line' and
        'col'. Otherwise on error returns zero and a related error
        shall be displayed.

>           No such bookmark

>           goto_bookmark: No such buffer

    Macro Examples:

        The following two examples deal with the bookmark labelled
        as '9'.

        Retrieves the bookmark definition and echos the details to
        the user.

>           int buf, line, column;
>
>           goto_bookmark(9, buf, line, column);
>           message("bookmark: buf=%d, %d/%d", buf, line, column);

        Applies the bookmark definition.

>           goto_bookmark(9);

    Macro Portability:
        n/a

    Macro See Also:
        bookmark_list, delete_bookmark, drop_bookmark
 */
void
do_goto_bookmark(void)          /* int (int bookid = NULL, [int &bufnum], [int &line], [int &column]) */
{
    accint_t bookid = 0;
    bookmark_t *bk;
    int ret = 1;

    if (get_iarg1("Go to bookmark: ", &bookid)) {
        ret = 0;

    } else if (NULL == (bk = bookmark_find(bookid, FALSE))) {
        ewprintf("No such bookmark.");
        ret = 0;

    } else {
        int apply = TRUE;

        /* return details */
        if (! isa_undef(2)) {
            sym_assign_int(get_symbol(2), (accint_t) bk->b_buffer);
            apply = FALSE;
        }

        if (! isa_undef(3)) {
            sym_assign_int(get_symbol(3), (accint_t) bk->b_line);
            apply = FALSE;
        }

        if (! isa_undef(4)) {
            sym_assign_int(get_symbol(4), (accint_t) bk->b_col);
            apply = FALSE;
        }

        /* otherwise apply the bookmrk */
        if (apply) {
            WINDOW_t *wp;

            /*
             *  If current buffer doesn't match the bookmark,
             *  then look for a window with the bookmark on display
             */
            if (bk->b_buffer != curbp->b_bufnum) {
                for (wp = window_first(); wp; wp = window_next(wp))
                    if (wp->w_bufp && wp->w_bufp->b_bufnum == bk->b_buffer) {
                        break;
                    }

                if (NULL == wp) {
                    BUFFER_t *bp = buf_lookup(bk->b_buffer);

                    if (NULL == bp) {
                        ewprintf("goto_bookmark: No such buffer.");
                        ret = 0;
                    } else {
                        buf_show(bp, curwp);
                        curbp = bp;
                    }
                } else {
                    curwp = wp;
                    curbp = curwp->w_bufp;
                }
            }

            if (ret) {
                set_hooked();
                move_abs(bk->b_line, bk->b_col);
            }
        }
    }
    acc_assign_int(ret);
}


/*  Function:           do_bookmark_list
 *      bookmark_list primitive.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: bookmark_list - Retrieve existing bookmark list.

        list
        bookmark_list()

    Macro Description:
        The 'bookmark_list()' primitive creates a list containing all
        currently defined bookmarks. For each definition the list
        shall contain a 4 integer record representing the bookmark as
        follows:

>               { bookid, bufnum, line, column }

            o bookid - Bookmark identifier.
            o bufnum - buffer number.
            o line   - buffer line number.
            o column - buffer column number.

        If no bookmarks exist then a *NULL* list shall be returned.

    Macro Parameters:
        none

    Macro Returns:
        The 'bookmark_list()' primitive returns a list containing the
        bookmark definitions otherwise NULL if no bookmarks exist.

    Macro Portability:
        n/a

    Macro See Also:
        delete_bookmark, drop_bookmark, goto_bookmark
 */
void
do_bookmark_list(void)          /* () */
{
    BOOKMARKLIST_t *bookmarks = &x_bookmarks;
    bookmark_t *bk = NULL;
    LIST *newlp, *lp;
    int count = 0, llength;

    TAILQ_FOREACH(bk, bookmarks, b_node) {
        assert(BOOKMARK_MAGIC == bk->b_magic);
        ++count;
    }
    llength = (count * (sizeof_atoms[F_INT] * 4)) + sizeof_atoms[F_HALT];
    if (0 == count || NULL == (newlp = lst_alloc(llength, count * 4))) {
        acc_assign_null();
        return;
    }
    lp = newlp;

    TAILQ_FOREACH(bk, bookmarks, b_node) {
        assert(BOOKMARK_MAGIC == bk->b_magic);
        lp = atom_push_int(lp, bk->b_ident);
        lp = atom_push_int(lp, bk->b_buffer);
        lp = atom_push_int(lp, bk->b_line);
        lp = atom_push_int(lp, bk->b_col);
        --count;
    }
    atom_push_halt(lp);

    assert(0 == count);
    lst_check(newlp);
    acc_donate_list(newlp, llength);
}


/*  Function:           bookmark_find
 *      Find the bookmark associated with a bookmark ID. If flag is TRUE then
 *      create it if not already defined.
 *
 *  Parameters:
 *      bookid - Bookmark identifier.
 *      create - Create flag, if *true* when a new the bookmark
 *         object shall be created if is does not pre-exist.
 *
 *   Returns:
 *      Bookmark object, otherwise NULL.
 */
static bookmark_t *
bookmark_find(accint_t bookid, int create)
{
    BOOKMARKLIST_t *bookmarks = &x_bookmarks;
    bookmark_t *bk = NULL;

    TAILQ_FOREACH(bk, bookmarks, b_node) {
        assert(BOOKMARK_MAGIC == bk->b_magic);
        if (bk->b_ident == bookid) {
            return bk;
        }
    }

    if (create) {
        if (NULL != (bk = (bookmark_t *) chk_calloc(sizeof(bookmark_t),1))) {
            TAILQ_INSERT_TAIL(bookmarks, bk, b_node);
            bk->b_magic = BOOKMARK_MAGIC;
            bk->b_ident = bookid;
            bk->b_buffer = 0;
            bk->b_line = 0;
            bk->b_col = 0;
        }
    }
    return bk;
}
/*end*/

