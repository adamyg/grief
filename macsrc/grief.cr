/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: grief.cr,v 1.98 2024/10/28 15:45:11 cvsuser Exp $
 * GRIEF startup macro.
 *
 *
 */

#include "grief.h"
#include "mode.h"

#if defined(__PROTOTYPES__)
static void             grinit_onload(void);
static void             grinit_onexit(void);

string                  _griget_profile(void);
void                    _griset_profile(string arg);
string                  _griget_color_index(void);
void                    _griset_color_index(string arg);
string                  _griget_borders(void);
void                    _griset_borders(string arg);
string                  _griget_syntax(void);
void                    _griset_syntax(string arg);
string                  _griget_package(void);
void                    _griset_package(string arg);
string                  _griget_modeline(void);
void                    _griset_modeline(string arg);
string                  _griget_echo_format(void);
void                    _griset_echo_format(string arg);
string                  _griget_echo_line(void);
void                    _griset_echo_line(string arg);
string                  _griget_mouse(void);
void                    _griset_mouse(string arg);
string                  _griget_menubar(void);
void                    _griset_menubar(string arg);
string                  _griget_colors(void);
void                    _griset_colors(string arg);
void                    _griset_colorschemegui(string arg);
string                  _griget_colorschemegui(void);
void                    _griset_colorscheme256(string arg);
string                  _griget_colorscheme256(void);
void                    _griset_colorscheme88(string arg);
string                  _griget_colorscheme88(void);
void                    _griset_colorscheme16(string arg);
string                  _griget_colorscheme16(void);
void                    _griset_colorscheme(string arg);
string                  _griget_colorscheme(void);

void                    console_detect(void);
#endif  /*__PROTOTYPES__*/

/*
 *  Global search/translate status
 */
string                  search__pattern;
string                  translate__pattern;
string                  translate__replacement;


/*
 *  Global save/restore objects.
 *
 *      For each listed property, the macros named '_griget_OBJECT' and
 *      '_griset_OBJECT' should be defined which understands how to get and set
 *      the attribute value.
 *
 *      Care should be taken when adding additional, avoid autoloading macros
 *      which are not going to be utilised most of the time.
 */
static list             gri_properties = {
    "profile",                  /* User profile */
    "abbrev",                   /* Abbrev list */
    "autoindent",               /* Auto indent setting */
    "autosave",                 /* Autosave times */
    "colorschemegui",           /* Color scheme (truecolor) */
    "colorscheme256",           /* Color scheme (colors >= 256) */
    "colorscheme88",            /* Color scheme (colors >= 88) */
    "colorscheme16",            /* Color scheme (colors >= 16) */
    "colorscheme",              /* Color scheme */
    "colors",                   /* Default colors */
    "color_index",              /* Default new window color index */
    "window_color",             /* Default first window color */
    "syntax",                   /* RE expression syntax (brief/unix ...) */
    "modeline",                 /* modeline parser */
    "echo_format",              /* Echo line area format specification */
    "echo_line",                /* Echo line flags */
    "menubar",                  /* Menubar on/off */
    "borders",                  /* Borders on/off */
    "case_sensitive",           /* Case sensitive searching */
    "autowrap",                 /* Word wrap */
    "justification",            /* Justification */
    "margin",                   /* Margin setting */
    "tabs",                     /* Tab settings for  extensions */
    "hard_tabs",                /* Hard tabs on/off */
    "indents",                  /* Indent settings for extensions */
    "package",                  /* Package configuration */
    "restore",                  /* Save state */
    "backup"                    /* Backup state */
    };

static list             gri_values;             // cached values


/*
 *  Global profile settings.
 *
 *      tabs -          physical tab setting.
 *      hardtabs -      either tabs are hard or soft (spaces).
 *      indents -       indentation.
 */
static string           gri_profile;

static int              gri_packageloaded = 0;  // whether language is loaded?
string                  gri_package = "";

static int              gri_modeline = FALSE;
static int              gri_mouse = FALSE;
static int              gri_menubar = FALSE;
static int              gri_changed = FALSE;

static int              coloriserenv = FALSE;
static string           gri_colorisergui = "";
static string           gri_coloriser256 = "";
static string           gri_coloriser88 = "";
static string           gri_coloriser16 = "";
static string           gri_coloriser = "";
static int              gri_window_color = 0;
static int              gri_color_index = 0;

static string           gri_tabs;
static string           gri_hardtabs;
static string           gri_indents;

static list             tabs_list;
static list             hardtabs_list;
static list             indents_list;


/*
 *  Global resources.
 */
string                  CRISP_DELIM, CRISP_SLASH, CRISP_DIRSEP;

int                     top_keyboard;

list                    kbd_labels;
string                  last_file_edited = "";

int                     popup_level;
int                     sel_warpable;

list                    color_labels;           // global color labels.


void
main(void)
{
    switch (CRISP_OPSYS) {
    case "VMS":
        CRISP_DELIM  = "";
        CRISP_SLASH  = "]";
        CRISP_DIRSEP = ";";
        break;

    case "OS/2":
    case "Win32":
    case "DOS":
        CRISP_DELIM  = "/";
        CRISP_SLASH  = "\\";
        CRISP_DIRSEP = ";";
        break;

    case "MACOSX":
    case "UNIX":
        CRISP_DELIM  = "/";
        CRISP_SLASH  = "/";
        CRISP_DIRSEP = ":";
        break;
    }
    _extension_init();
}


