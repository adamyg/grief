#ifndef GR_DIALOG_H_INCLUDED
#define GR_DIALOG_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_dialog_h,"$Id: dialog.h,v 1.18 2014/10/26 22:13:10 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: dialog.h,v 1.18 2014/10/26 22:13:10 ayoung Exp $
 * Dialog manager public interface.
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

#include "editor.h"
#include "tailqueue.h"
#include "circlequeue.h"

/*--export--defines--*/
/*
 *  Dialog widget classes
 *      Note, not all are implemented at the time; shall occur on a as needed basic.
 */
#define DLGC_MIN                0x2000
#define DLGC_NULL               0x2000

#define DLGC_CONTAINER          0x2001          /* Widget container */
#define DLGC_GROUP              0x2002          /* Group start */
#define DLGC_TAB                0x2003          /* Tab panel */
#define DLGC_END                0x200f          /* End of current container */

#define DLGC_PUSH_BUTTON        0x2011          /* Push button */
#define DLGC_RADIO_BUTTON       0x2012          /* Radio button */
#define DLGC_CHECK_BOX          0x2013          /* Check box */
#define DLGC_TOGGLE             0x2014          /* Toggle button */
#define DLGC_LABEL              0x2015          /* label */
#define DLGC_LIST_BOX           0x2016          /* List box */
#define DLGC_EDIT_FIELD         0x2017          /* Edit field */
#define DLGC_NUMERIC_FIELD      0x2018          /* Numeric edit field */
#define DLGC_COMBO_FIELD        0x2019          /* Edit field and drop list */

#define DLGC_SPACER             0x2030          /* Display spacer */
#define DLGC_SEPARATOR_HORIZONTAL 0x2031
#define DLGC_SEPARATOR_VERTICAL 0x2032

#define DLGC_TREE               0x2040          /* *not* implemented */
#define DLGC_GAUGE              0x2041          /* *not* implemented */
#define DLGC_SLIDER             0x2042          /* *not* implemented */
#define DLGC_VSCROLLBAR         0x2043          /* *not* implemented */
#define DLGC_HSCROLLBAR         0x2044          /* *not* implemented */
#define DLGC_GRID               0x2070          /* *not* implemented */
#define DLGC_MAX                0x2100

/*
 *  Dialog widget attributes.
 */
#define DLGA_MIN                0x3000
#define DLGA_NULL               0x3000          /* MIN */

#define DLGA_TITLE              0x3001          /* Title for certain widgets. */
#define DLGA_NAME               0x3002          /* Widget name. */
#define DLGA_IDENT              0x3003          /* Identifier (user supplied). */
#define DLGA_CALLBACK           0x3004          /* User callback function/macro. */
#define DLGA_HELP               0x3005          /* Help topic/command. */
#define DLGA_TOOLTIP            0x3006          /* Tooltip text. */
#define DLGA_X                  0x3007
#define DLGA_Y                  0x3008
#define DLGA_COLS               0x3009
#define DLGA_ROWS               0x300a
#define DLGA_VERSION            0x300b          /* Wiget specific version/feature set identifier */

#define DLGA_ATTACH_BOTTOM      0x3010          /* Attachment of widget within dialog box. */
#define DLGA_ATTACH_TOP         0x3011
#define DLGA_ATTACH_LEFT        0x3012
#define DLGA_ATTACH_RIGHT       0x3013

#define DLGA_ALIGN_N            0x3020          /* Alignment of widget within frame. */
#define DLGA_ALIGN_NE           0x3021
#define DLGA_ALIGN_E            0x3022
#define DLGA_ALIGN_SE           0x3023
#define DLGA_ALIGN_S            0x3024
#define DLGA_ALIGN_SW           0x3025
#define DLGA_ALIGN_W            0x3026
#define DLGA_ALIGN_NW           0x3027
#define DLGA_ALIGN_CENTER       0x3028

#define DLGA_ALLOW_RESIZE       0x3030
#define DLGA_ALLOW_EXPAND       0x3031
#define DLGA_ALLOW_FILLX        0x3032
#define DLGA_ALLOW_FILLY        0x3033
#define DLGA_PROPAGATE          0x3034

#define DLGA_CANCEL_BUTTON      0x3040
#define DLGA_DEFAULT_BUTTON     0x3041
#define DLGA_DEFAULT_FOCUS      0x3042          /* Move cursor to specified widget. */
#define DLGA_ACTIVATES_DEFAULT  0x3043          /* Whether <enter> actives default button */
#define DLGA_AUTOMOVE           0x3044          /* Auto-move status */

