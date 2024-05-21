#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyvio_c,"$Id: ttyvio.c,v 1.78 2024/05/20 17:06:25 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyvio.c,v 1.78 2024/05/20 17:06:25 cvsuser Exp $
 * TTY VIO implementation.
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

#if defined(USE_VIO_BUFFER)
#include <edtermio.h>
#include <edtrace.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <edalt.h>

#include "../libchartable/libchartable.h"
#include "../libwidechar/widechar.h"

#if defined(WIN32)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#elif (_WIN32_WINNT < 0x601)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif
#undef WINVER
#define WINVER _WIN32_WINNT

#if !defined(WINDOWS_MEAN_AND_LEAN)
#define WINDOWS_MEAN_AND_LEAN
#include <windows.h>
#endif
#pragma comment(lib, "gdi32.lib")
#endif

#include "accum.h"
#include "builtin.h"
#include "color.h"
#include "debug.h"                              /* trace ... */
#include "cmap.h"
#include "display.h"
#include "echo.h"
#include "getkey.h"
#include "keyboard.h"
#include "eval.h"
#include "line.h"
#include "main.h"
#include "map.h"
#include "mouse.h"
#include "playback.h"
#include "procspawn.h"
#include "system.h"
#include "tty.h"
#include "undo.h"
#include "window.h"
#include "vio.h"

#define VSET_BITS       (32)
#define VSET_MAX        (512)
#define VSET_SIZE       ((VSET_MAX / VSET_BITS) + 1)

#define vset_zero(s)    memset((s), (unsigned char) 0, sizeof(videoset_t))
#define vset_fill(s)    memset((s), (unsigned char) 0xff, sizeof(videoset_t))

#define vset_set(s,c)   ((s)[(c)/VSET_BITS] |=  (1 << (c)%VSET_BITS))
#define vset_clr(s,c)   ((s)[(c)/VSET_BITS] &= ~(1 << (c)%VSET_BITS))
#define vset_tst(s,c)   ((s)[(c)/VSET_BITS] &   (1 << (c)%VSET_BITS))

typedef uint32_t videoset_t[VSET_SIZE];

static void             term_open(scrprofile_t *profile);
static void             term_ready(int repaint, scrprofile_t *profile);
static void             term_colors(void);
static void             term_close(void);
static void             term_feature(int ident, scrprofile_t *profile);
static void             term_display(void);
static int              term_control(int action, int param, ...);
static void             term_tidy(void);
static int              term_cursor(int visible, int imode, int virtual_space);
static void             term_sizeget(int *nrows, int *ncols);

static int              term_names(const char *title, const char *icon);
static void             term_beep(int freq, int duration);
static int              term_font(int setlen, char *font);
static void             term_print(int row, int col, int len, const VCELL_t *vvp);
static void             term_putc(vbyte_t c);
static void             term_clear(void);
static void             term_flush(void);

static int              term_insl(int row, int bot, int nlines, vbyte_t fillcolor);
static int              term_dell(int row, int bot, int nlines, vbyte_t fillcolor);
static int              term_eeol(void);
static void             term_repeat(int cnt, vbyte_t fill, int where);
static void             term_attribute(const vbyte_t color);
static unsigned         term_colvalue(const colvalue_t ca, unsigned def);
static void             term_attr(int fg, int bg);
static void             term_hue(int fg, int bg);

static void             vio_reference(void);
static void             vio_image_save(void);
static void             vio_image_restore(void);
static void             vio_dirty(int row, int col, int erow, int ecol);

#if !defined(WIN32)
#define FOREGROUND_BLUE         0x0001
#define FOREGROUND_GREEN        0x0002
#define FOREGROUND_RED          0x0004
#define FOREGROUND_INTENSITY    0x0008
#endif

enum WIN32Color {
    WIN32_Black         = 0,
    WIN32_Grey          = FOREGROUND_INTENSITY,
    WIN32_LightGrey     = FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE,
    WIN32_White         = FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
    WIN32_Blue          = FOREGROUND_BLUE,
    WIN32_Green         = FOREGROUND_GREEN,
    WIN32_Cyan          = FOREGROUND_GREEN | FOREGROUND_BLUE,
    WIN32_Red           = FOREGROUND_RED,
    WIN32_Magenta       = FOREGROUND_RED   | FOREGROUND_BLUE,
    WIN32_LightBlue     = FOREGROUND_BLUE  | FOREGROUND_INTENSITY,
    WIN32_LightGreen    = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    WIN32_LightCyan     = FOREGROUND_GREEN | FOREGROUND_BLUE  | FOREGROUND_INTENSITY,
    WIN32_LightRed      = FOREGROUND_RED   | FOREGROUND_INTENSITY,
    WIN32_LightMagenta  = FOREGROUND_RED   | FOREGROUND_BLUE  | FOREGROUND_INTENSITY,
    WIN32_Orange        = FOREGROUND_RED   | FOREGROUND_GREEN,
    WIN32_Yellow        = FOREGROUND_RED   | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
    };

