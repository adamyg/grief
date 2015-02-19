#ifndef GR_VIO_H_INCLUDED
#define GR_VIO_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_vio_h,"$Id: vio.h,v 1.23 2014/10/26 22:13:14 ayoung Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: vio.h,v 1.23 2014/10/26 22:13:14 ayoung Exp $
 * Video I/O interface header.
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

#if defined(__OS2__)
#define INCL_WIN                                /* legacy code */
#define INCL_GPI
#define INCL_VIO
#define INCL_AVIO
#include <os2.h>

#else                                           /* support environments */
#if !defined(DJGPP) && !defined(__WATCOMC__) && !defined(WIN32)
#error  Unsupported environment ...
#endif
#define UCHAR               unsigned char
#define USHORT              unsigned short
#define ULONG               unsigned long
#define PULONG              ULONG *
#define HVIO                int

#define VIOKEY_NONE         (-1)                /* Special for Idle task */
#define VIOKEY_CTRLBREAK    (0)                 /* Only if ctrl-break disabled */

#define VIOKEY_CTRLA        (1)
#define VIOKEY_CTRLB        (2)
#define VIOKEY_CTRLC        (3)
#define VIOKEY_CTRLD        (4)
#define VIOKEY_CTRLE        (5)
#define VIOKEY_CTRLF        (6)
#define VIOKEY_CTRLG        (7)
#define VIOKEY_CTRLH        (8)
#define VIOKEY_CTRLI        (9)
#define VIOKEY_CTRLJ        (10)
#define VIOKEY_CTRLK        (11)
#define VIOKEY_CTRLL        (12)
#define VIOKEY_CTRLM        (13)
#define VIOKEY_CTRLN        (14)
#define VIOKEY_CTRLO        (15)
#define VIOKEY_CTRLP        (16)
#define VIOKEY_CTRLQ        (17)
#define VIOKEY_CTRLR        (18)
#define VIOKEY_CTRLS        (19)
#define VIOKEY_CTRLT        (20)
#define VIOKEY_CTRLU        (21)
#define VIOKEY_CTRLV        (22)
#define VIOKEY_CTRLW        (23)
#define VIOKEY_CTRLX        (24)
#define VIOKEY_CTRLY        (25)
#define VIOKEY_CTRLZ        (26)

#define VIOKEY_F1           (315)
#define VIOKEY_F2           (316)
#define VIOKEY_F3           (317)
#define VIOKEY_F4           (318)
#define VIOKEY_F5           (319)
#define VIOKEY_F6           (320)
#define VIOKEY_F7           (321)
#define VIOKEY_F8           (322)
#define VIOKEY_F9           (323)
#define VIOKEY_F10          (324)
#define VIOKEY_F11          (389)               /* 101 only */
#define VIOKEY_F12          (390)               /* 101 only */
#define VIOKEY_SF1          (340)
#define VIOKEY_SF2          (341)
#define VIOKEY_SF3          (342)
#define VIOKEY_SF4          (343)
#define VIOKEY_SF5          (344)
#define VIOKEY_SF6          (345)
#define VIOKEY_SF7          (346)
#define VIOKEY_SF8          (347)
#define VIOKEY_SF9          (348)
#define VIOKEY_SF10         (349)
#define VIOKEY_SF11         (391)               /* 101 only */
#define VIOKEY_SF12         (392)               /* 101 only */
#define VIOKEY_CF1          (350)
#define VIOKEY_CF2          (351)
#define VIOKEY_CF3          (352)
#define VIOKEY_CF4          (353)
#define VIOKEY_CF5          (354)
#define VIOKEY_CF6          (355)
#define VIOKEY_CF7          (356)
#define VIOKEY_CF8          (357)
#define VIOKEY_CF9          (358)
#define VIOKEY_CF10         (359)
#define VIOKEY_CF11         (393)               /* 101 only */
#define VIOKEY_CF12         (394)               /* 101 only */
#define VIOKEY_AF1          (360)
#define VIOKEY_AF2          (361)
#define VIOKEY_AF3          (362)
#define VIOKEY_AF4          (363)
#define VIOKEY_AF5          (364)
#define VIOKEY_AF6          (365)
#define VIOKEY_AF7          (366)
#define VIOKEY_AF8          (367)
#define VIOKEY_AF9          (368)
#define VIOKEY_AF10         (369)
#define VIOKEY_AF11         (395)               /* 101 only */
#define VIOKEY_AF12         (396)               /* 101 only */

