/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: texinfo.cr,v 1.7 2014/10/27 23:28:29 ayoung Exp $
 * GNU style TeX info interface.
 *
 *
 */

#include "grief.h"

#if defined(MSDOS)
#define INFO_PATH   "c:/cygwin/usr/share/info"
#else
#define INFO_PATH   "/usr/share/info"
#endif
#define INFO_EXT    ".info"

static string       visit_node(string node);

static void         tex_menu(void);
static void         tex_home(void);
static void         tex_end(void);
static void         tex_tab(int direction);
static void         tex_prev(void);
static void         tex_next(void);
static void         tex_up(void);


void
texinfo(~string info)
{
    int cur_buf = inq_buffer();
    string info_file;
    string info_dir;
    string info_name;
    string s, f;
    int l, line;
    list indir_list;
    int indir_len;
    list node_list;
    int node_len;
    int buf;
    string new_node;
    list lst;

    if (info == "")
        get_parm(NULL, info, "texinfo: ");

    info_dir = dirname(info);
    if (info_dir == "" || (info_dir == "." && substr(info,1,1) != "."))
        info_dir = INFO_PATH;
    info_dir += "/";

    info_file = basename(info, INFO_EXT);
    info_name = info_file;
    if (exist(info_dir + info_file + INFO_EXT))
        info_file += INFO_EXT;                  /*append .info*/

    save_position();
    if ((buf = create_buffer(info_name + " - Info", info_dir + info_file, 1)) <= 0) {
        error("Cannot open: %s%s", info_dir, info_file);
        return;
    }

    set_buffer(buf);

    /* Process indirection table.
     *   If it exists, files shall be broken into small and reasonably sized chunks.
     */
    top_of_buffer();
    if (re_search(NULL, "<Indirect:$") > 0) {
        down();
        while (rtrim(read()) != "\037") {
            s = read();
            l = index(s, ": ");
            f = substr(s, 1, l - 1);
            line = atoi(substr(s, l + 2));
            indir_list += make_list(line, f);
            down();
        }
    }
    indir_len = length_of_list(indir_list);

    /* Process Node the table.
     *   Create a list of <Node-name,line-no> pairs.
     */
    while (re_search(NULL, "<{Node}|{Ref}: \\c") > 0) {
        lst = split(read(), "\x7f");
        lst[1] = atoi(lst[1]);
        node_list += lst;
    }
    node_len = length_of_list(node_list);

    delete_buffer(buf);

    set_buffer(cur_buf);
    attach_buffer(cur_buf);
    restore_position();

    /* Start at the top-most node of the tree and keep on calling the visit_node
     * function to display each section of information.
     */
    new_node = "Top";
    while ((new_node = visit_node(new_node)) != "")
        ;
}


/* Following function takes name of a node to visit, looks it up
 * in the node list and creates a popup window with the
 * information on the screen, allowing the user to browse around.
 */
static string
visit_node(string node)
{
    extern list   node_list;
    extern int    indir_len;
    extern list   indir_list;
    extern string info_file;
    extern string info_dir;
    extern string info_name;

    int    cur_buf = inq_buffer();
    int    byte_pos;
    int    win, buf, win_size;
    int    line_top, line_bottom;
    int    i, ret;

    string file;
    string directions;
    string action = "";
    string s;

    /* Firstly find out which file the node is in. */
    if (node == "(dir)")
        node = "Top";

    i = re_search(NULL, quote_regexp(node), node_list);
    if (i < 0) {
        error("Cannot find entry: %s", node);
        return "";
    }

    byte_pos = node_list[i + 1];
    file = info_file;

    /* Search indirect table. */
    for (i = indir_len - 2; i >= 0; i -= 2) {
        if (byte_pos >= indir_list[i]) {
            byte_pos -= indir_list[i];
            file = indir_list[i + 1];
            break;
        }
    }

    if ((buf = create_buffer(info_name + " - Info", info_dir + file, 0)) <= 0) {
        error("Cannot open: %s%s", info_dir, file);
        return "";
    }
    message("%s: %s%s", node, info_dir, file);

    /* Seek the position */
    set_buffer(buf);
    top_of_buffer();
    next_char(byte_pos);
    s = rtrim(read());
    if (substr(s, 1, 5) != "File:") {
        if (s != "\037")
            re_search(NULL, "\037");
        down();
    }
    inq_position(line_top);

    /* Save the directions about what to do if use hits the Next/Prev/Up keys.
     *
     *  File: grief.info,  Node: Top,  Next: Introduction,  Up: (dir)
     */
    directions = rtrim(read());

    /*
     * Work out how many lines are in this section so that we can
     * create a window appropriate for this section.
     */
    next_char();
    if (re_search(NULL, "^\037") <= 0)
        end_of_buffer();
    inq_position(line_bottom);
    win_size = (line_bottom - line_top) + 1;

    win = sized_window(win_size, 80, int_to_key(ALT_M) + " Menu" + "<Tab/Return> Topic," +
                int_to_key(ALT_N) + " Next, " + int_to_key(ALT_P) + "Prev, " + int_to_key(ALT_U) + " Up");

    ret = select_buffer(buf, win, SEL_NORMAL | SEL_TOP_OF_WINDOW, "tex_keys", NULL, NULL, line_top);

    if (ret >= 0) {
        beginning_of_line();
        s = rtrim(read());
    }

    delete_buffer(buf);
    set_buffer(cur_buf);
    attach_buffer(cur_buf);

    /* If user aborted selection, then give up. */
    if (ret < 0 && action == "")
        return "";

    /* If user selects something, then check for a menu item and go directly to that. */
    if (ret > 0) {
        /*
         *  Look for a cross reference match.
         *
         *  Examples:
         *
         *      ... "* Note Introduction:: "
         *        < "* Introduction::"
         */
        if ((ret = re_search(NULL, "\\*[Nn]ote ", s)) > 0) {
            s = substr(s, ret + 6);
            s = substr(s, 1, index(s, ":") - 1);
        } else {
            if (substr(s, 1, 2) != "* ")
                return "";
            s = substr(s, 3);
            s = substr(s, 1, index(s, ":") - 1);
        }
        return s;
    }

    /* The user hit a next/prev/up key, so process the entry. */
    i = re_search(NULL, action, directions);
    if (i <= 0)
        return node;
    s = substr(directions, i + strlen(action));
    i = index(s, ",");
    if (i > 0)
        s = substr(s, 1, i - 1);
    return s;
}


