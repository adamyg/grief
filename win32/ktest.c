#include <edidentifier.h>
__CIDENT_RCSID(gr_ktest_c,"$Id: ktest.c,v 1.4 2014/10/27 23:28:35 ayoung Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * keyboard interface test application.
 *
 * Copyright (c) 1998 - 2012, Adam Young.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License for more details.
 * ==end==
 */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "edalt.h"

static int              GetKey(int tmo);
static char *           code_to_keyname(int key);


void
main(void)
{
    int ch;

    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), ENABLE_WINDOW_INPUT|ENABLE_MOUSE_INPUT);
    do {
        ch = GetKey(0);
    } while (ch != CTRL_C && ch != KEY_ESC);
}


static DWORD
WaitTicks(DWORD stick)                          /* start tick */
{
    DWORD etick;                                /* end tick */

    etick = GetTickCount();
    if (etick >= stick) {                       /* normal case */
        return (etick - stick) + 1;
    }
    return (0xffffffff - stick) + 1 + etick;    /* ticks has wrapped */
}


#define VKEYS       ((int)(sizeof(vks)/sizeof(vks[0])))

static struct {
    WORD        vk;
    int         mods;
    const char *desc;
    int         vio;
} vks[] = {
    { VK_LBUTTON,   -1,         "LBUTTON",  -1 },
    { VK_RBUTTON,   -1,         "RBUTTON",  -1 },
    { VK_CANCEL,    -1,         "Cancel",   -1 },
    { VK_MBUTTON,   -1,         "MBUTTON",  -1 },

    { VK_BACK,      0,          "Back",     KEY_BACKSPACE },
    { VK_TAB,       0,          "TAB",      KEY_TAB },
    { VK_BACK,      MOD_SHIFT,  "S-Back",   SHIFT_BACKSPACE },
    { VK_TAB,       MOD_SHIFT,  "S-TAB",    BACK_TAB },
    { VK_BACK,      MOD_CTRL,   "C-Back",   CTRL_BACKSPACE  },
    { VK_TAB,       MOD_CTRL,   "C-TAB",    CTRL_TAB },
    { VK_BACK,      MOD_META,   "A-Back",   ALT_BACKSPACE  },
    { VK_TAB,       MOD_META,   "A-TAB",    ALT_TAB },

    { VK_CLEAR,     -1,         "Clear",    -1 },
    { VK_RETURN,    -1,         "Return",   -1 },

//  { VK_SHIFT,
//  { VK_CONTROL,
//  { VK_MENU,
//  { VK_PAUSE,
//  { VK_CAPITAL,

//  { VK_KANA,
//  { VK_HANGUL,
//  { VK_JUNJA,
//  { VK_FINAL,
//  { VK_HANJA,
//  { VK_KANJI,

    { VK_ESCAPE,    -1,         "ESC",      KEY_ESC },

//  { VK_CONVERT,
//  { VK_NONCONVERT,
//  { VK_ACCEPT,
//  { VK_MODECHANGE,

    { VK_PRIOR,     -1,         "PRIOR",    KEY_PAGEDOWN },
    { VK_NEXT,      -1,         "NEXT",     KEY_PAGEUP },
    { VK_END,       -1,         "END",      KEY_END },
    { VK_HOME,      -1,         "HOME",     KEY_HOME },
    { VK_LEFT,      -1,         "LEFT",     KEY_LEFT },
    { VK_UP,        -1,         "UP",       KEY_UP },
    { VK_RIGHT,     -1,         "RIGHT",    KEY_RIGHT },
    { VK_DOWN,      -1,         "DOWN",     KEY_DOWN },
    { VK_SELECT,    -1,         "SELECT",   -1 },
    { VK_PRINT,     -1,         "PRINT",    -1 },
    { VK_EXECUTE,   -1,         "EXEC",     -1 },
    { VK_SNAPSHOT,  -1,         "SNAPSHOT", -1 },
    { VK_INSERT,    -1,         "INSERT",   KEY_INS },
    { VK_DELETE,    -1,         "DELETE",   KEY_DEL },
    { VK_SUBTRACT,  -1,         "-",        KEYPAD_MINUS },

    { VK_NUMPAD0,   -1,         "Numpad0",  -1 },
    { VK_NUMPAD1,   -1,         "Numpad1",  -1 },
    { VK_NUMPAD2,   -1,         "Numpad2",  -1 },
    { VK_NUMPAD3,   -1,         "Numpad3",  -1 },
    { VK_NUMPAD4,   -1,         "Numpad4",  -1 },
    { VK_NUMPAD5,   -1,         "Numpad5",  -1 },
    { VK_NUMPAD6,   -1,         "Numpad6",  -1 },
    { VK_NUMPAD7,   -1,         "Numpad7",  -1 },
    { VK_NUMPAD8,   -1,         "Numpad8",  -1 },
    { VK_NUMPAD9,   -1,         "Numpad9",  -1 },
    { VK_MULTIPLY,  -1,         "*",        KEYPAD_STAR },
    { VK_ADD,       -1,         "+",        KEYPAD_PLUS },
    { VK_SEPARATOR, -1,         "Sep",      -1 },
    { VK_DECIMAL,   -1,         "Dec",      -1 },
    { VK_DIVIDE,    -1,         "/",        KEYPAD_DIV  },

    { VK_F1,        -1,         "F1",       F(1) },
    { VK_F2,        -1,         "F2",       F(2) },
    { VK_F3,        -1,         "F3",       F(3) },
    { VK_F4,        -1,         "F4",       F(4) },
    { VK_F5,        -1,         "F5",       F(5) },
    { VK_F6,        -1,         "F6",       F(6) },
    { VK_F7,        -1,         "F7",       F(7) },
    { VK_F8,        -1,         "F8",       F(8) },
    { VK_F9,        -1,         "F9",       F(9) },
    { VK_F10,       -1,         "F10",      F(1) },
    { VK_F11,       -1,         "F11",      F(11) },
    { VK_F12,       -1,         "F12",      F(12) },
    { VK_F13,       -1,         "F13",      F(13) },
    { VK_F14,       -1,         "F14",      F(14) },
    { VK_F15,       -1,         "F15",      F(15) },
    { VK_F16,       -1,         "F16",      F(16) },
    { VK_F17,       -1,         "F17",      F(17) },
    { VK_F18,       -1,         "F18",      F(18) },
    { VK_F19,       -1,         "F19",      F(19) },
    { VK_F20,       -1,         "F20",      F(20) },

    { VK_NUMLOCK,   -1,         "Numlock",  -1 },
    { VK_NUMLOCK,   -1,         "Scroll",   -1 },
    };


