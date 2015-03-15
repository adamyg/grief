#include <edidentifier.h>
__CIDENT_RCSID(gr_ttywin32_c,"$Id: ttywin32.c,v 1.41 2015/03/14 23:14:50 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttywin32.c,v 1.41 2015/03/14 23:14:50 ayoung Exp $
 * WIN32 VIO driver.
 *
 *  Notes,
 *      the extended 256 color mode is experimental/work in progress.
 *
 *      Use of non-monospaced fonts are not advised unless UNICODE
 *      characters are required.
 *
 *      Neither wide nor combined characters are fully addressed.
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

#ifdef  WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT    0x0601
#endif
#define WINVER          0x0601

#include <editor.h>

#if (defined(USE_VIO_BUFFER) && defined(WIN32))

#include <libstr.h>                             /* str_...()/sxprintf() */
#include <assert.h>
#include <math.h>                               /* ceil */

#if !defined(WINDOWS_MEAN_AND_LEAN)
#define  WINDOWS_MEAN_AND_LEAN
#include <windows.h>
#include <wincon.h>
#endif

#include "debug.h"
#include "display.h"                            /* DISPTYPE_.. */
#include "main.h"                               /* panic() */
#include "tty.h"                                /* x_pt[] */
#include "ttyrgb.h"
#include "vio.h"                                /* VIO interface */
#include "window.h"

#if ((defined(_MSC_VER) && (_MSC_VER < 1600)) || defined(__MINGW32__)) && \
            !defined(CONSOLE_OVERSTRIKE)
#pragma pack(push, 1)

typedef struct _CONSOLE_FONT_INFOEX {
    ULONG           cbSize;
    DWORD           nFont;
    COORD           dwFontSize;
    UINT            FontFamily;
    UINT            FontWeight;
    WCHAR           FaceName[LF_FACESIZE];
} CONSOLE_FONT_INFOEX, *PCONSOLE_FONT_INFOEX;

typedef struct _CONSOLE_SCREEN_BUFFER_INFOEX {
    ULONG           cbSize;
    COORD           dwSize;
    COORD           dwCursorPosition;
    WORD            wAttributes;
    SMALL_RECT      srWindow;
    COORD           dwMaximumWindowSize;
    WORD            wPopupAttributes;
    BOOL            bFullscreenSupported;
    COLORREF        ColorTable[16];
} CONSOLE_SCREEN_BUFFER_INFOEX, *PCONSOLE_SCREEN_BUFFER_INFOEX;

#if defined(__MINGW32__)                        /* missing */
COORD WINAPI        GetConsoleFontSize(HANDLE hConsoleOutput, DWORD nFont);
BOOL WINAPI         GetCurrentConsoleFont(HANDLE hConsoleOutput, BOOL bMaximumWindow,
                        PCONSOLE_FONT_INFO lpConsoleCurrentFont);
#endif
#pragma pack(pop)
#endif  /*CONSOLE_FONT_INFOEX*/
#endif  /*WIN32*/

#if !defined(VIOCELL_SIZE)
#if defined(WIN32) && defined(WIN32_CONSOLEEXT)
#define VIOCELL_SIZE    8
#define VIOCELL_SHIFT   3
#elif defined(WIN32)
#define VIOCELL_SIZE    4
#define VIOCELL_SHIFT   2
#else
#define VIOCELL_SIZE    2
#define VIOCELL_SHIFT   1
#endif
#endif

#define WIN32_CONSOLE256                        /* enable 256 color console support */

#if defined(WIN32_CONSOLEEXT) && defined(WIN32_CONSOLE256)
#define WIN32_COLORS    256
#else
#undef  WIN32_CONSOLE256
#define WIN32_COLORS    16
#endif

static void             vioinit(void);
static void             viosize(int *nrows, int *ncols);
static void             vioprofile(void);
static void             vioencoding(void);

static void             consolefontsenum(void);
static int              consolefontset(int height, int width, const char *facename);
static HFONT            consolefontcreate(int height, int width, int weight, int italic, const char *facename);

static void             CopyIn(VIOCELL *buffer, size_t offset, size_t len);
static void             CopyOut(const VIOCELL *buffer, size_t offset, size_t len);
#if defined(WIN32_CONSOLEEXT)
#if defined(WIN32_CONSOLE256)
static void             CopyOutEx(const VIOCELL *buffer, size_t pos, size_t cnt);
#endif
static void             UnderlineOut(const VIOCELL *buffer, size_t pos, size_t cnt);
#endif

#define FACENAME_MAX    64
#define FONTS_MAX       64

typedef DWORD  (WINAPI *GetNumberOfConsoleFonts_t)(void);
typedef BOOL   (WINAPI *GetConsoleFontInfo_t)(HANDLE, BOOL, DWORD, CONSOLE_FONT_INFO *);
typedef BOOL   (WINAPI *GetConsoleFontInfoEx_t)(HANDLE, BOOL, DWORD, CONSOLE_FONT_INFOEX *);
typedef BOOL   (WINAPI *GetCurrentConsoleFontEx_t)(HANDLE, BOOL, CONSOLE_FONT_INFOEX *);
typedef BOOL   (WINAPI *SetConsoleFont_t)(HANDLE, DWORD);
typedef BOOL   (WINAPI *SetCurrentConsoleFontEx_t)(HANDLE, BOOL, CONSOLE_FONT_INFOEX *);
typedef BOOL   (WINAPI *GetConsoleScreenBufferInfoEx_t)(HANDLE, CONSOLE_SCREEN_BUFFER_INFOEX *);

static struct {                         // Video state
    //  Resource handles
    //
    HANDLE              cHandle;                // Console handle.
    BOOL		cLocal;			// TRUE/FALSE as to whether the handle is local.
    HANDLE              wHandle;                // Underlying window handle.
    HFONT               fnHandle;               // Current normal font.
    HFONT               fiHandle;               // Current italic font.
    HHOOK		hook;			// Keyboard hook.

    //  Dynamic bindings
    //
    BOOL		getDynamic;		// dynamic lookup status
    GetNumberOfConsoleFonts_t GetNumberOfConsoleFonts;
    GetConsoleFontInfo_t GetConsoleFontInfo;
    GetConsoleFontInfoEx_t GetConsoleFontInfoEx;
    GetCurrentConsoleFontEx_t GetCurrentConsoleFontEx;
    GetConsoleScreenBufferInfoEx_t GetConsoleScreenBufferInfoEx;
    SetConsoleFont_t SetConsoleFont;
    SetCurrentConsoleFontEx_t SetCurrentConsoleFontEx;

    //  Family:
    //      54      Consolas/Lucida Console
    //      48      Terminal
    //  Weight:
    //      400     Normal
    //      700     Bold
    //
    int32_t             fcwidth;
    int32_t             fcheight;
    int32_t             fcfamily;
    int32_t             fcweight;
    uint32_t            fcflags;
    char                fcfacename[LF_FACESIZE+1];
    struct fcname {
        const char *    name;
#define FCNPRO              0x0001              /* Proportional display qualities */
#define FCNPRO5             (FCNPRO|0x02)       /* Proportional, with 50% width scaling */
#define FCNPRO2             (FCNPRO|0x04)       /* Proportional, 2/3 width scaled */
#define FCNUNC              0x100               /* Unicode */
        uint32_t        flags;
        uint32_t        available;
    } fcnames[FACENAME_MAX];

    int                 fontindex;
    int                 fontnumber;
    CONSOLE_FONT_INFOEX fonts[FONTS_MAX];

    int                 maxcolors;
    int                 colors;                 /* Color depth (16 or 256) */
    COLORREF            colors256[256];         /* RGB color table */

    int                 displaymode;            /* Diplay mode (0=Normal,1=Full) */
    int                 cols, rows;             /* Video display */
    int                 isdirty;                /* Screen status 0=clean, otherwise dirty */
    size_t              size;                   /* Size, in video cells */
    uint16_t            style;                  /* Style accumulator */
    VIOCELL            *buffer;                 /* Exposed VIO video buffer (LVB) */
    CHAR_INFO          *image;                  /* Backing image */
    CHAR_INFO          *shadow;                 /* Black&white shadow backing image */
    int                 codepage;               /* Font code page */
    VIOCURSOR           cur_state;              /* Cursor status */
    USHORT              cur_attr;               /* Attribute (0=Off, 1=On) */
    DWORD               cur_mask;               /* Current cursor mask */
} vio;


static void 
ShowLastSystemError()
{
    LPSTR messageBuffer;
    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        0,          // source
        GetLastError(),
        0,          // lang
        (LPSTR)&messageBuffer,
        0,
        NULL);
    MessageBoxA(NULL, messageBuffer, "Error", MB_ICONERROR);
    LocalFree(messageBuffer);
}


