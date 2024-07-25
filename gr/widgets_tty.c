#include <edidentifier.h>
__CIDENT_RCSID(gr_widgets_tty_c,"$Id: widgets_tty.c,v 1.41 2024/07/25 15:39:11 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: widgets_tty.c,v 1.41 2024/07/25 15:39:11 cvsuser Exp $
 * Dialog widgets, tty interface.
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
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "builtin.h"
#include "color.h"                              /* ATTR_... */
#include "cmap.h"
#include "debug.h"
#include "dialog.h"
#include "dialog_tty.h"
#include "display.h"
#include "kill.h"
#include "line.h"
#include "lisp.h"
#include "mchar.h"
#include "main.h"

#include "tty.h"
#include "widgets.h"
#include "widgets_tty.h"
#include "window.h"
#include "word.h"


/*
 *  Attribute mapping.
 */
#define NORMAL              ATTR_DIALOG_NORMAL
#define FOCUS               ATTR_DIALOG_FOCUS
#define HILITE              ATTR_DIALOG_HILITE
#define GREYED              ATTR_DIALOG_GREYED
#define TITLE               ATTR_DIALOG_TITLE
#define FRAME               ATTR_DIALOG_FRAME

#define HOTKEY_NORMAL       ATTR_DIALOG_HOTKEY_NORMAL
#define HOTKEY_FOCUS        ATTR_DIALOG_HOTKEY_FOCUS

#define BUTTON_GREYED       ATTR_DIALOG_BUT_GREYED
#define BUTTON_NORMAL       ATTR_DIALOG_BUT_NORMAL
#define BUTTON_FOCUS        ATTR_DIALOG_BUT_FOCUS
#define BUTTON_KEY_NORMAL   ATTR_DIALOG_BUT_KEY_NORMAL
#define BUTTON_KEY_FOCUS    ATTR_DIALOG_BUT_KEY_FOCUS

#define EDIT_GREYED         ATTR_DIALOG_EDIT_GREYED
#define EDIT_NORMAL         ATTR_DIALOG_EDIT_NORMAL
#define EDIT_FOCUS          ATTR_DIALOG_EDIT_FOCUS
#define EDIT_COMPLETE       ATTR_DIALOG_EDIT_COMPLETE


/*  Package:            widgets_tty
 *  -----------------------------------------------------------------------
 *
 *  Standard keyboard navigation keys for widgets (following GNOME)

        Key                         Function

        Tab,Shift+Tab               Moves keyboard focus to the next/previous control.

        Ctrl+Tab,                   Moves keyboard out of enclosing widget to next/previous
        Shift+Ctrl+Tab              control, in those situations where Tab alone has
                                    another function.

        Ctrl+F1                     Popup tooltip for currently-focused control.

        Shift+F1                    Show context-sensitive help for the currently-focused
                                    window or control.

        F6, Shift+F6                Give focus to next/previous pane in a GtkPaned window.

        F8                          Give focus to splitter bar in paned window.

        F10                         Give focus to window's menu bar.

        Shift+F10                   Popup contextual menu for currently-select widgets.

        Space                       Toggle selected state of focused check-box, radio-button
                                    or toggle-button.

        Home, End                   Select/move to first/last item in selected widget.

        PageUp, PageDown            Scroll selected view by one page up/down.

**/


/*  Function:           tty_new
 *      Allocate and initialise a widget.
 *
 *  Parameters:
 *      size -              Size of derived widget in bytes.
 *      handler -           Address of the widget message handler.
 *
 *  Returns:
 *      Address of the new widget.
 */
WIDGET_t *
tty_new(uint32_t size, WIDGETCB_t handler)
{
    WIDGET_t *w;

    if (NULL != (w = chk_calloc(size, 1))) {
        widget_init(w, size, handler);
    }
    return (w);
}


/*  Function:           tty_default
 *      Default tty widget message handler.
 *
 *  Parameters:
 *      w -                 Widget base object.
 *      msg -               Message to tbe sent.
 *      p1 -                First message parameter.
 *      p2 -                Second message parameter.
 *
 *  Returns:
 *      FALSE(0) if the message wasn't handled, otherwise a non-zero completion code.
 *      Unhandled message are forwarded to the generic widget message handler.
 */
static int
tty_default(WIDGET_t *w, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    switch (msg) {
    case WIDGET_SETFOCUS:       /* cursor/focus request */
        if ((WIDGET_FGREYED|WIDGET_FHIDDEN) & w->w_flags) {
            return FALSE;
        }
        return TRUE;

    case WIDGET_SET:            /* set attribute */
        switch (DIALOGARGLO(p1)) {
        case DLGA_HIDDEN:
            if (0 == (WIDGET_FHIDDEN & w->w_flags)) {
                if (w->w_parent) {
                    w->w_parent->w_uflags |= WTTY_FDIRTY|WTTY_FCLEAR;
                }
                w->w_uflags |= WTTY_FDIRTY;
            }
            break;
        case DLGA_VISIBLE:
            if (0 != (WIDGET_FHIDDEN & w->w_flags)) {
                if (w->w_parent) {
                    w->w_parent->w_uflags |= WTTY_FDIRTY;
                }
                w->w_uflags |= WTTY_FDIRTY;
            }
            break;
        }
        break;

    case WIDGET_HOTKEY: {       /* hotkey action */
            const int32_t hotkey = w->w_hotkey;
            const int32_t key = (int32_t)p1;

            if (hotkey) {
                if (key == hotkey) {
                    return TRUE;
                }

                if ((hotkey > 0 && hotkey < 127) && (MOD_META & key)) {
                    const int t_hotkey = tolower(hotkey);
                    const int t_key = tolower(key & 0xff);

                    if (t_hotkey == t_key) {
                        return TRUE;
                    }
                }
            }
        }
        break;
    }
    return widget_default(w, msg, p1, p2);
}


/*  Function:           tty_exit
 *      Exit the tty dialog manager.
 *
 *  Parameters:
 *      w -                 Widget base object.
 *
 *  Returns:
 *      nothing.
 */
static void
tty_exit(WIDGET_t *w)
{
    w->w_root->d_running = 1;
}


/*  Function:           tty_move
 *      Position the tty cursor, relative to the specified widget.
 *
 *  Parameters:
 *      w -                 Widget base object.
 *      x, y -              Screen position.
 *
 *  Returns:
 *      nothing.
 */
static void
tty_move(WIDGET_t *w, int x, int y)
{
    *cur_line = w->w_absy + y + 1;
    *cur_col = w->w_absx + x + 1;
    assert(*cur_line >= 1);
    assert(*cur_col >= 1);
}


static void
tty_absmove(WIDGET_t *w, int x, int y)
{
    __CUNUSED(w)
    *cur_line = y + 1;
    *cur_col = x + 1;
    assert(*cur_line >= 1);
    assert(*cur_col >= 1);
}


/*  Function:           tty_strn
 *      Output a string at the current cursor position.
 *
 *  Parameters:
 *      attr -
 *      str -
 *      len -
 *
 *  Returns:
 *      nothing.
 */
static void
tty_strn(LINEATTR attr, const char *str, int len)
{
    const LINEATTR oattr = *cur_attr;
    *cur_attr = attr;
    lwrite(str, len, len);
    *cur_attr = oattr;
}


/*  Function:           tty_str
 *      Output a string at the current cursor position.
 *
 *  Parameters:
 *      attr -
 *      str -
 *
 *  Returns:
 *      nothing.
 */
static void
tty_str(LINEATTR attr, const char *str)
{
    tty_strn(attr, str, (int) strlen(str));
}


/*  Function:           tty_char
 *      Output a character at the current cursor position.
 *
 *  Parameters:
 *      attr -
 *      ch -
 *
 *  Returns:
 *      int - Character display width.
 */
static int
tty_char(LINEATTR attr, int ch)
{
    const LINEATTR oattr = *cur_attr;
    *cur_attr = attr;
    lwritec(ch);
    *cur_attr = oattr;
    return  Wcwidth(ch);
}


static __CINLINE int
tty_isunicode(int special)
{
    __CUNUSED(special)
#if defined(WIN32) || defined(__CYGWIN__)
    if (special)        /* FIXME; generally aren't available. */
        return FALSE;
#endif
    return (DC_UNICODE == ((DC_UNICODE|DC_ASCIIONLY) & x_display_ctrl));
}


/*  Function:           tty_border
 *      Output a box at the given coordinates.
 *
 *  Parameters:
 *      w -
 *      attr -
 *      x -
 *      y -
 *      rows -
 *      cols -
 *      vscroll -           *true* if vertical scroll-bars should be displayed.
 *
 *  Returns:
 *      nothing.
 *
 *  UNICODE Specials:
 *
 *      0x256D          BOX DRAWING LIGHT ARC DOWN AND RIGHT.
 *      0x256E          BOX DRAWING LIGHT ARC DOWN AND LEFT.
 *      0x256F          BOX DRAWING LIGHT ARC UP AND LEFT.
 *      0x2570          BOX DRAWING LIGHT ARC UP AND RIGHT.
 */
static void
tty_border(WIDGET_t *w, LINEATTR attr, int x, int y, int rows, int cols, int vscroll)
{
    const int unicode = tty_isunicode(FALSE);
    int i;

    --rows, --cols;

    tty_move(w, x, y);
    tty_char(attr, (unicode ? CH_TOP_LEFT2   : CH_TOP_LEFT));
    for (i = 1; i < cols; ++i) {
        tty_move(w, x+i, y);
        tty_char(attr, CH_HORIZONTAL);
    }
    tty_move(w, x+cols, y);
    tty_char(attr, (unicode ? CH_TOP_RIGHT2  : CH_TOP_RIGHT));

    for (i = 1; i < rows; ++i) {
        tty_move(w, x, y+i);
        tty_char(attr, CH_VERTICAL);
        tty_move(w, x+cols, y+i);
        tty_char(attr, (vscroll ? CH_VSCROLL : CH_VERTICAL));
    }

    tty_move(w, x, y+rows);
    tty_char(attr, (unicode ? CH_BOT_LEFT2   : CH_BOT_LEFT));
    for (i = 1; i < cols; ++i) {
        tty_move(w, x+i, y+rows);
        tty_char(attr, CH_HORIZONTAL);
    }
    tty_move(w, x+cols, y+rows);
    tty_char(attr, (unicode ? CH_BOT_RIGHT2  : CH_BOT_RIGHT));
}


/*  Group:              Button Support
 *  -------------------------------------------------------------------------------------
 *
 *      Support functionality for use with all button types, push, radio and check.
 *
**/
typedef struct {
    const char *        text;                   /* text buffer */
    char                hotkey;                 /* hotkey value */
    int                 offset;                 /* hilite offset within text */
    int                 lalign;
    int                 position;
    int                 multiline;
} WButtonText_t;

static void             bt_free(WButtonText_t *b);


/*  Function:           bt_text
 *      Parse button text, dealing with any embedded hotkey definition.
 *
 *      A hot-key is an hilited character in the text of a menu, menu item, or the
 *      label of a control such as a button. With a hot-key users can "click" a button
 *      by utilising the ALT key in combination with the defined hotkey.
 *
 *      For example, for a button which formats the content on a window captioned as
 *      "Format" adding an ampersand before the letter "F" causes the letter "F" to be
 *      hilited in the button text at run time. The user can run the command associated
 *      with the button by pressing ALT+F. You cannot have an access key for a control
 *      that cannot receive focus.
 *
 *      To include an ampersand in a caption without creating a hot-key, include two
 *      ampersands (&&). A single ampersand is displayed in the caption and no
 *      characters are underlined.
 *
 *  Parameters:
 *      b -                 Button object.
 *      text -              Text value.
 *
 *  Returns:
 *      nothing.
 */
static void
bt_text(WButtonText_t *b, const char *text)
{
    bt_free(b);

    if (NULL != (b->text =
            (text && *text ? chk_salloc((void *)text) : NULL))) {
        char *h;

        b->hotkey = 0;
        b->offset = 0;
        if (NULL != (h = strchr(b->text, '&')) && h[1] != '\0') {
            str_cpy(h, (const char *)(h + 1));  /* remove '&' marker */
            if ('&' != *h) {
                b->hotkey = (char)tolower(*h);
                b->offset = h - b->text;
            }
        }
        b->multiline = (NULL != strchr(b->text, '\n') ? 1 : 0);
        b->position = 0;
        b->lalign = 0;
    }
}


static void
bt_data(WButtonText_t *b, const WIDGETDATA_t *data)
{
    const char *s;

    if (NULL == (s = widgetdata_str(data))) {
        const int32_t v = widgetdata_int32(data);
        char ibuf[32];

        sxprintf(ibuf, sizeof(ibuf), "%d", (int)v);
        bt_text(b, ibuf);
        return;
    }
    bt_text(b, s);
}


/*  Function:           bt_free
 *      Clear any associated button text.
 *
 *  Parameters:
 *      b -                 Button text.
 *
 *  Returns:
 *      nothing.
 */
static void
bt_free(WButtonText_t *b)
{
    chk_free((void *)b->text), b->text = NULL;
    b->hotkey = 0;
    b->offset = 0;
}


/*  Function:           bt_ishotkey
 *      Check the specified buttons for hotkey matches.
 *
 *  Parameters:
 *      key -
 *      b -                 Button text.
 *      count -
 *
 *  Returns:
 *      nothing.
 */
static int
bt_ishotkey(uint32_t key, const WButtonText_t *b, int count)
{
    if (MOD_META & key) {                       /* ASCII hot key */
        unsigned char t_key = (unsigned char)(key & 0xff);
        int i;

        if (t_key > 0 && (t_key = (unsigned char)tolower(t_key)) > 0)
            for (i = 0; i < count; ++i, ++b) {
                if (b->text && b->hotkey == t_key) {
                    return i;
                }
            }
    }
    return -1;
}


/*
 *  Group:  Container
**/
typedef struct {
    WIDGET_t            cn_widget;
} WContainer_t;

static int              container_callback(WContainer_t *g, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
    container_new(void)
{
    return tty_new(sizeof(WContainer_t), (WIDGETCB_t)container_callback);
}


static int
container_callback(WContainer_t *wg, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)wg;

    switch (msg) {
    case WIDGET_INIT:           /* initialise */
        if (WIDGET_FTABNOT & w->w_flags) {
            const uint32_t containerid = w->w_containerid;
            DIALOG_t *d = w->w_root;
            WIDGET_t *cursor, *start;

            if (NULL != (cursor = (start = dialog_next(d, NULL)))) {
                do {
                    if (w != cursor && containerid == cursor->w_containerid) {
                        cursor->w_flags |= WIDGET_FTABNOT;
                    }
                } while ((cursor = dialog_next(d, cursor)) != start);
            }
        }
        break;

    case WIDGET_SET: {          /* set attribute */
#if defined(TODO)
            switch (DIALOGARGLO(p1)) {
            case DLGA_TABSTOP: {
                    const uint32_t containerid = w->w_containerid;
                    DIALOG_t *d = w->w_root;
                    WIDGET_t *cursor, *start;

                    if (NULL != (cursor = (start = dialog_next(d, NULL))))
                        do {
                            if (w != cursor && containerid == cursor->w_containerid) {
                                tty_default(cursor, msg, p1, p2);
                            }
                        } while ((cursor = dialog_next(d, cursor)) != start);
                }
                break;
            }
#endif
        }
        break;

    case WIDGET_SETFOCUS:       /* cursor/focus request */
        return FALSE;
    }
    return tty_default(w, msg, p1, p2);
}


/*
 *  Group:  Group
**/
typedef struct {
    WIDGET_t            gr_widget;
    const char *        gr_title;
} WGroup_t;

typedef struct {
    int                 gf_row;
    int                 gf_col;
    int                 gf_direction;
    WIDGET_t *          gf_find;
} WGroupFind_t;

static int              group_callback(WGroup_t *g, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
group_new(void)
{
    return tty_new(sizeof(WGroup_t), (WIDGETCB_t)group_callback);
}


static int
group_find(const WIDGET_t *group, int direction)
{
    const uint32_t groupid = group->w_groupid;
    DIALOG_t *d = group->w_root;
    const WIDGET_t *current = dialog_tty_current(d);
    const WIDGET_t *start = dialog_next(d, NULL);
    WIDGET_t *w, *best = NULL;

    trace_ilog("group_find(id:%u, x:%2d, y:%2d, direction:%d\n", (unsigned)groupid, group->w_absx, group->w_absy, direction);
    trace_ilog("\t= current id:%u, x:%2d, y:%2d\n", current->w_groupid, current->w_absx, current->w_absy);

    if (NULL != (w = (WIDGET_t *)start)) {
        do {
            if (groupid == w->w_groupid &&
                    w != current && 0 == ((WIDGET_FGREYED|WIDGET_FHIDDEN) & w->w_flags)) {
                switch (w->w_class) {
                case DLGC_GROUP:
                case DLGC_CONTAINER:
                case DLGC_TAB:
                case DLGC_SPACER:
                    break;

                default:
                    switch (direction) {
                    case KEY_LEFT:
                        if (current->w_absy == w->w_absy && w->w_absx < current->w_absx &&
                                (NULL == best || w->w_absx > best->w_absx)) {
                            best = w;
                        }
                        break;
                    case KEY_RIGHT:
                        if (current->w_absy == w->w_absy && w->w_absx > current->w_absx &&
                                (NULL == best || w->w_absx < best->w_absx)) {
                            best = w;
                        }
                        break;
                    case KEY_UP:
                        if (current->w_absx == w->w_absx && w->w_absy < current->w_absy &&
                                (NULL == best || w->w_absy > best->w_absy)) {
                            best = w;
                        }
                        break;
                    case KEY_DOWN:
                        if (current->w_absx == w->w_absx && w->w_absy > current->w_absy &&
                                (NULL == best || w->w_absy < best->w_absy)) {
                            best = w;
                        }
                        break;
                    }
                }
            }
        } while ((w = dialog_next(d, w)) != start);
    }

    if (best) {
        trace_ilog("\t=> best x:%2d, y:%2d (%s)\n", best->w_absx, best->w_absy, best->w_desc);
    }
    return (best ? dialog_tty_setfocus(d, best) : FALSE);
}


static int
group_callback(WGroup_t *g, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)g;

    switch (msg) {
    case WIDGET_INIT:           /* Initialise */
        w->w_border = 1;
        w->w_flags |= WIDGET_FKEYPARENT;
        break;

    case WIDGET_READY:          /* Dialog session ready */
        break;

    case WIDGET_SET: {          /* Set attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            w->w_uflags |= WTTY_FDIRTY;
            switch (DIALOGARGLO(p1)) {
            case DLGA_TITLE: {
                    const char *title = widgetdata_str(data);

                    chk_free((void *)g->gr_title);
                    g->gr_title = (title && *title ? chk_salloc(title) : NULL);
                }
                break;
            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_KEYDOWN:        /* Keyboard event */
        switch (p1) {
        case KEY_LEFT:
        case KEY_RIGHT:
        case KEY_UP:
        case KEY_DOWN:
            return group_find(w, p1);
        }
        break;

    case WIDGET_SETFOCUS:       /* Cursor/focus request */
        return FALSE;

    case WIDGET_PAINT:          /* Widget display */
        if (w->w_border) {
            tty_move(w, 0, 0);
            tty_border(w, FRAME, 0, 0, w->w_rows + 2, w->w_cols + 2, FALSE);
            if (g->gr_title) {
                tty_move(w, 1, 0);
                tty_str(TITLE, g->gr_title);
            }
        }
        return TRUE;

    case WIDGET_DESTROY:        /* Destructor */
        chk_free((void *)g->gr_title), g->gr_title = NULL;
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}


/*
 *  Group:  Tab
**/
typedef struct {
    WIDGET_t            tb_widget;
    const char *        tb_title;
} WTab_t;

static int              tab_callback(WTab_t *g, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
tab_new(void)
{
    return tty_new(sizeof(WTab_t), (WIDGETCB_t)tab_callback);
}


static int
tab_callback(WTab_t *g, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)g;

    switch (msg) {
    case WIDGET_INIT:           /* Initialise */
        w->w_flags |= WIDGET_FKEYPARENT;
        break;

    case WIDGET_READY:          /* Dialog session ready */
        w->w_uflags &= ~WTTY_FCLEAR;
        break;

    case WIDGET_SET: {          /* Set attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            w->w_uflags |= WTTY_FDIRTY;
            switch (DIALOGARGLO(p1)) {
            case DLGA_TITLE: {
                    const char *title = widgetdata_str(data);

                    chk_free((void *)g->tb_title);
                    g->tb_title = (title && *title ? chk_salloc(title) : NULL);
                }
                break;

            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_PAINT:          /* Widget display */
        if (0 == (WIDGET_FHIDDEN & w->w_flags)) {
            if (WTTY_FCLEAR & w->w_uflags) {
                const int cols = w->w_cols, rows = w->w_rows;
                int x, y;

                for (y = 0; y < rows; ++y) {
                    for (x = 0; x < cols; ++x) {
                        tty_move(w, x, y);
                        tty_char(NORMAL, ' ');
                    }
                }
                w->w_uflags &= ~WTTY_FCLEAR;
            }

            if (g->tb_title) {
                tty_move(w, 0, 0);
                tty_str(TITLE, g->tb_title);
            }
        }
        break;

    case WIDGET_SETFOCUS:       /* Cursor/focus request */
        return FALSE;

    case WIDGET_DESTROY:        /* Destructor */
        chk_free((void *)g->tb_title), g->tb_title = NULL;
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}


/*
 *  Group:  Spacer
**/
typedef struct {
    WIDGET_t            sp_widget;
    int                 sp_type;
} WSeparator_t;

static WIDGET_t *       separator_new(int type);
static int              separator_callback(WSeparator_t *s, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
spacer_new(void)
{
    return separator_new(' ');
}


WIDGET_t *
separatorh_new(void)
{
    return separator_new('h');
}


WIDGET_t *
separatorv_new(void)
{
    return separator_new('v');
}


static WIDGET_t *
separator_new(int type)
{
    WSeparator_t *sp;

    if (NULL == (sp = (WSeparator_t *)tty_new(sizeof(WSeparator_t), (WIDGETCB_t)separator_callback))) {
        return NULL;
    }
    sp->sp_type = type;
    return (WIDGET_t *)sp;
}


static int
separator_callback(WSeparator_t *sp, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)sp;

    switch (msg) {
    case WIDGET_INIT:           /* Initialise */
        if (w->w_reqrows > 0 || w->w_reqcols > 0) {
            if (0 == w->w_reqrows) w->w_reqrows = 1;
            if (0 == w->w_reqcols) w->w_reqcols = 1;
        }
        break;

    case WIDGET_SETFOCUS:       /* Cursor/focus request */
        return FALSE;

    case WIDGET_CARET:          /* Cursor selection */
        return FALSE;

    case WIDGET_PAINT:          /* Widget display */
        if (0 == (WIDGET_FHIDDEN & w->w_flags)) {
            const int cols = w->w_cols, rows = w->w_rows;

            switch (sp->sp_type) {
            case ' ':
                break;
            case 'h': {
                    int x;

                    for (x = 0; x < cols; ++x) {
                        tty_move(w, x, 0);
                        tty_char(NORMAL, CH_HORIZONTAL);
                    }
                }
                break;
            case 'v': {
                    int y;

                    for (y = 0; y < rows; ++y) {
                        tty_move(w, 0, y);
                        tty_char(NORMAL, CH_VERTICAL);
                    }
                }
                break;
            }
        }
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}


/*
 *  Group:  Push Buttons
**/
#define PBBORDER            2                   /* 2 + 2 */

typedef struct {
    WIDGET_t            pb_widget;              /* base widget */
    WButtonText_t       pb_button;              /* label */
    uint32_t            pb_flags;               /* type of button */
#define PBDEFAULT           0x01
#define PBCANCEL            0x02

} WPushButton_t;

static int              pushbutton_callback(WPushButton_t *b, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
pushbutton_new(void)
{
    return tty_new(sizeof(WPushButton_t), (WIDGETCB_t)pushbutton_callback);
}


static int
pushbutton_lalign(int len, int width)
{
    if (width > len) {
        return (width - len)/2;
    }
    return 0;
}


static int
pushbutton_callback(WPushButton_t *b, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)b;

    switch (msg) {
    case WIDGET_INIT:           /* Initialise */
        if (b->pb_button.text) {
            w->w_padx    = 1;
            w->w_reqrows = 2;                   /* space vertical */
            w->w_reqcols = (PBBORDER * 2) + strlen(b->pb_button.text);
            w->w_flags  |= WIDGET_FTABSTOP;
        }
        return TRUE;

    case WIDGET_READY:          /* Dialog session ready */
        break;

    case WIDGET_SET: {          /* Set attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            w->w_uflags |= WTTY_FDIRTY;
            switch (DIALOGARGLO(p1)) {
            case DLGA_LABEL:
                bt_text(&b->pb_button, widgetdata_str(data));
                break;
            case DLGA_CANCEL_BUTTON:            /* <Esc> */
                if (NULL == b->pb_button.text) {
                    bt_text(&b->pb_button, (const char *)"Cancel");
                }
                b->pb_flags |= PBCANCEL;
                break;
            case DLGA_DEFAULT_BUTTON:           /* <Enter> */
                b->pb_flags |= PBDEFAULT;
                break;
            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_MOUSE:          /* Mouse event */
        if (BUTTON1_DOWN == p2) {
            widget_send(w, WIDGET_KEYDOWN, KEY_ENTER, 0);
        }
        return TRUE;

    case WIDGET_KEYDOWN:        /* Keyboard event */
        if (KEY_ENTER == p1) {
            if (! widget_callback(w, DLGE_BUTTON, 0, 0)) {
                if (PBCANCEL & b->pb_flags) {
                    tty_exit(w);
                }
            }
            return TRUE;
        }
        break;

    case WIDGET_HOTKEY: {       /* Accelerator/hotkey event */
            const WButtonText_t *bt = &b->pb_button;
            const int ishotkey = bt_ishotkey((uint32_t) p1, bt, 1);

            if ((KEY_ENTER == p1 || ishotkey >= 0) && (PBDEFAULT & b->pb_flags)) {
                /*
                 *  PBDEFAULT ---
                 *      This property allows the designation of a default action to occur when
                 *      the user presses the ENTER key whilst within the dialog, even when the
                 *      button does not have the input focus.
                 *
                 *      This property is useful for enabling the user to quickly select the most
                 *      likely (default) option.
                 */
                if (! widget_callback(w, DLGE_BUTTON, 1, 0)) {
                    tty_exit(w);
                }
                return TRUE;

            } else if ((KEY_ESC == p1 || ishotkey >= 0) && (PBCANCEL & b->pb_flags)) {
                /*
                 *  PBCANCEL ---
                 *      This property allows the designation of a default action to occur when
                 *      the user presses the ESC key whilst within the dialog.
                 *
                 *      You can use this property to allow the user to quickly navigate a simple
                 *      form by allowing them to simply press the ESC key.
                 */
                if (! widget_callback(w, DLGE_BUTTON, 1, 0)) {
                    tty_exit(w);
                }
                return TRUE;

            } else if (ishotkey >= 0) {
                /*
                 *  HOTKEY match
                 */
                widget_callback(w, DLGE_BUTTON, 2, 0);
                return TRUE;
            }
        }
        return tty_default(w, msg, p1, p2);

    case WIDGET_COMMAND:        /* Accelerator */
        widget_callback(w, DLGE_COMMAND, p1, p2);
        return TRUE;

    case WIDGET_PAINT:          /* Widget display */
        if (0 == (WIDGET_FHIDDEN & w->w_flags) && b->pb_button.text) {
            WButtonText_t *bt = &b->pb_button;
            const int width = w->w_cols - (PBBORDER * 2);
            size_t length = strlen(bt->text);
            int ralign;

            const char *fmt;
            char buf[128];                      /* MAGIC */

            /* format the button, center text within arena */
            if ((int)length > width) {
                length = width;
            }
            bt->lalign = pushbutton_lalign(length, width);
            ralign = width - (length + bt->lalign);
            fmt = (PBDEFAULT & b->pb_flags ? "[<%*s%.*s%*s>]" : "[ %*s%.*s%*s ]");
            sxprintf(buf, sizeof(buf), fmt,
                bt->lalign, "", length, bt->text, ralign, "");

            /* display */
            tty_move(w, 0, 1);
            tty_str((HasFocus(w) ? BUTTON_FOCUS : BUTTON_NORMAL), buf);
            if (bt->hotkey) {
                tty_move(w, PBBORDER + bt->lalign + bt->offset, 1);
                tty_char((HasFocus(w) ? BUTTON_KEY_FOCUS : BUTTON_KEY_NORMAL), bt->text[bt->offset]);
            }
        }
        return TRUE;

    case WIDGET_CARET:          /* Cursor selection */
        if (b->pb_button.text) {
            const WButtonText_t *bt = &b->pb_button;

            tty_move(w, PBBORDER + bt->lalign + bt->offset, 1);
        }
        return TRUE;

    case WIDGET_DESTROY:        /* Destructor */
        bt_free(&b->pb_button);
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}


/*
 *  Group:  Radio Buttons
**/
typedef struct {
    WIDGET_t            rb_widget;              /* base widget */
    int                 rb_unicode;             /* active presentation character set */
    int                 rb_count;               /* count of button within exclusive group */
    int                 rb_active;              /* active button */
    int                 rb_cursor;              /* current cursor */

#define RBMAX               20
    WButtonText_t       rb_buttons[RBMAX];      /* button text */
} WRadioButton_t;

static int              radiobutton_callback(WRadioButton_t *b, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
radiobutton_new(void)
{
    return tty_new(sizeof(WRadioButton_t), (WIDGETCB_t)radiobutton_callback);
}


static int
radiobutton_callback(WRadioButton_t *b, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)b;
    int i;

    switch (msg) {
    case WIDGET_INIT:           /* Initialise */
        b->rb_unicode = tty_isunicode(TRUE);

        if (b->rb_count) {
            /*
             *  size the list-box
             */
            const int horiz = w->w_orientation;
            int reqcols = 0;

            if (horiz) {
                w->w_padx = 1;
                w->w_reqrows = 1;
            } else {
                w->w_padx = 1;
                w->w_reqrows = b->rb_count;
                reqcols = 1;
            }

            for (i = 0; i < b->rb_count; ++i) {
                const WButtonText_t *bt = b->rb_buttons + i;
                int len = (int)strlen(bt->text);

                len += (b->rb_unicode ? 1 : 3);
                if (horiz) {                    /* total length */
                    if (reqcols) ++reqcols;
                    reqcols += len + 2;
                } else {                        /* upper limit */
                    if ((len += 1) > reqcols) {
                        reqcols = len;
                    }
                }            }
            w->w_reqcols = reqcols;

            if (w->w_depth <= 1 || TAILQ_FIRST(&w->w_parent->w_children) == w) {
                w->w_flags |= WIDGET_FTABSTOP;
            }
        }
        return TRUE;

    case WIDGET_READY:          /* Dialog session ready */
        break;

    case WIDGET_SET: {          /* Set attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            w->w_uflags |= WTTY_FDIRTY;
            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:
                b->rb_active = widgetdata_uint32(data);
                break;
            case DLGA_LABEL:
                if (b->rb_count < RBMAX) {
                    bt_text(b->rb_buttons + b->rb_count, widgetdata_str(data));
                    ++b->rb_count;
                }
                break;
            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_GET: {          /* Get attribute */
            WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE: {
                    uint16_t attr = DIALOGARGHI(p1);

                    if (0 == attr) {            /* current active index [0...x] */
                        data->d_u.ivalue = b->rb_active;
                    } else {                    /* otherwise ... status of 'attr' */
                        data->d_u.ivalue = (--attr == b->rb_active ? 1 : 0);
                    }
                    data->d_type = D_INT;
                }
                return TRUE;

            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        break;

    case WIDGET_MOUSE:          /* Mouse event */
        if (p2 == BUTTON1_DOWN) {
            const int horiz = w->w_orientation;

            if (horiz) {
                /*TODO*/;
            } else {
                b->rb_cursor = GetYParam(p1);
                radiobutton_callback(b, WIDGET_KEYDOWN, ' ', 0);
            }
        }
        return TRUE;

    case WIDGET_KEYDOWN:        /* Keyboard event */
        if (b->rb_count) {
            switch (p1) {
            case ' ':
                b->rb_active = b->rb_cursor;
                widget_send(w, WIDGET_PAINT, 0, 0);
                widget_callback(w, DLGE_CHANGE, b->rb_active, 0);
                return TRUE;

            case KEY_LEFT:
            case KEY_UP:
            case WHEEL_UP:
                if (b->rb_cursor > 0) {
                    --b->rb_cursor;
                    return TRUE;
                }
                break;

            case KEY_RIGHT:
            case KEY_DOWN:
            case WHEEL_DOWN:
                if (b->rb_count - 1 > b->rb_cursor) {
                    ++b->rb_cursor;
                    return TRUE;
                }
                break;
            }
        }
        break;

    case WIDGET_HOTKEY: {       /* Accelerator/hotkey event */
            const WButtonText_t *bt = b->rb_buttons;
            int ishotkey = bt_ishotkey((uint32_t) p1, bt, b->rb_count);

            if (ishotkey >= 0) {
                b->rb_cursor = ishotkey;
                radiobutton_callback(b, WIDGET_KEYDOWN, ' ', 0);
                return TRUE;
            }
        }
        return tty_default(w, msg, p1, p2);

    case WIDGET_PAINT:          /* Widget display */
        if (0 == (w->w_flags & WIDGET_FHIDDEN) && b->rb_count) {
            const int horiz = w->w_orientation;
            int offset = 0;
            char buf[20];

            for (i = 0; i < b->rb_count; ++i) { /* display radio group */
                WButtonText_t *bt = b->rb_buttons + i;

                if (horiz) {
                    const size_t len = strlen(bt->text);

                    bt->position = offset;
                    tty_move(w, offset, 0);
                    if (b->rb_unicode) {        /* MCHAR */
                        tty_char(NORMAL, (i == b->rb_active ? CH_RADIO_ON : CH_RADIO_OFF));
                        offset += 2;
                    } else {
                        sprintf(buf, "(%c) ", (i == b->rb_active ? '*' : ' '));
                        tty_str(NORMAL, buf);
                        offset += 4;
                    }
                    tty_move(w, offset, 0);
                    tty_str((i == b->rb_cursor && HasFocus(w) ? FOCUS : NORMAL), bt->text);
                    if (bt->hotkey) {
                        tty_move(w, offset + bt->offset, 0);
                        tty_char((HasFocus(w) ? HOTKEY_FOCUS : HOTKEY_NORMAL), bt->text[bt->offset]);
                    }
                    offset += len + 2;

                } else {
                    bt->position = i;

                    tty_move(w, 0, i);
                    if (b->rb_unicode) {        /* MCHAR */
                        tty_char(NORMAL, (i == b->rb_active ? CH_RADIO_ON : CH_RADIO_OFF));
                        offset = 2;
                    } else {
                        sprintf(buf, "(%c) ", (i == b->rb_active ? '*' : ' '));
                        tty_str(NORMAL, buf);
                        offset = 4;
                    }
                    tty_move(w, offset, i);
                    tty_str((i == b->rb_cursor && HasFocus(w) ? FOCUS : NORMAL), bt->text);
                    if (bt->hotkey) {
                        tty_move(w, offset + bt->offset, i);
                        tty_char((HasFocus(w) ? HOTKEY_FOCUS : HOTKEY_NORMAL), bt->text[bt->offset]);
                    }
                }
            }
        }
        return TRUE;

    case WIDGET_CARET:          /* Cursor selection */
        if (b->rb_count) {
            const int horiz = w->w_orientation;
            const int offset = (b->rb_unicode ? 0 : 1);

            if (horiz) {
                tty_move(w, b->rb_buttons[b->rb_cursor].position + offset, 0);
            } else {
                tty_move(w, offset, b->rb_buttons[b->rb_cursor].position);
            }
            return TRUE;
        }
        break;

    case WIDGET_DESTROY:        /* Destructor */
        for (i = 0; i < b->rb_count; ++i) {
            bt_free(b->rb_buttons + i);
        }
        b->rb_count = 0;
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}


/*
 *  Group:  Check Box
**/
typedef struct {
    WIDGET_t            cb_widget;              /* base widget */
    int                 cb_unicode;             /* active presentation character set */
    int                 cb_value;               /* current value */
    WButtonText_t       cb_text;                /* button text */
} WCheckBox_t;

static int              checkbox_callback(WCheckBox_t *b, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
checkbox_new(void)
{
    return tty_new(sizeof(WCheckBox_t), (WIDGETCB_t)checkbox_callback);
}


static int
checkbox_callback(WCheckBox_t *b, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)b;

    switch (msg) {
    case WIDGET_INIT:           /* Initialise */
        b->cb_unicode = tty_isunicode(TRUE);

        if (b->cb_text.text) {
            w->w_reqrows = 1;
            w->w_reqcols = (b->cb_unicode ? 2 : 4) + strlen(b->cb_text.text);

            if (w->w_depth <= 1 || TAILQ_FIRST(&w->w_parent->w_children) == w) {
                w->w_flags |= WIDGET_FTABSTOP;
            }
        }
        return TRUE;

    case WIDGET_READY:          /* Dialog session ready */
        break;

    case WIDGET_SET: {          /* Set attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            w->w_uflags |= WTTY_FDIRTY;
            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:
                b->cb_value = widgetdata_uint32(data);
                break;
            case DLGA_LABEL:
                bt_text(&b->cb_text, widgetdata_str(data));
                break;
            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_GET: {          /* Get attribute */
            WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:
                data->d_u.ivalue = b->cb_value;
                data->d_type = D_INT;
                return TRUE;
            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        break;

    case WIDGET_MOUSE:          /* Mouse event */
        if (p2 != BUTTON1_DOWN) {
            break;
        }
        p1 = ' ';
        /*FALLTHRU*/

    case WIDGET_KEYDOWN:        /* Keyboard event */
        if (b->cb_text.text)
            switch (p1) {
            case ' ':
                b->cb_value = !b->cb_value;
                widget_send(w, WIDGET_PAINT, 0, 0);
                widget_callback(w, DLGE_CHANGE, b->cb_value, 0);
                return TRUE;
            }
        break;

    case WIDGET_HOTKEY: {       /* Accelerator/hotkey event */
            const WButtonText_t *bt = &b->cb_text;
            int ishotkey = bt_ishotkey((uint32_t) p1, bt, 1);

            if (ishotkey >= 0) {
                checkbox_callback(b, WIDGET_KEYDOWN, ' ', 0);
                return TRUE;
            }
        }
        return tty_default(w, msg, p1, p2);

    case WIDGET_PAINT:          /* Widget display */
        if (0 == (w->w_flags & WIDGET_FHIDDEN) && b->cb_text.text) {
            const WButtonText_t *bt = &b->cb_text;
            int offset;

            tty_move(w, 0, 0);
            if (b->cb_unicode) {                /* MCHAR */
                tty_char(NORMAL, (b->cb_value ? CH_CHECK_ON : CH_CHECK_OFF));
                offset = 2;
            } else {
                char buf[20];
                sprintf(buf, "[%c] ", (b->cb_value ? 'x' : ' '));
                tty_str(NORMAL, buf);
                offset = 4;
            }
            tty_move(w, offset, 0);
            tty_str(HasFocus(w) ? FOCUS : NORMAL, bt->text);
            if (bt->hotkey) {
                tty_move(w, offset + bt->offset, 0);
                tty_char((HasFocus(w) ? HOTKEY_FOCUS : HOTKEY_NORMAL), bt->text[bt->offset]);
            }
        }
        return TRUE;

    case WIDGET_CARET:          /* Cursor selection */
        tty_move(w, (b->cb_unicode ? 0 : 1), 0);
        return TRUE;

    case WIDGET_DESTROY:        /* Destructor */
        bt_free(&b->cb_text);
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}


/*
 *  Group:  Label
**/
typedef struct {
    WIDGET_t            wl_widget;              /* base widget */
    WButtonText_t       wl_text;                /* button label */
} WLabel_t;

static int              label_callback(WLabel_t *b, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
label_new(void)
{
    return tty_new(sizeof(WLabel_t), (WIDGETCB_t)label_callback);
}


static int
label_callback(WLabel_t *b, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)b;

    switch (msg) {
    case WIDGET_INIT:           /* Initialise */
        if (b->wl_text.text) {
            if (w->w_reqrows <= 0) w->w_reqrows = 1;
            if (w->w_reqcols <= 0) w->w_reqcols = strlen(b->wl_text.text);
        }
        return TRUE;

    case WIDGET_READY:          /* Dialog session ready */
        break;

    case WIDGET_SET: {          /* Set attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            w->w_uflags |= WTTY_FDIRTY;
            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:
            case DLGA_LABEL:
                bt_data(&b->wl_text, data);
                break;
            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_SETFOCUS:       /* Cursor/focus request */
        return FALSE;                           /* dont allow selection */

    case WIDGET_PAINT:          /* Widget display */
        if (0 == (w->w_flags & WIDGET_FHIDDEN) && b->wl_text.text) {
            const WButtonText_t *bt = &b->wl_text;
            const char *text = bt->text;
            int y;

            for (y = 0; y < w->w_rows; ++y) {
                int pad, len = 0;

                if (text) {
                    const char *nl;

                    nl = str_chrx(text, '\n', &len);
                    if (len > w->w_cols) {      /* clip length ? */
                        len = w->w_cols;
                    }

                    if (len) {                  /* current line */
                        tty_move(w, 0, y);
                        tty_strn(NORMAL, text, len);
                        if (0 == y && bt->hotkey) {
                            tty_move(w, bt->offset, y);
                            tty_char(ATTR_DIALOG_HOTKEY_NORMAL, text[bt->offset]);
                        }
                    }

                    if (NULL == nl) {
                        text = NULL;
                    } else {
                        text = nl+1;
                    }
                }

                pad = w->w_cols - len;
                while (pad-- > 0) {             /* padding */
                    tty_move(w, len++, y);
                    tty_char(NORMAL, ' ');
                }
            }
        }
        return TRUE;

    case WIDGET_DESTROY:        /* Destructor */
        bt_free(&b->wl_text);
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}


/*
 *  Group:  Gauge
**/
typedef struct {
    WIDGET_t            g_widget;               /* base widget */
    uint32_t            g_min;                  /* lower limit */
    uint32_t            g_max;                  /* upper limit */
    uint32_t            g_value;                /* current value */
} WGauge_t;

static int              gauge_callback(WGauge_t *g, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
gauge_new(void)
{
    return tty_new(sizeof(WGauge_t), (WIDGETCB_t)gauge_callback);
}


static int
gauge_callback(WGauge_t *g, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)g;

    switch (msg) {
    case WIDGET_INIT:           /* Initialise */
        break;

    case WIDGET_READY:          /* Dialog session ready */
        break;

    case WIDGET_SET: {          /* Set attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            w->w_uflags |= WTTY_FDIRTY;
            switch (DIALOGARGLO(p1)) {
            case DLGA_GAUGEMIN:
                g->g_min    = widgetdata_uint32(data);
                break;
            case DLGA_GAUGEMAX:
                g->g_max    = widgetdata_uint32(data);
                break;
            case DLGA_VALUE:
                g->g_value  = widgetdata_uint32(data);
                break;
            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_SETFOCUS:       /* Cursor/focus request */
        return FALSE;

    case WIDGET_PAINT:          /* Widget display */
        if (0 == (WIDGET_FHIDDEN & w->w_flags)) {
            if (w->w_rows && g->g_max > g->g_min) {
            }
        }
        tty_move(w, 0, 0);
        return TRUE;

    case WIDGET_DESTROY:        /* Destructor */
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}


/*
 *  Common List box implementation
**/
typedef TAILQ_HEAD(ListboxList, ListboxItem)
                        ListboxList_t;          /* Dialog list */

typedef struct ListboxItem {
    MAGIC_t             li_magic;               /* structure magic */
    TAILQ_ENTRY(ListboxItem)
                        li_node;                /* list node */
    uint16_t            li_flags;               /* control flags */
#define LBI_FHIDDEN                 0x0001
#define LBI_FFILTERED               0x0002

    int16_t             li_shortcut;            /* short cut key, eg. 'Y - Yes' */
    const char *        li_display;             /* display text */
    uint32_t            li_length;              /* length, in bytes */
    const char *        li_data;                /* item text/data */
} ListboxItem_t;

#define LB_POSNORM                  -1
#define LB_POSHEAD                  0
#define LB_POSTAIL                  0x7fff

typedef int (*lbmatch_t)(const char *, const char *, size_t len);


typedef struct Listbox {
    MAGIC_t             lb_magic;               /* structure magic */
    ListboxList_t       lb_list;
    uint32_t            lb_flags;
#define LB_FHASSTRINGS              0x00000001
#define LB_FICASESTRINGS            0x00000002

#define LB_FALLOWDUPLICATES         0x00000010
#define LB_FINDEXMODE               0x00000020
#define LB_FPAGEMODE                0x00000040

#define LB_FWIDGETSIZE              0x00000100

#define LB_FISPOPUP                 0x00010000
#define LB_FHASSHORTCUT             0x00020000

    int                 lb_open;                /* presentation status */
    int                 lb_unicode;             /* active presentation character set */
    int                 lb_sortmode;            /* element insertion sort-order */
    int32_t             lb_rows;                /* panel rows */
    int32_t             lb_cols;                /* panel cols */
    int32_t             lb_columns;             /* columns, 1 or more */
    int32_t             lb_width;               /* width of each column */

    int32_t             lb_count;               /* element count */
    int32_t             lb_maxlen;              /* maximum element, length */
    int32_t             lb_visible;             /* visible element count */
    int32_t             lb_active;              /* active element, -1 if none */
    int32_t             lb_top;                 /* top element */
    const ListboxItem_t *lb_topcache;           /* cached record on 'top' item, NULL == unknown */
    int32_t             lb_cursor;              /* display cursor/focus */
    int32_t             lb_focus;               /* row of focused/cursor on panel */
    lbmatch_t           lb_match;

#define LB_PAINT1                   0x01
#define LB_PAINT2                   0x02

} Listbox_t;

static void             lb_init(Listbox_t *lb, uint32_t flags);
static void             lb_open(Listbox_t *lb, WIDGET_t *w);
static void             lb_complete(Listbox_t *lb);
static void             lb_destroy(Listbox_t *lb);
static int              lb_active(Listbox_t *lb, int32_t active);

static ListboxItem_t *  lb_item_add(Listbox_t *lb, int32_t pos, uint16_t flags, const char *data);
static int              lb_item_delete(Listbox_t *lb, ListboxItem_t *n, int32_t index);
static int              lb_item_zap(Listbox_t *lb);
static const ListboxItem_t *lb_item_get(const Listbox_t *lb, int32_t index);
static const ListboxItem_t *lb_item_find(const Listbox_t *lb, const char *text, int32_t len, int32_t *index);
static const ListboxItem_t *lb_item_match(const Listbox_t *lb, const ListboxItem_t *n, const char *text, int32_t *index);
static const ListboxItem_t *lb_item_shortcut(const Listbox_t *lb, uint16_t ch, int32_t *index, int text0);
static void             lb_item_set(Listbox_t *lb, const WIDGETDATA_t *data);
static void             lb_item_elements(Listbox_t *lb, int32_t pos, const WIDGETDATA_t *data);
static void             lb_item_remove(Listbox_t *lb, int32_t pos);
static int              lb_item_value(const Listbox_t *lb, int32_t idx, WIDGETDATA_t *data);
static void             lb_item_node(Listbox_t *lb, int32_t pos, int type, const void *value);
static void             lb_item_list(Listbox_t *lb, int32_t pos, const LIST *lp);

static int              lb_match_strcmp(const char *a, const char *b, size_t len);
static int              lb_match_stricmp(const char *a, const char *b, size_t len);

static int              lb_forward(Listbox_t *lb, WIDGET_t *w, int count);
static int              lb_backward(Listbox_t *lb, WIDGET_t *w, int count);
static int              lb_cursor(Listbox_t *lb, WIDGET_t *w, int32_t cursor);
static void             lb_paint(Listbox_t *lb, WIDGET_t *w, int repaint);
static int              lb_caret(Listbox_t *lb, WIDGET_t *w);



static void
lb_init(Listbox_t *lb, uint32_t flags)
{
    TAILQ_INIT(&lb->lb_list);
    lb->lb_flags    = flags;
    lb->lb_unicode  = tty_isunicode(TRUE);
    lb->lb_sortmode = 0;                        /* 0=none,1 or -1 */
    lb->lb_rows     = 0;
    lb->lb_cols     = 0;
    lb->lb_maxlen   = 0;
    lb->lb_columns  = -1;                       /* dynamic */
    lb->lb_width    = -1;
    lb->lb_active   = -1;
    lb->lb_top      = 0;
    lb->lb_topcache = NULL;
    lb->lb_match    = lb_match_strcmp;
}


static void
lb_open(Listbox_t *lb, WIDGET_t *w)
{
    if (!lb->lb_open) {
        lb->lb_open = TRUE;

        if (w) {                                /* import widget sizing */
            if ((LB_FWIDGETSIZE & lb->lb_flags) || (0 == lb->lb_rows && 0 == lb->lb_cols)) {
                lb->lb_flags |= LB_FWIDGETSIZE;
                lb->lb_rows = w->w_rows;
                lb->lb_cols = w->w_cols;
            }
        }

        if (lb->lb_cursor < 0 || lb->lb_cursor >= lb->lb_count) {
            if (lb->lb_active >= 0) {
                lb_cursor(lb, w, lb->lb_active);
            } else {
                lb_cursor(lb, w, 0);
            }
        }
    }
}


static void
lb_complete(Listbox_t *lb)
{
    lb->lb_open     = FALSE;
}


static void
lb_destroy(Listbox_t *lb)
{
    lb_item_zap(lb);
}


static int
lb_active(Listbox_t *lb, int32_t active)
{
    if (active >= -1 && active < lb->lb_count) {
        lb->lb_active = active;
    }
    return lb->lb_active;
}


static int
lb_value_set(Listbox_t *lb, const WIDGETARG_t p1, const WIDGETDATA_t *data)
{
    const uint16_t attr = DIALOGARGLO(p1);
 /* const uint16_t idx = DIALOGARGHI(p1); */
    int32_t value;

    switch (attr) {
    case DLGA_LBCOUNT:
        return FALSE;

    case DLGA_LBSORTMODE:
        if ((value = widgetdata_int32(data)) >= -1 && value <= 1) {
            lb->lb_sortmode = value;
        }
        break;

    case DLGA_LBHASSTRINGS:
        if (widgetdata_int32(data)) {
            lb->lb_flags |= LB_FHASSTRINGS;
        } else {
            lb->lb_flags &= ~LB_FHASSTRINGS;
        }
        break;

    case DLGA_LBICASESTRINGS:
        if (widgetdata_int32(data)) {
            lb->lb_flags |= LB_FICASESTRINGS;
            lb->lb_match = lb_match_stricmp;
        } else {
            lb->lb_flags &= ~LB_FICASESTRINGS;
            lb->lb_match = lb_match_strcmp;
        }
        break;

    case DLGA_LBDUPLICATES:
        if (widgetdata_int32(data)) {
            lb->lb_flags |= LB_FALLOWDUPLICATES;
        } else {
            lb->lb_flags &= ~LB_FALLOWDUPLICATES;
        }
        break;

    case DLGA_LBCURSOR:
        if ((value = widgetdata_int32(data)) >= -1 && value < lb->lb_count) {
            lb_cursor(lb, NULL, value);
        }
        break;

    case DLGA_LBACTIVE:
        if ((value = widgetdata_int32(data)) >= -1 && value < lb->lb_count) {
            lb->lb_active = value;
        }
        break;

    case DLGA_LBTEXT:
    case DLGA_LBTEXTLEN:
    case DLGA_LBDISPLAYTEXT:
    case DLGA_LBDISPLAYTEXTLEN:
        return FALSE;

    case DLGA_LBPAGEMODE:
        if (widgetdata_int32(data)) {
            lb->lb_flags |= LB_FPAGEMODE;
        } else {
            lb->lb_flags &= ~LB_FPAGEMODE;
        }
        break;

    case DLGA_LBINDEXMODE:
        if (widgetdata_int32(data)) {
            lb->lb_flags |= LB_FINDEXMODE;
        } else {
            lb->lb_flags &= ~LB_FINDEXMODE;
        }
        break;

    case DLGA_LBROWS:
        if ((value = widgetdata_int32(data)) >= 0) {
            lb->lb_rows = value;
        }
        break;
    case DLGA_LBCOLS:
        if ((value = widgetdata_int32(data)) >= 0) {
            lb->lb_cols = value;
        }
        break;

    case DLGA_LBCOLUMNS:
        if ((value = widgetdata_int32(data)) >= 1) {
            if (! lb->lb_open) {
                lb->lb_columns = value;
            }
        }
        break;
    case DLGA_LBWIDTH:
        break;

    default:
        return FALSE;
    }
    return TRUE;
}


static int
lb_value_get(Listbox_t *lb, const WIDGETARG_t p1, WIDGETDATA_t *data)
{
    const uint16_t attr = DIALOGARGLO(p1);
    const uint16_t idx = DIALOGARGHI(p1);

    switch (attr) {
    case DLGA_LBCOUNT:
        data->d_u.ivalue = lb->lb_count;
        data->d_type = D_INT;
        break;

    case DLGA_LBSORTMODE:
        data->d_u.ivalue = lb->lb_sortmode;
        data->d_type = D_INT;
        break;

    case DLGA_LBHASSTRINGS:
        data->d_u.ivalue = ((LB_FHASSTRINGS & lb->lb_flags) ? TRUE : FALSE);
        data->d_type = D_INT;
        break;

    case DLGA_LBICASESTRINGS:
        data->d_u.ivalue = ((LB_FICASESTRINGS & lb->lb_flags) ? TRUE : FALSE);
        data->d_type = D_INT;
        break;

    case DLGA_LBDUPLICATES:
        data->d_u.ivalue = ((LB_FALLOWDUPLICATES & lb->lb_flags) ? TRUE : FALSE);
        data->d_type = D_INT;
        break;

    case DLGA_LBCURSOR:
        data->d_u.ivalue = lb->lb_cursor;
        data->d_type = D_INT;
        break;

    case DLGA_LBACTIVE:
        data->d_u.ivalue = lb->lb_active;
        data->d_type = D_INT;
        break;

    case DLGA_LBTEXT:
        return lb_item_value(lb, idx, data);

    case DLGA_LBTEXTLEN:
        if (lb_item_value(lb, idx, data) && D_STR == data->d_type) {
            data->d_u.ivalue = (accint_t)strlen(data->d_u.svalue);
        } else {
            data->d_u.ivalue = 0;
        }
        data->d_type = D_INT;
        break;

    case DLGA_LBDISPLAYTEXT:
        break;
    case DLGA_LBDISPLAYTEXTLEN:
        break;

    case DLGA_LBPAGEMODE:
        data->d_u.ivalue = ((LB_FPAGEMODE & lb->lb_flags) ? TRUE : FALSE);
        data->d_type = D_INT;
        break;
    case DLGA_LBINDEXMODE:
        data->d_u.ivalue = ((LB_FINDEXMODE & lb->lb_flags) ? TRUE : FALSE);
        data->d_type = D_INT;
        break;

    case DLGA_LBROWS:
        data->d_u.ivalue = lb->lb_rows;
        data->d_type = D_INT;
        break;
    case DLGA_LBCOLS:
        data->d_u.ivalue = lb->lb_cols;
        data->d_type = D_INT;
        break;

    case DLGA_LBCOLUMNS:
        data->d_u.ivalue = lb->lb_columns;
        data->d_type = D_INT;
        break;
    case DLGA_LBWIDTH:
        data->d_u.ivalue = lb->lb_width;
        data->d_type = D_INT;
        break;

    default:
        return FALSE;
    }
    return TRUE;
}


static ListboxItem_t *
lb_item_add(Listbox_t *lb, int32_t pos, uint16_t flags, const char *data)
{
    ListboxList_t *queue = &lb->lb_list;
    ListboxItem_t *n = NULL;

    /*
     *  build node
     */
    if (LB_FHASSTRINGS & lb->lb_flags) {        /* string, copy image */
        int datalen = 0, displaylen = 0, shortcut = 0;
        const char *display;
        char *buffer;

        trace_log("lb_item_add(pos:%d,flags:%x,data:<%s>)\n", (int)pos, (unsigned)flags, data);

        if (NULL != (display = data)) {         /* <short-cut><\001><display-text>\002<data-text> */
            const char *delim;

            if (*display && '\001' == display[1]) {
                shortcut = toupper(*display);
                lb->lb_flags |= LB_FHASSHORTCUT;
                display = display + 2;
            }

            if (NULL != (delim = strchr(display, '\002'))) {
                displaylen = delim - display;
                data = delim + 1;
            }

            datalen = (int)strlen(data);
        }

        if (0 == (LB_FALLOWDUPLICATES & lb->lb_flags) &&
                NULL != lb_item_find(lb, display, displaylen ? displaylen : datalen, NULL)) {
            trace_log("\t== duplicate:<%d/%s>\n", displaylen ? displaylen : datalen, display);
            return NULL;
        }

        if (NULL == (n = chk_alloc(sizeof(ListboxItem_t) + datalen + 1 + displaylen + 1))) {
            return NULL;
        }

        buffer = (char *)(n + 1);               /* append after node */
        memcpy(buffer, data, datalen + 1);
        if (displaylen) {
            if (displaylen > lb->lb_maxlen) {
                lb->lb_maxlen = displaylen;
            }
            memcpy(buffer + datalen + 1, display, displaylen);
            buffer[ datalen + 1 + displaylen ] = 0;
            display = buffer + datalen + 1;
        } else {
            if (datalen > lb->lb_maxlen) {
                lb->lb_maxlen = datalen;
            }
            display = buffer;
            displaylen = strlen(buffer);
        }

        trace_log("\t== flags:<%x>,shortcut:<%d>,display:<%d/%s>,data:<%d/%s>\n",
            flags, shortcut, displaylen, display, datalen, buffer);

        n->li_flags     = flags;
        n->li_shortcut  = (uint16_t)shortcut;
        n->li_display   = display;
        n->li_length    = displaylen;
        n->li_data      = buffer;

    } else {                                    /* non-strings, assign image */
        if (0 == (LB_FALLOWDUPLICATES & lb->lb_flags) &&
                NULL != lb_item_find(lb, data, 0, NULL)) {
            return NULL;
        }

        if (NULL == (n = chk_alloc(sizeof(ListboxItem_t)))) {
            return NULL;
        }

        trace_log("\t== flags:<%x>,shortcut:<0>,display:<-1/>,data:<%p>\n", flags, data);

        n->li_flags     = flags;
        n->li_shortcut  = 0;
        n->li_display   = "";
        n->li_length    = 0;
        n->li_data      = data;
    }

    /*
     *  insert
     */
    if (LB_POSHEAD == pos) {
        TAILQ_INSERT_HEAD(queue, n, li_node);

    } else if (LB_POSNORM == pos && (LB_FHASSTRINGS & lb->lb_flags)) {
        switch (lb->lb_sortmode) {
        case 1: {       /* Decending */
                const char *display = n->li_display;
                ListboxItem_t *ncursor;

                for (ncursor = TAILQ_FIRST(queue); ncursor; ncursor = TAILQ_NEXT(ncursor, li_node)) {
                    if (str_icmp(display, ncursor->li_display) < 0) {
                        TAILQ_INSERT_BEFORE(ncursor, n, li_node);
                        break;
                    }
                }
                if (NULL == ncursor) {
                    TAILQ_INSERT_TAIL(queue, n, li_node);
                }
            }
            break;

        case -1: {      /* Acending */
                const char *display = n->li_display;
                ListboxItem_t *ncursor;

                for (ncursor = TAILQ_LAST(queue, ListboxList); ncursor; ncursor = TAILQ_PREV(ncursor, ListboxList, li_node)) {
                    if (str_icmp(display, ncursor->li_display) < 0) {
                        TAILQ_INSERT_AFTER(queue, ncursor, n, li_node);
                        break;
                    }
                }
                if (NULL == ncursor) {
                    TAILQ_INSERT_HEAD(queue, n, li_node);
                }
            }
            break;

        default:        /* Unordered */
            TAILQ_INSERT_TAIL(queue, n, li_node);
            break;
        }

    } else if (pos < 0 || pos >= lb->lb_count) {
        TAILQ_INSERT_TAIL(queue, n, li_node);

    } else {
        ListboxItem_t *ncursor;
        int32_t count = 0;

        for (ncursor = TAILQ_FIRST(queue); ncursor; ncursor = TAILQ_NEXT(ncursor, li_node)) {
            if (pos == ++count) {
                TAILQ_INSERT_AFTER(queue, ncursor, n, li_node);
                n = NULL;
                break;
            }
        }
        assert(NULL != ncursor);
        assert(NULL == n);
    }

    ++lb->lb_count;
    if (n && 0 == ((LBI_FHIDDEN|LBI_FFILTERED) & n->li_flags)) {
        ++lb->lb_visible;
    }

    return n;
}


static int
lb_item_delete(Listbox_t *lb, ListboxItem_t *n, int32_t idx)
{
    if (n) {
        TAILQ_REMOVE(&lb->lb_list, n, li_node);
        chk_free((void *)n);

        lb->lb_maxlen = -1;
        if (0 == ((LBI_FHIDDEN|LBI_FFILTERED) & n->li_flags)) {
            --lb->lb_visible;
        }
        if (lb->lb_active >= 0) {
            if (idx <= lb->lb_active) {
                if (idx == lb->lb_active) {
                    lb->lb_active = -1;
                } else {
                    --lb->lb_active;
                }
            }
        }
        --lb->lb_count;
        lb->lb_topcache = NULL;
    }
    return 0;
}


static int
lb_item_zap(Listbox_t *lb)
{
    ListboxList_t *queue = &lb->lb_list;
    ListboxItem_t *n;

    while (NULL != (n = TAILQ_FIRST(queue))) {
        lb_item_delete(lb, n, -1);
    }
    TAILQ_INIT(&lb->lb_list);
    lb->lb_maxlen   = 0;
    lb->lb_active   = -1;
    lb->lb_count    = 0;
    lb->lb_visible  = 0;
    lb->lb_top      = 0;
    lb->lb_topcache = NULL;
    return TRUE;
}


static void
lb_item_remove(Listbox_t *lb, int32_t idx)
{
    ListboxItem_t *n;

    if (NULL != (n = (ListboxItem_t *)lb_item_get(lb, idx))) {
        lb_item_delete(lb, n, idx);
    }
}


static const ListboxItem_t *
lb_item_get(const Listbox_t *lb, int32_t idx)
{
    const ListboxList_t *queue = &lb->lb_list;
    const ListboxItem_t *n;

    if (idx >= 0 && idx < lb->lb_count) {
        for (n = TAILQ_FIRST(queue); n; n = TAILQ_NEXT(n, li_node)) {
            if (idx-- <= 0) {
                return n;
            }
        }
    }
    return NULL;
}


static const ListboxItem_t *
lb_item_find(const Listbox_t *lb, const char *text, int32_t len, int32_t *idx)
{
    const ListboxList_t *queue = &lb->lb_list;
    const ListboxItem_t *n;
    int32_t t_idx = 0;

    if (LB_FHASSTRINGS & lb->lb_flags) {
        if (text) {
            if (len < 0) len = (int)strlen(text);
            for (n = TAILQ_FIRST(queue); n; n = TAILQ_NEXT(n, li_node)) {
                if (len == (int32_t)n->li_length &&
                        0 == memcmp(n->li_display, text, len)) {
                    if (idx) *idx = t_idx;
                    return n;
                }
                ++t_idx;
            }
        }
    } else {
        for (n = TAILQ_FIRST(queue); n; n = TAILQ_NEXT(n, li_node)) {
            if (n->li_data == text) {
                if (idx) *idx = t_idx;
                return n;
            }
            ++t_idx;
        }
    }
    return NULL;
}


static const ListboxItem_t *
lb_item_match(const Listbox_t *lb, const ListboxItem_t *n, const char *text, int32_t *idx)
{
    if (text) {
        const size_t len = strlen(text);

        if (len > 0) {
            const ListboxList_t *queue = &lb->lb_list;
            int32_t t_idx = (n ? *idx : 0);
            lbmatch_t match = lb->lb_match;

            n = (n ? TAILQ_NEXT(n, li_node) : TAILQ_FIRST(queue));
            while (n) {
                if (0 == (LBI_FHIDDEN & n->li_flags)) {
                    if (0 == (match)(n->li_display, text, len)) {
                        if (idx) *idx = t_idx;
                        return n;
                    }                }
                n = TAILQ_NEXT(n, li_node);
                ++t_idx;
            }
        }
    }
    return NULL;
}


static int
lb_match_strcmp(const char *a, const char *b, size_t len)
{
    return strncmp(a, b, len);
}


static int
lb_match_stricmp(const char *a, const char *b, size_t len)
{
#if defined(HAVE_STRCASECMP)
    return strncasecmp(a, b, len);

#elif defined(HAVE__STRCASECMP)
    return _strncasecmp(a, b, len);

#elif defined(HAVE___STRCASECMP)
    return __strncasecmp(a, b, len);

#elif defined(WIN32)
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
    return _strnicmp(a, b, len);

#else
    return strnicmp(a, b, len);
#endif
#else
#error unknown environment ...
#endif
}


static const ListboxItem_t *
lb_item_shortcut(const Listbox_t *lb, uint16_t ch, int32_t *idx, int text0)
{
    if (ch) {
        const ListboxList_t *queue = &lb->lb_list;
        const ListboxItem_t *n;
        int32_t t_idx = 0;

        ch = (uint16_t)toupper(ch);
        if (text0) {
            const int32_t s_idx = (idx && *idx > 0 ? *idx : 0);

            for (n = TAILQ_FIRST(queue); n; n = TAILQ_NEXT(n, li_node)) {
                if (t_idx >= s_idx) {           /* start cursor */
                    if (n->li_display && ch == toupper(*(unsigned char *)n->li_display)) {
                        if (idx) *idx = t_idx;
                        return n;
                    }
                }
                ++t_idx;
            }
        } else {
            for (n = TAILQ_FIRST(queue); n; n = TAILQ_NEXT(n, li_node)) {
                if (ch == n->li_shortcut) {
                    if (idx) *idx = t_idx;
                    return n;
                }
                ++t_idx;
            }
        }
    }
    return NULL;
}


static void
lb_item_elements(Listbox_t *lb, int32_t pos, const WIDGETDATA_t *data)
{
    switch (data->d_type) {
    case D_INT:
        lb_item_node(lb, pos, D_INT, &data->d_u.ivalue);
        break;

    case D_STR:
        lb_item_node(lb, pos, D_STR, data->d_u.svalue);
        break;

    case D_LIST:
        lb_item_list(lb, pos, data->d_u.lvalue);
        break;

    case D_NULL:
        lb_item_zap(lb);
        break;

    default:
        break;
    }
}


static int
lb_item_value(const Listbox_t *lb, int32_t idx, WIDGETDATA_t *data)
{
    const ListboxItem_t *n = NULL;

    /* !STRINGS or buffer == NULL */
    if (0 == (lb->lb_flags & LB_FHASSTRINGS) || lb->lb_count <= 0) {
        return D_NULL;
    }

    /* current or specific */
    if (idx < lb->lb_count) {                   /* specific idx */
        const ListboxList_t *queue = &lb->lb_list;

        for (n = TAILQ_FIRST(queue); n; n = TAILQ_NEXT(n, li_node)) {
            if (idx-- == 0) {
                break;                          /* .. found */
            }
        }
    }

    trace_log("lb_item_value(%u): %s=%s\n", (unsigned)idx, (n ? n->li_display : ""), (n ? n->li_data : ""));
    data->d_u.svalue = (n ? n->li_data : "");
    data->d_type = D_STR;
    return TRUE;
}


static void
lb_item_node(Listbox_t *lb, int32_t idx, int type, const void *value)
{
    char buf[32];

    switch (type) {
    case D_INT:
        sxprintf(buf, sizeof(buf), "%" ACCINT_FMT, *((accint_t *)value));
        lb_item_add(lb, idx, 0, (const char *)buf);
        break;

    case D_FLOAT:
        sxprintf(buf, sizeof(buf), "%" ACCFLOAT_FMT, *((accfloat_t *)value));
        lb_item_add(lb, idx, 0, (const char *)buf);
        break;

    case D_STR:
        lb_item_add(lb, idx, 0, (const char *)value);
        break;

    default:
        break;
    }
}


static void
lb_item_list(Listbox_t *lb, int32_t pos, const LIST *lp)
{
    const LIST *nextlp;

    for (;(nextlp = atom_next(lp)) != lp; lp = nextlp) {
        accint_t ival;
        accfloat_t fval;
        const char *sval;
        const LIST *lval;

        if (atom_xint(lp, &ival)) {
            lb_item_node(lb, pos, D_INT, &ival);

        } else if (atom_xfloat(lp, &fval)) {
            lb_item_node(lb, pos, D_FLOAT, &fval);

        } else if (NULL != (sval = atom_xstr(lp))) {
            lb_item_node(lb, pos, D_STR, sval);

        } else if (NULL != (lval = atom_xlist(lp))) {
            lb_item_list(lb, pos, lval);

        } else if (! atom_xnull(lp)) {
            panic("lb_item_list <unknown type=%d>", *lp);
        }
    }
}


static int32_t
lb_item_count(const Listbox_t *lb)
{
    return lb->lb_count;
}


static int
lb_item_prefixlength(const Listbox_t *lb)
{
    if (LB_FINDEXMODE & lb->lb_flags) {
        int length = 2;

        if (lb->lb_unicode && lb->lb_count <= 20) {
            ++length;
        } else {
            int count = lb->lb_count;

            while ((count /= 10) > 0) {
                ++length;
            }
        }
        return length;

    } else if (LB_FHASSHORTCUT & lb->lb_flags) {
        return 2;
    }
    return 0;
}


static int
lb_item_maxlen(Listbox_t *lb)
{
    int32_t maxlen = 0;

    if (lb->lb_count > 0) {
        if (lb->lb_maxlen < 0) {
            const ListboxList_t *queue = &lb->lb_list;
            const ListboxItem_t *n;

            maxlen = 4;
            for (n = TAILQ_FIRST(queue); n; n = TAILQ_NEXT(n, li_node)) {
                const char *text;
                int32_t length;

                if (NULL != (text = n->li_display)) {
                    if ((length = n->li_length) > maxlen) {
                        maxlen = length;
                    }
                }
            }
            lb->lb_maxlen = maxlen;
        }
        maxlen = lb->lb_maxlen;
    }
    return lb_item_prefixlength(lb) + maxlen;
}


static int
lb_key(Listbox_t *lb, WIDGET_t *w, int key)
{
    const int32_t columns = (lb->lb_columns > 0 ? lb->lb_columns : 1);
    const int32_t rows    = (lb->lb_rows > 0 ? lb->lb_rows : (w && w->w_rows > 1 ? w->w_rows : 1));
    const int32_t page    = rows * columns;
    int repaint = TRUE;

    if (lb->lb_count <= 0) {
        return FALSE;
    }

    switch (key) {
    case KEY_ENTER:     /* <Enter> -    except */
    case ' ':
        lb->lb_active = lb->lb_cursor;
        widget_send(w, WIDGET_PAINT, 0, 0);

        /*
         *  p1 =    DLGE_CHANGE selection notification.
         *
         *  p2 =    Specifies the zero-based index of the item. this parameter is
         *          limited to 16-bit values, meaning list boxes cannot contain more than
         *          16^2 items. Although the number of items is restricted, the total size in
         *          bytes of the items in a list box is limited only by available memory.
         */
        widget_callback(w, DLGE_CHANGE, (WIDGETARG_t) lb->lb_active, 0);
        return TRUE;

    case KEY_HOME:      /* <Home> -     top of list */
        lb_cursor(lb, w, 0);
        break;

    case KEY_PAGEUP:    /* <PgUp> -     scroll up one page */
        repaint = lb_backward(lb, w, page);
        break;

    case KEY_UP:        /* <Up> -       scroll up one item */
    case WHEEL_UP:
        repaint = lb_backward(lb, w, 1);
        break;

    case KEY_LEFT:      /* <Left> -     previous item/column */
        repaint = lb_backward(lb, w, (columns > 1 ? rows : 1));
        break;

    case KEY_RIGHT:     /* <Right> -    previous item/column */
        repaint = lb_forward(lb, w, (columns > 1 ? rows : 1));
        break;

    case KEY_DOWN:      /* <Down> -     scroll down one item */
    case WHEEL_DOWN:
        repaint = lb_forward(lb, w, 1);
        break;

    case KEY_PAGEDOWN:  /* <PgDn> -     scroll down one page */
        repaint = lb_forward(lb, w, page);
        break;

    case KEY_END:       /* <End> -      bottom of list */
        if (lb->lb_count) {
            lb_cursor(lb, w, lb->lb_count - 1);
        }
        break;

    default:            /* shortcut or simple auto-complete/search */
        if (lb->lb_count) {
            int32_t idx = -1;

            if (LB_FHASSHORTCUT & lb->lb_flags) {
                if (lb_item_shortcut(lb, (uint16_t)key, &idx, FALSE)) {
                    lb_cursor(lb, w, idx);
                }

            } else if (key >= 32 && key <= 127) {
                idx = lb->lb_cursor + 1;        /* next match */
                if (lb_item_shortcut(lb, (uint16_t)key, &idx, TRUE)) {
                    lb_cursor(lb, w, idx);
                } else if (lb->lb_cursor > 0) {
                    idx = -1;                   /* otherwise loop */
                    if (lb_item_shortcut(lb, (uint16_t)key, &idx, TRUE)) {
                        lb_cursor(lb, w, idx);
                    }
                }
            }
        }
        return FALSE;
    }

    if (repaint) {
        widget_send(w, WIDGET_PAINT, repaint, 0);
    }
    return TRUE;
}


static int
lb_select(Listbox_t *lb, WIDGET_t *w, int y)
{
    const int32_t rows    = (lb->lb_rows > 0 ? lb->lb_rows : (w && w->w_rows ? w->w_rows : 1));

    if (y >= rows) {
        return FALSE;
    }
    lb_cursor(lb, w, lb->lb_top + y);           /* HIDDEN|FILTERED */
    return TRUE;
}


static void
lb_filter(Listbox_t *lb, int state, ListboxItem_t *n)
{
    if (n) {
        if (state) {                            /* filter */
            if (0 == ((LBI_FHIDDEN|LBI_FFILTERED) & n->li_flags)) {
                n->li_flags |= LBI_FFILTERED;
                --lb->lb_visible;
            }
        } else {                                /* unfilter */
            if (LBI_FFILTERED == ((LBI_FHIDDEN|LBI_FFILTERED) & n->li_flags)) {
                n->li_flags &= (uint16_t)~LBI_FFILTERED;
                ++lb->lb_visible;
            }
        }
    } else {
        const int32_t count = lb->lb_count;

        if (count) {
            const ListboxList_t *queue = &lb->lb_list;

            if (state) {                        /* filter */
                for (n = TAILQ_FIRST(queue); n; n = TAILQ_NEXT(n, li_node)) {
                    n->li_flags |= LBI_FFILTERED;
                }
                lb->lb_visible = 0;
            } else {                            /* unfilter, ignore HIDDEN elements */
                for (n = TAILQ_FIRST(queue); n; n = TAILQ_NEXT(n, li_node)) {
                    if (LBI_FFILTERED == ((LBI_FHIDDEN|LBI_FFILTERED) & n->li_flags)) {
                        n->li_flags &= (uint16_t)~LBI_FFILTERED;
                        if (count <= ++lb->lb_visible) {
                            break;
                        }
                    }
                }
            }
        }
    }
}


static int
lb_forward(Listbox_t *lb, WIDGET_t *w, int count)
{
    const int32_t columns = (lb->lb_columns > 0 ? lb->lb_columns : 1);
    const int32_t rows    = (lb->lb_rows > 0 ? lb->lb_rows : (w && w->w_rows > 0 ? w->w_rows : 1));
    const int32_t page    = rows * columns;
    int repaint = 0;

    while (count-- > 0) {
        if ((lb->lb_cursor + 1) < lb->lb_count) {
            int32_t newtop;

            ++lb->lb_cursor;
            if (LB_FPAGEMODE & lb->lb_flags) {
                newtop = (lb->lb_cursor / page) * page;
                if (lb->lb_top != newtop) {
                    lb->lb_top = newtop;
                    lb->lb_topcache = NULL;
                    repaint |= LB_PAINT1;       /* complete */
                }
            } else if (lb->lb_cursor >= rows) {
                newtop = lb->lb_cursor - (rows - 1);
                if (lb->lb_top < newtop) {
                    lb->lb_top = newtop;
                    if (lb->lb_topcache) {
                        lb->lb_topcache = TAILQ_NEXT(lb->lb_topcache, li_node);
                    }
                    repaint |= LB_PAINT1;       /* complete */
                }
            }
            repaint |= LB_PAINT2;               /* frame/elevator only */

        } else if (columns > 1) {
            if (lb->lb_top >= 0) {              /* paging in last column, wrap */
                lb->lb_cursor = lb->lb_top;
                repaint |= LB_PAINT2;
            }
        }
    }
    return repaint;
}


static int
lb_backward(Listbox_t *lb, WIDGET_t *w, int count)
{
    const int32_t columns = (lb->lb_columns > 0 ? lb->lb_columns : 1);
    const int32_t rows    = (lb->lb_rows > 0 ? lb->lb_rows : (w && w->w_rows > 0 ? w->w_rows : 1));
    const int32_t page    = rows * columns;
    int repaint = 0;

    __CUNUSED(w)
    while (count-- > 0) {
        if (lb->lb_cursor > 0) {
            if (--lb->lb_cursor < lb->lb_top) {
                if (LB_FPAGEMODE & lb->lb_flags) {
                    lb->lb_top = (lb->lb_cursor / page) * page;
                    lb->lb_topcache = NULL;
                } else {
                    --lb->lb_top;
                    if (lb->lb_topcache) {
                        lb->lb_topcache = TAILQ_PREV(lb->lb_topcache, ListboxList, li_node);
                    }
                }
                repaint |= LB_PAINT1;           /* complete */
            }
            repaint |= LB_PAINT2;               /* frame/elevator only */

        } else if (columns > 1) {
            if (lb->lb_top >= 0) {              /* paging in first column, wrap */
                if ((lb->lb_cursor = lb->lb_top + (page - 1)) >= lb->lb_count) {
                                                  lb->lb_cursor = (lb->lb_count - 1);
                }
                repaint |= LB_PAINT2;
            }
        }
    }
    return repaint;
}


static int
lb_cursor(Listbox_t *lb, WIDGET_t *w, int32_t cursor)
{
    const int32_t columns = (lb->lb_columns > 0 ? lb->lb_columns : 1);
    const int32_t rows    = (lb->lb_rows > 0 ? lb->lb_rows : (w && w->w_rows > 0 ? w->w_rows : 1));
    const int32_t page    = rows * columns;

    if (cursor >= 0 && cursor < lb->lb_count) {
        lb->lb_cursor = cursor;
        if (rows) {
            if (rows >= lb->lb_count) {         /* single page */
                if (lb->lb_top) {
                    lb->lb_top = 0;
                    lb->lb_topcache = NULL;
                }
            } else  {                           /* align top */
                const int newtop = (cursor / page) * page;

                if (newtop != lb->lb_top) {
                    lb->lb_top = newtop;
                    lb->lb_topcache = NULL;
                }
            }
        }
    }
    return 0;
}


static void
lb_paint(Listbox_t *lb, WIDGET_t *w, int repaint)
{
    const int32_t columns = (lb->lb_columns > 0 ? lb->lb_columns : 1);
    const int32_t rows    = (lb->lb_rows > 0 ? lb->lb_rows : (w && w->w_rows > 0 ? w->w_rows : 1));
    const int32_t cols    = (lb->lb_cols > 0 ? lb->lb_cols : (w && w->w_cols > 0 ? w->w_cols : 1));
    const int32_t page    = rows * columns;

    const int popup       = (LB_FISPOPUP & lb->lb_flags) ? TRUE : FALSE;
    const int frame       = (FALSE == popup && w && w->w_border ? TRUE : FALSE);
    const int vscroll     = (lb->lb_visible > page ? TRUE : FALSE);

    void (*move)(WIDGET_t *w, int x, int y) =
            (popup ? tty_absmove : tty_move);

    int32_t item = lb->lb_top;
    const ListboxItem_t *n;

    if (0 == repaint || NULL == lb->lb_topcache) {
        repaint = LB_PAINT1|LB_PAINT2;
    }

    if ((popup && !dialog_tty_popup_select(w->w_root, TRUE))) {
        return;
    }

    if (frame) {                                /* frame */
        tty_border(w, FRAME, 0, 0, rows + (frame * 2), cols + (frame * 2), vscroll);
    }

    n = lb_item_get(lb, item);                  /* first element, if any */
    assert(NULL == lb->lb_topcache || n == lb->lb_topcache);
    lb->lb_topcache = n;

    if ((LB_PAINT1 & repaint) && (LB_FHASSTRINGS & lb->lb_flags)) {
        const int32_t active = lb->lb_active;
        const int32_t cursor = lb->lb_cursor;
        const int width = (lb->lb_width > 0 ? lb->lb_width : cols / columns);
        int   prefixlength = lb_item_prefixlength(lb);
        int   maxlen = width - prefixlength;
        char  idxbuffer[32];
        int   x, y, idx = 0;

        lb->lb_width = width;
        lb->lb_focus = -1;

        for (x = 0; x < cols; x += width) {     /* foreach(column) */

            for (y = 0; y < rows;) {            /* foreach(row) */
                const LINEATTR attr = (n && active == item ? HILITE : NORMAL);
                int pad, len = 0;

                if (n) {
                    const char *text = n->li_display;

                    if ((LBI_FHIDDEN|LBI_FFILTERED) & n->li_flags) {
                        n = TAILQ_NEXT(n, li_node);
                        ++item;
                        continue;
                    }

                    if ((len = n->li_length) > maxlen) {
                        len = maxlen;
                    }

                    if (len) {                  /* current line */
                        if (prefixlength) {
                            int idxoffset = 0;

                            if (LB_FHASSHORTCUT & lb->lb_flags) {
                                sxprintf(idxbuffer, sizeof(idxbuffer), "%c%*s",
                                    (n->li_shortcut ? n->li_shortcut : ' '), prefixlength - 1, "");

                                                /* 1. .. 20. */
                            } else if (lb->lb_unicode && lb->lb_count <= 20) {
                                move(w, frame + x, frame + y);
                                tty_char(FOCUS, 0x2488 + item);
                                strcpy(idxbuffer, " ");
                                idxoffset = 2;

                            } else {
                                sxprintf(idxbuffer, sizeof(idxbuffer), "%-*d ", prefixlength - 1, item + 1);
                            }

                            move(w, frame + x + idxoffset, frame + y);
                            tty_str(FOCUS, idxbuffer);
                        }

                        move(w, frame + x + prefixlength, frame + y);
                        tty_strn(attr, text, len);
                    }
                    n = TAILQ_NEXT(n, li_node);
                    if (cursor == item++) {
                        lb->lb_focus = idx;     /* focus element */
                    }

                } else {
                    maxlen += prefixlength;
                    prefixlength = 0;
                }

                if ((pad = maxlen - len) > 0) {
                    move(w, frame + x + prefixlength + len, frame + y);
                    while (pad-- > 0) {         /* padding */
                        tty_char(attr, ' ');
                    }
                }

                move(w, 1, frame + y);          /* left align */
                ++y, ++idx;
            }
        }
    }

    if (vscroll) {                              /* elevator */
        const int y = (lb->lb_cursor * page) / lb->lb_visible;

        if (frame) {
            tty_move(w, frame + cols, frame + y);
            tty_char(NORMAL, (lb->lb_unicode ? cmap_specunicode(CH_VTHUMB) : '*'));

        } else if (popup) {
            window_ctrl_set(curwp, WCTRLO_VERT_SCROLL);
            window_ctrl_set(curwp, WCTRLO_USER_VTHUMB);
            curwp->w_ctrl_vthumb = y;
        }
    } else {
        if (popup) {
            window_ctrl_clr(curwp, WCTRLO_VERT_SCROLL);
            curwp->w_ctrl_vthumb = -1;
        }
    }

    if (popup) {
        dialog_tty_popup_select(w->w_root, FALSE);
    }
}


static int
lb_caret(Listbox_t *lb, WIDGET_t *w)
{
    const int32_t focus = lb->lb_focus;

    if (focus >= 0) {
        const int32_t rows = (lb->lb_rows > 0 ? lb->lb_rows : (w->w_rows > 0 ? w->w_rows : 1));
        const int popup = (LB_FISPOPUP & lb->lb_flags) ? TRUE : FALSE;

        if (popup) {
            if (! dialog_tty_popup_select(w->w_root, TRUE)) {
                return FALSE;
            }
            tty_absmove(w, (focus / rows) * lb->lb_width, focus % rows);
            dialog_tty_popup_select(w->w_root, FALSE);

        } else {
            const int frame = (w->w_border ? 1 : 0);

            tty_move(w, frame + ((focus / rows) * lb->lb_width), frame + (focus % rows));
        }
    }
    return TRUE;
}


/*
 *  Common edit field implementation
**/
typedef struct {
    uint32_t            ef_magic;               /* structure magic */
    uint32_t            ef_flags;               /* edit flags */
#define EF_FISAUTOCOMPLETE          0x0001
#define EF_FISCOMBO                 0x0002
#define EF_FISNUMERIC               0x0004

    int                 ef_open;                /* presentation status */
    int                 ef_unicode;             /* active presentation character set */
    int                 ef_insmode;             /* insert mode */
    int                 ef_cursor;              /* insert positions/cursor */
    int                 ef_loffset;             /* left offset */
    int                 ef_marked;              /* *true* if initial-edit text is marked, first edit
                                                 *      shall auto-delete field unless cursor is moved prior
                                                 */
    int                 ef_append;              /* *true* is auto-complete buffer should be appended */
    int                 ef_changed;
    int                 ef_seqno;               /* edit sequence number */
    int                 ef_restoreseqno;
    int                 ef_quote;
    int                 ef_format;              /* edit format (TODO) */
#define EDITFORMAT_TEXT             0
#define EDITFORMAT_BOOLEAN          1
#define EDITFORMAT_DATE             2
#define EDITFORMAT_TIME             3
#define EDITFORMAT_NUMERIC          4
#define EDITFORMAT_FLOAT            5

    int                 ef_editable;            /* TRUE/FALSE - field is editable */
    int                 ef_visibility;          /* TRUE/FALSE - character visiblity */
    int                 ef_validate;            /* TRUE/FALSE - field validation active */
    int                 ef_invisiblech;         /* character used during invisible mode */
    int                 ef_maxlength;           /* length limit */
    int                 ef_digits;              /* 0 or greater */
    int                 ef_base;                /* integer numeric 16, 10, 8, 2 or -1 */
    double              ef_increment;

    uint32_t            ef_filters;             /* filters to be applied */
#define EDITFILTER_IMIN             0x0001
#define EDITFILTER_IMAX             0x0002
#define EDITFILTER_DMIN             0x0010
#define EDITFILTER_DMAX             0x0020
    int32_t             ef_imin;                /* integer, lower range */
    int32_t             ef_imax;                /* integer, upper range */
    double              ef_dmin;                /* float, lower range */
    double              ef_dmax;                /* float, upper range */

    int                 ef_length;              /* buffer length, in bytes */
    const char *        ef_placeholder;         /* placeholder text, if any */

#define EWIDECHAR
#if defined(EWIDECHAR)
#define ECHAR WChar_t
    char                ef_xbuffer[ MAX_CMDLINE ];
#else
#define ECHAR char
#endif
    ECHAR               ef_buffer[ MAX_CMDLINE ];
    ECHAR               ef_restore[ MAX_CMDLINE ];
    char                ef_complete[ MAX_CMDLINE ];
} Editfield_t;

static void             ef_init(Editfield_t *ef, uint32_t flags);
static void             ef_open(Editfield_t *ef, WIDGET_t *w);
static void             ef_complete(Editfield_t *ef);
static void             ef_destroy(Editfield_t *ef);
static void             ef_save(Editfield_t *ef);
static void             ef_restore(Editfield_t *ef);
static void             ef_set(Editfield_t *ef, const char *value);
static const char *     ef_get(Editfield_t *ef);
static int              ef_empty(const Editfield_t *ef);
static int              ef_assign(Editfield_t *ef, const WIDGETDATA_t *data);
static int              ef_value_set(Editfield_t *ef, const WIDGETARG_t p1, const WIDGETDATA_t *data);
static int              ef_value_get(Editfield_t *ef, const WIDGETARG_t p1, WIDGETDATA_t *data);
static int              ef_key(Editfield_t *ef, WIDGET_t *w, int key, int (*validate)(Editfield_t *, WIDGET_t *w, void *), void *arg);
static void             ef_paint(Editfield_t *ef, WIDGET_t *w);
static void             ef_caret(Editfield_t *ef, WIDGET_t *w);


static void
ef_init(Editfield_t *ef, uint32_t flags)
{
    ef->ef_flags        = flags;
    ef->ef_open         = FALSE;
    ef->ef_unicode      = tty_isunicode(TRUE);
    ef->ef_insmode      = TRUE;
    ef->ef_cursor       = 0;
    ef->ef_loffset      = 0;
    ef->ef_marked       = FALSE;
    ef->ef_append       = FALSE;
    ef->ef_changed      = 0;
    ef->ef_format       = EDITFORMAT_TEXT;
    ef->ef_seqno        = 0;
    ef->ef_length       = _countof(ef->ef_buffer)-1;
    ef->ef_editable     = TRUE;
    ef->ef_visibility   = TRUE;
    ef->ef_validate     = FALSE;
    ef->ef_invisiblech  = '*';
    ef->ef_maxlength    = MAX_CMDLINE;
    ef->ef_base         = 10;
    ef->ef_digits       = 0;
    ef->ef_increment    = 1;
    ef->ef_filters      = 0;
    ef->ef_placeholder  = NULL;
    ef->ef_buffer[0]    = 0;
#if defined(EWIDECHAR)
    ef->ef_xbuffer[0]   = 0;
#endif
    ef->ef_complete[0]  = 0;
    ef->ef_restore[0]   = 0;
    ef->ef_restoreseqno = 0;
}


static void
ef_open(Editfield_t *ef, WIDGET_t *w)
{
    __CUNUSED(w)
    ef->ef_open         = TRUE;
}


static int
ef_change(Editfield_t *ef, WIDGET_t *w)
{
    if (!ef->ef_changed ||
            (!ef->ef_validate || widget_callback(w, DLGE_VALIDATE, 0, 0))) {
        widget_callback(w, DLGE_CHANGE, 0, 0);
        return TRUE;
    }
    return FALSE;
}


static void
ef_complete(Editfield_t *ef)
{
    ef->ef_quote        = FALSE;
    ef->ef_cursor       = 0;
    ef->ef_loffset      = 0;
    ef->ef_marked       = FALSE;
    ef->ef_append       = FALSE;
    ef->ef_changed      = 0;
    ef->ef_complete[0]  = 0;
    ef->ef_open         = FALSE;
}


static void
ef_destroy(Editfield_t *ef)
{
    __CUNUSED(ef)
}


static void
ef_save(Editfield_t *ef)
{
    const ECHAR *buffer = ef->ef_buffer;
#if defined(EWIDECHAR)
    const int diff = Wcscmp(buffer, ef->ef_restore);
    Wcscpy(ef->ef_restore, buffer);
#else
    const int diff = strcmp(buffer, ef->ef_restore);
    strcpy(ef->ef_restore, buffer);
#endif

    ef->ef_restoreseqno = ef->ef_seqno;
    if (diff) ++ef->ef_seqno;
    ef->ef_quote        = FALSE;
    ef->ef_cursor       = 0;
    ef->ef_loffset      = 0;
    ef->ef_marked       =
        (((EF_FISAUTOCOMPLETE & ef->ef_flags) || 1 == ef->ef_seqno) && buffer[0] ? TRUE : FALSE);
    ef->ef_append       = FALSE;
    ef->ef_changed      = 0;
    ef->ef_complete[0]  = 0;
}


static void
ef_restore(Editfield_t *ef)
{
    assert(sizeof(ef->ef_buffer) == sizeof(ef->ef_restore));
    memcpy(ef->ef_buffer, (const void *)ef->ef_restore, sizeof(ef->ef_buffer));
    ef->ef_seqno        = ef->ef_restoreseqno;
    ef->ef_quote        = FALSE;
    ef->ef_changed      = 0;
    ef->ef_complete[0]  = 0;
}


static void
ef_set(Editfield_t *ef, const char *value)
{
    ef->ef_seqno        = 0;
    ef->ef_quote        = FALSE;
    ef->ef_marked       = FALSE;
    ef->ef_append       = FALSE;
#if defined(EWIDECHAR)
    Wcsfromutf8((value ? value : ""), ef->ef_buffer, _countof(ef->ef_buffer));
#else
    strxcpy(ef->ef_buffer, (value ? value : ""), sizeof(ef->ef_buffer));
#endif
    ef->ef_complete[0]  = 0;
    ++ef->ef_changed;
}


static const char *
ef_get(Editfield_t *ef)
{
#if defined(EWIDECHAR)
    ef->ef_xbuffer[0] = 0;
    if (ef->ef_buffer[0]) {
        Wcstoutf8(ef->ef_buffer, ef->ef_xbuffer, sizeof(ef->ef_xbuffer));
    }
    return ef->ef_xbuffer;
#else
    return ef->ef_buffer;
#endif
}


static int
ef_empty(const Editfield_t *ef)
{
    return (*ef->ef_buffer == 0);
}


static int
ef_assign(Editfield_t *ef, const WIDGETDATA_t *data)
{
    switch (data->d_type) {
    case D_INT: {
            char buffer[32];
            sxprintf(buffer, sizeof(buffer), "%" ACCINT_FMT, data->d_u.ivalue);
            ef_set(ef, buffer);
        }
        break;
    case D_FLOAT: {
            char buffer[32];
            sxprintf(buffer, sizeof(buffer), "%" ACCFLOAT_FMT, data->d_u.fvalue);
            ef_set(ef, buffer);
        }
        break;
    case D_STR:
        ef_set(ef, data->d_u.svalue);
        break;
    case D_NULL:
    case D_NONE:
        ef_set(ef, "");
        break;
    default:
        return FALSE;
    }
    return TRUE;
}


static int
ef_value_set(Editfield_t *ef, const WIDGETARG_t p1, const WIDGETDATA_t *data)
{
    const uint16_t attr = DIALOGARGLO(p1);
    int32_t value;

    switch (attr) {
    case DLGA_VALUE:
        return ef_assign(ef, data);
    case DLGA_EDEDITABLE:
        if ((value = widgetdata_int32(data)) >= 0) {
            ef->ef_editable = (value ? TRUE : FALSE);
        }
        break;
    case DLGA_EDMAXLENGTH:
        if ((value = widgetdata_int32(data)) >= -1) {
            if (value <= 0 ||
                    (ef->ef_maxlength = value) > MAX_CMDLINE) {
                ef->ef_maxlength = MAX_CMDLINE;
            }
        }
        break;
    case DLGA_EDVISIBLITY:
        if ((value = widgetdata_int32(data)) >= 0) {
            ef->ef_visibility = (value ? TRUE : FALSE);
        }
        break;
    case DLGA_EDINVISIBLECHAR:
        if ((value = widgetdata_int32(data)) >= 0) {
            ef->ef_invisiblech = value;         /* 0=hide all, otherwise character-value */
        } else if (-1 == value) {
            ef->ef_invisiblech = '*';           /* default character, MCHAR??? */
        }
        break;
    case DLGA_EDPOSITION:
        if ((value = widgetdata_int32(data)) >= 0) {
            if ((ef->ef_cursor = value) >= ef->ef_maxlength) {
                ef->ef_cursor = (ef->ef_maxlength - 1);
            }
        }
        break;
    case DLGA_EDPLACEHOLDER: {
            const char *placeholder = widgetdata_str(data);

            chk_free((void *)ef->ef_placeholder);
            ef->ef_placeholder =
                (placeholder && *placeholder ? chk_salloc(placeholder) : NULL);
        }
        break;
    case DLGA_EDVALIDATE:
        if ((value = widgetdata_int32(data)) >= 0) {
            ef->ef_validate = (value ? TRUE : FALSE);
        }
        break;
    }
    return FALSE;
}


static int
ef_value_get(Editfield_t *ef, const WIDGETARG_t p1, WIDGETDATA_t *data)
{
    const uint16_t attr = DIALOGARGLO(p1);

    switch (attr) {
    case DLGA_VALUE:
        data->d_u.svalue = ef_get(ef);
        data->d_type = D_STR;
        return TRUE;
    case DLGA_EDEDITABLE:
        data->d_u.ivalue = ef->ef_editable;
        data->d_type = D_INT;
        break;
    case DLGA_EDMAXLENGTH:
        data->d_u.ivalue = ef->ef_maxlength;
        data->d_type = D_INT;
        break;
    case DLGA_EDVISIBLITY:
        data->d_u.ivalue = ef->ef_visibility;
        data->d_type = D_INT;
        break;
    case DLGA_EDINVISIBLECHAR:
        data->d_u.ivalue = ef->ef_invisiblech;
        data->d_type = D_INT;
        break;
    case DLGA_EDPOSITION:
        data->d_u.ivalue = ef->ef_cursor;
        data->d_type = D_INT;
        break;
    case DLGA_EDPLACEHOLDER:
        data->d_u.svalue = (ef->ef_placeholder ? ef->ef_placeholder : "");
        data->d_type = D_STR;
        break;
    case DLGA_EDVALIDATE:
        data->d_u.ivalue = ef->ef_validate;
        data->d_type = D_INT;
        break;
    }
    return FALSE;
}


/*  Function:           ef_key
 *      Edit-file keyboard event processor.
 *
 *  Parameters:
 *      w -                 Widget.
 *      e -                 Edit file object.
 *      key -               Key identifier.
 *      validate -          Field validation.
 *      arg -               Validator argument.
 *
 *  Navigation/actions keys:

        Key                         Function

        Right, Left                 Move cursor the back/forward one character.

        Ctrl+Right, Ctrl+Left       Move cursor the start/end of the current word.
                                    or for numeric field decrement and increment.

        Home, End                   Move to first/last character within the edit field.

        Alt+I, Ctrl-O               Toggle insert/overstrike mode.

        DEL                         Delete character under the cursor.

        Alt-D                       Delete current line.

        Alt+K                       Delete from cursor to the end of line.

        Backspace, Ctrl+H           Delete character prior to the cursor.

        Insert                      Paste from scape.

        ESC                         Abort current edit, restoring original content.

        Enter                       Process change.

        Alt+Q, Ctrl+Q               Quote next character.

        Ctrl+A (*)                  Move cursor to beginning of line.

        Ctrl+D (*)                  Delete character under cursor.

        Ctrl+K (*)                  Delete from cursor to the end of line.

        Ctrl+X, Ctrl+U (*)          Delete current line.

        Ctrl+V, Ctrl+Y (*)          Paste from clipboard.

    TODO:

        Ctrl+W (*)                  Cut to clipboard.

        Ctrl+Del, Alt+D (*)         Delete current cursor to end of word.

        Ctrl+Backspace (*)          Delete from cursor to start of word.

        Ctrl+Space (*)              Set mark.

        Alt+Space (*)               Delete all whitespace around cursor, reinsert singe space.

        Alt+\ (*)                   Delete all whitespace around cursor.

        (*) Emacs style key mappings.

 *  Returns:
 *      TRUE is the key was consumed, otherwise FALSE.
 *
 *  Note:
 *      ALT keys *may not* be caught due to HOTKEY usage.
 */
static int
ef_key(Editfield_t *ef, WIDGET_t *w, int key, int (*validate)(Editfield_t *, WIDGET_t *w, void *), void *arg)
{
    ECHAR previous_buffer[ MAX_CMDLINE ];
    ECHAR *buf = ef->ef_buffer;
    double dval = 0;
    int oipos = ef->ef_cursor, ipos = oipos;
    int repaint = FALSE;
    int edit = FALSE;

#if defined(EWIDECHAR)
    Wcsncpy(previous_buffer, (const WChar_t *)buf, _countof(previous_buffer));
#else
    strxcpy(previous_buffer, (const char *)buf, sizeof(previous_buffer));
#endif

    if (ef->ef_quote) {
        ef->ef_quote = FALSE;
        goto quote;
    }

    switch (key) {
    case KEY_ENTER:         /* <Enter>      Except */
        if (!ef_change(ef, w)) {
            ef_restore(ef);
            ipos = 0;
        }
        break;

    case KEY_ESC:           /* <ESC>        Abort edit */
        ef_restore(ef);
        ipos = 0;
        break;

    case KEY_RIGHT:         /* <Right>      Cursor right */
        if (ef->ef_marked) {
            repaint = TRUE, ef->ef_marked = 0;
        }
        if (buf[ipos]) {
            ++ipos;
        }
        break;

    case KEY_LEFT:          /* <Left>       Cursor left */
        if (ef->ef_marked) {
            repaint = TRUE, ef->ef_marked = 0;
        }
        if (ipos > 0) {
            --ipos;
        }
        break;

    case KEY_HOME:          /* <Home>       Start of line */
    case CTRL_A:
        if (ef->ef_marked) {
            repaint = TRUE, ef->ef_marked = 0;
        }
        ipos = 0;
        break;

    case KEY_END:           /* <End>        End of line */
        if (ef->ef_marked) {
            repaint = TRUE, ef->ef_marked = 0;
        }
#if defined(EWIDECHAR)
        ipos = (int)Wcslen(buf);
#else
        ipos = (int)strlen(buf);
#endif
        break;

    case KEY_WLEFT:         /* <Left>       Cursor word left/decrement */
    case KEY_WLEFT2: {
            if (EF_FISNUMERIC & ef->ef_flags) {
decrement:;     if (*buf) {
#if defined(EWIDECHAR)
                    dval = Wcstod(buf, NULL);
#else
                    dval = atof(buf);
#endif
                }
                dval -= ef->ef_increment;
#if defined(EWIDECHAR)
                if (ef->ef_digits <= 0) {
                    ipos = Wsnprintf(buf, _countof(ef->ef_buffer), "%d", (int) dval);
                } else {
                    ipos = Wsnprintf(buf, _countof(ef->ef_buffer), "%*f", ef->ef_digits, dval);
                }
#else
                if (ef->ef_digits <= 0) {
                    ipos = sprintf(buf, "%d", (int) dval);
                } else {
                    ipos = sprintf(buf, "%*f", ef->ef_digits, dval);
                }
#endif
                edit = TRUE;
            } else {
                while (isspace(buf[ipos]) && ipos > 0) {
                    --ipos;
                }
                while (!isspace(buf[ipos]) && ipos > 0) {
                    --ipos;
                }
            }
        }
        break;

    case KEY_WRIGHT:        /* <Right>      Cursor word right/increment */
    case KEY_WRIGHT2: {
            if (EF_FISNUMERIC & ef->ef_flags) {
increment:;     dval = 0;
                if (*buf) {
#if defined(EWIDECHAR)
                    dval = Wcstod(buf, NULL);
#else
                    dval = atof(buf);
#endif
                }
                dval += ef->ef_increment;
#if defined(EWIDECHAR)
                if (ef->ef_digits <= 0) {
                    ipos = Wsnprintf(buf, _countof(ef->ef_buffer), "%d", (int) dval);
                } else {
                    ipos = Wsnprintf(buf, _countof(ef->ef_buffer), "%*f", ef->ef_digits, dval);
                }
#else
                if (ef->ef_digits <= 0) {
                    ipos = sprintf(buf, "%d", (int) dval);
                } else {
                    ipos = sprintf(buf, "%*f", ef->ef_digits, dval);
                }
#endif
                edit = TRUE;
            } else {
                while (isspace(buf[ipos]) && buf[ipos]) {
                    ++ipos;
                }
                while (!isspace(buf[ipos]) && buf[ipos]) {
                    ++ipos;
                }
            }
        }
        break;

    case CTRL_H:            /* <Ctrl-h>     Delete previous character */
        if (ef->ef_editable) {
            if (ef->ef_marked) {
                repaint = TRUE, ef->ef_marked = 0;
            }
            if (ipos > 0) {
                --ipos;
#if defined(EWIDECHAR)
                Wcscpy(buf + ipos, buf + ipos + 1);
#else
                strcpy(buf + ipos, buf + ipos + 1);
#endif
                edit = TRUE;
            }
        }
        break;

    case KEY_DEL:           /* <DEL>        delete character under the cursor */
    case KEY_DELETE:
    case CTRL_D:
        if (ef->ef_editable) {
            if (ef->ef_marked) {
                ef->ef_marked = 0;
                memset(ef->ef_complete, 0, sizeof(ef->ef_complete));
                memset(buf, 0, sizeof(ef->ef_buffer));
                edit = TRUE;
                ipos = 0;

            } else if (buf[ipos]) {
#if defined(EWIDECHAR)
                Wcscpy(buf + ipos, buf + ipos + 1);
#else
                strcpy(buf + ipos, buf + ipos + 1);
#endif
                edit = TRUE;
            }
        }
        break;

    case ALT_K:             /* <Alt-k>      delete characters to end of input */
    case CTRL_K:
        if (ef->ef_editable) {
            buf[ipos] = '\0';
            edit = TRUE;
        }
        break;

    case ALT_D:             /* <Alt-d>      delete line/buffer */
    case CTRL_X:
    case CTRL_U:
        if (ef->ef_editable) {
            buf[0] = '\0';
            edit = TRUE;
            ipos = 0;
        }
        break;

    case KEY_INS:           /* <Ins>        insert scrap */
    case KEY_PASTE:
    case CTRL_V:
    case CTRL_Y:
        if (ef->ef_editable) {
            const char *cpp;
            int scount;

            k_seek();
            if ((scount = k_read(&cpp)) <= 0) {
                ttbeep();
            } else {
                while (ipos < ef->ef_length && scount--) {
                    if (ef->ef_insmode) {
#if defined(EWIDECHAR)
                        Wmemmove(buf + ipos + 1, (const WChar_t *)(buf + ipos), (size_t)ef->ef_length - ipos);
#else
                        memmove(buf + ipos + 1, (const char *)(buf + ipos), (size_t)ef->ef_length - ipos);
#endif
                    }
                    if (0 == buf[ ipos ]) {
                        buf[ ipos+1 ] = '\0';
                    }
                    buf[ ipos++ ] = *cpp++;
                }
                edit = TRUE;
            }
        }
        break;

    case KEY_TAB:           /* <Tab> */
    case KEY_NEXT:
    case BACK_TAB:
    case KEY_PREV:
    case CTRL_TAB:
    case ALT_TAB:
        return FALSE;

    case ALT_Q:             /* Quote the next input character */
    case CTRL_Q:
        if (ef->ef_editable) {
            if (0 == (EF_FISNUMERIC & ef->ef_flags)) {
                ef->ef_quote = TRUE;
            }
        }
        break;

    case ALT_I:             /* Toggle insert/overstrike mode */
    case ALT_O:
    case CTRL_O:
        if (ef->ef_editable) {
            ef->ef_insmode = !ef->ef_insmode;
        }
        break;

    default:                /* Others */
        if (! ef->ef_editable) {
            return FALSE;
        }

        if (EF_FISNUMERIC & ef->ef_flags) {
            if ('+' == key) {
                goto increment;

            } else if ('-' == key) {
                goto decrement;

            } else if ('.' == key) {
#if defined(EWIDECHAR)
                if (ef->ef_digits <= 0 || Wcschr(buf, '.')) {
#else
                if (ef->ef_digits <= 0 || strchr(buf, '.')) {
#endif
                    return FALSE;
                }

            } else if (key < '0' || key > '9') {
                return FALSE;
            }

        } else {
            if (key < 32 || key > 128) {        /* non ascii */
                return FALSE;
            }
        }

    quote:;
        if (ef->ef_marked) {                    /* zap marked text */
#if defined(EWIDECHAR)
            Wmemset(buf, 0, ef->ef_length);
#else
            memset(buf, 0, ef->ef_length);
#endif
            ef->ef_marked = 0;
            edit = TRUE;
            ipos = 0;
        }

        if (ipos >= ef->ef_length) {
            ttbeep();                           /* end-of-buffer */
        } else {
            if (ef->ef_insmode) {               /* insert mode */
#if defined(EWIDECHAR)
                Wmemmove(buf + ipos + 1, (const WChar_t *)(buf + ipos), (size_t)ef->ef_length - ipos);
#else
                memmove(buf + ipos + 1, (const char *)(buf + ipos), (size_t)ef->ef_length - ipos);
#endif
            }
            if (0 == buf[ ipos ]) {             /* make sure buffer is terminated */
                buf[ ipos+1 ] = '\0';
            }
            buf[ ipos++ ] = (char)key;          /* insert new key */
            edit = TRUE;
        }
        break;
    }

    ef->ef_cursor = ipos;
    if (edit) {
        ++ef->ef_changed;
        if (validate) {
            if (! validate(ef, w, arg)) {       /* reject, restore previous value */
#if defined(EWIDECHAR)
                Wcsncpy(buf, (const WChar_t *)previous_buffer, _countof(ef->ef_buffer));
#else
                strxcpy(buf, (const char *)previous_buffer, sizeof(ef->ef_buffer));
#endif
                --ef->ef_changed;
                ef->ef_cursor = oipos;
                edit = FALSE;
            } else {
                if (ef->ef_complete[0]) {       /* auto-complete, trim cursor */
#if defined(EWIDECHAR)
                    ipos = (int)Wcslen(buf);
#else
                    ipos = (int)strlen(buf);
#endif
                    if (ipos < ef->ef_cursor) {
                        ef->ef_cursor = ipos;
                    }
                }
            }
        }
        repaint = TRUE;
    }

    if (repaint) {
        widget_send(w, WIDGET_PAINT, 0, 0);
    }
    return TRUE;
}


//
//  Unicode Resources:
//      o 0x25BC    BLACK DOWN-POINTING TRIANGLE
//      o 0x25C2    BLACK LEFT-POINTING TRIANGLE
//      o 0x25B8    BLACK RIGHT-POINTING TRIANGLE
//
static void
ef_paint(Editfield_t *ef, WIDGET_t *w)
{
    const LINEATTR normal = ((WIDGET_FGREYED & w->w_flags) ? EDIT_GREYED : EDIT_NORMAL);
    const LINEATTR attr   = (HasFocus(w) ? (ef->ef_marked ? EDIT_COMPLETE : EDIT_FOCUS) : normal);
    const int32_t cols    =
                   (EF_FISCOMBO    & ef->ef_flags ? (w->w_cols > 3 ? w->w_cols - 3 : 1) :
                    (EF_FISNUMERIC & ef->ef_flags ? (w->w_cols > 4 ? w->w_cols - 4 : 1) : w->w_cols));

    const char *complete  = (ef->ef_append ? ef->ef_complete : "");
    const int placeholder = (!HasFocus(w) && ef->ef_placeholder && ef_empty(ef) && !*complete ? 1 : 0);

    int32_t len2 = (int32_t)strlen(complete);
#if defined(EWIDECHAR)
    int32_t len = (int32_t)(placeholder ? strlen(ef->ef_placeholder) : Wcslen(ef->ef_buffer));
#else
    int32_t len = (int32_t)(placeholder ? strlen(ef->ef_placeholder) : strlen(ef->ef_buffer));
#endif
    int32_t offset = ef->ef_loffset;
    int32_t cursor = 0;

    if (ef->ef_cursor < ef->ef_loffset) {       /* margins */
        ef->ef_loffset = ef->ef_cursor;
    } else if ((ef->ef_cursor - ef->ef_loffset) > cols) {
        ef->ef_loffset = ef->ef_cursor - cols;
    }

    if (offset < len) {                         /* text */
        if ((len -= offset) > cols) {
            len = cols;
        }

        if (placeholder) {                      /* placeholder text */
            const char *buffer = ef->ef_placeholder + offset;

            tty_move(w, 0, 0);
            tty_strn(attr, buffer, len);
            cursor += len;

        } else if (ef->ef_visibility) {         /* normal/visible mode text */
            const ECHAR *buffer = ef->ef_buffer + offset;

            while (len-- > 0) {
                tty_move(w, cursor, 0);
                cursor += tty_char(attr, *buffer++);
            }

        } else {                                /* invisible, only show previous character when focused */
            const int vcharacter = ef->ef_invisiblech;

            if (vcharacter > 0) {               /* 0=hidden text (ie. no user feeback) */
                const ECHAR *buffer = ef->ef_buffer + offset;
                const int vpos =
                    (HasFocus(w) && ef->ef_changed ? (ef->ef_cursor - offset) - 1 : -1);

                while (len-- > 0) {
                    tty_move(w, cursor, 0);     /* only if last changed character */
                    cursor += tty_char(attr, 0 == len && vpos == cursor ? *buffer : vcharacter);
                    ++buffer;
                }
            }
        }
        offset = 0;

    } else {
        offset -= len;
    }

    if (ef->ef_visibility) {
        if (len2 > 0 && offset < len2) {        /* auto-complete, unless invisible */
            if ((cursor + (len2 - offset)) > cols) {
                len2 = cols - cursor;
            }
            tty_move(w, cursor, 0);
            tty_strn((HasFocus(w) ? EDIT_COMPLETE : normal), ef->ef_complete + offset, len2);
            cursor += len2;
        }
    }

    while (cursor < cols) {                     /* padding */
        tty_move(w, cursor++, 0);
        tty_char(normal, ' ');
    }

    if (EF_FISCOMBO & ef->ef_flags) {           /* [v] */
        static const int unicode_icon[] =
                { '[', 0x25BC, ']', 0 };
        static const int ascii_icon[] =
                { '[', 'v', ']', 0 };
        const int *icon =
                (ef->ef_unicode ? unicode_icon : ascii_icon);

        while (*icon) {
            tty_move(w, cursor++, 0);
            tty_char(BUTTON_FOCUS, *icon++);
        }

    } else if (EF_FISNUMERIC & ef->ef_flags) {  /* [<>] or [-+] */
        static const int unicode_icon[] =
                { '[', 0x25C2, 0x25B8, ']', 0 };
        static const int ascii_icon[] =
                { '[', '-', '+', ']', 0 };
        const int *icon =
                (ef->ef_unicode ? unicode_icon : ascii_icon);

        while (*icon) {
            tty_move(w, cursor++, 0);
            tty_char(BUTTON_FOCUS, *icon++);
        }
    }

    ef_caret(ef, w);                            /* position cursor */
}


static void
ef_caret(Editfield_t *ef, WIDGET_t *w)
{
    tty_move(w, ef->ef_cursor - ef->ef_loffset, 0);
}


/*
 *  Group:  List Box
**/
typedef struct {
    WIDGET_t            lb_widget;              /* base widget */
    Listbox_t           lb_impl;
} WListbox_t;

static int              listbox_callback(WListbox_t *g, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
listbox_new(void)
{
    WListbox_t *lb;

    if (NULL == (lb = (WListbox_t *)tty_new(sizeof(WListbox_t), (WIDGETCB_t)listbox_callback))) {
        return NULL;
    }
    lb_init(&lb->lb_impl, LB_FHASSTRINGS);
    return (WIDGET_t *)lb;
}


static int
listbox_callback(WListbox_t *l, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)l;
    Listbox_t *lb = &l->lb_impl;

    switch (msg) {
    case WIDGET_INIT:           /* initialise */
        if (w->w_reqrows <= 0 &&
                (w->w_reqrows = w->w_rows) <= 0) {
            w->w_reqrows = 1;
        }
        if (w->w_reqcols <= 0 &&
                (w->w_reqcols = w->w_cols) <= 0) {
            w->w_reqcols = 1;
        }
        w->w_flags |= WIDGET_FTABSTOP;
        w->w_border = 1;
        return TRUE;

    case WIDGET_READY:          /* dialog session ready */
        return TRUE;

    case WIDGET_SET: {          /* set attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            w->w_uflags |= WTTY_FDIRTY;
            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:
            case DLGA_LBELEMENTS:
                lb_item_elements(lb, LB_POSNORM, data);
                break;
            case DLGA_LBINSERT:
                lb_item_elements(lb, DIALOGARGHI(p1), data);
                break;
            case DLGA_LBREMOVE:
                lb_item_remove(lb, DIALOGARGHI(p1));
                break;
            case DLGA_LBCLEAR:
                lb_item_zap(lb);
                break;

            case DLGA_LBCOUNT:
            case DLGA_LBSORTMODE:
            case DLGA_LBHASSTRINGS:
            case DLGA_LBICASESTRINGS:
            case DLGA_LBDUPLICATES:
            case DLGA_LBCURSOR:
            case DLGA_LBACTIVE:
            case DLGA_LBDISPLAYTEXT:
            case DLGA_LBDISPLAYTEXTLEN:
            case DLGA_LBTEXT:
            case DLGA_LBTEXTLEN:
            case DLGA_LBPAGEMODE:
            case DLGA_LBINDEXMODE:
            case DLGA_LBROWS:
            case DLGA_LBCOLS:
            case DLGA_LBCOLUMNS:
            case DLGA_LBWIDTH:
                return lb_value_set(lb, p1, data);

            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_GET: {          /* get attribute */
            WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:                    /* return active 'section' (if any) */
            case DLGA_LBELEMENTS:
                return lb_item_value(lb, lb->lb_active, data);

            case DLGA_LBINSERT:
            case DLGA_LBREMOVE:
            case DLGA_LBCLEAR:
                return FALSE;

            case DLGA_LBCOUNT:
            case DLGA_LBSORTMODE:
            case DLGA_LBHASSTRINGS:
            case DLGA_LBICASESTRINGS:
            case DLGA_LBDUPLICATES:
            case DLGA_LBCURSOR:
            case DLGA_LBACTIVE:
            case DLGA_LBDISPLAYTEXT:
            case DLGA_LBDISPLAYTEXTLEN:
            case DLGA_LBTEXT:
            case DLGA_LBTEXTLEN:
            case DLGA_LBPAGEMODE:
            case DLGA_LBINDEXMODE:
            case DLGA_LBROWS:
            case DLGA_LBCOLS:
            case DLGA_LBCOLUMNS:
            case DLGA_LBWIDTH:
                return lb_value_get(lb, p1, data);

            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        break;

    case WIDGET_SETFOCUS:       /* cursor/focus request */
        if ((WIDGET_FGREYED|WIDGET_FHIDDEN) & w->w_flags) {
            return FALSE;
        }
        if (p1) {
            lb_open(lb, w);
        } else {
            lb_complete(lb);
        }
        return TRUE;

    case WIDGET_MOUSE:          /* mouse event */
        if (BUTTON1_DOWN == p2) {
            int border = w->w_border;
            int y = GetYParam(p1);

            if (lb_select(lb, w, y - border)) {
                listbox_callback(l, WIDGET_KEYDOWN, ' ', 0);
            }
        }
        return TRUE;

    case WIDGET_KEYDOWN:        /* keyboard event */
        return lb_key(lb, w, p1);

    case WIDGET_PAINT:          /* widget display */
        if (0 == (w->w_flags & WIDGET_FHIDDEN)) {
            lb_paint(lb, w, p1);
        }
        return TRUE;

    case WIDGET_CARET:          /* cursor selection */
        lb_caret(lb, w);
        return TRUE;

    case WIDGET_DESTROY:        /* destructor */
        lb_destroy(lb);
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}


/*
 *  Group:  Edit Field
**/
typedef struct {
    WIDGET_t            ef_widget;              /* base widget */
    Editfield_t         ef_impl;
} WEditfield_t;

static int              editfield_callback(WEditfield_t *g, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
editfield_new(void)
{
    WEditfield_t *ef;

    if (NULL == (ef = (WEditfield_t *)tty_new(sizeof(WEditfield_t), (WIDGETCB_t)editfield_callback))) {
        return NULL;
    }
    ef_init(&ef->ef_impl, 0);
    return (WIDGET_t *)ef;
}


static int
editfield_callback(WEditfield_t *e, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)e;
    Editfield_t *ef = &e->ef_impl;

    switch (msg) {
    case WIDGET_INIT:           /* initialise */
        if (w->w_reqrows <= 0) w->w_reqrows = 1;
        if (w->w_reqcols <= 0) w->w_reqcols = 1;
        w->w_flags |= WIDGET_FTABSTOP;
        return TRUE;

    case WIDGET_READY:          /* dialog session ready */
        return TRUE;

    case WIDGET_SET: {          /* set attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            w->w_uflags |= WTTY_FDIRTY;
            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:
            case DLGA_EDEDITABLE:
            case DLGA_EDMAXLENGTH:
            case DLGA_EDVISIBLITY:
            case DLGA_EDINVISIBLECHAR:
            case DLGA_EDPOSITION:
            case DLGA_EDPLACEHOLDER:
            case DLGA_EDVALIDATE:
                return ef_value_set(ef, p1, data);
            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_GET: {          /* get attribute */
            WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:
            case DLGA_EDEDITABLE:
            case DLGA_EDMAXLENGTH:
            case DLGA_EDVISIBLITY:
            case DLGA_EDINVISIBLECHAR:
            case DLGA_EDPOSITION:
            case DLGA_EDPLACEHOLDER:
            case DLGA_EDVALIDATE:
                return ef_value_get(ef, p1, data);
            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        break;

    case WIDGET_SETFOCUS:       /* cursor/focus request */
        if ((WIDGET_FGREYED|WIDGET_FHIDDEN) & w->w_flags) {
            return FALSE;
        }

        if (p1) {
            ef_save(ef);
        } else {
            ef_change(ef, w);
            ef_complete(ef);
        }
        return TRUE;

    case WIDGET_KEYDOWN:        /* keyboard event */
        return ef_key(ef, w, p1, NULL, NULL);

    case WIDGET_PAINT:          /* widget display */
        if (0 == (w->w_flags & WIDGET_FHIDDEN)) {
            ef_paint(ef, w);
        }
        return TRUE;

    case WIDGET_CARET:          /* cursor selection */
        ef_caret(ef, w);
        return TRUE;

    case WIDGET_DESTROY:        /* destructor */
        ef_destroy(ef);
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}


/*
 *  Group:  Numeric Field
**/
typedef struct {
    WIDGET_t            nf_widget;              /* base widget */
    Editfield_t         nf_impl;
} WNumericfield_t;

static int              numericfield_callback(WNumericfield_t *g, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
numericfield_new(void)
{
    WNumericfield_t *ef;

    if (NULL == (ef = (WNumericfield_t *)tty_new(sizeof(WNumericfield_t), (WIDGETCB_t)numericfield_callback))) {
        return NULL;
    }
    ef_init(&ef->nf_impl, EF_FISNUMERIC);
    return (WIDGET_t *)ef;
}


static int
numericfield_callback(WNumericfield_t *n, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)n;
    Editfield_t *ef = &n->nf_impl;

    switch (msg) {
    case WIDGET_INIT:           /* initialise */
        if (w->w_reqrows <= 0) w->w_reqrows = 1;
        if (w->w_reqcols <= 0) w->w_reqcols = 1;
        w->w_flags |= WIDGET_FTABSTOP;
        return TRUE;

    case WIDGET_READY:          /* dialog session ready */
        return TRUE;

    case WIDGET_SET: {          /* set attribute*/
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            w->w_uflags |= WTTY_FDIRTY;
            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:
            case DLGA_EDEDITABLE:
            case DLGA_EDMAXLENGTH:
            case DLGA_EDVISIBLITY:
            case DLGA_EDINVISIBLECHAR:
            case DLGA_EDPOSITION:
            case DLGA_EDPLACEHOLDER:
            case DLGA_EDVALIDATE:
                return ef_value_set(ef, p1, data);
            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_GET: {          /* get attribute */
            WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:
            case DLGA_EDEDITABLE:
            case DLGA_EDMAXLENGTH:
            case DLGA_EDVISIBLITY:
            case DLGA_EDINVISIBLECHAR:
            case DLGA_EDPOSITION:
            case DLGA_EDPLACEHOLDER:
            case DLGA_EDVALIDATE:
                return ef_value_get(ef, p1, data);
            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        break;

    case WIDGET_SETFOCUS:       /* cursor/focus request */
        if ((WIDGET_FGREYED|WIDGET_FHIDDEN) & w->w_flags) {
            return FALSE;
        }

        if (p1) {
            ef_save(ef);
        } else {
            ef_change(ef, w);
            ef_complete(ef);
        }
        return TRUE;

    case WIDGET_KEYDOWN:        /* keyboard event */
        return ef_key(ef, w, p1, NULL, NULL);

    case WIDGET_PAINT:          /* widget display */
        if (0 == (w->w_flags & WIDGET_FHIDDEN)) {
            ef_paint(ef, w);
        }
        return TRUE;

    case WIDGET_CARET:          /* cursor selection */
        ef_caret(ef, w);
        return TRUE;

    case WIDGET_DESTROY:        /* destructor */
        ef_destroy(ef);
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}


/*
 *  Group:  Combo Field

    Key actions:
        <focus>         Opens a drop-down of available selections.

        <enter>         Selects the current selection and if auto-move moves the focus to
                        the next focusable element on the screen.

        <esc>           Aborts the current edit, restoring the original value.

        <Ctrl-down>     Opens the drop-down list and gives it focus.

        <Ctrl-up>       Closes the drop-down list and returns focus to the combo-box control.

        <down>          Moves the selection down one item.

        <home>          Moves the selection to the top of the collection.

        <pgdn>          Moves the list cursor down one page.

        <pgup>          Moves the list cursor up one page.

        <end>           Moves the selection to the bottom of the collection.

        <up>            Moves the selection up one item.

        <tab>           Moves focus to the next focusable element on the screen.

        <shift-tab>     Moves focus to the previous focusable element on the screen.

**/
typedef struct {
    WIDGET_t            cf_widget;              /* base widget */
    int                 cf_open;                /* presentation status */
    int16_t             cf_editable;            /* *true* if editable */
    int16_t             cf_relaxmode;           /* edit-text values relaxation mode */
#define CB_RELAXMODE_NONE           0
#define CB_RELAXMODE_NONBLANK       1
#define CB_RELAXMODE_ANY            2

    int16_t             cf_autocompletemode;
#define CB_AUTOMODE_NONE            0           /* no auto-completion */
#define CB_AUTOMODE_SUGGEST         1           /* populate auxiliary drop-down list with suggested completion(s) */
#define CB_AUTOMODE_APPEND          2           /* append/auto-complete with most likely candidate */
#define CB_AUTOMODE_SUGGEST_APPEND  3           /* suggest and append */

    int16_t             cf_popupmode;           /* popup mode */
#define CB_POPUPMODE_HIDDEN         -1          /* hidden, used only for auto-completion/validation */
#define CB_POPUPMODE_AUTO           0           /* automatic, if non editable OPEN otherwise based on completion-mode */
#define CB_POPUPMODE_NORMAL         1           /* dynamic, user may open/close (default) */
#define CB_POPUPMODE_OPEN           2           /* visible upto the combo becoming focused */
#define CB_POPUPMODE_FOCUS          3           /* auto focused */

    int16_t             cf_popupstate;          /* popup state */
#define CB_POPUPSTATE_NONE          0
#define CB_POPUPSTATE_VISIBLE       1
#define CB_POPUPSTATE_FOCUS         2

    Editfield_t         cf_editimpl;
    Listbox_t           cf_listimpl;
    char                cf_buffer[ MAX_CMDLINE ];
} WCombofield_t;

static void             combofield_build(WCombofield_t *cf);
static int              combofield_autocomplete(Editfield_t *ef, WIDGET_t *w, void *arg);
static void             combofield_listbox(WCombofield_t *cf, const int state);
static int              combofield_callback(WCombofield_t *cf, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);


WIDGET_t *
combofield_new(void)
{
    WCombofield_t *cf;

    if (NULL == (cf = (WCombofield_t *)tty_new(sizeof(WCombofield_t), (WIDGETCB_t)combofield_callback))) {
        return NULL;
    }

    cf->cf_open         = FALSE;
    cf->cf_editable     = FALSE;
    cf->cf_relaxmode    = CB_RELAXMODE_NONE;
    cf->cf_autocompletemode = CB_AUTOMODE_NONE;
    cf->cf_popupmode    = CB_POPUPMODE_AUTO;
    cf->cf_popupstate   = CB_POPUPSTATE_NONE;

    lb_init(&cf->cf_listimpl, LB_FISPOPUP|LB_FHASSTRINGS);
    ef_init(&cf->cf_editimpl, EF_FISCOMBO|EF_FISAUTOCOMPLETE);

    return (WIDGET_t *)cf;
}


static void
combofield_open(WCombofield_t *cf)
{
    WIDGET_t *w = (WIDGET_t *)cf;
    Editfield_t *ef = &cf->cf_editimpl;
    Listbox_t *lb = &cf->cf_listimpl;
    int32_t idx = -1;
                                                /* prime active and cursor */
    idx = lb_active(lb, lb_item_match(lb, NULL, ef_get(ef), &idx) ? idx : -1);
    lb_cursor(lb, w, idx);

    ef_save(ef);                                /* open and save edit buffer */
    if (! cf->cf_editable) {
        ef->ef_marked = 0;
    }
    cf->cf_open = TRUE;
    ef_open(ef, w);
    lb_open(lb, w);
}


static int
combofield_select(WCombofield_t *cf, int idx)
{
    Editfield_t *ef = &cf->cf_editimpl;
    Listbox_t *lb = &cf->cf_listimpl;
    const ListboxItem_t *n;

    if (NULL == (n = lb_item_get(lb, idx))) {
        lb_active(lb, -1);
        return FALSE;
    }
    lb_active(lb, idx);
    ef_set(ef, n->li_display);
    return TRUE;
}


static void
combofield_build(WCombofield_t *cf)
{
    const WIDGET_t *w = (WIDGET_t *)cf;
    Listbox_t *lb = &cf->cf_listimpl;

    const int32_t vtcols  = ttcols();           /* 33% or 50%, alignment of 5 */
    const int32_t maxcols = ((vtcols / 5) * 5) / 2;
    const int32_t vtrows  = ttrows();
    const int32_t maxrows = ((vtrows / 5) * 5) / (vtrows >= 40 ? 3 : 2);

    const int32_t mincols = w->w_cols - 1;      /* parent size, minus scroll-bar */
    const int32_t reqcols = lb_item_maxlen(lb) + 2;
    const int32_t reqrows = lb_item_count(lb);

    int32_t cols, rows, columns = lb->lb_columns;
    int32_t pgno = 0;

    assert(CB_POPUPSTATE_NONE == cf->cf_popupstate);

    if (reqrows <= maxrows) {                   /* simple case */
        columns = 1;
        rows    = reqrows;
        cols    = reqcols;
        if (cols < mincols) cols = mincols;
        if (cols > maxcols) cols = maxcols;

    } else {
        if (0 == (LB_FPAGEMODE & lb->lb_flags)) {
            columns = 1;                        /* implied unless paging */
        }

        if (columns < 0) {
            /*
             *  dynamic column, size panel
             *      expand column count until page fill-factor is below 50%
             *      or upper width is exceeded.
             */
            columns = 0;
            rows = (reqrows >= maxrows ? maxrows : reqrows);
            do {
                cols = ++columns * reqcols;
                if (cols < mincols) cols = mincols;
                if (cols > maxcols) cols = maxcols;
                pgno = (reqrows / (rows * columns)) + 1;

            } while (pgno > 1 &&                    /* size panel */
                        (columns * (maxrows / 2)) <= reqrows && (cols + reqcols) <= maxcols);

        } else {
            /*
            *  fixed columns
            */
            columns = (columns > 0 ? columns : 1);
            cols = columns * reqcols;
            if (cols < mincols) cols = mincols;
            if (cols > maxcols) cols = maxcols;
            rows = (reqrows >= maxrows ? maxrows : reqrows);
            pgno = (reqrows / (rows * columns)) + ((reqrows % (rows * columns)) ? 1 : 0);
        }
        rows = (reqrows / (pgno * columns))+1;  /* normalise page-size */
    }

    lb->lb_flags    &= ~LB_FWIDGETSIZE;
    lb->lb_columns  = columns;
    lb->lb_width    = cols / columns;
    lb->lb_rows     = rows;
    lb->lb_cols     = cols;
                                                /* build window/buffer */
    dialog_tty_popup_create(w->w_root, w, rows, cols,
            (CB_POPUPMODE_AUTO == cf->cf_popupmode && !cf->cf_editable) ?
                (pgno > 1 ? "<PgUp/Dn>" : "") : (pgno > 1 ? "<Pg,Ctrl-L>" : "<Ctrl-L>"));

    cf->cf_popupstate = CB_POPUPSTATE_VISIBLE;
}


static int
combofield_autocomplete(Editfield_t *ef, WIDGET_t *w, void *arg)
{
    WCombofield_t *cf = (WCombofield_t *)arg;
    Listbox_t *lb = &cf->cf_listimpl;
    const char *buffer = ef_get(ef);
    char *complete = ef->ef_complete;
    const ListboxItem_t *n = NULL;
    int32_t idx = 0;

    if (! cf->cf_editable) {
        return FALSE;
    }

    if (0 == *buffer) {                         /* empty buffer */
        n = lb_item_find(lb, "", 0, &idx);
                                                /* element index */
    } else if ((LB_FINDEXMODE & lb->lb_flags) && isdigit(*buffer)) {
        char t_buffer[32];
        int value;

        if ((value = strtol(buffer, NULL, 10)) < 1 || value > (int)lb_item_count(lb) ||
                NULL == (n = lb_item_get(lb, idx = (uint32_t)(value - 1)))) {
            return FALSE;
        }

        lb_filter(lb, 0, NULL);
        sxprintf(t_buffer, sizeof(t_buffer), "%d", value);
        ef_set(ef, t_buffer);

        strxcpy(complete + 1, n->li_display, sizeof(ef->ef_complete) - 1);
        complete[0] = '-';

    } else {                                    /* single character short-cut */
        if ((LB_FHASSHORTCUT & lb->lb_flags) && 0 == buffer[1] &&
                (NULL != (n = lb_item_shortcut(lb, (uint16_t) *buffer, &idx, FALSE)))) {

            strxcpy(complete + 1, n->li_display + strlen(buffer), sizeof(ef->ef_complete));
            complete[0] = '-';

        } else {                                /* element value */
            if (NULL == (n = lb_item_match(lb, NULL, buffer, &idx))) {
                if (CB_RELAXMODE_NONE == cf->cf_relaxmode) {
                    return FALSE;
                }
            } else {
                strxcpy(complete, n->li_display + strlen(buffer), sizeof(ef->ef_complete));
            }
        }

        if (NULL != n) {
            if (CB_AUTOMODE_SUGGEST & cf->cf_autocompletemode) {
                ListboxItem_t *ncursor = (ListboxItem_t *)n;
                int32_t t_idx = idx;

                lb_filter(lb, 1, NULL);         /* filter elements */
                do  {
                    lb_filter(lb, 0, ncursor);
                } while (NULL != (ncursor = (ListboxItem_t *)lb_item_match(lb, ncursor, buffer, &t_idx)));

            } else {
                lb_filter(lb, 0, NULL);         /* unfilter elements */
            }
        }
    }

    if (NULL == n) {                            /* no-match, clear suggestions */
        lb_cursor(lb, w, idx);
        lb_filter(lb, 0, NULL);
        complete[0] = 0;
        idx = -1;
    }

    ef->ef_append =                             /* append auto-complete buffer */
        ((complete[0] && (CB_AUTOMODE_APPEND & cf->cf_autocompletemode)) ? TRUE : FALSE);

    if (lb_active(lb, idx) >= 0) {
        lb_cursor(lb, w, idx);                  /* focus active element */
    }

    if ((CB_POPUPMODE_AUTO == cf->cf_popupmode) &&
            (CB_AUTOMODE_SUGGEST & cf->cf_autocompletemode)) {
        if (NULL == n) {                        /* sugguestion listbox */
            if (*buffer) {
                combofield_listbox(cf, CB_POPUPSTATE_NONE);
            }
        } else {
            combofield_listbox(cf, CB_POPUPSTATE_VISIBLE);
        }
    }

    return TRUE;
}


static void
combofield_listbox(WCombofield_t *cf, const int state)
{
    WIDGET_t *w = (WIDGET_t *)cf;
    const int current_state = cf->cf_popupstate;

    switch (state) {
//  case CB_POPUPSTATE_HIDDEN:
//      switch (current_state) {
//      case CB_POPUPSTATE_FOCUS:
//      case CB_POPUPSTATE_VISIBLE:
//          if (dialog_tty_popup_focus(w->w_root, -1)) {
//              cf->cf_popupstate = CB_POPUPSTATE_HIDDEN;
//          }
//          break;
//      }
//      break;

    case CB_POPUPSTATE_VISIBLE:
        switch (current_state) {
//      case CB_POPUPSTATE_HIDDEN:
        case CB_POPUPSTATE_FOCUS:
            if (dialog_tty_popup_focus(w->w_root, FALSE)) {
                cf->cf_popupstate = CB_POPUPSTATE_VISIBLE;
            }
            break;
        case CB_POPUPSTATE_NONE:
            combofield_build(cf);
            break;
        }
        break;

    case CB_POPUPSTATE_FOCUS:
        switch (current_state) {
        case CB_POPUPSTATE_NONE:
            combofield_build(cf);
            /*FALLTHRU*/
//      case CB_POPUPSTATE_HIDDEN:
        case CB_POPUPSTATE_VISIBLE:
            if (dialog_tty_popup_focus(w->w_root, TRUE)) {
                cf->cf_popupstate = CB_POPUPSTATE_FOCUS;
            }
            break;
        }
        break;

    case CB_POPUPSTATE_NONE:
        switch (current_state) {
        case CB_POPUPSTATE_FOCUS:
            dialog_tty_popup_focus(w->w_root, FALSE);
            /*FALLTHRU*/
        case CB_POPUPSTATE_VISIBLE:
            dialog_tty_popup_close(w->w_root);
            cf->cf_popupstate = CB_POPUPSTATE_NONE;
            break;
        }
        break;
    }
}


static const char *
combofield_value_get(WCombofield_t *cf, int iscomplete)
{
    Editfield_t *ef = &cf->cf_editimpl;
    Listbox_t *lb = &cf->cf_listimpl;
    const char *buffer = ef_get(ef);
    char *outbuffer = cf->cf_buffer;
    const ListboxItem_t *n;

    if (! iscomplete) {
        if (! cf->cf_open) {                    /* current value */
            strxcpy(outbuffer, buffer, sizeof(cf->cf_buffer));
            return outbuffer;
        }
    }

    if (lb->lb_active < 0 ||                    /* unknown */
            (NULL == (n = lb_item_get(lb, lb->lb_active)))) {
        switch (cf->cf_relaxmode) {
        case CB_RELAXMODE_ANY:
            strxcpy(outbuffer, buffer, sizeof(cf->cf_buffer));
            break;

        case CB_RELAXMODE_NONBLANK: {
                int len = (int)strlen(buffer);
                const char *trim;

                if ((NULL != (trim = str_trim(buffer, &len)) && len)) {
                    assert(len < (int)sizeof(cf->cf_buffer));
                    memmove(outbuffer, trim, len);
                    outbuffer[len] = 0;
                    break;                      /* non-blank */
                }
            }
            /*FALLTHRU*/

        default:
#if defined(EWIDECHAR)
            Wcstoutf8(ef->ef_restore, outbuffer, sizeof(cf->cf_buffer));
#else
            strxcpy(outbuffer, ef->ef_restore, sizeof(cf->cf_buffer));
#endif
            if (iscomplete) {
                ef_restore(ef);
                iscomplete = FALSE;
            }
            break;
        }
    } else {                                    /* assign result */
        strxcpy(outbuffer, n->li_display, sizeof(cf->cf_buffer));
    }

    if (iscomplete) {
        ef_set(ef, outbuffer);
    }
    return outbuffer;
}


static void
combofield_complete(WCombofield_t *cf)
{
    Editfield_t *ef = &cf->cf_editimpl;
    Listbox_t *lb = &cf->cf_listimpl;

    cf->cf_open = FALSE;
    combofield_value_get(cf, TRUE);
    lb_complete(lb);
    ef_complete(ef);
}


static int
combofield_callback(WCombofield_t *cf, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    WIDGET_t *w = (WIDGET_t *)cf;
    Editfield_t *ef = &cf->cf_editimpl;
    Listbox_t *lb = &cf->cf_listimpl;

    switch (msg) {
    case WIDGET_INIT:           /* initialise */
        if (w->w_reqrows <= 0)  w->w_reqrows = 1;
        if (w->w_reqcols <= 4)  w->w_reqcols = 4;
        w->w_flags |= WIDGET_FTABSTOP;
        return TRUE;

    case WIDGET_READY:          /* dialog session ready */
        return TRUE;

    case WIDGET_SET: {          /* set attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            w->w_uflags |= WTTY_FDIRTY;
            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:
            case DLGA_EDEDITABLE:
            case DLGA_EDMAXLENGTH:
            case DLGA_EDVISIBLITY:
            case DLGA_EDINVISIBLECHAR:
            case DLGA_EDPOSITION:
            case DLGA_EDPLACEHOLDER:
            case DLGA_EDVALIDATE:
                return ef_value_set(ef, p1, data);

            case DLGA_LBELEMENTS:
                lb_item_elements(lb, LB_POSNORM, data);
                break;

            case DLGA_LBINSERT:
            case DLGA_LBREMOVE:
                break;

            case DLGA_LBCLEAR:
                return lb_item_zap(lb);

            case DLGA_LBACTIVE:
                if (lb_value_set(lb, p1, data)) {
                    (void) combofield_select(cf, lb->lb_active);
                    return TRUE;
                }
                return FALSE;

            case DLGA_LBCOUNT:
            case DLGA_LBSORTMODE:
            case DLGA_LBHASSTRINGS:
            case DLGA_LBICASESTRINGS:
            case DLGA_LBCURSOR:
            case DLGA_LBDISPLAYTEXT:
            case DLGA_LBDISPLAYTEXTLEN:
            case DLGA_LBTEXT:
            case DLGA_LBTEXTLEN:
            case DLGA_LBPAGEMODE:
            case DLGA_LBINDEXMODE:
            case DLGA_LBROWS:
            case DLGA_LBCOLS:
            case DLGA_LBCOLUMNS:
            case DLGA_LBWIDTH:
                return lb_value_set(lb, p1, data);

            case DLGA_CBEDITABLE:
                cf->cf_editable = (int16_t)widgetdata_uint32(data);
                return TRUE;
            case DLGA_CBRELAXMODE:
                cf->cf_relaxmode = (int16_t)widgetdata_uint32(data);
                return TRUE;
            case DLGA_CBAUTOCOMPLETEMODE:
                cf->cf_autocompletemode = (int16_t)widgetdata_uint32(data);
                return TRUE;
            case DLGA_CBPOPUPMODE:
                cf->cf_popupmode = (int16_t)widgetdata_uint32(data);
                return TRUE;
            case DLGA_CBPOPUPSTATE:
                return FALSE;

            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_GET: {          /* get attribute */
            WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            switch (DIALOGARGLO(p1)) {
            case DLGA_VALUE:
                data->d_u.svalue = combofield_value_get(cf, FALSE);
                data->d_type = D_STR;
                return TRUE;

            case DLGA_EDEDITABLE:
            case DLGA_EDMAXLENGTH:
            case DLGA_EDVISIBLITY:
            case DLGA_EDINVISIBLECHAR:
            case DLGA_EDPOSITION:
            case DLGA_EDPLACEHOLDER:
            case DLGA_EDVALIDATE:
                return ef_value_get(ef, p1, data);

            case DLGA_LBELEMENTS:
                return FALSE;

            case DLGA_LBINSERT:
            case DLGA_LBREMOVE:
            case DLGA_LBCLEAR:
                return FALSE;

            case DLGA_LBCOUNT:
            case DLGA_LBSORTMODE:
            case DLGA_LBHASSTRINGS:
            case DLGA_LBICASESTRINGS:
            case DLGA_LBCURSOR:
            case DLGA_LBACTIVE:
            case DLGA_LBDISPLAYTEXT:
            case DLGA_LBDISPLAYTEXTLEN:
            case DLGA_LBTEXT:
            case DLGA_LBTEXTLEN:
            case DLGA_LBPAGEMODE:
            case DLGA_LBINDEXMODE:
            case DLGA_LBROWS:
            case DLGA_LBCOLS:
            case DLGA_LBCOLUMNS:
            case DLGA_LBWIDTH:
                return lb_value_get(lb, p1, data);

            case DLGA_CBEDITABLE:
                data->d_u.ivalue = cf->cf_editable;
                data->d_type = D_INT;
                break;
            case DLGA_CBRELAXMODE:
                data->d_u.ivalue = cf->cf_relaxmode;
                data->d_type = D_INT;
                return TRUE;
            case DLGA_CBAUTOCOMPLETEMODE:
                data->d_u.ivalue = cf->cf_autocompletemode;
                data->d_type = D_INT;
                break;
            case DLGA_CBPOPUPMODE:
                data->d_u.ivalue = cf->cf_popupmode;
                data->d_type = D_INT;
                break;
            case DLGA_CBPOPUPSTATE:
                data->d_u.ivalue = cf->cf_popupstate;
                data->d_type = D_INT;
                break;

            default:
                return tty_default(w, msg, p1, p2);
            }
        }
        break;

    case WIDGET_SETFOCUS:       /* cursor/focus request */
        if ((WIDGET_FGREYED|WIDGET_FHIDDEN) & w->w_flags) {
            return FALSE;
        }

        if (p1) {
            int state = CB_POPUPSTATE_NONE;

            if ((CB_POPUPMODE_AUTO == cf->cf_popupmode && !cf->cf_editable) ||
                    cf->cf_popupmode >= CB_POPUPMODE_OPEN) {
                state = (CB_POPUPMODE_FOCUS == cf->cf_popupmode ? CB_POPUPSTATE_FOCUS : CB_POPUPSTATE_VISIBLE);
            }
            combofield_open(cf);
            combofield_listbox(cf, state);
        } else {
            ef_change(ef, w);
            combofield_complete(cf);
            combofield_listbox(cf, CB_POPUPSTATE_NONE);
        }
        return TRUE;

    case WIDGET_KEYDOWN:        /* keyboard event */
        if (cf->cf_popupmode >= CB_POPUPMODE_AUTO) {
            switch (p1) {
            case KEY_WDOWN2:        /* <Ctrl-Down>  - focus list-box */
                combofield_listbox(cf, CB_POPUPSTATE_FOCUS);
                return TRUE;

            case KEY_WUP2:          /* <Ctrl-Up>    - unfocus list-box */
                combofield_listbox(cf, CB_POPUPSTATE_VISIBLE);
                break;

            case CTRL_TAB:          /* <Ctrl-Tab>   - switch focus */
            case CTRL_L:
            case ALT_L:
                combofield_listbox(cf, CB_POPUPSTATE_FOCUS == cf->cf_popupstate ?
                        CB_POPUPSTATE_VISIBLE : CB_POPUPSTATE_FOCUS);
                return TRUE;

            case KEY_ESC:           /* <ESC>        - escape */
                combofield_listbox(cf, CB_POPUPSTATE_VISIBLE);
                break;

            case KEY_PAGEUP:        /* <PgUp/PgDn>  - listbox page */
            case KEY_PAGEDOWN:
                return lb_key(lb, w, p1);

            case KEY_DOWN:          /* <Up/Down>    - listbox control */
            case KEY_UP:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_HOME:          /* <Home>/<End> */
            case KEY_END:
            case WHEEL_DOWN:        /* <Mouse> */
            case WHEEL_UP:
                if (CB_POPUPSTATE_FOCUS == cf->cf_popupstate) {
                    return lb_key(lb, w, p1);
                }
                break;

            case KEY_ENTER:         /* <Enter>      - listbox selection */
                if (CB_POPUPSTATE_FOCUS == cf->cf_popupstate) {
                    if (combofield_select(cf, lb->lb_cursor)) {
                        combofield_listbox(cf, CB_POPUPSTATE_VISIBLE);
                    }
                    return TRUE;
                }
                break;

            default:
                if (CB_POPUPSTATE_FOCUS == cf->cf_popupstate) {
                    return lb_key(lb, w, p1);
                }
                break;
            }
        }

        if (cf->cf_editable) {
            return ef_key(ef, w, p1, combofield_autocomplete, (void *)cf);
        }
        return FALSE;

#if defined(TODO)
    case WIDGET_MOUSE:          /* mouse event */
        if ((CB_POPUPSTATE_FOCUS == cf->cf_popupstate || CB_POPUPSTATE_VISIBLE == cf->cf_popupstate) &&
                    cf->cf_lbwidget == w) {
            if (BUTTON1_DOWN == p2) {
                const int y = GetYParam(p1) - w->w_border;

                if (combofield_select(cf, lb->lb_top + y)) {
                    combofield_listbox(cf, CB_POPUPSTATE_VISIBLE);
                }
            }
            return TRUE;
        }
        break;
#endif

    case WIDGET_PAINT:          /* widget display */
        if (0 == (w->w_flags & WIDGET_FHIDDEN)) {
            if (CB_POPUPSTATE_VISIBLE == cf->cf_popupstate || CB_POPUPSTATE_FOCUS == cf->cf_popupstate) {
                lb_paint(lb, w, p1);
            }
            ef_paint(ef, w);
        }
        return TRUE;

    case WIDGET_CARET:          /* cursor selection */
        if (CB_POPUPSTATE_FOCUS == cf->cf_popupstate) {
            dialog_tty_popup_select(w->w_root, TRUE);
            lb_caret(lb, w);
        } else {
            ef_caret(ef, w);
        }
        return TRUE;

    case WIDGET_DESTROY:        /* destructor */
        ef_destroy(ef);
        lb_destroy(lb);
        return TRUE;

    default:
        return tty_default(w, msg, p1, p2);
    }
    return FALSE;
}

/*end*/
