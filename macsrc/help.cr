/* $Id: help.cr,v 1.52 2024/09/06 14:35:57 cvsuser Exp $
 * Help subsystem.
 *
 *
 */

#include  "grief.h"
#include  "help.h"

#if defined(MSDOS) || defined(OS2)
#define DELIMIT         ";"
#else
#define DELIMIT         ":"
#endif

#define KDB_SUMMARY     "kdb_summary_coloriser"

#define WINDOW_WIDTH    82                      /* GRIEF help width */

#define HT_FTOPSTOP     0x10                    /* dont exit on TOS */
#define HT_PRIMITIVE    0x01
#define HT_CONTEXT      0x02
#define HT_FEATURE      0x03
#define HT_CSHELP       0x04
#define HT_FILE         0x05                    /* type mask */
#define HT_MASK         0x0f

#define HR_DISABLED     -7                      /* help_ret values */
#define HR_APROPOS      -6
#define HR_SPACE        -5
#define HR_EXIT         -4
#define HR_ESC          -3
#define HR_PREV         -2
#define HR_INACTIVE     -1

#if defined(__PROTOTYPES__)
static int              help_request(int type, string topic, ~string refer);
static int              help_primitive(string topic);
static int              help_context(string dir, string topic, int keyword);
static int              help_file(string topic);
static list             help_sumappend(list lst, string key, string mac);
static void             help_sumlist(string title, list l);
void                    help_mapkeys();
static int              help_rtopic(string entry);
static int              help_rpush(string entry);
#endif

static int              help_level, help_ret;
static list             help_paths;

static list             help_menulist = {
    "Keyboard Summary    ",       "kdb_summary",
    "Keyboard Mapping    ",       "kdb_mapping",
    "Macro Primitives  =>",       "help_primitives",
    "Explain           =>",       "explain",
    "Features          =>",       "feature_help",
    "Programmer Utils  =>",       "help_display \"features/program.hlp\" \"Programmer Utils\"",
    "User Guide        =>",       "ninfo \"user\"",
    "Programmers Guide =>",       "ninfo \"prog\"",
 // "Config Guide      =>",       "ninfo \"config\"",
    "About               ",       "about",
    };

static list             help_ninfolist = {
    "prim.idx",
    "user.idx",
    "prog.idx",
 // "config.idx"
    };

static list             help_primitiveslist = {
    //
    //  Primitive Sections/
    //      these *must* match the section labels contained within 'gr/keywd.c' and 'hlpdoc/makehelp.pl'.
    //
    "All - Table of Contents",    "help_section",
    "Arithmetic Ops",             "help_section \"arith\"",
    "Buffers",                    "help_section \"buffer\"",
    "Dialog",                     "help_section \"dialog\"",
    "Debugging",                  "help_section \"debug\"",
    "Environment",                "help_section \"env\"",
    "Files",                      "help_section \"file\"",
    "Floating Point",             "help_section \"float\"",
    "Keyboard",                   "help_section \"kbd\"",
    "Lists",                      "help_section \"list\"",
    "Macro Primitives",           "help_section \"macro\"",
    "Miscellaneous",              "help_section \"misc\"",
    "Movement",                   "help_section \"movement\"",
    "Process Management",         "help_section \"proc\"",
    "Scrap",                      "help_section \"scrap\"",
    "Screen",                     "help_section \"screen\"",
    "Search & Translate",         "help_section \"search\"",
    "String Manipulation",        "help_section \"string\"",
    "Syntax Highlighting",        "help_section \"syntax\"",
    "Variable Declaration",       "help_section \"var\"",
    "Windows",                    "help_section \"window\""
    };


/*  Function:       main
 *      Macro mainline
 */
void
main()
{
    string path;

    if ("" == (path = getenv("GRHELP")) &&
            "" == (path = getenv("BHELP"))) {
        error("GRHELP/BHELP environment variable not set.");

    } else {
        /* bust GRHELP */
        int i;

        help_paths = split(path, DELIMIT);

        /* remove non-existent paths */
        for (i = 0; i < length_of_list(help_paths); i++) {
            if (realpath(help_paths[i], path) == 0) {
                help_paths[i] = path;
            }
            if (access(help_paths[i], 0) == -1) {
                delete_nth(help_paths, i);
                --i;
            }
        }

        /* remove duplicates */
        for (i = 0; i < length_of_list(help_paths); i++) {
            int i2;

            for (i2 = i + 1; i2 < length_of_list(help_paths); i2++)
                if (help_paths[i] == help_paths[i2]) {
                    delete_nth(help_paths, i2);
                    --i2;
                }
        }
    }

    help_level = 0;
    create_syntax(KDB_SUMMARY);
    syntax_rule("\\[\\[.*\\]\\]", "comment");
    syntax_rule("<.*>", "keyword");
    syntax_rule("#[0-9]*", "keyword");
    syntax_build(__COMPILETIME__);
}


/*  Function:       help
 *      Top level help menu.
 *
 */
void
help(void)
{
    extern int top_line;                        /* sized_window() globals */
    int users_buf, cur_keyboard;                /* globals */
    int s_top_line;

    help_level++;
    s_top_line = top_line;
    users_buf = inq_buffer();
    cur_keyboard = inq_keyboard();
    select_list("Help Menu", "", 2,
        help_menulist, SEL_CENTER, NULL, NULL, 0, "help_menukeys");
    top_line = s_top_line;
    if (help_level > 0) {
        --help_level;
    }
}


/*  Function:       help_about
 *      About dialog.
 *
 */