/*
 *  grief ---
 *      The following macro is executed on startup and is responsible for setting up the
 *      initial environment.
 *
 *      In addition, it sets up several global variables which are utilised by the other
 *      macros to allow portability between operating systems.
 */
void
grief(void)
{
    string envvar;

    /*
     *  Inform GRIEF where to find macros, done prior 2 keyboard mapping to allow
     *  valid detection of 'unknown references'.
     *
     *  TODO - automate autoload() generation thru grunch support.
     *
     *      #pragma autoload <function>
     */
    autoload("abbrev",
        "_griset_abbrev",
        "abbrev_load");
    autoload("auditlog",
        "_griset_auditlog");
    autoload("autosave",
        "autosave_disable",
        "_griset_autosave");
    autoload("backup",
        "_griset_backup", "_griget_backup",
        "recover");
    autoload("box",
        "dobox");
    autoload("brace",
        "find_matching_brace");
    autoload("brief",
        "del",
        "dos",
        "inq_brief_level");
    autoload("dialog",
        "_dialog_menu");
    autoload("change",
        "change", "changes", "changelog",
        "todo");
    autoload("comment",
        "comment_block",
        "uncomment_block");
    autoload("colors",
        "coloriser",
        "inq_coloriser",
        "colorscheme");
    autoload("compile",
        "_griset_load",
        "load",
        "errors",
        "make", "gmake", "dmake", "xmake",
        "lint",
        "default_next_error",
        "default_previous_error");
    autoload("copyr",
        "ccopyr",
        "fn");
    autoload("core",
        "_fatal_error");
    autoload("debug",
        "__dbg_trace__",
        "trace", "evaluate", "brk",
        "vars", "bvars", "mvars");
    autoload("extra",
        "edit_again",
        "make_writeable");
    autoload("bufflags",
        "toggle_buffer_flags",
        "toggle_buffer_type");
    autoload("feature",
        "select_feature",
        "feature_help");
    autoload("ff",
        "ff",
        "dir",
        "tree",
        "treecd",
        "ts",
        "find",
        "bs");
    autoload("objects",
        "objects",
        "shift_left", "shift_right",
        "lshift", "shiftl",
        "rshift", "shiftr");
    autoload("routines",
        "routines",
        "c_routines",
        "h_routines",
        "m_routines",
        "mm_routines",
        "routines_search");
    autoload("help",
        "help",
        "explain",
        "cshelp",
        "kdb_summary",
        "kdb_mapping",
        "help_about",
        "help_resolve",
        "help_display",
        "help_window");
    autoload("history",
        "_inq_history",
        "_prompt_begin",
        "_prompt_end",
        "prompt_help");
    autoload("keys",
        "_back_tab",
        "_open_line");
    autoload("key",
        "key",
        "key_code",
        "key_test",
        "key_trace",
        "key_map",
        "key_val",
        "key_learn_menu",
        "key_learn",
        "key_termmapping");
    autoload("linenumbers",
        "_griset_linenumbers");
    autoload("man",
        "apropos");
    autoload("menu",
        "menu",
        "menuon",
        "menuoff",
        "menubar");
    autoload("misc",
        "search_path",
        "add_to_path",
        "delete_curr_buffer",
        "goto__line",
        "redo",
        "_indent",
        "ansi",
        "display_file_name",
        "end",
        "home",
        "cd",
        "sub",
        "gsub",
        "noundo",
        "insert_tab",
        "insert_backtab",
        "force_input",
        "previous_tab",
        "quote",
        "repeat",
        "join_line",
        "edit_next_buffer",
        "edit_previous_buffer",
        "edit__file",
        "previous_edited_buffer",
        "previous_alpha_buffer",
        "set_fs",
        "delete_character",
        "delete_blank_lines",
        "hex", "dec");
    autoload("popup",
        "popup_mouse");
    autoload("print",
        "_griget_print", "_griset_print");
    autoload("options",
        "options",
        "echo_line_options");
    autoload("region",
        "copy",
        "cut",
        "paste",
        "block_delete",
        "block_lower_case",
        "block_upper_case",
        "sum");
    autoload("remember",
        "remember",
        "keylib",
        "compl_keylib");
    autoload("restore",
        "_griget_restore", "_griset_restore",
        "_startup_complete",
        "save_state");
    autoload("scrap",
        "copy_named_scrap",
        "cut_named_scrap",
        "paste_named_scrap");
    autoload("startup",
        "load_indent",
        "load_compile");
    autoload("search",
        "isearch",
        "toggle_re", "toggle_re_case", "translate_again",
        "_griget_case_sensitive", "_griset_case_sensitive",
        "translate__back", "translate__fwd",
        "search__fwd","search__back",
        "search_next", "search_prev",
        "search_options", "search_hilite");
    autoload("search_replace",
        "search_replace");
    autoload("set",
        "setenv",
        "inq_env");
    autoload("select",
        "top_line", "window_offset",
        "field_list",
        "sized_window",
        "select_list",
        "select_slim_list",
        "select_file",
        "select_files",
        "select_buffer",
        "buffer_list");
    autoload("shell",
        "sh",
        "tcsh",
        "ksh",
        "bash",
#if defined(OS2)
        "os2"
#endif
#if defined(MSDOS)
        "cmd",
#endif
        "which",
        "create_shell");
    autoload("tags",
        "_griset_tags",
        "mtags",
        "tag",
        "tags",
        "tag_function");
    autoload("telnet",
        "rlogin",
        "ftp",
        "ncftp");
    autoload("text",
        "grep",
        "fgrep",
        "egrep",
        "wc");
    autoload("spell",
        "spell",
        "spelltest");
    autoload("uspell",
        "uspell");
    autoload("command",
        "fixslash",
        "tmpdir",
        "inq_shell",
        "perform_command");
    autoload("window",
        "goto_left_edge",
        "goto_right_edge",
        "set_top_of_window",
        "set_bottom_of_window",
        "set_center_of_window");
    autoload("wp",
        "format_paragraph",
        "format_block",
        "format_list",
        "center",
        "autowrap",
        "autoindent",
        "wp_options",
        "margin",
        "_griset_autowrap",
        "_griget_autowrap",
        "_griset_autoindent",
        "_griget_autoindent",
        "_griget_justification",
        "_griset_justification",
        "_griset_margin",
        "_griget_margin",
        "h_format_block",
        "cr_format_block",
        "y_format_block",
        "c_format_block",
        "default_format_block");
    autoload("view",
        "literal");
    autoload("utils",
        "rw",
        "write_buffer_as", "write_buffers",
        "wildcard_erase" );
    autoload("rcs",
        "co");
    autoload("tabs",
        "show_tabs",
        "entab", "detab",
        "detab_buffer", "entab_buffer",
        "detab_region",
        "detab_str");
    autoload("nc",
        "nc", "mc");

    /* demos, games and regression tests */
    autoload("demos/demo",
        "demo");
    autoload("demos/hanoi",
        "hanoi");
    autoload("demos/perf",
        "perf");
    autoload("demos/regress",
        "regress",
        "regress_renumber");
    autoload("demos/dialogtest",
        "dialogtest");
    autoload("demos/sieve",
        "sieve", "calc_primes");
    autoload("demos/tetris",
        "tetris");
    autoload("demos/invaders",
        "invaders");
    autoload("demos/sweeper",
        "sweeper");
    autoload("demos/snake",
        "snake");

    /* language support */
    autoload("mode",
        "cmode",
        "_mode_pkg_set", "_mode_pkg_get",
        "_mode_attr_set", "_mode_attr_get");
    load_macro("modes/modes");                  /* mode helper */
    autoload("modeline",
        "modeline",
        "mode");
    autoload("hier",
        "hier",
        "chier",
        "cpphier",
        "hier_show");
    autoload("funchead",
        "funchead",
        "fh");

    top_keyboard = inq_keyboard();

    /* define standard key bindings */
    assign_to_key("^A",                         "extra");
    assign_to_key("^B",                         "set_bottom_of_window");
    assign_to_key("^C",                         "set_center_of_window");
    assign_to_key("^E",                         "nlang");
    assign_to_key("^F",                         "format_paragraph");
    assign_to_key("^G",                         "routines");
    assign_to_key("^H",                         "backspace");
    assign_to_key("^I",                         "insert_tab");
    assign_to_key("^K",                         "objects delete_word_left");
    assign_to_key("^L",                         "objects delete_word_right");
    assign_to_key("^N",                         "edit_next_buffer");
    assign_to_key("^O",                         "options");
    assign_to_key("^P",                         "edit_previous_buffer");
    assign_to_key("^R",                         "repeat");
    assign_to_key("^T",                         "set_top_of_window");
    assign_to_key("^U",                         "redo");
    assign_to_key("^V",                         "paste 1");             /*system paste*/
    assign_to_key("^W",                         "set_backup");
    assign_to_key("^Z",                         "zoom");
    assign_to_key("^]",                         "tag_function");
    assign_to_key("^_",                         "edit_previous_buffer");
    assign_to_key("<Ctrl-^>",                   "find_matching_brace");
    assign_to_key("<Ctrl-6>",                   "find_matching_brace"); /*WIN32*/
    assign_to_key("<Ctrl-Return>",              "fullscreen");          /*WIN32*/
    assign_to_key("#127",                       "delete_character");
    assign_to_key("<Alt-B>",                    "buffer_list 1");
    assign_to_key("<Alt-E>",                    "edit__file");
    assign_to_key("<Alt-G>",                    "goto__line");
    assign_to_key("<Alt-H>",                    "help");
    assign_to_key("<Alt-N>",                    "edit_next_buffer");
    assign_to_key("<Alt-P>",                    "previous_alpha_buffer");
    assign_to_key("<Alt-Q>",                    "key");
    assign_to_key("<Alt-S>",                    "search__fwd");
    assign_to_key("<Alt-T>",                    "translate__fwd");
    assign_to_key("<Alt-Y>",                    "search__back");
    assign_to_key("<F5>",                       "search__fwd");
    assign_to_key("<F6>",                       "translate__fwd");
    assign_to_key("<F12>",                      "menubar");
    assign_to_key("<Alt-F5>",                   "search__back");
    assign_to_key("<Alt-F6>",                   "translate__back");
    assign_to_key("<Alt-F7>",                   "keylib");
    assign_to_key("<Alt-F10>",                  "load");
    assign_to_key("<Alt-End>",                  "goto_right_edge");
    assign_to_key("<Alt-Home>",                 "goto_left_edge");
    assign_to_key("<Alt-Minus>",                "previous_alpha_buffer");
    assign_to_key("<Alt-:>",                    "execute_macro");       /*alt F10*/
    assign_to_key("<Ctrl-F5>",                  "toggle_re_case");
    assign_to_key("<Ctrl-F6>",                  "toggle_re");
    assign_to_key("<Ctrl-F10>",                 "load");
    assign_to_key("<Ctrl-Left-Arrow>",          "objects word_left");
    assign_to_key("<Ctrl-Right-Arrow>",         "objects word_right");
    assign_to_key("<Ctrl-Minus>",               "delete_curr_buffer");
    assign_to_key("^_",                         "delete_curr_buffer");
    assign_to_key("<Del>",                      "delete_character");
    assign_to_key("<End>",                      "end");
    assign_to_key("<Home>",                     "home");
    assign_to_key("<Keypad-5>",                 "search_next");
    assign_to_key("<Shift-F5>",                 "search_next");
    assign_to_key("<Shift-F6>",                 "translate_again");
    assign_to_key("<Shift-F10>",                "view_screen");
    assign_to_key("<Shift-Tab>",                "insert_backtab");
    assign_to_key("<PrtSc>",                    "print");
    assign_to_key("<Keypad-Scroll>",            "scroll");
    assign_to_key("<Alt-0>",                    "drop_bookmark 0");
    assign_to_key("<Alt-1>",                    "drop_bookmark 1");
    assign_to_key("<Alt-2>",                    "drop_bookmark 2");
    assign_to_key("<Alt-3>",                    "drop_bookmark 3");
    assign_to_key("<Alt-4>",                    "drop_bookmark 4");
    assign_to_key("<Alt-5>",                    "drop_bookmark 5");
    assign_to_key("<Alt-6>",                    "drop_bookmark 6");
    assign_to_key("<Alt-7>",                    "drop_bookmark 7");
    assign_to_key("<Alt-8>",                    "drop_bookmark 8");
    assign_to_key("<Alt-9>",                    "drop_bookmark 9");
    assign_to_key("<Alt-Left>",                 "page_up");             /* scroll alt */
    assign_to_key("<Alt-Right>",                "page_down");
    assign_to_key("<Ctrl-F1>",                  "borders");

    /* Consume stray ESC and Flow-control keys */
    assign_to_key("<Esc>",                      "nothing");
    assign_to_key("<Ctrl-Q>",                   "nothing");             /* 1/4/2020 */
    assign_to_key("<Ctrl-S>",                   "nothing");

    /* Append the data to the scrap */
    assign_to_key("<Shift-Keypad-Plus>",        "copy 1");
    assign_to_key("<Shift-Keypad-Minus>",       "cut 1");

    /* Named scrap buffers */
    assign_to_key("<Ctrl-Keypad-Plus>",         "copy_named_scrap");
    assign_to_key("<Ctrl-Keypad-Minus>",        "cut_named_scrap");
    assign_to_key("<Ctrl-Ins>",                 "paste_named_scrap");

    /* Help system */
    register_macro(REG_ALT_H, "prompt_help");

    /*
     *  Determine the interface type, and initialise.
     */
    if (display_mode() & DC_WINDOW) {
        /*
         *  Windowed environment
         */
        int maj, min, edit, rel;
        string buf, buf1;

        version(maj, min, edit, rel);
        sprintf(buf, "%s v%d.%d.%d.%d", APPNAME, maj, min, edit, rel);
        sprintf(buf1, "v%d.%d%c", maj, min, edit);
        set_wm_name(buf, buf1);
        set_term_feature(TF_COLOR, TRUE);

    } else {
        /*
         *  Console
         */
        load_macro("tty/console");
        console_detect();
    }

    /* Display,mouse */
    if (display_mode() & (DC_WINDOW|DC_MOUSE)) {
        load_macro("mouse", 0);
    }
    if (inq_macro("mouse_enable") > 0) {
        mouse_enable();
    }
    display_windows(1);

    /* Options */
    grinit_onload();
    refresh();

    /* Localised keyboard description */
    envvar = lower(getenv("BKBD"));
    if (envvar != "") {
        load_macro("kbd/" + envvar);
    }
}