static const struct colormap {                  /* BRIEF -> VIO palette */
#if defined(WIN32)
#define C16(a,b)        b
#else
#define C16(a,b)        a
#endif
    int     c16;
    int     c256_compat;                        /* 8/16 color compat */
    int     c256;
} color_map[] = {
    /*Ident           16- PC/WIN                    56/C    256     */
    /*BLACK     */  { C16(0,  WIN32_Black),         0,      0       },
    /*BLUE      */  { C16(1,  WIN32_Blue),          4,      12      },
    /*GREEN     */  { C16(2,  WIN32_Green),         2,      10      },
    /*CYAN      */  { C16(3,  WIN32_Cyan),          6,      14      },
    /*RED       */  { C16(4,  WIN32_Red),           1,      9       },
    /*MAGENTA   */  { C16(5,  WIN32_Magenta),       5,      13      },
    /*BROWN     */  { C16(6,  WIN32_Orange),        130,    130     },
    /*WHITE     */  { C16(7,  WIN32_LightGrey),     7,      248     },

    /*GREY      */  { C16(8,  WIN32_Grey),          8,      7       },
    /*LTBLUE    */  { C16(9,  WIN32_LightBlue),     12,     81      },
    /*LTGREEN   */  { C16(10, WIN32_LightGreen),    10,     121     },
    /*LTCYAN    */  { C16(11, WIN32_LightCyan),     14,     159     },
    /*LTRED     */  { C16(12, WIN32_LightRed),      9,      224     },
    /*LTMAGENTA */  { C16(13, WIN32_LightMagenta),  13,     225     },
    /*YELLOW    */  { C16(14, WIN32_Yellow),        11,     11      },
    /*LTWHITE   */  { C16(15, WIN32_White),         15,     15      },

    /*DKGREY    */  { C16(0,  WIN32_Grey),          0,      242     },
    /*DKBLUE    */  { C16(1,  WIN32_Blue),          4,      4       },
    /*DKGREEN   */  { C16(2,  WIN32_Green),         2,      2       },
    /*DKCYAN    */  { C16(3,  WIN32_Cyan),          6,      6       },
    /*DKRED     */  { C16(4,  WIN32_Red),           1,      1       },
    /*DKMAGENTA */  { C16(5,  WIN32_Magenta),       5,      5       },
    /*DKYELLOW  */  { C16(6,  WIN32_Orange),        130,    130     },
    /*LTYELLOW  */  { C16(14, WIN32_Yellow),        11,     229     },

    /*COLOR_NONE*/  { -1, -1, -1 }
    };

static int              tt_open         = FALSE;
static int              tt_cursor       = TRUE;
static int              tt_active       = FALSE;
static int              tt_colors       = 16;
static unsigned         tt_defaultbg    = 0;
static unsigned         tt_defaultfg    = 7;
static VIOHUE           tt_hue;
static uint16_t         tt_style;
static char             tt_title[100];
static int              tt_colormap[COLOR_NONE + 1];

/*
 *  Physical buffer, original and current
 */
#if !defined(WIN32)
static USHORT           origAttribute;
static VIOCURSORINFO    origCursor;
static int              origRows;
static int              origCols;
#endif

static USHORT           origRow;
static USHORT           origCol;
static const VIOCELL *  origScreen;
static WCHAR            origTitle[100];

static int              currRows;
static int              currCols;
static int              currCodePage = -1;
static ULONG            currScreenCells;
static VIOCELL *        currScreen;
static int              currDirtyTop = -1;
static int              currDirtyEnd = -1;
static videoset_t       currDirtyFlg;


/*  Function:           ttinit
 *      Initialize the terminal when the editor gets started up.
 *
 *  Calling Sequence:
 *>     vtinit
 *>         ttinit
 *>     ttopen
 *>     ttready
 *>     macro tty/<terminal-type>
 *>     ttdisplay
 *>     [ttfeature]
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
ttinit(void)
{
    x_scrfn.scr_open    = term_open;
    x_scrfn.scr_ready   = term_ready;
    x_scrfn.scr_feature = term_feature;
    x_scrfn.scr_display = term_display;
    x_scrfn.scr_control = term_control;
    x_scrfn.scr_close   = term_close;

    x_scrfn.scr_cursor  = term_cursor;
    x_scrfn.scr_winch   = term_sizeget;

    x_scrfn.scr_names   = term_names;
    x_scrfn.scr_beep    = term_beep;
    x_scrfn.scr_font    = term_font;

#if defined(WIN32)
    x_scrfn.scr_event   = sys_getevent;
#endif

    x_scrfn.scr_clear   = term_clear;
    x_scrfn.scr_print   = term_print;
    x_scrfn.scr_putc    = term_putc;
    x_scrfn.scr_flush   = term_flush;

    x_scrfn.scr_insl    = term_insl;
    x_scrfn.scr_dell    = term_dell;
    x_scrfn.scr_eeol    = term_eeol;
    x_scrfn.scr_repeat  = term_repeat;

    (void)sprintf(tt_title, "%s (%s)", x_appname, x_version);
    vio_reference();
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
    int isdark = 1;                             /* Win32 default */

    if (x_pt.pt_schemedark >= 0) {              /* explicit configuration */
        isdark = x_pt.pt_schemedark;
    }
    x_pt.pt_schemedark = isdark;
    trace_log("ttdefaultscheme=%s\n", (isdark ? "dark" : "light"));
    return (isdark ? "dark" : "light");
}


