/* $id: menu.cr,v 1.11 2002/02/27 18:25:42 adamy Exp $
 * Menu macros for text mode.
 *
 *
 */

#include "grief.h"

#define MENU_ITEMS      8

#define MENU_ABORT      -1
#define MENU_RIGHT      -2
#define MENU_LEFT       -3

#if defined(__PROTOTYPES__)
static void             menu_build(void);
static void             menu_draw(void);
static void             menu_exit(string cmd);
static void             menu_alpha(void);
static void             menu_left(void);
static void             menu_right(void);
static void             menu_down(void);
static int              menu_popup(list menudef, int x, int y);
#endif

static int              menu_buf = -1;
static int              menu_win = -1;
static int              menu_kbd = -1;
static int              menu_idx;

static int              old_buf, old_win;

static string           menu_bar =
    "&File  &Edit  &View  &Find  &Options  &Tools  &Windows  &Help";

static list             menus =
    {
        {
            "File",
            " Command line",                "execute_macro",
            " Open",                        "edit__file",
            " Close",                       "delete_curr_buffer",
            " Insert file",                 "read_file",
            "", "",
            " Save File",                   "write_buffer",
            " Save file as ...",            "write_buffer_as",
            " Filter region",               "pipe_region",
            " Save all buffers",            "write_buffers",
            "", "",
            " Reload file",                 "edit_again",
            " Rename file",                 "output_file",
            " Make file writable",          "rw",
            "", "",
            " Change directory",            "change_directory",
            " Print",                       "print",
            " Exit",                        "exit"
        },

        {   "Edit",
            " Undo",                        "undo",
            " Redo",                        "redo",
            "", "",
            " Cut",                         "cut",
            " Copy",                        "copy",
            " Paste",                       "paste",
            "", "",
            " Delete",                      "delete_char",
            " Toggle insert",               "insert_mode",
            " Tab to spaces",               "block_tab_to_space",
            " Spaces to tabs",              "block_space_to_tab",
            "", "",
            " Macro start/stop",            "remember",
            " Macro playback",              "playback"
        },

        {   "View",
            " Literal Toggle",              "literal",
            " Bookmarks",                   "compl_bookmark",
            " Buffers",                     "buffer_list 1",
            " Keystroke macros",            "keylib",
            " Routines",                    "routines",
        },

        {   "Find",
            " Search for...",               "search__fwd",
            " Search&replace...",           "translate__fwd",
            " File find",                   "ff",
            " Text search...",              "ts",
            " Grep",                        "grep",
            "", "",
            " Find Matching brace",         "find_matching_brace",
            " Goto bookmark",               "goto__bookmark",
            " Goto line",                   "goto__line",
            " Goto tag",                    "tag",
        },

        {   "Options",
            " Color Scheme",                "colorscheme",
            " Base colors",                 "setcolor",
            " Buffer settings",             "options",
        },

        {   "Tools",
            " ASCII Chart...",              "ascii",
            " Operator Hierarchy...",       "hier",
            " Change Log",                  "change",
            " Features",                    "feature",
            "", "",
            " Make...",                     "make",
        },

        {   "Windows",
            " Scroll lock",                 "scroll",
            " Toggle borders",              "borders",
            " Zoom/unzoom",                 "zoom",
            " List",                        "buffer_list 2",
            "", "",
            " Change window",               "change_window",
            " Move edge",                   "move_edge",
            " Split window",                "create_edge",
            " Delete window",               "delete_edge",
        },

        {   "Help",
            " Index",                       "help",
            " Explain",                     "explain",
            "", "",
            " About",                       "help_about",
        }
    };


/*
 *  menubar ---
 *      activate the menu bar
 */
void
menubar(void)
{
    menu_build();

    old_buf = inq_buffer();
    old_win = inq_window();
    set_window(menu_win);
    set_buffer(menu_buf);
    attach_buffer(menu_buf);
    insert_mode(TRUE, menu_buf);                /* enable insert */
    use_local_keyboard(menu_kbd);
    beginning_of_line();
    menu_idx = MENU_ITEMS;
    menu_right();
}


/*
 *  menuon ---
 *      Enable the menu
 */
void
menuon(void)
{
    menu_build();
}


/*
 *  menuoff ---
 *      Disable the menu
 */
