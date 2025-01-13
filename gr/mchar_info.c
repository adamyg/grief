#include <edidentifier.h>
__CIDENT_RCSID(gr_mchar_info_c,"$Id: mchar_info.c,v 1.25 2025/01/13 15:12:17 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: mchar_info.c,v 1.25 2025/01/13 15:12:17 cvsuser Exp $
 * Locale/multibyte character information.
 *
 *
 * Copyright (c) 1998 - 2025, Adam Young.
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
#undef HAVE_LIBICU
#if defined(HAVE_LIBICU)
#include <unicode/uclean.h>                     /* u_init() */
#include <unicode/ucnv.h>
#endif
#include <edenv.h>                              /* gputenvv(), ggetenv() */
#include <libstr.h>                             /* str_...()/sxprintf() */
#include <asciidefs.h>                          /* ASCII_... */
#include "../libchartable/libchartable.h"

#include "mchar.h"                              /* public interface */
#include "main.h"                               /* panic() */
#include "debug.h"                              /* trace_...() */

struct charset {
    const char *            cs_name;
    unsigned                cs_namelen;
    int                     cs_type;
#define BFTYP_UCS4              BFTYP_UTF32
#define BFTYP_UCS2              BFTYP_UTF16

    uint32_t                cs_flags;
#define BFFLG_7BIT              0x0001
#define BFFLG_BE                0x0002
#define BFFLG_LE                0x0003
#define BFFLG_EUC               0x0006
#define BFFLG_EUC_JP            (BFFLG_EUC  |0x0100)
#define BFFLG_EUC_KR            (BFFLG_EUC  |0x0200)
#define BFFLG_EUC_TW            (BFFLG_EUC  |0x0300)
#define BFFLG_EUC_GB            (BFFLG_EUC  |0x0400)
#define BFFLG_SHIFT_JIS         0x0007
#define BFFLG_BIG5              0x0009
#define BFFLG_BIG5_HKSCS        (BFFLG_BIG5 |0x0100)
#define BFFLG_ISO2022           0x000a
#define BFFLG_ISO2022CN         (BFFLG_ISO2022|0x0100)
#define BFFLG_ISO2022KK         (BFFLG_ISO2022|0x0200)
#define BFFLG_ISO2022KP         (BFFLG_ISO2022|0x0300)
#define BFFLG_GB                0x000b
#define BFFLG_GBK               (BFFLG_GB   |0x0100)
#define BFFLG_GB2312            (BFFLG_GB   |0x0200)
#define BFFLG_GB18030           (BFFLG_GB   |0x0300)

    int                     cs_codepage;
    const char *            cs_desc;
};


static void                 ICU_initialise(void);
static const char *         ICU_canonicalName(const char *name, int namelen);
static int                  ICU_charsetinfo(const char *name, int namelen, mcharcharsetinfo_t *info);
static const mcharcharsetinfo_t *charsetinfo(const struct charset *map, mcharcharsetinfo_t *info);
static const struct charset * charset_map(const char *name, int namelen);


/*
 *  Character encoding --- common character sets.
 *
 *      References:
 *          UNICODE/ICU/IANA/GLIB
 *          http://en.wikipedia.org/wiki/Character_encoding
 *          http://en.wikipedia.org/wiki/ISO_8859
 */
#define X_CHARACTERSETS     (sizeof(x_charactersets)/sizeof(x_charactersets[0]))

static const struct charset x_charactersets[] = {
#undef  _C
#undef  _X
#define _C(_n,_t,_f,_c,_d)  { _n, sizeof(_n)-1, _t, _f, _c, _d }
#define _X(_n,_t,_f,_d)     _C(_n, _t, _f, 0, _d)

    _C("US-ASCII",          BFTYP_SBCS,     BFFLG_7BIT,         646,    "ANSI/ASCII"),

    _C("ISO-8859-1",        BFTYP_SBCS,     0,                  28591,  "Western Europe"),
    _C("ISO-8859-2",        BFTYP_SBCS,     0,                  28592,  "Western and Central Europe"),
    _C("ISO-8859-3",        BFTYP_SBCS,     0,                  28593,  "Western Europe and South European (Turkish, Maltese plus Esperanto)"),
    _C("ISO-8859-4",        BFTYP_SBCS,     0,                  28594,  "Western Europe and Baltic countries (Lithuania, Estonia and Lapp)"),
    _C("ISO-8859-5",        BFTYP_SBCS,     0,                  28595,  "Cyrillic alphabet"),
    _C("ISO-8859-6",        BFTYP_SBCS,     0,                  28596,  "Arabic"),
    _C("ISO-8859-7",        BFTYP_SBCS,     0,                  28597,  "Greek"),
    _C("ISO-8859-8",        BFTYP_SBCS,     0,                  28598,  "Hebrew"),
    _C("ISO-8859-9",        BFTYP_SBCS,     0,                  28599,  "Western Europe with amended Turkish character set"),
    _X("ISO-8859-10",       BFTYP_SBCS,     0,                          "Western Europe with rationalised character set for Nordic languages"),
    _C("ISO-8859-13",       BFTYP_SBCS,     0,                  28603,  "Baltic languages plus Polish"),
    _X("ISO-8859-14",       BFTYP_SBCS,     0,                          "Celtic languages (Irish Gaelic, Scottish, Welsh"),
    _C("ISO-8859-15",       BFTYP_SBCS,     0,                  28605,  "Euro sign and other rationalisations to ISO 8859-1"),
    _X("ISO-8859-16",       BFTYP_SBCS,     0,                          "Central, Eastern and Southern European languages"),

    _C("CP037",             BFTYP_EBCDIC,   0,                  37,     "EBCDIC-US"),
    _C("CP038",             BFTYP_EBCDIC,   0,                  38,     "EBCDIC-INT"),
    _C("CP930",             BFTYP_EBCDIC,   0,                  930,    NULL),
    _C("CP1047",            BFTYP_EBCDIC,   0,                  1047,   NULL),

    _C("UTF-8",             BFTYP_UTF8,     0,                  65001,  NULL),
    _X("UTF-16",            BFTYP_UTF16,    0,                          NULL),
    _C("UTF-16be",          BFTYP_UTF16,    BFFLG_BE,           1201,   NULL),
    _C("UTF-16le",          BFTYP_UTF16,    BFFLG_LE,           1200,   NULL),
    _X("UTF-32",            BFTYP_UTF32,    0,                          NULL),
    _X("UTF-32be",          BFTYP_UTF32,    BFFLG_BE,                   NULL),
    _X("UTF-32le",          BFTYP_UTF32,    BFFLG_LE,                   NULL),

    _X("BOCU-1",            BFTYP_BOCU1,    0,                          NULL),
    _X("SCSU",              BFTYP_SCSU,     0,                          NULL),
    _C("UTF-7",             BFTYP_UTF7,     0,                  65002,  NULL),

    _X("UTF-4",             BFTYP_UCS4,     0,                          NULL),
    _X("UTF-4be",           BFTYP_UCS4,     BFFLG_BE,                   NULL),
    _X("UTF-4le",           BFTYP_UCS4,     BFFLG_LE,                   NULL),
    _X("UTF-2",             BFTYP_UCS2,     0,                          NULL),
    _X("UTF-2be",           BFTYP_UCS2,     BFFLG_BE,                   NULL),
    _X("UTF-2le",           BFTYP_UCS2,     BFFLG_LE,                   NULL),

    _C("cp437",             BFTYP_SBCS,     0,                  437,    "OEM/US - ASCII"),
    _C("cp737",             BFTYP_SBCS,     0,                  737,    "Greek, ISO-8859-7"),
    _C("cp775",             BFTYP_SBCS,     0,                  775,    "Baltic"),
    _C("cp850",             BFTYP_SBCS,     0,                  850,    "Like ISO-8859-4"),
    _C("cp852",             BFTYP_SBCS,     0,                  852,    "Like ISO-8859-1"),
    _C("cp855",             BFTYP_SBCS,     0,                  855,    "Like ISO-8859-2"),
    _C("cp857",             BFTYP_SBCS,     0,                  857,    "Like ISO-8859-5"),
    _C("cp860",             BFTYP_SBCS,     0,                  860,    "Like ISO-8859-9"),
    _C("cp861",             BFTYP_SBCS,     0,                  861,    "Like ISO-8859-1"),
    _C("cp862",             BFTYP_SBCS,     0,                  862,    "Like ISO-8859-1"),
    _C("cp863",             BFTYP_SBCS,     0,                  863,    "Like ISO-8859-8"),
    _C("cp865",             BFTYP_SBCS,     0,                  865,    "Like ISO-8859-1"),
    _C("cp866",             BFTYP_SBCS,     0,                  866,    "Like ISO-8859-5"),
    _C("cp869",             BFTYP_SBCS,     0,                  869,    "Greek, like ISO-8859-7"),
    _C("cp874",             BFTYP_SBCS,     0,                  874,    "Thai"),
    _C("cp1046",            BFTYP_SBCS,     0,                  1046,   "Arabic DOS code"),

    _C("windows-1250",      BFTYP_SBCS,     0,                  1250,   "Central European languages that use Latin script (Polish, Czech etc)"),
    _C("windows-1251",      BFTYP_SBCS,     0,                  1251,   "Cyrillic alphabets"),
    _C("windows-1252",      BFTYP_SBCS,     0,                  1252,   "Western languages"),
    _C("windows-1253",      BFTYP_SBCS,     0,                  1253,   "Greek"),
    _C("windows-1254",      BFTYP_SBCS,     0,                  1254,   "Turkish"),
    _C("windows-1255",      BFTYP_SBCS,     0,                  1255,   "Hebrew"),
    _C("windows-1256",      BFTYP_SBCS,     0,                  1256,   "Arabic"),
    _C("windows-1257",      BFTYP_SBCS,     0,                  1257,   "Baltic languages"),
    _C("windows-1258",      BFTYP_SBCS,     0,                  1258,   "Vietnamese"),

    _X("Mac-Arabic",        BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Celtic",        BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Centeuro",      BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Croatian",      BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Cyrillic",      BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Devanaga",      BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Dingbats",      BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Farsi",         BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Gaelic",        BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Greek",         BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Gujarati",      BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Gurmukhi",      BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Hebrew",        BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Iceland",       BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Inuit",         BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Roman",         BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Romanian",      BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Thai",          BFTYP_SBCS,     0,                          NULL),
    _X("Mac-Turkish",       BFTYP_SBCS,     0,                          NULL),
/*  _X("Mac-Ukraine",       -1,     */
/*  _X("Mac-Chinsimp",      -1,     */
/*  _X("Mac-Chintrad",      -1,     */
/*  _X("Mac-Corpchar",      -1,     */
/*  _X("Mac-Japanese",      -1,     */
/*  _X("Mac-Keyboard",      -1,     */
/*  _X("Mac-Korean",        -1,     */

    _C("cp10000",           BFTYP_SBCS,     0,                  10000,  "MacRoman"),
    _C("cp10006",           BFTYP_SBCS,     0,                  10006,  "MacGreek"),
    _C("cp10007",           BFTYP_SBCS,     0,                  10007,  "MacCyrillic"),
    _C("cp10029",           BFTYP_SBCS,     0,                  10029,  "MacLatin2"),
    _C("cp10079",           BFTYP_SBCS,     0,                  10079,  "MacIcelandic"),
    _C("cp10081",           BFTYP_SBCS,     0,                  10081,  "MacTurkish"),

    _C("KOI8-R",            BFTYP_SBCS,     0,                  20866,  "Russian, using cynrillic alphabet"),
    _C("KOI8-U",            BFTYP_SBCS,     0,                  21866,  "Ukrainian, using cynrillic alphabet"),
    _X("KOI8-T",            BFTYP_SBCS,     0,                          "Ukrainian"),
    _X("PT154",             BFTYP_SBCS,     0,                          "Ukrainian"),
    _X("KOI7",              BFTYP_SBCS,     BFFLG_7BIT,                 "Ukrainian"),

    _C("MIK",               BFTYP_SBCS,     0,                  0,      "Bulgarian"),

    _X("ISCII",             BFTYP_SBCS,     0,                          "Indian Script Code for Information Interchange."),
    _X("TSCII",             BFTYP_SBCS,     0,                          "Tamil Script Code for Information Interchange."),
    _X("VSCII",             BFTYP_SBCS,     0,                          "Vietnamese Standard Code for Information Interchange."),

    _C("DEC-MCS",           BFTYP_SBCS,     0,                  -2,     NULL),
    _C("DEC-KANJI",         BFTYP_SBCS,     0,                  -2,     NULL),
    _C("DEC-HANYU",         BFTYP_SBCS,     0,                  -2,     NULL),

    _C("HP-Roman8",         BFTYP_SBCS,     0,                  -3,     NULL),
    _C("HP-Arabic8",        BFTYP_SBCS,     0,                  -3,     NULL),
    _C("HP-Greek8",         BFTYP_SBCS,     0,                  -3,     NULL),
    _C("HP-Hebrew8",        BFTYP_SBCS,     0,                  -3,     NULL),
    _C("HP-Turkish8",       BFTYP_SBCS,     0,                  -3,     NULL),
    _C("HP-Kana8",          BFTYP_SBCS,     0,                  -3,     NULL),

    _X("GB2312",            BFTYP_GB,       BFFLG_GB2312,               "Guojia Biaozhun/Simplified Chinese"),
    _C("GBK",               BFTYP_GB,       BFFLG_GBK,          936,    "Chinese/GB (CP936)"),
    _X("GB18030",           BFTYP_GB,       BFFLG_GB18030,              "Chinese National Standard/GB"),
#if defined(BFTYP_HZ)
    _X("HZ",                BFTYP_HZ,       BFFLG_GB18030,              "RFC1843 - Arbitrarily Mixed Chinese and ASCII"),   /*man hztty*/
#endif

    _C("Big5",              BFTYP_BIG5,     BFFLG_BIG5,         950,    "Chinese/Big-5 (CP950)"),
    _X("Big5-5E",           BFTYP_BIG5,     BFFLG_BIG5,                 "Big-5"),
    _X("Big5-2003",         BFTYP_BIG5,     BFFLG_BIG5,                 "Big-5"),
    _X("Big5-HKSCS",        BFTYP_BIG5,     BFFLG_BIG5_HKSCS,           "Big-5/Hong Kong Supplement"),

    _X("Shift_JIS",         BFTYP_MBCS,     BFFLG_SHIFT_JIS,            "Shift JIS"),
    _X("EUC-JP",            BFTYP_MBCS,     BFFLG_EUC_JP,               "Japan/EUC"),
    _C("CP932",             BFTYP_MBCS,     BFFLG_SHIFT_JIS,    932,    "Windows-31J"),

    _X("EUC-CN",            BFTYP_MBCS,     BFFLG_EUC_GB,               "Chinese/EUC"),
    _X("EUC-TW",            BFTYP_MBCS,     BFFLG_EUC_TW,               "Tawian/EUC"),
    _C("EUC-KR",            BFTYP_MBCS,     BFFLG_EUC_KR,       949,    "Korean/EUC (CP949)"),

    _X("ISO-2022-CN",       BFTYP_ISO2022,  BFFLG_ISO2022CN,            NULL),
    _X("ISO-2022-KK",       BFTYP_ISO2022,  BFFLG_ISO2022KK,            NULL),
    _X("ISO-2022-KP",       BFTYP_ISO2022,  BFFLG_ISO2022KP,            NULL),

#undef  _X
#undef  _C
    };


int
mchar_info_init(void)
{
    ICU_initialise();

    /*  CHARSETALIASDIR
     *      LIBDIR/charset.alias
     *      GRPATH/charset.alias
     */
    charset_alias_init();
    mchar_alias_load(CHARSET_MODE_INI, "grcharset.list");
    mchar_alias_load(CHARSET_MODE_X11, "charset.alias");
    return 0;
}


void
mchar_info_shutdown(void)
{
    charset_alias_shutdown();
}


int
mchar_alias_load(int type, const char *aliasfile)
{
    charset_alias_open(type, -1, NULL, aliasfile);
    return 0;
}


#if (XXX)
int
mchar_list(void)
{
#if defined(HAVE_LIBICU)
    const int count = ucnv_countAvailable();
    int i;

    for (i = 0; i <= count; ++i) {
        const char *name = ucnv_getAvailableName(i);

        if (name) {
        }
    }
#endif  /*ICU*/
}
#endif


/*  Function:               mchar_info
 *      Character map definition lookup.
 *
 *      The following logic is very forgiving at that most aliases/short-cut are permitted.
 *
 *  Parameters:
 *      name -                  Character set name.
 *      def -                   Default.
 *
 *  Returns:
 *      xxx
 */
const mcharcharsetinfo_t *
mchar_info(mcharcharsetinfo_t *info, const char *name, int namelen)
{
    const struct charset *map;
    const char *t_name, *charset;
    char buffer[64];
    int offset;

    if (NULL == name || 0 == *name) {
        return NULL;
    }
    if (namelen < 0) {
        namelen = (int)strlen(name);
    }
    if (name != (t_name = mchar_vim_strip(name))) {
        namelen -= (name - t_name);
        name = t_name;
    }
    trace_log("\tmchar_info(length:%d, name:%.*s)\n", namelen, namelen, name);

    /*
     *  canonicalize name
     */
    if (NULL != (charset = charset_canonicalize(name, namelen, buffer, sizeof(buffer))) ||
            NULL != (charset = charset_alias_lookup(name, namelen)) ||
                NULL != (charset = ICU_canonicalName(name, namelen))) {
        name = charset;
        namelen = (int)strlen(charset);
        trace_log("\t= name:<%.*s>\n", namelen, name);
    }

    /*
     *  lookup
     */
    if (NULL != (map = charset_map(name, namelen))) {
      return charsetinfo(map, info);
    } else if (ICU_charsetinfo(name, namelen, info)) {
        return info;
    }

    /*
     *  code-page search:
     *      cp[-]xxx
     *      ibm[-]xxx
     *      windows[-]xxx[x]
     */
    if (0 == str_nicmp(name, "cp", offset = 2) ||
            0 == str_nicmp(name, "ibm", offset = 3) ||
            0 == str_nicmp(name, "windows", offset = 7)) {
        const int codepage =
                    atoi('-' == name[offset] ? name + (offset + 1) : name + offset);
        unsigned idx;

        if (codepage > 0) {
            for (map = x_charactersets, idx = 0; idx < X_CHARACTERSETS; ++map, ++idx) {
                if (codepage == map->cs_codepage) {
                    return charsetinfo(map, info);
                }
            }
        }
    }

    trace_log("\t==> NULL\n");
    return NULL;
}

/*  Function:           mchar_vim_strip
 *      Remove any vim specific prefix from the encoding name.
 *
 *  Parameters:
 *      encoding -          Encoding specification.
 *
 *  Returns:
 *      Encoding with leading prefix removed.
 *
 *  Notes:
 *      8bit    - Single-byte encodings, 256 different characters. Mostly used in USA and
 *      Europe. Example: ISO-8859-1 (Latin1). All characters occupy one screen cell only.
 *
 *      2byte   - Double-byte encodings, over 10000 different characters. Mostly used in
 *      Asian countries. Example: euc-kr (Korean) The number of screen cells is equal
 *      to the number of bytes (except for euc-jp when the first byte is 0x8e).
 */
const char *
mchar_vim_strip(const char *encoding)
{
    if (0 == strncmp(encoding, "8bit-", 5)) {
        return encoding + 5;
    } else if (0 == strncmp(encoding, "2byte-", 6)) {
        return encoding + 6;
    }
    return encoding;
}


static void
ICU_initialise(void)
{
#if defined(HAVE_LIBICU)
    UErrorCode ecode = U_ZERO_ERROR;

    u_init(&ecode);
    if (U_FAILURE(ecode)) {
        panic("can not initialise ICU, status = %s (%d)\n", u_errorName(ecode), ecode);
    }

    if (x_dflags) {
        UEnumeration *allNameEnum;

        allNameEnum = ucnv_openAllNames(&ecode);
        if (allNameEnum) {
            const int32_t count = uenum_count(allNameEnum, &ecode);
            const char *name = NULL;
            int32_t len = 0;

            trace_log("ICU initialise(count: %d\n", (int)count);
            while (NULL != (name = uenum_next(allNameEnum, &len, &ecode))) {
                UConverter *converter = NULL;

                ecode = U_ZERO_ERROR;           /* ICU requirement, reset status */
                if (NULL != (converter = ucnv_open(name, &ecode)) && U_SUCCESS(ecode)) {
                    trace_log("\t[Y] %-32s type:%2d, cp:%5d, range:%d-%d\n", name, 
                        (int)ucnv_getType(converter), (int)ucnv_getCCSID(converter, &ecode),
                        (int)ucnv_getMinCharSize(converter), (int)ucnv_getMaxCharSize(converter));
                    ucnv_close(converter);
                } else {
                    trace_log("\t[N] %s\n", name);
                }
                ecode = U_ZERO_ERROR;
            }
        }
        uenum_close(allNameEnum);
    }
#endif  /*HAVE_LIBICU*/
}


static const char *
ICU_canonicalName(const char *name, int namelen)
{
#if defined(HAVE_LIBICU)
    UErrorCode ecode = U_ZERO_ERROR;
    const char *t_name = chk_snalloc(name, namelen);
    const char *tag = NULL;

    if (t_name) {
        if (NULL != (tag = ucnv_getCanonicalName(t_name, "MIME", &ecode)) ||
                NULL != (tag = ucnv_getCanonicalName(t_name, "IANA", &ecode)) ||
                    NULL != (tag = ucnv_getCanonicalName(t_name, "", &ecode))) {
            tag = ucnv_getAlias(t_name, 0, &ecode);
        }
        chk_free((void *)t_name);
    }
    return tag;
#else
    __CUNUSED(name)
    __CUNUSED(namelen)
    return NULL;
#endif  /*HAVE_LIBICU*/
}


static int
ICU_charsetinfo(const char *name, int namelen, mcharcharsetinfo_t *info)
{
#if defined(HAVE_LIBICU)
#define ICU2TYPE            (sizeof(icu2type)/sizeof(icu2type[0]))

    static const struct icumap {    /* ICU character-set mapping */
        UConverterType      im_icu;             /* ICU encoding type */
        int                 im_type;            /* Internal buffer type */
        uint32_t            im_flags;           /* Encoding specific flags */
    } icu2type[] = {
        #undef  _I
#define _I(_i,_t,_f)        { _i, _t, _f }
#define _X(_i,_t,_f, _x)    { _i, _t, _f }

        _I(UCNV_US_ASCII,           BFTYP_SBCS,     0),
        _I(UCNV_LATIN_1,            BFTYP_SBCS,     0),
        _I(UCNV_UTF8,               BFTYP_UTF8,     0),
        _I(UCNV_UTF16_BigEndian,    BFTYP_UTF16,    BFFLG_BE),
        _I(UCNV_UTF16_LittleEndian, BFTYP_UTF16,    BFFLG_LE),
        _I(UCNV_UTF32_BigEndian,    BFTYP_UTF32,    BFFLG_BE),
        _I(UCNV_UTF32_LittleEndian, BFTYP_UTF32,    BFFLG_LE),
        _I(UCNV_EBCDIC_STATEFUL,    BFTYP_SBCS,     0),
        _I(UCNV_ISO_2022,           BFTYP_ISO2022,  0),
#if defined(BFTYP_HZ)
        _X(UCNV_HZ,                 BFTYP_HZ,       0),
#endif
        _I(UCNV_SCSU,               BFTYP_SCSU,     0),
        _I(UCNV_ISCII,              BFTYP_SBCS,     0),
        _I(UCNV_UTF7,               BFTYP_UTF7,     0),
        _I(UCNV_BOCU1,              BFTYP_BOCU1,    0),
        _I(UCNV_UTF16,              BFTYP_UTF16,    0),
        _I(UCNV_UTF32,              BFTYP_UTF32,    0),
        _I(UCNV_SBCS,               BFTYP_SBCS,     0),
        _I(UCNV_DBCS,               BFTYP_DBCS,     0),
        _I(UCNV_MBCS,               BFTYP_MBCS,     0),

#undef  _I
#undef  _X
        };

    char cnvname[UCNV_MAX_CONVERTER_NAME_LENGTH + 1] = {0};
    UConverter *converter = NULL;
    UErrorCode ecode = U_ZERO_ERROR;
    UConverterType icutype;
    unsigned idx;

    memcpy(cnvname, name, (namelen < UCNV_MAX_CONVERTER_NAME_LENGTH ? namelen : UCNV_MAX_CONVERTER_NAME_LENGTH));
    if (NULL != (converter = ucnv_open(cnvname, &ecode))) {

        if (UCNV_UNSUPPORTED_CONVERTER != (icutype = ucnv_getType(converter))) {
            const struct icumap *icu;
            struct charset t_map = {0};

        //WARNING - name only visible whilst converter is open.
        //  t_map.cs_name     = ucnv_getName(converter, &ecode);
        //  t_map.cs_namelen  = strlen(t_map.cs_name);
            t_map.cs_type     = BFTYP_OTHER;
            t_map.cs_flags    = icutype;
            t_map.cs_codepage = ucnv_getCCSID(converter, &ecode);

            for (icu = icu2type, idx = 0; idx < ICU2TYPE; ++icu, ++idx) {
                if (icutype == icu->im_icu) {   /* well-known mapping */
                    t_map.cs_type = icu->im_type;
                    t_map.cs_flags = icu->im_flags;
                    break;
                }
            }

            if (BFTYP_OTHER == t_map.cs_type) { /* default mapping */
                const int maxchar = ucnv_getMaxCharSize(converter);
                const int minchar = ucnv_getMinCharSize(converter);

                if (1 == maxchar) {
                    t_map.cs_type = BFTYP_SBCS;
                } else if (maxchar == minchar) {
                    t_map.cs_type = BFTYP_DBCS;
                } else if (1 == minchar && maxchar > 1) {
                    t_map.cs_type = BFTYP_MBCS;
                }
            }

            trace_log("\tucnv3(icu:%d, type:%d)\n", (int)icutype, (int)t_map.cs_type);
            (void) charsetinfo(&t_map, info);
            ucnv_close(converter);
            return TRUE;

        } else {
            trace_log("\tucnv2(type-unknown)\n");
        }
        ucnv_close(converter);

    } else {
        trace_log("\tucnv1(error:%s/%d)\n", u_errorName(ecode), ecode);
    }
#else	/*HAVE_LIBICU*/
    __CUNUSED(name)
    __CUNUSED(namelen)
    __CUNUSED(info)
#endif
    return FALSE;
}


static const mcharcharsetinfo_t *
charsetinfo(const struct charset *map, mcharcharsetinfo_t *info)
{
    trace_log("\t==> name:%s, type:%d\n", map->cs_name, map->cs_type);
    if (info) {
        if (0 == map->cs_type) {
            info->cs_type   = BFTYP_UNDEFINED;
            info->cs_flags  = 0;
        } else if (map->cs_type < 0) {
            info->cs_type   = BFTYP_UNSUPPORTED;
            info->cs_flags  = 0;
        } else {
            info->cs_type   = map->cs_type;
            info->cs_flags  = map->cs_flags;
        }
        info->cs_name       = map->cs_name;
        info->cs_codepage   = map->cs_codepage;
        info->cs_desc       = map->cs_desc;
    }
    return info;
}


static const struct charset *
charset_map(const char *name, int namelen)
{
    const struct charset *maps = x_charactersets;
    unsigned idx;

    for (idx = 0; idx < X_CHARACTERSETS; ++maps, ++idx) {
        if (0 == charset_compare(maps->cs_name, name, namelen)) {
            return maps;
        }
    }
    return NULL;
}

/*end*/