/*  Function:           term_ready
 *      Runtime initialisation
 *
 *  Parameters:
 *      repaint -           *true* if the screen should be repainted.
 *      profile -           Terminal profile.
 *
 *  Returns:
 *      nothing
 */
static void
term_ready(int repaint, scrprofile_t *profile)
{
    vio_reference();
    if (repaint) {
        vio_image_save();
    }
#if defined(WIN32)
    SetConsoleTitleA(tt_title);
#endif
    if (profile) {
        VIOMODEINFO mi = { sizeof(VIOMODEINFO) };

        VioGetMode(&mi, 0);
        profile->sp_rows = (int)mi.row;
        profile->sp_cols = (int)mi.col;
        if (mi.color > 0 && mi.color <= 256) {
            if (-1 == xf_color || mi.attrib <= xf_color) {
                tt_colors = mi.color;
            }
        }
        profile->sp_colors = tt_colors;
        profile->sp_lastsafe = TRUE;
    }
    term_colors();
    term_attr(WHITE, BLACK);
    x_pt.pt_colordepth = tt_colors;             /* derived depth */
    if (tt_colors > 2) {
        x_display_ctrl |= DC_SHADOW_SHOWTHRU;
    }
    tt_active = 1;
    term_flush();
}


static void
term_colors(void)
{
    int col;

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
            } else {
                color = color_map[col].c16;
            }
        }

        tt_colormap[col] = color;
        trace_ilog("\t%16s [%2d] = %d\n", color_name(col, "unknown"), col, tt_colormap[col]);
    }
}