/*
 *  altfunckeys ---
 *      Remap Alt-1..Alt-0 to the F1-F10 for use on systems without function keys using
 *      '-maltfunckeys' during startup.
 */
void
altfunckeys(void)
{
    assign_to_key("<Alt-1>", inq_assignment("<F1>"));
    assign_to_key("<Alt-2>", inq_assignment("<F2>"));
    assign_to_key("<Alt-3>", inq_assignment("<F3>"));
    assign_to_key("<Alt-4>", inq_assignment("<F4>"));
    assign_to_key("<Alt-5>", inq_assignment("<F5>"));
    assign_to_key("<Alt-6>", inq_assignment("<F6>"));
    assign_to_key("<Alt-7>", inq_assignment("<F7>"));
    assign_to_key("<Alt-8>", inq_assignment("<F8>"));
    assign_to_key("<Alt-9>", inq_assignment("<F9>"));
    assign_to_key("<Alt-0>", inq_assignment("<F10>"));
}


/*
 *  _chg_properties ---
 *      Change properties signal.
 */
void
_chg_properties(void)
{
    gri_changed = TRUE;
}


void
shell_pop(string command)
{
    int curwin = inq_window();
    int curbuf = inq_buffer();
    int buf = create_buffer("Shell Pop-Up", NULL, 1);
    int line, col;

    create_window(55, 8, 77, 2);                /* XXX - verify display size */
    attach_buffer(buf);
    set_buffer_flags(NULL, BF_MAN);             /* man style highlighting */
    set_buffer_flags(NULL, BF_ANSI);            /* ansi style highlighting */
    connect();
    insert(command + "\n");
    inq_position(line, col);
    set_process_position(line, col);
    insert_process(command + "\n");
    refresh();

    wait();                                     /* wait for process to exit */

    delete_buffer(buf);
    delete_window();
    set_buffer(curbuf);
    set_window(curwin);
}


