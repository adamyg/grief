#include <edidentifier.h>
__CIDENT_RCSID(gr_ttywin32_c,"$Id: ttywin32.c,v 1.55 2024/01/01 10:52:12 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttywin32.c,v 1.55 2024/01/01 10:52:12 cvsuser Exp $
 * WIN32 VIO driver.
 *  see: http://www.edm2.com/index.php/Category:Vio
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

#if defined(WIN32)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#elif (_WIN32_WINNT < 0x601)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif
#undef WINVER
#define WINVER _WIN32_WINNT

#include <editor.h>

#if defined(USE_VIO_BUFFER)

#include <libstr.h>                             /* str_...()/sxprintf() */
#include <assert.h>

#if !defined(WINDOWS_MEAN_AND_LEAN)
#define WINDOWS_MEAN_AND_LEAN
#define PSAPI_VERSION               1           // EnumProcessModules and psapi.dll
#include <windows.h>
#include <wincon.h>
#endif  //WINDOWS_MEAN_AND_LEAN

#define TERMEMU_VIO_LOCAL                       /* private interface */

#include "debug.h"
#include "display.h"                            /* DISPTYPE_.. */
#include "main.h"                               /* panic() */
#include "tty.h"                                /* x_pt[] */
#include "ttyrgb.h"
#include "vio.h"                                /* VIO interface */
#include "window.h"

#include "termemu_vio.c"

static void             VioInitialise(void);
static void             VioEncoding(void);


/*private*/
//  Function:           VioInitialise
//      Runtime initialisation.
//
static void
VioInitialise()
{
    if (! vio.inited) {
        vio_open(NULL, NULL);
        if (0 == vio.activecolors) {
            vio.activecolors = vio.maxcolors;   /* inherit color limit */
            VioEncoding();
        }
    } else {
        if (vio_init()) {                       /* winch */
            VioEncoding();
        }
    }
}


/*private*/
//  Function:           VioEncoding
//      Derive the console encoding method from the code-page.
//
//  Parameters:
//      none
//
//  Returns:
//      nothing
//
static void
VioEncoding(void)
{
    static const struct win32codepages {
        int         codepage;
        const char *encoding;
    } codepages[] = {
        { 1250,     "windows-1250" },
        { 1251,     "windows-1251" },
        { 1252,     "windows-1252" },
        { 1253,     "windows-1253" },
        { 1254,     "windows-1254" },
        { 1255,     "windows-1255" },
        { 1256,     "windows-1256" },
        { 1257,     "windows-1257" },
        { 1258,     "windows-1258" },
        { 28591,    "iso-8859-1"   },
        { 28592,    "iso-8859-2"   },
        { 28593,    "iso-8859-3"   },
        { 28594,    "iso-8859-4"   },
        { 28595,    "iso-8859-5"   },
        { 28596,    "iso-8859-6"   },
        { 28597,    "iso-8859-7"   },
        { 28598,    "iso-8859-8"   },
        { 28599,    "iso-8859-9"   },
        { 28603,    "iso-8859-13"  },
        { 28605,    "iso-8859-15"  },
        { 1200,     "UTF-16LE"     },
        { 1201,     "UTF-16BE"     },
        { 65000,    "UTF-7"        },
        { 65001,    "UTF-8"        },
        };
    char encoding[32];
    CPINFOEX cpix = {0};
    unsigned cpd;
    int cp;

    if ((cp = vio.codepage) <= 0) {
        if (0 == (vio.codepage = GetConsoleOutputCP())) {
            vio.codepage = 437;
        }
        cp = vio.codepage;
    }

    if (cp <= 0 || 0 == GetCPInfoExA(cp, 0, &cpix)) {
        trace_log("vio: encoding, unable to resolve CP%03d\n", cp);
        return;
    }
    sxprintf(encoding, sizeof(encoding), "CP%d", cp);
    strcpy(x_pt.pt_encoding, encoding);
    for (cpd = 0; cpd < (sizeof(codepages)/sizeof(codepages[0])); ++cpd) {
        if (codepages[cpd].codepage == cp) {
            strxcpy(x_pt.pt_encoding, codepages[cpd].encoding, sizeof(x_pt.pt_encoding));
            break;
        }
    }

    if (DISPTYPE_UNKNOWN == xf_disptype) {
        switch (cp) {
        case 1200:          // Unicode UTF-16LE
        case 1201:          // Unicode UTF-16BE
        case 65000:         // Unicode UTF-7
        case 65001:         // Unicode UTF-8
            xf_disptype = DISPTYPE_UNICODE;
            break;
        default:            // others
            if (1 == cpix.MaxCharSize) {
                xf_disptype =
                    (256 == vio.maxcolors ? DISPTYPE_UNICODE : DISPTYPE_8BIT);
            } else if (cpix.LeadByte[0] || cpix.LeadByte[1]) {
                xf_disptype = DISPTYPE_DBCS;
            } else {
                xf_disptype = DISPTYPE_MBCS;
            }
            break;
        }
        trace_log("vio: encoding type:%d\n", xf_disptype);
    }

    if (DISPTYPE_UNICODE == xf_disptype) {
        x_pt.pt_attributes |= TF_AUNICODEENCODING;
        x_display_ctrl |= DC_CMAPFRAME;         // use unicode graphic characters
        x_display_ctrl |= DC_UNICODE;
    }
}


//  Function:           VioGetMode
//      Return the mode of the display.
//
//  Parameters:
//      info - Mode characteristics.
//      viohandle - VIO presentation-space handle.
//
//  VIOMODEINFO:
//      VIOMODEINFO data structure.
//
//      typedef struct _VIOMODEINFO {
//        USHORT    cb;                 Length of this data structure.
//        UCHAR     fbType;             Bit mask of mode being set.
//        UCHAR     color;              Number of colors (power of 2).
//        USHORT    col;                The number of text columns.
//        USHORT    row;                The number of text rows.
//        USHORT    hres;               Horizontal resolution.
//        USHORT    vres;               Vertical resolution.
//        UCHAR     fmt_ID;             Attribute format.
//        UCHAR     attrib;             Number of attributes.
//        USHORT    resv;               Reserved.
//        ULONG     buf_addr;           Video aperture address.
//        ULONG     buf_length;         Video aperture length.
//        ULONG     full_length;        Video state full save length.
//        ULONG     partial_length;     Video state partial save length.
//        ULONG     ext_data_addr;      Extra data address.
//      } VIOMODEINFO;
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//
int
VioGetMode(VIOMODEINFO *info, HVIO viohandle)
{
    __CUNUSED(viohandle)
    if (0 == vio.inited) {
        /*
         *  uninitialised - VioGetMode() is generally one of the first interface functions executed.
         */
        VioInitialise();
        if (info) {
            assert(info->cb == sizeof(VIOMODEINFO));
            info->row = (USHORT)vio.rows;
            info->col = (USHORT)vio.cols;
            info->color = (USHORT)vio.activecolors;
        }

    } else if (info) {
        /* 
         *  initialised  - report underlying size;
         *     utilised during resize operations, see also VioSetMode() and VioGetBuf().
         */
        int rows = 0, cols = 0;

        assert(info->cb == sizeof(VIOMODEINFO));
        vio_size(&rows, &cols);                 // current physical size.       
        info->row = (USHORT)rows;
        info->col = (USHORT)cols;
        info->color = (USHORT)vio.activecolors;
    }
    return 0;
}


//  Function:           VioSetMode
//      Sets the mode of the display.
//
//  Parameters:
//      info - Mode characteristics.
//      viohandle - VIO presentation-space handle.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//

int
VioSetMode(VIOMODEINFO *info, HVIO viohandle)
{
    if (info) {                                 // ega_switch support (12/10/2014)
        if (info->row || info->col) {
            vio_setsize(info->row, info->col);
            vio_close();
        }
    }
    VioInitialise();                            // (re)initialise
    if (info && (info->row || info->col)) {
        HWND hWnd = vio.whandle;
        if (0xffff == info->row || 0xffff == info->col) {
            RECT r = {0};

            GetWindowRect(hWnd, &r);
            ShowWindow(hWnd, SW_MAXIMIZE);
            MoveWindow(hWnd, 0, 0, r.right, r.bottom, TRUE);
        } else {
            ShowWindow(hWnd, /*SW_RESTORE*/ SW_NORMAL);
        }
    }
    if (info) {
        info->row = (USHORT)vio.rows;
        info->col = (USHORT)vio.cols;
    }
    VioGetCurType(NULL, viohandle);
    return 0;
}


//  Function:           VioSetColors
//      Set the color depth to either 16 or 256 - extension.
//
//  Parameters:
//      colors - Depth depth.
//
//  Returns:
//      nothing.
//
int
VioSetColors(int colors)
{
    switch (colors) {
#if defined(WIN32_CONSOLEEXT) && defined(WIN32_CONSOLE256)
    case 256:
        if (vio.maxcolors >= 255) {
            vio.activecolors = 256;
            return TRUE;
        }
        break;
#endif
    case 16:
        vio.activecolors = 16;
        return TRUE;
    }
    return FALSE;
}


//  Function:           VioGetColors
//      Retrieve the color depth to either 16 or 256.
//
//  Parameters:
//      colors - Depth depth.
//
//  Returns:
//      nothing.
//
int
VioGetColors(int *colors)
{
    if (colors) {
        *colors = vio.activecolors;
    }
    return 0;
}


//  Function:           VioGetCp
//      Queries the code page currently used to display text data.
//
//  Parameters:
//      reserved - Reserved; must be 0.
//      cp - Returned Code-page ID.
//      vioHandle - Presentation-space handle.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//
int
VioGetCp(ULONG reserved, USHORT *cp, HVIO viohandle)
{
    __CUNUSED(reserved)
    __CUNUSED(viohandle)
    if (cp) {
        *cp = (USHORT)vio.codepage;
    }
    return 0;
}


//  Function:           VioSetFont
//      Set the current console font.
//
//  Parameters:
//      fontname - Fontname.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//
static int  selectconsolefont(int nFont);

int
VioSetFont(const char *fontname)
{
    int fontnumber;

    if (fontname && *fontname &&
            vio.SetConsoleFont && (fontnumber = vio.fontnumber) > 0) {
        int offset = 0;

        if (isdigit((unsigned char) fontname[0]) ||
                (0 == _strnicmp(fontname, "font", offset = 4) && isdigit((unsigned char) fontname[4]))) {
            int w = -1, h = -1;                 // [font]8x8

            if (2 == sscanf(fontname + offset, "%d x %d", &w, &h)) {
                const CONSOLE_FONT_INFOEX *fonts = vio.fonts;

                while (--fontnumber > 0) {
                    if (fonts->dwFontSize.X == w && fonts->dwFontSize.Y == h) {
                        return selectconsolefont(fonts->nFont);
                    }
                    ++fonts;
                }
            }

        } else if (0 == _strnicmp(fontname, "index", 4)) {
            int n = -1;                         // index#

            if (1 == sscanf(fontname + 5, "%d", &n)) {
                if (n >= 0 && n < fontnumber) {
                    return selectconsolefont(n);
                }
            }
        }
    }
    return ERROR_VIO_INVALID_PARMS;
}


static int
selectconsolefont(int nFont)
{
    if (vio.SetConsoleFont(vio.chandle, nFont)) {
        InvalidateRect(vio.whandle, 0, FALSE);
        UpdateWindow(vio.whandle);
        return 0;
    }
    return -1;
}


//  Function:           VioSetFont
//      Set the current console font.
//
//  Parameters:
//      fontname - Fontname.
//
//  Returns:
//      0   -   NO_ERROR
//
int
VioGetFont(char *font, int buflen)
{
    if (font && buflen > 0) {
        vio_profile(TRUE);
        sxprintf(font, buflen, "%s %dx%d", vio.fcfacename, vio.fcwidth, vio.fcheight);
        return 0;
    }
    return ERROR_VIO_INVALID_PARMS;
}


//  Function:           VioCursor
//      Set the cursor state.
//
//  Parameters:
//      state - Cursor state.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//
int
VioCursor(VIOCURSOR state)
{
    if (state == VIOCUR_STATE) {
        state = vio.c_state;
    } else {
        unsigned mask;
        switch (state) {                        // new state
        case VIOCUR_THALF: mask = 50;  break;
        case VIOCUR_BHALF: mask = 50;  break;
        case VIOCUR_FULL:  mask = 100; break;
        case VIOCUR_OFF:   mask = 0;   break;
        case VIOCUR_ON:
        default:
            mask = 25;
        }
        if (vio.c_attr) {                       // change
            CONSOLE_CURSOR_INFO cinfo;

            cinfo.bVisible = TRUE;
            cinfo.dwSize = mask;
            (void) SetConsoleCursorInfo(vio.chandle, &cinfo);
        }
        vio.c_size = mask;
        vio.c_state = state;
    }
    return state;
}


//  Function:           VioGetCurType
//      Retrieve the cursor type.
//
//  Parameters:
//      info - Cursor characteristics.
//      viohandle - Presentation-space handle.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      436 -   ERROR_VIO_INVALID_HANDLE
//
int
VioGetCurType(
    VIOCURSORINFO *info, HVIO viohandle)
{
    CONSOLE_CURSOR_INFO cinfo;

    __CUNUSED(viohandle)

    assert(0 == viohandle);
    assert(vio.inited);                         /* uninitialised */

    if (!vio.inited || NULL == vio.image) {
        return ERROR_VIO_MODE;
    } else if (viohandle) {
        return ERROR_VIO_INVALID_HANDLE;
    }

    (void) GetConsoleCursorInfo(vio.chandle, &cinfo);
    vio.c_size = cinfo.dwSize;
    vio.c_attr = (USHORT) cinfo.bVisible;
    if (info) {
        assert(info->cb == sizeof(VIOCURSORINFO));
        info->mask = vio.c_size;
        info->attr = (SHORT)vio.c_attr;
    }
    return 0;
}


//  Function:           VioSetCurType
//      Set the cursor type.
//
//  Parameters:
//      info - Cursor information.
//      viohandle - VIO presentation-space handle.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      436 -   ERROR_VIO_INVALID_HANDLE
//
int
VioSetCurType(
    VIOCURSORINFO *info, HVIO viohandle)
{
    __CUNUSED(viohandle)

    assert(info && info->cb == sizeof(VIOCURSORINFO));
    assert(0 == viohandle);
    assert(vio.inited);                         /* uninitialised */

    if (!info || info->cb != sizeof(VIOCURSORINFO)) {
        return ERROR_VIO_INVALID_PARMS;
    } else if (!vio.inited || NULL == vio.image) {
        return ERROR_VIO_MODE;
    } else if (viohandle) {
        return ERROR_VIO_INVALID_HANDLE;
    }

    if (info->attr != vio.c_attr || info->mask != vio.c_size) {
        CONSOLE_CURSOR_INFO cinfo;

        cinfo.bVisible = (vio.c_attr = info->attr);
        cinfo.dwSize = (vio.c_size = info->mask);
        (void) SetConsoleCursorInfo(vio.chandle, &cinfo);
    }
    return 0;
}


//  Function:           VioGetCurPos
//      Retrieve the coordinates of the cursor.
//
//  Parameters:
//      row - Row return data.
//      col - Column return data.
//      vioHandle - Presentation-space handle.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      436 -   ERROR_VIO_INVALID_HANDLE
//
int
VioGetCurPos(USHORT *row, USHORT *col, HVIO viohandle)
{
    CONSOLE_SCREEN_BUFFER_INFO sbinfo = {0};

    __CUNUSED(viohandle)

    assert(0 == viohandle);
    assert(vio.inited);                         /* uninitialised */

    if (!row && !col) {
        return ERROR_VIO_INVALID_PARMS;
    } else if (!vio.inited || NULL == vio.image) {
        return ERROR_VIO_MODE;
    } else if (viohandle) {
        return ERROR_VIO_INVALID_HANDLE;
    }

    (void) GetConsoleScreenBufferInfo(vio.chandle, &sbinfo);
    if (col) *col = sbinfo.dwCursorPosition.X;
    if (row) *row = sbinfo.dwCursorPosition.Y;
    return 0;
}


//  Function:           VioSetCurPos
//      Set the coordinates of the cursor.
//
//  Parameters:
//      row - Row return data.
//      col - Column return data.
//      vioHandle - Presentation-space handle.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//
int
VioSetCurPos(USHORT row, USHORT col, HVIO viohandle)
{
    HANDLE chandle = vio.chandle;
    CONSOLE_SCREEN_BUFFER_INFO csbi = {0};
    COORD coord;

    __CUNUSED(viohandle)

    assert(0 == viohandle);
    assert(vio.inited);                         /* uninitialised */

    if (!vio.inited || NULL == vio.image) {
        return ERROR_VIO_MODE;
    } else if (viohandle) {
        return ERROR_VIO_INVALID_HANDLE;
    }

    (void) GetConsoleScreenBufferInfo(chandle, &csbi);
    if (csbi.srWindow.Left > 0 || csbi.srWindow.Top > 0) {
        coord.X = 0; coord.Y = 0;               // home console
        (void) SetConsoleCursorPosition(chandle, coord);
        ++vio.c_trashed;
    }

    vio_flush();

    coord.X = col; coord.Y = row;               // true position
    (void) SetConsoleCursorPosition(chandle, coord);
    return 0;
}


int
VioGetCurAttribute(USHORT *attribute, HVIO viohandle)
{
    CONSOLE_SCREEN_BUFFER_INFO sbinfo = {0};

    __CUNUSED(viohandle)

    assert(0 == viohandle);
    assert(vio.inited);                         /* uninitialised */

    if (!attribute) {
        return ERROR_VIO_INVALID_PARMS;
    } else if (!vio.inited || NULL == vio.image) {
        return ERROR_VIO_MODE;
    } else if (viohandle) {
        return ERROR_VIO_INVALID_HANDLE;
    }

    if (GetConsoleScreenBufferInfo(vio.chandle, &sbinfo)) {
        if (attribute) {
            *attribute = sbinfo.wAttributes;
        }
    }
    return 0;
}


int
VioSetCurAttribute(USHORT attribute, HVIO viohandle)
{
    __CUNUSED(viohandle)

    assert(0 == viohandle);
    assert(vio.inited);                         /* uninitialised */

    if (!vio.inited || NULL == vio.image) {
        return ERROR_VIO_MODE;
    } else if (viohandle) {
        return ERROR_VIO_INVALID_HANDLE;
    }

    (void) SetConsoleTextAttribute(vio.chandle, attribute);
    return 0;
}


//  Function:           VioSetFocus
//      Register a window focus event.
//
//  Parameters:
//      setfocus - Focus status.
//
//  Returns:
//      nothing.
//
int
VioSetFocus(int setfocus)
{
    if (setfocus) {
        ++vio.c_trashed;
    }
    return 0;
}


//  Function:           VioGetBuf
//      VioGetBuf returns the address of the logical video buffer (LVB).
//
//  Parameters:
//      pbuf -  Pointer to the logical video buffer address.
//      plength - Pointer to the length of the buffer, in bytes. The length
//          is the number of rows, times the number of columns, times the size of the cell.
//      viohandle - Presentation-space handle. This must be 0, unless the
//          caller is a Presentation Manager application; in this case,
//          it must be the value returned by VioCreatePS.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//
int
VioGetBuf(VIOCELL **pbuf, ULONG *plength, HVIO viohandle)
{
    __CUNUSED(viohandle)

    assert(pbuf && plength);
    assert(0 == viohandle);

    if (!pbuf || !plength) {
        return ERROR_VIO_INVALID_PARMS;
    } else if (viohandle) {
        return ERROR_VIO_INVALID_HANDLE;
    }

    VioInitialise();                            // (re)initialise

    assert(vio.size);
    assert(vio.image);

    if (pbuf) *pbuf = vio.image;
    if (plength) *plength = vio.size * sizeof(VIOCELL);
    return 0;
}


//  Function:           VioShowBuf
//      Updates the physical display buffer with the logical video buffer (LVB).
//
//  Parameters:
//      offSet - Offset into the LVB.
//      length - Length of the area to be updated.
//      viohandle - presentation-space handle.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//
int
VioShowBuf(ULONG offset, ULONG length, HVIO viohandle)
{
    __CUNUSED(viohandle)

    assert(length && 0 == (offset % sizeof(VIOCELL)) && 0 == (length % sizeof(VIOCELL)));
    assert(0 == viohandle);
    assert(vio.inited);                         /* uninitialised */

    if (0 == length ||
            (offset % sizeof(VIOCELL)) || (length % sizeof(VIOCELL))) {
        return ERROR_VIO_INVALID_PARMS;
    } else if (!vio.inited || NULL == vio.image) {
        return ERROR_VIO_MODE;
    } else if (viohandle) {
        return ERROR_VIO_INVALID_HANDLE;
    } else {
        copyoutctx_t ctx = {0};                 /* update context */
        const int rows = vio.rows, cols = vio.cols;
        int row;

        offset /= sizeof(VIOCELL), length /= sizeof(VIOCELL);

        assert(offset < vio.size);              /* starting position within window */
        assert((offset + length) <= vio.size);  /* end position within window */
        assert(0 == (offset % cols));           /* row aligned */

        CopyOutInit(&ctx);
        CopyOut(&ctx, offset, length, TOUCHED);
        CopyOutFinal(&ctx);

        for (row = offset / cols; row < rows && (int)length >= cols; ++row, length -= cols) {
            vio.c_screen[row].flags = 0;
        }
    }
    return 0;
}


int
VioShowInit(VIOSHOW *show, HVIO viohandle)
{
    __CUNUSED(viohandle)

    assert(show && show->cb == sizeof(VIOSHOW));
    assert(0 == viohandle);
    assert(vio.inited);                         /* uninitialised */

    if (!vio.inited || NULL == vio.image) {
        return ERROR_VIO_MODE;
    } else if (!show || show->cb != sizeof(VIOSHOW)) {
        return ERROR_VIO_INVALID_HANDLE;
    } else {
        copyoutctx_t *ctx = (copyoutctx_t *)show->opaque;

        assert(sizeof(show->opaque) >= sizeof(copyoutctx_t));
        show->state = 1;
        show->handle = viohandle;
        CopyOutInit(ctx);
    }
    return 0;
}



int
VioShowBlock(ULONG offset, ULONG length, VIOSHOW *show)
{
    assert(length && 0 == (offset % sizeof(VIOCELL)) && 0 == (length % sizeof(VIOCELL)));
    assert(show && show->cb == sizeof(VIOSHOW) && 1 == show->state);
    assert(vio.inited);                         /* uninitialised */

    if (0 == length ||
            (offset % sizeof(VIOCELL)) || (length % sizeof(VIOCELL))) {
        return ERROR_VIO_INVALID_PARMS;
    } else if (!vio.inited || NULL == vio.image) {
        return ERROR_VIO_MODE;
    } else if (!show || show->cb != sizeof(VIOSHOW) || 1 != show->state) {
        return ERROR_VIO_INVALID_HANDLE;
    } else {
        copyoutctx_t *ctx = (copyoutctx_t *)show->opaque;
        const int rows = vio.rows, cols = vio.cols;
        int row;

        offset /= sizeof(VIOCELL), length /= sizeof(VIOCELL);

        assert(offset < vio.size);              /* starting position within window */
        assert((offset + length) <= vio.size);  /* end position within window */
        assert(0 == (offset % cols));           /* row aligned */

        CopyOut(ctx, offset, length, TOUCHED);

        for (row = offset / cols; row < rows && (int)length >= cols; ++row, length -= cols) {
            vio.c_screen[row].flags = 0;
        }
    }
    return 0;
}


int
VioShowFinal(VIOSHOW *show)
{
    assert(show && show->cb == sizeof(VIOSHOW) && 1 == show->state);
    assert(vio.inited);                         /* uninitialised */

    if (!vio.inited || NULL == vio.image) {
        return ERROR_VIO_MODE;
    } else if (!show || show->cb != sizeof(VIOSHOW) || 1 != show->state) {
        return ERROR_VIO_INVALID_HANDLE;
    } else {
        copyoutctx_t *ctx = (copyoutctx_t *)show->opaque;

        CopyOutFinal(ctx);
        show->state = 0;
    }
    return 0;
}


//  Function:           VioReadCellStr
//      VioReadCellStr reads a string of character-attribute pairs (cells) from the
//      screen, starting at the specified location.
//
//  Parameters:
//      buf - Buffer address.
//      length - Address of the buffer length in bytes. Length must take into account
//          that each character-attribute entry in the buffer is 2 or 4 bytes.
//          If the length of the buffer is not sufficient, the last entry is not complete.
//      row - Starting row of the field to read, where 0 is the top row.
//      column - Starting column of the field to read, where 0 is the top row.
//      viohandle - Input Vio presentation-space handle.  This must be 0, unless the
//          caller is a Presentation Manager application; in this case it must be the
//          value returned by VioCreatePS.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//
int
VioReadCellStr(VIOCELL *buf, ULONG *plength, USHORT row, USHORT col, HVIO viohandle)
{
    ULONG size = 0, offset;
    int ret = 0;

    __CUNUSED(viohandle)

    assert(buf && plength);
    assert(*plength && 0 == (*plength % sizeof(VIOCELL)));
    assert(0 == viohandle);
    assert(vio.inited);                         /* uninitialised */
    assert(row < vio.rows && col < vio.cols);

    if (NULL == buf || NULL == plength) {
        return ERROR_VIO_INVALID_PARMS;
    } else if (!vio.inited || NULL == vio.image) {
        ret = ERROR_VIO_MODE;
    } else if (!plength || row >= vio.rows || col >= vio.cols ||
                    (offset = (row * col)) >= vio.size) {
        ret = ERROR_VIO_INVALID_PARMS;
    } else if (viohandle) {
        ret = ERROR_VIO_INVALID_HANDLE;
    } else {
        const size_t cnt = (size_t)(*plength) / sizeof(VIOCELL);
        if (cnt) {
            CopyIn(offset, cnt, buf);
            size = *plength;
        }
    }
    *plength = size;
    return ret;
}


/*
 *  DosBeep ---
 *      Beep
 */
void
DosBeep(int freq, int duration)
{
    Beep(freq, duration);
}

#endif // USE_VIO_BUFFER
#endif // WIN32

/*end*/