#define DLGA_VALUE              0x3050          /* Set value of a check/radio/list-box/edit-field/combo-box. */
#define DLGA_LABEL              0x3051          /* Label text. */

#define DLGA_ACCELERATOR        0x3060          /* Accelerator key. */
#define DLGA_SENSITIVE          0x3061          /* Sensitive to input. */
#define DLGA_GREYED             0x3062          /* Greyed, non-sensitive to input */
#define DLGA_ACTIVE             0x3063          /* Active, sensitive to input */
#define DLGA_PADX               0x3064
#define DLGA_PADY               0x3065
#define DLGA_ORIENTATION        0x3066          /* Vertical (0)/horizontal (1) */
#define DLGA_HIDDEN             0x3067
#define DLGA_VISIBLE            0x3068
#define DLGA_KEYDOWN            0x3069          /* Control keydown event */
#define DLGA_TABSTOP            0x306a
#define DLGA_HOTKEY             0x306b

#define DLGA_TEXT_ONLY          0x3070          /* Text dialog widgets only. */
#define DLGA_GUI_ONLY           0x3071          /* GUI dialog widgets only. */

#define DLGA_EDEDITABLE         0x3080
#define DLGA_EDMAXLENGTH        0x3081          /* Maximum length of the entry (limit 32k) */
#define DLGA_EDVISIBLITY        0x3082          /* Content visible status (eg. password entry) */
#define DLGA_EDINVISIBLECHAR    0x3083          /* Character displayed in place of the real characters */
#define DLGA_EDPOSITION         0x3084          /* Cursor position within the the text entry */
#define DLGA_EDPLACEHOLDER      0x3085          /* Placeholder text, displayed when the field is empty and unfocused */
#define DLGA_EDVALIDATE         0x3086          /* Validate events */

#define DLGA_NUMDIGITS          0x3090          /* Number of decimal points (default = 0) */
#define DLGA_NUMMIN             0x3091
#define DLGA_NUMMAX             0x3092
#define DLGA_NUMINCREMENT       0x3093
#define DLGA_NUMWRAP            0x3094
#define DLGA_NUMSNAP            0x3095

    /*
    //  DLGA_CBEDITABLE
    //      Determines whether the combo-field is editable.  An editable combo-box allows the user to type
    //      into the field or select an item from the list to initialise the field, after which it can be
    //      edited.,  An non-editable field displays the selected item in the field, but the selection
    //      cannot be modified.
    //
    //  DLGA_CBRELAXED
    //      Determines whether an editable combo-field may contain text which is not restricted to the
    //      values contained within the collection.
    //
    //  DLGA_CBPOPUPMODE
    //      -1  = Hidden.
    //      0   = Normal.
    //      1   = Open.
    //      2   = Focus.
    //
    //  DLGA_CBAUTOCOMPLETE
    //      0=None
    //          Disables the automatic completion feature for the combo-box and edit-field controls.
    //
    //      1=Suggest
    //          Displays the auxiliary drop-down list associated with the edit control. The drop-down
    //          is populated with one or more suggested completion strings.
    //
    //      2=Append
    //          Appends the remainder of the most likely candidate string to the existing characters,
    //          highlighting the appended characters.
    //
    //      3=SuggestAppend
    //          Applies both Suggest and Append options.
    */
#define DLGA_CBEDITABLE         0x30a1
#define DLGA_CBRELAXMODE        0x30a2
#define DLGA_CBAUTOCOMPLETEMODE 0x30a3
#define DLGA_CBPOPUPMODE        0x30a4          /* Popup mode. */
#define DLGA_CBPOPUPSTATE       0x30a5          /* Popup status. */

#define DLGA_LBCOUNT            0x30b0          /* Item count. */
#define DLGA_LBELEMENTS         0x30b1          /* Collection elements. */
#define DLGA_LBINSERT           0x30b2          /* Insert an item. */
#define DLGA_LBREMOVE           0x30b3          /* Remove an item. */
#define DLGA_LBCLEAR            0x30b4          /* Clear all items. */
#define DLGA_LBSORTMODE         0x30b5
#define DLGA_LBHASSTRINGS       0x30b6
#define DLGA_LBICASESTRINGS     0x30b7