/*
 *  _extension_init ---
 *      Load system defaults
 */
void
_extension_init(void)
{
    /* load default 'grinit' settings, most are defacto standards */
    gri_tabs =
        "default=9";                            /* 9, 17 etc */

    gri_indents =
        ".m=3 .cr=3 " +                         /* BRIEF/GRIEF/CRiSP */
        ".c=4 .cc=4 .cpp=4 .h=4 .hpp=4 " +      /* c and C++ */
        ".pl=4 .pm=4 " +                        /* perl */
        ".sh=4 .csh=4 .ksh=4 .bash=4 " +        /* shell */
        "default=0";                            /* default = none, same as tabs */

    /* bust into lists */
    tabs_list = split(gri_tabs, " =.", 1);
    indents_list = split(gri_indents, " =.", 1);
}


/*
 *  _extension ---
 *      Macro called when we edit a new file.
 *
 *      Used it setup the tabs, hard tabs and indent settings.
 */
void
_extension(void)
{
    string ext, quotedext;
    int i;

    inq_names(NULL, ext);
    quotedext = "^" + quote_regexp(ext) + "$";

    /*
     *  tab expansion, extension when default.
     */
    if ((i = re_search(NULL, quotedext, tabs_list)) < 0) {
        i = re_search(NULL, "default", tabs_list);
    }
    if (i >= 0) {
        tabs(tabs_list[i + 1]);
    } else {
        tabs(9);                                /* defacto standard */
    }

    /*
     *  Set hardtab expansion,
     *      we only need to change if an override is found,
     *      otherwise the global setting (system default) is used.
     */
    i = re_search(NULL, quotedext, hardtabs_list);
    if (i >= 0) {
        use_tab_char(hardtabs_list[i+1]);       /* local buffer only */
    }

    /*
     *  indenting
     */
    i = re_search(NULL, quotedext, indents_list);
    if (i < 0) {
        i = re_search(NULL, "default", indents_list);
    }
    if (i >= 0) {
        set_indent(indents_list[i + 1]);
    } else {
        set_indent(0);                          /* follow tabs */
    }

    /*
     *  modeline/package support
     */
    if (inq_macro("_mode_extension", 1) > 0) {
        _mode_extension(ext);
    }
}