/*
 *  tex_keys --
 *      Macro called by select_buffer() to set up private key mappings
 *      for the popup window.
 */
void
tex_keys()
{
    assign_to_key("<Alt-P>",        "::tex_prev");
    assign_to_key("<Alt-N>",        "::tex_next");
    assign_to_key("<Alt-U>",        "::tex_up");
    assign_to_key("<Alt-M>",        "::tex_menu");
    assign_to_key("<Home>",         "::tex_home");
    assign_to_key("<End>",          "::tex_end");
    assign_to_key("<Tab>",          "::tex_tab 1");
    assign_to_key("<Shift-Tab>",    "::tex_tab 0");
}


/*
 *  tex_menu ---
 *      Display a popup menu of items in the Menu part of this section and
 *      allow user to select from that.
 */
static void
tex_menu(void)
{
    extern int line_top, line_bottom;
    list lst;
    string s;
    int ret;

    save_position();
    move_abs(line_top);
    if (re_search(NULL, "^{\037}|{\\* Menu}") > 0) {
        down();
        while (re_search(NULL, "^{\037}|{\\* }") > 0) {
            if (rtrim(read(1)) == "\037")
                break;                          /*end-of-section*/
            s = substr(read(), 3);
            lst += substr(s, 1, index(s, ":") - 1);
            down();
        }
    }
    restore_position();
    if (length_of_list(lst) == 0) {
        message("No links available within this section.");
        return;
    }
    ret = select_list("Selection Menu", "", 1, lst, SEL_CENTER);
    if (ret < 0)
        return;
    re_search(NULL, "^\\* " + lst[ret-1] + "::");
    raise_anchor();
    drop_anchor(MK_LINE);
    push_back(key_to_int("<Enter>"));
}


static void
tex_home(void)
{
    raise_anchor();
    top_of_window();
    drop_anchor(MK_LINE);
}


static void
tex_end(void)
{
    raise_anchor();
    end_of_window();
    drop_anchor(MK_LINE);
}


static void
tex_tab(int fwd)
{
    extern int line_top, line_bottom;
    int ret;

    save_position();
    raise_anchor();

    if (fwd) {                                  /* forward selection */
        down();
        ret = re_search(NULL, "<\\**::");
    } else {                                    /* backwards */
        up();
        ret = re_search(SF_BACKWARDS, "<\\**::");
    }

    if (ret > 0) {
        int line;

        inq_position(line);
        if (line >= line_top && line <= line_bottom) {
            restore_position(0);
        } else {
            ret = 0;
        }
    }
    if (ret <= 0) {
        restore_position(1);
        beep();
    }

    drop_anchor(MK_LINE);
}


static void
tex_prev(void)
{
    extern string action;

    action = "Prev: ";
    push_back(key_to_int("<Esc>"));
}


static void
tex_next(void)
{
    extern string action;

    action = "Next: ";
    push_back(key_to_int("<Esc>"));
}


static void
tex_up(void)
{
    extern string action;

    action = "Up: ";
    push_back(key_to_int("<Esc>"));
}

/*eof*/