#define DLGA_LBCURSOR           0x30c0          /* Item under cursor. */
#define DLGA_LBACTIVE           0x30c1          /* Active/selected item(s). */
#define DLGA_LBDISPLAYTEXT      0x30c2
#define DLGA_LBDISPLAYTEXTLEN   0x30c3
#define DLGA_LBTEXT             0x30c4
#define DLGA_LBTEXTLEN          0x30c5
#define DLGA_LBPAGEMODE         0x30c6
#define DLGA_LBINDEXMODE        0x30c7
#define DLGA_LBROWS             0x30c8
#define DLGA_LBCOLS             0x30c9
#define DLGA_LBCOLUMNS          0x30ca
#define DLGA_LBWIDTH            0x30cb
#define DLGA_LBDUPLICATES       0x30cc

#define DLGA_GAUGEMIN           0x30e0          /* Minimum value. */
#define DLGA_GAUGEMAX           0x30e1          /* Maximum value. */

/*--end--*/
#define DLGA_MAX                0x3100

/*--export--defines--*/
/*
 *  Dialog callback events

    DLGE_KEYDOWN

    DLGE_COMMAND
        The DLGE_COMMAND message is sent when the user selects a command
        item from a menu, when a control sends a notification message to
        its parent window, or when an accelerator is encountered.

            p1 - If the message is from an accelerator, this value is 1.
            If the message is from a menu, this value is zero.

            p2 - Menu identifier which caused the event, otherwise the
            key code.

    DLGE_HELP
        Indicates that the user pressed the F1 key.

        If a menu is active when F1 is pressed, WM_HELP is sent to the
        window associated with the menu; otherwise, WM_HELP is sent to
        the widget that has the keyboard focus. If no widget has the
        focus, WM_HELP is sent to the currently active window.

 */
#define DLGE_INIT               0               /* Initlisation */
#define DLGE_CANCEL             1               /* Dialog cancelled */
#define DLGE_BUTTON             2               /* Button selected */
#define DLGE_VALIDATE           3               /* Validation */
#define DLGE_CHANGE             4               /* Object value change */
#define DLGE_SELECT             5               /* Selection (focus/unfocus) */
#define DLGE_KEYDOWN            6               /* Keydown event */
#define DLGE_COMMAND            7               /* Accelerator/Menu command */
#define DLGE_HELP               8               /* Help event */
/*--end--*/

/*--export--defines--*/
/*
 *  create_notice
 *      Button types.
 */
#define DLMB_OK                 0x0001
#define DLMB_YESNO              0x0010
#define DLMB_RETRY              0x0100
#define DLMB_CANCEL             0x1000
#define DLMB_OKCANCEL           (DLMB_OK|DLMB_CANCEL)
#define DLMB_YESNOCANCEL        (DLMB_YESNO|DLMB_CANCEL)
#define DLMB_RETRYCANCEL        (DLMB_RETRY|DLMB_CANCEL)
/*--end--*/

/*--export--defines--*/
/*
 *  create_notice
 *      Button identifiers.
 */
#define DLIDFIRST               1
#define DLIDSECOND              2
#define DLIDTHIRD               3
#define DLIDABORT               100
#define DLIDCANCEL              101
#define DLIDRETRY               102
#define DLIDNO                  103
#define DLIDYES                 104
/*--end--*/

/*
 *  BS_3STATE
 *      Creates a button that is the same as a check box, except that the box
 *      can be grayed as well as checked or cleared. Use the grayed state to
 *      show that the state of the check box is not determined.
 *
 *  BS_AUTO3STATE
 *      Creates a button that is the same as a three-state check box, except
 *      that the box changes its state when the user selects it. The state
 *      cycles through checked, grayed, and cleared.
 *
 *  BS_AUTOCHECKBOX
 *      Creates a button that is the same as a check box, except that the
 *      check state automatically toggles between checked and cleared each
 *      time the user selects the check box.
 */

    /* Helper macros */

#define DIALOGARG16(l, h)       ((WIDGETARG_t)(((uint8_t)(l))  | ((uint16_t)((uint8_t)(h)))  << 8))
#define DIALOGARG32(l, h)       ((WIDGETARG_t)(((uint16_t)(l)) | ((uint32_t)((uint16_t)(h))) << 16))
#define DIALOGARGLO(l)          ((uint16_t)(l))
#define DIALOGARGHI(l)          ((uint16_t)(((uint32_t)(l) >> 16) & 0xFFFF))

    /* Widget messages */

    /*  The INIT message is sent to a widget immediately before a dialog box
     *  is displayed. Dialog box procedures typically use this message to
     *  initialise controls and carry out any other initialixation tasks that
     *  affect the appearance of the dialog box.
     */
#define WIDGET_INIT             1

