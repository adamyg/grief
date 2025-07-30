#ifndef GR_COLOR_H_INCLUDED
#define GR_COLOR_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_color_h,"$Id: color.h,v 1.16 2025/02/07 03:03:20 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: color.h,v 1.16 2025/02/07 03:03:20 cvsuser Exp $
 * Color configuration.
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

#include <edsym.h>

__CBEGIN_DECLS

#define ANSIFG_MAX              16
#define ANSIBG_MAX              8

#define ANSICOLOR_FG(col)       ((0x7f & col) >> 3)
#define ANSICOLOR_BG(col)       ((0x7f & col) & 0x07)
#define ANSICOLOR_MK(fg,bg)     (ATTR_ANSI128 + ((fg * ANSIBG_MAX) + bg))

#define COLVALUE_INIT           { COLOR_UNKNOWN, 0, COLORSOURCE_NONE, 0 }

enum {
/*--export--enum--*/
/*
 *  Attribute enumations:
 *
 *        0                     Current.
 *        1 ...                 System.
 *      200 ...                 User colors.
 *      220 ... 236             16 colors.
 *      256 ...                 ANSI 16x8.
 *      400 ...                 Dynamic.
 */
    ATTR_UNKNOWN = -1,                          /* special */
    ATTR_CURRENT = 0,                           /* special */

    ATTR_NORMAL = 1,
    ATTR_TITLE,
    ATTR_MESSAGE,
    ATTR_ERROR,
    ATTR_HILITE,
    ATTR_FRAME,

    ATTR_CURSOR,
    ATTR_CURSOR_INSERT,
    ATTR_CURSOR_OVERTYPE,
    ATTR_CURSOR_ROW,
    ATTR_CURSOR_COL,

    ATTR_SHADOW,                                /* window shadows */

    ATTR_PROMPT,                                /* prompt */
    ATTR_PROMPT_STANDOUT,                       /* elements which standout within prompt */
    ATTR_PROMPT_COMPLETE,                       /* auto-completion prompt component */

    ATTR_QUESTION,                              /* yes/no questions */
    ATTR_ECHOLINE,                              /* echo line */
    ATTR_STANDOUT,
    ATTR_HILITERAL,                             /* literal characters */

    ATTR_SCROLLBAR,                             /* window scrollbar */
    ATTR_SCROLLBAR_THUMB,                       /* scrollbar thumb */

    ATTR_COLUMN_STATUS,                         /* line status column */
    ATTR_COLUMN_LINENO,                         /* buffer line number column */
    ATTR_NONBUFFER,                             /* non-buffer text, for example [EOF] */

    ATTR_SEARCH,                                /* buffer search */
    ATTR_SEARCH_INC,
    ATTR_SEARCH_MATCH,

    ATTR_COMMENT,                               /* comment */
    ATTR_COMMENT_STANDOUT,                      /* special elements within comments */
    ATTR_TODO,                                  /* words within comments */

    ATTR_LINK,                                  /* external links/html/www */
    ATTR_TAG,                                   /* external tag */
    ATTR_ALERT,                                 /* warning/error conditions */

    ATTR_SPELL,                                 /* spell check */
    ATTR_SPELL_LOCAL,
    ATTR_SPELL_SPECIAL,

    ATTR_RULER,                                 /* buffer ruler */
    ATTR_RULER_MARGIN,
    ATTR_RULER_IDENT,
    ATTR_RULER_MARK,
    ATTR_RULER_COLUMN,

    ATTR_POPUP_NORMAL,                          /* popups' */
    ATTR_POPUP_HILITE,
    ATTR_POPUP_STANDOUT,
    ATTR_POPUP_FRAME,
    ATTR_POPUP_TITLE,

    ATTR_DIALOG_NORMAL,                         /* dialog's */
    ATTR_DIALOG_FOCUS,
    ATTR_DIALOG_GREYED,
    ATTR_DIALOG_HOTKEY_NORMAL,
    ATTR_DIALOG_HOTKEY_FOCUS,
    ATTR_DIALOG_HILITE,                         /* list-box selected element */
    ATTR_DIALOG_FRAME,
    ATTR_DIALOG_TITLE,
    ATTR_DIALOG_SCROLLBAR,
    ATTR_DIALOG_THUMB,