/*
 *  inq_grinit ---
 *      Determine the 'grinit' path.
 */
string
inq_grinit(void)
{
    string profiledir,
        stdgrinit;

    profiledir = getenv("GRPROFILE");           // user override, use unconditionally.
    if (profiledir) {
        return file_canon(profiledir + "/" + GRINIT_FILE);
    }

    profiledir = inq_profile();                 // system specific.
    if (profiledir) {
        stdgrinit = profiledir + "/" + GRINIT_FILE;

        if (exist(stdgrinit)) {
            return stdgrinit;
        }
    }

    profiledir = inq_home();                    // users home directory.
    if (profiledir) {
        string altgrinit = profiledir + "/" + GRINIT_FILE;

        if (exist(altgrinit)) {
            return altgrinit;
        }
        if (stdgrinit) return stdgrinit;
        return altgrinit;
    }

    return "~/" + GRINIT_FILE;                  // default.
}



/*
 *  grinit_query ---
 *      Query a current grinit value.
 */
string
grinit_query(string section, string key)
{
    const string inifile = inq_grinit();
    string value;
    int ifd;

    if ((ifd = iniopen(inifile, IFILE_STANDARD|IFILE_COLON)) >= 0) {
        value = iniquery(ifd, section, key);
        iniclose(ifd);
    }
    return value;
}


/*
 *  grinit_update ---
 *      Update a current grinit value.
 */
