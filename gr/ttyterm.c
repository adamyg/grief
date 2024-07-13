#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyterm_c,"$Id: ttyterm.c,v 1.122 2024/07/13 17:14:09 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyterm.c,v 1.122 2024/07/13 17:14:09 cvsuser Exp $
 * TTY driver termcap/terminfo based.
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

//  #define ED_LEVEL 2

#include <editor.h>

#if !defined(USE_VIO_BUFFER) && !defined(DJGPP)

#if defined(__CYGWIN__)
#include <w32api/windows.h>                     /* GetConsoleOutputCP() */
#include <sys/utsname.h>                        /* uname */
#endif

#include <edtermio.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */

#if defined(_VMS)
#include <unixlib.h>
#include <unixio.h>
#include <dvidef.h>
#include <descrip.h>
#include <lib$routines.h>
#include <starlet.h>
#endif

#if defined(HAVE_LIBNCURSESW)
#if defined(HAVE_NCURSESW_CURSES_H)
#   include <ncursesw/curses.h>
#   include <ncursesw/termcap.h>
#   include <ncursesw/term.h>
#elif defined(HAVE_NCURSESW_H)
#   include <ncursesw.h>
#   if defined(HAVE_TERMCAP_H)
#       include <termcap.h>
#   endif
#   if defined(HAVE_TERM_H)
#       include <term.h>
#   endif
#elif defined(HAVE_NCURSES_CURSES_H)
#   include <ncurses/curses.h>
#   include <ncurses/termcap.h>
#   include <ncurses/term.h>
#elif defined(HAVE_NCURSES_H)
#   include <ncurses.h>
#   if defined(HAVE_TERMCAP_H)
#       include <termcap.h>
#   endif
#   if defined(HAVE_TERM_H)
#       include <term.h>
#   endif
#else  /*!HAVE_NCURSESW_CURSES_H || HAVE_NCURSESW_H*/
#error "HAVE_LIBNCURSEW defined yet missing headers, check config"
#endif

#elif defined(HAVE_LIBNCURSES)
#if defined(HAVE_NCURSES_CURSES_H)
#   include <ncurses/curses.h>
#   include <ncurses/termcap.h>
#   include <ncurses/term.h>
#elif defined(HAVE_NCURSES_H)
#   include <ncurses.h>
#   if defined(HAVE_TERMCAP_H)
#       include <termcap.h>
#   endif
#   if defined(HAVE_TERM_H)
#       include <term.h>
#   endif
#else  /*!HAVE_NCURSES_CURSES_H || HAVE_NCURSES_H*/
#error "HAVE_LIBNCURSE defined yet missing headers, check config"
#endif

#elif defined(HAVE_LIBCURSES)
#if defined(HAVE_CURSES_H)
#include <curses.h>
#if defined(HAVE_TERMCAP_H)
#include <termcap.h>
#endif
#if defined(HAVE_TERM_H)
#if defined(HAVE_TERMIO_H)
#include <termio.h>                             /* solaris (SGTTY definition) */
#endif
#include <term.h>
#else                                           /* missing prototypes */
extern char *           tgetstr(char *, char **);
extern char *           tgoto(const char * , int a, int b);
extern char *           tparm(const char *, ...);
#endif
#else  /*!HAVE_CURSES_H*/
#error "HAVE_LIBCURSES defined yet missing headers, check config"
#endif

#elif defined(HAVE_LIBTERMCAP)
#if defined(HAVE_TERMCAP_H)
#include <termcap.h>
#else  /*HAVE_TERMCAP_H*/
#error "HAVE_LIBTERMCAP defined yet missing headers, check config"
#endif

#elif defined(HAVE_LIBTERMLIB)
#include <edtermcap.h>
#define HAVE_TERMCAP

#else
#include "edtermcap.h"                          /* use local implementation -- BAD */
#endif

#if defined(HAVE_TERMINFO) && defined(HAVE_TERMCAP)
#define XF_TERMCAP      if (xf_termcap)         /* dynamic selection */
#define XF_TERMINFO     if (! xf_termcap)
#else
#if !defined(HAVE_TERMINFO) && !defined(HAVE_TERMCAP)
#error "no terminal interface style defined (TERMINFO nor TERMCAP), check cofig"
#endif
#define XF_TERMCAP
#define XF_TERMINFO
#endif

#if !defined(HAVE_OSPEED) && !defined(__CYGWIN__) && !defined(sun) && \
        defined(OSPEED_EXTERN)                  /* ospeed available? (diag only) */
#define HAVE_OSPEED
extern int ospeed;
#endif

#if defined(HAVE_OUTFUNTYPE)                    /* tputs() interface */
#define TPUTS_OUTFUNC   (outfuntype)
#define TPUTS_OUTTYPE   int
#elif defined(TPUTS_TAKES_CHAR)
#define TPUTS_OUTFUNC   (int (*)(char))
#define TPUTS_OUTTYPE   char
#else
#define TPUTS_OUTFUNC   (int (*)(int))
#define TPUTS_OUTTYPE   int
#endif

#include <edalt.h>                              /* needs to be after [n]curses.h */
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "accum.h"                              /* acc_ ... */
#include "asciidefs.h"
#include "cmap.h"                               /* cmap ... */
#include "color.h"                              /* color ... */
#include "debug.h"                              /* trace ... */
#include "display.h"
#include "echo.h"
#include "eval.h"                               /* get_str() ... */
#include "getkey.h"                             /* io_... */
#include "keyboard.h"
#include "line.h"
#include "main.h"
#include "map.h"
#include "mchar.h"                              /* mchar_...() */
#include "mouse.h"
#include "playback.h"
#include "procspawn.h"
#include "system.h"
#include "tty.h"
#include "undo.h"
#include "window.h"

#define ANSI_COLORS             16
#define XTERM_COLORS            16
#define NOCOLOR                 0x7fff

typedef struct {
    const char *        termfname;              /* function name */
    unsigned            userdef;                /* is userdef attribute */
    const char *        termcapname;            /* termcap name */
    const char *        terminfoname;           /* terminfo name */
    const char *        comment;                /* comment string */
#define TC_FNAME(__fname)       #__fname, 0
#define TC_UNAME(__uname)       "userdef_" #__uname, 1
#define TC_KNAME(__kname)       "keydef_" #__kname, 1
#define TC_DESC(__desc)         __desc
} Term_t;

typedef struct {
    Term_t              term;
    int                 key;
#define TC_TOKEN(__token)       __token
    const char *        svalue;                 /* runtime value */
} TermKey_t;

typedef struct {
    Term_t              term;
    const char **       stoken;
#define TC_STRING(__token)      __token
    const char *        svalue;                 /* runtime value */
} TermString_t;

typedef struct {
    Term_t              term;
    int *               itoken;
#define TC_FLAG(__token)        __token
    int                 ivalue;                 /* runtime value*/
} TermNumeric_t;

typedef uint32_t TAttributes_t;

static void             term_open(scrprofile_t *profile);
static void             term_ready(int repaint, scrprofile_t *profile);
static void             term_feature(int ident, scrprofile_t *profile);
static void             term_display(void);
static int              term_control(int action, int param, ...);
static void             term_close(void);

static int              isterm(const char *term, const char *what);
static int              hasfeature(const char *term, const char *what);

static const char *     ttisetup(void);
static const char *     ttiname(const Term_t *ti);
static const char *     ttigetstr(const Term_t *ti);
static int              ttigetnum(const Term_t *ti);
static int              ttigetflag(const Term_t *ti);

static TAttributes_t    term_xtermlike(const char *term);

static void             acs_dump(const char *bp);
static const char *     acs_box_characters(const char *bp);
static int              acs_locale_breaks(void);

static void             term_tidy(void);
static void             term_move(int row, int col);
static int              term_cursor(int visible, int imode, int virtual_space);
static int              term_names(const char *title, const char *icon);
static void             term_beep(int freq, int duration);

static void             term_config(void);
static void             term_colors(void);
static void             term_fgbg(void);
static const char *     fgbg_value(const char *src, int *result);
static unsigned         fgbg_import(unsigned edefault, int udefault);

static void             xterm_colors(const char *value);
static int              xterm_colors_get(char *buffer, int length);
static int              xterm_colors_set(const char *value);

static int              term_isutf8(void);
static int              term_attributes(void);
static int              term_identification(void);
static int              term_ocs_color(int code);
static int              term_read(char *buf, int blen, accint_t tmo);

static __CINLINE void   term_graphic_enter(void);
static __CINLINE void   term_graphic_exit(void);

static void             term_clear(void);
static void             term_zero(int restore);
static void             term_flush(void);
static void             term_sizeget(int *nrow, int *ncol);

static void             term_putc(vbyte_t c);
static int              term_eeol(void);
static int              term_ce(void);
static int              term_lastsafe(void);
static void             term_eeop(int restore);
static void             term_repeat(int cnt, vbyte_t fill, int where);
static int              term_insl(int row, int bot, int nlines, vbyte_t fillcolor);
static int              term_dell(int row, int bot, int nlines, vbyte_t fillcolor);
static void             term_scrollset(int top, int bot);
static void             term_scrollreset(void);
static void             term_attr(vbyte_t color);
static void             term_styleon(unsigned mode);
static void             term_styleoff(void);
static void             term_colorreset(void);
static int              term_cost(const char *s);

static void             ttputpad(const char *str);
static void             ttputctl(const char *str, int a);
static void             ttputctl2(const char *str, int a, int b);
static void             ttprintf(const char *str, ...);
static void             ttprintf2(const char *str, ...);

static void             ega_switch(int flag);

/*
 *  This is the output buffer. NOBUF is how many bytes we can put in the buffer. We
 *  leave a lot of room for expansion because a single character may cause us to
 *  output multiple characters (e.g. printing top bit or graphics characters).
 */
#define NOOVFLOW                100
#define NOBUF                   ((8 * 1024) - NOOVFLOW)

static unsigned         t_count;
static unsigned char    t_buffer[NOBUF + NOOVFLOW];

/*
 *  Termcap buffer
 */
#define TC_SLEN                 (1024 * 32)     /* 32k - tgetent wants >= 16k */

#if defined(HAVE_TERMCAP)
static char *           tcapcursor;
static char             tcapbuffer[TC_SLEN];    /* termcap buffer */
static char             tcapstrings[TC_SLEN];   /* termcap local storage */
#endif

/*
 *  Terminate attributes
 */
#define TA_XTERM                0x00000001
#define TA_VT100LIKE            0x00000002
#define TA_XTERMLIKE            0x00000004
#define TA_LINUX                0x00010000
#define TA_CYGWIN               0x00020000
#define TA_KONSOLE              0x00040000
#define TA_SCREEN               0x00080000
#define TA_MINTTY               0x00100000
#define TA_PUTTY                0x00200000
#define TA_MSTERMINAL           0x00400000
#define TA_GNOME                0x01000000
#define TA_KITTY                0x02000000
#define TA_ALACRITTY            0x04000000
#define TA_ITERM                0x08000000
#define TA_HTERM                0x10000000
#define TA_WEZTERM              0x20000000
#define TA_DARK                 0x00000010      /* generally dark background */
#define TA_LIGHT                0x00000020      /* generally light */
#define TA_MONO                 0x00000040      /* mono terminal */
#define TA_LUMINACE_DARK        0x00000100
#define TA_LUMINACE_LIGHT       0x00000200

static TAttributes_t    t_attributes;           /* attributes */
static int              t_charout;              /* number of characters output. */
static unsigned         t_insdel;               /* do we have both insert & delete line? */
static int              t_padchar = -1;         /* pad character (if any) */
static int              t_acs_locale_breaks;

/*
 *  Terminate status
 */
static unsigned         t_gmode;                /* graphics character set active */
static int              t_cost;                 /* terminal costing accumulator */
static unsigned         t_specials;             /* special bits */

/*
 *  Color control
 */
static char             t_colorsorg[XTERM_COLORS * 32];
static char             t_colorsuser[XTERM_COLORS * 32];

/*
 *  Cached termap controls
 *
 *      References:
 *          ANSI Standard X3.64-1979.
 *          X/Open System Interface Definitions, Issue 4, Version 2.
 *
 *          http://www.xfree86.org/current/ctlseqs.html
 *          http://invisible-island.net/ncurses/#download_database
 *          http://en.wikipedia.org/wiki/ANSI_escape_code
 */
static int
    tf_gn,                                      /* generic terminal. */
    tf_hc,                                      /* hard-copy terminal. */
    tf_npc,                                     /* no pad character. */
    tf_xonoff,                                  /* xon/xoff required. */
    tf_am,                                      /* has auto-margins. */
    tf_xn,                                      /* the cursor wraps in a strange way. */
    tf_hz,                                      /* no hazel line support. */
    tf_km,                                      /* meta key sets msb */
    tf_LP,                                      /* last column safe. */
    tf_NL,                                      /* move down using \n. */
    tf_bs,                                      /* move left using ^H. */
    tf_Colors,                                  /* max colors */
    tf_Pairs,                                   /* max color-pairs */
    tf_ut,                                      /* screen erased with background color. */
    tf_be,                                      /* back color erase (xterm). */
    tf_xs, tf_xt,                               /* attribute clear mode */
    tf_ms,                                      /* save to move cursor whilst standout/underlined */
    tf_XT;                                      /* supports xterm OCS and mouse */

static int
    tn_sg,                                      /* number of glitches, 0 for invisable, -1 for none */
    tn_li, tn_co,                               /* lines/columns */
    tn_NC;                                      /* attributes which dont mix with colors */

static const char
    *tc_ti,                                     /* Term init -- start using cursor motion. */
    *tc_te,                                     /* Term end --- end using cursor motion. */
    *tc_ke,                                     /* Enter keypad mode. */
    *tc_ks,                                     /* Exit keypad mode. */
    *tc_rs,                                     /* Reset sequence. */
    *tc_am_on,                                  /* Turn on automatic margins. */
    *tc_bl,                                     /* Bell. */

    *tc_cm,                                     /* Cursor motion. */
    *tc_ho,                                     /* Home cursor. */
    *tc_ll,                                     /* Left-end cursor. */
    *tc_cr,                                     /* Cursor newline. */
    *tc_up,                                     /* Cursor up on line. */
    *tc_pUP,                                    /* Cursor up on line - parameterised. */
    *tc_do,                                     /* Cursor down. */
    *tc_pDO,                                    /* Cursor down - parameterised. */
    *tc_le,                                     /* Cursor left (new style). */
    *tc_bc,                                     /* Cursor left (old style). */
    *tc_pLE,                                    /* Cursor left - parameterised. */
    *tc_nd,                                     /* Cursor right. */
    *tc_pRI,                                    /* Cursor right - parameterised. */
    *tc_cv,                                     /* Cursor vertical movement. */
    *tc_ch,                                     /* Cursor horz movement. */

    *tc_vs,                                     /* Enhance the cursor. */
    *tc_ve,                                     /* Normal cursor. */
    *tc_vb,                                     /* Visual bell. */
    *tc_vi,                                     /* Hide cursor. */

    *tc_cs,                                     /* Set scroll region. */
    *tc_cS,                                     /* Set scroll region, alt form (see Emacs). */
    *tc_sf,                                     /* Forward index (used with scroll region). */
    *tc_sr,                                     /* Back index (used with scroll region). */
    *tc_pSF,                                    /* Forward index */
    *tc_pSR,                                    /* Back index */

    *tc_al,                                     /* Add line */
    *tc_dl,                                     /* Delete line */
    *tc_pAL,                                    /* Parameterized add line. */
    *tc_pDL,                                    /* Parameterized delete line. */

    *tc_pc,                                     /* Padding character */
    *tc_dc,                                     /* Delete a character. */
    *tc_pDC,                                    /* Parameterized character delete. */
    *tc_ic,                                     /* Insert a single space. */
    *tc_cb,                                     /* Clear to beginning of line. */
    *tc_cd,                                     /* Clear to end of display. */
    *tc_ce,                                     /* Clear to end of line. */
    *tc_cl,                                     /* Clear screen. */
    *tc_rp,                                     /* repeat #1 #2 times */
    *tc_ech,                                    /* erase #1 characters */
    *tc_im,                                     /* Insert mode. */
    *tc_ei,                                     /* End insert mode. */

    *tc_oc, *tc_op,                             /* Reset colors. */
    *tc_me,                                     /* Turn off all attributes. */
    *tc_mm, *tc_mo,                             /* Turn on/off meta mode (8th-bit on). */
    *tc_so, *tc_se,                             /* Start/end standout mode. */
    *tc_us, *tc_ue,                             /* Turn on/off underline mode. */
    *tc_mb,                                     /* Turn on blinking. */
    *tc_md,                                     /* Make bold. */
    *tc_ZH, *tc_ZR,                             /* Italic (on/off). */
    *tc_mr, *tc_ZX,                             /* Reverse (on/off). */
    *tc_sa,                                     /* Attribute set. */

    *tc_RV;                                     /* XTerm version */

static const char
    *tc_acs_start,                              /* as */
    *tc_acs_end,                                /* ae */
    *tc_acs_enable;                             /* enable ACS */

static unsigned char                            /* ac */
    tc_acs_map[129];

static const char tc_graphic_default[] =
    "a:j+k+l+m+q-t+u+v+w+x|n+`+f\\g#~o,<+>.v-^h#0#";

static const char
    *tc_graphic_pairs,
    *tc_box_characters;

static const char
    *tc_ANSI_Color_Fg, *tc_Color_Fg,            /* color selection */
    *tc_ANSI_Color_Bg, *tc_Color_Bg;

typedef struct {
    char                ident;                  /* internal identifier */
    const char *        name;                   /* desc */
} GraphicChars_t;

enum {
    /*
     *  ACC mappings, includes ncurse's extensions
     */
    TACS_STERLING       = '}',
    TACS_DARROW         = '.',
    TACS_LARROW         = ',',
    TACS_RARROW         = '+',
    TACS_UARROW         = '-',
    TACS_BOARD          = 'h',
    TACS_BULLET         = '~',
    TACS_CKBOARD        = 'a',
    TACS_DEGREE         = 'f',
    TACS_DIAMOND        = '`',
    TACS_GEQUAL         = 'z',
    TACS_PI             = '{',
    TACS_HLINE          = 'q',
    TACS_LANTERN        = 'i',
    TACS_PLUS           = 'n',
    TACS_LEQUAL         = 'y',
    TACS_LLCORNER       = 'm',
    TACS_LRCORNER       = 'j',
    TACS_NEQUAL         = '|',
    TACS_PLMINUS        = 'g',
    TACS_S1             = 'o',
    TACS_S3             = 'p',
    TACS_S7             = 'r',
    TACS_S9             = 's',
    TACS_BLOCK          = '0',
    TACS_TTEE           = 'w',
    TACS_RTEE           = 'u',
    TACS_LTEE           = 't',
    TACS_BTEE           = 'v',
    TACS_ULCORNER       = 'l',
    TACS_URCORNER       = 'k',
    TACS_VLINE          = 'x'
    };

static GraphicChars_t term_characters[] =  {    /* graphic characters */
    { TACS_STERLING,        TC_DESC("UK pound sign") },
    { TACS_DARROW,          TC_DESC("arrow pointing down") },
    { TACS_LARROW,          TC_DESC("arrow pointing left") },
    { TACS_RARROW,          TC_DESC("arrow pointing right") },
    { TACS_UARROW,          TC_DESC("arrow pointing up") },
    { TACS_BOARD,           TC_DESC("board of squares") },
    { TACS_BULLET,          TC_DESC("bullet") },
    { TACS_CKBOARD,         TC_DESC("checker board (stipple)") },
    { TACS_DEGREE,          TC_DESC("degree symbol") },
    { TACS_DIAMOND,         TC_DESC("diamond") },
    { TACS_GEQUAL,          TC_DESC("greater-than-or-equal-to") },
    { TACS_PI,              TC_DESC("greek pi") },
    { TACS_HLINE,           TC_DESC("horizontal line") },
    { TACS_LANTERN,         TC_DESC("lantern symbol") },
    { TACS_PLUS,            TC_DESC("large plus or crossover") },
    { TACS_LEQUAL,          TC_DESC("less-than-or-equal-to") },
    { TACS_LLCORNER,        TC_DESC("lower left corner") },
    { TACS_LRCORNER,        TC_DESC("lower right corner") },
    { TACS_NEQUAL,          TC_DESC("not-equal") },
    { TACS_PLMINUS,         TC_DESC("plus/minus") },
    { TACS_S1,              TC_DESC("scan line 1") },
    { TACS_S3,              TC_DESC("scan line 3") },
    { TACS_S7,              TC_DESC("scan line 7") },
    { TACS_S9,              TC_DESC("scan line 9") },
    { TACS_BLOCK,           TC_DESC("solid square block") },
    { TACS_TTEE,            TC_DESC("tee pointing down") },
    { TACS_RTEE,            TC_DESC("tee pointing left") },
    { TACS_LTEE,            TC_DESC("tee pointing right") },
    { TACS_BTEE,            TC_DESC("tee pointing up") },
    { TACS_ULCORNER,        TC_DESC("upper left corner") },
    { TACS_URCORNER,        TC_DESC("upper right corner") },
    { TACS_VLINE,           TC_DESC("vertical line") }
    };