void
help_about(void)
{
    string  machtype, compiled;
    int     curbuf, aboutbuf, aboutwin;
    int     lines, width;
    int     maj, min, edit, rel, cmver;

    curbuf = inq_buffer();
    if ((aboutbuf = create_buffer("About", NULL, 1)) < 0) {
        return;
    }
    set_buffer(aboutbuf);
    insert("\n");
    version(maj, min, edit, rel, machtype, compiled, cmver);
    insertf("%s %s v%d.%d.%d (%d)\n\n", APPNAME, machtype, maj, min, edit, cmver);
    insertf("Built on %s\n", compiled);
    lines = inq_lines() + 1;
    width = inq_line_length() + 2;
    set_buffer(curbuf);                         /* restore buffer */
    aboutwin = sized_window(lines, width, "");
    select_buffer(aboutbuf, aboutwin, SEL_CENTER, "", NULL, NULL);
    delete_buffer(aboutbuf);                    /* release local buffer */
}


/*  Function:       help_primitives
 *      Macro primitives help menu.
 *
 */
void
help_primitives(void)
{
    help_level++;
    select_list("Macro Subjects", "", 2,
        help_primitiveslist, SEL_CENTER, NULL, NULL, 0, "help_menukeys");
    if (help_level > 0) {
        help_level--;
    }
}


/*  Function:       help_section
 *      Find and display the specified help section.
 *
 */
void
help_section(string section)
{
    string filename, cmd;
    int curbuf, buf, win;
    int width, lines, ret;

    curbuf = inq_buffer();
    if (length_of_list(help_paths) == 0) {
        error("GRHELP environment variable not set.");
        return;
    }

    if (section == "") {
        filename += "prim.toc";                 /* Table-Of-Contents */
    } else {
        filename += section + ".sec";           /* Primitive section */
    }
    filename = help_resolve(filename);

    if ((buf = create_buffer("Section Index", filename, TRUE)) < 0) {
        set_buffer(curbuf);

    } else {
        /*
         *  Display the section text, centered
         */
        ret = 0;
        while(1) {
            set_buffer(buf);
            if (ret > 0) move_abs(ret, 1);
            width = inq_line_length() + 2;
            if (width < 18) width = 18;
            lines = inq_lines() + 1;

            ++help_level;
            help_ret = HR_DISABLED;             /* refer processing */

            set_buffer(curbuf);                 /* restore buffer */
            win = sized_window(lines, width, "");
            ret = select_buffer(buf, win, (ret > 0 || section == "" ? 0 : SEL_CENTER),
                    "help_menukeys", NULL, NULL, ret );
            if (help_level > 0) {
                --help_level;
            }
            if (ret <= 0 || help_ret == HR_PREV) {
                break;
            }

            set_buffer(buf);
            cmd = rtrim(read());
            if (section == "") {
                cmd = substr(cmd, 1, index(cmd, " "));
            }
            set_buffer(curbuf);

            if (help_request(HT_PRIMITIVE, cmd) == -1) {
                break;
            }
        }
        delete_buffer(buf);                     /* release local buffer */
    }
}


/*  Function:       help_menukeys
 *      Additional select buffer keys for help, help_primitives and help_section menus,
 *      allowing arrows to navigate.
 *
 */
void
help_menukeys(void)
{
    assign_to_key("<F10>",       "::help_exit");
    assign_to_key("<Backspace>", "::help_prev");
    assign_to_key("<Space>",     "sel_esc");
    assign_to_key("<Left>",      "sel_esc");
    assign_to_key("<Right>",     "sel_list");
}


/*
 *  explain ----
 *      Retrieve and display the help related to the specified primitive.
 */
int
explain(~string, ...)
{
    string topic, t_topic;
    int idx = 0;

    if (get_parm(idx++, topic, "Explain: ") > 0) {
        while (get_parm(idx++, t_topic, NULL) > 0) {
            topic += " " + t_topic;             // multiple words
        }
        help_request(HT_FTOPSTOP | HT_PRIMITIVE, topic);
    }
}


string
_completion_Explain(string word)
{
    return word;
}


/*  Function:       cshelp
 *       Retrieve and display the specific context help.
 *
 *  Notes:
 *      <Tab> command line completion for explain shall glob the known primitive list
 *      via the associated abbreviations list.
 */
void
cshelp(string dir, string topic)
{
    if (dir == "cshelp") {
        help_request(HT_CSHELP, topic);

    } else if (dir == "features") {
        help_request(HT_FEATURE, topic);

    } else {
        help_request(HT_CONTEXT, topic, dir);
    }
}


/*  Function:       help_resolve
 *      Find the associated help file image within the GRHELP specification.
 *
 */
string
help_resolve(string filename)
{
    int i;

    filename = "/" + filename;
    for (i = 0; i < length_of_list(help_paths); i++)
        if (access(help_paths[i] + filename, 0) != -1) {
            return help_paths[i] + filename;
        }
    return "." + filename;
}


/*  Function:       help_display
 *      Load and display the specified help file.
 *
 *  Parameters:
 *      file -          File name, with prefix path.
 *      title -         Title (optional)
 *      section -       Initial section (optional)
 */
void
help_display(string file, string title, declare section)
{
    string topic;

    topic = file;                               /* source */

    if (title != "") {                          /* title */
        topic += ",title=" + title;
    }

    if (is_integer(section)) {                  /* section */
        topic += ",lineno=" + section;

    } else if (is_string(section) && section != "") {
        topic += ",section=" + section;
    }

    help_request(HT_FILE, topic);
}


/*  Function:       help_request
 *      Process a help request.
 *
 *  Parameters:
 *      type -          Request type flags.
 *      topic -         Topic.
 *      refer -         References.
 */
