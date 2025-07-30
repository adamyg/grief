#include <edidentifier.h>
__CIDENT_RCSID(gr_kbname_c,"$Id: kbname.c,v 1.3 2025/02/07 03:03:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: kbname.c,v 1.3 2025/02/07 03:03:21 cvsuser Exp $
 * Key names.
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

#include <edalt.h>
#include <libstr.h>                             /* str_...()/sxprintf() */

#include "kbname.h"

/*
 *  internal codes labels
 */
struct keypad {
    unsigned        length;         /* length, in bytes */
    const char *    name;           /* comparision name */
    const char *    label;          /* external label */
};

#define KEYPAD_LABELS       (sizeof(keypad_labels) / sizeof(keypad_labels[0]))

static struct keypad    keypad_labels[] = {
    { 3,     "INS",         "Ins" },        /* KEYPAD_0       - 0         */
    { 3,     "END",         "End" },        /* KEYPAD_1       - 1         */
    { 4,     "DOWN",        "Down" },       /* KEYPAD_2       - 2         */
    { 4,     "PGDN",        "PgDn" },       /* KEYPAD_3       - 3         */
    { 4,     "LEFT",        "Left" },       /* KEYPAD_4       - 4         */
    { 1,     "5",           "5" },          /* KEYPAD_5       - 5         */
    { 5,     "RIGHT",       "Right" },      /* KEYPAD_6       - 6         */
    { 4,     "HOME",        "Home" },       /* KEYPAD_7       - 7         */
    { 2,     "UP",          "Up" },         /* KEYPAD_8       - 8         */
    { 4,     "PGUP",        "PgUp" },       /* KEYPAD_9       - 9         */
    { 3,     "DEL",         "Del" },        /* KEYPAD_DEL     - 10 Delete */
    { 4,     "PLUS",        "Plus" },       /* KEYPAD_PLUS    - 11 +      */
    { 5,     "MINUS",       "Minus" },      /* KEYPAD_MINUS   - 12 -      */
    { 4,     "STAR",        "Star" },       /* KEYPAD_STAR    - 13 *      */
    { 6,     "DIVIDE",      "Divide" },     /* KEYPAD_DIV     - 14 /      */
    { 6,     "EQUALS",      "Equals" },     /* KEYPAD_EQUAL   - 15 =      */
    { 5,     "ENTER",       "Enter" },      /* KEYPAD_ENTER   - 16 <cr>   */
    { 5,     "PAUSE",       "Pause" },      /* KEYPAD_PAUSE   - 17        */
    { 5,     "PRTSC",       "PrtSc" },      /* KEYPAD_PRTSC   - 18        */
    { 6,     "SCROLL",      "Scroll" },     /* KEYPAD_SCROLL  - 19        */
    { 7,     "NUMLOCK",     "NumLock" }     /* KEYPAD_NUMLOCK - 20        */
    };


/*
 *  key-strings to internal key-codes.
 */
struct label {
    unsigned        length;         /* length, in bytes */
    const char *    name;           /* comparision name */
    KEY             modifier;
    KEY             value;
};

#define KEY_LABELS      (sizeof(key_labels) / sizeof(key_labels[0])))

static const struct label key_labels[] = {
    { 6,    "ESCAPE",       0,              KEY_ESC },      /*alias*/
    { 3,    "ESC",          0,              KEY_ESC },
    { 5,    "SPACE",        0,              KEY_SPACE },
    { 6,    "RETURN",       0,              KEY_ENTER },    /*alias*/
    { 5,    "ENTER",        0,              KEY_ENTER },
    { 3,    "TAB",          0,              KEY_TAB },
    { 5,    "ARROW",        0,              0 },            /*void, for example "Keypad-Left-Arrow"*/
    { 4,    "CTRL",         MOD_CTRL,       0 },
    { 5,    "SHIFT",        MOD_SHIFT,      0 },
    { 9,    "BACKSPACE",    0,              KEY_BACKSPACE },
    { 4,    "BACK",         MOD_SHIFT,      0 },            /*must be after BACKSPACE*/
    { 7,    "PRIVATE",      RANGE_PRIVATE,  0 },
    { 3,    "ALT",          MOD_META,       0 },
    { 4,    "META",         MOD_META,       0 },
    { 6,    "KEYPAD",       RANGE_KEYPAD,   0 },
    { 4,    "GREY",         RANGE_KEYPAD,   0 },
    { 6,    "INSERT",       RANGE_KEYPAD,   KEY_INS },      /*alias*/
    { 3,    "INS",          RANGE_KEYPAD,   KEY_INS },
    { 3,    "END",          RANGE_KEYPAD,   KEY_END },
    { 4,    "DOWN",         RANGE_KEYPAD,   KEY_DOWN },
    { 4,    "PGDN",         RANGE_KEYPAD,   KEY_PAGEDOWN },
    { 4,    "LEFT",         RANGE_KEYPAD,   KEY_LEFT },
    { 5,    "RIGHT",        RANGE_KEYPAD,   KEY_RIGHT },
    { 4,    "HOME",         RANGE_KEYPAD,   KEY_HOME },
    { 2,    "UP",           RANGE_KEYPAD,   KEY_UP },
    { 4,    "PGUP",         RANGE_KEYPAD,   KEY_PAGEUP },
    { 6,    "DELETE",       RANGE_KEYPAD,   KEY_DEL },      /*alias*/
    { 3,    "DEL",          RANGE_KEYPAD,   KEY_DEL },
    { 4,    "PLUS",         RANGE_KEYPAD,   KEYPAD_PLUS },
    { 5,    "MINUS",        RANGE_KEYPAD,   KEYPAD_MINUS },
    { 4,    "STAR",         RANGE_KEYPAD,   KEYPAD_STAR },
    { 5,    "PRTSC",        RANGE_KEYPAD,   KEYPAD_PRTSC },
    { 6,    "SCROLL",       RANGE_KEYPAD,   KEYPAD_SCROLL },
    { 10,   "MOUSEXTERM",   RANGE_SPECIAL,  MOUSE_XTERM_KEY },
    { 8,    "MOUSESGR",     RANGE_SPECIAL,  MOUSE_SGR_KEY },
    { 5,    "MOUSE",        RANGE_SPECIAL,  MOUSE_KEY },
    { 7,    "FOCUSIN",      RANGE_SPECIAL,  MOUSE_FOCUSIN_KEY },
    { 8,    "FOCUSOUT",     RANGE_SPECIAL,  MOUSE_FOCUSOUT_KEY },
    { 4,    "UNDO",         RANGE_MISC,     KEY_UNDO_CMD },
    { 4,    "REDO",         RANGE_MISC,     KEY_REDO },
    { 4,    "COPY",         RANGE_MISC,     KEY_COPY_CMD },
    { 3,    "CUT",          RANGE_MISC,     KEY_CUT_CMD },
    { 5,    "PASTE",        RANGE_MISC,     KEY_PASTE },
    { 4,    "HELP",         RANGE_MISC,     KEY_HELP },
    { 6,    "SEARCH",       RANGE_MISC,     KEY_SEARCH },
    { 7,    "REPLACE",      RANGE_MISC,     KEY_REPLACE },
    { 6,    "CANCEL",       RANGE_MISC,     KEY_CANCEL },
    { 7,    "COMMAND",      RANGE_MISC,     KEY_COMMAND },
    { 4,    "EXIT",         RANGE_MISC,     KEY_EXIT },
    { 4,    "NEXT",         RANGE_MISC,     KEY_NEXT },
    { 4,    "PREV",         RANGE_MISC,     KEY_PREV },
    { 4,    "OPEN",         RANGE_MISC,     KEY_OPEN },
    { 4,    "SAVE",         RANGE_MISC,     KEY_SAVE },
    { 4,    "MENU",         RANGE_MISC,     KEY_MENU },
    { 5,    "BREAK",        RANGE_MISC,     KEY_BREAK },
    { 10,   "UNASSIGNED",   0,              KEY_UNASSIGNED },
    { 0, NULL, 0, 0 }
    };

static int              x_multiseq = 0;         /* unique multi-key identifiers. */
static const char **    x_multikeys;            /* multi-keys definition table. */

static char *           key_aschar(KEY key, char *buf);


/*  Function:           kbmulti_allocate
 *      Allocate a new multikey definition.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      Associated keycode, otherwise -1 on error.
 */
int
kbmulti_allocate(void)
{
#define MULTIKEY_COUNT(__c) ((__c|0xf) + 1)
    const int count = MULTIKEY_COUNT(x_multiseq);
    int key = -1;

    if (x_multiseq >= MULTIKEY_SIZE) {
        return -1;                              /* overflow */
    }

    key = RANGE_MULTIKEY + x_multiseq++;
    if (NULL == x_multikeys) {
        x_multikeys = chk_alloc(count * sizeof(const char *));
    } else {
        if (x_multiseq >= count) {              /* expand */
            x_multikeys = chk_realloc((void *)x_multikeys, MULTIKEY_COUNT(x_multiseq) * sizeof(const char *));
        }
    }
    x_multikeys[key - RANGE_MULTIKEY] = NULL;
    return key;
}


/*  Function:           kbmulti_allocate
 *      Assign to an allocated multikey definition.
 *
 *  Parameters:
 *      key - Associated multikey-code
 *      seq - Sequence buffer.
 *
 *  Results:
 *      nothing
 */
void
kbmulti_assign(KEY key, const char *seq)
{
    assert(IS_MULTIKEY(key));
    if (IS_MULTIKEY(key)) {
        x_multikeys[key - RANGE_MULTIKEY] = seq;
    }
}


/*  Function:           kbname_nametokey
 *      Convert a key description to its internal key code; limited validation/error handling.
 *
 *    Formats:
 *      <Wheel-Events>
 *      <Button#-Events>
 *      <Fxx>
 *      #Numeric
 *      <KeyPad-Elements>
 *      <Specials>
 *
 *  Parameters:
 *      string - Key description buffer.
 *      len - Optional field populated with the characters consumed within 'string'.
 *
 *  Results:
 *      keycode
 */
int
kbname_tokey(const char *string, int *len)
{
    char buf[KBNAMELEN * 2], *cp;
    const struct label *mp;
    int isopen = 0, flags = 0, key = 0;

    strxcpy(buf, string, sizeof(buf));
    for (cp = buf; *cp; ++cp) {
        if (*cp >= 'a' && *cp <= 'z') {         /* case conversion */
            *cp -= 'a' - 'A';
        }
    }

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wchar-subscripts"
#endif

    for (cp = buf; *cp;) {
        /* Open/Close */
        if (*cp == '<' && cp[1]) {
            if (0 == isopen) {
                isopen = 1;
                ++cp;
                continue;
            }
        }

        /* Whitespace */
        if (' ' == *cp || '\t' == *cp) {
            ++cp;
            continue;
        }

        /* Button keys, Button#[-[Up|Double|Motion|Down]] */
        if (0 == strncmp(cp, "BUTTON", 6) && isdigit(cp[6])) {
            const int button = atoi(cp += 6) - 1;

            if (button >= 0 && button <= 4) {
                if ('-' == *++cp) {
                    if (0 == strncmp(++cp, "UP", 2)) {
                        key = button + BUTTON1_UP;
                        cp += 2;
                    } else if (0 == strncmp(cp, "DOWN", 4)) {
                        key = button + BUTTON1_DOWN;
                        cp += 4;
                    } else if (0 == strncmp(cp, "DOUBLE", 6)) {
                        key = button + BUTTON1_DOUBLE;
                        cp += 6;
                    } else if (0 == strncmp(cp, "MOTION", 6)) {
                        key = button + BUTTON1_MOTION;
                        cp += 6;
                    }
                } else {
                    flags |= RANGE_BUTTON;
                    key = button;
                }
            }
            break;
        }

        /* Wheel keys, Wheel-[Up|Down] */
        if (0 == strncmp(cp, "WHEEL", 5)) {
            cp += 5;
            if ('-' == *cp) {
                if (0 == strncmp(++cp, "UP", 2)) {
                    key = WHEEL_UP;
                    cp += 2;
                } else if (0 == strncmp(cp, "DOWN", 4)) {
                    key = WHEEL_DOWN;
                    cp += 4;
                } else if (0 == strncmp(cp, "LEFT", 4)) {
                    key = WHEEL_LEFT;
                    cp += 4;
                } else if (0 == strncmp(cp, "RIGHT", 5)) {
                    key = WHEEL_RIGHT;
                    cp += 5;
                }
            }
            flags |= RANGE_MISC;
            break;
        }

        /* Function key, Fxx */
        if ('F' == *cp && isdigit(cp[1])) {
            flags |= RANGE_FUNCTION;
            key = (int)strtoul(cp + 1, &cp, 10) - 1;
            goto next;
        }

        /* Numeric value, #xxxx */
        if ('#' == *cp && isdigit(cp[1])) {
            key = (int)strtoul(cp + 1, &cp, 10);
            goto next;
        }

        /* Key labels/specials (Alt, Ctrl, etc) */
        for (mp = key_labels; *cp && mp->name; ++mp) {
            const unsigned klen = mp->length;

            if (0 == strncmp(mp->name, (const char *)cp, klen) &&
                        ('>' == cp[klen] || '-' == cp[klen] || 0 == cp[klen])) {
                cp += klen;

                if (mp->modifier == RANGE_KEYPAD && mp->value == 0) {
                    const struct keypad *kp, *kpend;
                    char *kcp = cp + 1;         // next element

                    if (isdigit(*kcp)) {
                        // Keypad-<Numeric>
                        flags |= RANGE_KEYPAD;
                        key |= (int)strtoul(kcp, &kcp, 10);

                    } else {
                        // Keypad-<Identifier>
                        for (kp = keypad_labels, kpend = kp + KEYPAD_LABELS; kp != kpend; ++kp) {
                            const unsigned kplen = kp->length;

                            if (0 == strncmp(kp->name, (const char *)kcp, kplen) &&
                                        ('>' == kcp[kplen] || '-' == kcp[kplen] || 0 == kcp[kplen])) {
                                const unsigned kpidx = (unsigned)(kp - keypad_labels);

                                cp = kcp + kplen;
                                flags |= RANGE_KEYPAD;
                                key |= kpidx;
                                break;
                            }
                        }
                    }

                } else {
                    flags |= mp->modifier;
                    key |= mp->value;
                }
                goto next;
            }
        }

        /* Single character */
        if (*cp) {
            key |= *cp++;
            break;
        }

        /* Delimiter or terminator */
next:;  if ('>' == *cp && isopen) {
            isopen = 0;
            ++cp;
        } else if ('-' == *cp) {
            ++cp;
        }
    }

    if ('>' == *cp && isopen) {
        isopen = 0;
        ++cp;
    }

    if (len) *len = (int)(cp - buf);

    /* Special mappings */
    if (KEY_TAB == key) {
        if (MOD_SHIFT == flags) {               /* Shift-Tab */
            return BACK_TAB;
        }
        if (MOD_CTRL == flags) {                /* Ctrl-Tab */
            return CTRL_TAB;
        }
        if (MOD_META == flags) {                /* Alt-Tab */
            return ALT_TAB;
        }
    }

    if (KEY_BACKSPACE == key) {
        if (MOD_SHIFT == flags) {               /* Shift-Backspace */
            return SHIFT_BACKSPACE;
        }
        if (MOD_CTRL == flags) {                /* Ctrl-Backspace */
            return CTRL_BACKSPACE;
        }
        if (MOD_META == flags) {                /* Alt-Backspace */
            return ALT_BACKSPACE;
        }
    }

    if ((RANGE_MASK & flags) == RANGE_KEYPAD && key >= '0' && key <= '9') {
        key -= '0';                             /* KP-# */
    }

    if (MOD_CTRL == flags && key >= 0x40 && key <= 0x7f) {
        return (key & 0x1f);                    /* Ctrl A-Z */
    }

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

    return flags | key;
}


/*  Function:           kbname_fromkey
 *      Convert a numeric key identifier into a printable string in the standard format.
 *
 *  Parameters:
 *      key - Key code.
 *      buf - Buffer to place generated name; 32 or greater, also see KBNAMELEN.
 *      buflen - Length of the buffer, in bytes.
 *
 *  Returns:
 *      Address of the description buffer.
 */
const char *
kbname_fromkey(KEY key, char *buf, unsigned buflen)
{
    char *bp;

    assert(buf && buflen >= 32);
    if (0 == key || NULL == buf) {
        if (buf) buf[0] = '\0';
        return buf;
    }

    if (IS_CHARACTER(key)) {
        key_aschar(key, buf);
        return buf;
    }

    buf[0] = '<';
    buf[1] = '\0';

    // Modifiers
    if (key & MOD_META) strxcat(buf, "Alt-", buflen);
    if (key & MOD_CTRL) strxcat(buf, "Ctrl-", buflen);
    if (key & MOD_SHIFT) strxcat(buf, "Shift-", buflen);

    // Key classes
    bp = buf + strlen(buf);
    switch (key & RANGE_MASK) {
    case RANGE_SPECIAL: {
            const char *desc = NULL;

            switch (key) {
            case MOUSE_XTERM_KEY:
                desc = "MouseXterm";
                break;
            case MOUSE_SGR_KEY:
                desc = "MouseSGR";
                break;
            case MOUSE_KEY:
                desc = "Mouse";
                break;
            case PASTE_BRACKETED_EVT:
                desc = "PasteBracketed";
                break;
            case MOUSE_FOCUSOUT_KEY:
                desc = "FosusOut";
                break;
            case MOUSE_FOCUSIN_KEY:
                desc = "FosusIn";
                break;
            default:
                sprintf(bp, "#%u", (unsigned)key);
                break;
            }
            if (desc) strcpy(bp, desc);
        }
        break;

    case RANGE_MISC: {
            const char *desc = NULL;

            switch (key) {
            case BACK_TAB:
                desc = "Back-Tab";
                break;
            case CTRL_TAB:
                desc = "Ctrl-Tab";
                break;
            case ALT_TAB:
                desc = "Alt-Tab";
                break;
            case SHIFT_BACKSPACE:
                desc = "Shift-Backspace";
                break;
            case CTRL_BACKSPACE:
                desc = "Ctrl-Backspace";
                break;
            case ALT_BACKSPACE:
                desc = "Alt-Backspace";
                break;
            case KEY_UNDO_CMD:
            case KEY_UNDO:
                desc = "Undo";
                break;
            case KEY_COPY_CMD:
            case KEY_COPY:
                desc = "Copy";
                break;
            case KEY_CUT_CMD:
            case KEY_CUT:
                desc = "Cut";
                break;
            case KEY_PASTE:
                desc = "Paste";
                break;
            case KEY_HELP:
                desc = "Help";
                break;
            case KEY_REDO:
                desc = "Redo";
                break;
            case KEY_SEARCH:
                desc = "Search";
                break;
            case KEY_REPLACE:
                desc = "Replace";
                break;
            case KEY_CANCEL:
                desc = "Cancel";
                break;
            case KEY_COMMAND:
                desc = "Command";
                break;
            case KEY_EXIT:
                desc = "Exit";
                break;
            case KEY_NEXT:
                desc = "Next";
                break;
            case KEY_PREV:
                desc = "Prev";
                break;
            case KEY_OPEN:
                desc = "Open";
                break;
            case KEY_SAVE:
                desc = "Save";
                break;
            case KEY_MENU:
                desc = "Menu";
                break;
            case KEY_BREAK:
                desc = "Break";
                break;
            case WHEEL_UP:
                desc = "Wheel-Up";
                break;
            case WHEEL_DOWN:
                desc = "Wheel-Down";
                break;
            case WHEEL_LEFT:
                desc = "Wheel-Left";
                break;
            case WHEEL_RIGHT:
                desc = "Wheel-Right";
                break;
            default:
                sprintf(bp, "#%u", (unsigned)key);
                break;
            }
            if (desc) {
                strcpy(bp, desc);
            }
        }
        break;

    case RANGE_CHARACTER: {
            if ((key & KEY_MASK) > 0xff) {
                sprintf(bp, "#%u", (unsigned)key);
            } else {
                const char *desc = NULL,
                    key8 = (char) (key & KEY_MASK);

                switch (key8) {
                case KEY_ENTER:
                    desc = "Enter";
                    break;
                case KEY_ESC:
                    desc = "Esc";
                    break;
                case KEY_BACKSPACE:
                    desc = "Backspace";
                    break;
                case KEY_TAB:
                    desc = "Tab";
                    break;
                case KEY_SPACE:
                    if (key & (MOD_META|MOD_CTRL|MOD_SHIFT)) {
                        desc = "Space";
                    }
                    break;
                default:
                    break;
                }

                if (desc) {
                    strcpy(bp, desc);
                    bp += strlen(desc);
                } else {
                    *bp++ = key8;
                    *bp = '\0';
                }
            }
        }
        break;

    case RANGE_PRIVATE:
        sprintf(bp, "Private-%u", (unsigned)(key & KEY_MASK));
        break;

    case RANGE_FUNCTION:
        sprintf(bp, "F%u", ((unsigned)key & KEY_MASK) + 1);
        break;

    case RANGE_BUTTON:
        key &= ~(MOD_META | MOD_CTRL | MOD_SHIFT);
        if (key >= BUTTON1_MOTION) {
            sprintf(bp, "Button%u-Motion", (unsigned)(key - BUTTON1_MOTION) + 1);

        } else if (key >= BUTTON1_UP) {
            sprintf(bp, "Button%u-Up", (unsigned)(key - BUTTON1_UP) + 1);

        } else if (key >= BUTTON1_DOUBLE) {
            sprintf(bp, "Button%u-Double", (unsigned)(key - BUTTON1_DOUBLE) + 1);

        } else {
            sprintf(bp, "Button%u-Down", (unsigned)(key - BUTTON1_DOWN) + 1);
        }
        break;

    case RANGE_MULTIKEY:
        if (NULL == x_multikeys || (key >= RANGE_MULTIKEY + x_multiseq)) {
            strcpy(bp, "Undefined");            /* out side range */

        } else {
            const char *cp = x_multikeys[key - RANGE_MULTIKEY];

            assert(cp);
            if (cp) {
                for (bp = buf; *cp; ++cp) {
                    bp = key_aschar(*cp & 0xff, bp);
                }
            }
            assert(strlen(buf) < buflen);
            return buf;
        }
        break;

    case RANGE_KEYPAD: {
            const unsigned kp = (unsigned)key & KEY_MASK;

            if (kp < KEYPAD_LABELS) {
                if (kp == (KEYPAD_5 & KEY_MASK) ||
                        kp >= (KEYPAD_PLUS & KEY_MASK)) {
                    strcpy(bp, "Keypad-");
                    bp += 7;
                }
                strcpy(bp, keypad_labels[kp].label);
                break;
            }
            strcpy(bp, "Keypad-");
            bp += 7;
        }
        /*FALLTHRU*/

    default:
        sprintf(bp, "#%u", (unsigned)key);
        break;
    }
    strcat(buf, ">");

    assert(strlen(buf) < buflen);
    return buf;
}


/*
 *  Function:       key_aschar
 *      Convert a single character into thier canonic notation.
 */
static char *
key_aschar(KEY key, char *buf)
{
    switch (key) {
    case KEY_ENTER:
        strcpy(buf, "<Enter>");
        break;

    case KEY_ESC:
        strcpy(buf, "<Esc>");
        break;

    case KEY_BACKSPACE:
        strcpy(buf, "<Backspace>");
        break;

    case KEY_TAB:
        strcpy(buf, "<Tab>");
        break;

    case KEY_SPACE:
        strcpy(buf, "<Space>");
        break;

    case '{': case '}':
    case '#':
        buf[0] = '<';
        buf[1] = (char)key;
        buf[2] = '>';
        buf[3] = '\0';
        break;

    case '<':
    case '\\':
        buf[0] = '\\';
        buf[1] = (char)key;
        buf[2] = '\0';
        break;

    default:
        if (key <= 0) {
            buf[1] = '\0';
            return buf;

        } else if (key < ' ') {
            sprintf(buf, "<Ctrl-%c>", key + '@');

        } else if (key < 0x7f) {
            buf[0] = (char) key;
            buf[1] = '\0';

        } else {
            sprintf(buf, "#%u", (unsigned)key);
        }
        break;
    }

    buf += strlen(buf);
    return buf;
}

/*end*/
