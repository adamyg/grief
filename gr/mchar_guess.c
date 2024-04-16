#include <edidentifier.h>
__CIDENT_RCSID(gr_mchar_guess_c,"$Id: mchar_guess.c,v 1.33 2024/04/16 10:30:36 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: mchar_guess.c,v 1.33 2024/04/16 10:30:36 cvsuser Exp $
 * Character-set conversion/file type guess logic.
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

#include <editor.h>
#include <edendian.h>
#include <libstr.h>                             /* str_...()/sxprintf() */
#include <asciidefs.h>                          /* ASCIIDEF_... */

#if defined(HAVE_LIBCHARUDET_H) && defined(HAVE_LIBCHARUDET)
#include "../libcharudet/libcharudet.h"
#include <libcharudet.h>                        /* libchardet interface */
#endif

#if defined(HAVE_LIBMAGIC)                      /* libmagic */
#if defined(HAVE_MAGIC_H)
#if defined(GNUWIN32_LIBMAGIC)
#include <gnuwin32_magic.h>
#else
#include <magic.h>
#endif
#endif
#endif

#if defined(HAVE_LIBGUESS)                      /* libguess */
#if defined(HAVE_LIBGUESS_LIBGUESS_H)
#include <libguess/libguess.h>
#elif defined(HAVE_LIBGUESS_H)
#include <libguess.h>
#else
#undef HAVE_LIBGUESS
#endif
#endif

#if defined(HAVE_LIBENCA)                       /* libenca */
#if defined(HAVE_ENCA_ENCA_H)
#include <enca/enca.h>
#elif defined(HAVE_ENCA_H)
#include <enca.h>
#else
#undef HAVE_LIBENCA
#endif
#endif

#include "mchar.h"                              /* public interface */

#include "buffer.h"                             /* buf_...() */
#include "debug.h"                              /* trace_...() */
#include "main.h"                               /* xf_test */
#include "../libchartable/libchartable.h"

#if defined(HOST_BIG_ENDIAN)
#define DEFENDIAN       1
#elif defined(HOST_LITTLE_ENDIAN)
#define DEFENDIAN       0
#else
#error Unsupported target architecture ...
#endif

#if defined(DOSISH)
#define NULPERCENTAGE   5                       /* 5% */
#else
#define NULPERCENTAGE   1
#endif

typedef struct {        /* line terminator info */
    int32_t             state1;
    unsigned            nul;
    unsigned            esc;
    unsigned            crlf;
    unsigned            lf;
    unsigned            cr;
    unsigned            nel;
    unsigned            ucslf;
    unsigned            ucsbr;
} guessterm_t;

typedef struct {        /* file type guess support */
    mcharguessinfo_t    gi_fileinfo;
#define gi_type         gi_fileinfo.fi_type
#define gi_endian       gi_fileinfo.fi_endian
#define gi_flags        gi_fileinfo.fi_flags
#define gi_encoding     gi_fileinfo.fi_encoding
#define gi_termtype     gi_fileinfo.fi_termtype
#define gi_termlen      gi_fileinfo.fi_termlen
#define gi_termbuf      gi_fileinfo.fi_termbuf

    BUFTYPE_t           gi_bomtype;
    unsigned            gi_bomsalt;
    const struct bomdesc *gi_bommagic;

#define gi_bomdes       gi_fileinfo.fi_bomdes
#define gi_bomlen       gi_fileinfo.fi_bomlen
#define gi_bombuf       gi_fileinfo.fi_bombuf

    unsigned            gi_charflags;
    unsigned            gi_chartally[256];      /* tally by character */
    guessterm_t         gi_linebreak;
} guessinfo_t;

struct guessdecoder;

static int                      decodercall(guessinfo_t *guess, const struct guessdecoder *decoder,
                                        const void *buffer, unsigned length, const char *argument);
static const struct guessdecoder *decodernext(char **optionp, const char **valuep);

static void                     guess_summaries(guessinfo_t *guess,
                                        const unsigned char *map, const void *buffer, unsigned length);

static int                      bomcheck(const char *buffer, int length, guessinfo_t *guess);
static unsigned                 tallycount(const unsigned *chartally, unsigned char mask);
static int                      linereset(guessterm_t  *linebreak);
static int                      linetally(guessterm_t  *linebreak, int32_t ch);
static int                      setencoding(guessinfo_t *guess, const char *encoding, int length);

static void                     guess_lineterm(guessinfo_t *guess);

static int                      guess_charset(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_charudet(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_guess(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_enca(guessinfo_t *guess, const void *buffer, unsigned length);

static int                      guess_bom(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_marker(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      markerchars(const guessinfo_t *guess);
static const void *             markercheck(const void *text, const void *end, int *encodinglen);

static int                      guess_magic(guessinfo_t *guess, const void *buffer, unsigned length);

static int                      guess_utf32bom(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_utf32xx(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_utf32(guessinfo_t *guess, const void *buffer, unsigned length, int endian);

static int                      guess_utf16bom(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_utf16xx(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_utf16(guessinfo_t *guess, const void *buffer, unsigned length, int endian);

static int                      guess_utf8(guessinfo_t *guess, const void *buffer, unsigned length);

static int                      guess_ascii(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_latin1(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_xascii(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_ebcdic(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_binary(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_big5(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_gb18030(guessinfo_t *guess, const void *buffer, unsigned length);
static int                      guess_shiftjis(guessinfo_t *guess, const void *buffer, unsigned length);


/*
 *  File-type decoders.
 */
#define NAME(__n)               __n, sizeof(__n)-1

#define X_GUESSDECODERS (sizeof(x_guessdecoders)/sizeof(x_guessdecoders[0]))

static const struct guessdecoder {
    const char *        name;
    size_t              namelen;
    int              (* guess)(guessinfo_t *guess, const void *buffer, unsigned len);
    unsigned            flags;
#define GUESS_FDEFAULT          0x0001
#define GUESS_FEBCDIC           0x0002

} x_guessdecoders[] = {
    /*
     *  order = default scanner.
     *
     *      mark -          Encoding: <marker>
     *      utf32bom -      UTF-32 BOM marker.
     *      utf16bom -      UTF-16 BOM marker.
     *      udet -          Mozilla universal character detector (libcharudet).
     *      magic -         File magic (libmagic).
     *      guess -         Guesser (libguess).
     *      enca -          Extremely Naive Charset Analyser (libenca).
     *
     *  Note, order is some-what important as the MBCS checks can result in
     *  false positives, as such are generally last in line.
     *
     *  Module usage has evolved over time, with a number being pure
     *  experimental alternatives.
     */
    { NAME("mark"),             guess_marker,           GUESS_FDEFAULT },
    { NAME("utf32bom"),         guess_utf32bom,         GUESS_FDEFAULT },
    { NAME("utf16bom"),         guess_utf16bom,         GUESS_FDEFAULT },
    { NAME("utf8"),             guess_utf8,             GUESS_FDEFAULT },
    { NAME("utf32"),            guess_utf32xx,          GUESS_FDEFAULT },
    { NAME("utf16"),            guess_utf16xx,          GUESS_FDEFAULT },
    { NAME("bom"),              guess_bom,              GUESS_FDEFAULT },
    { NAME("udet"),             guess_charudet,         GUESS_FDEFAULT },
    { NAME("magic"),            guess_magic,            GUESS_FDEFAULT },
    { NAME("binary"),           guess_binary,           GUESS_FDEFAULT | GUESS_FEBCDIC },
    { NAME("ascii"),            guess_ascii,            GUESS_FDEFAULT | GUESS_FEBCDIC },
    { NAME("latin1"),           guess_latin1,           GUESS_FDEFAULT | GUESS_FEBCDIC },
    { NAME("big5"),             guess_big5,             GUESS_FDEFAULT },
    { NAME("gb18030"),          guess_gb18030,          GUESS_FDEFAULT },
    { NAME("shiftjis"),         guess_shiftjis,         GUESS_FDEFAULT },
    { NAME("xascii"),           guess_xascii,           GUESS_FDEFAULT },
    { NAME("charset"),          guess_charset,          0 },
    { NAME("guess"),            guess_guess,            0 },
    { NAME("enca"),             guess_enca,             0 },
    { NAME("ebcdic"),           guess_ebcdic,           0 },
//  { NAME("default"),          guess_default,          0 }
    };

/*
 *  Encoding line terminator modifies
 */
#if (0)
#define X_LINESPEC      (sizeof(x_linespec)/sizeof(x_linespec[0]))

static const struct linespec {
    const char *        name;
    size_t              namelen;
    int                 type;

} x_linespec[] = {
    /* line specifiers */
    { NAME("unix"),     LTERM_UNIX  },
    { NAME("dos"),      LTERM_DOS   },
    { NAME("mac"),      LTERM_MAC   },
    { NAME("nel"),      LTERM_NEL   },
    { NAME("ucsnl"),    LTERM_UCSNL },
    };
#endif


/*
 *  Byte-Order-Marker (BOM's)
 */
#define X_BOMMAGIC      (sizeof(x_bommagic)/sizeof(x_bommagic[0]))

static const struct bomdesc {
    const char *        name;
    size_t              namelen;
    const char *        des;
    unsigned            len;
    unsigned char       bom[4];
    BUFTYPE_t           type;
    unsigned            salt;
#define SALT_BE                 0x0001
#define SALT_LE                 0x0002
#define SALT_BAD                0x0010

} x_bommagic[] = {
    /* order by size */
    { NAME(MCHAR_UTF32BE),      "UTF-32 Big Endian",        4,  {0X00,0X00,0XFE,0XFF},  BFTYP_UTF32,        SALT_BE },
    { NAME(MCHAR_UTF32LE),      "UTF-32 Little Endian",     4,  {0XFF,0XFE,0X00,0X00},  BFTYP_UTF32,        SALT_LE },
    { NAME(MCHAR_UTF32XX),      "UTF-32 Unknown Order",     4,  {0X00,0X00,0XFF,0XFE},  BFTYP_UNSUPPORTED,  0x0100  },
    { NAME(MCHAR_UTF32XX),      "UTF-32 Unknown Order",     4,  {0XFE,0XFF,0X00,0X00},  BFTYP_UNSUPPORTED,  0x0101  },
    { NAME(MCHAR_UTF7),         "UTF-7",                    4,  {0X2B,0X2F,0X76,0X38},  BFTYP_UNSUPPORTED,  0x0200  },
    { NAME(MCHAR_UTF7),         "UTF-7",                    4,  {0X2B,0X2F,0X76,0X39},  BFTYP_UNSUPPORTED,  0x0201  },
    { NAME(MCHAR_UTF7),         "UTF-7",                    4,  {0X2B,0X2F,0X76,0X2B},  BFTYP_UNSUPPORTED,  0x0202  },
    { NAME(MCHAR_UTF7),         "UTF-7",                    4,  {0X2B,0X2F,0X76,0X2F},  BFTYP_UNSUPPORTED,  0x0204  },
    { NAME(MCHAR_UTFEBCDIC),    "UTF-EBCDIC",               4,  {0XDD,0X73,0X66,0X73},  BFTYP_UTFEBCDIC     },
    { NAME(MCHAR_GB18030),      "GB18030",                  4,  {0X84,0X31,0X95,0X33},  BFTYP_GB            },
    { NAME(MCHAR_BOCU1),        "BOCU-1",                   4,  {0XFB,0XEE,0X28,0xFF},  BFTYP_BOCU1         },
    { NAME(MCHAR_BOCU1),        "BOCU-1",                   3,  {0XFB,0XEE,0X28     },  BFTYP_BOCU1         },
    { NAME(MCHAR_SCSU),         "SCSU",                     3,  {0X0E,0XFE,0XFF     },  BFTYP_SCSU          },
    { NAME(MCHAR_UTF1),         "UTF-1",                    3,  {0XF7,0X64,0X4C     },  BFTYP_UNSUPPORTED,  0x0300  },
    { NAME(MCHAR_UTF8),         "UTF-8",                    3,  {0XEF,0XBB,0XBF     },  BFTYP_UTF8          },
    { NAME(MCHAR_UTF16BE),      "UTF-16 Big Endian",        2,  {0XFE,0XFF          },  BFTYP_UTF16,        SALT_BE },
    { NAME(MCHAR_UTF16LE),      "UTF-16 Little Endian",     2,  {0XFF,0XFE          },  BFTYP_UTF16,        SALT_LE }
    };


#define C0      0x01            /* Non-text/control character/c0 */
#define C1      0x02            /* Non-text/control character/c1/extended-ascii */
#define A       0x10            /* Plain ASCII text + plus standard controls */
#define X       0x20            /* Extended ASCII */
#define I       0x40            /* ISO-8859 (e.g. Latin1) text */

static const uint8_t
x_charflags[256] =              /* Character attribute flags, as above */
    {
 /*
  * NUL  SOH  STX  ETX  EOT  ENQ  ACK  BEL  BS   HT   NL   VT   NP   CR   SO   SI
  * DLE  DC1  DC2  DC3  DC4  NAK  SYN  ETB  CAN  EM   SUB  ESC  FS   GS   RS   US
  */
    C0,  C0,  C0,  C0,  C0,  C0,  C0,  A,   A,   A,   A,   C0,  A,   A,   C0,  C0,   /* 0x0X */
    C0,  C0,  C0,  C0,  C0,  C0,  C0,  C0,  C0,  C0,  C0,  A,   C0,  C0,  C0,  C0,   /* 0x1X */

 /*
  * !    "    #    $    %    &    '    (    )    *    +    ,    -    .    /    SP
  * 0    1    2    3    4    5    6    7    8    9    :    ;    <    =    >    ?
  * @    A    B    C    D    E    F    G    H    I    J    K    L    M    N    O
  * P    Q    R    S    T    U    V    W    X    Y    Z    [    \    ]    ^    _
  * `    a    b    c    d    e    f    g    h    i    j    k    l    m    n    o
  * p    q    r    s    t    u    v    w    x    y    z    {    |    }    ~    DEL
  */
    A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,    /* 0x2X */
    A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,    /* 0x3X */
    A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,    /* 0x4X */
    A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,    /* 0x5X */
    A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,    /* 0x6X */
    A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   A,   X,    /* 0x7X */

 /*
  * PAD  HOP  BPH  NBH  IND  NEL  SSA  ESA  HTS  HTJ  VTS  PLD  PLU  RI   SS2  SS3
  * DCS  PU1  PU2  STS  CCH  MW   SPA  EPA  SOS  SGC  SCI  CSI  ST   OSC  PM   APC
  */
    C1,  C1,  C1,  C1,  C1,  A,   C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,   /* 0x8X */
    C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,  C1,   /* 0x9X */

 /*
  *      ¡    ¢    £    ¤    ¥    ¦    §    ¨    ©    ª    «    ¬    ­    ®    ¯
  * °    ±    ²    ³    ´    µ    ¶    ·    ¸    ¹    º    »    ¼    ½    ¾    ¿
  * À    Á    Â    Ã    Ä    Å    Æ    Ç    È    É    Ê    Ë    Ì    Í    Î    Ï
  * Ð    Ñ    Ò    Ó    Ô    Õ    Ö    ×    Ø    Ù    Ú    Û    Ü    Ý    Þ    ß
  * à    á    â    ã    ä    å    æ    ç    è    é    ê    ë    ì    í    î    ï
  * ð    ñ    ò    ó    ô    õ    ö    ÷    ø    ù    ú    û    ü    ý    þ    ÿ
  */
    I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,    /* 0xaX */
    I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,    /* 0xbX */
    I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,    /* 0xcX */
    I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,    /* 0xdX */
    I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,    /* 0xeX */
    I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I,   I     /* 0xfX */
    };


#if (0)
static const uint8_t
x_ebcdic2ascii[256] =   /* CP037 - EBCDIC to ASCII mapping (see also Unicode Tech Reqport #16) */
    {
 /*
  * NUL  SOH  STX  ETX  SEL  HT   RNL  DEL  GE   SPS  RPT  VT   FF   CR   SO   SI
  * DLE  DC1  DC2  DC3  RES  NL   BS   POC  CAN  EM   UBS  CU1  IFS  IGS  IRS  IUS
  * DS   SOS  FS   WUS  BYP  LF   ETB  ESC  SA   SFE  SM   CSP  MFA  ENQ  ACK  BEL
  * .    .    SYN  IR   PP   TRN  NBS  EOT  SBS  IT   RFF  CU3  DC4  NAK  .    SUB
  */
    0x00,0x01,0x02,0x03,0x9c,0x09,0x86,0x7f,0x97,0x8d,0x8e,0x0b,0x0c,0x0d,0x0e,0x0f, /* 0x0X */
    0x10,0x11,0x12,0x13,0x9d,0x85,0x08,0x87,0x18,0x19,0x92,0x8f,0x1c,0x1d,0x1e,0x1f, /* 0x1X */
    0x80,0x81,0x82,0x83,0x84,0x0a,0x17,0x1b,0x88,0x89,0x8a,0x8b,0x8c,0x05,0x06,0x07, /* 0x2X */
    0x90,0x91,0x16,0x93,0x94,0x95,0x96,0x04,0x98,0x99,0x9a,0x9b,0x14,0x15,0x9e,0x1a, /* 0x3X */

 /*
  * SP   RSP  .    .    .    .    .    .    .    .    .    .    <    (    +    |
  * &    .    .    .    .    .    .    .    .    .    !    $    *    )    ;    .
  * -    /    .    .    .    .    .    .    .    .    |    ,    %    _    >    ?
  * .    .    .    .    .    .    .    .    .    `    :    #    @    '    =    "
  */
    0x20,0xa0,0xe2,0xe4,0xe0,0xe1,0xe3,0xe5,0xe7,0xf1,0xa2,0x2e,0x3c,0x28,0x2b,0x7c, /* 0x4X */
    0x26,0xe9,0xea,0xeb,0xe8,0xed,0xee,0xef,0xec,0xdf,0x21,0x24,0x2a,0x29,0x3b,0xac, /* 0x5X */
    0x2d,0x2f,0xc2,0xc4,0xc0,0xc1,0xc3,0xc5,0xc7,0xd1,0xa6,0x2c,0x25,0x5f,0x3e,0x3f, /* 0x6X */
    0xf8,0xc9,0xca,0xcb,0xc8,0xcd,0xce,0xcf,0xcc,0x60,0x3a,0x23,0x40,0x27,0x3d,0x22, /* 0x7X */

 /*
  * .    a    b    c    d    e    f    g    h    i    .    .    .    .    .    .
  * .    j    k    l    m    n    o    p    q    r    .    .    .    .    .    .
  * .    ~    s    t    u    v    w    x    y    z    .    .    .    .    .    .
  * ^    .    .    .    .    .    .    .    .    .    [    ]    .    .    .    .
  */
    0xd8,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0xab,0xbb,0xf0,0xfd,0xfe,0xb1, /* 0x9X */
    0xb0,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0xaa,0xba,0xe6,0xb8,0xc6,0xa4, /* 0x9X */
    0xb5,0x7e,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0xa1,0xbf,0xd0,0xdd,0xde,0xae, /* 0xaX */
    0x5e,0xa3,0xa5,0xb7,0xa9,0xa7,0xb6,0xbc,0xbd,0xbe,0x5b,0x5d,0xaf,0xa8,0xb4,0xd7, /* 0xbX */

 /*
  * {    A    B    C    D    E    F    G    H    I    .    .    .    .    .    .
  * }    J    K    L    M    N    O    P    Q    R    .    .    .    .    .    .
  * \    NSP  S    T    U    V    W    X    Y    Z    .    .    .    .    .    .
  * 0    1    2    3    4    5    6    7    8    9    .    .    .    .    .    E0
  */
    0x7b,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0xad,0xf4,0xf6,0xf2,0xf3,0xf5, /* 0xcX */
    0x7d,0x4a,0x4b,0x4c,0x4d,0x4e,0x4f,0x50,0x51,0x52,0xb9,0xfb,0xfc,0xf9,0xfa,0xff, /* 0xdX */
    0x5c,0xf7,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5a,0xb2,0xd4,0xd6,0xd2,0xd3,0xd5, /* 0xeX */
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0xb3,0xdb,0xdc,0xd9,0xda,0x9f  /* 0xfX */
    };
#endif

#if defined(HAVE_MAGIC_H) && defined(HAVE_LIBMAGIC)
static int              x_magicopen = 0;
static magic_t          x_magiclib = 0;
#endif


/*  Function:           mchar_guess_init
 *      Run-time initialisation.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
mchar_guess_init(void)
{
}


/*  Function:           mchar_guess_shutdown
 *      Run-time shutdown.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      none
 */
void
mchar_guess_shutdown(void)
{
#if defined(HAVE_MAGIC_H) && defined(HAVE_LIBMAGIC)
    if (x_magicopen) {
        magic_close(x_magiclib);
        x_magicopen = 0;
    }
#endif
#if defined(HAVE_LIBCHARUDET)
#endif
#if defined(HAVE_LIBENCA)
#endif
}


/*  Function:           mchar_guess
 *      Determine/guess the file-type/encoding.
 *
 *  Parameters:
 *      buffer -            Buffer address.
 *      len -               Length of the buffer.
 *      fileinfo -          Derived file information.
 *
 *  Returns:
 *      *true* if successful, otherwise *false*.
 */
int
mchar_guess(const char *specification, unsigned flags,
        const void *buffer, unsigned length, mcharguessinfo_t *fileinfo)
{
    const int quick = (0x01 & flags ? 1 : 0);   /* quick(er) detect */
    const unsigned limit = ((quick ? 16 : 32) * 1024);
    guessinfo_t guess = {0};

    if (length > limit) {
        length = limit;
    }

    trace_ilog("guess(decoders:%s,flags:%0x,buffer:%p,length:%u)\n", \
        (specification ? specification : ""), flags, buffer, length);

    guess.gi_endian = -1;
    guess_summaries(&guess, NULL, buffer, length);

    if (BFTYP_UNKNOWN == guess.gi_type || BFTYP_UNDEFINED == guess.gi_type) {
        if (specification) {
            /*
             *  encoders:   xxxx[=arg][, ...]
             *  languages:  xx[,..]             TODO/ENCA
             */
            if (NULL != (specification = chk_salloc(specification))) {
                char *cursor = (char *) specification;

                while (cursor && *cursor) {
                    const char *argument = NULL;
                    const struct guessdecoder *decoder =
                            decodernext(&cursor, &argument);

                    if (decoder)
                        if (decodercall(&guess, decoder, buffer, length, argument)) {
                            break;
                        }
                }
                chk_free((char *)specification);
            }
        } else {
            /*
             *  default
             */
            const struct guessdecoder *decoder = x_guessdecoders;
            unsigned idx;

            for (idx = 0; idx < X_GUESSDECODERS; ++idx, ++decoder) {
                if (GUESS_FDEFAULT & decoder->flags)
                    if (decodercall(&guess, decoder, buffer, length, NULL)) {
                        break;
                    }
            }
        }
    }

    guess_lineterm(&guess);

    trace_ilog("==>  type: <%d/%s>\n", \
        guess.gi_type, buf_type_desc(guess.gi_type, "n/a"));
    trace_ilog(" encoding: <%s>\n", guess.gi_encoding);
    if (guess.gi_endian >= 0) {
        trace_ilog("   endian: <%s>\n", guess.gi_endian ? "big" : "little");
    }
    trace_ilog(" termtype: <%d/%s>\n", \
        guess.gi_termtype, buf_termtype_desc(guess.gi_termtype, "n/a"));
    trace_ilog("     term: [");
        trace_data(guess.gi_termbuf, guess.gi_termlen, "]\n");

    if (fileinfo) {
        *fileinfo = guess.gi_fileinfo;
    }
    return (int)guess.gi_type;
}


/*  Function:           mchar_guess_default
 *      Retrieve the default detection rules.
 *
 *  Parameters:
 *      n/a
 *
 *  Returns:
 *      String containing the default detection rules.
 */
char *
mchar_guess_default(void)
{
    const struct guessdecoder *decoder;
    unsigned idx, len = 1;
    char *guess = NULL;

    for (idx = 0, decoder = x_guessdecoders; idx < X_GUESSDECODERS; ++idx, ++decoder) {
        if (GUESS_FDEFAULT & decoder->flags) {
            len += decoder->namelen + 1;        /* name plus delimiter */
        }
    }

    if (NULL != (guess = chk_alloc(len))) {
        char *cursor = guess;

        if (len > 1) {
            for (idx = 0, decoder = x_guessdecoders; idx < X_GUESSDECODERS; ++idx, ++decoder) {
                if (GUESS_FDEFAULT & decoder->flags) {
                    if (cursor > guess) *cursor++ = ',';
                    memcpy(cursor, decoder->name, decoder->namelen);
                    cursor += decoder->namelen;
                }
            }
        }
        *cursor = '\0';
    }

    return guess;
}


/*  Function:           mchar_guess_endian
 *      Determine/guess the file-type/encoding.
 *
 *  Parameters:
 *      encoding -          Character set identifier.
 *
 *  Returns:
 *      Endian value.
 */
int
mchar_guess_endian(const char *encoding)
{
    int endian = 1;                             /* big-endian by default, Unicode FAQ */

    if (encoding && *encoding) {
        const int len = (int)strlen(encoding);

        /*
         *  [utf-16|32][-<endian*>]
         *
         *          le/be
         *          Little/Big
         *          LittleEndian/BigEndian
         *          PlatformEndian
         *          OppositeEndian
         *
         *      (*) List of encountered names, some standard others not.
         */
        if ((len >= 2 && 0 == str_icmp(encoding + (len - 2), "le")) ||
                (len >= 6  && 0 == str_icmp(encoding + (len - 6), "Little")) ||
                (len >= 12 && 0 == str_icmp(encoding + (len - 12), "LittleEndian"))) {
            endian = 0;                         /* little-endian */

        } else if ((len >= 2 && 0 == str_icmp(encoding + (len - 2), "be")) ||
                    (len >= 3 && 0 == str_icmp(encoding + (len - 3), "Big")) ||
                    (len >= 9 && 0 == str_icmp(encoding + (len - 9), "BigEndian"))) {
            endian = 1;                         /* big-endian */

        } else if (len >= 14) {
            if (0 == str_icmp(encoding + (len - 14), "PlatformEndian")) {
#if defined(HOST_BIG_ENDIAN)
                endian = 1;
#else
                endian = 0;
#endif

            } else if (0 == str_icmp(encoding + (len - 14), "OppositeEndian")) {
#if defined(HOST_BIG_ENDIAN)
                endian = 0;
#else
                endian = 1;
#endif
            }
        }
    }
    return endian;
}


static int
decodercall(guessinfo_t *guess, const struct guessdecoder *decoder,
        const void *buffer, unsigned length, const char *argument)
{
    int ret;

    __CUNUSED(argument)

    ret = (*decoder->guess)(guess, buffer, length /*, argument*/);
    if (1 == ret) {
        trace_ilog("\t==> decoder(%s)%*s : 1", decoder->name, (int)(12 - decoder->namelen), "");
        if (BFTYP_UNDEFINED != guess->gi_type) {
            trace_log(", type:<%d/%s>",
                guess->gi_type, buf_type_desc(guess->gi_type, "n/a"));
        }
        trace_log(", encoding:<%s>", guess->gi_encoding);
        if (guess->gi_endian >= 0) {
            trace_log(", endian:<%s>", guess->gi_endian ? "big" : "little");
        }
        trace_log("\n");
        return 1;
    }
    trace_ilog("\t==> decoder(%s)%*s : %d\n", decoder->name, (int)(12 - decoder->namelen), "", ret);
    return 0;
}


static const struct guessdecoder *
decodernext(char **optionp, const char **valuep)
{
    register char *option = *optionp;
    const char *tag = NULL;

    *optionp = NULL;
    *valuep = NULL;

    if (*option) {                              /* leading white-space and delimiters */
        while (*option && strchr(", \t", *option)) {
            ++option;
        }
    }

    if (*option) {                              /* consume until white-space or delimiter */
        tag = option;
        while (*++option && !strchr("=, \t", *option)) {
            /*cont*/;
        }
    }

    if (*option) {
        if ('=' == *option) {                   /* value */
            *option++ = '\0';
            *valuep = option++;
            while (*option && !strchr(", \t", *option)) {
                ++option;
            }
            if (*option) {
                *option++ = '\0';               /* terminator value */
            }
        } else {
            *option++ = '\0';
        }

        while (*option && strchr(", \t", *option)) {
            ++option;                           /* consume trailing whitespace/delimiters */
        }
    }

    *optionp = option;                          /* cursor for next round */

    if (tag) {
        const struct guessdecoder *decoder = x_guessdecoders;
        unsigned idx;

        for (idx = 0; idx < X_GUESSDECODERS; ++idx, ++decoder) {
            if (0 == strcmp(tag, decoder->name)) {
                return decoder;
            }
        }
    }

    return NULL;
}


/*  Function:           guess_summaries
 *      Summaries the buffer content.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      map -               Character map.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if successful, otherwise *false*.
 */
static void
guess_summaries(
    guessinfo_t *guess, const unsigned char *map, const void *buffer, unsigned length)
{
    guessterm_t *linebreak = &guess->gi_linebreak;
    const unsigned char *cursor = buffer;
    const unsigned char *end = cursor + length;
    unsigned *chartally = guess->gi_chartally;
    unsigned charflags = 0;

    trace_ilog("summaries(mapped:%s)\n", (map ? "yes" : "no"));

    memset(chartally, 0, sizeof(guess->gi_chartally));
    linereset(linebreak);

    /* BOM */
    if (bomcheck(buffer, length, guess)) {
        cursor += guess->gi_bomlen;
    }

    /* character analysis */
    if (map) {
        while (cursor < end) {
            register const unsigned char ch = map[*cursor++];

            ++chartally[ch];
            charflags |= x_charflags[ch];
            linetally(linebreak, ch);
        }

    } else {
        while (cursor < end) {
            register const unsigned char ch = *cursor++;

            ++chartally[ch];
            charflags |= x_charflags[ch];
            linetally(linebreak, ch);
        }
    }
    guess->gi_charflags = charflags;

    /* dump results */
    trace_ilog("\t==> flags:0x%04x, lf:%u, crs:%u, crlf:%u, nel:%u\n",
        guess->gi_charflags, linebreak->lf, linebreak->cr, linebreak->crlf, linebreak->nel);

    trace_ilog("\t==> bin:%u, c0:%u, c1:%u, esc:%u, ascii:%u, latin1:%u\n",
        linebreak->nul, tallycount(chartally, C0), tallycount(chartally, C1),
            linebreak->esc, tallycount(chartally, A), tallycount(chartally, I));
}


static int
bomcheck(const char *buffer, int length, guessinfo_t *guess)
{
    const struct bomdesc *magic = x_bommagic;
    unsigned m;

    for (m = 0; m < X_BOMMAGIC; ++m, ++magic) {
        if (length >= (int)magic->len &&
                    0 == memcmp(buffer, magic->bom, magic->len)) {
            trace_ilog("\tBOM(%d)=%s\n", magic->type, magic->des);
            guess->gi_bomtype  = magic->type;
            guess->gi_bomsalt  = magic->salt;
            guess->gi_bomdes   = magic->des;
            guess->gi_bomlen   = magic->len;
            guess->gi_bommagic = magic;
            memcpy(guess->gi_bombuf, magic->bom, sizeof(magic->bom));
            return magic->len;
        }
    }
    guess->gi_bommagic = NULL;
    return 0;
}


static unsigned
tallycount(const unsigned *chartally, unsigned char mask)
{
    unsigned idx, count = 0;

    for (idx = 0; idx < 0xff; ++idx) {
        if ((mask & x_charflags[idx]) == mask) {
            count += chartally[idx];
        }
    }
    return count;
}


static int
linereset(guessterm_t *linebreak)
{
    (void) memset(linebreak, 0, sizeof(guessterm_t));
    return 0;
}


static int
linetally(guessterm_t *linebreak, int32_t ch)
{
    switch(ch) {
    case 0:             /* NUL */
        ++linebreak->nul;
        break;

    case ASCIIDEF_ESC:  /* ESC */
        ++linebreak->esc;
        break;

    case ASCIIDEF_CR:   /* CR */
        ++linebreak->cr;
        break;

    case ASCIIDEF_LF:   /* [CR] LF */
        if (ASCIIDEF_CR == linebreak->state1) {
            ++linebreak->crlf;
        } else {
            ++linebreak->lf;
        }
        break;

    case ASCIIDEF_NEL:  /* ANSI/ISO - NEL */
        ++linebreak->nel;
        break;

    case 0x2028:        /* Unicode - Line separator */
        ++linebreak->ucslf;
        break;

    case 0x2029:        /* Unicode - Paragraph separator */
        ++linebreak->ucsbr;
        break;
    }
    linebreak->state1 = ch;
    return 0;
}


static int
setencoding(guessinfo_t *guess, const char *encoding, int length)
{
    assert(encoding);
    if (length < 0) {
        length = (int)strlen(encoding);
    }
    if (length >= (int)sizeof(guess->gi_encoding)) {
        length = (sizeof(guess->gi_encoding) - 1);
    }
    memcpy(guess->gi_encoding, encoding, length);
    guess->gi_encoding[length] = 0;
    if (BFTYP_UTF16 == guess->gi_type || BFTYP_UTF32 == guess->gi_type) {
        if (-1 == guess->gi_endian) {
            guess->gi_endian = mchar_guess_endian(guess->gi_encoding);
        }
    } else {
        guess->gi_endian = -1;
    }
    return 0;
}


static void
guess_lineterm(guessinfo_t *guess)
{
    const guessterm_t *linebreak = &guess->gi_linebreak;
    BUFTYPE_t type = BFTYP_UNKNOWN, termtype = 0, termlen = 0;

    /*
     *  derive base-type and line-terminator
     */
    if (BFTYP_BINARY == guess->gi_type) {
        termlen = 0;
        termtype = LTERM_NONE;

    } else {
        char *termbuf = (char *)guess->gi_termbuf;

        if (linebreak->cr > (linebreak->crlf * 2 / 3)) {
            if (linebreak->cr > linebreak->nel) {
                type = BFTYP_DOS;               /* CR+LF */
                termtype = LTERM_DOS;
                termbuf[0] = ASCIIDEF_CR;
                termbuf[1] = ASCIIDEF_LF;
                termlen = 2;
            }
        } else if (linebreak->lf) {
            if (linebreak->lf > linebreak->nel) {
                type = BFTYP_UNIX;              /* LF */
                termtype = LTERM_UNIX;
                termbuf[0] = ASCIIDEF_LF;
                termlen = 1;
            }
        } else if (linebreak->cr) {
            if (linebreak->cr > linebreak->nel) {
                type = BFTYP_MAC;               /* CR */
                termtype = LTERM_MAC;
                termbuf[0] = ASCIIDEF_CR;
                termlen = 1;
            }
        }

        if (0 == termlen) {
            if (linebreak->nel) {               /* NEL */
                type = BFTYP_ANSI;
                termtype = LTERM_NEL;
                termlen = 1;
            } else if (linebreak->ucslf) {      /* UNICODE NEW LINE */
                type = BFTYP_UTF8;
                termtype = LTERM_UCSNL;
            }
        }
    }

    /*
     *  assign results
     */
    guess->gi_termtype = termtype;
    if (BFTYP_UNDEFINED == guess->gi_type) {
        guess->gi_type = type;                  /* assign base-type */
    }
    if (linebreak->crlf) {
        guess->gi_flags = MCHAR_FI_CRLF;
    }
    guess->gi_termtype  = termtype;
    guess->gi_termlen   = termlen;
}


/*  Function:           guess_charset
 *      Determine whether a user specified character-set.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if successful, otherwise *false*.
 */
static int
guess_charset(guessinfo_t *guess, const void *buffer, unsigned length)
{
    __CUNUSED(guess)
    __CUNUSED(buffer)
    __CUNUSED(length)
    return 0;
}


/*  Function:           guess_charudet
 *      Utilities the external libcharudet to determine the buffer encoding.
 *
 *      libcharudet is the Mozilla character detection library.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  References:
 *      http://www-archive.mozilla.org/projects/intl/UniversalCharsetDetection.html
 *
 *      http://mxr.mozilla.org/seamonkey/source/extensions/universalcharsetdet/src
 *
 *  Returns:
 *      *true* if successful, otherwise *false*.
 */
static int
guess_charudet(guessinfo_t *guess, const void *buffer, unsigned length)
{
#if defined(HAVE_LIBCHARUDET_H) && defined(HAVE_LIBCHARUDET)
    char encoding[64];

    if (chardet_analysis(buffer, length, encoding, sizeof(encoding))) {
        mcharcharsetinfo_t info;

        trace_ilog("\t ==> '%s'\n", encoding);
        if (mchar_info(&info, encoding, -1)) {
            if (info.cs_type > 0) {
                guess->gi_type = info.cs_type;
            }
            setencoding(guess, encoding, strlen(encoding));
            return 1;
        }
    }

#else
    __CUNUSED(guess)
    __CUNUSED(buffer)
    __CUNUSED(length)
#endif
    return 0;
}


/*  Function:           guess_guess
 *      Utilities the external libguess to determine the buffer encoding.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  References:
 *      http://www.atheme.org/project/libguess
 *
 *  Returns:
 *      *true* if successful, otherwise *false*.
 */
static int
guess_guess(guessinfo_t *guess, const void *buffer, unsigned length)
{
#if (0) && defined(HAVE_LIBGUESS)               /* TODO - work in progress */
    const char *encoding;

    //
    //  Languages:
    //      jp, tw, cn, kr, ru, ar, tr, gr, hw, pl, bl
    //
    if (NULL != (encoding = libguess_determine_encoding(buffer, length, NULL)) {
        mcharcharsetinfo_t info;

        trace_ilog("\t ==> '%s'\n", encoding);
        if (mchar_info(&info, encoding, -1)) {
            if (info.cs_type > 0) {
                guess->gi_type = info.cs_type;
            }
            setencoding(guess, encoding, strlen(encoding));
            return 1;
        }
    }

#else
    __CUNUSED(guess)
    __CUNUSED(buffer)
    __CUNUSED(length)
#endif
    return 0;
}


/*  Function:           guess_enca
 *      Utilities the external libenca to determine the buffer encoding.
 *
 *      libenca is the Extremely Naive Charset Analyser library.
 *
 *      ENCA detects the encoding of text files, on the basis of
 *      knowledge of their language. It can also convert them to
 *      other encodings, allowing you to recode files without knowing
 *      their current encoding. It supports most of Central and East
 *      European languages, and a few Unicode variants, independently
 *      on language.
 *
 *      Currently it supports Belarusian, Bulgarian, Croatian, Czech,
 *      Estonian, Hungarian, Latvian, Lithuanian, Polish, Russian,
 *      Slovak, Slovene, Ukrainian, Chinese, and some multibyte
 *      encodings independently on language.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Warning:
 *      libenca is licensed under the GPLv2; the user *must* explicitly
 *      build a local non-standard version and agree to *never* to
 *      release.
 *
 *  References:
 *      https://github.com/nijel/enca
 *
 *  Returns:
 *      *true* if successful, otherwise *false*.
 */
static int
guess_enca(guessinfo_t *guess, const void *buffer, unsigned length)
{
#if (0) && defined(HAVE_LIBENCA)                /* TODO - work in progress */
    static EncaAnalyser analysers[32] = {0};

    if (0 == analysers[0]) {
        //
        //  Languages:
        //      be, bg, cs, et, hr, hu, lt, lv, pl, ru, sk, sl, uk, zh, __)
        //
        const char **languages;
        size_t i, langcnt;
        unsigned cnt = 0;

        analysers[0] = (void *)-1;

        languages = enca_get_languages(&langcnt);
        trace_log("ENCA supported languages, (count:%d", langcnt);
        for (i = 0; i < langcnt; ++i) {
            trace_log(", %s", languages[i]);
        }
        trace_log(")\n");

        for (i = 0; i < langcnt && cnt < (sizeof(analysers)/sizeof(analysers[0])); ++i) {
            EncaAnalyser a = enca_analyser_alloc(languages[i]);

            if (a) {
                enca_set_multibyte(a, 1);       /* set for multibyte encodings */
            //  enca_set_interpreted_surfaces(a,1);
            //  enca_set_ambiguity(a, 1);       /* ambigous charsets, default=0 */
            //  enca_set_filtering(a, 1);       /* ignore binary/box characters, default=1 */
                enca_set_garbage_test(a, 1);    /* ignore white-space etc, default=1 */
                enca_set_termination_strictness(a, 0);  /* text is sample only, disable */
            //  enca_set_significant(a, 10);    /* significant characters, default=10 */
                enca_set_threshold(a, 1.38);    /* match ratio, default 1.4142 */
                trace_ilog("\tENCA[%d]<%s/%s> : open\n", cnt, languages[i], enca_analyser_language(a));
                analysers[cnt++] = a;
            }
        }
    }

    if ((void *)-1 != analysers[0]) {
        EncaAnalyser a;
        unsigned cnt = 0;

        for (cnt = 0; NULL != (a = analysers[cnt]); ++cnt) {
            const EncaEncoding e = enca_analyse_const(a, buffer, length);

            if (e.charset >= 0) {
                const char *charset =
                        enca_charset_name(e.charset, ENCA_NAME_STYLE_MIME);
                if (charset) {
                    const EncaSurface surface = e.surface;
                    const char *term = "";

                    if (ENCA_SURFACE_MASK_EOL & surface) {
                        if        (ENCA_SURFACE_EOL_CR   & surface) {
                            term = "CR";
                        } else if (ENCA_SURFACE_EOL_LF   & surface) {
                            term = "LF";
                        } else if (ENCA_SURFACE_EOL_CRLF & surface) {
                            term = "CRLF";
                        } else if (ENCA_SURFACE_EOL_MIX  & surface) {
                            term = "MIX";
                        } else if (ENCA_SURFACE_EOL_BIN  & surface) {
                            term = "BIN";
                        }
                    }
                    trace_ilog("\tENCA(%s) ==> <%s/%s>\n", enca_analyser_language(a), charset, term);
                    if (0 == strcmp(charset, enca_analyser_language(a))) {
                        break;
                    }
                }
            }
            trace_ilog("\tENCA(%s) : 0\n", enca_analyser_language(a));
        }
    }

//  enca_analyser_free(a);

#else
    __CUNUSED(guess)
    __CUNUSED(buffer)
    __CUNUSED(length)
#endif
    return 0;
}


/*  Function:           guess_marker
 *      Determine whether the buffer contains an embedded encoder specification.
 *
 *  Supported Formats:
 *      cr/vim -            encoding:<xxx>
 *      emacs -             coding:<xxx>
 *      html -              charset=["']<xxx>["']
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Notes:
 *      Maybe overwritten during modeline processing.
 *
 *  Returns:
 *      *true* if successful, otherwise *false*.
 */
static int
guess_marker(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const unsigned char *cursor = buffer;
    const unsigned char *end = cursor + length;
    const char *marker = NULL;
    int markerlen = 0, markers = 0;

    if (markerchars(guess)) {
        /*
         *  TODO - replace with a Boyer-Moore style search.
         */
        while (cursor < end) {
            const unsigned char *t_marker = NULL;
            int t_markerlen = 0;

            if (0 == *cursor ||
                    NULL == (t_marker = markercheck(cursor, end, &t_markerlen))) {
                ++cursor;
            } else {
                if (0 == markers++) {
                    marker = (const char *)t_marker;
                    markerlen = t_markerlen;
                }
                cursor = t_marker + t_markerlen + 1;
            }
        }

        if (1 == markers) {                      /* lookup encoding */
            mcharcharsetinfo_t info;

            trace_ilog("\t==> <%.*s>\n", markerlen, marker);
            if (mchar_info(&info, (const char *)marker, markerlen)) {
                if (info.cs_type > 0) {
                    guess->gi_type = (BUFTYPE_t)info.cs_type;
                }
                setencoding(guess, marker, markerlen);
                return 1;
            }
        } else if (markers > 1) {
            trace_ilog("\t==> duplicate markers encountered\n");
        }
    }
    return 0;
}


static int
markerchars(const guessinfo_t *guess)
{
    /*
     *  determine whether all the required characters are present.
     */
    const char *markers[] = {                   /* supported markers */
        "\001charset=",                         /* charset= */
        "\002CcOoDdIiNnGg::",                   /* [en]coding: */
        NULL
        };
    const unsigned *chartally = guess->gi_chartally;
    const unsigned char *m;
    unsigned i;

    for (i = 0; NULL != (m = (const unsigned char *)markers[i]); ++i) {
        const int type = *m++;                  /* 1=ABS, 2=Mixed case */

        /* look for all required characters */
        if (1 == type) {
            while (*m) {                        /* .. case sensitive */
                if (0 == chartally[*m++]) {
                    m = NULL;
                    break;
                }
            }
        } else /* 2 == type */ {
            while (*m) {                        /* .. mixed case */
                if (0 == chartally[*m++] && 0 == chartally[*m++]) {
                    m = NULL;
                    break;
                }
            }
        }

        if (m) {
            return 1;                           /* matched */
        }
    }
    return 0;
}


static const void *
markercheck(const void *text, const void *end, int *encodinglen)
{
    /*
     *  chedck and decode at embedded marker
     */
    const unsigned char *buffer = text;
    const int lch = tolower(*buffer);
    int leading = 0;
    unsigned char ch;

    if ('c' == lch && (buffer + 8 < (const unsigned char *)end)) {
        if (0 == memcmp(buffer + 1, "harset=", 7)) {
            leading = 7+1;                      /* a) charset */
        } else if (0 == str_nicmp((const char *)(buffer + 1), "oding:", 6)) {
            leading = 6+1;                      /* b) [en]coding */
        }
    }

    if (leading > 0) {
        buffer += leading;
                                                /* consume leading whitespace */
        while (buffer < (const unsigned char *)end) {
            if (' ' == (ch = *buffer) || '\t' == ch) {
                ++buffer;
                continue;
            }
            break;          /*non white-space*/
        }

        if (buffer < (const unsigned char *)end)  {
            const unsigned char *cursor = buffer;

            while (cursor < (const unsigned char *)end) {
                if (0 != (ch = *cursor)) {      /* pull name */
                    if (isalnum(ch) || strchr(".-_@:\"'", ch)) {
                        ++cursor;
                        continue;
                    }
                }
                break;      /*illegal character*/
            }

            if (cursor > buffer) {
                if (('"' == *buffer || '\'' == *buffer) &&
                        *buffer == cursor[-1]) {
                    ++buffer;                   /* remove quotes */
                    --cursor;
                }
                if (cursor > buffer) {          /* match */
                    *encodinglen = cursor - buffer;
                    return buffer;
                }
            }
        }
    }
    return NULL;
}


/*  Function:           guess_magic
 *      Utilities the external libmagic to determine the buffer encoding.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if successful, otherwise *false*.
 */
static int
guess_magic(guessinfo_t *guess, const void *buffer, unsigned length)
{
#if defined(HAVE_MAGIC_H) && defined(HAVE_LIBMAGIC)
    const char *cookie;

    if (! x_magicopen) {
        /*
         *  The default database file is named by the MAGIC environment variable.
         *  If that variable is not set, the default database file name is
         *  /usr/share/misc/magic.
         *
         *  magic_load() adds ``.mgc'' to the database file-name as appropriate.
         */
        int ret;

        trace_ilog("\t==> magic_init()\n");
        x_magiclib = magic_open(MAGIC_MIME);
        trace_ilog("\t==> magic_open() : 0x%p\n", x_magiclib);
        ret = magic_load(x_magiclib, NULL);
        trace_ilog("\t==> magic_load() : %d\n", ret);
        ++x_magicopen;
    }

    trace_ilog("\t==> magic_buffer(%.*s...)\n", (length > 80 ? 80 : length), (const char *)buffer);

    if (x_magicopen)
        if (NULL != (cookie = magic_buffer(x_magiclib, buffer, length))) {
            const char *marker, *end = cookie + strlen(cookie);
            int len = 0;

            trace_ilog("\t==> cookie(%s)\n", cookie);
            while (cookie < end) {
                if (NULL != (marker = markercheck(cookie, end, &len))) {
                    guess->gi_type = BFTYP_UNDEFINED;
                    setencoding(guess, marker, len);
                    return 1;
                }
                ++cookie;
            }
        }

#else
    __CUNUSED(guess)
    __CUNUSED(buffer)
    __CUNUSED(length)
#endif
    return 0;
}


/*  Function:           guess_utf32bom
 *      Determine whether the buffer contains BOM-prefixed UTF-32/USC4 encoded characters.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if UTF32/USC4 encoded, otherwise *false*.
 */
static int
guess_utf32bom(guessinfo_t *guess, const void *buffer, unsigned length)
{
    int ret = 0;

    if (BFTYP_UTF32 == guess->gi_bomtype && guess->gi_bomlen > 0) {
        const unsigned bomlen = guess->gi_bomlen;

        if (length >= bomlen) {                 /* BOM encoded? */
            switch (guess->gi_bomsalt) {
            case SALT_BE:
                ret = guess_utf32(guess, (const char *)buffer + bomlen, length - bomlen, 1);
                break;
            case SALT_LE:
                ret = guess_utf32(guess, (const char *)buffer + bomlen, length - bomlen, 0);
                break;
            default:
                break;
            }

            if (0 == ret) {
                guess->gi_bomsalt |= SALT_BAD;  /* on error, ignore BOM */
            }
        }
    }
    return ret;
}


/*  Function:           guess_utf32xx
 *      Determine whether the buffer contains NON-BOM prefixed UTF-32/USC4 encoded characters.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if UTF16/USC2 encoded, otherwise *false*.
 */
static int
guess_utf32xx(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const unsigned *chartally = guess->gi_chartally;

    if (BFTYP_UNKNOWN != guess->gi_bomtype) {
        return guess_utf32bom(guess, buffer, length);
    }

    if (length >= 512 && 0 == (length & 3)) {
        /*
        *   scan for 0x20000000 and 0x00000020,
        *      if multiple of *only* one type, force endian and decode.
        *
        *   Method only suitable against a sized buffer.
        */
        const unsigned char *cursor = buffer,
                *end = cursor + length;
        unsigned big = 0, little = 0;

        if (chartally[0x00] > (length/3) &&
                chartally[0x20] < chartally[0x00]) {
            while ((cursor + 3) < (unsigned char *)end) {
                const uint32_t ch = (((uint32_t)cursor[0] << 24) |
                                     ((uint32_t)cursor[1] << 16) |
                                     ((uint32_t)cursor[2] << 8 ) |
                                      (uint32_t)cursor[3]);

                if (0x00000020 == ch) {
                    ++big;
                } else if (0x20000000 == ch) {
                    ++little;
                }
                cursor += 4;
            }

            if (big > 2 && 0 == little) {
                return guess_utf32(guess, buffer, length, 1);
            } else if (little > 2 && 0 == big) {
                return guess_utf32(guess, buffer, length, 0);
            }
        }
    }
    return 0;
}


/*  Function:           guess_utf32
 *      Determine whether the buffer contains UTF-32/USC4 encoded characters.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *      endian -            Data endian order (0=small, 1=big).
 *
 *  Returns:
 *      *true* if UTF32/USC4 encoded, otherwise *false*.
 */
static int
guess_utf32(guessinfo_t *guess, const void *buffer, unsigned length, int endian)
{
    guessterm_t linebreak = {0};
    const char *cursor = buffer;
    int isutf32 = 1;

    assert(0 == endian || 1 == endian);

    if (length < 4) {
        isutf32 = -1;                           /* short buffer */

    } else {
        const char *end;
        int32_t raw, ch;

        length &= ~3;                           /* round */
        end = cursor + length;

        while (cursor < end && 1 == isutf32) {

            if (NULL == (cursor = charset_utf32_decode(endian, cursor, end, &ch, &raw)) || ch != raw) {
                trace_ilog("\t==> bad UTF32\n");
                isutf32 = -1;

            } else if (ch <= 0xff) {
                if (0 == (x_charflags[ch] & (A|I|X))) {
                    trace_ilog("\t==> non ASCII (%d/0x%02x)\n", ch, ch);
                    isutf32 = -1;               /* neither ASCII nor Latin1 */

                } else {
                    linetally(&linebreak, ch);
                }
            }
        }
    }

    if (1 == isutf32) {
        guess->gi_type = BFTYP_UTF32;
        guess->gi_endian = endian;
        setencoding(guess, (endian ? MCHAR_UTF32BE : MCHAR_UTF32LE), 8);
        guess->gi_linebreak = linebreak;
    }
    return isutf32;
}


/*  Function:           guess_utf16bom
 *      Determine whether the buffer contains BOM-prefixed UTF-16/USC2 encoded characters.
 *
 *  Parameters:
 *      guess -             Buffer populated with derived buffer type on success.
 *      buffer -            Address of the data buffer.
 *      length -            Length of the data, in bytes.
 *
 *  Returns:
 *      *true* if UTF16/USC2 encoded, otherwise *false*.
 */
static int
guess_utf16bom(guessinfo_t *guess, const void *buffer, unsigned length)
{
    int ret = 0;

    if (BFTYP_UTF16 == guess->gi_bomtype && guess->gi_bomlen > 0) {
        const unsigned bomlen = guess->gi_bomlen;

        if (length >= bomlen) {                 /* BOM encoded? */
            switch (guess->gi_bomsalt) {
            case SALT_BE:
                ret = guess_utf16(guess, (const char *)buffer + bomlen, length - bomlen, 1);
                break;
            case SALT_LE:
                ret = guess_utf16(guess, (const char *)buffer + bomlen, length - bomlen, 0);
                break;
            default:
                break;
            }

            if (0 == ret) {
                guess->gi_bomsalt |= SALT_BAD;  /* on error, ignore BOM */
            }
        }
    }
    return 0;
}


/*  Function:           guess_utf16xx
 *      Determine whether the buffer contains NON-BOM prefixed UTF-16/USC2 encoded characters.
 *
 *  Parameters:
 *      guess -             Buffer populated with derived buffer type on success.
 *      buffer -            Address of the data buffer.
 *      length -            Length of the data, in bytes.
 *
 *  Returns:
 *      *true* if UTF16/USC2 encoded, otherwise *false*.
 */
static int
guess_utf16xx(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const unsigned *chartally = guess->gi_chartally;

    if (BFTYP_UNKNOWN != guess->gi_bomtype) {
        return guess_utf16bom(guess, buffer, length);
    }

    if (length >= 256 && 0 == (length & 1)) {
        /*
         *  Scan for 0x2000 and 0x0020 (encoded spaces ' '),
         *      if multiple of *only* one type, force endian and attempt a decode.
         *
         *  Method only suitable against a sized buffer.
         */
        const unsigned char *cursor = buffer,
                *end = cursor + length;
        unsigned big = 0, little = 0;

        if (chartally[0x00] > (length/5) &&
                chartally[0x20] < chartally[0x00]) {
            while ((cursor + 1) < (unsigned char *)end) {
                const uint32_t ch = (((uint16_t)cursor[0]) << 8) |
                                     ((uint16_t)cursor[1]);

                if (0x0020 == ch) {
                    ++big;
                } else if (0x2000 == ch) {
                    ++little;
                }
                cursor += 2;
            }

            if (big > 2 && 0 == little) {
                return guess_utf16(guess, buffer, length, 1);
            } else if (little > 2 && 0 == big) {
                return guess_utf16(guess, buffer, length, 0);
            }
        }
    }
    return 0;
}


/*  Function:           guess_utf16
 *      Determine whether the buffer contains UTF-16/USC2 encoded characters.
 *
 *  Parameters:
 *      guess -             Buffer populated with derived buffer type on success.
 *      buffer -            Address of the data buffer.
 *      length -            Length of the data, in bytes.
 *      endian -            Data endian order (0=small, 1=big).
 *
 *  Returns:
 *      *true* if UTF16/USC2 encoded, otherwise *false*.
 */
static int
guess_utf16(guessinfo_t *guess, const void *buffer, unsigned length, int endian)
{
    guessterm_t linebreak = {0};
    const char *cursor = buffer;
    int isutf16 = 1;

    assert(0 == endian || 1 == endian);

    if (length < 2) {
        isutf16 = -1;                           /* short buffer */

    } else {
        const char *end;
        int32_t raw, ch;

        length &= ~1;                           /* round */
        end = cursor + length;

        while (cursor < end && 1 == isutf16) {

            if (NULL == (cursor = charset_utf16_decode(endian, cursor, end, &ch, &raw)) || ch != raw) {
                trace_ilog("\t==> bad UTF-16 (ch=%d)\n", ch);
                isutf16 = -1;

            } else if (ch <= 0xff) {
                if (0 == (x_charflags[ch] & (A|I))) {
                    trace_ilog("\t==> non ASCII (%d/0x%02x)\n", ch, ch);
                    isutf16 = -1;               /* neither ASCII nor Latin1 */

                } else {
                    linetally(&linebreak, ch);
                }
            }
        }
    }

    if (1 == isutf16) {
        guess->gi_type = BFTYP_UTF16;
        guess->gi_endian = endian;
        setencoding(guess, (endian ? MCHAR_UTF16BE : MCHAR_UTF16LE), 8);
        guess->gi_linebreak = linebreak;
    }
    return isutf16;
}


/*  Function:           guess_utf8
 *      Determine whether the buffer contains UTF-8 encoded characters.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if UTF-8 encoded, otherwise *false*.
 */
static int
guess_utf8(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const unsigned bomtype = guess->gi_bomtype;
    guessterm_t linebreak = {0};
    register const unsigned char *cursor = buffer;
    const unsigned char *end = cursor + (length - 1);
    unsigned nonascii = 0, noutf8 = 0;
    int isutf8 = 0;
    int32_t ch = 0;

    if (BFTYP_UNKNOWN == bomtype || BFTYP_UTF8 == bomtype) {

        if (BFTYP_UTF8 == bomtype) {            /* remove BOM */
            cursor += guess->gi_bomlen;
            isutf8 = 1;
        }

        while (cursor < end && isutf8 >= 0) {
            if ((ch = *cursor++) >= 0x80) {
                /*
                 *  utf8, allowing illegal and overlong only rejecting illformed.
                 */
                cursor = charset_utf8_decode(cursor - 1, end, &ch);
                if (ch <= 0) {
                    trace_ilog("\t\t==> bad UTF8\n");
                    if (++noutf8 > (length/1)) { /* non utf8 >1% */
                        isutf8 = -1;
                        break;
                    }
                    continue;
                }
                isutf8 = 1;
            }

            if (ch <= 0xff) {
                /*
                 *  [CR/LF], [BS], [FF], [0x20 - 0x7F], [0x85], [0xA0 - 0xFF]
                 */
                if (0 == (x_charflags[ch] & (A|X|I))) {
                    trace_ilog("\t\t==> non ASCII (%d/0x%02x)\n", ch, ch);
                    if (++nonascii > 2) {       /* neither ASCII nor Latin1 */
                        isutf8 = -1;
                    }
                 } else {
                    linetally(&linebreak, ch);
                }
            }
        }
    }

    if (1 == isutf8) {
        guess->gi_type = BFTYP_UTF8;
        setencoding(guess, "utf-8", 5);
        guess->gi_linebreak = linebreak;
    }
    return isutf8;
}


/*  Function:           guess_bom
 *      Check for a Unicode Byte-Order-Mark (BOM) marker.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if successful, otherwise *false*.
 */
static int
guess_bom(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const BUFTYPE_t bomtype = guess->gi_bomtype;

    __CUNUSED(buffer)
    __CUNUSED(length)

    if (BFTYP_UNKNOWN == bomtype) {
        return 0;
    }

    if (guess->gi_bomlen > 0 && 0 == (guess->gi_bomsalt & SALT_BAD)) {
        const struct bomdesc *magic = guess->gi_bommagic;

        guess->gi_type = bomtype;
        guess->gi_encoding[0] = 0;

        if (NULL != (magic = guess->gi_bommagic)) {
            guess->gi_endian = (SALT_BE & magic->salt ? 1 : 0);
            setencoding(guess, magic->name, magic->namelen);
        } else {
            setencoding(guess, "unsupported", -1);
        }
        return 1;
    }
    return 0;
}


/*  Function:           guess_binary
 *      Determine whether the buffer contains binary data.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if BINARY, otherwise *false*.
 */
static int
guess_binary(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const guessterm_t *linebreak = &guess->gi_linebreak;

    __CUNUSED(buffer)
    if (linebreak->nul > ((NULPERCENTAGE * length)/100)) {
        guess->gi_type = BFTYP_BINARY;
        strcpy(guess->gi_encoding, "binary");
        return 1;
    }
    return 0;
}


/*  Function:           guess_big5
 *      Determine whether the buffer contains Big5 data.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if Big5, otherwise *false*.
 */
static int
guess_big5(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const unsigned char *cursor = (const unsigned char *)buffer,
            *end = cursor + length;
    int isbig5 = 0;

    while (cursor < end && isbig5 >= 0) {
        const unsigned char c1 = *cursor++;

        /*
         *  Big5 does not conform to the ISO-2022 standard, but rather bears a certain
         *  similarity to Shift-JIS.
         *
         *  It is a double-byte character set with the following structure.
         *
         *      First Byte:         0xA1 - 0xf9 (non-user-defined characters)
         *                      or  0x81 - 0xfe (extended range)
         *      Second Byte:        0x40 - 0x7e or 0xa1 - 0xfe
         */
        if (0x80 == c1 || 0x00 == c1) {
            trace_ilog("\t\t==> bad Big5 c1 (%x)\n", c1);
            isbig5 = -1;                        /* bad */

        } else if (c1 >= 0xa1 && c1 <= 0xf9) {
            if (cursor < end) {                 /* Generic */
                const unsigned char c2 = *cursor++;

                if ((c2 >= 0x40 && c2 <= 0x7e) || (c2 >= 0xa1 && c2 <= 0xfe)) {
                    isbig5 = 1;
                    continue;
                }
                trace_ilog("\t\t==> bad Big5 c2 (%x)\n", c2);
                isbig5 = -1;
            }

        } if (c1 >= 0x81 && c1 <= 0xfe) {
            if (cursor < end) {                 /* Extended, assume Big5-HKSCS */
                const unsigned char c2 = *cursor++;

                if ((c2 >= 0x40 && c2 <= 0x7e) || (c2 >= 0xa1 && c2 <= 0xfe)) {
                    isbig5 = 2;
                    continue;
                }
                trace_ilog("\t\t==> bad Big5 c2 (%x)\n", c2);
                isbig5 = -1;
            }
        }
    }

    if (isbig5 >= 1) {
        setencoding(guess, (2 == isbig5 ? "Big5-HKSCS" : "Big5"), -1);
        return 1;
    }
    return 0;
}


/*  Function:           guess_gb18030
 *      Determine whether the buffer contains GB18030 data.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if GB18030, otherwise *false*.
 */
static int
guess_gb18030(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const unsigned char *cursor = (const unsigned char *)buffer,
            *end = cursor + length;
    int isgb = 0;

    while (cursor < end && isgb >= 0) {
        const unsigned char c1 = *cursor++;

        /*
         *  GB18030-2000 has the following significant properties:
         *
         *          * It incorporates Unicode's CJK Unified Ideographs Extension A completely.
         *
         *          * It provides code space for all used and unused code points of Unicode's plane 0
         *          (BMP) and its 15 additional planes. While being a code- and character-compatible
         *          "superset" of GBK, GB18030-2000, at the same time, intends to provide space for all
         *          remaining code points of Unicode. Thus, it effectively creates a one-to-one
         *          relationship between parts of GB18030-2000 and Unicode's complete encoding space.
         *
         *          * In order to accomplish the Unihan incorporation and code space allocation for
         *          Unicode 3.0, GB18030-2000 defines and applies a four-byte encoding mechanism.
         *
         *      GB18030-2000 encodes characters in sequences of one, two, or four bytes. The following
         *      are valid byte sequences (byte values are hexadecimal):
         *
         *          * Single-byte:  0x00-0x7f
         *          * Two-byte:     0x81-0xfe + 0x40-0x7e, 0x80-0xfe
         *          * Four-byte:    0x81-0xfe + 0x30-0x39 + 0x81-0xfe + 0x30-0x39
         *
         *      The single-byte portion applies the coding structure and principles of the standard GB
         *      11383 (identical to ISO 4873:1986) by using the code points 0x00 through 0x7f.
         *
         *      The two-byte portion uses two eight-bit binary sequences to express a character. The code
         *      points of the first (leading) byte range from 0x81 through 0xfe. The code points of the
         *      second (trailing) byte ranges from 0x40 through 0x7e and 0x80 through 0xfe.
         *
         *      The four-byte portion uses the code points 0x30 through 0x39, which are vacant in GB
         *      11383, as an additional means to extend the two-byte encodings, thus effectively
         *      increasing the number of four-byte codes to now include code points ranging from
         *      0x81308130 through 0xfe39fe39.
         *
         *      GB18030-2000 has 1.6 million valid byte sequences, but there are only 1.1 million code
         *      points in Unicode, so there are about 500, 000 byte sequences in GB18030-2000 that are
         *      currently unassigned.
         */
        if (c1 >= 0x81 && c1 <= 0xfe) {
            if (cursor < end) {
                const unsigned char c2 = *cursor++;

                if (c2 >= 0x30 && c2 <= 0x39) {
                    if (cursor+1 < end) {
                        const unsigned char c3 = *cursor++;
                        const unsigned char c4 = *cursor++;

                        if (c3 >= 0x81 && c3 <= 0xfe && c4 >= 0x30 && c4 <= 0x39) {
                            isgb = 1;
                        } else {
                            isgb = -1;
                        }
                    }

                } else if ((c2 >= 0x40 && c2 <= 0x7e) || (c2 >= 0x80 && c2 <= 0xfe)) {
                    isgb = 1;

                } else {
                    isgb = -1;
                }
            }
        }
    }

    if (isgb >= 1) {
        setencoding(guess, "gb18030", -1);
        return 1;
    }
    return 0;
}


/*  Function:           guess_shiftjis
 *      Determine whether the buffer contains Shift-JIS data.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if Shift-JIS, otherwise *false*.
 */
static int
guess_shiftjis(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const unsigned char *cursor = (const unsigned char *)buffer,
            *end = cursor + length;
    int isshiftjis = 0;

    while (cursor < end && isshiftjis >= 0) {
        const unsigned char c1 = *cursor++;

        /*
         *  Single Byte
         *      ASCII:                  0x21 - 0x7F    (also allow control)
         *      Katakana:               0xA1 - 0xDF
         *
         *  JIS X 0208 character
         *      First byte:             0x81 - 0x9F or 0xE0 - 0xEF
         *      Second byte (old 1st):  0x40 - 0x9E
         *      Second byte (even 1st): 0xA0 - 0xFD
         */
        if (c1 >= 0x81) {
            if ((/*c1 >= 0x81 &&*/ c1 <= 0x9F) || (c1 >= 0xe0 && c1 <= 0xef)) {
                if (cursor < end) {
                    const unsigned char c2 = *cursor++;

                    if (1 & c1) {
                        if (c2 >= 0x40 && c2 <= 0x9E) {
                            isshiftjis = 1;
                        } else {
                            isshiftjis = -1;
                        }
                    } else {
                        if (c2 >= 0xA0 && c2 <= 0xfd) {
                            isshiftjis = 1;
                        } else {
                            isshiftjis = -1;
                        }
                    }
                }
            } else if (c1 < 0xa1 || c1 > 0xdf) {
                isshiftjis = -1;
            }
        }
    }

    if (isshiftjis >= 1) {
        setencoding(guess, "Shift_JIS", -1);
        return 1;
    }
    return 0;
}


/*  Function:           guess_ascii
 *      Determine whether the buffer only contains ASCII data.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if ASCII, otherwise *false*.
 */
static int
guess_ascii(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const unsigned charflags = guess->gi_charflags;

    __CUNUSED(buffer)
    __CUNUSED(length)

    if (charflags & A) {
        if (0 == (charflags & ~(A))) {
            guess->gi_type = BFTYP_UNDEFINED;
            setencoding(guess, "us-ascii", 8);
            return 1;
        }
    }
    return 0;
}


/*  Function:           guess_latin1
 *      Determine whether the buffer only contains Latin-1 (ISO-8859-x) data.
 *
 *  Parameters:
 *      guess -             Buffer populated with derived buffer type on success.
 *      buffer -            Address of the data buffer.
 *      length -            Length of the data, in bytes.
 *
 *  Returns:
 *      *true* if Latin1, otherwise *false*.
 */
static int
guess_latin1(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const unsigned charflags = guess->gi_charflags;

    __CUNUSED(buffer)
    __CUNUSED(length)

    if (charflags & I) {
        if (0 == (charflags & ~(A|I))) {
            guess->gi_type = BFTYP_UNDEFINED;
            setencoding(guess, "latin1", 6);
            return 1;
        }
    }
    return 0;
}


/*  Function:           guess_xascii
 *      Determine whether the buffer only contains extended ASCII.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if extend-ASCII, otherwise *false*.
 */
static int
guess_xascii(guessinfo_t *guess, const void *buffer, unsigned length)
{
    const unsigned charflags = guess->gi_charflags;
    const unsigned *chartally = guess->gi_chartally;

    __CUNUSED(buffer)
    __CUNUSED(length)

    if (charflags & (X|C1|I)) {
        if (0 == chartally[0] && 0 == (charflags & ~(A|X|C1|I))) {
            guess->gi_type = BFTYP_OTHER;
            setencoding(guess, "cp437", 5);
            return 1;
        }
    }
    return 0;
}


/*  Function:           guess_ebcdic
 *      Determine whether the buffer contains EBCDIC.
 *
 *  Parameters:
 *      guess -             Derived file information populated on success.
 *      buffer -            Buffer address.
 *      length -            Length of the buffer.
 *
 *  Returns:
 *      *true* if EBCDIC, otherwise *false*.
 */
static int
guess_ebcdic(guessinfo_t *guess, const void *buffer, unsigned length)
{
#if (XXX_MCHAR_EBCDIC)
    guessinfo_t t_guess = {0};
    const struct guessdecoder *decoder = x_guessdecoders;
    unsigned idx;

    guess_ebcdic(&t_guess, buffer, length);

    for (idx = 0; idx < X_GUESSDECODERS; ++idx, ++decoder) {
        if (GUESS_FEBCDIC & decoder->flags)
            if (decodercall(&t_guess, decoder, buffer, length, NULL)) {
                *guess = t_guess;
                guess->type = BFTYP_EBCDIC;
                setencoding(guess, "ebcdic", 6);
                return 1;
            }
    }

#else
    __CUNUSED(guess)
    __CUNUSED(buffer)
    __CUNUSED(length)
#endif
    return 0;
}

/*end*/