/*private*/
//  Function:           vioinit
//      Initialise the video sub-system.
//
//  Parameters:
//      none
//
//  Returns:
//      nothing
//
//  TODO:
//      Computer\HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Command Processor
//      Computer\HKEY_USERS\.DEFAULT\Software\Microsoft\Command Processor
//          CompletionChar
//          DefaultColor
//          EnableExtensions
//          PathCompletionChar
//
//      Computer\HKEY_CURRENT_USER\Console
//          \<profiles>
//              <xxx>
//          ColorTable00 .. ColorTable15
//          ScreenColors
//
static void
vioinit(void)
{
    HANDLE cHandle = 0;
    int nrows, ncols;

    assert(VIOCELL_SIZE == sizeof(VIOCELL));

    if (0 == (cHandle = vio.cHandle)) {
        HMODULE hMod;
        struct rgbvalue rgb = {0};
        int color;

        // console handle
	vio.cLocal = FALSE;
        if ((cHandle = GetStdHandle(STD_OUTPUT_HANDLE)) == INVALID_HANDLE_VALUE ||
                    GetFileType(cHandle) != FILE_TYPE_CHAR) {
            //
            //  stdout has been redirected ... create local console
            //
            SECURITY_ATTRIBUTES sa;

            sa.nLength = sizeof(sa);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle = TRUE;           // inherited
	    vio.cLocal = TRUE;
            cHandle = CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_WRITE | FILE_SHARE_WRITE, &sa, OPEN_EXISTING, 0, 0);
	}
        vio.cHandle = cHandle;

        // default colour table
        for (color = 0; color < 256; ++color) {
            rgb_256win(color, &rgb);            // BBGGRR
            vio.colors256[color] = ((rgb.blue << 16) | (rgb.green << 8) | rgb.red);
        }

        if (0 == vio.colors) {
            int maxcolors = WIN32_COLORS;       // default/max color depth

#if defined(WIN32_CONSOLEEXT) && defined(WIN32_CONSOLE256)
            if (GetModuleHandleA("ConEmuHk.dll") ||
                    GetModuleHandleA("ConEmuHk64.dll")) {
                trace_log("Running under ConEmu, disabling 256 support\n");
                maxcolors = 16;
            }
#endif
            vio.maxcolors = maxcolors;
            vio.colors = maxcolors;
        }

        // undocumented/dynamic function bindings
	if (!vio.getDynamic) {
	    if (0 != (hMod = GetModuleHandleA("Kernel32.dll"))) {
						// resolve
		vio.GetConsoleFontInfo =
		    (GetConsoleFontInfo_t) GetProcAddress(hMod, "GetConsoleFontInfo");
		vio.GetConsoleFontInfoEx =
		    (GetConsoleFontInfoEx_t) GetProcAddress(hMod, "GetConsoleFontInfoEx");
		vio.GetNumberOfConsoleFonts =
		    (GetNumberOfConsoleFonts_t) GetProcAddress(hMod, "GetNumberOfConsoleFonts");
		vio.GetCurrentConsoleFontEx =
		    (GetCurrentConsoleFontEx_t) GetProcAddress(hMod, "GetCurrentConsoleFontEx");
		vio.GetConsoleScreenBufferInfoEx =
		    (GetConsoleScreenBufferInfoEx_t) GetProcAddress(hMod, "GetConsoleScreenBufferInfoEx");
		vio.SetConsoleFont =
		    (SetConsoleFont_t) GetProcAddress(hMod, "SetConsoleFont");
		vio.SetCurrentConsoleFontEx =
		    (SetCurrentConsoleFontEx_t) GetProcAddress(hMod, "SetCurrentConsoleFontEx");

		trace_log("Console Functions\n");
		trace_log("  GetConsoleFontInfo:           %p\n", vio.GetConsoleFontInfo);
		trace_log("  GetConsoleFontInfoEx:         %p\n", vio.GetConsoleFontInfoEx);
		trace_log("  GetNumberOfConsoleFonts:      %p\n", vio.GetNumberOfConsoleFonts);
		trace_log("  GetCurrentConsoleFontEx:      %p\n", vio.GetCurrentConsoleFontEx);
		trace_log("  GetConsoleScreenBufferInfoEx: %p\n", vio.GetConsoleScreenBufferInfoEx);
		trace_log("  SetConsoleFont:               %p\n", vio.SetConsoleFont);
		trace_log("  SetConsoleFontEx:             %p\n", vio.SetCurrentConsoleFontEx);
	    }
	    vio.getDynamic = TRUE;
	}
    }

    viosize(&nrows, &ncols);

    if (vio.cols != ncols || vio.rows != nrows) {
        const int orows = vio.rows, ocols = vio.cols;
        const VIOCELL *obuffer = vio.buffer;
        const CHAR_INFO *oimage = vio.image;
        const int nsize = nrows * ncols;
        VIOCELL *nbuffer = NULL;
        CHAR_INFO *nimage = NULL;
        CHAR_INFO *nshadow = NULL;

        //  SetConsoleOutputCP()
        //      UTF7                = 0xfde8    65000
        //      UTF8                = 0xfde9    65001
        //      UTF16               = 0x4b0     1200
        //      UTF16_BIGENDIAN     = 0x4b1     1201
        //      UTF32               =           12000
        //      UTF32_BIGENDIAN     =           12001
        //
        //  EnumSystemCodePages()
        //      test for support
        //
        if (0 == (vio.codepage = GetConsoleOutputCP())) {
            vio.codepage = 437;
        }
        vioencoding();

        if (NULL == (nbuffer = (VIOCELL *)chk_alloc(nsize * sizeof(VIOCELL))) ||
                NULL == (nimage = (CHAR_INFO *)chk_alloc(nsize * sizeof(CHAR_INFO))) ||
                   NULL == (nshadow = (CHAR_INFO *)chk_alloc(nsize * sizeof(CHAR_INFO)))) {
            if (NULL == obuffer || NULL == oimage) {
                panic("vio - no memory available");
            }
            chk_free((void *)nimage);
            chk_free((void *)nbuffer);
            return;
        }

        if (obuffer) {
            const int cnt = (ocols <= ncols ? ocols : ncols);
            const VIOCELL blank = VIO_INIT(VIO_FGBG(WHITE, BLACK), ' ');
            VIOCELL *cursor;
            int r;

            for (r = 0; r < nrows; ++r) {       /* transfer image and clear */
                int c = 0;
                if (r < orows) {
                    memcpy(nbuffer + (r * ncols), obuffer + (r * ocols), cnt * sizeof(VIOCELL));
                    if ((c = cnt) == ncols) {
                        continue;
                    }
                }
                cursor = nbuffer + (r * ncols) + c;
                while (c++ < ncols) {
                    *cursor++ = blank;
                }
            }
            chk_free((void *)obuffer);
        }

        if (oimage) {
            const int cnt = (ocols <= ncols ? ocols : ncols);
            const CHAR_INFO blank = {' ', FOREGROUND_INTENSITY};
            CHAR_INFO *cursor;
            int r;

            for (r = 0; r < nrows; ++r) {       /* transfer image and clear */
                int c = 0;
                if (r < orows) {
                    memcpy(nimage + (r * ncols), oimage + (r * ocols), cnt * sizeof(CHAR_INFO));
                    if ((c = cnt) == ncols) {
                        continue;
                    }
                }
                cursor = nimage + (r * ncols) + c;
                while (c++ < ncols) {
                    *cursor++ = blank;
                }
            }
            chk_free((void *)oimage);
            chk_free((void *)vio.shadow);
        }

        vio.size   = nsize;
        vio.rows   = nrows;
        vio.cols   = ncols;
        vio.image  = nimage;
        memcpy(nshadow, (const void *)nimage, nsize * sizeof(CHAR_INFO));
        vio.shadow = nshadow;
        vio.buffer = nbuffer;

        if (NULL == obuffer) {
            CopyIn(vio.buffer, 0, nsize);
        }
    }
}


