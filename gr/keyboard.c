#include <edidentifier.h>
__CIDENT_RCSID(gr_keyboard_c,"$Id: keyboard.c,v 1.72 2024/09/08 17:07:39 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: keyboard.c,v 1.72 2024/09/08 17:07:39 cvsuser Exp $
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

#include "accum.h"
#include "border.h"
#include "buffer.h"
#include "builtin.h"
#include "debug.h"
#include "echo.h"
#include "eval.h"
#include "getkey.h"
#include "lisp.h"
#include "macros.h"
#include "main.h"
#include "mouse.h"
#include "object.h"
#include "playback.h"
#include "register.h"                           /* trigger() */
#include "symbol.h"
#include "system.h"
#include "tty.h"
#include "ttyutil.h"
#include "undo.h"
#include "window.h"
#include "word.h"


/*
 *  NBLOCK defines the blocking sizes used during buffer allocation.
 */
#define NBLOCK          128                     /* keyboard buffer block chunk size */
#define LBLK(x)         ((int) (NBLOCK - 1 + (x)) & ~(NBLOCK-1))


/*
 *  Keyboard object
 */
typedef TAILQ_HEAD(_KeyboardList, _keyboard)
                        KEYBOARDLIST_t;

typedef struct _keyboard {
    MAGIC_t             kt_magic;               /* structure magic. */
#define KEYBOARD_MAGIC          MKMAGIC('K','y','B','d')
    TAILQ_ENTRY(_keyboard)
                        kt_node;                /* list node. */
    IDENTIFIER_t        kt_ident;               /* keyboard identifier. */
    int                 kt_refs;                /* number of references to this keyboard. */
    const char *        kt_name;                /* assigned name. */
    stype_t *           kt_macros;              /* macro assignments per key-stroke. */
    int                 kt_pushed;              /* push state. */
} keyboard_t;

#define IS_UNICODE(x)   (IS_CHARACTER(x) && x > 0xff)

#define HIST_DEPTH      16                      /* AUTOCONF - configuration item. */
#define HIST_NAME       64                      /* AUTOCONF */

static int              x_histhead = 0;         /* command history. */
static char             x_histbuf[HIST_DEPTH][HIST_NAME] = {0};

static IDENTIFIER_t     x_kbdident;             /* keyboard identifier sequence. */
static keyboard_t *     x_kbdcur;               /* current keyboard table. */

static KEYBOARDLIST_t   x_kbdlist;              /* active keyboards. */
static Head_p           x_kbdstack;             /* keyboard stack. */

static SPTREE *         x_kseqtree;             /* tree of key assignments. */

static int              x_multiseq = 0;         /* unique multi-key identifiers. */
static char **          x_multitbl;             /* multi-keys definition table. */

ref_t *                 x_push_ref = NULL;      /* keyboard push back buffer. */

int32_t                 x_character = 0;        /* current character typed. */


static keyboard_t *     keyboard_new(void);
static void             keyboard_delete(keyboard_t *kp);
static void             keyboard_free(keyboard_t *kp);
static keyboard_t *     keyboard_find(const int kbdid, int incref);
static void             keyboard_inq(const char *key, const keyboard_t *kp, char *buf, int buflen);

static void             keyboard_push(keyboard_t *bp);
static keyboard_t *     keyboard_pop(void);

static object_t *       key_macro_add(int key, const char *value);
static object_t *       key_macro_push(int key, object_t *def);
static const char *     key_macro_value(const object_t *def);
static const char *     key_macro_find(int key);

static char *           key_to_char(char *buf, int key);
static int              cygwin_to_int(const char *buf);
static int              msterminal_to_int(const char *buf);

static char *           historyget(int idx);


/*
 *  internal codes labels
 */
static const char *     keypad_names[] = {
    "Ins",                  /* KEYPAD_0       - 0         */
    "End",                  /* KEYPAD_1       - 1         */
    "Down",                 /* KEYPAD_2       - 2         */
    "PgDn",                 /* KEYPAD_3       - 3         */
    "Left",                 /* KEYPAD_4       - 4         */
    "5",                    /* KEYPAD_5       - 5         */
    "Right",                /* KEYPAD_6       - 6         */
    "Home",                 /* KEYPAD_7       - 7         */
    "Up",                   /* KEYPAD_8       - 8         */
    "PgUp",                 /* KEYPAD_9       - 9         */
    "Del",                  /* KEYPAD_DEL     - 10 Delete */
    "Plus",                 /* KEYPAD_PLUS    - 11 +      */
    "Minus",                /* KEYPAD_MINUS   - 12 -      */
    "Star",                 /* KEYPAD_STAR    - 13 *      */
    "Divide",               /* KEYPAD_DIV     - 14 /      */
    "Equals",               /* KEYPAD_EQUAL   - 15 =      */
    "Enter",                /* KEYPAD_ENTER   - 16 <cr>   */
    "Pause",                /* KEYPAD_PAUSE   - 17        */
    "PrtSc",                /* KEYPAD_PRTSC   - 18        */
    "Scroll",               /* KEYPAD_SCROLL  - 19        */
    "NumLock"               /* KEYPAD_NUMLOCK - 20        */
    };

struct map {
    int                 len;
    const char *        name;
    KEY                 modifier;
    KEY                 value;
};


/*
 *  key-strings to internal key-codes.
 */
static const struct map keystring_tbl[] = {
    { 3,    "ESC",          0,              KEY_ESC },
    { 1,    "{",            0,              '{' },
    { 1,    "}",            0,              '}' },
    { 5,    "SPACE",        0,              ' ' },
    { 5,    "ENTER",        0,              KEY_ENTER },
    { 6,    "RETURN",       0,              KEY_ENTER },    /*alias*/
    { 3,    "TAB",          0,              KEY_TAB },
    { 5,    "ARROW",        0,              0 },
    { 4,    "CTRL",         MOD_CTRL,       0 },
    { 5,    "SHIFT",        MOD_SHIFT,      0 },
    { 9,    "BACKSPACE",    0,              KEY_BACKSPACE },
    { 4,    "BACK",         MOD_SHIFT,      0 },            /*must be after BACKSPACE*/
    { 7,    "PRIVATE",      RANGE_PRIVATE,  0 },
    { 3,    "ALT",          MOD_META,       0 },
    { 4,    "META",         MOD_META,       0 },
    { 6,    "KEYPAD",       RANGE_KEYPAD,   0 },
    { 4,    "GREY",         RANGE_KEYPAD,   0 },
    { 2,    "UP",           RANGE_KEYPAD,   KEY_UP },
    { 4,    "DOWN",         RANGE_KEYPAD,   KEY_DOWN },
    { 4,    "LEFT",         RANGE_KEYPAD,   KEY_LEFT },
    { 5,    "RIGHT",        RANGE_KEYPAD,   KEY_RIGHT },
    { 4,    "HOME",         RANGE_KEYPAD,   KEY_HOME },
    { 3,    "END",          RANGE_KEYPAD,   KEY_END },
    { 4,    "PGUP",         RANGE_KEYPAD,   KEY_PAGEUP },
    { 4,    "PGDN",         RANGE_KEYPAD,   KEY_PAGEDOWN },
    { 4,    "STAR",         RANGE_KEYPAD,   KEYPAD_STAR },
    { 5,    "MINUS",        RANGE_KEYPAD,   KEYPAD_MINUS },
    { 4,    "PLUS",         RANGE_KEYPAD,   KEYPAD_PLUS },
    { 3,    "DEL",          RANGE_KEYPAD,   KEY_DEL },
    { 3,    "INS",          RANGE_KEYPAD,   KEY_INS },
    { 5,    "PRTSC",        RANGE_KEYPAD,   KEYPAD_PRTSC },
    { 6,    "SCROLL",       RANGE_KEYPAD,   KEYPAD_SCROLL },
    { 5,    "MOUSE",        RANGE_SPECIAL,  MOUSE_XTERM_KEY },
    { 5,    "MOUSE",        RANGE_SPECIAL,  MOUSE_SGR_KEY },
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
    { VK_BACK,          MOD_SHIFT,          "Shift-Back",       SHIFT_BACKSPACE },
    { VK_TAB,           MOD_SHIFT,          "Shift-Tab",        BACK_TAB },
    { VK_BACK,          MOD_CTRL,           "Ctrl-Back",        CTRL_BACKSPACE },
    { VK_TAB,           MOD_CTRL,           "Ctrl-Tab",         CTRL_TAB },
    { VK_BACK,          MOD_META,           "Alt-Back",         ALT_BACKSPACE },
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


/*  Function:           key_init
 *      Function called at start-up to initialise the first key map.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 */
void
key_init(void)
{
    unsigned i;

    assert((KEY_MASK   & (RANGE_MASK | MOD_MASK)) == 0);
    assert((RANGE_MASK & (KEY_MASK | MOD_MASK)) == 0);
    assert((MOD_MASK   & (KEY_MASK | RANGE_MASK)) == 0);

    assert((RANGE_CHARACTER & ~RANGE_MASK) == 0);
    assert((RANGE_KEYPAD & ~RANGE_MASK) == 0 && (RANGE_KEYPAD & RANGE_MASK));
    assert((RANGE_MISC & ~RANGE_MASK) == 0 && (RANGE_MISC & RANGE_MASK));
    assert((RANGE_MULTIKEY & ~RANGE_MASK) == 0 && (RANGE_MULTIKEY & RANGE_MASK));
    assert((RANGE_PRIVATE & ~RANGE_MASK) == 0 && (RANGE_PRIVATE & RANGE_MASK));
    assert((RANGE_BUTTON & ~RANGE_MASK) == 0 && (RANGE_BUTTON & RANGE_MASK));
    assert((RANGE_MASK & ~RANGE_MASK) == 0 && (RANGE_MASK & RANGE_MASK));
    assert((RANGE_MAX & ~RANGE_MASK) == 0 && (RANGE_MAX & RANGE_MASK));

    assert((MOD_SHIFT & ~MOD_MASK) == 0 && (MOD_SHIFT & MOD_MASK));
    assert((MOD_CTRL & ~MOD_MASK) == 0 && (MOD_CTRL & MOD_MASK));
    assert((MOD_META & ~MOD_MASK) == 0 && (MOD_META & MOD_MASK));
    assert((MOD_APP & ~MOD_MASK) == 0 && (MOD_APP & MOD_MASK));

    assert(IS_CHARACTER(' '));
    assert(IS_CHARACTER(0x1ff));
    assert(IS_FUNCTION(F(1)));
    assert(IS_BUTTON(BUTTON1_DOWN));
    assert(IS_BUTTON(BUTTON_DRAG));

    TAILQ_INIT(&x_kbdlist);
    x_kbdstack = ll_init();
    x_kseqtree = spinit();

    /*
     *  keyboard_typeables()
     *      set up the 'default' key definitions (see: config.c).
     *      Allocate push reference
     */
    x_kbdcur = keyboard_new();
    key_typeables();
    for (i = 0; x_key_table[i].name; ++i) {
        key_macro_add((int) x_key_table[i].key, x_key_table[i].name);
    }
    keyboard_push(x_kbdcur);
    x_push_ref = r_string("");

#if !defined(DOSISH)
    ttkeys();                                   /* termcap bindings */
#endif
}


/*  Function:           key_shutdown
 *      shut-down the keyboard interface, releasing any system resources
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 */
void
key_shutdown(void)
{
    KEYBOARDLIST_t *kbdlist = &x_kbdlist;
    keyboard_t *kp;

    /* key sequence */
    while (! spempty(x_kseqtree)) {
        SPBLK *sp = sphead(x_kseqtree);

        spdeq(sp, x_kseqtree);
        chk_free(sp->data);
        spfreeblk(sp);
    }
    spfree(x_kseqtree);
    x_kseqtree = NULL;

    r_dec(x_push_ref);
    x_push_ref = NULL;

    /* keyboard definitions */
    ll_clear2(x_kbdstack, NULL);
    ll_free(x_kbdstack);
    x_kbdstack = NULL;

    while (NULL != (kp = TAILQ_FIRST(kbdlist))) {
        keyboard_delete(kp);
    }
}


/*  Function:           key_typeables
 *      Enable all typeable key assignments as "self_insert".
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
void
key_typeables(void)
{
    unsigned i;

    for (i = 0; i <= 0xff; ++i) {               /* 0 .. 25, extended ASCII */
        key_macro_add(i, NULL);
    }
    key_macro_add(KEY_UNICODE, NULL);           /* >= 256 x <= KEY_UNICODE */
}


