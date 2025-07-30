#include <edidentifier.h>
__CIDENT_RCSID(gr_kbprotocols_c,"$Id: kbprotocols.c,v 1.6 2025/02/07 03:03:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: kbprotocols.c,v 1.6 2025/02/07 03:03:21 cvsuser Exp $
 * Keyboard input protocols.
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
#include <assert.h>

#if defined(WIN32)
#include <windows.h>                            /* window definitions - MUST be before alt.h */
#elif defined(__CYGWIN__)
#include <w32api/windows.h>
#endif
#include <edalt.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "kbprotocols.h"
#include "keyboard.h"

#include "debug.h"
#include "main.h"
#include "mouse.h"
#include "tty.h"
#include "ttyutil.h"

#if (!defined(USE_VIO_BUFFER) && !defined(DJGPP)) || defined(KBPROTOCOLS_TEST)
#define USE_KBPROTOCOL
#endif

#if defined(USE_KBPROTOCOL)
static int              kbutf8(const char *buf, unsigned buflen, int force);
static int              kbcygwin(const char *buf, unsigned buflen);
static int              kbss3(const char *buf, unsigned buflen);
static int              kbcsi(const char *buf, unsigned buflen);
static int              kbansi(const char *buf, unsigned buflen);

static int              mok2_to_key(const char *buf, unsigned buflen);
static KEY              ss3_to_key(unsigned char ch);
static int              vt220_to_key(const char *buf, unsigned buflen);
static unsigned         xterm_modifiers(unsigned mods);
static int              cygwin_to_int(const char *buf);
static int              msterminal_to_int(const char *buf);
#endif

static const struct {
    const char* name;
    size_t namelen;
    int mode;
} protocols[] = {       // Keyboard protocols
#define PROTONAME(__name)       __name, sizeof(__name)-1
        { PROTONAME("none"),        KBPROTOCOL_NONE },
        { PROTONAME("auto"),        KBPROTOCOL_AUTO },
        { PROTONAME("meta"),        KBPROTOCOL_META },          // metaSendsEscape/altSendsEscape
    //  { PROTONAME("vt220"),       KBPROTOCOL_VT220 },         // vt220+; implied
        { PROTONAME("cygwin"),      KBPROTOCOL_CYGWIN },
    //  { PROTONAME("vtmok"),       KBPROTOCOL_VT_MOK },        // SS3 <modifier> <ss3>; implied
        { PROTONAME("xterm-mok2"),  KBPROTOCOL_XTERM_MOK2 },
        { PROTONAME("mintty-mok2"), KBPROTOCOL_MINTTY_MOK2 },
#if defined(KBPROTOCOL_MSTERMINAL)
        { PROTONAME("msterminal"),  KBPROTOCOL_MSTERMINAL },
#endif
#if defined(KBPROTOCOL_KITTY)
        { PROTONAME("kitty"),       KBPROTOCOL_KITTY }
#endif
#undef PROTONAME
};


//  VT220/ANSI - Legacy key codes.
//
//      Key         SS3 (1)     CSI (2)     Modified/CSI (2)
//      ------------------------------------------------------
//      Up          ^[OA        ^[[A        ^[[1;mA
//      Down        ^[OB        ^[[B        ^[[1;mB
//      Right       ^[OC        ^[[C        ^[[1;mC
//      Left        ^[OD        ^[[D        ^[[1;mD
//      End         ^[OF        ^[[F        ^[[1;mF
//      Home        ^[OH        ^[[H        ^[[1;mH
//      F1          ^[OP        ^[[P        ^[[1;mP
//      F2          ^[OQ        ^[[Q        ^[[1;mQ
//      F3          ^[OR        ^[[R        ^[[1;mR
//      F4          ^[OS        ^[[S        ^[[1;mS
//
//      Enter       ^[OmM                   ^[[1;mM
//      Equal       ^[OmX                   ^[[1;mX
//      *           ^[Omj                   ^[[1;mj
//      +           ^[Omk                   ^[[1;mk
//      ,           ^[Oml                   ^[[1;ml
//      -           ^[Omm                   ^[[1;mm
//      .           ^[Omn                   ^[[1;mn
//      /           ^[Omo                   ^[[1;mo
//      0           ^[Omp                   ^[[1;mp
//      1           ^[Omq                   ^[[1;mq
//          ...
//      8           ^[Omx                   ^[[1;mx
//      9           ^[Omy                   ^[[1;my
//      ------------------------------------------------------
//
//      (1) Keypad Application Mode
//      (2) Normal Mode
//
static const struct {   // VTxxx, SS3
    unsigned char ch;
    KEY key;
} ss3_config[] = {
#define SS3_MIN         0x40
#define SS3_MAX         0x7f
#define SS3_RANGE       ((SS3_MAX - SS3_MIN) + 1)

        // DECNKM - Cursor key escape sequences
        { 'A', KEY_UP },            // SS3 A or CSI 1;mA
        { 'B', KEY_DOWN },          // SS3 B or CSI 1;mB
        { 'C', KEY_RIGHT },         // SS3 C or CSI 1;mC
        { 'D', KEY_LEFT },          // SS3 D or CSI 1;mD

        { 'F', KEY_END },           // SS3 F or CSI 1;mF
        { 'H', KEY_HOME },          // SS3 H or CSI 1;mH
        { 'I', KEY_TAB },

        // DECKPAM - Application mode escape sequences
        { 'P', F(1) },              // SS3 P or CSI 1;mP
        { 'Q', F(2) },              // SS3 Q or CSI 1;mQ
        { 'R', F(3) },              // SS3 R or CSI 1;mR
        { 'S', F(4) },              // SS3 S or CSI 1;mS

        { 'M', KEYPAD_ENTER },      // SS3 M or CSI 1;mM, <enter>
        { 'X', KEYPAD_EQUAL },      // SS3 X or CSI 1;mX, '='
        { 'j', KEYPAD_STAR },       // SS3 j or CSI 1;mj, '*'
        { 'k', KEYPAD_PLUS },       // SS3 k or CSI 1;mk, '+'
        { 'l', ',' },               // SS3 l or CSI 1;ml, ','
        { 'm', KEYPAD_MINUS },      // SS3 m or CSI 1;mm, '-'
        { 'n', KEYPAD_DEL },        // SS3 n or CSI 1;mn, '.'
        { 'o', KEYPAD_DIV },        // SS3 o or CSI 1;mo, '/'
        { 'p', KEYPAD_0 },          // SS3 p or CSI 1;mp, '0'
        { 'q', KEYPAD_1 },          // SS3 q or CSI 1;mq, '1'
        { 'r', KEYPAD_2 },          // SS3 r or CSI 1;mr, '2'
        { 's', KEYPAD_3 },          // SS3 s or CSI 1;ms, '3'
        { 't', KEYPAD_4 },          // SS3 t or CSI 1;mt, '4'
        { 'u', KEYPAD_5 },          // SS3 u or CSI 1;mu, '5'
        { 'v', KEYPAD_6 },          // SS3 v or CSI 1;mv, '6'
        { 'w', KEYPAD_7 },          // SS3 w or CSI 1;mw, '7'
        { 'x', KEYPAD_8 },          // SS3 x or CSI 1;mx, '8'
        { 'y', KEYPAD_9 },          // SS3 y or CSI 1;my, '9'
        };

static KEY ss3_map[SS3_RANGE] = {0};

static const KEY vt_map[] = {
        //              // VTxxx, CSI
        //  Note:
        //  (*)  Alternative encoding, see also sse3_config.
        //  (na) Discontinuity of numbered function keys, gap after 5th key.
        //
#define VT_MIN          1
#define VT_MAX          34

                                    // Legend       Key
        KEY_SEARCH,                 // Find         CSI 1 ~
        KEY_INS,                    // Ins Here     CSI 2 ~
        KEY_DEL,                    // Remove       CSI 3 ~
        KEY_END,                    // Select       CSI 4 ~
        KEY_PAGEUP,                 // Prev         CSI 5 ~
        KEY_PAGEDOWN,               // Next         CSI 6 ~
        KEY_HOME,                   //              CSI 7 ~  (*)
        KEY_END,                    //              CSI 8 ~  (*)
        0,                          //              9=n/a
        0,                          //              10=n/a
        F(1),                       // Hold         CSI 11 ~ (*)
        F(2),                       // Print        CSI 12 ~ (*)
        F(3),                       // Setup        CSI 13 ~ (*)
        F(4),                       // Session      CSI 14 ~ (*)
        F(5),                       // Break        CSI 15 ~
        0,                          //              16=n/a
        F(6),                       //              CSI 17 ~
        F(7),                       //              CSI 18 ~
        F(8),                       //              CSI 19 ~
        F(9),                       //              CSI 20 ~
        F(10),                      //              CSI 21 ~
        0,                          //              22=n/a
        F(11),                      // ESC          CSI 23 ~
        F(12),                      // BS           CSI 24 ~

        // 25+ ambiguous
        SF(1),                      // LF           CSI 25 ~
        SF(2),                      //              CSI 26 ~
        0,                          //              27=reserved
        SF(3),                      // Help         CSI 28 ~
        SF(4),                      // Do           CSI 29 ~
        0,                          //              30=n/a
        SF(5),                      //              CSI 31 ~
        SF(6),                      //              CSI 32 ~
        SF(7),                      //              CSI 33 ~
        SF(8),                      //              CSI 34 ~
        };


/*  Function:           kbprotocols_init()
 *      Keyboard protocols run-time initialisation.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 */
void
kbprotocols_init(void)
{
    unsigned i;

    assert((VT_MAX - VT_MIN) + 1 == (sizeof(vt_map)/sizeof(vt_map[0])));

    for (i = 0; i < (sizeof(ss3_config)/sizeof(ss3_config[0])); ++i) {
        const unsigned char ch = ss3_config[i].ch;
        assert(ch >= SS3_MIN && ch <= SS3_MAX);
        ss3_map[ch - SS3_MIN] = ss3_config[i].key;
    }
}


/*  Function:           key_protocolid
 *      Map a keyboard protocol to it's identifier.
 *
 *  Parameters:
 *      name - Keyboard protocol name.
 *
 *  Results:
 *      Protocol identifier, otherwise -1.
 */
int
key_protocolid(const char* name, int namelen)
{
    if (name && *name) {
        unsigned p = 0;

        if (namelen < 0) {
            namelen = (int)strlen(name);
        }
        for (p = 0; p < (sizeof(protocols) / sizeof(protocols[0])); ++p) {
            if (namelen == (int)protocols[p].namelen &&
                    0 == memcmp(protocols[p].name, name, namelen)) {
                return protocols[p].mode;
            }
        }
    }
    return -1;
}


/*  Function:           key_protocolname
 *      Map a keyboard identifier to it's protocol.
 *
 *  Parameters:
 *      mode - Keyboard protocol identifier.
 *      def - Default on non-match.
 *
 *  Results:
 *      Protocol name, otherwise the default value.
 */
const char *
key_protocolname(int mode, const char *def)
{
    unsigned p = 0;

    for (p = 0; p < (sizeof(protocols) / sizeof(protocols[0])); ++p) {
        if (mode == protocols[p].mode) {
            return protocols[p].name;
        }
    }
    return def;
}



/*  Function:           key_protocoldecode
 *      Keyboard identifier map description.
 *
 *  Parameters:
 *      mode - Keyboard protocol identifier.
 *      buffer - Destination buffer.
 *      buflen - Buffer length, in bytes.
 *
 *  Results:
 *      Protocol description.
 */
const char *
key_protocoldecode(int mode, char *buffer, unsigned buflen)
{
    if (buffer && buflen) {
        unsigned p = 0;

        buffer[0] = 0;
        for (p = 0; p < (sizeof(protocols) / sizeof(protocols[0])); ++p) {
            if (mode & protocols[p].mode) {
                if (buffer[0])                  // name[,name]
                    strxcat(buffer, ",", buflen);
                strxcat(buffer, protocols[p].name, buflen);
            }
        }
    }
    return buffer;
}


#if defined(USE_KBPROTOCOL)

/*  Function:           kbprotocols_parse
 *      Apply active keyboard protocols.
 *
 *      Representations:
 *
 *          Protocol        Leading         Syntax
 *          --------------------------------------------------------------------------------------------------------------------------------------------
 *          events          CSI ..          CSI <specials>... - example Mouse and Focus events, handled externally.
 *
 *          cygwin          ESC { ..        <ESC>{ Kd;Rc;Vk;Sc;Uc;Cs K
 *
 *          msterminal      CSI ..          CSI Vk;Sc;Uc;Kd;Cs;Rc _
 *
 *          kitty           CSI ..          CSI <codepoint:9,13,27,127,57344 - 63743>(;<modifier>) u
 *
 *          mok2            CSI ..          CSI <char> (;<modifier>) u
 *
 *                          CSI ...         CSI 27; <char> (;<modifier>) ~
 *
 *                          CSI ..          CSI 1 (;<modifier>) <sse3-key:full>
 *
 *          vt/ansi         CSI ..          CSI <vt-key> (;<modifier>) ~
 *
 *                          CSI ..          CSI 1 (;<modifier>) <sse3-key:{ABCDFHPQRS}>
 *
 *                          CSI ..          CSI <sse3-key:{ABCDFHPQRS}>
 *
 *                          SS3 ..          SS3 (<modifier>)<ss3-key:full>
 *
 *          meta            ESC ..          ESC <C0-control-code or character>
 *
 *          utf8            11xxxxxx        2byte/110xxxxx 10xxxxxx, 3byte/1110xxxx 10xxxxxx 10xxxxxx and 4byte/11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 *          control         ..              Ctrl-Space(0x00), Tab(0x09), Enter (0x0d), BackSpace(0x08 and 0x7f), and Escape (0x1b)
 *
 *          ascii           0xxxxxxx        0x20 >= char <= 0x7f
 *          -------------------------------------------------------------------------------------------------------------------------------------------
 *
 *      Specials (covered via define_key_seq):
 *
 *          Key                             Sequence
 *          -------------------------------------------------------
 *          Backspace                       0x7f
 *          Shift-Tab                       CSI Z
 *          Alt-Shift-Tab                   0x1b CSI Z
 *          Ctrl-Shift-Tab                  CSI 1 ; 5 Z
 *          -------------------------------------------------------
 *
 *  Parameters:
 *      buf - Escape sequence buffer.
 *      buflen - Buffer length in bytes.
 *      force - Force condition.
 *
 *  Results:
 *      Keycode, otherwise KEYRESULT_MORE or KEYRESULT_NONE.
 */
int
kbprotocols_parse(const char *buf, const unsigned buflen, const int force)
{
#if defined(KBPROTOCOL_MSTERMINAL)
#define KBPROTOCOL_1 KBPROTOCOL_MSTERMINAL
#else
#define KBPROTOCOL_1 0
#endif
#if defined(KBPROTOCOL_KITTY)
#define KBPROTOCOL_2 KBPROTOCOL_KITTY
#else
#define KBPROTOCOL_2 0
#endif

    const unsigned char *xbuf = ((const unsigned char *)buf);
    const unsigned char c0 = xbuf[0];

    if (c0 == 0) {
        //
        //  ASCII NUL = Ctrl-Space
        //
        if (buflen == 1) {
            return __CTRL(' ');
        }

    } else if (c0 < 0x20) {
        //
        //  0x01 .. 0x1f - C0 range
        //
        if (c0 == 0x1b) {
            if (buflen == 1) {
                trace_log("kbproto-esc: -1\n");
                return KEYRESULT_MORE;

            } else {
                const unsigned char c1 = xbuf[1];

                if (force && 2 == buflen) {     // unambiguous
                    if (xf_kbprotocol & KBPROTOCOL_META) {
                        goto meta;              // ESC <ch>
                    }
                }

                switch (c1) {
                case 0x1b:  // ESC-ESC
                    // Ambiguous; wait on force condition.
                    break;

                case 0x4f:  // ESC-SS3
                    return kbss3(buf, buflen);

             // case 0x50:  // ESC-DCS
             // case 0x5d:  // ESC-OSC
             //     return kbctrl(buf, buflen);

                case 0x5b:  // ESC-CSI
                    if (xf_kbprotocol & (KBPROTOCOL_MOK2|KBPROTOCOL_1|KBPROTOCOL_2)) {
                        return kbcsi(buf, buflen);
                    } else {
                        return kbansi(buf, buflen);
                    }
                    break;

                case '{':   /* cygwin-raw-mode */
                    if (xf_kbprotocol & KBPROTOCOL_CYGWIN) {
                        return kbcygwin(buf, buflen);
                    }
                    /*fallthru*/

                default:    /* metaSendsEscape/altSendsEscape */
                    if (xf_kbprotocol & KBPROTOCOL_META) {
                        if (2 == buflen) {      // ESC <ch>
meta:;                      if ((c1 >= 0x20 && c1 < 0x80) || c1 == KEY_TAB || c1 == 0x1b) {
                                int kcode = MOD_META | c1;
                                if (c1 >= 'a' && c1 <= 'z') {
                                    kcode += (unsigned)('A' - 'a');
                                }
                                trace_log("kbproto-meta: %d\n", kcode);
                                return kcode;

                            } else if (c1 >= 0x01 && c1 < 0x20) {
                                const int kcode = MOD_CTRL | MOD_META | (('A' - 1) + c1);
                                trace_log("kbproto-meta: %d\n", kcode);
                                return kcode;
                            }
                        }
                    }
                }
            }
        }

    } else if (c0 >= 0x20 && c0 < 0x80) {
        //
        //  0x20 .. 0x79 - ASCII range
        //

    } else if (c0 >= 0x80 && c0 < 0xa0) {
        //
        //  0x80 .. 0x9f - C1 range
        //
        if (buflen >= 2) {
            switch (xbuf[1]) {
            case 0x8f: // SS3
                return kbss3(buf, buflen);

          //case 0x90: // DCS
          //case 0x9d: // OSC
          //    return kbctrl(buf, buflen);

            case 0x9b: // CSI
                return kbcsi(buf, buflen);
            }
        }

    } else /*XXX - if (xf_disptype == DISPTYPE_UTF8)*/ {
        //
        //  .. 0xff - UTF-8 codepoint
        //
        return kbutf8(buf, buflen, force);
    }

    trace_log("kbproto: 0\n");
    return KEYRESULT_NONE;
}


/*  Function:           kbutf8
 *      UTF-8 keyboard protocols.
 *
 *          2byte - 110xxxxx 10xxxxxx
 *          3byte - 1110xxxx 10xxxxxx 10xxxxxx
 *          4byte - 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
 *
 *  Parameters:
 *      buf - Escape sequence buffer.
 *      buflen - Buffer length in bytes.
 *
 *  Results:
 *      Keycode, otherwise KEYRESULT_MORE or KEYRESULT_NONE.
 */
static int
kbutf8(const char *buf, unsigned buflen, int force)
{
    const unsigned char *xbuf = ((const unsigned char *)buf);
    const unsigned char c0 = xbuf[0];
    unsigned nbytes;
    int ch;

    if (c0 < 0xc2) {                            // ASCII, C1 or continuation
        // U+007F = 0x7f      <control>
        // U+0080 = 0xc2 0x80 <control>
        return KEYRESULT_NONE;
    } else if (c0 < 0xe0) {
        nbytes = 2;                             // 11000010-11011111 C2-DF start of 2-byte sequence
        ch = c0 & 0x1f;
    } else if (c0 < 0xf0) {
        nbytes = 3;                             // 11100000-11101111 E0-EF start of 3-byte sequence
        ch = c0 & 0x0f;
    } else if (c0 < 0xf8) {
        nbytes = 4;                             // 11110000-11110100 F0-F4 start of 4-byte sequence
        ch = c0 & 0x07;
    } else {
        return KEYRESULT_NONE;
    }

    if (nbytes > buflen) {
        if (force) {
            trace_log("kbproto-utf8: 0 (short)\n");
            return UNICODE_REPLACEMENT;         // REPLACEMENT CHARACTER
        }
        trace_log("kbproto-utf8: -1\n");
        return KEYRESULT_MORE;
    } else if (nbytes < buflen) {
        trace_log("kbproto-utf8: 0\n");
        return KEYRESULT_NONE;
    }

    while (--nbytes) {
        const unsigned char bx = *++xbuf;
        if ((bx & 0xC0) != 0x80) {              // 10xxxxxx
            trace_log("kbproto-utf8-bad: 0\n");
            return KEYRESULT_NONE;
        }
        ch <<= 6;
        ch |= bx & 0x3f;
    }
    trace_log("kbproto-utf8: %d\n", ch);
    return ch;
}



/*  Function:           kbcygwin
 *      Cygwin keyboard protocols.
 *
 *          <ESC>{0;1;13;28;13;01K
 *
 *  Parameters:
 *      buf - Escape sequence buffer.
 *      buflen - Buffer length in bytes.
 *
 *  Results:
 *      Keycode, otherwise KEYRESULT_MORE or KEYRESULT_NONE.
 */
static int
kbcygwin(const char *buf, unsigned buflen)
{
    const char *end = buf + (buflen - 1);
    const int isfinal = (buflen >= 4 && (*end == 'K'));
    int kcode;

    assert(buf[0] == 0x1b && buf[1] == '{');
    if (! isfinal) {
        trace_log("kbproto-cygwin: -1\n");
        return KEYRESULT_MORE;
    }

    if ((kcode = cygwin_to_int(buf)) > 0) {
        trace_log("kbproto-cygwin: %d\n", kcode);
        return kcode;
    }

    trace_log("kbproto-cygwin: 0\n");
    return KEYRESULT_NONE;
}


/*  Function:           kbss3
 *      SS3 keyboard protocol.
 *
 *  Parameters:
 *      buf - Escape sequence buffer.
 *      buflen - Buffer length in bytes.
 *
 *  Results:
 *      Keycode, otherwise KEYRESULT_MORE or KEYRESULT_NONE.
 */
static int
kbss3(const char *buf, unsigned buflen)
{
    const unsigned char *xbuf = ((const unsigned char *)buf);
    const unsigned char c0 = xbuf[0];
    unsigned modifiers = 0, mch, ch = 0;

    if (c0 == 0x1b && buflen >= 2) {
        if (xbuf[1] == 0x4f) {                  // SS3
            switch (buflen) {
            case 2:
                goto more;
            case 3:
                ch = xbuf[2];                   // SS3 <ss3-key>
                if (ch >= '1' && ch <= '8') {
                    goto more;
                }
                break;
            case 4:
                mch = xbuf[2];
                if (mch == '1' && xbuf[3] == ';') {
                    goto more;
                }
                if (mch >= '1' && mch <= '8') { // modifyOthersKeys=1
                    modifiers = xterm_modifiers(mch - '0');
                    ch = xbuf[3];               // SS3 <modifier> <ss3-key>
                }
                break;
            case 5:
                if (xbuf[2] == '1' && xbuf[3] == ';') {
                    mch = xbuf[4];
                    if (mch >= '2' && mch <= '8')
                        goto more;
                }
                break;
            case 6:
                if (xbuf[2] == '1' && xbuf[3] == ';') {
                    mch = xbuf[4];
                    if (mch >= '1' && mch <= '8') {
                        modifiers = xterm_modifiers(mch - '0');
                        ch = xbuf[5];           // SS3 1; <modifier> <ss3-key>
                    }
                }
                break;
            default:
                break;
            }
        }

    } else if (c0 == 0x8f) {                    // SS3 <ss3-key>
        if (buflen <= 2) {
            if (buflen == 1)
                goto more;
            ch = xbuf[1];
        }
    }

    if (ch) {
        const KEY k = ss3_to_key(ch);
        if (k) {
            const int kcode = (int)(modifiers | k);
            trace_log("kbproto-ss3: %d\n", kcode);
            return kcode;
        }
    }

    trace_log("kbproto-ss3: 0\n");
    return KEYRESULT_NONE;

more:;
    trace_log("kbproto-ss3: -1\n");
    return KEYRESULT_MORE;
}


static KEY
ss3_to_key(unsigned char ch)
{
    if (' ' == ch)                              // SS3 Sp
        return ' ';
    if (ch >= SS3_MIN && ch <= SS3_MAX) {
        return ss3_map[ch - SS3_MIN];           // SS3 Key
    }
    return 0;
}


/*  Function:           kbcsi
 *      CSI keyboard protocols.
 *
 *      Representations:
 *
 *          Style   Unmodified      Shift+          Alt+            Alt+Shift+      Ctrl+           Ctrl+Shift+     Ctrl+Alt+       Ctrl+Alt+Shift+
 *          --------------------------------------------------------------------------------------------------------------------------------------------
 *          mok2                    CSI ..;2 u      CSI ..;3 u      CSI ..;4 u      CSI ..;5 u      CSI ..;6 u      CSI ..;7 u      CSI ..;8 u
 *
 *          mok2                    CSI 27;2;.. ~   CSI 27;3;.. ~   CSI 27;4;.. ~   CSI 27;5;.. ~   CSI 27;6;.. ~   CSI 27;7;.. ~   CSI 27;8;.. ~
 *
 *          mok2/vt CSI # ~         CSI #;2 ~       CSI #;3 ~       CSI #;4 ~       CSI #;5 ~       CSI #;6 ~       CSI #;7 ~       CSI #;8 ~
 *
 *          vt      CSI 1;1 X       CSI 1;2 X       CSI 1;3 X       CSI 1;4 X       CSI 1;5 X       CSI 1;6 X       CSI 1;7 X       CSI 1;8 X
 *
 *          vt      CSI 1 X
 *
 *          vt      CSI X
 *          -------------------------------------------------------------------------------------------------------------------------------------------
 *
 *          .. = character value
 *          #  = 1..26, 28-34
 *          X  = {ABCDFHPQRS}
 *
 *  Parameters:
 *      buf - Escape sequence buffer.
 *      buflen - Buffer length in bytes.
 *
 *  Results:
 *      Keycode, otherwise KEYRESULT_MORE or KEYRESULT_NONE.
 */
static int
kbcsi(const char *buf, unsigned buflen)
{
    const char *end = buf + (buflen - 1);
    const int isfinal =                         // SGR final; (@A-Z[\]^_`a-z{\}~)
        (buflen >= 4 && (*end >= 0x40 && *end < 0x80)) ||
        (buflen == 3 && (*end >= 'A' && *end < 'Z'));
    int kcode;

    assert((buf[0] == 0x1b && buf[1] == '[') || buf[0] == 0x9b);
    if (! isfinal) {
        trace_log("kbproto-csi: -1\n");
        return KEYRESULT_MORE;
    }

    if ('_' == *end) {                          /* win32-input-mode */
#if defined(KBPROTOCOL_MSTERMINAL)
        if ((kcode = msterminal_to_int(buf)) > 0) {
            return kcode;
        }
#endif

    } else if ('~' == *end || 'u' == *end) {    /* xterm-mok2/mintty-mok2 or kitty-protocol */
#if defined(KBPROTOCOL_KITTY)
        if (xf_kbprotocol & (KBPROTOCOL_KITTY)) {
            if ((kcode = kitty_to_key(buf, buflen)) > 0) {
                return kcode;
            }
        }
#endif
        if (xf_kbprotocol & (KBPROTOCOL_MOK2)) {
            if ((kcode = mok2_to_key(buf, buflen)) > 0) {
                return kcode;
            }
        } else {
            if ((kcode = vt220_to_key(buf, buflen)) > 0) {
                return kcode;
            }
        }

    } else {                                    /* ansi/vt220+ */
        if ((kcode = vt220_to_key(buf, buflen)) > 0) {
            return kcode;
        }
    }

    trace_log("kbproto-csi: 0\n");
    return KEYRESULT_NONE;
}


/*  Function:           kbansi
 *      ANSI/VT220+ Cursor/Edit keyboard protocol.
 *
 *  Parameters:
 *      buf - Escape sequence buffer.
 *      buflen - Buffer length in bytes.
 *
 *  Results:
 *      Keycode, otherwise KEYRESULT_MORE or KEYRESULT_NONE.
 */
static int
kbansi(const char *buf, unsigned buflen)
{
    const char *end = buf + (buflen - 1);
    const int isfinal =                         // Special final; {ABCDFHPQRS}
        (buflen >= 3 && (*end >= 'A' && *end <= 'S'));
    int kcode;

    assert((buf[0] == 0x1b && buf[1] == '[') || buf[0] == 0x9b);
    if (! isfinal) {
        trace_log("kbproto-vt: -1\n");
        return KEYRESULT_MORE;
    }

    if ((kcode = vt220_to_key(buf, buflen)) > 0) {
        return kcode;
    }

    trace_log("kbproto-vt: 0\n");
    return KEYRESULT_NONE;
}


/*  Function:           mok2_to_key
 *      Decode a xterm modifyOtherKeys plus original unmodified (vt220+) definitions into our internal key-code.
 *
 *      Representation:
 *
 *          Unmodified      Shift+          Alt+            Alt+Shift+      Ctrl+           Ctrl+Shift+     Ctrl+Alt+       Ctrl+Alt+Shift+
 *          ----------------------------------------------------------------------------------------------------------------------------------
 *                          CSI ..;2 u      CSI ..;3 u      CSI ..;4 u      CSI ..;5 u      CSI ..;6 u      CSI ..;7 u      CSI ..;8 u
 *
 *                          CSI 27;2;.. ~   CSI 27;3;.. ~   CSI 27;4;.. ~   CSI 27;5;.. ~   CSI 27;6;.. ~   CSI 27;7;.. ~   CSI 27;8;.. ~
 *
 *          CSI # ~         CSI #;2 ~       CSI #;3 ~       CSI #;4 ~       CSI #;5 ~       CSI #;6 ~       CSI #;7 ~       CSI #;8 ~
 *          ----------------------------------------------------------------------------------------------------------------------------------
 *
 *          .. = character value
 *          #  = 1..26, 28-34
 *
 *  Parameters:
 *      buf - Escape sequence buffer.
 *      buflen - Buffer length in bytes.
 *
 *  Results:
 *      Key-code, otherwise -1.
 */
static int
mok2_to_key(const char *buf, unsigned buflen)
{
    unsigned args[4] = {0, 0, 0, 0}, nargs = 0;
    char params[3] = {0};

    /*
     *  xterm-mok2/mintty-mok2:
     *
     *      \e[27;<modifier>;<char>~        xterm.
     *   or \e[<char>;<modifier>u           formatOtherKeys=1 in xterm; which is the mintty default (modifyOtherKeys)
     *
     *      https://invisible-island.net/xterm/modified-keys.html
     *
     *  plus:
     *
     *      \e[<number>(;<modifier=1>) ~    Edit, cursor and function keys, xterm/vt.
     */
    if (tty_csi_parse(buf, buflen, 4, args, params, &nargs)) {
        unsigned key =
            ((params[0] == '~' && 3 == nargs && args[0] == 27) ? args[2]                    // CSI 27 ; <modifier> ; <character> ~
                : ((params[0] == 'u' && 2 == nargs) ? args[0]                               // CSI <character> ; <modifier> u
                    : ((params[0] == '~' && 2 == nargs) ? args[0]                           // CSI <character> ; <modifier> ~
                        : ((params[0] == '~' && 1 == nargs && args[0] <= VT_MAX) ? args[0]  // CSI <number> ~
                            : 0 /*other*/))));

        if (key) {
            unsigned modifiers = xterm_modifiers(nargs == 1 ? 1 : args[1]);
            int kcode;

            if (key == KEY_TAB) {               /* <Tab> */
                if (modifiers & MOD_SHIFT) {
                    key = BACK_TAB;
                } else if (modifiers & MOD_CTRL) {
                    key = CTRL_TAB;
                } else if (modifiers & MOD_META) {
                    key = ALT_TAB;
                }
                modifiers = 0;

            } else if (key == KEY_DELETE) {     /* <Delete> */
                if (modifiers & MOD_SHIFT) {
                    key = SHIFT_BACKSPACE;
                } else if (modifiers & MOD_CTRL) {
                    key = CTRL_BACKSPACE;
                } else if (modifiers & MOD_META) {
                    key = ALT_BACKSPACE;
                }
                modifiers = 0;

            } else if (modifiers && key >= ' ') {
                if (key >= 'a' && key <= 'z') {
                    key += (unsigned)('A' - 'a');
                }
                modifiers &= ~MOD_SHIFT;

            } else {                            // CSI <number> ([;<modifier>) ~
                if (key >= VT_MIN && key <= VT_MAX) {
                    if (0 == (key = vt_map[key - VT_MIN])) {
                        return -1;
                    }
                } else {
                    return -1;
                }
            }

            kcode = (int)(modifiers | key);
            trace_log("kbproto-mok2: %d\n", kcode);
            return kcode;
        }
    }
    return -1;
}


/*  Function:           vt220_to_key
 *      Decode a special-key, returning our internal representation.
 *
 *      Representation:
 *
 *          Unmodified      Shift+          Alt+            Alt+Shift+      Ctrl+           Ctrl+Shift+     Ctrl+Alt+       Ctrl+Alt+Shift+
 *          ----------------------------------------------------------------------------------------------------------------------------------
 *          CSI # ~         CSI #;2 ~       CSI #;3 ~       CSI #;4 ~       CSI #;5 ~       CSI #;6 ~       CSI #;7 ~       CSI #;8 ~
 *
 *          CSI 1;1 X (*)   CSI 1;2 X       CSI 1;3 X       CSI 1;4 X       CSI 1;5 X       CSI 1;6 X       CSI 1;7 X       CSI 1;8 X
 *
 *          CSI 1 X
 *
 *          CSI X
 *          ----------------------------------------------------------------------------------------------------------------------------------
 *
 *          # = 1..26, 28-34
 *          X = {ABCDFHPQRS}
 *
 *          (*) Terminals generally return the shorter alternative definition using "SS3 O X" or "CSI X".
 *
 *      Keys:
 *
 *          Key         Unmodified (*)      Modified
 *          -------------------------------------------------------
 *          vt_map      CSI # ~             CSI # ; <modifier> ~
 *
 *          Up          CSI A               CSI 1 ; <modifier> A
 *          Down        CSI B               CSI 1 ; <modifier> B
 *          Right       CSI C               CSI 1 ; <modifier> C
 *          Left        CSI D               CSI 1 ; <modifier> D
 *          End         CSI F               CSI 1 ; <modifier> F
 *          Home        CSI H               CSI 1 ; <modifier> H
 *
 *          F1          CSI P or CSI 1 P    CSI 1 ; <modifier> P
 *          F2          CSI Q or CSI 1 Q    CSI 1 ; <modifier> Q
 *          F3          CSI R or CSI 1 R    CSI 1 ; <modifier> R
 *          F4          CSI S or CSI 1 S    CSI 1 ; <modifier> S
 *
 *          (*) Normal mode
 *
 *  Parameters:
 *      buf - Escape sequence buffer.
 *      buflen - Buffer length in bytes.
 *
 *  Results:
 *      Key-code, otherwise -1.
 */
static int
vt220_to_key(const char *buf, unsigned buflen)
{
    unsigned args[4] = {1,0,0,0}, nargs = 0;
    char params[3] = {0};

    if (tty_csi_parse(buf, buflen, 4, args, params, &nargs)) {
        const unsigned arg0 = args[0];

        if (params[0] == '~') {
            if (arg0 >= VT_MIN && arg0 <= VT_MAX) {
                const KEY key = vt_map[arg0 - VT_MIN];

                if (key) {
                    int kcode;

                    if (2 == nargs) {           // CSI <number> ; <modifier> ~
                        kcode = (int)(xterm_modifiers(args[1]) | key);
                    } else if (1 == nargs) {    // CSI <number> ~
                        kcode = (int)key;
                    } else {
                        return -1;
                    }
                    trace_log("kbproto-xt: %d\n", kcode);
                    return kcode;
                }
            }

        } else /*if (params[0] >= 'A' && params[0] <= 'S')*/ {
            const KEY k = ss3_to_key(params[0]);

            if (k) {
                if (2 == nargs && arg0 == 1) {  // CSI 1 ; <modifier> {ABCDFHPQRS}
                    const unsigned modifiers = xterm_modifiers(args[1]);
                    const int kcode = (int)(modifiers | k);
                    trace_log("kbproto-vt: %d\n", kcode);
                    return kcode;
                }

                if (1 == nargs && arg0 == 1) {  // CSI 1 {ABCDFHPQRS}
                    const int kcode = (int)(k);
                    trace_log("kbproto-vt: %d\n", kcode);
                    return kcode;
                }

                if (0 == nargs) {               // CSI {ABCDFHPQRS}
                    const int kcode = (int)(k);
                    trace_log("kbproto-vt: %d\n", kcode);
                    return kcode;
                }
            }
        }
    }
    return -1;
}


/*  Function:           xterm_modifiers
 *      Decode a Xterm modifier value, returning our internal representation.
 *
 *      Modifier(s)                 Code        BitMask (-1)
 *      ----------------------------------------------------
 *      Shift                       2           0001    1
 *      Alt                         3           0010    2
 *      Shift + Alt                 4           ..xx    3
 *      Control                     5           0100    4
 *      Shift + Control             6           .x.x    5
 *      Alt + Control               7           .xx.    6
 *      Shift + Alt + Control       8           .xxx    7
 *      Meta                        9           1000    8
 *      Meta + Shift                10          x..x    9
 *      Meta + Alt                  11          x.x.    10
 *      Meta + Alt + Shift          12          x.xx    11
 *      Meta + Ctrl                 13          xx..    12
 *      Meta + Ctrl + Shift         14          xx.x    13
 *      Meta + Ctrl + Alt           15          xxx.    14
 *      Meta + Ctrl + Alt + Shift   16          xxxx    15
 *      ----------------------------------------------------
 *
 *  Parameters:
 *      mode - xterm modifier enumeration.
 *
 *  Results:
 *      Modifiers.
 */
static unsigned
xterm_modifiers(unsigned mods)
{
    unsigned code = mods - 1, modifiers = 0;    // -1, convert to bitmap.

    if (code & 1)
        modifiers |= MOD_SHIFT;
    if (code & 2)
        modifiers |= MOD_META; //ALT
    if (code & 4)
        modifiers |= MOD_CTRL;
    if (code & 8)
        modifiers |= MOD_META;
            //generally consumed by system, for example WINKEY/mintty.
    return modifiers;
}


/*  Function:           cygwin_to_int
 *      Decode a cygwin specific key escape sequence into our internal key-code.
 *
 *  Parameters:
 *      buf - Escape sequence buffer.
 *
 *  Results:
 *      nothing
 */
static int
cygwin_to_int(const char *buf)
{
#if defined(WIN32) || defined(__CYGWIN__)       /* cygwin-raw-mode */
    if (buf[0] == '\033' && buf[1] == '{') {
        /*
         *  \033[2000h - turn on raw keyboard mode,
         *  \033[2000l - turn off raw keyboard mode.
         *
         *  Format:
         *      <ESC>{Kd;Rc;Vk;Sc;Uc;CsK
         *
         *       Kd: the value of bKeyDown.
         *       Rc: the value of wRepeatCount.
         *       Vk: the value of wVirtualKeyCode.
         *       Sc: the value of wVirtualScanCode.
         *       Uc: the decimal value of UnicodeChar.
         *       Cs: the value of dwControlKeyState.
         *
         *  Example:
         *      <ESC>{0;1;13;28;13;0K
         *
         */
        unsigned bKeyDown, wRepeatCount;
        unsigned wVirtKeyCode, wVirtScanCode, UnicodeChar;
        unsigned dwControlKeyState;

        if (6 == sscanf(buf + 2, "%u;%u;%u;%u;%u;%uK",
                    &bKeyDown, &wRepeatCount, &wVirtKeyCode, &wVirtScanCode, &UnicodeChar, &dwControlKeyState)) {
            if (bKeyDown) {
                const int w32key =
                    key_mapwin32(dwControlKeyState, wVirtKeyCode, UnicodeChar);

                trace_log("cygkey[%s]=%d/0x%x\n", buf + 2, w32key, w32key);
                return w32key;
            }
            trace_log("cygkey[%s]=up\n", buf + 2);
            return KEY_VOID;
        }
    }
#else
    __CUNUSED(buf)
#endif
    return -1;
}


/*  Function:           msterminal_to_int
 *      Decode a MSTerminal specific key escape sequence into our internal key-code.
 *
 *  Parameters:
 *      buf - Escape sequence buffer.
 *
 *  Results:
 *      nothing
 */

enum {
    msVirtualKeyCode,
    msVirtualScanCode,
    msUnicodeChar,
    msKeyDown,
    msControlKeyState,
    msRepeatCount,
    msgArgumentMax
};


static int
msterminal_args(const char *buffer, unsigned arguments[])
{
    const char *cursor = buffer + 2, *value = NULL;
    unsigned args = 0;

    while (*cursor) {
        const unsigned char c = *cursor++;

        if (c >= '0' && c <= '9') {
            if (NULL == value) {
                value = cursor - 1;             // value
            }

        } else if (c == ';' || c == '_') {
            if (args >= msgArgumentMax) {
                args = 0;                       // overflow
                break;
            }

            if (value) {
                arguments[args] = (unsigned)strtoul((const char *)value, NULL, 10);
                value = NULL;
            }
            ++args;
            if (c == '_') {
                break;                          // terminator
            }

        } else {
            args = 0;                           // non-digit
            break;
        }
    }
    return (args >= 1);
}

static int
msterminal_to_int(const char *buf)
{
#if defined(WIN32) || defined(__CYGWIN__)       /* win32-input-mode */
    if (buf[0] == '\033' && buf[1] == '[') {
       /*
        *   Terminal win32-input-mode
        *
        *       <ESC>[17;29;0;1;8;1_
        *
        *   Format:
        *
        *       <ESC>[Vk;Sc;Uc;Kd;Cs;Rc_
        *
        *       Vk: the value of wVirtualKeyCode - any number. If omitted, defaults to '0'.
        *       Sc: the value of wVirtualScanCode - any number. If omitted, defaults to '0'.
        *       Uc: the decimal value of UnicodeChar - for example, NUL is "0", LF is
        *           "10", the character 'A' is "65". If omitted, defaults to '0'.
        *       Kd: the value of bKeyDown - either a '0' or '1'. If omitted, defaults to '0'.
        *       Cs: the value of dwControlKeyState - any number. If omitted, defaults to '0'.
        *       Rc: the value of wRepeatCount - any number. If omitted, defaults to '1'.
        *
        *   Reference
        *       https://github.com/microsoft/terminal/blob/main/doc/specs/%234999%20-%20Improved%20keyboard%20handling%20in%20Conpty.md
        */
        unsigned args[msgArgumentMax] = { 0, 0, 0, 0, 0, 1 };

        if (msterminal_args(buf, args)) {
            if (args[msKeyDown]) {
                const int w32key =
                    key_mapwin32(args[msControlKeyState], args[msVirtualKeyCode], args[msUnicodeChar]);

                trace_log("winkey[%s]=0x%x/%u\n", buf + 2, w32key, w32key);
                return w32key;
            }
            trace_log("winkey[%s]=up\n", buf + 2);
            return KEY_VOID;
        }
        trace_log("winkey[%s]=na\n", buf + 2);
    }
#else
    __CUNUSED(buf)
#endif
    return -1;
}

#else   //USE_KBPROTOCOL

int
kbprotocols_parse(const char *buf, const unsigned buflen, int force)
{
    __CUNUSED(buf)
    __CUNUSED(buflen)
    __CUNUSED(force)

    return KEYRESULT_NONE;
}

#endif  //USE_KBPROTOCOL

/*end*/

