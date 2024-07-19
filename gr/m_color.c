#include <edidentifier.h>
__CIDENT_RCSID(gr_m_color_c,"$Id: m_color.c,v 1.50 2024/07/11 10:20:05 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_color.c,v 1.50 2024/07/11 10:20:05 cvsuser Exp $
 * Color configuration.
 *
 *
 * Copyright (c) 1998 - 2024, Adam Young.
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

#define ED_ASSERT
#include <editor.h>
#include <edassert.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "m_color.h"                            /* public interface */
#include "color.h"                              /* public interface */

#include "accum.h"                              /* acc_...() */
#include "builtin.h"                            /* margc */
#include "debug.h"                              /* trace_...() */
#include "display.h"                            /* COLORIZER */
#include "echo.h"                               /* ewprintf()/ereply() */
#include "eval.h"                               /* get_...() */
#include "lisp.h"                               /* atom_...() */
#include "main.h"                               /* panic() */
#include "symbol.h"                             /* argv_assign_...() */
#include "ttyrgb.h"                             /* RGB support */
#include "word.h"                               /* LPUT_...() */

#define COLOR_UNKNOWN           -1


typedef struct attributelinks {
#define COLLINKS_INIT           {-1, -1}
    int                 cl_undef;               /* ATTR_..., 0 == ATTR_NORMAL */
    int                 cl_sticky;              /* ATTR_..., 0 if n/a */
} collinks_t;


struct attribute {
    const char *        ca_name;                /* enum name */
    int                 ca_length;              /* .. and length, in bytes */
    const char *        ca_desc;                /* description (optionally) */
    int                 ca_enum;                /* color identifier (external), -1 if n/a */
    unsigned            ca_flags;
#define CA_BG                   0x01            /* background */
#define CA_FG                   0x02            /* foreground */
#define CA_ATTR                 0x04            /* attribute encoding */
#define CA_FULL                 0x10            /* allow both fg + bg (hilite special) */
    int                 ca_attr;                /* ATTR_..., -1 if n/a */
    collinks_t          ca_links;
    colattr_t           ca_value;               /* active value */
    colattr_t           ca_working;             /* working value */
};


typedef struct {
    unsigned            c_magic;                /* structure magic */
    struct attribute *  c_attr_table;           /* table base address */
    unsigned            c_attr_alloc;           /* allocated element number */
    unsigned            c_attr_count;           /* assigned count */
    unsigned            c_attr_dynamic;         /* dynamic attribute count */
    int                 c_isdark;               /* *true* if scheme is dark, otherwise light */
    colattr_t *         c_background;           /* background */
    colattr_t *         c_normal;               /* normalforeground */
    colattr_t *         c_hilite_bg;            /* hilite background */
    colattr_t *         c_hilite_fg;            /* hilite foreground */
} colors_t;


static int                      set_color(colors_t *colors, const char *spec, int create, accint_t *ident);

static colors_t *               col_working(void);
static colors_t *               col_reworked(void);
#define                         col_normal(__colors) \
                                        (__colors->c_normal->fg)
#define                         col_background(__colors) \
                                        (__colors->c_background->fg)

static void                     col_clear(colors_t *colors);
static void                     col_references(colors_t *colors);
static int                      col_apply(colors_t *colors);
static colattr_t                col_build(const colors_t *colors, const struct attribute *ap, const colattr_t *ca);
static colvalue_t               col_dynamic(const colors_t *colors, const colvalue_t val);
static int                      isclear(const colattr_t *ca);

static struct attribute *       attr_byname(const colors_t *colors, const char *name, int create);
static struct attribute *       attr_bynname(const colors_t *colors, const char *name, int length, int create);
static struct attribute *       attr_byenum(const colors_t *colors, int ident);
static const char *             attr_toname(const colors_t *colors, int ident);

static int                      col_prompt(const colors_t *colors, const struct attribute *ap, colattr_t *val);
static colattr_t                col_encode(const colors_t *colors, const struct attribute *ap, const colattr_t *ca);
static colvalue_t               col_static(const colors_t *colors, const colvalue_t *val);
static int                      col_import(const colors_t *colors, const char *who, const struct attribute *ap, const char *spec, colattr_t *ca, collinks_t *links);
static int                      col_export(const colors_t *colors, const struct attribute *ap, const colattr_t *ca, char *buf, int len);

static int                      color_value(const char *name, colvalue_t *val, int bg);
static int                      color_nvalue(const char *name, int length, colvalue_t *val, int bg);
static const char *             color_label(const char *name, const int length_or_color, colvalue_t *result);
static colvalue_t               color_numeric(const char *name, unsigned length);
static int                      color_print(const colvalue_t *val, char *buf, int len);

static int                      style_import(const colors_t *colors, const char *who, const char *spec, collinks_t *links, const int attr);
static int                      style_nvalue(const char *name, int length);
static int                      style_print(int sf, char *buf, int length);

#define COLOR_NAMES             ((unsigned)(sizeof(x_color_names)/sizeof(x_color_names[0])))

static const struct name {
    const char *        cn_name;                /* name */
    int                 cn_length;              /* length of name in bytes */
    int                 cn_value;               /* bitmap value */

} x_color_names[] = {
#define NFIELD(__x)     __x, (sizeof(__x) - 1)
            /*
             *  crisp-style                             alternative name      ****
             */
    /*0 */  { NFIELD("black"),          BLACK },
    /*1 */  { NFIELD("blue"),           BLUE },
    /*2 */  { NFIELD("green"),          GREEN },
    /*3 */  { NFIELD("cyan"),           CYAN },
    /*4 */  { NFIELD("red"),            RED },
    /*5 */  { NFIELD("magenta"),        MAGENTA },
    /*6 */  { NFIELD("brown"),          BROWN },
    /*7 */  { NFIELD("white"),          WHITE },
    /*8 */  { NFIELD("grey"),           GREY },         { NFIELD("gray"),           GREY },
    /*9 */  { NFIELD("light-blue"),     LTBLUE },       { NFIELD("LightBlue"),      LTBLUE },
    /*10*/  { NFIELD("light-green"),    LTGREEN },      { NFIELD("LightGreen"),     LTGREEN },
    /*11*/  { NFIELD("light-cyan"),     LTCYAN },       { NFIELD("LightCyan"),      LTCYAN },
    /*12*/  { NFIELD("light-red"),      LTRED },        { NFIELD("LightRed"),       LTRED },
    /*13*/  { NFIELD("light-magenta"),  LTMAGENTA },    { NFIELD("LightMagenta"),   LTMAGENTA },
    /*14*/  { NFIELD("yellow"),         YELLOW },
    /*15*/  { NFIELD("light-white"),    LTWHITE },      { NFIELD("LightWhite"),     LTWHITE },
            { NFIELD("bright-white"),   LTWHITE },
            { NFIELD("light-grey"),     LTWHITE },      { NFIELD("LightGrey"),      LTWHITE },
            { NFIELD("light-gray"),     LTWHITE },      { NFIELD("LightGray"),      LTWHITE },
    /*16*/  { NFIELD("dark-grey"),      DKGREY },       { NFIELD("DarkGrey"),       DKGREY },
            { NFIELD("dark-gray"),      DKGREY },       { NFIELD("DarkGray"),       DKGREY },
    /*17*/  { NFIELD("dark-blue"),      DKBLUE },       { NFIELD("DarkBlue"),       DKBLUE },
    /*18*/  { NFIELD("dark-green"),     DKGREEN },      { NFIELD("DarkGreen"),      DKGREEN },
    /*19*/  { NFIELD("dark-cyan"),      DKCYAN },       { NFIELD("Darkcyan"),       DKCYAN },
    /*20*/  { NFIELD("dark-red"),       DKRED },        { NFIELD("DarkRed"),        DKRED },
    /*21*/  { NFIELD("dark-magenta"),   DKMAGENTA },    { NFIELD("DarkMagenta"),    DKMAGENTA },
    /*22*/  { NFIELD("dark-yellow"),    DKYELLOW },     { NFIELD("DarkYellow"),     DKYELLOW },
    /*23*/  { NFIELD("light-yellow"),   LTYELLOW },     { NFIELD("LightYellow"),    LTYELLOW },

            /*
             *  Specials:
             *      none -              transparent/default-color.
             *      dynamic-fg -        dynamic normal.
             *      dynamic-bg -        dynamic background.
             *      fg/foreground -     current normal.
             *      bg/background -     current background.
             */
            { NFIELD("none"),           COLOR_NONE },
            { NFIELD("dynamic-fg"),     COLOUR_DYNAMIC_FOREGROUND },
            { NFIELD("dynamic-bg"),     COLOUR_DYNAMIC_BACKGROUND },
            { NFIELD("fg"),             COLOUR_FOREGROUND },
            { NFIELD("foreground"),     COLOUR_FOREGROUND },
            { NFIELD("bg"),             COLOUR_BACKGROUND },
            { NFIELD("background"),     COLOUR_BACKGROUND },

#undef  NFIELD
    };


static const struct {
    const char *        sn_name;                /* name */
    int                 sn_length;              /* length of name in bytes */
    int                 sn_value;               /* bitmap value */

} x_style_names[] = {
#define NFIELD(__x)     __x, (sizeof(__x) - 1)

    { NFIELD("bold"),           COLORSTYLE_BOLD },
    { NFIELD("inverse"),        COLORSTYLE_INVERSE },
    { NFIELD("underline"),      COLORSTYLE_UNDERLINE },
    { NFIELD("blink"),          COLORSTYLE_BLINK },
    { NFIELD("italic"),         COLORSTYLE_ITALIC },
    { NFIELD("reverse"),        COLORSTYLE_REVERSE },       /* generally same as INVERSE */
    { NFIELD("standout"),       COLORSTYLE_STANDOUT },
    { NFIELD("dim"),            COLORSTYLE_DIM },
    { NFIELD("undercurl"),      COLORSTYLE_UNDERLINE },     /* undercurl is a curly underline, generally underline */

    { NFIELD("bold"),           COLORSTYLE_ISBOLD },        /* BOLD, yet applied */
    { NFIELD("dim"),            COLORSTYLE_ISDIM },         /* DIM, yet applied */

    { NFIELD("none"),           0 }

#undef  NFIELD
    };

#define ATTRIBUTES_WIDTH        (80 * 4)        /* attribute=foreground[,background][:link|sticky|styles] */
#define ATTRIBUTES_COUNT        (sizeof(attributes_default)/sizeof(attributes_default[0]))

#define OFFSETOF(x)             offsetof(colors_t, x)


static const struct attribute   attributes_default[] = {
    /*
     *  Normal/standard colors
     *      Note: configuration is a little messy due to the need for compatibility with
     *      the original colour support.
     *
     *      Order of COL_xxx emumerated attributes *must* not change.
     *
     *  Special Cases:
     *      BACKGROUND          Use only first specification.
     *      HILITE              Treat as background, unless full specification.
     *      Basic colors        Ignore background (normal etc).
     *      others              Full color specification.
     *
     *  Name, length, desc              enum,                   flags,          attr,                           link,
     */
#define _A2(_a,_b)      _a, (sizeof(_a)-1), _b
#define _A1(_c)         _A2(_c, NULL)

/* primary attributes */

    { _A1("background"),                COL_BACKGROUND,         CA_BG,          -1,                             0 },

    { _A1("normal"),                    COL_FOREGROUND,         CA_FG,          ATTR_NORMAL,                    0 },
    { _A2("select", "selected title"),  COL_SELECTED_WINDOW,    CA_FG,          ATTR_TITLE,                     0 },
    { _A1("message"),                   COL_MESSAGES,           CA_FG,          ATTR_MESSAGE,                   0 },
    { _A1("error"),                     COL_ERRORS,             CA_FG,          ATTR_ERROR,                     0 },
    { _A1("hilite"),                    COL_HILITE_BACKGROUND,  CA_BG|CA_FULL,  ATTR_HILITE,                    0 },
    { _A1("hilite_fg"),                 COL_HILITE_FOREGROUND,  CA_FG,          -1,                             0 },
    { _A1("frame"),                     COL_BORDERS,            CA_FG,          ATTR_FRAME,                     0 },
//  { _A1("vframe"),                    COL_VFRAME,             CA_FG,          ATTR_FRAME,                     0 },

    { _A1("cursor_insert"),             COL_INSERT_CURSOR,      CA_FG,          ATTR_CURSOR_INSERT,             0 },
    { _A1("cursor_overtype"),           COL_OVERTYPE_CURSOR,    CA_FG,          ATTR_CURSOR_OVERTYPE,           0 },

/* extended attributes */
    /*
     *  UI components
     */

    { _A1("cursor"),                    -1,                     CA_ATTR,        ATTR_CURSOR,                    0 },
    { _A1("cursor_row"),                -1,                     CA_ATTR,        ATTR_CURSOR_ROW,                0 },
    { _A1("cursor_col"),                -1,                     CA_ATTR,        ATTR_CURSOR_COL,                0 },

    { _A1("shadow"),                    COL_SHADOW,             CA_ATTR,        ATTR_SHADOW,                    0 },
    { _A1("prompt"),                    COL_PROMPT,             CA_ATTR,        ATTR_PROMPT,                    ATTR_MESSAGE },
    { _A1("prompt_standout"),           -1,                     CA_ATTR,        ATTR_PROMPT_STANDOUT,           ATTR_PROMPT },
    { _A1("prompt_complete"),           COL_COMPLETION,         CA_ATTR,        ATTR_PROMPT_COMPLETE,           ATTR_HILITE },
    { _A1("question"),                  COL_QUESTION,           CA_ATTR,        ATTR_QUESTION,                  ATTR_MESSAGE },
    { _A1("echo_line"),                 COL_ECHOLINE,           CA_ATTR,        ATTR_ECHOLINE,                  0 },
    { _A1("standout"),                  COL_STANDOUT,           CA_ATTR,        ATTR_STANDOUT,                  ATTR_MESSAGE },
    { _A1("literalchar"),               -1,                     CA_ATTR,        ATTR_HILITERAL,                 ATTR_STANDOUT },
    { _A1("whitespace"),                -1,                     CA_ATTR,        ATTR_WHITESPACE,                ATTR_HILITE },

    { _A1("scrollbar"),                 -1,                     CA_ATTR,        ATTR_SCROLLBAR,                 ATTR_FRAME },
    { _A1("scrollbar_thumb"),           -1,                     CA_ATTR,        ATTR_SCROLLBAR_THUMB,           0 },

    { _A1("column_status"),             -1,                     CA_ATTR,        ATTR_COLUMN_STATUS,             ATTR_STANDOUT },
    { _A1("column_lineno"),             -1,                     CA_ATTR,        ATTR_COLUMN_LINENO,             ATTR_STANDOUT },
    { _A1("nonbuffer"),                 -1,                     CA_ATTR,        ATTR_NONBUFFER,                 ATTR_STANDOUT },

    { _A1("search"),                    -1,                     CA_ATTR,        ATTR_SEARCH,                    ATTR_HILITE },
    { _A1("search_inc"),                -1,                     CA_ATTR,        ATTR_SEARCH_INC,                ATTR_SEARCH },
    { _A1("search_match"),              -1,                     CA_ATTR,        ATTR_SEARCH_MATCH,              ATTR_SEARCH },

    { _A1("ruler"),                     -1,                     CA_ATTR,        ATTR_RULER,                     ATTR_FRAME },
    { _A1("ruler_margin"),              -1,                     CA_ATTR,        ATTR_RULER_MARGIN,              ATTR_MESSAGE },
    { _A1("ruler_ident"),               -1,                     CA_ATTR,        ATTR_RULER_IDENT,               ATTR_ERROR },
    { _A1("ruler_mark"),                -1,                     CA_ATTR,        ATTR_RULER_MARK,                ATTR_TITLE },
    { _A1("ruler_column"),              -1,                     CA_ATTR,        ATTR_RULER_COLUMN,              ATTR_WHITESPACE },

    { _A1("popup_normal"),              -1,                     CA_ATTR,        ATTR_POPUP_NORMAL,              0 },
    { _A1("popup_hilite"),              -1,                     CA_ATTR,        ATTR_POPUP_HILITE,              ATTR_HILITE },
    { _A1("popup_standout"),            -1,                     CA_ATTR,        ATTR_POPUP_STANDOUT,            ATTR_STANDOUT },
    { _A1("popup_title"),               -1,                     CA_ATTR,        ATTR_POPUP_TITLE,               ATTR_TITLE },
    { _A1("popop_frame"),               -1,                     CA_ATTR,        ATTR_POPUP_FRAME,               ATTR_FRAME },

    /*
     *  Dialog widgets
     */
    { _A1("dialog_normal"),             -1,                     CA_ATTR,        ATTR_DIALOG_NORMAL,             0 },
    { _A1("dialog_focus"),              -1,                     CA_ATTR,        ATTR_DIALOG_FOCUS,              ATTR_PROMPT },
    { _A1("dialog_hilite"),             -1,                     CA_ATTR,        ATTR_DIALOG_HILITE,             ATTR_HILITE },
    { _A1("dialog_greyed"),             -1,                     CA_ATTR,        ATTR_DIALOG_GREYED,             ATTR_FRAME },
    { _A1("dialog_hotkey_normal"),      -1,                     CA_ATTR,        ATTR_DIALOG_HOTKEY_NORMAL,      ATTR_STANDOUT },
    { _A1("dialog_hotkey_focus"),       -1,                     CA_ATTR,        ATTR_DIALOG_HOTKEY_FOCUS,       ATTR_ERROR },
    { _A1("dialog_frame"),              -1,                     CA_ATTR,        ATTR_DIALOG_FRAME,              ATTR_FRAME },
    { _A1("dialog_title"),              -1,                     CA_ATTR,        ATTR_DIALOG_TITLE,              ATTR_TITLE },
    { _A1("dialog_scrollbar"),          -1,                     CA_ATTR,        ATTR_DIALOG_SCROLLBAR,          ATTR_SCROLLBAR },
    { _A1("dialog_thumb"),              -1,                     CA_ATTR,        ATTR_DIALOG_THUMB,              ATTR_SCROLLBAR_THUMB },

    { _A1("dialog_but_greyed"),         -1,                     CA_ATTR,        ATTR_DIALOG_BUT_GREYED,         ATTR_DIALOG_GREYED },
    { _A1("dialog_but_normal"),         -1,                     CA_ATTR,        ATTR_DIALOG_BUT_NORMAL,         ATTR_DIALOG_NORMAL },
    { _A1("dialog_but_focus"),          -1,                     CA_ATTR,        ATTR_DIALOG_BUT_FOCUS,          ATTR_DIALOG_FOCUS },
    { _A1("dialog_but_key_normal"),     -1,                     CA_ATTR,        ATTR_DIALOG_BUT_KEY_NORMAL,     ATTR_DIALOG_HOTKEY_NORMAL },
    { _A1("dialog_but_key_focus"),      -1,                     CA_ATTR,        ATTR_DIALOG_BUT_KEY_FOCUS,      ATTR_DIALOG_HOTKEY_FOCUS },

    { _A1("dialog_edit_greyed"),        -1,                     CA_ATTR,        ATTR_DIALOG_EDIT_GREYED,        ATTR_DIALOG_GREYED },
    { _A1("dialog_edit_normal"),        -1,                     CA_ATTR,        ATTR_DIALOG_EDIT_NORMAL,        ATTR_DIALOG_NORMAL },
    { _A1("dialog_edit_focus"),         -1,                     CA_ATTR,        ATTR_DIALOG_EDIT_FOCUS ,        ATTR_DIALOG_FOCUS },
    { _A1("dialog_edit_complete"),      -1,                     CA_ATTR,        ATTR_DIALOG_EDIT_COMPLETE,      ATTR_DIALOG_HILITE },

    /*
     *  Miscellaneous
     */
    { _A1("lsdirectory"),               -1,                     CA_ATTR,        ATTR_LSDIRECTORY,               0 },
    { _A1("lsexecute"),                 -1,                     CA_ATTR,        ATTR_LSEXECUTE,                 ATTR_STANDOUT },
    { _A1("lssymlink"),                 -1,                     CA_ATTR,        ATTR_LSSYMLINK,                 0 },
    { _A1("lspipe"),                    -1,                     CA_ATTR,        ATTR_LSPIPE,                    0 },
    { _A1("lsspecial"),                 -1,                     CA_ATTR,        ATTR_LSSPECIAL,                 0 },
    { _A1("lserror"),                   -1,                     CA_ATTR,        ATTR_LSERROR,                   ATTR_ERROR },
    { _A1("lsreadonly"),                -1,                     CA_ATTR,        ATTR_LSREADONLY,                0 },
    { _A1("lsnormal"),                  -1,                     CA_ATTR,        ATTR_LSNORMAL,                  0 },
    { _A1("lsattribute"),               -1,                     CA_ATTR,        ATTR_LSATTRIBUTE,               0 },
    { _A1("lssize"),                    -1,                     CA_ATTR,        ATTR_LSSIZE,                    0 },

    { _A1("modified"),                  -1,                     CA_ATTR,        ATTR_MODIFIED,                  0 },
    { _A1("additional"),                -1,                     CA_ATTR,        ATTR_ADDITIONAL,                ATTR_MODIFIED },
    { _A1("difftext"),                  -1,                     CA_ATTR,        ATTR_DIFFTEXT,                  ATTR_STANDOUT },
    { _A1("diffdelete"),                -1,                     CA_ATTR,        ATTR_DIFFDELETE,                ATTR_DIFFTEXT },

//  { _A1("match"),                     -1,                     CA_ATTR,        ATTR_MATCH,                     ATTR_STANDOUT },
        // TODO, auto-matched text, e.g. bracket matching

    { _A1("link"),                      -1,                     CA_ATTR,        ATTR_LINK,                      ATTR_STANDOUT },
    { _A1("tag"),                       -1,                     CA_ATTR,        ATTR_TAG,                       ATTR_STANDOUT },
    { _A1("alert"),                     -1,                     CA_ATTR,        ATTR_ALERT,                     ATTR_STANDOUT },

//  { _A1("ansi_bold"),                 -1,                     CA_ATTR,        ATTR_ANSI_BOLD,                 ATTR_STANDOUT },
//  { _A1("ansi_underline"),            -1,                     CA_ATTR,        ATTR_ANSI_UNDERLINE,            ATTR_STANDOUT },
        // TODO - ansi mode, 'underlined' text

    { _A1("spell"),                     -1,                     CA_ATTR,        ATTR_SPELL,                     ATTR_STANDOUT },
//  { _A1("spell_caps"),                -1,                     CA_ATTR,        ATTR_SPELL_CAPS,                ATTR_SPELL },
//  { _A1("spell_local"),               -1,                     CA_ATTR,        ATTR_SPELL_LOCAL,               ATTR_SPELL },
//  { _A1("spell_special"),             -1,                     CA_ATTR,        ATTR_SPELL_SPECIAL,             ATTR_SPELL },
        // TODO - additional special attritbutes

    { _A1("comment"),                   -1,                     CA_ATTR,        ATTR_COMMENT,                   0 },
    { _A1("comment_standout"),          -1,                     CA_ATTR,        ATTR_COMMENT_STANDOUT,          ATTR_STANDOUT },
    { _A1("todo"),                      -1,                     CA_ATTR,        ATTR_TODO,                      ATTR_COMMENT },

    /*
     *  Base syntax colors
     */
    { _A1("code"),                      -1,                     CA_ATTR,        ATTR_CODE,                      0 },
    { _A1("constant"),                  -1,                     CA_ATTR,        ATTR_CONSTANT,                  ATTR_CODE },
    { _A1("constant_standout"),         -1,                     CA_ATTR,        ATTR_CONSTANT_STANDOUT,         ATTR_STANDOUT },

    { _A1("string"),                    -1,                     CA_ATTR,        ATTR_STRING,                    ATTR_CODE },
    { _A1("character"),                 -1,                     CA_ATTR,        ATTR_CHARACTER,                 ATTR_STRING },
    { _A1("operator"),                  -1,                     CA_ATTR,        ATTR_OPERATOR,                  ATTR_CODE },
    { _A1("number"),                    -1,                     CA_ATTR,        ATTR_NUMBER,                    ATTR_CODE },
    { _A1("float"),                     -1,                     CA_ATTR,        ATTR_FLOAT,                     ATTR_NUMBER },
    { _A1("delimiter"),                 -1,                     CA_ATTR,        ATTR_DELIMITER,                 ATTR_CODE },
    { _A1("word"),                      -1,                     CA_ATTR,        ATTR_WORD,                      ATTR_CODE },
    { _A1("boolean"),                   -1,                     CA_ATTR,        ATTR_BOOLEAN,                   ATTR_CONSTANT },

    { _A1("preprocessor"),              -1,                     CA_ATTR,        ATTR_PREPROCESSOR,              ATTR_CODE },
    { _A1("preprocessor_define"),       -1,                     CA_ATTR,        ATTR_PREPROCESSOR_DEFINE,       ATTR_PREPROCESSOR },
    { _A1("preprocessor_include"),      -1,                     CA_ATTR,        ATTR_PREPROCESSOR_INCLUDE,      ATTR_PREPROCESSOR },
    { _A1("preprocessor_conditional"),  -1,                     CA_ATTR,        ATTR_PREPROCESSOR_CONDITIONAL,  ATTR_PREPROCESSOR },
    { _A1("preprocessor_keyword"),      -1,                     CA_ATTR,        ATTR_PREPROCESSOR_KEYWORD,      ATTR_PREPROCESSOR },
    { _A1("preprocessor_word"),         -1,                     CA_ATTR,        ATTR_PREPROCESSOR_WORD,         ATTR_WORD },

    { _A1("keyword"),                   -1,                     CA_ATTR,        ATTR_KEYWORD,                   ATTR_CODE },
    { _A1("keyword_function"),          -1,                     CA_ATTR,        ATTR_KEYWORD_FUNCTION,          ATTR_KEYWORD },
    { _A1("keyword_extension"),         -1,                     CA_ATTR,        ATTR_KEYWORD_EXTENSION,         ATTR_KEYWORD },
    { _A1("keyword_type"),              -1,                     CA_ATTR,        ATTR_KEYWORD_TYPE,              ATTR_KEYWORD },
    { _A1("keyword_storageclass"),      -1,                     CA_ATTR,        ATTR_KEYWORD_STORAGECLASS,      ATTR_KEYWORD_TYPE },
    { _A1("keyword_definition"),        -1,                     CA_ATTR,        ATTR_KEYWORD_DEFINTION,         ATTR_KEYWORD },
    { _A1("keyword_conditional"),       -1,                     CA_ATTR,        ATTR_KEYWORD_CONDITIONAL,       ATTR_KEYWORD },
    { _A1("keyword_repeat"),            -1,                     CA_ATTR,        ATTR_KEYWORD_REPEAT,            ATTR_KEYWORD },
    { _A1("keyword_exception"),         -1,                     CA_ATTR,        ATTR_KEYWORD_EXCEPTION,         ATTR_KEYWORD },
    { _A1("keyword_debug"),             -1,                     CA_ATTR,        ATTR_KEYWORD_DEBUG,             ATTR_KEYWORD },
    { _A1("keyword_label"),             -1,                     CA_ATTR,        ATTR_KEYWORD_LABEL,             ATTR_KEYWORD },
    { _A1("keyword_structure"),         -1,                     CA_ATTR,        ATTR_KEYWORD_STRUCTURE,         ATTR_KEYWORD_TYPE },
    { _A1("keyword_typedef"),           -1,                     CA_ATTR,        ATTR_KEYWORD_TYPEDEF,           ATTR_KEYWORD_TYPE },

    /*
     *  Window colors
     */
    { _A1("user1"),                     -1,                     CA_ATTR,        ATTR_USER1,                     0 },
    { _A1("user2"),                     -1,                     CA_ATTR,        ATTR_USER2,                     0 },
    { _A1("user3"),                     -1,                     CA_ATTR,        ATTR_USER3,                     0 },
    { _A1("user4"),                     -1,                     CA_ATTR,        ATTR_USER4,                     0 },
    { _A1("user5"),                     -1,                     CA_ATTR,        ATTR_USER5,                     0 },
    { _A1("user6"),                     -1,                     CA_ATTR,        ATTR_USER6,                     0 },
    { _A1("user7"),                     -1,                     CA_ATTR,        ATTR_USER7,                     0 },
    { _A1("user8"),                     -1,                     CA_ATTR,        ATTR_USER8,                     0 },
    { _A1("user9"),                     -1,                     CA_ATTR,        ATTR_USER9,                     0 },
    { _A1("user10"),                    -1,                     CA_ATTR,        ATTR_USER10,                    0 },

    { _A1("window1"),                   -1,                     CA_ATTR,        ATTR_WINDOW1,                   0 },
    { _A1("window2"),                   -1,                     CA_ATTR,        ATTR_WINDOW2,                   0 },
    { _A1("window3"),                   -1,                     CA_ATTR,        ATTR_WINDOW3,                   0 },
    { _A1("window4"),                   -1,                     CA_ATTR,        ATTR_WINDOW4,                   0 },
    { _A1("window5"),                   -1,                     CA_ATTR,        ATTR_WINDOW5,                   0 },
    { _A1("window6"),                   -1,                     CA_ATTR,        ATTR_WINDOW6,                   0 },
    { _A1("window7"),                   -1,                     CA_ATTR,        ATTR_WINDOW7,                   0 },
    { _A1("window8"),                   -1,                     CA_ATTR,        ATTR_WINDOW8,                   0 },
    { _A1("window9"),                   -1,                     CA_ATTR,        ATTR_WINDOW9,                   0 },
    { _A1("window10"),                  -1,                     CA_ATTR,        ATTR_WINDOW10,                  0 },
    { _A1("window11"),                  -1,                     CA_ATTR,        ATTR_WINDOW11,                  0 },
    { _A1("window12"),                  -1,                     CA_ATTR,        ATTR_WINDOW12,                  0 },
    { _A1("window13"),                  -1,                     CA_ATTR,        ATTR_WINDOW13,                  0 },
    { _A1("window14"),                  -1,                     CA_ATTR,        ATTR_WINDOW14,                  0 },
    { _A1("window15"),                  -1,                     CA_ATTR,        ATTR_WINDOW15,                  0 },
    { _A1("window16"),                  -1,                     CA_ATTR,        ATTR_WINDOW16,                  0 },

#undef  _A2
#undef  _A1
    };

static colors_t         x_colors = {0};         /* color definitions */
static int              x_valueclr = 0;         /* default colour 'user' value */
static colattr_t        x_attrs[ATTR_MAX];      /* active color table */


/*  Function:           do_color
 *      color primitive, used to set the current screen colors.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: color - Set the basic colors.

        int
        color([int background], [int normal],
            [int selected], [int message], [int error],
                [int hilite], [int hilite_fg], ...)

    Macro Description:
        The 'color()' primitive configures the standard color
        attributes. 'color' is only intended for use with the basic
        color set and is provided only for backward compatibility.

        If 'background' is omitted, all colors shall be prompted.
        Colors are numeric values in the range '0..15' or their
        symbolic name as follows:

    Basic Colors::

(start table,format=basic)
        [Value  [Name           [Value  [Name           ]
        0       Black           8       Dark-grey
        1       Blue            9       Bright-blue
        2       Green           10      Bright-green
        3       Cyan            11      Bright-cyan
        4       Red             12      Bright-red
        5       Magenta         13      Bright-magenta
        6       Brown           14      Yellow
        7       White           15      bright-white
(end table)

        Additional colors maybe modified, yet would advice only the
        use of symbolic color names and not color values to avoid
        issues when dealing with terminals which support greater then
        16 colors.

        The alternative interfaces of <set_color> and
        <set_color_pair> are the preferred interfaces for new macros.

    Macro Parameters:
        background - The background color.

        normal - The normal text color.

        selected - Highlighting color for the selected window title.

        message - The color for normal messages.

        error - The color for error messages.

        hilite - Color of marked areas. The value either states both
            the foreground and background of the marked areas as
            high nibble is background and the low is foreground.
            Unless hilite_fg is stated in which case only the
            foreground.

        hilite_fg - Foreground color of marked areas.

    Macro Returns:
        The 'color()' primitive returns 1 on success, otherwise 0 if
        one or more of the colors is invalid.

    Macro Example:

        Set the background color to blue and the foreground color
        to white, and active window title to bright cyan.

>           color(1, 7, 11);

    Macro Portability:
        Colors definitions beyond 'frame' are system dependent.

    Macro See Also:
        get_color, get_color_pair
 */
void
do_color(void)                  /* ([background], [normal], [selected], [messages], [errors],
                                        [hilite], [hilite_fg], [frame] */
{
    const int prompt = isa_undef(1);
    colors_t *colors = col_working();           /* working table */
    char buf[ATTRIBUTES_WIDTH];
    struct attribute *ap;
    unsigned i;
    int arg;
    trace_ilog("color:\n");

    for (i = 0, ap = colors->c_attr_table, arg = 1; i < colors->c_attr_count; ++i, ++ap, ++arg) {
        colattr_t ca = COLATTR_INIT;

        if (prompt) {
            const int ret = col_prompt(colors, ap, &ca);

            if (-1 == ret || -3 == ret) {
                return;                         /* abort */

            } else if (-2 == ret) {
                continue;                       /* continue/skip */
            }

        } else {
            if (arg >= margc) {
                break;                          /* end of argument list */
            }

            if (isa_undef(arg)) {
                continue;                       /* next */
            }

            if (isa_integer(arg)) {             /* numeric, byvalue */
                /*
                 *  Special case handling/compat:
                 *
                 *    HILITE unless both background/foreground- top nibble specifies
                 *      the background and bottom nibble specifies the foreground.
                 *
                 *    Treat all numeric values as symbolic, where possible.
                 */
                int val = get_xinteger(arg, 0);

                if (COL_HILITE_BACKGROUND == ap->ca_enum &&
                            isa_undef(arg+1)) { /* no hilite_fg */
                    colors->c_hilite_fg->fg.color = (val & 0x0f);
                    colors->c_hilite_fg->fg.source = COLORSOURCE_SYMBOLIC;
                    colors->c_hilite_fg->sf = 0;
                    val >>= 4;
                }

                if (val >= 0 && val <= COLOR_NONE) {
                    ca.fg.source = COLORSOURCE_SYMBOLIC;
                    ca.fg.color = val;
                }

            } else if (isa_string(arg)) {       /* symbolic */
                collinks_t links = ap->ca_links;
                const char *spec = get_str(arg);

                if (col_import(colors, "color", ap, spec, &ca, &links) < 0) {
                    continue;
                }
                ca = col_encode(colors, ap, &ca);
            }
        }

        col_export(colors, ap, &ca, buf, sizeof(buf));
        trace_ilog("\t%-30s = %s\n", ap->ca_name, buf);
        ap->ca_working = ca;
    }

    col_apply(colors);
    vtcolorscheme(FALSE, NULL);
}


/*  Function:           inq_colour
 *      inq_color primitive, used to retrieve the current screen colors.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_color - Retrieve the basic colors.

        string
        inq_color([int &background], [int &normal],
                [int &selected], [int &messages], [int &errors],
                    [int &hilite], [int &hilite_fg])

    Macro Description:
        The 'inq_color()' primitive retrieves color details of the
        primary color attributes. 'inq_color' is only intended for
        use with the basic color set and is provided only for
        backward compatibility.

        The alternative interfaces of <get_color>, <set_color>,
        <set_color_pair> and <get_color_pair> are the preferred
        interfaces for new macros.

    Macro Parameters:
        background - Background color.

        normal - Normal text color.

        selected - Highlighting color for the selected window title.

        message - Normal messages.

        error - Error messages.

        hilite - Color of marked areas. The value either states both
            the foreground and background of the marked areas as
            high nibble is background and the low is foreground.
            Unless hilite_fg is stated in which case only the
            foreground.

        hilite_fg - Foreground color of marked areas.

        frame - Window frame.

    Macro Returns:
        The 'inq_color()' primitive returns a string containing the
        current color specification, being a space seperated list
        of color names

    Macro Portability:
        n/a

    Macro See Also:
        color, get_color
 */
void
inq_color(void)                 /* string ([background], [normal], [selected], [messages], [errors],
                                                [hilite], [hilite_fg], [frame]) */
{
    const colors_t *colors = col_working();     /* working table */
    const struct attribute *ap;
    const unsigned size = ((colors->c_attr_count + 1) * ATTRIBUTES_WIDTH);
    char t_buf[ATTRIBUTES_WIDTH] = {0};
    unsigned i, len = 0;
    char *buf;
    int arg;

    if (NULL == (buf = (char *)chk_calloc(size, 1))) {
        acc_assign_str("", -1);
        return;
    }

    for (i = 0, ap = colors->c_attr_table, arg = 1; i < colors->c_attr_count; ++i, ++ap) {
        const colattr_t ca = ap->ca_value;

        if (ap->ca_enum < 0) {
            continue;                           /* base colors only */
        }

        if (arg < margc) {                      /* simple colour values. */
            if (isa_string(arg)) {
                color_print(&ca.fg, t_buf, sizeof(t_buf));
                argv_assign_str(arg, buf);

            } else if (isa_integer(arg)) {
                if (COLORSOURCE_SYMBOLIC == ca.fg.source) {
                    argv_assign_int(arg, (accint_t) ca.fg.color);

                } else {
                    argv_assign_int(arg, -1);
                }
            }
            ++arg;
        }

        if (len)  {                             /* export */
            buf[len++] = ' ';
        }

        len += col_export(colors, ap, &ca, buf + len, size - len);
    }

    acc_assign_str(buf, -1);
    chk_free(buf);
}


/*  Function:           do_set_colour_pair
 *      set_color_pair primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_color_pair - Set a specific color.

        void
        set_color_pair(string|int ident,
            [int|string fg], [int|string bg], [int|string sf])

    Macro Description:
        The 'set_color_pair()' primitive sets the pair of foreground
        and background colors associated with the color attribute
        'indent'.

        The specified attribute shall be assigned the given
        foreground color 'fg', with an optional background 'bg' and
        style 'sf'. If the foreground is omitted the user shall be
        prompted.

    Macro Parameters:
        ident - Attribute identifier either using its their
            manifest integer constants or string aliases, with
            names being case-insensitive; see <set_color> for
            details.

        fg - Optional foreground color. If omitted, then the
            foreground and (if required) background are prompted.

        bg - Optional background color.

        sf - Optional style and flags.

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_color, get_color_pair, get_color
 */
void
do_set_color_pair(void)         /* (string name|int ident, [int|string] fg, [int|string] bg, [int|string] sf) */
{
    static const char who[] = "set_color_pair";
    colors_t *colors = col_working();           /* working table */
    struct attribute *ap = NULL;
    int colordepth = vtcolordepth();

    /* attribute */
    if (isa_integer(1)) {                       /* numeric, byenum */
        const int ident = get_xinteger(1, -1);

        if (NULL == (ap = attr_byenum(colors, ident))) {
            errorf("%s: unknown attribute '%d'.", who, ident);
        }

    } else if (isa_string(1)) {                 /* symbolic, byname */
        const char *name = get_str(1);

        if (NULL == (ap = attr_byname(colors, name, FALSE))) {
            errorf("%s: unknown attribute '%s'.", who, name);
        }
    }

    /* value */
    if (ap) {
        const char *name;
        colattr_t ca = COLATTR_INIT;

        /* foreground/clear */
        if (isa_integer(2)) {                   /* numeric value */
            const int value = get_xinteger(2, 0);

            ca.fg.color = (value >= colordepth ? COLOR_UNKNOWN : value);
            ca.fg.source = COLORSOURCE_NUMERIC;
                                                /* symbolic */
        } else if (NULL != (name = get_xstr(2)) && *name) {
            if (col_import(colors, who, ap, name, &ca, NULL) < 0) {
                return;
            }

        } else {                                /* default foreground if background not omitted */
            if ((CA_ATTR & ap->ca_flags) && !isa_undef(3)) {
                ca.fg.color = COLOUR_DYNAMIC_FOREGROUND;
                ca.fg.source = COLORSOURCE_SYMBOLIC;

            } else {                            /* omitted, prompt */
                if (col_prompt(colors, ap, &ca)) {
                    return;
                }
            }
        }

        /* background (optional) */
        if (isa_integer(3)) {                   /* numeric */
            const int value = get_xinteger(3, 0);

            ca.bg.color = (value >= colordepth ? value : value);
            ca.bg.source = COLORSOURCE_NUMERIC;
                                                /* symbolic */
        } else if (NULL != (name = get_xstr(3)) && *name) {
            if (! color_value(name, &ca.bg, TRUE)) {
                errorf("%s: unknown background '%s'.", who, name);
                return;
            }
        }

        /* style (optional) */
        if (isa_integer(4)) {
            ca.sf = get_xinteger(4, 0);

        } else if (isa_string(4)) {
            ca.sf = style_import(colors, who, get_str(4), NULL, 0);
        }

        /* set value */
        if (ca.fg.color > COLOR_UNKNOWN) {
            char buf[ATTRIBUTES_WIDTH];

            ca = col_encode(colors, ap, &ca);
            col_export(colors, ap, &ca, buf, sizeof(buf));
            trace_ilog("\t%s=%s\n", ap->ca_name, buf);
            ap->ca_working = ca;
            col_apply(colors);
            vtcolorscheme(FALSE, NULL);
        }
    }
}


/*  Function:           do_get_colour_pair
 *      get_color_pair primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: get_color_pair - Retrieve the specific color.

        void
        get_color_pair(string name|int ident,
                [int|string fg], [int|string bg], [int|string sf])

    Macro Description:
        The 'get_color_pair()' primitive retrieves a specific
        attribute that GRIEF utilities on a color display. Attributes
        may be specified as integers or strings, with strings being
        case-insensitive see <set_color> for more details.

        The specified attribute color values shall be assigned to the
        specified arguments 'foreground', 'backbround' and 'style'.

    Macro Parameters:
        ident - Attribute identifier.

        fg - Optional variable reference to be populated with the
            foreground color. If an integer reference the numeric
            colour value is assigned otherwise if a string reference
            the associated name.

        bg - Optional variable reference to be populated with the
            background color. If an integer reference the numeric
            colour value is assigned otherwise if a string reference
            the associated name.

        sf - Optional variable reference to be populated with the
            style and flags. If an integer reference the numeric
            value is assigned otherwise when a string reference the
            human readable decoding version is assigned.

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_color_pair, set_color, get_color
 */
void
do_get_color_pair(void)         /* (string name|int ident, [int|string fg], [int|string bg], [int|string sf]) */
{
    static const char who[] = "get_color_pair";
    const colors_t *colors = col_working();     /* working table */
    const struct attribute *ap = NULL;

    /* attribute */
    if (isa_integer(1)) {                       /* numeric, byenum */
        const int value = get_xinteger(1, -1);

        if (NULL == (ap = attr_byenum(colors, value))) {
            errorf("%s: unknown attribute '%d'.", who, value);
        }

    } else if (isa_string(1)) {                 /* symbolic, byname */
        const char *name = get_xstr(1);

        if (NULL == (ap = attr_byname(colors, name, FALSE))) {
            errorf("%s: unknown attribute color '%s'.", who, name);
        }
    }

    /* value */
    if (ap) {
        const colattr_t ca = ap->ca_value;
        char buf[ATTRIBUTES_WIDTH] = {0};

        if (isa_string(2)) {
            color_print(&ca.fg, buf, sizeof(buf));
            argv_assign_str(2, buf);
        } else {
            argv_assign_int(2, (accint_t) ca.fg.color);
        }

        if (isa_string(3)) {
            color_print(&ca.bg, buf, sizeof(buf));
            argv_assign_str(3, buf);
        } else {
            argv_assign_int(3, (accint_t) ca.bg.color);
        }

        if (isa_string(4)) {
            style_print(ca.sf, buf, sizeof(buf));
            argv_assign_str(4, *buf ? buf + 1 : "");
        } else {
            argv_assign_int(4, (accint_t) ca.sf);
        }
    }
}


/*  Function:           do_set_color
 *      set_color primitive
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_color - Set screen colors.

        int
        set_color([list|string spec], [int create = TRUE])

    Macro Description:
        The 'set_color()' primitive controls the colors that are
        utilised on a color display; the values are ignored for
        monochrome displays.

        It is designed to compliment and enhance the functionality
        of the 'color' primitive, with color() being a user level
        interface and set_color() being a script level interface.

    Macro Parameters:
        spec - Color specification, either a list or comma
            separated set of attributes. The specification may use
            one of two forms, firstly an explicit set of attributes
            pairs or secondary as an implicit list of colors, see
            below.

        create - Optional boolean flag, if either omitted or *true*
            non-existent attributes shall be created.

    Color Specification::

        The specification may take one of the following forms. For
        details on available attributes and possible colors, see
        the 'Color Attributes' and 'Color Name' sections below.

        *Explicit:*

            A list of strings containing attribute color pairs
            Simple attributes are assigned a color delimited by a
            '=' take the form:

>               "attribute=color"

            For the highlight and syntax attributes an extended
            form permits explicit selection of the background over
            the implied default of 'background' in additional to
            optional 'style' settings, as follows:

>               "attr=color[:style][,background]"

        *Implicit:*

            An ordered list of strings corresponding to each color
            attribute to be set, using the form:

>               "color-name color-name ..."

            A value of NULL in the list means to skip the
            assignment of the related attribute. An integer value
            within the list allows the index reference to be
            explicitly set for subsequent attribute assignment.

            Note, provided primary for CRisP compatibility as the
            explicit interface is the recommended use of the
            <set_color> primitive to guard against future
            enhancements/color attributes.

            Also note as the color interfaces have developed
            independently the COL enumeration values differ.

    Scheme Specification:

        A special attribute 'scheme' sets the default color scheme,
        taking the form:

>           scheme=d[ark]|l[ight]

    Color Names::

        The following color names are recognised which are
        case-insensitive, with the color number used which are
        available on most systems. The first name listed shall be
        the primary name as returned from inquiry functions (see
        get_color) with the additional as aliases.

(start table,format=nd)
        [Color          [Constant           [Aliases                ]
      ! Black           BLACK
      ! Blue            BLUE
      ! green           GREEN
      ! Cyan            CYAN
      ! Red             RED
      ! Magenta         MAGENTA
      ! Brown           BROWN
      ! White           WHITE
      ! Grey            GREY                Gray
      ! Light-blue      LTBLUE              LightBlue
      ! Light-green     LTGREEN             LightGreen
      ! Light-cyan      LTCYAN              LightCyan
      ! Light-red       LTRED               LightRed
      ! Light-magenta   LTMAGENTA           LightMagenta
      ! Yellow          YELLOW
      ! Light-white     LTWHITE             LightWhite
      ! Bright-white    LTWHITE
      ! Light-grey      LTWHITE             LightGrey
      ! Light-gray      LTWHITE             LightGray
      ! Dark-grey       DKGREY              DarkGrey
      ! Dark-gray       DKGREY              DarkGray
      ! Dark-blue       DKBLUE              DarkBlue
      ! Dark-green      DKGREEN             DarkGreen
      ! Dark-cyan       DKCYAN              Darkcyan
      ! Dark-red        DKRED               DarkRed
      ! Dark-magenta    DKMAGENTA           DarkMagenta
      ! Dark-yellow     DKYELLOW            DarkYellow
      ! Light-yellow    LTYELLOW            LightYellow


(end table)

        plus the following specials to support black-white and
        terminal features:

(start table,format=nd)
        [Color              [Description                            ]
      ! Normal              Normal terminal text.
      ! Inverse             Inverse contrast.
      ! Blink               Blinking text.
      ! Reverse             Reverse contrast.
      ! Standout            Stand-out terminal text.
      ! Dim                 Dim colors.
(end table)

    Color Codes::

        Under environments which support a larger color range,
        examples include xterm256 and WIN32+, the following
        additional system colors are available. Either by its color
        values (e.g. 0x32) or by its Red, Green and Blue (RGB) value.

        The format is "#rrggbb" being hexadecimal value in range
        00-ff, where:

            rr - is the Red value.
            gg - is the Green value.
            bb - is the Blue value.

    Color Attributes::

        There are a number of display attributes which can be
        assigned specific colors. These attributes represent a
        number of different display objects from basis editing,
        syntax hiliting and dialog.

        The table below lists all the attributes, their
        manifest-constant within the <set_color> and <get_color>
        interface and description:

(start table,format=nd)
      [Name                     [Constant               [Description                        ]
    ! background                COL_BACKGROUND          Background color.
    ! normal                    COL_NORMAL_FG           Normal text.
    ! select                    COL_SELECT_FG           Title of selected window.
    ! message                   COL_MESSAGE_FG          Prompt, messages and status fields.
    ! error                     COL_ERROR_FG            Error messages.
    ! hilite                    COL_HILITE_BG           Highlighted/marked background.
    ! hilite                    COL_HILITE_FG           Highlighted foreground.
    ! frame                     COL_BORDERS             Window frame/borders.
    ! cursor_insert             COL_INSERT_CURSOR       Insert mode cursor.
    ! cursor_overtype           COL_OVERTYPE_CURSOR     Over-type mode cursor.
    ! cursor                    n/a
    ! cursor_row                n/a
    ! cursor_col                n/a
    ! shadow                    COL_SHADOW
    ! prompt                    COL_PROMPT
    ! prompt_standout           n/a
    ! prompt_complete           COL_COMPLETION
    ! question                  COL_QUESTION
    ! echo_line                 COL_ECHOLINE
    ! standout                  COL_STANDOUT
    ! literalchar               n/a
    ! whitespace                n/a                     Highlighted white-space/tabs.
    ! scrollbar                 n/a
    ! scrollbar_thumb           n/a
    ! column_status             n/a
    ! column_lineno             n/a                     Line numbers.
    ! nonbuffer                 n/a
    ! search                    n/a
    ! search_inc                n/a
    ! search_match              n/a
    ! ruler                     n/a
    ! ruler_margin              n/a
    ! ruler_ident               n/a
    ! ruler_mark                n/a
    ! ruler_column              n/a
    ! popup_normal              n/a
    ! popup_hilite              n/a
    ! popup_standout            n/a
    ! popup_title               n/a
    ! popop_frame               n/a
    ! dialog_normal             n/a
    ! dialog_focus              n/a
    ! dialog_hilite             n/a
    ! dialog_greyed             n/a
    ! dialog_hotkey_normal      n/a
    ! dialog_hotkey_focus       n/a
    ! dialog_frame              n/a
    ! dialog_title              n/a
    ! dialog_scrollbar          n/a
    ! dialog_thumb              n/a
    ! dialog_but_greyed         n/a
    ! dialog_but_normal         n/a
    ! dialog_but_focus          n/a
    ! dialog_but_key_normal     n/a
    ! dialog_but_key_focus      n/a
    ! dialog_edit_greyed        n/a
    ! dialog_edit_normal        n/a
    ! dialog_edit_focus         n/a
    ! dialog_edit_complete      n/a
    ! lsdirectory               n/a
    ! lsexecute                 n/a
    ! lssymlink                 n/a
    ! lspipe                    n/a
    ! lsspecial                 n/a
    ! lserror                   n/a
    ! lsreadonly                n/a
    ! lsnormal                  n/a
    ! lsattribute               n/a
    ! lssize                    n/a
    ! modified                  n/a
    ! additional                n/a
    ! difftext                  n/a
    ! diffdelete                n/a
    ! match                     n/a
    ! link                      n/a
    ! tag                       n/a
    ! alert                     n/a
    ! ansi_bold                 n/a
    ! ansi_underline            n/a
    ! spell                     n/a
    ! spell_local               n/a
    ! spell_special             n/a
    ! comment                   n/a                     Language comments.
    ! comment_standout          n/a
    ! todo                      n/a
    ! code                      n/a
    ! constant                  n/a
    ! constant_standout         n/a
    ! string                    n/a                     Language string elements.
    ! character                 n/a
    ! operator                  n/a                     Language operator elements.
    ! number                    n/a                     Language numeric values.
    ! float                     n/a
    ! delimiter                 n/a                     Language delimiter elements.
    ! word                      n/a
    ! boolean                   n/a
    ! preprocessor              n/a                     Language preprocesor elements.
    ! preprocessor_define       n/a
    ! preprocessor_include      n/a
    ! preprocessor_conditional  n/a
    ! preprocessor_keyword      n/a
    ! preprocessor_word         n/a
    ! keyword                   n/a                     Language keywords, primary.
    ! keyword_function          n/a
    ! keyword_extension         n/a
    ! keyword_type              n/a
    ! keyword_storageclass      n/a
    ! keyword_definition        n/a
    ! keyword_conditional       n/a
    ! keyword_repeat            n/a
    ! keyword_exception         n/a
    ! keyword_debug             n/a
    ! keyword_label             n/a
    ! keyword_structure         n/a
    ! keyword_typedef           n/a
    ! user1                     n/a                     User specified 1
    ! user2                     n/a                     User specified 2
    ! user3                     n/a                     User specified 3
    ! user4                     n/a                     User specified 4
    ! user5                     n/a                     User specified 5
    ! user6                     n/a                     User specified 6
    ! user7                     n/a                     User specified 7
    ! user8                     n/a                     User specified 8
    ! user9                     n/a                     User specified 9
    ! user10                    n/a                     User specified 10
    ! window1                   n/a
    ! window2                   n/a
    ! window3                   n/a
    ! window4                   n/a
    ! window5                   n/a
    ! window6                   n/a
    ! window7                   n/a
    ! window8                   n/a
    ! window9                   n/a
    ! window10                  n/a
    ! window11                  n/a
    ! window12                  n/a
    ! window13                  n/a
    ! window14                  n/a
    ! window15                  n/a
    ! window16                  n/a
(end table)

    Styles::

        The following styles are recognised which are case-insensitive.

(start table,format=nd)
        [Featuree           [Description                            ]
      ! Underline           Underlined text.
      ! Italic              Italic typeface.
      ! Bold                Bold typeface.
      ! Italic              Italic typeface.
      ! Underline           Under lined text.
      ! Undercurl           Curly underline, generally underline.
(end table)

    Macro Returns:
        The 'set_color()' primitive returns 0 on success, otherwise -1
        -1 on failure to set colors.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        get_color, color, inq_color, set_color_pair
 */
void
do_set_color(void)              /* ([list|string], [int create = TRUE]) */
{
    static const char who[] = "set_color";
    const LIST *lp = get_xlist(1);
    const int create = get_xinteger(2, FALSE);
    colors_t *colors = col_working();           /* working table */
    const char *spec;
    int count = 0;

    trace_ilog("%s:\n", who);
    if (lp) {
        const LIST *nextlp;
        accint_t ident = 0;                     /* cursor */

        for (; (nextlp = atom_next(lp)) != lp; lp = nextlp) {
            LISTV result = {0};

            if (F_ERROR == eval(lp, &result)) {
                errorf("%s: invalid colour index '%d'", who, (int)ident);
                return;
            }

            if (listv_str(&result, &spec)) {    /* color specification */
                if (set_color(colors, spec, create, &ident)) {
                    ++count;
                }

            } else {                            /* index specification */
                listv_int(&result, &ident);
                if (ident < 0 || ident > COL_MAX) {
                    errorf("%s: invalid colour index '%d'", who, (int)ident);
                    return;
                }
            }
        }

    } else if (NULL != (spec = get_xstr(1))) {
        const char *nl;

        if (NULL != (nl = strchr(spec, '\n'))) {
            char buf[ATTRIBUTES_WIDTH];         /* multi-part */

            do {                                /* .. foreach() */
                const int len = nl - spec;

                if (len > 0 && len < (int)sizeof(buf)) {
                    memcpy(buf, spec, len);
                    buf[len] = 0;
                    if (set_color(colors, buf, create, NULL)) {
                        ++count;
                    }
                }
                spec = nl + 1;

            } while (NULL != (nl = strchr(spec, '\n')));

            if (*spec) {                        /* .. last */
                if (set_color(colors, spec, create, NULL)) {
                    ++count;
                }
            }

        } else {                                /* single */
            if (set_color(colors, spec, create, NULL)) {
                ++count;
            }
        }
    }

    if (count) {
        col_apply(colors);
        vtcolorscheme(count > 1 ? TRUE : FALSE, NULL);
    }

    acc_assign_int(count);
}


/*  Function:           do_get_color
 *      get_color primitive, retrieve the current screen colors.
 *
 *  Parameters:
 *      none
 *
 *<<GRIEF>>
    Macro: get_color - Retrieve screen colors.

        list
        get_color([int flags = 0])

    Macro Description:
        The 'get_color()' primitive retrieves the current display
        color scheme as a list of strings, each string containing the
        specification of an individual color attribute.

        The reported specifications following the form based upon
        the optional 'flags'.

>           [<id>,][<flags>,][<name>=]<spec>

            id     - Numeric identifier of the attribute.
            flags  - Control flags.
            name   - Attribute name.
            spec   - Color specification.

    Color Specification:

        Each color specification followings the following form.

>           attribute=
>                sticky@<attribute>|none
>              | link@<attribute>|none
>              | <foreground-color>[,<background-color>][|font][:style ...]
>              | clear

        Where colors take one the following forms, see <set_color>
        for a list of the reported attributes.

>           color-name|none
>           foreground|fg|background|bg|dynamic_fg|dyanmic_bg
>           decimal (xx), octal (0xxx) or hex (0x###)
>           #RRGGBB
>           color[#]ddd

        style specifications are in the form :<style> [,<style> ...]

>           bold
>           inverse
>           underline
>           blink
>           italic
>           reverse
>           standout
>           dim
>           undercurl

    Macro Parameters:
        flags - Optional integer flags, one or more of the
            following flags OR'ed together control the components
            to be reported against each attribute.

    Flags::

(start table,format=nd)
        [Constant       [Definition                             ]
      ! COLORGET_FVALUE Attribute numeric value/identifier.
      ! COLORGET_FFLAGS Type flags.
      ! COLORGET_FNAME  Name.
(end table)

    Macro Returns:
        Returns a list of colors associated with the color
        attributes.

        If 'flags' was specified then the color attribute flags are
        returned in a list, instead of the color names.

    Macro Example:

        Example list content using 'COLORGET_FNAME'

>           "background=none"
>           "normal=none"
>           "select=light-cyan"
>           "message=light-green"
>           "error=light-red"
>           "hilite=red"
>           "hilite_fg=light-white"
>           "frame=white"
>           "inscursor=black"
>           "ovcursor=black"
>           "shadow=clear"
>           "prompt=light-magenta,dynamic-bg:link@message"
>           "question=clear:link@message"
>           "echo_line=yellow,dynamic-bg"
>           "statuscolumn=clear:link@message"
>           "linenocolumn=clear:link@message"
>           "nonbuffer=brown,dynamic-bg:link@message"
>               :
>               :

    Macro Portability:
        A Grief extension.

    Macro See Also:
        color, inq_color, set_color
 */
void
do_get_color(void)              /* list ([int flags = 0]) */
{
    const colors_t *colors = col_working();     /* working values */
    unsigned attrs = colors->c_attr_count;
    const int flags = get_xinteger(1, 0);
    const int llen = (attrs * sizeof_atoms[F_RSTR]) + sizeof_atoms[F_HALT];
    LIST *newlp;

    if (NULL != (newlp = lst_alloc(llen, attrs))) {
        LIST *lp = newlp;
        const struct attribute *ap;
        char buf[ATTRIBUTES_WIDTH];
        unsigned i;

        for (i = 0, ap = colors->c_attr_table; i < colors->c_attr_count; ++i, ++ap) {
            const colattr_t val = ap->ca_value;
            int xlen = 0;

            if (COLORGET_FVALUE & flags) {      /* attribute value */
                xlen += sxprintf(buf + xlen, sizeof(buf) - xlen, "%d,", ap->ca_attr);
            }

            if (COLORGET_FFLAGS & flags) {      /* type flags */
                xlen += sxprintf(buf + xlen, sizeof(buf) - xlen, "%d,", ap->ca_flags);
            }

            if (COLORGET_FNAME  & flags) {      /* name */
                xlen += sxprintf(buf + xlen, sizeof(buf) - xlen, "%s=", ap->ca_name);
            }

                                                /* value */
            col_export(colors, ap, &val, buf + xlen, sizeof(buf) - xlen);
            lp = atom_push_str(lp, buf);
            --attrs;
        }

        assert(0 == attrs);
        atom_push_halt(lp);
    }
    acc_donate_list(newlp, llen);
}


/*  Function:           color_setscheme
 *      Load one of the default color schemes.
 *
 *>         scheme=d[ark]|l[ight]
 *
 *  Parameters:
 *      scheme - Scheme name.
 *
 *  Returns:
 *      *true* otherwise *false*.
 */
int
color_setscheme(const char *scheme)
{
    if (NULL == scheme || 0 == str_icmp(scheme, "default")) {
        if (NULL == (scheme = vtdefaultscheme())) {
            scheme = "dark";
        }
    }

    if (scheme && *scheme) {
        const char **colorspec = NULL;
        int isdark = FALSE;

        trace_ilog("color_setscheme=%s\n", scheme);
        if (0 == str_icmp(scheme, "l") || 0 == str_icmp(scheme, "light")) {
            colorspec = x_col_table_light;

        } else if (0 == str_icmp(scheme, "d") || 0 == str_icmp(scheme, "dark")) {
            colorspec = x_col_table_dark;
            isdark = TRUE;
        }

        if (colorspec) {
            colors_t *colors = col_reworked();
            unsigned i;

            for (i = 0; colorspec[i]; ++i) {
                set_color(colors, colorspec[i], FALSE, NULL);
            }

            for (i = 0; x_col_windows[i]; ++i) {
                set_color(colors, x_col_windows[i], FALSE, NULL);
            }

            col_apply(colors);
            vtcolorscheme(TRUE, (isdark ? "dark" : "light"));
            colors->c_isdark = isdark;
            return TRUE;
        }
        errorf("color: unknown scheme '%s'.", scheme);
    }
    return FALSE;
}


/*  Function:           set_color
 *      Command color specification decoder.
 *
 *>         scheme=d[ark]|l[ight]default
 *>         attribute=
 *>             [sticky]link@<attribute>|none
 *>             <foreground-color> [,<background-color>] [|font] [:style ...]
 *>             clear
 *
 *      where colors take one the following forms
 *
 *>         color-name|none
 *>         foreground|fg|background|bg|dynamic_fg|dyanmic_bg
 *>         decimal (xx), octal (0xxx) or hex (0x###)
 *>         #RRGGBB
 *>         color[#]ddd
 *
 *      style specifications are in the form :<style> [,<style> ...]
 *
 *>         bold
 *>         inverse
 *>         underline
 *>         blink
 *>         italic
 *>         reverse
 *>         standout
 *>         dim
 *>         undercurl
 *
 *  Parameters:
 *      colors - Color table.
 *      spec - Specification.
 *      create - *true* if non-existent attributes should be created.
 *      ident - Implied attribute ident, otherwise -1.
 *
 *  Returns:
 *      *true* otherwise *false*.
 */
static int
set_color(colors_t *colors, const char *spec, int create, accint_t *ident)
{
    static const char who[] = "set_color";
    struct attribute *ap = NULL;
    const char *eq = NULL;
    colattr_t ca = COLATTR_INIT;
    int ret = -1;

    trace_ilog("set_color<spec:%s,create:%d,ident:%d>\n",
        spec, create, (int)(ident ? *ident : -1));

    if (NULL == (eq = strchr(spec, '='))) {
        /*
         *  implicit color index
         *
         *      foreground [,background] [:style]
         */
        if (NULL == ident) {
            errorf("%s: attribute missing", who);
        } else {
            const int nident = *ident++;        // next identifier

            if (nident >= COL_MAX) {
                errorf("%s: attribute ident unexpected '%d'", who, nident);
            } else if (NULL == (ap = attr_byenum(colors, nident))) {
                errorf("%s: unknown attribute ident '%d'", who, nident);
            } else {
                ret = col_import(colors, who, ap, spec, &ca, NULL);
            }
        }

    } else {
        /*
         *      scheme=<l[ight]|d[ark]|default>
         *
         *  by name, explicit attribute
         *
         *      attribute=
         *          [sticky]link@[none|<attribute>]
         *          foreground [,background] [:style]
         */
        if (0 == str_nicmp(spec, "scheme=", 7)) {
            color_setscheme(eq + 1);
            return FALSE;
        }

        if (NULL == (ap = attr_bynname(colors, spec, eq - spec, create))) {
            errorf("%s: unknown attribute '%.*s'", who, (int)(eq - spec), spec);

        } else {
            collinks_t lks = ap->ca_links;

            ++eq;
            if (NULL == strchr(eq, ':') &&
                      NULL != strchr(eq, '@') && NULL != strchr(eq, '=')) {
                if (style_import(colors, who, eq, &lks, ap->ca_attr)) {
                    ret = -1;
                }
            } else {
                ret = col_import(colors, who, ap, eq, &ca, &lks);
            }

            if (0 == ret) {
                ap->ca_links = lks;
            }
        }
    }

    if (0 == ret) {
        char buf[ATTRIBUTES_WIDTH];
        colattr_t val;

        val = col_encode(colors, ap, &ca);
        col_export(colors, ap, &val, buf, sizeof(buf));
        trace_ilog("\t%-30s = %s\n", ap->ca_name, buf);
        ap->ca_working = val;
        return TRUE;
    }
    return FALSE;
}


#if defined(NOT_USED)
/*  Function:           link_cycle
 *      Determine whether a cycle exists within the current link definitions.
 *
 *  Parameters:
 *      colors - Color table.
 *      attr - Attribute to be checked.
 *
 *  Returns:
 *      *true* or *false*.
 */
static int
link_cycle(const colors_t *colors, struct attribute *ap, struct attribute *lnk)
{
    return 0;
}
#endif


/*  Function:           color_definition
 *      Retrieve the color definition for a specific attribute.
 *
 *  Parameters:
 *      attr - Attribute.
 *      ca - Color attribute.
 *
 *  Returns:
 *      Color definition.
 */
const colattr_t *
color_definition(int attr, colattr_t *ca)
{
    assert(attr >= 0);
    assert(attr <= ATTR_MAX);
    assert(attr <= (int)(sizeof(x_attrs)/sizeof(x_attrs[0])));
    *ca = x_attrs[ attr ];
    return ca;
}


void
color_valueset(int attr, int val)
{
    assert(attr >= 0);
    assert(attr <= ATTR_MAX);
    assert(attr <= (int)(sizeof(x_attrs)/sizeof(x_attrs[0])));
    x_attrs[ attr ].val = val;
}


void
color_valueclr(int clr)
{
    unsigned attr;

    for (attr = 0; attr < ATTR_MAX; ++attr) {
        x_attrs[ attr ].val = clr;
    }
    x_valueclr = clr;
}


/*  Function:           col_encode
 *      Encode the color for the specification attribute 'ap'.
 *
 *      The foreground 'fg', background 'bg' and style values are applied
 *      based on the attribute configuration.
 *
 *  Parameters:
 *      colors - Color table.
 *      ap - Attribute object.
 *      ca - Color attribute.
 *
 *  Special case handling:
 *      BACKGROUND          First specification 'fg' as the background.
 *      HILITE              Optional background.
 *      ATTR                Full color specification.
 *      others              Ignore background.
 *
 *  Returns:
 *      Encoded color value.
 */
static colattr_t
col_encode(const colors_t *colors, const struct attribute *ap, const colattr_t *ca)
{
    char buf[ATTRIBUTES_WIDTH];
    colattr_t ret = *ca;

    if (ret.sf < 0) {
        ret.sf = 0;                             /* assign default */
    }

    ret.fg = col_static(colors, &ret.fg);

    if (COL_HILITE_BACKGROUND == ap->ca_enum) { /* special -- hilite, both fg and bg */
        if (ret.bg.color > COLOR_UNKNOWN) {
            colors->c_hilite_fg->fg = ret.fg;
            ret.fg = ret.bg;
        }
        ret.bg.color = COLOR_UNKNOWN;
        ret.bg.source = COLORSOURCE_NONE;

    } else if (CA_ATTR & ap->ca_flags) {        /* full attributes */
        ret.bg = col_static(colors, &ret.bg);

    } else {                                    /* implied background */
        ret.bg.color = COLOR_UNKNOWN;
        ret.bg.source = COLORSOURCE_NONE;
    }

    col_export(colors, ap, &ret, buf, sizeof(buf));
    trace_ilog("\tcol_encode(%s, fg:0x%x-%d/%d, bg:0x%x-%d/%d, sf:%d) = %s\n",
        ap->ca_name, ca->fg.color, ca->fg.color, ca->fg.source, ca->bg.color, ca->bg.color, ca->bg.source, ca->sf, buf);
    return ret;
}


static colvalue_t
col_static(const colors_t *colors, const colvalue_t *val)
{
    if (COLORSOURCE_SYMBOLIC == val->source) {
        switch (val->color) {
        case COLOUR_BACKGROUND:
            return col_background(colors);

        case COLOUR_FOREGROUND:
            return col_normal(colors);

        default:
            assert(val->color > COLOR_UNKNOWN);
            assert(val->color <= COLOUR_DYNAMIC_BACKGROUND);
            break;
        }
    }
    return *val;
}


/*  Function:           attribute_value
 *      Map the attribute 'name' to its attribute enumeration.
 *
 *  Parameters:
 *      name - Attribute name.
 *
 *  Returns:
 *      Attribute identifier otherwise -1.
 */
int
attribute_value(const char *name)
{
    unsigned length = strlen(name);
    const struct attribute *ap;

    if (length) {
        if (NULL != (ap = attr_bynname(col_working(), name, length, FALSE))) {
            return ap->ca_attr;                 /* standard */
        }
    }
    return -1;
}


/*  Function:           attribute_new
 *      Map the attribute 'name' to its attribute enumeration, allocating a new
 *      attribute enumeration if required using the stated specification 'spec'.
 *
 *  Parameters:
 *      name - Attribute name.
 *      spec - Color specification.
 *
 *  Returns:
 *      Attribute identifier otherwise -1.
 */
int
attribute_new(const char *name, const char *spec)
{
    unsigned length = strlen(name);
    struct attribute *ap;

    if (length) {
        const colors_t *colors = col_working();

        if (NULL != (ap = attr_bynname(colors, name, length, TRUE))) {
            if (0 == ap->ca_flags) {
                colattr_t ca = COLATTR_INIT;
                collinks_t lks = {0, 0};

                if (0 == col_import(colors, "new", ap, spec, &ca, &lks)) {
                    ap->ca_flags   = CA_ATTR;
                    ap->ca_working = ca;
                    ap->ca_links   = lks;

                } else {
                    //TODO
                }
            }
            return ap->ca_attr;                 /* standard */
        }
    }
    return -1;
}


/*static*/ colors_t *
col_working(void)
{
    colors_t *colors = &x_colors;               /* global color table */
    struct attribute *ap;
    unsigned i;

    if (NULL == colors->c_attr_table) {         /* build table */
        assert(0 == BLACK);
        assert(8 == GREY);
        assert(ATTR_SYNTAX_MAX < ATTR_USER);

        colors->c_attr_count   = 0;
        colors->c_attr_dynamic = 0;
        colors->c_attr_alloc   = (ATTRIBUTES_COUNT * 2);
        if (NULL == (colors->c_attr_table =
                chk_alloc(colors->c_attr_alloc * sizeof(struct attribute)))) {
            colors->c_attr_alloc = 0;
            panic("Unable to build color table -- memory error.");
            return colors;
        }
        col_clear(colors);
        col_references(colors);
    }
                                                /* prime working */
    for (i = colors->c_attr_count, ap = colors->c_attr_table; i-- > 0; ++ap) {
        ap->ca_working = ap->ca_value;
    }

    return colors;
}


/*static*/ colors_t *
col_reworked(void)
{
    colors_t *colors = &x_colors;               /* global color table */

    if (colors->c_attr_table) {
        col_clear(colors);
    }
    return col_working();
}


/*static*/ void
col_clear(colors_t *colors)
{
    struct attribute *ap;
    unsigned fg, bg, a;

    assert(colors);
    assert(colors->c_attr_table);

    if ((a = colors->c_attr_dynamic) > 0)       /* user attributes */
        for (ap = colors->c_attr_table + ATTRIBUTES_COUNT; a-- > 0; ++ap) {
            chk_free((char *)ap->ca_name);
            ap->ca_name = NULL;
        }
    colors->c_attr_dynamic = 0;
                                                /* initialise */
    memset(colors->c_attr_table, 0, colors->c_attr_alloc * sizeof(struct attribute));
    for (a = 0, ap = colors->c_attr_table; a < ATTRIBUTES_COUNT; ++a, ++ap) {
        *ap = attributes_default[a];
    }
    colors->c_attr_count = ATTRIBUTES_COUNT;

    memset(x_attrs, 0, sizeof(x_attrs));        /* ANSI attributes, 8x16 colors=128 */
    for (fg = 0, a = ATTR_ANSI128; fg < ANSIFG_MAX; ++fg)
        for (bg = 0; bg < ANSIBG_MAX; ++bg) {
            colattr_t *ca = x_attrs + a++;

            ca->fg.color  = fg;
            ca->fg.source = COLORSOURCE_SYMBOLIC;
            ca->bg.color  = bg;
            ca->bg.source = COLORSOURCE_SYMBOLIC;
            ca->sf = 0;
        }
    color_valueclr(x_valueclr);
}


/*static*/ void
col_references(colors_t *colors)
{
    struct attribute *ap;
    unsigned i;

    for (i = 0, ap = colors->c_attr_table; i < colors->c_attr_count; ++i, ++ap) {
        switch (ap->ca_enum) {
        case COL_BACKGROUND:
            x_colors.c_background = &ap->ca_working;
            break;
        case COL_FOREGROUND:
            x_colors.c_normal     = &ap->ca_working;
            break;
        case COL_HILITE_BACKGROUND:
            x_colors.c_hilite_bg  = &ap->ca_working;
            break;
        case COL_HILITE_FOREGROUND:
            x_colors.c_hilite_fg  = &ap->ca_working;
            break;
        }
    }
}


/*  Function:           col_apply
 *      Apply the coloriser 'colors', preassigning defaults if desired.
 *
 *  Parameters:
 *      colors - Working colors.
 *
 *  Returns:
 *      TRUE if changes where detected, otherwise FALSE.
 */
/*static*/ int
col_apply(colors_t *colors)
{
    struct attribute *ap;
    int change = 0;
    unsigned i;

    trace_ilog("col_apply:\n");
    for (i = 0, ap = colors->c_attr_table; i < colors->c_attr_count; ++i, ++ap) {
        const int attr = ap->ca_attr;
        colattr_t val = ap->ca_working;

        ap->ca_value = val;
        if (attr >= 0) {
            colattr_t ca = col_build(colors, ap, &val);

            assert(attr <= ATTR_MAX);
            if (ca.sf != x_attrs[attr].sf ||
                    memcmp(&ca.fg, &x_attrs[attr].fg, sizeof(ca.fg)) ||
                    memcmp(&ca.bg, &x_attrs[attr].bg, sizeof(ca.bg))) {
                /*
                 *  attribute change
                 */
                char buf[ATTRIBUTES_WIDTH];

                col_export(colors, ap, &ca, buf, sizeof(buf));
                trace_ilog("  %-24s %4d [fg:%3d/%d, bg:%3d/%d, sf:%03x] = [fg:%3d/%d, bg:%3d/%d, sf:%03x] %s\n",
                    ap->ca_name, attr, val.fg.color, val.fg.source, val.bg.color, val.bg.source, val.sf,
                        ca.fg.color, ca.fg.source, ca.bg.color, ca.bg.source, ca.sf, buf);

                assert(ca.fg.source >= COLORSOURCE_NONE);   //>= SYMBOLIC
                assert(ca.fg.color  >= -1);                 //>= 0
                assert(ca.bg.source >= COLORSOURCE_NONE);   //>= SYMBOLIC
                assert(ca.bg.color  >= -1);                 //>= 0
                assert(ca.sf >= 0);

                x_attrs[ attr ].fg = ca.fg;
                x_attrs[ attr ].bg = ca.bg;
                x_attrs[ attr ].sf = ca.sf;
                ++change;
            }
        }
    }
    return change;
}


/*static*/ colattr_t
col_build(const colors_t *colors, const struct attribute *ap, const colattr_t *ca)
{
    colattr_t ret = {0};

    switch (ap->ca_attr) {
    case ATTR_HILITE:                           /* hilite */
        ret.fg = colors->c_hilite_fg->fg;
        ret.bg = colors->c_hilite_bg->fg;
        ret.sf = colors->c_hilite_fg->sf | colors->c_hilite_bg->sf;
        break;

    case ATTR_SHADOW:
        if (isclear(ca)) {                      /* default, based on background */
            if (colors->c_isdark) {
                ret.fg.color = BLACK;
                ret.bg.color = GREY;
            } else {
                ret.fg.color = GREY;
                ret.bg.color = BLACK;
            }
            ret.fg.source = COLORSOURCE_SYMBOLIC;
            ret.bg.source = COLORSOURCE_SYMBOLIC;
            return ret;
        }
        return *ca;

    default:
        if (ap->ca_links.cl_sticky > 0) {       /* sticky */
            assert(ap->ca_links.cl_sticky < ATTR_MAX);
            ret = x_attrs[ ap->ca_links.cl_sticky ];
        }

        if (isclear(&ret)) {
            if (CA_FG & ap->ca_flags) {         /* foreground + background */
                ret.fg = col_dynamic(colors, ca->fg);
                ret.bg = col_dynamic(colors, col_background(colors));
                ret.sf = ca->sf;
            } else {                            /* full color */
                ret.fg = col_dynamic(colors, ca->fg);
                ret.bg = col_dynamic(colors, ca->bg);
                ret.sf = ca->sf;
            }

            if (isclear(&ret)) {                /* redirect, then normal */
                int undef;

                if ((undef = ap->ca_links.cl_undef) <= 0) {
                    ret = x_attrs[ ATTR_NORMAL ];
                } else {
                    assert(undef < ATTR_MAX);
                    ret = x_attrs[ undef ];
                    if (isclear(&ret)) {
                        ret = x_attrs[ ATTR_NORMAL ];
                    }
                }
            }
        }

        if (isclear(&ret)) {                    /* default */
            ret.fg.color = (colors->c_isdark ? WHITE : BLACK);
            ret.fg.source = COLORSOURCE_SYMBOLIC;
            ret.bg = col_dynamic(colors, col_background(colors));
            ret.sf = 0;

        } else if (ret.bg.source == COLORSOURCE_NONE) {
            ret.bg = col_dynamic(colors, col_background(colors));
            if (COLORSOURCE_NONE == ret.bg.source) {
                ret.bg.color = (colors->c_isdark ? BLACK : WHITE);
                ret.bg.source = COLORSOURCE_SYMBOLIC;
            }
        }

        if (ret.fg.color == ret.bg.color) {
            switch (ret.fg.color) {             /* same, flip */
            case COLOR_NONE:
                break;
            case BLACK:
                ret.fg.color = WHITE;
                ret.fg.source = COLORSOURCE_SYMBOLIC;
                break;
            default:
                ret.fg.color = BLACK;
                ret.fg.source = COLORSOURCE_SYMBOLIC;
                break;
            }
        }

        if (COLORSOURCE_SYMBOLIC == ret.fg.source) {
            if (COLORSTYLE_BOLD & ret.sf) {     /* BOLD/DIM processing, BOLD has priority */
                if (ret.fg.color < 8) {
                    ret.fg.color += 8;
                }
                ret.sf &= ~(COLORSTYLE_BOLD|COLORSTYLE_DIM);
                ret.sf |=   COLORSTYLE_ISBOLD;

            } else if (COLORSTYLE_DIM & ret.sf) {
                if (ret.fg.color >= 8 && ret.fg.color < 16) {
                    ret.fg.color -= 8;
                }
                ret.sf &= ~COLORSTYLE_DIM;
                ret.sf |=  COLORSTYLE_ISDIM;
            }
        }
    }

    return ret;
}


/*static*/ colvalue_t
col_dynamic(const colors_t *colors, const colvalue_t val)
{
    switch (val.source) {
    case COLORSOURCE_NONE:
        break;

    case COLORSOURCE_SYMBOLIC:
        switch (val.color) {
        case COLOUR_DYNAMIC_FOREGROUND:
            return col_dynamic(colors, col_normal(colors));

        case COLOUR_DYNAMIC_BACKGROUND:
            return col_dynamic(colors, col_background(colors));

        default:
            assert(val.color >  COLOR_UNKNOWN);
            assert(val.color <= COLOR_NONE);
            break;
        }
        break;

    case COLORSOURCE_NUMERIC:
    case COLORSOURCE_COLOR:
        break;

    case COLORSOURCE_RGBLABEL:
    case COLORSOURCE_RGB: {
            colvalue_t nval = val;
            const int colordepth = vtcolordepth();

            if (colordepth <= 256) {
                /*
                 *  Convert to current color depth.
                 */
                const unsigned color = (unsigned)nval.color;
                struct rgbvalue rgb;

                rgb.type  = nval.type;
                rgb.red   = 0xff & (color);
                rgb.green = 0xff & (color >> 8);
                rgb.blue  = 0xff & (color >> 16);

                if (256 == colordepth) {
                    nval.color = rgb_xterm256(&rgb);
                } else if (colordepth >= 88) {
                    nval.color = rgb_xterm88(&rgb);
                } else {
                    nval.color = rgb_xterm16(&rgb);
                }
                nval.source = COLORSOURCE_RGBCVT;
                nval.type = (uint8_t)color;
            }
            return nval;
        }

    case COLORSOURCE_RGBCVT:
        break;
    }
    return val;
}


static int
isclear(const colattr_t *val)
{
    if (val->fg.source <= COLORSOURCE_NONE || val->fg.color < 0) {
        return TRUE;
    }
    return FALSE;
}


/*  Function:           attr_byname
 *      Lookup an attribute by name.
 *
 *  Parameters:
 *      colors - Color table.
 *      name - Attribute name.
 *      create - If *true* creating non-existent entries.
 *
 *  Returns:
 *      Attribute object.
 */
static struct attribute *
attr_byname(const colors_t *colors, const char *name, int create)
{
    return attr_bynname(colors, name, (int)strlen(name), create);
}


/*  Function:           attr_bynname
 *      Lookup an attribute by name length delimited, optionally create a new entry
 *      if it does not exist.
 *
 *  Parameters:
 *      colors - Color table.
 *      name - Attribute name.
 *      length - Buffer length.
 *      create - If *true* create non-existent entries.
 *
 *  Returns:
 *      Attribute object.
 */
static struct attribute *
attr_bynname(const colors_t *colors, const char *name, int length, int create)
{
    name = str_trim(name, &length);

    if (length) {                               /* search */
        const int trailing = (tolower(name[length - 1]) == 's' ? length - 1 : 0);
        struct attribute *ap;
        unsigned i;

        for (i = 0, ap = colors->c_attr_table; i < colors->c_attr_count; ++i, ++ap)
            if ((length == ap->ca_length &&
                    0 == str_nicmp(ap->ca_name, name, length)) ||
                (trailing && trailing == ap->ca_length &&
                    0 == str_nicmp(ap->ca_name, name, trailing))) {
                return ap;
            }
    }

    if (create) {                               /* create, if desired */
        colors_t *mutable_colors = (colors_t *)colors;
        collinks_t nulllks = COLLINKS_INIT;
        colattr_t nullca = COLATTR_INIT;
        struct attribute *ap = NULL;
        unsigned attr_alloc;

        if ((attr_alloc = mutable_colors->c_attr_alloc) >= mutable_colors->c_attr_count) {
            struct attribute *t_attributes;     /* expand table */

            attr_alloc += ATTRIBUTES_COUNT;
            if (NULL == (t_attributes =
                    chk_realloc(mutable_colors->c_attr_table, attr_alloc * sizeof(struct attribute)))) {
                return NULL;
            }

            mutable_colors->c_attr_alloc = attr_alloc;
            if (t_attributes != mutable_colors->c_attr_table) {
                mutable_colors->c_attr_table = t_attributes;
                col_references(mutable_colors);
            }
        }

        ap = mutable_colors->c_attr_table + mutable_colors->c_attr_count;
        ap->ca_name     = chk_salloc(name);
        ap->ca_length   = length;
        ap->ca_desc     = NULL;
        ap->ca_enum     = -1;
     // ap->ca_flags    = 0;                    // CA_ATTR;
        ap->ca_attr     = ATTR_DYNAMIC + mutable_colors->c_attr_dynamic++;
        ap->ca_links    = nulllks;
        ap->ca_value    = nullca;
        ap->ca_working  = nullca;
        return ap;
    }
    return NULL;
}


/*  Function:           attr_byenum
 *      Lookup an attribute by it coloriser enum value.
 *
 *  Parameters:
 *      colors - Color table.
 *      indent - Color enumeration.
 *
 *  Returns:
 *      Attribute object.
 */
static struct attribute *
attr_byenum(const colors_t *colors, int ident)
{
    struct attribute *ap;
    unsigned i;

    if (ident >= 0)
        for (i = 0, ap = colors->c_attr_table; i < colors->c_attr_count; ++i, ++ap)
            if (ap->ca_length && ap->ca_enum == ident) {
                return ap;
            }
    return NULL;
}


/*  Function:           attr_toname
 *      Lookup an attribute by it coloriser attribute value.
 *
 *  Parameters:
 *      colors - Color table.
 *      indent - Color enumeration.
 *
 *  Returns:
 *      Attribute object.
 */
static const char *
attr_toname(const colors_t *colors, int ident)
{
    struct attribute *ap;
    unsigned i;

    if (ident >= 0)
        for (i = 0, ap = colors->c_attr_table; i < colors->c_attr_count; ++i, ++ap)
            if (ap->ca_attr == ident) {
                return ap->ca_name;
            }
    return "none";
}


/*  Function:           col_prompt
 *      Prompt the user for a color specification.
 *
 *  Parameters:
 *      colors - Color definition.
 *      ap - Attribute object.
 *      ca - Returned color attribute.
 *
 *  Returns:
 *      Zero(0) on success, otherwise non-zero on error.
 *
 *      -1  - Abort.
 *      -2  - Skip.
 *      -3  - Range invalid.
 */
static int
col_prompt(const colors_t *colors, const struct attribute *ap, colattr_t *ca)
{
    const char *desc = (ap->ca_desc ? ap->ca_desc : ap->ca_name);
    const colattr_t oca = ap->ca_working;
    int colordepth = vtcolordepth();
    char prompt[MAX_CMDLINE], buf[MAX_CMDLINE] = {0};
    colattr_t ret = {0};

    assert(ap);
    assert(ca);

    if (CA_ATTR & ap->ca_flags) {
        /*
         *  foreground, [background]
         */
        int fg_done = 0;

        for (;;) {
            const int ocolor = (!fg_done ? oca.fg.color : oca.bg.color);
            const char *msg = "Enter '%s %s' color number <%u>:";
            const char *suffix = (!fg_done ? "foreground" : "background");
            colvalue_t val = COLVALUE_INIT;

            sxprintf(prompt, sizeof(prompt), msg, desc, suffix, (unsigned)ocolor);

            if (ereply(prompt, buf, sizeof(buf)) != TRUE) {
                return -1;                      /* abort */
            }

            if (0 == buf[0]) {
                val = col_background(colors);
                if (!fg_done || val.color < 0) {
                    return -2;                  /* skip */
                }

            } else {
                if (! color_value(buf, &val, fg_done) || val.color >= colordepth) {
                    errorf("color: invalid '%s' color: must be 0-%d.", desc, colordepth-1);
                    return -3;                  /* range/io error */
                }
            }

            /* set value */
            if (! fg_done) {
                ret.fg = val;
                fg_done = 1;
            } else {
                ret.bg = val;
                break;
            }
        }

    } else {
        /*
         *  primary
         *
         *      CA_FULL -   Background and foreground combined (hilite special).
         *      CA_FG -     Foreground.
         *      CA_BG -     Background.
         */
        const int full = (ap->ca_flags & CA_FULL);
        const int limit = (ap->ca_flags & CA_FULL) ? colordepth * colordepth : colordepth;
        const char *msg = (full ? "Enter '%s%s' color number <0x%x>:" : "Enter '%s%s' color number <%u>:");
        const char *suffix = (full || COL_BACKGROUND == ap->ca_enum ? "" :
                                (ap->ca_flags & CA_FG ? " foreground" : " background"));
        colvalue_t val = COLVALUE_INIT;

        sxprintf(prompt, sizeof(prompt), msg, desc, suffix, (unsigned)oca.fg.color);

        if (ereply(prompt, buf, sizeof(buf)) != TRUE) {
            return -1;                          /* abort */
        }

        if (0 == buf[0]) {
            return -2;                          /* next */
        }

        if (! color_value(buf, &val,  FALSE) || val.color > limit) {
            errorf("color: invalid '%s' color: must be 0-%d.", desc, limit - 1);
            return -3;                          /* range/io error */
        }

        ret.fg = val;
        ret.bg.color = COLOR_UNKNOWN;
        ret.bg.source = COLORSOURCE_NONE;
    }

    *ca = ret;                                  /* result */
    return 0;
}


/*  Function:           col_import
 *      Decode a color specification string.
 *
 *  Format:
 *      foreground [,background] [:style ...]
 *      clear
 *
 *  Parameters:
 *      colors - Color table.
 *      who - Caller label.
 *      ap - Attribute definition.
 *      spec - Specification.
 *      ca - Color attribute.
 *      links - Links (optional).
 *
 *  Returns:
 *      Zero(0) on success, otherwise -1.
 */
static int
col_import(const colors_t *colors, const char *who, const struct attribute *ap, const char *spec,
        colattr_t *ca, collinks_t *links)
{
    const char *comma, *colon;
    int ret = 0;

    comma = strchr(spec, ',');
    if (NULL != (colon = strchr(spec, ':'))) {
        if (comma > colon) {                    /* foreground:style,style */
            comma = NULL;
        }
    }

    if (comma || colon) {
        /*
         *      foreground [,background] [:style]
         *  or  color [:style]
         */
        const char *end = (comma ? comma : colon);
        colvalue_t fg = COLVALUE_INIT, bg = COLVALUE_INIT;
        int sf = 0;

        if (end == spec ||                      /* foreground */
                ! color_nvalue(spec, end - spec, &fg, FALSE)) {
            errorf("%s: invalid %scolor '%s'", who, (comma ? "foreground " : ""), spec);
            ret = -1;

        } else {
            if (comma && *++comma) {            /* ,background */
                if (0 == ((CA_ATTR|CA_FULL) & ap->ca_flags)) {
                    errorf("%s: unexpected background color '%s'", who, spec);
                    ret = -1;
                } else {
                    if ((NULL == colon && ! color_value(comma, &bg, TRUE)) ||
                            (NULL != colon && ! color_nvalue(comma, colon - comma, &bg, TRUE))) {
                        errorf("%s: invalid background color '%s'", who, spec);
                        ret = -1;
                    }
                }
            } else if (CA_ATTR & ap->ca_flags) {
                bg.color = COLOUR_DYNAMIC_BACKGROUND;
                bg.source = COLORSOURCE_SYMBOLIC;
            }

            if (0 == ret) {                     /* :style[,...] */
                if (links && colon && *++colon) {
                    if ((sf = style_import(colors, who, colon, links, ap->ca_attr)) < 0) {
                        ret = -1;
                    }
                }
            }

            if (0 == ret) {                     /* commit */
                ca->fg = fg;
                ca->bg = bg;
                ca->sf = sf;
            }
        }

    } else {
        /*
         *      clear, zero all value (link or NORMAL shall result)
         *  or  single color, apply default background
         */
        colvalue_t fg = COLVALUE_INIT;

        if (! color_value(spec, &fg, FALSE)) {
            if (0 == str_icmp(spec, "clear")) {
                ca->fg.color  = COLOR_UNKNOWN;
                ca->fg.source = COLORSOURCE_NONE;
                ca->bg.color  = COLOR_UNKNOWN;
                ca->bg.source = COLORSOURCE_NONE;
                ca->sf = 0;

            } else {
                errorf("%s: invalid color '%s'", who, spec);
                ret = -1;
            }

        } else {
            ca->fg = fg;
            ca->bg.color  = (CA_ATTR & ap->ca_flags ? COLOUR_DYNAMIC_BACKGROUND : COLOR_UNKNOWN);
            ca->bg.source = (CA_ATTR & ap->ca_flags ? COLORSOURCE_SYMBOLIC : COLORSOURCE_NONE);
            ca->sf = 0;
        }
    }

    if (0 == ret) {
        if (0 == (CA_ATTR & ap->ca_flags)) {
            switch (ca->fg.source) {
            case COLORSOURCE_RGBLABEL:
            case COLORSOURCE_RGB: {
                    const int colordepth = vtcolordepth();

                    if (colordepth < 88) {      /* base attributes, limit to 88 and 256 modes */
                        errorf("%s: base attributes do not support RGB '%s' in %d mode", who, spec, colordepth);
                        ca->fg.color = COLOR_UNKNOWN;
                        ca->fg.source = COLORSOURCE_NONE;
                        ret = -1;
                    }
                }
                break;
            default:
                break;
            }
        }

#if defined(TODO)
        if (0 == ret) {
            ca->id = macro_identifier();
        }
#endif
    }
    return ret;
}


/*  Function:           col_export
 *      Export a color specification string.
 *
 *  Format:
 *      attribute=foreground [,background] [:styles...] [sticky@] [link@]
 *
 *  Parameters:
 *      colors - Color table.
 *      ap - Attribute object.
 *      ca - Color value.
 *      buf - Destination buffer.
 *      len - Buffer length.
 *
 *  returns:
 *      Resulting buffer length.
 */
static int
col_export(const colors_t *colors, const struct attribute *ap, const colattr_t *ca, char *buf, int len)
{
    char delimiter = ':';
    int val, idx = 0;

    /*
     *      clear
     *  or  foreground [,background]
     *  or  color
     */
    if (CA_ATTR & ap->ca_flags) {
        if (isclear(ca)) {                      /* clear */
            idx = sxprintf(buf, len, "clear");
        } else {                                /* foreground [,background] */
            idx = color_print(&ca->fg, buf, len);
            if (ca->bg.color > COLOR_UNKNOWN) {
                buf[idx++] = ',';
                idx += color_print(&ca->bg, buf + idx, len - idx);
            }
        }
    } else {                                    /* color */
        idx = color_print(&ca->fg, buf, len);
    }

    /* [:style...] */
    if ((val = ca->sf) > 0) {
        int sidx = style_print(val, buf + idx, len - idx);
        if (sidx) {
            delimiter = ',';
            idx += sidx;
        }
    }

    /* links */
    if ((val = ap->ca_links.cl_sticky) > 0) {   /* sticky@ attribute */
        idx += sxprintf(buf + idx, len - idx, "%csticky@%s", delimiter, attr_toname(colors, val));
        delimiter = ',';
    }

    if ((val = ap->ca_links.cl_undef) > 0) {    /* link@ attribute */
        idx += sxprintf(buf + idx, len - idx, "%clink@%s", delimiter, attr_toname(colors, val));
        delimiter = ',';
    }

    return idx;
}


/*  Function:           color_value
 *      Map a color name to its value, see color_nvalue() for details
 *
 *  Parameters:
 *      name - Name buffer.
 *      val - Pointer to storage populated with color.
 *      bg - *true* if background, otherwise foreground.
 *
 *  Returns:
 *      TRUE on success, otherwise FALSE.
 */
static int
color_value(const char *name, colvalue_t *val, int bg)
{
    return color_nvalue(name, strlen(name), val, bg);
}


/*  Function:           color_nvalue
 *      Map a color name to its value, length delimited search.
 *
 *      The following formats are supported:
 *
 *>         rgb:<red>/<green>/<blue>            X11 RGB by value specification.
 *>         rgbi:<red>/<green>/<blue>           X11 RGB by percentage specification.
 *>         rgb(<red>, <green>, <blue)          WEB/CSS integer range 0 - max.
 *>         rgb(<red>%, <green>%, <blue>%)      WEB/CSS float range 0.0% - 100.0%.
 *>         #rrggbb                             RGB value.
 *>         color-<dec>                         where <dec> represents a color value.
 *>         color#<hex>                         where <hex> represents a color value.
 *>         0[X]xx                              Numeric valus (decimal, octal or hex).
 *>         <name>                              Symbolic name, sourced from 'color_labels'.
 *
 *  Parameters:
 *      name - Name buffer.
 *      length - Buffer length in bytes.
 *      val - Pointer to storage populated with color.
 *      bg - *true* if background, otherwise foreground.
 *
 *  Returns:
 *      TRUE on success and val is populated, otherwise FALSE.
 */
static int
color_nvalue(const char *name, int length, colvalue_t *val, int bg)
{
    colvalue_t ca = COLVALUE_INIT;

    __CUNUSED(bg)

    /*  Color values forms;
     *
     *      1.  0Xxx, 0ooo and ddd
     *      2.  #RRGGBB
     *      3.  color[#]xxx
     */
    assert(name && *name);
    assert(length > 0);
    name = str_trim(name, &length);
    ca = color_numeric(name, length);

    /*  Search;
     *      Standard colors
     *      and NONE
     */
    if ((COLOR_UNKNOWN == ca.color || COLORSOURCE_NONE == ca.source) && length >= 2) {
        const struct name *np;
        unsigned i;

        for (i = 0, np = x_color_names; i < COLOR_NAMES; ++i, ++np) {
            if (length == np->cn_length &&
                    0 == str_nicmp(np->cn_name, name, length)) {
                ca.color = np->cn_value;
                ca.source = COLORSOURCE_SYMBOLIC;
                break;
            }
        }
    }

    /*
     *  User definable 'color_labels' list, see colors.cr
     */
    if (COLOR_UNKNOWN == ca.color || COLORSOURCE_NONE == ca.source) {
        colvalue_t result;

        if (color_label(name, length, &result)) {
            ca = result;
        }
    }

    if (COLOR_UNKNOWN != ca.color) {
        if (val) *val = ca;
    }
    return (COLOR_UNKNOWN == ca.color ? FALSE : TRUE);
}


/*
 *  Iterate table, string + string
 *
 *      Key             Value
 *      snow            #FFFAFA
 *      ghost white     #F8F8FF
 *      GhostWhite      #F8F8FF
 *          :              :
 */
static const char *
color_label(const char *name, const int length_or_color, colvalue_t *result)
{
    colvalue_t val = COLVALUE_INIT;
    SYMBOL *sp;

    if (NULL != (sp = sym_global_lookup("color_labels")) && F_LIST == sp->s_type) {
        const LIST *nextlp, *lp = (const LIST *) r_ptr(sp->s_obj);
        const char *key;

        while (lp && (nextlp = atom_next(lp)) != lp) {
            if (NULL != (key = atom_xstr(lp))) {
                lp = nextlp;

                if ((nextlp = atom_next(lp)) != lp) {
                    int keylen = (int)strlen(key);

                    if (NULL != (key = str_trim(key, &keylen)) &&
                            (NULL == name || (keylen == length_or_color && 0 == str_icmp(key, name)))) {
                        const char *sval;
                        accint_t ival;

                        if (atom_xint(lp, &ival)) {
                            val.color = (int) ival;
                            val.source = COLORSOURCE_NUMERIC;

                        } else if (NULL != (sval = atom_xstr(lp))) {
                            int slen = (int)strlen(sval);

                            sval = str_trim(sval, &slen);
                            val = color_numeric(sval, slen);
                            if (COLORSOURCE_RGB == val.source) {
                                val.source = COLORSOURCE_RGBLABEL;
                            }

                        } else {
                            continue;           /* neither integer nor string */
                        }

                        if (NULL == name) {
                            if (length_or_color == val.color)  {
                                if (result) *result = val;
                                return key;
                            }
                            continue;
                        }

                        if (result) *result = val;
                        return name;
                    }
                }
            }
            lp = nextlp;
        }
    }
    return NULL;
}


static int
isterm(const char *endp)
{
    return (endp && (!*endp || strchr(",: ", *endp)) ? 1 : 0);
}


static colvalue_t
color_numeric(const char *name, unsigned length)
{
    colvalue_t val = COLVALUE_INIT;

    if (length) {
        char *endp = NULL;
        struct rgbvalue rgb = {0};
        int type;
                                                /* RGB (rgb:, rgbi:, #xxx) */
        if ((type = rgb_import(name, length, &rgb, 0xff)) >= 0) {

            val.source = COLORSOURCE_RGB;
            val.color = (int)(rgb.red | (rgb.green << 8) | (rgb.blue << 16));
            val.type = (unsigned char) type;

        } else if (length > 6 &&                /* color#<hex> */
                    0 == strncmp(name, "color#", 6) && isxdigit(name[6])) {

            val.color = (int) strtol(name + 6, &endp, 16);
            if (isterm(endp) && val.color >= 0 && val.color <= 0xff) {
                val.source = COLORSOURCE_COLOR;
                val.type = 16;
            }

        } else if (length > 5 &&                /* color-<dec> */
                    0 == strncmp(name, "color-", 6) && isdigit(name[6])) {

            val.color = (int) strtol(name + 6, &endp, 10);
            if (isterm(endp) && val.color >= 0 && val.color <= 0xff) {
                val.source = COLORSOURCE_COLOR;
                val.type = 10;
            }

        } else if (isxdigit(name[0])) {         /* oct, decimal or hex */

            val.color = (int) strtol(name, &endp, 0);
            if (isterm(endp) && val.color >= 0 && val.color <= 0xff) {
                val.source = COLORSOURCE_NUMERIC;
                val.type = 10;
                if ('0' == name[0]) {
                    if ('x' == name[1] || 'X' == name[0]) {
                        val.type = 16;
                    } else if (name[1]) {
                        val.type = 8;
                    }
                }
            }
        }
    }
    return val;
}


static int
color_print(const colvalue_t *val, char *buf, int len)
{
    switch (val->source) {
    case COLORSOURCE_NONE:
        return sxprintf(buf, len, "clear");

    case COLORSOURCE_SYMBOLIC:      /* <name> */
        return sxprintf(buf, len, "%s", color_name(val->color, "clear"));

    case COLORSOURCE_NUMERIC:       /* 0x<hex>, 0<oct> or <dec> */
        switch (val->type) {
        case 16:
            return sxprintf(buf, len, "0x%x", val->color);
        case 8:
            return sxprintf(buf, len, "0%o", val->color);
        default:
            break;
        }
        return sxprintf(buf, len, "%d", val->color);

    case COLORSOURCE_COLOR:         /* color#<hex> or color-<dec> */
        if (16 == val->type) {
            return sxprintf(buf, len, "color#%x", val->color);
        }
        return sxprintf(buf, len, "color-%d", val->color);

    case COLORSOURCE_RGBLABEL: {    /* %<label> */
            const char *label;

            if (NULL != (label = color_label(NULL, val->color, NULL))) {
                buf[0] = '%';
                strxcpy(buf+1, label, len-1);
                return (int)strlen(buf);
            }
        }
        /*FALLTHRU*/

    case COLORSOURCE_RGB: {         /* rgb(<def>) or #<rrggbb> */
            const unsigned color = (unsigned) val->color;
            struct rgbvalue rgb;

            rgb.type  = val->type;
            rgb.red   = 0xff & (color);
            rgb.green = 0xff & (color >> 8);
            rgb.blue  = 0xff & (color >> 16);
            return rgb_export(buf, len, &rgb, 0xff);
        }
        /*NOTREACHED*/

    case COLORSOURCE_RGBCVT: {      /* cvt(#<rrggbb>/<color>) */
            const unsigned rgb = (unsigned) val->type;

            return sxprintf(buf, len, "cvt(#%02x%02x%02x/0x%x)",
                (0xff & rgb), (0xff & (rgb >> 8)), (0xff & (rgb >> 16)), val->color);
        }
        /*NOTREACHED*/
    }
    assert(0);
    return 0;
}


/*  Function:           color_name
 *      Map a color 'value' to is standard name.
 *
 *  Parameters:
 *      value - Color value.
 *
 *  Returns:
 *      Static string containing the name, otherwise NULL.
 */
const char *
color_name(int value, const char *def)
{
    const struct name *np;
    unsigned i;

    for (i = 0, np = x_color_names; i < COLOR_NAMES; ++i, ++np)
        if (value == np->cn_value) {
            return np->cn_name;
        }
    return def;
}


/*  Function:           color_enum
 *      Map a color 'name' to is enumeration.
 *
 *  Parameters:
 *      name - Color name
 *      length - Length, in bytes.
 *
 *  Returns:
 *      Enumeration value, otherwise -1 if unknown.
 */
int
color_enum(const char *name, int length)
{
    if (name && length) {
        const struct name *np;
        unsigned i;

        for (i = 0, np = x_color_names; i < COLOR_NAMES; ++i, ++np) {
            if (length == np->cn_length &&
                    0 == str_nicmp(np->cn_name, name, length)) {
                return np->cn_value;
            }
        }
    }
    return -1;
}


/*  Function:           style_import
 *      Map a list of styles to their value, length delimited search.
 *
 *  Parameters:
 *      colors - Color table.
 *      who - Caller label.
 *      spec - Specification.
 *
 *  Returns:
 *      Style map, otherwise -1 (unknown style) or -2 (unknown link attribute) on error.
 */
static int
style_import(const colors_t *colors, const char *who, const char *spec, collinks_t *links, const int attr)
{
    const char *comma, *cursor = spec;
    int t_style, map = 0;

    while (NULL != (comma = strchr(cursor, ',')) || *cursor) {
        if ((t_style = (NULL == comma ?
                style_nvalue(cursor, strlen(cursor)) : style_nvalue(cursor, comma - cursor))) >= 0) {
            map |= t_style;

        } else {
            int islink = 0;

            if (NULL == links ||
                    (0 != str_nicmp(cursor, "sticky@", islink = 7) &&
                     0 != str_nicmp(cursor, "sticky=", islink = 7) &&
                     0 != str_nicmp(cursor, "link@",   islink = 5) &&
                     0 != str_nicmp(cursor, "link=",   islink = 5))) {
                errorf("%s: invalid style '%s'", who, spec);
                return -1;
            }
            cursor += islink;

            if (0 == str_nicmp(cursor, "none", 4)) {
                if (7 == islink) {
                    links->cl_sticky = -1;
                } else {
                    links->cl_undef = -1;
                }
            } else {
                struct attribute *linkap;

                if (NULL == (linkap = (NULL == comma ?
                        attr_byname(colors, cursor, FALSE) : attr_bynname(colors, cursor, comma - cursor, FALSE)))) {
                    errorf("%s: unknown link attribute '%s'.", who, spec);
                    return -2;
                }

                if (attr > 0 && linkap->ca_attr == attr) {
                    errorf("%s: link self reference '%s'.", who, spec);
                    return -2;
                }

                if (7 == islink) {
                    links->cl_sticky = linkap->ca_attr;
                    trace_ilog("\t\tsticky=%d (%s)\n", linkap->ca_attr, linkap->ca_name);
                } else {
                    links->cl_undef = linkap->ca_attr;
                    trace_ilog("\t\tlink  =%d (%s)\n", linkap->ca_attr, linkap->ca_name);
                }
            }
        }

        if (NULL == comma) break;
        cursor = comma + 1;
    }
    return map;
}


/*  Function:           style_nvalue
 *      Map a style name to its value, length delimited search.
 *
 *  Parameters:
 *      name - Name buffer.
 *      length - Buffer length in bytes.
 *
 *  Returns:
 *      Style index, otherwise -1.
 */
static int
style_nvalue(const char *name, int length)
{
    int ret = -1;

    if (NULL == (name = str_trim(name, &length)) || 0 == length) {
        ret = 0;                                /* empty */
    } else {
        unsigned i;

        for (i = 0; i < (unsigned)(sizeof(x_style_names)/sizeof(x_style_names[0])); ++i)
            if (length == x_style_names[i].sn_length &&
                    0 == str_nicmp(x_style_names[i].sn_name, name, length)) {
                ret = x_style_names[i].sn_value;
                break;
            }
    }
    return ret;
}


static int
style_print(int sf, char *buf, int length)
{
    int idx = 0;

    if (sf > 0) {
        char delimiter = ':';
        unsigned i;

        for (i = 0; idx < length && i < (unsigned)(sizeof(x_style_names)/sizeof(x_style_names[0])); ++i) {
            if (sf & x_style_names[i].sn_value) {
                idx += sxprintf(buf + idx, length - idx, "%c%s", delimiter, x_style_names[i].sn_name);
                delimiter = ',';
            }
        }
    }
    return idx;
}

/*end*/