int
grinit_update(string section, string key, string value)
{
    const string inifile = inq_grinit();
    int ifd;

    if (inq_buffer(inifile) <= 0) {             // local image, ignore updates
        if ((ifd = iniopen(inifile,
                IFILE_STANDARD|IFILE_COLON|IFILE_COMMENTS)) >= 0) {
            inipush(ifd, section, key, value);
            iniexport(ifd);
            iniclose(ifd);
            return 0;
        }
    }
    return -1;
}


/*
 *  grinit_onload ---
 *      Executed on startup to read the properties file and restore the saved attributes.
 */
void
grinit_onload(void)
{
    //  guard and register matching completion
    //
    if (! first_time()) {
         return;
    }

    register_macro(REG_EXIT, "grinit_onexit");

    //  parse 'inifile'
    //
    const string sect = "GRIEF",
        inifile = inq_grinit();
    string grscheme;
    int ifd;

    grscheme = getenv("GRCOLORSCHEME");         // env/cmdline override, use unconditionally.
    if (grscheme) {
        if (colorscheme(grscheme) == TRUE) {
            coloriserenv = TRUE;
        }
    }

    if (! exist(inifile)) {                     // apply defaults
        string pkg =
            getenv("BPACKAGES");                // user BPACKAGES specification, import

        if ("" == pkg) {                        // .. otherwise default
            pkg = ".c.cc.CC.cpp.h.H.hpp-c:hilite,t;.default:hilite,template,regular";
        }
        _griset_package(pkg);
        _griset_colors("");

        message("%s does not exist -- using defaults", inifile);
        gri_changed = TRUE;
        return;
    }

    if ((ifd = iniopen(inifile, IFILE_STANDARD|IFILE_COLON)) >= 0) {
        string key, value, fn;
        int p, ret;
                                                // foreach(property)
        for (ret = inifirst(ifd, NULL, key, value, sect); 1 == ret;
                ret = ininext(ifd, NULL, key, value)) {

            // cache property
            if ((p = re_search(NULL, "<" + key + ">", gri_properties)) <= -1) {
                p = length_of_list(gri_properties);
                gri_properties += key;          // new property.
            }

            gri_values[p] =
                (p < length_of_list(gri_values) ? gri_values[p] + "\n" : "") + value;

            // apply property
            //  TODO - restricted mode, only permit gri_properties.
            if (inq_macro("_griset_" + key) > 0) {
                fn = "_griset_" + key;

            } else if (inq_macro("set_" + key) > 0) {
                fn = "set_" + key;              // FIXME - security hole, remove?

            } else {
                error("unknown configuration key '%s', ignored.", key);
                continue;
            }

            execute_macro(format("%s \"%s\"", fn, value));
        }

        iniclose(ifd);
    }
}


/*
 *  grinit_onexit ---
 *      Executed on exit to save the properties which are exported by various macros.
 */
void
grinit_onexit(void)
{
    //  Export configuration
    //
    //      rebuild 'grinit'
    //
    const string sect = "GRIEF",
        inifile = inq_grinit();
    int ifd;

    if (inq_buffer(inifile) > 0) {
        return;                                 // local image, ignore updates
    }

    if ((ifd = iniopen(inifile,
            IFILE_STANDARD|IFILE_COLON|IFILE_COMMENTS)) >= 0) {
        int p;


        //  header
        //
        if (! exist(inifile)) {                 // initial image
            inipush(ifd, NULL, NULL, NULL, "");
            inipush(ifd, NULL, NULL, NULL, " GRIEF user configuration.");
            inipush(ifd, NULL, NULL, NULL, "");
        }
        inipush(ifd, NULL, "version", "2");     // interface version.

        //  properties
        //
        iniremove(ifd, sect, NULL, TRUE);       // clear section, retain comments.

                                                // foreach(property)
        for (p = 0; p < length_of_list(gri_properties); ++p) {
            const string prop = gri_properties[p];

            if (inq_macro("_griget_" + prop) > 0) {
                //
                //  property interface -- string
                //
                const declare values = execute_macro("_griget_" + prop);

                if (is_list(values)) {          // element list.
                    int v;

                    for (v = 0; v < length_of_list(values); ++v) {
                        inipush(ifd, sect, prop, values[v], NULL, FALSE);
                    }

                } else {                        // element.
                    if (is_string(values)) {
                        if (strlen(values)) {
                            inipush(ifd, sect, prop, trim(values));
                        }

                    } else if (is_integer(values)) {
                        inipush(ifd, sect, prop, format("%d", values));
                    }
                }

            } else if (inq_macro("get_" + prop) > 0) {
                //
                //  old property interface -- string
                //
                string value = execute_macro("get_" + prop);

                inipush(ifd, sect, prop, value);

            } else if (p < length_of_list(gri_values)) {
                //
                //  no property function --- echo previous value
                //
                declare value = gri_values[p];

                if (is_string(value)) {
                    const list values = split(value, "\n");
                    int v;

                    for (v = 0; v < length_of_list(values); ++v) {
                        inipush(ifd, sect, prop, values[v], NULL, FALSE);
                    }
                }
            }
        }

        //  export result
        //
        remove(inifile + ".sav");
        rename(inifile, inifile + ".sav");
        iniexport(ifd);
        iniclose(ifd);
    }
}


/*
 *  Macro called by properties macro to get the current tab
 *  settings so we can save it
 */
