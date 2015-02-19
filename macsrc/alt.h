/* $Id: alt.h,v 1.9 2011/11/03 23:41:50 cvsuser Exp $
 * Key definitions
 *
 */

#ifndef __ALT_H
#define __ALT_H

/*--export--*/
/* The following scheme is used for encoding function keys. This
 * scheme is used because it simplifies converting ASCII key
 * names to the internal codes and vice-versa. Also we keep the
 * internal keycodes out of the ASCII range so users are free to
 * use those for input if necessary, e.g. on foreign language
 * keyboards.
 *
 * 0x000..0x0ff       ASCII             ASCII range.
 * 0x100..0x1ff       Fn keys           Support for upto 256
 *                                      unshifted function keys
 * 0x200..0x2ff       Keypad            Upto 256 keypad keys.
 * 0x300..0x3ff       Misc              Miscellaneous
 * 0x400..0x7ff       Multikey          Used when user does something
 *                                      like: assign_to_key("xyz", ..
 *                                      i.e. a multi-key stroke.
 * 0x800..0x8ff       Private           Private key definitions for
 *                                      users.
 * 0x900..0x91f       Button down       Mouse buttons
 * 0x920..0x93f       Button up         Mouse buttons
 * 0x940..0x95f       Pointer motion    Mouse buttons
 *
 * These ranges can be OR'ed with the following bits to indicate
 * a modifier key is in operation.
 *
 * 0x1000             SHIFT
 * 0x2000             CTRL    (Not used for ASCII range)
 * 0x4000             META
 *
 * Hopefully there is more than enough room for expansion
 * purposes and any new keys which appear on the keyboard. If
 * you use your own encodings you may or may not have trouble
 * with the int_to_key() and key_to_int() primitives which
 * understand this encoding scheme.
 *
 */

#if defined(WIN32)
#undef  MOD_SHIFT
#undef  MOS_CTRL
#endif
#if defined(KEY_DOWN) || defined(KEY_END) ||    /* [n]curses.h refinitions */ \
        defined(KEY_UNDO) || defined(KEY_COPY)
#undef  KEY_DOWN
#undef  KEY_LEFT
#undef  KEY_RIGHT
#undef  KEY_HOME
#undef  KEY_UP
#undef  KEY_END
#undef  KEY_DEL
#undef  KEY_COPY
#undef  KEY_UNDO
#undef  KEY_SEARCH
#undef  KEY_REPLACE
#undef  KEY_CANCEL
#undef  KEY_COMMAND
#undef  KEY_EXIT
#endif

/* Standard names
 */
#define ESC                     0x1b
#define BACKSPACE               0x08
#define TAB                     '\t'
#define ENTER                   '\r'

/* Macro to check whether key is a normal ASCII key.
 */
#define IS_ASCII(x)             ((x & ~KEY_MASK) == 0)
#define IS_MULTIKEY(x)          ((x) >= RANGE_MULTIKEY && (x) <= RANGE_MULTIKEY + MULTIKEY_SIZE)
#define KEY_MASK                0xff


/* Define the modifier bits.
 */
#define MOD_SHIFT               0x1000
#define MOD_CTRL                0x2000
#define MOD_META                0x4000


/* Define the ranges for the keys.
 */
#define RANGE_ASCII             0x000
#define RANGE_FN                0x100
#define RANGE_KEYPAD            0x200
#define RANGE_MISC              0x300
#define RANGE_MULTIKEY          0x400   /* 0x400 .. 0x7ff */
#define RANGE_PRIVATE           0x800   /* 0x800 .. 0x8ff */
#define RANGE_BUTTON            0x900
#define RANGE_MASK              0xf00

#define MULTIKEY_SIZE           0x400


/* Define some ASCII characters which we use in the code.
 */
#define CCHR(x)                 ((x) & 0x1f)
#define __CTRL(x)               ((x) & 0x1f)    /*???*/

#define CTRL_A                  CCHR('a')
#define CTRL_B                  CCHR('b')
#define CTRL_C                  CCHR('c')
#define CTRL_D                  CCHR('d')
#define CTRL_E                  CCHR('e')
#define CTRL_F                  CCHR('f')
#define CTRL_G                  CCHR('g')
#define CTRL_H                  CCHR('h')
#define CTRL_I                  CCHR('i')
#define CTRL_J                  CCHR('j')
#define CTRL_K                  CCHR('k')
#define CTRL_L                  CCHR('l')
#define CTRL_M                  CCHR('m')
#define CTRL_N                  CCHR('n')
#define CTRL_O                  CCHR('o')
#define CTRL_P                  CCHR('p')
#define CTRL_Q                  CCHR('q')
#define CTRL_R                  CCHR('r')
#define CTRL_S                  CCHR('s')
#define CTRL_T                  CCHR('t')
#define CTRL_U                  CCHR('u')
#define CTRL_V                  CCHR('v')
#define CTRL_W                  CCHR('w')
#define CTRL_X                  CCHR('x')
#define CTRL_Y                  CCHR('y')
#define CTRL_Z                  CCHR('z')


/* Function key definitions.
 */
#define F(x)                    (RANGE_FN + x - 1)
#define SF(x)                   (MOD_SHIFT | (RANGE_FN + x - 1))
#define CF(x)                   (MOD_CTRL  | (RANGE_FN + x - 1))
#define CSF(x)                  (MOD_CTRL  | MOD_SHIFT | (RANGE_FN + x - 1))
#define AF(x)                   (MOD_META  | (RANGE_FN + x - 1))


/* Alt-letter definitions.
 */
#define __ALT(x)                (MOD_META | x)

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


/* Alt and normal digit key.
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


/* Alt and normal digit key.
 */
#define ALT_MINUS               __ALT('-')
#define ALT_EQUALS              __ALT('=')


/* Keypad keys.
 */
#define __KEYPAD(x)             (RANGE_KEYPAD | x)
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


/* Aliases for the keypad keys. PC keyboard layout.
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


/* Control keypad keys.
 */
#define __CTRL_KEYPAD(x)        (MOD_CTRL | RANGE_KEYPAD | x)
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

#define __ALT_KEYPAD(x)         (MOD_META | RANGE_KEYPAD | x)
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


/* Shift keypad keys.
 */
#define __SHIFT_KEYPAD(x)       (MOD_SHIFT | RANGE_KEYPAD | x)
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


/* Aliases
 */
#define KEY_WDOWN               SHIFT_KEYPAD_2
#define KEY_WLEFT               SHIFT_KEYPAD_4
#define KEY_WRIGHT              SHIFT_KEYPAD_6
#define KEY_WUP                 SHIFT_KEYPAD_8

#define KEY_WLEFT2              CTRL_KEYPAD_4
#define KEY_WRIGHT2             CTRL_KEYPAD_6


/* Miscellaneous keys.
 */
#define MOUSE_KEY               (RANGE_MISC | 0)    /* Xterm Mouse, not really a key. */
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

#define WHEEL_UP                (RANGE_MISC | 31)   /* Mouse scroll wheel */
#define WHEEL_DOWN              (RANGE_MISC | 32)

/* Following are like 'events' for mouse key trapping.
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

#endif  /*__ALT_H*/