static void
vioclose(void)
{
    if (vio.cLocal) {
	CloseHandle(vio.cHandle);
    }
    vio.cLocal = FALSE;
    vio.cHandle = 0;
}


static void
viosize(int *rows, int *cols)
{
    vioprofile();

    if (vio.cHandle) {
        CONSOLE_SCREEN_BUFFER_INFO sbinfo = {0};

        GetConsoleScreenBufferInfo(vio.cHandle, &sbinfo);
        *rows = 1 + sbinfo.srWindow.Bottom - sbinfo.srWindow.Top;
        *cols = 1 + sbinfo.srWindow.Right - sbinfo.srWindow.Left;
    } else {
        *rows = 25;
        *cols = 80;
    }
}


static void
vioprofile(void)
{
    DWORD consolemode = 0;
    HANDLE cHandle;

    if (0 == (cHandle = vio.cHandle)) {
        return;
    }
    trace_log("vioprofile()\n");

    //  basic
    //
    vio.wHandle = GetConsoleWindow();		// underlying console window handle
    vio.displaymode =                           // 0=Normal or 1=Full-screen mode
        (GetConsoleDisplayMode(&consolemode) && consolemode ? 1 : 0);

    //  font configuration
    //
    if (NULL == vio.fcnames[0].name) {
        consolefontsenum();
    }

    if (0 == vio.fontnumber) {
        // dynamic colors
        if (vio.GetConsoleScreenBufferInfoEx) { // vista+
            CONSOLE_SCREEN_BUFFER_INFOEX csbix = {0};
            int i;

            csbix.cbSize = sizeof(csbix);
            vio.GetConsoleScreenBufferInfoEx(cHandle, &csbix);
            trace_log("Console Colors (BBGGRR)\n");
            for (i = 0; i < 16; ++i) {
                trace_log("  [%2d] 0x%06x\n", i, (unsigned)csbix.ColorTable[i]);
                vio.colors256[i] = csbix.ColorTable[i];
            }
        }

        // current fonts
        if (vio.GetCurrentConsoleFontEx) {
            CONSOLE_FONT_INFOEX cfix = {0};

            cfix.cbSize = sizeof(cfix);
            if (vio.GetCurrentConsoleFontEx(cHandle, FALSE, &cfix)) {
                COORD coord;

                vio.fontindex = cfix.nFont;
                coord = GetConsoleFontSize(cHandle, cfix.nFont);
                vio.fcheight  = coord.Y;        // cfix.dwFontSize.Y;
                vio.fcwidth   = coord.X;        // cfix.dwFontSize.X;
                vio.fcfamily  = cfix.FontFamily;
                vio.fcweight  = cfix.FontWeight;
                vio.fcfamily  = -1;
                wcstombs(vio.fcfacename, cfix.FaceName, sizeof(vio.fcfacename));
            }

        } else {
            CONSOLE_FONT_INFO cfi = {0};

            if (GetCurrentConsoleFont(cHandle, FALSE, &cfi)) {
                COORD coord;

                vio.fontindex = cfi.nFont;
                coord = GetConsoleFontSize(cHandle, cfi.nFont);
                vio.fcheight  = coord.Y;        // cfi.dwFontSize.Y;
                vio.fcwidth   = coord.X;        // cfi.dwFontSize.X;
                vio.fcfamily  = -1;
                vio.fcweight  = -1;
                vio.fcfacename[0] = 0;
            } else {
                vio.fontindex = -1;             // full screen
                vio.fcheight  = 16;
                vio.fcwidth   =  8;
                vio.fcweight  = -1;
                vio.fcfacename[0] = 0;
            }
        }

        trace_log("Current Font: Idx:%d, %dx%d, Family:%d, Weight:%d, Mode:%d, Name:<%s>\n",
        vio.fontindex, vio.fcwidth, vio.fcheight,
                vio.fcfamily, vio.fcweight, vio.displaymode, vio.fcfacename);

        // available fonts
        vio.fontnumber = -1;
        if (vio.GetConsoleFontInfo && vio.GetNumberOfConsoleFonts) {
            CONSOLE_FONT_INFOEX *fonts = vio.fonts;
            CONSOLE_FONT_INFO t_fonts[FONTS_MAX] = {0};
            DWORD fontnumber = vio.GetNumberOfConsoleFonts();

            if (fontnumber > FONTS_MAX) {
                fontnumber = FONTS_MAX;
            }

            if (vio.GetConsoleFontInfoEx) {     // extension
                fonts->cbSize = sizeof(vio.fonts);
                if (vio.GetConsoleFontInfoEx(cHandle, 0, fontnumber, fonts)) {
                    vio.fontnumber = fontnumber;
                }

            } else if (vio.GetConsoleFontInfo(cHandle, 0, fontnumber, t_fonts)) {
                DWORD f;                        // xp+

                for (f = 0; f < fontnumber; ++f, ++fonts) {
                    fonts->nFont = t_fonts[f].nFont;
                    fonts->dwFontSize = GetConsoleFontSize(cHandle, fonts->nFont);
                }
                vio.fontnumber = fontnumber;
            }

            if (fontnumber) {
                //
                //  Generally the first entry represents a <Terminal> entry,
                //  with secondary elements representing True-Type fonts.
                //
                //    Idx    W x  H   Fam   Wgt  Facename
                //      0:   4 x  6,    0,    0, <Terminal>
                //      1:   6 x  8,    0,    0, <Terminal>
                //      2:   8 x  8,    0,    0, <Terminal>
                //      3:  16 x  8,    0,    0, <Terminal>
                //      4:   5 x 10,    0,    0, <Consolas> [Normal]
                //      5:   5 x 10,    0,    0, <Consolas> [Bold]
                //      6:   5 x 12,    0,    0, <Terminal>
                //      7:   7 x 12,    0,    0, <Terminal>
                //      8:   8 x 12,    0,    0, <Terminal>
                //      9:  16 x 12,    0,    0, <Terminal>
                //      10:  8 x 16,    0,    0, <>
                //      11:  8 x 16,    0,    0, <>
                //      12: 12 x 16,    0,    0, <Terminal>
                //      13:  8 x 18,    0,    0, <>
                //      14:  8 x 18,    0,    0, <>
                //      15: 10 x 18,    0,    0, <Terminal>
                //
                //  Note: The font table is console session specific.
                //
                DWORD f;

                trace_log("Console Facenames (%u)\n", (unsigned)fontnumber);
                for (f = 0, fonts = vio.fonts; f < fontnumber; ++f, ++fonts) {
                    char t_facename[32] = {0};
                    wcstombs(t_facename, fonts->FaceName, sizeof(t_facename));
                    trace_log("  %2d: %2u x %2u, %4u, %4u, <%s>\n", (int)fonts->nFont,
                            (unsigned)fonts->dwFontSize.X, (unsigned)fonts->dwFontSize.Y,
                            (unsigned)fonts->FontFamily, (unsigned)fonts->FontWeight,
                            t_facename);
                }
            }
        }
    }
}


