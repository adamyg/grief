/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: colors.cr,v 1.25 2024/10/07 16:14:59 cvsuser Exp $
 * Enhanced colour/colorscheme support.
 *
 *
 */

#include "grief.h"
#include "colorsvim.h"

static list             coloriser_list(void);
static list             vimcoloriser_list(void);

static int              schemeview(int ids);
static int              schemevim(~ string base, ~int flags);
static int              schemeload(string scheme, ~list args, ~string base, ~int flags);

static void             ca_keys(void);
static void             ca_attribute(void);
static void             ca_dialoginit(void);
static string           ca_type(string value);
static void             ca_setfg(string type, int set);
static void             ca_setbg(string type, int set);
static int              ca_callback(int ident, string name, int p1, int p2);

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

static string           scheme_background = "default";

static list             vim_paths = {           // TODO, interface
    "~/.vim/colors/",
    "/usr/share/vim/vimcurrent/colors/"
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
//  "underdouble",
//  "underdotted",
//  "underdashed",
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
    autoload("colorsvim",
        "vim_colorscript",
        "vim_colorscheme",
        "vim_condition_tests"
        );
}


/*  Function:           coloriser
 *      Coloriser loader.
 *
 *  Parameters:
 *      scheme - Optional color scheme name.
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
    string name;
    get_term_feature(TF_COLORSCHEME, name);
    return name;
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
 *      scheme - Color scheme name.
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
        schemeview(FALSE);
        return FALSE;

    } else if ("--ids" == scheme) {
        schemeview(TRUE);
        return FALSE;

    } else if ("--vim" == scheme) {
        return schemevim();

    } else if ("--vim-gui" == scheme) {
        return schemevim(NULL, SCHEME_GUIONLY);

    } else if ("--vim-cterm" == scheme) {
        return schemevim(NULL, SCHEME_CTERMONLY);

    } else if ("--vim=dark" == scheme) {
        return schemevim("dark");

    } else if ("--vim=light" == scheme) {
        return schemevim("light");
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
schemeview(int ids)
{
    string title, coloriser = inq_coloriser();
    int buf, idx;

    save_position();
    sprintf(title, "Color Scheme [%s]", (coloriser ? coloriser : "none"));

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

    // flags,attribute=foreground[,background][: [link@name|sticky@name|styles][, ...]]
    move_abs(NULL, 81);
    string spec = trim(read());
    move_abs(NULL, 1);
    if (0 == strlen(spec)) {
        return;
    }

    // clear dialog
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

    // initialise dialog
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
schemevim(~ string base, ~int flags)
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
// Load the specific color scheme.
//
static int
schemeload(string scheme, ~list args, ~string base, ~int flags)
{
    //  Scheme, examples:
    //
    //   o colorscheme-package:
    //
    //      o sunset-light
    //            ==> colors/sunset/cscheme
    //                  colorschemepkg_sunset("light" ..)
    //
    //      o sunset
    //            ==> colors/sunset/cscheme
    //                  colorschemepkg_sunset(NULL, ..)
    //
    //   o colorscheme:
    //
    //      o sunset-light
    //            ==> colors/sunset
    //                  colorscheme_sunset_light
    //
    //      o sunset-light-lowcontrast
    //            ==> colors/sunset
    //                  colorscheme_sunset_light_lowcontrast
    //
    //      o sunset
    //            ==> colors/sunset
    //                  colorscheme_sunset
    //
    scheme = re_translate(SF_GLOBAL, "[ \t]+", "_", scheme);
    list components = split(scheme, "-", NULL, NULL, NULL, 2);

    // colorscheme-package
    if (require("colors/" + components[0] + "/cscheme") >= 0) {
        string name = "colorschemepkg_" + components[0];
        int ret = -1;

        if (inq_macro(name) > 0) {
            if (length_of_list(components) == 2) {
                if (! is_null(args)) {
                    ret = execute_macro(name, components[1], args);
                } else {
                    ret = execute_macro(name, components[1]);
                }
            } else {
                if (! is_null(args)) {
                    ret = execute_macro(name, NULL, args);
                } else {
                    ret = execute_macro(name);
                }
            }

        } else {
            error("colorschemepkg: '%s', not available", scheme);
        }

        if (ret == -1)
            return FALSE;
        set_term_feature(TF_COLORSCHEME, scheme);
        message("colorscheme: '%s'", scheme);
        return TRUE;
    }

    // colorscheme
    if (require("colors/" + components[0]) >= 0) {
        string name = "colorscheme_" + re_translate(SF_GLOBAL, "-", "_", scheme);
        int ret = -1;

        if (inq_macro(name) > 0) {
            if (! is_null(args)) {
                ret = execute_macro(name, args);
            } else {
                ret = execute_macro(name);
            }
        } else {
            error("colorscheme: '%s', not available", scheme);
            return FALSE;
        }

        if (ret == -1)
            return FALSE;
        set_term_feature(TF_COLORSCHEME, scheme);
        message("colorscheme: '%s'", scheme);
        return TRUE;
    }

    //  Import a vim colorscheme/
    //      use with caution -- limited vim script support.
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
                vim_colorscript(scheme, base, flags, path + file + ".vim");
                list_reset(vim_paths);
                return TRUE;
            }
        }
    }

    message("colorscheme: unavailable to locate '%s'", scheme);
    return FALSE;
}

/*end*/
