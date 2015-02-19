/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: colors.cr,v 1.15 2014/11/27 15:54:09 ayoung Exp $
 * Enhanced colour/colorscheme support.
 *
 *
 */

#include "grief.h"

#define SCHEME_CTERMONLY    0x0001
#define SCHEME_GUIONLY      0x0002

static int              colorscheme_view(int ids);

static void             ca_keys(void);
static void             ca_attribute(void);
static void             ca_dialoginit(void);
static string           ca_type(string value);
static void             ca_setfg(string type, int set);
static void             ca_setbg(string type, int set);
static int              ca_callback(int ident, string name, int p1, int p2);

static int              colorscheme_vim(~ string base, ~int flags);
static int              schemeload(string scheme, ~list args, ~string base, ~int flags);
static void             schemeload_vim(string scheme, string base, int flags, string file);

void                    vim_groupmap(string name);
string                  vim_attrmap(string attr);

static list             scheme_colors = {
    //  VIM to GRIEF attribute (where possible)
    //
    //      default highlighting groups.
    //
    "=ColorColumn=",    "ruler_column",         // used for the columns set with 'colorcolumn'.  (ie. colorcolumn=80)

    "=Conceal=",        "",                     // place-holder characters substituted for concealed text.

    "=Cursor=",         "cursor",               // the character under the cursor.
    "=CursorIM=",       "",                     // like Cursor, but used when in IME mode.
    "=CursorColumn=",   "cursor_col",           // the screen column that the cursor is in when 'cursorcolumn' is set.
    "=CursorLine=",     "cursor_row",           // the screen line that the cursor is in when 'cursorline' is set.

    "=Directory=",      "lsdirectory",          // directory names (and other special names in listings).

    "=DiffAdd=",        "additional",           // diff: added line.
    "=DiffChange=",     "modified",             // diff: changed line.
    "=DiffDelete=",     "diffdelete",           // diff: deleted line.
    "=DiffText=",       "difftext",             // diff: changed text within a changed line.

    "=ModeMsg=",        "",                     // 'showmode' message.
    "=MoreMsg=",        "message",              // |more-prompt|.
    "=WarningMsg=",     "",                     // warning messages.
    "=ErrorMsg=",       "error",                // error messages on the command line.

    "=VertSplit=",      "frame",                // the column separating vertically split   windows.

            // TODO ==> folded/fold_column
    "=Folded=",         "",                     // line used for closed folds.
    "=FoldColumn=",     "",                     // Foldcolumn.

    "=SignColumn=",     "column_status",        // column where *signs* are displayed.
    "=LineNr=",         "column_lineno",        // line number.

    "=IncSearch=",      "search_inc",           // 'incsearch' highlighting; also used for the text replaced with.
    "=MatchParen=",     "search_match",         // a paired character.

    "=NonText=",        "nonbuffer",            // '~' and '@' at the end of the window.

    "=Normal=",         "normal",               // normal text.

    "=Pmenu=",          "popup_normal",         // popup: normal item.
    "=PmenuSel=",       "popup_hilite",         // popup: selected item.
    "=PmenuSbar=",      "scrollbar",            // popup: scrollbar.
    "=PmenuThumb=",     "scrollbar_thumb",      // popup: thumb of the scrollbar.

    "=Question=",       "prompt",               // prompt and yes/no questions.

    "=Search=",         "search",               // last search pattern highlighting.

    "=SpecialKey=",     "",                     // meta and special keys listed with ":map", also unprintable characters.

    "=Todo=",           "todo",                 // anything that needs extra attention; mostly the keywords TODO FIXME and XXX.

    "=SpellBad=",       "spell",                // spell: recognized word.
    "=SpellCap=",       "",                     // spell: word that should start with a capital.
    "=SpellLocal=",     "",                     // spell: recognized word as one that is used in another region.
    "=SpellRare=",      "",                     // spell: recognized word as one that is hardly ever used.
    "=Spell=",          "spell",

    "=StatusLine=",     "echo_line",            // status line of current window.
    "=StatusLineNC=",   "",                     // status lines of not-current windows.

    "=TabLine=",        "",                     // tab pages line, not active tab page label.
    "=TabLineFill=",    "",                     // tab pages line, where there are no labels.
    "=TabLineSel=",     "",                     // tab pages line, active tab page label.

    "=Title=",          "select",               // titles for output from ":set all", ":autocmd" etc.

    "=Visual=",         "hilite",               // visual mode selection.
    "=VisualNOS=",      "",                     // visual mode selection when vim is "Not Owning the Selection".

    "=WildMenu=",       "prompt_complete",      // current match in 'wildmenu' completion.

    //  syntax groups/
    //      naming conventions.
    //
    "=Comment=",        "comment",              // any comment.

    "=Constant=",       "constant",             // any constant.
    "=String=",         "string",               //  a string constant:      "this is a string"
    "=Character=",      "character",            //  a character constant:   'c', '\n'
    "=Number=",         "number",               //  a number constant:      123, 012, 0x12
    "=Boolean=",        "boolean",              //  a boolean constant:     TRUE, false
    "=Float=",          "float",                //  a floating constant:    2.3e10

            // TODO ==> identifier
    "=Identifier=",     "keyword_function",     // any variable name.
    "=Function=",       "keyword_function",     //  function name (also: methods for classes).

    "=PreProc=",        "preprocessor",         // preprocessor.
    "=Include=",        "preprocessor_include",     //  preprocessor #include
    "=Define=",         "preprocessor_define",      //  preprocessor #define
    "=Macro=",          "preprocessor_keyword",     //  same as Define
    "=PreCondit=",      "preprocessor_conditional", //  preprocessor #if, #else, #endif, etc.

    "=Statement=",      "code",                 // any statement.
    "=Conditional=",    "keyword_conditional",  //  if, then, else, endif, switch, etc.
    "=Repeat=",         "keyword_repeat",       //  for, do, while, etc.
    "=Label=",          "keyword_label",        //  case, default, etc.
    "=Operator=",       "operator",             //  operators
    "=Keyword=",        "keyword",              //  any other keyword
    "=Exception=",      "keyword_exception",    //  try, catch, throw

    "=Type=",           "keyword_type",         // int, long, char, etc.
    "=StorageClass=",   "keyword_storageclass", //  static, register, volatile, etc.
    "=Structure=",      "keyword_structure",    //  struct, union, enum, etc.
    "=Typedef=",        "keyword_typedef",      //  Atypedef.

    "=Special=",        "standout",             // any special symbol.
    "=SpecialChar=",    "constant_standout",    //  special character in a constant.
    "=Tag=",            "tag",                  //  you can use CTRL-] on this.
    "=Delimiter=",      "delimiter",            //  character that needs attention.
    "=SpecialComment=", "comment_standout",     //  special things inside a comment.
    "=Debug=",          "keyword_debug",        //  debugging statements.

    "=Underlined=",     "link",                 // text that stands out, HTML links.

    "=Ignore=",         "",                     // left blank, hidden.

    "=Error=",          "alert",                // any erroneous construct.

    //  misc/
    //
    "=Tab=",            "whitespace",
    "=Space=",          "",
    };

