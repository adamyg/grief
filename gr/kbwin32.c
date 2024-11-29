#include <edidentifier.h>
__CIDENT_RCSID(gr_kbwin32_c,"$Id: kbwin32.c,v 1.1 2024/11/18 13:42:22 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: kbwin32.c,v 1.1 2024/11/18 13:42:22 cvsuser Exp $
 * Manipulate key maps and bindings.
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

#include "keyboard.h"
#include "debug.h"

#if defined(WIN32) || defined(__CYGWIN__)
/*
 *  WIN32 Keyboard mapping table:
 *
 *  Notes:
 *      Enhanced keys for the IBM 101- and 102-key keyboards are the INS, DEL, HOME,
 *      END, PAGE UP, PAGE DOWN, and direction keys in the clusters to the left of the
 *      keypad; and the divide (/) and ENTER keys in the keypad.
 */
static const struct w32key {
    WORD                vk;                     /* windows virtual key code */
    int32_t             mods;                   /* modifiers */
#define VKMOD_ANY           -1
#define VKMOD_ENHANCED      -2
#define VKMOD_NONENHANCED   -3
#define VKMOD_NONSHIFT      -4

    const char *        desc;                   /* description */
    KEY                 code;                   /* interval key value */

} w32Keys[] = {
    // Only reportsd as an up event, down redirected to event handler.
//  { VK_CANCEL,        MOD_CTRL,           "Ctrl-Break",       KEY_BREAK },

//  { VK_KANA,                              "IME Kana mode",    0 },
//  { VK_HANGUL,                            "IME Hangul mode",  0 },
//  { VK_IME_ON,                            "IME On",           0 },
//  { VK_JUNJA,                             "IME Junja mode",   0 },
//  { VK_FINAL,                             "IME final mode",   0 },
//  { VK_HANJA,                             "IME Hanja mode",   0 },
//  { VK_KANJI,                             "IME Kanji mode",   0 },
//  { VK_IME_OFF,                           "IME Off",          0 },
//  { VK_CONVERT,                           "IME convert",      0 },
//  { VK_NONCONVERT,                        "IME nonconvert",   0 },
//  { VK_ACCEPT,                            "IME accept",       0 },
//  { VK_MODECHANGE,                        "IME mode change",  0 },

    { VK_BACK,          0,                  "Back",             KEY_BACKSPACE },
    { VK_TAB,           0,                  "Tab",              KEY_TAB },
    { VK_BACK,          MOD_SHIFT,          "Shift-Backspace",  SHIFT_BACKSPACE },
    { VK_TAB,           MOD_SHIFT,          "Shift-Tab",        BACK_TAB },
    { VK_BACK,          MOD_CTRL,           "Ctrl-Backspace",   CTRL_BACKSPACE },
    { VK_TAB,           MOD_CTRL,           "Ctrl-Tab",         CTRL_TAB },
    { VK_BACK,          MOD_META,           "Alt-Backspace",    ALT_BACKSPACE },
    { VK_TAB,           MOD_META,           "Alt-Tab",          ALT_TAB },
    { VK_ESCAPE,        VKMOD_ANY,          "Esc",              KEY_ESC },
    { VK_RETURN,        VKMOD_ANY,          "Return",           KEY_ENTER },
    { VK_RETURN,        VKMOD_ENHANCED,     "Return",           KEYPAD_ENTER },
    { VK_PAUSE,         VKMOD_ANY,          "Pause",            KEYPAD_PAUSE },
    { VK_PRIOR,         VKMOD_ANY,          "Prior",            KEY_PAGEUP },
    { VK_NEXT,          VKMOD_ANY,          "Next",             KEY_PAGEDOWN },
    { VK_END,           VKMOD_ANY,          "End",              KEY_END },
    { VK_HOME,          VKMOD_ANY,          "Home",             KEY_HOME },
    { VK_LEFT,          VKMOD_ANY,          "Left",             KEY_LEFT },
    { VK_UP,            VKMOD_ANY,          "Uo",               KEY_UP },
    { VK_RIGHT,         VKMOD_ANY,          "Right",            KEY_RIGHT },
    { VK_DOWN,          VKMOD_ANY,          "Down",             KEY_DOWN },
    { VK_INSERT,        VKMOD_ANY,          "Insert",           KEY_INS },
    { VK_DELETE,        VKMOD_ANY,          "Delete",           KEY_DEL },
    { VK_HELP,          VKMOD_ANY,          "Help",             KEY_HELP },

    /* VK_NUMPAD1 thru VK_NUMPAD0 are ignored allowing user selection via the NumLock */

//  { VK_PRIOR,         VKMOD_NONENHANCED,  "Keypad-PgUp",      KEYPAD_PAGEUP },
//  { VK_NEXT,          VKMOD_NONENHANCED,  "Keypad-PgDn",      KEYPAD_PAGEDOWN },
//  { VK_END,           VKMOD_NONENHANCED,  "Keypad-End",       KEYPAD_END },
//  { VK_HOME,          VKMOD_NONENHANCED,  "Keypad-Home",      KEYPAD_HOME },
//  { VK_LEFT,          VKMOD_NONENHANCED,  "Keypad-Left",      KEYPAD_LEFT },
//  { VK_CLEAR,         VKMOD_NONENHANCED,  "Keypad-5",         KEYPAD_5 },
//  { VK_UP,            VKMOD_NONENHANCED,  "Keypad-Up",        KEYPAD_UP },
//  { VK_RIGHT,         VKMOD_NONENHANCED,  "Keypad-Right",     KEYPAD_RIGHT },
//  { VK_DOWN,          VKMOD_NONENHANCED,  "Keypad-Down",      KEYPAD_DOWN },
//  { VK_INSERT,        VKMOD_NONENHANCED,  "Keypad-Ins",       KEYPAD_INS },
//  { VK_DELETE,        VKMOD_NONENHANCED,  "Keypad-Delete",    KEYPAD_DEL },
    { VK_SUBTRACT,      VKMOD_ANY,          "Keypad-Minus",     KEYPAD_MINUS },
    { VK_MULTIPLY,      VKMOD_ANY,          "Keypad-Star",      KEYPAD_STAR },
    { VK_ADD,           VKMOD_ANY,          "Keypad-Plus",      KEYPAD_PLUS },
    { VK_DIVIDE,        VKMOD_ANY,          "Keypad-Divide",    KEYPAD_DIV },

    /* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */

    { 0x30,             MOD_CTRL,           "0",                CTRL_0 },
    { 0x31,             MOD_CTRL,           "1",                CTRL_1 },
    { 0x32,             MOD_CTRL,           "2",                CTRL_2 },
    { 0x33,             MOD_CTRL,           "3",                CTRL_3 },
    { 0x34,             MOD_CTRL,           "4",                CTRL_4 },
    { 0x35,             MOD_CTRL,           "5",                CTRL_5 },
    { 0x36,             MOD_CTRL,           "6",                CTRL_6 },
    { 0x37,             MOD_CTRL,           "7",                CTRL_7 },
    { 0x38,             MOD_CTRL,           "8",                CTRL_8 },
    { 0x39,             MOD_CTRL,           "9",                CTRL_9 },

    /* VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A) */

    { VK_F1,            VKMOD_ANY,          "F1",               F(1) },
    { VK_F2,            VKMOD_ANY,          "F2",               F(2) },
    { VK_F3,            VKMOD_ANY,          "F3",               F(3) },
    { VK_F4,            VKMOD_ANY,          "F4",               F(4) },
    { VK_F5,            VKMOD_ANY,          "F5",               F(5) },
    { VK_F6,            VKMOD_ANY,          "F6",               F(6) },
    { VK_F7,            VKMOD_ANY,          "F7",               F(7) },
    { VK_F8,            VKMOD_ANY,          "F8",               F(8) },
    { VK_F9,            VKMOD_ANY,          "F9",               F(9) },
    { VK_F10,           VKMOD_ANY,          "F10",              F(10) },
    { VK_F11,           VKMOD_ANY,          "F11",              F(11) },
    { VK_F12,           VKMOD_ANY,          "F12",              F(12) },
    { VK_F13,           VKMOD_ANY,          "F13",              F(13) },
    { VK_F14,           VKMOD_ANY,          "F14",              F(14) },
    { VK_F15,           VKMOD_ANY,          "F15",              F(15) },
    { VK_F16,           VKMOD_ANY,          "F16",              F(16) },
    { VK_F17,           VKMOD_ANY,          "F17",              F(17) },
    { VK_F18,           VKMOD_ANY,          "F18",              F(18) },
    { VK_F19,           VKMOD_ANY,          "F19",              F(19) },
    { VK_F20,           VKMOD_ANY,          "F20",              F(20) },

    { VK_NUMLOCK,       VKMOD_ANY,          "Numlock",          KEYPAD_NUMLOCK },
    { VK_SCROLL,        VKMOD_ANY,          "Scroll",           KEYPAD_SCROLL },

//  { VK_OEM_1,                             // ';:' for US
//  { VK_OEM_PLUS,      VKMOD_NONSHIFT,     "+"                 '+' },
//  { VK_OEM_COMMA,     VKMOD_NONSHIFT,     ","                 ',' },
//  { VK_OEM_MINUS,     VKMOD_NONSHIFT,     "-"                 '-' },
//  { VK_OEM_PERIOD,    VKMOD_NONSHIFT,     "."                 '.' },
//  { VK_OEM_2,                             // '/?' for US
//  { VK_OEM_3,         VKMOD_NONSHIFT,     "~",                '~' },
//  { VK_OEM_4,                             //  '[{' for US
//  { VK_OEM_5,                             //  '\|' for US
//  { VK_OEM_6,                             //  ']}' for US
//  { VK_OEM_7,                             //  ''"' for US

#ifndef VK_OEM_NEC_EQUAL
#define VK_OEM_NEC_EQUAL    0x92
#endif
#ifndef VK_ICO_CLEAR
#define VK_ICO_CLEAR        0xE6
#endif
#ifndef VK_ICO_HELP
#define VK_ICO_HELP         0xE3
#endif
#ifndef VK_OEM_CLEAR
#define VK_OEM_CLEAR        0xFE
#endif

    { VK_OEM_NEC_EQUAL, VKMOD_ANY,          "Keypad-Equal",     KEYPAD_EQUAL },
//  { VK_ICO_CLEAR,     VKMOD_ANY,          "Clear",            KEYPAD_CLEAR },
    { VK_ICO_HELP,      VKMOD_ANY,          "Help",             KEY_HELP },
//  { VK_OEM_CLEAR,     VKMOD_ANY,          "Clear",            KEYPAD_CLEAR },

    };
#endif  /*WIN32 || __CYGWIN__*/


#if defined(WIN32) || defined(__CYGWIN__)
/*  Function:           key_mapwin32
 *      Translate the key press into a GRIEF identifier.
 *
 *  Parameters:
 *      dwCtrlKeyState - Control key status.
 *      wVirtKeyCode - Virtual key code.
 *      CharCode - Character code, if any.
 *
 *  Results:
 *      nothing
 */
int
key_mapwin32(unsigned dwCtrlKeyState, unsigned wVirtKeyCode, unsigned CharCode)
{
    const struct w32key *key = w32Keys + _countof(w32Keys);
    int mod = 0, ch = -1;

    /* modifiers */
    if (dwCtrlKeyState &
            (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) {
        mod |= MOD_META;
    }

    if (dwCtrlKeyState &
            (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) {
        mod |= MOD_CTRL;
    }

    if (dwCtrlKeyState & (SHIFT_PRESSED)) {
        mod |= MOD_SHIFT;
    }

    /* virtual keys */
    while (--key >= w32Keys) {
        if (key->vk == wVirtKeyCode &&
                ((key->mods == VKMOD_ANY) ||
                 (key->mods == VKMOD_ENHANCED    && 0 != (dwCtrlKeyState & (ENHANCED_KEY))) ||
                 (key->mods == VKMOD_NONENHANCED && 0 == (dwCtrlKeyState & (ENHANCED_KEY))) ||
                 (key->mods == VKMOD_NONSHIFT    && 0 == (dwCtrlKeyState & (SHIFT_PRESSED))) ||
                 (key->mods >= 0 && key->mods == mod) )) {
            if ((ch = key->code) >= 0) {
                if (key->mods == VKMOD_ANY) {
                    ch |= mod;                  /* apply modifiers */
                }
            }
            break;
        }
    }

    /* ascii */
    assert((CharCode & ~KEY_MASK) == 0);
    if (-1 == ch && (CharCode & KEY_MASK)) {
        ch = (CharCode & KEY_MASK);             /* UNICODE value */

        if (MOD_META == mod || (MOD_META|MOD_SHIFT) == mod) {
            /*
             *  Special handling for ALT-ASCII ..
             *  other modifiers SHIFT and CONTROL are already applied to the ASCII value.
             */
            if (ch >= 'a' && ch <= 'z') {
                ch = toupper(ch);
            }
            ch |= MOD_META;
        }
    }

#if !defined(KBPROTOCOLS_TEST)
    if (ch != -1) {
        trace_log("W32KEY %c%c%c = %d/0x%x (%s=%s)\n",
            (mod & MOD_META ? 'M' : '.'), (mod & MOD_CTRL ? 'C' : '.'), (mod & MOD_SHIFT ? 'S' : '.'),
                ch, ch, (key >= w32Keys ? key->desc : "ASCII"), key_code2name(ch));
    }
#endif

    return ch;
}
#endif  /*WIN32 || __CYGWIN__*/

/*end*/