#define VIOKEY_ALT1         (376)
#define VIOKEY_ALT2         (377)
#define VIOKEY_ALT3         (378)
#define VIOKEY_ALT4         (379)
#define VIOKEY_ALT5         (380)
#define VIOKEY_ALT6         (381)
#define VIOKEY_ALT7         (382)
#define VIOKEY_ALT8         (383)
#define VIOKEY_ALT9         (384)
#define VIOKEY_ALT0         (385)
#define VIOKEY_ALTA         (286)
#define VIOKEY_ALTB         (304)
#define VIOKEY_ALTC         (302)
#define VIOKEY_ALTD         (288)
#define VIOKEY_ALTE         (274)
#define VIOKEY_ALTF         (289)
#define VIOKEY_ALTG         (290)
#define VIOKEY_ALTH         (291)
#define VIOKEY_ALTI         (279)
#define VIOKEY_ALTJ         (292)
#define VIOKEY_ALTK         (293)
#define VIOKEY_ALTL         (294)
#define VIOKEY_ALTM         (306)
#define VIOKEY_ALTN         (305)
#define VIOKEY_ALTO         (280)
#define VIOKEY_ALTP         (281)
#define VIOKEY_ALTQ         (272)
#define VIOKEY_ALTR         (275)
#define VIOKEY_ALTS         (287)
#define VIOKEY_ALTT         (276)
#define VIOKEY_ALTU         (278)
#define VIOKEY_ALTV         (303)
#define VIOKEY_ALTW         (273)
#define VIOKEY_ALTX         (301)
#define VIOKEY_ALTY         (277)
#define VIOKEY_ALTZ         (300)

#define VIOKEY_HOME         (327)               /* Home                 */
#define VIOKEY_END          (335)               /* End                  */
#define VIOKEY_UP           (328)               /* Cursor up            */
#define VIOKEY_DOWN         (336)               /* Cursor down          */
#define VIOKEY_PGUP         (329)               /* Page up              */
#define VIOKEY_PGDN         (337)               /* Page down            */
#define VIOKEY_LEFT         (331)               /* Cursor left          */
#define VIOKEY_INS          (338)               /* Insert               */
#define VIOKEY_RIGHT        (333)               /* Right                */
#define VIOKEY_DEL          (339)               /* Delete               */
#define VIOKEY_CNTR         (332)               /* Center               */

#define VIOKEY_STAB         (271)               /* Shift TAB            */
#define VIOKEY_SHIFTDEL     (0x200|0x53)        /* Shift Del            */
#define VIOKEY_SHIFTDOWN    (0x200|0x50)        /* Shift Cursor down    */
#define VIOKEY_SHIFTEND     (0x200|0x4f)        /* Shift End            */
#define VIOKEY_SHIFTCNTR    (0X200|0X4c)        /* Shift Center         */
#define VIOKEY_SHIFTHOME    (0x200|0x47)        /* Shift Home           */
#define VIOKEY_SHIFTINS     (0x200|0x52)        /* Shift Ins            */
#define VIOKEY_SHIFTLEFT    (0x200|0x4b)        /* Shift Cursor left    */
#define VIOKEY_SHIFTPGDN    (0x200|0x51)        /* Shift Page down      */
#define VIOKEY_SHIFTPGUP    (0x200|0x49)        /* Shift Page up        */
#define VIOKEY_SHIFTRIGHT   (0x200|0x4d)        /* Shift Right          */
#define VIOKEY_SHIFTUP      (0x200|0x48)        /* Shift Cursor up      */