/*private*/
//  Function:           vioencoding
//      Derive the console encoding method from the code-page.
//
//  Parameters:
//      none
//
//  Returns:
//      nothing
//
static void
vioencoding(void)
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
    const int cp = vio.codepage;
    char encoding[32];
    CPINFOEX cpix = {0};
    unsigned cpd;

    if (cp <= 0 || 0 == GetCPInfoEx(cp, 0, &cpix)) {
        trace_log("vioencoding: unable to resolve CP%03d\n", cp);
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
        trace_log("vioencoding: type:%d\n", xf_disptype);
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
//      info -              Mode characteristics.
//      viohandle -         VIO presentation-space handle.
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
    if (NULL == vio.buffer) {
        vioinit();
        if (info) {
            info->row = (USHORT)vio.rows;
            info->col = (USHORT)vio.cols;
            info->color = (USHORT)vio.colors;
        }

    } else if (info) {
        int nrows, ncols;

        viosize(&nrows, &ncols);
        info->row = (USHORT)nrows;
        info->col = (USHORT)ncols;
        info->color = (USHORT)vio.colors;
    }
    return 0;
}


//  Function:           VioSetMode
//      Sets the mode of the display.
//
//  Parameters:
//      info -              Mode characteristics.
//      viohandle -         VIO presentation-space handle.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//

static void
SetSize(int rows, int cols)
{
    HANDLE cHandle = vio.cHandle;
    int orows = vio.rows, ocols = vio.cols;
    SMALL_RECT rect = {0, 0, 0, 0};
    COORD msize, nbufsz;
    int bufwin = FALSE;

    msize = GetLargestConsoleWindowSize(cHandle);

    if (rows <= 0) rows = orows;                // current
    else if (rows >= msize.Y) rows = msize.Y-1;	// limit

    if (cols <= 0) cols = ocols;		// current
    else if (cols >= msize.X) cols = msize.X-1;	// limit

    rect.Top    = 0;
    rect.Bottom = (SHORT)(rows - 1);
    rect.Left   = 0;
    rect.Right  = (SHORT)(cols - 1);

    nbufsz.Y    = (SHORT)rows;
    nbufsz.X    = (SHORT)cols;

    if (orows <= rows) {
        if (ocols <= cols) {                    // +cols, +rows
            bufwin = TRUE;

        } else {                                // -cols, -rows
            SMALL_RECT nwinsz = {0, 0, (SHORT)(cols - 1), (SHORT)(orows - 1)};
            SetConsoleWindowInfo(cHandle, TRUE, &nwinsz);
            bufwin = TRUE;
        }
    } else {
        if (ocols <= cols) {                    // +cols, -rows
            SMALL_RECT nwinsz = {0, 0, (SHORT)(ocols - 1), (SHORT)(rows- 1)};
            SetConsoleWindowInfo(cHandle, TRUE, &nwinsz);
            bufwin = TRUE;

        } else {                                // -cols, -rows
            SetConsoleWindowInfo(cHandle, TRUE, &rect);
            SetConsoleScreenBufferSize(cHandle, nbufsz);
        }
    }

    if (bufwin) {                               // set buffer and window
        SetConsoleScreenBufferSize(cHandle, nbufsz);
        SetConsoleWindowInfo(cHandle, TRUE, &rect);
    }
}


int
VioSetMode(VIOMODEINFO *info, HVIO viohandle)
{
    if (info) {					// ega_switch support (12/10/2014)
	if (info->row || info->col) {
            SetSize(info->row, info->col);
	    vioclose();
        }
    }
    vioinit();
    if (info && (info->row || info->col)) {
	if (0xffff == info->row || 0xffff == info->col) {
	    HWND hWnd = vio.wHandle;
	    RECT r;

	    GetWindowRect(hWnd, &r);
	    ShowWindow(hWnd, SW_MAXIMIZE);
	    MoveWindow(hWnd, 0, 0, r.right, r.bottom, TRUE);
	} else {
	    ShowWindow(vio.wHandle, /*SW_RESTORE*/ SW_NORMAL);
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
//      Set the color depth to either 16 or 256.
//
//  Parameters:
//      colors -            Depth depth.
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
        if (256 == vio.maxcolors) {
            vio.colors = 256;
            return TRUE;
        }
        break;
#endif
    case 16:
        vio.colors = 16;
        return TRUE;
    }
    return FALSE;
}


//  Function:           VioGetColors
//      Retrieve the color depth to either 16 or 256.
//
//  Parameters:
//      colors -            Depth depth.
//
//  Returns:
//      nothing.
//
int
VioGetColors(int *colors)
{
    if (colors) {
        *colors = vio.colors;
    }
    return 0;
}


//  Function:           VioGetCp
//      Queries the code page currently used to display text data.
//
//  Parameters:
//      reserved -          Reserved; must be 0.
//      cp -                Returned Code-page ID.
//      vioHandle -         Presentation-space handle.
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
//      fontname -          Fontname.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//
static int              selectconsolefont(int nFont);

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

                // TODO
		if (vio.SetCurrentConsoleFontEx && (0)) {
		    CONSOLE_FONT_INFOEX font = {0};

		    font.FontFamily = 54;	// Lucida console font
		    font.FontWeight = 400;	// normal
		    font.dwFontSize.X = (SHORT)w;
		    font.dwFontSize.Y = (SHORT)h;
		    vio.SetCurrentConsoleFontEx(vio.cHandle, FALSE, &font);
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
    return 421;     //ERROR_VIO_INVALID_PARMS;
}


static int
selectconsolefont(int nFont)
{
    if (vio.SetConsoleFont(vio.cHandle, nFont)) {
        InvalidateRect(vio.wHandle, 0, FALSE);
        UpdateWindow(vio.wHandle);
        return 0;
    }
    return -1;
}


//  Function:           VioSetFont
//      Set the current console font.
//
//  Parameters:
//      fontname -          Fontname.
//
//  Returns:
//      0   -   NO_ERROR
//
int
VioGetFont(char *font, int buflen)
{
    if (font && buflen > 0) {
        sxprintf(font, buflen, "%s %dx%d", vio.fcfacename, vio.fcwidth, vio.fcheight);
        return 0;
    }
    return 421;     //ERROR_VIO_INVALID_PARMS;
}


//  Function:           VioCursor
//
//  Parameters:
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
    unsigned mask;

    if (state == VIOCUR_STATE) {
        state = vio.cur_state;
    } else {
        switch (state) {                        // new state
        case VIOCUR_THALF: mask = 50; break;
        case VIOCUR_BHALF: mask = 50; break;
        case VIOCUR_FULL: mask = 100; break;
        case VIOCUR_OFF: mask = 0; break;
        case VIOCUR_ON:
        default:
            mask = 25;
        }
        if (vio.cur_attr) {                     // change
            CONSOLE_CURSOR_INFO cinfo;

            cinfo.bVisible = TRUE;
            cinfo.dwSize = (vio.cur_mask = mask);
            SetConsoleCursorInfo(vio.cHandle, &cinfo);
        }
        vio.cur_mask = mask;
        vio.cur_state = state;
    }
    return (state);
}


//  Function:           VioGetCurType
//      Retrieve the cursor type.
//
//  Parameters:
//      info -              Cursor characteristics.
//      viohandle -         Presentation-space handle.
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
    GetConsoleCursorInfo(vio.cHandle, &cinfo);
    vio.cur_mask = cinfo.dwSize;
    vio.cur_attr = (USHORT) cinfo.bVisible;
    if (info) {
        info->mask = vio.cur_mask;
        info->attr = vio.cur_attr;
    }
    return 0;
}


//  Function:           VioSetCurType
//      Set the cursor type.
//
//  Parameters:
//      info -              Cursor information.
//      viohandle -         VIO presentation-space handle.
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
    if (info->attr != vio.cur_attr || info->mask != vio.cur_mask) {
        CONSOLE_CURSOR_INFO cinfo;

        cinfo.bVisible = (vio.cur_attr = info->attr);
        cinfo.dwSize = (vio.cur_mask = info->mask);
        SetConsoleCursorInfo(vio.cHandle, &cinfo);
    }
    return 0;
}