static list             vim_paths = {           // FIXME - needs work
    "~/.vim/colors/",
    "/usr/share/vim/vimcurrent/colors/"
    };

static string           scheme_background = "default";
static string           scheme_name = "";       // current color scheme.

enum {
#define IDENT_BASE          1000

    IDENT_BOLD = IDENT_BASE,
    IDENT_INVERSE,
    IDENT_UNDERLINE,
    IDENT_BLINK,
    IDENT_ITALIC,
    IDENT_REVERSE,
    IDENT_STANDOUT,
    IDENT_UNDERCURL,
    IDENT_DIM,
    IDENT_NONE          // must be last
};

static list             attrsset = {
    "bold",
    "inverse",
    "underline",
    "blink",
    "italic",
    "reverse",
    "standout",
    "undercurl",
    "dim"
    };

static list             color_types = {
    "none",
    "clear",
    "symbolic",
    "label",
    "value",
    "rgb"
    };

static list             color_names = {
    COLORS_ALL,
    "dynamic-fg",
    "dynamic-bg",
    "foreground",
    "fg",
    "background",
    "bg",
    };

static int              ca_dialog;              // dialog resource.

static list             coloriser_list(void);
static list             vimcoloriser_list(void);


/*  Function:           main
 *      Colorscheme initialisation, build the editor dialog as follows;
 *
 *>         Name        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
 *>         Foregound   xxxxxxxxxxx[v] xxxxxxxxxxxxxxxxxxxx
 *>         Background  xxxxxxxxxxx[v] xxxxxxxxxxxxxxxxxxxx
 *>
 *>                        [x] flag   [x] flag   [x] flag
 *>            [x] none    [x] flag   [x] flag   [x] flag
 *>                        [x] flag   [x] flag   [x] flag
 *>
 *>         Link        xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
 *>         Sticky      xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
 *>
 *>                 < Done >  < Apply >  < Help >
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 */
void
main(void)
{
    require("colorlabels");
}


/*  Function:           coloriser
 *      Coloriser loader.
 *
 *  Parameters:
 *      scheme -            Optional color scheme name.
 *
 *  Returns:
 *      nothing
 */
void
coloriser(~ string)
{
    string scheme;

    if (get_parm(0, scheme, "Coloriser: ", NULL, NULL) < 0) {
        return;
    }
    colorscheme(trim(scheme, "\t\n\r "));
}


string
inq_coloriser(void)
{
    return scheme_name;
}


string
_completion_Coloriser(string arg)
{
    list t_schemes, schemes;
    string pat, mode;
    int count, idx;
    int ret;

    count = -1;
    if (arg) {
        t_schemes = coloriser_list();
        t_schemes += vimcoloriser_list();
        pat = "<_" + quote_regexp(arg) + "[a-zA-Z]@_mode>";
        idx = re_search(NULL, pat, t_schemes);
        while (idx >= 0) {
            mode = substr(t_schemes[idx], 2);
            schemes[++count] = substr(mode, 1, index(mode, "_")-1);
            idx = re_search(NULL, pat, t_schemes, idx+1);
        }
    } else {
        schemes = coloriser_list();
        schemes += vimcoloriser_list();
        count = length_of_list(schemes);
    }

    if (count < 0) {
        beep();
        return "";
    }

    if (0 == count) {
        ret = 0;
    } else {
        schemes = sort_list(schemes);
        ret = select_list("Coloriser", "", 1, schemes, SEL_CENTER,
                "help_display \"features/coloriser.hlp\" \"Help on Coloriser\"");
        refresh();
    }

    if (ret < 0) {
        return "";
    }
    return schemes[ret - 1];
}


/*  Function:           colorscheme
 *      colorscheme loader/viewer/editor.
 *
 *  Parameters:
 *      scheme -            Color scheme name.
 *
 *  Returns:
 *      *true* if successful, otherwise *false*.
 */
int
colorscheme(~ string scheme, ...)
{
    //  view or loader
    //
    if ("" == scheme) {
        colorscheme_view(FALSE);
        return FALSE;

    } else if ("--ids" == scheme) {
        colorscheme_view(TRUE);
        return FALSE;

    } else if ("--vim" == scheme) {
        return colorscheme_vim();

    } else if ("--vim-gui" == scheme) {
        return colorscheme_vim(NULL, SCHEME_GUIONLY);

    } else if ("--vim-cterm" == scheme) {
        return colorscheme_vim(NULL, SCHEME_CTERMONLY);

    } else if ("--vim=dark" == scheme) {
        return colorscheme_vim("dark");

    } else if ("--vim=light" == scheme) {
        return colorscheme_vim("light");
    }

    //  default schemes/
    //      dark, light or default
    //
    if ("dark" == scheme || "light" == scheme || "default" == scheme) {
        set_color("scheme=" + scheme);
        message("colorscheme: '%s'", scheme);
        return TRUE;
    }

    return schemeload(scheme, arg_list(TRUE, 1));
}


//
//  Build and return list of available coloriser modes.
//
static list
coloriser_list(void)
{
    string path, name;
    list paths, ret;

    path = expandpath("${GRPATH}", TRUE);
    paths = split(path, CRISP_DIRSEP);
    while (list_each(paths, path) >= 0) {
        path = expandpath(path + "/colors/");
        file_pattern(path + "*" + GREXTENSION);
        while (find_file(name)) {
            name = basename(name, GREXTENSION);
            if (re_search(SF_IGNORE_CASE, "<" + name + ">", ret) < 0) {
                ret += name;
            }
        }
    }
    return ret;
}


static list
vimcoloriser_list(void)
{
    string path, name;
    list ret;

    while (list_each(vim_paths, path) >= 0) {
        path = expandpath(path, TRUE);
        file_pattern(path + "*.vim");
        while (find_file(name)) {
            name = basename(name, ".vim");
            if (re_search(SF_IGNORE_CASE, "<" + name + ">", ret) < 0) {
                ret += name;
            }
        }
    }
    return ret;
}