void
menuoff(void)
{
    if (menu_win >= 0)
    {
        delete_window(menu_win);
        delete_buffer(menu_buf);
        menu_win = menu_buf = -1;
        redraw();
    }
}


/*
 *  menu ---
 *      Toggle the menu status
 */
void
menu(void)
{
    if (menu_win == -1) {
        menuon();
    } else {
        menuoff();
    }
}


/*
 *  menu_build ---
 *      Build and display the menu bar
 */
static void
menu_build(void)
{
    int oldbuf, oldwin;

    if (menu_win != -1)
        return;

    oldbuf = inq_buffer();
    oldwin = inq_window();
    if ((menu_buf = create_buffer("-menu-buffer-", NULL, 1)) == -1)
        return;

    set_buffer(menu_buf);
    menu_win = create_menu_window();            /* create menu bar */
    set_window(menu_win);
    attach_buffer(menu_buf);

    if (menu_kbd == -1) {
        keyboard_push();
        keyboard_typeables();
        assign_to_key("<Tab>",                 "::menu_right");
        assign_to_key("^B",                    "::menu_left");
        assign_to_key("^F",                    "::menu_right");
        assign_to_key("<Left-Arrow>",          "::menu_left");
        assign_to_key("<Shift-Tab>",           "::menu_left");
        assign_to_key("<Right-Arrow>",         "::menu_right");
        assign_to_key("<Enter>",               "::menu_down");
        assign_to_key("<Down-Arrow>",          "::menu_down");
        assign_to_key("^N",                    "::menu_down");
        assign_to_key("<Esc>",                 "::menu_exit");
        assign_to_key("<Del>",                 "nothing");
        assign_to_key("<F10>",                 "nothing");
        assign_to_key("<F11>",                 "::menu_exit");
        register_macro(REG_TYPED,              "::menu_alpha", TRUE);
        menu_kbd = inq_keyboard();
        keyboard_pop(1);                        /* save keyboard */
    }

    menu_draw();
    redraw();

    set_buffer(oldbuf);
    set_window(oldwin);
    attach_buffer(oldbuf);
}


/*
 *  menu_draw ---
 *      draw the menu bar
 */
static void
menu_draw(void)
{
    string text;
    int len, i;

    text = menu_bar;
    set_buffer_flags(NULL, BF_ANSI);            /* enable ansi */
    beginning_of_line();
    insert("\033[0m");
    while ((len = strlen(text)) > 0) {
        if ((i = index(text, "&")) <= 0) {
            /* display remaining */
            i = 1;

        } else if (i > 1) {
            /* display upto & */
            len = i-1, i = 1;

        } else {
            /* highlight next char */
            insertf("\033[1m" + "%s" + "\033[0m", substr(text, 2, 1));
            len = 2, i = 0;
        }
        if (i == 1) {
            insert(substr(text, 1, len));
        }
        text = substr(text, len+1);
    }
}


/*
 *  menu_exit ---
 *    exit the menu bar.
 */
static void
menu_exit(string cmd)
{
    set_buffer(old_buf);
    set_window(old_win);
    attach_buffer(inq_buffer());
    if (cmd) {
        refresh();
        execute_macro(cmd);
    }
}


/*
 *  menu_alpha ---
 *      process 'alpha' key hits
 */
static void
menu_alpha(void)
{
    string ch;

    prev_char();
    save_position();
    ch = upper(read(1));
    delete_char();
    if (re_search(SF_UNIX, "\\<" + quote_regexp(ch)) <= 0) {
        beginning_of_line();
        if (re_search(SF_UNIX, "\\<" + quote_regexp(ch)) > 0) {
            restore_position(0);

        } else {
            restore_position();
            return;
        }
    } else {
        restore_position(0);
    }
    drop_anchor(MK_NORMAL);
    re_search(SF_UNIX, "\\>");
}


/*
 *  menu_left ---
 *      move the menu cursor to the left, wrapping if required.
 */
static void
menu_left(void)
{
    raise_anchor();
    if (--menu_idx >= 0) {
        re_search(SF_UNIX | SF_BACKWARDS, "[A-Z]");
        left();
    } else {
        end_of_line();
        menu_idx = MENU_ITEMS-1;
    }
    re_search(SF_UNIX | SF_BACKWARDS, "[A-Z]");
    drop_anchor(MK_NORMAL);
    re_search(SF_UNIX, " ");
    prev_char();
}