//  Function:           VioGetCurPos
//      Retrieve the coordinates of the cursor.
//
//  Parameters:
//      row -               Row return data.
//      col -               Column return data.
//      vioHandle -         Presentation-space handle.
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
    CONSOLE_SCREEN_BUFFER_INFO sbinfo;

    __CUNUSED(viohandle)
    GetConsoleScreenBufferInfo(vio.cHandle, &sbinfo);
    if (col) *col = sbinfo.dwCursorPosition.X;
    if (row) *row = sbinfo.dwCursorPosition.Y;
    return 0;
}


//  Function:           VioSetCurPos
//      Set the coordinates of the cursor.
//
//  Parameters:
//      row -               Row return data.
//      col -               Column return data.
//      vioHandle -         Presentation-space handle.
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
    HANDLE cHandle = vio.cHandle;
    CONSOLE_SCREEN_BUFFER_INFO csbi = {0};
    COORD coord;

    __CUNUSED(viohandle)
    GetConsoleScreenBufferInfo(cHandle, &csbi);
    if (csbi.srWindow.Left > 0 || csbi.srWindow.Top > 0) {
        coord.X = 0; coord.Y = 0;               // home console
        SetConsoleCursorPosition(vio.cHandle, coord);
        ++vio.isdirty;
    }

    if (vio.isdirty) {
#if defined(WIN32_CONSOLEEXT)
#if defined(WIN32_CONSOLE256)
        if ((VIO_COLOR256|VIO_ITALIC) & vio.style) {
            CopyOutEx(vio.buffer, 0, vio.size);
        }
#endif
        if (VIO_UNDERLINE & vio.style) {
            UnderlineOut(vio.buffer, 0, vio.size);
        }
#endif
        vio.isdirty = 0;
    }
    coord.X = col; coord.Y = row;               // true position
    SetConsoleCursorPosition(vio.cHandle, coord);
    return 0;
}


//  Function:           VioSetFocus
//      Register a window focus event.
//
//  Parameters:
//      setfocus -          Focus status.
//
//  Returns:
//      nothing.
//
int
VioSetFocus(int setfocus)
{
    if (setfocus) {
        ++vio.isdirty;
    }
    return 0;
}


//  Function:           VioGetBuf
//      VioGetBuf returns the address of the logical video buffer (LVB).
//
//  Parameters:
//      pbuf -              Pointer to the logical video buffer address.
//      plength -           Pointer to the length of the buffer, in bytes. The length
//                          is the number of rows, times the number of columns,
//                          times the size of the cell.
//      viohandle -         Presentation-space handle. This must be 0, unless the
//                          caller is a Presentation Manager application; in this case,
//                          it must be the value returned by VioCreatePS.
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
    if (NULL == pbuf || NULL == plength) {
        return 421;
    }
    vioinit();
    assert(vio.size);
    assert(vio.buffer);
    *pbuf = vio.buffer;
    *plength = vio.size * sizeof(VIOCELL);
    return 0;
}


//  Function:           VioShowBuf
//      Updates the physical display buffer with the logical video buffer (LVB).
//
//  Parameters:
//      offSet -            Offset into the LVB.
//      length -            Length of the area to be updated.
//      viohandle -         presentation-space handle.
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
    if (0 == length) {
        return 421;
    } else if (NULL == vio.buffer) {
        return 355;
    }
    CopyOut(vio.buffer, offset >> VIOCELL_SHIFT, length >> VIOCELL_SHIFT);
    return 0;
}


//  Function:           VioReadCellStr
//      VioReadCellStr reads a string of character-attribute pairs (cells) from the
//      screen, starting at the specified location.
//
//  Parameters:
//      buf -
//      length -            Address of the buffer length in bytes. Length must take into account
//                          that each character-attribute entry in the buffer is 2 or 4 bytes.
//                          If the length of the buffer is not sufficient, the last entry
//                          is not complete.
//      row -               Starting row of the field to read, where 0 is the top row.
//      column -            Starting column of the field to read, where 0 is the top row.
//      viohandle -         Input Vio presentation-space handle.  This must be 0, unless the caller
//                          is a Presentation Manager application; in this case it must be the
//                          value returned by VioCreatePS.
//
//  Returns:
//      0   -   NO_ERROR
//      355 -   ERROR_VIO_MODE
//      421 -   ERROR_VIO_INVALID_PARMS
//      430 -   ERROR_VIO_ILLEGAL_DURING_POPUP
//      436 -   ERROR_VIO_INVALID_HANDLE
//
int
VioReadCellStr(VIOCELL *buf, ULONG *length, USHORT row, USHORT col, HVIO viohandle)
{
    ULONG size = 0, offset;
    int ret = 0;

    __CUNUSED(viohandle)
    if (NULL == buf || NULL == length) {
        return 421;
    }

    if (NULL == vio.buffer) {
        ret = 355;
    } else if (row >= vio.rows || col >= vio.cols) {
        ret = 421;
    } else if ((offset = (row * col)) >= vio.size) {
        ret = 436;
    } else {
        size_t cnt = (size_t)(*length) >> VIOCELL_SHIFT;
        if (cnt) {
            CopyIn(buf, offset, cnt);
        }
    }
    *length = size;
    return ret;
}


/*private*/
/*  Function;           CopyOut
 *      Copy out to video memory.
 *
 *  Parameters
 *      buffer -            Video buffer base address.
 *      pos -               Starting offset in video cells from top left.
 *      cnt -               Video cell count.
 *
 *  Returns:
 *      nothing.
 */
static void
CopyOut(const VIOCELL *buffer, size_t pos, size_t cnt)
{
    const int rows = vio.rows, cols = vio.cols;
    HANDLE cHandle = vio.cHandle;
    const VIOCELL *t_buffer, *t_end;
    CHAR_INFO *image = vio.image + pos,
        *shadow = vio.shadow + pos;
    COORD imgSize, imgCoord;
#if defined(WIN32_CONSOLEEXT)
    uint16_t style = 0;
#endif
    SMALL_RECT rect;

    assert(pos < vio.size);
    assert(0 == (pos % cols));
    assert((pos + cnt) <= vio.size);

    rect.Left   = 0;                            // destination screen rectangle
    rect.Right  = (SHORT)(cols - 1);
    rect.Top    = (SHORT)(pos / cols);
    rect.Bottom = (SHORT)((pos + (cnt - 1)) / cols);
    assert(rect.Bottom < rows);
    assert(pos == (size_t)(rect.Top * cols));

    imgCoord.X  = 0;                            // top left src cell in image
    imgCoord.Y  = 0;
    imgSize.Y   = (SHORT)(rows - rect.Top);     // size of image
    imgSize.X   = (SHORT) cols;

    t_buffer = buffer + pos, t_end = t_buffer + cnt;
    while (t_buffer < t_end) {
        const VIOCELL cell = *t_buffer++;

        // primary image
        image->Attributes = ((WORD)VIO_EXPORT(cell));
        image->Char.UnicodeChar = (WCHAR)VIO_CHAR(cell);
        ++image;

        // shadow black & white image, written to console when 256 colour mode.
        shadow->Attributes = FOREGROUND_INTENSITY;
        shadow->Char.UnicodeChar = (WCHAR)VIO_CHAR(cell);
        ++shadow;

#if defined(WIN32_CONSOLEEXT)
#if defined(WIN32_CONSOLE256)
        if (((0xf0 << VIO_BG_SHIFT)|(0xf0 << VIO_FG_SHIFT)) & cell.attribute) {
            style |= VIO_COLOR256;              // extended colour
        }
#endif
        style |= cell.style;
#endif
    }

#if !defined(WIN32_CONSOLE256)                  // MCHAR
    WriteConsoleOutputW(cHandle, vio.image + pos, imgSize, imgCoord, &rect);
#endif

#if defined(WIN32_CONSOLEEXT)
    if (0 == style && 0 == vio.style) {		// initialise accumulator
	t_buffer = buffer, t_end = t_buffer + vio.size;
        while (t_buffer < t_end) {              // accumulate extended colors/styles
	    if (((0xf0 << VIO_BG_SHIFT)|(0xf0 << VIO_FG_SHIFT)) & t_buffer->attribute) {
		style |= VIO_COLOR256;
            }
            style |= t_buffer->style;
            ++t_buffer;
        }
    }
    vio.style |= style;				// accumulate styles

#if defined(WIN32_CONSOLE256)
    if (256 == vio.maxcolors && ((VIO_COLOR256|VIO_ITALIC) & vio.style)) {
        //
        //  cursor-get
        //  cursor-hide
        //  updates-disable         (stop screen update flicker)
        //      console-write-std
        //  update-enable
        //  console-write-extended
        //  cursor-show
        //
        CONSOLE_CURSOR_INFO cinfo = {0};
        BOOL omode;
						// update console buffer
        GetConsoleCursorInfo(vio.cHandle, &cinfo);
        if (0 != (omode = cinfo.bVisible)) {
            cinfo.bVisible = FALSE;             // hide cursor
            SetConsoleCursorInfo(cHandle, &cinfo);
        }
                                                // flush changes, disable updates
	SendMessage(vio.wHandle, WM_SETREDRAW, FALSE, 0);
	WriteConsoleOutputW(cHandle, vio.shadow + pos, imgSize, imgCoord, &rect);
	SendMessage(vio.wHandle, WM_SETREDRAW, TRUE, 0);

	CopyOutEx(buffer, pos, cnt);		// export text
	if (0 != (cinfo.bVisible = omode)) {	// restore cursor
	    SetConsoleCursorInfo(cHandle, &cinfo);
        }

    } else {
        WriteConsoleOutputW(cHandle, vio.image + pos, imgSize, imgCoord, &rect);
    }
#endif  //CONSOLE256

    if (VIO_UNDERLINE & style) {
        UnderlineOut(buffer, pos, cnt);         // underline region
    }
#endif  //CONSOLEEXT
}


