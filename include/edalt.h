#ifndef GR_EDALT_H_INCLUDED
#define GR_EDALT_H_INCLUDED
#include <edidentifier.h>
__CIDENT_RCSID(gr_edalt_h,"$Id: edalt.h,v 1.38 2024/09/02 14:04:03 cvsuser Exp $")
__CPRAGMA_ONCE

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: edalt.h,v 1.38 2024/09/02 14:04:03 cvsuser Exp $
 * Key definitions.
 * ==noguard==
 *
 * Copyright (c) 1998 - 2024, Adam Young.
 * All rights reserved.
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * Redistributions of source code must retain the above copyright
 * notice, and must be distributed with the license document above.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, and must include the license document above in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

/*--export--*/
/*
 *  The following scheme is used for encoding function keys.
 *
 *  Unicode defines a codespace for 1,114,112 code points in range
 *  0 to 10FFFF leaving the top bits.
 *
 *  These are utilised for attributes are used to create seperate
 *  namespaces for UNICODE, FUNCTION and others.
 *
 *  Within these ranges can be OR'ed with the modifiers SHIFT, CTRL and META.
 *
 *      -----------------------------------------------------------------
 *      |   attributes        |             character                   |
 *      -----------------------------------------------------------------
 *      4 3 2 1 4 3 2 1 4 3 2   1 4 2 3 1 4 3 2 1 4 2 3 1 4 3 2 1 4 2 3 1
 *                              1 f . . . f . . . f . . . f . . . f . . .
 *
 *                          x   Shift                       - MOD_SHIFT
 *                        x     Ctrl/control                - MOD_CTRL
 *                      x       Meta                        - MOD_META
 *                    x         App                         - MOD_APP
 *      s . r r r r .           Character ranges/namespaces - RANGE_MASK
 *
 *              RANGE_CHARACTER, RANGE_FUNCTION, RANGE_KEYPAD,
 *              RANGE_MISC, RANGE_MULTIKEY, RANGE_PRIVATE, RANGE_BUTTON
 *
 *      s = sign/reserved.
 *      . = reserved/unused.
 *

typedef int32_t KEY;

 */

#if defined(_WIN32) || defined(WIN32)
#undef MOD_SHIFT
#undef MOS_CTRL
#endif
#if defined(KEY_DOWN) || defined(KEY_END) ||    /* [n]curses.h refinitions */ \
        defined(KEY_HELP) || defined(KEY_UNDO) || defined(KEY_COPY)
#undef KEY_COMMAND
#undef KEY_HELP
#undef KEY_MENU
#undef KEY_EXIT
#undef KEY_CANCEL
#undef KEY_COPY
#undef KEY_DEL
#undef KEY_UP
#undef KEY_DOWN
#undef KEY_HOME
#undef KEY_END
#undef KEY_LEFT
#undef KEY_RIGHT
#undef KEY_NEXT
#undef KEY_PREV
#undef KEY_OPEN
#undef KEY_SAVE
#undef KEY_CLOSE
#undef KEY_UNDO
#undef KEY_REDO
#undef KEY_SEARCH
#undef KEY_REPLACE
#undef KEY_ENTER
#undef KEY_BACKSPACE
#undef KEY_BREAK
#endif

/*
 *  Standard names
 */
#define __ESC                   0x1b
#define __BACKSPACE             0x08
#define __TAB                   '\t'
#define __ENTER                 '\r'

#define KEY_ESC                 __ESC
#define KEY_BACKSPACE           __BACKSPACE
#define KEY_TAB                 __TAB
#define KEY_ENTER               __ENTER
#define KEY_NEWLINE             '\n'
#define KEY_DELETE              0x7f

/*
 *  Namespaces and modifiers.
 */