/*  Function:           key_define_key_seq
 *      Define a key-sequence
 *
 *  Parameters:
 *      key - Key identifier.
 *      str - Control sequence.
 *
 *  Returns:
 *      Actual key code assigned if we are given a multi-key code,
 *      or -1 if all entries consumed.
 */
int
key_define_key_seq(int key, const char *str)
{
    const int len = (int)strlen(str);
    int key_code = key;
    SPBLK *sp;
    keyseq_t *ks;

    if ((sp = splookup(str, x_kseqtree)) != NULL) {
        /*
         *  If we already have a definition for this key sequence, remove it.
         *  For multi-key sequences we can re-use the entry.
         */
        ks = (keyseq_t *) sp->data;
        if (key < 0) {
            key_code = ks->ks_code;

        } else if (key_code == key) {           /* 15/06/10 */
            trace_ilog("define_key_seq(%d/0x%x,%s) dup\n", key, key, str);
            return key_code;
        }

        trace_ilog("define_key_seq(%d/0x%x,%s) replaced\n", key, key, str);
        spdeq(sp, x_kseqtree);
        chk_free(ks);
        spfreeblk(sp);

    } else {
        /*
         *  new
         */
        if (key < 0) {
            /*
             *  Make sure we don't run out of room in the multikey range
             */
            if (x_multiseq >= MULTIKEY_SIZE) {
                return -1;
            }
            key_code = RANGE_MULTIKEY + x_multiseq++;

            /*
             *  Keep a pointer to the defining string handy so we can
             *  implement int_to_key() properly for these sequences.
             */
            if (NULL == x_multitbl) {
                x_multitbl = chk_alloc(sizeof(char *));

            } else {
                x_multitbl = chk_realloc(x_multitbl, x_multiseq * sizeof(char *));
            }
        }
        trace_ilog("define_key_seq(%d/0x%x,%s) new\n", key, key, str);
    }

    /*
     *  Allocate new node for this entry
     */
    sp = spblk(sizeof(keyseq_t) + len);
    ks = (keyseq_t *) sp->data;
    ks->ks_code = (KEY) key_code;
    memcpy(ks->ks_buf, str, len + 1);
    sp->key = ks->ks_buf;
    spenq(sp, x_kseqtree);
    if (key < 0) {
        x_multitbl[key_code - RANGE_MULTIKEY] = ks->ks_buf;
    }
    return key_code;
}


/*  Function:           key_get_seq_list
 *      Retrieves the current key-sequence list.
 *
 *  Parameters:
 *      num_syms - Storage populated by the total symbol number.
 *
 *  Results:
 *      Flatten version of the key escape sequence tree.
 */
void *
key_get_seq_list(int *num_syms)
{
    *num_syms = spsize(x_kseqtree);
    return (void *)spflatten(x_kseqtree);
}


/*  Function:           keyboard_new
 *      Allocate and initialise a new keyboard object.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      Keyboard object.
 */
static keyboard_t *
keyboard_new(void)
{
    KEYBOARDLIST_t *kbdlist = &x_kbdlist;
    keyboard_t *kp;

    if (NULL != (kp = (keyboard_t *)
            chk_calloc(sizeof(keyboard_t), 1))) {
        kp->kt_magic  = KEYBOARD_MAGIC;
        kp->kt_ident  = ++x_kbdident;
        kp->kt_refs   = 1;
        kp->kt_name   = NULL;
        kp->kt_macros = stype_alloc();
        kp->kt_pushed = 0;
        TAILQ_INSERT_HEAD(kbdlist, kp, kt_node);
    }
    return kp;
}


static void
keyboard_delete(keyboard_t *kp)
{
    KEYBOARDLIST_t *kbdlist = &x_kbdlist;
    const sentry_t *sep;
    int count;

    assert(KEYBOARD_MAGIC == kp->kt_magic);
    count = stype_used(kp->kt_macros);
    for (sep = stype_block(kp->kt_macros); count-- > 0; ++sep) {
        obj_free((object_t *)sep->se_ptr);
    }
    stype_free(kp->kt_macros);

    chk_free((void *)kp->kt_name);
    TAILQ_REMOVE(kbdlist, kp, kt_node);
    kp->kt_magic = 0xDEADBEEF;
    chk_free((void *)kp);
}


/*  Function:           keyboard_free
 *      Unreference and delete the keyboard object if no longer referenced.
 *
 *  Parameters:
 *      kp - Keyboard object.
 *
 *  Results:
 *      nothing
 */
static void
keyboard_free(keyboard_t *kp)
{
    if (kp) {
        assert(KEYBOARD_MAGIC == kp->kt_magic);
        if (--kp->kt_refs <= 0) {
            keyboard_delete(kp);
        }
    }
}


/*  Function:           keyboard_find
 *      Keyboard lookup, searchings both the pushed stack or the popped stack.
 *
 *  Parameters:
 *      id - Keyboard identifier.
 *      incref - Reference count increment on success.
 *
 *  Results:
 *      Keyboard object, otherwise NULL.
 */
static keyboard_t *
keyboard_find(const int kbdid, int incref)
{
    KEYBOARDLIST_t *kbdlist = &x_kbdlist;
    keyboard_t *kp;

    TAILQ_FOREACH(kp, kbdlist, kt_node) {
        assert(KEYBOARD_MAGIC == kp->kt_magic);
        if (kbdid == kp->kt_ident) {
            if (incref) {
                TAILQ_REMOVE(kbdlist, kp, kt_node);
                TAILQ_INSERT_HEAD(kbdlist, kp, kt_node);
                ++kp->kt_refs;
            }
            return kp;
        }
    }
    return NULL;
}


static void
keyboard_push(keyboard_t *kp)
{
    if (kp->kt_pushed) {
        errorf("keyboard_push: warning non-unique keyboard <%d>", kp->kt_ident);
    }
    ll_push(x_kbdstack, (void *) kp);
    ++kp->kt_pushed;
}


static keyboard_t *
keyboard_pop(void)
{
    List_p lp = ll_first(x_kbdstack);
    keyboard_t *kp;

    if (lp) {
        kp = (keyboard_t *) ll_elem(lp);
        assert(KEYBOARD_MAGIC == kp->kt_magic);
        (void) ll_pop(x_kbdstack);
        --kp->kt_pushed;
        return kp;
    }
    return NULL;
}


/*  Function:           keyboard_inq
 *      Called by inq_assignment() to look for keys assigned by a particular macro.
 *
 *  Parameters:
 *      key - Key to match.
 *      kp - Keyboard object.
 *      buf - Working buffer to which results are concatenated.
 *      buflen - Maximum length in bytes, including NUL.
 *
 *  Results:
 *      nothing
 */
static void
keyboard_inq(const char *key, const keyboard_t *kp, char *buf, int buflen)
{
    const sentry_t *sep;
    char *cursor;                               /* buffer cursor */
    int count;

    assert(key);
    assert(buf && buflen > MAX_KEYBUF);

    cursor = buf + strlen(buf);
    count = stype_used( kp->kt_macros);
    for (sep = stype_block(kp->kt_macros); count-- > 0; ++sep) {
        const char *cp = key_macro_value((const object_t *)sep->se_ptr);

        if (0 == strcmp(cp, key)) {
            const char *p = key_code2name(sep->se_key);
            const int len = (int)strlen(p);

            if (((cursor + len + 7) - buf) >= buflen) {
                break;                          /* overflow */
            }
            if (cursor > buf) {
                strcpy(cursor, "<-also>");      /* <...><-also><...> */
                cursor += 7;
            }
            strcpy(cursor, p);
            cursor += len;
        }
    }
}


/*  Function:           key_macro_add
 *      Add an assignment of a macro to a key binding.
 *
 *  Parameters:
 *      key - Key code.
 *      value - Key definition.
 *
 *  Results:
 *      nothing
 */
static object_t *
key_macro_add(int key, const char *value)
{
    object_t *def;

    if (NULL != (def = obj_alloc())) {
        // TODO: compile into a ARGV; reduce execute overheads.
        if (value) {                            /* assign value, otherwise NULL */
            obj_assign_str(def, value, -1);
        }
        key_macro_push(key, def);
    }
    return def;
}


static object_t *
key_macro_push(int key, object_t *def)
{
    if (x_kbdcur) {
        sentry_t *sep;
                                                /* insert? */
        if (NULL == (sep = stype_lookup(x_kbdcur->kt_macros, (stypekey_t) key))) {
            stype_insert(x_kbdcur->kt_macros, (stypekey_t) key, (void *)def);

        } else {                                /* replace */
            obj_free((object_t *) sep->se_ptr);
            stype_replace(x_kbdcur->kt_macros, sep, (void *)def);
        }
    }
    return def;
}


static const char *
key_macro_value(const object_t *def)
{
    const char *cp;

    if (obj_isnull(def)) {
        cp = "self_insert";
    } else {
        cp = obj_get_sval(def);
        assert(cp);
    }
    return cp;
}


/*  Function:           key_macro_find
 *      Find the name of a macro bound to a key.
 *
 *      Called by inq_assignment() and when we get a mouse paste action (we need to
 *      check if <Ins> is bound to the paste macro.
 *
 *  Parameters:
 *      key - Key code.
 *
 *  Results:
 *      Assigned macro, "self_insert", otherwise "nothing".
 */