string
_griget_tabs(void)
{
    return gri_tabs;                            /* return current 'grinit' setting */
}


string
_griget_hard_tabs(void)
{
    return gri_hardtabs +                       /* grinit setting, plus global setting */
            "default=" + (use_tab_char(-1, 1) ? "yes" : "no");
}


string
_griget_indents(void)
{
    return gri_indents;
}


void
_griset_tabs(string arg)
{
    int i;

    gri_tabs = "";                              /* clear grinit value */

    tabs_list = split(arg, " =.", 1);           /* import parameters */
    for (i = 0; i < length_of_list(tabs_list); i += 2) {
        if (tabs_list[i] != "default") {        /* export grinit */
            gri_tabs += ".";
        }
        gri_tabs += tabs_list[i] + "=" + tabs_list[i+1] + " ";
    }
}


void
_griset_hard_tabs(string arg)
{
    int i;

    gri_hardtabs = "";                          /* clear grinit value */
    hardtabs_list = split(arg, " =.", 1);       /* import parameters */
    for (i = 0; i < length_of_list(hardtabs_list); i += 2) {
        if (hardtabs_list[i] == "default") {    /* set global setting */
            use_tab_char(hardtabs_list[i+1], 1);
        } else {                                /* export grinit */
            gri_hardtabs += "." + hardtabs_list[i] + "=" + hardtabs_list[i+1] + " ";
        }
    }
}


void
_griset_indents(string arg)
{
    int i;

    gri_indents = "";                           /* clear grinit value */
    indents_list = split(arg, " =.", 1);        /* import parameters */
    for (i = 0; i < length_of_list(indents_list); i += 2) {
        if (indents_list[i] != "default") {     /* export grinit */
            gri_indents += ".";
        }
        gri_indents += indents_list[i] + "=" + indents_list[i+1] + " ";
    }
}


string
_griget_profile(void)
{
    return gri_profile;
}


void
_griset_profile(string arg)
{
    const list macros = split(arg, ",");
    int i;

    for (i = 0; i < length_of_list(macros); ++i) {
        string func = macros[i];

        if (strlen(gri_profile)) {
            gri_profile += ",";
        }
        execute_macro(func);
    }
    gri_profile = arg;
}


string
_griget_color_index(void)
{
    return gri_color_index;
}


void
_griset_color_index(string arg)
{
    gri_color_index = atoi(arg);
    color_index(gri_color_index);
}


string
_griget_borders(void)
{
    return (inq_borders() ? "yes" : "no");
}


void
_griset_borders(string arg)
{
    if (upper(substr(trim(arg), 1, 1)) == "Y") {
        borders(1);
    } else {
        borders(0);
    }
}


string
_griget_syntax(void)
{
    return (re_syntax() ? "unix" : "grief");
}


void
_griset_syntax(string arg)
{
    if (arg == "unix") {
        re_syntax(1);
    } else {
        re_syntax(0);                           /* brief/crisp/grief */
    }
}


void
load_package(void)
{
    if (! gri_packageloaded++) {
        load_macro("language");
    }
}


string
_griget_package(void)
{
    return gri_package;                         /* return "global" setting */
}


void
_griset_package(string arg)
{
    gri_package = trim(arg);
    if (strlen(gri_package)) {                  /* only load if required ? */
        load_package();                         /* package support (startup.cr) */
    }
}


string
_griget_modeline(void)
{
    return (gri_modeline ? "yes" : "no");       /* return "global" setting */
}


void
_griset_modeline(string arg)
{
    if (upper(substr(trim(arg), 1, 1)) == "Y") {
        load_package();                         /* modeline support (startup.cr) */
        load_macro("modeline", 0);              /* note, order as events are FIFO */
        gri_modeline = TRUE;
    } else {
        gri_modeline = FALSE;
    }
}


string
_griget_echo_format(void)
{
    return inq_echo_format();
}


void
_griset_echo_format(string arg)
{
    set_echo_format(arg);
}


string
_griget_echo_line(void)
{
    return format("0x%x", inq_echo_line());
}


void
_griset_echo_line(string arg)
{
    if (strlen(arg)) {
        int flags = strtol(arg);
        if (flags >= 0) {
            echo_line(flags);
        }
    }
}


string
_griget_mouse(void)
{
    return (gri_mouse ? "yes" : "no");          /* return "global" setting */
}


void
_griset_mouse(string arg)
{
    if (upper(substr(trim(arg), 1, 1)) == "Y") {
        load_macro("mouse", 0);
        mouse_enable();
        gri_mouse = 1;
    }
}


string
_griget_menubar(void)
{
    return (gri_menubar ? "yes" : "no");        /* return "global" setting */
}


void
_griset_menubar(string arg)
{
    if (upper(substr(trim(arg), 1, 1)) == "Y") {
        gri_menubar = 1;
        menuon();
    }
}


string
_griget_colors(void)
{
    return inq_color();                         /* return "base" colors */
}


