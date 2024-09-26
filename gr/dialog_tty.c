#include <edidentifier.h>
__CIDENT_RCSID(gr_dialog_tty_c,"$Id: dialog_tty.c,v 1.32 2024/09/25 13:58:06 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: dialog_tty.c,v 1.32 2024/09/25 13:58:06 cvsuser Exp $
 * Dialog manager, TTY interface.
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

#include "accum.h"                              /* acc_...() */
#include "border.h"
#include "buffer.h"                             /* buf_find */
#include "builtin.h"                            /* set_hooked */
#include "color.h"
#include "debug.h"
#include "display.h"
#include "eval.h"
#include "getkey.h"
#include "keyboard.h"
#include "kill.h"
#include "lisp.h"                               /* atom_...() */
#include "macros.h"                             /* loadup_macro */
#include "main.h"
#include "mchar.h"                              /* mchar_...() */
#include "mouse.h"
#include "tty.h"
#include "window.h"
#include "word.h"

#include "dialog.h"
#include "dialog_tty.h"
#include "widgets.h"
#include "widgets_tty.h"

#define DIALOG_UFPACKED     0x4242

typedef struct {
    uint32_t            f_magic;                /* structure magic */
    WINDOW_t *          f_wp;                   /* our window */
    BUFFER_t *          f_bp;                   /* and buffer */
} TTYFrame_t;

typedef struct {
    uint32_t            cx_magic;               /* structure magic */
    WINDOW_t *          cx_owp;                 /* original window */
    BUFFER_t *          cx_obp;                 /* original buffer */
    TTYFrame_t          cx_base;
    TTYFrame_t          cx_popup;
    TTYFrame_t *        cx_focus;
    struct _widget *    cx_current;             /* current active widget (if any) */
    int                 cx_packerr;             /* packer error status */
    int                 cx_dirty;               /* dirty event count */
    uint32_t            cx_magic2;              /* structure magic */
} DialogContext_t;

static int              dlg_open(DIALOG_t *d, DialogContext_t *context);
static void             dlg_close(DIALOG_t *d);

static int              ttyframe_create(TTYFrame_t *frame, int clear, const char *title, int x, int y, int cols, int rows);
static int              ttyframe_focus(const TTYFrame_t *frame);
static void             ttyframe_close(TTYFrame_t *frame, int buffer);

static int              dlg_controller(DIALOG_t *dialog, unsigned op, int p1, ...);
static void             dlg_update(DIALOG_t *d);
static int              dlg_hotkey(DIALOG_t *d, const int key);
static void             dlg_keydown(DIALOG_t *d, const int key);
static int              dlg_accelerator(WIDGET_t *w, const int key);
static void             dlg_mouse(DIALOG_t *d, struct IOEvent *evt);
static void             dlg_focusfirst(DIALOG_t *d);
static void             dlg_select(DIALOG_t *d, int action, int tabstops);

static int              dlg_clrfocus(DIALOG_t *d, int force);

static int              dlg_pack(DIALOG_t *d);
static void             dlg_pack_init(WIDGET_t *parent);
static void             dlg_pack_calc(WIDGET_t *parent);
static void             dlg_pack_apply(WIDGET_t *parent);

#define widget_name(_w) \
            ((_w) && (_w)->w_name ? (_w)->w_name : "unnamed")

static int              bcast_all(WIDGET_t *w, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);
static int              bcast_dirty(WIDGET_t *w, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);

static void             packer(WIDGET_t *parent);
static int              pack_expandx(WIDGET_t *w, int cavity);
static int              pack_expandy(WIDGET_t *w, int cavity);


/*  Function:           dialog_tty_run
 *      Run a TTY based dialog box.
 *
 *  Parameters:
 *      d - Dialog instances.
 *
 *  Returns:
 *      nothing
 */
void
dialog_tty_run(DIALOG_t *d)
{
    DialogContext_t c = {0};                    /* local context */
    struct IOEvent evt = {0};

    if (0 == dlg_open(d, &c)) {

        dialog_send(d, WIDGET_READY, 0, 0);
        dialog_bcast(d, NULL, NULL, FALSE, WIDGET_READY, 0, 0);

        dlg_focusfirst(d);

        while (d->d_running > 0) {
            int ret;

            ttyframe_focus(c.cx_focus);

            vtupdate();
            ret = io_get_event(&evt, EVT_SECOND(1));
            trace_ilog("dialog_event(event:%d,(curbp:%p->%d, curwp:%p)\n", \
                ret, curbp, (curbp ? curbp->b_bufnum : -1), curwp);

            ttyframe_focus(&c.cx_base);

            if (EVT_KEYDOWN == ret) {
                const int key = evt.code;
                int done = FALSE;

                if (c.cx_current) {             /* current widget first */
                    WIDGET_t *w = c.cx_current;

                    trace_ilog("\t=> KEYDOWN current (%s)\n", widget_name(w));
                    done = widget_send(w, WIDGET_KEYDOWN, key, (WIDGETARG_t) &evt);

                    if (done) {                 /* automove, next field */
                        if (KEY_ENTER == key && (WIDGET_FAUTOMOVE & w->w_flags)) {
                            if (! widget_send(w, WIDGET_KEYDOWN, KEY_NEXT, 0)) {
                                dlg_keydown(d, KEY_NEXT);
                            }
                        }
                    }
                                                /* parents */
                    while (!done && NULL != (w = w->w_parent)) {
                        if (WIDGET_FKEYPARENT & w->w_flags) {
                            trace_ilog("\t=> KEYDOWN parent (%s)\n", widget_name(w));
                            done = widget_send(w, WIDGET_KEYDOWN, key, (WIDGETARG_t) &evt);
                        }
                    }
                }

                if (!done) {                    /* alternative widgets */
                    done = dlg_hotkey(d, key);
                }

                if (!done) {                    /* default key handler */
                    dlg_keydown(d, key);
                }

            } else if (EVT_MOUSE == ret) {
                dlg_mouse(d, &evt);

            } else if (EVT_TIMEOUT == ret) {
                if (c.cx_current) {
                    widget_send(c.cx_current, WIDGET_IDLE, 0, 0);
                }
                dialog_send(d, WIDGET_IDLE, 0, 0);
            }
        }
        dlg_close(d);
    }
}


WIDGET_t *
dialog_tty_current(DIALOG_t *d)
{
    if (d) {
        DialogContext_t *c = d->d_ucontrol;
        if (c) {
            return c->cx_current;
        }
    }
    return NULL;
}


int
dialog_tty_popup_create(DIALOG_t *d, const WIDGET_t *w, int rows, int cols, const char *msg)
{
    const int maxcols = ttcols(), maxrows = ttrows();
    DialogContext_t *c = d->d_ucontrol;
    WINDOW_t *wp2 = NULL;
    int below = TRUE;
    int x, y;

    if (w) {
        const WINDOW_t *owp = c->cx_base.f_wp;

        x = owp->w_x + w->w_absx;
        if ((x + cols + 3) >= maxcols) {
            x -= (cols - w->w_cols) + 1;        /* left */
        }

        y = owp->w_y + w->w_absy + w->w_rows;
        if ((y + rows + 3) >= maxrows) {
            y -= (w->w_rows + rows);            /* above */
            below = FALSE;
        }

    } else {
        x = (maxcols - (d->d_cols + 2)) / 2;
        y = (maxrows - (d->d_rows + 2)) / 3;
    }

    if (! ttyframe_create(&c->cx_popup, TRUE, NULL, x, y, cols, rows)) {
        return -1;
    }

    wp2 = c->cx_popup.f_wp;
    WFSET(wp2, WF_SELECTED);
    if (below) {
        if (w) {
            WFSET(wp2, WF_NO_TITLE);            /* no top line */
        }
        if (msg) {
            window_title(wp2, "", msg);
        }
    } else {
        if (w) {
            WFSET(wp2, WF_NO_MESSAGE);          /* no bottom line */
        }
        if (msg) {
            window_title(wp2, msg, "");
       }
    }
    if (!msg) {
        window_title(wp2, "", "");
    }
    return 0;
}


int
dialog_tty_popup_select(DIALOG_t *d, int state)
{
    DialogContext_t *c = d->d_ucontrol;

    if (state) {
        return ttyframe_focus(&c->cx_popup);
    }
    ttyframe_focus(&c->cx_base);
    return TRUE;
}


int
dialog_tty_popup_focus(DIALOG_t *d, int state)
{
    DialogContext_t *c = d->d_ucontrol;
    WINDOW_t *wp = c->cx_popup.f_wp;

    if (state < 0) {                            /* hide */
        if (NULL == wp) {
            return FALSE;
        }
        if (0 == WFTST(wp, WF_HIDDEN)) {
            ++c->cx_dirty;
        }
        WFSET(wp, WF_HIDDEN);
        c->cx_focus = NULL;

    } else if (state > 0) {                     /* show */
        if (NULL == wp || NULL == c->cx_popup.f_bp) {
            return FALSE;
        }
        if (WFTST(wp, WF_HIDDEN)) {
            ++c->cx_dirty;
        }
        WFCLR(wp, WF_HIDDEN);
        c->cx_focus = &c->cx_popup;

    } else {                                    /* unfocus */
        c->cx_focus = NULL;
    }
    return TRUE;
}


int
dialog_tty_popup_close(DIALOG_t *d)
{
    DialogContext_t *c = d->d_ucontrol;

    (void) dialog_tty_popup_focus(d, FALSE);
    ttyframe_close(&c->cx_popup, TRUE);
    return 0;
}


/*  Function:           dlg_open
 *      Open a dialog instance in preparation for running, assigning buffer and window resources.
 *
 *  Parameters:
 *      d - Dialog instances.
 *      c - Local context.
 *
 *  Returns:
 *      0 = success, otherwise -1 on error.
 */
static int
dlg_open(DIALOG_t *d, DialogContext_t *c)
{
    const int cols = ttcols() - 1, rows = ttrows() - 3;
    int x, y;

    /*
     *  TTY context
     */
    d->d_ucontrol = c;
    d->d_controller = dlg_controller;

    c->cx_obp = curbp;                          /* save */
    c->cx_owp = curwp;

    /*
     *  widget sizing/
     *      either run the widget packer or grid processing logic.
     */
    widget_clear(d);
    if (0 == d->d_uflags) {
        dialog_send(d, WIDGET_INIT, 0, 0);
        dialog_bcast(d, NULL, NULL, FALSE, WIDGET_INIT, 0, 0);
        dialog_callback(d, DLGE_INIT, 0, 0);    /* dialog initialisation */
        d->d_uflags |= DIALOG_UFPACKED;
        dlg_pack(d);

    } else {
        dialog_callback(d, DLGE_INIT, 0, 0);    /* dialog initialisation */
    }

    /*
     *  Create the window,
     *      positioned (if possible) centered 1/2 vert and 1/3 horz.
     */
    if ((x = d->d_xhint) < 0) {                 /* dynamic position */
        if ((x = (cols - (d->d_cols + 2)) / 2) < 0) {
            x = 0;
        }
    } 

    if ((x + d->d_cols) >= cols) {              /* limit to cols */
        if ((x = cols - (d->d_cols + 2)) < 0) {
            x = 0;
        }
    }

    if ((y = d->d_yhint) < 0) {                 /* dynamic position */
        if ((y = (rows - (d->d_rows + 2)) / 3) < 0) {
            y = 0;
        }
    }  

    if ((y + d->d_rows) >= rows) {              /* limit to rows */
        if ((y = rows - (d->d_rows + 2)) < 0) {
            y = 0;
        }
    }

    if (! ttyframe_create(&c->cx_base, TRUE,
            (d->d_title ? d->d_title : ""), x + 1, y + 1, d->d_cols, d->d_rows)) {
        return -1;
    }

    if (d->d_styles & DLGS_SYSCLOSE) {
        window_ctrl_set(c->cx_base.f_wp, WCTRLO_CLOSE_BTN);
    }

    ttyframe_focus(&c->cx_base);
    curwp->w_dialogp = d;
    c->cx_dirty = TRUE;
    return (0);
}


/*  Function:           dlg_close
 *      Close a dialog instance post a running, releasing the associated window
 *      and buffer resources.
 *
 *  Parameters:
 *      d - Dialog instances.
 *
 *  Returns:
 *      nothing
 */
static void
dlg_close(DIALOG_t *d)
{
    DialogContext_t *c = d->d_ucontrol;

    d->d_ucontrol = NULL;
    c->cx_current = NULL;
    set_curwpbp(c->cx_owp, c->cx_obp);          /* restore */

    ttyframe_close(&c->cx_popup, TRUE);
    ttyframe_close(&c->cx_base, TRUE);
}


static int
ttyframe_create(TTYFrame_t *frame, int clear, const char *title, int x, int y, int cols, int rows)
{
    WINDOW_t *owp = curwp, *wp;
    BUFFER_t *obp = curbp, *bp;

    /*
     *  Create a buffer for the dialog window.
     *      If buffer by same name already exists, then create new one with a different name.
     */
    if (NULL == (bp = frame->f_bp)) {
        if (NULL == title) {
            title = "dialogframe";
        }

        if (NULL == (bp = buf_find(title))) {
            bp = buf_create(title, MCHAR_UTF8, TRUE);
        } else {
            char *cp = chk_alloc(strlen(title) + 10);
            int id = 0;

            while (1) {                         /* until unique */
                sprintf(cp, "%s[%d]", title, ++id);
                if (NULL == buf_find(cp)) {
                    break;
                }
            }
            bp = buf_create(cp, MCHAR_UTF8, TRUE);
            chk_free(cp);
        }

        set_curwpbp(NULL, bp);

        bp->b_termtype = LTERM_UNIX;            /* UNIX style line feeds */
        bp->b_imode = TRUE;                     /* localised insert-mode */
        bp->b_attrcurrent = ATTR_DIALOG_NORMAL; /* base colors */
        bp->b_attrnormal = ATTR_DIALOG_NORMAL;

        BFCLR(bp, BF_TABS);                     /* disable <Tab> expansion */
        BFSET(bp, BF_SYSBUF);                   /* system buffer */
        BFSET(bp, BF_NO_UNDO);                  /* disable 'undo' */
        BF2SET(bp, BF2_DIALOG);                 /* dialog attached */

    } else {
        if (clear) {
            buf_clear(bp);
        }
    }

    /*
     *  Create the window,
     */
    if (-1 == window_create(W_POPUP, "", x, y, cols, rows)) {
        set_curwpbp(owp, obp);
        buf_kill(bp->b_bufnum);
        return FALSE;
    }

    /*
     *  Attach buffer, window and dialog.
     */
    wp = curwp;
    WFSET(wp, WF_DIALOG);
    attach_buffer(wp, bp);
    wp->w_ctrl_state = 0;                       /* disable scrollbars etc */
    set_curwpbp(owp, obp);
    frame->f_wp = wp;
    frame->f_bp = bp;
    return TRUE;
}


static int
ttyframe_focus(const TTYFrame_t *frame)
{
    if (frame && frame->f_wp && frame->f_bp) {
        if (curwp != frame->f_wp || curbp != frame->f_bp) {
            set_curwpbp(frame->f_wp, frame->f_bp);
        }
        return TRUE;
    }
    return FALSE;
}


static void
ttyframe_close(TTYFrame_t *frame, int buffer)
{
    if (frame->f_bp) {
        assert(curbp != frame->f_bp);
        if (frame->f_wp) {
            assert(curwp != frame->f_wp);
            frame->f_wp->w_dialogp = NULL;
            detach_buffer(frame->f_wp);
            window_delete(frame->f_wp);
            frame->f_wp = NULL;
        }

        if (buffer) {
            buf_kill(frame->f_bp->b_bufnum);
            frame->f_bp = NULL;
        }
    }
}


/*  Function:           dlg_controller
 *      Dialog system event handler.
 *
 *  Parameters:
 *      d - Dialog instances.
 *      op - Operation.
 *
 *  Returns:
 *      nothing
 */
static int
dlg_controller(DIALOG_t *d, unsigned op, int p1, ...)
{
    const DialogContext_t *c = d->d_ucontrol;

    __CUNUSED(p1)
    switch (op) {
    case DIALOG_UUPDATE: {      /* vtupdate request */
            WINDOW_t *savwp = curwp;
            BUFFER_t *savbp = curbp;

            ttyframe_focus(&c->cx_base);
            dlg_update(d);
            set_curwpbp(savwp, savbp);
        }
        break;

    case DIALOG_UCALLBACK:      /* callback */
        if (p1) {
            /*TODO*/
        }
        break;
    }
    return 0;
}


/*  Function:           dlg_update
 *      Update the content of a TTY based dialog box, to be contained within the
 *      attached window.
 *
 *      This interface is called via the update() primitive this is invoked prior
 *      to any keyboard interaction, or directly by macro.
 *
 *  Parameters:
 *      d - Dialog instances.
 *
 *  Returns:
 *      nothing
 */
static void
dlg_update(DIALOG_t *d)
{
    DialogContext_t *c = d->d_ucontrol;

    if (c->cx_dirty) {
        dialog_send(d, WIDGET_PAINT, 0, 0);
    }
    dialog_bcast(d, c->cx_current, (c->cx_dirty ? bcast_all : bcast_dirty), TRUE, WIDGET_PAINT, 0, 0);
    if (c->cx_current) {
        widget_send(c->cx_current, WIDGET_CARET, 1, 0);
    }
    c->cx_dirty = 0;
}


static int
bcast_all(WIDGET_t *w, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    __CUNUSED(msg)
    __CUNUSED(p1)
    __CUNUSED(p2)
    w->w_uflags &= ~WTTY_FDIRTY;
    return TRUE;
}


static int
bcast_dirty(WIDGET_t *w, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    __CUNUSED(msg)
    __CUNUSED(p1)
    __CUNUSED(p2)
    if (0 == (WTTY_FDIRTY & w->w_uflags)) {
        return FALSE;
    }
    w->w_uflags &= ~WTTY_FDIRTY;
    return TRUE;
}


static void
dlg_stop(DIALOG_t *d)
{
    d->d_running = 0;
}


/*  Function:           dlg_hotkey
 *      Process dialog hotkey/accelerators.
 *
 *  Parameters:
 *      d - Dialog instance.
 *      key - Key code.
 *
 *  Returns:
 *      TRUE if the key was handled, otherwise FALSE.
 */
static int
dlg_hotkey(DIALOG_t *d, const int key)
{
    DialogContext_t *c = d->d_ucontrol;
    WIDGET_t *w, *current = c->cx_current;
    int accelerator = FALSE;
    int done = FALSE;

    /* current widget */
    if (NULL != (w = current)) {
        if (dlg_accelerator(w, key)) {
            accelerator = done = widget_send(w, WIDGET_COMMAND, TRUE /*accelerator*/, key);
        } else {
            done = widget_send(w, WIDGET_HOTKEY, key, 0);
        }
    }

    /* others */
    if (! done) {
        WIDGET_t *start = dialog_next(d, NULL);

        if (NULL != (w = start)) {
            do {
                if (w != current && 0 == ((WIDGET_FGREYED|WIDGET_FHIDDEN) & w->w_flags)) {
                    if (dlg_accelerator(w, key)) {
                        accelerator = done = widget_send(w, WIDGET_COMMAND, TRUE /*accelerator*/, key);
                    } else {
                        done = widget_send(w, WIDGET_HOTKEY, key, 0);
                    }
                }
            } while (! done && (w = dialog_next(d, w)) != start && w);
        }
    }

    /* select */
    if (done) {
        if (! accelerator) {
            if (current != w) {
                assert(w);
                if (dlg_clrfocus(d, FALSE)) {
                    if (dialog_tty_setfocus(d, w)) {
                        current = NULL;
                    }
                }
            }
        }
        if (w) {
            widget_send(w, WIDGET_PAINT, 0, 0);
        }
    }
    return done;
}


static int
dlg_accelerator(WIDGET_t *w, const int key)
{
    if (w->w_accelerator &&                     /* allow multiple ??? */
            key == key_name2code(w->w_accelerator, NULL)) {
        trace_ilog("\tACCELERATOR(%s)\n", w->w_accelerator);
        return TRUE;
    }
    return FALSE;
}


/*  Function:           dlg_keydown
 *      Default KEYDOWN message processing.
 *
 *  Parameters:
 *      d - Dialog instance.
 *      key - Key code.
 *
 *  Returns:
 *      nothing
 */
static void
dlg_keydown(DIALOG_t *d, const int key)
{
    int done = FALSE;

    trace_ilog("dlg_keydown(%d/0x%x)\n", key, key);

    if (! done) {
        WIDGET_t *w, *start = dialog_next(d, NULL);

        if (NULL != (w = start)) {
            do {
                if (WIDGET_FKEYDOWN & w->w_flags) {
                    int rval = FALSE;
                                                /* optional KEYDOWN events */
                    if (widget_callback2(w, DLGE_KEYDOWN, key, 0, &rval) && TRUE == rval) {
                        done = TRUE;
                    }
                }
            } while (! done && (w = dialog_next(d, w)) != start);
        }
    }

    if (! done) {
        if (WIDGET_FKEYDOWN & d->d_flags) {
            int rval = FALSE;
                                                /* optional KEYDOWN events */
            if (dialog_callback2(d, DLGE_KEYDOWN, key, 0, &rval) && TRUE == rval) {
                done = TRUE;
            }
        }
    }

    if (! done) {
        DialogContext_t *c = d->d_ucontrol;
        WIDGET_t *current = c->cx_current;

        trace_ilog("\tdefault action (current:%p->%s(%d)\n",
            current, widget_name(current), (current ? current->w_ident : 0));

        switch (key) {
        case CTRL_P:
        case KEY_LEFT:
        case KEY_UP:
        case WHEEL_UP:
            if (current) {
                dlg_select(d, -1, FALSE);
            }
            done = TRUE;
            break;

        case CTRL_N:
        case KEY_RIGHT:
        case KEY_DOWN:
        case WHEEL_DOWN:
            if (current) {
                dlg_select(d, 1, FALSE);
            }
            done = TRUE;
            break;

        case BACK_TAB:
        case KEY_PREV:
            if (current) {
                trace_ilog("\tBACK_TAB\n");
                dlg_select(d, -1, TRUE);
            }
            done = TRUE;
            break;

        case KEY_TAB:
        case KEY_NEXT:
    /*- case KEY_AUTOMOVE: -*/
            if (current) {
                trace_ilog("\tTAB\n");
                dlg_select(d, 1, TRUE);
            }
            done = TRUE;
            break;

        case F(1):
            if (d->d_widget.w_help) {
                LIST tmpl[LIST_SIZEOF(2)], *lp = tmpl;

                lp = atom_push_sym(lp, "explain");
                lp = atom_push_const(lp, d->d_widget.w_help);
                atom_push_halt(lp);
                assert(lp < (tmpl + sizeof(tmpl)));
                trace_ilog("dlg_keydown(explain %s)\n", d->d_widget.w_help);
                execute_nmacro(tmpl);
            }
            done = TRUE;
            break;

        case F(10):     /* XXX - backdoor exit */
            d->d_retval = -1;
            dlg_stop(d);
            done = TRUE;
            break;
        }
    }

    trace_ilog("dlg_keydown() : %d\n", done);
}


/*  Function:           dlg_mouse
 *      Default MOUSE message processing.
 *
 *  Parameters:
 *      d - Dialog instance.
 *      e - Input event.
 *
 *  Returns:
 *      nothing
 */
static void
dlg_mouse(DIALOG_t *d, struct IOEvent *evt)
{
    DialogContext_t *c = d->d_ucontrol;
    WIDGET_t *parent, *first, *w;
    WINDOW_t* wp = NULL;
    int wid, where = MOBJ_NOWHERE;
    int x, y;

    x = evt->mouse.x;
    y = evt->mouse.y;

    if (NULL == (first = c->cx_current)) {
        first = dialog_next(d, NULL);           /* retrieve first */
    }

    if (NULL != (w = first)) {
        /*
         *  popup/child
         */
        if (curwp != (wp = mouse_pos(x, y, &wid, &where))) {
            if (wp && c->cx_popup.f_wp == wp) {
                switch (where) {
                case MOBJ_INSIDE:
                    x -= wp->w_x + win_lborder(wp) + 1;
                    y -= wp->w_y + win_tborder(wp) + 1;
                    widget_send(w, WIDGET_MOUSE_POPUP, DIALOGARG32(x, y), evt->code);
                    break;
                }
            }
            return;
        }

        /*
         *  primary
         */
        assert(wp == curwp);
        switch (where) {
        case MOBJ_INSIDE:
            x -= wp->w_x + win_lborder(wp) + 1;
            y -= wp->w_y + win_tborder(wp) + 1;

            parent = NULL;
            do {
                if (0 == ((WIDGET_FGREYED | WIDGET_FHIDDEN) & w->w_flags)) {
                    if ((x >= w->w_absx) && (x < w->w_absx + w->w_cols + (w->w_border * 2)) &&
                        (y >= w->w_absy) && (y < w->w_absy + w->w_rows + (w->w_border * 2))) {

                        /* inner most widget */
                        if (NULL == TAILQ_FIRST(&w->w_children)) {
                            x -= w->w_absx;     /* normalise to widget */
                            y -= w->w_absy;
                            widget_send(w, WIDGET_MOUSE, DIALOGARG32(x, y), evt->code);
                            parent = NULL;
                            break;
                        } else {                /* parent */
                            parent = w;
                        }
                    }
                }
            } while ((w = dialog_next(d, w)) != first);

            __CUNUSED(parent)
#if (TODO)  // style WIDGET_NORMALISE
            if (parent) {
                x -= w->w_absx;                 /* normalise to widget */
                y -= w->w_absy;
                widget_send(parent, WIDGET_MOUSE, DIALOGARG32(x, y), evt->code);
            }
#endif
            break;
        case MOBJ_CLOSE:
            if (FALSE == widget_send(&d->d_widget, WIDGET_SYSCOMMAND, DLSC_CLOSE, DIALOGARG32(x, y))) {
                dlg_stop(d);
            }
            break;
        case MOBJ_TITLE:
            widget_send(&d->d_widget, WIDGET_SYSCOMMAND, DLSC_TITLE, DIALOGARG32(x, y));
            break;
        default:
            break;
        }
   }
}


#if (XXX)
    case WIDGET_SYSCOMMAND:
        if (dialog_callback2(d, DLGE_COMMAND, p1, p2, &iret)) {
            return iret;
        }
        break;
#endif


/*  Function:           dlg_focusfirst
 *      Select the first widget that takes focus.
 *
 *  Parameters:
 *      d - Dialog instances.
 *
 *  Returns:
 *      nothing
 */
static void
dlg_focusfirst(DIALOG_t *d)
{
    DialogContext_t *c = d->d_ucontrol;
    WidgetQueue_t *head = &d->d_widgetq;
    WIDGET_t *w;

    for (w = CIRCLEQ_FIRST(head); w != CIRCLEQ_END(head, _widget); w = CIRCLEQ_NEXT(w, w_node))
        if (w->w_flags & WIDGET_FDEFFOCUS) {
            if (dialog_tty_setfocus(d, w)) {
                break;
            }
        }

    if (NULL == c->cx_current)
        for (w = CIRCLEQ_FIRST(head); w != CIRCLEQ_END(head, _widget); w = CIRCLEQ_NEXT(w, w_node)) {
            if (dialog_tty_setfocus(d, w)) {
                break;
            }
        }
}


static void
dlg_select(DIALOG_t *d, int action, int tabstops)
{
    DialogContext_t *c = d->d_ucontrol;
    WIDGET_t *ocurrent = c->cx_current;

    if (dlg_clrfocus(d, FALSE)) {
        int count = dialog_count(d);
        WIDGET_t *w = ocurrent;                 /* starting point */
        int ret = 0;

        do {                                    /* loop thru all widgets */
            w = (action > 0 ? dialog_next : dialog_prev)(d, w);

            if (0 == tabstops ||                /* TABSTOP */
                    WIDGET_FTABSTOP == ((WIDGET_FTABSTOP|WIDGET_FTABNOT) & w->w_flags)) {
                if (0 != (ret = dialog_tty_setfocus(d, w))) {
                    break;
                }
            }
        } while (--count > 0 && w != ocurrent);

        if (! ret) {
            if (ocurrent) {
                dialog_tty_setfocus(d, ocurrent);
            }
        }
    }
}


/*  Function:           dialog_tty_setfocus
 *      Focus the specified widget.
 *
 *  Parameters:
 *      d - Dialog instances.
 *      w - Widget to focus.
 *
 *  Returns:
 *      TRUE if the current widget was unfocused, otherwise *FALSE*.
 */
int
dialog_tty_setfocus(DIALOG_t *d, WIDGET_t *w)
{
    int ret = FALSE;

    if (w) {
        DialogContext_t *c = d->d_ucontrol;
        WIDGET_t *current = c->cx_current;

        if (current) {
            if (current == w) {
                assert(HasFocus(w));
                return TRUE;
            }
            dlg_clrfocus(d, TRUE);              /* unfocus current */
        }

        c->cx_current = w;                      /* focus new/current */
        assert(! HasFocus(w));

        if (FALSE == widget_send(w, WIDGET_SETFOCUS, 1, 0)) {
            if (NULL == current || FALSE == widget_send(current, WIDGET_SETFOCUS, 1, 0)) {
                c->cx_current = w = NULL;
            } else {
                c->cx_current = w = current;
            }
        }

        if (w) {
            w->w_uflags |= WTTY_FFOCUS;
            (void) widget_send(w, WIDGET_PAINT, 0, 0);
            ret = TRUE;
        }
    }

    trace_ilog("\tsetfocus (current:%p->%s(%d)) : %d\n",
        w, widget_name(w), (w ? w->w_ident : 0), ret);
    return ret;
}


/*  Function:           dlg_clrfocus
 *      Unfocus the current widget, if any,
 *
 *  Parameters:
 *      d - Dialog instances.
 *      force - *TRUE* if the focus should be forcefully unfocused.
 *
 *  Returns:
 *      TRUE if the current widget was unfocused, otherwise *FALSE*.
 */
static int
dlg_clrfocus(DIALOG_t *d, int force)
{
    DialogContext_t *c = d->d_ucontrol;
    WIDGET_t *current = c->cx_current;
    int ret = FALSE;

    if (NULL == current)  {
        ret = TRUE;

    } else if (widget_send(current, WIDGET_SETFOCUS, 0, 0) || force) {
        current->w_uflags &= ~WTTY_FFOCUS;
        widget_send(current, WIDGET_PAINT, 0, 0);
        c->cx_current = NULL;
        ret = TRUE;
    }

    trace_ilog("\tclrfocus (force: %d, current:%p->%s(%d)) : %d\n",
        force, current, widget_name(current), (current ? current->w_ident : 0), ret);
    return ret;
}


static int
dlg_pack(DIALOG_t *d)
{
    DialogContext_t *c = d->d_ucontrol;
    WIDGET_t *root = &d->d_widget;

    c->cx_packerr = 0;
    dlg_pack_init(root);                        /* mark all widget as dirty */

    do {
        dlg_pack_calc(root);                    /* pack widgets */
    } while (0 == c->cx_packerr && (WTTY_FREPACK & root->w_uflags));

    if (0 == c->cx_packerr) {
        dlg_pack_apply(root);                   /* assign positions */
    }

    return (c->cx_packerr);
}


static void
dlg_pack_init(WIDGET_t *parent)
{
    WIDGET_t *w;

    parent->w_uflags |= WTTY_FREPACK;
    if (0 == parent->w_reqcols || 0 == parent->w_reqrows) {
        parent->w_uflags |= WTTY_FRESIZE;
    }
    for (w = TAILQ_FIRST(&parent->w_children); w; w = TAILQ_NEXT(w, w_sibling)) {
        dlg_pack_init(w);
    }
}


static void
dlg_pack_calc(WIDGET_t *parent)
{
    WIDGET_t *w;

    for (w = TAILQ_FIRST(&parent->w_children); w; w = TAILQ_NEXT(w, w_sibling)) {
        dlg_pack_calc(w);                       /* depth first */
    }

    if (WTTY_FREPACK & parent->w_uflags) {      /* apply child sizing */
        parent->w_uflags &= ~WTTY_FREPACK;

        if (WTTY_FRESIZE & parent->w_uflags) {
            parent->w_uflags &= ~WTTY_FRESIZE;

            parent->w_rows = parent->w_reqrows;
            parent->w_cols = parent->w_reqcols;
        }
        packer(parent);
    }
}


static void
dlg_pack_apply(WIDGET_t *parent)
{
    WIDGET_t *w;

    parent->w_absx = parent->w_x;               /* frame position */
    parent->w_absy = parent->w_y;

    if (NULL != (w = parent->w_parent)) {
        parent->w_absx += w->w_absx;            /* inherit parents position */
        parent->w_absy += w->w_absy;
    }
                                                /* allow children to inherit */
    for (w = TAILQ_FIRST(&parent->w_children); w; w = TAILQ_NEXT(w, w_sibling)) {
        dlg_pack_apply(w);
    }
}


#define X(w)                ((w)->w_x)
#define Y(w)                ((w)->w_y)
#define Border(w)           ((w)->w_border * 2)
#define Rows(w)             (((w)->w_rows))
#define Cols(w)             (((w)->w_cols))
#define PaddingRows(w)      (((w)->w_pady + (w)->w_ipady) * 2)
#define PaddingCols(w)      (((w)->w_padx + (w)->w_ipadx) * 2)

static void
RequestResize(WIDGET_t *w, int rows, int cols)
{
    w->w_uflags |= WTTY_FRESIZE;
    w->w_reqrows = rows - Border(w);
    w->w_reqcols = cols - Border(w);
    do {
        w->w_uflags |= WTTY_FREPACK;
    } while ((w = w->w_parent) != NULL);        /* repack parents */
}


static int
ReqRows(WIDGET_t *w)
{
    if ((w->w_uflags | WTTY_FRESIZE) && w->w_reqrows) {
        return w->w_reqrows + Border(w);
    }
    return w->w_rows + Border(w);
}


static int
ReqCols(WIDGET_t *w)
{
    if ((w->w_uflags | WTTY_FRESIZE) && w->w_reqcols) {
        return w->w_reqcols + Border(w);
    }
    return w->w_cols + Border(w);
}


static void
Resize(WIDGET_t *w, int x, int y, int cols, int rows)
{
    if (x != X(w) || y != Y(w) || cols != Cols(w) || rows != Rows(w)) {
        w->w_x = x;
        w->w_y = y;
        w->w_cols = cols;
        w->w_rows = rows;
        widget_send(w, WIDGET_RESIZED, 0, 0);
    }
}


static void
packer(WIDGET_t *parent)
{
    DialogContext_t *c = parent->w_root->d_ucontrol;
    int cavityX, cavityY, cavityCols, cavityRows;
    int frameX, frameY, frameCols, frameRows;
    int borderX, borderY;
    int borderTop, borderBtm;
    int borderLeft, borderRight;
    int maxCols, maxRows;
    int cols, rows;
    int x, y;
    WIDGET_t *w;

    if (NULL == TAILQ_FIRST(&parent->w_children)) {
        return;                                 /* no children */
    }

    /*
     *  Pass #1: scan all the slaves to figure out the total amount of space needed.
     *  Two separate cols and rows values are computed:
     */
ED_TRACE(("packing %s/%s (%d)\n", (parent->w_desc ? parent->w_desc : "n.a"),\
        widget_name(parent), parent->w_depth))

    cols = maxCols = Border(parent);
    rows = maxRows = Border(parent);

    for (w = TAILQ_FIRST(&parent->w_children); w; w = TAILQ_NEXT(w, w_sibling)) {
ED_TRACE(("        %s (%d)\n", widget_name(w), w->w_depth))
ED_TRACE(("            c, r    %d, %d\n", ReqCols(w) + PaddingCols(w), ReqRows(w) + PaddingRows(w)))

        if (DLGC_TAB == parent->w_class) {
            const int reqCols = ReqCols(w) + PaddingCols(w);
            const int reqRows = ReqRows(w) + PaddingRows(w);

            if (reqCols > cols) cols = reqCols;
            if (reqRows > rows) rows = reqRows;

        } else {
            if ((DLGA_ATTACH_TOP == w->w_attach) || (DLGA_ATTACH_BOTTOM == w->w_attach)) {
                                                /* append to height */
                const int reqCols = ReqCols(w) + PaddingCols(w) + cols;

                if (reqCols > maxCols) maxCols = reqCols;
                rows += ReqRows(w) + PaddingRows(w);

            } else {                            /* append to width */
                const int reqRows = ReqRows(w) + PaddingRows(w) + rows;

                if (reqRows > maxRows) maxRows = reqRows;
                cols += ReqCols(w) + PaddingCols(w);
            }
        }
    }

    if (cols > maxCols) maxCols = cols;
    if (rows > maxRows) maxRows = rows;

    /*
     *  If the total amount of space needed in the parent window has changed, and if
     *  we're propagating geometry information, then notify the next geometry manager
     *  up and requeue ourselves to start again after the arent has had a chance to
     *  resize us.
     */
ED_TRACE(("    want    c, r    %d, %d\n", maxCols, maxRows))
ED_TRACE(("    require c, r    %d, %d\n", ReqCols(parent), ReqRows(parent)))
ED_TRACE(("    curr    c, r    %d, %d\n", Cols(parent), Rows(parent)))
ED_TRACE(("    propagate       %d\n", (0 == (parent->w_flags & WIDGET_FDONTPROPAGATE)) ? TRUE : FALSE))

    if (0 == (parent->w_flags & WIDGET_FDONTPROPAGATE)) {
        if ((maxCols != ReqCols(parent)) || (maxRows != ReqRows(parent))) {
ED_TRACE(("    resizing parent\n"))
            RequestResize(parent, maxRows, maxCols);
            return;
        }
    }

    /*
     *  Pass #2: scan the slaves a second time assigning new sizes.
     *
     *  The "cavity" variables keep track of the unclaimed space in the cavity of the
     *  window; this shrinks inward as we allocate windows around the edges.
     *
     *  The "frame" variables keep track of the space allocated to the current window
     *  and its frame.
     *
     *  The current window is then placed somewhere in the frame, depending on anchor.
     */
    x = cavityX = Border(parent) / 2;
    y = cavityY = Border(parent) / 2;
    cavityCols  = Cols(parent);
    cavityRows  = Rows(parent);

ED_TRACE(("reorg children:\n"))

    for (w = TAILQ_FIRST(&parent->w_children); w; w = TAILQ_NEXT(w, w_sibling)) {

ED_TRACE(("    %s (%d)\n", widget_name(w), w->w_depth))
ED_TRACE(("            x, y    %d, %d\n", w->w_x, w->w_y))
ED_TRACE(("            c, r    %d, %d   (pady=%d,%d,padx=%d,%d)\n",\
                        ReqCols(w) + PaddingCols(w), ReqRows(w) + PaddingRows(w),\
                            w->w_pady, w->w_ipady, w->w_padx, w->w_ipadx))

ED_TRACE(("        cavity\n"))
ED_TRACE(("            x, y    %d, %d\n", cavityX, cavityY))
ED_TRACE(("            c, r    %d, %d\n", cavityCols, cavityRows))

ED_TRACE(("        attach      %c\n",\
                        (DLGA_ATTACH_TOP    == w->w_attach? 'T' : \
                        (DLGA_ATTACH_BOTTOM == w->w_attach? 'B': \
                        (DLGA_ATTACH_LEFT   == w->w_attach? 'L' : 'R')))))

        if (DLGC_TAB == parent->w_class) {
            frameCols = ReqCols(w) + PaddingCols(w);
            frameRows = ReqRows(w) + PaddingRows(w);
            frameX    = cavityX;
            frameY    = cavityY;

        } else {
            if ((DLGA_ATTACH_TOP == w->w_attach) || (DLGA_ATTACH_BOTTOM == w->w_attach)) {
                frameCols = cavityCols;
                frameRows = ReqRows(w) + PaddingRows(w);
                if (w->w_flags & WIDGET_FEXPAND) {
                    frameRows += pack_expandy(w, cavityRows);
                }
                cavityRows -= frameRows;
                if (cavityRows < 0) {
                    frameRows += cavityRows;
                    cavityRows = 0;
                }
                frameX = cavityX;
                if (DLGA_ATTACH_TOP == w->w_attach) {
                    frameY = cavityY;
                    cavityY += frameRows;
                } else {
                    frameY = cavityY + cavityRows;
                }

            } else {        /* LEFT/RIGHT */
                frameRows = cavityRows;
                frameCols = ReqCols(w) + PaddingCols(w);
                if (w->w_flags & WIDGET_FEXPAND) {
                    frameCols += pack_expandx(w, cavityCols);
                }
                cavityCols -= frameCols;
                if (cavityCols < 0) {
                    frameCols += cavityCols;
                    cavityCols = 0;
                }
                frameY = cavityY;
                if (DLGA_ATTACH_LEFT == w->w_attach) {
                    frameX = cavityX;
                    cavityX += frameCols;
                } else {
                    frameX = cavityX + cavityCols;
                }
            }
        }

        /*
         *  Now that we've got the size of the frame for the window, compute the
         *  window's actual size and location using the fill, padding, and frame factors.
         */
        borderX = borderLeft = borderRight = w->w_padx;
        borderY = borderTop = borderBtm = w->w_pady;

ED_TRACE(("    +   cavity\n"))
ED_TRACE(("            x, y    %d, %d\n", cavityX, cavityY))
ED_TRACE(("            c, r    %d, %d\n", cavityCols, cavityRows))
ED_TRACE(("        frame\n"))
ED_TRACE(("            x, y    %d, %d\n", frameX, frameY))
ED_TRACE(("            c, r    %d, %d\n", frameCols, frameRows))
ED_TRACE(("        border\n"))
ED_TRACE(("            x, y    %d, %d\n", borderX, borderY))
ED_TRACE(("            l, r    %d, %d\n", borderLeft, borderRight))
ED_TRACE(("            t, b    %d, %d\n", borderTop, borderBtm))

        cols = ReqCols(w) + (w->w_ipadx * 2);
        rows = ReqRows(w) + (w->w_ipady * 2);

        if (cols > (frameCols - borderX)) {
ED_TRACE(("        limiting-x (%d)\n", rows))
            cols = frameCols - borderX;
        } else if (WIDGET_FFILLX & w->w_flags) {
ED_TRACE(("        filling-x (%d)\n", rows))
            cols = frameCols - borderX;
        }

        if (rows > (frameRows - borderY)) {
ED_TRACE(("        limiting-y (%d)\n", rows))
            rows = frameRows - borderY;
        } else if (WIDGET_FFILLY & w->w_flags) {
ED_TRACE(("         filling-y (%d)\n", rows))
            rows = frameRows - borderY;
        }

ED_TRACE(("    +   cols, rows  %d, %d\n", cols, rows))
ED_TRACE(("        anchor      0x%x\n", w->w_align))

        switch (w->w_align) {
        case DLGA_ALIGN_N:
            x = frameX + (borderLeft + frameCols - cols - borderRight) / 2;
            y = frameY + borderTop;
            break;
        case DLGA_ALIGN_NE:
            x = frameX + frameCols - cols - borderRight;
            y = frameY + borderTop;
            break;
        case DLGA_ALIGN_E:
            x = frameX + frameCols - cols - borderRight;
            y = frameY + (borderTop + frameRows - rows - borderBtm) / 2;
            break;
        case DLGA_ALIGN_SE:
            x = frameX + frameCols - cols - borderRight;
            y = frameY + frameRows - rows - borderBtm;
            break;
        case DLGA_ALIGN_S:
            x = frameX + (borderLeft + frameCols - cols - borderRight) / 2;
            y = frameY + frameRows - rows - borderBtm;
            break;
        case DLGA_ALIGN_SW:
            x = frameX + borderLeft;
            y = frameY + frameRows - rows - borderBtm;
            break;
        case DLGA_ALIGN_W:
            x = frameX + borderLeft;
            y = frameY + (borderTop + frameRows - rows - borderBtm) / 2;
            break;
        case DLGA_ALIGN_NW:
            x = frameX + borderLeft;
            y = frameY + borderTop;
            break;
        case DLGA_ALIGN_CENTER:     /* default */
            x = frameX + (borderLeft + frameCols - cols - borderRight) / 2;
            y = frameY + (borderTop + frameRows - rows - borderBtm) / 2;
            break;
        default:
            panic("bad frame factor in ArrangePacking");
            break;
        }
        cols -= Border(w);
        rows -= Border(w);

        /*
         *  Set the position, size, and state of the child.
         */
ED_TRACE(("    = Final size\n"))
ED_TRACE(("            x, y    %d, %d\n", x, y))
ED_TRACE(("            c, r    %d, %d\n", cols, rows))

        if (cols <= 0 || rows <= 0) {
ED_TRACE(("            is HIDDEN\n"))
            w->w_uflags |= WTTY_FHIDDEN;
        } else {
            Resize(w, x, y, cols, rows);
            if (c->cx_packerr) {
                return;
            }
            w->w_uflags &= ~WTTY_FHIDDEN;
        }
    }
}


/*  Function:           pack_expandx
 *      Given a list of packed slaves, the first of which is packed on the left or
 *      right and is expandable, compute how much to expand the child.
 *
 *  Parameters:
 *       w - Widget reference.
 *       cavityCols - Size of the cavity.
 *
 *  Results:
 *      The return value is the number of additional pixels to give to the child.
 */
static int
pack_expandx(WIDGET_t *w, int cavityCols)
{
    int numExpand, minExpand, curExpand;
    int childCols;

    /*
     *  This procedure is tricky because windows packed top or bottom can be
     *  interspersed among expandable windows packed left or right. Scan through the
     *  list, keeping a running sum of the widths of all left and right windows
     *  (actually, count the cavity space not allocated) and a running count of all
     *  expandable left and right windows. At each top or bottom window, and at the end
     *  of the list, compute the expansion factor that seems reasonable at that point.
     *  Return the smallest factor seen at any of these points.
     */
    minExpand = cavityCols;
    numExpand = 0;
    for (; NULL != w; w = TAILQ_NEXT(w, w_sibling) ) {
        childCols = ReqCols(w) + PaddingCols(w);
        if ((DLGA_ATTACH_TOP == w->w_attach) || (DLGA_ATTACH_BOTTOM == w->w_attach)) {
            curExpand = (cavityCols - childCols)/numExpand;
            if (curExpand < minExpand) {
                minExpand = curExpand;
            }
        } else {
            cavityCols -= childCols;
            if (WIDGET_FEXPAND & w->w_flags) {
                ++numExpand;
            }
        }
    }

    curExpand = cavityCols/numExpand;
    if (curExpand < minExpand) {
        minExpand = curExpand;
    }
    return (minExpand < 0) ? 0 : minExpand;
}


/*  Function:           pack_expandy
 *      Given a list of packed slaves, the first of which is packed on the top or
 *      bottom and is expandable, compute how much to expand the child.
 *
 *  Parameters:
 *       w - Widget reference.
 *       cavityRows - Size of the cavity.
 *
 *  Results:
 *      The return value is the number of additional pixels to give to the child.
 */
static int
pack_expandy(WIDGET_t *w, int cavityRows)
{
    int numExpand, minExpand, curExpand;
    int childRows;

    /*
     *  See comments for pack_expandx.
     */
    minExpand = cavityRows;
    numExpand = 0;
    for (; NULL != w; w = TAILQ_NEXT(w, w_sibling) ) {
        childRows = ReqRows(w) + PaddingRows(w);
        if ((DLGA_ATTACH_LEFT == w->w_attach) || (DLGA_ATTACH_RIGHT == w->w_attach)) {
            curExpand = (cavityRows - childRows)/numExpand;
            if (curExpand < minExpand) {
                minExpand = curExpand;
            }
        } else {
            cavityRows -= childRows;
            if (WIDGET_FEXPAND & w->w_flags) {
                ++numExpand;
            }
        }
    }

    curExpand = cavityRows/numExpand;
    if (curExpand < minExpand) {
        minExpand = curExpand;
    }
    return (minExpand < 0) ? 0 : minExpand;
}

/*end*/