#define KEY_MASK                0x001fffff      // 0..10ffff
#define RANGE_CHARACTER         0x00000000      // namespaces
#define RANGE_FUNCTION          (1 << 26)
#define RANGE_KEYPAD            (2 << 26)
#define RANGE_MISC              (3 << 26)
#define RANGE_MULTIKEY          (4 << 26)
#define RANGE_PRIVATE           (5 << 26)
#define RANGE_BUTTON            (6 << 26)
#define RANGE_SPECIAL           (7 << 26)
#define RANGE_MAX               (15 << 26)
#define RANGE_MASK              0x3c000000

#define MULTIKEY_SIZE           0x0400
#define IS_CHARACTER(x)         (((x) & ~KEY_MASK) == 0) // unmodified character.
#define IS_FUNCTION(x)          (((x) & RANGE_MASK) == RANGE_FUNCTION) // function key.
#define IS_MULTIKEY(x)          ((x) >= RANGE_MULTIKEY && (x) <= (RANGE_MULTIKEY + MULTIKEY_SIZE))
#define IS_BUTTON(x)            (((x) & RANGE_MASK) == RANGE_BUTTON) // mouse button.
#define IS_SPECIAL(x)           (((x) & RANGE_MASK) == RANGE_SPECIAL) // special events.

#define MOD_SHIFT               0x00200000      // modifiers
#define MOD_CTRL                0x00400000
#define MOD_META                0x00800000
#define MOD_APP                 0x01000000      // reserved
#define MOD_MASK                0x01e00000

/*
 *  Specials
 */
#define KEY_VOID                0x001fffff      // null
#define KEY_WINCH               0x001ffffe      // winch/resize event
#define KEY_UNASSIGNED          0x001ffffd      // unassigned hook
#define KEY_UNICODE             0x001ffff0      // keyboard special

/*
 *  Control characters
 */
#define __CTRLAZ(__x)           ((__x) & 0x1f)  /* Ctrl A-Z */
#define __CTRL(__x)             (MOD_CTRL | (__x))

#define CTRL_A                  __CTRLAZ('a')
#define CTRL_B                  __CTRLAZ('b')
#define CTRL_C                  __CTRLAZ('c')
#define CTRL_D                  __CTRLAZ('d')
#define CTRL_E                  __CTRLAZ('e')
#define CTRL_F                  __CTRLAZ('f')
#define CTRL_G                  __CTRLAZ('g')
#define CTRL_H                  __CTRLAZ('h')
#define CTRL_I                  __CTRLAZ('i')
#define CTRL_J                  __CTRLAZ('j')
#define CTRL_K                  __CTRLAZ('k')
#define CTRL_L                  __CTRLAZ('l')
#define CTRL_M                  __CTRLAZ('m')
#define CTRL_N                  __CTRLAZ('n')
#define CTRL_O                  __CTRLAZ('o')
#define CTRL_P                  __CTRLAZ('p')
#define CTRL_Q                  __CTRLAZ('q')
#define CTRL_R                  __CTRLAZ('r')
#define CTRL_S                  __CTRLAZ('s')
#define CTRL_T                  __CTRLAZ('t')
#define CTRL_U                  __CTRLAZ('u')
#define CTRL_V                  __CTRLAZ('v')
#define CTRL_W                  __CTRLAZ('w')
#define CTRL_X                  __CTRLAZ('x')
#define CTRL_Y                  __CTRLAZ('y')
#define CTRL_Z                  __CTRLAZ('z')

#define CTRL_0                  __CTRL('0')
#define CTRL_1                  __CTRL('1')
#define CTRL_2                  __CTRL('2')
#define CTRL_3                  __CTRL('3')
#define CTRL_4                  __CTRL('4')
#define CTRL_5                  __CTRL('5')
#define CTRL_6                  __CTRL('6')
#define CTRL_7                  __CTRL('7')
#define CTRL_8                  __CTRL('8')
#define CTRL_9                  __CTRL('9')

/*
 *  Function key definitions.
 */