//
//  Display current color scheme definition.
//
static int
colorscheme_view(int ids)
{
    string title;
    int buf, idx;

    save_position();
    sprintf(title, "Color Scheme [%s]", (scheme_name ? scheme_name : "none"));

    if ((buf = create_buffer(title, NULL, 1)) >= 0) {
        list colors = get_color(ids ? (COLORGET_FNAME|COLORGET_FVALUE|COLORGET_FFLAGS) : COLORGET_FNAME|COLORGET_FFLAGS);
        int depth, win, attr;
        string spec;

        set_buffer(buf);
        attr = inq_attribute();                 // current attribute
        get_term_feature(TF_COLORDEPTH, depth);
        set_buffer_flags(buf, "attributes");
        while (list_each(colors, spec) >= 0) {
            //
            //  [<id>,] <flags>, <name> = <spec>
            //
            const list parts = split(spec, "=");
            const list name = split(parts[0], ",");

            if (ids) {                          // [<id>,] <name>
                insertf("%s,%s", name[0], name[2]);
            } else {
                insertf("%s", name[1]);
            }

            move_abs(NULL, 26);                 // demo text
            if (depth > 2) {
                set_attribute(ids ? name[2] : name[1]);
            }
            insert("[AaBc12]");
            set_attribute(attr);

            insertf(" %s", parts[1]);           // specification

            move_abs(NULL, 80);                 // <flag>, <specification>
            delete_to_eol();
            if (ids) {
                insertf(";%s,%s=%s\n", name[1], name[2], parts[1]);
            } else {
                insertf(";%s,%s=%s\n", name[0], name[1], parts[1]);
            }
        }

        win = sized_window(inq_lines(), 78, "<Esc> exit");
        idx = select_buffer(buf, win, SEL_NORMAL, inq_module() + "::ca_keys");
        delete_buffer(buf);
    }
    restore_position(2);
}


static void
ca_keys(void)
{
    assign_to_key("<Enter>", "::ca_attribute");
}


static void
ca_attribute(void)
{
#define CA_BG           0x01
#define CA_FG           0x02
#define CA_ATTR         0x04

    int     none = 1, k;
    list    parts, fgbg;
    string  ca_name;
    int     ca_flags;

    //  flags,attribute=foreground[,background][: [link@name|sticky@name|styles][, ...]]
    //
    move_abs(NULL, 81);
    string spec = trim(read());
    move_abs(NULL, 1);
    if (0 == strlen(spec)) {
        return;
    }

    //  clear dialog
    if (0 == ca_dialog) {
        ca_dialoginit();
    }
    parts = split(spec, "=");
    sscanf(parts[0], "%u,%s", ca_flags, ca_name);
    widget_set(ca_dialog, "name", ca_name);
    for (k = IDENT_BASE; k < IDENT_NONE; ++k) {
        widget_set(ca_dialog, k, 0);
    }
    widget_set(ca_dialog, "link", "");
    widget_set(ca_dialog, "sticky", "");

    //  initialise dialog
    if (index(parts[1], ':')) {
        list colors_attrs = split(parts[1], ':');
        list attrs = split(colors_attrs[1], ",@");
        int  i;

        for (i = 0; i < length_of_list(attrs); ++i) {
            switch(attrs[i]) {
            case "link":        // link@name
                widget_set(ca_dialog, "link", attrs[++i]);
                break;

            case "sticky":      // sticky@name
                widget_set(ca_dialog, "sticky", attrs[++i]);
                break;

            default:            // bold etc
                if ((k = re_search(SF_IGNORE_CASE, "<" + attrs[i] + ">", attrsset)) >= 0)  {
                    widget_set(ca_dialog, IDENT_BASE + k, 1);
                    none = 0;
                }
                break;
            }
        }
        fgbg = split(colors_attrs[0], ',');

    } else {
        fgbg = split(parts[1], ',');
    }

    if ((CA_FG|CA_ATTR) & ca_flags) {
        ca_setfg(fgbg[0], 1);
    } else {
        ca_setfg("n/a", -1);
    }

    if ((CA_BG|CA_ATTR) & ca_flags) {
        if (CA_ATTR & ca_flags) {
            ca_setbg(fgbg[1], 1);
        } else {
            ca_setbg(fgbg[0], 1);
        }
    } else {
        ca_setbg("n/a", -1);
    }

    widget_set(ca_dialog, IDENT_NONE, none);

    if (1 == dialog_run(ca_dialog)) {
    }

    sel_down();
}


