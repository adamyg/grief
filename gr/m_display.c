#include <edidentifier.h>
__CIDENT_RCSID(gr_m_display_c,"$Id: m_display.c,v 1.29 2021/07/05 15:01:27 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_display.c,v 1.29 2021/07/05 15:01:27 cvsuser Exp $
 * Display primitives.
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
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_display.h"

#include "accum.h"
#include "builtin.h"
#include "debug.h"
#include "display.h"
#include "echo.h"
#include "eval.h"
#include "getkey.h"
#include "lisp.h"
#include "symbol.h"
#include "tty.h"
#include "window.h"

enum override_mode {
    OVERRIDE_CLR        = -1,
    OVERRIDE_NONE       = 0,
    OVERRIDE_SET        = 1
};

static int              flag_decode(const char *who, int mode, const char *spec, uint32_t *value, enum override_mode smode);
static int *            flag_override(const char *name, int length, int *value);
static const struct dcflag *flag_lookup(const char *name, int length);

static const char       SCROLL_COLS[]  = "scroll_cols";
static const char       SCROLL_ROWS[]  = "scroll_rows";
static const char       VISIBLE_COLS[] = "visible_cols";
static const char       VISIBLE_ROWS[] = "visible_rows";
static const char       NUMBER_COLS[]  = "number_cols";

static const struct dcflag {        /* display control flags */
    const char *        f_name;                 /* name/label */
    int                 f_length;
    uint32_t            f_value;                /* flag value */
} dcflagnames[] = {
#define NFIELD(__x)     __x, (sizeof(__x) - 1)
#define DC_READONLYBITS (DC_WINDOW|DC_MOUSE|DC_READONLY|DC_CHARMODE|DC_UNICODE)

    { NFIELD("window"),             DC_WINDOW },            /* Running under a windowing system (read-only). */
    { NFIELD("mouse"),              DC_MOUSE },             /* Mouse enabled/available (read-only). */
    { NFIELD("readonly"),           DC_READONLY },          /* Read-only mode (read-only). */
    { NFIELD("charmode"),           DC_CHARMODE },          /* Character-mode with basic GUI features (read-only). */

    { NFIELD("shadow"),             DC_SHADOW },            /* Display shadow around popups. */
    { NFIELD("showthru"),           DC_SHADOW_SHOWTHRU },   /* Show-thru shadow around popups. */
    { NFIELD("statusline"),         DC_STATUSLINE },

    { NFIELD("unicode"),            DC_UNICODE },           /* UNICODE character encoding available (read-only). */
    { NFIELD("asciionly"),          DC_ASCIIONLY },         /* ASCII only characters witihin UI/dialogs. */

    { NFIELD("rosuffix"),           DC_ROSUFFIX },          /* Read-only suffix on titles. */
    { NFIELD("modsuffix"),          DC_MODSUFFIX },         /* Modified suffix. */

    { NFIELD("eof_display"),        DC_EOF_DISPLAY },       /* Show <EOF> marker. */
    { NFIELD("tilde_display"),      DC_TILDE_DISPLAY },     /* Show <~> marker. */
    { NFIELD("eol_hilite"),         DC_EOL_HILITE },        /* Limit hilites to EOL. */

    { NFIELD("himodified"),         DC_HIMODIFIED },        /* Hilite modified lines. */
    { NFIELD("hiadditional"),       DC_HIADDITIONAL },      /* Hilite additional lines. */
#undef  NFIELD
    };

static const struct doflag {        /* display overrides */
    const char *        f_name;                 /* name/label */
    int                 f_length;
    int *               f_value;                /* flag value */
} doflagnames[] = {
#define OFIELD(__x)     __x, (sizeof(__x) - 1)
    { OFIELD(SCROLL_COLS),          &x_display_scrollcols },
    { OFIELD(SCROLL_ROWS),          &x_display_scrollrows },
    { OFIELD(VISIBLE_COLS),         &x_display_mincols    },
    { OFIELD(VISIBLE_ROWS),         &x_display_minrows    },
    { OFIELD(NUMBER_COLS),          &x_display_numbercols }
#undef  OFIELD
    };


/*  Function:           do_display_mode
 *      display_mode primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: display_mode - Set/retrieve display control flags.

        int
        display_mode(
            [int or_mask|string set-list], [int and_mask|string clear-list],
                [int scroll_cols], [int scroll_rows],
                [int visible_cols], [int visible_rows],
                [int number_cols])

    Macro Description:
        The 'display_mode()' primitive controls primary features of the
        display interface. Features are either stated using their
        manifest constant or in the case of values an explicit parameters

        If specified one or more flags shall be cleared using the
        'and_mask', in additional one or more flags are set using the
        'or_mask'.

(start table,format=basic)
    [Constant           [Name           [Description                               ]
    DC_WINDOW           window          Running under a windowing system (ro).
    DC_MOUSE            mouse           Mouse enabled/available (ro).
    DC_READONLY         readonly        Read-only mode (ro).
    DC_CHARMODE         charmode        Character-mode with basic GUI features (ro).
    DC_SHADOW           shadow          Display shadow around popups.
    DC_SHADOW_SHOWTHRU  showthru        Show-thru shadow around popups.
    DC_STATUSLINE       statusline      Status line.
    DC_UNICODE          unicode         UNICODE character encoding available (ro).
    DC_ASCIIONLY        asciionly       ASCII only characters witihin UI/dialogs.
    DC_ROSUFFIX         rosuffix        Read-only suffix on titles.
    DC_MODSUFFIX        modsuffix       Modified suffix.
    DC_EOF_DISPLAY      eof_display     Show <EOF> marker.
    DC_TILDE_DISPLAY    tilde_display   Show <~> marker.
    DC_EOL_HILITE       eol_hilite      Limit hilites to EOL.
    DC_HIMODIFIED       himodified      Hilite modified lines.
    DC_HIADDITIONAL     hiadditional    Hilite additional lines.
(end table)

        Note!:
        Items marked as (ro) are Read-Only with any specified
        changes against the attribute shall be quietly ignored.

        The optional parameters 'scroll_cols' and 'scroll_rows' define
        the screen distance each scroll operation shall employ. Values
        greater than one result in scroll jumps, which may be desired on
        slower terminals.

        The optional parameters 'visual_cols' and 'visual_rows' define
        the smallest display arena which shall be permitted, with
        'number_cols' defining the number line arena width.

    Macro Parameters:
        set_mask - Optional mask of flags to set. May either be an
            integer of AND'ed together flag constants, or alternatively
            a string of comma separated flag names.

        clear_mask - Optional mask of flags to clear. May either be an
            integer of AND'ed together flag constants, or alternatively
            a string of comma separated flag names.

        scroll_cols - Optional integer value, if stated sets the number
            of screen columns scroll operations shall employ. A value of
            zero shall clear the scroll override, defaulting the value
            to 1. Upon a negative value, the current override shall be
            returned.

        scroll_rows - Optional integer value, if stated sets the number
            of screen rows scroll operations shall employ. A value of
            zero shall clear the scroll override, defaulting the value
            to 1. Upon a negative value, the current override shall be
            returned.

        visible_cols - Optional integer value, if stated sets the lower
            bounds of the display arena width. Upon a negative value,
            the current override shall be returned.

        visible_rows - Optional integer value, if stated sets the lower
            bounds of the display arena height. Upon a negative value,
            the current override shall be returned.

        number_cols - Optional integer value, if stated as a positive
            integer sets the width of the number-line column within
            windows, disabling dynamic width selection. A value of zero
            shall clear the width override, enabling the default dynamic
            width based upon the buffer length. Upon a negative value,
            the current override shall be returned.

    Macro Returns:
        The 'display_mode()' primitive by default returns the previous
        value of the display control flags prior to any modifications.

        If one of the integer parameters is a negative value (e.g. -1)
        then 'display_mode' returns the current value of the associated
        parameter. If more than one is negative, then the value of last
        shall be returned.

    Macro Portability:
        The string mask variants and 'set' parameter are GRIEF extensions.

        Many of the flags are GRIEF specific; CRiSPEdit has a similar
        primitive yet as the two were developed independently features
        differ.

    Macro See Also:
        inq_display_mode, set_font, inq_font
 */
void
do_display_mode(void)           /* int ([int or_mask|string clear-list], [int and_mask|string set-list],
                                            [int scroll_cols], [int scroll_rows], [int visible_cols], [int visible_rows],
                                                [int number_cols]) */
{
    static const char who[] = "display_mode";
    accint_t ret = x_display_ctrl;

    /* and/clear */
    if (isa_string(2)) {                        /* extension */
        uint32_t value = 0;

        if (flag_decode(who, 1, get_xstr(2), &value, OVERRIDE_CLR) > 0) {
            x_display_ctrl &= (value | DC_READONLYBITS);
        }
    } else if (isa_integer(2)) {
        x_display_ctrl &= get_xinteger(2, 0) | DC_READONLYBITS;
    }

    /* or/set */
    if (isa_string(1)) {                        /* extension */
        uint32_t value = 0;

        if (flag_decode(who, 0, get_xstr(1), &value, OVERRIDE_SET) > 0) {
            x_display_ctrl |= (value & ~DC_READONLYBITS);
        }
    } else if (isa_integer(1)) {
        x_display_ctrl |= (get_xinteger(1, 0) & ~DC_READONLYBITS);
    }

    /* scroll_cols */
    if (isa_integer(3)) {
        const int value = get_xinteger(3, 0);
        if (value < 0) {                        /* .. return current (<0) */
            ret = x_display_scrollcols;
        } else {                                /* .. otherwise set */
            x_display_scrollcols = value;
        }
    }

    /* scroll_rows */
    if (isa_integer(4)) {
        const int value = get_xinteger(4, 0);
        if (value < 0) {                        /* .. return current (<0) */
            ret = x_display_scrollrows;
        } else {                                /* .. otherwise set */
            x_display_scrollrows = value;
        }
    }

    /* visible_cols */
    if (isa_integer(5)) {
        const int value = get_xinteger(5, 0);
        if (value < 0) {                        /* .. return current (<0) */
            ret = x_display_mincols;
        } else {                                /* .. otherwise set */
            x_display_mincols = value;
        }
    }

    /* visible_rows */
    if (isa_integer(6)) {
        const int value = get_xinteger(6, 0);
        if (value < 0) {                        /* .. return current (<0) */
            ret = x_display_minrows;
        } else {                                /* .. otherwise set */
            x_display_minrows = value;
        }
    }

    /* number_cols */
    if (isa_integer(7)) {
        const int value = get_xinteger(7, 0);
        if (value < 0) {                        /* .. return current (<0) */
            ret = x_display_numbercols;
        } else {                                /* .. otherwise set */
            x_display_numbercols = value;
        }
    }

    window_harden();
    acc_assign_int(ret);
}


/*  Function:           inq_display_mode
 *      inq_display_mode primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_display_mode - Inquire display control flags.

        int
        inq_display_mode([string flagname], [string ~flags])

    Macro Description:
        The 'inq_display_mode()' primitive retrieves the value of the
        associated display attribute, given by the parameter 'flagname'.

(start table,format=basic)
    [Name           [Constant           [Description                                ]
    window          DC_WINDOW           Running under a windowing system (ro).
    mouse           DC_MOUSE            Mouse enabled/available (ro).
    readonly        DC_READONLY         Read-only mode (ro).
    charmode        DC_CHARMODE         Character-mode with basic GUI features (ro).
    shadow          DC_SHADOW           Display shadow around popups.
    showthru        DC_SHADOW_SHOWTHRU  Show-thru shadow around popups.
    statusline      DC_STATUSLINE       Status line.
    unicode         DC_UNICODE          UNICODE character encoding available (ro).
    asciionly       DC_ASCIIONLY        ASCII only characters witihin UI/dialogs.
    rosuffix        DC_ROSUFFIX         Read-only suffix on titles.
    modsuffix       DC_MODSUFFIX        Modified suffix.
    eof_display     DC_EOF_DISPLAY      Show <EOF> marker.
    tilde_display   DC_TILDE_DISPLAY    Show <~> marker.
    eol_hilite      DC_EOL_HILITE       Limit hilites to EOL.
    himodified      DC_HIMODIFIED       Hilite modified lines.
    hiadditional    DC_HIADDITIONAL     Hilite additional lines.
    scroll_cols     n/a                 Scroll jump column override.
    scroll_rows     n/a                 Scroll Jump row override.
    visible_cols    n/a                 Visible display window width lower bounds.
    visible_rows    n/a                 Display window height lower bounds.
    number_cols     n/a                 Line number column width override.
(end table)

    Macro Parameter:
        flagname - Optional string parameter, if stated gives the name of
            the attribute to be retrieved (see display_mode). If omitted
            the display mode are retrieved.

        flags - Optional string parameter, if stated shall be populated
            with a comma separated list of active attribute names.

    Macro Returns:
        The 'inq_display_mode()' primitive returns the value of the
        associated attribute, otherwise -1 on error.

    Macro Portability:
        Grief extended.

        For system portability use of the manifest constants is
        advised.

    Macro See Also:
        display_mode, set_font, inq_font
 */
void
inq_display_mode(void)          /* int ([string flag], [string ~flags]) */
{
    static const char who[] = "inq_display_mode";
    accint_t ret = x_display_ctrl;

    if (isa_string(1)) {                        /* by name */
        const char *str = get_str(1);
        int *override;

        if (NULL != (override = flag_override(str, strlen(str), NULL))) {
            ret = *override;
        } else {
            uint32_t value = 0;

            if (flag_decode(who, 0, str, &value, OVERRIDE_NONE) > 0) {
                if (value) {
                    ret = (x_display_ctrl & value);
                }
            }
        }
    }

    if (isa_string(2)) {                        /* export specification */
        const unsigned DXSIZE = (16 * (sizeof(dcflagnames)/sizeof(dcflagnames[0]))) +
                                    (16 * (sizeof(doflagnames)/sizeof(doflagnames[0])));
        char *flags;

        if (NULL != (flags = (char *)chk_alloc(DXSIZE))) {
            const struct dcflag *dcflag = dcflagnames;
            const struct doflag *doflag = doflagnames;
            const uint32_t value = x_display_ctrl;
            unsigned i, idx = 0;

                                                /* control flags yes|no */
            for (i = 0; i < (unsigned)(sizeof(dcflagnames)/sizeof(dcflagnames[0])); ++i, ++dcflag) {
                idx += sxprintf(flags + idx, DXSIZE - idx, "%s%s=%s", (i ? "," :""),
                            dcflag->f_name, (value & dcflag->f_value ? "yes" : "no"));
            }
                                                /* override values */
            for (i = 0; i < (unsigned)(sizeof(doflagnames)/sizeof(doflagnames[0])); ++i, ++doflag) {
                idx += sxprintf(flags + idx, DXSIZE - idx, "%s%s=%d", ",",
                            doflag->f_name, *doflag->f_value);
            }

            argv_assign_str(2, (const char *)flags);
            chk_free((void *)flags);
        }
    }

    acc_assign_int(ret);
}


static int
flag_decode(const char *who, int mode, const char *spec, uint32_t *value, enum override_mode omode)
{
    const char *comma, *cursor = spec;
    uint32_t nvalue = 0;
    const struct dcflag *flag;
    int flags = 0;

    while (NULL != (comma = strchr(cursor, ',')) || *cursor) {
        const size_t length =                   /* <value>[,<value>] */
            (NULL == comma ? strlen(cursor) : (size_t)(comma - cursor));

        if (length) {                           /* ignore empty (,,) */
            int done = FALSE;

            if (NULL != (flag = flag_lookup(cursor, length))) {
                nvalue |= flag->f_value;
                done = TRUE;

            } else if (OVERRIDE_NONE != omode) {
                const int set = (OVERRIDE_SET == omode);
                int ovvalue = 0, *override;

                if (NULL != (override = flag_override(cursor, length, (set ? &ovvalue : NULL)))) {
                    *override =                 /* override[=0] */
                        (set && ovvalue > 0 ? ovvalue : 0);
                    done = TRUE;
                }
            }

            if (!done) {
                if (comma)  {
                    errorf("%s: unknown flag '%*s'.", who, (int)(comma - spec), spec);
                } else {
                    errorf("%s: unknown flag '%s'.", who, spec);
                }
                return -1;
            }

            ++flags;                            /* flag count */
        }

        if (NULL == comma) break;
        cursor = comma + 1;
    }

    *value = (1 == mode ? ~nvalue : nvalue);
    trace_log(" == flags=0x%x [ 0x%08x ]\n", flags, *value);
    return flags;
}

static const struct dcflag *
flag_lookup(const char *name, int length)
{
    if (NULL != (name = str_trim(name, &length)) && length > 0) {
        unsigned i;

        trace_ilog("\t %*s\n", length, name);
        for (i = 0; i < (unsigned)(sizeof(dcflagnames)/sizeof(dcflagnames[0])); ++i)
            if (length == dcflagnames[i].f_length &&
                    0 == str_nicmp(dcflagnames[i].f_name, name, length)) {
                return dcflagnames + i;
            }
    }
    return NULL;
}

static int *
flag_override(const char *name, int length, int *value)
{
    const char *eq = NULL;

    if (value) {                                /* override=value */
        if (NULL == (eq = strchr(name, '=')) ||
                eq > (name + length)) {
            return NULL;
        }
        *value = atoi(eq + 1);
    }

    {   const int namelength = (eq ? (int)(eq - name) : length);
        unsigned i;

        trace_ilog(value ? "\t %*s=%d\n" : "\t %*s\n", namelength, name, (value ? *value : -1));
        for (i = 0; i < (unsigned)(sizeof(doflagnames)/sizeof(doflagnames[0])); ++i)
            if (namelength == doflagnames[i].f_length &&
                    0 == str_nicmp(doflagnames[i].f_name, name, namelength)) {
                return doflagnames[i].f_value;
            }
    }
    return NULL;
}


/*  Function:           inq_screen_size
 *      inq_screen_size primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
     Macro: inq_screen_size - Determine screen dimensions

        int
        inq_screen_size([int &rows], [int &cols], [int &colors])

    Macro Description:
        The 'inq_screen_size()' primitive retrieves the screen dimensions,
        being the number of text rows and character columns.

    Macro Parameters:
        rows - Optional integer reference populated with the number
            of text rows.

        cols - Optional integer reference to be populated with the
            number of character columns.

        colors - Optional integer reference populated with the color
            depth supported by the display.

    Macro Returns:
        The 'inq_screen_size()' primitive returns zero if terminal is
        configured as a monochrome screen, or non-zero if in color
        mode.

    Macro Portability:
        Grief extended.

    Macro See Also:
        display_mode
 */
void
inq_screen_size(void)           /* int ([int &rows], [int &cols], [int &colors]) */
{
    argv_assign_int(1, (accint_t)ttrows());
    argv_assign_int(2, (accint_t)ttcols());
    argv_assign_int(3, (accint_t)vtcolordepth());   /*extension*/
    acc_assign_int((accint_t) vtiscolor());
}


/*  Function:           do_cursor
 *      cursor primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: cursor - Control cursor display.

        int
        cursor(int state)

    Macro Description:
        The 'cursor()' primitive controls whether the cursor is
        visible, by default the cursor is enabled.

        The cursor visibility is set to 'state' with a non-zero
        value enabling and a zero value disabling, if omitted then
        the current value is toggled.

    Macro Parameters:
        state - Optional boolean cursor status, if omitted the current
            status is toggled.

    Macro Returns:
        The 'cursor()' primitive returns 0 on success, otherwise
        non-zero.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        inq_screen_size, borders
 */
void
do_cursor(void)
{
    acc_assign_int(ttcursor(get_xinteger(1, -1), -1, -1));
}


/*  Function:           do_set_wm_name
 *      set_wm_name primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_wm_name - Set the window and/or icon name.

        void
        set_wm_name([string wname], [string iname])

    Macro Description:
        The 'set_wm_name()' primitive configures the window and/or
        the minimised icon name of the current running Grief image.

        Note!:
        Only available when running under a suitable windowing
        system, otherwise this primitive is a no-op.

    Macro Parameterss:
        wname - Optional string containing the window name.
        iname - Optional icon name.

    Macro Returns:
        The 'set_wm_name()' primitive returns zero or greater on
        success, otherwise -1 on error.

    Macro Portability:
        Grief extended.

    Macro See Also:
        set_font
 */
void
do_set_wm_name(void)            /* void ([string wname], [string iname]) */
{
    const char *wname = get_xstr(1);
    const char *iname = get_xstr(2);

    if (x_scrfn.scr_names) {
        (*x_scrfn.scr_names)(wname, iname);
    }
}


/*  Function:           do_set_font
 *      set_font primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_font - Set the current window fonts.

        int
        set_font([string normalfont], [string italicfont])

    Macro Description:
        The 'set_font()' primitive configures the active normal
        and/or italic font of the current running Grief image.

        Note!:
        Only available when running under a suitable windowing
        system, otherwise this primitive is a no-op.

    Macro Parameters:
        normalfont - Optional string containing the normal text font.
        italicfont - Optional italic font.

    Macro Returns:
        The 'set_font()' primitive returns zero or greater on success,
        otherwise -1 on error.

    Macro Portability:
        Grief extended.

    Macro See Also:
        inq_font, set_wm_name
 */
void
do_set_font(void)               /* int (string normalfont, [string italicfont]) */
{
    const char *norm = get_xstr(1);
//  const char *italic = get_xstr(2);           /* TODO */        
    int ret = -1;

    if (norm && x_scrfn.scr_font) {
        ret = (*x_scrfn.scr_font)(-1, (char *)norm /*TODO, (char *)italic*/);
    }
    acc_assign_int((accint_t)ret);
}


/*  Function:           do_inq_font
 *      inq_font primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_font - Inquire the current window fonts.

        int
        inq_font(string &normalfont, [string &italicfont])

    Macro Description:
        The 'inq_font()' primitive retrieves the active normal and/or
        italic font of the current running Grief image.

        Note!:
        Only available when running under a suitable windowing
        system, otherwise this primitive is a no-op.

    Macro Parameters:
        normalfont - Optional string variable reference to be
            populated with the normal text font name.

        italicfont - Optional string variable reference to be
            populated with the italic text font name.

    Macro Returns:
        The 'inq_font()' primitive returns zero or greater on success,
        otherwise -1 on error.

    Macro Portability:
        Grief extended.

    Macro See Also:
        set_font
 */
void
do_inq_font(void)               /* int (string &normalfont, [string &italicfont]) */
{
    int ret = -1;

    if (x_scrfn.scr_font) {
        char norm[128] = {0}, italic[128]={0};  /* MAGIC */

        ret = (*x_scrfn.scr_font)(sizeof(norm), norm /*TODO, sizeof(italic), italic*/);
        argv_assign_str(1, (const char *)norm);
        argv_assign_str(2, (const char *)italic);
    }
    acc_assign_int((accint_t)ret);
}


/*  Function:           do_beep
 *      beep primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: beep - Issue a beep sound.

        int
        beep([int freq], [int duration])

    Macro Description:
        The 'beep()' primitive sends a bell or beep to the screen
        causing an audible sound.

        The function is synchronous; it performs an alertable wait
        and does not return control to its caller until the sound
        finishes.

        'freq' and 'duration' are system dependent values.

    Macro Parameters:
        freq - The frequency of the sound, in hertz. This parameter
            must be in the range 37 through 32,767.

        duration - The duration of the sound, in milliseconds.

    Macro Returns:
        The 'beep()' primitive returns 0 on success, otherwise -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        cursor
 */
void
do_beep(void)                   /* ([int freq], [int duration]) */
{
    const int freq = get_xinteger(1, 0);
    const int duration = get_xinteger(2, 0);
    int ret = -1;

    if (x_scrfn.scr_beep) {
        (*x_scrfn.scr_beep)(freq, duration);
        ret = 0;
    }
    acc_assign_int(ret);
}


/*  Function:           do_view_screen
 *      view_screen primitive
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: view_screen - View the content of underlying screen.

        int
        view_screen()

    Macro Description:
        The 'view_screen()' primitive restores the original screen
        image visible prior to Grief's execution, returning upon a
        key press.

    Macro Parameters:
        none

    Macro Returns:
        The 'view_screen()' primitive returns zero on success otherwise
        -1 on error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        cursor
 */
void
do_view_screen(void)            /* int () */
{
    int ret = -1;

    if (x_scrfn.scr_control) {
        (*x_scrfn.scr_control)(SCR_CTRL_SAVE, 0);
        (void) io_get_raw(60000L);
        (*x_scrfn.scr_control)(SCR_CTRL_RESTORE, 0);
        vtgarbled();
        vtupdate();
    }
    acc_assign_int(ret);
}

/*end*/