static TermString_t term_strings[] = {          /* strings - termcap/terminfo elements */
    /*
     *  Source: https://invisible-island.net/ncurses/man/terminfo.5.html
     */
    { TC_FNAME(back_tab),                   "bt", "cbt",        TC_DESC("back tab") },
    { TC_FNAME(bell),                       "bl", "bel",        TC_DESC("audible signal (bell)"), TC_STRING(&tc_bl) },
    { TC_FNAME(carriage_return),            "cr", "cr",         TC_DESC("carriage return"), TC_STRING(&tc_cr) },
    { TC_FNAME(change_scroll_region),       "cs", "csr",        TC_DESC("change region to line #1 to line #2"), &tc_cs },
    { TC_FNAME(na),                         "cS", NULL,         TC_DESC("change region to line #1 to line #2, alt form"), &tc_cS },
    { TC_FNAME(clear_all_tabs),             "ct", "tbc",        TC_DESC("clear all tab stops") },
    { TC_FNAME(clear_screen),               "cl", "clear",      TC_DESC("clear screen and home cursor"), &tc_cl },
    { TC_FNAME(clr_eol),                    "ce", "el",         TC_DESC("clr eol"), &tc_ce },
    { TC_FNAME(clr_eos),                    "cd", "ed",         TC_DESC("clear to end of screen"), &tc_cd },
    { TC_FNAME(column_address),             "ch", "hpa",        TC_DESC("horizontal position #1, absolute"), &tc_ch },
    { TC_FNAME(command_character),          "CC", "cmdch",      TC_DESC("terminal settable cmd character in prototype !?") },
    { TC_FNAME(cursor_address),             "cm", "cup",        TC_DESC("move to row #1 columns #2"), &tc_cm },
    { TC_FNAME(cursor_down),                "do", "cud1",       TC_DESC("down one line"), &tc_do },
    { TC_FNAME(cursor_home),                "ho", "home",       TC_DESC("home cursor (if no cup)"), &tc_ho },
    { TC_FNAME(cursor_invisible),           "vi", "civis",      TC_DESC("make cursor invisible"), &tc_vi },
    { TC_FNAME(cursor_left),                "le", "cub1",       TC_DESC("move left one space"), &tc_le },
    { TC_FNAME(cursor_mem_address),         "CM", "mrcup",      TC_DESC("memory relative cursor addressing") },
    { TC_FNAME(cursor_normal),              "ve", "cnorm",      TC_DESC("make cursor appear normal (undo civis/cvvis)"), &tc_ve },
    { TC_FNAME(cursor_right),               "nd", "cuf1",       TC_DESC("move right one space"), &tc_nd },
    { TC_FNAME(cursor_to_ll),               "ll", "ll",         TC_DESC("last line, first column (if no cup)"), &tc_ll },
    { TC_FNAME(cursor_up),                  "up", "cuu1",       TC_DESC("up one line"), &tc_up },
    { TC_FNAME(cursor_visible),             "vs", "cvvis",      TC_DESC("make cursor very visible"), &tc_vs },
    { TC_FNAME(delete_character),           "dc", "dch1",       TC_DESC("delete character"), &tc_dc },
    { TC_FNAME(delete_line),                "dl", "dl1",        TC_DESC("delete line"), &tc_dl },
    { TC_FNAME(dis_status_line),            "ds", "dsl",        TC_DESC("disable status line") },
    { TC_FNAME(down_half_line),             "hd", "hd",         TC_DESC("half a line down") },
    { TC_FNAME(enter_alt_charset_mode),     "as", "smacs",      TC_DESC("enter alt charset mode"), &tc_acs_start },
    { TC_FNAME(enter_blink_mode),           "mb", "blink",      TC_DESC("turn on blinking"), &tc_mb },
    { TC_FNAME(enter_bold_mode),            "md", "bold",       TC_DESC("turn on bold (extra bright) mode"), &tc_md },
    { TC_FNAME(enter_ca_mode),              "ti", "smcup",      TC_DESC("string to start programs using cup"), &tc_ti },
    { TC_FNAME(enter_delete_mode),          "dm", "smdc",       TC_DESC("enter delete mode") },
    { TC_FNAME(enter_dim_mode),             "mh", "dim",        TC_DESC("turn on half-bright mode") },
    { TC_FNAME(enter_insert_mode),          "im", "smir",       TC_DESC("enter insert mode"), &tc_im },
    { TC_FNAME(enter_secure_mode),          "mk", "invis",      TC_DESC("turn on blank mode (characters invisible)") },
    { TC_FNAME(enter_protected_mode),       "mp", "prot",       TC_DESC("turn on protected mode") },
    { TC_FNAME(enter_reverse_mode),         "mr", "rev",        TC_DESC("turn on reverse video mode"), &tc_mr },
    { TC_FNAME(enter_standout_mode),        "so", "smso",       TC_DESC("begin standout mode"), &tc_so },
    { TC_FNAME(enter_underline_mode),       "us", "smul",       TC_DESC("begin underline mode"), &tc_us },
    { TC_FNAME(erase_chars),                "ec", "ech",        TC_DESC("erase #1 characters"), &tc_ech },
    { TC_FNAME(exit_alt_charset_mode),      "ae", "rmacs",      TC_DESC("exit alt charset mode"), &tc_acs_end },
    { TC_FNAME(exit_attribute_mode),        "me", "sgr0",       TC_DESC("turn off all attributes"), &tc_me },
    { TC_FNAME(exit_ca_mode),               "te", "rmcup",      TC_DESC("strings to end programs using cup"), &tc_te },
    { TC_FNAME(exit_delete_mode),           "ed", "rmdc",       TC_DESC("end delete mode") },
    { TC_FNAME(exit_insert_mode),           "ei", "rmir",       TC_DESC("exit insert mode"), &tc_ei },
    { TC_FNAME(exit_standout_mode),         "se", "rmso",       TC_DESC("exit standout mode"), &tc_se },
    { TC_FNAME(exit_underline_mode),        "ue", "rmul",       TC_DESC("exit underline mode"), &tc_ue },
    { TC_FNAME(flash_screen),               "vb", "flash",      TC_DESC("visible bell (may not move cursor)"), &tc_vb },
    { TC_FNAME(form_feed),                  "ff", "ff",         TC_DESC("hardcopy terminal page eject") },
    { TC_FNAME(from_status_line),           "fs", "fsl",        TC_DESC("return from status line") },
    { TC_FNAME(init_1string),               "i1", "is1",        TC_DESC("initialization string #1") },
    { TC_FNAME(init_2string),               "is", "is2",        TC_DESC("initialization string #2") },
    { TC_FNAME(init_3string),               "i3", "is3",        TC_DESC("initialization string #3") },
    { TC_FNAME(init_file),                  "if", "if",         TC_DESC("name of initialization file") },
    { TC_FNAME(insert_character),           "ic", "ich1",       TC_DESC("insert character"), &tc_ic },
    { TC_FNAME(insert_line),                "al", "il1",        TC_DESC("insert line"), &tc_al },
    { TC_FNAME(insert_padding),             "ip", "ip",         TC_DESC("insert padding after inserted character") },
    { TC_FNAME(keypad_local),               "ke", "rmkx",       TC_DESC("leave 'keyboard_transmit' mode"), &tc_ke },
    { TC_FNAME(keypad_xmit),                "ks", "smkx",       TC_DESC("enter 'keyboard_transmit' mode"), &tc_ks },
    { TC_FNAME(lab_f0),                     "l0", "lf0",        TC_DESC("label on function key f0 if not f0") },
    { TC_FNAME(lab_f1),                     "l1", "lf1",        TC_DESC("label on function key f1 if not f1") },
    { TC_FNAME(lab_f2),                     "l2", "lf2",        TC_DESC("label on function key f2 if not f2") },
    { TC_FNAME(lab_f3),                     "l3", "lf3",        TC_DESC("label on function key f3 if not f3") },
    { TC_FNAME(lab_f4),                     "l4", "lf4",        TC_DESC("label on function key f4 if not f4") },
    { TC_FNAME(lab_f5),                     "l5", "lf5",        TC_DESC("label on function key f5 if not f5") },
    { TC_FNAME(lab_f6),                     "l6", "lf6",        TC_DESC("label on function key f6 if not f6") },
    { TC_FNAME(lab_f7),                     "l7", "lf7",        TC_DESC("label on function key f7 if not f7") },
    { TC_FNAME(lab_f8),                     "l8", "lf8",        TC_DESC("label on function key f8 if not f8") },
    { TC_FNAME(lab_f9),                     "l9", "lf9",        TC_DESC("label on function key f9 if not f9") },
    { TC_FNAME(lab_f10),                    "la", "lf10",       TC_DESC("label on function key f10 if not f10") },
    { TC_FNAME(meta_off),                   "mo", "rmm",        TC_DESC("turn off meta mode"), &tc_mo },
    { TC_FNAME(meta_on),                    "mm", "smm",        TC_DESC("turn on meta mode (8th-bit on)"), &tc_mm },
    { TC_FNAME(newline),                    "nw", "nel",        TC_DESC("newline (behave like cr followed by lf)") },
    { TC_FNAME(pad_char),                   "pc", "pad",        TC_DESC("padding char (instead of null)"), &tc_pc },
    { TC_FNAME(parm_dch),                   "DC", "dch",        TC_DESC("delete #1 chars"), &tc_pDC },
    { TC_FNAME(parm_delete_line),           "DL", "dl",         TC_DESC("parm_delete_line"), &tc_pDL },
    { TC_FNAME(parm_down_cursor),           "DO", "cud",        TC_DESC("down #1 lines"), &tc_pDO },
    { TC_FNAME(parm_ich),                   "IC", "ich",        TC_DESC("insert #1 chars") },
    { TC_FNAME(parm_index),                 "SF", "indn",       TC_DESC("scroll forward #1 lines"), &tc_pSF },
    { TC_FNAME(parm_insert_line),           "AL", "il",         TC_DESC("parm insert line"), &tc_pAL },
    { TC_FNAME(parm_left_cursor),           "LE", "cub",        TC_DESC("move #1 chars to the left"), &tc_pLE },
    { TC_FNAME(parm_right_cursor),          "RI", "cuf",        TC_DESC("parm right cursor"), &tc_pRI },
    { TC_FNAME(parm_rindex),                "SR", "rin",        TC_DESC("scroll back #1 lines"), &tc_pSR },
    { TC_FNAME(parm_up_cursor),             "UP", "cuu",        TC_DESC("up #1 lines"), &tc_pUP },
    { TC_FNAME(pkey_key),                   "pk", "pfkey",      TC_DESC("program function key #1 to type string #2") },
    { TC_FNAME(pkey_local),                 "pl", "pfloc",      TC_DESC("program function key #1 to execute string #2") },
    { TC_FNAME(pkey_xmit),                  "px", "pfx",        TC_DESC("program function key #1 to transmit string #2") },
    { TC_FNAME(print_screen),               "ps", "mc0",        TC_DESC("print contents of screen") },
    { TC_FNAME(prtr_off),                   "pf", "mc4",        TC_DESC("turn off printer") },
    { TC_FNAME(prtr_on),                    "po", "mc5",        TC_DESC("turn on printer") },
    { TC_FNAME(repeat_char),                "rp", "rep",        TC_DESC("repeat char #1 #2 times"), &tc_rp },
    { TC_FNAME(reset_1string),              "r1", "rs1",        TC_DESC("reset string #1") },
    { TC_FNAME(reset_2string),              "r2", "rs2",        TC_DESC("reset string #2") },
    { TC_FNAME(reset_3string),              "r3", "rs3",        TC_DESC("reset string #2") },
    { TC_FNAME(reset_file),                 "rf", "rf",         TC_DESC("name of reset file") },
    { TC_FNAME(restore_cursor),             "rc", "rc",         TC_DESC("restore cursor to last position of sc") },
    { TC_FNAME(row_address),                "cv", "vpa",        TC_DESC("vertical position #1 absolute"), &tc_cv },
    { TC_FNAME(save_cursor),                "sc", "sc",         TC_DESC("save current cursor position") },
    { TC_FNAME(scroll_forward),             "sf", "ind",        TC_DESC("scroll text up"), &tc_sf },
    { TC_FNAME(scroll_reverse),             "sr", "ri",         TC_DESC("scroll text down"), &tc_sr },
    { TC_FNAME(set_attributes),             "sa", "sgr",        TC_DESC("define video attributes #1-#9 (PG9)"), &tc_sa },
    { TC_FNAME(set_tab),                    "st", "hts",        TC_DESC("set a tab in every row, current columns") },
    { TC_FNAME(set_window),                 "wi", "wind",       TC_DESC("current window is lines #1-#2 cols #3-#4") },
    { TC_FNAME(tab),                        "ta", "ht",         TC_DESC("tab to next 8-space hardware tab stop") },
    { TC_FNAME(to_status_line),             "ts", "tsl",        TC_DESC("move to status line") },
    { TC_FNAME(underline_char),             "uc", "uc",         TC_DESC("underline char and move past it") },
    { TC_FNAME(up_half_line),               "hu", "hu",         TC_DESC("half a line up") },
    { TC_FNAME(init_prog),                  "iP", "iprog",      TC_DESC("path name of program for initialization") },
    { TC_FNAME(prtr_non),                   "pO", "mc5p",       TC_DESC("turn on printer for #1 bytes") },
    { TC_FNAME(char_padding),               "rP", "rmp",        TC_DESC("like ip but when in insert mode") },
    { TC_FNAME(acs_chars),                  "ac", "acsc",       TC_DESC("acs characters"), TC_STRING(&tc_graphic_pairs) },
    { TC_FNAME(plab_norm),                  "pn", "pln",        TC_DESC("program label #1 to show string #2") },
    { TC_FNAME(enter_xon_mode),             "SX", "smxon",      TC_DESC("turn on xon/xoff handshaking") },
    { TC_FNAME(exit_xon_mode),              "RX", "rmxon",      TC_DESC("turn off xon/xoff handshaking") },
    { TC_FNAME(enter_am_mode),              "SA", "smam",       TC_DESC("turn on automatic margins"), &tc_am_on },
    { TC_FNAME(exit_am_mode),               "RA", "rmam",       TC_DESC("turn off automatic margins") },
    { TC_FNAME(xon_character),              "XN", "xonc",       TC_DESC("XON character") },
    { TC_FNAME(xoff_character),             "XF", "xoffc",      TC_DESC("XOFF character") },
    { TC_FNAME(ena_acs),                    "eA", "enacs",      TC_DESC("enable alternate char set"), &tc_acs_enable },
    { TC_FNAME(label_on),                   "LO", "smln",       TC_DESC("turn on soft labels") },
    { TC_FNAME(label_off),                  "LF", "rmln",       TC_DESC("turn off soft labels") },
    { TC_FNAME(req_for_input),              "RF", "rfi",        TC_DESC("send next input char (for ptys)") },
    { TC_FNAME(clr_bol),                    "cb", "el1",        TC_DESC("clear to beginning of line"), &tc_cb },
    { TC_FNAME(clear_margins),              "MC", "mgc",        TC_DESC("clear right and left soft margins") },
    { TC_FNAME(set_left_margin),            "ML", "smgl",       TC_DESC("set left soft margin") },
    { TC_FNAME(set_right_margin),           "MR", "smgr",       TC_DESC("set right soft margin") },
    { TC_FNAME(label_format),               "Lf", "fln",        TC_DESC("label format") },
    { TC_FNAME(set_clock),                  "SC", "sclk",       TC_DESC("set clock, #1 hrs #2 mins #3 secs") },
    { TC_FNAME(display_clock),              "DK", "dclk",       TC_DESC("display clock at (#1,#2)") },
    { TC_FNAME(remove_clock),               "RC", "rmclk",      TC_DESC("remove clock") },
    { TC_FNAME(create_window),              "CW", "cwin",       TC_DESC("define a window #1 from #2, #3 to #4, #5") },
    { TC_FNAME(goto_window),                "WG", "wingo",      TC_DESC("go to window #1") },
    { TC_FNAME(hangup ),                    "HU", "hup",        TC_DESC("hang-up phone") },
    { TC_FNAME(dial_phone),                 "DI", "dial",       TC_DESC("dial number #1") },
    { TC_FNAME(quick_dial),                 "QD", "qdial",      TC_DESC("dial number #1 without checking") },
    { TC_FNAME(tone),                       "TO", "tone",       TC_DESC("select touch tone dialing") },
    { TC_FNAME(pulse),                      "PU", "pulse",      TC_DESC("select pulse dialling") },
    { TC_FNAME(flash_hook),                 "fh", "hook",       TC_DESC("flash switch hook") },
    { TC_FNAME(fixed_pause),                "PA", "pause",      TC_DESC("pause for 2-3 seconds") },
    { TC_FNAME(wait_tone),                  "WA", "wait",       TC_DESC("wait for dial-tone") },
    { TC_FNAME(user0),                      "u0", "u0",         TC_DESC("user string #0") },
    { TC_FNAME(user1),                      "u1", "u1",         TC_DESC("user string #1") },
    { TC_FNAME(user2),                      "u2", "u2",         TC_DESC("user string #2") },
    { TC_FNAME(user3),                      "u3", "u3",         TC_DESC("user string #3") },
    { TC_FNAME(user4),                      "u4", "u4",         TC_DESC("user string #4") },
    { TC_FNAME(user5),                      "u5", "u5",         TC_DESC("user string #5") },
    { TC_FNAME(user6),                      "u6", "u6",         TC_DESC("user string #6") },
    { TC_FNAME(user7),                      "u7", "u7",         TC_DESC("user string #7") },
    { TC_FNAME(user8),                      "u8", "u8",         TC_DESC("user string #8") },
    { TC_FNAME(user9),                      "u9", "u9",         TC_DESC("user string #9") },
    { TC_FNAME(orig_pair),                  "op", "op",         TC_DESC("set default pair to its original value"), &tc_op },
    { TC_FNAME(orig_colors),                "oc", "oc",         TC_DESC("set all color pairs to the original ones"), &tc_oc },
    { TC_FNAME(initialize_color),           "Ic", "initc",      TC_DESC("initialize color #1 to (#2,#3,#4)") },
    { TC_FNAME(initialize_pair),            "Ip", "initp",      TC_DESC("initialize color pair #1 to fg=(#2,#3,#4), bg=(#5,#6,#7)") },
    { TC_FNAME(set_color_pair),             "sp", "scp",        TC_DESC("set current color pair to #1") },
    { TC_FNAME(set_foreground),             "Sf", "setf",       TC_DESC("set foreground (color)"), &tc_Color_Fg },
    { TC_FNAME(set_background),             "Sb", "setb",       TC_DESC("set background (color)"), &tc_Color_Bg },
    { TC_FNAME(change_char_pitch),          "ZA", "cpi",        TC_DESC("change number of characters per inch") },
    { TC_FNAME(change_line_pitch),          "ZB", "lpi",        TC_DESC("change number of lines per inch") },
    { TC_FNAME(change_res_horz),            "ZC", "chr",        TC_DESC("change horizontal resolution") },
    { TC_FNAME(change_res_vert),            "ZD", "cvr",        TC_DESC("change vertical resolution") },
    { TC_FNAME(define_char),                "ZE", "defc",       TC_DESC("define a character") },
    { TC_FNAME(enter_doublewide_mode),      "ZF", "swidm",      TC_DESC("enter double-wide mode") },
    { TC_FNAME(enter_draft_quality),        "ZG", "sdrfq",      TC_DESC("enter draft-quality mode") },
    { TC_FNAME(enter_italics_mode),         "ZH", "sitm",       TC_DESC("enter italic mode"), &tc_ZH },
    { TC_FNAME(enter_leftward_mode),        "ZI", "slm",        TC_DESC("start leftward carriage motion") },
    { TC_FNAME(enter_micro_mode),           "ZJ", "smicm",      TC_DESC("start micro-motion mode") },
    { TC_FNAME(enter_near_letter_quality),  "ZK", "snlq",       TC_DESC("enter NLQ mode") },
    { TC_FNAME(enter_normal_quality),       "ZL", "snrmq",      TC_DESC("enter normal-quality mode") },
    { TC_FNAME(enter_shadow_mode),          "ZM", "sshm",       TC_DESC("enter shadow-print mode") },
    { TC_FNAME(enter_subscript_mode),       "ZN", "ssubm",      TC_DESC("enter subscript mode") },
    { TC_FNAME(enter_superscript_mode),     "ZO", "ssupm",      TC_DESC("enter superscript mode") },
    { TC_FNAME(enter_upward_mode),          "ZP", "sum",        TC_DESC("start upward carriage motion") },
    { TC_FNAME(exit_doublewide_mode),       "ZQ", "rwidm",      TC_DESC("end double-wide mode") },
    { TC_FNAME(exit_italics_mode),          "ZR", "ritm",       TC_DESC("end italic mode"), &tc_ZR },
    { TC_FNAME(exit_leftward_mode),         "ZS", "rlm",        TC_DESC("end left-motion mode") },
    { TC_FNAME(exit_micro_mode),            "ZT", "rmicm",      TC_DESC("end micro-motion mode") },
    { TC_FNAME(exit_shadow_mode),           "ZU", "rshm",       TC_DESC("end shadow-print mode") },
    { TC_FNAME(exit_subscript_mode),        "ZV", "rsubm",      TC_DESC("exit_subscript_mode") },
    { TC_FNAME(exit_superscript_mode),      "ZW", "rsupm",      TC_DESC("end superscript mode") },
    { TC_FNAME(exit_upward_mode),           "ZX", "rum",        TC_DESC("end reverse character motion"), &tc_ZX },
    { TC_FNAME(micro_column_address),       "ZY", "mhpa",       TC_DESC("like column_address in micro mode") },
    { TC_FNAME(micro_down),                 "ZZ", "mcud1",      TC_DESC("like cursor_down in micro mode") },
    { TC_FNAME(micro_left),                 "Za", "mcub1",      TC_DESC("like cursor_left in micro mode") },
    { TC_FNAME(micro_right),                "Zb", "mcuf1",      TC_DESC("like cursor_right in micro mode") },
    { TC_FNAME(micro_row_address),          "Zc", "mvpa",       TC_DESC("like row_address in micro mode") },
    { TC_FNAME(micro_up),                   "Zd", "mcuu1",      TC_DESC("like cursor_up in micro mode") },
    { TC_FNAME(order_of_pins),              "Ze", "porder",     TC_DESC("match software bits to print-head pins") },
    { TC_FNAME(parm_down_micro),            "Zf", "mcud",       TC_DESC("like parm down cursor in micro mode") },
    { TC_FNAME(parm_left_micro),            "Zg", "mcub",       TC_DESC("Like parm left cursor in micro mode") },
    { TC_FNAME(parm_right_micro),           "Zh", "mcuf",       TC_DESC("Like parm right cursor in micro mode") },
    { TC_FNAME(parm_up_micro),              "Zi", "mcuu",       TC_DESC("Like parm_up_cursor in micro mode") },
    { TC_FNAME(select_char_set),            "Zj", "scs",        TC_DESC("select character set") },
    { TC_FNAME(set_bottom_margin),          "Zk", "smgb",       TC_DESC("set bottom margin at current line") },
    { TC_FNAME(set_bottom_margin_parm),     "Zl", "smgbp",      TC_DESC("set bottom margin at line #1 or #2 lines from bottom") },
    { TC_FNAME(set_left_margin_parm),       "Zm", "smglp",      TC_DESC("set left (right) margin at column #1 (#2)") },
    { TC_FNAME(set_right_margin_parm),      "Zn", "smgrp",      TC_DESC("set right margin at column #1") },
    { TC_FNAME(set_top_margin),             "Zo", "smgt",       TC_DESC("set top margin at current line") },
    { TC_FNAME(set_top_margin_parm),        "Zp", "smgtp",      TC_DESC("start printing bit image braphics") },
    { TC_FNAME(start_bit_image),            "Zq", "sbim",       TC_DESC("start bit image") },
    { TC_FNAME(start_char_set_def),         "Zr", "scsd",       TC_DESC("start character set definition") },
    { TC_FNAME(stop_bit_image),             "Zs", "rbim",       TC_DESC("stop printing bit image graphics") },
    { TC_FNAME(stop_char_set_def),          "Zt", "rcsd",       TC_DESC("end definition of character aet") },
    { TC_FNAME(subscript_characters),       "Zu", "subcs",      TC_DESC("list of subscriptable characters") },
    { TC_FNAME(superscript_characters),     "Zv", "supcs",      TC_DESC("list of superscriptable characters") },
    { TC_FNAME(these_cause_cr),             "Zw", "docr",       TC_DESC("printing any of these chars causes CR") },
    { TC_FNAME(zero_motion),                "Zx", "zerom",      TC_DESC("no motion for subsequent character") },

       /*
        *  The following string  capabilities  are  present  in  the  SVr4.0  term
        *  structure, but were originally not documented in the man page.
        */
    { TC_FNAME(char_set_names),             "Zy", "csnm",       TC_DESC("list of character set names") },
    { TC_FNAME(mouse_info),                 "Mi", "minfo",      TC_DESC("mouse status information") },
    { TC_FNAME(req_mouse_pos),              "RQ", "reqmp",      TC_DESC("request mouse position") },
    { TC_FNAME(get_mouse),                  "Gm", "getm",       TC_DESC("curses should get button events") },
    { TC_FNAME(set_a_foreground),           "AF", "setaf",      TC_DESC("set ANSI color foreground"), &tc_ANSI_Color_Fg },
    { TC_FNAME(set_a_background),           "AB", "setab",      TC_DESC("set ANSI color background"), &tc_ANSI_Color_Bg },
    { TC_FNAME(pkey_plab),                  "xl", "pfxl",       TC_DESC("program function key #1 to type string #2 and show string #3") },
    { TC_FNAME(device_type),                "dv", "devt",       TC_DESC("indicate language/codeset support") },
    { TC_FNAME(code_set_init),              "ci", "csin",       TC_DESC("init sequence for multiple codesets") },
    { TC_FNAME(set0_des_seq),               "s0", "s0ds",       TC_DESC("shift to code set 0 (EUC set 0, ASCII)") },
    { TC_FNAME(set1_des_seq),               "s1", "s1ds",       TC_DESC("shift to code set 1") },
    { TC_FNAME(set2_des_seq),               "s2", "s2ds",       TC_DESC("shift to code set 2") },
    { TC_FNAME(set3_des_seq),               "s3", "s3ds",       TC_DESC("shift to code set 3") },
    { TC_FNAME(set_lr_margin),              "ML", "smglr",      TC_DESC("set both left and right margins to #1, #2") },
    { TC_FNAME(set_tb_margin),              "MT", "smgtb",      TC_DESC("sets both top and bottom margins to #1, #2") },
    { TC_FNAME(bit_image_repeat),           "Xy", "birep",      TC_DESC("repeat bit image cell #1 #2 times") },
    { TC_FNAME(bit_image_newline),          "Zz", "binel",      TC_DESC("move to next row of the bit image") },
    { TC_FNAME(bit_image_carriage_return),  "Yv", "bicr",       TC_DESC("move to beginning of same row") },
    { TC_FNAME(color_names),                "Yw", "colornm",    TC_DESC("give name for color #1") },
    { TC_FNAME(define_bit_image_region),    "Yx", "defbi",      TC_DESC("define rectangualar bit image region") },
    { TC_FNAME(end_bit_image_region),       "Yy", "endbi",      TC_DESC("end a bit-image region") },
    { TC_FNAME(set_color_band),             "Yz", "setcolor",   TC_DESC("change to ribbon color #1") },
    { TC_FNAME(set_page_length),            "YZ", "slines",     TC_DESC("set page length to #1 lines") },
    { TC_FNAME(display_pc_char),            "S1", "dispc",      TC_DESC("display PC character") },
    { TC_FNAME(enter_pc_charset_mode),      "S2", "smpch",      TC_DESC("enter PC character display mode") },
    { TC_FNAME(exit_pc_charset_mode),       "S3", "rmpch",      TC_DESC("exit PC character display mode") },
    { TC_FNAME(enter_scancode_mode),        "S4", "smsc",       TC_DESC("enter PC scancode mode") },
    { TC_FNAME(exit_scancode_mode),         "S5", "rmsc",       TC_DESC("exit PC scancode mode") },
    { TC_FNAME(pc_term_options),            "S6", "pctrm",      TC_DESC("PC terminal options") },
    { TC_FNAME(scancode_escape),            "S7", "scesc",      TC_DESC("escape for scancode emulation") },
    { TC_FNAME(alt_scancode_esc),           "S8", "scesa",      TC_DESC("alternate escape for scancode emulation") },

        /*
         *  The XSI Curses standard added these.
         *
         *  They are some post-4.1 versions of System V curses, e.g., Solaris 2.5 and IRIX 6.x.
         *  The ncurses termcap names for them are invented; according to the XSI Curses standard, they have no termcap names.
         *  If your compiled terminfo entries use these, they may not be binary-compatible with System V terminfo entries after SVr4.1; beware!
         */
    { TC_FNAME(enter_horizontal_hl_mode),   "Xh", "ehhlm",      TC_DESC("enter horizontal highlight mode") },
    { TC_FNAME(enter_left_hl_mode),         "Xl", "elhlm",      TC_DESC("enter left highlight mode") },
    { TC_FNAME(enter_low_hl_mode),          "Xo", "elohlm",     TC_DESC("enter low highlight mode") },
    { TC_FNAME(enter_right_hl_mode),        "Xr", "erhlm",      TC_DESC("enter right highlight mode") },
    { TC_FNAME(enter_top_hl_mode),          "Xt", "ethlm",      TC_DESC("enter top highlight mode") },
    { TC_FNAME(enter_vertical_hl_mode),     "Xv", "evhlm",      TC_DESC("enter vertical highlight mode") },
    { TC_FNAME(set_a_attributes),           "sA", "sgr1",       TC_DESC("Define second set of video attributes #1-#6") },
    { TC_FNAME(set_pglen_inch),             "YI", "slength",    TC_DESC("Set page length to #1 hundredth of an inch") },
    { TC_FNAME(termcap_init2),              "i2", "OTi2",       TC_DESC("secondary initialization string") },
    { TC_FNAME(termcap_reset),              "rs", "OTrs",       TC_DESC("terminal reset string"), &tc_rs },
    { TC_FNAME(linefeed_if_not_lf),         "nl", "OTnl",       TC_DESC("use to move down") },
    { TC_FNAME(backspace_if_not_bs),        "bc", "OTbc",       TC_DESC("move left, if not ^H (old-style)"), &tc_bc },
    { TC_FNAME(other_non_function_keys),    "ko", "OTko",       TC_DESC("list of self-mapped keycaps") },
    { TC_FNAME(arrow_key_map),              "ma", "OTma",       TC_DESC("map arrow keys rogue(1) motion keys") },

        /*
         *  Speical characters
         *
         *  Note: 'bx' only under AIX, which must be ordered after "ac" for correct selection.
         */
    { TC_FNAME(acs_ulcorner),               "G2", "OTG2",       TC_DESC("single upper left") },
    { TC_FNAME(acs_llcorner),               "G3", "OTG3",       TC_DESC("single lower left") },
    { TC_FNAME(acs_urcorner),               "G1", "OTG1",       TC_DESC("single upper right") },
    { TC_FNAME(acs_lrcorner),               "G4", "OTG4",       TC_DESC("single lower right") },
    { TC_FNAME(acs_ltee),                   "GR", "OTGR",       TC_DESC("tee pointing left") },
    { TC_FNAME(acs_rtee),                   "GL", "OTGL",       TC_DESC("tee pointing left") },
    { TC_FNAME(acs_btee),                   "GU", "OTGU",       TC_DESC("tee pointing up") },
    { TC_FNAME(acs_ttee),                   "GD", "OTGD",       TC_DESC("tee pointing down") },
    { TC_FNAME(acs_hline),                  "GH", "OTGH",       TC_DESC("single horizontal line") },
    { TC_FNAME(acs_vline),                  "GV", "OTGV",       TC_DESC("single vertical line") },
    { TC_FNAME(acs_plus),                   "GC", "OTGC",       TC_DESC("single intersection") },
    { TC_FNAME(memory_lock),                "ml", "meml",       TC_DESC("memory lock above") },
    { TC_FNAME(memory_unlock),              "mu", "memu",       TC_DESC("memory unlock") },
    { TC_FNAME(box_chars_1),                "bx", "box1",       TC_DESC("box chars primary set"), &tc_box_characters }, // aix-only

        /*
         *  User-Defined Capabilities
         *
         *  https://invisible-island.net/ncurses/man/user_caps.5.html
         *  See: ncurses/include/Caps-ncurses
         */
    { TC_UNAME(nc),                         NULL, "C0",         TC_DESC("number of indexed colors overlaying RGB space") },
    { TC_UNAME(nc),                         NULL, "E3",         TC_DESC("clears the terminal's scrollback buffer") },
    { TC_UNAME(nc),                         NULL, "RGB",        TC_DESC("use direct colors with given bit-layout") },
    { TC_UNAME(nc),                         NULL, "TS",         TC_DESC("parameterless tsl") },
    { TC_UNAME(nc),                         NULL, "XM",         TC_DESC("initialize alternate xterm mouse mode") },

        /* mintty specials */
    { TC_UNAME(mintty),                     NULL, "Rmol",       TC_DESC("remove overline-mode") },
    { TC_UNAME(mintty),                     NULL, "Smol",       TC_DESC("set overline-mode") },
    { TC_UNAME(mintty),                     NULL, "blink2",     TC_DESC("turn on rapid blinking") },
    { TC_UNAME(mintty),                     NULL, "norm",       TC_DESC("turn off bold and half-bright mode") },
    { TC_UNAME(mintty),                     NULL, "opaq",       TC_DESC("turn off blank mode") },
    { TC_UNAME(mintty),                     NULL, "setal",      TC_DESC("set underline-color") },
    { TC_UNAME(mintty),                     NULL, "smul2",      TC_DESC("begin double underline mode") },

        /* screen specials */
    { TC_UNAME(screen),                     NULL, "C0",         TC_DESC("use the string as a conversion table for font '0', like acsc") },
    { TC_UNAME(screen),                     NULL, "CE",         TC_DESC("switch cursor-keys back to normal mode") },
    { TC_UNAME(screen),                     NULL, "CS",         TC_DESC("switch cursor-keys to application mode") },
    { TC_UNAME(screen),                     NULL, "E0",         TC_DESC("switch charset 'G0' back to standard charset") },
    { TC_UNAME(screen),                     NULL, "KJ",         TC_DESC("set the encoding of the terminal") },
    { TC_UNAME(screen),                     NULL, "S0",         TC_DESC("switch charset 'G0' to the specified charset") },
    { TC_UNAME(screen),                     NULL, "WS",         TC_DESC("resize display") },
    { TC_UNAME(screen),                     NULL, "XC",         TC_DESC("character translation") },
    { TC_UNAME(screen),                     NULL, "Z0",         TC_DESC("change width to 132 columns") },
    { TC_UNAME(screen),                     NULL, "Z1",         TC_DESC("change width to 80 columns") },

        /* tmux specials */
    { TC_UNAME(tmux),                       NULL, "Cr",         TC_DESC("restore the default cursor color") },
    { TC_UNAME(tmux),                       NULL, "Cs",         TC_DESC("set the cursor color") },
    { TC_UNAME(tmux),                       NULL, "Csr",        TC_DESC("change the cursor style, overriding Ss") },
    { TC_UNAME(tmux),                       NULL, "Ms",         TC_DESC("store buffer in the host clipboard") },
    { TC_UNAME(tmux),                       NULL, "Se",         TC_DESC("reset the cursor style") },
    { TC_UNAME(tmux),                       NULL, "Smulx",      TC_DESC("modify the appearance of underlines in VTE") },
                                                                        //
                                                                        // 0 for no underscore,
                                                                        // 1 for normal underscore,
                                                                        // 2 for double underscore,
                                                                        // 3 for curly underscore,
                                                                        // 4 for dotted underscore and;
                                                                        // 5 for dashed underscore.
                                                                        //
    { TC_UNAME(tmux),                       NULL, "Ss",         TC_DESC("change the cursor style") },
                                                                        //
                                                                        // 0, 1 or none - blinking block cursor
                                                                        // 2 - block cursor
                                                                        // 3 - blinking underline cursor
                                                                        // 4 - underline cursor
                                                                        // 5 - blinking vertical bar cursor
                                                                        // 6 - vertical bar cursor
                                                                        //
    { TC_UNAME(tmux),                       NULL, "rmxx",       TC_DESC("reset ECMA-48 strikeout/crossed-out attributes") },
    { TC_UNAME(tmux),                       NULL, "smxx",       TC_DESC("set ECMA-48 strikeout/crossed-out attributes") },

        /* VIM specials */
    { TC_UNAME(vim),                        NULL, "BD",         TC_DESC("disables bracketed paste") },
    { TC_UNAME(vim),                        NULL, "BE",         TC_DESC("enables bracketed paste") },
    { TC_UNAME(vim),                        NULL, "PE",         TC_DESC("is sent after pasted text") },
    { TC_UNAME(vim),                        NULL, "PS",         TC_DESC("is sent before pasted text") },
    { TC_UNAME(vim),                        NULL, "RV",         TC_DESC("report terminal secondary device attributes"), &tc_RV },
    { TC_UNAME(vim),                        NULL, "XR",         TC_DESC("report terminal version as a free-format string.") },
    { TC_UNAME(vim),                        NULL, "fd",         TC_DESC("disable xterm focus-events") },
    { TC_UNAME(vim),                        NULL, "fe",         TC_DESC("enable xterm focus-events") },
    { TC_UNAME(vim),                        NULL, "rv",         TC_DESC("response to RV, regular expression") },
    { TC_UNAME(vim),                        NULL, "xr",         TC_DESC("response to XR, regular expression") },
    { TC_UNAME(vim),                        NULL, "SH",         TC_DESC("cursor shape") } //XXX
                                                                        //
                                                                        // 0, 1 or none - blinking block cursor
                                                                        // 2 - block cursor
                                                                        // 3 - blinking underline cursor
                                                                        // 4 - underline cursor
                                                                        // 5 - blinking vertical bar cursor
                                                                        // 6 - vertical bar cursor
                                                                        //
};