static void
ca_dialoginit(void)
{
    extern list color_labels;                   // global color labels
    list labels = list_extract(color_labels, NULL, NULL, 1);

    ca_dialog = dialog_create( make_list(
        DLGA_TITLE,                             "Attribute",
        DLGA_CALLBACK,                          "::ca_callback",
        DLGA_ALIGN_W,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGA_PADY,                          1,
            DLGC_LABEL,
                DLGA_ATTACH_LEFT,
                DLGA_LABEL,                     "Name:",
                DLGA_COLS,                      12,
            DLGC_LABEL,
                DLGA_ATTACH_LEFT,
                DLGA_NAME,                      "name",
                DLGA_COLS,                      24,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGC_LABEL,
                DLGA_ATTACH_LEFT,
                DLGA_LABEL,                     "&Foreground:",
                DLGA_COLS,                      12,
                DLGA_ROWS,                      2,
            DLGC_COMBO_FIELD,
                DLGA_ATTACH_LEFT,
                DLGA_NAME,                      "fg_type",
                DLGA_COLS,                      12,
                DLGA_HOTKEY,                    'f',        // Alt-F
                DLGA_LBELEMENTS,                color_types,
                DLGA_LBINDEXMODE,               TRUE,
                DLGA_CBEDITABLE,                TRUE,
                DLGA_CBPOPUPMODE,               2,          // Open
                DLGA_CBAUTOCOMPLETEMODE,        2,          // Append
            DLGC_TAB,
                DLGA_ATTACH_LEFT,
                DLGA_PADX,                      1,
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_RIGHT,
                    DLGA_VISIBLE,
                    DLGA_NAME,                  "fg_name",
                    DLGA_COLS,                  20,
                    DLGA_LBELEMENTS,            color_names,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    3,          // SuggestAppend
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_RIGHT,
                    DLGA_VISIBLE,
                    DLGA_NAME,                  "fg_label",
                    DLGA_COLS,                  20,
                    DLGA_LBSORTMODE,            1,
                    DLGA_LBELEMENTS,            labels,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    3,          // SuggestAppend
                DLGC_EDIT_FIELD,
                    DLGA_ATTACH_RIGHT,
                    DLGA_VISIBLE,
                    DLGA_NAME,                  "fg_edit",
                    DLGA_COLS,                  23,
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGC_LABEL,
                DLGA_ATTACH_LEFT,
                DLGA_LABEL,                     "&Background:",
                DLGA_COLS,                      12,
            DLGC_COMBO_FIELD,
                DLGA_ATTACH_LEFT,
                DLGA_NAME,                      "bg_type",
                DLGA_COLS,                      12,
                DLGA_HOTKEY,                    'b',        // Alt-B
                DLGA_LBELEMENTS,                color_types,
                DLGA_LBINDEXMODE,               TRUE,
                DLGA_CBEDITABLE,                TRUE,
                DLGA_CBPOPUPMODE,               2,          // Open
                DLGA_CBAUTOCOMPLETEMODE,        2,          // Append
                DLGA_CBRELAXMODE,               2,          // Any
            DLGC_TAB,
                DLGA_ATTACH_LEFT,
                DLGA_PADX,                      1,
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_RIGHT,
                    DLGA_VISIBLE,
                    DLGA_NAME,                  "bg_name",
                    DLGA_COLS,                  20,
                    DLGA_LBELEMENTS,            color_names,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    3,          // SuggestAppend
                DLGC_COMBO_FIELD,
                    DLGA_ATTACH_RIGHT,
                    DLGA_VISIBLE,
                    DLGA_NAME,                  "bg_label",
                    DLGA_COLS,                  20,
                    DLGA_LBSORTMODE,            1,
                    DLGA_LBELEMENTS,            labels,
                    DLGA_LBINDEXMODE,           TRUE,
                    DLGA_LBPAGEMODE,            TRUE,
                    DLGA_CBEDITABLE,            TRUE,
                    DLGA_CBPOPUPMODE,           2,          // Open
                    DLGA_CBAUTOCOMPLETEMODE,    3,          // SuggestAppend
                DLGC_EDIT_FIELD,
                    DLGA_ATTACH_RIGHT,
                    DLGA_VISIBLE,
                    DLGA_NAME,                  "bg_edit",
                    DLGA_COLS,                  23,
            DLGC_END,
        DLGC_END,

        DLGC_GROUP,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_CENTER,
            DLGA_TITLE,                         "Attributes",
            DLGA_PADY,                          1,
            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_ALIGN_W,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_PADX,                  1,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "none",
                        DLGA_IDENT,             IDENT_NONE,
                        DLGA_ALIGN_W,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_PADX,                  1,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "bold",
                        DLGA_IDENT,             IDENT_BOLD,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "standout",
                        DLGA_IDENT,             IDENT_STANDOUT,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "italic",
                        DLGA_IDENT,             IDENT_ITALIC,
                        DLGA_ALIGN_W,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_PADX,                  1,
                    DLGA_TABSTOP,               0,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "inverse",
                        DLGA_IDENT,             IDENT_INVERSE,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "reverse",
                        DLGA_IDENT,             IDENT_REVERSE,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "blink",
                        DLGA_IDENT,             IDENT_BLINK,
                        DLGA_ALIGN_W,
                DLGC_END,
                DLGC_CONTAINER,
                    DLGA_ATTACH_LEFT,
                    DLGA_PADX,                  1,
                    DLGA_TABSTOP,               0,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "underline",
                        DLGA_IDENT,             IDENT_UNDERLINE,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "undercurl",
                        DLGA_IDENT,             IDENT_UNDERCURL,
                        DLGA_ALIGN_W,
                    DLGC_CHECK_BOX,
                        DLGA_LABEL,             "dim",
                        DLGA_IDENT,             IDENT_DIM,
                        DLGA_ALIGN_W,
                DLGC_END,
            DLGC_END,                           // CONTAINER
        DLGC_END,                               // GROUP

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGA_ALIGN_W,
            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_PADX,                      1,
                DLGC_LABEL,
                    DLGA_LABEL,                 "Link:",
                    DLGA_COLS,                  11,
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,                 "Sticky:",
                    DLGA_COLS,                  11,
                    DLGA_ALIGN_W,
            DLGC_END,
            DLGC_CONTAINER,
                DLGA_ATTACH_RIGHT,
                DLGA_PADX,                      1,
                DLGC_EDIT_FIELD,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "link",
                    DLGA_ROWS,                  1,
                    DLGA_COLS,                  24,
                    DLGA_ALLOW_FILLX,
                DLGC_EDIT_FIELD,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "sticky",
                    DLGA_ROWS,                  1,
                    DLGA_COLS,                  24,
                    DLGA_ALLOW_FILLX,
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_BOTTOM,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,                     "&Done",
                DLGA_NAME,                      "done",
                DLGA_ATTACH_LEFT,
                DLGA_DEFAULT_FOCUS,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,                     "&Apply",
                DLGA_NAME,                      "apply",
                DLGA_ATTACH_LEFT,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,                     "&Help",
                DLGA_NAME,                      "help",
                DLGA_ATTACH_LEFT,
        DLGC_END                                // CONTAINER
        ));
}


static string
ca_type(string value)
{
    if (value) {
        if (re_search(SF_UNIX, "^#", value) > 0 ||
                re_search(SF_UNIX, "^rgb", value) > 0 ||
                re_search(SF_UNIX, "^hsl", value) > 0 ||
                re_search(SF_UNIX, "^cvt", value) > 0) {
            return "rgb";

        } else if (re_search(SF_UNIX, "^%", value) > 0) {
            return "label";

        } else if (re_search(SF_UNIX, "^[0-9]+$", value) > 0) {
            return "value";
        }
    }
    return value;
}


static void
ca_setfg(string value, int set)
{
    string type = ca_type(value);
    int edit = DLGA_HIDDEN, label = DLGA_HIDDEN, name = DLGA_HIDDEN;

    if (set >= 0) {
        switch (type) {
        case "":
        case "none":
        case "clear":
            break;

        case "rgb":
        case "value":
            if (set) widget_set(ca_dialog, "fg_edit", value);
            edit = DLGA_VISIBLE;
            break;

        case "label":
            if (set) widget_set(ca_dialog, "fg_label", value);
            label = DLGA_VISIBLE;
            break;

        default:
            if (set) widget_set(ca_dialog, "fg_name", value);
            name = DLGA_VISIBLE;
            type = "symbolic";
            break;
        }
    }
    widget_set(ca_dialog, "fg_type", type);
    widget_set(ca_dialog, "fg_type", 1, (set >= 0 ? DLGA_ACTIVE : DLGA_GREYED));
    widget_set(ca_dialog, "fg_edit", 1, edit);
    widget_set(ca_dialog, "fg_name", 1, name);
    widget_set(ca_dialog, "fg_label", 1, label);
}


