#include <edidentifier.h>
__CIDENT_RCSID(gr_dialog_c,"$Id: dialog.c,v 1.32 2024/09/08 16:29:24 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: dialog.c,v 1.32 2024/09/08 16:29:24 cvsuser Exp $
 * Dialog manager.
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
#include <edhandles.h>

#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"
#include "buffer.h"                             /* buf_find */
#include "builtin.h"                            /* execute_str */
#include "debug.h"
#include "echo.h"
#include "eval.h"
#include "lisp.h"                               /* lst_check/atom_...() */
#include "macros.h"                             /* loadup_macro */
#include "main.h"
#include "word.h"

#include "dialog.h"
#include "dialog_tty.h"
#include "widgets.h"

typedef WIDGET_t * (* WIDGETNEW_t)(void);

static DIALOG_t *       dlg_new(void);
static void             dlg_destroy(DIALOG_t *d);
static void             dlg_queue(DIALOG_t *d, WIDGET_t *w);
static void             dlg_inherit(WIDGET_t *parent, WIDGET_t *w);

static int              callback(const char *label, const char *callback,
                            WIDGET_t *w, int event, accint_t val1, accint_t val2, int *iret);

typedef struct {                                /* Widget definition */
    uint16_t            w_class;
    const char *        w_desc;
    WIDGETNEW_t         w_new;
} WidgetDef;

typedef struct {                                /* Attribute definition */
    uint16_t            a_ident;
    const char *        a_desc;
    int32_t             a_type;
#define A_NONE                  0x00
#define A_STR                   0x01
#define A_INT                   0x02
#define A_BOOL                  A_INT
#define A_FLOAT                 0x04
#define A_LIST                  0x08
#define A_NULL                  0x10
} AttributeDef;

static WidgetDef frame_widgets[] = {            /* Frame widget classes */
        /*
         *  containers
         */
        { DLGC_TAB,                     "TAB",                  tab_new },
    /*- { DLGC_GRID,                    "GRID",                 grid_new },         -*/
        { DLGC_GROUP,                   "GROUP",                group_new },
        { DLGC_CONTAINER,               "CONTAINER",            container_new },

        /*
         *  elements
         */
        { DLGC_SPACER,                  "SPACER",               spacer_new },
        { DLGC_SEPARATOR_HORIZONTAL,    "SEPARATOR_HORIZONTAL", separatorh_new },
        { DLGC_SEPARATOR_VERTICAL,      "SEPARATOR_VERTICAL",   separatorv_new },
        { DLGC_PUSH_BUTTON,             "PUSH_BUTTON",          pushbutton_new },
        { DLGC_RADIO_BUTTON,            "RADIO_BUTTON",         radiobutton_new },
        { DLGC_CHECK_BOX,               "CHECK_BOX",            checkbox_new },
        { DLGC_LABEL,                   "LABEL",                label_new },
        { DLGC_LIST_BOX,                "LIST_BOX",             listbox_new },
        { DLGC_EDIT_FIELD,              "EDIT_FIELD",           editfield_new },
        { DLGC_NUMERIC_FIELD,           "NUMERIC_FIELD",        numericfield_new },
        { DLGC_COMBO_FIELD,             "COMBO_FIELD",          combofield_new },
    /*- { DLGC_TREE,                    "TREE",                 tree_new },         -*/
    /*- { DLGC_TABLE,                   "TABLE",                table_new },        -*/
        { DLGC_GAUGE,                   "GAUGE",                gauge_new },
    /*- { DLGC_SLIDER,                  "SLIDER",               slider_new },       -*/
    /*- { DLGC_VSCROLLBAR,              "VSCROLLBAR",           vscrollbar_new },   -*/
    /*- { DLGC_HSCROLLBAR,              "HSCROLLBAR",           hscrollbar_new },   -*/
    /*- { DLGC_TOGGLE,                  "TOGGLE",               toggle_new },       -*/
        { 0, NULL, NULL }
        };

static WidgetDef menu_widgets[] = {             /* Menu context, widget classes */
        /*
         *  containers
         */
        { DLGC_MENU,                    "MENU",                 menu_new },

        /*
         *  elements
         */
        { DLGC_MENU_ITEM,               "MENU_ITEM",            menu_item_new },
        { DLGC_MENU_SEPARATOR,          "MENU_SEPARATOR",       menu_separator_new },
        { 0, NULL, 0 }
        };

static AttributeDef attrs[] = {
        /*
         *  Attribute definitions
         */
        { DLGA_TITLE,                   "TITLE",                A_STR },
        { DLGA_NAME,                    "NAME",                 A_STR },
        { DLGA_IDENT,                   "IDENT",                A_INT },
        { DLGA_CALLBACK,                "CALLBACK",             A_STR },
        { DLGA_HELP,                    "HELP",                 A_STR },
        { DLGA_TOOLTIP,                 "TOOLTIP",              A_STR },
        { DLGA_X,                       "X",                    A_INT|A_STR },
        { DLGA_Y,                       "Y",                    A_INT|A_STR },
        { DLGA_COLS,                    "COLS",                 A_INT|A_STR },
        { DLGA_ROWS,                    "ROWS",                 A_INT|A_STR },

        { DLGA_ATTACH_TOP,              "ATTACH_TOP",           A_NONE },
        { DLGA_ATTACH_BOTTOM,           "ATTACH_BOTTOM",        A_NONE },
        { DLGA_ATTACH_LEFT,             "ATTACH_LEFT",          A_NONE },
        { DLGA_ATTACH_RIGHT,            "ATTACH_RIGHT",         A_NONE },

        { DLGA_ALIGN_N,                 "ALIGN_N",              A_NONE },
        { DLGA_ALIGN_NE,                "ALIGN_NE",             A_NONE },
        { DLGA_ALIGN_E,                 "ALIGN_E",              A_NONE },
        { DLGA_ALIGN_SE,                "ALIGN_SE",             A_NONE },
        { DLGA_ALIGN_S,                 "ALIGN_S",              A_NONE },
        { DLGA_ALIGN_SW,                "ALIGN_SW",             A_NONE },
        { DLGA_ALIGN_W,                 "ALIGN_W",              A_NONE },
        { DLGA_ALIGN_NW,                "ALIGN_NW",             A_NONE },
        { DLGA_ALIGN_CENTER,            "ALIGN_CENTER",         A_NONE },

        { DLGA_ALLOW_EXPAND,            "ALLOW_EXPAND",         A_NONE },
        { DLGA_ALLOW_FILLX,             "ALLOW_FILLX",          A_NONE },
        { DLGA_ALLOW_FILLY,             "ALLOW_FILLY",          A_NONE },
        { DLGA_PROPAGATE,               "DLGA_PROPAGATE",       A_BOOL },

        { DLGA_CANCEL_BUTTON,           "CANCEL_BUTTON",        A_NONE },
        { DLGA_DEFAULT_BUTTON,          "DEFAULT_BUTTON",       A_NONE },
        { DLGA_DEFAULT_FOCUS,           "DEFAULT_FOCUS",        A_NONE },
        { DLGA_VALUE,                   "VALUE",                A_INT|A_STR|A_LIST|A_NULL },
        { DLGA_LABEL,                   "LABEL",                A_STR  },
        { DLGA_TEXT_ONLY,               "TEXT_ONLY",            A_NONE },
        { DLGA_GUI_ONLY,                "GUI_ONLY",             A_NONE },
        { DLGA_ACCELERATOR,             "ACCELERATOR",          A_STR  },
        { DLGA_HOTKEY,                  "HOTKEY",               A_INT  },
        { DLGA_GREYED,                  "GREYED",               A_NONE },
        { DLGA_ACTIVE,                  "ACTIVE",               A_NONE },
        { DLGA_SENSITIVE,               "SENSITIVE",            A_INT  },
        { DLGA_PADX,                    "PADX",                 A_INT  },
        { DLGA_PADY,                    "PADY",                 A_INT  },

        { DLGA_ORIENTATION,             "ORIENTATION",          A_INT  },
        { DLGA_HIDDEN,                  "HIDDEN",               A_NONE },
        { DLGA_VISIBLE,                 "VISIBLE",              A_NONE },
        { DLGA_KEYDOWN,                 "KEYDOWN",              A_BOOL },
        { DLGA_TABSTOP,                 "TABSTOP",              A_BOOL },
        { DLGA_AUTOMOVE,                "AUTOMOVE",             A_BOOL },

    /*  { DLCA_USERDATA,                "USERDATA",             A_INT  },  */

        { DLGA_EDEDITABLE,              "EDEDITABLE",           A_BOOL },
        { DLGA_EDMAXLENGTH,             "EDMAXLENGTH",          A_INT  },
        { DLGA_EDVISIBLITY,             "EDVISIBLITY",          A_BOOL },
        { DLGA_EDPOSITION,              "EDPOSITION",           A_INT  },
        { DLGA_EDPLACEHOLDER,           "EDPLACEHOLDER",        A_STR  },

    /*  { DLGA_NUMDIGITS,               "NUMDIGITS",            A_INT  },  */
    /*  { DLGA_NUMMIN,                  "NUMMIN",               A_INT|A_FLOAT },  */
    /*  { DLGA_NUMMAX,                  "NUMMAX",               A_INT|A_FLOAT },  */
    /*  { DLGA_NUMINCREMENT,            "NUMINCREMENT",         A_INT|A_FLOAT },  */
    /*  { DLGA_NUMWRAP,                 "NUMWRAP",              A_BOOL },  */
    /*  { DLGA_NUMSNAP,                 "NUMSNAP",              A_BOOL },  */

        { DLGA_LBCOUNT,                 "LBCOUNT",              A_INT  },
        { DLGA_LBELEMENTS,              "LBELEMENTS",           A_INT|A_STR|A_LIST|A_NULL },
        { DLGA_LBSORTMODE,              "LBSORTMODE",           A_INT  },
        { DLGA_LBHASSTRINGS,            "LBHASSTRINGS",         A_INT  },
        { DLGA_LBICASESTRINGS,          "LBICASESTRINGS",       A_INT  },
        { DLGA_LBDUPLICATES,            "LBDUPLICATES",         A_INT  },
    /*  { DLGA_LBINSERT,                "LBINSERT",             A_INT|A_STR|A_LIST },  */
    /*  { DLGA_LBREMOVE,                "LBREMOVE",             A_INT|A_STR|A_LIST },  */
        { DLGA_LBCLEAR,                 "LBCLEAR",              A_NONE },

        { DLGA_LBCURSOR,                "LBCURSOR",             A_INT  },
        { DLGA_LBACTIVE,                "LBACTIVE",             A_INT  },
        { DLGA_LBDISPLAYTEXT,           "LBDISPLAYTEXT",        A_INT  },
        { DLGA_LBDISPLAYTEXTLEN,        "LBDISPLAYTEXTLEN",     A_INT  },
        { DLGA_LBTEXT,                  "LBTEXT",               A_INT  },
        { DLGA_LBTEXTLEN,               "LBTEXTLEN",            A_INT  },
        { DLGA_LBPAGEMODE,              "LBPAGEMODE",           A_BOOL },
        { DLGA_LBINDEXMODE,             "LBINDEXMODE",          A_BOOL },

        { DLGA_CBEDITABLE,              "CBEDITABLE",           A_BOOL },
        { DLGA_CBRELAXMODE,             "CBRELAXMODE",          A_INT  },
        { DLGA_CBAUTOCOMPLETEMODE,      "CBAUTOCOMPLETEMODE",   A_INT  },
        { DLGA_CBPOPUPMODE,             "CBPOPUPMODE",          A_INT  },
        { DLGA_CBPOPUPSTATE,            "CBPOPUPSTATE",         A_INT  },

        { DLGA_GAUGEMIN,                "GAUGEMIN",             A_INT|A_STR },
        { DLGA_GAUGEMAX,                "GAUGEMAX",             A_INT|A_STR },

        { 0, NULL, 0 }
        };

static uint16_t         dialogseq;              /* Identifier sequence */
static TAILQ_HEAD(_dialoglist, _dialog)         /* Dialog queue */
                        dialogq;

static int              curdlg = 0;             /* Current (running) dialog */


/*  Function:           dialog_shutdown
 *      dialog_shutdown release any and all dialog related resources.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
dialog_shutdown(void)
{
    DIALOG_t *d;

    if (dialogseq) {
        while (NULL != (d = TAILQ_FIRST(&dialogq))) {
            dlg_destroy(d);
        }
    }
}


/*  Function:           do_dialog_create
 *      dialog_create() primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: dialog_create - Build a dialog resource.

        int
        dialog_create(list decl)

    Macro Description:
        The 'dialog_create()' primitive creates a dialog resource
        from the specified definition 'decl'.

        The buffer information dialog is just one example.

                        (See images/bufinfo.png)

        The declaration list should the contain the dialog elements
        plus their associated value, if any. Elements classifications
        are 'Containers', 'Widgets' and 'Attributes'.

     *Containers*

(start table,table=nd)
        [Atom                       [Description                            ]
      ! DLGC_GROUP                  Variable sized visual and logical
                                    group container, with an optional
                                    frame and title.

      ! DLGC_CONTAINER              General widget group, used to logical
                                    group a collection of widgets.

      ! DLGC_TAB                    Fixed size visual and logical
                                    container, with an optional title.

      ! DLGC_MENU                   Specialised menu container.
                                    Note: work-in-progress
(end table)

     *Widgets*

        Non-menu containers support the following widgets.

(start table,table=nd)
        [Atom                       [Description                            ]
      ! DLGC_CHECK_BOX              Check box
      ! DLGC_COMBO_FIELD            Combo field
      ! DLGC_EDIT_FIELD             Edit field
      ! DLGC_LABEL                  Text label
      ! DLGC_LIST_BOX               List box
      ! DLGC_NUMERIC_FIELD          Numeric field
      ! DLGC_PUSH_BUTTON            Push button
      ! DLGC_RADIO_BUTTON           Radio button
      ! DLGC_SEPARATOR_HORIZONTAL   Horizontal separator
      ! DLGC_SEPARATOR_VERTICAL     Vertical separator
      ! DLGC_SPACER                 Spacer
(end table)

        Menu containers support the following widgets only.

(start table,table=nd)
        [Atom                       [Description                            ]
      ! DLGC_MENU_ITEM              Item
      ! DLGC_MENU_SEPARATOR         Separator
(end table)

     *Attributes*

        Frame/non-menu dialog widgets support the following attributes.

(start table,table=nd)
        [Atom                   [Type       [Description                    ]
      ! DLGA_TITLE              String      Attribute title.
      ! DLGA_NAME               String      Attribute name.
      ! DLGA_IDENT              Int         Identifier.
      ! DLGA_CALLBACK           String      Macro callback.
      ! DLGA_HELP               String      Help topic.
      ! DLGA_TOOLTIP            String      Tooltip topic.

      ! DLGA_X                  Int         Horizontal position.
      ! DLGA_Y                  Int         Vertical position.
      ! DLGA_COLS               Int         Width in columns.
      ! DLGA_ROWS               Int         Height in rows.

      ! DLGA_ATTACH_TOP         n/a         Attachment within frame.
      ! DLGA_ATTACH_BOTTOM      n/a
      ! DLGA_ATTACH_LEFT        n/a
      ! DLGA_ATTACH_RIGHT       n/a

      ! DLGA_ALIGN_N            n/a         Alignment within frame.
      ! DLGA_ALIGN_NE           n/a
      ! DLGA_ALIGN_E            n/a
      ! DLGA_ALIGN_SE           n/a
      ! DLGA_ALIGN_S            n/a
      ! DLGA_ALIGN_SW           n/a
      ! DLGA_ALIGN_W            n/a
      ! DLGA_ALIGN_NW           n/a
      ! DLGA_ALIGN_CENTER       n/a

      ! DLGA_ALLOW_EXPAND       n/a         Auto sizing
      ! DLGA_ALLOW_FILLX        n/a
      ! DLGA_ALLOW_FILLY        n/a
      ! DLGA_PROPAGATE          Boolean

      ! DLGA_CANCEL_BUTTON      n/a
      ! DLGA_DEFAULT_BUTTON     n/a
      ! DLGA_DEFAULT_FOCUS      n/a
      ! DLGA_VALUE              Any
      ! DLGA_LABEL              String
      ! DLGA_TEXT_ONLY          n/a
      ! DLGA_GUI_ONLY           n/a
      ! DLGA_ACCELERATOR        String
      ! DLGA_HOTKEY             Integer
      ! DLGA_GREYED             n/a
      ! DLGA_ACTIVE             n/a
      ! DLGA_SENSITIVE          Integer
      ! DLGA_PADX               Integer
      ! DLGA_PADY               Integer

      ! DLGA_ORIENTATION        Integer
      ! DLGA_HIDDEN             n/a
      ! DLGA_VISIBLE            n/a
      ! DLGA_KEYDOWN            Boolean
      ! DLGA_TABSTOP            Boolean
      ! DLGA_AUTOMOVE           Boolean

      ! DLCA_USERDATA           Int

      ! DLGA_EDEDITABLE         Boolean
      ! DLGA_EDMAXLENGTH        Integer
      ! DLGA_EDVISIBLITY        Boolean
      ! DLGA_EDPOSITION         Integer
      ! DLGA_EDPLACEHOLDER      String

      ! DLGA_LBCOUNT            Integer
      ! DLGA_LBELEMENTS         Any
      ! DLGA_LBSORTMODE         Integer
      ! DLGA_LBHASSTRINGS       Integer
      ! DLGA_LBICASESTRINGS     Integer
      ! DLGA_LBDUPLICATES       Integer
      ! DLGA_LBCLEAR            n/a

      ! DLGA_LBCURSOR           Integer
      ! DLGA_LBACTIVE           Integer
      ! DLGA_LBDISPLAYTEXT      Integer
      ! DLGA_LBDISPLAYTEXTLEN   Integer
      ! DLGA_LBTEXT             Integer
      ! DLGA_LBTEXTLEN          Integer
      ! DLGA_LBPAGEMODE         Boolean
      ! DLGA_LBINDEXMODE        Boolean

      ! DLGA_CBEDITABLE         Boolean
      ! DLGA_CBRELAXMODE        Integer
      ! DLGA_CBAUTOCOMPLETEMODE Integer
      ! DLGA_CBPOPUPMODE        Integer
      ! DLGA_CBPOPUPSTATE       Integer

      ! DLGA_GAUGEMIN           Int|Str
      ! DLGA_GAUGEMAX           Int|Str
(end table)

        Menu widgets support the following attributes (work-in-progress).

(start table,table=nd)
        [Atom                   [Type       [Description                    ]
      ! DLGA_NAME               String      Attribute name.
      ! DLGA_IDENT              Int         Identifier.
      ! DLGA_CALLBACK           String      Macro callback.
      ! DLGA_HELP               String      Help topic.
      ! DLGA_TOOLTIP            String      Tooltip topic.

      ! DLGA_VALUE              Any
      ! DLGA_LABEL              String
      ! DLGA_ACCELERATOR        String
      ! DLGA_HOTKEY             Integer
      ! DLGA_GREYED             n/a
      ! DLGA_ACTIVE             n/a
(end table)

    Macro Parameters:
        decl - Dialog definition.

    Macro TODO:
        Allow the dialog definition to be json.

    Macro Returns:
        On success returns a unique dialog resource identifier otherwise
        -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        dialog_run, dialog_delete
 */
void
do_dialog_create(void)          /* int (list decl) */
{
    DIALOG_t *dialog;
    WIDGET_t *stack[WIDGET_MAXDEPTH] = {0},     /* container stack */
        *parent = NULL, *widget = NULL;
    const LIST *nextlp, *lp;

    unsigned dlgcontext = 0;
#define DLGCONTEXT_UNDEFINED 0
#define DLGCONTEXT_FRAME 1
#define DLGCONTEXT_MENU 2

    uint32_t groupid = 1, containerid = 10000;
    int length, padding;
    int32_t attribute;
    int depth = 0, object = 0;                  /* current depths */
    int i, idx = 0;

    if (NULL == (lp = get_list(1)) || NULL == (dialog = dlg_new())) {
        acc_assign_int(0);
        return;
    }

    widget = &dialog->d_widget;                 /* root of tree */
    widget->w_groupid = groupid;
    widget->w_containerid = containerid;
    parent = widget;

    while (idx >= 0 && (nextlp = atom_next(lp)) != lp) {
        /*
         *  Attribute value
         */
        accint_t idxval = 0;

        ++idx;                                  /* attribute index */
        if (! atom_xint(lp, &idxval)) {
            ewprintf("dialog_create: expected numeric at idx '%d'.", idx);
            idx = -idx;                         /* error */
            continue;
        }

        lp = nextlp;
        if ((attribute = (int32_t)idxval) < DLGC_MIN || attribute > DLGA_MAX) {
            ewprintf("dialog_create: attribute out-of-range at idx '%d' = 0x%x/%d.",
                idx, (unsigned int)idxval, (int)idxval); /*ACCINT*/
            idx = -idx;                         /* error */
            continue;
        }

        /*
         *  Firstly attempt match against WIDGET classes.
         *      if matched create and inherit the new widget.
         */
        if (attribute <= DLGC_MAX) {
            if (DLGC_END == attribute) {
                if (depth) {                    /* pop */
                    parent = stack[ --depth ];
                    switch (parent->w_class) {
                    case DLGC_GROUP:
                        ++groupid;
                        break;
                    case DLGC_CONTAINER:
                    case DLGC_TAB:
                    case DLGC_MENU:
                        ++containerid;
                        break;
                    }
                    widget = NULL;
                } else {
                    ewprintf("dialog_create: container nesting incorrect.");
                    idx = -idx;                 /* error */
                }
            } else {
                const WidgetDef *def = NULL;

                if (dlgcontext != DLGCONTEXT_MENU)
                    for (i = 0; frame_widgets[i].w_class; ++i)
                        if (attribute == frame_widgets[i].w_class) {
                            dlgcontext = DLGCONTEXT_FRAME;
                            def = frame_widgets + i;
                            break;
                        }
                if (dlgcontext != DLGCONTEXT_FRAME && NULL == def)
                    for (i = 0; menu_widgets[i].w_class; ++i)
                        if (attribute == menu_widgets[i].w_class) {
                            dlgcontext = DLGCONTEXT_MENU;
                            def = menu_widgets + i;
                            break;
                        }

                if (def) {
                    WIDGET_t *t_widget;

                    if ((WIDGET_t *)NULL != (t_widget = def->w_new())) {
                        widget = t_widget;

                        widget->w_desc = def->w_desc;
                        widget->w_class = (uint16_t)attribute;

                        dlg_queue(dialog, widget);
                        dlg_inherit(parent, widget);

                        object = 1;             /* containers, push and inc identifiers */
                        if (attribute < DLGC_END) {
                            if (depth >= WIDGET_MAXDEPTH) {
                                ewprintf("dialog_create: nesting depth >%d.", WIDGET_MAXDEPTH);
                                break;
                            }
                            if (0 == depth) {
                                ++containerid, ++groupid;
                            }
                            stack[ depth++ ] = parent;
                            parent = widget;
                            object = 0;
                        }

                        widget->w_groupid = groupid;
                        widget->w_containerid = containerid;
                        length = trace_log("\t[%3d/%3d] %*sDLGC_%s,",
                                    idx, depth, (depth + object) * 4, "", def->w_desc);
                        trace_log(" %*s// group:%u, container:%u\n",
                            (length > 59 ? 0 : 59 - length), "", (unsigned)groupid, (unsigned)containerid);

                        attribute = 0;          /* done */

                    } else {
                        ewprintf("dialog_create: unable to create %swidget '%d/0x%x'.",
                            (dlgcontext == DLGCONTEXT_FRAME ? "dialog " : (dlgcontext == DLGCONTEXT_MENU ? "menu " : "")), attribute, attribute);
                        idx = -idx;             /* error */
                    }

                } else {
                    ewprintf("dialog_create: invalid %swidget '%d/0x%x'.",
                        (dlgcontext == DLGCONTEXT_FRAME ? "dialog " : (dlgcontext == DLGCONTEXT_MENU ? "menu " : "")), attribute, attribute);
                    idx = -idx;                 /* error */
                }
            }

        /*
         *  Secondly against WIDGET attributes,
         *      if matched apply the attribute against the current widget.
         */
        } else if (NULL == widget && attribute >= DLGA_MIN) {
            ewprintf("dialog_create: attribute outside container '%d/0x%x'.", attribute, attribute);

        } else if (widget && attribute >= DLGA_MIN) {
            for (i = 0; attrs[i].a_ident; ++i)
                if (attribute == attrs[i].a_ident) {
                    const char *desc = attrs[i].a_desc;
                    const uint32_t type = attrs[i].a_type;
                    WIDGETDATA_t data;
                    accint_t ival;
                    accfloat_t fval;
                    const char *sval;
                    const LIST *lval;

                    data.d_type = D_ERROR;      /* error */

                    length = trace_log("\t[%3d/%3d] %*sDLGA_%s,", \
                                idx, depth, (depth + object + 1) * 4, "", desc);
                    padding = 1 + (length > 49 ? 0 : 49 - length);

                    if (A_NONE == type) {
                        data.d_type = D_NONE;
                        trace_log("\n");

                    } else if ((type & A_INT) && atom_xint(lp, &ival)) {
                        data.d_type = D_INT;
                        data.d_u.ivalue = ival;
                        trace_log("%*.s%" ACCINT_FMT "/0x%x\n", padding, "", ival, (unsigned)ival); /*ACCUINT*/

                    } else if ((type & A_FLOAT) && atom_xfloat(lp, &fval)) {
                        data.d_type = D_FLOAT;
                        data.d_u.fvalue = fval;
                        trace_log("%*.s%" ACCFLOAT_FMT "\n", padding, "", fval);

                    } else if ((type & A_STR) && NULL != (sval = atom_xstr(lp))) {
                        data.d_type = D_STR;
                        data.d_u.svalue = sval;
                        trace_log("%*.s\"%s\"\n", padding, "", data.d_u.svalue);

                    } else if ((type & A_LIST) && NULL != (lval = atom_xlist(lp))) {
                        data.d_type = D_LIST;
                        data.d_u.lvalue = lval;
                        trace_log("%*.s", padding, "");
                        trace_list(data.d_u.lvalue);

                    } else if ((type & A_NULL) && atom_xnull(lp)) {
                        data.d_type = D_NULL;
                        trace_log("%*.sNULL\n", padding, "");

                    } else {
                        trace_log("\n");
                    }

                    if (D_ERROR == data.d_type) {
                        ewprintf("dialog_create: attribute '%s', invalid argument.", attrs[i].a_desc);
                        idx = -idx;             /* error */

                    } else {                    /* call handler */
                        (widget->w_handler)(widget, WIDGET_SET, DIALOGARG32(attribute, 0), (WIDGETARG_t)&data);
                        if (D_NONE != data.d_type) {
                            lp = atom_next(lp);
                        }
                    }
                    attribute = 0;
                    break;
                }

            if (attribute) {                    /* ignore unknown */
                ewprintf("dialog_create: unknown attribute '%d/0x%x'.", attribute, attribute);
            }

        } else {
            ewprintf("dialog_create: unknown attribute '%d/0x%x'.", attribute, attribute);
        }
    }

    if (idx <= 0) {
        acc_assign_int(idx);                    /* bad attribute */
        if (dialog) {
            dlg_destroy(dialog);
        }
    } else {
        acc_assign_int(dialog->d_ident);        /* dialog identifier */
    }
}


/*  Function:           do_dialog_run
 *      dialog_run() primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: dialog_run - Execute a dialog resource

        int
        dialog_run(int dialog, [int x], [int y], [string args])

    Macro Description:
        The 'dialog_run()' primitive executes the dialog associated
        with the instance 'dialog', supplying the arguments 'args'.

    Macro Parameters:
        dialog - Dialog instance handle.
        x, y - Optional coordinates of the top left corner.
        args - Optional arguments; future extensions.

    Macro Returns:
        The 'dialog_run()' primitive returns the exit value of the
        terminating <dialog_exit>.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        dialog_create
 */
void
do_dialog_run(void)             /* int (int dialog, [string args]) */
{
    const int ident = get_xinteger(1, 0);
    DIALOG_t *d;
    int ret = -2;

    if (NULL != (d = dialog_find(ident))) {
        int ocurdlg;

        ocurdlg = curdlg;
        curdlg = ident;

        d->d_xhint = get_xinteger(2, -1);       /* coordinates */
        d->d_yhint = get_xinteger(3, -1);
        d->d_retval = -1;
        d->d_running = 1;

        dialog_tty_run(d);                      /* tty or gui */
        ret = d->d_retval;

        d->d_running = 0;
        curdlg = ocurdlg;

        if (d->d_delete) {
            dlg_destroy(d);
        }
    }
    acc_assign_int(ret);
}


/*  Function:           do_dialog_delete
 *      dialog_delete) primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: dialog_delete - Delete a dialog resource

        int
        dialog_delete(int dialog)

    Macro Description:
        The 'dialog_delete()' primitive deletes the specified dialog
        resource.

    Macro Parameters:
        dialog - Optional dialog instance handle, if omitted the
            current dialog is referenced.

    Macro Returns:
        The 'dialog_delete()' primitive return non-zero on success,
        otherwise zero on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        dialog_create
 */
void
do_dialog_delete(void)          /* int (int dialog) */
{
    const int ident = get_xinteger(1, curdlg);
    DIALOG_t *d;
    int ret = 0;

    if (NULL != (d = dialog_find(ident))) {
        if (! d->d_running) {
            dlg_destroy(d);
            ret = 1;
        } else {
            d->d_running = -1;
            d->d_delete = 1;
            ret = 2;
        }
    }
    acc_assign_int(ret);
}


/*  Function:           do_dialog_exit
 *      dialog_exit primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: dialog_exit - Exit a dialog resource

        int
        dialog_exit(int retval = 0, [int dialog])

    Macro Description:
        The 'dialog_exit()' primitive exits the current dialog.

    Macro Parameters:
        retval - Integer return value, returned to the
                    original <dialog_run> caller.

        dialog - Optional dialog instance handle, if omitted the
            current dialog is referenced.

    Macro Returns:
        Returns 1 on success otherwise 0.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        dialog_run
 */
void
do_dialog_exit(void)            /* int (int retval = 0, [int dialog]) */
{
    const int retval = get_xinteger(1, 0);
    const int ident = get_xinteger(2, curdlg);
    DIALOG_t *d;
    int ret = 0;

    if (NULL != (d = dialog_find(ident))) {
        d->d_running = -1;
        d->d_retval = retval;
        ret = 1;
    }
    acc_assign_int(ret);
}


/*  Function:           inq_dialog
 *      inq_dialog primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_dialog - Retrieve the current dialog resource.

        int
        inq_dialog()

    Macro Description:
        The 'inq_dialog()' primitive retrieves the resource handle of the
        current executing dialog source (if any), otherwise zero.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_dialog()' primitive returns an integer representing the
        dialog resource handle (see dialog_create) for more details.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        dialog_create, dialog_run
 */
void
inq_dialog(void)                /* int () */
{
    acc_assign_int(curdlg);
}


/*  Function:           do_widget_set
 *      widget_set primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: widget_set - Set a widget attribute

        declare
        widget_set([int dialog],
                [int name|string name], declare value,
                [int attr = DLGA_VALUE], [int index = 0])

    Macro Description:
        The 'widget_set()' primitive sets the value of a dialog
        resource.

    Macro Parameters:
        dialog - Optional dialog instance handle, if omitted the
            current dialog is referenced.
        name - Resource identifier or name.
        value - Value to assign.
        attr - Attribute, if omitted DLGA_VALUE is assumed.
        index - Optional integer index, required for attributes with
            mutliple elements.

    Macro Returns:
        The 'widget_set()' primitive returns the assigned value,
        otherwise NULL on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        widget_get
 */
void
do_widget_set(void)             /* ([int dialog], [int name|string name], declare value,
                                        [int attr = DLGA_VALUE], [int index = 0]) */
{
    const int ident = get_xinteger(1, curdlg);
    const uint16_t attr = (uint16_t) get_xinteger(4, DLGA_VALUE);
    const uint16_t idx = (uint16_t) get_xinteger(5, 0);
    WIDGETDATA_t data;
    const char *str;
    const LIST *lp;
    DIALOG_t *d;
    WIDGET_t *w;

    trace_ilog("widget_set(%u)", ident);

    if (NULL == (d = dialog_find(ident))) {
        trace_log(": unknown dialog\n");
        return;
    }

    if (isa_integer(2)) {
        w = widget_byident(d, get_xinteger(2, 0));
    } else {
        w = widget_byname(d, get_str(2));
    }
    if (NULL == w) {
        trace_log(": unknown widget\n");
        return;
    }

    if (isa_undef(3)) {
        data.d_type = D_NULL;
        data.d_u.ivalue = 0;
        trace_log(" = NUL\n");

    } else if (isa_integer(3)) {
        data.d_type = D_INT;
        data.d_u.ivalue = get_accint(3);
        trace_log(" = INT %" ACCINT_FMT "\n", data.d_u.ivalue);

    } else if (isa_float(3)) {
        data.d_type = D_FLOAT;
        data.d_u.fvalue = get_accfloat(3);
        trace_log(" = FLT %" ACCFLOAT_FMT "\n", data.d_u.fvalue);

    } else if (NULL != (str = get_xstr(3))) {
        data.d_type = D_STR;
        data.d_u.svalue = str;
        trace_log(" = STR \"%s\"\n", data.d_u.svalue);

    } else if (NULL != (lp = get_xlist(3))) {
        data.d_type = D_LIST;
        data.d_u.lvalue = lp;
        trace_log(" = LIST\n");

    } else {
        data.d_type = D_NONE;
        data.d_u.lvalue = 0;
        trace_log(" = NONE\n");
    }

    widget_send(w, WIDGET_SET, DIALOGARG32(attr, idx), (WIDGETARG_t)&data);
}


/*  Function:           do_widget_get
 *      widget_get primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: widget_get - Retrieve a widget attribute.

        declare
        widget_get([int dialog],
            [int name|string name],
            [int attr = DLGA_VALUE],
            [int index = 0])

    Macro Description:
        The 'widget_get()' primitive retrieves the value of a dialog
        resource.

    Macro Parameters:
        dialog - Optional dialog instance handle, if omitted the
            current dialog is referenced.
        name - Resource identifier or name.
        attr - Attribute, if omitted DLGA_VALUE is assumed.
        index - Optional integer index, required for attributes with
            multiple elements.

    Macro Returns:
        The 'widget_get()' primitive returns the assigned value,
        otherwise NULL on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        widget_set
 */
void
do_widget_get(void)             /* declare ([int dialog], [int name|string name],
                                        [int attr = DLGA_VALUE], [int index = 0]) */
{
    const int ident = get_xinteger(1, curdlg);
    const uint16_t attr = (uint16_t) get_xinteger(3, DLGA_VALUE);
    const uint16_t idx = (uint16_t) get_xinteger(4, 0);
    WIDGETDATA_t data;
    DIALOG_t *d;
    WIDGET_t *w;

    trace_ilog("widget_get(%u)", ident);

    acc_assign_null();

    if (NULL == (d = dialog_find(ident))) {
        trace_log(": unknown dialog\n");
        return;
    }

    if (isa_integer(2)) {
        w = widget_byident(d, get_xinteger(2, 0));
    } else {
        w = widget_byname(d, get_str(2));
    }
    if (NULL == w) {
        trace_log(": unknown widget\n");
        return;
    }

    data.d_type = D_NULL;
    data.d_u.svalue = NULL;
    (void) widget_send(w, WIDGET_GET, DIALOGARG32(attr, idx), (WIDGETARG_t)&data);

    switch (data.d_type) {
    case D_INT:
        acc_assign_int(data.d_u.ivalue);
        trace_log(" = INT %" ACCINT_FMT "\n", data.d_u.ivalue);
        break;

    case D_FLOAT:
        acc_assign_float(data.d_u.fvalue);
        trace_log(" = FLT %" ACCFLOAT_FMT "\n", data.d_u.fvalue);
        break;

    case D_STR:
        acc_assign_str(data.d_u.svalue, -1);
        trace_log(" = STR \"%s\"\n", data.d_u.svalue);
        break;

    case D_LIST:
        acc_assign_list(data.d_u.lvalue, -1);
        trace_log(" = LIST\n");
        break;

 /* case D_NULL: */
 /* case D_NONE: */
    default:
        trace_log(" = NULL(%d)\n", data.d_type);
        break;
    }
}


/*  Function:           create_message
 *      create_message primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF-TODO>>
    Macro: create_message - Create a message box.

        int
        create_message(string msg, int|string button, [...])

    Macro Description:
        The 'create_notice()' primitive displays a modal message box and
        prompts the user to acknowledge with one or more buttons.

    Macros Parameters:
        msg - String containing the message to be displayed. If the
            string consists of more than one line, you can separate the
            lines using a carriage return and/or linefeed character
            between each line.

        button - Either a set of flags indicating the buttons
            displayed in the message box, otherwise the text of the first
            button with the implied index of 0.

        ... - Optional text of buttons in addition to the text contained
            within the 'button0'.

    *Button Mode*

(start table,format=basic)
        [Constant               [Description                                    ]
        MB_OK                   The message box contains one push button, "OK".

        MB_OKCANCEL             The message box contains two push buttons,
                                "OK" and "Cancel".

        MB_RETRYCANCEL          The message box contains two push buttons:
                                "Retry" and "Cancel".

        MB_YESNO                The message box contains two push buttons:
                                "Yes" and "No".

        MB_YESNOCANCEL          The message box contains three push buttons:
                                "Yes", "No" and "Cancel".
(end table)

    Macro Portability:
        Available in both console and GUI versions.

    Macro Return:
        The 'create_notice' returns the button index selected by the user
        as follows.

(start table,format=basic)
        [Constant   [Value      [Description                                    ]
        IDFIRST     1           The 'First' button.

        IDSECOND    2           The 'Second' button.

        IDTHIRD     3           The 'Third' button.

        IDABORT     100         The 'Abort' button was selected.

        IDCANCEL    101         The 'Cancel' button was selected.

        IDRETRY     102         The 'Retry' button was selected.

        IDNO        103         The 'No' button was selected.

        IDYES       104         The 'Yes' button was selected.
(end table)

 */
void
do_create_notice(void)                          /* 12/09/10, rename to match similar Crisp primitive */
{
    //TODO
}


/*  Function:           dialog_update
 *      update interface for dialog controls.
 *
 *  Parameters:
 *      d - Dialog instance.
 *
 *  Returns:
 *      nothing.
 */
void
dialog_update(DIALOG_t *d, WINDOW_t *wp)
{
    __CUNUSED(wp)
    (d->d_controller)(d, DIALOG_UUPDATE, 0);
}


static DIALOG_t *
dlg_new(void)
{
    DIALOG_t *d;

    if (! dialogseq) {
        TAILQ_INIT(&dialogq);
        dialogseq = GRBASE_DIALOG-1;            /* seed sequence */
    }

    if (NULL == (d = chk_calloc(sizeof(DIALOG_t),1))) {
        return NULL;
    }
    d->d_ident = ++dialogseq;                   /* dialog identifier */
    TAILQ_INSERT_HEAD(&dialogq, d, d_node);
    CIRCLEQ_INIT(&d->d_widgetq);
    widget_init(&d->d_widget, sizeof(WIDGET_t), (WIDGETCB_t)dialog_default);
    d->d_widget.w_root = d;
    d->d_widget.w_ident = d->d_ident;           /* need by callback() */

    return d;
}


static void
dlg_destroy(DIALOG_t *d)
{
    WidgetQueue_t *head = &d->d_widgetq;
    WIDGET_t *w;

    dialog_send(d, WIDGET_DESTROY, 0, 0);
    dialog_bcast(d, NULL, NULL, FALSE, WIDGET_DESTROY, 0, 0);

    while ((w = CIRCLEQ_FIRST(head)) != CIRCLEQ_END(head, _widget)) {
        CIRCLEQ_REMOVE(head, w, w_node);
        widget_destroy(w);
        chk_free(w);
    }

    widget_destroy(&d->d_widget);
    TAILQ_REMOVE(&dialogq, d, d_node);
    chk_free(d);
}


DIALOG_t *
dialog_find(int ident)
{
    DIALOG_t *d;

    for (d = TAILQ_FIRST(&dialogq); d; d = TAILQ_NEXT(d, d_node)) {
        if (d->d_ident == ident) {
            return d;
        }
    }
    return NULL;
}


WIDGET_t *
widget_byname(DIALOG_t *d, const char *name)
{
    WIDGET_t *first, *w;

    if (NULL != (first = CIRCLEQ_FIRST(&d->d_widgetq))) {
        w = first;
        do {
            assert(WIDGET_MAGIC == w->w_magic);
            if (w->w_name) {
                if (0 == strcmp(w->w_name, name)) {
                    return w;
                }
            }
        } while (first != (w = dialog_next(d, w)));
    }
    return NULL;
}


WIDGET_t *
widget_byident(DIALOG_t *d, int ident)
{
    WIDGET_t *first, *w;

    if (NULL != (first = CIRCLEQ_FIRST(&d->d_widgetq))) {
        w = first;
        do {
            assert(WIDGET_MAGIC == w->w_magic);
            if (w->w_ident == ident) {
                return w;
            }
        } while (first != (w = dialog_next(d, w)));
    }
    return NULL;
}


static void
dlg_queue(DIALOG_t *d, WIDGET_t *w)
{
    const uint16_t ident = (uint16_t) ++d->d_widgetseq;

    assert(WIDGET_MAGIC == w->w_magic);
    w->w_root = d;
    w->w_ident = DIALOGARG32(d->d_ident, ident); /* default identify [dialog|sequence] */
    CIRCLEQ_INSERT_TAIL(&d->d_widgetq, w, w_node);
}


static void
dlg_inherit(WIDGET_t *parent, WIDGET_t *w)
{
    WidgetList_t *queue = &parent->w_children;

    assert(WIDGET_MAGIC == w->w_magic);
    w->w_parent = parent;
    w->w_depth = parent->w_depth+1;
    TAILQ_INSERT_TAIL(queue, w, w_sibling);
}


/*  Function:           dialog_count
 *      Retrieve the widget count owned by the dialog 'd'.
 *
 *  Parameters:
 *      d - Dialog instance.
 *
 *  Returns:
 *      Widget count
 */
int
dialog_count(DIALOG_t *d)
{
    WidgetQueue_t *head = &d->d_widgetq;
    WIDGET_t *w, *end = CIRCLEQ_END(head, _widget);
    int count = 0;

    for (w = CIRCLEQ_FIRST(head); w && w != end; w = CIRCLEQ_NEXT(w, w_node)) {
        assert(WIDGET_MAGIC == w->w_magic);
        ++count;
    }
    return count;
}


/*  Function:           dialog_next
 *      Retrieve the next widget within the dialogs 'd' widget queue from the
 *      anchor widget 'w'.
 *
 *  Parameters:
 *      d - Dialog instance.
 *      w - Widget reference.
 *
 *  Returns:
 *      Address of the next widget within the chain.
 *
 *  Warning:
 *      This interface shall loop.
 */
WIDGET_t *
dialog_next(DIALOG_t *d, WIDGET_t *w)
{
    WidgetQueue_t *head = &d->d_widgetq;

    assert(NULL == w || WIDGET_MAGIC == w->w_magic);
    if (NULL == w || (w = CIRCLEQ_NEXT(w, w_node)) == CIRCLEQ_END(head, _widget)) {
        w = CIRCLEQ_FIRST(head);
    }
    assert(NULL == w || WIDGET_MAGIC == w->w_magic);
    return w;
}


/*  Function:           dialog_prev
 *      Retrieve the previous widget within the dialogs 'd' widget queue
 *      from the anchor widget 'w'.
 *
 *  Parameters:
 *      d - Dialog instance.
 *      w - Widget reference.
 *
 *  Returns:
 *      Address of the next widget within the chain.
 *
 *  Warning:
 *      This interface shall loop.
 */
WIDGET_t *
dialog_prev(DIALOG_t *d, WIDGET_t *w)
{
    WidgetQueue_t *head = &d->d_widgetq;

    assert(NULL == w || WIDGET_MAGIC == w->w_magic);
    if (NULL == w || (w = CIRCLEQ_PREV(w, w_node)) == CIRCLEQ_END(head, _widget)) {
        w = CIRCLEQ_LAST(head);
    }
    assert(NULL == w || WIDGET_MAGIC == w->w_magic);
    return w;
}


/*  Function:           dialog_callback2
 *      Send a message to the dialog "callback". The message shall be directed to the
 *      dialog callback, if assigned.
 *
 *  Parameters:
 *      d - Dialog reference.
 *      event - Event identifier.
 *      val1 - First value.
 *      val2 - Second value.
 *      rval - Return value.
 *
 *  Return:
 *      Non-zero if macro was called, otherwise zero.
 */
int
dialog_callback2(DIALOG_t *d, int event, accint_t val1, accint_t val2, int *rval)
{
    int ret = FALSE;
    const char *c1;

    c1 = d->d_callback;                         /* dialog default */
    if (c1) {
        trace_ilog("hooked(curbp:%p, curwp:%p)\n", curbp, curwp);
        ret = callback("dialog", c1, &d->d_widget, event, val1, val2, rval);
        trace_ilog("hooked(curbp:%p, curwp:%p)\n", curbp, curwp);
    }
    return ret;
}


int
dialog_callback(DIALOG_t *d, int event, accint_t val1, accint_t val2)
{
    return dialog_callback2(d, event, val1, val2, NULL);
}


/*  Function:           widget_callback2
 *      Send a message to the widget "callback". The message shall be
 *      directed firstly at the widget specific callback, if assigned,
 *      otherwise the dialog callback, if assigned.
 *
 *  Parameters:
 *      w - Widget reference.
 *      event - Event identifier.
 *      val1 - First value.
 *      val2 - Second value.
 *      rval - Return value.
 *
 *  Return:
 *      Non-zero if macro was called, otherwise zero.
 */
int
widget_callback2(WIDGET_t *w, int event, accint_t val1, accint_t val2, int *rval)
{
    const char *c1, *c2;
    int ret = FALSE;

    assert(WIDGET_MAGIC == w->w_magic);
    c1 = w->w_callback;                         /* widget specific */
    c2 = w->w_root->d_callback;                 /* dialog default */

    (w->w_root->d_controller)(w->w_root, DIALOG_UCALLBACK, TRUE);

    if (c1) {
        ret = callback("widget", c1, w, event, val1, val2, rval);

    } else if (c2) {
        ret = callback("widget", c2, w, event, val1, val2, rval);
    }

    (w->w_root->d_controller)(w->w_root, DIALOG_UCALLBACK, FALSE);
    return ret;
}


int
widget_callback(WIDGET_t *w, int event, accint_t val1, accint_t val2)
{
    return widget_callback2(w, event, val1, val2, NULL);
}


/*  Function:           callback
 *      Send a message to the specified 'callback'.
 *
 *  Parameters:
 *      label -
 *      cbname -
 *      w - Widget reference.
 *      event - Event identifier.
 *      val1 - First value.
 *      val2 - Second value.
 *      rval - Return value.
 *
 *  Return:
 *      Non-zero if macro was called, otherwise zero.
 */
static int
callback(const char *label,
    const char *cbname, WIDGET_t *w, int event, accint_t val1, accint_t val2, int *rval)
{
    LIST tmpl[LIST_SIZEOF(9)], *lp = tmpl;      /* 9 atoms */
    int ret;

    assert(WIDGET_MAGIC == w->w_magic);

    if (! macro_exist(cbname, "dialog")) {
        return FALSE;
    }

    trace_ilog("%s_callback(%p->%s(%d), bp:%p, wp:%p: event:%d/0x%x, val1:%d/0x%x, val2:%d/0x%x)\n",
        label, w, (w->w_name ? w->w_name : "unnamed"), w->w_ident,
        curbp, curwp, event, event, (int)val1, (int)val1, (int)val2, (int)val2);

    /*
     *  macro-name +
     *      Object identifier.
     *      Assigned name (if any).
     *      Event.
     *      Value one.
     *      Value two.
     */
    lp = atom_push_sym(lp, cbname);             /* macro-name */
    lp = atom_push_int(lp, (accint_t) w->w_ident);
    lp = atom_push_const(lp, (w->w_name ? w->w_name : ""));
    lp = atom_push_int(lp, (accint_t) event);
    lp = atom_push_int(lp, (accint_t) val1);
    lp = atom_push_int(lp, (accint_t) val2);
    atom_push_halt(lp);
    assert(lp < (tmpl + sizeof(tmpl)));
    execute_nmacro(tmpl);
    ret = (int) acc_get_ival();
    trace_ilog("%s_callback ; %d\n", label, ret);
    if (rval) {
        *rval = ret;
    }
    return TRUE;
}


/*  Function:           dialog_bcast
 *      Broadcast a message to all dialog widgets.
 *
 *  Parameters:
 *      d - Dialog reference.
 *      first - First widget to execute.
 *      selector - Selector callback.
 *      reverse - Reverse otherwise forward widget iteration.
 *      msg - Message identifier.
 *      p1 - First parameter.
 *      p2 - Second parameter.
 *
 *  Returns:
 *      nothing
 */
void
dialog_bcast(DIALOG_t *d, WIDGET_t *first, DIALOGSELECTOR_t selector,
        int reverse, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    if (NULL == first) {
        first = CIRCLEQ_FIRST(&d->d_widgetq);
        if (NULL == first) {
            return;
        }
    }

    assert(WIDGET_MAGIC == first->w_magic);

    if (reverse) {
        WIDGET_t *prev, *current = dialog_prev(d, first);

        while (first != current) {
            prev = dialog_prev(d, current);
            if (NULL == selector || (selector)(current, msg, p2, p2)) {
                widget_send(current, msg, p1, p2);
            }
            current = prev;
        }
        widget_send(first, msg, p1, p2);

    } else {
        WIDGET_t *next, *current = first;

        do {
            next = dialog_next(d, current);
            if (NULL == selector || (selector)(current, msg, p2, p2)) {
                widget_send(current, msg, p1, p2);
            }
            current = next;
        } while (first != current);
    }
}


/*  Function:           dialog_send
 *      Send a message the dialog widget.
 *
 *  Parameters:
 *      d - Dialog reference.
 *      msg - Message identifier.
 *      p1 - First parameter.
 *      p2 - Second parameter.
 *
 *  Returns:
 *      nothing
 */
uint32_t
dialog_send(DIALOG_t *d, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    return widget_send(&d->d_widget, msg, p1, p2);
}


/*  Function:           dialog_default
 *      Default dialog message handler.
 *
 *  Parameters:
 *      w - Widget reference.
 *      msg - Message identifier.
 *      p1 - First parameter.
 *      p2 - Second parameter.
 *
 *  Returns:
 *      FALSE if the message wasn't handled, otherwise a non-zero completion code. Note
 *      that unprocessed message are passed onto widget_default.
 */
uint32_t
dialog_default(WIDGET_t *w, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    DIALOG_t *d = (DIALOG_t *)w;

    switch (msg) {
    case WIDGET_SET: {          /* set attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            switch (DIALOGARGLO(p1)) {
            case DLGA_TITLE:
                chk_free((void *)d->d_title);
                d->d_title = (data->d_u.svalue ? chk_salloc(data->d_u.svalue) : NULL);
                if (NULL == w->w_name) {
                    w->w_name = (d->d_title ? chk_salloc(d->d_title) : NULL);
                }
                return TRUE;

            case DLGA_TEXT_ONLY:
            case DLGA_GUI_ONLY:
                d->d_type = DIALOGARGLO(p1);
                break;

            default:
                return widget_default(w, msg, p1, p2);
            }
        }
        return TRUE;

    case WIDGET_DESTROY:
        chk_free((void *)d->d_title), d->d_title = NULL;
        break;
    }
    return widget_default(w, msg, p1, p2);
}


/*  Function:           widget_init
 *      Initialise a widget.
 *
 *  Parameters:
 *      w - Widget reference.
 *      size - Base object size, in bytes.
 *      handler - Callback handler.
 *
 *  Returns:
 *      nothing.
 */
void
widget_init(WIDGET_t *w, uint32_t size, WIDGETCB_t handler)
{
    (void) memset(w, 0, size);

    TAILQ_INIT(&w->w_children);
    w->w_magic   = WIDGET_MAGIC;
    w->w_handler = (handler ? handler : widget_default);
    w->w_attach  = DLGA_ATTACH_TOP;
    w->w_align   = DLGA_ALIGN_CENTER;
}


/*  Function:           widget_destroy
 *      Destroy a widget, releasing any allocated storage.
 *
 *  Parameters:
 *      w - Widget reference.
 *
 *  Returns:
 *      nothing.
 */
void
widget_destroy(WIDGET_t *w)
{
    assert(WIDGET_MAGIC == w->w_magic);

    chk_free((void *)w->w_name), w->w_name = NULL;
    chk_free((void *)w->w_callback), w->w_callback = NULL;
    chk_free((void *)w->w_help), w->w_help = NULL;
    chk_free((void *)w->w_tooltip), w->w_tooltip = NULL;
}


/*  Function:           widget_send
 *      Send a message to the widget "handler".
 *
 *  Parameters:
 *      w - Widget reference.
 *      msg - Message identifier.
 *      p1 - First parameter.
 *      p2 - Second parameter.
 *
 *  Returns:
 *      FALSE(0) if the message wasn't handled, otherwise a non-zero completion code.
 */
uint32_t
widget_send(WIDGET_t *w, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    assert(WIDGET_MAGIC == w->w_magic);
    if (w->w_handler) {
        return (*w->w_handler)(w, msg, p1, p2);
    }
    return FALSE;
}


/*  Function:           widget_default
 *      Default widget message handler.
 *
 *  Parameters:
 *      w - Widget reference.
 *      msg - Message identifier.
 *      p1 - Parameter one.
 *      p2 - Second parameter.
 *
 *  Returns:
 *      FALSE(0) if the message wasn't handled, otherwise a non-zero completion code.
 */
uint32_t
widget_default(WIDGET_t *w, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2)
{
    switch (msg) {
    case WIDGET_INIT:           /* creation/destruction */
        return TRUE;

    case WIDGET_DESTROY:
        widget_destroy(w);
        return TRUE;

    case WIDGET_SET: {          /* set widget attribute */
            const WIDGETDATA_t *data = (WIDGETDATA_t *)p2;

            switch (DIALOGARGLO(p1)) {
            case DLGA_NAME: {
                    const char *name = widgetdata_str(data);

                    chk_free((void *)w->w_name);
                    w->w_name = (name ? chk_salloc(name) : NULL);
                }
                break;

            case DLGA_IDENT:
                w->w_ident = widgetdata_uint32(data);
                break;

            case DLGA_CALLBACK: {
                    const char *cb = widgetdata_str(data);

                    chk_free((void *)w->w_callback);
                    if (NULL == cb) {
                        w->w_callback = NULL;
                    } else {
                        if (cb == (w->w_callback = macro_resolve(cb))) {
                            w->w_callback = chk_salloc(cb);
                        }
                    }
                }
                break;

            case DLGA_HELP: {
                    const char *help = widgetdata_str(data);

                    chk_free((void *)w->w_help);
                    w->w_help = (help ? chk_salloc(help) : NULL);
                }
                break;

            case DLGA_TOOLTIP: {
                    const char *help = widgetdata_str(data);

                    chk_free((void *)w->w_tooltip);
                    w->w_tooltip = (help ? chk_salloc(help) : NULL);
                }
                break;

            case DLGA_X:                /* position */
                if ((w->w_x = (int)widgetdata_int32(data)) < 1) {
                    w->w_x = 1;
                }
                break;
            case DLGA_Y:
                if ((w->w_y = (int)widgetdata_int32(data)) < 1) {
                    w->w_y = 1;
                }
                break;

            case DLGA_COLS:             /* sizing */
                if ((w->w_reqcols = (int)widgetdata_int32(data)) < 1) {
                    w->w_reqcols = 1;
                }
                break;
            case DLGA_ROWS:
                if ((w->w_reqrows = (int)widgetdata_int32(data)) < 1) {
                    w->w_reqrows = 1;
                }
                break;

            case DLGA_PADX:
                if ((w->w_padx = (int)widgetdata_int32(data)) < 0) {
                    w->w_padx = 0;
                }
                break;
            case DLGA_PADY:
                if ((w->w_pady = (int)widgetdata_int32(data)) < 0) {
                    w->w_pady = 0;
                }
                break;

            case DLGA_ATTACH_TOP:       /* frame attachment */
            case DLGA_ATTACH_BOTTOM:
            case DLGA_ATTACH_LEFT:
            case DLGA_ATTACH_RIGHT:
                w->w_attach = (int)p1;
                break;

            case DLGA_ALIGN_N:          /* content alignment */
            case DLGA_ALIGN_NE:
            case DLGA_ALIGN_E:
            case DLGA_ALIGN_SE:
            case DLGA_ALIGN_S:
            case DLGA_ALIGN_SW:
            case DLGA_ALIGN_W:
            case DLGA_ALIGN_NW:
            case DLGA_ALIGN_CENTER:
                w->w_align = (int)p1;
                break;

            case DLGA_ORIENTATION:      /* element orientation within boxes */
                if ((w->w_orientation = (int)widgetdata_int32(data)) < 0) {
                    w->w_orientation = 0;
                }
                break;

            case DLGA_DEFAULT_FOCUS:
                w->w_flags |= WIDGET_FDEFFOCUS;
                break;

            case DLGA_ALLOW_RESIZE:
                w->w_flags |= WIDGET_FRESIZE;
                break;
            case DLGA_ALLOW_EXPAND:
                w->w_flags |= WIDGET_FEXPAND;
                break;
            case DLGA_ALLOW_FILLX:
                w->w_flags |= WIDGET_FFILLX;
                break;
            case DLGA_ALLOW_FILLY:
                w->w_flags |= WIDGET_FFILLY;
                break;

            case DLGA_PROPAGATE:        /* geometry propagation */
                /*
                 *  The packer normally computes how large a master must be to just exactly meet
                 *  the needs of its slaves, and it sets the requested width and height of the
                 *  master to these dimensions. This causes geometry information to propagate up
                 *  through a window hierarchy to a top-level window so that the entire sub-tree
                 *  sizes itself to fit the needs of the leaf windows. However, the pack
                 *  propagate command may be used to turn off propagation for one or more
                 *  masters. If propagation is disabled then the packer will not set the
                 *  requested width and height of the packer. This may be useful if, for example,
                 *  you wish for a master window to have a fixed size that you specify.;;) }
                 *
                 *  Propagation is enabled by default.
                 */
                if (0 == widgetdata_int32(data)) {
                    w->w_flags &= ~WIDGET_FDONTPROPAGATE;
                } else {
                    w->w_flags |= WIDGET_FDONTPROPAGATE;
                }
                break;

            case DLGA_SENSITIVE:
                if (widgetdata_int32(data)) {
                    w->w_flags &= ~WIDGET_FGREYED;
                } else {
                    w->w_flags |= WIDGET_FGREYED;
                }
                break;
            case DLGA_GREYED:
                w->w_flags |= WIDGET_FGREYED;
                break;
            case DLGA_ACTIVE:
                w->w_flags &= ~(WIDGET_FGREYED|WIDGET_FHIDDEN);
                break;

            case DLGA_TABSTOP:          /* <tab> stop */
                if (widgetdata_int32(data)) {
                    w->w_flags |=  WIDGET_FTABSTOP;
                    w->w_flags &= ~WIDGET_FTABNOT;
                } else {
                    w->w_flags &= ~WIDGET_FTABSTOP;
                    w->w_flags |=  WIDGET_FTABNOT;
                }
                break;

            case DLGA_AUTOMOVE:         /* auto-move on selection */
                if (widgetdata_int32(data)) {
                    w->w_flags |=  WIDGET_FAUTOMOVE;
                } else {
                    w->w_flags &= ~WIDGET_FAUTOMOVE;
                }
                break;

            case DLGA_HIDDEN:
                w->w_flags |= WIDGET_FHIDDEN;
                break;
            case DLGA_VISIBLE:
                w->w_flags &= ~WIDGET_FHIDDEN;
                break;

            case DLGA_KEYDOWN:          /* control keydown event */
                if (widgetdata_int32(data)) {
                    w->w_flags |=  WIDGET_FKEYDOWN;
                } else {
                    w->w_flags &= ~WIDGET_FKEYDOWN;
                }
                break;

            case DLGA_HOTKEY:           /* hot-key */
                if ((w->w_hotkey = (int)widgetdata_int32(data)) < 0) {
                    w->w_hotkey = 0;
                }
                break;

            case DLGA_ACCELERATOR: {    /* set accelerator */
                    const char *acc = widgetdata_str(data);

                    chk_free((void *)w->w_accelerator);
                    w->w_accelerator = acc ? chk_salloc(acc) : NULL;
                }
                break;

            default:                    /* unknown*/
                return FALSE;                   /* unknown attribute */
            }
            return TRUE;                        /* attribute ok */
        }

    case WIDGET_GET:            /* get attribute */
        return FALSE;
    }

    return FALSE;                               /* unknown request */
}


uint32_t
widgetdata_uint32(const WIDGETDATA_t *data)
{
    switch (data->d_type) {
    case D_INT:
        return (uint32_t)data->d_u.ivalue;
    case D_STR:
        return (uint32_t)strtoul(data->d_u.svalue, NULL, 10);
    case D_LIST:
        return (uint32_t)lst_length(data->d_u.lvalue);
    default:
        break;
    }
    return 0;
}


int32_t
widgetdata_int32(const WIDGETDATA_t *data)
{
    switch (data->d_type) {
    case D_INT:
        return (int32_t)data->d_u.ivalue;
    case D_STR:
        return (int32_t)strtol(data->d_u.svalue, NULL, 10);
    case D_LIST:
        return (int32_t)lst_length(data->d_u.lvalue);
    default:
        break;
    }
    return 0;
}


const char *
widgetdata_str(const WIDGETDATA_t *data)
{
    switch (data->d_type) {
    case D_INT:
        return NULL;
    case D_STR:
        return data->d_u.svalue;
    case D_LIST:
        return NULL;                            /* XXX - take first element?? */
    default:
        break;
    }
    return NULL;
}


const char *
widgetdata_get(const WIDGETDATA_t *data, char *buf, int len)
{
    const char *s;

    if (NULL == (s = widgetdata_str(data))) {
        if (buf && len > 0) {
            int32_t v = widgetdata_int32(data);

            sxprintf(buf, len, "%d", (int)v);
            s = buf;
        }
    }
    return s;
}

/*end*/