#define F(__x)                  (RANGE_FUNCTION + (__x) - 1)
#define SF(__x)                 (MOD_SHIFT | (RANGE_FUNCTION + (__x) - 1))
#define CF(__x)                 (MOD_CTRL  | (RANGE_FUNCTION + (__x) - 1))
#define CSF(__x)                (MOD_CTRL  | MOD_SHIFT | (RANGE_FUNCTION + (__x) - 1))
#define AF(__x)                 (MOD_META  | (RANGE_FUNCTION + (__x) - 1))

/*
 *  Alt-letter definitions.
 */
#define __ALT(__x)              (MOD_META | (__x))

#define ALT_A                   __ALT('A')
#define ALT_B                   __ALT('B')
#define ALT_C                   __ALT('C')
#define ALT_D                   __ALT('D')
#define ALT_E                   __ALT('E')
#define ALT_F                   __ALT('F')
#define ALT_G                   __ALT('G')
#define ALT_H                   __ALT('H')
#define ALT_I                   __ALT('I')
#define ALT_J                   __ALT('J')
#define ALT_K                   __ALT('K')
#define ALT_L                   __ALT('L')
#define ALT_M                   __ALT('M')
#define ALT_N                   __ALT('N')
#define ALT_O                   __ALT('O')
#define ALT_P                   __ALT('P')
#define ALT_Q                   __ALT('Q')
#define ALT_R                   __ALT('R')
#define ALT_S                   __ALT('S')
#define ALT_T                   __ALT('T')
#define ALT_U                   __ALT('U')
#define ALT_V                   __ALT('V')
#define ALT_W                   __ALT('W')
#define ALT_X                   __ALT('X')
#define ALT_Y                   __ALT('Y')
#define ALT_Z                   __ALT('Z')

/*
 *  Alt and normal digit key.
 */
#define ALT_0                   __ALT('0')
#define ALT_1                   __ALT('1')
#define ALT_2                   __ALT('2')
#define ALT_3                   __ALT('3')
#define ALT_4                   __ALT('4')
#define ALT_5                   __ALT('5')
#define ALT_6                   __ALT('6')
#define ALT_7                   __ALT('7')
#define ALT_8                   __ALT('8')
#define ALT_9                   __ALT('9')

/*
 *  Alt and normal digit key.
 */
#define ALT_MINUS               __ALT('-')
#define ALT_EQUALS              __ALT('=')

/*
 *  Keypad keys.
 */
#define __KEYPAD(__x)           (RANGE_KEYPAD | (__x))
#define KEYPAD_0                __KEYPAD(0)
#define KEYPAD_1                __KEYPAD(1)
#define KEYPAD_2                __KEYPAD(2)
#define KEYPAD_3                __KEYPAD(3)
#define KEYPAD_4                __KEYPAD(4)
#define KEYPAD_5                __KEYPAD(5)
#define KEYPAD_6                __KEYPAD(6)
#define KEYPAD_7                __KEYPAD(7)
#define KEYPAD_8                __KEYPAD(8)
#define KEYPAD_9                __KEYPAD(9)
#define KEYPAD_DEL              __KEYPAD(10)
#define KEYPAD_PLUS             __KEYPAD(11)
#define KEYPAD_MINUS            __KEYPAD(12)
#define KEYPAD_STAR             __KEYPAD(13)
#define KEYPAD_DIV              __KEYPAD(14)
#define KEYPAD_EQUAL            __KEYPAD(15)
#define KEYPAD_ENTER            __KEYPAD(16)
#define KEYPAD_PAUSE            __KEYPAD(17)
#define KEYPAD_PRTSC            __KEYPAD(18)
#define KEYPAD_SCROLL           __KEYPAD(19)
#define KEYPAD_NUMLOCK          __KEYPAD(20)

/*
 *  Aliases for the keypad keys, PC keyboard layout.
 */