#define VIOKEY_CTRLBS       (127)               /* Ctrl Backspace       */
#define VIOKEY_CTRLBSLASH   (28)                /* Ctrl \               */
#define VIOKEY_CTRLCLOSEB   (29)                /* Ctrl Close bracket   */
#define VIOKEY_CTRLCNTR     (399)               /* Ctrl Center          */
#define VIOKEY_CTRLDEL      (403)               /* Ctrl Delete          */
#define VIOKEY_CTRLDOWN     (401)               /* Ctrl Cursor down     */
#define VIOKEY_CTRLEND      (373)               /* Ctrl End             */
#define VIOKEY_CTRLHOME     (375)               /* Ctrl Home            */
#define VIOKEY_CTRLINS      (402)               /* Ctrl Insert          */
#define VIOKEY_CTRLLEFT     (371)               /* Ctrl Cursor left     */
#define VIOKEY_CTRLMINUS    (31)                /* Ctrl -               */
#define VIOKEY_CTRLOPENB    (0x200|27)          /* Ctrl Open bracket    */
#define VIOKEY_CTRLPGDN     (388)               /* Ctrl PgDn            */
#define VIOKEY_CTRLPGUP     (374)               /* Ctrl PgUp            */
#define VIOKEY_CTRLPRTSC    (370)               /* Ctrl Print Screen    */
#define VIOKEY_CTRLRIGHT    (372)               /* Ctrl Cursor right    */
#define VIOKEY_CTRLTAB      (404)               /* Ctrl Tab             */
#define VIOKEY_CTRLUP       (397)               /* Ctrl Cursor up       */

#define VIOKEY_ALTACCENT    (297)               /* Alt `                */
#define VIOKEY_ALTAPOST     (296)               /* Alt ' Apostrophe     */
#define VIOKEY_ALTBS        (270)               /* Alt Backspace        */
#define VIOKEY_ALTBSLASH    (299)               /* Alt \                */
#define VIOKEY_ALTCLOSEB    (283)               /* Alt close bracket    */
#define VIOKEY_ALTCOMMA     (307)               /* Alt ,                */
#define VIOKEY_ALTDEL       (419)               /* Alt Del              */
#define VIOKEY_ALTDOWN      (416)               /* Alt Cursor down      */
#define VIOKEY_ALTEND       (415)               /* Alt End              */
#define VIOKEY_ALTENTER     (284)               /* Alt enter            */
#define VIOKEY_ALTEQUAL     (387)               /* Alt =                */
#define VIOKEY_ALTESC       (257)               /* Alt Esc              */
#define VIOKEY_ALTHOME      (407)               /* Alt Home             */
#define VIOKEY_ALTINS       (418)               /* Alt Ins              */
#define VIOKEY_ALTLEFT      (411)               /* Alt Cursor left      */
#define VIOKEY_ALTMINUS     (386)               /* Alt -                */
#define VIOKEY_ALTOPENB     (282)               /* Alt open bracket     */
#define VIOKEY_ALTPERIOD    (308)               /* Alt .                */
#define VIOKEY_ALTPGDN      (417)               /* Alt PgDn             */
#define VIOKEY_ALTPGUP      (409)               /* Alt PgUp             */
#define VIOKEY_ALTRIGHT     (413)               /* Alt Cursor right     */
#define VIOKEY_ALTSEMI      (295)               /* Alt Semicolon        */
#define VIOKEY_ALTSLASH     (309)               /* Alt /                */
#define VIOKEY_ALTSTAR      (311)               /* Alt *                */
#define VIOKEY_ALTTAB       (421)               /* Alt Tab              */
#define VIOKEY_ALTUP        (408)               /* Alt Cursor up        */

#define VIOKEY_KPSLASH      (0x200|'/')         /* Keypad /             */
#define VIOKEY_KPSTAR       (0x200|'*')         /* Keypad *             */
#define VIOKEY_KPMINUS      (0x200|'-')         /* Keypad -             */
#define VIOKEY_KPPLUS       (0x200|'+')         /* Keypad +             */
#define VIOKEY_KPENTER      (0x200|0x0d)        /* Keypad Enter         */

