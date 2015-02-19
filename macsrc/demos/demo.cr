/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: demo.cr,v 1.8 2014/10/22 02:34:28 ayoung Exp $
 * Demo.
 *
 *
 */

#include "../grief.h"


/* Time to delay waiting before pushing each keystroke
 */
#define TIMEOUT_SLOW    400
#define TIMEOUT_FAST    100


/* Delay to allow users time to read popup menu
 */
#define PAUSE_DELAY     5           /* * TIMEOUT_SLOW */


/* The following commands are used to put in commands in the
 * script to affect the way it work
 */
#define SET_DELAY       0           /* Following argument sets the timeout */
#define PAUSE           1           /* Pause to allow user to read a menu */
#define SLOW            2           /* Type in slowly */
#define FAST            3           /* Type in quickly */


/*
 *  The following list constitute the strings to force into the
 *  keyboard buffer.
 */
static list demo_list =
    {
        FAST,
        "<Enter><Enter><Alt-0>y",
        "    This is a demonstration and resume of the features available\n",
        "    in CRISP v2.1. CRISP is a programmers editor it contains\n",
        "    features to aid in writing programs and look at files.\n",
        "    CRISP is designed to avoid the limitations found in other\n",
        "    editors, e.g. maximum line length or sizes of files.\n",
        "\n",
        "    (This feature list and demonstration is available via the\n",
        "    'demo' macro. If you are looking at this feature list from\n",
        "    outside of CRISP then you will not see the examples being\n",
        "    executed).\n",
        "\n<Alt-1>y",
        "    The following list hilights and demonstrates major features\n",
        "    of CRISP.\n\n",
        "\n",
        "    o  CRISP supports some text-processing, e.g. spell checking,\n",
        "       autowrap, and auto-indent. It will even format and optionally\n",
        "       justify text - even whilst it is being typed.\n\n",
        "       CRISP also understands how to put C-style box comments around\n",
        "       text. For example, if we change the file extension of this\n",
        "       demonstration buffer to be '.c' (via Alt-O)...\n\n",
        SLOW,
        "<Alt-O>demo.c<Enter>",
        FAST,
        "       We can now hilight this paragraph and the last one\n",
        "       and reformat it (via Ctrl-F).",
        SLOW,
        "<Alt-L><Up><Up><Up><Up><Up><Ctrl-F>\n",
        FAST,
        "    o  Support for unlimited number of bookmarks (place-holders)\n",
        "       in buffers. Popup window available to allow selection of\n",
        "       bookmarks if user forgets which ones have been assigned.\n",
        "<Alt-2>y",
        SLOW,
        "<Alt-J><Tab>", PAUSE, PAUSE, FAST, "<Esc><Esc>",

        "\n",
        "    o  Unlimited undo and redo facility (ability to undo an undo).\n",
        "\n",
        "    o  Color support and hilighted regions which can be copied,\n",
        "       or cut to a scrap buffer. Column cut and paste.\n",
        "\n",
        "    o  Autosave, automatic backups and ability to define how\n",
        "       many backups to keep.\n",
        "\n",
        "    o  On-line help, via the Alt-H key...",
        "\n",
        SLOW,
        "<Alt-H><Enter><Down><Down><Down><PgDn><PgDn><PgDn><Esc>",
        "<Down><Down><Enter><Down><Down><Down><Down><Down><Down><Enter><End><Enter>",
        PAUSE, "<Esc>",

        /* Explain macro
         */
        "<Down><Enter>create_<Tab><Down><Down><Down><Down><Enter><Enter>",
        PAUSE, "<Esc>",

        /* User/programmers/config guides
         */
        "<Down><Enter>", PAUSE, "<Esc>",
        "<Down><Enter>", PAUSE, "<Esc>",
        "<Down><Enter>", PAUSE, "<Down><Down><Enter>", PAUSE, "<Esc><Esc>",
        "<Esc>",
        FAST,

        "\n",
        "    o  CRISP supports multiple display windows:\n",
        "        - tiled windows, both horizontally..\n",
        SLOW,
        "<F3><Down>",
        FAST,

        "          and vertically..",
        "<F3><Right>even at the same time!\n",
        "<F4><Left><F4><Up>",
        "        - and popup windows for making selections easily.\n",
        "\n",
        "    o  Popup windows are available giving the programmer\n",
        "       frequently wanted information, e.g. ASCII charts..\n",
        SLOW,
        "<F10>ascii\n<Down><Down><Enter>", PAUSE, PAUSE, "<Esc><Esc>",
        FAST,

        "       C precedence chart..\n",
        SLOW, "<F10>chier\n", PAUSE, PAUSE, "<Esc>", FAST,

        "\n",
        "    o  Abbreviations to aid in typing in. For example, we can\n",
        "       define an abbreviation, like IBM so that when we type it\n",
        "       in it automatically gets expanded..",
        SLOW,
        "<F10>abbrev<Enter>IBM<Enter>International Business Machines<Enter>",
        FAST,

        " Now when we type\n",
        "       these magic characters we get: IBM \n",
        "\n",
        "    o  Command line history recall and editing...\n\n",
        "<F10>this is command one\n",
        "<F10>this is command two\n",
        SLOW,
        "<F10>this is command three<Up><Left><Left><Left><Alt-I>three\n",
        FAST,

        "    o  Popup calculators for those times when you need to do\n",
        "       some quick arithmetic. Both normal and Reverse Polish\n",
        "       calculators supported.\n",
        "        - 'calc' is a normal infix calculator.\n",
        SLOW,
        "<F10>calc<Enter>123+456=<Esc>",
        FAST,

        "        - 'hpcalc' is a Reverse Polish calculator.\n",
        SLOW,
        "<F10>hpcalc<Enter>123<Enter>456<Enter>R->B<Enter>+<Esc>",
        FAST,

        "\n",
        "    o  Sophisticated C-like macro programming language.\n",
        "       This demo is written entirely in the macro language.\n",
        "\n",
        "    o  Access to on-line manual pages via the 'man' command...",
        SET_DELAY, 0,
        "<F10>man pwd<Enter>",
        PAUSE, PAUSE,
        "<Esc>",
        FAST,

        "\n\n",
        "    o  Menu driven for those difficult to get at commands...(Alt-F)",
        "<Alt-F>", PAUSE,
        "<Down><Down><Down><Down><Down><Down><Down><Down><Down><Down>",
        "<Down><Down><Down><Down><Down><Down><Down><Down><Down><Down>",
        "<Down><Down><Down><Down><Down><Down><Down><Down><Down><Down>",
        "<Down><Down><Down><Down><Down><Down><Down><Down><Down><Down>",
        "<Down><Down><Down><Down><Down><Down><Down><Down><Down><Down>",
        "<Down><Down><Down><Down><Down><Down><Down><Down><Down><Down>",
        "<Esc>",
        FAST,

        "\n\n",
        "    o  CRISP even comes with a set of regression tests to check out\n",
        "       itself and make sure it is working properly. This is available\n",
        "       as the regress macro...\n",
        SLOW, "<F10>regress\n",
        FAST,

        "\n",
        "    o  Shell buffers...",
        "<F3><Down>",
        SLOW,
        "<F10>sh\n",

        PAUSE, "df<Enter>", PAUSE, PAUSE, "ls<Enter>", PAUSE,
        PAUSE, "<F1><Up><F4><Down>",
        FAST,

        "\n\n",
        "    o  Towers of Hanoi...",
        SLOW,
        "<F10>hanoi 3<Enter> ",
        FAST, "\n\n          = = =    T H E   E N D   = = ="
    };