    ATTR_DIALOG_BUT_GREYED,
    ATTR_DIALOG_BUT_NORMAL,
    ATTR_DIALOG_BUT_FOCUS,
    ATTR_DIALOG_BUT_KEY_NORMAL,
    ATTR_DIALOG_BUT_KEY_FOCUS,

    ATTR_DIALOG_EDIT_GREYED,
    ATTR_DIALOG_EDIT_NORMAL,
    ATTR_DIALOG_EDIT_FOCUS ,
    ATTR_DIALOG_EDIT_COMPLETE,                  /* edit-field auto-completion component */

    ATTR_LSNORMAL,                              /* ls - normal files/text */
    ATTR_LSDIRECTORY,                           /* ls - directories */
    ATTR_LSSYMLINK,                             /* ls - symlinks */
    ATTR_LSEXECUTE,                             /* ls - executable */
    ATTR_LSPIPE,                                /* ls - pipes */
    ATTR_LSSPECIAL,                             /* ls - sockets/doors etc */
    ATTR_LSERROR,                               /* ls - bad links etc */
    ATTR_LSREADONLY,                            /* ls - read only */
    ATTR_LSATTRIBUTE,                           /* ls - file attribute */
    ATTR_LSSIZE,                                /* ls - file size */

    ATTR_MODIFIED,                              /* modified line */
    ATTR_ADDITIONAL,                            /* added line */
    ATTR_DIFFTEXT,                              /* diff - changed text within a modified line */
    ATTR_DIFFDELETE,                            /* diff - deleted line */

    ATTR_CODE,                                  /* code/statements */
    ATTR_CONSTANT,                              /* general constants */
    ATTR_CONSTANT_STANDOUT,                     /* special constant attributes */
    ATTR_STRING,                                /* string constants */
    ATTR_CHARACTER,                             /* character constants */
    ATTR_OPERATOR,
    ATTR_NUMBER,                                /* numeric */
    ATTR_FLOAT,                                 /* float-point */
    ATTR_DELIMITER,
    ATTR_WHITESPACE,                            /* tabs ... */
    ATTR_WORD,                                  /* other stuff/identifers */
    ATTR_BOOLEAN,                               /* boolean constants */

    ATTR_PREPROCESSOR,                          /* directives */
    ATTR_PREPROCESSOR_INCLUDE,                  /* #include */
    ATTR_PREPROCESSOR_DEFINE,                   /* #define */
    ATTR_PREPROCESSOR_CONDITIONAL,              /* conditional */
    ATTR_PREPROCESSOR_KEYWORD,                  /* keywords within preprocessor definitions */
    ATTR_PREPROCESSOR_WORD,                     /* identifiers etc */

    ATTR_KEYWORD,                               /* Language reserve keywords */
    ATTR_KEYWORD_FUNCTION,                      /* standard libary functions */
    ATTR_KEYWORD_EXTENSION,                     /* language extensions */
    ATTR_KEYWORD_TYPE,                          /* int, float ... */
    ATTR_KEYWORD_STORAGECLASS,                  /* const, extern, register ... */
    ATTR_KEYWORD_DEFINTION,                     /* function, struct ... */
    ATTR_KEYWORD_CONDITIONAL,                   /* if, else ... */
    ATTR_KEYWORD_REPEAT,                        /* while, for ... */
    ATTR_KEYWORD_EXCEPTION,                     /* throw, catch ... */
    ATTR_KEYWORD_DEBUG,                         /* assert */
    ATTR_KEYWORD_LABEL,                         /* labels */
    ATTR_KEYWORD_STRUCTURE,                     /* structure/class definitions */
    ATTR_KEYWORD_TYPEDEF,                       /* user defined types (needs tags) */

//TODO, see display.c
//  ATTR_ANSI_BOLD,                             /* ANSI bold */
//  ATTR_ANSI_UNDERLINE,                        /* ANSI underline */

/*--end--*/
    ATTR_USER           = 200,                  /* user attributes */
        ATTR_USER1 = ATTR_USER,
        ATTR_USER2,
        ATTR_USER3,
        ATTR_USER4,
        ATTR_USER5,
        ATTR_USER6,
        ATTR_USER7,
        ATTR_USER8,
        ATTR_USER9,
        ATTR_USER10,