#define VIOKEY_SHIFTKPSLASH (0x300|'/')         /* Shift Keypad /       */
#define VIOKEY_SHIFTKPSTAR  (0x300|'*')         /* Shift Keypad *       */
#define VIOKEY_SHIFTKPMINUS (0x300|'-')         /* Shift Keypad -       */
#define VIOKEY_SHIFTKPPLUS  (0x300|'+')         /* Shift Keypad +       */
#define VIOKEY_SHIFTKPENTER (0x300|0x0d)        /* Shift Keypad Enter   */

#define VIOKEY_CTRLKPSLASH  (405)               /* Ctrl Keypad /        */
#define VIOKEY_CTRLKPSTAR   (406)               /* Ctrl Keypad *        */
#define VIOKEY_CTRLKPMINUS  (398)               /* Ctrl Keypad -        */
#define VIOKEY_CTRLKPPLUS   (400)               /* Ctrl Keypad +        */
#define VIOKEY_CTRLKPENTER  (0x300|0x0a)        /* Ctrl Keypad Enter    */

#define VIOKEY_ALTKPSLASH   (420)               /* Alt keypad /         */
#define VIOKEY_ALTKPMINUS   (330)               /* Alt Keypad -         */
#define VIOKEY_ALTKPPLUS    (334)               /* Alt Keypad +         */
#define VIOKEY_ALTKPENTER   (422)               /* Alt keypad enter     */

#define VIOCOL_BLACK_F      (0x00)
#define VIOCOL_BLUE_F       (0x01)
#define VIOCOL_GREEN_F      (0x02)
#define VIOCOL_CYAN_F       (0x03)
#define VIOCOL_RED_F        (0x04)
#define VIOCOL_MAGENTA_F    (0x05)
#define VIOCOL_BROWN_F      (0x06)
#define VIOCOL_GREY_F       (0x07)
#define VIOCOL_BRITE        (0x08)
#define VIOCOL_LTBLUE_F     (VIOCOL_BLUE_F    | VIOCOL_BRITE)
#define VIOCOL_LTGREEN_F    (VIOCOL_GREEN_F   | VIOCOL_BRITE)
#define VIOCOL_LTCYAN_F     (VIOCOL_CYAN_F    | VIOCOL_BRITE)
#define VIOCOL_LTRED_F      (VIOCOL_RED_F     | VIOCOL_BRITE)
#define VIOCOL_LTMAGENTA_F  (VIOCOL_MAGENTA_F | VIOCOL_BRITE)
#define VIOCOL_YELLOW_F     (VIOCOL_BROWN_F   | VIOCOL_BRITE)
#define VIOCOL_WHITE_F      (VIOCOL_GREY_F    | VIOCOL_BRITE)
#define VIOCOL_BLACK_B      (0x00)
#define VIOCOL_BLUE_B       (0x10)
#define VIOCOL_GREEN_B      (0x20)
#define VIOCOL_CYAN_B       (0X30)
#define VIOCOL_RED_B        (0x40)
#define VIOCOL_MAGENTA_B    (0x50)
#define VIOCOL_BROWN_B      (0x60)
#define VIOCOL_GREY_B       (0x70)
#define VIOCOL_FLASH        (0x80)

#define VIOVID_UNDEF        (-1)                /* Internal video modes */
#define VIOVID_MONO         (-110)
#define VIOVID_CO40x12      (-111)
#define VIOVID_BW40x25      (-112)
#define VIOVID_CO40x14      (-113)
#define VIOVID_CO40x25      (-114)
#define VIOVID_CO40x28      (-115)
#define VIOVID_BW80x25      (-116)
#define VIOVID_CO80x25      (-117)
#define VIOVID_CO80x12      (-118)
#define VIOVID_CO80x14      (-119)
#define VIOVID_CO80x28      (-120)
#define VIOVID_CO80x43      (-121)
#define VIOVID_CO80x50      (-122)

#define VIOVID_BIOS40BW     0                   /* BIOS video modes */
#define VIOVID_BIOS40COLOUR 1
#define VIOVID_BIOS80BW     2
#define VIOVID_BIOS80COLOUR 3
#define VIOVID_BIOS80MONO   7

#if defined(WIN32)

