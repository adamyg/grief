#include <edidentifier.h>
__CIDENT_RCSID(gr_ttydos_c,"$Id: ttydos.c,v 1.16 2014/10/22 02:33:22 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttydos.c,v 1.16 2014/10/22 02:33:22 ayoung Exp $
 * TTY dos implementation.
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

#if !defined(STANDALONE)
#include <editor.h>                     /* global header */
#include "debug.h"
#endif

#if defined(STANDALONE) || \
        (!defined(WIN32) && defined(USE_VIO_BUFFER) && (defined(DJGPP) || defined(__WATCOMC__)))

#include "vio.h"                        /* VIO interface */
#include <dos.h>                        /* DOS interface */
#include <assert.h>

#if defined(DJGPP)
# include <sys\farptr.h>
# include <go32.h>
# define REGAX(r)       (r).x.ax
# define REGBX(r)       (r).x.bx
# define REGCX(r)       (r).x.cx
# define REGDX(r)       (r).x.dx
# define REGDI(r)       (r).x.di
# define REGSI(r)       (r).x.si
#elif defined(__WATCOMC__) && defined(__386__)
# define int86          int386
# define int86x         int386x
# define REGAX(r)       (r).x.eax
# define REGBX(r)       (r).x.ebx
# define REGCX(r)       (r).x.ecx
# define REGDX(r)       (r).x.edx
# define REGDI(r)       (r).x.edi
# define REGSI(r)       (r).x.esi
#else
# define REGAX(r)       (r).x.ax
# define REGBX(r)       (r).x.bx
# define REGCX(r)       (r).x.cx
# define REGDX(r)       (r).x.dx
# define REGDI(r)       (r).x.di
# define REGSI(r)       (r).x.si
#endif
#define REGAH(r)        (r).h.ah
#define REGAL(r)        (r).h.al
#define REGBH(r)        (r).h.bh
#define REGBL(r)        (r).h.bl
#define REGCH(r)        (r).h.ch
#define REGCL(r)        (r).h.cl
#define REGDH(r)        (r).h.dh
#define REGDL(r)        (r).h.dl

#define VINTERRUPT      (0x10)          /* BIOS video interrupt */
#define KINTERRUPT      (0x16)          /* BIOS key interrupt */
#define VDOS            (0x21)

#define VROWS           (50)
#define VCOLS           (132)

#define VFONT_8X8       (0x12)          /* Font */
#define VFONT_8X14      (0x11)
#define VFONT_8X16      (0x14)

#define VSLINES_200     (0)             /* Scan lines */
#define VSLINES_350     (1)
#define VSLINES_400     (2)

#define RIGHTSHIFT      (0x01)          /* Shift key values */
#define LEFTSHIFT       (0x02)
#define CTRLKEY         (0x04)
#define ALTKEY          (0x08)
#define SCROLLLOCK      (0x10)
#define NUMLOCK         (0x20)
#define CAPSLOCK        (0x40)
#define INSERTKEY       (0x80)

#if defined(DJGPP)
#elif defined(__WATCOMC__) && defined(__386__)
#define SS_DOSMEM       0x0034          /* PharLap BIOS Segment */
#endif

#define ismono()        (vio.bios == VIOVID_BIOS80MONO)
#define istext()        (ismono() || vio.bios <= VIOVID_BIOS80COLOUR)

static void             Config(int mode);
static int              ModeGetRaw(void);
static void             ModeSetRaw(int biosmode);
static int              IsVga(void);
static int              RowsGet(void);
static int              ColsGet(void);
static void             FontGet(int *pBytes, int *pRows);
static void             FontSet(int font);
static void             LinesSet(int lines);
static unsigned         CurTypeGet(void);
static void             Copyout(void *bf, USHORT offset, USHORT len);
static void             Copyin(void *bf, USHORT offset, USHORT len);
static void             KeyInit(void);
static int              Key101(void);

#if defined(STANDALONE)
static int              key_echo;
#endif

static struct {                         /* Video state */
    int         mode;                           /* Mode */
    int         init;                           /* Initialised */
    int         bios;                           /* BIOS mode */
    int         cols, rows;                     /* Video display */
    USHORT      size;
    int         page;
    int         codepage;
    unsigned    base;

    VIOCURSOR   cur_state;
    ULONG       cur_attr;                       /* Attribute (-1=Off, 0=On) */
    ULONG       cur_mask;                       /* Current cursor mask */

    unsigned    cur_on;                         /* Cursor masks */
    unsigned    cur_thalf;
    unsigned    cur_bhalf;
    unsigned    cur_full;
    unsigned    cur_off;
} vio;

static USHORT   viobuf[VROWS][VCOLS];   /* Video buffer (attr/char) */

static struct {                         /* Internal video modes */
    int         vga;                            /* Vga(1) */
    int         mode;                           /* Internal video mode */
    int         bios;                           /* BIOS mode */
    int         cols, rows;                     /* Video display */
    int         lines;                          /* CRT lines */
    int         font;                           /* font */
} viomodes[] =
    {   /*
         *  Note:   Table MUST be ordered from lowest to highest ROW resolution
         */
        { 1, VIOVID_MONO,       VIOVID_BIOS80MONO,   80,    25, VSLINES_400, VFONT_8X16 },
        { 1, VIOVID_BW40x25,    VIOVID_BIOS40BW,     40,    25, VSLINES_400, VFONT_8X16 },
        { 1, VIOVID_BW80x25,    VIOVID_BIOS80BW,     80,    25, VSLINES_400, VFONT_8X16 },

        { 1, VIOVID_CO40x12,    VIOVID_BIOS40COLOUR, 40,    12, VSLINES_200, VFONT_8X16 },
        { 1, VIOVID_CO40x14,    VIOVID_BIOS40COLOUR, 40,    14, VSLINES_200, VFONT_8X14 },
        { 1, VIOVID_CO40x25,    VIOVID_BIOS40COLOUR, 40,    25, VSLINES_400, VFONT_8X16 },
        { 1, VIOVID_CO40x28,    VIOVID_BIOS40COLOUR, 40,    28, VSLINES_400, VFONT_8X14 },

        { 1, VIOVID_CO80x12,    VIOVID_BIOS80COLOUR, 80,    12, VSLINES_200, VFONT_8X16 },
        { 1, VIOVID_CO80x14,    VIOVID_BIOS80COLOUR, 80,    14, VSLINES_200, VFONT_8X14 },
        { 1, VIOVID_CO80x25,    VIOVID_BIOS80COLOUR, 80,    25, VSLINES_400, VFONT_8X16 },
        { 1, VIOVID_CO80x28,    VIOVID_BIOS80COLOUR, 80,    28, VSLINES_400, VFONT_8X14 },
        { 1, VIOVID_CO80x43,    VIOVID_BIOS80COLOUR, 80,    43, VSLINES_350, VFONT_8X8  },
        { 1, VIOVID_CO80x50,    VIOVID_BIOS80COLOUR, 80,    50, VSLINES_400, VFONT_8X8  },

        { 0, VIOVID_MONO,       VIOVID_BIOS80MONO,   80,    25, VSLINES_350, VFONT_8X14 },
        { 0, VIOVID_BW40x25,    VIOVID_BIOS40BW,     40,    25, VSLINES_350, VFONT_8X14 },
        { 0, VIOVID_BW80x25,    VIOVID_BIOS80BW,     80,    25, VSLINES_350, VFONT_8X14 },
        { 0, VIOVID_CO40x14,    VIOVID_BIOS40COLOUR, 40,    14, VSLINES_200, VFONT_8X14 },
        { 0, VIOVID_CO40x25,    VIOVID_BIOS40COLOUR, 40,    25, VSLINES_350, VFONT_8X14 },
        { 0, VIOVID_CO80x14,    VIOVID_BIOS80COLOUR, 80,    14, VSLINES_200, VFONT_8X14 },
        { 0, VIOVID_CO80x25,    VIOVID_BIOS80COLOUR, 80,    25, VSLINES_350, VFONT_8X14 },
        { 0, VIOVID_CO80x43,    VIOVID_BIOS80COLOUR, 80,    43, VSLINES_350, VFONT_8X8  },
    };


/*  VioGetMode ----
 *      Get the current video mode
 */
int
VioGetMode(
    VIOMODEINFO *info, int page )
{
    (void) page;

    Config(VIOVID_UNDEF);
    info->row = vio.rows;
    info->col = vio.cols;
    info->codepage = vio.codepage;
    return vio.mode;
}


/*  VioSetMode ----
 *      Set the current video mode
 *
 *  o On a MCGA/VGA the following VIDEO modes are available:
 *      +-----------+---------------+--------------+
 *      | Row mode  |    Character  |   Vertical   |
 *      |           |    Size       |   resolution |
 *      +-----------+---------------+--------------+
 *      |     12    |     8x16      |     200      |
 *      |     14    |     8x14      |     200      |
 *      |     21    |     8x16      |     350      |
 *      |  (*)25    |     8x16      |     400      |
 *      |     28    |     8x14      |     400      |
 *      |     43    |     8x8       |     350      |
 *      |     50    |     8x8       |     400      |
 *      +-----------+---------------+--------------+
 *      (*) = default
 *
 *  o On a EGA the following VIDEO modes are available:
 *      +-----------+---------------+---------------
 *      | Row mode  |    Character  |   Vertical   |
 *      |           |    Size       |   resolution |
 *      +-----------+---------------+--------------+
 *      |     14    |     8x14      |     200      |
 *      |  (*)25    |     8x14      |     350      |
 *      |     43    |     8x8       |     350      |
 *      +-----------+---------------+---------------
 *      (*) = default
 */
int
VioSetMode(
    VIOMODEINFO *info, int page )
{
    int vga, mode, obios, bios, font, lines;
    int i;

    (void) page;

    /* default video mode */
    mode = VIOVID_CO80x25;
    bios = VIOVID_BIOS80COLOUR;
    if ((vga = IsVga()) == 1) {                 /* VGA/MGA and greater */
        font  = VFONT_8X16;
        lines = VSLINES_400;
    } else {                                    /* EGA */
        font  = VFONT_8X14;
        lines = VSLINES_350;
    }

    /* locate best match */
    for (i = (int)(sizeof(viomodes)/sizeof(viomodes[0]))-1; i >= 0; i--)
        if (vga == viomodes[i].vga &&
                (viomodes[i].cols == info->col ||
                    (info->col > 80 && viomodes[i].cols == 80)) &&
                viomodes[i].rows <= info->row) {
            mode = viomodes[i].mode;
            bios = viomodes[i].bios;
            font = viomodes[i].font;
            lines = viomodes[i].lines;
            break;
        }

    /* set new video mode */
    LinesSet(lines);                            /* set scan lines */
    obios = ModeGetRaw();
    if ((obios == mode) || obios & 0x80)
        bios |= 0x80;                           /* dont clear bit */
    ModeSetRaw(bios);
    FontSet(font);                              /* set font */
    Config(mode);                               /* update configuration */
    return (mode);
}


/*  VioPageGet/VioPageSet ---
 *      Get/set current video page
 */
int
VioPageGet(void)
{
    union REGS r;

    REGAH(r) = 0xf;                             /* get state */
    int86(VINTERRUPT, &r, &r);
    return REGBH(r);                            /* page */
}


void
VioPageSet(int page)
{
    union REGS r;

    REGAH(r) = 0x05;                            /* get active page */
    REGAL(r) = page;
    int86(VINTERRUPT, &r, &r);
    vio.page = page;                            /* save current page */
}


int
VioCodePageGet(void)
{
    union REGS r;

    REGAX(r) = 0x6601;                          /* get codepage */
    int86(VDOS, &r, &r);
    return REGBX(r);                            /* active codepage */
}


int
VioCursor(VIOCURSOR state)
{
    union REGS r;
    unsigned mask;

    if (state == VIOCUR_STATE) {
        state = vio.cur_state;

    } else {
        switch(state) {
        case VIOCUR_THALF: mask = vio.cur_thalf; break;
        case VIOCUR_BHALF: mask = vio.cur_bhalf; break;
        case VIOCUR_FULL: mask = vio.cur_full; break;
        case VIOCUR_OFF: mask = vio.cur_off; break;
        case VIOCUR_ON:
        default:
            mask = vio.cur_on;
        }

        if (vio.cur_attr == 0) {                /* change */
            REGAX(r) = 0x100;                   /* AH = 0x01 */
            REGBX(r) = 0x0;
            REGCX(r) = mask;
            REGDX(r) = 0x0;
            int86(VINTERRUPT, &r, &r);
        }

        vio.cur_mask = mask;
        vio.cur_state = state;
    }
    return (state);
}


int
VioGetCurType(
    VIOCURSORINFO *info, int page )
{
    (void) page;

    info->mask = vio.cur_mask;
    info->attr = vio.cur_attr;
    return (0);
}


int
VioSetCurType(
    VIOCURSORINFO *info, int page )
{
    union REGS r;
    unsigned mask;

    (void) page;

    if (info->attr != vio.cur_attr || info->mask != vio.cur_mask) {
        if (info->attr == (USHORT)-1) {
            vio.cur_attr = -1;                  /* off */
            mask = vio.cur_off;
        } else {
            vio.cur_attr = 0;                   /* on */
            vio.cur_mask = info->mask;          /* set mask */
            mask = vio.cur_mask;
        }
        REGAX(r) = 0x100;
        REGBX(r) = 0x0;
        REGCX(r) = mask;
        REGDX(r) = 0x0;
        int86(VINTERRUPT, &r, &r);
    }
    return (0);
}


int
VioGetCurPos(
    USHORT *row, USHORT *col, int page )
{
    union REGS r;

    (void) page;

    REGAH(r) = 3;
    REGBH(r) = 0;
    int86(VINTERRUPT, &r, &r);
    *row = REGDH(r);
    *col = REGDL(r);
    return (0);
}


int
VioSetCurPos(
    USHORT row, USHORT col, int page )
{
    union REGS r;

    (void) page;

    REGAX(r) = 0x200;
    REGBX(r) = 0x0;
    REGCX(r) = 0x0;
    REGDX(r) = (USHORT)((row << 8) + col);
    int86(VINTERRUPT, &r, &r);
    return (0);
}


int
VioGetBuf(
    USHORT **pBuf, ULONG *pSize, int page )
{
    ULONG size;

    (void) page;

    Config(VIOVID_UNDEF);
    size = vio.size;
    Copyin(viobuf, 0, size);
    *pBuf = (USHORT *)viobuf;
    *pSize = size;
    return (0);
}


int
VioShowBuf(
    USHORT offset, ULONG length, int page )
{
    (void) page;

    Copyout(((UCHAR *)viobuf) + offset, offset, length);
    return (0);
}


int
VioReadCellStr(
    USHORT *buf, ULONG *pSize, USHORT row, USHORT col, int page )
{
    ULONG size, offset;

    (void) page;

    if (pSize == NULL || (size = *pSize) > vio.size)
        size = vio.size;

    if ((offset = (row *col * sizeof(USHORT))) >= vio.size) {
        size = 0;
    } else {
        if (offset + size > vio.size) {
            size = vio.size - offset;
        }
        Copyin( buf, offset, size );
    }

    if (pSize) *pSize = size;

    return 0;
}


/*
 *  VioPutChar ---
 *      High level character write.
 */
int
VioPutChar(
    USHORT row, USHORT col, int ch, int attr )
{
    USHORT offset;

    if (row < vio.rows && col < vio.cols) {
        offset = ((row * vio.cols) + col) * sizeof(USHORT);
        *((USHORT *)viobuf + offset) = ((USHORT)(attr & 0xff) << 8) | (ch & 0xff);
    }
    return 0;
}


/*
 *  VioRefresh ---
 *      Refresh the video buffer.
 */
void
VioRefresh(void)
{
    VioShowBuf( 0, vio.size, -1 );
}


/*
 *  Config ---
 *      Setup video configuration
 */
static void
Config(int mode)
{
    unsigned lines, start, stop;

    if (mode == VIOVID_UNDEF && vio.init)
        return;                                 /* Initialised */

    if (vio.init == 0)
        KeyInit();                              /* keyboard initialisation */

    vio.init = 1;
    vio.bios = ModeGetRaw();                    /* BIOS mode */
    vio.rows = RowsGet()+1;
    vio.cols = ColsGet()+1;
    vio.size = vio.rows * vio.cols * sizeof(USHORT);
    vio.page = VioPageGet();
    vio.codepage = VioCodePageGet();

    if (mode == VIOVID_UNDEF) {                 /* locate best match */
        int vga = IsVga();                      /* VGA or greater */
        register int i;

        for (i = (int)(sizeof(viomodes)/sizeof(viomodes[0]))-1; i >= 0; i--)
            if (vga == viomodes[i].vga && viomodes[i].bios == vio.bios &&
                    (viomodes[i].cols == vio.cols ||
                        (vio.cols > 80 && viomodes[i].cols == 80)) &&
                    viomodes[i].rows <= vio.rows) {
               break;                           /* match */
            }

        if (i >= 0)
            mode = viomodes[i].mode;            /* known, map to internal id */
        else
            mode = vio.bios;                    /* unknown, use bios value */
    }
    vio.mode = mode;                            /* update video state info. */

#if defined(__WATCOMC__) && defined(__386__)
    if (ismono())
        vio.base = 0xb0000;                     /* mono */
    else vio.base = 0xb8000;                    /* colour */
#else
    if (ismono())
        vio.base = 0xb000;                      /* mono */
    else vio.base = 0xb800;                     /* colour */
#endif

    lines = CurTypeGet();                       /* Initialise Cursor */
    start = lines & 0x1f00;
    stop = lines & 0x1f;

    vio.cur_mask   = lines;                     /* current (assumed) */
    vio.cur_on     = start | stop;              /* underline */
    vio.cur_bhalf  = (start>>1) | stop;         /* bottom on */
    vio.cur_thalf  = 0 | (stop>>1);             /* top on */
    vio.cur_full   = 0 | stop;                  /* fully on */
    vio.cur_off    = 0x2000;                    /* off */

#if !defined(STANDALONE)
    trace_log( "ttydos:\n" );
    trace_log( "\tmode   %d\n", vio.mode );
    trace_log( "\tbios   0x%x\n", vio.bios );
    trace_log( "\trows   %d\n", vio.rows );
    trace_log( "\tcols   %d\n", vio.cols );
    trace_log( "\tsize   %d\n", vio.size );
    trace_log( "\tpage   %d\n", vio.page );
    trace_log( "\tcpage  %d\n", vio.codepage );
    trace_log( "\tcursor:\n" );
    trace_log( "\t on    0x%04x\n", vio.cur_on );
    trace_log( "\t off   0x%04x\n", vio.cur_off );
    trace_log( "\t top   0x%04x\n", vio.cur_thalf );
    trace_log( "\t bot   0x%04x\n", vio.cur_bhalf );
    trace_log( "\t full  0x%04x\n", vio.cur_full );
#endif
}


/*  ModeGetRaw/ModeSetRaw ---
 *      Retrieve/set the current BIOS video mode
 */
static int
ModeGetRaw(void)
{
    union REGS r;

    REGAH(r) = 0xf;                             /* get state */
    int86(VINTERRUPT, &r, &r);
    vio.page = REGBH(r);                        /* save page details */
    return REGAL(r)&0x7f;                       /* mode */
}


static void
ModeSetRaw(int biosmode)
{
    union REGS r;

    REGAH(r) = 0;                               /* set video mode */
    REGAL(r) = biosmode;                        /* required mode */
    int86(VINTERRUPT,&r,&r);
    vio.page = REGBH(r);                        /* update page */
}


static int
IsVga(void)
{
    union REGS r;

    REGAX(r) = 0x1a00;
    int86(VINTERRUPT, &r, &r);
    return ((REGAL(r) == 0x1a && REGBL(r) > 6) ? 1 : 0);
}


/*  RowsGet/ColsGet ---
 *      Returns the current number of character pRows in use on
 *      the screen, minus 1.
 */
static int
RowsGet(void)
{
    int rows;

    FontGet((void *)0, &rows);
    return (rows > VROWS-1 ? VROWS-1 : rows);
}


static int
ColsGet(void)
{
    int cols;

#if defined(DJGPP)
    cols = (_farpeekb(_dos_ds, 0x44a)-1);

#elif defined(__WATCOMC__)
    UCHAR far *pCols;

  #if defined(__386__)
    pCols = MK_FP(SS_DOSMEM, (0x40<<4)|0x4a);
  #else
    pCols = (UCHAR far *)MK_FP(0x40, 0x4a);
  #endif
    cols = (int)(*pCols-1);
#endif
    return (cols > VCOLS-1 ? VCOLS-1 : cols);
}


/*  FontGet ---
 *      Retreive the currrent font details
 */
static void
FontGet(int *pBytes, int *pRows)
{
    union REGS r;

    REGAX(r) = 0x1130;                          /* Get extended font information */
    REGBH(r) = 0x02;                            /* Select current pointer (43h) */
    int86(VINTERRUPT, &r, &r);                  /* Returns: ES:BP->Pointer */
                                                /*          DL Char pRows on screen */
                                                /*          CX Bytes/char  */

    if (pBytes) *pBytes = REGCX(r)&0xff;
    if (pRows) *pRows = REGDL(r)&0xff;
}


/*  FontSet ---
 *      Call the video BIOS to set the current ROM font.
 */
static void
FontSet(int font)
{
    union REGS r;

    /* select BIOS font */
    REGAH(r) = 0x11;
    REGAL(r) = font;
    REGBL(r) = 0;
    int86(VINTERRUPT,&r,&r);

    /* select alternate print-screen */
    REGAX(r) = 0x1120;
    REGBX(r) = 0x20;
    int86(VINTERRUPT,&r,&r);
}


/*  LinesSet ---
 *      Call the video BIOS to set the current number of video
 *      scan lines. After this function a video mode reset should
 *      be performed.
 *
 *          0  -  200 Scan lines
 *          1  -  350 Scan lines
 *          2  -  400 Scan lines (VGA only)
 */
static void
LinesSet(int lines)
{
    union REGS r;

    REGAH(r) = 0x12;
    REGBL(r) = 0x30;
    REGAL(r) = lines;
    int86(VINTERRUPT, &r, &r);
}


/* CurTypeGet ---
 *      Get the current hardware cursor type.
 *
 *  Notes:  Video function 3 reports the current location of the cursor
 *          on the screen, it also reports the current type.
 */
static unsigned
CurTypeGet(void)
{
    struct SREGS sr;
    union REGS r;

    REGBH(r) = 0;
    REGAH(r) = 3;
#if defined(DJGPP)
    (void) memset(&sr, 0, sizeof(sr));
#elif defined(__WATCOMC__)
    segread(&sr);
#endif
    int86x(VINTERRUPT, &r, &r, &sr);
    return (unsigned) REGCX(r)&0xffff;          /* mask */
}


/*  Copyout ---
 *      Copy out to video memory
 */
#if defined(DJGPP)
static void
Copyout(void *bf, USHORT offset, USHORT len)
{
    assert(offset < vio.size);

    if (offset + len > vio.size) {              /* trim to image size */
        len = vio.size - offset;
    }

    if (len--) {
        int  i, ad = vio.base*16+offset;
        char *as = (char *)bf;

        _farpokeb(_go32_conventional_mem_selector(), ad++, *as++);
        for (i = 0; i < len; ++i) {
            _farnspokeb(ad++,*as++);
        }
    }
}

#elif defined(__WATCOMC__) && defined(__386__)
static void
Copyout(void *bf, USHORT offset, USHORT len)
{
    assert(offset < vio.size);
    if (offset + len > vio.size) {              /* trim to image size */
        len = vio.size - offset;
    }
    (void) memcpy((USHORT *)(vio.base|offset), bf, len);
}

#elif defined(__WATCOMC__)
static void
Copyout(void *bf, USHORT offset, USHORT len)
{
    assert(offset < vio.size);
    if (offset + len > vio.size) {              /* trim to image size */
        len = vio.size - offset;
    }
    (void) movedata(FP_SEG(bf), FP_OFF(bf), vio.base, offset, len);
}
#endif


/*  Copyin ---
 *      Copy in to video memory
 */
#if defined(DJGPP)
static void
Copyin(void *bf, USHORT offset, USHORT len)
{
    assert(offset+len <= vio.size);

    if (len--) {
        int i, as = vio.base*16+offset;
        char *ad = (char*)bf;

        *ad++ = _farpeekb(_go32_conventional_mem_selector(), as++);
        for (i = 0; i < len; ++i) {
            *ad++ = _farnspeekb(as++);
        }
    }
}

#elif defined(__WATCOMC__) && defined (__386__)
static void
Copyin(void *bf, USHORT offset, USHORT len)
{
    assert(offset+len <= vio.size);

    (void) memcpy(bf, (USHORT *)(vio.base|offset), len);
}

#elif defined(__WATCOMC__)
static void
Copyin(void *bf, USHORT offset, USHORT len)
{
    assert(offset+len <= vio.size);

    (void) movedata(vio.base, offset, FP_SEG(bf), FP_OFF(bf), len);
}
#endif


/* * * * * * * * * * * * * * * Keyb Functions * * * * * * * * * * * * * * * */

static unsigned char        vio_kbget = 0;
static unsigned char        vio_kbhit = 1;
static unsigned char        vio_kbsft = 2;

static void
KeyInit(void)
{
    static int inited;

    if (inited == 0)
    {
        if (Key101())
        {/* Update BIOS function codes to handle 101 key board */
            vio_kbget |= 0x10;
            vio_kbhit |= 0x10;
            vio_kbsft |= 0x10;
        }
        inited = 1;
    }
}


static int
Key101(void)
{
    unsigned char value;

#if defined(DJGPP)
    value = _farpeekw(_dos_ds, 0x496);

#else
    unsigned char far *pState;
 #if defined(__386__)
    pState = MK_FP(SS_DOSMEM, (0x40<<4)|0x96);
 #else
    pState = (unsigned short far *)MK_FP(0x40, 0x96);
 #endif
    value = *pState;
#endif
    if (value & 0x10)
        return (1);
    return (0);
}


/*  VioKeyGet ---
 *      Waits for the user to press one key, then returns that key.
 *
 *      a) Alt-key combinations have 0x100 added to them.
 *      b) Keypad and specials key combinations have 0x200 added to them.
 *      c) Other extended keys return their non-extended codes.
 */
int
VioKeyGet(void)
{
    int shift, lo, hi;
    union REGS r;

    REGAH(r) = (unsigned char) vio_kbget;
    int86(KINTERRUPT, &r, &r);                  /* key */
    lo = REGAL(r);
    hi = REGAH(r);

    REGAH(r) = (unsigned char) vio_kbsft;
    int86(KINTERRUPT, &r, &r);                  /* shift status */
    shift = REGAL(r);

#if defined(STANDALONE)
    if (key_echo)
        printf("Scancode lo %02x,hi %02x, ", lo, hi);
#endif

    if (hi == 0xe0)                             /* 101 Keyboard code        */
        switch (lo) {                               /* Test key code        */
        case '/':                                   /* Keypad-/             */
        case 0x0d:                                  /* Keypad-Enter         */
            if (shift & (LEFTSHIFT|RIGHTSHIFT))
                return (0x300|lo);                  /* Shift-Keypad-...     */
            return (0x200|lo);                      /* Keypad-...           */
        case 0x0a:                                  /* Ctrl-Keypad-Enter    */
            return (0x300|0x0a);
        default:
#if defined(STANDALONE)
            printf( "101 unknown " );
#endif
            break;
        }

    if (hi != 0 && lo == 0xe0)                  /* 101 Keyboard code        */
        lo = 0;                                     /* Return non-extended  */
    else if (lo)
    {
        switch (hi) {                           /* Test scan codes          */
        case 0x1a:                              /* Ctrl-{, clashs with ESC  */
            if (lo == 0x1b)
                return (0x200|27);
            break;
        case 0x37:                              /* Keypad-star              */
            if (shift & (LEFTSHIFT|RIGHTSHIFT))
                return (0x300|'*');
            return (0x200|'*');
        case 0x4a:                              /* Keypad-minus             */
            if (shift & (LEFTSHIFT|RIGHTSHIFT))
                return (0x300|'-');
            return (0x200|'-');
        case 0x4e:                              /* Keypad-plus              */
            if (shift & (LEFTSHIFT|RIGHTSHIFT))
                return (0x300|'+');
            return (0x200|'+');
        }
        return (lo);
    }

    switch (hi) {                               /* Test key codes           */
    case 0x47:                                      /* Home                 */
    case 0x48:                                      /* Up                   */
    case 0x49:                                      /* PgUp                 */
    case 0x4b:                                      /* Left                 */
    case 0x4c:                                      /* Center               */
    case 0x4d:                                      /* Right                */
    case 0x4f:                                      /* End                  */
    case 0x50:                                      /* Down                 */
    case 0x51:                                      /* PgDn                 */
    case 0x52:                                      /* Ins                  */
    case 0x53:                                      /* Del                  */
        if (shift & (LEFTSHIFT|RIGHTSHIFT))
            return (0x200|hi);
    }

    return (0x100|hi);
}


#if !defined(__WATCOMC__)
int
VioKeyHit(void)
{
    union REGS r;

#if defined(DJGPP)
    if (_farpeekw(_dos_ds, 0x41a) == _farpeekw(_dos_ds, 0x41c))
        return 0;

#else
    unsigned short far *pStart, *pEnd;
 #if defined(__386__)
    pStart = MK_FP(SS_DOSMEM, (0x40<<4)|0x1a);
    pEnd = MK_FP(SS_DOSMEM, (0x40<<4)|0x1c);
 #else
    pStart = (unsigned char far *)MK_FP(0x40, 0x1a);
    pEnd = (unsigned char far *)MK_FP(0x40, 0x1c);
 #endif
    if (*pStart == *pEnd)
        return (0);
#endif

    REGAH(r) = (unsigned char) vio_kbhit;
    int86(KINTERRUPT, &r, &r);
    if (r.x.flags & 0x40)                       /* Zero flag */
        return 0;
    return 1;
}
#endif


/* * * * * * * * * * * * * * * Misc Functions * * * * * * * * * * * * * * * */

void
DosBeep(
    int freq, int duration )
{
    sound(freq);
    delay(duration);
    sound(0);
}


/* * * * * * * * * * * * * * * Test Functions * * * * * * * * * * * * * * * */

#if defined(STANDALONE)
int
main(void)
{
    const char *keystr;
    int key;

    Config( VIOVID_UNDEF );

/*
//  printf( "Cursor test ... press any key\n");
//  printf( "  FULL"); VioCursor( VIOCUR_FULL );  VioKeyGet(); printf("\n");
//  printf( " THALF"); VioCursor( VIOCUR_THALF ); VioKeyGet(); printf("\n");
//  printf( "   OFF"); VioCursor( VIOCUR_OFF );   VioKeyGet(); printf("\n");
//  printf( " BHALF"); VioCursor( VIOCUR_BHALF ); VioKeyGet(); printf("\n");
//  printf( "    ON"); VioCursor( VIOCUR_ON );    VioKeyGet(); printf("\n");
*/

    key_echo = 1;
    printf("Key test ... press ESC to exit\n");
    while ((key = VioKeyGet()) != '\x1b' && key)
    {
       printf("Key Code: %04x (%c) ", key,
          (key < 256 && isprint(key) ? (char)key : ' '));

       switch (key) {
       case VIOKEY_CTRLA:          keystr = "Ctrl A";       break;
       case VIOKEY_CTRLB:          keystr = "Ctrl B";       break;
       case VIOKEY_CTRLC:          keystr = "Ctrl C";       break;
       case VIOKEY_CTRLD:          keystr = "Ctrl D";       break;
       case VIOKEY_CTRLE:          keystr = "Ctrl E";       break;
       case VIOKEY_CTRLF:          keystr = "Ctrl F";       break;
       case VIOKEY_CTRLG:          keystr = "Ctrl G";       break;
       case VIOKEY_CTRLH:          keystr = "Ctrl H";       break;
       case VIOKEY_CTRLI:          keystr = "Ctrl I";       break;
       case VIOKEY_CTRLJ:          keystr = "Ctrl J";       break;
       case VIOKEY_CTRLK:          keystr = "Ctrl K";       break;
       case VIOKEY_CTRLL:          keystr = "Ctrl L";       break;
       case VIOKEY_CTRLM:          keystr = "Ctrl M";       break;
       case VIOKEY_CTRLN:          keystr = "Ctrl N";       break;
       case VIOKEY_CTRLO:          keystr = "Ctrl O";       break;
       case VIOKEY_CTRLP:          keystr = "Ctrl P";       break;
       case VIOKEY_CTRLQ:          keystr = "Ctrl Q";       break;
       case VIOKEY_CTRLR:          keystr = "Ctrl R";       break;
       case VIOKEY_CTRLS:          keystr = "Ctrl S";       break;
       case VIOKEY_CTRLT:          keystr = "Ctrl T";       break;
       case VIOKEY_CTRLU:          keystr = "Ctrl U";       break;
       case VIOKEY_CTRLV:          keystr = "Ctrl V";       break;
       case VIOKEY_CTRLW:          keystr = "Ctrl W";       break;
       case VIOKEY_CTRLX:          keystr = "Ctrl X";       break;
       case VIOKEY_CTRLY:          keystr = "Ctrl Y";       break;
       case VIOKEY_CTRLZ:          keystr = "Ctrl Z";       break;

       case VIOKEY_F1:             keystr = "F1";           break;
       case VIOKEY_F2:             keystr = "F2";           break;
       case VIOKEY_F3:             keystr = "F3";           break;
       case VIOKEY_F4:             keystr = "F4";           break;
       case VIOKEY_F5:             keystr = "F5";           break;
       case VIOKEY_F6:             keystr = "F6";           break;
       case VIOKEY_F7:             keystr = "F7";           break;
       case VIOKEY_F8:             keystr = "F8";           break;
       case VIOKEY_F9:             keystr = "F9";           break;
       case VIOKEY_F10:            keystr = "F10";          break;
       case VIOKEY_F11:            keystr = "F11";          break;
       case VIOKEY_F12:            keystr = "F12";          break;
       case VIOKEY_SF1:            keystr = "Shift F1";     break;
       case VIOKEY_SF2:            keystr = "Shift F2";     break;
       case VIOKEY_SF3:            keystr = "Shift F3";     break;
       case VIOKEY_SF4:            keystr = "Shift F4";     break;
       case VIOKEY_SF5:            keystr = "Shift F5";     break;
       case VIOKEY_SF6:            keystr = "Shift F6";     break;
       case VIOKEY_SF7:            keystr = "Shift F7";     break;
       case VIOKEY_SF8:            keystr = "Shift F8";     break;
       case VIOKEY_SF9:            keystr = "Shift F9";     break;
       case VIOKEY_SF10:           keystr = "Shift F10";    break;
       case VIOKEY_SF11:           keystr = "Shift F11";    break;
       case VIOKEY_SF12:           keystr = "Shift F12";    break;
       case VIOKEY_CF1:            keystr = "Ctrl F1";      break;
       case VIOKEY_CF2:            keystr = "Ctrl F2";      break;
       case VIOKEY_CF3:            keystr = "Ctrl F3";      break;
       case VIOKEY_CF4:            keystr = "Ctrl F4";      break;
       case VIOKEY_CF5:            keystr = "Ctrl F5";      break;
       case VIOKEY_CF6:            keystr = "Ctrl F6";      break;
       case VIOKEY_CF7:            keystr = "Ctrl F7";      break;
       case VIOKEY_CF8:            keystr = "Ctrl F8";      break;
       case VIOKEY_CF9:            keystr = "Ctrl F9";      break;
       case VIOKEY_CF10:           keystr = "Ctrl F10";     break;
       case VIOKEY_CF11:           keystr = "Ctrl F11";     break;
       case VIOKEY_CF12:           keystr = "Ctrl F12";     break;
       case VIOKEY_AF1:            keystr = "Alt F1";       break;
       case VIOKEY_AF2:            keystr = "Alt F2";       break;
       case VIOKEY_AF3:            keystr = "Alt F3";       break;
       case VIOKEY_AF4:            keystr = "Alt F4";       break;
       case VIOKEY_AF5:            keystr = "Alt F5";       break;
       case VIOKEY_AF6:            keystr = "Alt F6";       break;
       case VIOKEY_AF7:            keystr = "Alt F7";       break;
       case VIOKEY_AF8:            keystr = "Alt F8";       break;
       case VIOKEY_AF9:            keystr = "Alt F9";       break;
       case VIOKEY_AF10:           keystr = "Alt F10";      break;
       case VIOKEY_AF11:           keystr = "Alt F11";      break;
       case VIOKEY_AF12:           keystr = "Alt F12";      break;

       case VIOKEY_ALT1:           keystr = "Alt 1";        break;
       case VIOKEY_ALT2:           keystr = "Alt 2";        break;
       case VIOKEY_ALT3:           keystr = "Alt 3";        break;
       case VIOKEY_ALT4:           keystr = "Alt 4";        break;
       case VIOKEY_ALT5:           keystr = "Alt 5";        break;
       case VIOKEY_ALT6:           keystr = "Alt 6";        break;
       case VIOKEY_ALT7:           keystr = "Alt 7";        break;
       case VIOKEY_ALT8:           keystr = "Alt 8";        break;
       case VIOKEY_ALT9:           keystr = "Alt 9";        break;
       case VIOKEY_ALT0:           keystr = "Alt 0";        break;
       case VIOKEY_ALTA:           keystr = "Alt A";        break;
       case VIOKEY_ALTB:           keystr = "Alt B";        break;
       case VIOKEY_ALTC:           keystr = "Alt C";        break;
       case VIOKEY_ALTD:           keystr = "Alt D";        break;
       case VIOKEY_ALTE:           keystr = "Alt E";        break;
       case VIOKEY_ALTF:           keystr = "Alt F";        break;
       case VIOKEY_ALTG:           keystr = "Alt G";        break;
       case VIOKEY_ALTH:           keystr = "Alt H";        break;
       case VIOKEY_ALTI:           keystr = "Alt I";        break;
       case VIOKEY_ALTJ:           keystr = "Alt J";        break;
       case VIOKEY_ALTK:           keystr = "Alt K";        break;
       case VIOKEY_ALTL:           keystr = "Alt L";        break;
       case VIOKEY_ALTM:           keystr = "Alt M";        break;
       case VIOKEY_ALTN:           keystr = "Alt N";        break;
       case VIOKEY_ALTO:           keystr = "Alt O";        break;
       case VIOKEY_ALTP:           keystr = "Alt P";        break;
       case VIOKEY_ALTQ:           keystr = "Alt Q";        break;
       case VIOKEY_ALTR:           keystr = "Alt R";        break;
       case VIOKEY_ALTS:           keystr = "Alt S";        break;
       case VIOKEY_ALTT:           keystr = "Alt T";        break;
       case VIOKEY_ALTU:           keystr = "Alt U";        break;
       case VIOKEY_ALTV:           keystr = "Alt V";        break;
       case VIOKEY_ALTW:           keystr = "Alt W";        break;
       case VIOKEY_ALTX:           keystr = "Alt X";        break;
       case VIOKEY_ALTY:           keystr = "Alt Y";        break;
       case VIOKEY_ALTZ:           keystr = "Alt Z";        break;

       case VIOKEY_HOME:           keystr = "Home";         break;
       case VIOKEY_END:            keystr = "End";          break;
       case VIOKEY_UP:             keystr = "Up";           break;
       case VIOKEY_DOWN:           keystr = "Down";         break;
       case VIOKEY_PGUP:           keystr = "Pgup";         break;
       case VIOKEY_PGDN:           keystr = "Pgdn";         break;
       case VIOKEY_LEFT:           keystr = "Left";         break;
       case VIOKEY_INS:            keystr = "Ins";          break;
       case VIOKEY_RIGHT:          keystr = "Right";        break;
       case VIOKEY_DEL:            keystr = "Del";          break;
       case VIOKEY_CNTR:           keystr = "Center";       break;

       case VIOKEY_STAB:           keystr = "Shift Tab";    break;
       case VIOKEY_SHIFTDEL:       keystr = "Shift Del";    break;
       case VIOKEY_SHIFTDOWN:      keystr = "Shift Down";   break;
       case VIOKEY_SHIFTEND:       keystr = "Shift End";    break;
       case VIOKEY_SHIFTHOME:      keystr = "Shift Home";   break;
       case VIOKEY_SHIFTCNTR:      keystr = "Shift Center"; break;
       case VIOKEY_SHIFTINS:       keystr = "Shift Ins";    break;
       case VIOKEY_SHIFTLEFT:      keystr = "Shift Left";   break;
       case VIOKEY_SHIFTPGDN:      keystr = "Shift PgeDn";  break;
       case VIOKEY_SHIFTPGUP:      keystr = "Shift PgeUp";  break;
       case VIOKEY_SHIFTRIGHT:     keystr = "Shift Right";  break;
       case VIOKEY_SHIFTUP:        keystr = "Shift up";     break;

       case VIOKEY_CTRLBS:         keystr = "Ctrl Bs";      break;
       case VIOKEY_CTRLBSLASH:     keystr = "Ctrl Bslash";  break;
       case VIOKEY_CTRLCLOSEB:     keystr = "Ctrl Closeb";  break;
       case VIOKEY_CTRLCNTR:       keystr = "Ctrl Cntr";    break;
       case VIOKEY_CTRLDEL:        keystr = "Ctrl Del";     break;
       case VIOKEY_CTRLDOWN:       keystr = "Ctrl Down";    break;
       case VIOKEY_CTRLEND:        keystr = "Ctrl End";     break;
       case VIOKEY_CTRLHOME:       keystr = "Ctrl Home";    break;
       case VIOKEY_CTRLINS:        keystr = "Ctrl Ins";     break;
       case VIOKEY_CTRLLEFT:       keystr = "Ctrl Left";    break;
       case VIOKEY_CTRLMINUS:      keystr = "Ctrl Minus";   break;
       case VIOKEY_CTRLOPENB:      keystr = "Ctrl Openb";   break;
       case VIOKEY_CTRLPGDN:       keystr = "Ctrl Pgdn";    break;
       case VIOKEY_CTRLPGUP:       keystr = "Ctrl Pgup";    break;
       case VIOKEY_CTRLPRTSC:      keystr = "Ctrl Prtsc";   break;
       case VIOKEY_CTRLRIGHT:      keystr = "Ctrl Right";   break;
       case VIOKEY_CTRLTAB:        keystr = "Ctrl Tab";     break;
       case VIOKEY_CTRLUP:         keystr = "Ctrl Up";      break;

       case VIOKEY_ALTACCENT:      keystr = "Alt Accent";   break;
       case VIOKEY_ALTAPOST:       keystr = "Alt Apost";    break;
       case VIOKEY_ALTSTAR:        keystr = "Alt Asterix";  break;
       case VIOKEY_ALTBS:          keystr = "Alt Bs";       break;
       case VIOKEY_ALTBSLASH:      keystr = "Alt Bslash";   break;
       case VIOKEY_ALTCLOSEB:      keystr = "Alt Closeb";   break;
       case VIOKEY_ALTCOMMA:       keystr = "Alt Comma";    break;
       case VIOKEY_ALTDEL:         keystr = "Alt Del";      break;
       case VIOKEY_ALTDOWN:        keystr = "Alt Down";     break;
       case VIOKEY_ALTEND:         keystr = "Alt End";      break;
       case VIOKEY_ALTENTER:       keystr = "Alt Enter";    break;
       case VIOKEY_ALTEQUAL:       keystr = "Alt Equal";    break;
       case VIOKEY_ALTESC:         keystr = "Alt Esc ";     break;
       case VIOKEY_ALTHOME:        keystr = "Alt Home";     break;
       case VIOKEY_ALTINS:         keystr = "Alt Ins";      break;
       case VIOKEY_ALTLEFT:        keystr = "Alt Left";     break;
       case VIOKEY_ALTMINUS:       keystr = "Alt Minus";    break;
       case VIOKEY_ALTOPENB:       keystr = "Alt Openb";    break;
       case VIOKEY_ALTPERIOD:      keystr = "Alt Period";   break;
       case VIOKEY_ALTPGDN:        keystr = "Alt Pgdn";     break;
       case VIOKEY_ALTPGUP:        keystr = "Alt Pgup";     break;
       case VIOKEY_ALTRIGHT:       keystr = "Alt Right";    break;
       case VIOKEY_ALTSEMI:        keystr = "Alt Semi";     break;
       case VIOKEY_ALTSLASH:       keystr = "Alt Slash";    break;
       case VIOKEY_ALTTAB:         keystr = "Alt Tab";      break;
       case VIOKEY_ALTUP:          keystr = "Alt Up";       break;

/*keypad*/
       case VIOKEY_KPSLASH:        keystr = "Keypad Slash"; break;
       case VIOKEY_KPSTAR:         keystr = "Keypad Star";  break;
       case VIOKEY_KPMINUS:        keystr = "Keypad Minus"; break;
       case VIOKEY_KPPLUS:         keystr = "Keypad Plus";  break;
       case VIOKEY_KPENTER:        keystr = "Keypad Enter"; break;

       case VIOKEY_SHIFTKPSLASH:   keystr = "Shift KPSlash"; break;
       case VIOKEY_SHIFTKPSTAR:    keystr = "Shift KPStar";  break;
       case VIOKEY_SHIFTKPMINUS:   keystr = "Shift KPMinus"; break;
       case VIOKEY_SHIFTKPPLUS:    keystr = "Shift KPPlus";  break;
       case VIOKEY_SHIFTKPENTER:   keystr = "Shift KPEnter"; break;

       case VIOKEY_CTRLKPSLASH:    keystr = "Ctrl KPSlash"; break;
       case VIOKEY_CTRLKPSTAR:     keystr = "Ctrl KPStar";  break;
       case VIOKEY_CTRLKPMINUS:    keystr = "Ctrl KPMinus"; break;
       case VIOKEY_CTRLKPPLUS:     keystr = "Ctrl KPPlus";  break;
       case VIOKEY_CTRLKPENTER:    keystr = "Ctrl KPEnter"; break;

       case VIOKEY_ALTKPSLASH:     keystr = "Alt KPSlash";  break;
   /*- case VIOKEY_ALTKPSTAR:      keystr = "Alt KPStar";   break; -*/
       case VIOKEY_ALTKPMINUS:     keystr = "Alt KPMinus";  break;
       case VIOKEY_ALTKPPLUS:      keystr = "Alt KPPlus ";  break;
       case VIOKEY_ALTKPENTER:     keystr = "Alt KPEnter";  break;

       default:
         keystr = "Unknown";
         if (key <= 255) {
            if ( (key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z') )
               keystr = "Alpha";
            else if (key >= '0' && key <= '9')
               keystr = "Number";
            else if (key == ' ')
               keystr = "Space";
            else if (strchr("-=\\[];',./_+|{}:\"<>?", key))
               keystr = "Special";
            else if (key < 32)
               keystr = "Control";
            else
               keystr = "Misc";
         }
         break;
       }
       printf("%s\n", keystr);
    }
    return 0;
}
#endif /*STANDALONE*/

#endif /*VIO_BUFFER(DJGPP || WATCOMC)*/
