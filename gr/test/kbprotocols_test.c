#include <edidentifier.h>
__CIDENT_RCSID(gr_kbprotocols_test_c,"$Id: kbprotocols_test.c,v 1.8 2024/11/29 15:59:33 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: kbprotocols_test.c,v 1.8 2024/11/29 15:59:33 cvsuser Exp $
 *
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
#include <edalt.h>
#include <libstr.h>
#include <libtrie.h>

#include "kbname.h"
#include "kbprotocols.h"
#include "kbsequence.h"

#if defined(NDEBUG)
#undef NDEBUG
#endif

#include <assert.h>

#define F1_F12                  F(1), "F1-F12"
#define SHIFT_F1_F12            SF(1), "SHIFT-F1-F12"
#define CTRL_F1_F12             CF(1), "CTRL-F1-F12"
#define CTRLSHIFT_F1_F12        CSF(1), "CTRLSHIFT-F1-F12"
#define ALT_F1_F12              AF(1), "ALT-F1-F12"
#define ALTSHIFT_F1_F12         ASF(1), "ALT-SHIFT_F1-F12"
#define ALTCTRL_F1_F12          ACF(1), "ALT-CTRL-F1-F12"
#define ALTCTRLSHIFT_F1_F12     ACSF(1), "ALT-CTRL-SHIFT-F1-F12"

#define KEYPAD_0_9              KEYPAD_0, "KEYPAD"
#define SHIFT_KEYPAD_0_9        SHIFT_KEYPAD_0, "SHIFT-KEYPAD"
#define CTRL_KEYPAD_0_9         CTRL_KEYPAD_0, "CTRL-KEYPAD"
#define CTRLSHIFT_KEYPAD_0_9    CTRLSHIFT_KEYPAD_0, "CTRL-SHIFT-KEYPAD"
#define ALT_KEYPAD_0_9          ALT_KEYPAD_0, "ALT-KEYPAD"
#define ALTSHIFT_KEYPAD_0_9     ALTSHIFT_KEYPAD_0, "ALT-SHIFT_KEYPAD"
#define ALTCTRL_KEYPAD_0_9      ALTCTRL_KEYPAD_0, "ALT-CTRL-KEYPAD"
#define ALTCTRLSHIFT_KEYPAD_0_9 ALTCTRLSHIFT_KEYPAD_0, "ALT-CTRL-SHIFT-KEYPAD"

#define CTRL_A_Z                CTRL_A
#define ALT_A_Z                 ALT_A, "ALT-ALPHA"
#define ALT_0_9                 ALT_0, "ALT-NUMERIC"

static const struct {
    int         keys;
    const char  *desc;
#define NSEQS                   36
    const char  *seqs[NSEQS];
    const char  *ctx;

} keys[] = {
        /// Function keys:
        //
        //      F1,                 F2,                 F3,                 F4,                 F5,
        //      F6,                 F7,                 F8,                 F9,                 F10,
        //      F11,                F12
        //

#if (DO_UNSUPPORTED)
        // --- vt100
        {  F1_F12, {                // SS3 <vt100-key>
                "\x1bOP",           "\x1bOQ",           "\x1bOR",           "\x1bOS",           "\x1bOt",
                "\x1bOu",           "\x1bOv",           "\x1bOl",           "\x1bOw",           "\x1bOx",
                "\x1b[23~",         "\x1b[24~"
                }, "vt100-function"
        },
#endif

        // --- vt220
                // CSI = Cursor Key Mode Reset (Normal)
                // SS3 = Cursor Key Mode Set (Application)

                // SS3 (<modifier>;) <sse3-key:{ABCDFHPQRS}>
                //
        {   F1_F12, {               // unshifted, optional modifier
                "\x1bOP",           "\x1bOQ",           "\x1bOR",           "\x1bOS",           NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }, "vt220-function"
        },
        {   F1_F12, {               // unshifted, modifier=1
                "\x1bO1;1P",        "\x1bO1;1Q",        "\x1bO1;1R",        "\x1bO1;1S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   F1_F12, {               // unshifted, modifier=1
                "\x1bO1P",          "\x1bO1Q",          "\x1bO1R",          "\x1bO1S",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   SHIFT_F1_F12, {
                "\x1bO1;2P",        "\x1bO1;2Q",        "\x1bO1;2R",        "\x1bO1;2S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   SHIFT_F1_F12, {
                "\x1bO2P",          "\x1bO2Q",          "\x1bO2R",          "\x1bO2S",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALT_F1_F12, {
                "\x1bO1;3P",        "\x1bO1;3Q",        "\x1bO1;3R",        "\x1bO1;3S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALT_F1_F12, {
                "\x1bO3P",          "\x1bO3Q",          "\x1bO3R",          "\x1bO3S",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALTSHIFT_F1_F12, {
                "\x1bO1;4P",        "\x1bO1;4Q",        "\x1bO1;4R",        "\x1bO1;4S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALTSHIFT_F1_F12, {
                "\x1bO4P",          "\x1bO4Q",          "\x1bO4R",          "\x1bO4S",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   CTRL_F1_F12, {
                "\x1bO1;5P",        "\x1bO1;5Q",        "\x1bO1;5R",        "\x1bO1;5S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   CTRL_F1_F12, {
                "\x1bO5P",          "\x1bO5Q",          "\x1bO5R",          "\x1bO5S",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   CTRLSHIFT_F1_F12, {
                "\x1bO1;6P",        "\x1bO1;6Q",        "\x1bO1;6R",        "\x1bO1;6S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   CTRLSHIFT_F1_F12, {
                "\x1bO6P",          "\x1bO6Q",          "\x1bO6R",          "\x1bO6S",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALTCTRL_F1_F12, {
                "\x1bO1;7P",        "\x1bO1;7Q",        "\x1bO1;7R",        "\x1bO1;7S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALTCTRL_F1_F12, {
                "\x1bO7P",          "\x1bO7Q",          "\x1bO7R",          "\x1bO7S",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALTCTRLSHIFT_F1_F12, {
                "\x1bO1;8P",        "\x1bO1;8Q",        "\x1bO1;8R",        "\x1bO1;8S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALTCTRLSHIFT_F1_F12, {
                "\x1bO8P",          "\x1bO8Q",          "\x1bO8R",          "\x1bO8S",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },

                // CSI (<modifier>;) <sse3-key:{ABCDFHPQRS}>
        {   F1_F12, {               // unshifted, optional modifier
                "\x1b[P",           "\x1b[Q",           "\x1b[R",           "\x1b[S",           NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   F1_F12, {               // unshifted, modifier=1
                "\x1b[1;1P",        "\x1b[1;1Q",        "\x1b[1;1R",        "\x1b[1;1S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   SHIFT_F1_F12, {         // shifted
                "\x1b[1;2P",        "\x1b[1;2Q",        "\x1b[1;2R",        "\x1b[1;2S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALT_F1_F12, {
                "\x1b[1;3P",        "\x1b[1;3Q",        "\x1b[1;3R",        "\x1b[1;3S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALTSHIFT_F1_F12, {
                "\x1bO1;4P",        "\x1bO1;4Q",        "\x1bO1;4R",        "\x1bO1;4S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   CTRL_F1_F12, {
                "\x1bO1;5P",        "\x1bO1;5Q",        "\x1bO1;5R",        "\x1bO1;5S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   CTRLSHIFT_F1_F12, {
                "\x1b[1;6P",        "\x1b[1;6Q",        "\x1b[1;6R",        "\x1b[1;6S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALTCTRL_F1_F12, {
                "\x1b[1;7P",        "\x1b[1;7Q",        "\x1b[1;7R",        "\x1b[1;7S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
                }
        },
        {   ALTCTRLSHIFT_F1_F12, {
                "\x1b[1;8P",        "\x1b[1;8Q",        "\x1b[1;8R",        "\x1b[1;8S",        NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL
            }
        },

        // --- xterm

                // CSI <number:11-24> ~
        {   F1_F12, {               // unshifted; optional modifier
                "\x1b[11~",         "\x1b[12~",         "\x1b[13~",         "\x1b[14~",         "\x1b[15~",
                "\x1b[17~",         "\x1b[18~",         "\x1b[19~",         "\x1b[20~",         "\x1b[21~",
                "\x1b[23~",         "\x1b[24~"
                }, "xterm-function"
        },

                // CSI <number:25-34> ~
        {   SHIFT_F1_F12, {         // shifted; alternative number
                "\x1b[25~",         "\x1b[26~",         "\x1b[28~",         "\x1b[29~",         "\x1b[31~",
                "\x1b[32~",         "\x1b[33~",         "\x1b[34~",         NULL,               NULL,
                NULL,               NULL
                },
        },

                // CSI <number> (;<modifier>) ~
        {   F1_F12, {               // unshifted, modifier=1
                "\x1b[27;1;11~",    "\x1b[27;1;12~",    "\x1b[27;1;13~",    "\x1b[27;1;14~",    "\x1b[27;1;15~",
                "\x1b[27;1;17~",    "\x1b[27;1;18~",    "\x1b[27;1;19~",    "\x1b[27;1;20~",    "\x1b[27;1;21~",
                "\x1b[27;1;23~",    "\x1b[27;1;24~"
                }
        },
        {   F1_F12, {               // unshifted, modifier=1
                "\x1b[11;1~",       "\x1b[12;1~",       "\x1b[13;1~",       "\x1b[14;1~",       "\x1b[15;1~",
                "\x1b[17;1~",       "\x1b[18;1~",       "\x1b[19;1~",       "\x1b[20;1~",       "\x1b[21;1~",
                "\x1b[23;1~",       "\x1b[24;1~"
                }
        },
        {   F1_F12, {               // unshifted, modifier=1
                "\x1b[11;1u",       "\x1b[12;1u",       "\x1b[13;1u",       "\x1b[14;1u",       "\x1b[15;1u",
                "\x1b[17;1u",       "\x1b[18;1u",       "\x1b[19;1u",       "\x1b[20;1u",       "\x1b[21;1u",
                "\x1b[23;1u",       "\x1b[24;1u"
                }
        },
        {   SHIFT_F1_F12, {         // shifted, modifier
                "\x1b[27;2;11~",    "\x1b[27;2;12~",    "\x1b[27;2;13~",    "\x1b[27;2;14~",    "\x1b[27;2;15~",
                "\x1b[27;2;17~",    "\x1b[27;2;18~",    "\x1b[27;2;19~",    "\x1b[27;2;20~",    "\x1b[27;2;21~",
                "\x1b[27;2;23~",    "\x1b[27;2;24~"
                }
        },
        {   SHIFT_F1_F12, {         // shifted, modifier
                "\x1b[11;2~",       "\x1b[12;2~",       "\x1b[13;2~",       "\x1b[14;2~",       "\x1b[15;2~",
                "\x1b[17;2~",       "\x1b[18;2~",       "\x1b[19;2~",       "\x1b[20;2~",       "\x1b[21;2~",
                "\x1b[23;2~",       "\x1b[24;2~"
                }
        },
        {   SHIFT_F1_F12, {
                "\x1b[11;2u",       "\x1b[12;2u",       "\x1b[13;2u",       "\x1b[14;2u",       "\x1b[15;2u",
                "\x1b[17;2u",       "\x1b[18;2u",       "\x1b[19;2u",       "\x1b[20;2u",       "\x1b[21;2u",
                "\x1b[23;2u",       "\x1b[24;2u"
                }
        },
        {   ALT_F1_F12, {
                "\x1b[27;3;11~",    "\x1b[27;3;12~",    "\x1b[27;3;13~",    "\x1b[27;3;14~",    "\x1b[27;3;15~",
                "\x1b[27;3;17~",    "\x1b[27;3;18~",    "\x1b[27;3;19~",    "\x1b[27;3;20~",    "\x1b[27;3;21~",
                "\x1b[27;3;23~",    "\x1b[27;3;24~"
                }
        },
        {   ALT_F1_F12, {
                "\x1b[11;3~",       "\x1b[12;3~",       "\x1b[13;3~",       "\x1b[14;3~",       "\x1b[15;3~",
                "\x1b[17;3~",       "\x1b[18;3~",       "\x1b[19;3~",       "\x1b[20;3~",       "\x1b[21;3~",
                "\x1b[23;3~",       "\x1b[24;3~"
                }
        },
        {   ALT_F1_F12, {
                "\x1b[11;3u",       "\x1b[12;3u",       "\x1b[13;3u",       "\x1b[14;3u",       "\x1b[15;3u",
                "\x1b[17;3u",       "\x1b[18;3u",       "\x1b[19;3u",       "\x1b[20;3u",       "\x1b[21;3u",
                "\x1b[23;3u",       "\x1b[24;3u"
                }
        },
        {   ALTSHIFT_F1_F12, {
                "\x1b[27;4;11~",    "\x1b[27;4;12~",    "\x1b[27;4;13~",    "\x1b[27;4;14~",    "\x1b[27;4;15~",
                "\x1b[27;4;17~",    "\x1b[27;4;18~",    "\x1b[27;4;19~",    "\x1b[27;4;20~",    "\x1b[27;4;21~",
                "\x1b[27;4;23~",    "\x1b[27;4;24~"
                }
        },
        {   ALTSHIFT_F1_F12, {
                "\x1b[11;4~",       "\x1b[12;4~",       "\x1b[13;4~",       "\x1b[14;4~",       "\x1b[15;4~",
                "\x1b[17;4~",       "\x1b[18;4~",       "\x1b[19;4~",       "\x1b[20;4~",       "\x1b[21;4~",
                "\x1b[23;4~",       "\x1b[24;4~"
                }
        },
        {   ALTSHIFT_F1_F12, {
                "\x1b[11;4u",       "\x1b[12;4u",       "\x1b[13;4u",       "\x1b[14;4u",       "\x1b[15;4u",
                "\x1b[17;4u",       "\x1b[18;4u",       "\x1b[19;4u",       "\x1b[20;4u",       "\x1b[21;4u",
                "\x1b[23;4u",       "\x1b[24;4u"
                }
        },
#if (DO_UNSUPPORTED)
        {   ALTSHIFT_F1_F12, {
                "\x1b\x1b[1;4P",    "\x1b\x1b[1;4Q",    "\x1b\x1b[1;4R",    "\x1b\x1b[1;4S",    "\x1b\x1b[15;4~",
                "\x1b\x1b[17;4~",   "\x1b\x1b[18;4~",   "\x1b\x1b[19;4~",   "\x1b\x1b[20;4~",   "\x1b\x1b[21;4~",
                "\x1b\x1b[23;4~",   "\x1b\x1b[24;4~"
        }
#endif
        {   CTRL_F1_F12, {
                "\x1b[27;5;11~",    "\x1b[27;5;12~",    "\x1b[27;5;13~",    "\x1b[27;5;14~",    "\x1b[27;5;15~",
                "\x1b[27;5;17~",    "\x1b[27;5;18~",    "\x1b[27;5;19~",    "\x1b[27;5;20~",    "\x1b[27;5;21~",
                "\x1b[27;5;23~",    "\x1b[27;5;24~"
                }
        },
        {   CTRL_F1_F12, {
                "\x1b[11;5~",       "\x1b[12;5~",       "\x1b[13;5~",       "\x1b[14;5~",       "\x1b[15;5~",
                "\x1b[17;5~",       "\x1b[18;5~",       "\x1b[19;5~",       "\x1b[20;5~",       "\x1b[21;5~",
                "\x1b[23;5~",       "\x1b[24;5~"
                }
        },
        {   CTRL_F1_F12, {
                "\x1b[11;5u",       "\x1b[12;5u",       "\x1b[13;5u",       "\x1b[14;5u",       "\x1b[15;5u",
                "\x1b[17;5u",       "\x1b[18;5u",       "\x1b[19;5u",       "\x1b[20;5u",       "\x1b[21;5u",
                "\x1b[23;5u",       "\x1b[24;5u"
                }
        },
        {   CTRLSHIFT_F1_F12, {
                "\x1b[27;6;11~",    "\x1b[27;6;12~",    "\x1b[27;6;13~",    "\x1b[27;6;14~",    "\x1b[27;6;15~",
                "\x1b[27;6;17~",    "\x1b[27;6;18~",    "\x1b[27;6;19~",    "\x1b[27;6;20~",    "\x1b[27;6;21~",
                "\x1b[27;6;23~",    "\x1b[27;6;24~"
                }
        },
        {   CTRLSHIFT_F1_F12, {
                "\x1b[11;6~",       "\x1b[12;6~",       "\x1b[13;6~",       "\x1b[14;6~",       "\x1b[15;6~",
                "\x1b[17;6~",       "\x1b[18;6~",       "\x1b[19;6~",       "\x1b[20;6~",       "\x1b[21;6~",
                "\x1b[23;6~",       "\x1b[24;6~"
                }
        },
        {   CTRLSHIFT_F1_F12, {
                "\x1b[11;6u",       "\x1b[12;6u",       "\x1b[13;6u",       "\x1b[14;6u",       "\x1b[15;6u",
                "\x1b[17;6u",       "\x1b[18;6u",       "\x1b[19;6u",       "\x1b[20;6u",       "\x1b[21;6u",
                "\x1b[23;6u",       "\x1b[24;6u"
                }
        },
        {   ALTCTRL_F1_F12, {
                "\x1b[27;7;11~",    "\x1b[27;7;12~",    "\x1b[27;7;13~",    "\x1b[27;7;14~",    "\x1b[27;7;15~",
                "\x1b[27;7;17~",    "\x1b[27;7;18~",    "\x1b[27;7;19~",    "\x1b[27;7;20~",    "\x1b[27;7;21~",
                "\x1b[27;7;23~",    "\x1b[27;7;24~"
                }
        },
        {   ALTCTRL_F1_F12, {
                "\x1b[11;7~",       "\x1b[12;7~",       "\x1b[13;7~",       "\x1b[14;7~",       "\x1b[15;7~",
                "\x1b[17;7~",       "\x1b[18;7~",       "\x1b[19;7~",       "\x1b[20;7~",       "\x1b[21;7~",
                "\x1b[23;7~",       "\x1b[24;7~"
                }
        },
        {   ALTCTRL_F1_F12, {
                "\x1b[11;7u",       "\x1b[12;7u",       "\x1b[13;7u",       "\x1b[14;7u",       "\x1b[15;7u",
                "\x1b[17;7u",       "\x1b[18;7u",       "\x1b[19;7u",       "\x1b[20;7u",       "\x1b[21;7u",
                "\x1b[23;7u",       "\x1b[24;7u"
                }
        },
        {   ALTCTRLSHIFT_F1_F12, {
                "\x1b[27;8;11~",    "\x1b[27;8;12~",    "\x1b[27;8;13~",    "\x1b[27;8;14~",    "\x1b[27;8;15~",
                "\x1b[27;8;17~",    "\x1b[27;8;18~",    "\x1b[27;8;19~",    "\x1b[27;8;20~",    "\x1b[27;8;21~",
                "\x1b[27;8;23~",    "\x1b[27;8;24~"
            }
        },
        {   ALTCTRLSHIFT_F1_F12, {
                "\x1b[11;8~",       "\x1b[12;8~",       "\x1b[13;8~",       "\x1b[14;8~",       "\x1b[15;8~",
                "\x1b[17;8~",       "\x1b[18;8~",       "\x1b[19;8~",       "\x1b[20;8~",       "\x1b[21;8~",
                "\x1b[23;8~",       "\x1b[24;8~"
            }
        },
        {   ALTCTRLSHIFT_F1_F12, {
                "\x1b[11;8u",       "\x1b[12;8u",       "\x1b[13;8u",       "\x1b[14;8u",       "\x1b[15;8u",
                "\x1b[17;8u",       "\x1b[18;8u",       "\x1b[19;8u",       "\x1b[20;8u",       "\x1b[21;8u",
                "\x1b[23;8u",       "\x1b[24;8u"
            }
        },

#if (DO_UNSUPPORTED)
        // --- sun
        {   F1_F12, {
                "\x1b[192z",        "\x1b[193z",        "\x1b[194z",        "\x1b[195z",        "\x1b[196z",
                "\x1b[197z",        "\x1b[198z",        "\x1b[199z",        "\x1b[200z",        "\x1b[201z",
                "\x1b[234z",        "\x1b[235z"
            }, "sun-function"
        },
#endif

        /// Keypad:
        //
        //      Ins/0               End/1               Down/2              PgDn/3              Left/4
        //      5                   Right/6             Home/7              Up/8                PgUp/9
        //      Del/.               Plus                Minus               Star                Divide
        //      Equals              Enter               Pause               PrtSc               Scroll
        //      NumLock
        //

                // SSE3 <number>
        {   KEYPAD_0_9, {           // vt220
                "\x1bOp",           "\x1bOq",           "\x1bOr",           "\x1bOs",           "\x1bOt",
                "\x1bOu",           "\x1bOv",           "\x1bOw",           "\x1bOx",           "\x1bOy",
                "\x1bOn",           "\x1bOk",           "\x1bOm",           "\x1bOj",           "\x1bOo",
                "\x1bOX",           "\x1bOM",           NULL,               NULL,               NULL,
                NULL
            }, "keypad"
        },

                // SS3 {ABCDFHPQRS} {MX}
        {   KEYPAD_0_9, {
                NULL,               "\x1bOF",           "\x1bOB",           NULL,               "\x1bOD",
                NULL,               "\x1bOC",           "\x1bOH",           "\x1bOA",           NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                "\x1bOX",           "\x1bOM",           NULL,               NULL,               NULL,
                NULL
            }
        },

                // SS3 <modifier> <ss3-key>
        {   KEYPAD_0_9, {
                NULL,               "\x1bO1F",          "\x1bO1B",          NULL,               "\x1bO1D",
                NULL,               "\x1bO1C",          "\x1bO1H",          "\x1bO1A",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                "\x1bO1X",          "\x1bO1M",          NULL,               NULL,               NULL,
                NULL
            }
        },
        {   SHIFT_KEYPAD_0_9, {
                NULL,               "\x1bO2F",          "\x1bO2B",          NULL,               "\x1bO2D",
                NULL,               "\x1bO2C",          "\x1bO2H",          "\x1bO2A",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                "\x1bO2X",          "\x1bO2M",          NULL,               NULL,               NULL,
                NULL
            }
        },
        {   ALT_KEYPAD_0_9, {
                NULL,               "\x1bO3F",          "\x1bO3B",          NULL,               "\x1bO3D",
                NULL,               "\x1bO3C",          "\x1bO3H",          "\x1bO3A",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                "\x1bO3X",          "\x1bO3M",          NULL,               NULL,               NULL,
                NULL
            }
        },
        {   ALTSHIFT_KEYPAD_0_9, {
                NULL,               "\x1bO4F",          "\x1bO4B",          NULL,               "\x1bO4D",
                NULL,               "\x1bO4C",          "\x1bO4H",          "\x1bO4A",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                "\x1bO4X",          "\x1bO4M",          NULL,               NULL,               NULL,
                NULL
            }
        },
        {   CTRL_KEYPAD_0_9, {
                NULL,               "\x1bO5F",          "\x1bO5B",          NULL,               "\x1bO5D",
                NULL,               "\x1bO5C",          "\x1bO5H",          "\x1bO5A",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                "\x1bO5X",          "\x1bO5M",          NULL,               NULL,               NULL,
                NULL
            }
        },
        {   CTRLSHIFT_KEYPAD_0_9, {
                NULL,               "\x1bO6F",          "\x1bO6B",          NULL,               "\x1bO6D",
                NULL,               "\x1bO6C",          "\x1bO6H",          "\x1bO6A",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                "\x1bO6X",          "\x1bO6M",          NULL,               NULL,               NULL,
                NULL
            }
        },
        {   ALTCTRL_KEYPAD_0_9, {
                NULL,               "\x1bO7F",          "\x1bO7B",          NULL,               "\x1bO7D",
                NULL,               "\x1bO7C",          "\x1bO7H",          "\x1bO7A",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                "\x1bO7X",          "\x1bO7M",          NULL,               NULL,               NULL,
                NULL
            }
        },
        {   ALTCTRLSHIFT_KEYPAD_0_9, {
                NULL,               "\x1bO8F",          "\x1bO8B",          NULL,               "\x1bO8D",
                NULL,               "\x1bO8C",          "\x1bO8H",          "\x1bO8A",          NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                "\x1bO8X",          "\x1bO8M",          NULL,               NULL,               NULL,
                NULL
            }
        },


                // CSI <number> (;<modifier>) ~
                // CSI (1;<modifier>) {ABCDFHPQRS} {MX}
        {   KEYPAD_0_9, {           // unshifted; optional modifier
                "\x1b[2~",          "\x1b[F",           "\x1b[B",           "\x1b[6~",          "\x1b[D",
                NULL,               "\x1b[C",           "\x1b[H",           "\x1b[A",           "\x1b[5~",
                "\x1b[3~",          NULL,               NULL,               NULL,               NULL,
                "\x1b[X",           "\x1b[M",           NULL,               NULL,               NULL,
                NULL
            }
        },
        {   KEYPAD_0_9, {           // shifted
                "\x1b[2;1~",        "\x1b[1;1F",        "\x1b[1;1B",        "\x1b[6;1~",        "\x1b[1;1D",
                NULL,               "\x1b[1;1C",        "\x1b[1;1H",        "\x1b[1;1A",        "\x1b[5;1~",
                "\x1b[3;1~",        NULL,               NULL,               "\x1b[1;1j",        NULL,
                "\x1b[1;1X",        "\x1b[1;1M",        NULL,               NULL,               NULL,
                NULL
            }
        },
        {   SHIFT_KEYPAD_0_9, {
                "\x1b[2;2~",        "\x1b[1;2F",        "\x1b[1;2B",        "\x1b[6;2~",        "\x1b[1;2D",
                NULL,               "\x1b[1;2C",        "\x1b[1;2H",        "\x1b[1;2A",        "\x1b[5;2~",
                "\x1b[3;2~",        NULL,               NULL,               "\x1b[1;2j",        NULL,
                "\x1b[1;2X",        "\x1b[1;2M",        NULL,               NULL,               NULL,
                NULL
            }
        },
        {   ALT_KEYPAD_0_9, {
                "\x1b[2;3~",        "\x1b[1;3F",        "\x1b[1;3B",        "\x1b[6;3~",        "\x1b[1;3D",
                NULL,               "\x1b[1;3C",        "\x1b[1;3H",        "\x1b[1;3A",        "\x1b[5;3~",
                "\x1b[3;3~",        NULL,               NULL,               "\x1b[1;3j",        NULL,
                "\x1b[1;3X",        "\x1b[1;3M",        NULL,               NULL,               NULL,
                NULL
            }
        },
        {   ALTSHIFT_KEYPAD_0_9, {
                "\x1b[2;4~",        "\x1b[1;4F",        "\x1b[1;4B",        "\x1b[6;4~",        "\x1b[1;4D",
                NULL,               "\x1b[1;4C",        "\x1b[1;4H",        "\x1b[1;4A",        "\x1b[5;4~",
                "\x1b[3;4~",        NULL,               NULL,               "\x1b[1;4j",        NULL,
                "\x1b[1;4X",        "\x1b[1;4M",        NULL,               NULL,               NULL,
                NULL
            }
        },
        {   CTRL_KEYPAD_0_9, {
                "\x1b[2;5~",        "\x1b[1;5F",        "\x1b[1;5B",        "\x1b[6;5~",        "\x1b[1;5D",
                NULL,               "\x1b[1;5C",        "\x1b[1;5H",        "\x1b[1;5A",        "\x1b[5;5~",
                "\x1b[3;5~",        NULL,               NULL,               "\x1b[1;5j",        NULL,
                "\x1b[1;5X",        "\x1b[1;5M",        NULL,               NULL,               NULL,
                NULL
            }
        },
        {   CTRLSHIFT_KEYPAD_0_9, {
                "\x1b[2;6~",        "\x1b[1;6F",        "\x1b[1;6B",        "\x1b[6;6~",        "\x1b[1;6D",
                NULL,               "\x1b[1;6C",        "\x1b[1;6H",        "\x1b[1;6A",        "\x1b[5;6~",
                "\x1b[3;6~",        NULL,               NULL,               "\x1b[1;6j",        NULL,
                "\x1b[1;6X",        "\x1b[1;6M",        NULL,               NULL,               NULL,
                NULL
            }
        },
        {   ALTCTRL_KEYPAD_0_9, {
                "\x1b[2;7~",        "\x1b[1;7F",        "\x1b[1;7B",        "\x1b[6;7~",        "\x1b[1;7D",
                NULL,               "\x1b[1;7C",        "\x1b[1;7H",        "\x1b[1;7A",        "\x1b[5;7~",
                "\x1b[3;7~",        NULL,               NULL,               "\x1b[1;7j",        NULL,
                "\x1b[1;7X",        "\x1b[1;7M",        NULL,               NULL,               NULL,
                NULL
            }
        },
        {   ALTCTRLSHIFT_KEYPAD_0_9, {
                "\x1b[2;8~",        "\x1b[1;8F",        "\x1b[1;8B",        "\x1b[6;8~",        "\x1b[1;8D",
                NULL,               "\x1b[1;8C",        "\x1b[1;8H",        "\x1b[1;8A",        "\x1b[5;8~",
                "\x1b[3;8~",        NULL,               NULL,               "\x1b[1;8j",        NULL,
                "\x1b[1;8X",        "\x1b[1;8M",        NULL,               NULL,               NULL,
                NULL
            }
        },

        /// Meta

        {   ALT_A_Z, {              /* ESC <lower-case> */
                "\x1b""a",          "\x1b""b",          "\x1b""c",          "\x1b""d",          "\x1b""e",
                "\x1b""f",          "\x1b""g",          "\x1b""h",          "\x1b""i",          "\x1b""j",
                "\x1b""k",          "\x1b""l",          "\x1b""m",          "\x1b""n",          "\x1b""o",
                "\x1b""p",          "\x1b""q",          "\x1b""r",          "\x1b""s",          "\x1b""t",
                "\x1b""u",          "\x1b""v",          "\x1b""w",          "\x1b""x",          "\x1b""y",
                "\x1b""z"
            }, "meta"
        },
        {   ALT_A_Z, {              /* ESC <upper-case> */
                "\x1b""A",          "\x1b""B",          "\x1b""C",          "\x1b""D",          "\x1b""E",
                "\x1b""F",          "\x1b""G",          "\x1b""H",          "\x1b""I",          "\x1b""J",
                "\x1b""K",          "\x1b""L",          "\x1b""M",          "\x1b""N",          "\x1b""O",
                "\x1b""P",          "\x1b""Q",          "\x1b""R",          "\x1b""S",          "\x1b""T",
                "\x1b""U",          "\x1b""V",          "\x1b""W",          "\x1b""X",          "\x1b""Y",
                "\x1b""Z"
            }
        },
        {   ALT_0_9, {              /* ESC <numeric> */
                "\x1b""0",          "\x1b""1",          "\x1b""2",          "\x1b""3",          "\x1b""4",
                "\x1b""5",          "\x1b""6",          "\x1b""7",          "\x1b""8",          "\x1b""9"
            }
        },
        {   __ALT(0x20), "0x20-0x2f", {         // SP !"#$%&'()*+,-./
                "\x1b\x20",         "\x1b\x21",         "\x1b\x22",         "\x1b\x23",         "\x1b\x24",
                "\x1b\x25",         "\x1b\x26",         "\x1b\x27",         "\x1b\x28",         "\x1b\x29",
                "\x1b\x2A",         "\x1b\x2B",         "\x1b\x2C",         "\x1b\x2D",         "\x1b\x2E",
                "\x1b\x2F"
            }
        },
        {   __ALT(0x3A), "0x3A-0x40", {         // :;<=>?@
                "\x1b\x3A",         "\x1b\x3B",         "\x1b\x3C",         "\x1b\x3D",         "\x1b\x3E",
                "\x1b\x3F",         "\x1b\x40"
            }
        },
        {   __ALT(0x5b), "0x5b-0x60", {         // [\]^_`
                "\x1b\x5B",         "\x1b\x5C",         "\x1b\x5D",         "\x1b\x5E",         "\x1b\x5F",
                "\x1b\x60"
            }
        },
        {   __ALT(0x7b), "0x7b-0x7e", {         // {|}~
                "\x1b\x7B",         "\x1b\x7C",         "\x1b\x7D",         "\x1b\x7E"
            }
        },

#if (DO_UNSUPPORTED)
        {   ALT_F1_F12, {                       // ESC CSI <number> ~
                "\x1b\x1b[11~",     "\x1b\x1b[12~",     "\x1b\x1b[13~",     "\x1b\x1b[14~",     "\x1b\x1b[15~",
                "\x1b\x1b[17~",     "\x1b\x1b[18~",     "\x1b\x1b[19~",     "\x1b\x1b[20~",     "\x1b\x1b[21~",
                "\x1b\x1b[23~",     "\x1b\x1b[24~"
                }
        },
        {   ALT_KEYPAD_0_9, {                   // SS3 CSI {ABCD} ~
                NULL,               NULL,               "\x1b\x1bOB",       NULL,               "\x1b\x1bOD",
                NULL,               "\x1b\x1bOC",       NULL,               "\x1b\x1bOA",       NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL
            }
        },
        {   ALT_KEYPAD_0_9, {                   // ESC CSI {ABCD} ~
                NULL,               NULL,               "\x1b\x1b[B",       NULL,               "\x1b\x1b[D",
                NULL,               "\x1b\x1b[C",       NULL,               "\x1b\x1b[A",       NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL,               NULL,               NULL,               NULL,               NULL,
                NULL
            }
        },
#endif      // DO_UNSUPPORTED

        {   CTRL_A_Z, "CTRL_A_Z", {             /* 0x01-0x1f */
                "\x01", "\x02", "\x03", "\x04", "\x05", "\x06", "\x07", "\x08", "\x09", "\x0a",
                "\x0b", "\x0c", "\x0d", "\x0e", "\x0f", "\x10", "\x11", "\x12", "\x13", "\x14",
                "\x15", "\x16", "\x17", "\x18", "\x19", "\x1a", "\x1b", "\x1c", "\x1d", "\x1e",
                "\x1f"
            }, "control"
        },

        {   0x3b1, "Lower Case Greek", {        /* #945-#969, 2-byte */
                "\xce\xb1",     // GREEK SMALL LETTER ALPHA             #945    #x03B1
                "\xce\xb2",     // GREEK SMALL LETTER BETA              #946    #x03B2
                "\xce\xb3",     // GREEK SMALL LETTER GAMMA             #947    #x03B3
                "\xce\xb4",     // GREEK SMALL LETTER DELTA             #948    #x03B4
                "\xce\xb5",     // GREEK SMALL LETTER EPSILON           #949    #x03B5
                "\xce\xb6",     // GREEK SMALL LETTER ZETA              #950    #x03B6
                "\xce\xb7",     // GREEK SMALL LETTER ETA               #951    #x03B7
                "\xce\xb8",     // GREEK SMALL LETTER THETA             #952    #x03B8
                "\xce\xb9",     // GREEK SMALL LETTER IOTA              #953    #x03B9
                "\xce\xbA",     // GREEK SMALL LETTER KAPPA             #954    #x03BA
                "\xce\xbB",     // GREEK SMALL LETTER LAM(B)DA          #955    #x03BB
                "\xce\xbC",     // GREEK SMALL LETTER MU                #956    #x03BC
                "\xce\xbD",     // GREEK SMALL LETTER NU                #957    #x03BD
                "\xce\xbE",     // GREEK SMALL LETTER XI                #958    #x03BE
                "\xce\xbF",     // GREEK SMALL LETTER OMICRON           #959    #x03BF
                "\xcf\x80",     // GREEK SMALL LETTER PI                #960    #x03C0
                "\xcf\x81",     // GREEK SMALL LETTER RHO               #961    #x03C1
                "\xcf\x82",     // GREEK SMALL LETTER FINAL SIGMA       #962    #x03C2
                "\xcf\x83",     // GREEK SMALL LETTER SIGMA             #963    #x03C3
                "\xcf\x84",     // GREEK SMALL LETTER TAU               #964    #x03C4
                "\xcf\x85",     // GREEK SMALL LETTER UPSILON           #965    #x03C5
                "\xcf\x86",     // GREEK SMALL LETTER PHI               #966    #x03C6
                "\xcf\x87",     // GREEK SMALL LETTER CHI               #967    #x03C7
                "\xcf\x88",     // GREEK SMALL LETTER PSI               #968    #x03C8
                "\xcf\x89",     // GREEK SMALL LETTER OMEGA             #969    #x03C9
            }, "unicode"
        },
        {   0x2170, "Small Roman", {            /* #8560-#8575, 3-byte */
                "\xe2\x85\xb0", // SMALL ROMAN 1                        #8560   #x2170
                "\xe2\x85\xb1", // SMALL ROMAN 2                        #8561   #x2171
                "\xe2\x85\xb2", // SMALL ROMAN 3                        #8562   #x2172
                "\xe2\x85\xb3", // SMALL ROMAN 4                        #8563   #x2173
                "\xe2\x85\xb4", // SMALL ROMAN 5                        #8564   #x2174
                "\xe2\x85\xb5", // SMALL ROMAN 6                        #8565   #x2175
                "\xe2\x85\xb6", // SMALL ROMAN 7                        #8566   #x2176
                "\xe2\x85\xb7", // SMALL ROMAN 8                        #8567   #x2177
                "\xe2\x85\xb8", // SMALL ROMAN 9                        #8568   #x2178
                "\xe2\x85\xb9", // SMALL ROMAN 10                       #8569   #x2179
                "\xe2\x85\xba", // SMALL ROMAN 11                       #8570   #x217A
                "\xe2\x85\xbb", // SMALL ROMAN 12                       #8571   #x217B
                "\xe2\x85\xbc", // SMALL ROMAN 50                       #8572   #x217C
                "\xe2\x85\xbd", // SMALL ROMAN 100                      #8573   #x217D
                "\xe2\x85\xbe", // SMALL ROMAN 500                      #8574   #x217E
                "\xe2\x85\xbf", // SMALL ROMAN 1000                     #8575   #x217F
                "\xe2\x86\x80", // Roman numeral one thousand CD        #8576   #x2180
                "\xe2\x86\x81", // Roman numeral five thousand          #8577   #x2181
                "\xe2\x86\x82", // Roman numeral ten thousand           #8578   #x2182
                "\xe2\x86\x83", // Roman numeral reversed one hundred   #8579   #x2183
                "\xe2\x86\x84", // Latin Small Letter Reversed C
                "\xe2\x86\x85", // Roman six late form                  #8581   #x2185
                "\xe2\x86\x86", // Roman numeral fifty early form       #8582   #x2186
                "\xe2\x86\x87", // Roman numeral fifty thousand         #8583   #x2187
                "\xe2\x86\x88", // Roman numeral one hundred thousand   #8584   #x2188
            }
        },

        /// Special

            //  Ctrl-Space      (0x00)
            //  Tab(0x09)
            //  Enter (0x0d)
            //  BackSpace(0x08 and 0x7f)

            //  BACK_TAB,       "\x1b\t"
            //  BACK_TAB,       "\x1b[Z",
            //  KEY_BACKSPACE,  "\x7f"
    };


static const struct Label {
    KEY key;
    const char *name;

} labels[] = {
    { F(1),                     "F1" },
    { SF(1),                    "SHIFT-F1" },
    { CF(1),                    "CTRL-F1" },
    { CSF(1),                   "CTRL-SHIFT-F1" },
    { AF(1),                    "ALT-F1" },
    { ASF(1),                   "ALT-SHIFT-F1" },
    { ACF(1),                   "ALT-CTRL-F1" },
    { ACSF(1),                  "ALT-CTRL-SHIFT-F1" },
    { KEYPAD_0,                 "KEYPAD-0" },
    { SHIFT_KEYPAD_0,           "SHIFT-KEYPAD-0" },
    { CTRL_KEYPAD_0,            "CTRL-KEYPAD-0" },
    { CTRLSHIFT_KEYPAD_0,       "CTRL-SHIFT-KEYPAD-0" },
    { ALT_KEYPAD_0,             "ALT-KEYPAD-0" },
    { ALTSHIFT_KEYPAD_0,        "ALT-SHIFT-KEYPAD-0" },
    { ALTCTRL_KEYPAD_0,         "ALT-CTRL-KEYPAD-0" },
    { ALTCTRLSHIFT_KEYPAD_0,    "ALT-CTRL-SHIFT-KEYPAD-0" },
    { CTRL_A,                   "Ctrl-A" },
    { ALT_A,                    "Alt-A" },
    { ALT_0,                    "Alt-0" },


    { BACK_TAB,                 "Shift-Tab" },
    { BACK_TAB,                 "Back-Tab" },   // alias/internal-name
    { CTRL_TAB,                 "Ctrl-Tab" },
    { ALT_TAB,                  "Alt-Tab" },

    { SHIFT_BACKSPACE,          "Shift-Backspace" },
    { CTRL_BACKSPACE,           "Ctrl-Backspace" },
    { ALT_BACKSPACE,            "Alt-Backspace" },

    { KEY_UNDO_CMD,             "Undo" },
  //{ KEY_UNDO,                 "Undo" },
    { KEY_COPY_CMD,             "Copy" },
  //{ KEY_COPY,                 "Copy" },
    { KEY_CUT_CMD,              "Cut" },
  //{ KEY_CUT,                  "Cut" },
    { KEY_PASTE,                "Paste" },
    { KEY_HELP,                 "Help" },
    { KEY_REDO,                 "Redo" },
    { KEY_SEARCH,               "Search" },
    { KEY_REPLACE,              "Replace" },
    { KEY_CANCEL,               "Cancel" },
    { KEY_COMMAND,              "Command" },
    { KEY_EXIT,                 "Exit" },
    { KEY_NEXT,                 "Next" },
    { KEY_PREV,                 "Prev" },
    { KEY_OPEN,                 "Open" },
    { KEY_SAVE,                 "Save" },
    { KEY_MENU,                 "Menu" },
    { KEY_BREAK,                "Break" },

    { BUTTON1_DOWN,             "Button1-Down" },
    { BUTTON2_DOUBLE,           "Button2-Double" },
    { BUTTON3_MOTION,           "Button3-Motion" },
    { BUTTON1_UP,               "Button1-Up" },
    { WHEEL_DOWN,               "Wheel-Down" },
    { WHEEL_LEFT,               "Wheel-Left" },
    { WHEEL_RIGHT,              "Wheel-Right" },
    { WHEEL_UP,                 "Wheel-Up" },

    { KEY_ENTER,                "Enter" },
    { KEY_ESC,                  "Escape" },
    { KEY_ESC,                  "Esc" },
    { KEY_SPACE,                "Space" },

    { KEYPAD_0,                 "Keypad-Ins" },
    { KEYPAD_1,                 "Keypad-End" },
    { KEYPAD_2,                 "Keypad-Down" },
    { KEYPAD_3,                 "Keypad-PgDn" },
    { KEYPAD_4,                 "Keypad-Left" },
    { KEYPAD_4,                 "Keypad-Left-Arrow" }, // special
    { KEYPAD_5,                 "Keypad-5" },
    { KEYPAD_6,                 "Keypad-Right" },
    { KEYPAD_7,                 "Keypad-Home" },
    { KEYPAD_8,                 "Keypad-Up" },
    { KEYPAD_9,                 "Keypad-PgUp" },
    { KEYPAD_DEL,               "Keypad-Del" },
    { KEYPAD_PLUS,              "Keypad-Plus" },
    { KEYPAD_MINUS,             "Keypad-Minus" },
    { KEYPAD_STAR,              "Keypad-Star" },
    { KEYPAD_DIV,               "Keypad-Divide" },
    { KEYPAD_EQUAL,             "Keypad-Equals" },
    { KEYPAD_ENTER,             "Keypad-Enter" },
    { KEYPAD_PAUSE,             "Keypad-Pause" },
    { KEYPAD_PRTSC,             "Keypad-PrtSc" },
    { KEYPAD_SCROLL,            "Keypad-Scroll" },
    { KEYPAD_NUMLOCK,           "Keypad-NumLock" },

        // keypad aliases
    { KEY_INS,                  "Insert" },     // KEYPAD_0
    { KEY_INS,                  "Ins" },        //    :
    { KEY_END,                  "End" },        // KEYPAD_1
    { KEY_DOWN,                 "Down" },       // KEYPAD_2
    { KEY_PAGEDOWN,             "PgDn" },       // KEYPAD_3
    { KEY_LEFT,                 "Left" },       // KEYPAD_4
    { KEY_RIGHT,                "Right" },      // KEYPAD_6
    { KEY_HOME,                 "Home" },       // KEYPAD_7
    { KEY_UP,                   "Up" },         // KEYPAD_8
    { KEY_PAGEUP,               "PgUp" },       // KEYPAD_9
    { KEY_DEL,                  "Delete" },     // KEYPAD_DEL
    { KEY_DEL,                  "Del" },        //    :

        // special
    { MOUSE_XTERM_KEY,          "MouseXTerm" },
    { MOUSE_SGR_KEY,            "MouseSGR" },
    { MOUSE_KEY,                "Mouse" },
    { MOUSE_FOCUSOUT_KEY,       "FocusOut" },
    { MOUSE_FOCUSIN_KEY,        "FocusIn" }
    };

static void die(const char *fmt, ...);
static void hex(const char *data, unsigned length);

void trace_ilog(const char *fmt, ...);
void trace_log(const char *fmt, ...);


////////////////////////////////////////////////////////////////////////////////////////
/// kbprotocols api test

int xf_kbprotocol = 0x7fffffff;                 // enable all

static void
protocols_test(void)
{
    const char *ctx = "";
    unsigned i, k;

    kbprotocols_init();

    for (i = 0; i < (sizeof(keys)/sizeof(keys[0])); ++i) {
        KEY key = keys[i].keys;                 // key base

        if (keys[i].ctx) {                      // set context
            ctx = keys[i].ctx;
        }

        trace_ilog("\n%s - %s:\n", ctx, keys[i].desc);
        for (k = 0; k < NSEQS; ++k) {
            const char *seq = keys[i].seqs[k];
            if (seq) {
                const unsigned seqlen = strlen(seq);
                const int ret = kbprotocols_parse(seq, seqlen, 0);

                trace_ilog(" %2d:%2d] %08x = %08x [%s] ",
                    i, k, key, ret, (ret == key ? "OK" : (ret == 0 ? "na" : "??")));
                hex(seq, seqlen);
            }
            ++key;
        }
    }
}



////////////////////////////////////////////////////////////////////////////////////////
/// trie use-case test
///

struct TableNode {
    const char *seq;
    KEY key;
};


static const struct TableNode *
tree_node(const char *seq, KEY key)
{
    struct TableNode *node =
        (struct TableNode *)malloc(sizeof(struct TableNode));
    node->seq = seq;
    node->key = key;
    return node;
}


static void
trie_test(void)
{
    struct trie *t = trie_create();
    unsigned count = 0, i, k;
    char kbname[KBNAMELEN];

    trace_ilog("\ntrie_test:\n");

    trace_ilog("kbseq: insert\n");
    for (i = 0; i < (sizeof(keys)/sizeof(keys[0])); ++i) {
        KEY key = keys[i].keys;                 // key base

        for (k = 0; k < NSEQS; ++k) {
            const char *seq = keys[i].seqs[k];
            if (seq && *seq) {
                const struct TableNode *node = tree_node(seq, key);

                if (trie_insert(t, seq, (void *)node)) {
                    die("failure: out of memory");
                }

                if (trie_search(t, seq) != (void *)node) {
                    die("failure: missing key(1)");
                }
                ++count;
            }
            ++key;
        }
    }

    trace_ilog("kbseq: search, count=%d, unique=%d\n", count, trie_count(t, NULL));
    for (i = 0; i < (sizeof(keys)/sizeof(keys[0])); ++i) {
        KEY key = keys[i].keys;                 // key base

        for (k = 0; k < NSEQS; ++k) {
            const char *seq = keys[i].seqs[k];
            if (seq) {
                const struct TableNode *node = trie_search(t, seq);

                if (NULL == node || node->key != key) {
                    die("failure: missing key(2)");
                }
            }
            ++key;
        }
    }

    trace_ilog("kbseq: search, count=%d, unique=%d\n", count, trie_count(t, NULL));
    for (i = 0; i < (sizeof(keys)/sizeof(keys[0])); ++i) {
        KEY key = keys[i].keys;                 // key base

        for (k = 0; k < NSEQS; ++k) {
            const char *seq = keys[i].seqs[k];
            if (seq) {
                unsigned seqlen = strlen(seq);

                while (seqlen) {
                    unsigned nchildren = 0;     // number-of-children, ambiguous
                    const struct TableNode *ret, *partial;

                    ret = (struct TableNode *)trie_nsearch_ambiguous(t, seq, seqlen, &nchildren, (void  **)&partial);
                    trace_ilog(" %2d:%2d.%2u] %3d [%s] %08x %-32s = ",
                        i, k, seqlen, nchildren, (ret && ret->key == key ? "OK" : "na"), (ret ? ret->key : 0),
                            kbname_fromkey(ret ? ret->key : 0, kbname, sizeof(kbname)));
                    hex(seq, seqlen);
                    --seqlen;

                    if (ret) {                  // round-trip test
                        int keylen = 0;
                        const KEY nkey = kbname_tokey(kbname, &keylen);
                        if (ret->key != nkey) {
                            die("failure: invalid kbname \"%s\" %d/%08x != %d/%08x",
                                kbname, ret->key, ret->key, nkey, nkey);
                        }
                        if (keylen != strlen(kbname)) {
                            die("failure: invalid kbname length \"%s\" %d/%08x != %d/%08x",
                                kbname, ret->key, ret->key, nkey, nkey);
                        }
                    }

                    if (partial) {
                        trace_ilog("%11s partial %08x %-32s = ", "",
                            partial->key, kbname_fromkey(partial->key, kbname, sizeof(kbname)));
                        hex(partial->seq, (unsigned)strlen(partial->seq));
                    }

                    if (0 == nchildren) {       // should match
                        if (ret == NULL || ret->key != key) {
                            die("failure: unmatched seq");
                        }
                        if (partial != NULL) {
                            die("failure: partial unexpected");
                        }

                    } else {                    // ambiguous, partial expected
                        if (partial == NULL) {
                            die("failure: partial expected");
                        }
                    }
                }
            }
            ++key;
        }
    }

    trace_ilog("kbseq: destroy\n");
    {   struct trie_it *it = trie_it_create(t, NULL);

        if (it) {
            if (! trie_it_done(it)) {
                do {
                    void *data = trie_it_data(it);
                    free(data);
                } while (trie_it_next(it));
            }
            trie_it_free(it);
        }
        trie_free(t);
    }
}


////////////////////////////////////////////////////////////////////////////////////////
/// kbsequence api tests
///

static void
seqtable_test(void)
{
    unsigned i, k;

    trace_ilog("\nkbtable: init\n");
    kbsequence_init();

    trace_ilog("kbtable: insert\n");
    for (i = 0; i < (sizeof(keys)/sizeof(keys[0])); ++i) {
        KEY key = keys[i].keys;                 // key base

        for (k = 0; k < NSEQS; ++k) {
            const char *seq = keys[i].seqs[k];
            if (seq && *seq) {
                kbsequence_update(seq, key);
            }
            ++key;  // next
        }
    }

    trace_ilog("kbtable: lookup\n");
    for (i = 0; i < (sizeof(keys)/sizeof(keys[0])); ++i) {
        KEY key = keys[i].keys;                 // key base

        for (k = 0; k < NSEQS; ++k) {
            const char *seq = keys[i].seqs[k];
            if (seq && *seq) {
                const keyseq_t *ks = kbsequence_lookup(seq);
                unsigned ambiguous = 0;

                if (NULL == ks)
                    die("failure: unmatched");
                if (ks->ks_code != key)
                    die("failure: invalid keycode");
                if (strcmp(ks->ks_buf, seq))
                    die("failure: invalid seq");

                if (ks != kbsequence_match(seq, strlen(seq), &ambiguous, NULL))
                    die("failure: invalid match");
            }
            ++key;  // next
        }
    }

    trace_ilog("kbtable: shutdown\n");
    kbsequence_shutdown();
}


////////////////////////////////////////////////////////////////////////////////////////
/// kbname api tests
///


static void
kbname_test(void)
{
    char kbtemp[KBNAMELEN], kbname[KBNAMELEN];
    unsigned i;

    trace_ilog("\nkbnames: start\n");

    for (i = 0; i < (sizeof(labels)/sizeof(labels[0])); ++i) {
        const struct Label *label = labels + i;
        int key1len = 0, key2len = 0;
        KEY key1, key2;

        kbname_fromkey(label->key, kbname, sizeof(kbname));
        sxprintf(kbtemp, sizeof(kbtemp), "<%s>", label->name);

        key1 = kbname_tokey(label->name, &key1len);
        key2 = kbname_tokey(kbtemp, &key2len);

        trace_ilog(" %2d] %-32s|%-32s = %d/%08x\n", i, label->name, kbname, key1, key1);

        if (label->key != key1) {
            die("failure: invalid kbname code \"%s\" %d/%08x != %d/%08x",
                label->name, label->key, label->key, key1, key1);
        }

        if (key1len != strlen(label->name)) {
            die("failure: invalid kbname length \"%s\" %d/%08x != %d/%08x",
                label->name, label->key, label->key, key1, key1);
        }

        if (key1 != key2) {
            die("failure: invalid kbname alt-code \"%s\" %d/%08x != %d/%08x",
                kbtemp, key2, key2, key1, key1);
        }

        if (key2len != strlen(kbtemp)) {
            die("failure: invalid kbname alt-length \"%s\" %d/%08x != %d/%08x",
                kbtemp, key2, key2, key1, key1);
        }
    }

    trace_ilog("kbnames: end\n");
}


int
main(void)
{
    protocols_test();
    trie_test();
    seqtable_test();
    kbname_test();
    return 0;
}


static void
die(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    fflush(stdout);
    vfprintf(stderr, fmt, ap);
    fputs("\n", stderr);
    fflush(stderr);
    exit(3);
}


static void
hex(const char *data, unsigned length)
{
#define HEXWIDTH 12
    char buf1[HEXWIDTH * 4], buf2[HEXWIDTH * 2];
    const unsigned char *cursor = (const unsigned char *)data,
        *end = cursor + length;
    unsigned offset = 0, len1, len2;
    int done;

    if (length <= 0) {
        return;
    }

    buf1[len1 = 0] = 0;
    len2 = 0;

    do {
        const unsigned ch = *cursor++;
        done = (cursor == end);

        len1 += sprintf(buf1 + len1, "%02x ", ch);
        len2 += sprintf(buf2 + len2, "%c", (ch >= ' ' && ch < 0x7f ? ch : '.'));

        if ((offset++ && 0 == (offset % HEXWIDTH)) || done) {
            trace_ilog("%2d: %-*s | %-*s |\n", length, HEXWIDTH*3, buf1, HEXWIDTH, buf2);
            buf1[len1 = 0] = 0;
            len2 = 0;
        }
    } while (! done);
}


#if !defined(_MSC_VER)
void
trace_log(const char *fmt, ...)
{
//  va_list ap;
//  va_start(ap, fmt);
//  vprintf(fmt, ap);
//  va_end(ap);
}
#endif //_MSC_VER


void
trace_ilog(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

//end