static const char *
key_macro_find(int key)
{
    sentry_t *sep = NULL;
    const char *cp;

    if (curbp->b_keyboard) {                    /* buffer specific */
        sep = stype_lookup(curbp->b_keyboard->kt_macros, (stypekey_t) key);
        if (NULL == sep) {
            if (IS_UNICODE(key)) {
                sep = stype_lookup(curbp->b_keyboard->kt_macros, (stypekey_t) KEY_UNICODE);
            }
            if (NULL == sep) {
                sep = stype_lookup(curbp->b_keyboard->kt_macros, (stypekey_t) KEY_UNASSIGNED);
            }
        }
    }

    if (NULL == sep) {                          /* keyboard */
        sep = stype_lookup(x_kbdcur->kt_macros, (stypekey_t) key);
        if (NULL == sep) {
            if (IS_UNICODE(key)) {
                sep = stype_lookup(x_kbdcur->kt_macros, (stypekey_t) KEY_UNICODE);
            }
            if (NULL == sep) {
                sep = stype_lookup(x_kbdcur->kt_macros, (stypekey_t) KEY_UNASSIGNED);
            }
        }
    }

    if (NULL == sep) {
        cp = "nothing";
    } else {
        cp = key_macro_value((const object_t *)sep->se_ptr);
    }
    return cp;
}


/*  Function:           key_string2code
 *      Convert a key string in ascii format to the internal key code value.
 *
 *      If a multi-key is specified more than one key-code wide, return -1.
 *
 *  Parameters:
 *      cp - Key description buffer.
 *      buf - Resulting ASCII definition.
 *      buflen - Buffer length in bytes.
 *
 *  Results:
 *      Key-code, otherwise -1 if unknown or -2 on error.
 */
int
key_string2code(const char *cp, char *keybuf, int buflen)
{
    const KEY *keys;
    int ret = -1, size = 0;

    if (NULL == (keys = key_string2seq(cp, &size)) || size <= 0) {
        ret = -2;                               /* invalid key definition */

    } else if (1 == size) {
        ret = keys[0];                          /* single key */

    } else {                                    /* multi-key */
        char t_keybuf[MAX_KEYBUF] = {0};
        SPBLK *sp;
        int i;

        if (NULL == keybuf) {
            keybuf = t_keybuf;
            buflen = sizeof(t_keybuf);
        }

        for (i = 0, --buflen; i < size && i < buflen; ++i) {
            if (keys[i] & ~0xff) {              /* filter 16bit character codes */
                continue;
            }
            keybuf[i] = (char) keys[i];
        }
        keybuf[i] = '\0';
                                                /* code table lookup */
        if (NULL != (sp = splookup(keybuf, x_kseqtree))) {
            const keyseq_t *ks = (const keyseq_t *) sp->data;

            ret = ks->ks_code;
        }
    }
    return ret;
}


/*  Function:           key_string2seq
 *      Convert a sequence of keystrokes in external format to the internal values.
 *
 *  Parameters:
 *      cp - Key description buffer.
 *      sizep - Storage populated with the resulting length, in keys.
 *
 *  Results:
 *      Key buffer.
 */
const KEY *
key_string2seq(const char *cp, int *sizep)
{
    static ref_t *rp = NULL;                    /* FIXME - leak */
    KEY us = 0;

    if (NULL == rp) {
        rp = r_string("");                      /* allocate initial storage */
    } else {
        r_clear(rp);
    }

    while (*cp) {
        if (*cp == '<' && cp[1] && cp[2] == '>') {
            us = (KEY) cp[1];                   /* <character> */
            cp += 3;

        } else if (*cp == '<') {                /* <name> */
            const char *start = cp;
            int len = 0;

            while (*cp) {
                if (*cp++ == '>') {             /* closing '>' */
                    us = (KEY) key_name2code(start, &len);
                    if (/*us < 0 ||*/ (cp - 1) != (start + len)) {
                        return NULL;
                    }
                    break;
                }
            }

        } else if (*cp == '#') {                /* #digit[s] */
            us = (KEY) atoi(++cp);
            while (isdigit(*cp)) {
                ++cp;
            }

        } else {
            if (*cp == '%') ++cp;
            if (*cp == '\\') {
                us = (KEY) *++cp;
            } else if (*cp == '^') {
                us = (KEY) *++cp & 0x1f;
            } else {
                us = (KEY) *cp;
            }
            ++cp;
        }

        r_append(rp, (void *) &us, sizeof(us), 64);
    }

    if (sizep) {
        *sizep = r_used(rp) / sizeof(KEY);      /* resulting length */
    }
    return (KEY *) r_ptr(rp);
}


/*  Function:           key_name2code
 *      Attempt to convert a key description to its internal key code;
 *      this function handles the key-type descriptions.
 *
 *  Parameters:
 *      string - Key description buffer.
 *      lenp - Populated with the characters consumed within 'string'.
 *
 *  Results:
 *      keycode
 */
int
key_name2code(const char *string, int *lenp)
{
    char buf[128], *cp;                         /* MAGIC */
    const struct map *mp;
    int flags = 0, key = 0;

    strxcpy(buf, string + 1, sizeof(buf));
    for (cp = buf; *cp; ++cp) {
        if (*cp > 0) {                          /* case conversion */
            *cp = (char)toupper((unsigned char)*cp);
        }
    }

    for (cp = buf; *cp;) {
        /* termination, whitespace and punctuation */
        if ('>' == *cp) {
            ++cp;
            break;
        } else if (' ' == *cp || '-' == *cp || '\t' == *cp) {
            ++cp;
            continue;
        }

        /* function key, F<xx> */
        if ('F' == *cp && isdigit(cp[1])) {
            key = atoi(++cp) - 1;
            flags |= RANGE_FUNCTION;
            while (isdigit(*cp)) {
                ++cp;
            }
            continue;
        }

        /* button keys, Button#[-[Up|Double|Motion|Down]] */
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
            if ('>' == *cp) ++cp;
            break;
        }

        /* wheel keys, Wheel-[Up|Down] */
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
            if ('>' == *cp) ++cp;
            break;
        }

        /* special keywords (Alt, Ctrl, etc) */
        for (mp = keystring_tbl; mp->name; ++mp) {
            const int mlen = mp->len;

            if (0 == strncmp(mp->name, (const char *)cp, mlen) &&
                    ('>' == cp[mlen] || '-' == cp[mlen])) {
                cp += mlen;
                flags |= mp->modifier;
                key |= mp->value;
                break;
            }
        }

        /* single character */
        if (NULL == mp->name) {
            key |= *cp++;
            if ('>' == *cp) ++cp;
            break;
        }
    }

    /* consumed character count */
    if (lenp) *lenp = cp - buf;

    /* apply modifiers */
    if (KEY_TAB == key) {
        if (MOD_SHIFT == flags) {
            return BACK_TAB;
        }
        if (MOD_CTRL == flags) {
            return CTRL_TAB;
        }
        if (MOD_META == flags) {
            return ALT_TAB;
        }
    }

    if (KEY_BACKSPACE == key) {
        if (MOD_SHIFT == flags) {
            return SHIFT_BACKSPACE;
        }
        if (MOD_CTRL == flags) {
            return CTRL_BACKSPACE;
        }
        if (MOD_META == flags) {
            return ALT_BACKSPACE;
        }
    }

    if ((RANGE_MASK & flags) == RANGE_KEYPAD && key >= '0' && key <= '9') {
        key -= '0';                             /* KP-# */
    }

    if (MOD_CTRL == flags && key >= 0x40 && key <= 0x7f) {
        return (key & 0x1f);                    /* Ctrl A-Z */
    }

    return flags | key;
}


/*  Function:           do_assign_to_key
 *      assign_to_key primitivem, define a macro to be called when
 *          the specified key is pressed.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: assign_to_key - Assign command to key or key sequence.

        int
        assign_to_key([string key], [string macro])

    Macro Description:
        The 'assign_to_key' primitive assigns a key or key sequence
        to a function.

        The key is defined by the string parameter 'key', which if
        omitted shall be prompted. key may be specified as either an
        internal key code, or may be specified by a portable
        abbreviation, see below.

        A key is "unassigned" by assigning the function <nothing> to
        the key.

        If key evaluates to more than one keystroke, this shall
        define a multikey sequence, that is more than one key must be
        pressed to execute the macro. In this case, an internal key
        code is assigned for the key. This internal key code is
        returned as the value of this macro. Multikey sequences do
        not time out between key presses unlike the other internal
        keys when there is an ambiguity.

    Macro Parameters:
        key - String identifying the particular keystroke that is
            being assigned.

        macro - String containing the command to be invoked, which
            may optionally contain one or more space seperated
            integer or string arguments, to be passed upon the
            macro upon execution.

    Key Sequences:

        Generally keys are defined using their associated mnemonic
        possiblity prefixed with one or modifiers enclosed within a
        set of "<>" brackets, examples.

            o <F1>
            o <Ctrl-F1>
            o <Alt-Ctrl-Up>.

        The following are the set of supported modifiers

            o Shift     - Key Shift, for example 'a' to 'Z'.
            o Ctrl      - Control Key.
            o Alt       - Alt Key.
            o Meta      - Alias for Alt.
            o Keypad    - Keypad key variate.

        In the case of simple ASCII keys their character value can be
        used, for example "a".

        In addition to a mnemonic "<xxx>" or ASCII "x" syntax
        additional alternative forms and special characters are
        supported.

            #xxx -  Substitutes the '#' lead sequence of digits with
                    the represent value. For example '#123' result in
                    the key code 123.

            ^x -    The '^' characters treats the following character
                    as a control code, For example '^a', is control-a.

            \ -     The '\' character escapes the next character,
                    removing any special meaning.

        When it is required to state any of the special characters
        "<, >, %, # or ^" these can be referenced using their escaped
        form, example "\\<".

        The following table details the supported key sequence
        encoding and suitable modifiers; all names are matched case
        insensitive.

        For examples review current supplied macro code.

(start table,format=simple)
    |Key            |Description           |Keypad|Shift|Ctrl |Alt  |Meta |
    |ASCII          |ASCII key             |      |  x  |  x  |  x  |  x  |

    |F1..F12        |Function keys         |      |  x  |  x  |  x  |  x  |

    |PgDn           |Page Down             |      |     |     |     |     |
    |PgUp           |Page Up               |      |     |     |     |     |

    |Left           |Cursor Left           |  x   |  x  |  x  |  x  |     |
    |Right          |Cursor Right          |  x   |  x  |  x  |  x  |     |
    |Up             |Cursor Up             |  x   |  x  |  x  |  x  |     |
    |Down           |Cursor Down           |  x   |  x  |  x  |  x  |     |

    |Tab            |                      |      |     |     |     |     |
    |Back-Tab       |Shifted Tab           |      |     |     |     |     |
    |Backspace      |                      |      |     |     |     |     |
    |Back           |                      |      |     |     |     |     |
    |Del            |Delete                |      |     |     |     |     |

    |Enter          |Enter/Return Key      |  x   |     |     |     |     |
    |Esc            |Escape key            |      |     |     |     |     |
    |Space          |Space ( )             |      |     |     |     |     |

    |Home           |Cursor Home           |  x   |     |     |     |     |
    |End            |Cursor End            |  x   |     |     |     |     |

    |Ins            |Insert                |  x   |     |     |     |     |
    |Plus           |Plus (+)              |  x   |     |     |     |     |
    |Minus          |Minus (-)             |  x   |     |     |     |     |
    |Star           |Multiply (*)          |  x   |     |     |     |     |
    |Divide         |Div (/)               |  x   |     |     |     |     |
    |Equals         |Equal (=)             |  x   |     |     |     |     |

    |Cancel         |Cancel Key            |      |     |     |     |     |
    |Command        |Command Key           |      |     |     |     |     |
    |Copy           |Copy Key              |      |     |     |     |     |
    |Cut            |Cut Key               |      |     |     |     |     |
    |Exit           |Exit Key              |      |     |     |     |     |
    |Help           |Help Key              |      |     |     |     |     |
    |Menu           |Menu Key              |      |     |     |     |     |
    |Next           |Next Key              |      |     |     |     |     |
    |Open           |Open key              |      |     |     |     |     |
    |Paste          |Paste key             |      |     |     |     |     |
    |Prev           |Prev Key              |      |     |     |     |     |
    |Prtsc          |Print-Screen Key      |      |     |     |     |     |
    |Redo           |Redo Key              |      |     |     |     |     |
    |Replace        |Replace               |      |     |     |     |     |
    |Save           |Save                  |      |     |     |     |     |
    |Scroll         |Scroll                |      |     |     |     |     |
    |Search         |Search                |      |     |     |     |     |
    |Undo           |Undo                  |      |     |     |     |     |

    |Keypad-#       |Keypad 0..9           |      |  x  |  x  |  x  |  x  |

    |Grey-#         |Aliases for keypad    |      |     |     |     |     |

    |Button#        |Button number #       |      |     |     |     |     |

    |Button#-Up     |                      |      |     |     |     |     |

    |Button#-Double |                      |      |     |     |     |     |

    |Button#-Motion |                      |      |     |     |     |     |

    |Button#-Down   |                      |      |     |     |     |     |

    |Private#       |Private keys          |      |     |     |     |     |

    |Mouse          |Special Mouse Event   |      |     |     |     |     |

    |Wheel-Up       |Mousewheel up movement|      |     |     |     |     |

    |wheel-Down     |Mousewheel down       |      |     |     |     |     |
                     movement

    |Unassigned     |Default key Event     |  x   |  x  |  x  |  x  |  x  |

    |FocusIn        |Mouse focus Events    |  x   |  x  |  x  |  x  |  x  |
    |FocusOut       |                      |  x   |  x  |  x  |  x  |  x  |

(end table)

        Note!:
        The special 'Unassigned' key matches any unregistered key events,
        similar to <register_macro> REG_UNASSIGNED yet is assignable within
        the context of a specific keyboard.

    Macro Returns:
        The 'assign_to_key' returns the key value assigned to
        sequence, otherwise -1 if the key sequence is invalid or the
        operation aborted.

    Macro Portability:
        n/a

    Macro See Also:
        key_to_int
 */
