#include <edidentifier.h>
__CIDENT_RCSID(gr_ttyx11_c,"$Id: ttyx11.c,v 1.18 2025/07/02 16:52:04 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttyx11.c,v 1.18 2025/07/02 16:52:04 cvsuser Exp $ */
/*
 * Copyright (c) 2012 - 2025 Adam Young.
 * Copyright (c) 2009 Jeremy Cooper.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <editor.h>

#if defined(HAVE_LIBX11) && defined(HAVE_X11_XLIB_H)

#include <edtermio.h>
#include <edtrace.h>
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <edalt.h>

#include <sys/ioctl.h>
#include <sys/time.h>
#include <termios.h>
#include <errno.h>
#include <poll.h>

#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>

#define  Xscreen()      DefaultScreen(x_disp)

#include <libstr.h>                             /* str_...()/sxprintf() */

#include "color.h"
#include "debug.h"                              /* trace_...() */
#include "display.h"
#include "getkey.h"
#include "echo.h"
#include "cmap.h"
#include "main.h"
#include "tty.h"
#include "ttyx11if.h"                           /* Xlib dynamic binding */
#include "system.h"
#include "window.h"

#include "m_pty.h"

#define APP_CLASS       "Xgr"
#define APP_NAME        "Xgr"

#define ADVCLIPBOARD    1
#define BOOL            int

/*
 *  timers
 */

#define DELAY_UNIT      100                     /* timer delay, in milliseconds */
#define AUTOCLICK_FIRST 400                     /* first */
#define AUTOCLICK_NEXT  200                     /* reply */

/*
 *  Default window geometry.
 */

#define DEF_GEO_WIDTH   80
#define DEF_GEO_HEIGHT  25
#define BORDER_WIDTH    1

//  Description of an X11 color string update block as determined
//  by the screen update engine.
//

typedef struct {
#define XCHAR           XChar2b
    const XCHAR *   data;                       /* character buffers */
    vbyte_t         attr;                       /* attribute */
    int             len;                        /* number of characters */
    int             x;                          /* column */
    int             state;                      /* associated state */
#define UPD_NONE        0
#define UPD_UPDATE      1
#define UPD_REDUNDANT   2
} UpdateBlock;

static int              grXEventGet(struct IOEvent *event, int tmp);
static void             grXWriteRow(int dst, const struct _VCELL *src, int len, BOOL isExpose);
static Bool             grXNotifyEvent(Display *d, XEvent *e, XPointer arg);

static void             grXResourceLoad(const char *filename);
static void             grXFontLoad(void);
static void             grXFontSize(int direction);
static int              grXFontChange(const char *normal, const char *italic);
static XFontStruct *    grXLoadQueryFont(int type, int order, const char *fontspec, int size, char *name, int namelen);
static void             grXGCSetup(Display *disp, Window win, Font font, Font ifont);

static void             BuffersAlloc(int width, int height);
static void             BuffersFree(void);
static void             ResourceFree(void);

static void             functor_init(void);
static void             functor_close(void);
static void             functor_handler(void);


/*
 *  Driver status
 */

#define eventQSize      32

static int              maxViewWidth = 320;
static int              maxViewHeight = 120;

static Display *        x_disp;                 // display handle
static Window           x_win;                  // window handle
static int              x_dispfd;               // display file descriptor
static GC               x_cursorGC;
static GC               x_normalGC;
static GC               x_italicGC;

static int              x_colorDepth;           // color depth (16, 88 or 256)
static Colormap         x_colorMap;             // display color map
static XColor           x_colors[256];          // 0 ... 255 color table

static Region           x_region;               // current region
static XEvent           x_xEvent;               // last X event recieved

static int              x_functor_pipes[2];
static void           (*x_functor_collector)(void *) = NULL;

static int              x_evLength;             // number of events in the queue
static struct IOEvent * x_evIn = NULL;          // message queue system
static struct IOEvent * x_evOut = NULL;
static struct IOEvent   x_evQueue[eventQSize] = {0};

static int              x_msButtons;            // mouse button statusB
static struct IOEvent   x_msEvent;              // last mouse event
static Time             x_msLastRelease;
static int              x_msLastButtons;

static BOOL             x_cursor_state;         // is cursor visible?
static int              x_curx;                 // current cursor coordinates
static int              x_cury;
static int              x_dirty;

static int              tt_colors;              // export color depth
static int              tt_colormap[COLOR_NONE];
static int              tt_defaultbg = BLACK;
static int              tt_defaultfg = WHITE;

#if defined(ADVCLIPBOARD)                       // clipboard
static Atom             XA_CLIPBOARD;
static Atom             clipboardAtom;
static char *           clipboardBuffer;
static int              clipboardSize;
#endif

/*
 *  Driver resouces
 */

#define MAX_FONTNAMES   32

struct fontName {
    int             width;                      // display width/height
    int             height;
    int             ptsize;                     // point size
    const char *    name;
    const char *    italic;
};

static Font             x_fontID;
static Font             x_italicID;
static int              x_fontSize;
static char             x_fontName[120];
static char             x_fontItalicName[120];
static int              x_fontNumber;
static int              x_fontIndex;
static struct fontName  x_fontNames[MAX_FONTNAMES];
static int              x_fontAscent;
static int              x_fontDescent;
static int              x_fontHeight;
static int              x_fontWidth;

/*
 *  Keyboard translation lookup table routines.
 *
 *      These structures and functions implement a simple lookup table
 *      using a hashing scheme.
 */

#include "ttyx11kb.h"                           // keyboard translation tables

typedef struct KeyBucket {
    unsigned        noentries;
    KeyboardXlat *  entries;
} KeyBucket;

typedef struct KbXlat {
    uint32_t        magic;
    unsigned        nobuckets;
    unsigned        hash_key;
    KeyBucket *     buckets;
} KbXlat;

static KbXlat *         plainKeyTable;
static KbXlat *         shiftKeyTable;
static KbXlat *         ctrlKeyTable;
static KbXlat *         ctrlShiftKeyTable;
static KbXlat *         altKeyTable;

/*
 *  Color tables ...
 */

#include "ttyx11co.h"

typedef struct XResource {
    const char *    name;
    const char *    cls;
    const char **   value;
} XResource;

static const char *     xr_fontName;
static const char *     xr_fontItalicName;
static const char *     xr_fontPointSize;
static const char *     xr_geometry = NULL;

static XResource        XgrResources[] = {
#include "ttyx11rc.h"
    { "xgr.text.font",          "Xgr.Text.Font",        &xr_fontName        },
    { "xgr.text.pointsize",     "Xgr.Text.PointSize",   &xr_fontPointSize   },
    { "xgr.text.italicfont",    "Xgr.Text.ItalicFont",  &xr_fontItalicName  },
//  { "xgr.mouse.click",        "Xgr.Mouse.click",      &xr_mouse_click     },
//  { "xgr.mouse.dblclick",     "Xgr.Mouse.dblclick",   &xr_mouse_dblclick  },
//  { "xgr.icon.bitmap",        "Xgr.Icon.bitmap",      &xr_icon_bitmap     },
//  { "xgr.icon.pixmap",        "Xgr.Icon.pixmap",      &xr_icon_pixmap     },
//  { "xgr.bordercolor",        "Xgr.Bordercolor",      &xr_border_color    },
//  { "xgr.borderwidth",        "Xgr.Borderwidth",      &xr_border_width    },
    { "xgr.geometry",           "Xgr.Geometry",         &xr_geometry        },
    };
static const size_t     XgrResources_count = VSIZEOF(XgrResources);

enum { FT_USER, FT_FIXED, FT_ISO10646 };

static struct FontPair {
    int             type;
    const char *    name;
    const char *    italic;

} xr_fonts[] = {
    { FT_USER,          NULL, NULL },
    { FT_ISO10646,      "-*-monospace-bold-r-normal-",      "-*-monospace-bold-o-normal-" },        //DejaVuSansMono
    { FT_ISO10646,      "-adobe-courier-medium-r-normal-",  "-adobe-courier-medium-o-normal-" },
    { FT_ISO10646,      "-*-courier-medium-r-normal-",      "-*-courier-medium-o-normal-" },
    { FT_ISO10646,      "-misc-fixed-medium-r-normal-",     "-misc-fixed-medium-o-normal-" },
    { FT_FIXED,         "9x15" }
    };

static uint16_t         x_clickDelay = 200;     // click delay
static uint16_t         x_doubleDelay = 450;    // double click delay

static int              x_screenWidth;          // width in characters
static int              x_screenHeight;         // height in characters
static struct _VCELL *  x_screenBuffer;

static UpdateBlock *    x_update_chunks;        // update actions
static XCHAR *          x_update_buffer;        // update character data

/*
 *  Simple timer.
 */

typedef struct {
   uint64_t             value;
} IOTimer_t;

static uint64_t         x_iotimer;              // timer reference
static IOTimer_t        x_msTimer;              // time when generate next cmMouseAuto

static void             xgr_init(int *argcp, char **argv);
static void             xgr_open(scrprofile_t *profile);
static void             xgr_ready(int repaint, scrprofile_t *profile);
static void             xgr_display(void);
static void             xgr_feature(int ident, scrprofile_t *profile);
static int              xgr_control(int action, int param, ...);
static int              xgr_event(struct IOEvent *evt, accint_t tmo);
static void             xgr_close(void);

static int              xgr_cursor(int visible, int imode, int virtual_space);
static void             xgr_drawCursor(int show);
static void             xgr_move(int row, int col);

static void             xgr_clear(void);
static void             xgr_redraw(void);
static void             xgr_print(int row, int col, int len, const struct _VCELL *vvp);
static void             xgr_flush(void);
static int              xgr_names(const char *title, const char *icon);
static void             xgr_beep(int freq, int duration);
static int              xgr_font(int setlen, char *font);

static void             xgr_colors(void);
static int              xgr_color(const colvalue_t ca, int def);


/*
 *  Register the x11 tty driver.
 */
void
ttx11(void)
{
    XLibInitialise();

    x_scrfn.scr_init    = xgr_init;
    x_scrfn.scr_open    = xgr_open;
    x_scrfn.scr_ready   = xgr_ready;
    x_scrfn.scr_display = xgr_display;
    x_scrfn.scr_feature = xgr_feature;
    x_scrfn.scr_control = xgr_control;
    x_scrfn.scr_close   = xgr_close;

    x_scrfn.scr_winch   = NULL;
    x_scrfn.scr_cursor  = xgr_cursor;
    x_scrfn.scr_move    = xgr_move;
    x_scrfn.scr_event   = xgr_event;

    x_scrfn.scr_clear   = xgr_clear;
    x_scrfn.scr_print   = xgr_print;
    x_scrfn.scr_putc    = NULL;
    x_scrfn.scr_flush   = xgr_flush;

    x_scrfn.scr_insl    = NULL;
    x_scrfn.scr_dell    = NULL;
    x_scrfn.scr_eeol    = NULL;
    x_scrfn.scr_repeat  = NULL;

    x_scrfn.scr_names   = xgr_names;
    x_scrfn.scr_beep    = xgr_beep;
    x_scrfn.scr_font    = xgr_font;
}


void
ttx11_test(void)
{
    extern const char *key_code2name(int code);
    struct IOEvent evt = {0};

    for (;;) {
        if (0 == xgr_event(&evt, 1000)) {
            if (EVT_KEYDOWN == evt.type) {
                if ('x' == evt.code) {
                    break;
                }
            }
            printf("%s", key_code2name(evt.code));
        } else {
            printf("*");
        }
        fflush(stdout);
    }

    printf("x11: %d (%d,%d)\n", evt.type, evt.mouse.x, evt.mouse.y);
    fflush(stdout);
}


static void
xgr_open(scrprofile_t *profile)
{
    unsigned col;

printf("x11: open\n");
    x_display_ctrl |= DC_UNICODE;

    profile->sp_rows = x_screenHeight;
    profile->sp_cols = x_screenWidth;

    for (col = 0; col < (sizeof(tt_colormap)/sizeof(tt_colormap[0])); ++col) {
        tt_colormap[col] = -1;
    }
    xgr_colors();
}


static void
xgr_ready(int repaint, scrprofile_t *profile)
{
printf("x11: ready\n");
    xf_disptype = DISPTYPE_UTF8;
}


static void
xgr_display(void)
{
printf("x11: display\n");
    strxcpy(x_pt.pt_name, "curses-x11", sizeof(x_pt.pt_name));

    sxprintf(x_pt.pt_encoding, sizeof(x_pt.pt_encoding), "x11%s",
                (DISPTYPE_UTF8 == xf_disptype ? ".utf-8" : ""));

    grXGCSetup(x_disp, x_win, x_fontID, x_italicID);
}


static void
xgr_feature(int ident, scrprofile_t *profile)
{
    __CUNUSED(profile)
    switch (ident) {
    case TF_COLORDEPTH:
        break;

    case TF_COLORMAP: {
            /*
             * Example;
             *      "black,red3,green3,yellow3,blue3,magenta3,cyan3,\
             *          gray90,gray30,red,green,yellow,blue,magenta,cyan,white"
             */
        }
//printf("x11: TF_COLORMAP\n");
        break;

    case TF_COLORPALETTE: {                     /* user defined palette */
            /*
             * Example;
             *      "0,   4,   2,   6,   1,   5, 130, 7,  8,  12,  10,   14,
             *       9,  13,  11,  15,   0,   4, 2,   6,  1,  5,   130,  11"
             */
            const char *cursor = x_pt.pt_colorpalette;
            unsigned col = 0;

            for (col = 0; col < (sizeof(tt_colormap)/sizeof(tt_colormap[0])); ++col) {
                if (cursor && *cursor) {
                    if (isdigit(*cursor)) {
                        int val;

                        if ((val = atoi(cursor)) >= 0 && val <= 255) {
                            tt_colormap[col] = val;
                        }
                    }

                    if (NULL != (cursor = strchr(cursor, ','))) {
                        ++cursor;
                    }
                }
            }
        }
//printf("x11: TF_COLORPALETTE\n");
        break;

    case TF_DEFAULT_FG:
    case TF_DEFAULT_BG:
        if (x_pt.pt_defaultfg >= 0 && x_pt.pt_defaultbg >= 0) {
            tt_defaultfg = x_pt.pt_defaultfg;
            tt_defaultbg = x_pt.pt_defaultfg;
        }
//printf("x11: TF_DEFAULT_XX\n");
        break;

    case TF_ENCODING:
//printf("x11: TF_ENCODING\n");
        break;
    }
}


static int
xgr_control(int action, int param, ...)
{
    switch (action) {
    case SCR_CTRL_NORMAL:
//printf("x11: CTRL_NORMAL\n");
        break;
    case SCR_CTRL_GARBLED:
//printf("x11: CTRL_GARBLED\n");
        ++x_dirty;
        break;
    case SCR_CTRL_SAVE:
//printf("x11: CTRL_SAVE\n");
        if (param) {
        } else {
            xgr_redraw();
        }
        break;
    case SCR_CTRL_RESTORE:
//printf("x11: CTRL_RESTORE\n");
        break;
    case SCR_CTRL_COLORS:
//printf("x11: CTRL_COLORS\n");
        color_valueclr(-1);
        break;
    default:
//printf("x11: CTRL_UNKNOWN(%d)\n", action);
        return -1;
    }
    return 0;
}


static void
xgr_clear(void)
{
    const int rows = x_screenHeight, cols = x_screenWidth;
    const vbyte_t space = (vbyte_t)(VBYTE_CHAR(' ') | VBYTE_ATTR(ATTR_NORMAL));
    int row, col;

    XClearWindow(x_disp, x_win);
    for (row = 0; row < rows; ++row) {
        struct _VCELL *vvp = x_screenBuffer + (row * cols);
        for (col = 0; row > cols; ++col, ++vvp) {
            vvp->primary = space;
        }
        grXWriteRow(row * cols, x_screenBuffer + (row * cols), cols, TRUE);
    }
}


static void
xgr_redraw(void)
{
    const int rows = x_screenHeight, cols = x_screenWidth;
    int row;

    for (row = 0; row < rows; ++row) {
        grXWriteRow(row * cols, x_screenBuffer + (row * cols), cols, TRUE);
    }
}


static void
xgr_print(int row, int col, int len, const struct _VCELL *vvp)
{
    const int rows = x_screenHeight, cols = x_screenWidth;

    assert(row >= 0);
    assert(row < rows);
    assert(col >= 0);
    assert(col < cols);
    assert(len > 0);
    assert(len <= x_screenWidth);
    assert(col + len <= x_screenWidth);

    grXWriteRow((row * cols) + col, vvp, len, FALSE);
}


static void
xgr_flush(void)
{
    XFlush(x_disp);
}


static int
xgr_names(const char *title, const char *icon)
{
    __CUNUSED(title)
    __CUNUSED(icon)
    return -1;
}


/*
 *  Timers
 */

static void
iot_current(void)
    {
        int ms = 0;
        time_t now = sys_time(&ms);
        x_iotimer = ((uint64_t)now * 1000) + ms;
    }


static void
iot_start(IOTimer_t *tmr, int tmo)
    {
        if (tmo > 0) tmr->value = x_iotimer + tmo;
        else tmr->value = 0;
    }


static void
iot_starts(IOTimer_t *tmr, int tmo)
    {
        if (tmo > 0) tmr->value = x_iotimer + (tmo * 1000);
        else tmr->value = 0;
    }


static void
iot_stop(IOTimer_t *tmr)
    { tmr->value = 0; }


static int
iot_expired(const IOTimer_t *tmr)
    { return (tmr->value && x_iotimer >= tmr->value); }


static int
iot_active(const IOTimer_t *tmr)
    { return (tmr->value ? 1 : 0); }


static int
iot_remaining(const IOTimer_t *tmr)
    {
        const uint64_t value = tmr->value;

        if (value > 0) {
            if (x_iotimer >= value) {           /* expired */
                return 0;
            }
            return (value - x_iotimer);
        }
        return -1;
    }

/*
 *  X DRAWING FUNCTIONS
 */
static void
handleXExpose(const XEvent *event)
{
    XRectangle r;

    r.x = event->xexpose.x;
    r.y = event->xexpose.y;
    r.width = event->xexpose.width;
    r.height = event->xexpose.height;

    XUnionRectWithRegion(&r, x_region, x_region);

    if (0 == event->xexpose.count) {
        const int cols = x_screenWidth;
        int row, res;

        for (row = 0; row < x_screenHeight; ++row) {
            /*
             *  First, calculate if anything in this entire row is exposed.
             */
            r.x      = 0;
            r.y      = row * x_fontHeight;
            r.width  = x_screenWidth * x_fontWidth;
            r.height = x_fontHeight;

            res = XRectInRegion(x_region, 0,
                        row * x_fontHeight, x_fontWidth * x_screenWidth, x_fontHeight);

            /*
             *  Something in this row is exposed, just draw the whole row.
             */
            if (res != RectangleOut) {
                grXWriteRow(row * cols, x_screenBuffer + (row * cols), cols, TRUE);
            }
        }

        XDestroyRegion(x_region);
        x_region = XCreateRegion();
    }
}


static void
drawTextCursor(void)
{
    XFillRectangle(x_disp, x_win, x_cursorGC,
            x_curx * x_fontWidth, x_cury * x_fontHeight, x_fontWidth, x_fontHeight);
}


static void
eraseTextCursor(void)
{
    XFillRectangle(x_disp, x_win, x_cursorGC,
            x_curx * x_fontWidth, x_cury * x_fontHeight, x_fontWidth, x_fontHeight);
}


/*
 *  Translate keyboard modifiers.
 */
static __CINLINE unsigned
translateXModifier(int state)
{
    unsigned modifier = 0;

    if (state & ShiftMask)
        modifier |= MOD_SHIFT;
    if (state & ControlMask)
        modifier |= MOD_CTRL;
    if (state & Mod1Mask)
        modifier |= MOD_META;
    return modifier;
}


/*
 *  Translate an X button number into a button number.
 */
static __CINLINE int
translateXButton(int button, int type, int isDouble)
{
    switch (button) {
    case Button1:
        switch (type) {
        case ButtonPress:
            return (isDouble ? BUTTON1_DOUBLE : BUTTON1_DOWN);
        case ButtonRelease:
            return BUTTON1_UP;
        case MotionNotify:
            return BUTTON1_MOTION;
        }
        break;

    case Button2:
        switch (type) {
        case ButtonPress:
            return (isDouble ? BUTTON2_DOUBLE : BUTTON2_DOWN);
        case ButtonRelease:
            return BUTTON2_UP;
        case MotionNotify:
            return BUTTON2_MOTION;
        }
        break;

    case Button3:
        switch (type) {
        case ButtonPress:
            return (isDouble ? BUTTON3_DOUBLE : BUTTON3_DOWN);
        case ButtonRelease:
            return BUTTON3_UP;
        case MotionNotify:
            return BUTTON3_MOTION;
        }
        break;

    default:
        break;
    }
    return 0;
}


static void
error(const char *fmt, ...)
{
    const int xerrno = errno;
    va_list ap;

    va_start(ap, fmt);
    printf("xgr: ");
    vprintf(fmt, ap);
    va_end(ap);
    exit(3);
}


// Create a new hash-based keyboard key lookup table.
static KbXlat *
KbXlat_new(int nobuckets, int hash_key)
{
    KbXlat *h;
    unsigned i;

    assert(nobuckets > 0);

    h = (KbXlat *) chk_alloc(sizeof(KbXlat));

    h->nobuckets = nobuckets;
    h->hash_key  = hash_key;
    h->buckets   = (KeyBucket *) chk_alloc(sizeof(KeyBucket) * nobuckets);

    for (i = 0; i < nobuckets; ++i) {
        h->buckets[i].noentries = 0;
        h->buckets[i].entries = NULL;
    }

    h->magic = 0xdeadbeef;
    return h;
}


// Add an entry to a hash-based keyboard key lookup table.
static int
KbXlat_add(KbXlat *h, int key, int result)
{
    unsigned i;

    assert(h->magic == 0xdeadbeef);

    i = ((unsigned)key * h->hash_key) % h->nobuckets;

    h->buckets[i].noentries++;
    h->buckets[i].entries = (KeyboardXlat *)
        chk_realloc(h->buckets[i].entries, h->buckets[i].noentries * sizeof(KeyboardXlat));

    h->buckets[i].entries[h->buckets[i].noentries-1].xcode = key;
    h->buckets[i].entries[h->buckets[i].noentries-1].ocode = result;
}


static void
KbXlat_free(KbXlat *h)
{
    unsigned i, j;

    assert(h->magic == 0xdeadbeef);

    for (i = 0; i < h->nobuckets; ++i) {
        chk_free(h->buckets[i].entries);
    }
    h->magic = 0;
    chk_free(h->buckets);
    chk_free(h);
}


static int
KbXlat_lookup(const KbXlat *h, int xcode)
{
    unsigned i, j;

    assert(h->magic == 0xdeadbeef);

    i = (xcode * h->hash_key) % h->nobuckets;

    for (j = 0; j < h->buckets[i].noentries; ++j) {
        if (xcode == h->buckets[i].entries[j].xcode) {
            return h->buckets[i].entries[j].ocode;
        }
    }
    return -1;
}


static KbXlat *
createKbXlat(int hash_key, int nobuckets, const KeyboardXlat *seed, int seed_count)
{
    KbXlat *h;
    unsigned i;

    h = KbXlat_new(hash_key, nobuckets);

    for (i = 0; i < seed_count; ++i) {
        KbXlat_add(h, seed[i].xcode, seed[i].ocode);
    }
    return h;
}


static void
setupKeyTables(void)
{
    plainKeyTable = createKbXlat(17, 64, plainXlatSeed, VSIZEOF(plainXlatSeed));
    shiftKeyTable = createKbXlat(17, 16, shiftXlatSeed, VSIZEOF(shiftXlatSeed));
    ctrlKeyTable  = createKbXlat(17, 64, ctrlXlatSeed,  VSIZEOF(ctrlXlatSeed));
    ctrlShiftKeyTable = createKbXlat(17, 16, ctrlShiftXlatSeed, VSIZEOF(ctrlShiftXlatSeed));
    altKeyTable   = createKbXlat(17, 64, altXlatSeed,   VSIZEOF(altXlatSeed));
}


static __CINLINE int
range(int test, int min, int max)
{
    return (test < min ? min : test > max ? max : test);
}


/*
 *  Reads a key from the keyboard.
 */
static int
grXEventGet(struct IOEvent *event, int tmo)
{
    BOOL xFdReady = FALSE;
    int noevents;

    event->type = EVT_NONE;

    /*
     *  suspend until there is a signal or some data in file descriptors
     */
    while (0 == XEventsQueued(x_disp, QueuedAfterFlush) && !xFdReady) {

        int i, pollCount = 0;
        struct pollfd *polls = io_device_pollfds(&pollCount);

        // Await events
        //
        int res = poll(polls, pollCount, tmo);
        if (0 == res) {
            return -1;                          // timeout
        }
        if (-1 == res) {
            if (EINTR == errno) {
                continue;
            }
            error("poll: %s\n", strerror(errno));
        }

        //  Examine all returned descriptors for results.
        //
        for (i = 0; i < pollCount; ++i) {
            //
            //  Did this descriptor return an event?
            //
            if (polls[i].revents) {
                //
                //  Descriptor returned an event.  Are there listeners for it?
                //  (There ought to be if it was in the pollList!)
                //
                //  Was it the X display file descriptor?
                //
                if (polls[i].fd == x_dispfd) {
                    xFdReady = TRUE;            // X event

                } else if (polls[i].fd == x_functor_pipes[0]) {
                    functor_handler();          // async notification

                } else {
                    pty_poll();                 // connected pty's
                }
            }
        }
    }

    /*
     *  Read all of the X events in the X queue until we encounter one we
     *  can deliver to the application, or run out of events.
     */
    noevents = XEventsQueued(x_disp, QueuedAfterFlush);

    for (;noevents > 0; --noevents) {
        XNextEvent(x_disp, &x_xEvent);

        switch (x_xEvent.type) {
        case Expose:
            /*
             *  X Expose event, part (or all) of a window needs to be redrawn.
             */
            handleXExpose(&x_xEvent);
            break;

        case ButtonPress: {
                /*
                 *  X ButtonPress event.
                 *
                 *      X sends a ButtonPress event whenever the user presses a button
                 *      on the pointing device (mouse).
                 */
                const int xbutton = x_xEvent.xbutton.button;
                unsigned modifiers;             // keyboard modifiers
                int button;                     // mouse button

//printf("x11: .. ButtonPress (%d)\n", x_xEvent.xbutton.button);

                if (4 == xbutton || 5 == xbutton) {
                    event->type = EVT_KEYDOWN;  // wheel event
                    event->code = (4 == xbutton ? WHEEL_UP : WHEEL_DOWN);
                    return 0;
                }

                if (0 >= (button =
                        translateXButton(xbutton, ButtonPress, FALSE))) {
                    break;                      // ignore, unsupported
                }

                modifiers = translateXModifier(x_xEvent.xbutton.state);

                event->type = EVT_MOUSE;
                event->code = button;
                event->modifiers = modifiers;
                event->mouse.x = x_xEvent.xbutton.x / x_fontWidth;
                event->mouse.y = x_xEvent.xbutton.y / x_fontHeight;

                if (button == x_msLastButtons &&
                        event->mouse.where == x_msEvent.mouse.where &&
                        x_msLastRelease != 0 &&
                            (x_xEvent.xbutton.time - x_msLastRelease) < x_doubleDelay) {
                    button = translateXButton(x_xEvent.xbutton.button, ButtonPress, TRUE);
                    x_msLastRelease = 0;
                }
                                                // start the autoclick timer.
                iot_start(&x_msTimer, AUTOCLICK_FIRST);

                x_msEvent = *event;
            }
            return 0;

        case ButtonRelease: {
                unsigned modifiers;             // keyboard modifiers
                int button;                     // mouse buttons

//printf("x11: .. ButtonRelease (%d)\n", x_xEvent.xbutton.button);

                if (0 >= (button =
                        translateXButton(x_xEvent.xbutton.button, ButtonRelease, FALSE))) {
                    break;                      // ignore, unsupported
                }

                modifiers = translateXModifier(x_xEvent.xbutton.state);

                event->type = EVT_MOUSE;
                event->code = button;
                event->modifiers = modifiers;
                event->mouse.x = x_xEvent.xbutton.x / x_fontWidth;
                event->mouse.y = x_xEvent.xbutton.y / x_fontHeight;

                x_msLastRelease = x_xEvent.xbutton.time;
                x_msLastButtons = button;

                iot_stop(&x_msTimer);           // stop auto-click timer

                x_msEvent = *event;
            }
            return 0;

        case MotionNotify: {
                //  X reports mouse movements at every pixel position, whereas we
                //  only care if the mouse changes character positions.
                //  Calculate the character position for the event and see if it
                //  has changed from the last event.
                //
                unsigned modifiers;             // keyboard modifiers
                int button,                     // mouse buttons
                    x, y;                       // mouse position

                modifiers = translateXModifier(x_xEvent.xkey.state);

                if (0 >= (button =
                        translateXButton(x_xEvent.xbutton.button, MotionNotify, FALSE))) {
                    switch (x_xEvent.xbutton.button) {
                    case Button4:
                        event->type = EVT_KEYDOWN;
                        event->code = WHEEL_UP;
                        event->modifiers = modifiers;
                        return 0;
                    case Button5:
                        event->type = EVT_KEYDOWN;
                        event->code = WHEEL_DOWN;
                        event->modifiers = modifiers;
                        return 0;
                    }
                    break;                      // ignore, unsupported
                }

                x = x_xEvent.xbutton.x / x_fontWidth;
                y = x_xEvent.xbutton.y / x_fontHeight;

                if (x_msEvent.mouse.x != x || x_msEvent.mouse.y != y) {
                    event->type = EVT_MOUSE;
                    event->code = button;
                    event->modifiers = modifiers;
                    event->mouse.x = x;
                    event->mouse.y = y;
                    x_msEvent = *event;
                    return 0;
                }
            }
//printf("x11: .. Motion\n");
            break;

        case KeyPress: {
                KeySym xcode = 0;               // Key symbol identifier
                unsigned modifiers;             // keyboard modifiers
                int mcode,                      // modified internal key-code
                    code;                       // internal key-code

                XLookupString(&x_xEvent.xkey, NULL, 0, &xcode, NULL);
                if (xcode == NoSymbol) {
                    break;
                }

                modifiers = translateXModifier(x_xEvent.xkey.state);

                if ((code = KbXlat_lookup(plainKeyTable, xcode)) > 0) {
                    if (code == KEY_VOID) {
                        //
                        //  Explicitly ignored. see key-tables for details.
                        //
                        break;
                    }
                } else {
                    //
                    //  Most key symbols in X are direct ASCII equivalents.  Since we there
                    //  wasn't a plain mapping, just use the X key symbol directly, as it is
                    //  probably plain ASCII.
                    //
                    if (xcode < 0x100) {
                        //
                        //  The character is a plain ASCII or extended character.
                        //  No translation needed.
                        //
                        code = xcode;
                    } else {
                        //
                        //  The character is outside the ASCII or extended set.  It slipped
                        //  through our translation routines and thus, is probably incorrectly
                        //  translated.
                        //
//printf("x11: .. KeyPress [%d/0x%x] !!\n", xcode, xcode);
                        break;
                    }
                }

                if ((MOD_META & modifiers) &&
                        (mcode = KbXlat_lookup(altKeyTable, xcode)) > 0) {
                    code = mcode;               // Alt-<key>

                } else if (MOD_SHIFT & modifiers) {
                    if ((MOD_CTRL & modifiers) &&
                            (mcode = KbXlat_lookup(ctrlShiftKeyTable, xcode)) > 0) {
                        code = mcode;           // Ctrl-Shift-<key>

                    } else if ((mcode = KbXlat_lookup(shiftKeyTable, xcode)) > 0) {
                        code = mcode;           // Shift-<key>

                    }

                } else if (MOD_CTRL & modifiers) {
                    if ((mcode = KbXlat_lookup(ctrlKeyTable, xcode)) > 0) {
                        if (__CTRL('+') == mcode /*|| __CTRL('=') == mcode*/) {
                            grXFontSize(1);
                            break;
                        } else if (__CTRL('-') == mcode) {
                            grXFontSize(0);
                            break;
                        }
                        code = mcode;           // Ctrl-<key>
                    }
                }

                event->type = EVT_KEYDOWN;
                event->code = code;
                event->modifiers = modifiers;

//printf("x11: .. KeyPress [%d/0x%x] (%d/'%c') %s\n", xcode, xcode,
//        code, (code >= ' ' && code < 0x7f ? code : '?'), key_code2name(code));
            }
            return 0;

        case KeyRelease:
            break;

        case MappingNotify:
            XRefreshKeyboardMapping(&x_xEvent.xmapping);
            break;

#if defined(ADVCLIPBOARD)
        case SelectionClear:
            //
            // Another application has posted something to the clipboard.
            // We are no longer the clipboard source.
            // There's really nothing to do.
            //
            break;

        case SelectionRequest: {
                //
                // We're in charge of the clipboard and another application has requested
                // its contents.
                //
                XEvent reply = {0};

                trace_log("x11: got selection request.\n");

                //
                // We must have something on the internal clipboard and the requestor
                // must ask for the data as a string.
                //
                if (clipboardSize > 0 && x_xEvent.xselectionrequest.target == XA_STRING) {
                    //
                    // The requestor has asked for the data as a string.
                    // Send it the data by attaching it to the specified property on the
                    // specified window.
                    //
                    XChangeProperty(
                        x_disp,
                        x_xEvent.xselectionrequest.requestor,
                        x_xEvent.xselectionrequest.property,
                        XA_STRING,
                        8,                      // Format.  8 = 8-bit character array
                        PropModeReplace,
                        (unsigned char *) clipboardBuffer,
                        clipboardSize);

                    //
                    // Set up the reply to indicate that the transfer succeeded.
                    //
                    reply.xselection.property = x_xEvent.xselectionrequest.property;
                } else {
                    //
                    // Couldn't satisfy the request.
                    // Set up the reply to indicate that the transfer failed.
                    //
                    reply.xselection.property = None;
                }

                //
                // Notify the requestor that the transfer has completed.
                //
                reply.type = SelectionNotify;
                reply.xselection.display = x_disp;
                reply.xselection.requestor = x_xEvent.xselectionrequest.requestor;
                reply.xselection.selection = x_xEvent.xselectionrequest.selection;
                reply.xselection.target = x_xEvent.xselectionrequest.target;
                reply.xselection.time = x_xEvent.xselectionrequest.time;
                XSendEvent(x_disp, x_xEvent.xselectionrequest.requestor, False, 0, &reply);
            }
            break;
#endif //ADVCLIPBOARD

        case ConfigureNotify: {
                int width, height;

                width = range(x_xEvent.xconfigure.width / x_fontWidth, 4, maxViewWidth);
                height = range(x_xEvent.xconfigure.height / x_fontHeight, 4, maxViewHeight);

                if (x_screenWidth != width || x_screenHeight != height) {
                    trace_log("x11: screen resized to %dx%d\n", width, height);

                    xgr_drawCursor(0);          /* hide the cursor */
                    BuffersAlloc(width, height);
                    ttwinched(height, width);   /* enact screen change */
                }
            }
            break;
        }
    }

    /* see if there is data available */
    return 1;
}


static void
BuffersAlloc(int width, int height)
{
    const vbyte_t space = (vbyte_t)(VBYTE_CHAR(' ') | VBYTE_ATTR(ATTR_NORMAL));
    int cell, cells = width * height;
    struct _VCELL *vvp;

    BuffersFree();

    x_screenBuffer = chk_alloc(sizeof(struct _VCELL) * cells);
    for (cell = 0, vvp = x_screenBuffer; cell < cells; ++cell) {
        vvp->primary = space;
        ++vvp;
    }

    x_update_chunks = chk_alloc(sizeof(UpdateBlock) * width * 2);
    x_update_buffer = chk_alloc(sizeof(XCHAR) * width);

    x_screenWidth   = width;
    x_screenHeight  = height;
}


static void
BuffersFree(void)
{
    if (x_screenBuffer) {
        chk_free(x_update_chunks), x_update_chunks = NULL;
        chk_free(x_update_buffer), x_update_buffer = NULL;
        chk_free(x_screenBuffer), x_screenBuffer = NULL;
    }
}


static void
ResourceFree(void)
{
#if defined(ADVCLIPBOARD)
    chk_free(clipboardBuffer), clipboardBuffer = NULL;
#endif

    KbXlat_free(plainKeyTable), plainKeyTable = NULL;
    KbXlat_free(shiftKeyTable), shiftKeyTable = NULL;
    KbXlat_free(ctrlKeyTable), ctrlKeyTable = NULL;
    KbXlat_free(ctrlShiftKeyTable), ctrlShiftKeyTable = NULL;
    KbXlat_free(altKeyTable), altKeyTable = NULL;

    BuffersFree();

    functor_close();
}


/*
 *  Constructor.
 */
static void
xgr_init(int *argc, char **argv)
{
    int xdepth;
    const char *p;

    if (NULL == (p = ggetenv("DISPLAY"))) {
        error("No DISPLAY defined.\n");
    }

    /*
     *  Connect to X server.
     */
    if (NULL == (x_disp = XOpenDisplay(p))) {
        error("could not open display '%s'.\n", p);
    }

    x_colorDepth = DefaultDepth(x_disp, Xscreen());
    x_colorMap = DefaultColormap(x_disp, Xscreen());

    /*
     *  Force 24BIT .. if available
     */
    if (x_colorDepth < 24) {
        XVisualInfo vinfo = {0};

        if (XMatchVisualInfo(x_disp, Xscreen(), 24, TrueColor, &vinfo)) {
            trace_log("x11: TrueColor\n");
            x_colorMap = XCreateColormap(x_disp, RootWindow(x_disp, Xscreen()), vinfo.visual, AllocNone);
            x_colorDepth = 24;
        }
        XSetWindowColormap(x_disp, x_win, x_colorMap);
    }

    trace_log("x11: xdepth=%d\n", x_colorDepth);

#if defined(ADVCLIPBOARD)
    /*
     *  "Intern" an atom that we can use as a target identifier when
     *  asking X for the clipboard contents.
     */
    clipboardAtom = XInternAtom(x_disp, "XCR_CLIPBOARD", False);
    if (clipboardAtom == None)
        error("could not intern clipboard atom.\n");
    XA_CLIPBOARD = XInternAtom(x_disp, "CLIPBOARD", False);
    if (clipboardAtom == None)
        error("could not intern clipboard selection atom.\n");
#endif

    /*
     *  Resource
     */
    if (NULL != (p = ggetenv("HOME"))) {
        char rdbFilename[PATH_MAX];
        snprintf(rdbFilename, sizeof(rdbFilename), "%s/.Xdefaults", p);
        grXResourceLoad(rdbFilename);
    }

    grXFontLoad();

    /*
     * Negotiate window size with user settings and our defaults.
     */
    XSizeHints *sh;

    if (NULL == (sh = XAllocSizeHints())) {
        error("alloc");
    }

    sh->min_width  = 5 * x_fontWidth;
    sh->max_width  = maxViewWidth  * x_fontWidth;
    sh->min_height = 5 * x_fontHeight;
    sh->max_height = maxViewHeight * x_fontHeight;
    sh->width_inc  = x_fontWidth;
    sh->height_inc = x_fontHeight;
    sh->flags      = (PSize | PMinSize | PResizeInc);

    char default_geometry[80];

    snprintf(default_geometry, sizeof(default_geometry),
                "%dx%d+%d+%d", DEF_GEO_WIDTH, DEF_GEO_HEIGHT, 0, 0);

    int geometryFlags =
            XGeometry(x_disp, Xscreen(), xr_geometry, default_geometry,
                BORDER_WIDTH, x_fontWidth, x_fontHeight, 0, 0, &sh->x, &sh->y, &sh->width, &sh->height);

    if (geometryFlags & (XValue | YValue))
        sh->flags |= USPosition;
    if (geometryFlags & (WidthValue | HeightValue))
        sh->flags |= USSize;

    sh->width  *= x_fontWidth;
    sh->height *= x_fontHeight;

    /*
     * Create our window.
     */
    if (24 == x_colorDepth) {
        XSetWindowAttributes attributes = {0};

        attributes.background_pixel = BlackPixel(x_disp, DefaultScreen(x_disp));
        attributes.border_pixel = WhitePixel(x_disp, DefaultScreen(x_disp));
        attributes.colormap = x_colorMap;
        x_win = XCreateWindow(x_disp,
                        DefaultRootWindow(x_disp),
                        sh->x, sh->y, sh->width, sh->height,
                        BORDER_WIDTH,
                        24,
                        InputOutput,
                        DefaultVisual(x_disp, Xscreen()),
                        CWBackPixel | CWBorderPixel | CWColormap, &attributes);

    } else {
        x_win = XCreateSimpleWindow(x_disp,
                        DefaultRootWindow(x_disp),
                        sh->x, sh->y, sh->width, sh->height, BORDER_WIDTH,
                        BlackPixel(x_disp, Xscreen()),
                        WhitePixel(x_disp, Xscreen()));
    }

    /*
     * Setup class hint for the window manager.
     */
    XClassHint *clh;

    if (NULL == (clh = XAllocClassHint())) {
        error("alloc");
    }

    clh->res_name = (char *)APP_NAME;
    clh->res_class = (char *)APP_CLASS;

    /*
     * Setup hints to the window manager.
     */
    XWMHints *wmh;

    if (NULL == (wmh = XAllocWMHints())) {
        error("alloc");
    }

    wmh->flags = (InputHint | StateHint);
    wmh->input = True;
    wmh->initial_state = NormalState;

    XTextProperty WName, IName;
    const char *app_name = APP_NAME;

    if (XStringListToTextProperty((char **)&app_name, 1, &WName) == 0) {
        error("error creating text property");
    }
    if (XStringListToTextProperty((char **)&app_name, 1, &IName) == 0) {
        error("error creating text property");
    }

    /*
     * Send all of these hints to the X server.
     */
    XSetWMProperties(x_disp, x_win, &WName, &IName, argv, *argc, sh, wmh, clh);

    grXGCSetup(x_disp, x_win, x_fontID, x_italicID);

    /*
     * Set the window's bit gravity so that resize events keep the
     * upper left corner contents.
     */
    XSetWindowAttributes wa;

    wa.bit_gravity   = NorthWestGravity;
    wa.backing_store = WhenMapped;
    wa.event_mask    = ExposureMask|KeyPressMask|ButtonPressMask|
                            ButtonReleaseMask|ButtonMotionMask|
                            KeymapStateMask|ExposureMask|
                            StructureNotifyMask;

    XChangeWindowAttributes(x_disp, x_win, CWBitGravity|CWBackingStore|CWEventMask, &wa);

    x_screenWidth   = sh->width / x_fontWidth;
    x_screenHeight  = sh->height / x_fontHeight;
    x_region        = XCreateRegion();

    XFree(wmh);
    XFree(clh);
    XFree(sh);

    /*
     * Cause the window to be displayed.
     */
    XMapWindow(x_disp, x_win);

    trace_log("x11: screen size is %dx%d\n", x_screenWidth, x_screenHeight);

    BuffersAlloc(x_screenWidth, x_screenHeight);

#if defined(ADVCLIPBOARD)
    clipboardBuffer = NULL;
    clipboardSize = 0;
#endif

    /* internal stuff */

    /* Setup keyboard translations */
    setupKeyTables();

    /* setup file descriptors */
    x_dispfd = ConnectionNumber(x_disp);

    // Setup a pipe for syncronization objects
    functor_init();

    // Set up a polling entry for the X display file descriptor.
    io_device_add(x_dispfd /*POLLIN*/);

    // Set up a polling entry for the syncronization pipe
    io_device_add(x_functor_pipes[0] /*POLLIN*/);

    // Event resources
    x_curx = x_cury = 0;
    x_cursor_state = TRUE;
    x_evIn = x_evOut = &x_evQueue[0];
    x_evLength = 0;
    x_msButtons = 0;

    iot_current();
    iot_stop(&x_msTimer);

printf("x11: screen size is %dx%d\n", x_screenWidth, x_screenHeight);
}


/*
 *  Load X resource overrides from the specified file.
 */
static void
grXResourceLoad(const char *filename)
{
    XrmDatabase xdb;
    XrmValue value;
    char *type;
    unsigned i;
    int res;

    XrmInitialize();

    if (NULL == (xdb = XrmGetFileDatabase(filename))) {
        trace_log("x11: cannot open Xdefaults file '%s'.\n", filename);
        return;
    }

    for (i = 0; i < XgrResources_count; ++i) {
        res = XrmGetResource(xdb, XgrResources[i].name, XgrResources[i].cls, &type, &value);
        if (res) {
            *XgrResources[i].value = value.addr;
        }
    }
}


/*
 *  Load X fonts
 */
static int
xgrFontNameCmp(const void *a, const void *b)
{
    const struct fontName *fna = (const struct fontName *)a,
            *fnb = (const struct fontName *)b;

    if (fna->width == fnb->width) {
        if (fna->height == fnb->height) {
            if (fna->ptsize == fnb->ptsize) {
                return str_icmp(fna->name, fnb->name);
            }
            return (fna->ptsize - fnb->ptsize);
        }
        return (fna->height - fnb->height);
    }
    return (fna->width - fnb->width);
}


static void
crxCFontDump(XFontStruct *fs, const char *normal, const char *italic)
{
    unsigned long value;

    printf("x11: font <%s> <%s>\n", normal, (italic ? italic : ""));
    if (XGetFontProperty(fs, XA_FONT_NAME, &value))             printf("  Name: %s,", (char *) &value);
    if (XGetFontProperty(fs, XA_FAMILY_NAME, &value))           printf("  Family: %s,", XGetAtomName(x_disp, (Atom)value));
    if (XGetFontProperty(fs, XA_POINT_SIZE, &value))            printf("  Point Size: %ld,", value);
    if (XGetFontProperty(fs, XA_RESOLUTION, &value))            printf("  Resolution: %ld,", value);
    if (XGetFontProperty(fs, XA_FULL_NAME, &value))             printf("  FullName: %s,", XGetAtomName(x_disp, (Atom)value));
    if (XGetFontProperty(fs, XA_WEIGHT, &value))                printf("  Weight: %ld,", value);
    if (XGetFontProperty(fs, XA_X_HEIGHT, &value))              printf("  X Height: %ld,", value);
    if (XGetFontProperty(fs, XA_QUAD_WIDTH, &value))            printf("  Quad Width: %ld", value);
    if (XGetFontProperty(fs, XA_SUPERSCRIPT_X, &value))         printf("  SuperScriptX: %ld", value);
    if (XGetFontProperty(fs, XA_SUPERSCRIPT_Y, &value))         printf("  SupercriptY: %ld", value);
    if (XGetFontProperty(fs, XA_SUBSCRIPT_X, &value))           printf("  SubScriptX: %ld", value);
    if (XGetFontProperty(fs, XA_SUBSCRIPT_Y, &value))           printf("  SubScriptY: %ld", value);
    if (XGetFontProperty(fs, XA_UNDERLINE_POSITION, &value))    printf("  Uline Pos: %ld", value);
    if (XGetFontProperty(fs, XA_UNDERLINE_THICKNESS, &value))   printf("  Uline Size: %ld", value);
    printf("\n");
}


static void
grXFontLoad(void)
{
    XFontStruct *font = NULL, *ifont = NULL;
    const struct FontPair *fp;
    unsigned fx;

    //  locate available font
    //
    x_fontSize = 14;                            // default point size
    if (xr_fontPointSize) {
        int fontSize;

        if ((fontSize = atoi(xr_fontPointSize)) >= 8 && fontSize <= 64) {
            x_fontSize = fontSize;              // user override 8 .. 64
        }
    }

    xr_fonts[0].name = xr_fontName;
    xr_fonts[0].italic = xr_fontItalicName;

    for (fx = 0, fp = xr_fonts; fx < VSIZEOF(xr_fonts); ++fx, ++fp) {
        if (NULL != (font = grXLoadQueryFont(fp->type, 1, fp->name, x_fontSize,
                                    x_fontName, sizeof(x_fontName)))) {
            if (fp->italic) {
                ifont = grXLoadQueryFont(fp->type, -1, fp->italic, x_fontSize,
                                x_fontItalicName, sizeof(x_fontItalicName));
            }
            break;
        }
    }

    if (NULL == font) {
        if (xr_fontName) {
            error("cannot load font '%s' or any of the default alternatives.\n", xr_fontName);
        } else {
            error("cannot load a suitable font\n");
        }
    }

    //  check font specification
    //
    if (font->min_bounds.width != font->max_bounds.width) {
        infof("x11: font '%s' is not fixed-width.\n", xr_fontName);
    }

    x_fontID      = font->fid;                  // normal font
    x_fontWidth   = font->max_bounds.width;
    x_fontAscent  = font->ascent;
    x_fontDescent = font->descent;
    x_fontHeight  = x_fontAscent + x_fontDescent;

printf("x11: font <%dx%d>\n", x_fontWidth, x_fontHeight);

    if (ifont) {                                // italic font
        const int iwidth = ifont->max_bounds.width,
                iheight = ifont->ascent + ifont->descent;
        Font fd = ifont->fid;

printf("x11: italic-font <%dx%d>\n", iwidth, iheight);

        if ((iwidth - 1) > x_fontWidth || (iheight - 1) > x_fontHeight) {
            infof("x11: italic and normal font differ in size, ignored\n");
            XUnloadFont(x_disp, fd);
            fd = 0;

        } else if (ifont->min_bounds.width != ifont->max_bounds.width) {
            infof("x11: italic font '%s' is not fixed-width.\n", x_fontItalicName);
        }

        x_italicID = fd;
    }

    XFreeFontInfo(NULL, font, 0);
    if (ifont) {
        XFreeFontInfo(NULL, ifont, 0);
    }

    //  derive list of associated fonts
    //
    int fontNumber = 0;
    XFontStruct *finfos = NULL;
    char **fnames = XListFontsWithInfo(x_disp, x_fontName, MAX_FONTNAMES, &fontNumber, &finfos);

    if (fnames) {

        int ifontNumber = 0;
        XFontStruct *ifinfos = NULL;
        char **ifnames = (0 == ifont ? NULL :
                XListFontsWithInfo(x_disp, x_fontItalicName, MAX_FONTNAMES, &ifontNumber, &ifinfos));

        const XFontStruct *fs;
        struct fontName *fn, *tfn;
        unsigned x;

        x_fontNumber = 0;
        x_fontIndex = -1;
                                                // cache suitable font
        for (fx = 0, fn = x_fontNames, fs = finfos; fx < fontNumber; ++fx, ++fs) {

            const int width = fs->max_bounds.width,
                    height = fs->ascent + fs->descent;
            const char *name = fnames[fx];
            unsigned long ptsize = 0;

            if (fs->min_bounds.width != fs->max_bounds.width ||
                    0 == XGetFontProperty((XFontStruct *)fs, XA_POINT_SIZE, &ptsize)) {
                continue;                       // ignore
            }

            for (x = 0, tfn = x_fontNames; x < x_fontNumber; ++x, ++tfn) {
                if (width == tfn->width && height == tfn->height &&
                        tfn->ptsize == (int)ptsize) {
                    name = NULL;                // remove duplicate entries
                    break;
                }
            }

            if (name) {
                //
                //  unique font
                //
                fn->width   = width;
                fn->height  = height;
                fn->ptsize  = (int)ptsize;
                fn->name    = chk_salloc(name);

                if (ifnames) {                  // associate suitable italic font
                    const XFontStruct *ifs;
                    unsigned ifx;

                    for (ifx = 0, ifs = ifinfos; ifx < ifontNumber; ++ifx, ++ifs) {
                        const int iwidth = ifs->max_bounds.width,
                                iheight = ifs->ascent + ifs->descent;
                        unsigned long iptsize = 0;

                        if (0 == XGetFontProperty((XFontStruct *)ifs, XA_POINT_SIZE, &iptsize)) {
                            continue;           // ignore
                        }

                        if (iwidth == width && iheight == height && iptsize == ptsize) {
                            fn->italic = chk_salloc(ifnames[ifx]);
                            break;
                        }
                    }
                }

                ++x_fontNumber;
                ++fn;
            }
        }
                                                // sort by size
        qsort(x_fontNames, x_fontNumber, sizeof(struct fontName), xgrFontNameCmp);

                                                // determine index of 'current' font
        for (fx = 0, fn = x_fontNames; fx < x_fontNumber; ++fx, ++fn) {
            if (-1 == x_fontIndex) {
                if (fn->width == x_fontWidth && fn->height == x_fontHeight) {
                    x_fontIndex = fx;

                } else if (fn->width > x_fontWidth && fn > 0) {
                    x_fontIndex = fx - 1;
                }
            }

printf("x11: font [%2d] %2dx%2d-%3d\n", fx, fn->width, fn->height, fn->ptsize);
printf("\t\t %c <%s>\n", (fx == x_fontIndex ? '*' : ' '), fn->name);
printf("\t\t   <%s>\n", (fn->italic ? fn->italic : ""));
        }

        XFreeFontInfo(fnames, finfos, fontNumber);
        if (ifnames) {
            XFreeFontInfo(ifnames, ifinfos, ifontNumber);
        }
    }
}


static XFontStruct *
grXLoadQueryFont(int type, int order, const char *fontspec, int size, char *name, int namelen)
{
    char t_fontname[100];
    XFontStruct *result = NULL;

    if (fontspec && fontspec[0]) {              // absolute or search?
        switch (type) {
        case FT_USER:
        case FT_FIXED:
            /*
             *  Absolute name ..
             */
            result = XLoadQueryFont(x_disp, fontspec);
            break;

        case FT_ISO10646:
            if (size >= 8) {
                /*
                 *  Allow varying sizes until we get a hit
                 *
                 *      -adobe-courier-medium-r-normal--0-0-100-100-m-0-iso10646-1
                 *                                      ^size
                 *
                 *  Element
                 *
                 *      foundry     The name of the company that digitized the font.
                 *      family      The typeface family - Courier, Times Roman, Helvetica, etc.
                 *      weight      The "blackness" of the font - bold, medium, demibold, etc.
                 *      slant       The "posture" of the font. "R" for upright, "I" for italic, etc.
                 *      setwidth    Typographic proportionate width of the font.
                 *      addstyle    Additional typographic style information.
                 *      pixels      Size of the font in device-dependent pixels
                 *      points      Size of the font in device-independent points
                 *      horiz       Horizontal dots-per-inch for which the font was designed.
                 *      vert        Vertical dots-per-inch for which the font was designed
                 *      spacing     The spacing class of the font.  "P" for proportional, "M" for monospaced, etc
                 *      avgwidth    Average width of all glyphs in the font
                 *      rgstry      Tells the language the characters conform to. (iso8859 = Latin characters)
                 *      encoding    Further language encoding information
                 *
                 */
                const unsigned so_1[] = {0, +1, -1, -2, +2, -3, +3, -4, +4, +5, 9999};
                const unsigned so_0[] = {0, -1, -2, -4, 9999};
                unsigned i;

                for (i = 0;; ++i) {             // point size search
                    const int ps = size + (order > 0 ? so_1[i] : so_0[i]);

                    if (ps > 999) break;

                    snprintf(t_fontname, sizeof(t_fontname), "%s-%d-*-*-*-*-*-%s", fontspec, ps, "iso10646-1");
                    t_fontname[sizeof(t_fontname)-1] = 0;
                    if (0 != (result = XLoadQueryFont(x_disp, t_fontname))) {
                        if (name && namelen > 0) {
                            snprintf(t_fontname, sizeof(t_fontname), "%s-*-*-*-*-*-*-%s", fontspec, "iso10646-1");
                            strxcpy(name, t_fontname, namelen);
                        }
                        fontspec = t_fontname;  // success
                        break;
                    }
                }
            }
            break;
        }
    }

printf("x11: font <%s>\n", (result ? fontspec : "na"));
    return result;
}


static void
grXFontSize(int direction)
{
    int fontIndex;

    // determine new font index
    if (x_fontNumber <= 1 || x_fontIndex < 0) {
        return;
    }

    if (direction > 0) {                        // next
        if ((fontIndex = x_fontIndex + 1) >= x_fontNumber) {
            return;
        }
    } else {                                    // previous
        if ((fontIndex = x_fontIndex - 1) < 0) {
            return;
        }
    }

printf("x11: size-font <%d,%s>\n", direction, x_fontNames[fontIndex].name);

    grXFontChange(x_fontNames[fontIndex].name, x_fontNames[fontIndex].italic);

    x_fontIndex = fontIndex;
}


static int
grXFontChange(const char *normal, const char *italic)
{
    XFontStruct *font = NULL, *ifont = NULL;
    unsigned width, height, temp;

    // load fonts
    if (NULL == (font = XLoadQueryFont(x_disp, normal))) {
        return -1;
    }

    if (italic) {
        ifont = XLoadQueryFont(x_disp, italic);
    }

    // reapply new font size
    XGetGeometry(x_disp, x_win, (void *)&temp, (int *)&temp, (int *)&temp, &width, &height, &temp, &temp);

    XUnloadFont(x_disp, x_fontID);
    if (x_italicID) {
        XUnloadFont(x_disp, x_italicID);
    }

    x_fontID      = font->fid;
    x_fontAscent  = font->ascent;
    x_fontDescent = font->descent;
    x_fontHeight  = x_fontAscent + x_fontDescent;
    x_fontWidth   = font->max_bounds.width;
    x_italicID    = (ifont ? ifont->fid : 0);

    strxcpy(x_fontName, normal, sizeof(x_fontName));
    strxcpy(x_fontItalicName, (italic ? italic : ""), sizeof(x_fontItalicName));

    XFreeFontInfo(NULL, font, 0);
    if (ifont) {
        XFreeFontInfo(NULL, ifont, 0);
    }

    width /= x_fontWidth;
    height /= x_fontHeight;

    xgr_drawCursor(0);                          // hide the cursor
    grXGCSetup(x_disp, x_win, x_fontID, x_italicID);

    if (x_screenWidth == width && x_screenHeight == height) {
        vtgarbled();                            // force screen update

    } else {
        BuffersAlloc(width, height);
        XClearWindow(x_disp, x_win);
        ttwinched(height, width);               // enact screen change
    }

    return 0;
}


/*
 *  Setup graphic contexts
 */
static void
grXGCSetup(Display *disp, Window win, Font font, Font ifont)
{
printf("x11: gc-setup\n");

    if (! x_normalGC) {                         // working context
        XGCValues gcv = {0};

        gcv.font = font;
        gcv.graphics_exposures = 1;
        gcv.foreground = WhitePixel(disp, Xscreen());
        x_normalGC = XCreateGC(disp, win, GCForeground | GCFont | GCGraphicsExposures, &gcv);

        gcv.font = (ifont ? ifont : font);
        gcv.graphics_exposures = 1;
        gcv.foreground = WhitePixel(disp, Xscreen());
        x_italicGC = XCreateGC(disp, win, GCForeground | GCFont | GCGraphicsExposures, &gcv);
    }

    XSetFont(disp, x_normalGC, font);
    XSetFont(disp, x_italicGC, ifont ? ifont : font);

    if (! x_cursorGC) {                         // cursor
        XGCValues gcv = {0};

        gcv.function = GXxor;
        gcv.plane_mask = AllPlanes;
        gcv.foreground = WhitePixel(disp, Xscreen());
        x_cursorGC = XCreateGC(disp, win, GCFunction|GCForeground|GCPlaneMask, &gcv);
    }
}


/*
 *  Destructor.
 */
static void
xgr_close(void)
{
printf("x11: close\n");
    ResourceFree();
}


/*
 *  Event queue retrieval.
 */
static int
xgr_event(struct IOEvent *evt, accint_t tmo)
{
    IOTimer_t timer;

    iot_start(&timer, tmo);

    for (;;) {
        evt->type = EVT_NONE;

        iot_current();

        if (x_evLength > 0) {                   // pending events
            --x_evLength;
            *evt = *x_evOut;
            if (++x_evOut >= &x_evQueue[eventQSize]) {
                x_evOut = &x_evQueue[0];
            }
            break;

        } else if (iot_expired(&x_msTimer)) {   // mouse auto-click
            iot_start(&x_msTimer, AUTOCLICK_NEXT);
            *evt = x_msEvent;
            break;

        } else if (iot_expired(&timer)) {       // user timeout
            evt->type = EVT_TIMEOUT;
            return (evt->code = -1);

        } else if (0 == grXEventGet(evt, DELAY_UNIT)) {
            break;

        }
    }

//printf("x11: event: %d (%d,%d)\n", evt->type, evt->mouse.x, evt->mouse.y);
    return 0;
}


/*
 *  Pushes an event in the queue.  If the queue is full the event will be discarded.
 */
static void
xgr_push(const struct IOEvent *evt)
{
    if (x_evLength < eventQSize) {
        ++x_evLength;
        *x_evIn = *evt;
        if (++x_evIn >= &x_evQueue[eventQSize]) {
            x_evIn = &x_evQueue[0];
        }
    }
}


/*
 *  Generates a beep.
 */
static void
xgr_beep(int freq, int duration)
{
    __CUNUSED(freq)
    __CUNUSED(duration)
    XBell(x_disp, 0);
}


/*
 *  Set/get font.
 */
static int
xgr_font(int setlen, char *font)
{
    if (font) {
        if (setlen > 0) {
            strxcpy(font, x_fontName, setlen);
            return 0;
        }
        return grXFontChange(font, NULL);       // 0=success, otherwise non-zero
    }
    return -1;
}


/*
 *  Hides or shows the cursor.
 */
static void
xgr_drawCursor(int show)
{
    if (x_cursor_state != show) {
        x_cursor_state = show;
        if (show) {                             // turn-on the cursor.
            drawTextCursor();
        } else {                                // turn-off the cursor.
            eraseTextCursor();
        }
    }
}


/*
 *  Cursor control.
 */
static int
xgr_cursor(int visible, int imode /*TODO*/, int virtual_space /*TODO*/)
{
    xgr_drawCursor(visible ? TRUE : FALSE);
}


/*
 *  Moves the cursor to another place.
 */
static void
xgr_move(int row, int col)
{
    if (x_cursor_state) {
        eraseTextCursor();
    }
    x_curx = col;
    x_cury = row;
    if (x_cursor_state) {
        drawTextCursor();
    }
}


/*
 *  Color initialisation.
 */
static int
AllocColor(Display *disp, const char *name, XColor *color)
{
    if (XParseColor(disp, x_colorMap, name, color)) {
        return XAllocColor(disp, x_colorMap, color);
    }
    return 0;
}


static void
xgr_colors(void)
{
    if (0 == tt_colors)
    {
        XColor dummy = {0}, *colors = x_colors;
        unsigned c;

        for (c = 0; c < 256; ++c) {             // color mapping
            if (0 == AllocColor(x_disp, xgr_color256[c], colors + c)) {
                error("cannot allocate color %s", xgr_color256[c]);
            }

            trace_log("x11: color<%s> %04x/%04x/%04x (%08lx)\n", xgr_color256[c],
                colors[c].red, colors[c].green, x_colors[c].blue, colors[c].pixel);
        }
    }

    tt_colors = 256;
    tt_defaultfg = WHITE;
    tt_defaultbg = BLACK;

    if (x_pt.pt_defaultfg >= 0 && x_pt.pt_defaultbg >= 0) {
        if (x_pt.pt_defaultfg < tt_colors && x_pt.pt_defaultbg < tt_colors) {
            tt_defaultfg = x_pt.pt_defaultfg;
            tt_defaultbg = x_pt.pt_defaultbg;
        }
    }

    x_pt.pt_colordepth = tt_colors;
    x_pt.pt_schemedark = TRUE;                  // dark
    x_display_ctrl |= DC_SHADOW_SHOWTHRU;
}


/*
 *  Color lookup.
 */
static int
xgr_color(const colvalue_t ca, int def)
{
    int t_color, color = 0;

    if (COLORSOURCE_SYMBOLIC == ca.source) {
        if ((color = ca.color) < 0 || color >= COLOR_NONE) {
            color = def;
        }

        if ((t_color = tt_colormap[color]) >= 0) {
            color = t_color;

        } else {
            color = xgr_colormap[color];
        }

    } else {
        if ((color = ca.color) >= tt_colors || color < 0) {
            if ((t_color = tt_colormap[def]) >= 0) {
                color = t_color;

            } else {
                color = xgr_colormap[def];
            }
        }
    }
    return color;
}


/*  Function:           grXWriteRow
 *      Updates the X display with new text.
 *
 *  Parameters:
 *      dst -               Destination offset, character offset from beginning of the
 *                          display buffer.
 *
 *      src -               Source buffer.
 *
 *      len -               Length of the source buffer, in charaters.
 *
 *      isExpose -          Whether a *redraw* event.
 *
 *  Returns:
 *      void
 */
static void
grXWriteRow(int dst, const struct _VCELL *src, int len, BOOL isExpose)
{
    const struct _VCELL *osrc = src, *cache = x_screenBuffer + dst;
    int x, y, i, j, chunkno = 0;
    UpdateBlock *chunk;
    XCHAR *update;

    assert(dst >= 0);
    assert(dst < (x_screenWidth * x_screenHeight));
    assert(len > 0);
    assert(len <= x_screenWidth);

    // If this isn't a forced update, it may be possible to skip this update
    // completely if it is redundant.
    //
    if (! isExpose) {
        if (0 == memcmp(src, cache, len * sizeof(struct _VCELL))) {
            return;
        }
    }

    // Gather up a list of update blocks for the current row, taking into
    // account what is already on the screen.
    //
    update = x_update_buffer;
    chunk  = x_update_chunks;
    chunk->state = UPD_NONE;

    for (i = len; i > 0; ++src, ++cache, ++update, --i) {

        const vbyte_t cattr = VBYTE_ATTR_GET(cache->primary);
        const vbyte_t ccode = VBYTE_CHAR_GET(cache->primary);
        const vbyte_t attr  = VBYTE_ATTR_GET(src->primary);
        const vbyte_t code  = VBYTE_CHAR_GET(src->primary);

        // Determine if a new update block needs to be created.
        //
        if (attr == cattr && code == ccode && !isExpose) {
            //  Current character is already on the screen.
            //
            if (chunk->state != UPD_REDUNDANT ||
                chunk->attr  != attr) {
                // Start a new redundant block.
                if (chunk->state != UPD_NONE) {
                    ++chunkno;
                    ++chunk;
                }
                chunk->state = UPD_REDUNDANT;
                chunk->attr  = attr;
                chunk->data  = update;
                chunk->len   = 0;
            }

        } else {
            // Current character is not already on the screen.
            //
            if (chunk->state != UPD_UPDATE ||
                chunk->attr  != attr) {
                // Start a new update block.
                if (chunk->state != UPD_NONE) {
                    ++chunkno;
                    ++chunk;
                }
                chunk->state = UPD_UPDATE;
                chunk->attr  = attr;
                chunk->data  = update;
                chunk->len   = 0;
            }
        }

        // Add this character to the current block.
        //
        ++chunk->len;

        if (code >= CH_MIN && code <= CH_MAX) {
            int ucode;                          /* drawing characters */

            if ((ucode = cmap_specunicode(code)) > 0) {
                update->byte2 =  ucode & 0xff;
                update->byte1 = (ucode >> 8) & 0xff;
                continue;
            }
        }

        update->byte2 =  code & 0xff;
        update->byte1 = (code >> 8) & 0xff;
    }

    if (x_update_chunks[0].state == UPD_NONE) {
        //
        // There's nothing to do.
        //
        return;
    }

    //  Now collapse update blocks into an efficient form.
    //
    //  This algorithm takes into account the network cost of an  XDrawImageString
    //  request (known as an ImageText8 request in the X11 protocol specification).
    //
    //  If two draw image requests can be collapsed into a single request
    //  of smaller size, it will do so.
    //
    //  The size of an ImageText8 message header.
    //
    //      ImageText8
    //          drawable: DRAWABLE
    //          gc: GCONTEXT
    //          x, y: INT16
    //          string: STRING8
    //
    static const int IMAGETEXT8_SIZE = 1 + 4 + 4 + 2 + 2 + 4;

    // Compute the current screen x and y position.
    //
    x = dst % x_screenWidth;
    y = dst / x_screenWidth;

    for (i = 0; i <= chunkno; ++i) {
        //
        // Skip any leading redundant blocks.
        //
        if (x_update_chunks[i].state == UPD_REDUNDANT) {
#ifdef DEBUG_OPTIMIZATION
            trace_log("x11: skipping %d redundant at (%d, %d) color %02x.\n",
                    x_update_chunks[i].len, y, x, x_update_chunks[i].attr);
#endif
            x += x_update_chunks[i].len;
            continue;
        }

        // We've got an update block.  See if it can be expanded.
        //
        while ((i + 2 <= chunkno) &&
                x_update_chunks[i + 1].state == UPD_REDUNDANT &&
                x_update_chunks[i + 1].attr  == x_update_chunks[i].attr &&
                x_update_chunks[i + 2].state == UPD_UPDATE &&
                x_update_chunks[i + 2].attr  == x_update_chunks[i].attr) {

            // We've found a possible group to collapse; is it worth it?
            //
            if (x_update_chunks[i + 1].len <= IMAGETEXT8_SIZE) {
                //
                // collapse is worth it .. do it.
                //
#ifdef DEBUG_OPTIMIZATION
                trace_log("x11: collapsing %d:%d:%d color %02x run.\n",
                    x_update_chunks[i].len,
                    x_update_chunks[i + 1].len,
                    x_update_chunks[i + 2].len,
                    x_update_chunks[i].attr);
#endif
                x_update_chunks[i].len += x_update_chunks[i + 1].len;
                x_update_chunks[i].len += x_update_chunks[i + 2].len;
                for (j = i + 1; (j + 2) <= chunkno; ++j) {
                    x_update_chunks[j] = x_update_chunks[j + 2];
                }
                chunkno -= 2;

                // See if we can collapse some more.
                //
                continue;
            } else
                break;
        }

        // Draw the current block.
        //
        colattr_t ca = {0};
        XGCValues gcv;

        chunk = x_update_chunks + i;

        if (color_definition(chunk->attr, &ca)) {
            const int len = chunk->len,
                xpos = x * x_fontWidth, ypos = y * x_fontHeight;
            GC gc = x_normalGC;

            int fg = xgr_color(ca.fg, tt_defaultfg),
                    bg = xgr_color(ca.bg, tt_defaultbg);

            if (fg == bg) {                     // normal
                fg = tt_defaultfg;
                bg = tt_defaultbg;
            }

            gcv.foreground = x_colors[bg].pixel;
            XChangeGC(x_disp, gc, GCForeground, &gcv);
            XFillRectangle(x_disp, x_win, gc,
                xpos, ypos, x_fontWidth * len, x_fontHeight);

            if (COLORSTYLE_ITALIC & ca.sf) {    // italic, if available
                gc = x_italicGC;
            }

            gcv.foreground = x_colors[fg].pixel;
//          gcv.background = x_colors[bg].pixel;
            XChangeGC(x_disp, gc, GCForeground|GCBackground, &gcv);
//          XDrawImageString16(x_disp, x_win, gc,
            XDrawString16(x_disp, x_win, gc,
                x * x_fontWidth, (y * x_fontHeight) + x_fontAscent, chunk->data, len);

            if (COLORSTYLE_UNDERLINE & ca.sf) { // underline
                //  if (fg >= 0) {
                //      gcv.foreground = x_colors[fg].pixel;
                //      XChangeGC(x_disp, gc, GCForeground|GCBackground, &gcv);
                //  }
                XDrawLine(x_disp, x_win, gc,
                    xpos, ypos + x_fontAscent + 1, (x + len) * x_fontWidth, (y * x_fontHeight) + x_fontAscent + 1);
            }
        }

#ifdef DEBUG_OPTIMIZATION
        XFlush(x_disp);
        trace_log("x11: wrote %d characters at (%d, %d) color %02x.\n",
            x_update_chunks[i].len, y, x, x_update_chunks[i].color);
#endif

        // If the cursor is on and we've just drawn over it, redisplay it.
        //
        if (x_cursor_state &&
                x_cury == y && x_curx >= x && x_curx < (x + x_update_chunks[i].len)) {
            drawTextCursor();
        }

        x += x_update_chunks[i].len;
    }

    //
    // Update the current screen buffer cache.
    //
    memmove(x_screenBuffer + dst, osrc, len * sizeof(struct _VCELL));
}


//
//  Get the contents of the system clipboard.
//
char *
xgr_clipboard_get(size_t *sz, BOOL line)
{
    char *answer = NULL, *data, *clip;
    BOOL xfree = FALSE;
    size_t clsz;
#if defined(ADVCLIPBOARD)
    Window currentOwner;
    int r;

    //
    // See who is the current selection owner.
    //
    currentOwner = XGetSelectionOwner(x_disp, XA_CLIPBOARD);

    if (currentOwner == None) {
        //
        // No one has posted anything to the clipboard.
        //
        goto fail;

    } else if (currentOwner == x_win) {
        //
        // We're the current selection owner.
        // Bypass the normal X11 query protocol and use what is on our
        // internal clipboard.
        //
        clsz = clipboardSize;
        data = clipboardBuffer;
        trace_log("x11: Using internal clipboard data, size %lu.\n", (unsigned long)clsz);

    } else {
        //
        // Someone else owns the current selection.
        // Ask them to convert the current selection to an XA_STRING.
        //
        r = XConvertSelection(x_disp, XA_CLIPBOARD, XA_STRING, clipboardAtom, x_win, CurrentTime);
        if (r == BadAtom || r == BadWindow) {
            trace_log("x11: ConvertSelection failed (%d).\n", r);
            goto fail;
        }

        //
        // Await a response from the selection owner, notifying us that the
        // selection transfer has been completed.
        //
        XEvent responseEvent;

        //
        // Pull the next SelectionNotify event out of the event queue, waiting
        // for one to arrive if none are currently available, and leaving all
        // other queued events undisturbed.
        //
        r = XIfEvent(x_disp, &responseEvent, grXNotifyEvent, NULL);
        if (r != Success) {
            trace_log("x11: wait for selectionNotify failed.\n");
            goto fail;
        }

        //
        // The filter function should have ensured that the only event we receive
        // is a SelectionNotify event, but double check just to make sure.
        //
        if (responseEvent.type != SelectionNotify) {
            trace_log("x11: expected selection notify, got %d.\n", responseEvent.type);
            goto fail;
        }

        //
        // Did the sender indicate a successful transfer?
        //
        if (responseEvent.xselection.property == None) {
            trace_log("x11: responder says transfer failed.\n");
            goto fail;
        }

        //
        // Query our window (which we designated as the receipient of the
        // transfer when we made the selection request) for the property
        // to which we asked the sender to write the results.
        //
        Atom actual_type_atom;
        int actual_format;
        unsigned long bytes_returned, bytes_remaining;

        r = XGetWindowProperty(x_disp, x_win, clipboardAtom, 0, 1024 * 1024, False,
                        XA_STRING, &actual_type_atom, &actual_format,
                        &bytes_returned, &bytes_remaining,
                        (unsigned char **)&data);
        if (r != Success) {
            trace_log("x11: couldn't get window property.\n");
            goto fail;
        }

        if (bytes_remaining > 0)
            //
            // Wow.  There are more bytes in the clipboard than the insane amount
            // that we requested.  The clipboard text will truncated.
            // Log a warning.
            //
            infof("x11: dropped %lu bytes from clipboard transfer.", bytes_remaining);

        clsz = bytes_returned;
        xfree = TRUE;
    }

#else
    int bytes_returned;

    data = XFetchBytes(x_disp, &bytes_returned);
    if (data == NULL) {
        infof("x11: XFetchBytes failed.\n");
        goto fail;
    }
    trace_log("x11: XFetchBytes ok: %p, %d.\n", data, bytes_returned);
    xfree = TRUE;
    clsz = bytes_returned;
#endif

    //
    // This code stolen from the Windows NT TVision clipboard code.
    // It appears to filter out unwanted characters.
    //
    if (clsz != 0) {
        //
        // Is there more available than the caller is willing to accept?
        //
        if (*sz < clsz) {
            //
            // We've got more than the caller wants.  Truncate.
            //
            clsz = *sz;
        }

        int allocsz = clsz;

        //
        // If this is a multi-line retreival, scan the string for newlines not
        // preceeded by a carriage return.  These "UNIX" convention newlines will
        // not be interpreted correctly by TVision unless they are preceeded by
        // carriage returns, so we must insert them ourselves.  We need to know
        // how many we are going to insert so that we can allocate a proper buffer.
        //
        if (line) {
            int newlines, i;
            BOOL cr;

            for (i = newlines = 0, cr = FALSE; i < clsz; ++i) {
                if (data[i] == '\r') {
                    cr = TRUE;
                } else {
                    if (data[i] == '\n') {
                        if (cr == FALSE) {
                            newlines++;
                        }
                    }
                    cr = FALSE;
                }
            }
            allocsz += newlines;
        }

        //
        // Allocate a buffer to return to the caller.
        //
        answer = (char *)chk_alloc(allocsz);

        //
        // Copy characters into the buffer, trimming unprintables and
        // mapping newline and carriage returns into spaces, if needed.
        //
        if (answer != NULL) {
            char *dst = answer;
            char *src = data;
            BOOL cr = FALSE;

            while (*src != '\0' && clsz > 0) {
                char c = *src++;
                clsz--;
                if (c < ' ' || c == 127) {
                    //
                    // Character is non-printable.
                    //
                    if (line && dst == answer) {
                        //
                        // We haven't seen any printables yet, so
                        // just skip this character entirely.
                        // (This has the effect of eating leading whitespace).
                        //
                        continue;
                    }

                    if (line || (c != '\r' && c != '\n')) {
                        //
                        // We've written printable characters already.
                        // Convert this weird character to a space.
                        //
                        c = ' ';
                    }
                }

                if (c == '\n' && !line && !cr) {
                    //
                    // We need to insert a carriage return in front
                    // of this newline.
                    //
                    *dst++ = '\r';
                }

                if (c == '\r')
                    cr = TRUE;
                else
                    cr = FALSE;
                *dst++ = c;
            }

            if (line) {
                //
                // Trim trailing whitespace.
                //
                for ( ; dst > answer; --dst ) {
                    if ( dst[-1] > ' ' ) {
                        break;
                    }
                }
            }

            //
            // Is the entire result empty?
            //
            if (dst == answer) {
                chk_free (answer);
                answer = NULL;
            } else {
                *dst = '\0';
                *sz = dst - answer;
            }
        }
    }

    if (xfree) {
        XFree(data);
    }

    return answer;

fail:;
    *sz = 0;
    return NULL;
}


//
//  Put data on the system clipboard.
//
BOOL
xgr_clipboard_put(const char *str, size_t from, size_t to)
{
#if defined(ADVCLIPBOARD)
    Time theTime;

    //
    // We need to figure out what XEvent caused this program to want to
    // paste something on the clipboard so that we can tell the X server
    // exactly _when_ we want it on the clipboard.
    //
    // We do so by assuming that the last received X event must have caused
    // this behavior and using that event's timestamp.
    //
    switch (x_xEvent.type) {
    case ButtonPress:
    case ButtonRelease:
        theTime = x_xEvent.xbutton.time;
        break;

    case KeyPress:
    case KeyRelease:
        theTime = x_xEvent.xkey.time;
        break;

    default:
        trace_log("x11: can't paste to clipboard because I can't correlate an X event.\n");
        return FALSE;
    }

    // Notify the X server that we want to own the clipboard.
    //
    XSetSelectionOwner(x_disp, XA_CLIPBOARD, x_win, theTime);

    // See if we managed to gain ownership.
    //
    if (XGetSelectionOwner(x_disp, XA_CLIPBOARD) != x_win) {
        //
        // We failed to acquire the selection.
        //
        return FALSE;
    }

    // We've got ownership.
    //  Remember the characters that the application wants to send so that we
    //  can send them later, when the server asks for them.
    //
    chk_free(clipboardBuffer);
    clipboardSize = to - from;
    clipboardBuffer = (char *) chk_alloc(clipboardSize);
    memcpy(clipboardBuffer, str + from, clipboardSize);
    trace_log("x11: stored %d bytes on internal clipboard.\n", clipboardSize);

    return TRUE;

#else
    int r;

    r = XStoreBytes(x_disp, str + from, to - from);
    if (r == Success)
        return TRUE;
    infof("x11: XStoreBytes failed (%d).\n", r);
    return FALSE;
#endif
}


#if defined(ADVCLIPBOARD)
static Bool
grXNotifyEvent(Display *d, XEvent *e, XPointer arg)
{
    if (e->type == SelectionNotify)
        return True;
    return False;
}
#endif // ADVCLIPBOARD


static void
functor_init(void)
{
    if (pipe(x_functor_pipes) != 0) {
        error("failed to create syncronization objects\n");
    }
}


static void
functor_close(void)
{
    close(x_functor_pipes[0]);
    close(x_functor_pipes[1]);
}


//
//  Send an asynchronious request
//
void
functor_send(void *req)
{
    if (write(x_functor_pipes[1], &req, sizeof(req)) != sizeof(req)) {
        abort();
    }
}


//
//  Register a callback for asycnhronious requests
//
void
functor_collector(void (*func)(void *))
{
    x_functor_collector = func;
}


//
//  Store asynchronious event in the queue.
//
static void
functor_handler(void)
{
    void *req;

    if (read(x_functor_pipes[0], &req, sizeof(req)) == sizeof(req)) {
        if (x_functor_collector) {
            x_functor_collector(req);
        }
    }
}
#endif // HAVE_LIBX11 and HAVE_X11_XLIB_H