static int
help_request(int type, string topic, ~string refer)
{
    list ptype, ptopic, pline;                  /* previous stack */
    int flags, curbuf, curwin, buf;
    int width, line, i;
    int pidx;

    if (0 == length_of_list(help_paths)) {
        error("GRHELP environment variable not setup.");
        return 0;
    }

    flags = type & ~HT_MASK;                    /* control flags */
    type &= HT_MASK;                            /* type */

    curbuf = inq_buffer();                      /* current buffer */
    curwin = inq_window();

    while ((topic = trim(topic)) != "") {
        /* specified */
        if (type == HT_FILE) {
            buf = help_file(topic);

        } else if (type == HT_CONTEXT) {
            buf = help_context(refer, topic, 0);

        } else if (type == HT_PRIMITIVE) {
            if ((buf = help_primitive(topic)) >= 0) {
                inq_position(line);
            }

        } else if (type == HT_FEATURE) {
            buf = help_context("features", topic, 1);

        } else if (type == HT_CSHELP) {
            buf = help_context("cshelp", topic, 1);
        }

        /* .. alternatives */
        if (buf < 0 && type != HT_PRIMITIVE) {
            if ((buf = help_primitive(topic)) >= 0) {
                type = HT_PRIMITIVE;
                inq_position(line);
            }
        }

        if (buf < 0 && type != HT_FEATURE) {
            if ((buf = help_context("features", topic, 1)) >= 0) {
                type = HT_FEATURE;
            }
        }

        if (buf < 0 && type != HT_CSHELP) {
            if ((buf = help_context("cshelp", topic, 1)) >= 0) {
                type = HT_CSHELP;
            }
        }

        /* handle error conditions */
        if (buf < 0) {
            /*
             *  Dont print errors for context-sensitive help because we have prompt on
             *  the status line so there's nowhere to print out message.
             */
            if (inq_called() != "cshelp") {
                error("Sorry, no help topic (%s).", topic);
            }
            beep();
            if (pidx > 0) {                     /* pop */
                pidx--;
                type = ptype[pidx]; topic = ptopic[pidx]; line = pline[pidx];
                continue;
            }
            return 1;                           /* no match */
        }

        /*  Display the help window,
         *
         *      Limit primitive pages to the width of 70 being the standard formating for GRIEF help.
         */
        if (type == HT_FILE) {                  /* width, current line */
            width = -WINDOW_WIDTH;
            line = -1;

        } else if (type == HT_PRIMITIVE) {      /* width, top */
            width = -WINDOW_WIDTH;

        } else {    /*CONTEXT|FEATURE*/
            width = 0;
        }

        refer = help_window(HELP_GRIEF, buf, 0, width, line);

        set_buffer(curbuf);                     /* restore window */
        set_window(curwin);
        attach_buffer(curbuf);

        delete_buffer(buf);                     /* release local buffer */
        buf = -1;

        ptype[pidx] = type; ptopic[pidx] = topic; pline[pidx] = line;

        /* Process completion */
        if (refer == "--exit--") {
            return -1;                          /* destroy and exit */
        }

        if (refer == "" || refer == "--space--" || refer == "--esc--") {
            break;                              /* destroy and exit */
        }

        if (refer == "--prev--") {
            if (pidx) {
                pidx--;                         /* pop */
            } else {
                if ((flags & HT_FTOPSTOP) == 0) {
                    break;
                }
                beep();                         /* top */
            }
            type = ptype[pidx]; topic = ptopic[pidx]; line = pline[pidx];

        } else {            /* new topic */
            line = 0;
            topic = compress(refer);
            if ((i = index(topic, "(")) > 0) {  /* topic with section */
                topic = trim(substr(topic, 1, i-1));    /* strip */
            }
            ++pidx;                             /* push */
        }
    }

    return 0;
}


/*  Function:       help_lookup
 *      Lookup the help topic index record.
 *
 *      Search the index file (e.g. prim.idx) to determine the file and
 *      the position of the topic description.
 *
 *  Entry syntax:
 *      primitive-name: file-name start-line [end-line] ["Topic Heading"]
 *
 */
static string
help_lookup(string topic)
{
    int curbuf, idxbuf;
    string fidx, fname;

    curbuf = inq_buffer();

    for (list_reset(help_ninfolist); list_each(help_ninfolist, fidx) >= 0;) {

        fname = help_resolve(fidx);

        if ((idxbuf = create_buffer(fname, fname, TRUE)) < 0) {
            error("help: index '%s' missing.", fidx);

        } else {
            set_buffer(idxbuf);
            top_of_buffer();

            if (re_search(SF_IGNORE_CASE, "<" + quote_regexp(topic) + ":") <= 0) {
                set_buffer(curbuf);             /* restore buffer */
                delete_buffer(idxbuf);          /* release local (index) buffer */

            } else {
                string result = read();

                set_buffer(curbuf);             /* restore buffer */
                delete_buffer(idxbuf);

                return trim(substr(result, strlen(topic)+2));
            }
        }
    }
    return "";
}


/*  Function:       help_primitive
 *      Retrieve the help related to the specified topic, which
 *      include language primitives.
 *
 */
static int
help_primitive(string topic)
{
    int sl, el, hlpbuf;
    string result, fn;
    list lst;

    /* Index lookup */
    result = help_lookup(topic);
    if (0 == strlen(result)) {
        return -1;
    }

    /* Read image */
    lst = split(result, " ", TRUE);             /* file-name start-line [end-line] */
    fn  = help_resolve(lst[0]);
    sl  = lst[1];
    el  = lst[2];

                                                /* FIXME, topic not correct for non-primitive pages */
    if ((hlpbuf = create_buffer(topic, fn, TRUE)) < 0) {
        error("help: '%s' missing.", fn);
        return -1;
    }
    set_buffer(hlpbuf);

    /* Remove text outside section. */
    if (el > sl) {                              /* end > start */
        goto_line(el + 1);                      /* trailing */
        drop_anchor(MK_LINE);
        end_of_buffer();
        delete_block();

        if (sl > 1) {                           /* leading */
            top_of_buffer();
            drop_anchor(MK_LINE);
            goto_line(sl - 1);
            delete_block();
        }
        goto_line(1);

    } else {
        goto_line(sl);
    }
    return hlpbuf;
}


/*  Function:       help_context
 *      Retrieve context-sensitive help on the topic.
 *
 */