static TermNumeric_t term_numbers[] = {         /* numeric - termcap/terminfo elements */
    /*
     *  Standard
     */
    { TC_FNAME(columns),                    "co", "cols",       TC_DESC("number of columns"), &tn_co },
    { TC_FNAME(init_tabs),                  "it", "it",         TC_DESC("tabs initially every # spaces") },
    { TC_FNAME(lines),                      "li", "lines",      TC_DESC("number of lines on screen or page"), &tn_li },
    { TC_FNAME(lines_of_memory),            "lm", "lm",         TC_DESC("lines of memory if > line, 0 means varies") },
    { TC_FNAME(magic_cookie_glitch),        "sg", "xmc",        TC_DESC("number of blank chars left by smso or rmso"), &tn_sg },
    { TC_FNAME(padding_baud_rate),          "pb", "pb",         TC_DESC("lowest baud rate needing padding") },
    { TC_FNAME(virtual_terminal),           "vt", "vt",         TC_DESC("virtual terminal number") },
    { TC_FNAME(width_status_line),          "ws", "wsl",        TC_DESC("number of columns within status line") },
    { TC_FNAME(num_labels),                 "Nl", "nlab",       TC_DESC("number of labels") },
    { TC_FNAME(label_height),               "lh", "lh",         TC_DESC("rows in each label") },
    { TC_FNAME(label_width),                "lw", "lw",         TC_DESC("columns in each label") },
    { TC_FNAME(max_attributes),             "ma", "ma",         TC_DESC("max combined attributes") },
    { TC_FNAME(maximum_windows),            "MW", "wnum",       TC_DESC("max number of defineable windows") },

    { TC_FNAME(max_colors),                 "Co", "colors",     TC_DESC("max color"), &tf_Colors },
    { TC_FNAME(max_pairs),                  "pa", "pairs",      TC_DESC("max of color-pairs"), &tf_Pairs },
    { TC_FNAME(no_color_video),             "NC", "ncv",        TC_DESC("video attributes that cannot be used with colors"), &tn_NC },

    { TC_FNAME(buffer_capacity),            "Ya", "bufsz",      TC_DESC("numbers of bytes buffered before printing") },
    { TC_FNAME(dot_vert_spacing),           "Yb", "spinv",      TC_DESC("spacing of pins vertically in pins per inch") },
    { TC_FNAME(dot_horz_spacing),           "Yc", "spinh",      TC_DESC("spacing of dots horizontally in dots per inch") },
    { TC_FNAME(max_micro_address),          "Yd", "maddr",      TC_DESC("maximum value in micro_..._address") },
    { TC_FNAME(max_micro_jump),             "Ye", "mjump",      TC_DESC("maximum value in parm_..._micro") },
    { TC_FNAME(micro_col_size),             "Yf", "mcs",        TC_DESC("character step size when in micro mode") },
    { TC_FNAME(micro_line_size),            "Yg", "mls",        TC_DESC("line step size when in micro mode") },
    { TC_FNAME(number_of_pins),             "Yh", "npins",      TC_DESC("numbers of pins in print-head") },
    { TC_FNAME(output_res_char),            "Yi", "orc",        TC_DESC("horizontal resolution in units per line") },
    { TC_FNAME(output_res_line),            "Yj", "orl",        TC_DESC("vertical resolution in units per line") },
    { TC_FNAME(output_res_horz_inch),       "Yk", "orhi",       TC_DESC("horizontal resolution in units per inch") },
    { TC_FNAME(output_res_vert_inch),       "Yl", "orvi",       TC_DESC("vertical resolution in units per inch") },
    { TC_FNAME(print_rate),                 "Ym", "cps",        TC_DESC("print rate in characters per second") },
    { TC_FNAME(wide_char_size),             "Yn", "widcs",      TC_DESC("character step size when in double wide mode") },
    { TC_FNAME(buttons),                    "BT", "btns",       TC_DESC("number of buttons on mouse") },
    { TC_FNAME(bit_image_entwining),        "Yo", "bitwin",     TC_DESC("number of passes for each bit-image row") },
    { TC_FNAME(bit_image_type),             "Yp", "bitype",     TC_DESC("type of bit-image device") },
    { TC_FNAME(magic_cookie_glitch_ul),     "ug", "OTug",       TC_DESC("number of blanks left by ul") },

    { TC_FNAME(carriage_return_delay),      "dC", "OTdC",       TC_DESC("msec of padding for carriage-return character") },
    { TC_FNAME(new_line_delay),             "dN", "OTdN",       TC_DESC("msec of padding for newline (linefeed) character") },
    { TC_FNAME(backspace_delay),            "dB", "OTdB",       TC_DESC("msec of padding for backspace character") },
    { TC_FNAME(horizontal_tab_delay),       "dT", "OTdT",       TC_DESC("msec of padding for tab character") },
    { TC_FNAME(number_of_function_keys),    "kn", "OTkn",       TC_DESC("msec of padding for formfeed character") },

        /*
         *  User-Defined Capabilities
         *
         *  https://invisible-island.net/ncurses/man/user_caps.5.html
         *  See: ncurses/include/Caps-ncurses
         */
    { TC_UNAME(nc),                         NULL, "RGB",        TC_DESC("use direct colors with given number of bits") },
    { TC_UNAME(nc),                         NULL, "U8",         TC_DESC("terminal VT100 SI/SO processing UTF-8 encoding") },

        /* screen */
    { TC_UNAME(screen),                     NULL, "OL",         TC_DESC("set the screen program's output buffer limit.") },
    };


static TermKey_t term_keys[] = {                /* keys - termcap/terminfo elements */
    /*
     *  Standard
     */
    { TC_FNAME(key_backspace),              "kb", "kbs",        TC_DESC("backspace key"), TC_TOKEN(CTRL_H) },
    { TC_FNAME(key_catab),                  "ka", "ktbc",       TC_DESC("clear-all-tabs key") },
    { TC_FNAME(key_clear),                  "kC", "kclr",       TC_DESC("clear-screen or erase key") },
    { TC_FNAME(key_ctab),                   "kt", "kctab",      TC_DESC("clear-tab key") },
    { TC_FNAME(key_dc),                     "kD", "kdch1",      TC_DESC("delete-character key"), TC_TOKEN(KEY_DEL) },
    { TC_FNAME(key_dl),                     "kL", "kdl1",       TC_DESC("delete-line key") },
    { TC_FNAME(key_down),                   "kd", "kcud1",      TC_DESC("down-arrow key"), TC_TOKEN(KEY_DOWN) },
    { TC_FNAME(key_eic),                    "kM", "krmir",      TC_DESC("sent by rmir or smi rin insert mode") },
    { TC_FNAME(key_eol),                    "kE", "kel",        TC_DESC("clear-to-end-of-line key") },
    { TC_FNAME(key_eos),                    "kS", "ked",        TC_DESC("clear-to-end-of-screen key") },
    { TC_FNAME(key_f0),                     "k0", "kf0",        TC_DESC("F0 function key"), TC_TOKEN(F(10)) },
    { TC_FNAME(key_f1),                     "k1", "kf1",        TC_DESC("F1 function key"), TC_TOKEN(F(1)) },
    { TC_FNAME(key_f10),                    "k;", "kf10",       TC_DESC("F10 function key"), TC_TOKEN(F(10)) },
    { TC_FNAME(key_f2),                     "k2", "kf2",        TC_DESC("F2 function key"), TC_TOKEN(F(2)) },
    { TC_FNAME(key_f3),                     "k3", "kf3",        TC_DESC("F3 function key"), TC_TOKEN(F(3)) },
    { TC_FNAME(key_f4),                     "k4", "kf4",        TC_DESC("F4 function key"), TC_TOKEN(F(4)) },
    { TC_FNAME(key_f5),                     "k5", "kf5",        TC_DESC("F5 function key"), TC_TOKEN(F(5)) },
    { TC_FNAME(key_f6),                     "k6", "kf6",        TC_DESC("F6 function key"), TC_TOKEN(F(6)) },
    { TC_FNAME(key_f7),                     "k7", "kf7",        TC_DESC("F7 function key"), TC_TOKEN(F(7)) },
    { TC_FNAME(key_f8),                     "k8", "kf8",        TC_DESC("F8 function key"), TC_TOKEN(F(8)) },
    { TC_FNAME(key_f9),                     "k9", "kf9",        TC_DESC("F9 function key"), TC_TOKEN(F(9)) },
    { TC_FNAME(key_home),                   "kh", "khome",      TC_DESC("home key"), TC_TOKEN(KEY_HOME) },
    { TC_FNAME(key_ic),                     "kI", "kich1",      TC_DESC("insert-character key"), TC_TOKEN(KEY_INS) },
    { TC_FNAME(key_il),                     "kA", "kil1",       TC_DESC("insert-line key") },
    { TC_FNAME(key_left),                   "kl", "kcub1",      TC_DESC("left-arrow key"), TC_TOKEN(KEY_LEFT) },
    { TC_FNAME(key_ll),                     "kH", "kll",        TC_DESC("lower-left key (home down)"), TC_TOKEN(KEY_END) },
    { TC_FNAME(key_npage),                  "kN", "knp",        TC_DESC("next-page key"), TC_TOKEN(KEY_PAGEDOWN) },
    { TC_FNAME(key_ppage),                  "kP", "kpp",        TC_DESC("previous-page key"), TC_TOKEN(KEY_PAGEUP) },
    { TC_FNAME(key_right),                  "kr", "kcuf1",      TC_DESC("right-arrow key"), TC_TOKEN(KEY_RIGHT) },
    { TC_FNAME(key_sf),                     "kF", "kind",       TC_DESC("scroll-forward key") },
    { TC_FNAME(key_sr),                     "kR", "kri",        TC_DESC("scroll-backward key") },
    { TC_FNAME(key_stab),                   "kT", "khts",       TC_DESC("set-tab key") },
    { TC_FNAME(key_up),                     "ku", "kcuu1",      TC_DESC("up-arrow key"), TC_TOKEN(KEY_UP) },
    { TC_FNAME(key_a1),                     "K1", "ka1",        TC_DESC("upper left of keypad"), TC_TOKEN(KEYPAD_7) },
    { TC_FNAME(key_a3),                     "K3", "ka3",        TC_DESC("center of keypad"), TC_TOKEN(KEYPAD_9) },
    { TC_FNAME(key_b2),                     "K2", "kb2",        TC_DESC("upper right of key-pad"), TC_TOKEN(KEYPAD_5) },
    { TC_FNAME(key_c1),                     "K4", "kc1",        TC_DESC("lower left of keypad"), TC_TOKEN(KEYPAD_1) },
    { TC_FNAME(key_c3),                     "K5", "kc3",        TC_DESC("lower right of key-pad"), TC_TOKEN(KEYPAD_3) },
    { TC_FNAME(key_btab),                   "kB", "kcbt",       TC_DESC("back-tab key"), TC_TOKEN(BACK_TAB) },
    { TC_FNAME(key_beg),                    "@1", "kbeg",       TC_DESC("reference key") },
    { TC_FNAME(key_cancel),                 "@2", "kcan",       TC_DESC("refresh key") },
    { TC_FNAME(key_close),                  "@3", "kclo",       TC_DESC("replace key"), TC_TOKEN(KEY_REPLACE) },
    { TC_FNAME(key_command),                "@4", "kcmd",       TC_DESC("restart key") },
    { TC_FNAME(key_copy),                   "@5", "kcpy",       TC_DESC("resume key") },
    { TC_FNAME(key_create),                 "@6", "kcrt",       TC_DESC("save key"), TC_TOKEN(KEY_SAVE) },
    { TC_FNAME(key_end),                    "@7", "kend",       TC_DESC("suspend key") },
    { TC_FNAME(key_enter),                  "@8", "kent",       TC_DESC("undo key"), TC_TOKEN(KEY_UNDO_CMD) },
    { TC_FNAME(key_exit),                   "@9", "kext",       TC_DESC("shifted begin key") },
    { TC_FNAME(key_find),                   "@0", "kfnd",       TC_DESC("shifted cancel key") },
    { TC_FNAME(key_help),                   "%1", "khlp",       TC_DESC("help key"), TC_TOKEN(KEY_HELP) },
    { TC_FNAME(key_mark),                   "%2", "kmrk",       TC_DESC("mark key") },                  /* KEY_MARK */
    { TC_FNAME(key_message),                "%3", "kmsg",       TC_DESC("message key") },
    { TC_FNAME(key_move),                   "%4", "kmov",       TC_DESC("move key") },
    { TC_FNAME(key_next),                   "%5", "knxt",       TC_DESC("next key"), TC_TOKEN(KEY_NEXT) },
    { TC_FNAME(key_open),                   "%6", "kopn",       TC_DESC("open key"), TC_TOKEN(KEY_OPEN) },
    { TC_FNAME(key_options),                "%7", "kopt",       TC_DESC("options key"), TC_TOKEN(KEY_MENU) },
    { TC_FNAME(key_previous),               "%8", "kprv",       TC_DESC("previous key"), TC_TOKEN(KEY_PREV) },
    { TC_FNAME(key_print),                  "%9", "kprt",       TC_DESC("print key") },                 /* KEY_PRINT */
    { TC_FNAME(key_redo),                   "%0", "krdo",       TC_DESC("redo key"), TC_TOKEN(KEY_REDO) },
    { TC_FNAME(key_reference),              "&1", "kref",       TC_DESC("reference key") },
    { TC_FNAME(key_refresh),                "&2", "krfr",       TC_DESC("refresh key") },               /* KEY_REFRESH */
    { TC_FNAME(key_replace),                "&3", "krpl",       TC_DESC("replace key"), TC_TOKEN(KEY_REPLACE) },
    { TC_FNAME(key_restart),                "&4", "krst",       TC_DESC("restart key") },
    { TC_FNAME(key_resume),                 "&5", "kres",       TC_DESC("resume key") },
    { TC_FNAME(key_save),                   "&6", "ksav",       TC_DESC("save key"), TC_TOKEN(KEY_SAVE) },
    { TC_FNAME(key_suspend),                "&7", "kspd",       TC_DESC("suspend key") },
    { TC_FNAME(key_undo),                   "&8", "kund",       TC_DESC("undo key"), TC_TOKEN(KEY_UNDO_CMD) },
    { TC_FNAME(key_sbeg),                   "&9", "kBEG",       TC_DESC("shifted begin key") },
    { TC_FNAME(key_scancel),                "&0", "kCAN",       TC_DESC("shifted cancel key") },
    { TC_FNAME(key_scommand),               "*1", "kCMD",       TC_DESC("shifted command key") },
    { TC_FNAME(key_scopy),                  "*2", "kCPY",       TC_DESC("shifted copy key") },
    { TC_FNAME(key_screate),                "*3", "kCRT",       TC_DESC("shifted create key") },
    { TC_FNAME(key_sdc),                    "*4", "kDC",        TC_DESC("shifted delete-char") },
    { TC_FNAME(key_sdl),                    "*5", "kDL",        TC_DESC("shifted delete-line key") },
    { TC_FNAME(key_select),                 "*6", "kslt",       TC_DESC("select key") },
    { TC_FNAME(key_send),                   "*7", "kEND",       TC_DESC("shifted end key"), TC_TOKEN(MOD_SHIFT|KEY_END) },
    { TC_FNAME(key_seol),                   "*8", "kEOL",       TC_DESC("shifted clear-to-end-of-line key") },
    { TC_FNAME(key_sexit),                  "*9", "kEXT",       TC_DESC("shifted exit key") },
    { TC_FNAME(key_sfind),                  "*0", "kFND",       TC_DESC("shifted find key") },
    { TC_FNAME(key_shelp),                  "#1", "kHLP",       TC_DESC("shifted help key") },          /* KEY_HELP */
    { TC_FNAME(key_shome),                  "#2", "kHOM",       TC_DESC("shifted home key"), TC_TOKEN(MOD_SHIFT|KEY_HOME) },
    { TC_FNAME(key_sic),                    "#3", "kIC",        TC_DESC("shifted insert-character key") },
    { TC_FNAME(key_sleft),                  "#4", "kLFT",       TC_DESC("shifted left-arrow key") },
    { TC_FNAME(key_smessage),               "%a", "kMSG",       TC_DESC("shifted message key") },
    { TC_FNAME(key_smove),                  "%b", "kMOV",       TC_DESC("shifted move key") },
    { TC_FNAME(key_snext),                  "%c", "kNXT",       TC_DESC("shifted next key") },
    { TC_FNAME(key_soptions),               "%d", "kOPT",       TC_DESC("shifted options key") },
    { TC_FNAME(key_sprevious),              "%e", "kPRV",       TC_DESC("shifted previous key") },
    { TC_FNAME(key_sprint),                 "%f", "kPRT",       TC_DESC("shifted print key") },
    { TC_FNAME(key_sredo),                  "%g", "kRDO",       TC_DESC("shifted redo key") },
    { TC_FNAME(key_sreplace),               "%h", "kRPL",       TC_DESC("shifted replace key") },
    { TC_FNAME(key_sright),                 "%i", "kRIT",       TC_DESC("shifted right-arrow key") },
    { TC_FNAME(key_srsume),                 "%j", "kRES",       TC_DESC("shifted resume key") },
    { TC_FNAME(key_ssave),                  "!1", "kSAV",       TC_DESC("shifted save key") },
    { TC_FNAME(key_ssuspend),               "!2", "kSPD",       TC_DESC("shifted suspend key") },
    { TC_FNAME(key_sundo),                  "!3", "kUND",       TC_DESC("shifted undo key"), TC_TOKEN(KEY_REDO) },
    { TC_FNAME(key_f11),                    "F1", "kf11",       TC_DESC("F11 function key"), TC_TOKEN(F(11)) },
    { TC_FNAME(key_f12),                    "F2", "kf12",       TC_DESC("F12 function key"), TC_TOKEN(F(12)) },
    { TC_FNAME(key_f13),                    "F3", "kf13",       TC_DESC("F13 function key") /*, TC_TOKEN(F(13))*/ },
    { TC_FNAME(key_f14),                    "F4", "kf14",       TC_DESC("F14 function key") /*, TC_TOKEN(F(14))*/ },
    { TC_FNAME(key_f15),                    "F5", "kf15",       TC_DESC("F15 function key") /*, TC_TOKEN(F(15))*/ },
    { TC_FNAME(key_f16),                    "F6", "kf16",       TC_DESC("F16 function key") /*, TC_TOKEN(F(16))*/ },
    { TC_FNAME(key_f17),                    "F7", "kf17",       TC_DESC("F17 function key") /*, TC_TOKEN(F(17))*/ },
    { TC_FNAME(key_f18),                    "F8", "kf18",       TC_DESC("F18 function key") /*, TC_TOKEN(F(18))*/ },
    { TC_FNAME(key_f19),                    "F9", "kf19",       TC_DESC("F19 function key") /*, TC_TOKEN(F(19))*/ },
    { TC_FNAME(key_f20),                    "FA", "kf20",       TC_DESC("F20 function key") /*, TC_TOKEN(F(20))*/ },
    { TC_FNAME(key_f21),                    "FB", "kf21",       TC_DESC("F21 function key") },
    { TC_FNAME(key_f22),                    "FC", "kf22",       TC_DESC("F22 function key") },
    { TC_FNAME(key_f23),                    "FD", "kf23",       TC_DESC("F23 function key") },
    { TC_FNAME(key_f24),                    "FE", "kf24",       TC_DESC("F24 function key") },
    { TC_FNAME(key_f25),                    "FF", "kf25",       TC_DESC("F25 function key") },
    { TC_FNAME(key_f26),                    "FG", "kf26",       TC_DESC("F26 function key") },
    { TC_FNAME(key_f27),                    "FH", "kf27",       TC_DESC("F27 function key") },
    { TC_FNAME(key_f28),                    "FI", "kf28",       TC_DESC("F28 function key") },
    { TC_FNAME(key_f29),                    "FJ", "kf29",       TC_DESC("F29 function key") },
    { TC_FNAME(key_f30),                    "FK", "kf30",       TC_DESC("F30 function key") },
    { TC_FNAME(key_f31),                    "FL", "kf31",       TC_DESC("F31 function key") },
    { TC_FNAME(key_f32),                    "FM", "kf32",       TC_DESC("F32 function key") },
    { TC_FNAME(key_f33),                    "FN", "kf33",       TC_DESC("F33 function key") },
    { TC_FNAME(key_f34),                    "FO", "kf34",       TC_DESC("F34 function key") },
    { TC_FNAME(key_f35),                    "FP", "kf35",       TC_DESC("F35 function key") },
    { TC_FNAME(key_f36),                    "FQ", "kf36",       TC_DESC("F36 function key") },
    { TC_FNAME(key_f37),                    "FR", "kf37",       TC_DESC("F37 function key") },
    { TC_FNAME(key_f38),                    "FS", "kf38",       TC_DESC("F38 function key") },
    { TC_FNAME(key_f39),                    "FT", "kf39",       TC_DESC("F39 function key") },
    { TC_FNAME(key_f40),                    "FU", "kf40",       TC_DESC("F40 function key") },
    { TC_FNAME(key_f41),                    "FV", "kf41",       TC_DESC("F41 function key") },
    { TC_FNAME(key_f42),                    "FW", "kf42",       TC_DESC("F42 function key") },
    { TC_FNAME(key_f43),                    "FX", "kf43",       TC_DESC("F43 function key") },
    { TC_FNAME(key_f44),                    "FY", "kf44",       TC_DESC("F44 function key") },
    { TC_FNAME(key_f45),                    "FZ", "kf45",       TC_DESC("F45 function key") },
    { TC_FNAME(key_f46),                    "Fa", "kf46",       TC_DESC("F46 function key") },
    { TC_FNAME(key_f47),                    "Fb", "kf47",       TC_DESC("F47 function key") },
    { TC_FNAME(key_f48),                    "Fc", "kf48",       TC_DESC("F48 function key") },
    { TC_FNAME(key_f49),                    "Fd", "kf49",       TC_DESC("F49 function key") },
    { TC_FNAME(key_f50),                    "Fe", "kf50",       TC_DESC("F50 function key") },
    { TC_FNAME(key_f51),                    "Ff", "kf51",       TC_DESC("F51 function key") },
    { TC_FNAME(key_f52),                    "Fg", "kf52",       TC_DESC("F52 function key") },
    { TC_FNAME(key_f53),                    "Fh", "kf53",       TC_DESC("F53 function key") },
    { TC_FNAME(key_f54),                    "Fi", "kf54",       TC_DESC("F54 function key") },
    { TC_FNAME(key_f55),                    "Fj", "kf55",       TC_DESC("F55 function key") },
    { TC_FNAME(key_f56),                    "Fk", "kf56",       TC_DESC("F56 function key") },
    { TC_FNAME(key_f57),                    "Fl", "kf57",       TC_DESC("F57 function key") },
    { TC_FNAME(key_f58),                    "Fm", "kf58",       TC_DESC("F58 function key") },
    { TC_FNAME(key_f59),                    "Fn", "kf59",       TC_DESC("F59 function key") },
    { TC_FNAME(key_f60),                    "Fo", "kf60",       TC_DESC("F60 function key") },
    { TC_FNAME(key_f61),                    "Fp", "kf61",       TC_DESC("F61 function key") },
    { TC_FNAME(key_f62),                    "Fq", "kf62",       TC_DESC("F62 function key") },
    { TC_FNAME(key_f63),                    "Fr", "kf63",       TC_DESC("F63 function key") },
    { TC_FNAME(key_mouse),                  "Km", "kmous",      TC_DESC("mouse event has occurred") },

    /*xterm keys */
    { TC_KNAME(xt),                         NULL, "kDC3",       TC_DESC("alt delete-character") },
    { TC_KNAME(xt),                         NULL, "kDC4",       TC_DESC("shift+alt delete-character") },
    { TC_KNAME(xt),                         NULL, "kDC5",       TC_DESC("control delete-character") },
    { TC_KNAME(xt),                         NULL, "kDC6",       TC_DESC("shift+control delete-character") },
    { TC_KNAME(xt),                         NULL, "kDC7",       TC_DESC("alt+control delete-character") },
    { TC_KNAME(xt),                         NULL, "kDN",        TC_DESC("shift down-cursor") },
    { TC_KNAME(xt),                         NULL, "kDN3",       TC_DESC("alt down-cursor") },
    { TC_KNAME(xt),                         NULL, "kDN4",       TC_DESC("shift+alt down-cursor") },
    { TC_KNAME(xt),                         NULL, "kDN5",       TC_DESC("control down-cursor") },
    { TC_KNAME(xt),                         NULL, "kDN6",       TC_DESC("shift+control down-cursor") },
    { TC_KNAME(xt),                         NULL, "kDN7",       TC_DESC("alt+control down-cursor") },
    { TC_KNAME(xt),                         NULL, "kEND3",      TC_DESC("alt end") },
    { TC_KNAME(xt),                         NULL, "kEND4",      TC_DESC("shift+alt end") },
    { TC_KNAME(xt),                         NULL, "kEND5",      TC_DESC("control end") },
    { TC_KNAME(xt),                         NULL, "kEND6",      TC_DESC("shift+control end") },
    { TC_KNAME(xt),                         NULL, "kEND7",      TC_DESC("alt+control end") },
    { TC_KNAME(xt),                         NULL, "kHOM3",      TC_DESC("alt home") },
    { TC_KNAME(xt),                         NULL, "kHOM4",      TC_DESC("shift+alt home") },
    { TC_KNAME(xt),                         NULL, "kHOM5",      TC_DESC("control home") },
    { TC_KNAME(xt),                         NULL, "kHOM6",      TC_DESC("shift+control home") },
    { TC_KNAME(xt),                         NULL, "kHOM7",      TC_DESC("alt+control home") },
    { TC_KNAME(xt),                         NULL, "kIC3",       TC_DESC("alt insert-character") },
    { TC_KNAME(xt),                         NULL, "kIC4",       TC_DESC("shift+alt insert-character") },
    { TC_KNAME(xt),                         NULL, "kIC5",       TC_DESC("control insert-character") },
    { TC_KNAME(xt),                         NULL, "kIC6",       TC_DESC("shift+control insert-character") },
    { TC_KNAME(xt),                         NULL, "kIC7",       TC_DESC("alt+control insert-character") },
    { TC_KNAME(xt),                         NULL, "kLFT3",      TC_DESC("alt left-cursor") },
    { TC_KNAME(xt),                         NULL, "kLFT4",      TC_DESC("shift+alt left-cursor") },
    { TC_KNAME(xt),                         NULL, "kLFT5",      TC_DESC("control left-cursor") },
    { TC_KNAME(xt),                         NULL, "kLFT6",      TC_DESC("shift+control left-cursor") },
    { TC_KNAME(xt),                         NULL, "kLFT7",      TC_DESC("alt+control left-cursor") },
    { TC_KNAME(xt),                         NULL, "kNXT3",      TC_DESC("alt next") },
    { TC_KNAME(xt),                         NULL, "kNXT4",      TC_DESC("shift+alt next") },
    { TC_KNAME(xt),                         NULL, "kNXT5",      TC_DESC("control next") },
    { TC_KNAME(xt),                         NULL, "kNXT6",      TC_DESC("shift+control next") },
    { TC_KNAME(xt),                         NULL, "kNXT7",      TC_DESC("alt+control next") },
    { TC_KNAME(xt),                         NULL, "kPRV3",      TC_DESC("alt previous") },
    { TC_KNAME(xt),                         NULL, "kPRV4",      TC_DESC("shift+alt previous") },
    { TC_KNAME(xt),                         NULL, "kPRV5",      TC_DESC("control previous") },
    { TC_KNAME(xt),                         NULL, "kPRV6",      TC_DESC("shift+control previous") },
    { TC_KNAME(xt),                         NULL, "kPRV7",      TC_DESC("alt+control previous") },
    { TC_KNAME(xt),                         NULL, "kRIT3",      TC_DESC("alt right-cursor") },
    { TC_KNAME(xt),                         NULL, "kRIT4",      TC_DESC("shift+alt right-cursor") },
    { TC_KNAME(xt),                         NULL, "kRIT5",      TC_DESC("control right-cursor") },
    { TC_KNAME(xt),                         NULL, "kRIT6",      TC_DESC("shift+control right-cursor") },
    { TC_KNAME(xt),                         NULL, "kRIT7",      TC_DESC("alt+control right-cursor") },
    { TC_KNAME(xt),                         NULL, "kUP",        TC_DESC("shift up-cursor") },
    { TC_KNAME(xt),                         NULL, "kUP3",       TC_DESC("alt up-cursor") },
    { TC_KNAME(xt),                         NULL, "kUP4",       TC_DESC("shift+alt up-cursor") },
    { TC_KNAME(xt),                         NULL, "kUP5",       TC_DESC("control up-cursor") },
    { TC_KNAME(xt),                         NULL, "kUP6",       TC_DESC("shift+control up-cursor") },
    { TC_KNAME(xt),                         NULL, "kUP7",       TC_DESC("alt+control up-cursor") },
    { TC_KNAME(xt),                         NULL, "ka2",        TC_DESC("vt220-keypad extensions") },
    { TC_KNAME(xt),                         NULL, "kb1",        TC_DESC("vt220-keypad extensions") },
    { TC_KNAME(xt),                         NULL, "kb3",        TC_DESC("vt220-keypad extensions") },
    { TC_KNAME(xt),                         NULL, "kc2",        TC_DESC("vt220-keypad extensions") },
    { TC_KNAME(xt),                         NULL, "kxIN",       TC_DESC("mouse response on focus-in") },
    { TC_KNAME(xt),                         NULL, "kxOU",       TC_DESC("mouse response on focus-out") },
    };