static int
GetKey(int tmo)
{
    HANDLE hKbd = GetStdHandle(STD_INPUT_HANDLE);
    DWORD ticks, tmticks;
    INPUT_RECORD k;
    DWORD count;
    DWORD rc;

    while (1) {

        if ((tmticks = tmo) == 0) {
            tmticks = INFINITE;                 /* block forever */
        } else if (tmo < 0) {
            tmticks = 0;                        /* no timeout */
        }

        ticks = GetTickCount();                 /* ticks (ms) as start */
        rc = WaitForSingleObject( hKbd, tmticks );
        ticks = WaitTicks( ticks );             /* ticks (ms) as end */

        if ( rc == WAIT_OBJECT_0 &&
                ReadConsoleInput( hKbd, &k, 1, &count ) ) {
            
            switch (k.EventType) {
            case KEY_EVENT:
                if ( k.Event.KeyEvent.bKeyDown )
                {
                    const KEY_EVENT_RECORD *pKey = &k.Event.KeyEvent;
                    int mod = 0, ch = -1, i;

                    printf( "KeyCode=0x%x, ScanCode=0x%x",
                        pKey->wVirtualKeyCode, pKey->wVirtualScanCode );

                    if (pKey->dwControlKeyState &
                              (LEFT_ALT_PRESSED   | RIGHT_ALT_PRESSED)) {
                        mod |= MOD_META;
                    }

                    if (pKey->dwControlKeyState &
                              (LEFT_CTRL_PRESSED  | RIGHT_CTRL_PRESSED)) {
                        mod |= MOD_CTRL;
                    }

                    if (pKey->dwControlKeyState & (SHIFT_PRESSED)) {
                        mod |= MOD_SHIFT;
                    }

                    for (i = VKEYS-1; i >= 0; i--)
                        if (vks[i].vk == pKey->wVirtualKeyCode &&
                                (vks[i].mods == -1 || vks[i].mods == mod)) {
                            printf( " (%s)", vks[i].desc );

                            if ((ch = vks[i].vio) >= 0) {
                                if (vks[i].mods == -1)
                                    ch |= mod;  // apply modifiers
                            }
                            break;
                        }

                    if (ch == -1 && pKey->uChar.AsciiChar) {
                        ch = pKey->uChar.AsciiChar;

                        if (mod == MOD_META) {
                            if (ch >= 'a' && ch <= 'z')
                                ch = toupper(ch);
                            ch |= MOD_META;
                        }
                    }

                    if (ch >= 0) {
                        printf( " key=%s\n", code_to_keyname(ch) );
                        return (ch);
                    }
                    printf( "\n" );
                }
                break;

            case MOUSE_EVENT:
                break;

            case WINDOW_BUFFER_SIZE_EVENT:
                break;

            case FOCUS_EVENT:
                break;

            case MENU_EVENT:
                break;

            default:                /* unknown event */
                break;
            }
        }

                                                /* timeout ? */
        if (tmo < 0 || (tmo > 0 && (tmo -= ticks) <= 0))
            break;
    }
    return (0);
}


/*
 *  Function to convert a numeric key identifier into a printable
 *  string in the standard format
 */