static int
help_context(string dir, string topic, int keyword)
{
    string fname;
    int curbuf, idxbuf, hlpbuf;
    int len, sret;

    curbuf = inq_buffer();                      /* current buffer */

    /* Read the index file and see if we can find a matching string */
    if (dir != "") {                            /* full path */
        dir += "/";
    }
    fname = help_resolve(dir + "INDEX");
    if ((idxbuf = create_buffer(fname, fname, TRUE)) < 0) {
        error("help: index '%s' missing.", fname);
        return -1;
    }

    /* Remove trailing newline (if any) */
    len = strlen(topic);
    if (substr(topic, len) == "\n") {
        topic = substr(topic, 1, len - 1);
    }

    /*
     *  Attempt to be intelligent and remove anything on the
     *  right hand side of the menu item.
     */
    topic = ltrim(topic);
    if (substr(topic, strlen(topic) - 1) == "=>") {
        topic = substr(topic, 1, strlen(topic) - 2);

    } else if (re_search(NULL, "  ", topic) > 0) {
        topic = substr(topic, 1, re_search(0, "  ", topic) - 1);
    }
    topic = rtrim(topic);

    /* Search index:
     *
     *  Format:
     *    "Description:[keyword[,...] =]filename"
     *
     *  Examples:
     *    "Print buffer       :print"
     *    "Retab buffer       :entab=detab"
     *    "Detab buffer       :detab,detab_buffer=detab"
     */
    set_buffer(idxbuf);                         /* index buffer */

    sret = re_search(0, "^" + quote_regexp(topic) + "[\\t ]@:\\c");
    if (sret <= 0 && keyword) {                 /* keyword match */
        sret = re_search(0, "[:,]\\c" + quote_regexp(topic) + "{[,=]|>}");
    }
    if (sret <= 0) {
        set_buffer(curbuf);
        delete_buffer(idxbuf);
        return -1;
    }

    /* Retrieve filename */
    fname = trim(read());                       /* remaining of line */
    if ((len = index(fname, "=")) > 0) {
        fname = substr(fname, len + 1);         /* actual filename */
    }
    delete_buffer(idxbuf);                      /* release local buffer */
    set_buffer(curbuf);

    /* Read image */
    fname = help_resolve(dir + fname + ".hlp");
    if ((hlpbuf = create_buffer(topic, fname, TRUE)) < 0) {
        error("help: '%s' missing.", fname);
        return -1;
    }
    return hlpbuf;
}


/*  Funtion:        help_file
 *       Load the specified help file.
 *
 *  Parameters:
 *      topic -         File name, with encoded section or lineno.
 *
 *  Example:
 *      "file[*section=<xxx>|line=<xxx>][title=<xxx>]
 */
static int
help_file(string topic)
{
    string filename, title, section;
    string option, value;
    int curbuf, hlpbuf, lineno;
    int i, e;

    /* Split topic */
    if ((i = index(topic, ",")) > 0) {          /* locate file */
        filename = help_resolve(substr(topic, 1, i - 1));
    } else {
        filename = help_resolve(topic);
    }

    title = filename;                           /* default */
    if ((e = rindex(title, ".")) > 0) {         /* locate extension */
        title = substr(title, 1, e - 1);        /* and strip */
    }

    while (i > 0) {
        topic = substr(topic, i + 1);           /* remove previous */

        if ((e = index(topic, "=")) <= 0) {     /* locate = */
            break;
        }

        option = substr(topic, 1, e - 1);
        if ((i = index(topic, "," )) <= 0) {    /* locate next option */
            value = substr(topic, e + 1);
        } else {
            value = substr(topic, e + 1, i - (e + 1));
        }

        if (option == "title") {
            title = value;
        } else if (option == "section") {
            section = value;
        } else if (option == "lineno") {
            lineno = atoi(value);
        }
    }

    /* Load image */
    if ((hlpbuf = create_buffer(title, filename, 1)) < 0) {
        error("help: '%s' missing.", filename);
        return -1;
    }

    /* Position at the section */
    curbuf = inq_buffer();
    set_buffer(hlpbuf);
    top_of_buffer();
    if (section != "") {                        /* seek section */
        re_search(NULL, "<" + quote_regexp(section));
    } else if (lineno > 0) {                    /* abs line number */
        move_abs(lineno, 0);
    }
    set_buffer(curbuf);

    return hlpbuf;
}


/*  Function:       kdb_summary
 *      Generate a keyboard summary.
 *
 */
void
kdb_summary()
{
    extern int users_buf, cur_keyboard;         /* help() globals */
    list   l_bookmarks;
    list   l_buffers;
    list   l_marks;
    list   l_deletes;
    list   l_misc;
    list   l_window;
    list   l_searches;
    list   l_moves;
    list   klist;
    string mac, key;
    int    curbuf, buf;
    int    klen, i;

    message("Generating keyboard summary...");
    klist = key_list(cur_keyboard, NULL, users_buf);
    klen = length_of_list(klist);
    for (i = 0; i < klen; i += 2) {
        mac = klist[i];
        mac = re_translate(NULL, "objects ", "", mac);

        key = klist[i + 1];
        key = re_translate(SF_GLOBAL, "_", " ", key);
        key = compress(key, TRUE);

        if (index(key, "bookmark")) {
            if (key == "drop_bookmark 0") {
                l_bookmarks += "drop_bookmark";
                l_bookmarks += mac;

            } else if (index(key, "drop_bookmark") <= 0) {
                l_bookmarks += key;
                l_bookmarks += mac;
            }

        } else if (index(key, "mark") || index(key, "cut") || index(key, "copy") ||
                        index(key, "paste") || index(key, "block")) {
            switch (key) {
            case "mark 2":
                key = "column mark";
                break;
            case "mark 3":
                key = "line mark";
                break;
            }
            l_marks = help_sumappend(l_marks, mac, key);

        } else if (index(key, "buffer") || index(key, "file")) {
            l_buffers = help_sumappend(l_buffers, mac, key);

        } else if (index(key, "window") || index(key, "edge")) {
            l_window = help_sumappend(l_window, mac, key);

        } else if (index(key, "delete")) {
            l_deletes = help_sumappend(l_deletes, mac, key);

        } else if (index(key, "search") || index(key, "translate") || index(key, "find")) {
            l_searches = help_sumappend(l_searches, mac, key);

        } else if (index(key, "goto") || index(key, "home") || index(key, "end") ||
                        index(key, "up") || index(key, "down") || index(key, "left") || index(key, "right")) {
            l_moves = help_sumappend(l_moves, mac, key);

        } else {
            l_misc = help_sumappend(l_misc, mac, key);
        }
    }

    curbuf = inq_buffer();
    buf = create_buffer("Command-Summary", NULL, TRUE);
    set_buffer(buf);
    attach_syntax(KDB_SUMMARY);

    /* Now insert the lists into the buffer */
    if (length_of_list(l_moves))
        help_sumlist("Movement", l_moves);
    if (length_of_list(l_deletes))
        help_sumlist("Deletions", l_deletes);
    if (length_of_list(l_searches))
        help_sumlist("Search & Translates", l_searches);
    if (length_of_list(l_buffers))
        help_sumlist("Files & Buffers", l_buffers);
    if (length_of_list(l_marks))
        help_sumlist("Markers & Regions", l_marks);
    if (length_of_list(l_window))
        help_sumlist("Windows", l_window);
    if (length_of_list(l_bookmarks))
        help_sumlist("Bookmarks", l_bookmarks);
    if (length_of_list(l_misc))
        help_sumlist("Miscellaneous", l_misc);

    set_buffer(curbuf);
    message("");
    help_window(HELP_STANDARD, buf, 0, 0, 0);
    delete_buffer(buf);
}