/*  Function:           term_open
 *      Open the console
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static void
term_open(scrprofile_t *profile)
{
    tt_open = tt_cursor = TRUE;
    io_device_add(TTY_INFD);
    if (profile) {
        term_sizeget(&profile->sp_rows, &profile->sp_cols);
        profile->sp_colors = tt_colors;
    }
    sys_initialise();
}


/*  Function:           term_close
 *      Close the console
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static void
term_close(void)
{
    term_tidy();
    sys_shutdown();
    tty_egaflag = -1;
}


/*  Function:           term_feature
 *      Signal a terminal feature change.
 *
 *  Parameters:
 *      ident -             Feature identifier.
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
        break;

    case TF_COLORDEPTH:
    case TF_DEFAULT_FG:
    case TF_DEFAULT_BG:
    case TF_XTERM_PALETTE:
        if (tty_open) {
            profile->sp_colors = tt_colors;
        }
        break;

    case TF_COLORMAP:                           /* terminal color palette */
        break;

    case TF_COLORPALETTE:                       /* user defined palette */
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

    if (xf_disptype < 0) {                      /* default disp-type */
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


static int
term_control(int action, int param, ...)
{
    switch (action) {
    case SCR_CTRL_NORMAL:       /* normal color */
        break;

    case SCR_CTRL_GARBLED:
        vtgarbled();
        vio_dirty(0, 0, ttrows() - 1, ttcols() - 1);
        break;

    case SCR_CTRL_SAVE:
        if (param) {
            term_tidy();
        } else {
            ttmove(ttrows() - 1, 0);
            term_tidy();
        }
        break;

    case SCR_CTRL_RESTORE:
        term_ready(TRUE, NULL);
        vtgarbled();
        vio_dirty(0, 0, ttrows() - 1, ttcols() - 1);
        break;

    case SCR_CTRL_COLORS:       /* color change */
        break;

    default:
        return -1;
    }
    return 0;
}


/*  Function:           term_tidy
 *      Cleanup/restore the state of the console.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static void
term_tidy(void)
{
    vio_image_restore();
    tt_active = 0;
}


/*  Function:           vio_reference
//      Reference the current video buffer.
//
//  Parameters:
//      none
//
//  Returns:
//      nothing
*/
static void
vio_reference(void)
{
    VIOMODEINFO mi = { sizeof(VIOMODEINFO) };
    USHORT cp = 0;

    VioGetMode(&mi, 0);

#if defined(__OS2__)
    {                                           /* defunct */
        _far16ptr p;
        VioGetBuf((PULONG) &p, &currScreenCells, 0);
        currScreen = _emx_16to32(p);
        VioReadCellPtr(currScreen, &currScreenCells, 0, 0, 0);
    }
#else
    VioGetBuf(&currScreen, &currScreenCells, 0);
#endif

    VioGetCp(0, &cp, 0);
    x_pt.pt_codepage = (int)cp;
    currRows = mi.row;
    currCols = mi.col;
    currCodePage = (int)cp;
}


/*  Function:           vio_image_save
//      Save the current video buffer.
//
//  Parameters:
//      none
//
//  Returns:
//      nothing
*/
static void
vio_image_save(void)
{
#if defined(WIN32)
    if (0 == origTitle[0])
        GetConsoleTitleW(origTitle, _countof(origTitle));
    vio_save();

#else
    ULONG length = currRows * currCols * sizeof(VIOCELL);
    VIOCELL *screen;
    int rc;

    assert(currScreen);
    if (origScreen) {
        chk_free((void *)origScreen);
    }
    origRows = currRows;
    origCols = currCols;

    if (NULL != (screen = chk_calloc(length, 1))) {
        rc = VioReadCellStr(screen, &length, 0, 0, 0);           
        assert(0 == rc);                        /* image */

        origCursor.cb = sizeof(VIOCURSORINFO);
        VioGetCurType(&origCursor, 0);          /* cursor */
        VioGetCurPos(&origRow, &origCol, 0);
        origScreen = screen;
    }
#endif
}


/*  Function:           vio_image_restore
//      Restore the previously saved video buffer. If the event the current image size
//      has changed, the previous image if clicked or padded as required.
//
//  Parameters:
//      none
//
//  Returns:
//      nothing
*/
static void
vio_image_restore(void)
{
#if defined(WIN32)
    if (origTitle[0]) 
        SetConsoleTitleW(origTitle);
    vio_restore();

#else   //!WIN32
    if (origScreen) {
        const int rows = currRows, cols = currCols;
        const int cnt = (origCols <= cols ? origCols : cols);
        const VIOCELL blank = VIO_INIT(tt_hue, ' ');
        VIOCELL *cursor;
        int r;

        for (r = 0; r < rows; ++r) {            /* transfer image and clear */
            int c = 0;
            if (r < origRows) {
                memcpy(currScreen + (r * cols), origScreen + (r * origCols), cnt * sizeof(VIOCELL));
                if ((c = cnt) == cols) {
                    continue;
                }
            }
            cursor = currScreen + (r*cols) + c; /* blank out new columns */
            while (c++ < cols) {
                *cursor++ = blank;
            }
        }

        VioShowBuf(0, currScreenCells, 0);      /* restore image */
        chk_free((void *)origScreen);
        origScreen = NULL;

        VioSetCurPos(origRow, origCol, 0);      /* cursor */
        VioSetCurType(&origCursor, 0);
    }
#endif  //WIN32
}


/*  Function:           vio_dirty
//      Mark the stated area as dirty, updating the update arena.
//
//  Parameters:
//      none
//
//  Returns:
//      nothing
*/
static void
vio_dirty(int row, int col, int erow, int ecol)
{
    const int rows = ttrows(), cols = ttcols();

    assert(row  >= 0);
    assert(row  <  rows);
    assert(erow >= row);
    assert(erow <  rows);

    assert(col  >= 0);
    assert(col  <  cols);
    assert(ecol >= col);
    assert(ecol <  cols);

    if (-1 == currDirtyTop) {
        currDirtyTop = row;
        currDirtyEnd = erow;
    } else {
        if (row  < currDirtyTop) currDirtyTop = row;
        if (erow > currDirtyEnd) currDirtyEnd = erow;
    }

    while (row <= erow && row < VSET_MAX) {
        vset_set(currDirtyFlg, row);
        ++row;
    }
}


/*  Function:           term_sizeget
 *      Retrieve the display screen.
 *
 *  Parameters:
 *      nrows -             Storage for row number.
 *      ncols -             Number of columns.
 *
 *  Returns:
 *      void
 */
static void
term_sizeget(int *nrows, int *ncols)
{
    VIOMODEINFO mi = { sizeof(VIOMODEINFO) };

    VioGetMode(&mi, 0);
    if (ncols) *ncols = (int)mi.col;
    if (nrows) *nrows = (int)mi.row;
}


/*  Function:           term_cursor
 *      Set the cursor status.
 *
 *  Parameters:
 *      visible -           Visible status.
 *      imode -             Insert mode.
 *      virtual_space -     Virtual space mode.
 *
 *  Returns:
 *      returns the current visible status, otherwise -1 on error.
 */
static int
term_cursor(int visible, int imode, int virtual_space)
{
    if (visible >= 0) {
        VIOCURSORINFO ci = { sizeof(VIOCURSORINFO) };

        VioGetCurType(&ci, 0);
        ci.attr = (visible ? 1 : -1);           /* 1 = visible otherwise invisible */
        VioSetCurType(&ci, 0);
        tt_cursor = visible;
    }

    if (tt_cursor && imode >= 0) {
        if (imode) {
            if (virtual_space) {
                VioCursor(VIOCUR_BHALF);        /* bottom half */
            } else {
                VioCursor(VIOCUR_ON);           /* underline */
            }
        } else {
            if (virtual_space) {
                VioCursor(VIOCUR_THALF);        /* top half */
            } else {
                VioCursor(VIOCUR_FULL);         /* full */
            }
        }
    }
    return tt_cursor;
}


/*  Function:           term_names
 *      Set the console title
 *
 *  Parameters:
 *      title -             Title buffer.
 *
 *  Returns:
 *      nothing
 */
static int
term_names(const char *title, const char *icon)
{
    __CUNUSED(title)
    __CUNUSED(icon)
#if defined(WIN32)
    if (title && title[0]) {
        SetConsoleTitleA(title);
    } else if (tt_title[0]) {
        SetConsoleTitleA(tt_title);
    }
#endif
    return 0;
}


/*  Function:           term_beep
 *      Terminal beep/visual notice.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static void
term_beep(int freq, int duration)
{
    DosBeep(freq > 0 ? freq : 2048, duration > 0 ? duration : 50);
}


/*  Function:           term_font
 *      Terminal font set/get.
 *
 *  Parameters:
 *      setlen -            Output buffer length in byte, implied an enquiry.
 *      font -              Font number.
 *
 *  Returns:
 *      0 on successful, otherwise a non-zero error code.
 */
static int
term_font(int setlen, char *font)
{
#if defined(WIN32)
    if (font) {
        if (setlen > 0) {
            return VioGetFont(font, setlen);
        }
        return (0 == VioSetFont((const char *)font) ? 0 : -1);
    }
#endif
    return -1;
}


/*  Function:           term_print
 *      Write character cells to the display.
 *
 *  Parameters:
 *      row -               Row.
 *      col -               Column.
 *      len -               Cell count.
 *      vvp -               Character cell array.
 *
 *  Returns:
 *      nothing
 */
static void
term_print(int row, int col, int len, const VCELL_t *vvp)
{
    static VIOCELL null = { 0 };
    
    if (len > 0) {
        const int isuc = (DC_CMAPFRAME & x_display_ctrl) || vtisunicode() || vtisutf8();
        VIOCELL *p = currScreen + (row * ttcols()) + col;
        vbyte_t cattr = VBYTE_ATTR_GET(vvp->primary);

        term_attribute(cattr);
        vio_dirty(row, col, row, (col + len) - 1);
        while (len--) {
            const vbyte_t attr = VBYTE_ATTR_GET(vvp->primary);
            vbyte_t c = VBYTE_CHAR_GET(vvp->primary);

            if (CH_PADDING == c) {              /* wide character padding */
                *p++ = null;
                ++vvp;
                continue;
            }

            if (cattr != attr) {		/* attribute change */
                cattr = attr, term_attribute(cattr);
            }

            if (c >= CH_MIN && c <= CH_MAX) {   /* drawing characters */
                int unicode;

                if (isuc && (unicode = cmap_specunicode(c)) > 0) {
                    c = unicode;
                } else {
                    const unsigned char *cp;
                    if (NULL != (cp = (unsigned char *)ttspecchar(c))) {
                        c = *cp;
                    }
                }
            }

            VIO_ASSIGN(p, tt_hue, c, tt_style)
            ++p;
            ++vvp;
        }
        term_attribute(ATTR_NORMAL);
    }
}


/*  Function:           term_putc
 *      Write character to the display. Generally unsed, as term_print() has priority.
 *
 *  Parameters:
 *      c -                 Character value.
 *
 *  Returns:
 *      nothing
 */
static void
term_putc(vbyte_t c)
{
    int ttrow = ttatrow(), ttcol = ttatcol();

    if (CH_PADDING == c) {                      /* wide character padding */
        return;
    }

    term_attribute(VBYTE_ATTR_GET(c));
    c = VBYTE_CHAR_GET(c);

    /*
     *  special characters
     */
    if (c >= CH_MIN && c <= CH_MAX) {
        const int isuc = (DC_CMAPFRAME & x_display_ctrl) || vtisunicode() || vtisutf8();
        int unicode;

        if (isuc && (unicode = cmap_specunicode(c)) > 0) {
            c = unicode;
        } else {
            const char *cp;
            if (NULL != (cp = ttspecchar(c))) {
                c = (unsigned char) *cp;
            }
        }
    }
    if (0 == c) {
        return;
    }

    /*
     *  Push result
     */
    if ('\b' == c) {                            /* backspace */
        if (ttcol) {
            --ttcol;
        }
    } else {
        VIOCELL *p = currScreen + (ttrow * ttcols()) + ttcol;

        VIO_ASSIGN(p, tt_hue, c, tt_style)
        vio_dirty(ttrow, ttcol, ttrow, ttcol + 1);
        if (++ttcol >= ttcols()) {
            ttcol = 0;
            ++ttrow;
        }
    }
    ttposset(ttrow, ttcol);
}


/*  Function:           term_clear
 *      clear the screen to the current NORMAL color.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static void
term_clear(void)
{
  //assert(currRows == ttrows() || tty_needresize /*FIXME: very small window*/);
  //assert(currCols == ttcols() || tty_needresize /*FIXME: very small window*/);
    term_attribute(ATTR_NORMAL);
    {
        const VIOCELL blank = VIO_INIT(tt_hue, ' ');
        int cells = ttcols() * ttrows();
        VIOCELL *p = currScreen;
        while (cells-- > 0) {
            *p++ = blank;
        }
    }
    ttposset(0, 0);
    vio_dirty(0, 0, ttrows()-1, ttcols()-1);
}