    ATTR_WINDOW         = 220,                  /* borderless window backgrounds, 16 */
        ATTR_WINDOW1 = ATTR_WINDOW,
        ATTR_WINDOW2,
        ATTR_WINDOW3,
        ATTR_WINDOW4,
        ATTR_WINDOW5,
        ATTR_WINDOW6,
        ATTR_WINDOW7,
        ATTR_WINDOW8,
        ATTR_WINDOW9,
        ATTR_WINDOW10,
        ATTR_WINDOW11,
        ATTR_WINDOW12,
        ATTR_WINDOW13,
        ATTR_WINDOW14,
        ATTR_WINDOW15,
        ATTR_WINDOW16,

    ATTR_ANSI128        = 256,                  /* ANSI 16x8, 128 */
    ATTR_DYNAMIC        = 400,                  /* dynamic attribute base */
    ATTR_MAX            = 512,                  /* in truth 2047, yet limit to support 4 pages */

    ATTR_SYNTAX_MIN     = ATTR_CODE,
    ATTR_KEYWORD_MIN    = ATTR_KEYWORD,
    ATTR_KEYWORD_MAX    = ATTR_KEYWORD_TYPEDEF,
    ATTR_SYNTAX_MAX     = ATTR_KEYWORD_TYPEDEF,
};


typedef struct {
    int                 color;                  /* color value, -1 == undefined */
    unsigned            rgbcolor;               /* RGB */
    unsigned char       source;                 /* definition source (see below) */
    unsigned char       type;                   /* source specific type information */

#define COLORSOURCE_NONE        0               /* unknown/undefined */
#define COLORSOURCE_SYMBOLIC    1               /* standard symbolic name */
#define COLORSOURCE_NUMERIC     2               /* absolute numeric (0x<hex>, 0<oct> or <dec> */
#define COLORSOURCE_COLOR       3               /* color#xxx */
#define COLORSOURCE_RGB         4               /* #RRGGBB|rgb:<red>/<green>/<blue>|rgbi:<red><green><blue> */
#define COLORSOURCE_RGBLABEL    5               /* <name> == RGB */
#define COLORSOURCE_RGBCVT      6               /* converted RGB value to current color depth */

#define COLOR_RGB(_r, _g, _b)   (((unsigned)((unsigned char)(_r) << 0) | ((unsigned)(unsigned char)(_g) << 8) | ((unsigned)(unsigned char)(_b) << 16)))
#define COLOR_RVAL(_rgb)        (((_rgb) >> 0)  & 0xff)
#define COLOR_GVAL(_rgb)        (((_rgb) >> 8)  & 0xff)
#define COLOR_BVAL(_rgb)        (((_rgb) >> 16) & 0xff)

} colvalue_t;

typedef unsigned colstyles_t;

#define COLATTR_INIT            { COLVALUE_INIT, COLVALUE_INIT, 0 }

typedef struct {
    colvalue_t          bg;
    colvalue_t          fg;

#define COLORSTYLE_BOLD         0x0001
#define COLORSTYLE_DIM          0x0002
#define COLORSTYLE_STANDOUT     0x0004
#define COLORSTYLE_INVERSE      0x0008
#define COLORSTYLE_UNDERLINE    0x0010
#define COLORSTYLE_BLINK        0x0020
#define COLORSTYLE_ITALIC       0x0040
#define COLORSTYLE_REVERSE      0x0080

#define COLORSTYLE_UNDERMASK    0x1f00
#define COLORSTYLE_UNDERSTYLE(_a)   ((_a) & COLORSTYLE_UNDERMASK)
#define COLORSTYLE_UNDERSINGLE  0x0100
#define COLORSTYLE_UNDERDOUBLE  0x0200
#define COLORSTYLE_UNDERCURLY   0x0400
#define COLORSTYLE_UNDERDOTTED  0x0800
#define COLORSTYLE_UNDERDASHED  0x1000

#define COLORSTYLE_STRIKEOUT    0x2000
#define COLORSTYLE_ISBOLD       0x4000          /* BOLD has been applied */
#define COLORSTYLE_ISDIM        0x8000          /* DIM has been applied */
    colstyles_t         sf;
    int                 val;
} colattr_t;

extern const colattr_t *    color_definition(int attr, colattr_t *ca);
extern void                 color_valueset(int attr, int val);
extern void                 color_valueclr(int clr);

extern int                  color_setscheme(const char *scheme);
extern const char *         color_name(int ident, const char *def);
extern int                  color_enum(const char *name, size_t length);

extern int                  attribute_value(const char *name);
extern int                  attribute_new(const char *name, const char *spec);

__CEND_DECLS

#endif /*GR_COLOR_H_INCLUDED*/