static TermNumeric_t term_flags[] = {           /* boolean - termcap/terminfo elements */
    { TC_FNAME(auto_left_margin),           "bw", "bw",         TC_DESC("cub1 wraps from column 0 to last column") },
    { TC_FNAME(auto_right_margin),          "am", "am",         TC_DESC("terminal has automatic margins"), TC_FLAG(&tf_am) },
    { TC_FNAME(no_esc_ctlc),                "xb", "xsb",        TC_DESC("beehive (f1=escape, f2=ctrl C)") },
    { TC_FNAME(ceol_standout_glitch),       "xs", "xhp",        TC_DESC("standout not erased by overwriting (hp)"), TC_FLAG(&tf_xs) },
    { TC_FNAME(eat_newline_glitch),         "xn", "xenl",       TC_DESC("newline ignored after 80 cols (concept)"), TC_FLAG(&tf_xn) },
    { TC_FNAME(erase_overstrike),           "eo", "eo",         TC_DESC("can erase overstrikes with a blank") },
    { TC_FNAME(generic_type),               "gn", "gn",         TC_DESC("generic line type"), TC_FLAG(&tf_gn) },
    { TC_FNAME(hard_copy),                  "hc", "hc",         TC_DESC("hardcopy terminal"), TC_FLAG(&tf_hc) },
    { TC_FNAME(has_meta_key),               "km", "km",         TC_DESC("has a meta key, sets msb high"), TC_FLAG(&tf_km) },
    { TC_FNAME(has_status_line),            "hs", "hs",         TC_DESC("has extra status line") },
    { TC_FNAME(insert_null_glitch),         "in", "in",         TC_DESC("insert mode distinguishes nulls") },
    { TC_FNAME(memory_above),               "da", "da",         TC_DESC("display may be retained above the screen") },
    { TC_FNAME(memory_below),               "db", "db",         TC_DESC("display may be retained below the screen") },
    { TC_FNAME(move_insert_mode),           "mi", "mir",        TC_DESC("safe to move while in insert mode") },
    { TC_FNAME(move_standout_mode),         "ms", "msgr",       TC_DESC("safe to move while in standout/underline mode"), TC_FLAG(&tf_ms) },
    { TC_FNAME(over_strike),                "os", "os",         TC_DESC("terminal can overstrike") },
    { TC_FNAME(status_line_esc_ok),         "es", "eslok",      TC_DESC("escape can be used on the status line") },
    { TC_FNAME(dest_tabs_magic_smso),       "xt", "xt",         TC_DESC("tabs destructive, magic so char (Telray 1061)"), TC_FLAG(&tf_xt) },
    { TC_FNAME(tilde_glitch),               "hz", "hz",         TC_DESC("can't print ~'s (hazeltine)"), TC_FLAG(&tf_hz) },
    { TC_FNAME(transparent_underline),      "ul", "ul",         TC_DESC("underline character overstrikes") },
    { TC_FNAME(xon_xoff),                   "xo", "xon",        TC_DESC("terminal uses xon/xoff handshaking"), TC_FLAG(&tf_xonoff) },
    { TC_FNAME(needs_xon_xoff),             "nx", "nxon",       TC_DESC("padding won't work, xon/xoff required"), TC_FLAG(&tf_xonoff) },
    { TC_FNAME(prtr_silent),                "5i", "mc5i",       TC_DESC("printer wont echo on screen") },
    { TC_FNAME(hard_cursor),                "HC", "chts",       TC_DESC("cursor is hard to see") },
    { TC_FNAME(non_rev_rmcup),              "NR", "nrrmc",      TC_DESC("smcup does not reverse rmcup") },
    { TC_FNAME(no_pad_char),                "NP", "npc",        TC_DESC("pad character does not exist"), TC_FLAG(&tf_npc) },
    { TC_FNAME(non_dest_scroll_region),     "ND", "ndscr",      TC_DESC("scrolling region is non-destructive") },
    { TC_FNAME(can_change),                 "cc", "ccc",        TC_DESC("terminal can re-define existing colors") },
    { TC_FNAME(back_color_erase),           "ut", "bce",        TC_DESC("screen erased with background color"), TC_FLAG(&tf_ut) },
    { TC_FNAME(hue_lightness_saturation),   "hl", "hls",        TC_DESC("terminal uses only HLS color notation (tektronix)") },
    { TC_FNAME(col_addr_glitch),            "YA", "xhpa",       TC_DESC("only positive motion for hpa/mhpa caps") },
    { TC_FNAME(cr_cancels_micro_mode),      "YB", "crxm",       TC_DESC("using cr turns off micro mode") },
    { TC_FNAME(has_print_wheel),            "YC", "daisy",      TC_DESC("printer needs operator to change character set") },
    { TC_FNAME(row_addr_glitch),            "YD", "xvpa",       TC_DESC("only positive motion for vpa/mvpa caps") },
    { TC_FNAME(semi_auto_right_margin),     "YE", "sam",        TC_DESC("printing in last column causes cr") },
    { TC_FNAME(cpi_changes_res),            "YF", "cpix",       TC_DESC("changing character pitch changes resolution") },
    { TC_FNAME(lpi_changes_res),            "YG", "lpix",       TC_DESC("changing line pitch changes resolution") },
    { TC_FNAME(backspaces_with_bs),         "bs", "OTbs",       TC_DESC("uses ^H to move left"), TC_FLAG(&tf_bs) },
    { TC_FNAME(crt_no_scrolling),           "ns", "OTns",       TC_DESC("crt cannot scroll") },
    { TC_FNAME(no_correctly_working_cr),    "nc", "OTnc",       TC_DESC("no way to go to start of line") },
    { TC_FNAME(gnu_has_meta_key),           "MT", "OTMT",       TC_DESC("has meta key") },          /* TODO */
    { TC_FNAME(linefeed_is_newline),        "NL", "OTNL",       TC_DESC("move down with \\n"), TC_FLAG(&tf_NL) },
    { TC_FNAME(has_hardware_tabs),          "pt", "OTpt",       TC_DESC("has 8-char tabs invoked with ^I") },
    { TC_FNAME(return_does_clr_eol),        "xr", "OTxr",       TC_DESC("return clears the line") },

        /*
         *  User-Defined Capabilities
         *
         *  https://invisible-island.net/ncurses/man/user_caps.5.html
         *  See: ncurses/include/Caps-ncurses
         */
    { TC_UNAME(xt),                         NULL, "NQ",         TC_DESC("terminal does not support query/response") },
    { TC_UNAME(xt),                         NULL, "RGB",        TC_DESC("RGB color support") },

        /* screen */
    { TC_UNAME(xt),                         NULL, "AN",         TC_DESC("turn on autonuke") },
    { TC_UNAME(xt),                         NULL, "AX",         TC_DESC("understands ANSI set default fg/bg color") },
    { TC_UNAME(xt),                         NULL, "C8",         TC_DESC("terminal shows bold as high-intensity colors") },
    { TC_UNAME(xt),                         NULL, "G0",         TC_DESC("can deal with ISO 2022 font selection") },
    { TC_UNAME(xt),                         NULL, "TF",         TC_DESC("add missing capabilities to screen") },
    { TC_UNAME(xt),                         NULL, "XT",         TC_DESC("Supports xterm OCS and mouse"), TC_FLAG(&tf_XT) },

        /* vim */
    { TC_UNAME(xt),                         NULL, "XF",         TC_DESC("terminal supports xterm focus in/out") },
    };

static const unsigned  ansicolor_map[] = {      /* BRIEF -> ANSI color map */
/*  BLACK,  BLUE,   GREEN,  CYAN,   RED,    MAGENTA,    BROWN,  WHITE   */
    0,      4,      2,      6,      1,      5,          3,      7,
    8,      12,     10,     14,     9,      13,         11,     15
    };


static const struct colormap {                  /* BRIEF -> XTERM color map */
    int     ident;
    int     c8;
    int     c16;
    int     c16_pc;
    int     c88_compat;     /* 8/16 color compat */
    int     c256_compat;    /* 8/16 color compat */
    int     c88;
    int     c256;

} color_map[] =  {
    /*  Ident       8       16      16/PC   88/C    256/C   88      256     */
    { BLACK,        0,      0,      0,      0,      0,      0,      0       },
    { BLUE,         4,      4,      1,      1,      4,      12,     12      },
    { GREEN,        2,      2,      2,      2,      2,      10,     10      },
    { CYAN,         6,      6,      3,      3,      6,      14,     14      },
    { RED,          1,      1,      4,      4,      1,      9,      9       },
    { MAGENTA,      5,      5,      5,      5,      5,      13,     13      },
    { BROWN,        3,      3,      6,      6,      130,    32,     130     },
    { WHITE,        7,      7,      7,      7,      7,      84,     248     },

    { GREY,         0,      8,      8,      8,      8,      7,      7       },
    { LTBLUE,       4,      12,     9,      12,     12,     43,     81      },
    { LTGREEN,      2,      10,     10,     10,     10,     61,     121     },
    { LTCYAN,       6,      14,     11,     14,     14,     63,     159     },
    { LTRED ,       1,      9,      12,     9,      9,      74,     224     },
    { LTMAGENTA,    5,      13,     13,     13,     13,     75,     225     },
    { YELLOW ,      3,      11,     14,     11,     11,     11,     11      },
    { LTWHITE,      7,      15,     15,     15,     15,     1,      15      },

    { DKGREY,       0,      0,      0,      0,      0,      82,     242     },
    { DKBLUE,       4,      4,      1,      1,      4,      4,      4       },
    { DKGREEN,      2,      2,      2,      2,      2,      2,      2       },
    { DKCYAN,       6,      6,      3,      3,      6,      6,      6       },
    { DKRED,        1,      1,      4,      4,      1,      1,      1       },
    { DKMAGENTA,    5,      5,      5,      5,      5,      5,      5       },
    { DKYELLOW,     3,      3,      6,      6,      130,    72,     130     },
    { LTYELLOW,     3,      11,     14,     11,     11,     78,     229     },

    { COLOR_NONE,   -1,     -1,     -1,     -1,     -1,     -1,     -1      }
    };


/*
 *  Scroll window
 */
static int              tt_top = -1;            /* Top of scroll region. */
static int              tt_bot = -1;            /* Bottom of scroll region. */


/*
 *  Color information
 */
static int              tt_colors = 8;          /* color depth (-1 unknown) */

static int              tt_defaultfg = NOCOLOR;
static int              tt_defaultbg = NOCOLOR;

static int              tt_fg = NOCOLOR;        /* foreground color */
static int              tt_bg = NOCOLOR;        /* background color */
static int              tt_style;               /* text standout/underline mode */

static int              tt_active = 0;          /* display status */
static int              tt_cursor = 0;          /* cursor status */

static vbyte_t          tt_hue = 0;             /* current attribute */

                                                /* BRIEF -> TERM-COLORS */
static int              tt_colormap[COLOR_NONE + 1];


/*  Function:           ttinit
 *      Initialize the terminal when the editor gets started up.
 *
 *  Calling Sequence:
 *>     ttinit
 *>         -> term_init
 *>     ttopen
 *>         -> term_open
 *>     ttready
 *>         -> term_ready
 *>     macro tty/<terminal-type>
 *>         [ttfeature]
 *>     ttdisplay
 *>         -> term_display
 *>     [ttfeature]
 *
 *>     ttfeature
 *>     ttprocess
 *>         -> term_control
 *>     ttwinch
 *>         -> term_control
 *
 *>     ttclose
 *>         -> term_close
 *
 *  Parameters:
 *      none.
 *
 *  Configuration:
 *
 *      Terminal identification.
 *
 *          o TERM -
 *              General terminal identification.
 *          o KONSOLE_DCOP or KONSOLE_DBUS_SESSION -
 *              kconsole identification.
 *          o XTERM_VERSION -
 *              xterm version.
 *          o LINES/COLUMNS -
 *              Terminal size.
 *
 *      Terminal capabilities.
 *
 *          o COLORTERM_BCE -
 *              Assume BCE capabilities.
 *          o COLORTERM -
 *              slang style color terminal configuration, if set color assumed.
 *          o OSC 11
 *              Default background; rgb luminance.
 *          o NCURSES_NO_UTF8_ACS -
 *              When running in a UTF-8 locale several terminals (including Linux console and GNU screen)
 *              ignore alternative character selection. If set use unicode box drawing characters in all
 *              cases.
 *          o NCURSES_ASSUMED_COLORS -
 *              Foreground and background colors.
 *          o COLORFGBG -
 *              rxvt/mrxvt terminal default color specification.
 *          o DEFAULT_COLORS -
 *              Default colors.
 *
 *  Returns:
 *      nothing.
 */
void
ttinit(void)
{
    const char *term;

    trace_log("ttinit()\n");

    /*
     *  profile
     */
    x_scrfn.scr_open    = term_open;
    x_scrfn.scr_ready   = term_ready;
    x_scrfn.scr_feature = term_feature;
    x_scrfn.scr_display = term_display;
    x_scrfn.scr_control = term_control;
    x_scrfn.scr_close   = term_close;

    x_scrfn.scr_move    = term_move;
    x_scrfn.scr_cursor  = term_cursor;
    x_scrfn.scr_winch   = term_sizeget;

    x_scrfn.scr_names   = term_names;
    x_scrfn.scr_beep    = term_beep;

    x_scrfn.scr_clear   = term_clear;
    x_scrfn.scr_print   = NULL;
    x_scrfn.scr_putc    = term_putc;
    x_scrfn.scr_flush   = term_flush;

    x_scrfn.scr_insl    = term_insl;
    x_scrfn.scr_dell    = term_dell;
    x_scrfn.scr_eeol    = term_eeol;
    x_scrfn.scr_repeat  = term_repeat;

    term = ttisetup();

    /*
     *  build attributes, non-xterm
     */
    if (isterm(term, "linux")) {                /* linux console */
        t_attributes = TA_LINUX | TA_DARK;

    } else if (isterm(term, "cygwin")) {
        const char *cygwin = ggetenv("CYGWIN");

        if (cygwin) {
            trace_log("\tCYGWIN:%s\n", cygwin);
            if (strstr(cygwin, "codepage:oem")) {
                if (xf_disptype < 0) {          /* 8bit terminal */
                    xf_disptype = DISPTYPE_8BIT;
                }
                x_pt.pt_codepage = 850;

            } else if (strstr(cygwin, "codepage:utf8")) {
                if (xf_disptype < 0) {          /* UTF8 enabled */
                    xf_disptype = DISPTYPE_UTF8;
                }
            }
        }
        t_attributes = TA_CYGWIN|TA_XTERMLIKE|TA_DARK;

#if defined(linux)
    } else if (isterm(term, "console") ||       /* console, con80x25 etc */
            (0 == strncmp(term, "con", 3) && term[3] && isdigit(term[3]))) {
        t_attributes = TA_LINUX|TA_DARK;
#endif

    } else if (isterm(term, "konsole") ||
                getenv("KONSOLE_DCOP") || getenv("KONSOLE_DBUS_SESSION")) {
        t_attributes = TA_KONSOLE|TA_XTERMLIKE;

    } else if (isterm(term, "screen")) {
        t_attributes = TA_SCREEN;               /* screen[.linux] */
        if (0 == isterm(term, "screen.linux")) {
            t_attributes |= TA_DARK;
        }

    } else if (strcmp(term, "vt52") != 0 &&     /* vt100+ */
            term[0] == 'v' && term[1] == 't' && term[2] >= '1' && term[2] <= '9') {
        t_attributes = TA_VT100LIKE;

    } else {
        t_attributes = term_xtermlike(term);
#if defined(__CYGWIN__)
        if (ggetenv("WT_SESSION")) {
            t_attributes |= TA_DARK;            /* ms-terminal */
        } else if (ggetenv("COMSPEC")) {
            t_attributes |= TA_DARK;            /* cmd/mintty */
        }
#endif /*__CYGWIN__*/
    }

    if (hasfeature(term, "rv")) {
        t_attributes |= TA_LIGHT;               /* rv = reverse video (black on white) */

    } else if (hasfeature(term, "m") || hasfeature(term, "mono")) {
        t_attributes |= TA_MONO;                /* m  = monochrome, suppress color support */
    }

    trace_log("terminal: %s (0x%08x=%s%s%s%s%s%s)\n", term, t_attributes,
        (t_attributes & TA_VT100LIKE ? "vt100like," : ""),
        (t_attributes & TA_XTERM ? "xterm," : ""),
            (t_attributes & TA_XTERMLIKE ? "xtermlike," : ""),
        (t_attributes & TA_LIGHT ? "light," : ""),
        (t_attributes & TA_DARK ? "dark," : ""),
        (t_attributes & TA_MONO ? "mono," : ""));

    /*
     *  Load terminal values.
     */
    term_config();
    term_fgbg();

    /*
     *  Fixup defective termcap/terminfo databases
     */
    if (NULL != ggetenv("COLORTERM_BCE")) {
        tf_be = 1;                              /* slang compat override */
    }

    if (NULL == tc_Color_Fg || NULL == tc_Color_Bg) {
        tc_Color_Fg = tc_Color_Bg = NULL;
    }

    if (NULL == tc_ANSI_Color_Fg || NULL == tc_ANSI_Color_Bg) {
        tc_ANSI_Color_Fg = tc_ANSI_Color_Bg = NULL;
    }

    if (NULL == tc_graphic_pairs &&
            (t_attributes & TA_VT100LIKE)) {    /* VT1xx */
        tc_acs_start  = "\016";
        tc_acs_end    = "\017";
        tc_acs_enable = "\033)0";
    }

    if (NULL == tc_graphic_pairs &&
            isterm(term, "aixterm")) {          /* aixterm (VT102) */
        tc_acs_start  = "\016";
        tc_acs_end    = "\017";
        tc_acs_enable = "\033(B\033)0";
    }

    if (tc_graphic_pairs && NULL == tc_acs_start) {
        tc_acs_start  = "\016";
        tc_acs_end    = "\017";
    }
                                                /* VT2xx+ */
    if (((t_attributes & TA_VT100LIKE) && term[2] != '1') ||
                (t_attributes & (TA_LINUX|TA_XTERMLIKE)) ) {
        if (NULL == tc_pDL) tc_pDL = "\033[%dM";
        if (NULL == tc_pAL) tc_pAL = "\033[%dL";
        if (NULL == tc_cm)  tc_cm  = "\033[%i%d;%dH";
    }

    if (t_attributes & (TA_LINUX|TA_XTERM)) {   /* VT2xx */
        if (NULL == tc_cb)  tc_cb = "\033[1K";
        if (NULL == tc_ce)  tc_ce = "\033[K";
    }

    /* stand-out mode equiv checks */
    if (tc_se) {
        if (tc_ZH && 0 == strcmp(tc_ZH, tc_se)) {
            tc_ZH = tc_se;

        } else if (NULL == tc_ZH) {
            tc_mr = NULL;                       /* ignore or assume same as 'se' */
        }

        if (tc_ue && 0 == strcmp(tc_us, tc_se)) {
            tc_ue = tc_se;
        }

        if (tc_ZR && 0 == strcmp(tc_ZR, tc_se)) {
            tc_ZR = tc_se;
        }
    }

    if (NULL == tc_do && tf_NL) {
        tc_do = "\n";
    } else if (tc_do && tc_do[0] == '\n' && !tf_NL) {
        tc_do = NULL;
    }

    if (NULL == tc_le && NULL == tc_bc &&
            (tf_bs || (t_attributes & TA_XTERMLIKE))) {
        tc_bc = "\b";
    }

    /*
     *  Verify min requirements
     */
    if (tf_gn) {
        fprintf(stderr, "Generic terminal '%s', you have not specified your real terminal type.\n", term);
        exit(1);

    } else if (tf_hc) {
        fprintf(stderr, "Hard copy terminal '%s', not supported.\n", term);
        exit(1);

    } else if (tf_xonoff) {
        fprintf(stderr, "Terminal '%s' requires xon/xoff, not supported (try --curses).\n", term);
        exit(1);

    } else if (tn_sg >= 1) {
        fprintf(stderr, "Terminal '%s' using magic-cookie's, not supported (try --curses).\n", term);
        exit(1);

    } else if (NULL == tc_cm && (NULL == tc_cv || NULL == tc_ch)) {
        fprintf(stderr, "Terminal '%s' missing cursor move capabilities, not supported.\n", term);
        exit(1);

    } else if (tf_xs >= 1 || tf_xt >= 1) {
        fprintf(stderr, "Terminal '%s' requires old-style attribute handling, not supported.\n", term);
        exit(1);
    }

#if defined(__CYGWIN__)
    /*
     *  o retrieve display code page
     *  o plus enable tf_xn as the terminal cursor wrap misbehaves
     *      then in full screen mode resulting in screen corruption.
     */
    if (-1 == x_pt.pt_codepage) {
        x_pt.pt_codepage = GetConsoleOutputCP();
        trace_log("\tcodepage: %d\n", x_pt.pt_codepage);
    }
    if (TA_CYGWIN & t_attributes) {
        x_pt.pt_attributes |= TF_ACYGWIN;       /* Cygwin terminal */
        tf_xn = 1;
    }
#endif  /*__CYGWIN__*/

    if (tf_km) {
        x_pt.pt_attributes |= TF_AMETAKEY;
    }

    if (0 == x_pt.pt_codepage) {
        x_pt.pt_codepage = 437;                 /* default */
    }
}


static int
isterm(const char *term, const char *name)
{
    const size_t tlen = (size_t)strlen(term);
    const size_t nlen = (size_t)strlen(name);

    if (tlen >= nlen) {                         /* xxxx[\0.-] */
        if (0 == memcmp(term, name, nlen)) {    /* dot, allow <screen.xterm> */
            return (term[nlen] == '\0' || term[nlen] == '-' || term[nlen] == '.');
        }
    }
    return 0;
}


static int
hasfeature(const char *term, const char *what)
{
    const size_t wlen = (size_t)strlen(what);
    const char *elm;

    for (elm = strchr(term, '-'); elm; elm = strchr(elm, '-')) {
        ++elm;
        if (0 == strncmp(elm, what, wlen)) {    /* -xxxx[\0-] */
            if (elm[wlen] == '\0' || elm[wlen] == '-') {
                return 1;
            }
        }
    }
    return 0;
}


static void
term_config(void)
{
    unsigned i, fkeys = 0;
    const char *cp;

    trace_log("termcap:\n");

    trace_log("  String:\n");
    for (i = 0; i < (sizeof(term_strings)/sizeof(term_strings[0])); i++) {
        /*
         *  string values
         */
        TermString_t *ti = term_strings + i;
        const Term_t *term = &ti->term;
        const char *name = ttiname(term);

        if (name && NULL != (cp = ttigetstr(term))) {
            const char **token = ti->stoken;

            ti->svalue = cp;
            if (token) {
                *token = ti->svalue;
            }
            trace_log("\t%-24s %-50s%c %-6s : %s\n",
                term->termfname, term->comment, (token ? '*' : ' '), name, c_string(cp));

            if (token == &tc_graphic_pairs) {
                acs_dump(cp);
                x_pt.pt_attributes |= TF_AGRAPHICCHARACTERS;

            } else if (token == &tc_box_characters) {
                const char *t_acs;

                if (NULL == tc_graphic_pairs &&
                            (NULL != (t_acs = acs_box_characters(cp)))) {
                    acs_dump(t_acs);
                    tc_graphic_pairs = t_acs;
                    x_pt.pt_attributes |= TF_AGRAPHICCHARACTERS;
                }
            }
        }
    }

    fkeys = 0;
    trace_log("  Keys:\n");
    for (i = 0; i < (sizeof(term_keys)/sizeof(term_keys[0])); ++i) {
        /*
         *  keys
         */
        TermKey_t *ti = term_keys + i;
        const Term_t *term = &ti->term;
        const char *name = ttiname(term);

        if (name && NULL != (cp = ttigetstr(term))) {
            const size_t kcode = (size_t)ti->key;

            ti->svalue = cp;                /* loaded later by ttkeys() */

            trace_log("\t%-24s %-50s%c %-6s : %s\n",
                term->termfname, term->comment, (kcode ? '*' : ' '), name, c_string(cp));

            if (kcode >= F(1) && kcode <= F(10)) {
                ++fkeys;
            }
        }
    }

    if (fkeys >= 10) {                      /* have all 10 function keys */
        x_pt.pt_attributes |= TF_AFUNCTIONKEYS;
    }

    trace_log("  Numeric:\n");
    for (i = 0; i < (sizeof(term_numbers)/sizeof(term_numbers[0])); ++i) {
        /*
         *  numbers
         */
        TermNumeric_t *ti = term_numbers + i;
        const Term_t *term = &ti->term;
        const char *name = ttiname(term);

        if (name) {
            ti->ivalue = ttigetnum(term);
            if (ti->itoken) {
                *ti->itoken = ti->ivalue;
            }
            trace_log("\t%-24s %-50s%c %-6s : %d\n",
                term->termfname, term->comment, (ti->itoken ? '*' : ' '), name, ti->ivalue);
        }
    }

    trace_log("  Boolean/Flags:\n");
    for (i = 0; i < (sizeof(term_flags)/sizeof(term_flags[0])); ++i) {
        /*
         *  flags
         */
        TermNumeric_t *ti = term_flags + i;
        const Term_t *term = &ti->term;
        const char *name = ttiname(term);

        if (name) {
            ti->ivalue = ttigetflag(term);
            if (ti->itoken) {
                *ti->itoken = ti->ivalue;
            }
            trace_log("\t%-24s %-50s%c %-6s : %d\n",
                term->termfname, term->comment, (ti->itoken ? '*' : ' '), name, ti->ivalue);
        }
    }
}

/*  Function:           ttdefaultscheme
 *      Retrieve the derived/guessed default background color based on the either
 *      the published terminal background or the terminal type.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      Name of default color scheme dark or light.
 */
