#include <edidentifier.h>
__CIDENT_RCSID(gr_ttykitty_c,"$Id: ttykitty.c,v 1.2 2024/11/18 13:42:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ttykitty.c,v 1.2 2024/11/18 13:42:22 cvsuser Exp $
 * kitty common utility functions - alpha
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

//  All numbers are in the Unicode Private Use Area (57344 - 63743)
//  except for a handful of keys that use numbers under 32 and 127 (C0 control codes) for legacy compatibility reasons (vt220+).
//
//	Name		CSI	     |	Name            CSI
//	-----------------------------|--------------------------------------
//	ESCAPE		27 u	     |	ENTER           13 u
//	TAB             9 u          |  BACKSPACE       127 u
//	INSERT          2 ~          |  DELETE          3 ~
//	LEFT            1 D          |  RIGHT           1 C
//	UP              1 A          |  DOWN            1 B
//	PAGE_UP         5 ~          |  PAGE_DOWN       6 ~
//	HOME            1 H or 7 ~   |  END             1 F or 8 ~
//	F1              1 P or 11 ~  |  F2              1 Q or 12 ~
//	F3              13 ~         |  F4              1 S or 14 ~
//	F5              15 ~         |  F6              17 ~
//	F7              18 ~         |  F8              19 ~
//	F9              20 ~         |  F10             21 ~
//	F11             23 ~         |  F12             24 ~
//				     |

enum KittyKey {
    KITTY_KEY_ESCAPE                = 57344,
    KITTY_KEY_ENTER                 = 57345,
    KITTY_KEY_TAB                   = 57346,
    KITTY_KEY_BACKSPACE             = 57347,
    KITTY_KEY_INSERT                = 57348,
    KITTY_KEY_DELETE                = 57349,
    KITTY_KEY_LEFT                  = 57350,
    KITTY_KEY_RIGHT                 = 57351,
    KITTY_KEY_UP                    = 57352,
    KITTY_KEY_DOWN                  = 57353,
    KITTY_KEY_PAGE_UP               = 57354,
    KITTY_KEY_PAGE_DOWN             = 57355,
    KITTY_KEY_HOME                  = 57356,
    KITTY_KEY_END                   = 57357,
    KITTY_KEY_CAPS_LOCK             = 57358,
    KITTY_KEY_SCROLL_LOCK           = 57359,
    KITTY_KEY_NUM_LOCK              = 57360,
    KITTY_KEY_PRINT_SCREEN          = 57361,
    KITTY_KEY_PAUSE                 = 57362,
    KITTY_KEY_MENU                  = 57363,
    KITTY_KEY_F1                    = 57364,
    KITTY_KEY_F2                    = 57365,
    KITTY_KEY_F3                    = 57366,
    KITTY_KEY_F4                    = 57367,
    KITTY_KEY_F5                    = 57368,
    KITTY_KEY_F6                    = 57369,
    KITTY_KEY_F7                    = 57370,
    KITTY_KEY_F8                    = 57371,
    KITTY_KEY_F9                    = 57372,
    KITTY_KEY_F10                   = 57373,
    KITTY_KEY_F11                   = 57374,
    KITTY_KEY_F12                   = 57375,
    KITTY_KEY_F13                   = 57376,
    KITTY_KEY_F14                   = 57377,
    KITTY_KEY_F15                   = 57378,
    KITTY_KEY_F16                   = 57379,
    KITTY_KEY_F17                   = 57380,
    KITTY_KEY_F18                   = 57381,
    KITTY_KEY_F19                   = 57382,
    KITTY_KEY_F20                   = 57383,
    KITTY_KEY_F21                   = 57384,
    KITTY_KEY_F22                   = 57385,
    KITTY_KEY_F23                   = 57386,
    KITTY_KEY_F24                   = 57387,
    KITTY_KEY_F25                   = 57388,
    KITTY_KEY_F26                   = 57389,
    KITTY_KEY_F27                   = 57390,
    KITTY_KEY_F28                   = 57391,
    KITTY_KEY_F29                   = 57392,
    KITTY_KEY_F30                   = 57393,
    KITTY_KEY_F31                   = 57394,
    KITTY_KEY_F32                   = 57395,
    KITTY_KEY_F33                   = 57396,
    KITTY_KEY_F34                   = 57397,
    KITTY_KEY_F35                   = 57398,
    KITTY_KEY_KP_0                  = 57399,
    KITTY_KEY_KP_1                  = 57400,
    KITTY_KEY_KP_2                  = 57401,
    KITTY_KEY_KP_3                  = 57402,
    KITTY_KEY_KP_4                  = 57403,
    KITTY_KEY_KP_5                  = 57404,
    KITTY_KEY_KP_6                  = 57405,
    KITTY_KEY_KP_7                  = 57406,
    KITTY_KEY_KP_8                  = 57407,
    KITTY_KEY_KP_9                  = 57408,
    KITTY_KEY_KP_DECIMAL            = 57409,
    KITTY_KEY_KP_DIVIDE             = 57410,
    KITTY_KEY_KP_MULTIPLY           = 57411,
    KITTY_KEY_KP_SUBTRACT           = 57412,
    KITTY_KEY_KP_ADD                = 57413,
    KITTY_KEY_KP_ENTER              = 57414,
    KITTY_KEY_KP_EQUAL              = 57415,
    KITTY_KEY_KP_SEPARATOR          = 57416,
    KITTY_KEY_KP_LEFT               = 57417,
    KITTY_KEY_KP_RIGHT              = 57418,
    KITTY_KEY_KP_UP                 = 57419,
    KITTY_KEY_KP_DOWN               = 57420,
    KITTY_KEY_KP_PAGE_UP            = 57421,
    KITTY_KEY_KP_PAGE_DOWN          = 57422,
    KITTY_KEY_KP_HOME               = 57423,
    KITTY_KEY_KP_END                = 57424,
    KITTY_KEY_KP_INSERT             = 57425,
    KITTY_KEY_KP_DELETE             = 57426,
    KITTY_KEY_KP_BEGIN              = 57427,
    KITTY_KEY_MEDIA_PLAY            = 57428,
    KITTY_KEY_MEDIA_PAUSE           = 57429,
    KITTY_KEY_MEDIA_PLAY_PAUSE      = 57430,
    KITTY_KEY_MEDIA_REVERSE         = 57431,
    KITTY_KEY_MEDIA_STOP            = 57432,
    KITTY_KEY_MEDIA_FAST_FORWARD    = 57433,
    KITTY_KEY_MEDIA_REWIND          = 57434,
    KITTY_KEY_MEDIA_TRACK_NEXT      = 57435,
    KITTY_KEY_MEDIA_TRACK_PREVIOUS  = 57436,
    KITTY_KEY_MEDIA_RECORD          = 57437,
    KITTY_KEY_LOWER_VOLUME          = 57438,
    KITTY_KEY_RAISE_VOLUME          = 57439,
    KITTY_KEY_MUTE_VOLUME           = 57440,
    KITTY_KEY_LEFT_SHIFT            = 57441,
    KITTY_KEY_LEFT_CONTROL          = 57442,
    KITTY_KEY_LEFT_ALT              = 57443,
    KITTY_KEY_LEFT_SUPER            = 57444,
    KITTY_KEY_LEFT_HYPER            = 57445,
    KITTY_KEY_LEFT_META             = 57446,
    KITTY_KEY_RIGHT_SHIFT           = 57447,
    KITTY_KEY_RIGHT_CONTROL         = 57448,
    KITTY_KEY_RIGHT_ALT             = 57449,
    KITTY_KEY_RIGHT_SUPER           = 57450,
    KITTY_KEY_RIGHT_HYPER           = 57451,
    KITTY_KEY_RIGHT_META            = 57452,
    KITTY_KEY_ISO_LEVEL3_SHIFT      = 57453,
    KITTY_KEY_ISO_LEVEL5_SHIFT      = 57454,
};

static const struct {
    enum KittyKey key;
    const char *name;
} kitty_keys[] = {
    { KITTY_KEY_ESCAPE,             "Esc"           },
    { KITTY_KEY_ENTER,              "CR"            },
    { KITTY_KEY_TAB,                "Tab"           },
    { KITTY_KEY_BACKSPACE,          "BS"            },
    { KITTY_KEY_INSERT,             "Insert"        },
    { KITTY_KEY_DELETE,             "Del"           },
    { KITTY_KEY_LEFT,               "Left"          },
    { KITTY_KEY_RIGHT,              "Right"         },
    { KITTY_KEY_UP,                 "Up"            },
    { KITTY_KEY_DOWN,               "Down"          },
    { KITTY_KEY_PAGE_UP,            "PageUp"        },
    { KITTY_KEY_PAGE_DOWN,          "PageDown"      },
    { KITTY_KEY_HOME,               "Home"          },
    { KITTY_KEY_END,                "End"           },
    { KITTY_KEY_F1,                 "F1"            },
    { KITTY_KEY_F2,                 "F2"            },
    { KITTY_KEY_F3,                 "F3"            },
    { KITTY_KEY_F4,                 "F4"            },
    { KITTY_KEY_F5,                 "F5"            },
    { KITTY_KEY_F6,                 "F6"            },
    { KITTY_KEY_F7,                 "F7"            },
    { KITTY_KEY_F8,                 "F8"            },
    { KITTY_KEY_F9,                 "F9"            },
    { KITTY_KEY_F10,                "F10"           },
    { KITTY_KEY_F11,                "F11"           },
    { KITTY_KEY_F12,                "F12"           },
    { KITTY_KEY_F13,                "F13"           },
    { KITTY_KEY_F14,                "F14"           },
    { KITTY_KEY_F15,                "F15"           },
    { KITTY_KEY_F16,                "F16"           },
    { KITTY_KEY_F17,                "F17"           },
    { KITTY_KEY_F18,                "F18"           },
    { KITTY_KEY_F19,                "F19"           },
    { KITTY_KEY_F20,                "F20"           },
    { KITTY_KEY_F21,                "F21"           },
    { KITTY_KEY_F22,                "F22"           },
    { KITTY_KEY_F23,                "F23"           },
    { KITTY_KEY_F24,                "F24"           },
    { KITTY_KEY_F25,                "F25"           },
    { KITTY_KEY_F26,                "F26"           },
    { KITTY_KEY_F27,                "F27"           },
    { KITTY_KEY_F28,                "F28"           },
    { KITTY_KEY_F29,                "F29"           },
    { KITTY_KEY_F30,                "F30"           },
    { KITTY_KEY_F31,                "F31"           },
    { KITTY_KEY_F32,                "F32"           },
    { KITTY_KEY_F33,                "F33"           },
    { KITTY_KEY_F34,                "F34"           },
    { KITTY_KEY_F35,                "F35"           },
    { KITTY_KEY_KP_0,               "KP_0"          },
    { KITTY_KEY_KP_1,               "KP_1"          },
    { KITTY_KEY_KP_2,               "KP_2"          },
    { KITTY_KEY_KP_3,               "KP_3"          },
    { KITTY_KEY_KP_4,               "KP_4"          },
    { KITTY_KEY_KP_5,               "KP_5"          },
    { KITTY_KEY_KP_6,               "KP_6"          },
    { KITTY_KEY_KP_7,               "KP_7"          },
    { KITTY_KEY_KP_8,               "KP_8"          },
    { KITTY_KEY_KP_9,               "KP_9"          },
    { KITTY_KEY_KP_DECIMAL,         "KP_Point"      },
    { KITTY_KEY_KP_DIVIDE,          "KP_Divide"     },
    { KITTY_KEY_KP_MULTIPLY,        "KP_Multiply"   },
    { KITTY_KEY_KP_SUBTRACT,        "KP_Subtract"   },
    { KITTY_KEY_KP_ADD,             "KP_Add"        },
    { KITTY_KEY_KP_ENTER,           "KP_Enter"      },
    { KITTY_KEY_KP_EQUAL,           "KP_Equal"      },
    { KITTY_KEY_KP_LEFT,            "KP_Left"       },
    { KITTY_KEY_KP_RIGHT,           "KP_Right"      },
    { KITTY_KEY_KP_UP,              "KP_Up"         },
    { KITTY_KEY_KP_DOWN,            "KP_Down"       },
    { KITTY_KEY_KP_PAGE_UP,         "KP_PageUp"     },
    { KITTY_KEY_KP_PAGE_DOWN,       "KP_PageDown"   },
    { KITTY_KEY_KP_HOME,            "KP_Home"       },
    { KITTY_KEY_KP_END,             "KP_End"        },
    { KITTY_KEY_KP_INSERT,          "KP_Insert"     },
    { KITTY_KEY_KP_DELETE,          "KP_Del"        },
    { KITTY_KEY_KP_BEGIN,           "KP_Origin"     },
};


/*
 *  tty_kitty_key ---
 *      Parse a Kitty enchanced key encoded event.
 *
 *      Progressively enhance mode:
 *
 *>         CSI unicode-key-code : alternate-key-codes ; modifiers : event-type ; text-as-codepoints u
 *
 *      CSI is the bytes 0x1b 0x5b. All parameters are decimal numbers. Fields are separated by the semi-colon and sub-fields by the colon.
 *      Only the unicode-key-code field is mandatory, everything else is optional. The escape code is terminated by the u character (the byte 0x75).
 *
 *      See: https://sw.kovidgoyal.net/kitty/keyboard-protocol/
 */

int
tty_kitty_key(const char *spec)
{
    const unsigned char *cursor = (unsigned char *)spec;
    unsigned arguments[2] = {0}, args = 0, digits = 0;
    int function = 0;

    //
    //  \x1B[<parameters>u":
    //
    if (cursor[0] != 0x1b || cursor[1] != '[') {
        return -1;                              // CSI, expected
    }

    for (cursor += 2; *cursor;) {
        const unsigned char c = *cursor++;

        if (c >= '0' && c <= '9') {
            if (0 == digits++) {
                if (args > 5) {
                    break;                      // overflow
                }
                arguments[args] = (c - '0');
            } else {
                arguments[args] = (arguments[args] * 10) + (c - '0');
            }

        } else if (c == ';' || c == 'u') {
            digits = 0;
            ++args;
            if (c == 'u') {
                break;                          // terminator
            }
        }
    }

#if (1)
    if (key->code.codepoint >= 0xE000 && key->code.codepoint <= 0xF8FF) {
    }
#endif
}

/*end*