/*  Function:       help_sumappend
 *      Append to the keyboard summary list.
 *
 */
static list
help_sumappend(list lst, string key, string mac)
{
    string s;
    int i;

    if ((i = re_search(NULL, "<" + quote_regexp(mac) + ">", lst)) >= 0) {
        s = lst[i + 1];                         /* entry exists */
        s += ", " + key;
        lst[i + 1] = s;

    } else {                                    /* add new entry */
        lst += compress(mac);
        lst += key;
    }
    return lst;
}


/*  Function:       help_sumlist
 *      Insert into the keyboard summary list.
 *
 */
static void
help_sumlist(string title, list l)
{
    string s;
    int i;

    insert("\t\t[[ " + title + " ]]\n\n");
    drop_anchor(MK_LINE);
    for (i = 0; i < length_of_list(l); i += 2) {
        s = l[i];
        if (substr(s, 1, 8) == "objects ") {
            s = substr(s, 9);
        }
        s = re_translate(SF_UNIX, "\\$[0-9a-zA-Z]+::", "", s);
        insertf("%-21s %s\n", s, l[i + 1]);
    }
    up();
    sort_buffer();
    raise_anchor();
    down();
    insert("\n");
}


/*  Function:       kdb_mapping
 *      Generate a keyboard summary.
 *
 */
void
kdb_mapping()
{
    extern int top_line;                        /* sized_window() globals */
    extern int users_buf, cur_keyboard;         /* help() globals */
    int    curbuf, buf, win;
    list   lst;
    int    i, len;

    message("Generating key list...");
    lst = key_list(cur_keyboard, NULL, users_buf);
    len = length_of_list(lst);

    curbuf = inq_buffer();
    buf = create_buffer("Key Mapping", NULL, TRUE);
    set_buffer(buf);
    for (i = 0; i < len; i += 2) {
        insert(lst[i]);
        move_abs(0, 24);
        insert(lst[i + 1]);
        insert("\n");
    }
    delete_line();
    top_of_buffer();
    re_translate(SF_GLOBAL|SF_UNIX, "\\$[0-9a-zA-Z]+::", "");
    sort_buffer();
    attach_syntax(KDB_SUMMARY);

    top_line++;
    help_level++;
    message("");
    set_buffer(curbuf);                         /* restore buffer */
    win = sized_window(inq_lines(buf), inq_line_length(buf),
                "<F10> exit, <Back/Esc> previous." );
    select_buffer(buf, win, SEL_NORMAL,
                "help_mapkeys", NULL, NULL);
    if (help_level > 0) {
        help_level--;
    }
    top_line--;

    delete_buffer(buf);                         /* release local buffer */
}


/*  Function:     help_mapkeys
 *      Additional select buffer keys, for keyboard summary window
 *
 */
void
help_mapkeys()
{
    assign_to_key("<F10>",         "::help_exit");
    assign_to_key("<Backspace>",   "sel_esc");
    assign_to_key("<Space>",       "sel_esc");
    assign_to_key("<Enter>",       "::mapcs");
    assign_to_key("<Left>",        "sel_esc");
    assign_to_key("<Right>",       "::mapcs");
}


/*  Function:       mapcs
 *      Process a Alt-H press key, being context senitive help key stroke within the
 *      keyboard summary window.
 *
 */
static void
mapcs()
{
    extern int top_line;                        /* sized_window() globals */
    string cmd;
    int i;

    cmd = trim(read());                         /* read macro (right column) */
    cmd = ltrim(substr(cmd, index(cmd, ">")+1));
    if ((i = index(cmd, " ")) > 0)              /* stripe arguments */
        cmd = substr(cmd, 1, i-1);
    if (cmd != "") {                            /* explain primitive */
        top_line += 2;
        help_request(HT_PRIMITIVE, cmd);
        top_line -= 2;
    }
}


/*  Function:       help_window
 *       Display the help window
 *
 *   Parameters:
 *       buf            Help buffer text.
 *       lines          Display window lines.
 *       width          Required width of the display window.
 *       line           Initial line (0 = top of buffer, -1 = current line).
 *       [level]        Help level.
 *       [msg]          Message.
 *
 *  Notes:
 *      Current window and buffer are saved and restored on return.
 */