#if defined(WIN32_CONSOLE256)
/*private*/
/*  Function:           SameAttribute
 *      Determine whether the same attribute,  ignore foreground colors for spaces 
 *	allowing correct italic display.
 */
static __CINLINE int
IsSpace(const uint32_t ch)
{
    switch (ch) {
    case ' ':    // SPACE
    case 0x00A0: // NO-BREAK SPACE
    case 0x2000: // EN QUAD
    case 0x2001: // EM QUAD
    case 0x2002: // EN SPACE
    case 0x2003: // EM SPACE
    case 0x2004: // THREE-PER-EM SPACE
    case 0x2005: // FOUR-PER-EM SPACE
    case 0x2006: // SIX-PER-EM SPACE
    case 0x2007: // FIGURE SPACE
    case 0x2008: // PUNCTUATION SPACE
    case 0x2009: // THIN SPACE
    case 0x200A: // HAIR SPACE
    case 0x200B: // ZERO WIDTH SPACE
    case 0x202F: // NARROW NO-BREAK SPACE
        return 1;
    }
    return 0;
}


static __CINLINE int
SameAttributes(const VIOCELL cell, const uint32_t attribute, const uint32_t style)
{
    if (IsSpace(cell.character)) {  
        return ((cell.attribute & VIO_BK_MASK) == (attribute & VIO_BK_MASK));
    }
    return ((cell.attribute == attribute) && (cell.style & ~VIO_UNDERLINE) == style);
}


/*private*/
/*  Function:           TextOut
 *      Export characters within the specified region to the console window.
 *
 *  Parameters
 *      buffer -            Video buffer base address.
 *      pos -               Starting offset in video cells from top left.
 *      cnt -               Video cell count.
 *
 *  Returns:
 *      nothing.
 */
static void
CopyOutEx(const VIOCELL *buffer, size_t pos, size_t cnt)
{
    const VIOCELL *t_buffer, *t_end;
    HANDLE wHandle = vio.wHandle;
    const int cols = vio.cols;
    float fcwidth, fcheight;                    // proportional sizing
    int row = pos / cols;
    RECT rect = {0};
    WCHAR text[1024];				// ExtTextOut limit 8192
    HFONT oldfont;
    HDC wdc;

    assert(pos < vio.size);
    assert(0 == (pos % cols));
    assert((pos + cnt) <= vio.size);

    wdc = GetDC(wHandle);                       // client area DC
    GetClientRect(wHandle, &rect);
    fcwidth = (float)rect.right / vio.cols;
    fcheight = (float)rect.bottom / vio.rows;

    if (! vio.fnHandle) {                       // allocate font handle
        consolefontset(-1, -1, vio.fcfacename);
        if (! vio.fnHandle) {
            return;
        }
    }
    oldfont = SelectObject(wdc, vio.fnHandle);

    t_buffer = buffer + pos, t_end = t_buffer + cnt;
    do {
	uint32_t attribute = 0xff000000, style = 0;

        int start = -1, col = 0, len = 0;

        while (col < cols) {
	    while (col < cols) {
                const VIOCELL cell = *t_buffer++;

		if (start >= 0) {
		    if (SameAttributes(cell, attribute,  style)) {
			text[len] = (WCHAR)cell.character;
			++col;
			if (++len >= (int)(sizeof(text)/sizeof(text[0]))-1) {
                            break;		// flush
	                }
	                continue;
		    } else {
			--t_buffer;
			break;			// flush
		    }
		}
                text[0] = (WCHAR)cell.character;
                attribute = cell.attribute;
                style = (~VIO_UNDERLINE) & cell.style;
                start = col++;
                len = 1;
	    }

            if (start >= 0) {			// write text
                const COLORREF bg = vio.colors256[(attribute >> VIO_BG_SHIFT) & VIO_FB_MASK],
                        fg = vio.colors256[(attribute >> VIO_FG_SHIFT) & VIO_FB_MASK];

		if (vio.fiHandle) {
		    SelectObject(wdc, (VIO_ITALIC & style) ? vio.fiHandle : vio.fnHandle);
		}
		text[len] = 0;

                if (FCNPRO & vio.fcflags) {
                    //
                    //  Variable width font,
                    //      display character-by-character.
                    //
                    const int left = (int)(fcwidth * start);
                    const int top = (int)(fcheight * row);
                    HPEN oldbrush, oldpen;
                    const WCHAR *ch = text;
                    int idx;

                    oldbrush = SelectObject(wdc, CreateSolidBrush(bg));
                    oldpen = SelectObject(wdc, CreatePen(PS_SOLID, 0, bg));
                    Rectangle(wdc, left, top, left + (int)(fcwidth * len), top + (int)fcheight);
                    oldpen = SelectObject(wdc, oldpen);
                    oldbrush = SelectObject(wdc, oldbrush);
                    DeleteObject(oldpen);
                    DeleteObject(oldbrush);

		    SetBkColor(wdc, bg);
		    SetTextColor(wdc, fg);
                    SetTextAlign(wdc, GetTextAlign(wdc) | TA_CENTER);
                    for (idx = 0; idx < len; ++idx) {
			const WCHAR t_ch = *ch++;
			if (t_ch && !IsSpace(t_ch)) {
			    ExtTextOutW(wdc, left + (int)(fcwidth * (0.5 + idx)), top, ETO_OPAQUE, NULL, &t_ch, 1, NULL);
			}
                    }

                } else {
                    const int left = vio.fcwidth * start;
                    const int top = vio.fcheight * row;

		    SetBkColor(wdc, bg);
		    SetTextColor(wdc, fg);
                    ExtTextOutW(wdc, left, top, ETO_OPAQUE, NULL, text, len, NULL);
                }
                start = -1;
            }
        }
        ++row;
    } while (t_buffer < t_end);

    SelectObject(wdc, oldfont);
    DeleteDC(wdc);
}
#endif  //CONSOLE256


#if defined(WIN32_CONSOLEEXT)
/*private*/
/*  Function:           UnderlineOut
 *      Underline characters within the specified region.
 *
 *  Parameters
 *      buffer -            Video buffer base address.
 *      pos -               Starting offset in video cells from top left.
 *      cnt -               Video cell count.
 *
 *  Returns:
 *      nothing.
 */