/*  Function:           term_flush
 *      Flush the tty buffer.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static void
term_flush(void)
{
    const int cols = ttcols(), rows = ttrows();
    const int garbled = vtisgarbled();

#if defined(HAVE_MOUSE)
    mouse_pointer(0);
#endif

    if (garbled) {                              // refresh entire screen
        VioShowBuf(0, rows * cols * sizeof(VIOCELL), 0);

    } else if (currDirtyTop >= 0) {             // dirty
        VIOSHOW show = {sizeof(VIOSHOW)};
        const int end = (currDirtyEnd < rows ? currDirtyEnd : rows - 1);
        int top, cnt = 0;

        VioShowInit(&show, 0);                  // show context initialisation.

        // update dirty line sections within dirty arena
        for (top = currDirtyTop, cnt = 0; top <= end; ++top) {
            if (top < VSET_MAX && 0 == vset_tst(currDirtyFlg, top)) {
                if (cnt) {                      // flush dirty columns.
                    const ULONG start   =
                            (ULONG) (((top - cnt) * cols) * sizeof(VIOCELL));
                    const ULONG length  =
                            (ULONG) ((cnt * cols) * sizeof(VIOCELL));
                    VioShowBlock(start, length, &show);
                    cnt = 0;
                }
            } else {
                ++cnt;                          // assume dirty
            }
        }

        if (cnt) {                              // flush remaining
            const ULONG start   =
                    (ULONG) (((top - cnt) * cols) * sizeof(VIOCELL));
            const ULONG length  =
                    (ULONG) ((cnt * cols) * sizeof(VIOCELL));
            VioShowBlock(start, length, &show);
        }

        VioShowFinal(&show);                    // completion
    }

    if (garbled || currDirtyTop >= 0) {         // reset dirty metadata
        vset_zero(currDirtyFlg);
        currDirtyTop = -1;
    }

    VioSetCurPos((unsigned short)ttatrow(), (unsigned short)ttatcol(), 0);

#if defined(HAVE_MOUSE)
    mouse_pointer(1);
#endif
}


/*  Function:           term_insl
 *      Insert 'nchunk' blank line(s) onto the screen, scrolling the last line(s) within
 *      the region off the bottom.
 *
 *  Parameters:
 *      row -               Top row of region.
 *      bot -               Bottom of region.
 *      nchunk -            Number of line to be inserted.
 *      fillcolor -         Fill attribute.
 *
 *  Returns:
 *      TRUE if the operation could be performed, otherwise FALSE.
 */