static void
ca_setbg(string value, int set)
{
    string type = ca_type(value);
    int edit = DLGA_HIDDEN, label = DLGA_HIDDEN, name = DLGA_HIDDEN;

    if (set >= 0) {
        switch (type) {
        case "":
        case "none":
        case "clear":
            break;

        case "rgb":
        case "value":
            if (set) widget_set(ca_dialog, "bg_edit", value);
            edit = DLGA_VISIBLE;
            break;

        case "label":
            if (set) widget_set(ca_dialog, "bg_label", value);
            label = DLGA_VISIBLE;
            break;

        default:
            if (set) widget_set(ca_dialog, "bg_name", value);
            name = DLGA_VISIBLE;
            type = "symbolic";
            break;
        }
    }
    widget_set(ca_dialog, "bg_type", type);
    widget_set(ca_dialog, "bg_type", 1, (set >= 0 ? DLGA_ACTIVE : DLGA_GREYED));
    widget_set(ca_dialog, "bg_edit", 1, edit);
    widget_set(ca_dialog, "bg_name", 1, name);
    widget_set(ca_dialog, "bg_label", 1, label);
}


static string
ca_getfg(void)
{
    string value = widget_get(ca_dialog, "fg_type");
    switch (value) {
    case "":
    case "none":
    case "clear":
        break;
    case "value":
    case "rgb":
        value = widget_get(ca_dialog, "fg_edit");
        break;
    case "label":
        value = widget_get(ca_dialog, "fg_label");
        break;
    case "symbolic":
        value = widget_get(ca_dialog, "fg_name");
        break;
    }
    return value;
}


static string
ca_getbg(void)
{
    string value = widget_get(ca_dialog, "bg_type");
    switch (value) {
    case "":
    case "none":
    case "clear":
        break;
    case "value":
    case "rgb":
        value = widget_get(ca_dialog, "bg_edit");
        break;
    case "label":
        value = widget_get(ca_dialog, "bg_label");
        break;
    case "symbolic":
        value = widget_get(ca_dialog, "bg_name");
        break;
    }
    return value;
}


static int
ca_callback(int ident, string name, int p1, int p2)
{
    extern string ca_name;
    extern int ca_flags;
    UNUSED(ident, p2);

    message("ident=0x%x, name=%s, p1=%d/0x%x, p2=%d/0x%x", ident, name, p1, p1, p2, p2);
    switch (p1) {
    case DLGE_INIT:
        break;

    case DLGE_CHANGE:
        switch (name) {
        case "fg_type":
            ca_setfg(widget_get(NULL, name), 0);
            break;
        case "bg_type":
            ca_setbg(widget_get(NULL, name), 0);
            break;
        default:
            break;
        }
        break;

    case DLGE_BUTTON:
        switch (name) {
        case "apply": {
                string fg = (((CA_FG|CA_ATTR) & ca_flags) ? ca_getfg() : "");
                string bg = (((CA_BG|CA_ATTR) & ca_flags) ? ca_getbg() : "");
                string delim, val;
                int k;

                if (fg) {                       // foreground
                    if (strlen(bg)) {
                        if (fg != bg || ("none" != bg && "clear" != bg)) {
                            fg += "," + bg;     // foreground, background
                        }
                    }
                } else if (bg) {
                    fg = bg;                    // background
                } else {
                    fg = "clear";
                }

                delim = ":";                    // bold ...
                for (k = 0; k < length_of_list(attrsset); ++k) {
                    if (widget_get(ca_dialog, IDENT_BASE + k)) {
                        fg += delim + attrsset[k];
                        delim = ",";
                    }
                }

                val = widget_get(NULL, "link");
                if (val) {                      // link@<link>
                    fg += delim + "link@" + val;
                    delim = ",";
                }

                val = widget_get(NULL, "sticky");
                if (val) {                      // sticky@<name>
                    fg += delim + "sticky@" + val;
                    delim = ",";
                }

                val = ca_name + "=" + fg;
                message("%s", val);
                set_color(val);
            }
            break;
        case "done":
            dialog_exit();
            break;
        case "help":
            execute_macro("explain colorscheme");
            break;
        }
        break;
    }
    return TRUE;
}


//
//  Search for loadable 'vim' color schemes.
//
static int
colorscheme_vim(~ string base, ~int flags)
{
    list schemes;

    schemes = vimcoloriser_list();
    if (0 == length_of_list(schemes)) {
        message("colorscheme: no vim style color schemes located");
        return FALSE;
    }

    int idx = 0;

    schemes = sort_list(schemes);
    while (1) {
        if ((idx = select_list("Color Schemes", "select or <esc>", 1,
                        schemes, SEL_CENTER, NULL, NULL, idx + 1)) <= 0) {
            break;
        }
        schemeload(schemes[idx - 1], NULL, base, flags);
        redraw();
    }
    return TRUE;
}


//
//  Load the specific color scheme.
//
static int
schemeload(string scheme, ~list args, ~string base, ~int flags)
{
    scheme = re_translate(SF_GLOBAL, "[ \t]+", "_", scheme);

    if (require("colors/" + scheme) >= 0) {
        if (inq_macro("colorscheme_" + scheme)) {
            if (is_null(args)) {
                execute_macro("colorscheme_" + scheme);
            } else {
                execute_macro("colorscheme_" + scheme, args);
            }
        }
        message("colorscheme: '%s'", scheme);
        scheme_name = scheme;
        return TRUE;
    }

    //  import a vim colorscheme/
    //      use with caution.
    //
    //  Example:
    //      runtimepath=
    //          ~/.vim
    //          /usr/share/vim/vimcurrent
    //              --> /usr/share/vim/vim[version]
    //
    string path, file;
    int colors, mode;

    get_term_feature(TF_COLORDEPTH, colors);    // colors
    for (mode = (colors > 16 ? 0 : 5); mode <= 5; ++mode) {
        while (list_each(vim_paths, path) >= 0) {
            //
            //  1 - scheme<colours>.vim
            //  2 - scheme_<colors>.vim
            //  3 - <colors>scheme.vim
            //  4 - <colors>_scheme.vim
            //  5 - scheme.vim
            //
            path = expandpath(path);
            switch (mode) {
            case 1:                             // i.e. scheme256.vim
                file = scheme + colors;
                break;
            case 2:                             // i.e. scheme_256.vim
                file = scheme + "_" + colors;
                break;
            case 3:                             // i.e. 256scheme.vim
                file = colors + scheme;
                break;
            case 4:                             // i.e. 256_scheme.vim
                file = colors + "_" + scheme;
                break;
            case 5:                             // i.e. scheme.vim
                file = scheme;
                break;
            }

            if (exist(path + file + ".vim")) {
                schemeload_vim(scheme, base, flags, path + file + ".vim");
                list_reset(vim_paths);
                return TRUE;
            }
        }
    }
    message("colorscheme: unavailable to locate '%s'", scheme);
    return FALSE;
}