#define WIDGET_READY            2

    /*  The DESTROY message is sent when a widget is being destroyed. It is
     *  sent to the widget handler being destroyed after the dialog
     *  has been removed from the screen.
     */
#define WIDGET_DESTROY          3

    /*  The SET message is sent when a user has request a widget
     *  attribute to be set to a specific value.
     */
#define WIDGET_SET              4

    /*  The GET message is sent when a user has request a widget
     *  attribute to be retrieved from a specific value.
     */
#define WIDGET_GET              5

    /*  BASE for implementation specific messages
     */
#define WIDGET_BASE             10

    /* WIDGET construct */

#define WIDGET_MAXDEPTH         8               /* Max allowed container depth */

typedef int32_t WIDGETMSG_t;                    /* Message */

#if (SIZEOF_VOID_P == 8)
typedef int64_t WIDGETARG_t;                    /* arguments, must be >=int32 and >= sizeof(void *p) */
#elif (SIZEOF_VOID_P <= 4)
typedef int32_t WIDGETARG_t;
#else
#error Cannot size WIDGETARG_t ...
#endif

typedef TAILQ_HEAD(WidgetList, _widget)
                        WidgetList_t;           /* Widget list */

typedef CIRCLEQ_HEAD(WidgetQueue, _widget)
                        WidgetQueue_t;          /* Widget queue */

typedef TAILQ_HEAD(DialogList, _dialog)
                        DialogList_t;           /* Dialog list */

struct _widget;                                 /* callback */

typedef uint32_t (* WIDGETCB_t)(struct _widget *, WIDGETMSG_t, WIDGETARG_t, WIDGETARG_t);

typedef struct _widget {
    MAGIC_t             w_magic;                /* Structure magic */
#define WIDGET_MAGIC            MKMAGIC('D','l','G','w')

    const char *        w_desc;                 /* Description (internal) */
    uint16_t            w_class;                /* Widget class */
    int                 w_x, w_y;               /* Top left corner, within frame. */
    int                 w_absx, w_absy;         /* Top left corner, within buffer. */
    int                 w_cols, w_rows;         /* Columns and rows */
    int32_t             w_ident;                /* Number of the widget */
    WIDGETCB_t          w_handler;              /* Infrastructure callback handler */

    int                 w_reqcols, w_reqrows;   /* Required columns and rows */
    int                 w_attach;               /* Attach to frame side (TOP/BOTTOM/LEFT/RIGHT) */
    int                 w_align;                /* Anchor within frame */
    int                 w_padx;                 /* Outer-padding */
    int                 w_pady;
    int                 w_ipadx;                /* Inter-padding */
    int                 w_ipady;
    int                 w_border;               /* Border size */
    int                 w_orientation;
    int32_t             w_hotkey;
    uint32_t            w_flags;

#define WIDGET_FDEFFOCUS        0x00000001
#define WIDGET_FGREYED          0x00000002
#define WIDGET_FHIDDEN          0x00000004

#define WIDGET_FKEYDOWN         0x00000010
#define WIDGET_FKEYPARENT       0x00000020

#define WIDGET_FTABSTOP         0x00000100
#define WIDGET_FTABNOT          0x00000200
#define WIDGET_FAUTOMOVE        0x00000400      /* Auto-move to selection/change */

#define WIDGET_FRESIZE          0x00100000
#define WIDGET_FEXPAND          0x00200000
#define WIDGET_FFILLX           0x00400000
#define WIDGET_FFILLY           0x00800000
#define WIDGET_FDONTPROPAGATE   0x01000000

    const char *        w_name;                 /* Widget name */
    const char *        w_callback;             /* Widget specific callback */
    const char *        w_help;                 /* Help context/topic */
    const char *        w_accelerator;          /* Accelerator key */
    const char *        w_tooltip;              /* Tool-tip text */

    struct _dialog     *w_root;                 /* Owning dialog box */
    void *              w_frame;                /* Owning frame */
    CIRCLEQ_ENTRY(_widget)
                        w_node;                 /* Linear queue of widgets */

    int                 w_depth;                /* Tree depth */
    struct _widget     *w_parent;               /* Parent, NULL if dialog */
    TAILQ_ENTRY(_widget)
                        w_sibling;              /* Siblings within same container */
    WidgetList_t        w_children;             /* List of children (containers only) */
    uint32_t            w_groupid;              /* Group identifier */
    uint32_t            w_containerid;          /* Container identifier */

    void               *w_ucontrol;             /* Fields for use by controller */
    uint32_t            w_uflags;               /* User flags, widget specific */
} WIDGET_t;