void
do_assign_to_key(void)          /* int ([string key], [string macro]) */
{
    char keybuf[MAX_KEYBUF] = {0};
    char buf[MAX_CMDLINE] = {0};
    const char *cp, *value;
    int key_code;

    /*
     *  Get the key-name. If not specified in macro then prompt for it
     */
    if (NULL == (cp =
            get_arg1("Enter key: ", buf, sizeof(buf)))) {
        acc_assign_int(-1);
        return;
    }

    /*
     *  Convert ASCII representation to internal key name
     *
     *      If a multi-key definition, this key needs to be inserted into the
     *      key-sequence table in the private range.
     */
    if ((key_code = key_string2code(cp, keybuf, sizeof(keybuf))) < 0) {
        if (-2 == key_code) {
            errorf("assign_to_key: invalid key definition '%s'", cp);
            acc_assign_int(-1);
            return;
        }
        key_code = key_define_key_seq(-1, keybuf);
    }

    acc_assign_int((accint_t) key_code);

    /*
     *  Now get the macro name to be assigned. If it comes from the
     *  command line we need to make sure we allocate memory for it
     */
    if (isa_undef(2)) {
        if (TRUE == ereply("Enter macro name to assign: ", buf, sizeof(buf))) {
            if ((value = macro_resolve(buf)) == buf) {
                key_macro_add(key_code, buf);

            } else {
                key_macro_add(key_code, value);
                chk_free((void *)value);
            }
        }

    } else if (isa_string(2)) {
        const char *str = get_str(2);

        if ((value = macro_resolve(str)) == str) {
            key_macro_add(key_code, str);

        } else {
            key_macro_add(key_code, value);
            chk_free((void *)value);
        }

    } else {
        panic("assign_to_key: what?");
    }
}


/*  Function:           do_keyboard_typeables
 *      keyboard_typeable primitive. Fill in the ASCII typeable characters
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: keyboard_typeables - Assign self_insert to all typeable keys.

        int
        keyboard_typeables()

    Macro Description:
        The 'keyboard_typeables()' primitive populates the keyboard
        with all of the standard keys, example include 'ASCII keys'
        (for example 'A-Z' and '0-9'), 'Backspace', 'Tab', and
        'Enter' are bound to <self_insert>.

    Macro Parameters:
        none

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        inq_keyboard
 */
void
do_keyboard_typeables(void)     /* int (void) */
{
    key_typeables();
}


/*  Function:           do_push_back
 *       push_back primitive
 *
 *  Macro Parameters:
 *
 *  Macro Returns:
 *      nothing
 *
 *<<GRIEF>>
    Macro: push_back - Push back a character into the keyboard.

        void
        push_back(int key,
            [int front], [int x], [int y])

    Macro Description:
        The 'push_back()' primitive pushes the specified key code
        'key' into the keyboard input buffer.

        The 'front' is zero or omitted the key shall pushed to the
        back of the keyboard buffer, otherwise to the front.

    Macro Parameters:
        key - Key code.

        front - Optional boolean value, which if specified and is
            *TRUE* (non-zero) then the character is pushed pushed at
            the front of any previous characters. Otherwise if either
            omitted or *FALSE* (zero) then the key is pushed back
            after all previously pushed back characters.

        x, y - Mouse position, required when pushing back mouse
            events.

    Macro Returns:
        nothing

    Macro Portability:
        Mouse support is a Grief extension.

    Macro See Also:
        inq_kbd_char, key_to_int, read_char
 */
void
do_push_back(void)              /* (int key, [int front], [int x], [int y]) */
{
    const accint_t key = get_xinteger(1, 0);
    const int front = (int)get_xinteger(2, FALSE);

    if (key > 0 && key < KEY_VOID) {
        if ((RANGE_MASK & key) != RANGE_BUTTON) {
            key_cache_key(x_push_ref, key, front);

        } else {                                /* mouse-event, extension */
            mouseevt_t evt = { 0 };

            evt.x = get_xinteger(3, -1);
            evt.y = get_xinteger(4, -1);

            if (evt.x >= 0 && evt.y >= 0) {
                if (mouse_pos(evt.x, evt.y, &evt.win, &evt.where)) {
                    key_cache_mouse(x_push_ref, key, front, &evt);
                }
            }
        }
    }
}


/*  Function:           key_cache_key
 *      Push a character back on the keyboard input buffer.
 *
 *  Parameters:
 *      pp - Cache reference.
 *      ch - Key code.
 *      front - If TRUE, insert to the front of the keyboard queue
 *          otherwise append to the end.
 *
 *  Results:
 *      nothing
 */
void
key_cache_key(ref_t *pp, int ch, int front)
{
    assert(RANGE_BUTTON != (ch & RANGE_MASK));
    key_cache_mouse(pp, ch, front, NULL);
}


/*  Function:           key_cache_mouse
 *      Push back a mouse button down/up event together with the (x,y)
 *      coordinates into the keystroke input buffer.
 *
 *  Parameters:
 *      pp - Cache reference.
 *      code - Key code.
 *      front - If TRUE, insert to the front of the keyboard queue
 *          otherwise append to the end.
 *      evt - Mouse event, containing
 *
 *          seq - Optional escape sequence.
 *          x,y - Mouse coordinates.
 *          win - Where within the window.
 *          where - Timestamp, in milliseconds.
 *
 *  Returns:
 *      nothing
 */

void
key_cache_mouse(ref_t *pp, int code, int front, const mouseevt_t* evt)
{
    unsigned char buffer[sizeof(KEY) + sizeof(struct IOMouse) + IOSEQUENCE_LENGTH] = {0},
        *cursor = buffer;

    assert(sizeof(buffer) <= 128);
    assert(code > 0 && code <= (MOD_MASK|RANGE_MASK|KEY_MASK) && code != KEY_VOID);
    assert(evt || 0 == (RANGE_BUTTON == (RANGE_MASK & code)));

    *((KEY *)cursor) = (KEY)code;
    cursor += sizeof(KEY);

    if (RANGE_BUTTON == (RANGE_MASK & code)) {
        if (NULL == evt) {
            return;
        } else {
            unsigned char seqlen = (evt->seq ? (unsigned char)strlen(evt->seq) : 0);
            struct IOMouse* mouse = (struct IOMouse*)(cursor);
            int ms = 0;
            time_t now = sys_time(&ms);

            assert(seqlen <= IOSEQUENCE_LENGTH);
            if (seqlen > IOSEQUENCE_LENGTH) seqlen = IOSEQUENCE_LENGTH;

            mouse->x = evt->x;
            mouse->y = evt->y;
            mouse->win = evt->win;
            mouse->where = evt->where;
            mouse->when =
                (accint_t)(((now - x_startup_time) * 1000) + ms);
            cursor += sizeof(struct IOMouse);
            *cursor++ = seqlen;

            if (seqlen) {
                (void)memcpy(cursor, evt->seq, seqlen);
                cursor += seqlen;
            }
        }
    }

    assert(1 == r_refs(pp));
    if (TRUE == front) {
        r_push(pp, (void *)buffer, cursor - buffer, 128);
    } else {
        r_append(pp, (void *)buffer, cursor - buffer, 128);
    }
}


/*  Function:           key_cache_pop
 *      Retrieve the next input-event from the pushback buffer.
 *
 *   Parameters:
 *      pp - Cache reference.
 *      evt - Input event.
 *
 *  Returns:
 *      Character value, otherwise 0 if none.
 */
int
key_cache_pop(ref_t *pp, struct IOEvent *evt)
{
    const unsigned char *msg = (const unsigned char *)r_ptr(pp),
        *cursor = msg;
    size_t used;
    int code;

    if (0 == (used = r_used(pp))) {
        return 0;                               /* empty queue */
    }

    assert(used >= (int)sizeof(KEY));
    code = (int) *((const KEY *)cursor);
    cursor += sizeof(KEY);

    assert(code > 0 && code <= (MOD_MASK|RANGE_MASK|KEY_MASK) && code != KEY_VOID);
    evt->type = EVT_KEYDOWN;

    if (RANGE_BUTTON == (RANGE_MASK & code)) {
        /*
         *  mouse actions ...
         */
        if (used >= (sizeof(KEY) + sizeof(struct IOMouse) + 1)) {
            unsigned char seqlen;

            memcpy(&evt->mouse, (const void *)(cursor), sizeof(struct IOMouse));
            evt->type = EVT_MOUSE;
            cursor += sizeof(struct IOMouse);
            seqlen = *cursor++;

            if (seqlen) {
                if (used >= (sizeof(KEY) + sizeof(struct IOMouse) + 1 + seqlen)) {
                    assert(seqlen && seqlen < IOSEQUENCE_LENGTH);
                    evt->sequence.len = seqlen;
                    (void) memcpy(evt->sequence.data, (const void *)(cursor), seqlen);
                    cursor += seqlen;
                }
            }
        }
    }

    r_pop(pp, cursor - msg);
    return (evt->code = code);
}