#ifndef WIN32_CONSOLENORM
#define WIN32_CONSOLEEXT                        /* extended console */
#endif

#if defined(WIN32_CONSOLEEXT)
/*
 *  Extended console support ...
 *      BG/8 + FG/8 + STYLE/16 + CHAR/32
 */
typedef uint16_t VIOHUE;
typedef struct {                                /* VIO cell unit (extended unicode) */
    VIOHUE       attribute;                     /* FG + BG */
    uint16_t     style;
#define VIO_UNDERLINE       0x0001
#define VIO_ITALIC          0x0002
#define VIO_COLOR256        0x1000
    uint32_t    character;                      /* unicode code */
} VIOCELL;

#define VIO_ATTR_MASK       0xffff              /* 0xffff */
#define VIO_FG_SHIFT        0                   /* 0x00ff */
#define VIO_BG_SHIFT        8                   /* 0xff00 */
#define VIO_FB_MASK         0xff
#define VIO_FG(_f)          ((_f) << VIO_FG_SHIFT)
#define VIO_BG(_b)          ((_b) << VIO_BG_SHIFT)
#define VIO_FGBG(_f, _b)    ((VIOHUE)(VIO_FG(_f)|VIO_BG(_b)))
#define VIO_INIT(_a, _c)    { _a, 0, _c }
#define VIO_ASSIGN(_v, _a, _c, _s) { \
                    _v->attribute = _a; \
                    _v->style     = _s; \
                    _v->character = _c; \
                    }
#define VIO_IMPORT(_v, _a, _c) { \
                    const VIOHUE fg = (_a & 0x0f), \
                            bg = ((_a >> 4) & 0x0f); \
                    _v->attribute = VIO_FGBG(fg, bg); \
                    _v->style     = 0;  \
                    _v->character = _c; \
                    }
#define VIO_EXPORT(_cell) \
                    ((((_cell.attribute >> VIO_BG_SHIFT) & 0xf) << 4) \
                     |((_cell.attribute >> VIO_FG_SHIFT) & 0xf))
#define VIO_CHAR(_cell)     (_cell.character)

#else   //!WIN32_CONSOLEEXT
/*
 *  Generic Unicode
 *      32bit cell - BG/4 + FG/4 + CHAR/24
 */
typedef uint32_t VIOHUE;
typedef uint32_t VIOCELL;                       /* VIO cell unit (unicode) */

#define VIO_CHAR_MASK       0x001fffff          /* 0x00 00 ff ff ff */
#define VIO_CHAR_SHIFT      0
#define VIO_ATTR_MASK       0xff000000          /* 0xff 00 00 00 00 */
#define VIO_ATTR_SHIFT      24
#define VIO_FG_SHIFT        24                  /* 0x0f 00 00 00 00 */
#define VIO_BG_SHIFT        28                  /* 0xf0 00 00 00 00 */
#define VIO_FB_MASK         0xf
#endif

#else   //!WIN32
/*
 *  Generic ANSI/8BIT
 *      16bit cell - BG/4 + FG/4 + CHAR/8
 */
typedef uint16_t VIOHUE;
typedef uint16_t VIOCELL;                       /* VIO cell unit (ascii only) */

#define VIO_CHAR_MASK       0x00ff              /* 0x00 ff */
#define VIO_CHAR_SHIFT      0
#define VIO_ATTR_MASK       0xff00              /* 0xff 00 */
#define VIO_ATTR_SHIFT      8
#define VIO_FG_SHIFT        8                   /* 0x0f 00 */
#define VIO_BG_SHIFT        12                  /* 0xf0 00 */
#endif

#if !defined(VIO_CHAR)
#define VIO_FG(_f)          ((_f) << VIO_FG_SHIFT)
#define VIO_BG(_b)          ((_b) << VIO_BG_SHIFT)
#define VIO_FGBG(_f, _b)    ((VIO_FG(_f)|VIO_BG(_b)) & VIO_ATTR_MASK)
#define __VIO_MAKE(_a, _c)  ((VIOCELL)(((_a) & VIO_ATTR_MASK) | ((_c) & VIO_CHAR_MASK)))
#define VIO_INIT(_a,_c)     __VIO_MAKE(_a, _c)
#define VIO_ASSIGN(_v, _a, _c, _s) \
                { *_v = __VIO_MAKE(_a, _c); }