/*
 *  menu_right ---
 *      move the menu cursor to the right, wrapping if required.
 */
static void
menu_right(void)
{
    raise_anchor();
    if (++menu_idx >= MENU_ITEMS) {
        beginning_of_line();
        menu_idx = 0;
    }
    re_search(SF_UNIX, "[A-Z]");
    drop_anchor(MK_NORMAL);
    if (! re_search(SF_UNIX, " ")) {
        end_of_line();
    }
    prev_char();
}


/*
 *  menu_down ---
 *      Process a down key, popup a menu associated with the current option.
 */
static void
menu_down(void)
{
    int ret;

    while (1) {
        ret = menu_popup(menus[menu_idx], menu_idx, 2);
        if (ret >= 0) {
            menu_exit(menus[menu_idx][2 * (ret - 1) + 2]);
            return;
        }

        set_buffer(menu_buf);
        switch (ret) {
        case MENU_LEFT:
            menu_left();
            break;
        case MENU_RIGHT:
            menu_right();
            break;
        case MENU_ABORT:
            menu_exit("");
            return;
        }
    }
}


/*
 *  menu_popup ---
 *    build and process a sub-menu.
 */
static int
menu_popup(list menudef, int x, int y)
{
    list    key_lst;
    string  seperator, item;
    int     width, len;
    int     curbuf, buf, win;
    int     i, j, k;
    int     ret;

    UNUSED(x, y);

    /* build menu */
    curbuf = inq_buffer();                      /* current buffer */
    key_lst = key_list(NULL, NULL, old_buf);

    len = length_of_list(menudef);
    for (i = 1; i < len; i += 2) {              /* determine width */
        if (width < (j = strlen(menudef[i]))) {
            width = j;
        }
    }
    width += 4;

    buf = create_buffer(menudef[0], NULL, TRUE);
    set_buffer(buf);

    seperator = "  ";                           /* build seperator */
    for (i = 5; i < width; ++i) {
        seperator += "-";
    }

    for (i = 1; i < len; i += 2) {              /* build menu */
        if (menudef[i] == "") {
            insert(seperator);
        } else {
            insert(menudef[i]);                 /* item */
            move_abs(0, width);

            k = 0;                              /* hotkey count */
            j = -1;                             /* index */
            item = "<" + quote_regexp(menudef[i+1]) + ">";
            while (k < 3 &&                     /* first 2 matches */
                        (j = re_search(SF_MAXIMAL, item, key_lst, j+1)) >= 0)
            {
                if (substr(key_lst[j-1], 1, 1) == "#")
                    continue;                   /* ignore specials */
                if (k++)
                    insert(",");                /* delimiter */
                insert(key_lst[j-1]);           /* text */
            }

            if (k == 0) {
                insert("<F10> "+menudef[i+1]);  /* default, <F10> xxx */
            }
        }
        insert("\n");
    }
    delete_line();
    set_buffer(curbuf);                         /* restore buffer */

    /* select */
    if ((i = re_search(SF_NOT_REGEXP, menudef[0], menu_bar )) > 0) {
        i -= menu_idx + 1;
    }

    win = sized_window(inq_lines(buf) + 1, inq_line_length(buf) + 1, NULL, i, 1);
    ret = select_buffer(buf, win, SEL_NORMAL, "menu_keys", NULL, NULL);

    /* cleanup */
    delete_buffer(buf);
    delete_window(win);
    return ret;
}


/*
 *  menu_keys ---
 *    local key bindings for menu_popup
 */
void
menu_keys(void)
{
    assign_to_key("<Enter>",    "sel_enter");
    assign_to_key("<Esc>",      "sel_exit -1");
    assign_to_key("<Left>",     "sel_exit -3");
    assign_to_key("<Right>",    "sel_exit -2");
}


void
change_directory()
{
    string dir;

    if (get_parm(NULL, dir, "Change to: ") <= 0) {
        message("");
        return;
    }
    set_calling_name("");
    cd(dir);
}


/*
 *  Local Variables: ***
 *  mode: cr ***
 *  tabs: 4 ***
 *  End: ***
 */