string
help_window(int type, int buf, int lines, int width, int initial_line, ~int, ~string)
{
#define HR_ELEMS        4                       /* text,line,col,lenw */

    list   refer_list;                          /* refer elements [name,line,col1,col2] */
    list   unique_refer_list;                   /* unique refer names */

    string refer;
    int    help_type = type;
    int    msglevel;

    string msg, pattern, entry, c;
    int    sbuf, sline, scol;                   /* buffer for striping \b's */
    int    curbuf, hlpwin;
    int    ret;

    UNUSED(unique_refer_list);
    UNUSED(help_type);

    curbuf = inq_buffer();
    get_parm(5, help_level);                    /* optional help level */
    get_parm(6, msg);                           /* optional help level */

    /*
     *  Stripe \b's from the buffer, this image is used to determine the true display
     *  width, plus allow 'SEE ALSO' section processing without needed to know the
     *  method of hilighting.
     */
    if ((sbuf = create_buffer("-help-striped-", NULL, 1)) < 0) {
        return "";
    }

    msglevel = set_msg_level(3);                /* quiet search */
    set_buffer(buf);
    if (initial_line < 0) {                     /* current line ? */
        inq_position(initial_line);
    }
    end_of_buffer();
    inq_position(sline, scol);                  /* total lines */

    set_buffer(sbuf);
    transfer(buf, 1, 1, sline, scol);
    top_of_buffer();
    re_translate(SF_GLOBAL, "?\b", "");         /* remove x\b's */
    tabs(8);                                    /* defacto terminal standard */

    if (lines == 0) {                           /* size and position */
        lines = inq_lines();
    }

    if (width <= 0) {
        if (width == 0) {
            width = inq_line_length()+1;        /* true display length */
        } else if ((width *= -1) < width) {
            width = inq_line_length()+1;        /* true display length */
        }
    }

    help_ret = HR_DISABLED;

    /* 'SEE ALSO' and '(see' refers */
    if (type != HELP_STANDARD) {                /* man or grief */

        if (type == HELP_GRIEF) {
            pattern = "{<SEE[ \t]ALSO}|{\\(see}|\\<";
        } else {
            pattern = "<SEE[ \t]ALSO";
        }

        set_buffer(sbuf);
        top_of_buffer();                        /* refer search */

        while (re_search(SF_IGNORE_CASE, pattern)) {
            string accum;                       /* help_rtopic accumulator */

            UNUSED(accum);

            c = substr(read(1), 1, 1);

            if (c == "(") {                     /* "(see xxx[xxx])" */
                /*
                 *  Look for the end of see section.
                 */
                int len, n = 1;                 /* length and bracket nesting */

                next_char(4);                   /* consume leading */
                inq_position(sline, scol);      /* "(see xxx [,xxx])" */
                entry = read();
                while (1) {
                    c = substr(entry, ++len, 1);
                    if (c == "\n" || c == "") {
                        break;
                    } else if (c == "(") {
                        n++;
                    } else if (c == ")" && --n <= 0) {
                        break;
                    }
                }

                if (len > 1) {
                    if (help_rtopic(substr(entry, 1, len - 1))) {
                        help_ret = HR_INACTIVE;
                    }
                }

            } else if (c == "<") {              /* embedded <link> */
                /*
                 *  Look for the end of link section.
                 */
                int len;

                next_char(1);                   /* consume leading '<' */
                inq_position(sline, scol);
                entry = read();
                if (re_search(SF_BRIEF, "<[A-Za-z0-9_ ()]+\\>", entry, NULL, len) > 0) {
                    entry = substr(entry, 1, len - 1);
                    re_translate(SF_BRIEF, "()$", "", entry);
                    if (strlen(entry) > 1) {
                        if (key_to_int("<" + entry + ">") <= 0) {
                            if (help_rpush(entry)) {
                                help_ret = HR_INACTIVE;
                            }
                        }
                    }
                }

            } else {
                /*
                 *  Look at following lines beginning with white space (space or a tabs) and
                 *  contains a comma seperated list of refers.  Blank lines are consumed.
                 *
                 *  The search is terminated with text starting with the first column or EOF.
                 */
                inq_position(sline);            /* "SEE ALSO" line */

                set_buffer(buf);
                move_abs(++sline, 1);           /* next line */

                while (re_search(NULL, "[ \t\n]", read(1)) > 0 && !inq_position()) {
                    raise_anchor();
                    drop_anchor(MK_LINE);       /* stripe \b's and \t's */
                    re_translate(SF_GLOBAL|SF_BLOCK, "?\b", "");
                    raise_anchor();
                    scol = 2;
                    move_abs(sline, 2);
                    if (help_rtopic(trim(read()))) {
                        help_ret = HR_INACTIVE;
                    }
                    move_abs(++sline, 1);       /* next line */
                }

                /*
                 *  Continue searching for other "SEE ALSO" sections
                 */
                set_buffer(sbuf);
                move_abs(sline, 1);
            }
        }
    }
    delete_buffer(sbuf);                        /* destroy local resource */
    set_msg_level(msglevel);

    /*
     *  Display the window
     */
    ++help_level;
    set_buffer(buf);
    set_buffer_flags(NULL, BF_MAN);             /* man style highlighting */
    set_buffer_flags(NULL, BF_ANSI);            /* ansi style highlighting */
    tabs(8);                                    /* defacto terminal standard */

    set_buffer(curbuf);                         /* restore buffer */
    hlpwin = sized_window( lines, width,
                ((msg != "" ? msg : help_level > 1 ?
                    "<F10> exit, <Back>" : "<F10> exit")) +
                (help_ret == HR_INACTIVE ?
                    ", <Tab/Enter/Alt-R> refer" : "") +
                (type == HELP_MAN ? ", <Alt-L>" : "") );

    cursor(0);
    ret = select_buffer(buf, hlpwin, SEL_NORMAL,
                "help_winkeys", NULL, NULL, initial_line);
    cursor(1);
    if (help_level > 0) {
        --help_level;
    }

    if (help_ret == HR_EXIT) {                  /* F10 */
        refer = "--exit--";

    } else if (help_ret == HR_ESC) {            /* <Esc> */
        refer = "--esc--";

    } else if (help_ret == HR_PREV) {           /* <Backspace> */
        refer = "--prev--";

    } else if (help_ret == HR_SPACE) {          /* <Space> */
        refer = "--space--";

    } else if (help_ret == HR_APROPOS) {        /* <Alt-L> */
        refer = "--apropos--";

    } else if (ret > 0 && help_ret > HR_INACTIVE) {
        /*
         *  selection, either <Return> or indirect refer
         */
        refer = refer_list[help_ret];           /* current hilighted word */
        put_parm(3, refer_list[help_ret+1]);    /* current line */
        re_translate(NULL, "?\b", "", refer);   /* remove man hilights */

    } else {
        refer = "";
    }
    return refer;
}