static int
term_insl(int row, int bot, int nchunk, vbyte_t fillcolor)
{
    const int cols = ttcols();

    term_attribute(VBYTE_ATTR_GET(fillcolor));
    {
        const VIOCELL blank = VIO_INIT(tt_hue, ' ');
        VIOCELL *p;

        while (nchunk-- > 0) {
            int i, rows = bot - row;

            p = currScreen + ((bot + 1) * ttcols()) - 1;
            while (rows-- > 0) {
                for (i = ttcols(); i--; --p) {
                    *p = *(p - cols);
                }
            }
            for (i = cols; i; --i) {
                *p-- = blank;
            }
        }
    }
    vio_dirty(row, 0, bot, cols-1);
    return TRUE;
}


/*  Function:           term_dell
 *      Delete nchunk line(s) from "row", replacing the bottom line on the screen with a blank line.
 *
 *  Parameters:
 *      row -               Top row of region.
 *      bot -               Bottom of region.
 *      nchunk -            Number of line to be deleted.
 *      fillcolor -         Fill attribute.
 *
 *  Returns:
 *      TRUE if the operation could be performed, otherwise FALSE.
 */
static int
term_dell(int row, int bot, int nchunk, vbyte_t fillcolor)
{
    if (fillcolor & VBYTE_ATTR_MASK) {          /* 0 == NULL */
        term_attribute(VBYTE_ATTR_GET(fillcolor));
    }
    {
        const VIOCELL blank = VIO_INIT(tt_hue, ' ');
        VIOCELL *p;

        while (nchunk-- > 0) {
            int i, rows = bot - row;

            p = currScreen + (row * ttcols());
            while (rows--) {
                for (i = ttcols(); i--; ++p) {
                    *p = *(p + ttcols());
                }
            }
            for (i = ttcols(); i; --i) {
                *p++ = blank;
            }
        }
    }
    vio_dirty(row, 0, bot, ttcols()-1);
    return TRUE;
}