/*  Function:           key_cache_test
 *      Determine whether there is an available input-event.
 *
 *   Parameters:
 *      pp - Cache reference.
 *
 *  Returns:
 *      *true* or *false*.
 */
int
key_cache_test(ref_t *pp)
{
    return r_used(pp);                          /* test only */
}


/*  Function:           do_key_to_int
 *      key_to_int primitive -- convert key name to internal key code.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: key_to_int - Convert key name to a code.

        int
        key_to_int(string key, int raw)

    Macro Description:
        The 'key_to_int()' primitive converts a mnemonic key string to an integer.

        The following scheme is utilised for encoding internal key-codes,
        allowing for simple conversion of ASCII character code to the internal
        codes and vice-versa.

        Firstly key-codes are divided into several ranges.

(start table)
        [Key Code           [Range          [Description                        ]
      ! RANGE_CHARACTER     0x0 ... 1fffff  Character ASCII/Unicode range.
      ! RANGE_FUNCTION      0x02000...      Function keys.
      ! RANGE_KEYPAD        0x03000...      Keypad keys.
      ! RANGE_MISC          0x04000...      Miscellaneous.
      ! RANGE_MULTIKEY      0x05000...      Multi-key stroke.
      ! RANGE_PRIVATE       0x06000...      Private key definitions for users.
      ! RANGE_BUTTON        0x07000...      Mouse buttons and movement.
(end table)

        These ranges can be OR'ed with one or more of the following
        bits to indicate a modifier key, for example a 'Shift-F1' key
        or 'Ctrl-Shift-F2'.

(start table)
        [Modifier           [Code           [Description                        ]
      ! MOD_SHIFT           0x00200000      Shift'ed.
      ! MOD_CTRL            0x00400000      Control.
      ! MOD_META            0x00800000      Meta or Alt.
(end table)

        To further simplify key handling, the follow special key
        manifest constants are predefined.

(start table)
        [Key Code           [Description                                        ]
      ! CTRL_1 .. CTRL_10   Control 1 thru 10.
      ! ALT_1 .. ALT_10     Alt 1 thru 10.
      ! CTRL_A .. CTRL_Z    Control A thru Z.
      ! ALT_Z .. ALT_Z      Alt A thru Z.
      ! KEY_BACKSPACE       Backspace.
      ! KEY_BREAK           Break.
      ! KEY_CANCEL          Cancel key.
      ! KEY_CLOSE           Close key.
      ! KEY_COMMAND
      ! KEY_COPY            Copy to clipboard.
      ! KEY_COPY_CMD
      ! KEY_CUT             Cut to clipboard.
      ! KEY_CUT_CMD
      ! KEY_DEL             Delete, rubout.
      ! KEY_DOWN            Move down, down arrow.
      ! KEY_END             End key.
      ! KEY_ENTER           Enter key.
      ! KEY_ESC             Escape.
      ! KEY_EXIT
      ! KEY_HELP            Help, usage.
      ! KEY_HOME            Home key.
      ! KEY_INS             Insert.
      ! KEY_LEFT            Move left, left arrow.
      ! KEY_MENU            Menu key.
      ! KEY_NEWLINE         New line.
      ! KEY_NEXT            Next.
      ! KEY_OPEN            Open key.
      ! KEY_PAGEDOWN        Page down.
      ! KEY_PAGEUP          Page up.
      ! KEY_PASTE           Paste clipboard.
      ! KEY_PREV            Prior, previous.
      ! KEY_REDO            Redo, again.
      ! KEY_REPLACE
      ! KEY_RIGHT           Move right, right arrow.
      ! KEY_SAVE
      ! KEY_SEARCH          Search.
      ! KEY_TAB             Tab.
      ! KEY_UNDO            Undo key.
      ! KEY_UNDO_CMD        Undo key.
      ! KEY_UP              Move up, up arrow.
      ! KEY_WDOWN
      ! KEY_WDOWN2
      ! KEY_WLEFT
      ! KEY_WLEFT2
      ! KEY_WRIGHT
      ! KEY_WRIGHT2
      ! KEY_WUP
      ! KEY_WUP2
      ! KEY_VOID            NUL key-code.
(end table)

        Warning!:
        The key encoding are exposed only for reference and may
        change without notice; for example to fully support Unicode.
        As such its advised keys should only be referenced using
        their string mnemonic representation when dealing directly
        with key-codes, for example.

>               switch (keycode) {
>               case key_to_int("<F1>"):
>                   f1();
>                   break;
>               case key_to_int("<Ctrl-A>"):
>                   ctrla();
>                   break;
>               case key_to_int("<Ctrl-B>"):
>                   ctrlb();
>                   break;
>               :

    Macro Parameters:
        key - String contains a single mnemonic key description
                (like "i" or "<Ctrl-z>").

        raw - Optional boolean flag. If non-zero, then 'key' is
            taken to be a raw key stroke/escape sequence, as
            entered by a function key on the keyboard. In this case,
            the key-code assigned to that key is returned.

    Macro Returns:
        The 'key_to_int()' primitive returns an integer representing
        the internal key code assigned to the specified key
        sequence, or -1 if the sequence does not correspond to an
        valid key.

    Macro Portability:
        n/a

    Macro See Also:
        assign_to_key, int_to_key
 */
void
do_key_to_int(void)             /* (string key, int raw) */
{
    const char *cp = get_str(1);
    const int raw = get_xinteger(2, FALSE);
    SPBLK *sp;

    if (! raw) {
        acc_assign_int((accint_t) key_string2code(cp, NULL, -1));

    } else {
        if (NULL == (sp = splookup(cp, x_kseqtree))) {
            int kcode;

            if ((kcode = cygwin_to_int(cp)) > 0 ||
                    (kcode = msterminal_to_int(cp)) > 0) {
                acc_assign_int((accint_t) kcode);
                return;
            }

#if !defined(USE_VIO_BUFFER) && !defined(DJGPP)
            if ((kcode = tty_mouse_xterm(NULL, cp)) > 0 ||
                    (kcode = tty_mouse_sgr(NULL, cp)) > 0) {
                acc_assign_int((accint_t) kcode);
                return;
            }
#endif

            acc_assign_int((accint_t) -1);

        } else {
            const keyseq_t *ks = (const keyseq_t *) sp->data;

            acc_assign_int((accint_t) ks->ks_code);
        }
    }
}


/*  Function:           do_int_to_key
 *      int_to_key primitive -- convert internal key code
 *      to the external name.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: int_to_key - Convert an keycode to mnemonic key string.

        string
        int_to_key(int key)

    Macro Description:
        The 'int_to_key()' primitive generates the corresponding
        mnemonic representation for the specified keycode 'key';
        which can be used by <assign_to_key>

    Macro Parameters:
        key - Integer keycode.

    Macro Returns:
        The 'int_to_key()' primitive returns the key mnemonic
        string associated with the specified keycode, for example
        "<Ctrl-D>"; see <assign_to_key> for full description of
        key mnemonics.

    Macro Portability:
        n/a

    Macro See Also:
        assign_to_key
 */
void
do_int_to_key(void)             /* string (int key) */
{
    acc_assign_str(key_code2name(get_xinteger(1, 0)), -1);
}


/*  Function:           key_code2name
 *      Convert a numeric key identifier into a printable string in the standard format.
 *
 *  Parameters:
 *      key - Key code.
 *
 *  Returns:
 *      Address of a static description buffer.
 */