static void
schemeload_vim(string scheme, string base, int flags, string file)
{
    int buf, curbuf = inq_buffer();
    list spec;

    if ((buf = create_buffer("-colorscheme-vim-", file, TRUE)) >= 0) {
        string ln;
        int gui;

        message("loading: %s", file);
        set_buffer(buf);
        top_of_buffer();
        do {
            ln = compress(read(), TRUE);        // hi or :hi, set or let, conditionals
            if (re_search(0, "^[:]@hi", ln) > 0 ||
                re_search(0, "^[sl]et", ln) > 0 ||
                re_search(0, "^{if}|{else}|{endif}", ln) > 0) {
                spec += ln;
            }
        } while (down());
        set_buffer(curbuf);
        delete_buffer(buf);

        if (SCHEME_GUIONLY & flags) {
            //
            //  GUI only
            //
            vim_colorschemex(scheme, 0, base, spec, TRUE, gui);

        } else if (0 == vim_colorschemex(scheme, 0, base, spec, FALSE, gui) && gui > 15) {
            //
            //  Console load failure, yet there maybe suitable GUI color settings
            //      try loading the GUI settings (unless CTERMONLY).
            //
            if (0 == (SCHEME_CTERMONLY & flags)) {
                vim_colorschemex(scheme, 0, base, spec, TRUE, gui);
            }
        }
    }
}


/*  Function:           vim_colorscheme
 *      Load a VIM style colorscheme.
 *
 *      This interface is simple and requires a strict structure yet follows many of the
 *      basic colour schemas reviewed.
 *
 *      See blackdust for an example schema.
 *
 *      The latest top 100 VIM color schemes are www.vim.org are available as a single
 *      package "Color Scheme Pack", which generally fit the required structure for a
 *      successful import.
 *
 *      In additional are a number of sites which demo the colorschemes, to name a few.
 *
 *          http://www.vi-improved.org/color_sampler_pack/
 *          http://vimcolorschemetest.googlecode.com/svn/html/index-c.html
 *
 *      Color-Scheme editor
 *
 *          http://bytefluent.com/vivify/
 *
 *  Parameters:
 *      label -             Colorscheme name.
 *      colors -            Supported colors (8, 16, 88 or 256).
 *      base -              Base color scheme "dark" or "light".
 *      spec -              Highlight command list.
 *      asgui -             If *1* utilise gui colors otherwise terminal, *-1* dynamic.
 *      gui -               GUI element count.
 *
 *  Returns:
 *      Imported color count.
 */

int
vim_colorscheme(string label, int colors, ~string base, list spec, int asgui)
{
    int gui; return vim_colorschemex(label, colors, base, spec, asgui, gui);
}

