#include <edidentifier.h>
__CIDENT_RCSID(gr_m_mchar_c,"$Id: m_mchar.c,v 1.11 2014/10/22 02:33:05 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_mchar.c,v 1.11 2014/10/22 02:33:05 ayoung Exp $
 * Multibyte/locale primitives.
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
#include <chkalloc.h>

#include "m_mchar.h"

#include "accum.h"                              /* acc_...() */
#include "buffer.h"                             /* buf_...() */
#include "builtin.h"
#include "eval.h"                               /* get/isa_...() */


/*  Function:           do_set_encoding
 *      set_encoding primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: set_encoding - Set a buffers character encoding.

        int
        set_encoding([string encoding = NULL],
            [int bufnum = NULL])

    Macro Description:
        The 'set_encoding()' primitive sets or clears the character
        encoding associated with the referenced buffer.

        The following table lists the character encodes which maybe
        available dependent on build options and system support.

(start table,format=nd)
        [Name           [Buffer Type    [Code Page  [Description                                                        ]

      ! US-ASCII        BFTYP_SBCS      646         ANSI/ASCII

      ! ISO-8859-1      BFTYP_SBCS      28591       Western Europe
      ! ISO-8859-2      BFTYP_SBCS      28592       Western and Central Europe
      ! ISO-8859-3      BFTYP_SBCS      28593       Western Europe and South European (Turkish, Maltese plus Esperanto)
      ! ISO-8859-4      BFTYP_SBCS      28594       Western Europe and Baltic countries (Lithuania, Estonia and Lapp)
      ! ISO-8859-5      BFTYP_SBCS      28595       Cyrillic alphabet
      ! ISO-8859-6      BFTYP_SBCS      28596       Arabic
      ! ISO-8859-7      BFTYP_SBCS      28597       Greek
      ! ISO-8859-8      BFTYP_SBCS      28598       Hebrew
      ! ISO-8859-9      BFTYP_SBCS      28599       Western Europe with amended Turkish character set
      ! ISO-8859-10     BFTYP_SBCS                  Western Europe with rationalised character set for Nordic languages
      ! ISO-8859-13     BFTYP_SBCS      28603       Baltic languages plus Polish
      ! ISO-8859-14     BFTYP_SBCS                  Celtic languages (Irish Gaelic, Scottish, Welsh
      ! ISO-8859-15     BFTYP_SBCS      28605       Euro sign and other rationalisations to ISO 8859-1
      ! ISO-8859-16     BFTYP_SBCS                  Central, Eastern and Southern European languages

      ! CP037           BFTYP_EBCDIC    37          EBCDIC-US
      ! CP038           BFTYP_EBCDIC    38          EBCDIC-INT
      ! CP930           BFTYP_EBCDIC    930
      ! CP1047          BFTYP_EBCDIC    1047

      ! UTF-8           BFTYP_UTF8      65001
      ! UTF-16          BFTYP_UTF16
      ! UTF-16be        BFTYP_UTF16     1201
      ! UTF-16le        BFTYP_UTF16     1200
      ! UTF-32          BFTYP_UTF32
      ! UTF-32be        BFTYP_UTF32
      ! UTF-32le        BFTYP_UTF32

      ! BOCU-1          BFTYP_BOCU1
      ! SCSU            BFTYP_SCSU
      ! UTF-7           BFTYP_UTF7      65002

      ! UTF-4           BFTYP_UCS4
      ! UTF-4be         BFTYP_UCS4
      ! UTF-4le         BFTYP_UCS4
      ! UTF-2           BFTYP_UCS2
      ! UTF-2be         BFTYP_UCS2
      ! UTF-2le         BFTYP_UCS2

      ! cp437           BFTYP_SBCS      437         OEM/US, ASCII
      ! cp737           BFTYP_SBCS      737         Greek, ISO-8859-7
      ! cp775           BFTYP_SBCS      775         Baltic
      ! cp850           BFTYP_SBCS      850         Like ISO-8859-4
      ! cp852           BFTYP_SBCS      852         Like ISO-8859-1
      ! cp855           BFTYP_SBCS      855         Like ISO-8859-2
      ! cp857           BFTYP_SBCS      857         Like ISO-8859-5
      ! cp860           BFTYP_SBCS      860         Like ISO-8859-9
      ! cp861           BFTYP_SBCS      861         Like ISO-8859-1
      ! cp862           BFTYP_SBCS      862         Like ISO-8859-1
      ! cp863           BFTYP_SBCS      863         Like ISO-8859-8
      ! cp865           BFTYP_SBCS      865         Like ISO-8859-1
      ! cp866           BFTYP_SBCS      866         Like ISO-8859-5
      ! cp869           BFTYP_SBCS      869         Greek, like ISO-8859-7
      ! cp874           BFTYP_SBCS      874         Thai
      ! cp1046          BFTYP_SBCS      1046        Arabic DOS code

      ! windows-1250    BFTYP_SBCS      1250        Central European languages that use Latin script (Polish, Czech etc).
      ! windows-1251    BFTYP_SBCS      1251        Cyrillic alphabets
      ! windows-1252    BFTYP_SBCS      1252        Western languages
      ! windows-1253    BFTYP_SBCS      1253        Greek
      ! windows-1254    BFTYP_SBCS      1254        Turkish
      ! windows-1255    BFTYP_SBCS      1255        Hebrew
      ! windows-1256    BFTYP_SBCS      1256        Arabic
      ! windows-1257    BFTYP_SBCS      1257        Baltic languages
      ! windows-1258    BFTYP_SBCS      1258        Vietnamese

      ! Mac-Arabic      BFTYP_SBCS
      ! Mac-Celtic      BFTYP_SBCS
      ! Mac-Centeuro    BFTYP_SBCS
      ! Mac-Croatian    BFTYP_SBCS
      ! Mac-Cyrillic    BFTYP_SBCS
      ! Mac-Devanaga    BFTYP_SBCS
      ! Mac-Dingbats    BFTYP_SBCS
      ! Mac-Farsi       BFTYP_SBCS
      ! Mac-Gaelic      BFTYP_SBCS
      ! Mac-Greek       BFTYP_SBCS
      ! Mac-Gujarati    BFTYP_SBCS
      ! Mac-Gurmukhi    BFTYP_SBCS
      ! Mac-Hebrew      BFTYP_SBCS
      ! Mac-Iceland     BFTYP_SBCS
      ! Mac-Inuit       BFTYP_SBCS
      ! Mac-Roman       BFTYP_SBCS
      ! Mac-Romanian    BFTYP_SBCS
      ! Mac-Thai        BFTYP_SBCS
      ! Mac-Turkish     BFTYP_SBCS

      ! cp10000         BFTYP_SBCS      10000       MacRoman
      ! cp10006         BFTYP_SBCS      10006       MacGreek
      ! cp10007         BFTYP_SBCS      10007       MacCyrillic
      ! cp10029         BFTYP_SBCS      10029       MacLatin2
      ! cp10079         BFTYP_SBCS      10079       MacIcelandic
      ! cp10081         BFTYP_SBCS      10081       MacTurkish

      ! KOI8-R          BFTYP_SBCS      20866       Russian, using cynrillic alphabet.
      ! KOI8-U          BFTYP_SBCS      21866       Ukrainian, using cynrillic alphabet.
      ! KOI8-T          BFTYP_SBCS                  Ukrainian
      ! PT154           BFTYP_SBCS                  Ukrainian
      ! KOI7            BFTYP_SBCS                  Ukrainian

      ! MIK             BFTYP_SBCS      0           Bulgarian

      ! ISCII           BFTYP_SBCS                  Indian Script Code for Information Interchange.
      ! TSCII           BFTYP_SBCS                  Tamil Script Code for Information Interchange.
      ! VSCII           BFTYP_SBCS                  Vietnamese Standard Code for Information Interchange.

      ! DEC-MCS         BFTYP_SBCS      -2
      ! DEC-KANJI       BFTYP_SBCS      -2
      ! DEC-HANYU       BFTYP_SBCS      -2

      ! HP-Roman8       BFTYP_SBCS      -3
      ! HP-Arabic8      BFTYP_SBCS      -3
      ! HP-Greek8       BFTYP_SBCS      -3
      ! HP-Hebrew8      BFTYP_SBCS      -3
      ! HP-Turkish8     BFTYP_SBCS      -3
      ! HP-Kana8        BFTYP_SBCS      -3

      ! GB2312          BFTYP_GB                    Guojia Biaozhun/Simplified Chinese.
      ! GBK             BFTYP_GB        936         Chinese/GB (CP936).
      ! GB18030         BFTYP_GB                    Chinese National Standard/GB.
      ! HZ              BFTYP_HZ                    RFC1843, Arbitrarily Mixed Chinese and ASCII.

      ! Big5            BFTYP_BIG5      950         Chinese/Big-5 (CP950).
      ! Big5-5E         BFTYP_BIG5                  Big-5.
      ! Big5-2003       BFTYP_BIG5                  Big-5.
      ! Big5-HKSCS      BFTYP_BIG5                  Big-5/Hong Kong Supplement.

      ! Shift_JIS       BFTYP_MBCS                  Shift JIS.
      ! EUC-JP          BFTYP_MBCS                  Japan/EUC.
      ! CP932           BFTYP_MBCS      932         Windows-31J.

      ! EUC-CN          BFTYP_MBCS                  Chinese/EUC.
      ! EUC-TW          BFTYP_MBCS                  Tawian/EUC.
      ! EUC-KR          BFTYP_MBCS      949         Korean/EUC (CP949).

      ! ISO-2022-CN     BFTYP_ISO2022
      ! ISO-2022-KK     BFTYP_ISO2022
      ! ISO-2022-KP     BFTYP_ISO2022
(end table)

    Macro Parameters:
        encoding - Optional encoding name, if omitted the encoding is
            derived from the buffer type (see set_buffer_type).

        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        nothing

    Macro Portability:
        A Grief extension

    Macro See Also:
        inq_encoding, inq_buffer_type
 */
void
do_set_encoding(void)           /* void ([string encoding = NULL], [int bufnum = NULL]) */
{
    const char *encoding = get_xstr(1);
    BUFFER_t *bp = buf_argument(2);

    buf_encoding_set(bp, encoding);
}


/*  Function:           inq__encoding
 *      inq_encoding primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: inq_encoding - Retrieve a buffers character encoding.

        string
        inq_encoding([int bufnum])

    Macro Description:
        The 'inq_encoding()' primitive retrieves the character encoding
        associated with the referenced buffer. See <set_encoding> for
        possible encodings.

    Macro Parameters:
        bufnum - Optional buffer number, if omitted the current buffer
            shall be referenced.

    Macro Returns:
        The 'inq_encoding()' primitive returns the associated encoding.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_encoding
 */
void
inq_encoding(void)              /* string ([int bufnum]) */
{
    const BUFFER_t *bp = buf_argument(1);

    acc_assign_str(bp && bp->b_encoding ? bp->b_encoding : "", -1);
}


/*  Function:           inq__encodings
 *      inq_encodings primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<GRIEF-TODO>
    Macro: inq_encodings - List of available encodings.

        list
        inq_encodings([int flags])

    Macro Description:
        The 'set_encodings()' primitive retrieves a list of available
        character encoding's.

    Macro Parameters:
        flags - Optional integer flags.

    Macro Returns:
        List of string + integer pairs defining the available character
        encoding's.

    Macro Portability:
        A Grief extension.

    Macro See Also:
        set_buffer_encoding
 */
void
inq_encodings(void)             /* list ([int flags]) */
{
    //TODO
}
/*end*/