/*  Function:       help_rtopic
 *      Process refer topic specifications.
 *
 *  Description:
 *
 *      o GRIEF - Should not contain white-space, unless in the form
 *          of "aaa and bbb". Trim trailing dots of list item.
 *
 *      o MAN - Should end with a section and if contains whitespace
 *          should only be of the form "xxx (*)".
 *
 *  Returns:
 *      Number of elements pushed.
 */
static int
help_rtopic(string entry)
{
    extern string accum;                        /* see: help_window() */
    extern int type, scol;

    string secondary;
    list entries;
    int ret, extra, i;
    int w, b;                                   /* white and bracket index */

    entries = split(entry, ",");                /* split the line */
    for (i = 0; i < length_of_list(entries); ++i) {
        if ((entry = trim(entries[i])) != "") {
            w = re_search(NULL, "[ \t]", entry);

            extra = 2;                          /* ",[ \t]" */

            /* deal with last entry specifics */
            if (i == length_of_list(entries)-1) {
                if (type == HELP_GRIEF) {       /* aaa and bbb */
                    if (w && substr(entry, w, 5) == " and ") {
                        secondary = rtrim(substr(entry, w + 5), ".");
                        entry = substr(entry, 1, w-1);
                        extra += 3;
                        w *= -1;
                    } else {
                        entry = rtrim(entry, ".");
                    }

                } else if (type == HELP_MAN) {  /* aaaa- */
                    if (substr(entry, strlen(entry), 1) == "-") {
                        accum = rtrim(entry, "-");
                        break;
                    }
                }
            }

            /* apply static rules */
            if ((type == HELP_GRIEF && w <= 0) ||
                    (type == HELP_MAN &&
                        (b = re_search(NULL, "(*)", entry)) > 0 && (w == 0 || w + 1 == b)
                    )) {
                help_rpush(accum + entry);
                accum = "";
                ++ret;
            }

            scol += strlen(entry) + extra;

            /* secondary */
            if (w < 0) {
                help_rpush(secondary);
            }
        }
    }

    return ret;
}


/*  Function:       help_rpush
 *      Push a refer topics into the global refer_list.
 *
 */
static int
help_rpush(string entry)
{
    extern list refer_list, unique_refer_list;
    extern int sline, scol;
    int col = scol;

    if (re_search(SF_BRIEF|SF_CAPTURES, "<\\<{[^>]+}\\>>", entry) > 0) {
        re_result(1, entry);                    /* remove link delimiters "<xxxx>" */
        col += 1;
    }

    refer_list += entry;                        /* non-unique references */
    refer_list += sline;
    refer_list += col;
    refer_list += col + strlen(entry);

    if (-1 == re_search(SF_IGNORE_CASE,         /* unique references */
                    "<" + entry + ">", unique_refer_list)) {
        unique_refer_list += entry;
        return 1;
    }
    return 0;
}


/*  Function;       help_winkeys
 *      Addition keys during help buffer
 *
 */
void
replace_assigned_keys(string ocmd, string ncmd)
{
    string keys, key;
    int e;

    keys = inq_assignment(ocmd, 1);
    while (substr(keys, 1, 1) == "<" && (e = index(keys, ">")) > 0) {
        key = substr(keys, 1, e);
        keys = substr(keys, e + 1);
        if (key != "<-also>") {
            message("%s=%s", key, ncmd);
            assign_to_key(key, ncmd);
        }
    }
}


void
help_winkeys()
{
    extern int help_type;

#if (WORK_IN_PROGRESS)
    if (help_type != HELP_STANDARD) {           /* replacement search */
        replace_assigned_keys("search__fwd", "::help_search_fwd");
        replace_assigned_keys("search__back", "::help_search_back");
    }
#endif
    assign_to_key("^G",             "routines hlp");
    assign_to_key("<Alt-R>",        "::help_refer");
    if (help_type == HELP_MAN) {
        assign_to_key("<Alt-L>",    "::help_list");
    }
    assign_to_key("<Esc>",          "::help_esc");
    assign_to_key("<Space>",        "::help_space");
    assign_to_key("<Backspace>",    "::help_prev");
    assign_to_key("<F10>",          "::help_exit");
    assign_to_key("<Tab>",          "::help_tab 1");
    assign_to_key("<Right>",        "::help_tab 1");
    assign_to_key("<Shift-Tab>",    "::help_tab 0");
    assign_to_key("<Left>",         "::help_tab 0");
    assign_to_key("<Button1-Down>", "::help_link");
}


#if (WORK_IN_PROGRESS)
static string
help_pattern2( string pattern )
{
    string npattern;
    int i, len = strlen(pattern);

    for (i = 1; i <= len; i++) {
        npattern += substr(pattern, i, 1)+"\b"+substr(pattern, i, 1);
    }
    return npattern;
}


static void
help_search_fwd()
{
    extern int searching_fwd, search__flags;
    extern string search__pattern;

    int old_msg_level, match_len, oline, ocol;
    int len1, line1, col1;
    int len2, line2, col2;

    if (get_parm(NULL, search__pattern, "Search help for: ", NULL, search__pattern) <= 0) {
        return;
    }

    searching_fwd = TRUE;
    inq_position(oline, ocol);

    len1 = re_search(search__flags, search__pattern);
    if (len1 >= 1) {
        inq_position(line1, col1);
        inq_position(oline, ocol);
    }

    len2 = re_search(search__flags, help_pattern2(search__pattern));
    if (len2 >= 1)
        inq_position(line2, col2);

    if (len1 >= 1 &&
            (len2 < 1 || line1 < line2 || (line1 == line2 && col1 < col2))) {
        match_len = len1;

    } else if (len2 >= 1) {
        match_len = len2;

    } else {
        int old_msg_level = set_msg_level(0);

        message("Not found");
        set_msg_level(old_msg_level);
        move_abs(oline, ocol);
        match_len = -1;
    }

    search_hilite(match_len);
}