static int      demo_index;             /* Index into demo_strings for current element */
static string   demo_str;               /* Current string being processed */
static int      demo_fast;
static int      demo_slow;
static int      demo_tmo;               /* Current delay between characters */


void
demo(/*[fast], [slow]*/)
{
    demo_fast = demo_slow = 0;

    get_parm(0, demo_fast);
    get_parm(1, demo_slow);

    if (demo_fast <= 0)
        demo_fast = TIMEOUT_FAST;

    if (demo_slow <= 0)
        demo_slow = TIMEOUT_SLOW;

    if (demo_slow > demo_fast)
        demo_slow = demo_fast;

    /*
     *  Drop three bookmarks corresponding to the ones we'll use in demo mode.
     *  This way we can predict that we WILL be prompted to overwrite bookmark
     *  and can handle that
     */
    drop_bookmark(0, "y");
    drop_bookmark(1, "y");
    drop_bookmark(2, "y");

    edit_file("/tmp/Demonstration-Mode");
    set_buffer_flags(NULL, BF_NO_UNDO);

    clear_buffer();

    demo_index = 0;
    demo_str = "";
    demo_tmo = demo_slow;

    message("Demo package started.");
    register_macro(REG_KEYBOARD, "demo_1");
}


int
demo_1()
{
    declare v;
    string fn;
    int i;

    /* Force a wait  */
    read_char(demo_tmo);

    /* See if we need to get the next string  */
    while ("" == demo_str) {
        if (demo_index >= length_of_list(demo_list)) {
            unregister_macro(REG_KEYBOARD, "demo_1");
            message("Demonstration Complete.");
            return 0;
        }

        v = demo_list[demo_index++];
        if (is_integer(v)) {
            switch (v) {
            case PAUSE:
                read_char(demo_slow * PAUSE_DELAY);
                break;

            case SET_DELAY:
                demo_tmo = demo_list[demo_index++];
                if (demo_tmo <= 0) {
                    demo_tmo = demo_slow;
                }
                read_char(demo_slow * PAUSE_DELAY);
                break;

            case SLOW:
                demo_tmo = demo_slow;
                break;

            case FAST:
                demo_tmo = demo_fast;
                break;
            }
        } else {
            demo_str = v;
        }
    }

    if (substr(demo_str, 1, 1) == "<") {
        i = index(demo_str, ">");
        fn = substr(demo_str, 1, i);
        demo_str = substr(demo_str, i + 1);
        push_back(key_to_int(fn));

    } else {
        push_back(key_to_int(substr(demo_str, 1, 1)));
        demo_str = substr(demo_str, 2);
    }
    return 1;
}