int
vim_colorschemex(string label, int colors, ~string base, list spec, int asgui, int &gui)
{
    list ifstack;
    int  iflevel = -1, ifactive = 1, ifdepth;
    int  idx, done, links, ignored, cond;
    declare value;

    //  verify colors against current display environment
    get_term_feature(TF_COLORDEPTH, ifdepth);
    if (colors > 0) {
        if (ifdepth != colors) {
            message("%s: color depth %d not available (%d)", label, colors, ifdepth);
            return -1;
        }
    }

    if (-1 == asgui) {                          // dynamic
        asgui = (ifdepth > 256 ? TRUE : FALSE);
    }

    if (0 == strlen(base)) {                    // dark or light base
        get_term_feature(TF_COLORSCHEME, base);
        base = "dark";
    }

    scheme_name = label;

    //  load schema
    //      best effort load logic, with weak condition expression evaluation.
    //
    gui = 0;
    while ((idx = list_each(spec, value)) >= 0) {
        const list parts =
                tokenize(value, " ", TOK_DOUBLE_QUOTES|TOK_PRESERVE_QUOTES|TOK_WHITESPACE|TOK_COLLAPSE_MULTIPLE|TOK_TRIM);

        switch (parts[0]) {
        case "let":
        case "set":             // set command
            if (length_of_list(parts) >= 2) {
                //  set bg=[dark|light]/
                //  set background=[dark|light]/
                //      When set to "dark", use colors that look good on a dark background.  When
                //      set to "light", try to use colors that look good on a light background.
                //      Any other value is illegal.
                //
                //  set g:colors_name=<scheme>
                //  set colors_name=<scheme>
                //
                const list parts2 =
                        tokenize(parts[1], "=", TOK_DOUBLE_QUOTES|TOK_WHITESPACE|TOK_TRIM);

                if (length_of_list(parts2) >= 2) {
                    if ("bg" == parts2[0] || "background" == parts2[0]) {
                        const string arg = parts2[1];

                        if (0 == strcasecmp("dark", arg) ||
                                0 == strcasecmp("light", arg) ||
                                0 == strcasecmp("default", arg)) {
                            scheme_background = arg;
                        }
                    }
                }
            }
            break;

        case "hi":
        case "hi!":
        case ":hi":
        case "highlight":
        case ":highlight":      // hi[ghtlight] command
            if (length_of_list(parts) >= 2 && 1 == ifactive) {
                if ("clear" == parts[1]) {
                    //  clear
                    //      Reset all highlighting to the defaults. Removes all highlighting for groups
                    //      added by the user. Uses the current value of 'background' to decide which
                    //      default colors to use.
                    //
                    //  clear {group-name}
                    //      Disable the highlighting for the specified highlight group. It is *not* set
                    //      wback to the default colors.
                    //
                    const string groupname =
                        (length_of_list(parts) == 3 ? parts[2] : "");

                    // scheme=dark|light|default
                    if (0 == strlen(groupname)) {
                        set_color("scheme=" + scheme_background);
                    }
                    vim_groupmap(groupname);

                } else if ("link" == parts[1] ||
                                (length_of_list(parts) >= 3 && "link" == parts[2])) {
                    //  [default] link {from-group} {to-group}
                    //      Setup an attribute link.
                    //
                    //  [default] link {from-group} NONE
                    //      Remove an attribute link.
                    //
                    string link;

                    if ("default" == parts[1]) {
                        if (5 == length_of_list(parts)) {
                            string id1 = vim_attrmap(parts[3]),
                                    id2 = vim_attrmap(parts[4]);

                            if (strlen(id1) && strlen(id2)) {
                                sprintf(link, "%s=none:sticky@%s", id1, id2);
                            }
                        }
                    } else if (4 == length_of_list(parts)) {
                        string id1 = vim_attrmap(parts[2]),
                                id2 = vim_attrmap(parts[3]);

                        if (strlen(id1) && strlen(id2)) {
                            sprintf(link, "%s=none:link@%s", id1, id2);
                        }
                    }

                    if (link) {
                        set_color(link);
                        ++links;
                    } else {
                        ++ignored;
                    }

                } else {
                    //  [default] {group-name} key=arg
                    //      State an attributes color specification.
                    //
                    int baseidx = ("default" == parts[1] ? 2 : 1);
                    string fg, bg, sf,
                        ident = vim_attrmap(parts[baseidx]);
                    int i;

                    if (ident) {
                        for (i = baseidx + 1; i < length_of_list(parts); ++i) {
                            if ("\"" == parts[i]) {
                                break;          // comment
                            }

                            const list parts2 =
                                    tokenize(parts[i], "=", TOK_DOUBLE_QUOTES|TOK_WHITESPACE|TOK_TRIM);

                            if (length_of_list(parts2) >= 2) {
                                string val = parts2[1];

                                if (0 == asgui) {
                                    //  cterm={attr-list}
                                    //      Color termination attribute list. One or more of the
                                    //      following comma seperated values (without spaces),
                                    //
                                    //          bold, underline, undercurl, none
                                    //          reverse, inverse, italic, standout,
                                    //
                                    //  ctermfg={color}
                                    //      Color terminal foreground color.
                                    //
                                    //  ctermbg={color}
                                    //      Color terminal background color.
                                    //
                                    switch(parts2[0]) {
                                    case "cterm":   sf  = val; break;
                                    case "ctermfg": fg  = val; break;
                                    case "ctermbg": bg  = val; break;
                                    case "gui": ++gui; break;
                                    }
                                } else {
                                    //  gui={attr-list}
                                    //      Color termination attribute list. One or more of the
                                    //      following comma seperated values (without spaces).
                                    //
                                    //  font={font-name}
                                    //      Font (ignored).
                                    //
                                    //  guifg={color}
                                    //      Color terminal foreground color.
                                    //
                                    //  guibg={color}
                                    //      Color terminal background color.
                                    //
                                    //  guisp={color}
                                    //      Foregroud. background and special color.
                                    //
                                    switch(parts2[0]) {
                                    case "gui":     sf  = val; break;
                                    case "guifg":   fg  = val; break;
                                    case "guibg":   bg  = val; break;
                                    case "font":    break;
                                    case "guisp":   break;
                                    }
                                }
                            }
                        }

                        if (strlen(fg) || strlen(bg)) {
                            dprintf("%s: attribute: %s=%s fg:%s, bg:%s, sf:%s",
                                label, parts[baseidx], ident, fg, bg, sf);

                            switch (ident) {
                            case "normal":
                                if (bg) set_color_pair("background", bg);
                                if (fg) set_color_pair("normal", fg);
                                break;
                            case "hilite":
                                if (fg) {
                                    if (bg) {
                                        set_color_pair("hilite", fg, bg);
                                    } else {
                                        set_color_pair("hilite_fg", fg);
                                    }
                                } else {
                                    set_color_pair("hilite", "fg", bg);
                                }
                                break;
                            default:
                                if (0 == strlen(fg)) {
                                    fg = "fg";                  // current foreground

                                } else if (0 == strlen(bg) || 0 == strcasecmp(bg, "NONE")) {
                                    bg = "bg";                  // current background
                                }

                                if ("frame" == ident) sf = "";  // ignore underline etc

                                set_color_pair(ident, fg, bg, sf);
                                break;
                            }
                            if (asgui) ++gui;
                            ++done;
                        }
                    } else {
                        dprintf("%s: attribute: %s=unknown", label, parts[baseidx]);
                        ++ignored;
                    }
                }
            } else {
                dprintf("%s: <%s> in-active", label, value);
            }
            break;

        case "set_color":       // set_color
            //
            //  set_color <attribute> <specification>
            //
            if (3 == length_of_list(parts)) {
                set_color(parts[1], parts[2]);
            }
            break;

        case "if":              // if <condition>
            //
            //  if &background == "dark"
            //  if &background == "light"
            //
            //  if &t_Co >= 8, 16, 88, 256
            //  if &t_Co == 8, 16, 88, 256
            //
            //      note, other color condition expressions has been sighted yet the above
            //      represent 90% of all cases.
            //
            ifstack[ ++iflevel ] = ifactive;

            if (1 == ifactive) {
                ifactive = 1;                   // unknown condition, assume active

                if (length_of_list(parts) >= 2) {
                    if (strstr(value, "&background")) {
                        if (strstr(value, "==")) {
                            if (strstr(value, "&background")) {
                                ifactive = (0 != strstr(value, base));
                            }

                        } else if (strstr(value, "!=")) {
                            if (strstr(value, "&background")) {
                                ifactive = (0 == strstr(value, base));
                            }
                        }

                    } else if (strstr(value, "&t_Co") && colors > 0) {
                        if (strstr(value, "==")) {
                            ifactive = (strstr(value, "" + colors));

                        } else if (strstr(value, ">=")) {
                            if (strstr(value, "256")) {
                                ifactive = (colors >= 256);
                            } else if (strstr(value, "88")) {
                                ifactive = (colors >= 88);
                            } else if (strstr(value, "16")) {
                                ifactive = (colors >= 16);
                            } else if (strstr(value, "8")) {
                                ifactive = (colors >= 8);
                            }
                        }
                    }
                }
                if (1 == ifactive) ++cond;
            } else {
                ifactive = 2;                   // inactive branch
            }
            dprintf("%s: COND/if     (%d) %*s <%s> == %d\n", label, iflevel, iflevel*4, "", value, ifactive);
            break;

        case "elseif":          // elseif <condition>
            if (0 == ifactive) {
                if (length_of_list(parts) >= 2) {
                    if (strstr(value, "&background")) {
                        if (strstr(value, "==")) {
                            if (strstr(value, "&background")) {
                                ifactive = (0 != strstr(value, base));
                            }

                        } else if (strstr(value, "!=")) {
                            if (strstr(value, "&background")) {
                                ifactive = (0 == strstr(value, base));
                            }
                        }

                    } else if (strstr(value, "&t_Co")) {
                        if (strstr(value, "==")) {
                            ifactive = (0 == strstr(value, "" + colors));

                        } else if (strstr(value, ">=")) {
                            if (strstr(value, "256")) {
                                ifactive = (colors >= 256);
                            } else if (strstr(value, "88")) {
                                ifactive = (colors >= 88);
                            } else if (strstr(value, "16")) {
                                ifactive = (colors >= 16);
                            } else if (strstr(value, "8")) {
                                ifactive = (colors >= 8);
                            }
                        }
                    }
                }
            } else {
                ifactive = 2;                   // inactive branch
            }
            dprintf("%s: COND/elseif (%d) %*s <%s> == %d\n", label, iflevel, iflevel*4, "", value, ifactive);
            break;

        case "else":            // else
            if (0 == ifactive) {
                ++ifactive;                     // default condition
            }
            dprintf("%s: COND/else   (%d) %*s <%s> == %d\n", label, iflevel, iflevel*4, "", value, ifactive);
            break;

        case "endif":           // endif
            if (iflevel >= 0) {                 // pop level
                ifactive = ifstack[ iflevel ];
            } else {
                ifactive = 1;
            }
            dprintf("%s: COND/endif  (%d) %*s <%s> == %d\n", label, iflevel, iflevel*4, "", value, ifactive);
            if (iflevel >= 0) iflevel--;
            break;

        default:                // unknown
            break;
        }
    }

    if (done > 0) {
        if (0 == asgui) gui = 0;
        if (colors > 0) {
            message("%s:%d(%d): gui=%d, links=%d, ignored=%d, cond=%d", label, colors, done, gui, links, ignored, cond);
        } else {
            message("%s(%d): gui=%d, links=%d, ignored=%d, cond=%d", label, done, gui, links, ignored, cond);
        }
    }
    return done;
}