#define KEY_INS                 KEYPAD_0
#define KEY_END                 KEYPAD_1
#define KEY_DOWN                KEYPAD_2
#define KEY_PAGEDOWN            KEYPAD_3
#define KEY_LEFT                KEYPAD_4
#define KEY_RIGHT               KEYPAD_6
#define KEY_HOME                KEYPAD_7
#define KEY_UP                  KEYPAD_8
#define KEY_PAGEUP              KEYPAD_9
#define KEY_DEL                 KEYPAD_DEL
#define KEY_COPY                KEYPAD_PLUS
#define KEY_CUT                 KEYPAD_MINUS
#define KEY_UNDO                KEYPAD_STAR


/*
 *  Control keypad keys.
 */
#define __CTRL_KEYPAD(__x)      (MOD_CTRL | RANGE_KEYPAD | (__x))
#define CTRL_KEYPAD_0           __CTRL_KEYPAD(0)
#define CTRL_KEYPAD_1           __CTRL_KEYPAD(1)
#define CTRL_KEYPAD_2           __CTRL_KEYPAD(2)
#define CTRL_KEYPAD_3           __CTRL_KEYPAD(3)
#define CTRL_KEYPAD_4           __CTRL_KEYPAD(4)
#define CTRL_KEYPAD_5           __CTRL_KEYPAD(5)
#define CTRL_KEYPAD_6           __CTRL_KEYPAD(6)
#define CTRL_KEYPAD_7           __CTRL_KEYPAD(7)
#define CTRL_KEYPAD_8           __CTRL_KEYPAD(8)
#define CTRL_KEYPAD_9           __CTRL_KEYPAD(9)
#define CTRL_KEYPAD_DEL         __CTRL_KEYPAD(10)
#define CTRL_KEYPAD_PLUS        __CTRL_KEYPAD(11)
#define CTRL_KEYPAD_MINUS       __CTRL_KEYPAD(12)
#define CTRL_KEYPAD_STAR        __CTRL_KEYPAD(13)
#define CTRL_KEYPAD_DIV         __CTRL_KEYPAD(14)
#define CTRL_KEYPAD_EQUAL       __CTRL_KEYPAD(15)
#define CTRL_KEYPAD_ENTER       __CTRL_KEYPAD(16)
#define CTRL_KEYPAD_PAUSE       __CTRL_KEYPAD(17)
#define CTRL_KEYPAD_PRTSC       __CTRL_KEYPAD(18)
#define CTRL_KEYPAD_SCROLL      __CTRL_KEYPAD(19)
#define CTRL_KEYPAD_NUMLOCK     __CTRL_KEYPAD(20)

#define __ALT_KEYPAD(__x)       (MOD_META | RANGE_KEYPAD | (__x))
#define ALT_KEYPAD_0            __ALT_KEYPAD(0)
#define ALT_KEYPAD_1            __ALT_KEYPAD(1)
#define ALT_KEYPAD_2            __ALT_KEYPAD(2)
#define ALT_KEYPAD_3            __ALT_KEYPAD(3)
#define ALT_KEYPAD_4            __ALT_KEYPAD(4)
#define ALT_KEYPAD_5            __ALT_KEYPAD(5)
#define ALT_KEYPAD_6            __ALT_KEYPAD(6)
#define ALT_KEYPAD_7            __ALT_KEYPAD(7)
#define ALT_KEYPAD_8            __ALT_KEYPAD(8)
#define ALT_KEYPAD_9            __ALT_KEYPAD(9)

#define ALT_KEYPAD_END          __ALT_KEYPAD(1)
#define ALT_KEYPAD_LEFT         __ALT_KEYPAD(4)
#define ALT_KEYPAD_RIGHT        __ALT_KEYPAD(6)
#define ALT_KEYPAD_HOME         __ALT_KEYPAD(7)
#define ALT_KEYPAD_DEL          __ALT_KEYPAD(10)
#define ALT_KEYPAD_PLUS         __ALT_KEYPAD(11)
#define ALT_KEYPAD_MINUS        __ALT_KEYPAD(12)
#define ALT_KEYPAD_STAR         __ALT_KEYPAD(13)
#define ALT_KEYPAD_DIV          __ALT_KEYPAD(14)
#define ALT_KEYPAD_EQUAL        __ALT_KEYPAD(15)
#define ALT_KEYPAD_ENTER        __ALT_KEYPAD(16)
#define ALT_KEYPAD_PAUSE        __ALT_KEYPAD(17)
#define ALT_KEYPAD_PRTSC        __ALT_KEYPAD(18)
#define ALT_KEYPAD_SCROLL       __ALT_KEYPAD(19)
#define ALT_KEYPAD_NUMLOCK      __ALT_KEYPAD(20)