static const char * keypad_names[] = {
    "Ins",          /* 0           */
    "End",          /* 1           */
    "Down",         /* 2           */
    "PgDn",         /* 3           */
    "Left",         /* 4           */
    "5",            /* 5           */
    "Right",        /* 6           */
    "Home",         /* 7           */
    "Up",           /* 8           */
    "PgUp",         /* 9           */
    "Del",          /* 10 Delete   */
    "Plus",         /* 11 +        */
    "Minus",        /* 12 -        */
    "Star",         /* 13 *        */
    "Divide",       /* 14 /        */
    "Equals",       /* 15 =        */
    "Enter",        /* 16 <cr>     */
    "Pause",        /* 17          */
    "PrtSc",        /* 18          */
    "Scroll",       /* 19          */
    "NumLock"       /* 20          */
    };


static char *       key_to_char(char * buf, int key);

static char *
code_to_keyname(int key)
{
    static char buf[48];
    const char *cp;
    register char *bp;
    int i;

do_normal:
    if (IS_ASCII(key)) {
        key_to_char(buf, key);
        return buf;
    }
    buf[0] = '<';
    buf[1] = '\0';
    if (key & MOD_META)
        strcat(buf, "Alt-");
    if (key & MOD_CTRL)
        strcat(buf, "Ctrl-");
    if (key & MOD_SHIFT)
        strcat(buf, "Shift-");

    bp = buf + strlen(buf);
    switch (key & RANGE_MASK) {
    case RANGE_MISC:
        switch (key) {
        case MOUSE_KEY:
            strcpy(bp, "Mouse");
            break;
        case BACK_TAB:
            strcpy(bp, "Back-Tab");
            break;
        case CTRL_TAB:
            strcpy(bp, "Ctrl-Tab");
            break;
        case ALT_TAB:
            strcpy(bp, "Alt-Tab");
            break;
        case SHIFT_BACKSPACE:
            strcpy(bp, "Shift-Backspace");
            break;
        case CTRL_BACKSPACE:
            strcpy(bp, "Ctrl-Backspace");
            break;
        case ALT_BACKSPACE:
            strcpy(bp, "Alt-Backspace");
            break;
        case KEY_UNDO:
            strcpy(bp, "Undo");
            break;
        case KEY_COPY:
            strcpy(bp, "Copy");
            break;
        case KEY_CUT:
            strcpy(bp, "Cut");
            break;
        case KEY_PASTE:
            strcpy(bp, "Paste");
            break;
        case KEY_HELP:
            strcpy(bp, "Help");
            break;
        default:
            goto DEFAULT;
        }
        break;

    case RANGE_ASCII:
        *bp++ = (char) (key & KEY_MASK);
        *bp = '\0';
        break;

    case RANGE_PRIVATE:
        sprintf(bp, "Private-%d", key & KEY_MASK);
        break;

    case RANGE_FN:
        sprintf(bp, "F%d", (key & KEY_MASK) + 1);
        break;

    case RANGE_BUTTON:
        key &= ~(MOD_META | MOD_CTRL | MOD_SHIFT);
        if (key >= BUTTON1_MOTION) {
            sprintf(bp, "Button%d-motion", key - BUTTON1_MOTION + 1);
        } else if (key >= BUTTON1_UP) {
            sprintf(bp, "Button%d-up", key - BUTTON1_UP + 1);
        } else {
            sprintf(bp, "Button%d-down", key - BUTTON1_DOWN + 1);
        }
        break;

    case RANGE_KEYPAD:
        i = key & KEY_MASK;
        if (i < (int) (sizeof keypad_names / sizeof keypad_names[0])) {

            cp = keypad_names[i];
            if (isdigit(*cp) ||
                    (key & KEY_MASK) >= (KEYPAD_PLUS & KEY_MASK)) {
                strcpy(bp, "Keypad-");
                bp += 7;
            }

            while (*cp && *cp != '-')
                *bp++ = *cp++;
            *bp = '\0';
            break;
        }
        strcpy(bp, "Keypad-");
        bp += 7;
        /*FALLTHRU*/

    default:
DEFAULT:
        sprintf(bp, "#%d", key);
        break;
    }
    strcat(buf, ">");
    return buf;
}


static char *
key_to_char(char * buf, int key)
{
    switch (key) {
    case KEY_ENTER:
        strcpy(buf, "<Enter>");
        break;
    case KEY_ESC:
        strcpy(buf, "<Esc>");
        break;
    case '{':
        strcpy(buf, "<{>");
        break;
    case '}':
        strcpy(buf, "<}>");
        break;
    case ' ':
        strcpy(buf, "<Space>");
        break;
    case KEY_BACKSPACE:
        strcpy(buf, "<Backspace>");
        break;
    case KEY_TAB:
        strcpy(buf, "<Tab>");
        break;
    case '<':
    case '\\':
        buf[0] = '\\';
        buf[1] = key;
        buf[2] = '\0';
        break;
    default:
        if (key < ' ') {
            sprintf(buf, "<Ctrl-%c>", key + '@');

        } else if (key < 0x7f) {
            buf[0] = (char) key;
            buf[1] = '\0';

        } else {
            sprintf(buf, "#%d", key);
        }
        break;
    }
    buf += strlen(buf);
    return buf;
}
/*end*/