const char *
key_code2name(int key)
{
    static unsigned oneshot = 0;
    static char buf[128];                       /* MAGIC */
    char *bp;
    SYMBOL *sp;
    int i;

    /*
     *  parse the user definable 'kbd_labels' list, if defined.
     */
    if (! IS_CHARACTER(key) &&
            NULL != (sp = sym_global_lookup("kbd_labels")) && F_LIST == sp->s_type) {
        /*
         *  iterate table, abort on error.
         *      INT + STRING
         */
        const LIST *nextlp, *lp = (const LIST *) r_ptr(sp->s_obj);
        accint_t val = 0;
        int element = 0;

        while ((nextlp = atom_next(lp)) != lp) {

            if (! atom_xint(lp, &val)) {        /* INT */
                if (! oneshot) {
                    errorf("kbd_labels: invalid numeric element [%d]", element);
                    ++oneshot;
                }
                goto do_normal;
            }
            lp = nextlp;

            if ((nextlp = atom_next(lp)) != lp) {
                if (val == key) {               /* STRING */
                    const char *str = NULL;

                    if (NULL == (str = atom_xstr(lp))) {
                        if (! oneshot) {
                            errorf("kbd_labels: invalid string element [%d]", element);
                            ++oneshot;
                        }
                        goto do_normal;
                    }
                    strxcpy(buf, str, sizeof(buf));
                    assert(strlen(buf) < sizeof(buf));
                    return buf;
                }
                lp = nextlp;
            }
            ++element;
        }
    }

    /* normal character case */
do_normal:
    if (IS_CHARACTER(key)) {
        key_to_char(buf, key);
        assert(strlen(buf) < sizeof(buf));
        return buf;
    }

    buf[0] = '<';
    buf[1] = '\0';

    if (key & MOD_META) strcat(buf, "Alt-");
    if (key & MOD_CTRL) strcat(buf, "Ctrl-");
    if (key & MOD_SHIFT) strcat(buf, "Shift-");

    /* function keys */
    bp = buf + strlen(buf);
    switch (key & RANGE_MASK) {
    case RANGE_SPECIAL: {
            const char *desc = NULL;

            switch (key) {
            case MOUSE_XTERM_KEY:
            case MOUSE_SGR_KEY:
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
                sprintf(bp, "#%u", key);
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
                sprintf(bp, "#%u", key);
                break;
            }
            if (desc) strcpy(bp, desc);
        }
        break;

    case RANGE_CHARACTER: {
            if ((key & KEY_MASK) > 0xff) {
                sprintf(bp, "#%u", key);
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
                case ' ':
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
        sprintf(bp, "Private-%d", key & KEY_MASK);
        break;

    case RANGE_FUNCTION:
        sprintf(bp, "F%d", (key & KEY_MASK) + 1);
        break;

    case RANGE_BUTTON:
        key &= ~(MOD_META | MOD_CTRL | MOD_SHIFT);
        if (key >= BUTTON1_MOTION) {
            sprintf(bp, "Button%u-Motion", (key - BUTTON1_MOTION) + 1);

        } else if (key >= BUTTON1_UP) {
            sprintf(bp, "Button%u-Up", (key - BUTTON1_UP) + 1);

        } else if (key >= BUTTON1_DOUBLE) {
            sprintf(bp, "Button%u-Double", (key - BUTTON1_DOUBLE) + 1);

        } else {
            sprintf(bp, "Button%u-Down", (key - BUTTON1_DOWN) + 1);
        }
        break;

    case RANGE_MULTIKEY:
        if (NULL == x_multitbl || (key >= RANGE_MULTIKEY + x_multiseq)) {
            strcpy(bp, "undefined");            /* out side range */

        } else {
            const char *cp = x_multitbl[key - RANGE_MULTIKEY];

            assert(cp);
            for (bp = buf; *cp; ++cp) {
                bp = key_to_char(bp, *cp & 0xff);
            }
            assert(strlen(buf) < sizeof(buf));
            *bp = '\0';
            return buf;
        }
        break;

    case RANGE_KEYPAD:
        i = key & KEY_MASK;
        if (i < (int) (sizeof(keypad_names) / sizeof(keypad_names[0]))) {
            const char *cp = keypad_names[i];

            if (isdigit(*cp) || (key & KEY_MASK) >= (KEYPAD_PLUS & KEY_MASK)) {
                strcpy(bp, "Keypad-");
                bp += 7;
            }
            while (*cp && *cp != '-') {
                *bp++ = *cp++;
            }
            *bp = '\0';
            break;
        }
        strcpy(bp, "Keypad-");
        bp += 7;
        /*FALLTHRU*/

    default:
        sprintf(bp, "#%u", key);
        break;
    }
    strcat(buf, ">");
    assert(strlen(buf) < sizeof(buf));
    return buf;
}


/*  Function:           key_to_char
 *      Convert a single 8-bit character into the canonic notation.
 */
static char *
key_to_char(char *buf, int key)
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

    case ' ':
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


/*  Function:           key_execute
 *      Execute the macro associated with an internal key code.
 *
 *  Parameter:
 *      key - Keycode.
 *
 *  Returns:
 *      void
 */
void
key_execute(int key, const char *seq)
{
    sentry_t *sep = NULL;
    const char *cp = "";
    int badkey = 0;

    assert(key >= 0);
    if (KEY_WINCH == key) {
        trace_log("\nKEY_EXEC(WINCH) 0x%x\n", KEY_WINCH);
        return;                                 /* WINCH event; ignore */
    }

    if (curbp && curbp->b_keyboard) {           /* buffer specific */
        sep = stype_lookup(curbp->b_keyboard->kt_macros, key);
        if (NULL == sep) {
            if (IS_UNICODE(key)) {
                sep = stype_lookup(curbp->b_keyboard->kt_macros, (stypekey_t) KEY_UNICODE);
            }
            if (NULL == sep) {
                sep = stype_lookup(curbp->b_keyboard->kt_macros, (stypekey_t) KEY_UNASSIGNED);
                if (sep) badkey = 1;
            }
        }
    }

    if (NULL == sep) {                          /* current keyboard */
        sep = stype_lookup(x_kbdcur->kt_macros, key);
        if (NULL == sep) {
            if (IS_UNICODE(key)) {
                sep = stype_lookup(x_kbdcur->kt_macros, (stypekey_t) KEY_UNICODE);
            }
            if (NULL == sep) {
                sep = stype_lookup(x_kbdcur->kt_macros, (stypekey_t) KEY_UNASSIGNED);
                if (sep) badkey = 1;
            }
        }
    }

    if (sep) {
        cp = key_macro_value((const object_t *)sep->se_ptr);
    }

    /*
     *  execute/
     */
    u_chain();
    x_character = (int32_t) key;                /* save internal key-code */
    playback_macro(cp);                         /* record key-stroke */

    /*
     *  history/
     *      Commands with names beginning with an underscore (_) are ignored.
     */
    assert(x_histhead >= 0);
    assert(x_histhead <  HIST_DEPTH);

    if (*cp && cp[0] != '_') {                  /* history */

        if (--x_histhead < 0) {
            x_histhead = HIST_DEPTH - 1;
        }

        strxcpy(x_histbuf[x_histhead], cp, sizeof(x_histbuf[0]));

        if (DB_HISTORY & x_dflags) {            /* dump history */
            const char *hist;
            int idx;

            trace_log("\nHistory:\n");
            for (idx = 0; idx < 10; ++idx) {
                if (NULL == (hist = historyget(idx)) || 0 == hist[0]) {
                    break;
                }
                trace_log("\t[%d] %s\n", idx, hist);
            }
        }
    }

    trace_log("\nKEY_EXEC(%s) 0x%x/%d (%02d) => %s\n",
        key_code2name(key), (unsigned)key, (int)x_character, x_histhead, cp);

    if (0 == *cp) {
        trigger(REG_UNASSIGNED /*REG_INVALID*/);
    } else {
        if (badkey) {
            execute_unassigned(cp, key, seq);
        } else {
            execute_str(cp);
        }
    }
}


/*  Function:           historyget
 *      Retrieve an entry from the command history.
 *
 *  Parameter:
 *      idx - History index.
 *
 *  Returns:
 *      History buffer, otherwise NULL.
 */
static char *
historyget(int idx)
{
    assert(x_histhead >= 0);
    assert(x_histhead <  HIST_DEPTH);

    if (idx >= 0 && idx < HIST_DEPTH) {
        return x_histbuf[(x_histhead + idx) % HIST_DEPTH];
    }
    return NULL;
}


/*  Function:           inq_command
 *      inq_command() primitive.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_command - Retrieve name of last keyboard command.

        string
        inq_command()

    Macro Description:
        The 'inq_command()' primitive retrieves the name of last
        command invoked from keyboard.

        Commands with names beginning with an underscore (_) are
        ignored.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_command()' primitive returns a string containing
        the name of the last macro called by the user.

    Macro Portability:
        n/a

    Macro See Also:
        inq_message
 */
void
inq_command(void)               /* string () */
{
    const char *hist = historyget(0);

    acc_assign_str(hist ? hist : "", -1);
}


/*  Function:           inq_macro_history
 *      inq_macro_history primitive.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_macro_history - Retrieve macro execution history.

        string
        inq_macro_history([int index = 0])

    Macro Description:
        The 'inq_macro_history()' primitive retrieves the name of
        the previously executed macro as a result of a keyboard
        binding.

        If 'index' is specified, it specifies the history index
        starting at an offset of zero, otherwise the most recent
        (i.e. index 0) is returns.

        The 'home' and 'end' macros are good examples of its usage,
        which modify their behaviour based upon previous operations.

    Macro Parameters:
        index - Optional int index.

    Macro Returns:
        The 'inq_macro_history()' primitive returns the name of the
        macro invoked, otherwise an empty string if the item does
        not exist.

    Macro Portability:
        A Grief extension, matching similar CRiSPEdit
        functionality of the same name.

    Macro See Also:
        set_macro_history, inq_command
 */
void
inq_macro_history(void)         /* string ([int pos]) */
{
    const int idx = get_xinteger(1, 0);
    const char *hist = historyget(idx);

    acc_assign_str(hist ? hist : "", -1);
}


/*  Function:           do_set_macro_history
 *      set_macro_history primitive.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_macro_history - Set the macro execution history.

        string
        set_macro_history(
            [int index = 0], [string value])

    Macro Description:
        The 'set_macro_history()' primitive replaces an entry
        within the macro execution history.

        'index' specifies the history index starting at an offset
        of zero. 'value' is new replace name.

        Note!:
        GRIEF maintains the history to the depth of 16.

    Macro Parameters:
        index - Optional int index.

    Macro Returns:
        The 'set_macro_history()' primitive returns the previous
        value name of entry replaced, otherwise an empty string
        if the item does not exist.

    Macro Portability:
        A Grief extension, matching similar CRiSPEdit
        functionality of the same name.

    Macro See Also:
        inq_macro_history
 */
void
do_set_macro_history(void)      /* string ([int index = 0]) */
{
    const int idx = get_xinteger(1, 0);
    const char *str = get_xstr(2);
    char *hist = historyget(idx);

    acc_assign_str(hist ? hist : "", -1);
    if (hist) {
        strxcpy(hist, (str ? str : ""), sizeof(x_histbuf[0]));
    }
}


/*  Function:           inq_keyboard
 *      inq_keyboard primitive, return current keyboard identifier.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_keyboard - Retrieve the keyboard identifier.

        int
        inq_keyboard()

    Macro Description:
        The 'inq_keyboard()' primitive retrieves the identifier
        associated with the current keyboard.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_keyboard()' primitive returns the current keyboard
        identifier.

    Macro Portability:
        n/a

    Macro See Also:
        keyboard_pop, keyboard_push
 */
void
inq_keyboard(void)              /* int () */
{
    acc_assign_int((accint_t) (x_kbdcur ? x_kbdcur->kt_ident : -1));
}


/*  Function:           inq_local_keyboard
 *      inq_local_keyboard primitive.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_local_keyboard - Retrieve local keyboard identifier.

        int
        inq_local_keyboard()

    Macro Description:
        The 'inq_local_keyboard()' primitive retrieves the
        identifier associated with current local keyboard.

    Macro Parameters:
        none

    Macro Returns:
        The 'inq_local_keyboard()' primitive returns the associated
        keyboard identifier, otherwise 0 if none is available.

    Macro Portability:
        n/a

    Macro See Also:
        use_local_keyboard
 */
void
inq_local_keyboard(void)        /* int inq_local_keyboard() */
{
    acc_assign_int((accint_t) (curbp->b_keyboard ? curbp->b_keyboard->kt_ident : 0));
}


/*  Function:           key_check
 *      Utility used whilst assembling a keystroke to test whether a full key sequence has been encountered.
 *      Return key code if we have an unambiguous keystroke.
 *
 *      If we have an ambiguity, check for a multikey press, and set 'multi' if so.
 *      This way we can force the caller to wait until the ambiguity is resolved.
 *
 *  Parameters:
 *      buf - Key buffer; nul terminated.
 *      buflen - Buffer length, in bytes; excluding nul.
 *      multi - Multiple key flag storage.
 *      noambig - Ambiguity flag.
 *
 *  Results:
 *      Key code, otherwise -1.
 */
int
key_check(const char *buf, unsigned buflen, int *multi, int noambig)
{
    SPBLK *sp_first, *sp;
    int kcode, ambig = 0;

    *multi = FALSE;

    /*
     *  WIN32 specials
     *
     *  Cygwin raw mode, cygwin-raw-mode, for example
     *
     *      <ESC>{0;1;13;28;13;0K
     *
     *  also allow MSTerminal, win32-input-mode, for example
     *
     *      <ESC>[17;29;0;1;8;1_
     */
#if defined(__CYGWIN__)
    if ('\033' == buf[0]) {
        const char *end = buf + (buflen - 1);

        if (xf_rawkb & RAWKB_CYGWIN)            /* cygwin-raw-mode */
            if ('{' == buf[1]) {
                const int final = (buflen >= 4 && (*end == 'K'));
                if (! final) {
                    *multi = TRUE;
                    return -1;
                }
                kcode = cygwin_to_int(buf);
                return (-1 == kcode ? 0 : kcode);
            }

        if (xf_rawkb & RAWKB_MSTERMINAL)        /* win32-input-mode */
            if ('[' == buf[1]) {
                const int final = (buflen >= 4 && (*end >= 0x40 && *end < 0x80)); // SGR final
                if (! final) {
                    *multi = TRUE;
                    return -1;
                }

                if ('_' == *end) {
                    kcode = msterminal_to_int(buf);
                    return (-1 == kcode ? 0 : kcode);
                }
            }
    }
#endif /*__CYGWIN__*/

    trace_log("KEYSEQ=");
    trace_hex(buf, buflen);

    if (NULL == (sp = sp_partial_lookup(buf, x_kseqtree, &ambig, &sp_first))) {
        if (sp_first) {
            const keyseq_t *kst = (const keyseq_t *) sp_first->data;

            if (IS_MULTIKEY(kst->ks_code)) {
                *multi = TRUE;
            }
        }
        trace_log("keycode: %d\n", ambig ? -1 : 0);
        return (ambig ? -1 : 0);
    }

    if (ambig) trace_log("amig(%d)-", ambig);
    if (noambig) {
        ambig = 0;                              /* ignore ambiguity */
    }

    kcode = (ambig ? -1 : ((const keyseq_t *) sp->data)->ks_code);
    trace_log("keycode: %d\n", kcode);
    return kcode;
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
    if (buf[0] == '\033' && buf[1] == '{' && buf[2]) {
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
 *      buf - Escape sequnence buffer.
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
    if (buf[0] == '\033' && buf[1] == '[' && buf[2]) {
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

    if (ch != -1) {
        trace_log("W32KEY %c%c%c = %d/0x%x (%s=%s)\n",
            (mod & MOD_META ? 'M' : '.'), (mod & MOD_CTRL ? 'C' : '.'), (mod & MOD_SHIFT ? 'S' : '.'),
                ch, ch, (key >= w32Keys ? key->desc : "ASCII"), key_code2name(ch));
    }
    return ch;
}
#endif  /*WIN32 || __CYGWIN__*/


/*  Function:           do_keyboard_push
 *      keyboard_push primitive. Save current keyboard on a stack
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: keyboard_push - Push a keyboard onto the keyboard stack.

        void
        keyboard_push([int kbdid])

    Macro Description:
        The 'keyboard_push()' primitive pushes a keyboard, either an
        existing or new, onto the last-in/first-out (LIFO) keyboard
        stack. Being a stack each invocation of 'keyboard_push' must
        have a corresponding invocation of <keyboard_push>; otherwise
        the keyboard objects shall never to released, resulting in a
        resource/memory leak.

        Either the specified keyboard 'kbdid' or if omitted a new
        empty keyboard object shall be pushed onto the stack and
        become the current keyboard (see inq_keyboard). When a new
        keyboard is created, it is empty and the current local
        keyboard is temporarily unassigned.

        A keyboard resource is a table of key bindings, that is keys
        mapped against the macro which should be executed when
        encountered. During initialisation the default keyboard
        resource binds the usual editing keys, 'keyboard_push()'
        permits the creation of macro specific keyboards, supporting
        temporarily bind macros to keys for their operation.

    Defaults::

        The default key bindings along with <keyboard_typeables>
        includes -

(start table)
        [Key                [Macro                                  ]
      ! F1                  <change_window>
      ! F2                  <move_edge>
      ! F3                  <create_edge>
      ! F4                  <delete_edge>
      ! F5                  <search_fwd>
      ! F6                  <translate>
      ! F7                  <remember>
      ! F8                  <playback>
      ! F9                  <load_macro>
      ! F10                 <execute_macro>
      ! Shift-F7            <pause>
      ! Ins                 <paste>
      ! End                 <end_of_line>
      ! Down                <down>
      ! Left                <left>
      ! Right               <right>
      ! Home                <beginning_of_line>
      ! Up                  <up>
      ! Del                 <delete_block>
      ! Cut                 <cut>
      ! Copy                <copy>
      ! Undo                <undo>
      ! Page-Down           <page_down>
      ! Page-Up             <page_up>
      ! Wheel-Down          <page_down>
      ! Wheel-Up            <page_up>
      ! Shift-Keypad-2      "change_window 2", see <change_window>
      ! Shift-Keypad-4      "change_window 3"
      ! Shift-Keypad-6      "change_window 1"
      ! Shift-Keypad-8      "change_window 0"
      ! Ctrl-Keypad-1       <end_of_window>
      ! Ctrl-Keypad-3       <end_of_buffer>
      ! Ctrl-Keypad-7       <top_of_window>
      ! Ctrl-Keypad-9       <top_of_buffer>
      ! Ctrl-X              <exit>
      ! Alt-0               "drop_bookmark 0", see <drop_bookmark>
      ! Alt-1               "drop_bookmark 1"
      ! Alt-2               "drop_bookmark 2"
      ! Alt-3               "drop_bookmark 3"
      ! Alt-4               "drop_bookmark 4"
      ! Alt-5               "drop_bookmark 5"
      ! Alt-6               "drop_bookmark 6"
      ! Alt-7               "drop_bookmark 7"
      ! Alt-8               "drop_bookmark 8"
      ! Alt-9               "drop_bookmark 9"
      ! Alt-A               "mark 4", see <mark>
      ! Alt-B               <buffer_list>
      ! Alt-C               "mark 2", see <mark>
      ! Alt-D               <delete_line>
      ! Alt-E               <edit_file>
      ! Alt-F               <feature>
      ! Alt-G               <goto_line>
      ! Alt-I               <insert_mode>
      ! Alt-J               <goto_bookmark>
      ! Alt-K               <delete_to_eol>
      ! Alt-L               <mark>
      ! Alt-M               <mark>
      ! Alt-O               <output_file>
      ! Alt-P               <print>
      ! Alt-Q               <quote>
      ! Alt-R               <read_file>
      ! Alt-S               <search_fwd>
      ! Alt-T               <translate>
      ! Alt-U               <undo>
      ! Alt-V               <version>
      ! Alt-W               <write_buffer>
      ! Alt-X               <exit>
      ! Alt-Z               <shell>
(end table)

    Macro Parameters:
        kbdid - Integer keyboard identifier, if omitted or zero a new
            keyboard is created and pushed. Keyboards within the
            stack are assumed to be unique; beware pushing a keyboard
            which already exists which shall have undefined effects.

    Macro Returns:
        The 'keyboard_push()' primitive returns the identifier of the
        referenced keyboard, otherwise -1 on error.

    Macro Example:

        The following example creates a new keyboard with only
        typeable key binds active.

>           keyboard_pop();
>           keyboard_push();
>           keyboad_typeables();

        The following example shows how to create a temporary
        keyboard mapping for use within a macro.

>      keyboard_push();
>      assign_to_key("<Alt-H>", "help");
>          // additional key bindings
>                       ::
>      process();
>      keyboard_pop();

    Macro Portability:
        n/a

    Macro See Also:
        inq_keyboard, keyboard_pop, keyboard_typeables, process
 */
void
do_keyboard_push(void)          /* void ([int kbdid]) */
{
    const int kbdid = get_xinteger(1, 0);
    keyboard_t *kp;

    if (kbdid > 0) {                            /* existing */
        if (NULL == (kp = keyboard_find(kbdid, TRUE))) {
            errorf("keyboard_push: keyboard <%d> not found.", kbdid);
            acc_assign_int(-1);
            return;
        }
    } else {                                    /* otherwise new */
        kp = keyboard_new();
    }
    acc_assign_int(kp->kt_ident);
    keyboard_push(kp);
    x_kbdcur = kp;
}


/*  Function:           do_keyboard_pop
 *      keyboard_pop primitive - pop keyboard possible keeping
 *      current one for later reuse.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: keyboard_pop - Pop a keyboard from the keyboard stack.

        void
        keyboard_pop([int save = FALSE])

    Macro Description:
        The 'keyboard_pop()' primitive removes the top keyboard, from
        the last-in/first-out (LIFO) keyboard stack.

        Each invocation of <keyboard_push> must have a corresponding
        invocation of 'keyboard_pop'.

        The keyboard resource and associated identifier shall only
        remain valid if the 'save' argument is non-zero, otherwise
        the keyboard definition is deleted and its memory reclaimed.

        If the current buffer is the same as it was when the keyboard
        was pushed, the current local keyboard is also restored to
        its former value.

    Macro Parameters:
        save - Optional boolean value, if specified as *true* the
            keyboard resource is retained otherwise if *false* or
            omitted it is discarded.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        keyboard_push, inq_keyboard
 */
void
do_keyboard_pop(void)           /* void (int save = FALSE) */
{
    const int save = get_xinteger(1, FALSE);
    keyboard_t *kp;

    if (NULL != (kp = keyboard_pop())) {
        List_p lp = ll_first(x_kbdstack);

        x_kbdcur = (lp ? (keyboard_t *) ll_elem(lp) : NULL);
        if (! save) {
            if (0 == kp->kt_pushed) {           /* guard non-unique references */
                keyboard_free(kp);
            }
        }
    }
}


/*  Function:           do_use_local_keyboard
 *      use_local_keyboard primitive -- cause current keyboard to
 *      be associated with the current buffer
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: use_local_keyboard - Associate a keyboard with a buffer.

        int
        use_local_keyboard(int kbdid)

    Macro Description:
        The 'use_local_keyboard()' primitive cause the current
        keyboard to be associated with the current buffer.

    Macro Parameters:
        kbdid - Keyboard identifier.

    Macro Returns:
        The 'use_local_keyboard' primitive return 0 on success
        otherwise -1 on error.

    Macro Portability:
        n/a

    Macro See Also:
        inq_local_keyboard, keyboard_pop, keyboard_push
 */
void
do_use_local_keyboard(void)     /* int (int kbdid) */
{
    const int kbdid = get_xinteger(1, 0);
    int ret = -1;

    if (kbdid <= 0) {
        key_local_detach(curbp);
        ret = 0;

    } else if (curbp) {
        keyboard_t *kp;

        if (NULL != (kp = keyboard_find(kbdid, TRUE))) {
            key_local_detach(curbp);
            curbp->b_keyboard = kp;
            ret = 0;
        }
    }
    acc_assign_int(ret);
}


/*  Function:           key_local_detach
 *      Function to remove a reference to a local keyboard.
 *
 *  Parameters:
 *      bp - Buffer object address.
 *
 *  Results:
 *      nothing
 */
void
key_local_detach(BUFFER_t *bp)
{
    if (bp && bp->b_keyboard) {
        keyboard_free(bp->b_keyboard);
        bp->b_keyboard = NULL;
    }
}


/*  Function:           inq_assignment
 *      inq_assignment primitive, returns names of macros
 *      assigned to keys or keys assigned to macros.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_assignment - Get key assignment for function.

        string
        inq_assignment(int|string val, [int tokey = FALSE])

    Macro Description:
        The 'inq_assignment()' primitive either retrieves the
        command that is assigned to the particular key 'val' or
        the key sequence(s) that will invoke a specific command.

    Macro Parameters:
        val - String denoting the key sequence to be decoded or
            an integer representing the internal key code. The
            string representation should be of the form described
            by <assign_to_key>.

        tokey - Optional boolean value, if *true* then 'val' is
            taken as a macro name and the keys assigned to invoke
            this macro are returned. The key assignment returned
            is returned using the portable key definitions
            defined for <assign_to_key>.

    Macro Returns:
        The 'inq_assignment()' primitive returns a string
        containing the desired conversion.

        For key sequence to command: the name of the command; OR
        "nothing" if no command is assigned; OR "ambiguous" if there
        is more than key sequence starting with the 'val'.

        For command to key sequence enabled when 'tokey' is non-zero:
        a list of the valid key sequences for the command. The list
        elements are formatted with the following separators:

            -or -   Separates different ways of specifying a
                    single key.

            -and -  Separates keys in a multi-key sequence.

            -also - Separates multiple (different) key sequences.

    Macro Portability:
        n/a

    Macro See Also:
        assign_to_key
 */
void
inq_assignment(void)            /* string (string key, [int convert]) */
{
    const char *key = get_xstr(1);
    const int convert = (get_xinteger(2, 0) == 0);

    if (convert) {
        const int key_code =
            (key ? key_string2code(key, NULL, -1) : get_xinteger(1, -1));

        acc_assign_str(key_macro_find(key_code), -1);

    } else {
        char buf[BUFSIZ];

        buf[0] = 0;
        if (key) {
            if (curbp->b_keyboard) {
                keyboard_inq(key, curbp->b_keyboard, buf, sizeof(buf));
            }
            keyboard_inq(key, x_kbdcur, buf, sizeof(buf));
        }

        if (0 == buf[0]) {
            acc_assign_str("nothing", 7);
        } else {
            acc_assign_str(buf, -1);
        }
    }
}


/*  Function:           do_key_list
 *      key_list primitive -- return list of key bindings
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: key_list - Retrieve keyboard bindings.

        list
        key_list(int kbdid,
            [int self_inserts = 0], [int bufnum])

    Macro Description:
        The 'key_list()' primitive retrieves the key bindings
        associated with the keyboard 'kbdid'. The definitions are
        returned as a list of string pairs, the first element being
        the key name with the second being assigned macro.

        Up to two keyboards can be specified. If 'bufnum' is
        specified, then the local keyboard assigned to that buffer is
        used. If not specified then the local keyboard map of the
        current buffer is used. The returned list is a union of the
        set keyboards.

        This primitive is designed to display a list of all valid
        keys currently mapped in both the current local and global
        keyboard maps. For an example see the 'key_map' macro.

    Macro Parameters:
        kbdid - Optional integer keyboard identifier. If omitted
            then firstly the local keyboard if available is
            referenced otherwise the current keyboard is referenced.

        self_inserts - Optional boolean flag when non-zero keys
            assigned to <self_insert> shall be including, otherwise
            they are omitted from the generated list.

        bufnum - Optional buffer identifier to source the secondary
            local keyboard. If omitted then the local keyboard map of
            the current buffer is used.

    Macro Returns:
        The 'key_list()' primitive returns a list of key binding, as
        a set of string pairs, the first element is the key name and
        second being assigned macro to that key. Otherwise on error a
        null list.

    Macro Portability:
        n/a

    Macro See Also:
        keyboard_pop, keyboard_push
 */
void
do_key_list(void)               /* list (int kbdid, [int self_inserts = 0], [int bufnum]) */
{
    const int do_self_inserts = (isa_integer(2) && get_xinteger(2, 0) != 0);
    const keyboard_t *kp = NULL, *kpl = NULL;

    const sentry_t *seps[2] = {0};
    LIST *newlp, *lp;
    int atoms, llen;
    int cnts[2], i;

    /*
     *  If keybd id not specified then use local keyboard if available
     *  otherwise go for the current keyboard.
     */
    if (isa_integer(3)) {                       /* local keyboard */
        const BUFFER_t *bp = buf_lookup(get_xinteger(3, 0));

        kpl = (bp ? bp->b_keyboard : NULL);
    }

    if (isa_integer(1)) {                       /* explicit */
        const int kbdid = get_xinteger(1, 0);

        if (NULL == (kp = keyboard_find(kbdid, FALSE))) {
            acc_assign_null();
            return;
        }

    } else {                                    /* buffer-local otherwise current */
        kp = x_kbdcur;
        if (NULL == kpl) {
            kpl = (curbp ? curbp->b_keyboard : NULL);
        }
    }

    if (NULL == kp && NULL == kpl) {
        acc_assign_null();
        return;
    }

    if (kp) {
        cnts[0] = stype_used(kp->kt_macros);
        seps[0] = stype_block(kp->kt_macros);
    }

    if (kpl) {
        cnts[1] = stype_used(kpl->kt_macros);
        seps[1] = stype_block(kpl->kt_macros);
    }

    /* size */
    atoms = 0;
    for (i = 0; i < 2; ++i) {
        const sentry_t *sep;
        int cnt;

        if (NULL != (sep = seps[i])) {
            for (cnt = cnts[i]; cnt-- > 0; ++sep) {
                if (do_self_inserts) {
                    atoms += 2;
                } else {
                    const object_t *obj = (const object_t *)sep->se_ptr;

                    if (! obj_isnull(obj)) {
                        atoms += 2;             /* non NULL's */
                    }
                }
            }
        }
    }

    /* allocate memory for list */
    llen = (atoms * sizeof_atoms[F_LIT]) + sizeof_atoms[F_HALT];
    if (0 == atoms || NULL == (newlp = lst_alloc(llen, atoms))) {
        acc_assign_null();
        return;
    }

    /* build */
    lp = newlp;
    for (i = 0; i < 2; ++i) {
        const sentry_t *sep;
        int cnt;

        if (NULL != (sep = seps[i])) {
            for (cnt = cnts[i]; cnt-- > 0; ++sep) {
                const object_t *obj = (const object_t *)sep->se_ptr;
                const char *macro;

                if (obj_isnull(obj)) {
                    if (! do_self_inserts) {    /* skip NULL's */
                        continue;
                    }
                    macro = "self_insert";

                } else {
                    macro = obj_get_sval(obj);
                    assert(macro);
                }

                lp = atom_push_str(lp, key_code2name(sep->se_key));
                lp = atom_push_str(lp, macro);
            }
        }
    }
    atom_push_halt(lp);                         /* terminator */
    acc_donate_list(newlp, llen);
}


/*  Function:           do_copy_keyboard
 *      copy_keyboard primitive.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: copy_keyboard - Copy a keyboard

        int
        copy_keyboard(int kbdid,
                [string cmd ...])

    Macro Description:
        The 'copy_keyboard()' primitive copies the key bindings
        associated with the keyboard identifier 'kbdid'. The keyboard
        is either copied in its entirety or optionally the subset of
        key bindings associated with the specified list of commands
        'cmd'.

        The primary use of this primitive is to obtain an explicit
        subset of a full keyboard for a specific use; for example a
        keyboard to be used by an popup for a special editing window.
        Another way of thinking, it a macro can inherit a keyboard,
        modify and then utilise locally without knowledge of the
        callers keyboard bindings.

    Macro Parameters:
        kbdbid - Keyboard identifier.
        ... - Optional list of command names whose key
            assignments should be duplicated in the current
            keyboard. If no commands are given, then this just
            creates a duplicate keyboard.

    Macro Returns:
        The 'copy_keyboard()' primitive returns the Keyboard
        identifier of the new keyboard. On error 0 is returned if the
        commands given are not found.

    Macro Portability:
        n/a

    Macro See Also:
        keyboard_push, keyboard_pop, assign_to_key
 */
void
do_copy_keyboard(void)          /* int (int kbdid, string cmd ...) */
{
    const int kbdid = get_xinteger(1, 0);
    const LIST *nextlp, *lp = get_list(2);
    keyboard_t *kp;

    trace_log("copy_keyboard(%d)\n", kbdid);

    if (NULL == (kp = keyboard_find(kbdid, FALSE)) || kp == x_kbdcur) {
        acc_assign_int(0);
        return;
    }
    acc_assign_int((accint_t) x_kbdcur->kt_ident);

    for (;(nextlp = atom_next(lp)) != lp; lp = nextlp) {
        /*
         *  search for matches
         */
        const char *cmd;

        if (NULL != (cmd = atom_xstr(lp))) {
            const sentry_t *sep;
            int count;

            count = stype_used(kp->kt_macros);
            for (sep = stype_block(kp->kt_macros); count-- > 0; ++sep) {
                const object_t *def = (const object_t *)sep->se_ptr;
                const char *defcmd = obj_get_sval(def);

                if (defcmd && 0 == strcmp(defcmd, cmd)) {
                    object_t *ndef;

                    if (NULL != (ndef = obj_copy(def))) {
                        key_macro_push(sep->se_key, ndef);
                        trace_ilog("pushed '%s' (0x%x)\n", \
                            key_code2name(sep->se_key), sep->se_key);
                    }
                }
            }
        }
    }
}


/*  Function:           do_set_kbd_name
 *      set_kbd_name primitive.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: set_kbd_name - Set the keyboard name.

        void
        set_kbd_name(string name, [int kbdid])

    Macro Description:
        The 'set_kbd_name()' primitive assigned the label 'name' as
        the name of the keyboard 'kbdid', allowing primitives to
        assign a descriptive or meaningful definition to a keyboard.
        These names or descriptions may then be utilised by macros.

    Macro Parameters:
        name - String containing the name to be assigned. If empty
            the current name shall be cleared.

        kbdid - Optional integer keyboard identifier, if omitted the
            current keyboard shall be referenced.

    Macro Returns:
        nothing

    Macro Portability:
        n/a

    Macro See Also:
        inq_kbd_name
 */
void
do_set_kbd_name(void)           /* void (string name, [int kbdid]) */
{
    const char *name = get_str(1);

    if (name) {
        keyboard_t *kp = NULL;

        if (isa_integer(2)) {                   /* explicit (extension) */
            const int kbdid = get_xinteger(2, 0);

            if (NULL == (kp = keyboard_find(kbdid, FALSE))) {
                errorf("set_kbd_name: keyboard <%d> not found.", kbdid);
            }
        } else {                                /* current */
            kp = x_kbdcur;
        }

        if (kp) {
            if (kp->kt_name) {
                chk_free((void *)kp->kt_name);
            }
            kp->kt_name = (*name ? chk_salloc(name) : 0);
        }
    }
}


/*  Function:           do_inq_kbd_name
 *      inq_kbd_name primitive.
 *
 *  Parameters:
 *      none
 *
 *  Results:
 *      nothing
 *
 *<<GRIEF>>
    Macro: inq_kbd_name - Retrieve the assigned keyboard name.

        string
        inq_kbd_name([int kbdid])

    Macro Description:
        The 'inq_kbd_name()' primitive retrieves the name assigned to
        the keyboard 'kbdid' using <set_kdb_name>.

    Macro Parameters:
        kbdid - Optional integer keyboard identifier, if omitted the
            current keyboard shall be referenced.

    Macro Returns:
        The 'inq_kbd_name()' primitive retrieves the keyboard name
        otherwise an empty string is none has been assigned.

    Macro Portability:
        n/a

    Macro See Also:
        set_kbd_name
 */
void
inq_kbd_name(void)              /* string ([int kbdid]) */
{
    keyboard_t *kp = NULL;

    if (isa_integer(1)) {                       /* explicit (extension) */
        const int kbdid = get_xinteger(1, 0);

        if (NULL == (kp = keyboard_find(kbdid, FALSE))) {
            errorf("inq_kbd_name: keyboard <%d> not found.", kbdid);
        }
    } else {                                    /* current */
        kp = x_kbdcur;
    }

    if (kp) {
        acc_assign_str(kp->kt_name ? kp->kt_name : "", -1);
    }
}

/*end*/