/*
 *  Shift keypad keys.
 */
#define __SHIFT_KEYPAD(__x)     (MOD_SHIFT | RANGE_KEYPAD | (__x))
#define SHIFT_KEYPAD_0          __SHIFT_KEYPAD(0)
#define SHIFT_KEYPAD_1          __SHIFT_KEYPAD(1)
#define SHIFT_KEYPAD_2          __SHIFT_KEYPAD(2)
#define SHIFT_KEYPAD_3          __SHIFT_KEYPAD(3)
#define SHIFT_KEYPAD_4          __SHIFT_KEYPAD(4)
#define SHIFT_KEYPAD_5          __SHIFT_KEYPAD(5)
#define SHIFT_KEYPAD_6          __SHIFT_KEYPAD(6)
#define SHIFT_KEYPAD_7          __SHIFT_KEYPAD(7)
#define SHIFT_KEYPAD_8          __SHIFT_KEYPAD(8)
#define SHIFT_KEYPAD_9          __SHIFT_KEYPAD(9)
#define SHIFT_KEYPAD_DEL        __SHIFT_KEYPAD(10)
#define SHIFT_KEYPAD_PLUS       __SHIFT_KEYPAD(11)
#define SHIFT_KEYPAD_MINUS      __SHIFT_KEYPAD(12)
#define SHIFT_KEYPAD_STAR       __SHIFT_KEYPAD(13)
#define SHIFT_KEYPAD_DIV        __SHIFT_KEYPAD(14)
#define SHIFT_KEYPAD_EQUAL      __SHIFT_KEYPAD(15)
#define SHIFT_KEYPAD_ENTER      __SHIFT_KEYPAD(16)
#define SHIFT_KEYPAD_PAUSE      __SHIFT_KEYPAD(17)
#define SHIFT_KEYPAD_PRTSC      __SHIFT_KEYPAD(18)
#define SHIFT_KEYPAD_SCROLL     __SHIFT_KEYPAD(19)
#define SHIFT_KEYPAD_NUMLOCK    __SHIFT_KEYPAD(20)

/*
 *  Aliases.
 */
#define KEY_WDOWN               SHIFT_KEYPAD_2
#define KEY_WLEFT               SHIFT_KEYPAD_4
#define KEY_WRIGHT              SHIFT_KEYPAD_6
#define KEY_WUP                 SHIFT_KEYPAD_8

#define KEY_WDOWN2              CTRL_KEYPAD_2
#define KEY_WLEFT2              CTRL_KEYPAD_4
#define KEY_WRIGHT2             CTRL_KEYPAD_6
#define KEY_WUP2                CTRL_KEYPAD_8

/*
 *  Special events, not keys.
 */
#define MOUSE_XTERM_KEY         (RANGE_SPECIAL | 1)
#define MOUSE_SGR_KEY           (RANGE_SPECIAL | 2)
#define PASTE_BRACKETED_EVT     (RANGE_SPECIAL | 10)
#define MOUSE_FOCUSOUT_KEY      (RANGE_SPECIAL | 11)
#define MOUSE_FOCUSIN_KEY       (RANGE_SPECIAL | 12)

/*
 *  Miscellaneous keys.
 */