#define VIO_IMPORT(_v,_a,_c) \
                { *_v = ((VIOCELL)(((_a) << VIO_ATTR_SHIFT) | ((_c) & VIO_CHAR_MASK))); }
#define VIO_CHAR(_cell)     ((_cell) & VIO_CHAR_MASK)
#define VIO_EXPORT(_cell)   ((_cell) >> VIO_ATTR_SHIFT)
#endif

typedef enum {
    VIOCUR_STATE,                               /* Cursor status */
    VIOCUR_OFF,
    VIOCUR_ON,
    VIOCUR_BHALF,
    VIOCUR_THALF,
    VIOCUR_FULL
} VIOCURSOR;

typedef struct {
    USHORT              cb;                     /* Structure size */
    UCHAR               fbType;                 /* Bit mask of mode being set. */
    USHORT              color;                  /* Number of colors (OS/2 -- power of 2). */
    USHORT              col;                    /* The number of text columns. */
    USHORT              row;                    /* The number of text rows. */
    USHORT              hres;                   /* Horizontal resolution. */
    USHORT              vres;                   /* Vertical resolution. */
    UCHAR               fmt_ID;                 /* Attribute format. */
    UCHAR               attrib;                 /* Number of attributes. */
    USHORT              resv;                   /* Reserved. */
    ULONG               buf_addr;               /* Video aperture address. */
    ULONG               buf_length;             /* Video aperture length. */
    ULONG               full_length;            /* Video state full save length. */
    ULONG               partial_length;         /* Video state partial save length. */
    ULONG               ext_data_addr;          /* Extra data address. */
} VIOMODEINFO;

typedef struct {
    USHORT              cb;                     /* Structure size */
    USHORT              yStart;                 /* Cursor start line. */
    USHORT              cEnd;                   /* Cursor end line. */
    USHORT              cx;                     /* Cursor width. */
    USHORT              attr;                   /* -1 =hidden cursor. */
    DWORD               mask;                   /* cursor line maks, extension */
} VIOCURSORINFO;

    /* OS2/NT/VIO interface compat */
extern int                  VioGetMode(VIOMODEINFO *mode, HVIO viohandle);
extern int                  VioSetMode(VIOMODEINFO *mode, HVIO viohandle);
extern int                  VioGetCurType(VIOCURSORINFO *info, HVIO viohandle);
extern int                  VioSetCurType(VIOCURSORINFO *info, HVIO viohandle);
extern int                  VioGetCp(ULONG reserved, USHORT *cp, HVIO viohandle);
extern int                  VioGetCurPos(USHORT *row, USHORT *col, HVIO viohandle);
extern int                  VioSetCurPos(USHORT row, USHORT col, HVIO viohandle);
extern int                  VioGetBuf(VIOCELL **pBuf, ULONG *size, HVIO viohandle);
extern int                  VioShowBuf(ULONG offset, ULONG length, HVIO viohandle);
extern int                  VioReadCellStr(VIOCELL *buf, ULONG *length, USHORT row, USHORT col, HVIO viohandle);

    /* MSDOS extensions */
extern int                  VioCursor(VIOCURSOR state);
extern int                  VioSetFont(const char *font);
extern int                  VioGetFont(char *font, int buflen);
extern int                  VioPageGet(void);
extern void                 VioPageSet(int page);
extern int                  VioCodePageGet(void);
extern int                  VioKeyHit(void);
extern int                  VioKeyGet(void);
extern int                  VioPutChar(USHORT row, USHORT col, int ch, int attr);
extern void                 VioRefresh(void);
extern int                  VioSetFocus(int setfocus);
extern int                  VioSetColors(int colors);
extern int                  VioGetColors(int *colors);

    /* Misc functions */
extern void                 DosBeep(int freq, int duration);

#endif
#endif /*GR_VIO_H_INCLUDED*/