/*  Function:           term_eeol
 *      Erase to end of line.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static int
term_eeol(void)
{
    const int atrow = ttatrow(), atcol = ttatcol();
    const VIOCELL blank = VIO_INIT(tt_hue, ' ');
    VIOCELL *p = currScreen + (atrow * ttcols()) + atcol;
    int n = ttcols() - atcol;

    vio_dirty(atrow, atcol, atrow, n);
    while (n-- > 0) {
        *p++ = blank;
    }
    return TRUE;
}


/*  Function:           term_repeat
 *      Function to repeat a character from the current location.
 *
 *      'where' says whether we want the cursor to stick at the starting
 *      point or stick at the end of the region. If set to 2 then this is a
 *      don't care -- i.e. leave cursor as it is.
 *
 *  Parameters:
 *      n -                 Character count.
 *      fill -              Fill character.
 *      where -             Where to leave the cursor (0=end, 1=start, 2=dontcare).
 *
 *  Returns:
 *      nothing
 */
static void
term_repeat(int cnt, vbyte_t fill, int where)
{
    if (cnt > 0) {
        const int row = ttatrow(), col = ttatcol();
        term_attribute(VBYTE_ATTR_GET(fill));
        {
            const VIOCELL blank = VIO_INIT(tt_hue, ' ');
            VIOCELL *p;

            p = currScreen + (row * ttcols()) + col;
            vio_dirty(row, col, row, col + cnt);
            if (WHERE_END == where) {
                ttmove(row, col + cnt);
            }
            while (cnt--) {
                *p++ = blank;
            }
        }
    }
}


/*  Function:           term_attribute
 *      Set the current writing color to the specified color.
 *
 *      The following function optimises based on the required color change and the
 *      current known state (if any).
 *
 *  Parameters:
 *      color -             Required color attribute.
 *
 *  Returns:
 *      nothing
 */
static void
term_attribute(const vbyte_t attr)
{
    assert(attr <= 512);
    if (vtiscolor()) {
        /*
         *  color terminal/
         *      either 16 or 256 (when available).
         */
        colattr_t ca;

        color_definition(attr, &ca);
        term_hue(term_colvalue(ca.fg, tt_defaultfg), term_colvalue(ca.bg, tt_defaultbg));
#if defined(VIO_UNDERLINE)
        if (COLORSTYLE_UNDERLINE & ca.sf)       tt_style |= VIO_UNDERLINE;
        if (COLORSTYLE_ITALIC & ca.sf)          tt_style |= VIO_ITALIC;
        if (COLORSTYLE_BOLD & ca.sf)            tt_style |= VIO_BOLD;
#endif

    } else {
        /*
         *  black and white coloriser/
         *      back-white display emulation, based on linux console.
         */
#define __NORMAL__      GREY,   BLACK
#define __BOLD__        CYAN,   BLACK
#define __STANDOUT__    BLACK,  WHITE
#define __REVERSE__     BLACK,  GREY
#define __INVERSE__     BLACK,  WHITE

#if defined(VIO_UNDERLINE)
        const int nstyle = ttbandw(attr, TRUE, TRUE, FALSE);
#else
        const int nstyle = ttbandw(attr, FALSE, FALSE, FALSE);
#endif

        tt_style = 0;
        if (0 == nstyle) {
            term_attr(__NORMAL__);
        } else {
            if (nstyle & COLORSTYLE_BOLD)      term_attr(__BOLD__);
            if (nstyle & COLORSTYLE_STANDOUT)  term_attr(__STANDOUT__);
            if (nstyle & COLORSTYLE_INVERSE)   term_attr(__INVERSE__);
#if defined(VIO_UNDERLINE)
            if (nstyle & COLORSTYLE_UNDERLINE) tt_style |= VIO_UNDERLINE;
            if (nstyle & COLORSTYLE_ITALIC)    tt_style |= VIO_ITALIC;
            if (nstyle & COLORSTYLE_BOLD)      tt_style |= VIO_BOLD;
#endif
            if (nstyle & COLORSTYLE_REVERSE)   term_attr(__REVERSE__);
        }

#undef  __NORMAL__
#undef  __BOLD__
#undef  __STANDOUT__
#undef  __REVERSE__
#undef  __INVERSE__
    }
}