const char *
ttdefaultscheme(void)
{
    int isdark = 0;

    if (x_pt.pt_schemedark >= 0) {              /* explicit configuration */
        isdark = x_pt.pt_schemedark;

    } else if ((TA_LUMINACE_DARK|TA_LUMINACE_LIGHT) & t_attributes) {
        if (TA_LUMINACE_DARK & t_attributes) {  /* luminace calc */
            isdark = 1;
        }

    } else if ((TA_DARK|TA_LIGHT) & t_attributes) {
        if (TA_DARK & t_attributes) {           /* assumed */
            isdark = 1;
        }

    } else {
        if (tt_defaultbg != NOCOLOR) {          /* dark=0-6 or 8 */
            if (tt_defaultbg <= 6 || 8 == tt_defaultbg) {
                isdark = 1;
            }

        } else {                                /* generally dark */
            if (TA_DARK & t_attributes) {
                isdark = 1;
            }
        }
    }

    x_pt.pt_schemedark = isdark;
    trace_log("ttdefaultscheme=%s\n", (isdark ? "dark" : "light"));
    return (isdark ? "dark" : "light");
}


/*  Function:           term_open
 *      Open the console.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
term_open(scrprofile_t *profile)
{
    io_device_add(TTY_INFD);                    /* stream registration */
    sys_initialise();
    term_attributes();
    term_identification();                      /* terminal identification */
    term_ocs_color(11);
    term_sizeget(&profile->sp_rows, &profile->sp_cols);
    profile->sp_colors = tt_colors;
}


/*  Function:           term_close
 *      Close the console.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
term_close(void)
{
    term_tidy();
    sys_shutdown();
    tty_egaflag = -1;
}


/*  Function:           term_ready
 *      Run-time initialisation.
 *
 *  Parameters:
 *      repaint - *true* if the screen should be repainted.
 *      profile - console profile.
 *
 *  Returns:
 *      nothing.
 */
static void
term_ready(int repaint, scrprofile_t *profile)
{
    static unsigned once;

    trace_log("term_ready(%d)\n", repaint);

    /*
     *  dump configuration
     */
    if (! once) {
        const char *o_CS = tc_cs;

        if (! xf_scrollregions) {
            tc_cs = tc_cS = NULL;               /* disable scrolling region */
        }

        if (! tc_ce) {                          /* erasing a line */
            tty_tceeol = ttcols();
        } else {
            tty_tceeol = term_cost(tc_ce);
        }

        if (tc_cs && tc_sr) {                   /* inserting a line */
            tty_tcinsl = term_cost(tc_cs) * 2 + term_cost(tc_sr);
        } else if (tc_pAL) {
            tty_tcinsl = term_cost(tc_pAL);
        } else if (tc_al) {
            tty_tcinsl = term_cost(tc_al);
        } else {
            tty_tcinsl = 0xffff;
        }

        if (tc_cs) {                            /* delete a line */
            tty_tcdell = term_cost(tc_cs) * 2 + 1;
        } else if (tc_pDL) {
            tty_tcdell = term_cost(tc_pDL);
        } else if (tc_dl) {
            tty_tcdell = term_cost(tc_dl);
        } else {
            tty_tcdell = 0xffff;
        }
                                                /* we can ins/del lines */
        t_insdel = (tc_al || tc_pAL) && (tc_dl || tc_pDL);
        t_padchar = (tf_npc ? -1 : (tc_pc ? *tc_pc : 0));
        t_acs_locale_breaks = acs_locale_breaks();

        trace_log("TTY summary:\n");

        if (-1 == t_padchar) {
            trace_log("\tNo padding character\n");
        } else {
#if defined(HAVE_OSPEED)
            trace_log("\tPadding character %d, at ospeed %d\n", t_padchar, ospeed);
#else
            trace_log("\tPadding character %d, at baud %d\n", t_padchar, baudrate());
#endif
        }

        if (t_insdel) {
            trace_log("\tInsert/delete scrolling available.\n");
        }

        if (o_CS) {
            trace_log("\tScroll regions available.\n");
        }

        if (! xf_graph) {
            trace_log("\tAlternative character support disabled.\n");
        } else if (tc_acs_enable || x_pt.pt_graphics_mode[0]) {
            trace_log("\tAlternative character support available.\n");
        }
        if (t_acs_locale_breaks) {
            trace_log("\tlocale breaks alternative character support.\n");
        }

        if (xf_noinit) {
            trace_log("\ttermcap init/reinit disabled.\n");
        }

        if (xf_nokeypad)  {
            trace_log("\ttermcap keypad init/reinit disabled.\n");
        }

        trace_log("\tDeleting lines using %s\n",
            ((tc_cs) ? "scrolling regions" : t_insdel ? "line del/ins" : "hard refresh"));
        trace_log("\tInserting lines using %s\n",
            ((tc_cs && tc_sr) ? "scrolling regions" : t_insdel ? "line del/ins" : "hard refresh"));
        trace_log("\t%-55s : %d\n", "Erase line cost", tty_tceeol);
        trace_log("\t%-55s : %d\n", "Insert line cost", tty_tcinsl);
        trace_log("\t%-55s : %d\n", "Delete line cost", tty_tcdell);
        trace_log("\t%-55s : %d\n", "Colors", tt_colors);
        trace_log("\t%-55s : %d\n", "  Default fg", tt_defaultfg);
        trace_log("\t%-55s : %d\n", "  Default bg", tt_defaultbg);
        ++once;
    }

    /*
     *  configure terminal
     */
    term_colorreset();

    if (repaint) {
        if (x_pt.pt_init[0])                    /* terminal specific init */
            ttputctl2(x_pt.pt_init, 0, 0);
        xterm_colors(t_colorsuser);
        if (! xf_noinit)
            ttputpad(tc_ti);                    /* enable cup (cursor addressing) */
        if (! xf_nokeypad)
            ttputpad(tc_ks);                    /* put terminal in ``keypad-transmit'' */
        if (xf_graph)
            ttputpad(tc_acs_enable);            /* enable alt character set */
        ttputpad(tc_vs ? tc_vs : tc_ve);        /* enable enhanced cursor */
        if (tf_am)
            ttputpad(tc_am_on);                 /* enable automatic margins */
        ttputpad(tc_mm);                        /* enable meta key-codes */
    }

    /*
     *  terminal specific specials
     */
    if (t_attributes & TA_CYGWIN) {
        if (xf_cygwinkb) {
            ttpush("\033[?2000h");              /* enable RAW mode (gives us WIN32 scancodes) */
        }

    } else if (t_attributes & TA_MINTTY) {
        if (xf_mouse) {                         /* mouse enabled? */
            ttpush("\033[?7786h");              /* mouse-wheel reports only */
        }

    } else if (t_attributes & TA_VT100LIKE) {
        ttpush("\033=\033[?1]");                /* enable cursor keys */
    }

    if (profile) {
        profile->sp_lastsafe = term_lastsafe();
        profile->sp_colors = tt_colors;
    }

    tt_style  = 0;
    tt_cursor = 1;
    tt_active = 1;
    term_flush();
}


/*  Function:           term_feature
 *      Signal a terminal feature change.
 *
 *  Parameters:
 *      ident -   Feature identifier.
 *
 *  Returns:
 *      nothing.
 */
static void
term_feature(int ident, scrprofile_t *profile)
{
    trace_log("term_feature(%d)\n", ident);

    switch (ident) {
    case TF_INIT:
        if (tty_open && x_pt.pt_init[0]) {
            ttputctl2(x_pt.pt_init, 0, 0);
        }
        break;

    case TF_COLORDEPTH:
    case TF_DEFAULT_FG:
    case TF_DEFAULT_BG:
    case TF_XTERM_PALETTE:
        if (tty_open) {
            term_colors();
            profile->sp_colors = tt_colors;
        }
        break;

    case TF_COLORMAP:                           /* terminal color palette */
        if (x_pt.pt_colormap[0]) {
            strxcpy(t_colorsuser, x_pt.pt_colormap, sizeof(t_colorsuser));
            if (tty_open) {
                xterm_colors(t_colorsuser);
            }
        }
        break;

    case TF_COLORPALETTE: {                     /* user defined palette */
            const char *cursor = x_pt.pt_colorpalette;
            unsigned col = 0;

            for (col = 0; col < (sizeof(tt_colormap)/sizeof(tt_colormap[0])); ++col) {
                if (cursor && *cursor) {
                    if (isdigit(*cursor)) {
                        int val;

                        if ((val = atoi(cursor)) >= 0) {
                            tt_colormap[col] = val;
                        }
                    }
                    if (NULL != (cursor = strchr(cursor, ','))) {
                        ++cursor;
                    }
                }
            }
            x_pt.pt_xtpalette = -2;
        }
        break;

    case TF_ENCODING:
        break;
    case TF_UNICODE_VERSION:
        if (x_pt.pt_unicode_version[0]) {
            ucs_width_set(x_pt.pt_unicode_version);
        }
        break;
    }
}


/*  Function:           term_display
 *      Invoked upon the display being enabled.
 *
 *      This interface is hooked into the display_windows() primitive which (by default) is
 *      post execution of any terminal specific macros, for example <tty/xterm.cr>
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
term_display(void)
{
    trace_log("term_display()\n");

    if (term_isutf8()) {                        /* auto-detect UTF8 term support */
        x_pt.pt_attributes |= TF_AUTF8ENCODING; /* publish UTF8 */
        if (0 == x_pt.pt_encoding[0]) {
            strcpy(x_pt.pt_encoding, "UTF-8");
        }
        if (xf_disptype < 0) {
            xf_disptype = DISPTYPE_UTF8;
        }
        x_display_ctrl |= DC_UNICODE;

    } else {
        if (xf_disptype < 0) {                  /* default disp-type */
            if (vtis8bit()) {
                if (0 == x_pt.pt_encoding[0]) {
                    strcpy(x_pt.pt_encoding, "latin1");
                }
                xf_disptype = DISPTYPE_8BIT;
            } else {
                if (0 == x_pt.pt_encoding[0]) {
                    strcpy(x_pt.pt_encoding, "us-ascii");
                }
                xf_disptype = DISPTYPE_7BIT;
            }
        }
    }

    if (! xf_graph) {                           /* no graphic */
        ttboxcharacters(TRUE);
    }
}


static int
term_control(int action, int param, ...)
{
    switch (action) {
    case SCR_CTRL_NORMAL:       /* normal color */
        if (tty_open) {
            term_colorreset();
        }
        break;

    case SCR_CTRL_SAVE:
        if (tty_open) {
            ttmove(ttrows()-1, 0);
            if (param) {
                term_tidy();
            } else {
                term_colorreset();
                term_styleoff();
                tteeol();
            }
        }
        break;

    case SCR_CTRL_RESTORE:
        if (! tty_open) {
            ttopen();
        }
        term_ready(TRUE, NULL);
        break;

    case SCR_CTRL_COLORS:       /* color change */
        break;

    default:
        return -1;
    }
    return 0;
}


/*  Function:           ttisetup
 *      TERMINFO/TERMCAP setup.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      Terminal name on success, otherwise the interface exits.
 */
static const char *
ttisetup(void)
{
    const char *term;

    if (NULL == (term = ggetenv("TERM"))) {
        fprintf(stderr, "Environment variable TERM not defined!\n");
        exit(1);
    }

#if defined(HAVE_TERMINFO)
    XF_TERMINFO {
        trace_log("TERMINFO(%s) open\n", term);
        if (setupterm((char *)term, fileno(stdout), NULL) != 0) {
            fprintf(stderr, "Terminal type '%s' not found in terminfo database.\n", term);
            exit(1);
        }
    }
#endif
#if defined(HAVE_TERMCAP)
    XF_TERMCAP {
        tcapcursor = tcapstrings;
        memset(tcapstrings, 0, sizeof(tcapstrings));

        trace_log("TERMCAP(%s) open\n", term);
        if (tgetent(tcapbuffer, (char *)term) != 1) {
            fprintf(stderr, "Terminal type '%s' not found in termcap.\n", term);
            exit(1);
        }
    }
#endif

    return term;
}


/*  Function:           ttiname
 *      Retrieve the attribute name.
 *
 *  Parameters:
 *      ti -      Attribute specification.
 *
 *  Returns:
 *      Attribute name.
 */
static const char *
ttiname(const Term_t *ti)
{
    const char *name = NULL;

#if defined(HAVE_TERMINFO)
    XF_TERMINFO name = ti->terminfoname;
#endif
#if defined(HAVE_TERMCAP)
    XF_TERMCAP  name = ti->termcapname;
#endif
    return name;
}


/*  Function:           ttigetstr
 *      t[i]getstr() interface.
 *
 *  Parameters:
 *      ti -      Attribute specification.
 *
 *  Returns:
 *      Sequence buffer address, otherwis NULL.
 */
static const char *
ttigetstr(const Term_t *ti)
{
    char *s = NULL;

#if defined(HAVE_TERMINFO)
    XF_TERMINFO {
        const char *name = ti->terminfoname;

        s = tigetstr((char *) name);
        if ((char *)-1 == s) {                  /* 'name' is not a string capability */
            if (0 == ti->userdef)
                trace_log("\ttgetstr(%s) = unknown\n", name);
            s = NULL;
        }
    }
#endif
#if defined(HAVE_TERMCAP)
    XF_TERMCAP {
        /*
         *  Under normal conditions tgetstr() should perform as descripted but doesn't on
         *  all systems so workout known issues.
         *
         *      "The tgetstr routine returns the string entry for id, or zero if it is not
         *       available. Use tputs to output the returned string. The return value will also
         *       be copied to the buffer pointed to by area, and the area value will be updated
         *       to point past the null ending this value."
         *
         *  Note, only seen under an older curses implementation.
         */
        const char *name =  ti->termcapname;
        char *ocursor = tcapcursor;             /* current storage addr */

        if (NULL != (s = tgetstr((char *) name, &tcapcursor)) && *s) {
            const int slen = strlen(s);

            if (ocursor == tcapcursor) {        /* broken tgetstr() */
                strcpy(ocursor, s);
                tcapcursor = ocursor + slen + 1;
            }
            s = ocursor;                        /* lookup result */
            if (tcapcursor >= tcapstrings + TC_SLEN) {
                fprintf(stderr, "Terminal description too large (>%dk)\n", (TC_SLEN/1024) + 1);
                exit(1);
            }
            assert(tcapcursor == ocursor + slen + 1);
        } else {
            s = NULL;                           /* not found, zero length */
        }
    }
#endif
    return s;
}


/*  Function:           ttigetnum
 *      t[i]getnum() interface.
 *
 *  Parameters:
 *      ti -      Attribute specification.
 *
 *  Returns:
 *      Attribute value, otherwise -1.
 */
static int
ttigetnum(const Term_t *ti)
{
    const char *name = "n/a";
    int num = -1;

#if defined(HAVE_TERMINFO)
    XF_TERMINFO {
        name = ti->terminfoname;
        num  = tigetnum((char *) name);
    }
#endif
#if defined(HAVE_TERMCAP)
    XF_TERMCAP {
        name = ti->termcapname;
        num  = tgetnum((char *) name);
    }
#endif
    if (num < -1 && 0 == ti->userdef) {
        trace_log("\ttgetnum(%s) = error (%d)\n", name, num);
    }
    return (num >= -1 ? num : -1);              /* -1 or greater */
}


/*  Function:           ttigetflag
 *      t[i]getflag() interface.
 *
 *  Parameters:
 *      ti - Attribute specification.
 *
 *  Returns:
 *      Attribute value of either 0 or 1.
 */
static int
ttigetflag(const Term_t *ti)
{
    const char *name = "n/a";
    int flag = -1;

#if defined(HAVE_TERMINFO)
    XF_TERMINFO {
        name = ti->terminfoname;
        flag = tigetflag((char *) name);
    }
#endif
#if defined(HAVE_TERMCAP)
    XF_TERMCAP {
        name = ti->termcapname;
        flag = tgetflag((char *) name);
    }
#endif
    if (flag < 0 && 0 == ti->userdef) {
        trace_log("\ttgetflag(%s) = error (%d)\n", name, flag);
    }
    return (flag >= 0 ? flag : 0);              /* 0 or 1 */
}


int
ttisetstr(const char *tag, int taglen, const char *value)
{
    __CUNUSED(tag) __CUNUSED(taglen) __CUNUSED(value)
    return -1;
}


int
ttisetnum(const char *tag, int taglen, const char *value)
{
    __CUNUSED(tag) __CUNUSED(taglen) __CUNUSED(value)
    return -1;
}


int
ttisetflag(const char *tag, int taglen, const char *value)
{
    __CUNUSED(tag) __CUNUSED(taglen) __CUNUSED(value)
    return -1;
}


/*  Function:           acs_dump
 *      Alternative character dump.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Returns:
 *      nothing.
 */
static void
acs_dump(const char *bp)
{
    const unsigned char *p = (unsigned char *)bp;
    unsigned char ident, ch;
    unsigned i;

    while (*p) {
        const char *desc = "unknown";

        ident = *p++;
        if (! isprint(ident)) {
            continue;                           /* unsure, yet needed for rxvt-cygwin */
        }
        ch = *p++;

        for (i = 0; i < (sizeof(term_characters)/sizeof(term_characters[0])); ++i)
            if (term_characters[i].ident == ident) {
                desc = term_characters[i].name;
                break;
            }

        trace_log("\t\t%-30s %c/0x%x : %u/0x%x\n", desc, ident, ident, ch, ch);
    }
}


/*  Function:           acs_boxcharacters
 *      Alternative character box character import.
 *
 *  Parameters:
 *      bx - Box character spec.
 *
 *  Returns:
 *      Converted box characters into an acs specification.
 */
static const char *
acs_box_characters(const char *bx)
{
    char acs[32], *t_acs = acs;

#define acs_push_bx(_i, _c)     *t_acs++ = _i, *t_acs++ = _c

    acs_push_bx(TACS_ULCORNER,  bx[0]);         /* 'l' */
    acs_push_bx(TACS_HLINE,     bx[1]);         /* 'q' */
    acs_push_bx(TACS_URCORNER,  bx[2]);         /* 'k' */
    acs_push_bx(TACS_VLINE,     bx[3]);         /* 'x' */
    acs_push_bx(TACS_LRCORNER,  bx[4]);         /* 'j' */
    acs_push_bx(TACS_LLCORNER,  bx[5]);         /* 'm' */
    acs_push_bx(TACS_TTEE,      bx[6]);         /* 'w' */
    acs_push_bx(TACS_RTEE,      bx[7]);         /* 'u' */
    acs_push_bx(TACS_BTEE,      bx[8]);         /* 'v' */
    acs_push_bx(TACS_LTEE,      bx[9]);         /* 't' */
    acs_push_bx(TACS_PLUS,      bx[10]);        /* 'n' */

#undef acs_push_bx
    *t_acs = 0;

    return chk_salloc(acs);
}


/*  Function:           acs_locale_breaks
 *      Check for known cases where a UTF-8 locale breaks the alternate character set.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      TRUE or FALSE.
 */
static int
acs_locale_breaks(void)
{
    const char *env;

    if ((env = ggetenv("NCURSES_NO_UTF8_ACS")) != 0) {
        return atoi(env);                       /* ncurses compatibility */

    } else if ((env = ggetenv("TERM")) != 0) {
        if (strstr(env, "linux")) {
            return TRUE;
        }

        if (strstr(env, "screen") &&
                ((env = ggetenv("TERMCAP")) != 0 && strstr(env, "screen") != 0) &&
                strstr(env, "hhII00") != 0) {

#define IS_CTRLN(s)         ((s) != 0 && strstr(s, "\016") != 0)
#define IS_CTRLO(s)         ((s) != 0 && strstr(s, "\017") != 0)

            if (IS_CTRLN(tc_acs_start) || IS_CTRLO(tc_acs_start) ||
                    IS_CTRLN(set_attributes) || IS_CTRLO(set_attributes)) {
                return TRUE;
            }

#undef  IS_CTRLN
#undef  IS_CTRLO
        }
    }
    return FALSE;
}


/*  Function:           term_xtermlike
 *      Determine the current terminal is an xterm style.
 *
 *  Parameters:
 *      term - Terminate name.
 *
 *  Returns:
 *      Associated flags, if any.
 */
static TAttributes_t
term_xtermlike(const char *term)
{
    static const struct {
        const char *name;
        TAttributes_t flags;
    } xtermlike[] = {
        { "xterm", TA_XTERM },                  /* Generic */
        { "mintty", TA_MINTTY },                /* Non-standard. normally "xterm-256color" */
        { "putty", TA_PUTTY | TA_DARK },
        { "ms-terminal", TA_MSTERMINAL | TA_DARK },
        { "gnome", TA_GNOME },
        { "vte", TA_GNOME },
        { "xterm-kitty", TA_KITTY },
        { "alacritty", TA_ALACRITTY },
        { "iterm", TA_ITERM },
        { "iterm2", TA_ITERM },
        { "iTerm.app", TA_ITERM },
        { "iTerm2.app", TA_ITERM },
        { "hterm", TA_HTERM },
        { "wezterm", TA_WEZTERM },
        { "aixterm" },                          /* AIX */
        { "rxvt" },                             /* [ou]r xvt */
        { "urxvt" },                            /* Unicode rxvt */
        { "Eterm" },                            /* Color vt102 terminal emulator */
        { "dtterm" }                            /* CDE terminal */
        };
    TAttributes_t ret = 0;

    if (term) {
        unsigned i;

        for (i = 0; i < (unsigned)(sizeof(xtermlike)/sizeof(xtermlike[0])); ++i)
            if (isterm(term, xtermlike[i].name)) {
                ret = xtermlike[i].flags | TA_XTERMLIKE;
                break;
            }
    }

    trace_log("\txtermlike(%s) : 0x%lx\n", (term ? term : ""), (unsigned long)ret);
    x_pt.pt_xtcompat = ret;
    return ret;
}


/*  Function:           ttkeys
 *      Map keys from termcap database to our keys. These are often overridden by the term
 *      macros, but at least we can set up some sensible default that are likely to work.
 *
 *  Parameters:
 *      repaint - *true* if the screen should be repainted.
 *
 *  Returns:
 *      nothing.
 */
void
ttkeys(void)
{
    unsigned i;

    trace_log("ttkeys()\n");

    /*
     *  Keys
     */
    for (i = 0; i < (sizeof(term_keys)/sizeof(term_keys[0])); ++i)
        if (term_keys[i].svalue && term_keys[i].key) {
            key_define_key_seq(term_keys[i].key, term_keys[i].svalue);
        }

    /*
     *  Color
     */
    if (tf_Colors > 2 /*user specification*/ ||
            (tc_Color_Fg && tc_Color_Bg) || (tc_ANSI_Color_Fg && tc_ANSI_Color_Bg) ||
                ggetenv("COLORTERM") /*override*/) {
        x_pt.pt_color = TRUE;
    }

    term_colors();

    /*
     *  Graphic (alt) characters
     */
    for (i = 0; i < sizeof(tc_acs_map); ++i) {  /* one-to-one */
        tc_acs_map[i] = (unsigned char)(i < 32 ? ' ' : i);
    }

    if (tc_acs_start && xf_graph /*-1 or 1*/) {
        const char *p;

        if (NULL == (p = tc_graphic_pairs)) {   /* build character map */
            p = tc_graphic_default;
        }

        while (*p) {
            i = *p++;
            if (! isprint(i)) {
                continue;                       /* unsure, yet needed for rxvt-cygwin */
            }
            tc_acs_map[i & 0x7f] = *p++;
        }

        x_pt.pt_tty_graphicsbox = TRUE;
        x_pt.pt_top_left[0]     = tc_acs_map[TACS_ULCORNER];
        x_pt.pt_top_right[0]    = tc_acs_map[TACS_URCORNER];
        x_pt.pt_bot_left[0]     = tc_acs_map[TACS_LLCORNER];
        x_pt.pt_bot_right[0]    = tc_acs_map[TACS_LRCORNER];
        x_pt.pt_vertical[0]     = tc_acs_map[TACS_VLINE];
        x_pt.pt_horizontal[0]   = tc_acs_map[TACS_HLINE];
        x_pt.pt_top_join[0]     = tc_acs_map[TACS_TTEE];
        x_pt.pt_bot_join[0]     = tc_acs_map[TACS_BTEE];
        x_pt.pt_cross[0]        = tc_acs_map[TACS_PLUS];
        x_pt.pt_left_join[0]    = tc_acs_map[TACS_RTEE];
        x_pt.pt_right_join[0]   = tc_acs_map[TACS_LTEE];

        trace_ilog("ACS mapping\n");
        trace_ilog("   top   %d/0x%x\n", x_pt.pt_top_right[0],  x_pt.pt_top_right[0]);
        trace_ilog("   bot   %d/0x%x\n", x_pt.pt_bot_left[0],   x_pt.pt_bot_left[0]);
        trace_ilog("   bot   %d/0x%x\n", x_pt.pt_bot_right[0],  x_pt.pt_bot_right[0]);
        trace_ilog("   verti %d/0x%x\n", x_pt.pt_vertical[0],   x_pt.pt_vertical[0]);
        trace_ilog("   horiz %d/0x%x\n", x_pt.pt_horizontal[0], x_pt.pt_horizontal[0]);
        trace_ilog("   top   %d/0x%x\n", x_pt.pt_top_join[0],   x_pt.pt_top_join[0]);
        trace_ilog("   bot   %d/0x%x\n", x_pt.pt_bot_join[0],   x_pt.pt_bot_join[0]);
        trace_ilog("   cross %d/0x%x\n", x_pt.pt_cross[0],      x_pt.pt_cross[0]);
        trace_ilog("   left  %d/0x%x\n", x_pt.pt_left_join[0],  x_pt.pt_left_join[0]);
        trace_ilog("   right %d/0x%x\n", x_pt.pt_right_join[0], x_pt.pt_right_join[0]);
    }
}


/*  Function:           term_colors
 *      Load the given color specification.
 *
 *  Parameters:
 *      value - Color scheme.
 *
 *  Returns:
 *      nothing.
 */
static void
term_colors(void)
{                                               /* save user color specification */
    int col, source = 0;

    /* configure color depth */
    if (xf_color > 1) {
        tt_colors = xf_color;                   /* command line override */
        source = 2;

    } else if (TA_MONO & t_attributes) {
        tt_colors = 2;                          /* mono feature (xterm-mono) */
        source = 3;

    } else if (x_pt.pt_colordepth > 1) {
        tt_colors = x_pt.pt_colordepth;         /* set_term_feature() */
        source = 1;

    } else if (x_pt.pt_color /*-1 or 1*/) {
        if (t_attributes & TA_XTERMLIKE) {
            tt_colors = XTERM_COLORS;

        } else if (0 == (tt_colors = tf_Colors)) {
            if (tc_ANSI_Color_Fg && tc_ANSI_Color_Bg) {
                tt_colors = ANSI_COLORS;
            } else if (tc_Color_Fg && tc_Color_Bg) {
                tt_colors = 8;
            } else {
                tt_colors = 0;
            }
        }
    }

    /* configure colors */
    assert((COLOR_NONE + 1) == (sizeof(tt_colormap)/sizeof(tt_colormap[0])));
    assert((COLOR_NONE + 1) == (sizeof(color_map)/sizeof(color_map[0])));

    trace_ilog("ttcolormap:\n");
    for (col = 0; col <= COLOR_NONE; ++col) {
        int color = -1;

        if (-2 == x_pt.pt_xtpalette) {          /* user defined palette */
            if ((color = tt_colormap[col]) <= 0) {
                color = -1;
            }
        }

        if (color < 0) {
            if (tt_colors >= 256) {
                if (0 == x_pt.pt_xtpalette) {   /* compat-16 mode */
                    color = color_map[col].c256_compat;
                } else {
                    color = color_map[col].c256;
                }

            } else if (tt_colors >= 88) {
                if (0 == x_pt.pt_xtpalette) {   /* compat-16 mode */
                    color = color_map[col].c88_compat;
                } else {
                    color = color_map[col].c88;
                }

            } else if (tt_colors >= 16) {
                color = color_map[col].c16;

            } else {
                color = color_map[col].c8;
            }
        }

        tt_colormap[col] = color;
        trace_ilog("\t%16s [%2d] = %d\n", color_name(col, "unknown"), col, tt_colormap[col]);
        assert(color_map[col].ident == col);
    }

    trace_ilog("term_colors(source=%d,depth=%d)\n", source, tt_colors);
    x_pt.pt_colordepth = tt_colors;             /* derived depth */

    if (tt_colors > 2) {
        x_display_ctrl |= DC_SHADOW_SHOWTHRU;
    }
    term_fgbg();
}


/*  Function:           term_fgbg
 *      Load the environment settings (if any) which define the default colors configurations.
 *
 *      The supported configurations are NCURSES_ASSUMED_COLORS, COLORFGBG (rxvt/mrxvt)
 *      and DEFAULT_COLORS.
 *
 *      The following are examples of their settings;
 *
 *          NCURSES_ASSUMED_COLORS='0;15'
 *
 *          COLORFGBG='0;default;15'
 *
 *          DEFAULT_COLORS='0;15'
 *
 *  Notes:
 *      The format of the COLORFGBG variable is not documented as such and there two
 *      known formats. The two formats correspond to whether the xpm library is used or
 *      not.
 *
 *      If rxvt is compiled with xpm support, the variable has three fields. In either
 *      case, the background color is the last field.
 *
 *  Parameters:
 *      src - Buffer cursor.
 *      result - Storage for decoded result.
 *
 *  Returns:
 *      Final cursor position.
 */