#define BACK_TAB                (RANGE_MISC | 1)
#define CTRL_TAB                (RANGE_MISC | 2)
#define ALT_TAB                 (RANGE_MISC | 3)
#define SHIFT_BACKSPACE         (RANGE_MISC | 4)
#define CTRL_BACKSPACE          (RANGE_MISC | 5)
#define ALT_BACKSPACE           (RANGE_MISC | 6)

#define KEY_UNDO_CMD            (RANGE_MISC | 7)
#define KEY_COPY_CMD            (RANGE_MISC | 8)
#define KEY_CUT_CMD             (RANGE_MISC | 9)
#define KEY_PASTE               (RANGE_MISC | 10)
#define KEY_HELP                (RANGE_MISC | 11)
#define KEY_REDO                (RANGE_MISC | 12)
#define KEY_SEARCH              (RANGE_MISC | 13)
#define KEY_REPLACE             (RANGE_MISC | 14)
#define KEY_CANCEL              (RANGE_MISC | 15)
#define KEY_COMMAND             (RANGE_MISC | 16)
#define KEY_EXIT                (RANGE_MISC | 17)
#define KEY_NEXT                (RANGE_MISC | 18)
#define KEY_PREV                (RANGE_MISC | 19)
#define KEY_OPEN                (RANGE_MISC | 20)
#define KEY_SAVE                (RANGE_MISC | 21)
#define KEY_MENU                (RANGE_MISC | 22)
#define KEY_BREAK               (RANGE_MISC | 23)

#define WHEEL_UP                (RANGE_MISC | 31)
#define WHEEL_DOWN              (RANGE_MISC | 32)
#define WHEEL_LEFT              (RANGE_MISC | 33)
#define WHEEL_RIGHT             (RANGE_MISC | 34)

/*
 *  Mouse events.
 */
#define BUTTON1_DOWN            (RANGE_BUTTON | 0x00)
#define BUTTON2_DOWN            (RANGE_BUTTON | 0x01)
#define BUTTON3_DOWN            (RANGE_BUTTON | 0x02)
#define BUTTON4_DOWN            (RANGE_BUTTON | 0x03)
#define BUTTON5_DOWN            (RANGE_BUTTON | 0x04)

#define BUTTON1_DOUBLE          (RANGE_BUTTON | 0x10)
#define BUTTON2_DOUBLE          (RANGE_BUTTON | 0x11)
#define BUTTON3_DOUBLE          (RANGE_BUTTON | 0x12)
#define BUTTON4_DOUBLE          (RANGE_BUTTON | 0x13)
#define BUTTON5_DOUBLE          (RANGE_BUTTON | 0x14)

#define BUTTON1_UP              (RANGE_BUTTON | 0x20)
#define BUTTON2_UP              (RANGE_BUTTON | 0x21)
#define BUTTON3_UP              (RANGE_BUTTON | 0x22)
#define BUTTON4_UP              (RANGE_BUTTON | 0x23)
#define BUTTON5_UP              (RANGE_BUTTON | 0x24)

#define BUTTON1_MOTION          (RANGE_BUTTON | 0x40)
#define BUTTON2_MOTION          (RANGE_BUTTON | 0x41)
#define BUTTON3_MOTION          (RANGE_BUTTON | 0x42)
#define BUTTON4_MOTION          (RANGE_BUTTON | 0x43)
#define BUTTON5_MOTION          (RANGE_BUTTON | 0x44)

#define BUTTON_LINEUP           (RANGE_BUTTON | 0x50)
#define BUTTON_LINEDOWN         (RANGE_BUTTON | 0x51)
#define BUTTON_PAGEUP           (RANGE_BUTTON | 0x52)
#define BUTTON_PAGEDOWN         (RANGE_BUTTON | 0x53)
#define BUTTON_TOTOP            (RANGE_BUTTON | 0x54)
#define BUTTON_TOEND            (RANGE_BUTTON | 0x55)
#define BUTTON_DRAG             (RANGE_BUTTON | 0x56)

/*--end--*/

#endif /*GR_EDALT_H_INCLUDED*/