/*import a color value*/
static unsigned
term_colvalue(const colvalue_t ca, unsigned def)
{
    int color = 0;

    if (COLORSOURCE_SYMBOLIC == ca.source) {
        if ((color = ca.color) < 0 || color >= COLOR_NONE ||
                (color = tt_colormap[color]) < 0 || color >= tt_colors) {
            color = def;
        }
    } else {
        if ((color = ca.color) < 0 || color >= tt_colors) {
            color = def;
        }
    }
    return (unsigned)color;
}


/*import a standard color value*/
static void
term_attr(int fg, int bg)
{
    if ((fg <= COLOR_NONE && (fg = tt_colormap[fg]) < 0) || fg >= tt_colors) {
        fg = tt_defaultfg;
    }
    if ((fg <= COLOR_NONE && (bg = tt_colormap[bg]) < 0) || bg >= tt_colors) {
        bg = tt_defaultbg;
    }
    term_hue(fg, bg);
}


/*assign the color/hue, guarding against fg and bg issues*/
static void
term_hue(int fg, int bg)
{
    if (fg == bg) {
        fg = tt_colormap[tt_defaultfg];
        bg = tt_colormap[tt_defaultbg];
        if (fg == bg) {
            fg = tt_colormap[WHITE];
            bg = tt_colormap[BLACK];
        }
    }
    tt_hue = VIO_FGBG(fg, bg);
    tt_style = 0;
}


/*  Function:           do_ega
 *      ega primitive, console mode switch support.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
do_ega(void)		    /* void (int flag) */
{
    static USHORT orows, ocols, mrows, mcols, state;

    acc_assign_int((accint_t) xf_ega43);

    if (isa_integer(1)) {                       /* (mode < 80 sets rows), otherwise (mode >= 80 sets columns). */
        const USHORT crows = (USHORT)currRows, ccols = (USHORT)currCols;
        const int flag = get_xinteger(1, 0);
        VIOMODEINFO mi = { sizeof(VIOMODEINFO) };

        if (flag < 0) {                         /* min/max toggle (-1), restore (-2) used on exit */
            if (state && (-1 == flag || -2 == flag)) {						
                if (crows == mrows && ccols == mcols) {
                    mi.row = orows;             /* restore, unless modified */
                    mi.col = ocols;
                }
            state = 0;
            } else if (0 == state && (-1 == flag || -3 == flag)) {
                orows  = crows;                 /* maximise */
                ocols  = ccols;
                mi.row = 0xffff;
                mi.col = 0xffff;
                state = 1;
            }
        } else {
            mi.row = (USHORT)crows;
            mi.col = (USHORT)ccols;
            if (flag >= 80) {                   /* setting columns */
                mi.col = (USHORT)flag;
            } else {
                mi.row = (USHORT)(flag > 10 ? flag : 10);
            }
        }

        if (mi.row && mi.col &&
                (crows != mi.row || ccols != mi.col)) {
            vio_restore();
            VioSetMode(&mi, 0);
            VioSetCurPos(origRow, origCol, 0);  /* restore cursor, mode changes home */
            ttwinched(mi.row, mi.col);
            if (flag < 0 && state) {
                mrows = mi.row;
                mcols = mi.col;
            }
        }
    }
}


/*  Function:           do_copy_screen
 *      copy_screen primitive, defunct
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
do_copy_screen(void)
{
    static const char *trim_str = " \t\r\n";
    BYTE *tmp = chk_alloc(ttcols() + 2);
    int ccol, crow, cofs;

    for (cofs = crow = 0; crow < ttrows(); ++crow) {
        for (ccol = 0; ccol < ttcols(); ++ccol, ++cofs) {
            tmp[ccol] = (BYTE) VIO_CHAR(origScreen[cofs]);
        }

        /* Strip off trailing white space */
        while (--ccol >= 0 && strchr(trim_str, (char) tmp[ccol])) {
            ;
        }

        /* Terminate and insert */
        tmp[++ccol] = '\n';
        tmp[++ccol] = '\0';
        linserts((const char *) tmp, ccol);
    }
    chk_free(tmp);
}
#endif  /*USE_VIO_BUFFER*/