void
_griset_colors(string arg)
{
    if (inq_macro("inq_coloriser", 2) > 0) {
        string scheme = inq_coloriser();
        if (scheme) {                           /* colorscheme active, ignore */
            return;
        }
    }

    if (strlen(arg)) {
        /*
         *  base colors, using old-style interface.
         */
        execute_macro("color " + arg);          /* set color list */

        /*
         *  highlight defaults
         *      note, omitted entries are included to detail their default assignment.
         *      The remaining set are the minimal pairs required.
         */
        set_color_pair("prompt",                "light-magenta");   /* def: message */
        set_color_pair("echo_line",             "yellow");          /* def: normal */
        set_color_pair("nonbuffer",             "brown");           /* def: message */

        set_color_pair("dialog_focus",          "magenta");
        set_color_pair("dialog_hilite",         "light-white,dark-blue");
        set_color_pair("dialog_but_normal",     "grey");
        set_color_pair("dialog_but_focus",      "light-white");
        set_color_pair("dialog_but_key_normal", "dark-red:underline");
        set_color_pair("dialog_but_key_focus",  "red:underline");

        set_color_pair("string",                "light-green");
        set_color_pair("operator",              "light-cyan");
        set_color_pair("number",                "light-green");
        set_color_pair("float",                 "light-green");     /* def: number */
        set_color_pair("comment",               "light-magenta");
        set_color_pair("delimiter",             "light-cyan");
        set_color_pair("whitespace",            "white", "red");
        set_color_pair("preprocessor",          "cyan");
        set_color_pair("preprocessor_define",   "light-cyan");
        set_color_pair("preprocessor_include",  "light-cyan");
        set_color_pair("preprocessor_keyword",  "light-red");
        set_color_pair("keywords",              "yellow");
        set_color_pair("keyword_function",      "cyan");
        set_color_pair("keyword_extension",     "red");

        /*
         *  user profile override
         */
        if (inq_macro("_highlight_colors") > 0) {
            extern void _highlight_colors(void);
            _highlight_colors();
        }
    }
}


void
_griset_colorschemegui(string arg)
{
    if (strlen(arg)) {
        gri_colorisergui = arg;
        if (!coloriserenv) {
            int truecolor;

            get_term_feature(TF_TRUECOLOR, truecolor);
            if (truecolor) {
                colorscheme(arg);
            }
        }
    }
}


void
_griset_colorscheme256(string arg)
{
    if (strlen(arg)) {
        gri_coloriser256 = arg;
        if (!coloriserenv) {
            int depth, truecolor;

            inq_screen_size(NULL, NULL, depth);
            if (depth >= 256) {
                get_term_feature(TF_TRUECOLOR, truecolor);
                if (gri_colorisergui == "" || truecolor == 0) {
                    colorscheme(arg);
                }
            }
        }
    }
}


void
_griset_colorscheme88(string arg)
{
    if (strlen(arg)) {
        gri_coloriser88 = arg;
        if (!coloriserenv) {
            int depth;

            inq_screen_size(NULL, NULL, depth);
            if (88 == depth ||
                    (depth > 88 && !gri_coloriser256)) {
                colorscheme(arg);
            }
        }
    }
}


void
_griset_colorscheme16(string arg)
{
    if (strlen(arg)) {
        gri_coloriser16 = arg;
        if (!coloriserenv) {
            int depth;

            inq_screen_size(NULL, NULL, depth);
            if (16 == depth ||
                    (depth > 16 && !gri_coloriser256 && !gri_coloriser88)) {
                colorscheme(arg);
            }
        }
    }
}


void
_griset_colorscheme(string arg)
{
    if (strlen(arg)) {
        gri_coloriser = arg;
        if (!coloriserenv) {
            int depth;

            inq_screen_size(NULL, NULL, depth);
            if (!gri_coloriser256 && !gri_coloriser88 && !gri_coloriser16) {
                colorscheme(arg);
            }
        }
    }
}


string
_griget_colorschemegui(void)
{
    return gri_colorisergui;
}


string
_griget_colorscheme256(void)
{
    return gri_coloriser256;
}


string
_griget_colorscheme88(void)
{
    return gri_coloriser88;
}


string
_griget_colorscheme16(void)
{
    return gri_coloriser16;
}

string
_griget_colorscheme(void)
{
    if (inq_macro("inq_coloriser", 2) > 0) {
        string scheme = inq_coloriser();
        if (scheme) {
            return scheme;                      // active colorscheme
        }
    }
    return gri_coloriser;
}


string
_griget_window_color(void)
{
    return gri_window_color;
}


void
_griset_window_color(string arg)
{
    int attrib = atoi(arg);

    if (attrib) {
        gri_window_color = attrib;
        window_color(attrib);
    }
}


/*
 *  write_buffer ---
 *      Write either the marked or current buffer content to disc.
 */
replacement int
write_buffer()
{
    int ret, old_msg_level;

    if (inq_called() != "") {
        return write_buffer();
    }
    old_msg_level = inq_msg_level();
    if (inq_marked()) {
        set_msg_level(1);
        ret = write_block();
    } else {
        set_msg_level(0);
        ret = write_buffer();
    }
    set_msg_level(old_msg_level);
    return ret;
}


/*
 *  clear_buffer ---
 *      Clear the current buffer.
 */
void
clear_buffer(void)
{
    top_of_buffer();
    drop_anchor();
    end_of_buffer();
    delete_block();
}

/*end*/