static void
help_search_back()
{
    /*TODO*/
}
#endif  /*WORK_IN_PROGRESS*/


static void
help_refer()
{
    extern int window_offset, top_line;         /* sized_window() globals */
    extern list refer_list, unique_refer_list;  /* refer lists */

    int owindow_offset = window_offset;
    int otop_line = top_line;
    int ret;

    if (0 == length_of_list(refer_list)) {      /* list length */
        beep();
        return;
    }

    inq_window_info(NULL, NULL, window_offset, NULL, NULL, top_line);
    window_offset += 10, top_line += 2;
    ret = select_slim_list("refer", "", unique_refer_list, SEL_CENTER, NULL, NULL, 1);
    window_offset = owindow_offset;
    top_line = otop_line;

    if (ret > 0) {
        if ((ret = re_search(SF_IGNORE_CASE,    /* map unique to first refer element */
                        "<" + unique_refer_list[ret-1] + ">", refer_list)) >= 0) {
            help_ret = ret;
            sel_enter();
        }
    }
}

static void
refer_highlight()
{
    extern list refer_list;

    raise_anchor();
    if (help_ret >= 0) {
        move_abs( refer_list[ help_ret+1 ], refer_list[ help_ret+2 ] );
        re_search( SF_NOT_REGEXP, refer_list[ help_ret ] );
        drop_anchor( MK_COLUMN );
        next_char( strlen(refer_list[ help_ret ])-1 );
    }
}


static void
help_list()
{
    extern int window_offset, top_line;         /* sized_window() globals */
    extern list flist;                          /* see man.cr */
    extern int fidx;                            /* see man.cr */

    int owindow_offset = window_offset;
    int otop_line = top_line;
    int ret;

    if (0 == length_of_list(flist)) {           /* list length */
        beep();
        return;
    }

    inq_window_info( NULL, NULL, window_offset, NULL, NULL, top_line );
    window_offset += 10, top_line += 2;
    ret = select_slim_list("apropos", "", flist, 0, NULL, NULL, 1);
    window_offset = owindow_offset;
    top_line = otop_line;

    if (ret > 0 && fidx != (ret - 1)) {
        fidx = ret - 1;
        help_ret = HR_APROPOS;
        sel_exit();
    }
}


static void
help_item()
{
    if (help_ret >= 0 &&
            inq_marked() != MK_COLUMN) {        /* item current selected */
        help_ret = HR_INACTIVE;                 /* nop */
    }
    sel_exit(0);                                /* return=0 */
}


static void
help_exit()
{
    help_level--;                               /* top level */
    while (help_level > 0) {
        help_level--;
        exit();                                 /* .. sublevels */
    }
    help_ret = HR_EXIT;
    sel_esc();                                  /* return=-1 */
}


static void
help_esc()
{
    help_ret = HR_ESC;
    sel_esc();                                  /* return=-1 */
}


static void
help_prev()
{
    help_ret = HR_PREV;
    sel_esc();                                  /* return=-1 */
}


static void
help_space()
{
    help_ret = HR_SPACE;
    sel_esc();                                  /* return=-1 */
}


static void
help_tab()
{
    extern list refer_list;                     /* refer list */

    int fwd, len;
    int curline, i;

    if (help_ret < HR_INACTIVE)                 /* refers active ? */
        return;

    len = length_of_list(refer_list);           /* list length */
    inq_position(curline);

    get_parm(0, fwd);                           /* direction */
    if (fwd) {
        if (help_ret >= 0 && inq_marked() == MK_COLUMN) {
            if ((help_ret += HR_ELEMS) >= len) {
                help_ret = 0;                   /* wrap */
            }

        } else {                                /* next line, find closest next line */
            help_ret = 0;
            for (i = help_ret; i < len; i += HR_ELEMS)
                if (refer_list[i+1] >= curline) {
                    help_ret = i;               /* cloest line */
                    break;
                }
        }

    } else {
        if (help_ret >= 0 && inq_marked() == MK_COLUMN) {
            if ((help_ret -= HR_ELEMS) < 0) {
                help_ret = len - HR_ELEMS;      /* wrap */
            }

        } else {
            help_ret = len - HR_ELEMS;          /* EOF, find closest prev line */
            for (i = help_ret; i >= 0; i -= HR_ELEMS)
                if (refer_list[i+1] <= curline) {
                    help_ret = i;               /* prev line */
                    break;
                }
        }
    }
    refer_highlight();
}


static void
help_link()
{
    extern list refer_list;                     /* refer list */
    extern int hlpwin;

    int i, cwin, line, col, where;

    get_mouse_pos(NULL, NULL, cwin, line, col, where, NULL, NULL);
    if (cwin == hlpwin) {
        switch (where) {
        case MOBJ_INSIDE:
            help_ret = HR_INACTIVE;
            for (i = 0; i < length_of_list(refer_list); i += HR_ELEMS) {
                if (refer_list[i + 1] == line) {
                    if (col >= refer_list[i + 2] && col <= refer_list[i + 3]) {
                        help_ret = i;
                        sel_enter();
                        break;
                    }
                }
            }
            refer_highlight();
            break;
        case MOBJ_TOP_EDGE:
        case MOBJ_BOTTOM_EDGE:
        case MOBJ_TITLE:
        case MOBJ_CLOSE:
            sel_esc();
            break;
        }
    }
}


/*
 *  Local Variables: ***
 *  mode: cr ***
 *  indent-width: 4 ***
 *  End: ***
 */