static void
term_fgbg(void)
{
    const char *env;

    tt_defaultfg = tt_defaultbg = NOCOLOR;      /* unknown */

    if (NULL != (env = ggetenv("NCURSES_ASSUMED_COLORS"))) {
        int count, fg = NOCOLOR, bg = NOCOLOR;
        char sep;

        if ((count = sscanf(env, "%d%c%d%c", &fg, &sep, &bg, &sep)) >= 1) {
            if (fg >= 0 && fg <= 255) tt_defaultfg = fg;
            if (count >= 3) {
                if (bg >= 0 && bg <= 255) tt_defaultbg = bg;
            }
        }

    } else {
        if (NULL != (env = ggetenv("COLORFGBG")) ||
                NULL != (env = ggetenv("DEFAULT_COLORS"))) {
            env = fgbg_value(env, &tt_defaultfg);
            env = fgbg_value(env, &tt_defaultbg);
            if (*env)  {                        /* XPM support, last field is background */
                fgbg_value(env, &tt_defaultbg);
            }
        }
    }
                                                /* apply color */
    tt_defaultfg = fgbg_import(tt_defaultfg, x_pt.pt_defaultfg);
    tt_defaultbg = fgbg_import(tt_defaultbg, x_pt.pt_defaultbg);
}


/*  Function:           fgbg_value
 *      Decode a COLORFGBG or DEFAULT_COLORS color specification.
 *
 *  Parameters:
 *      src - Buffer cursor.
 *      result - Storage for decoded result.
 *
 *  Returns:
 *      Final cursor position.
 */
static const char *
fgbg_value(const char *src, int *result)
{
    const char *end = NULL;

    if (0 == strncmp(src, "default", 7)) {
        end = src + 7;                          /* special case */
    } else {
        char *endp = 0;
        int value = (int) strtol(src, &endp, 0);

        if (0 == endp) {
            end = src;
        } else if (value >= 0 && value <= 255) {
            *result = (unsigned) value;
            end = endp;
        }
        while (*end && *end != ';') {
            ++end;
        }
    }

    if (*end == ';') {
        ++end;
    }
    return end;
}


static unsigned
fgbg_import(unsigned edefault, int udefault)
{
    if (NOCOLOR == edefault) {
        if (udefault >= 0) {
            return udefault;
        }
        return NOCOLOR;
    }
    if (edefault < ANSI_COLORS) {
        return ansicolor_map[ edefault ];
    }
    return edefault;
}


/*  Function:           xterm_colors
 *       Manage the xterm color pairs
 *
 *  Parameters:
 *      value - Specification.
 *
 *  Notes:
 *      ESC]4; c; spec ?
 *
 *      Change Color Number c to the color specified by spec, i.e., a name or RGB
 *      specification as per XParseColor. Any number of c name pairs may be given.
 *
 *      The color numbers correspond to the ANSI colors 0-7, their bright versions 8-15,
 *      and if supported, the remainder of the 88-color or 256-color table. Each successive
 *      parameter changes the next color in the list. The value of Ps tells the starting
 *      point in the list. The colors are specified by name or RGB specification as per
 *      XParseColor.
 *
 *      If a "?" is given rather than a name or RGB specification, xterm replies with a
 *      control sequence of the same form which can be used to set the corresponding color.
 *      Because more than one pair of color number and specification can be given in one
 *      control sequence, xterm can make more than one reply.
 *
 *  Returns:
 *      nothing.
 */
static void
xterm_colors(const char *value)
{
    if (0 == (t_attributes & TA_XTERMLIKE))     /* xterm specific */
        return;

    if (NULL == value || 0 == value[0])
        return;

    trace_ilog("xterm_colors(%s)\n", value);

    if (value != t_colorsorg) {
        if (0 == t_colorsorg[0]) {              /* retrieve and save existing */
            xterm_colors_get(t_colorsorg, sizeof(t_colorsorg));
        }
    }

    if (0 != t_colorsorg[0]) {                  /* set new value (if save was possible) */
        xterm_colors_set(value);
    }
}


static int
xterm_colors_get(char *buffer, int length)
{
    int color, len;

    if (0 == (t_attributes & TA_XTERM)) {       /* xterm specific */
        return -1;
    }

    term_flush();                               /* clear existing obuf */

    for (len = 0, color = 0; color < XTERM_COLORS; ++color) {
        /*
         *  foreach(color)
         */
        int cnt = sprintf((char *)t_buffer, "\033]4;%d;?\007", color);

        if (cnt != sys_write(TTY_OUTFD, t_buffer, cnt) ||
                (cnt = term_read((char *)t_buffer, sizeof(t_buffer), 5 * 1000)) <= 0 ||
                sscanf((char *)t_buffer, "%*[^;];%*[^;];%s", buffer + len + (len ? 1 : 0)) != 1) {
            /*
             *  Note, 'rxvt' and the 'linux console' only support set.
             *
             *  rxvt shall report (on stderr)/
             *      rxvt: can't determine colour: ?
             */
            trace_ilog("\terr[%d] (%*s)\n", color, cnt, t_buffer);
            break;
        }

        trace_ilog("\tget[%d]=%s\n", color, buffer + len + (len ? 1 : 0));

        if (len) {
            buffer[len] = ',';                  /* separator */
        }
        len += strlen(buffer + len);
        assert(len < length-1);

        if ('\007' == buffer[len-1]) {          /* strip terminator */
            buffer[len-1] = '\0', --len;
        }
    }
    return 0;
}


static int
xterm_colors_set(const char *value)
{
    const char *term;
    int color, len, cnt;

    if (0 == (t_attributes & TA_XTERMLIKE)) {   /* xterm specific */
        return -1;
    }

    term_flush();                               /* clear existing obuf */

    cnt = sprintf((char *)t_buffer, "\033]4");  /* dynamic color command */

    for (color = 0; color < XTERM_COLORS; ++color) {

        if ((const char *)NULL == (term = strchr(value, ','))) {
            len = strlen(value);
        } else {
            len = term - value;
        }

        trace_ilog("\tset[%d]=%.*s\n", color, len, value );
        if (len) {
            cnt += sprintf((char *)(t_buffer + cnt), ";%d;%.*s", color, len, value);
        }

        assert(cnt < (int)sizeof(t_buffer));
        if (NULL == term) {
            break;
        }

        value = term + 1;                       /* next field */
    }

    cnt += sprintf((char *)(t_buffer + cnt), "\007");
    sys_write(TTY_OUTFD, t_buffer, cnt);        /* terminator */
    return 0;
}


/*  Function:           term_attributes
 *      Request the terminal (VT100 and xterm style) to echo the "DA1 - Primary Device Attributes"
 *      report containing terminal terminal's architectural class and basic attributes.
 *
 *      Send an extended query, test to see if it supports Kitty's keyboard protocol.
 *
 *      DA1 - Primary Device Attributes
 *          In this DA exchange, the host asks for the terminal's architectural class and basic attributes.
 *
 *          Host Request:
 *              The host uses the following sequence to send this request:
 *
 *              CSI     c       or      CSI     0       c
 *              9/11    6/3             9/11    0       6/3
 *
 *          Terminal Response:
 *              The terminal responds by sending its architectural class and basic attributes to the host. This response depends on the terminal's current operating VT level.
 *
 *              CSI     ?       6       4       ;       Ps1     ...
 *              9/11    3/15    3/6     3/4     3/11    3/n     ..
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      Length of the buffer retrieved.
 */
static int
term_attributes(void)
{
    int ret = -1;

    if ((TA_KITTY|TA_ALACRITTY|TA_WEZTERM) & t_attributes) {
#define XTERM_DA1X          (sizeof(xterm_da1x_cmd)-1)
        static char xterm_da1x_cmd[] = "\033[?u\033[c";
                //
                // Extended DA1 request.
                //
                // An application can query the terminal for support of this protocol by sending the escape code
                // querying for the current progressive enhancement status followed by request for the primary
                // device attributes <https://vt100.net/docs/vt510-rm/DA1.html>.
                //
                // If an answer for the device attributes is received without getting back an answer for the
                // progressive enhancement the terminal does not support this protocol.
                //
                // Terminals: kitty, foot, WezTerm, alacritty, rio, crossterm
                //
        char buffer[64] = {0};
        int len = 0;

        term_flush();
        if (XTERM_DA1X == sys_write(TTY_OUTFD, xterm_da1x_cmd, XTERM_DA1X) &&
                (len = term_read(buffer, sizeof(buffer), -2)) > 1) {
            char *cursor = buffer;

            trace_ilog("term_da1x(%d, %s)\n", len, buffer);
            if ('\033' == cursor[0] && '[' == cursor[1]) {
                if (cursor[3] == 'u' && cursor[4] == '?') {
                    x_pt.pt_attributes |= TF_AKITTYKEYS;
                    trace_ilog("\t==> kitty-keys\n");
                    cursor += 4;
                }
            }

        //  if ('\033' == cursor[0] && '[' == cursor[1]) {
        //      if (cursor[2] == '?' && cursor[3]) {
        //          trace_ilog("\t==> da1 response\n");
        //      }
        //  }
        }
    }

    if (0 == (t_attributes & TA_KITTY)) {
        const char *vte_version_env = ggetenv("VTE_VERSION");
        int vte_version = vte_version_env ? (int)strtol(vte_version_env, NULL, 10) : 0;

        trace_ilog("term_vte_version=%d\n", vte_version);
        if (vte_version == 0 || vte_version >= 5400) {
            x_pt.pt_attributes |= TF_AXTERMKEYS;/* xterm modifyOtherKeys */
        }
    }

    return ret;
}


/*  Function:           term_identification
 *      Request the terminal (VT100 and xterm style) to echo the "DA2 - Secondary Device Attributes"
 *      report containing terminal type and version.
 *
 *      DA2 - Secondary Device Attributes
 *          In this DA exchange, the host requests the terminal's identification code, firmware version level, and hardware options.
 *
 *          Host Request:
 *              The host uses the following sequence to send this request.
 *
 *              CSI     >       c       or      CSI     >       0       c
 *              9/11    3/14    6/3             9/11    3/14    3/0     6/3
 *
 *          Terminal Response:
 *              The terminal with a VT keyboard uses the following sequence to respond.
 *
 *              CSI     >       6       1       ;       Pv      ;       0       c
 *              9/11    3/14    3/6     3/1     3/11    3/n     3/11    3/0     6/3     DA2R for terminal with STD keyboard.
 *
 *              CSI     >       6       1       ;       Pv      ;       1       c
 *              9/11    3/14    3/6     3/1     3/11    3/n     3/11    3/1     6/3     DA2R for terminal with PC keyboard.
 *
 *      Example:
 *          \033[>82;20710;0c
 *                ^type
 *                   ^version
 *
 *          Terminal                    Type        Version         Example
 *          ------------------------------------------------------------------
 *          Gnome-terminal (legacy)     1           >= 1115         1;3801;0
 *          PuTTY                       0           136             0;136;0
 *          MinTTY                      77(=M)                      77;20005;0c
 *          rxvt                        82(=R)
 *          screen                      83(=S)                      83;40500;0
 *          urxvt                       85(=U)
 *          xterm                       -2(a)
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      Length of the buffer retrieved.
 */
static int
term_identification(void)
{
#define XTERM_DA2_LEN       (sizeof(xterm_da2_cmd)-1)
    static char xterm_da2_cmd[] = "\033[>c";
    int ret = -1;

#if defined(__CYGWIN__)
    if (TA_CYGWIN & t_attributes) {
        /*
         *  CYGWIN
         */
        struct utsname u = {0};
        int r1 = 0, r2 = 0;

        ret = -3;
        if (uname(&u) >= 0) {
            if (2 == sscanf(u.release, "%d.%d", &r1, &r2)) {
                x_pt.pt_vtdatype = -3;            /* source: xterm_cygwin */
                x_pt.pt_vtdaversion = (r1 * 100) + (r2 > 99 ? 99 : r2);
            }
            trace_ilog("\tcygwin_uname(%s) = %d.%d\n", u.release, r1, r2);
        }
        return ret;
    }
#endif  /*__CYGWIN__*/

    if (((TA_VT100LIKE|TA_XTERM|TA_XTERMLIKE) & t_attributes) || tc_RV) {
        /*
         *  xterm and compatible terminals
         */
        const char *rvcmd = (tc_RV ? tc_RV : xterm_da2_cmd);
        const int rvlen = (tc_RV ? strlen(tc_RV) : XTERM_DA2_LEN);
        const char *vstr;

        /*
         *  XTERM_VERSION/
         *      Xterm(256) ==> 256
         */
        if (NULL != (vstr = ggetenv("XTERM_VERSION"))) {
            char vname[32+1] = {0};
            int vnumber = 0;
                                                /* decode and return patch/version number */
            if (2 == sscanf(vstr, "%32[^(](%u)", vname, &vnumber))
                if (vnumber > 0) {
                    x_pt.pt_vtdatype = -2;      /* source: xterm_version */
                    x_pt.pt_vtdaversion = vnumber;
                    ret = 0;
                }
            trace_ilog("XTERM_VERSION(%s) = %d (%s)\n", vstr, vnumber, vname);
        }

        /*
         *  Device attribute
         */
        if (-1 == ret) {
            char buffer[32] = {0};
            int len = 0;

            term_flush();
            if (rvlen == sys_write(TTY_OUTFD, rvcmd, rvlen) &&
                    (len = term_read(buffer, sizeof(buffer), -2)) > 1) {
                trace_ilog("term_da2(%d, %s)\n", len, buffer);

                if ('\033' == buffer[0] && '[' == buffer[1] &&
                        ('>' == buffer[2] || '?' == buffer[2])) {
                    int datype, daversion;

                    if (2 == sscanf(buffer + 3, "%d;%d", &datype, &daversion)) {
                        trace_ilog("\t==> type:%d, version:%d\n", datype, daversion);
                        x_pt.pt_vtdatype = datype;
                        x_pt.pt_vtdaversion = daversion;
                        if (77 == datype) {
                            t_attributes |= TA_MINTTY;
                            ++tf_xn;
                        }
                        ret = 0;
                    }
                }
            }
        }
    }

    return ret;
}


/*  Function:           term_ocs_color
 *      Request the terminals foreground or background RGB color.
 *
 *  Parameters:
 *      code - 10=foregroud or 11=background.
 *
 *  Returns:
 *      0 on success, otherwise non-zero on error.
 */
static int
term_ocs_color(int code)
{
#define XTERM_OCS10_LEN     (sizeof(xterm_ocs10) - 1)
#define XTERM_OCS11_LEN     (sizeof(xterm_ocs11) - 1)

    static char xterm_ocs10[] = "\x1b]10;?\007";
    static char xterm_ocs11[] = "\x1b]11;?\007";

    const unsigned timeoutms = io_escdelay();
    unsigned rgb[3] = {0,0,0}, rgbmax = 0;
    char *cp, buffer[32] = {0};
    int len = 0;

    assert(10 == code || 11 == code);
    if (!tf_XT && 0 == ((TA_XTERM|TA_XTERMLIKE) & t_attributes)) {
        trace_ilog("term_ocs%d : not supported\n", code);
        return -1;                              /* supported? */
    }

    /*
     *  Format:
     *
     *          <ESC>]rgb:xx/xx/xx<ESC|DEL>
     *      or  <ESC>]rgb:xxxx/xxxx/xxxx<ESC|DEL>
     *
     *  Example:
     *
     *      echo -ne '\e]11;?\a'; cat
     *
     *      ESC]11;rgb:0000/0000/0000
     */
    term_flush();
    if ((10 == code && XTERM_OCS10_LEN != sys_write(TTY_OUTFD, xterm_ocs10, XTERM_OCS10_LEN)) ||
        (11 == code && XTERM_OCS11_LEN != sys_write(TTY_OUTFD, xterm_ocs11, XTERM_OCS11_LEN)) ||
            (len = sys_read_timed(TTY_INFD, buffer, sizeof(buffer), timeoutms, NULL)) < 1) {
        trace_ilog("term_ocs%d : io (tm=%d, len=%d)\n", code, timeoutms, len);
        return -1;
    }

    cp = buffer;
    if (cp[0] == '\033' && cp[1] == ']') {        /* ESC] */
        cp += 2;
    } else if (cp[0] == 0x9d) {                   /* OSC */
        cp += 1;
    } else {
        cp = NULL;
    }

    if (cp && cp[0] == '1' && (cp[1] == '0' || cp[1] == '1') && cp[2] == ';') {
        /*
         *  parse RGB values
         */
        if (cp[11] == '/') {
            if (sscanf(cp + 3, "rgb:%4x/%4x/%4x\033", rgb+0, rgb+1, rgb+2) == 3 ||
                    sscanf(cp + 3, "rgb:%4x/%4x/%4x\007", rgb+0, rgb+1, rgb+2) == 3) {
                rgbmax = 0xffff;
            }

        } else if (cp[9] == '/') {
            if (sscanf(cp + 3, "rgb:%2x/%2x/%2x\033", rgb+0, rgb+1, rgb+2) == 3 ||
                    sscanf(cp + 3, "rgb:%2x/%2x/%2x\007", rgb+0, rgb+1, rgb+2) == 3) {
                rgbmax = 0xff;
            }
        }
    }

    if (0 == rgbmax)
        trace_ilog("term_ocs%d(%d, %s) : n/a", code, len, buffer);

    if (rgbmax && code == 11) {
        /*
         *  Luminance (perceived)
         *  Reference: https://www.w3.org/TR/AERT/#color-contrast
         */
        const double r = (double)rgb[0] / (double)rgbmax;
        const double g = (double)rgb[1] / (double)rgbmax;
        const double b = (double)rgb[2] / (double)rgbmax;
        const double l = (0.299 * r) + (0.587 * g) + (0.114 * b);

        trace_ilog("term_ocs%d(%d, %s) : %04x/%04x/%04x\n", code, len, buffer, rgb[0], rgb[1], rgb[2]);
        trace_ilog("\t==> luminance (%g) [%s]\n", l, l < 0.5 ? "dark" : "light");

        if (l < 0.5) {
            t_attributes |= TA_LUMINACE_DARK;
        } else {
            t_attributes |= TA_LUMINACE_LIGHT;
        }
    }
}


/*  Function:           term_isutf8
 *      Determine whether the terminal supports UTF8 character encoding.
 *
 *      Where possible limit terminal interaction, as such test *all*
 *      configuration/environment setting prior.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      *true* if UTF8, otherwise *false*.
 */
static int
term_isutf8(void)
{
    int ret = 0;

    if (DISPTYPE_UTF8 == xf_disptype) {         /* explicitly enabled -- command line */
        ret = 1;

    } else if (DISPTYPE_7BIT == xf_disptype || DISPTYPE_8BIT == xf_disptype) {
        ret = 0;                                /* explicitly disabled -- command line */

    } else if (TA_LINUX & t_attributes) {       /* linux console, not supported */
        ret = -2;

    } else if (TA_CYGWIN & t_attributes) {      /* cygwin version (1.7+) */
        if (-3 == x_pt.pt_vtdatype) {
            const int r1 = x_pt.pt_vtdaversion / 100;
            const int r2 = x_pt.pt_vtdaversion % 100;

            if (2 <= r1 || (1 == r1 && 7 <= r2)) {
                ret = 3;
            }
        }

    } else if (x_pt.pt_encoding[0]) {           /* terminal encoding/locale */
        ret = (mchar_locale_utf8(x_pt.pt_encoding) ? 2 : -1);

    } else if (sys_unicode_locale(TRUE)) {      /* BTERM_LOCAL or LOCALE */
        ret = 4;

    } else if (TA_XTERM & t_attributes) {
        /*
         *  Write a single utf8 character and check resulting cursor position.
         *
         *  Source:  ICU27 and general discussions
         */
#define XTERM_UTF8_TEST1        (sizeof(xterm_utf8_test1)-1)
#define XTERM_UTF8_CLEAN1       (sizeof(xterm_utf8_clean1)-1)

        static unsigned char xterm_utf8_test1[]  = "\r" "\xc3\xb6" "\033[6n";
        static unsigned char xterm_utf8_clean1[] = "\r  \r";

        ret = -5;
        term_flush();
        if (XTERM_UTF8_TEST1 == sys_write(TTY_OUTFD, (void *)xterm_utf8_test1, XTERM_UTF8_TEST1)) {
            int row = -1, col = -1;
            char buffer[32] = {0};
            int len;

            if ((len = term_read(buffer, sizeof(buffer), -2)) >= 4 &&
                    2 == sscanf(buffer, "\033[%d;%dR", &row, &col)) {
                if (2 == col) {
                    ret = 5;                    /* cursor in second column */
                }
            }
            trace_ilog("\tisutf8A(%d) = col:%d\n", len, col);
            sys_write(TTY_OUTFD, (void *)xterm_utf8_clean1, XTERM_UTF8_CLEAN1);
        }

    }

    trace_ilog("UTF8 support=%d\n", ret);
    return (ret >= 1 ? 1 : 0);
}


#if (TODO_MCHAR_DETECT)                         /* TODO: terminal ambiguous width */
static void
term_utf8_features(void)
{
#define XTERM_UTF8_TEST2        (sizeof(xterm_utf8_test2)-1)

    static unsigned char xterm_utf8_test2[] = {
            '\r',                               /* UTF features */
            0xa5,
            0xc3, 0x84, 0xd9,
            0xa7,
            0xd8, 0xb8, 0xe0, 0xe0,
            0xa9,
            0xa9,
            0xb8, 0x88, 0xe5, 0xe5, 0x88,
            0xa2, 0xa2,
            0x1b, '[',  '6',  'n',              /* cursor position */
            0x00                                /* NUL */
            };

    term_flush();

    if (XTERM_UTF8_TEST2 == sys_write(TTY_OUTFD, xterm_utf8_test2, XTERM_UTF8_TEST2)) {
        int row = -1, col = -1;
        char buffer[32] = {0};
        int len;

        /*
         *  check cursor column after test string, determine screen mode
         *
         *      6       -> UTF-8, no double-width, with LAM/ALEF ligature joining
         *      7       -> UTF-8, no double-width, no LAM/ALEF ligature joining
         *      8       -> UTF-8, double-width, with LAM/ALEF ligature joining
         *      9       -> UTF-8, double-width, no LAM/ALEF ligature joining
         *      11,16   -> CJK terminal (with luit)
         *      10,15   -> 8 bit terminal or CJK terminal
         *      13      -> Poderosa terminal emulator, UTF-8, or TIS620 terminal
         *      14,17   -> CJK terminal
         *      16      -> Poderosa, ISO Latin-1
         *      (17)    -> Poderosa, (JIS)
         *      18      -> CJK terminal (or 8 bit terminal, e.g. Linux console)
         */
        if ((len = term_read(buffer, sizeof(buffer), -2)) >= 4) {
            sscanf(buffer, "\033[%d;%dR", &row, &col);
        }

        trace_ilog("\tisutf8B(%d) = col:%d\n", len, col);
    }
}
#endif  /*XXX_MCHAR_DETECT*/


/*  Function:           term_read
 *      Block read from the terminal.
 *
 *  Parameters:
 *      buffer - Buffer.
 *      length - Length of the buffer, in bytes.
 *      timeoutms - Timeout is milliseconds.
 *
 *  Returns:
 *      Number of bytes read.
 */
static int
term_read(char *buffer, int length, accint_t timeoutms)
{
    int cnt = 0;

    if (timeoutms <= -2) {
        timeoutms = io_escdelay();
    }

    assert(buffer && length);
    assert(timeoutms >= -1);
    if (NULL == buffer || 0 == length)
        return 0;

    if (length > 1) {
        int ret;

        --length;                               /* null terminator */

        if (timeoutms < 0) {                    /* blocking */
            if ((ret = sys_read(TTY_INFD, buffer + cnt, length - cnt)) > 0) {
                cnt = ret;
                while (cnt < length &&
                        (ret = sys_read_timed(TTY_INFD, buffer + cnt, length - cnt, 50, NULL)) > 0) {
                    cnt += ret;                 /* secondary characters */
                }
            }

        } else {                                /* timed */
            if ((ret = sys_read_timed(TTY_INFD, buffer, length, timeoutms, NULL)) > 0) {
                cnt = ret;
            }
        }
    }

    buffer[cnt] = '\0';
    return cnt;
}


/*  Function:           term_tidy
 *      Cleanup/restore the state of the console.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
term_tidy(void)
{
    if (0 == tt_active)
        return;

    term_flush();

    if (t_attributes & TA_CYGWIN) {
        if (xf_cygwinkb) {
            ttpush("\033[?2000l");              /* disable RAW mode */
        }
    } else if (t_attributes & TA_MINTTY) {
        if (xf_mouse) {                         /* mouse enabled? */
            ttpush("\033[?7786l");              /* disable mouse-wheel reports */
        }
    }

    term_graphic_exit();                        /* graphic mode */

    term_colorreset();
    term_styleoff();
    xterm_colors_set(t_colorsorg);

 /* ttputpad(tc_mo);                            -* restore meta key-codes */

    ttputpad(tc_ve);                            /* restore normal cursor */

    if (! xf_nokeypad) {
        ttputpad(tc_ke);                        /* out of ``keypad-transmit'' mode */
    }

    if ((NULL == tc_rs && 0 == x_pt.pt_reset[0]) || x_pt.pt_noaltscreen) {
        term_zero(TRUE);                        /* remove image */
        ttmove(ttrows() - 1, 0);
    } else {
        term_scrollreset();                     /* scroll window */
    }

    ttputpad(tc_rs);                            /* set the term back to normal mode */

    if (x_pt.pt_reset[0]) {                     /* terminal specific reset */
        ttputctl2(x_pt.pt_reset, 0, 0);
    }
    term_flush();

    if (! xf_noinit) {
        ttputpad(tc_te);                        /* disable cup (cursor addressing) */
    }
    term_flush();

    trace_log("term_tidy(0x%0x)\n", t_specials);

    tt_cursor = 0;
    tt_active = 0;
}


/*  Function:           term_cursor
 *      Set the cursor status.
 *
 *  Parameters:
 *      visible - Visible status.
 *      imode - Insert mode.
 *      virtual_space - Virtual space mode.
 *
 *  Returns:
 *      returns the current visible status, otherwise -1 on error.
 */
static int
term_cursor(int visible, int imode, int virtual_space)
{
    if (visible >= 0) {
        if (visible != tt_cursor)
            if (tc_vi && (tc_vs || tc_ve)) {
                if (visible) {                  /* enable cursor, enhanced if available */
                    ttputpad(tc_vs ? tc_vs : tc_ve);
                } else {
                    ttputpad(tc_vi);            /* hide cursor */
                }
            }
        tt_cursor = visible;
    }

    if (tt_cursor && imode >= 0) {
        if (imode) {                            /* insert mode */
            if (x_pt.pt_vicursor[0]) {
                ttprintf(virtual_space ? x_pt.pt_vicursor : x_pt.pt_icursor);

            } else if (x_pt.pt_xtcursor >= 1 ||
                            (-1 == x_pt.pt_xtcursor && (t_attributes & TA_XTERMLIKE))) {
#if defined(DO_SPECIALS)
                if (0 == (t_specials & 0x0010))
                    t_specials |= 0x0010, trace_log("SPECIAL(ttcursor) 'imode'\n");
                ttprintf("\033]12;%s\007", "Black");
#endif
            }
        } else {                                /* overstrike/overtype mode */
            if (x_pt.pt_vocursor[0]) {
                ttputpad(virtual_space ? x_pt.pt_vocursor : x_pt.pt_ocursor);

            } else if (x_pt.pt_xtcursor >= 1 ||
                            (-1 == x_pt.pt_xtcursor && (t_attributes & TA_XTERMLIKE))) {
#if defined(DO_SPECIALS)
                if (0 == (t_specials & 0x0020)) {
                    t_specials |= 0x0020, trace_log("SPECIALS(ttcursor) 'omode'\n");
                }
                ttprintf("\033]12;%s\007", "RoyalBlue1");
#endif
            }
        }
    }

    return (tt_cursor ? 1 : 0);
}


/*  Function:           ttmove
 *      Move the cursor to the specified origin 0 row and column position. Try to
 *      optimize out extra moves; redisplay may have left the cursor in the right
 *      location last time!
 *
 *  Parameters:
 *      row - Screen row (0 .. rows - 1).
 *      col - Column (0 .. rows - 1).
 *
 *  Returns:
 *      nothing.
 */