/*  Function:           vim_groupmap
 *      Setup the VIM style color links.
 *
 *  Parameters:
 *      name - Groupname to be initialised, otherwise "". Note that highlight group names are not
 *              case sensitive.  "String" and "string" can be used for the same group.
 *
 *  Note:
 *      The names marked with '*' below are the preferred groups; the others are minor groups.
 *
 *  Returns:
 *      nothing
 */
void
vim_groupmap(string name)
{
    // Constant [constant]
    //     *Constant	    any constant
    //      String	    a string constant: "this is a string"
    //      Character	    a character constant: 'c', '\n'
    //      Number	    a number constant: 234, 0xff
    //      Boolean	    a boolean constant: TRUE, false
    //      Float	    a floating point constant: 2.3e10
    //
    if (0 == strlen(name) || 0 == strcasecmp(name, "Constant")) {
        const string link = "=clear:link@constant";
        set_color("string"   +link);
        set_color("character"+link);
        set_color("number"   +link);
        set_color("boolean"  +link);
        set_color("float"    +link);
    }

    // Identifier [n/a]
    //      *Identifier     any variable name
    //       Function	    function name (also: methods for classes)
    //
    if (0 == strlen(name) || 0 == strcasecmp(name, "Identifier")) {
        set_color("word=clear:link@normal");
        set_color("keyword_function=clear:link@normal");
    }

    // Statement [code]
    //      *Statement      any statement
    //       Conditional    if, then, else, endif, switch, etc.
    //       Repeat         for, do, while, etc.
    //       Label          case, default, etc.
    //       Operator       "sizeof", "+", "*", etc.
    //       Keyword        any other keyword
    //       Exception      try, catch, throw
    //
    if (0 == strlen(name) || 0 == strcasecmp(name, "Statement")) {
        const string link = "=clear:link@code";
        set_color("keyword"            +link);
        set_color("keyword_extension"  +link);
        set_color("keyword_definition" +link);
        set_color("keyword_conditional"+link);
        set_color("keyword_repeat"     +link);
        set_color("keyword_exception"  +link);
        set_color("keyword_label"      +link);
    }

    //  PreProc [preprocessor]
    //      *PreProc        generic Preprocessor
    //       Include        preprocessor #include
    //       Define         preprocessor #define
    //       Macro          same as Define
    //       PreCondit      preprocessor #if, #else, #endif, etc.
    //
    if (0 == strlen(name) || 0 == strcasecmp(name, "PreProc")) {
        const string link = "=clear:link@preprocessor";
        set_color("preprocessor_include"    +link);
        set_color("preprocessor_define"     +link);
        set_color("preprocessor_keyword"    +link);
        set_color("preprocessor_conditional"+link);
    }

    // Type [storageclass]
    //      *Type           int, long, char, etc.
    //      StorageClass    static, register, volatile, etc.
    //      Structure       struct, union, enum, etc.
    //      Typedef         A typedef
    //
    if (0 == strlen(name) || 0 == strcasecmp(name, "Type")) {
        const string link = "=clear:link@keyword_type";
        set_color("keyword_storageclass"    +link);
        set_color("keyword_structure"       +link);
        set_color("keyword_typedef"         +link);
    }

    // Special [standout]
    //     *Special         any special symbol
    //      SpecialChar     special character in a constant
    //      Tag             you can use CTRL-] on this
    //      Delimiter       character that needs attention
    //      SpecialComment  special things inside a comment
    //      Debug           debugging statements
    //
    if (0 == strlen(name) || 0 == strcasecmp(name, "Special")) {
        const string link = "=clear:link@standout";
        set_color("constant_standout"       +link);
        set_color("tag"                     +link);
        set_color("delimiter"               +link);
        set_color("comment_standout"        +link);
        set_color("keyword_debug"           +link);
    }
}


/*  Function:           vim_attrmap
 *      Map a VIM style hi[ghlight] attribute to a GRIEF attribute.
 *
 *      Many of the attributes are simple one-to-one mappings. Unsupported are simply ignored.
 *
 *  Parameters:
 *      attr - VIM attribute name.
 *
 *  Returns:
 *      Attribute name, otherwise an empty string.
 */
string
vim_attrmap(string attr)
{
    int idx;

    if ((idx = re_search(SF_NOT_REGEXP|SF_IGNORE_CASE, "=" + attr + "=", scheme_colors)) >= 0) {
        return scheme_colors[idx + 1];
    }
    return "";
}
/*eof*/