static void
UnderlineOut(const VIOCELL *buffer, size_t pos, size_t cnt)
{
    const int cols = vio.cols;
    HANDLE wHandle = vio.wHandle;
    const VIOCELL *t_buffer, *t_end;
    float fcwidth, fcheight;                    // proportional sizing
    RECT rect = {0};
    int row = pos / cols;
    HDC wdc;

    if (256 != vio.maxcolors) {
        return;
    }

    wdc = GetDC(wHandle);                       // client area DC
    GetClientRect(wHandle, &rect);
    fcwidth = (float)rect.right / vio.cols;
    fcheight = (float)rect.bottom / vio.rows;

    t_buffer = buffer + pos, t_end = t_buffer + cnt;
    do {
        int start = -1, col = 0;
        uint32_t fg = 0xff;

        while (col < cols) {
            while (col < cols) {
                const VIOCELL cell = *t_buffer++;

                if (VIO_UNDERLINE & cell.style) {
                    if (start < 0) {            // inside
                        fg = (cell.attribute >> VIO_FG_SHIFT) & VIO_FB_MASK;
                        start = col;
                    }
                } else if (start >= 0) {
                    ++col;
                    break;                      // flush
                }
                ++col;
            }

            if (start >= 0) {                   // underline current extent
                const int y = (int)(fcheight * (row + 1)) - 1,
                        x = (int)(fcwidth * start);
                HPEN oldpen;

                oldpen = SelectObject(wdc, CreatePen(PS_SOLID, 0, vio.colors256[fg]));
                MoveToEx(wdc, x, y, NULL);
                LineTo(wdc, x + (int)(fcwidth * (col - (start + 1))), y);
                oldpen = SelectObject(wdc, oldpen);
                DeleteObject(oldpen);
                start = -1;
            }
        }
        ++row;
    } while (t_buffer < t_end);
    DeleteDC(wdc);
}
#endif  //CONSOLEEXT


static int CALLBACK
enumFontFamExProc(const LOGFONTA *lpelfe, const TEXTMETRICA *lpntme, DWORD FontType, LPARAM lParam)
{
    struct fcname *fcname;

    __CUNUSED(lpntme)
    __CUNUSED(FontType)
    __CUNUSED(lParam)

//  trace_log("\t[%s] <%s>\n", (DEVICE_FONTTYPE == FontType ? "dv" : RASTER_FONTTYPE == FontType ? "fx" :
//      TRUETYPE_FONTTYPE == FontType ? "tt" : "na"), lpelfe->lfFaceName);
    for (fcname = vio.fcnames; fcname->name; ++fcname) {
        if (0 == _stricmp(lpelfe->lfFaceName,  fcname->name)) {
            ++fcname->available;
            break;
        }
    }
    return TRUE;                                // next
}


static void
fcndump(void)
{
    struct fcname *fcname;
    unsigned names;

    trace_log("Console Fonts\n");
    for (names = 0, fcname = vio.fcnames; fcname->name; ++names, ++fcname) {
        trace_log("  [%u] 0x%04x <%s> %s\n", names, fcname->flags,
            fcname->name, (fcname->available ? " (*)" : ""));
    }
}


static const struct fcname *
fcnfind(const char *name)
{
    struct fcname *fcname;
    unsigned names;

    for (names = 0, fcname = vio.fcnames; fcname->name; ++names, ++fcname) {
        if (0 == _stricmp(name, fcname->name)) {
            return fcname;
        }
    }
    return NULL;
}


static int
fcnpush(const char *name, unsigned flags)
{
    struct fcname *fcname;
    unsigned names;

    for (names = 0, fcname = vio.fcnames; fcname->name; ++names, ++fcname) {
        if (0 == _stricmp(name, fcname->name)) {
            return -1;
        }
    }
    if (names < ((sizeof(vio.fcnames)/sizeof(vio.fcnames[0]))-1)) {
        fcname->name = name;
        fcname->flags = flags;
        return names;
    }
    return -1;
}


static void
consolefontsenum(void)
{
    LOGFONTA logFont = {0};
    unsigned names = 0;
    HKEY hKey;
    HDC wdc;

    // Console font list, as seen on console properties
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
            "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Console\\TrueTypeFont",
            0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD rc, cValues = 0;

        if (ERROR_SUCCESS ==
                (rc = RegQueryInfoKey(hKey, NULL, NULL, NULL, NULL, NULL, NULL, &cValues, NULL, NULL, NULL, NULL))) {
            char valueName[128];
            BYTE data[64];
            DWORD i;

            for (i = 0, rc = ERROR_SUCCESS; i < cValues; ++i) {
                DWORD type, cchValueName = sizeof(valueName),
                    cchData = (sizeof(data) - 1);

                valueName[0] = '\0';
                if (ERROR_SUCCESS ==
                        (rc = RegEnumValueA(hKey, i, valueName, &cchValueName, NULL, &type, data, &cchData))) {
                    if (REG_SZ == type && '0' == valueName[0]) {
                        //
                        //  0       Lucida Console
                        //  00      Consolas
                        //  9xx     ... others ...
                        //
                        data[cchData] = 0;
                        fcnpush(_strdup((const char *)data), 0);
                        ++names;
                    }
                }
            }
        }
        RegCloseKey(hKey);
    }

    if (0 == names) {                           // default, iso-8859-x coverage
        fcnpush("Lucida Console", 0);
    }

    // Alternatives,
    //  See,
    //      <charmap.exe> for font details
    //      <http://www.microsoft.com/typography/TrueTypeProperty21.mspx>
    //          Font properties extension, version 2.30, allows
    //      http://www.alanwood.net/unicode
    //          Good list of available Unicode fonts.
    //

    fcnpush("DejaVu Sans Mono", FCNUNC);        // nice, mono-spaced font dejavu-fonts.org

    fcnpush("FreeMono", FCNUNC);                // GNU Freefont project (8x16 or better advised)

    fcnpush("Quivira", FCNUNC|FCNPRO2);         // www.quivira-font.com
    fcnpush("TITUS Cyberbit Basic", FCNUNC|FCNPRO2);
    fcnpush("Marin", FCNUNC|FCNPRO2);

    fcnpush("Arial Unicode MS", FCNUNC|FCNPRO5);// 'most complete' standard windows unicode font (Office)

    fcnpush("Courier New", FCNUNC|FCNPRO);      // next most complete font, yet monospaced.

    fcnpush("Consolas", 0);                     // consolas Font Pack for Microsoft Visual Studio.

    fcnpush("Terminal", 0);                     // implied rastor font.

    // Determine availability
    logFont.lfCharSet = DEFAULT_CHARSET;
    wdc = GetDC(vio.wHandle);
    EnumFontFamiliesExA(wdc, &logFont, enumFontFamExProc, 0, 0);
    ReleaseDC(vio.wHandle, wdc);
    fcndump();
}


#if defined(DO_EXPERIMENTAL)
static BOOL
GetConsoleFontInfoEx(HANDLE cHandle, BOOL flag, DWORD count, CONSOLE_FONT_INFOEX *cfix)
{
    int ret = 0;

    if (vio.GetNumberOfConsoleFonts &&
            vio.GetCurrentConsoleFontEx && vio.SetConsoleFont) {
        CONSOLE_FONT_INFO cfi = {0};

        SendMessage(vio.wHandle, WM_SETREDRAW, FALSE, 0);
        if (GetCurrentConsoleFont(cHandle, 0, &cfi)) {
            DWORD f, fontnumber = vio.GetNumberOfConsoleFonts();

            for (f = 0; f < fontnumber && f < count; ++f, ++cfix) {
                vio.SetConsoleFont(cHandle, f);
                vio.GetCurrentConsoleFontEx(cHandle, flag, cfix);
            }
            vio.SetConsoleFont(cHandle, cfi.nFont);
            ret = TRUE;
        }
        SendMessage(vio.wHandle, WM_SETREDRAW, TRUE, 0);
    }
    return ret;
}
#endif	/*DO_EXPERIMENTAL*/