static void
term_move(int row, int col)
{
    const int rows = ttrows() - 1, cols = ttcols() - 1,
            ttrow = ttatrow(), ttcol = ttatcol();

    assert(row >= 0);
    assert(row <= rows);
    assert(col >= 0);
    assert(col <= cols);

    if (ttrow == row && ttcol == col) {
        ED_TERM(("ttmove(%d,%d->%d,%d)\n", ttrow, ttcol, row, col))
        return;
    }
    ED_TERM(("ttmove(%d,%d->%d,%d)", ttrow, ttcol, row, col))

    /*
     *  Moving cursor, check ms flag.
     *      the 'ms' flag whose presence means that it is safe to move the cursor
     *      while the appearance modes are not in the normal state. If this flag
     *      is absent, programs should always turn off underlining/standout
     *      before moving the cursor.
     */
    if (1 != tf_ms)
        if (tt_style) {
            term_styleoff();
            tt_hue = -1;
        }

    if (tc_ho && 0 == col && 0 == row) {        /* <home> */
        ED_TERM(("->putpad(ho)\n"))
        ttputpad(tc_ho);

    } else {
        /*
         *  optimise based on current position (if known)
         */
        int done = TRUE;

        if (ttcol < 0 || ttrow < 0) {
            /*
             *  invalid existing position.
             */
            done = FALSE;

        } else if (tc_ll && row == rows && 0 == col) {
            ED_TERM(("->putpad(ll)\n"))         /* <left-end> */
            ttputpad(tc_ll);

        } else {
            /*
             *  If within column zero avoid using UP and DO under XTERM (others??)
             *  otherwise we shall find ourselves in the last column of the
             *  previous line; believe related to terminal automatic-margin
             *  support.
             *
             *  Note: this is generally only a visible problem when running borderless mode.
             */
#define COLUMNOK()  (col > 0 || 0 == (t_attributes & TA_XTERM))

#define PCOST       4                           /* parameterised version cost */

            if (col == ttcol) {                 /* row changes */

                /* cursor up - one line (ignore first column) */
                if (tc_up && COLUMNOK() && row == ttrow - 1) {
                    ED_TERM(("->putpad(UP)\n"))
                    ttputpad(tc_up);

                /* cursor up - parameterized (ignore first column) */
                } else if (tc_pUP && COLUMNOK() && row < ttrow /*-PCOST*/) {
                    const int p1 = (int)(ttrow - row);

                    ED_TERM(("->putctl(pUP,%d)\n", p1))
                    ttputctl(tc_pUP, p1);

                /* cursor down - one line (ignore bottom left corner) */
                } else if (tc_do && COLUMNOK() && row == ttrow + 1 && (row < rows || col < (cols - 1))) {
                    ED_TERM(("->putpad(DO)\n"))
                    ttputpad(tc_do);

                /* cursor down - parameterized */
                } else if (tc_pDO && COLUMNOK() && row > ttrow /*+PCOST*/) {
                    const int p1 = (int)(row - ttrow);

                    ED_TERM(("->putctl(pDO,%d)\n", p1))
                    ttputctl(tc_pDO, p1);

                /* cursor vertical movement */
                } else if (tc_cv) {
                    ED_TERM(("->putctl(cv,%d)\n", row))
                    ttputctl(tc_cv, row);

                } else {
                    done = FALSE;
                }

            } else if (row == ttrow) {          /* column changes */

                /* cursor left */
                if (tc_cr && 0 == col) {
                    ED_TERM(("->putpad(cr)\n"))
                    ttputpad(tc_cr);

                /* cursor left - one column */
                } else if (tc_le && col < ttcol && col >= (ttcol - 3)) {
                    int p1 = (int)(ttcol - col);

                    ED_TERM(("->putpad(le,%d)\n", p1))
                    while (p1--) ttputpad(tc_le);

                /* cursor back */
                } else if (tc_bc && col < ttcol && col >= (ttcol - 3)) {
                    int p1 = (int)(ttcol - col);

                    ED_TERM(("->putpad(bc,%d)\n", p1))
                    while (p1-- > 0) ttputpad(tc_bc);

                /* cursor left - parameterised */
                } else if (tc_pLE && col < (ttcol - 2)) {
                    const int p1 = (int)(ttcol - col);

                    ED_TERM(("->putctl(pLE,%d)\n", p1))
                    ttputctl(tc_pLE, p1);

                /* cursor right - one column */
                } else if (tc_nd && col == ttcol + 1) {
                    ED_TERM(("->putpad(ND)\n"))
                    ttputpad(tc_nd);

                /* cursor right - parameterised */
                } else if (tc_pRI && col > ttcol && col > 1 && col < cols) {
                    const int p1 = (int)(col - ttcol);

                    ED_TERM(("->putctl(pRI,%d)\n", p1))
                    ttputctl(tc_pRI, p1);

                /* cursor right - parameterised */
                } else if (x_pt.pt_cursor_right[0] && col > ttcol /*+PCOST*/) {
                    const int p1 = (int)(col - ttcol);

                    ED_TERM(("->putctl(pCR,%d)\n", p1))
                    ttprintf(x_pt.pt_cursor_right, p1);

                /* cursor horz movement */
                } else if (tc_ch) {
                    ED_TERM(("->putctl(ch,%d)\n", col))
                    ttputctl(tc_ch, col);

                } else {
                    done = FALSE;
                }

            } else {
                done = FALSE;                   /* do by hand */
            }
        }

#undef PCOST

        /* absolute cursor movement */
        if (! done) {
            if (tc_cm) {
                ED_TERM(("->cm(%d, %d)\n", col, row))
                ttputctl2(tc_cm, col, row);
            } else {                            /* use vertical/horiz movement */
                ED_TERM(("->cvch(%d, d)\n", col, row))
                ttputctl(tc_cv, row);
                ttputctl(tc_ch, col);
            }
        }
    }

    ttposset(row, col);
}


/*  Function:           term_names
 *      Set the console title.
 *
 *  Parameters:
 *      title - Title buffer.
 *
 *  Returns:
 *      nothing.
 */
static int
term_names(const char *title, const char *icon)
{
    __CUNUSED(icon)

    if (x_pt.pt_xttitle >= 1 ||
            (-1 == x_pt.pt_xttitle && (t_attributes & TA_XTERMLIKE))) {

        const unsigned char *t = (const unsigned char *) title;
        const size_t len = 16 + strlen(title);
        char *c;

        if (NULL != (c = chk_alloc(len))) {
            unsigned char *p =
                (unsigned char *)(c + sprintf(c, "\033]0;"));

            do {
                *p++ = (unsigned char)(*t < 32 || *t > 127 ? '?' : *t);
            } while (*++t);
            *p++ = '\007';
            *p = '\0';

            ttpush(c);
            chk_free(c);
        }
    }
    return 0;
}


/*  Function:           term_beep
 *      Make a noise.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
term_beep(int freq, int duration)
{
    __CUNUSED(freq) __CUNUSED(duration)

    if (xf_visbell && tc_vb) {                  /* visual bell */
        ttputpad(tc_vb);
    } else if (tc_bl) {
        ttputpad(tc_bl);
    } else {
        static const char bel[] = {ASCIIDEF_BEL, 0};
        ttpush(bel);
    }
    term_flush();
}


/*  Function:           normalbg
 *      Determine whether the current background is the terminals
 *      default/normal background color.
 *
 *      One of the following conditions are confirmed, checking the current
 *      hues ground against,
 *
 *          o Background is the terminal default (see COLORFGBG and tt_defaultfg).
 *
 *          o Background is NONE.
 *
 *  Parameters:
 *      bg - Storage returned with the cooked color enumeration.
 *
 *  Returns:
 *      *true* or *false*
 */
static int
normalbg(int *bg)
{
    colattr_t ca;

    *bg = -1;
    color_definition(tt_hue, &ca);
    return (COLORSOURCE_SYMBOLIC == ca.bg.source &&
                (tt_defaultbg == (*bg = (int)tt_colormap[ca.bg.color]) || COLOR_NONE == ca.bg.color));
}


/*  Function:           clearbg
 *      Determine whether a terminal clear command will (hopefully) function correctly with
 *      the current background color, otherwise the operation shall fail resulting in the
 *      cleared region being an incorrect background color; hence should be avoided.
 *
 *      One of the following conditions are confirmed,
 *
 *          o BE or UT, if BE (Back color erase - xterm) or UT (Screen erased with background
 *            color) are true the terminal clears using the current hue.
 *
 *          o Current hue is the terminals default/normal background.
 *
 *          o Terminal feature clrisblack is enabled and background is BLACK.
 *
 *          o No color mode and the hue is NORMAL.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static int
clearbg(void)
{
    int bg;

    if (tf_be || tf_ut)
        return TRUE;                            /* erase in current color */
    if (normalbg(&bg))
        return TRUE;                            /* current background is normal/default */
    if (x_pt.pt_clrisblack && BLACK == bg) {
        return TRUE;
    }
    if (!vtiscolor() && ATTR_NORMAL == tt_hue) {
        return TRUE;
    }
    return FALSE;
}


/*  Function:           term_clear
 *      Clear the terminal.
 *
 *  Parameters:
 *      restore - *true* if the screen image should be restored.
 *
 *  Returns:
 *      nothing.
 */
static void
term_clear(void)
{
    ED_TERM(("term_clear()\n"))
    term_zero(FALSE);
}


/*  Function:           term_zero
 *      Zero the terminal image.
 *
 *  Parameters:
 *      restore - *true* if the screen image should be restored.
 *
 *  Returns:
 *      nothing.
 */
static void
term_zero(int restore)
{
    ED_TERM(("term_zero(%d)\n", restore))
    term_scrollreset();
    ttmove(0, 0);
    term_attr(VBYTE_ATTR(ATTR_NORMAL));
    if (tc_cl && (restore || clearbg())) {      /* clear screen command (and home cursor) */
        ED_TERM(("->putpad(CL)\n"))
        ttputpad(tc_cl);
    } else {                                    /* erase to end of page */
        ED_TERM(("->eeop()\n"))
        term_eeop(restore);
    }
    term_colorreset();
    term_flush();
}


/*  Function:           term_eeol
 *      Erase to end of line in the 'current' color.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      *true* if eeol successful, otherwise *false*.
 */
static int
term_eeol(void)
{
    const int rows = ttrows() - 1, cols = ttcols() - 1,
        row = ttatrow(), col = ttatcol();

    ED_TERM(("term_eeol(%d, %d)\n", row, col))

    assert(row <= rows);
    assert(col <= cols);

    if (col < cols) {
        /*
         *  Clear to end of line.
         */
        if (term_ce() && clearbg()) {
            ED_TERM(("->putpad(CE)\n"))
            ttputpad(tc_ce);

        /*
         *  Character based
         *      Avoid writing to the lower right corner; as the terminal does not have CE,
         *      then it probably does not have what it takes not to scroll upon hitting
         *      the bottom-right corner.
         */
        } else {
            int cnt = cols - col;               /* column count to end-of-line */

            if (row >= rows) {
                --cnt;                          /* ignore last column */
            }
            term_repeat(cnt, ' ' | VBYTE_ATTR(tt_hue), WHERE_START);
        }
    }
    return TRUE;
}


/*  Function:           term_ce
 *      Is clear to end of line (CE) available.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static int
term_ce(void)
{
    return (tc_ce && tc_ce[0]);
}


/*  Function:           term_lastsafe
 *      Return whether it is safe to write to the last character within the bottom column.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      TRUE or FALSE.
 */
static int
term_lastsafe(void)
{
    if (TA_CYGWIN & t_attributes) {
        return FALSE;                           /* needs further investigations */
    }
    return (term_ce() || tf_LP > 0 ? TRUE : FALSE);
}


/*  Function:           term_eeop
 *      Erase to end of page.
 *
 *  Parameters:
 *      restore - Executed during a restore operation.
 *
 *  Returns:
 *      nothing.
 */
static void
term_eeop(int restore)
{
    const int rows = ttrows() - 1, cols = ttcols() - 1;
    int row = ttatrow(), col = ttatcol();

    ED_TERM(("term_eeop(%d, %d)\n", row, col))

    assert(row <= rows);
    assert(col <= cols);

    if (tc_cd && (restore || clearbg())) {      /* clear to end of display (cursor unchanged). */
        ED_TERM(("->putpad(CD)\n"))
        ttputpad(tc_cd);

    } else {                                    /* long-hand */
        int done = FALSE;

        if (col > 0) {
            term_eeol();                        /* remove remaining of line */
            ++row;
        }

        if (row <= rows) {
            if (t_insdel && !x_pt.pt_noinsdel && (restore || clearbg())) {
                done = term_dell(row, rows, rows - row, 0 /*current*/);
                if (done) {
                    ED_TERM(("->ttdell()\n"))
                }
            }

            if (! done) {                       /* do it by hand. */
                ED_TERM(("->line-by-line()\n"))
                for (; row <= rows; ++row) {
                    ttmove(row, 0);
                    term_eeol();
                }
            }
        }
    }
}


/*  Function:           term_repeat
 *      Function to repeat a character from the current location.
 *
 *      'where' says whether we want the cursor to stick at the starting point or stick
 *      at the end of the region. If set to 2 then this is a don't care -- i.e. leave
 *      cursor as it is.
 *
 *  Parameters:
 *      cnt - Character count.
 *      fill - Fill character.
 *      where - Where to leave the cursor (0=end, 1=start, 2=dontcare).
 *
 *  Returns:
 *      nothing.
 */
static void
term_repeat(int cnt, vbyte_t fill, int where)
{
    const int orow = ttatrow(), ocol = ttatcol();

    ED_TERM(("term_repeat(%d,'%c')\n", cnt, VBYTE_CHAR_GET(fill)))

    if (cnt <= 0) return;

    term_attr(fill & VBYTE_ATTR_MASK);

    if (cnt >= (4 + (WHERE_DONTCARE == where ? 0 : 4))) {
        const int ch = (int) VBYTE_CHAR_GET(fill);
        int bg = 0;
                                                /* erase #1 characters (no cursor move) */
        if (tc_ech && ' ' == ch && normalbg(&bg)) {

            if (0 == (t_specials & 0x0001))
                t_specials |= 0x0001, trace_log("SPECIAL(erase-repeat,%s,%d)\n", tc_ech, cnt);

            ttputctl(tc_ech, cnt);
            ttposinvalid();
            if (WHERE_END == where) {
                ttmove(orow, ocol + cnt);
            } else if (WHERE_START == where) {
                ttmove(orow, ocol);
            }
            cnt = 0;
                                                /* repeat character (cursor move) */
        } else if (tc_rp && (orow < (ttrows() - 1))) {

            if (0 == (t_specials & 0x0002))
                t_specials |= 0x0002, trace_log("SPECIAL(repeat-char,%s,%d)\n", tc_rp, cnt);

            ttputctl2(tc_rp, cnt, ch);
            ttposinvalid();
            if (WHERE_END == where) {
                ttmove(orow, ocol + cnt);
            } else if (WHERE_START == where) {
                ttmove(orow, ocol);
            }
            cnt = 0;
                                                /* erase #1 characters (no cursor move) */
        } else if (x_pt.pt_repeat_space[0] && ' ' == ch && normalbg(&bg)) {

            if (0 == (t_specials & 0x0004))
                t_specials |= 0x0004, trace_log("SPECIAL(erase-repeat,%s)\n", x_pt.pt_repeat_space);

            ttprintf(x_pt.pt_repeat_space, cnt);
            ttposinvalid();
            if (WHERE_END == where) {
                ttmove(orow, ocol + cnt);
            } else if (WHERE_START == where) {
                ttmove(orow, ocol);
            }
            cnt = 0;

        } else if (x_pt.pt_repeat_last[0] &&    /* repeat last (cursor move *not* assumed) */
                        (orow < (ttrows() - 1)) && normalbg(&bg)) {

            if (0 == (t_specials & 0x0008))
                t_specials |= 0x0008, trace_log("SPECIAL(repeat-last,%s,%d)\n", x_pt.pt_repeat_last, cnt);

            term_putc(fill);
            ttprintf(x_pt.pt_repeat_last, cnt - 1);
            ttposinvalid();
            if (WHERE_END == where) {
                ttmove(orow, ocol + cnt);
            } else if (WHERE_START == where) {
                ttmove(orow, ocol);
            }
            cnt = 0;
        }
    }

    if (cnt > 0) {                              /* by-hand */
        ED_TERM(("->byhand()\n"))
        while (cnt-- > 0) {
            term_putc(fill);
        }
        if (WHERE_START == where) {
            ttmove(orow, ocol);
        }
    }
}


/*  Function:           term_insl
 *      Insert the specified number of blank lines 'lines' onto the screen using the region
 *      'row' and 'bot', scrolling the last line within the region off the screen.
 *
 *      Use the scrolling region commands if possible for a smoother display, otherwise
 *      when no scrolling region, use a set of insert and delete line sequences.
 *
 *  Parameters:
 *      row - Top row of region.
 *      bot - Bottom of region.
 *      nlines - Number of lines to be inserted.
 *      fillcolor - Fill attribute.
 *
 *  Returns:
 *      TRUE if the operation could be performed, otherwise FALSE.
 */
static int
term_insl(int row, int bot, int nlines, vbyte_t fillcolor)
{
    int i;

    trace_log("term_insl(row:%d, bot:%d, nlines:%d)\n", row, bot, nlines);

    assert(row >= 0);
    assert(bot >= 0);
    assert(row < ttrows());
    assert(bot < ttrows());
    assert(bot >= row);
    assert(nlines > 0);
    assert(nlines <= (bot - row));

    term_attr(fillcolor);

    if (row == bot) {                           /* one line, simple optimisation */
        ttmove(row, 0);
        term_eeol();
        return TRUE;
    }
                                                /* scroll region and back index */
    if (tc_cs && tc_sr && !x_pt.pt_scroll_disable) {
        term_scrollset(row, bot);
        ttmove(row, 0);
        term_attr(VBYTE_ATTR(ATTR_NORMAL));
        if (nlines > 1 && tc_pSR) {
            ttputctl(tc_pSR, nlines);
        } else {
            for (i = 0; i < nlines; ++i) {
                ttputpad(tc_sr), term_eeol();
            }
        }
        term_scrollreset();
        ttmove(row, 0);
        return TRUE;
    }

    if (t_insdel && !x_pt.pt_noinsdel) {        /* line ins/del */
        ttmove(1 + bot - nlines, 0);
        term_attr(VBYTE_ATTR(ATTR_NORMAL));
        if (nlines > 1 && tc_pDL) {
            ttputctl(tc_pDL, nlines);
        } else {
            for (i = 0; i < nlines; ++i) {
                ttputpad(tc_dl);
            }
        }

        ttmove(row, 0);
        if (nlines > 1 && tc_pAL) {
            ttputctl(tc_pAL, nlines);
        } else {
            for (i = 0; i < nlines; ++i) {
                ttputpad(tc_al);
            }
        }

        if (!tf_be || !tf_ut)                   /* must erase line for correct colouring */
            for (i = 0; i < nlines; ++i) {
                ttmove(row + i, 0);
                term_eeol();
            }

        ttposinvalid();
        return TRUE;
    }
    return FALSE;
}


/*  Function:           term_dell
 *      Delete the specified number of blank lines 'nlines' onto the screen using the region
 *      'row' and 'bot', replacing the last line within a blank line.
 *
 *      Use the scrolling region commands if possible for a smoother display, otherwise
 *      when no scrolling region, use a set of insert and delete line sequences.
 *
 *      Knownledge that the echo-line shall always be presence, removes the end-of-screeen
 *      boundary condition.
 *
 *  Parameters:
 *      row - Top row of region.
 *      bot - Bottom of region.
 *      nlines - Number of lines to be deleted.
 *      fillcolor - Fill attribute.
 *
 *  Returns:
 *      TRUE if the operation could be performed, otherwise FALSE.
 */
static int
term_dell(int row, int bot, int nlines, vbyte_t fillcolor)
{
    int i;

    trace_log("term_dell(row:%d, bot:%d, nlines:%d)\n", row, bot, nlines);

    assert(row >= 0);
    assert(bot >= 0);
    assert(row < ttrows());
    assert(bot < ttrows());
    assert(bot >= row);
    assert(nlines > 0);
    assert(nlines <= (bot - row));

    term_attr(fillcolor);

    if (row == bot) {                           /* one line special case */
        ttmove(row, 0);
        term_eeol();
        return TRUE;
    }

    if (tc_cs && !x_pt.pt_scroll_disable &&     /* scrolling region and within limits */
            (x_pt.pt_scroll_max <= 2 || (bot - row) <= x_pt.pt_scroll_max)) {
        term_scrollset(row, bot);
        term_attr(VBYTE_ATTR(ATTR_NORMAL));
        ttmove(bot, 0);
        if (nlines > 1 && tc_pSF) {
            ttputctl(tc_pSF, nlines);           /* 24/11/08 */
        } else {
            for (i = 0; i < nlines; ++i) {
                ttputpad(tc_sf ? tc_sf : "\n"), term_eeol();
            }
        }
        term_scrollreset();
        ttmove(bot, 0);
        return TRUE;
    }
                                                /* ins/del, unless disabled or fast */
    if (t_insdel && !x_pt.pt_noinsdel && x_pt.pt_tty_fast <= 0) {
        ttmove(row, 0);
        term_attr(VBYTE_ATTR(ATTR_NORMAL));
        if (nlines > 1 && tc_pDL) {
            ttputctl(tc_pDL, nlines);
        } else {
            for (i = 0; i < nlines; ++i) {
                ttputpad(tc_dl);
            }
        }
        ttmove(1 + bot - nlines, 0);
        if (nlines > 1 && tc_pAL) {
            ttputctl(tc_pAL, nlines);
        } else {
            for (i = 0; i < nlines; ++i) {
                ttputpad(tc_al);
            }
        }
        ttposinvalid();
        return TRUE;
    }
    return FALSE;
}


/*  Function:           term_scrollset
 *      Setup the display scrolling window.
 *
 *  Note:
 *      Setting a scroll region invalidates the cursor position. As such the cursor
 *      position is also invalidated to ensure that the next call to "ttmove" does
 *      not turn into a no-op (the window adjustment moves the cursor).
 *
 *  Parameters:
 *      top - Top row of the scroll region.
 *      bot - Bottom row.
 *
 *  Returns:
 *      nothing.
 */
static void
term_scrollset(int top, int bot)
{
    assert(top >= 0);
    assert(top < ttrows());
    assert(bot >= 0);
    assert(bot < ttrows());
    assert(-1 == top || top != bot);            /* one line, should be avoided */
    assert(top < bot);
    assert(tc_cs);                              /* should always be the case */

    if (tc_cs) {                                /* scroll region avail ? */
        if (-1 == top || -1 == bot) {
            top = 0, bot = ttrows() - 1;        /* reset */
        }

        if (tt_top != top || tt_bot != bot) {   /* update */
            ttputctl2(tc_cs, tt_bot = bot, tt_top = top);
            ttposinvalid();
        }
    }
}


/*  Function:           term_scrollreset
 *      Reset the current scroll region.
 *
 *  Note:
 *      Setting a scroll region invalidates the cursor position. As such the cursor
 *      position is also invalidated to ensure that the next call to "ttmove" does
 *      not turn into a no-op (the window adjustment moves the cursor).
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
term_scrollreset(void)
{
    if (tc_cs) {
        ttputctl2(tc_cs, tt_bot = ttrows() - 1, tt_top = 0);
        ttposinvalid();
    }
}


/*  Function:           term_attr
 *      Set the current attribute color to the specified color.
 *
 *      The following function optimises based on the required color change and the
 *      current known state (if any).
 *
 *  Parameters:
 *      color - Required color attribute.
 *
 *  Returns:
 *      nothing.
 */