typedef struct {
    enum {D_ERROR = -1, D_NONE, D_INT, D_FLOAT, D_LIST, D_STR, D_NULL}
                        d_type;                 /* Type */
    union {
        accint_t        ivalue;                 /* Integer */
        accfloat_t      fvalue;                 /* Float */
        const LIST     *lvalue;                 /* LIST */
        const char     *svalue;                 /* String */
    } d_u;
} WIDGETDATA_t;

    /* DIALOG construct */

typedef struct _dialog {
    WIDGET_t            d_widget;
#define d_x             d_widget.w_x
#define d_y             d_widget.w_y
#define d_cols          d_widget.w_cols
#define d_rows          d_widget.w_rows
#define d_flags         d_widget.w_flags
#define d_callback      d_widget.w_callback
#define d_name          d_widget.w_name
#define d_help          d_widget.w_help

    int                 d_retval;               /* Last return from dialog_run() */
    const char *        d_title;                /* Title */
    int                 d_running;              /* Are we running? */
    int                 d_delete;               /* Have we been deleted? */
    int                 d_type;                 /* Force type TEXT or GUI */

    int32_t             d_ident;                /* Object identifier (handle) */
    uint32_t            d_widgetseq;            /* Widget sequence */

    TAILQ_ENTRY(_dialog)                        /* Dialog queue */
                        d_node;
    WidgetQueue_t       d_widgetq;              /* Widget queue */

#define DIALOG_UUPDATE          0x01            /* vtupdate request */
#define DIALOG_UCALLBACK        0x02            /* Callback - pre(p1=0)/post(p1=1) */
    int               (*d_controller)(struct _dialog *, unsigned op, int p1, ...);

    void               *d_ucontrol;             /* Fields for use by controller */
    uint32_t            d_uflags;
} DIALOG_t;

typedef int (*DIALOGSELECTOR_t)(WIDGET_t *, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);

extern void                 dialog_shutdown(void);

extern DIALOG_t *           dialog_find(int ident);
extern uint32_t             dialog_send(DIALOG_t *d, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);
extern void                 dialog_bcast(DIALOG_t *d, WIDGET_t *first, DIALOGSELECTOR_t selector,
                                            int reverse, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);
extern int                  dialog_focus(DIALOG_t *d);
extern int                  dialog_callback(DIALOG_t *d, int event, accint_t val1, accint_t val2);
extern int                  dialog_callback2(DIALOG_t *d, int event, accint_t val1, accint_t val2, int *rval);
extern void                 dialog_update(DIALOG_t *d, WINDOW_t *wp);
extern uint32_t             dialog_default(WIDGET_t *w, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);
extern int                  dialog_count(DIALOG_t *d);
extern WIDGET_t *           dialog_next(DIALOG_t *d, WIDGET_t *w);
extern WIDGET_t *           dialog_prev(DIALOG_t *d, WIDGET_t *w);

extern WIDGET_t *           widget_byname(DIALOG_t *d, const char *name);
extern WIDGET_t *           widget_byident(DIALOG_t *d, int ident);

extern WIDGET_t *           widget_new(uint32_t size, WIDGETCB_t handler);
extern void                 widget_init(WIDGET_t *w, uint32_t size, WIDGETCB_t handler);
extern void                 widget_destroy(WIDGET_t *w);
extern uint32_t             widget_send(WIDGET_t *w, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);
extern uint32_t             widget_default(WIDGET_t *w, WIDGETMSG_t msg, WIDGETARG_t p1, WIDGETARG_t p2);
extern int                  widget_callback(WIDGET_t *w, int event, accint_t val1, accint_t val2);
extern int                  widget_callback2(WIDGET_t *w, int event, accint_t val1, accint_t val2, int *rval);

extern int32                widgetdata_int32(const WIDGETDATA_t *data);
extern uint32_t             widgetdata_uint32(const WIDGETDATA_t *data);
extern const char *         widgetdata_str(const WIDGETDATA_t *data);
extern const char *         widgetdata_get(const WIDGETDATA_t *data, char *buf, int len);

extern void                 do_create_notice(void);
extern void                 do_dialog_create(void);
extern void                 do_dialog_stock(void);
extern void                 do_dialog_run(void);
extern void                 do_dialog_exit(void);
extern void                 do_dialog_delete(void);
extern void                 do_widget_set(void);
extern void                 do_widget_get(void);
extern void                 inq_dialog(void);

#endif /*GR_DIALOG_H_INCLUDED*/