//  References:
//      http://blogs.msdn.com/b/oldnewthing/archive/2007/05/16/2659903.aspx
//      http://support.microsoft.com/kb/247815
//
//  To change the default console font family, apply the following Windows NT / Windows 2000 registry hack.
//
//      Hive:   HKEY_CURRENT_USER
//      Key:    Console
//      Name:   FontFamily
//      Type:   REG_DWORD
//      Value:  0   Don't care, any True Type
//      Value:  10  Roman
//      Value:  20  Swiss
//      Value:  30  Modern
//      Value:  40  Script
//      Value:  50  Decorative
//
static int
consolefontset(int height, int width, const char *facename)
{
    HANDLE wHandle = vio.wHandle;
    HFONT fnHandle = 0, fiHandle = 0, fHandleOrg = 0;
    int weight, faceindex = 0;
    const struct fcname *fcname = NULL;
    char t_facename[LF_FACESIZE+1] = {0};
    TEXTMETRIC tm = {0};
    RECT rect = {0};
    HDC wdc;
    SIZE size;

    wdc = GetDC(wHandle);
    GetClientRect(wHandle, &rect);

    if (-1 == height)				// -1, implied
        height = (int)((float)rect.bottom/vio.rows);
    if (-1 == width)
        width = (int)((float)rect.right/vio.cols);

    do {
        // select font
        weight = FW_REGULAR;                    // NORMAL(400) or BOLD(700)
        if (facename && *facename) {
            fcname = fcnfind(facename);
            fnHandle = consolefontcreate(height, width, weight, FALSE, facename);
            if (fnHandle) {
                fiHandle = consolefontcreate(height, width, weight, TRUE, facename);
            }
        }
        if (! fnHandle) {
            while (1) {                         // test available
                fcname = vio.fcnames + faceindex;

                if (NULL == (facename = fcname->name)) {
                    break;
                }

                if (fcname->available) {
                    const uint32_t flags = fcname->flags;
                    int t_width = width;

                    if (FCNPRO2 & flags) {      // resize variable width, 2/3
                        t_width = (int)ceil(((float)width/3)*2);
                    } else if (FCNPRO5 & flags) {   // resize variable width, 45%
                        t_width = (int)ceil(((float)width/100)*45);
                    }
                    fnHandle = consolefontcreate(height, t_width, weight, FALSE, facename);
                    if (fnHandle) {
                        fiHandle = consolefontcreate(height, t_width, weight, TRUE, facename);
                        break;
                    }
                }
                ++faceindex;
            }
            facename = NULL;
        }
        if (! fnHandle) {
            return -1;
        }

        // size characters
        t_facename[0] = 0;
        fHandleOrg = SelectObject(wdc, fnHandle);
        GetTextExtentPoint32A(wdc, "[", 1, &size);
        GetTextMetrics(wdc, &tm);
        GetTextFaceA(wdc, sizeof(t_facename), t_facename);
        SelectObject(wdc, fHandleOrg);
        if (vio.fnHandle) {
            DeleteObject(vio.fnHandle);
            if (vio.fiHandle) {
                DeleteObject(vio.fiHandle);
            }
        }
        vio.fnHandle = fnHandle;
        vio.fiHandle = fiHandle;
        vio.fcwidth  = (int)size.cx;            // tm.tmMaxCharWidth
        vio.fcheight = (int)size.cy;            // tm.tmHeight

        // verify selection
#if (XXX_SIZE_SEARCH)
        if (NULL == facename && faceindex >= 0 &&
                (NULL == fcname || 0 == (FCNPRO & fcname->flags)) {

            if ((vio.fcwidth * vio.cols) != (rect.right - rect.left) ||
                        (vio.fcheight * vio.rows) != (rect.bottom - rect.top)) {

                trace_log("!Console Font: [%d]<%s/%s>, %dx%d-%dx%d, %d/%d, %dx%d\n",
                    (facename ? -1 : faceindex), (facename ? facename : vio.fcnames[faceindex].name), t_facename,
                        vio.fcwidth, vio.fcheight, tm.tmMaxCharWidth, tm.tmHeight,
                        vio.cols, vio.rows, rect.right - rect.left, rect.bottom - rect.top);

                if (NULL != vio.fcnames[++faceindex].name) {
                    continue;                   // try next
                }
                facename = vio.fcnames[0].name;
                faceindex = -1;
                continue;                       // no match, use [0]
            }
        }
#endif
        break;
    } while (1);
    ReleaseDC(wHandle, wdc);

    strxcpy(vio.fcfacename, (const char *)t_facename, sizeof(vio.fcfacename));
    vio.fcflags  = (fcname ? fcname->flags : 0);
    vio.fcheight = height;
    vio.fcwidth  = width;

    trace_log("Console Font: <%s>, width:%d, height:%d\n",
        vio.fcfacename, vio.fcwidth, vio.fcheight);
    return 0;
}


static HFONT
consolefontcreate(int height, int width, int weight, int italic, const char *facename)
{
    return CreateFontA(
        height, width,                          // logic (device dependent pixels) height and width
        0, 0, weight,
        (italic ? TRUE : FALSE),                // italic, underline, strikeout
            FALSE,
            FALSE,
        DEFAULT_CHARSET,                        // DEFAULT, ANSI, OEM ...
        (0 == strcmp(facename, "Terminal") ? OUT_RASTER_PRECIS : OUT_TT_PRECIS),
        CLIP_DEFAULT_PRECIS,			// default clipping behavior.
        ANTIALIASED_QUALITY,                    // PROOF
        FF_MODERN,                              // DECORATIVE, DONTCARE, MODERN, ROMAN, SCRIPT, SWISS
        facename);
}


/*private*/
/*  Function:           CopyIn
 *      Copy in from video memory.
 *
 *  Parameters:
 *      buffer -            Video buffer base address.
 *      pos -               Starting offset in video cells from top left.
 *      cnt -               Video cell count.
 *
 *  Returns:
 *      nothing.
 */
static void
CopyIn(VIOCELL *buffer, size_t pos, size_t cnt)
{
    const int rows = vio.rows, cols = vio.cols;
    CHAR_INFO *image = vio.image + pos;
    COORD imgSize, imgCoord;
    SMALL_RECT rect;
    BOOL rc;

    assert((pos % cols) == 0);
  //assert((pos + cnt) <= vio.size);
    if (pos + cnt > vio.size) {                 // trim to image size
        cnt = vio.size - pos;
    }
    assert((cnt % cols) == 0);

    rect.Left   = 0;                            // dest. screen rectangle
    rect.Right  = (SHORT)(cols - 1);
    rect.Top    = (SHORT)(pos / cols);
    rect.Bottom = (SHORT)(((pos + (cnt - 1)) / cols));
    assert(rect.Bottom < rows);
    assert(pos == (size_t)(rect.Top * cols));

    imgCoord.X  = 0;                            // top left src cell in image
    imgCoord.Y  = 0;
    imgSize.Y   = (SHORT)(rows - rect.Top);     // size of image
    imgSize.X   = (SHORT)cols;

    rc = ReadConsoleOutputA(vio.cHandle,
		vio.image + pos, imgSize, imgCoord, &rect);
    if (0 == rc && ERROR_NOT_ENOUGH_MEMORY == GetLastError()) {
	if (cnt >= (cols * 4)) {		// sub-divide request
	    const int cnt2 = cols * ((cnt / cols) / 2);

	    CopyIn(buffer, pos, cnt2);
	    CopyIn(buffer, pos + cnt2, cnt - cnt2);
	    return;
	}
    }
    assert(rc);

    buffer += pos;
    while (cnt-- > 0) {
        VIO_IMPORT(buffer, image->Attributes, image->Char.UnicodeChar)
        ++buffer;
        ++image;
    }
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
#endif /*WIN32*/