static void
term_attr(vbyte_t color)
{
    vbyte_t attr = VBYTE_ATTR_GET(color);       /* encode attribute */

    if (tt_hue == attr) {
        return;
    }
    tt_hue = attr;

    ED_TERM(("ttattr(%d/0x%x)", attr, attr))

    if (! vtiscolor()) {
        /*
         *  black-wbite/no-color mode
         *      \033[0m     Normal.
         *      \033[1m     Bold (extra bright).
         *      \033[4m     Underline.
         *      \033[5m     Blink (appears as Bold on Xterm etc).
         *      \033[7m     Inverse (hilight).
         *
         *  optional:
         *      \033[22m    Normal (neither bold nor faint).
         *      \033[24m    End underline.
         */
        const int nstyle = ttbandw(attr, tc_us?1:0, tc_ZH?1:0, tc_mb?1:0);

        if (tt_style != nstyle) {
            term_styleoff();
            term_styleon(nstyle);
        }

    } else {
        const char *reset = (tc_me ? tc_me : "\033[0m");
        colattr_t ca = {0};
        int fg, bg, sf;

        color_definition(attr, &ca);
        fg = ca.fg.color;
        bg = ca.bg.color;
        sf = ca.sf;

        ED_TERM((", curr(fg:%d, bg:%d, sf:%d), attr(fg:%d, bg:%d, sf:%d)", tt_fg, tt_bg, tt_style, fg, bg, sf))

        if (tt_colors >= 88 /*88 or 256*/) {
            /* 256 color mode
             *
             *      \033[0m         Set normal (foreground, background and styles)
             *      \033[39m        Default foreground color (Implementation defined)
             *      \033[49m        Default background color (Implementation defined)
             *
             *  SGR 38 and 48/
             *
             *      \033[38;5;#m    Set the foreground color to index #
             *      \033[48;5;#m    Set the background color to index #
             *
             *          0-7:     Standard colors (as in "ESC [ 30-37 m")
             *          8-15:    High intensity colors (as in "ESC [ 90-97 m")
             *          16-231:  6x6x6 cube (216 colors)
             *          232-255: Grayscale from dark to light in 24 steps.
             */
            if (COLORSOURCE_SYMBOLIC == ca.fg.source) fg = tt_colormap[fg];
            if (COLORSOURCE_SYMBOLIC == ca.bg.source) bg = tt_colormap[bg];

            ED_TERM(("->map256(fg:%d, bg:%d)", fg, bg))

            if (fg != tt_fg || bg != tt_bg || sf != tt_style) {
                char ebuf[64];

                if (tt_style) term_styleoff();

                if (fg >= 0 && bg >= 0) {       /* foreground,background */
                //  if (x_pt.pt_colorsetfgbg[0]) {
                //      sxprintf(ebuf, sizeof(ebuf), x_pt.pt_colorsetfgbg, 0xff & fg, 0xff & bg);
                //
                //  } else if (x_pt.pt_colorsetfg[0] && x_pt.pt_colorsetbg[0]) {
                //      int len = sxprintf(ebuf, sizeof(ebuf), x_pt.pt_colorsetfg, 0xff & fg);
                //      sxprintf(ebuf + len, sizeof(ebuf) - len, x_pt.pt_colorsetbg, 0xff & bg);
                //
                //  } else {
                        sxprintf(ebuf, sizeof(ebuf), "\033[38;5;%u;48;5;%um", 0xff & fg, 0xff & bg);
                //  }
                    ttputpad(ebuf);

                } else if (fg >= 0) {           /* foreground,normal */
                //  if (x_pt.pt_colorsetfgbg[0]) {
                //      sxprintf(ebuf, sizeof(ebuf), x_pt.pt_colorsetfgbg, 0xff & fg, 0);
                //
                //  } else if (x_pt.pt_colorsetfg[0] && x_pt.pt_colorsetbg[0]) {
                //      int len = sxprintf(ebuf, sizeof(ebuf), x_pt.pt_colorsetfg, 0xff & fg);
                //      sxprintf(ebuf + len, sizeof(ebuf) - len, x_pt.pt_colorsetbg, 0);
                //
                //  } else {
                        sxprintf(ebuf, sizeof(ebuf), "\033[49;38;5;%um", 0xff & fg);
                //  }
                    ttputpad(ebuf);

                } else if (bg >= 0) {           /* normal,background */
                //  if (x_pt.pt_colorsetfgbg[0]) {
                //      sxprintf(ebuf, sizeof(ebuf), x_pt.pt_colorsetfgbg, 0, 0xff & bg);
                //
                //  } else if (x_pt.pt_colorsetfg[0] && x_pt.pt_colorsetbg[0]) {
                //      int len = sxprintf(ebuf, sizeof(ebuf), x_pt.pt_colorsetfg, 0);
                //      sxprintf(ebuf + len, sizeof(ebuf) - len, x_pt.pt_colorsetbg, 0xff & bg);
                //
                //  } else {
                        sxprintf(ebuf, sizeof(ebuf), "\033[39;48;5;%um", 0xff & bg);
                //  }
                    ttputpad(ebuf);

                } else {                        /* normal */
                    ttputpad(reset);
                }

                if (sf) term_styleon(sf);
            }

        } else if ((tt_colors >= 16 && tc_ANSI_Color_Fg) ||
                        (NULL == tc_Color_Fg && NULL == tc_ANSI_Color_Fg)) {
            /* 16 color mode (standard - ANSI)
             *
             *      \033[0m         Set normal (foreground and background)
             *      \033[39m        Default foreground color (Implementation defined)
             *      \033[49m        Default background color (Implementation defined)
             *
             *      \033[30m        Set foreground color to Black
             *      \033[31m        Set foreground color to Red
             *      \033[32m        Set foreground color to Green
             *      \033[33m        Set foreground color to Yellow
             *      \033[34m        Set foreground color to Blue
             *      \033[35m        Set foreground color to Magenta
             *      \033[36m        Set foreground color to Cyan
             *      \033[37m        Set foreground color to White
             *      \033[38;5;%dm   Set foreground color (0 .. 15)
             *
             *      \033[40m        Set background color to Black
             *      \033[41m        Set background color to Red
             *      \033[42m        Set background color to Green
             *      \033[43m        Set background color to Yellow
             *      \033[44m        Set background color to Blue
             *      \033[45m        Set background color to Magenta
             *      \033[46m        Set background color to Cyan
             *      \033[47m        Set background color to White
             *      \033[48;5;%dm   Set background color (0 .. 15)
             */
            if (COLORSOURCE_SYMBOLIC == ca.fg.source) fg = tt_colormap[fg];
            if (COLORSOURCE_SYMBOLIC == ca.bg.source) bg = tt_colormap[bg];
            if (fg == bg && bg >= 0) {
                if (TA_DARK & t_attributes) {
                    fg = tt_colormap[WHITE];
                    bg = tt_colormap[BLACK];
                } else {
                    fg = tt_colormap[BLACK];
                    bg = tt_colormap[WHITE];
                }
            }

            ED_TERM(("->map16(fg:%d, bg:%d)", fg, bg))

            if (fg != tt_fg || bg != tt_bg || sf != tt_style) {
                char ebuf[64];
                int l = 0;

                if (tt_style) {
                    if (tc_me) reset = "";
                    term_styleoff();
                }

                if (fg > 0 && bg > 0) {         /* foreground (30-37,38+), background (40-47,48+) */
                    if (fg > 7) {
                        l = sprintf(ebuf, "\033[38;5;%u;", fg & 0xff);
                    } else {
                        l = sprintf(ebuf, "\033[%u;", 30 + (fg & 7));
                    }
                    if (bg > 7) {
                        l += sprintf(ebuf+l, "48;5;%um", bg & 0xff);
                    } else {
                        l += sprintf(ebuf+l, "%um", 40 + (bg & 7));
                    }
                    ttputpad(ebuf);

                } else if (fg > 0) {            /* normal + foreground (30-37,38+) */
                    if (fg > 7) {
                        l = sprintf(ebuf, "%s\033[38;5;%um", reset, fg & 0xff);
                    } else {
                        l = sprintf(ebuf, "%s\033[%um", reset, 30 + (fg & 7));
                    }
                    ttputpad(ebuf);

                } else if (bg > 0) {            /* normal + background (40-47,48+) */
                    if (bg > 7) {
                        l = sprintf(ebuf, "%s\033[48;5;%um", reset, bg & 0xff);
                    } else {
                        l = sprintf(ebuf, "%s\033[%um", reset, 40 + (bg & 7));
                    }
                    ttputpad(ebuf);

                } else if (*reset) {            /* normal */
                    ttputpad(reset);
                }
                assert(l < sizeof(ebuf));

                if (sf) term_styleon(sf);
            }

        } else if (tc_ANSI_Color_Fg) {
            /*
             *  ANSI - 16+8 color mode
             */
            if (COLORSOURCE_SYMBOLIC == ca.fg.source) fg = color_map[fg].c16;
            if (COLORSOURCE_SYMBOLIC == ca.bg.source) bg = color_map[bg].c16;
            if (fg == bg && bg >= 0) {
                if (TA_DARK & t_attributes) {
                    fg = color_map[WHITE].c16;
                    bg = color_map[BLACK].c16;
                } else {
                    fg = color_map[BLACK].c16;
                    bg = color_map[WHITE].c16;
                }
            }

            ED_TERM(("->mapansi(fg:%d, bg:%d)", fg, bg))

            if (fg != tt_fg || bg != tt_bg || sf != tt_style) {
                if (tt_style) {
                    if (tc_me) reset = NULL;
                    term_styleoff();
                }

                if (reset && (fg <= 0 || bg <= 0)) {
                    ttputpad(reset);            /* normal */
                }

                if (fg > 0) {                   /* foreground */
                    if (tt_colors == 8) ttputpad((fg & 0x8) ? tc_md : tc_se);
                    ttputctl(tc_ANSI_Color_Fg, fg);
                }

                if (bg > 0) {                   /* background */
                    ttputctl(tc_ANSI_Color_Bg, bg & 7);
                }

                if (sf) term_styleon(sf);
            }

        } else {
            /*
             *  PC - 16+8 color mode
             *  Note: assumes alternative color mapping.
             */
            if (COLORSOURCE_SYMBOLIC == ca.fg.source) fg = color_map[fg].c16_pc;
            if (COLORSOURCE_SYMBOLIC == ca.bg.source) bg = color_map[bg].c16_pc;
            if (fg == bg && bg >= 0) {
                if (TA_DARK & t_attributes) {
                    fg = color_map[WHITE].c16_pc;
                    bg = color_map[BLACK].c16_pc;
                } else {
                    fg = color_map[BLACK].c16_pc;
                    bg = color_map[WHITE].c16_pc;
                }
            }

            ED_TERM(("->mappc(fg:%d, bg:%d)", fg, bg))

            if (fg != tt_fg || bg != tt_bg || sf != tt_style) {
                if (tt_style) {
                    if (tc_me) reset = NULL;
                    term_styleoff();
                }

                if (reset && (fg <= 0 || bg <= 0)) {
                    ttputpad(reset);            /* normal */
                }

                if (fg > 0) {                   /* foreground */
                    if (tt_colors == 8) ttputpad((fg & 0x8) ? tc_md : tc_se);
                    ttputctl(tc_Color_Fg, fg);
                }

                if (bg > 0) {                   /* background */
                    ttputctl(tc_Color_Bg, bg & 7);
                }

                if (sf) term_styleon(sf);
            }
        }

        tt_fg = fg;
        tt_bg = bg;
    }

    ED_TERM(("\n"))
}


/*  Function:           term_styleon
 *      Enable the states styles.
 *
 *  Parameters:
 *      ntype -   New style bitmap.
 *
 *  Returns:
 *      nothing.
 */
static void
term_styleon(unsigned nstyle)
{
    if (vtiscolor() && tn_NC > 0) {
        return;                                 /* XXX - color/attributes dont mix, should mask */
    }

#if defined(HAVE_TERMINFO)
    if (tc_sa && !vtiscolor()) {
        ttputpad(tparm((char *)tc_sa,           /* attribute set, mono only */
            /*STANDOUT*/    (nstyle & (COLORSTYLE_STANDOUT|COLORSTYLE_INVERSE)) != 0,
            /*UNDERLINE*/   (nstyle &  COLORSTYLE_UNDERLINE) != 0,
            /*REVERSE*/     (nstyle &  COLORSTYLE_REVERSE) != 0,
            /*BLINK*/       (nstyle &  COLORSTYLE_BLINK) != 0,
            /*DIM*/         (nstyle &  COLORSTYLE_DIM) != 0,
            /*BOLD*/        (nstyle &  COLORSTYLE_BOLD) != 0,
            /*INVISIBLE*/   0,
            /*PROTECT*/     0,
            /*ALTCHAR*/     (t_gmode ? 1 : 0)));
        tt_style |= nstyle;
        return;
    }
#endif  /*HAVE_TERMINFO*/

#define ACTIVATE_STYLE(x) \
                ((nstyle & (x)) && 0 == (tt_style & (x)))

    if (ACTIVATE_STYLE(COLORSTYLE_STANDOUT)) {
        ttputpad(tc_so);                        /* standout mode. */
    } else if (ACTIVATE_STYLE(COLORSTYLE_INVERSE)) {
        ttputpad(tc_so);
    }

    if (ACTIVATE_STYLE(COLORSTYLE_REVERSE)) {
        ttputpad(tc_mr ? tc_mr : tc_so);        /* reverse  */
    }

    if (ACTIVATE_STYLE(COLORSTYLE_UNDERLINE)) {
        ttputpad(tc_us);                        /* underline */
    }

    if (ACTIVATE_STYLE(COLORSTYLE_BOLD)) {
        ttputpad(tc_md);                        /* bold */
    }

    if (ACTIVATE_STYLE(COLORSTYLE_BLINK)) {
        ttputpad(tc_mb);                        /* blink */
    }

    if (ACTIVATE_STYLE(COLORSTYLE_ITALIC)) {
        ttputpad(tc_ZH);                        /* italic */
    }

    tt_style |= nstyle;
}


/*  Function:           term_styleoff
 *      Disable any current styles.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
term_styleoff(void)
{
    if (tt_style && tc_me) {                    /* simple */
        ttputpad(tc_me);

    } else {                                    /* case-by-case */
        if (tt_style & (COLORSTYLE_STANDOUT|COLORSTYLE_INVERSE|COLORSTYLE_BOLD|COLORSTYLE_BLINK)) {
            /*
             *  end standout mode
             */
            if (tc_se == tc_ZH) tt_style &= (unsigned)(~COLORSTYLE_REVERSE);
            if (tc_se == tc_ue) tt_style &= (unsigned)(~COLORSTYLE_UNDERLINE);
            if (tc_se == tc_ZR) tt_style &= (unsigned)(~COLORSTYLE_ITALIC);
            ttputpad(tc_se);                    /* inverse, bold ... */
        }

        if (tt_style & COLORSTYLE_REVERSE) {
            ttputpad(tc_ZX);                    /* reverse */
        }

        if (tt_style & COLORSTYLE_UNDERLINE) {  /* underline */
            ttputpad(tc_ue);
        }

        if (tt_style & COLORSTYLE_ITALIC) {     /* italic */
            ttputpad(tc_ZR);
        }
    }
    tt_style = 0;
}


/*  Function:           term_colorreset
 *      Reset the terminal color to the system default.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
term_colorreset(void)
{
    ttputpad(tc_me ? tc_me : "\033[0m");
    tt_fg = tt_bg = NOCOLOR;
    tt_hue = 0;
}


/*  Function:           putfakech
 *      Fake character output support callback for term_cost().
 *
 *  Parameters:
 *      c - Character value.
 *
 *  Returns:
 *      Always zero.
 */
static int
putfakech(TPUTS_OUTTYPE c)
{
    (void)c;
    ++t_cost;
    return 0;
}


/*  Function:           term_cost
 *      Calculate the cost of displaying the specified string 'str'.
 *
 *  Parameters:
 *      str - Control sequence.
 *
 *  Returns:
 *      cost in characters.
 */
static int
term_cost(const char *str)
{
    t_cost = 0;
    tputs((char *)str, ttrows(), TPUTS_OUTFUNC putfakech);
    return t_cost;
}


/*  Function:           putpadch
 *      termcap ttputpad() callback.
 *
 *  Parameters:
 *      ch - Character value.
 *
 *  Returns:
 *      Always zero,
 */
static int
putpadch(TPUTS_OUTTYPE ch)
{
    if (ch == t_padchar && x_pt.pt_tty_fast > 0) {
        return 0;                               /* consume padding characters */
    }
    if (t_count >= sizeof(t_buffer)) {
        panic("ttputpad buffer overflow.");
    }
    t_buffer[ t_count++ ] = (unsigned char)ch;
    return 0;
}


/*  Function:           ttputpad
 *      termcap/terminfo output primitive.
 *
 *  Parameters:
 *      str - Control sequence.
 *
 *  Returns:
 *      nothing.
 */
static void
ttputpad(const char *str)
{
    if (str && str[0]) {
        if (t_count + strlen(str) >= NOBUF) {
            term_flush();
        }
        ED_TRACE3(("ttputpad(\"%s\")\n", str))
        tputs((char *)str, 1, TPUTS_OUTFUNC putpadch);
    }
}


/*  Function:           ttputctl
 *      termcap/terminfo print output primitive.
 *
 *  Parameters:
 *      str - Control sequence.
 *      p1 - Argument.
 *
 *  Returns:
 *      nothing.
 */
static void
ttputctl(const char *str, int p1)
{
    if (str && str[0]) {
#if defined(HAVE_TERMINFO)
        ttputpad(tparm((char *)str, p1, 0));
#else
        ttputpad(tgoto((char *)str, 0, p1));
#endif
    }
}


/*  Function:           ttputctl2
 *      termcap/terminfo print output primitive.
 *
 *  Parameters:
 *      str - Control sequence.
 *      p2 - Second parameter.
 *      p1 - First parameter.
 *
 *  Returns:
 *      nothing.
 */
static void
ttputctl2(const char *str, int p2, int p1)
{
    if (str && str[0]) {
#if defined(HAVE_TERMINFO)
        ttputpad(tparm((char *)str, p1, p2, 0));
#else
        ttputpad(tgoto((char *)str, p2, p1));
#endif
    }
}


/*  Function:           ttprintf
 *      Non-termcap print output primitive.
 *
 *  Parameters:
 *      str - Format specification.
 *      ... - optional argument.
 *
 *  Returns:
 *      nothing.
 */
static void
ttprintf(const char *str, ...)
{
    va_list ap;
    char buf[512];

    va_start(ap, str);
    vsxprintf(buf, sizeof(buf), str, ap);
    ttputpad(buf);
    va_end(ap);
}


/*  Function:           ttpush
 *      Push characters into the output buffer.
 *
 *  Parameters:
 *      cp - Character buffer.
 *
 *  Returns:
 *      nothing.
 */
void
ttpush(const char *cp)
{
    const int len = strlen(cp);

    if ((t_count + len) > sizeof(t_buffer)) {
        panic("ttpush buffer overflow.");
    }
    memcpy(t_buffer + t_count, cp, len);
    t_count += len;
}


/*  Function:           ttputc
 *      Write character to the display.
 *
 *      Characters are buffered up, to make things a little bit more efficient.
 *
 *  Parameters:
 *      c - Character value.
 *
 *  Returns:
 *      nothing.
 */
static void
term_putc(vbyte_t c)
{
    const int rows = ttrows() - 1, cols = ttcols() - 1;
    int ttrow = ttatrow(), ttcol = ttatcol();
    int width = 1;

    assert(ttrow <= rows);
    assert(ttcol <= cols);

    if (t_count >= (NOBUF - (MCHAR_MAX_LENGTH * 2))) {
        term_flush();
    }

    /*
     *  colorize
     */
    term_attr(c & VBYTE_ATTR_MASK);
    c &= VBYTE_CHAR_MASK;
    if (0 == c) {                               /* color-change only */
        return;
    }

    /*
     *  UNICODE and special characters
     */
    if (c >= 0x80) {
        const int isutf8 = vtisutf8();

        /*
         *  internal characters
         */
        if (c >= CH_MIN && c <= CH_MAX) {
            const int unicode = (xf_graph && isutf8 &&
                        (t_acs_locale_breaks || 0 == (DC_ASCIIONLY & x_display_ctrl)));
            int uch;

            ED_TRACE3(("graphic(isutf8:%d, char:%d/0x%x)\n", isutf8, c, c))
            if (unicode && (uch = cmap_specunicode(c)) > 0) {
                if (CH_PADDING == (c = uch)) {
                    ED_TRACE3(("ttputc_pad(row:%d, col:%d\n", ttrow, ttcol))
                    return;
                }
            } else {
                const char *cp;

                if (NULL != (cp = ttspecchar(c))) {
                    ED_TRACE3(("ttputc_spec(row:%d, col:%d, char:%d/0x%x, cp:%s)\n",\
                        ttrow, ttcol, c, c, cp))
                    if (x_pt.pt_tty_graphicsbox) {
                        term_graphic_enter();
                    }
                    ttpush(cp);
                    goto complete;
                }
                ED_TRACE3(("--> no mapping\n"))
            }
        }

        /*
         *  width character enabled displays
         */
        if (isutf8 && MCHAR_ISUTF8(c)) {        /* MCHAR */
            if ((width = ucs_width(c)) >= 0) {
                ED_TRACE3(("ttputc_utf8(row:%d, col:%d, char:%d/0x%x, width:%d)\n",\
                    ttrow, ttcol, c, c, width))
                term_graphic_exit();
                t_count += charset_utf8_encode(c, (char *)(t_buffer + t_count));
                goto complete;
            }
            width = 1;                          /* control */
        }
    }

    /*
     *  others
     */
    ED_TRACE3(("ttputc_norm(row:%d, col:%d, char:%d/0x%x/%c)\n",\
        ttrow, ttcol, c, c, (c >= 32 && c < 0x7f ? c : '.')))

    term_graphic_exit();
    if (ASCIIDEF_ESC == c && x_pt.pt_escape[0]) {
        ttpush(x_pt.pt_escape);
        goto complete;

    } else if (c >= 0x80) {
        if (c >= 0xa0 || 0 == x_pt.pt_character[0]) {
            goto hell;
        }

        ED_TERM(("ttchar(ch:0x%2x/?, wid:%d, pos:%2d/%2d, cnt:%d)\n",\
            c, width, ttrow, ttcol, t_count))
        t_count += sprintf((char *)(t_buffer + t_count), x_pt.pt_character, (int)c);
        goto complete;
    }

    if ('~' == c && tf_hz) {                    /* no hazel support (rare) - remap */
        c = '^';
    }

hell:;
    ED_TERM(("ttchar(ch:0x%2x/%c, attr:0x%2x, wid:%d, pos:%2d/%2d, cnt:%d)\n", \
        c, (c <= 0x79 ? c : ' '), width, ttrow, ttcol, t_count))

    t_buffer[t_count++] = (unsigned char)(c);
    assert(c >= ' ');

    if ('\b' == c) {                            /* backspace */
        if (ttcol) {
            ED_TRACE3(("ttmove(%d,%d->back\n", ttrow, ttcol))
            ttposset(ttrow, ttcol - 1);
        } else {
            ttposinvalid();
        }
        return;

    } else if ('\n' == c) {                     /* new-line */
        if (ttrow < rows) {
            ED_TRACE3(("ttmove(%d,%d->newline\n", ttrow, ttcol))
            ttposset(ttrow + 1, ttcol);
        } else {
            ttposinvalid();
        }
        return;

    } else if ('\r' == c) {                     /* return */
        ED_TRACE3(("ttmove(%d,%d->return\n", ttrow, ttcol))
        ttposset(ttrow, 0);
        return;

    } else if (ASCIIDEF_BEL == c) {             /* bell (noisy) */
        ED_TRACE3(("ttmove(%d,%d)->bell\n", ttrow, ttcol))
        return;
    }

complete:;
    ttcol += width;

    ED_TRACE3(("ttmove(%d,%d)-", ttrow, ttcol))

    if (ttcol > cols) {                         /* cursor wrap */
        /*
         *  Source: http://www.gnu.org/software/termutils/manual/
         */
        if (tf_xn /*&& (cols >= 80 || ttrow == rows)*/) {
            /*
             *  undefined/strange eol cursor rules/NL ignore after 80th column
             */
            ED_TRACE3(("xn-(-1,-1)\n"))
            ttposinvalid();
            return;

        } else if (tf_am) {
            /*
             *  obeys margins
             */
            ED_TRACE3(("wrap(%d,%d)", tt_top, tt_bot))
            if (tt_bot >= 0) {
                if (++ttrow > tt_bot) {
                    ttrow = tt_bot;             /* scroll region active */
                }
            } else if (++ttrow > rows) {
                ttrow = rows;                   /* otherwise physical screen */
            }
            ttcol = 0;

        } else {
            /*
             *  nowrap/ignored
             */
            ED_TRACE3(("->nowrap", ttrow, ttcol))
            --ttcol;
        }
    }

    ED_TRACE3(("-(%d,%d)\n", ttrow, ttcol))
    ttposset(ttrow, ttcol);
}


/*  Function:           term_graphic_enter
 *      Switch terminal into graphics/altnative character set mode.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static __CINLINE void
term_graphic_enter(void)
{
    if (xf_graph) {
        if (FALSE == t_gmode) {
            const char *cmd = x_pt.pt_graphics_mode;

            if (0 == cmd[0]) {
                cmd = tc_acs_start;             /* default */
            }
            if (cmd[0]) {
                ttputpad(cmd);
                t_gmode = TRUE;
            }
            ED_TRACE3(("term_graphic_enter(%d)\n", t_gmode))
        }
    }
}


/*  Function:           term_graphic_exit
 *      Switch terminal out of graphics/alternative character set mode.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static __CINLINE void
term_graphic_exit(void)
{
    if (t_gmode) {
        const char *cmd = x_pt.pt_text_mode;

        if (0 == cmd[0]) {
            cmd = tc_acs_end;                   /* default */
        }
        if (cmd[0]) {
            ttputpad(cmd);
        }
        t_gmode = FALSE;
        ED_TRACE3(("term_graphic_exit()\n"))
    }
}


/*  Function:           term_flush
 *      Flush the tty buffer.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
static void
term_flush(void)
{
    if (t_count > sizeof(t_buffer)) {
        panic("ttbuffer overflow.");
    }

    if (t_count > 0) {
        t_charout += t_count;
        if (FALSE == x_background) {
            if (!xf_compat) {                   /* TRUE=normal i/o otherwise optimised */
                sys_tty_delay(TTY_OUTFD, 1);
            }
            ED_TERM(("term_flush(length:%d)\n", t_count))
#if defined(ED_LEVEL) && (ED_LEVEL >= 1)
            if (DB_TERMINAL & x_dflags) {
                trace_data(t_buffer, t_count, "\n");
            }
#endif
            sys_write(TTY_OUTFD, t_buffer, t_count);
        }
        t_count = 0;
    }
}


/*  Function:           term_sizeget
 *      Retrieve the display screen.
 *
 *  Parameters:
 *      nrow - Storage for row number.
 *      ncol - Number of columns.
 *
 *  Returns:
 *      void
 */
static void
term_sizeget(int *nrow, int *ncol)
{
    const char *env;
#if defined(TIOCGWINSZ)
    struct winsize ws = {0};

    do {
        if ((ioctl(1, TIOCGWINSZ, (char *) &ws) == 0) ||
                (ioctl(0, TIOCGWINSZ, (char *) &ws) == 0) ||
                (ioctl(2, TIOCGWINSZ, (char *) &ws) == 0)) {
            if (ws.ws_row > 0 && ws.ws_col > 0) {
                *nrow = ws.ws_row;
                *ncol = ws.ws_col;
                trace_ilog("term_size1(%d, %d)\n", *nrow, *ncol);
                return;
            }
        }
    } while (errno == EINTR);

#elif defined(TIOCGSIZE)
    struct ttysize ts = {0};                    /* SUN */

    do {
        if ((ioctl(1, TIOCGSIZE, (char *) &ts) == 0) ||
                (ioctl(0, TIOCGSIZE, (char *) &ts) == 0) ||
                (ioctl(2, TIOCGSIZE, (char *) &ts) == 0)) {
            if (ts.ts_lines > 0 && ts.ts_linesncol > 0) {
                *nrow = ts.ts_lines;
                *ncol = ts.ts_cols;
                trace_ilog("term_size2(%d, %d)\n", *nrow, *ncol);
                return;
            }
        }
    } while (errno == EINTR);
#endif

#if defined(_VMS)
    {                                           /* UNTESTED/OLD */
        unsigned short chan;

        $DESCRIPTOR(dev_dsc, "SYS$INPUT:");
        if (sys$assign(&dev_dsc, &chan, 0, 0, 0) & 1) {
            int status_col, status_row, code;

            code = DVI$_DEVBUFSIZ;
            status_col = lib$getdvi(&code, &chan, 0, ncol, 0, 0);
            code = DVI$_TT_PAGE;
            status_row = lib$getdvi(&code, &chan, 0, nrow, 0, 0);
            sys$dassgn(chan);
            if ((status_col & 1) && (status_row & 1)) {
                return;
            }
        }
    }
#endif

    if (tn_li > 0 && tn_co > 0) {               /* terminal */
        *nrow = tn_li;
        *ncol = tn_co;
    } else {
        *nrow = 24;
        *ncol = 80;
    }
                                                /* old school */
    if (NULL != (env = ggetenv("LINES")) && *env) {
        *nrow = atoi(env);
        tf_am = FALSE;
    }

    if (NULL != (env = ggetenv("COLUMNS")) && *env) {
        *ncol = atoi(env);
        tf_am = FALSE;
    }
}


#if (NOTUSED)
/*  Function:           term_sizeset
 *      Attempt to set the current window size.
 *
 *  Parameters:
 *      row - Screen rows.
 *      col - Columns.
 *
 *  Returns:
 *      On success 0, otherwise -1.
 */
static int
term_sizeset(int rows, int cols)
{
    if (x_pt.pt_winsetsize[0]) {
        ttprintf(x_pt.pt_winsetsize, rows, cols);
        return 0;
    }
    return -1;
}
#endif


/*  Function:           ega_switch
 *      EGA/VGA console mode switch support
 *
 *  Parameters:
 *      flag - EGA mode.
 *
 *  Returns:
 *      nothing.
 */
static void
ega_switch(int flag)
{
    const int orows = ttrows();

    if (tty_egaflag == flag) {
        return;
    }

    switch (flag) {
#if defined(SW_VGA_C80x60)
    case 60:                /* 60 line mode */
        ioctl(1, SW_VGA_C80x60, 1);
        ttrows() = 60;
        break;
#endif

#if defined(SW_VGA_C80x50)
    case 50:                /* 50 line mode */
        ioctl(1, SW_VGA_C80x50, 1);
        ttrows() = 50;
        break;
#endif

#if defined(SW_ENH_C80x43)
    case 43:                /* 43 line mode */
        if (ioctl(1, SW_ENH_C80x43, 1) == 0) {
            ttrows() = 43;
        }
        break;
#endif

#if defined(SW_VGA_C80x25) || defined(SW_ENH_C80x25)
    case 25:                /* 25 line mode */
#if defined(SW_VGA_C80x25)
        ioctl(1, SW_VGA_C80x25, 1);
#endif
#if defined(SW_ENH_C80x25)
        ioctl(1, SW_ENH_C80x25, 1);
#endif
        ttrows() = 25;
        break
#endif

    default:
        acc_assign_int(-1);
        return;
    }

    tty_egaflag = xf_ega43 = flag;
    vtwinch(ttcols(), orows);
    term_colorreset();
}


/*  Function:           do_ega
 *      ega primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: ega - Terminal line display.

        int
        ega(int lines)

    Macro Description:
        The 'ega()' primitive attempts to configure the console size
        to the specified number of 'lines' with an implied width of
        80 columns on supporting terminals.

        Possible screen dimensions include.

            o 60 x 80
            o 50 x 80
            o 43 x 80
            o 25 x 80

    Macro Parameters:
        lines - Optional integer specifing the required number of
            console lines. Under WIN32 if -1 the console size shall toggled
            bewteen minimized and maximised. If omitted, then only the
            current state is returned.

    Notes!:
        When running within a windows console, before Windows 7 one could press
        <Alt+Enter> to run the application in full screen. As of Windows 7 this
        functionality is no longer available resulting in the following.

>           This system does not support full screen mode

        The ega() primitive can be used to emulate this functionality, using
        the special -1 flag, which shall toggle between a minimized or maximised
        sized console.

>           ega(-1)

    Macro Returns:
        The 'ega()' primitive returns the previous status, otherwise -1 upon
        an error.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_term_feature
 */
void
do_ega(void)                    /* int (int mode) */
{
    acc_assign_int((accint_t) xf_ega43);
    if (isa_integer(1)) {                       /* (mode < 80 sets rows), otherwise (mode >= 80 sets columns). */
        const int flag = get_xinteger(1, 0);
        if (flag >= 0) {
            ega_switch(flag);
        }
    }
}


/*  Function:           do_copy_screen
 *      copy_screen primitive, defunct.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: copy_screen - Copy the current screen.

        void
        copy_screen()

    Macro Description:
        The 'copy_screen()' primitive exports an image of the
        current editor windows to the current buffer.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension; this primitive is subject for removal,
        and has been removed from recent CrispEdit(tm) releases.

    Macro See Also:
        transfer
 */
void
do_copy_screen(void)            /* void () */
{
}

#if (TODO_TESTS)
    assert(isterm("screen", "screen"));
    assert(isterm("screen-xxx", "screen"));
    assert(isterm("screen.linux", "screen"));
    assert(isterm("screen.linux", "screen.linux"));
    assert(! isterm("screen1", "screen"));
    assert(! isterm("screen2", "screen"));
    assert(hasfeature("screen-rv", "rv"));
    assert(hasfeature("screen-rv-xxx", "rv"));
    assert(hasfeature("screen-xxx-rv", "rv"));
#endif

#endif  /*!USE_VIO_BUFFER && !DJGPP */








