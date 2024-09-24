/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: colorsvim.cr,v 1.5 2024/09/24 16:39:18 cvsuser Exp $
 * Enhanced colour/colorscheme support.
 *
 *
 */

#include "grief.h"
#include "colorsvim.h"

static void             groupclear(string name);
static void             syntaxreset(void);
static string           attrmap(string attr);

static list             scheme_colors = {
    //
    //  VIM to GRIEF attribute (where possible)
    //
    //      default highlighting groups.
    //
    //  Reference:      https://vimhelp.org/syntax.txt.html#highlight-groups
    //  Online editor:  https://sputnick.fr/scripts/vim-color.html
    //
    "=ColorColumn=",    "ruler_column",         // Used for the columns set with 'colorcolumn'.  (ie. colorcolumn=80)

    "=Conceal=",        "",                     // Place-holder characters substituted for concealed text.

    "=Cursor=",         "cursor",               // Character under the cursor.
    "=lCursor=",        "",                     // Character under the cursor when language-mapping is used.
    "=CursorIM=",       "",                     // Like Cursor, but used when in IME mode.
    "=CursorColumn=",   "cursor_col",           // Screen column that the cursor is in when 'cursorcolumn' is set.
    "=CursorLine=",     "cursor_row",           // Screen line that the cursor is in when 'cursorline' is set.

    "=Directory=",      "lsdirectory",          // Directory names (and other special names in listings).

    "=DiffAdd=",        "additional",           // Diff: added line.
    "=DiffChange=",     "modified",             // Diff: changed line.
    "=DiffDelete=",     "diffdelete",           // Diff: deleted line.
    "=DiffText=",       "difftext",             // Diff: changed text within a changed line.

    "=ModeMsg=",        "",                     // 'showmode' message.
    "=MoreMsg=",        "message",              // |more-prompt|.
    "=WarningMsg=",     "",                     // warning messages.
    "=ErrorMsg=",       "error",                // error messages on the command line.

    "=MessageWindow=",  "",                     // Messages popup window.

    "=VertSplit=",      "frame",                // the column separating vertically split windows.

    "=Folded=",         "",                     // Fold: line used for closed folds.
    "=FoldColumn=",     "",                     // Fold: column.
    "=CursorLineFold=", "",                     // Fole: like FoldColumn when 'cursorline' is set for the cursor line.

    "=SignColumn=",     "column_status",        // column where *signs* are displayed.
    "=CursorLineSign=", "",                     // Like SignColumn when 'cursorline' is set for the cursor line.
    "=LineNr=",         "column_lineno",        // Line number.
    "=LineNrAbove=",    "",                     // Line number for when the 'relativenumber' option is set, above the cursor line.
    "=LineNrBelow",     "",                     // Line number for when the 'relativenumber' option is set, below the cursor line.

    "=IncSearch=",      "search_inc",           // 'incsearch' highlighting; also used for the text replaced with.
    "=MatchParen=",     "search_match",         // a paired character (matching parenthesis).

    "=EndOfBuffer=",    "",                     // Filler lines (~) after the last line in the buffer.
    "=NonText=",        "nonbuffer",            // '~' and '@' at the end of the window.

    "=Normal=",         "normal",               // normal text.

    "=Pmenu=",          "popup_normal",         // Popup: normal item.
    "=PmenuKind=",      "",                     // Popup: Normal item "kind".
    "=PmenuKindSel=",   "",                     // Popup: Selected item "kind".
    "=PmenuExtra=",     "",                     // Popup: Normal item "extra text".
    "=PmenuExtraSel=",  "",                     // Popup: selected item "extra text".
    "=PmenuSel=",       "popup_hilite",         // Popup: elected item.
    "=PmenuSbar=",      "scrollbar",            // Popup: scrollbar.
    "=PmenuThumb=",     "scrollbar_thumb",      // Popup: thumb of the scrollbar.
    "=PmenuMatch=",     "",                     // Popup: Matched text in normal item.
    "=PmenuMatchSel=",  "",                     // Popup: Matched text in selected item.

    "=PopupNotification=", "",                  // Popup window created with popup_notification().

    "=QuickFixLine=",   "",                     // Current quickfix item in the quickfix window.

    "=Question=",       "prompt",               // prompt and yes/no questions.

    "=Search=",         "search",               // Last search pattern highlighting.
    "=CurSearch=",      "",                     // Current match for the last search pattern.

    "=SpecialKey=",     "",                     // Meta and special keys listed with ":map", also unprintable characters.

    "=Todo=",           "todo",                 // Anything that needs extra attention; mostly the keywords TODO FIXME and XXX.

    "=SpellBad=",       "spell",                // Spell: recognized word.
    "=SpellCap=",       "",                     // Spell: word that should start with a capital.
    "=SpellLocal=",     "",                     // Spell: recognized word as one that is used in another region.
    "=SpellRare=",      "",                     // Spell: recognized word as one that is hardly ever used.
    "=Spell=",          "spell",

    "=StatusLine=",     "echo_line",            // Status line of current window.
    "=StatusLineNC=",   "",                     // Status lines of not-current windows.
    "=StatusLineTerm=", "",                     // Status line of current window, if it is a terminal window.
    "=StatusLineTermNC=", "",                   // Status lines of not-current windows that is a terminal window.

    "=Terminal=",       "",                     // Terminal window (see terminal-size-color).

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
    "=Identifier=",     "keyword_definition",   // any variable name.
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
    "=Operator=",       "operator",             //  operators [non-standard]
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

    "=Underlined=",     "link",                 // text that stands out.

    "=Ignore=",         "",                     // left blank, hidden.

    "=Error=",          "alert",                // any erroneous construct.

    //  misc/
    //
    "=Tab=",            "whitespace",
    "=Space=",          "",
    };


/*  Function:           vim_colorscript
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
 *      scheme - Scheme name.
 *      base - "light" or "dark".
 *      flags - Color flags.
 *      file - Scheme source script.
 *
 *  Returns:
 *      nothing
 */

static int condition(list parts);

void
vim_colorscript(string scheme, string base, int flags, string file)
{
    int buf, curbuf = inq_buffer();

    //
    //  Load vim-script.
    //
    message("loading: %s", file);
    if ((buf = create_buffer("-colorscheme-vim-", file, TRUE)) < 0) {
        return;
    }

    //
    //  Parse conditional blocks/comments.
    //
    //      best effort load logic, with weak condition expression evaluation (TODO/FIXME).
    //
    int colordepth, iflevel = -1, ifactive = 1;
    list ifstack, spec;
    string ln;

    get_term_feature(TF_COLORDEPTH, colordepth);
    if (SCHEME_GUIONLY & flags) {
        if (colordepth < 256) {
            message("%s: GUI color depth not available", scheme);
            return;
        }

    } else if (SCHEME_CTERMONLY & flags) {
        if (colordepth > 256) {
            colordepth = 256;                   // limit
        }
    }

    if (base) {                                 // dark or light base
        base = lower(base);
    } else {
        int isdark;
        get_term_feature(TF_SCHEMEDARK, isdark);
        base = (isdark ? "dark" : "light");
    }

    set_buffer(buf);
    top_of_buffer();
    do {
        ln = compress(read(), TRUE);            // line, compress white-space and trimmed.

        // comments
        if ('"' == characterat(ln, 1)) {
            continue;
        }

        // set or let and conditionals
        if (re_search(0, "^{if}|{else}|{endif}", ln) > 0) {
            const list parts =
                tokenize(ln, " ", TOK_DOUBLE_QUOTES|TOK_PRESERVE_QUOTES|TOK_WHITESPACE|TOK_COLLAPSE_MULTIPLE|TOK_TRIM);

            switch (parts[0]) {
            case "if":      // if <condition>
                ifstack[ ++iflevel ] = ifactive;

                if (1 == ifactive) {
                    ifactive = condition(parts);
                } else {
                    ifactive = 2;               // inactive branch
                }
                break;

            case "elseif":  // elseif <condition>
                if (0 == ifactive) {
                    ifactive = condition(parts);
                } else {
                    ifactive = 2;               // inactive branch
                }
                break;

            case "else":    // else
                if (0 == ifactive) {
                    ++ifactive;                 // default condition
                }
                break;

            case "endif":   // endif
                if (iflevel >= 0) {             // pop level
                    ifactive = ifstack[ iflevel ];
                } else {
                    ifactive = 1;
                }
                if (iflevel >= 0) {
                    iflevel--;
                }
                break;
            }

            dprintf("vim: COND/%5s     (%d) %*s <%s> == %d\n", parts[0], iflevel, iflevel*4, "", ln, ifactive);

        } else {
            if (ifactive) {
                dprintf("vim: COND/line   (%d) %*s <%s> == %d\n", iflevel, iflevel*4, "", ln, ifactive);
                spec == ln;
            }
        }

    } while (down());

    set_buffer(curbuf);
    delete_buffer(buf);

    //
    //  Parse hightlights
    //
    vim_colorscheme(scheme, -1, base, spec, -1);
}


static int
condition(list parts)
{
    //
    //  Conditions:
    //
    //      if &background == "dark"
    //      if &background ==# 'dark'
    //      if &background == "light"
    //      if &background ==# 'light'
    //
    //      if &t_Co >= 8, 16, 88, 256
    //      if &t_Co == 8, 16, 88, 256
    //
    //      if has("gui_running") || &t_Co == 88 || &t_Co == 256
    //
    //      if exists("syntax_on")
    //      endif
    //
    //      if version > xxx
    //      endif
    //
    //  Notes:
    //  o Parser is very crude, logical AND/OR ignored.
    //  o Other color condition expressions has been sighted yet the above represent 90% of all cases.
    //
    extern string base;                         // "light" or "dark"
    extern int colordepth;

    string word, lhs, op;
    int ret = 0;

    while (list_each(parts, word) >= 0 && 0 == ret) {

        // left-hand side
        if (characterat(word, 1) == '&') {
            lhs = word;                         // variable
            op = "";
            continue;

        } else if (word == "||" || word == "&&") {
            lhs = "";                           // logical operators
            continue;

        } else if (strstr(word, "has(")) {      // has(xxx) function
            if (strstr(word, "gui_running")) {
                if (colordepth > 256) {
                    ret = 1;
                    break;
                }
            }
            lhs = "";
            continue;

        } else if (word == "exists(\"syntax_on\")") {
            ret = 1;                            // enable "syntax reset"
            break;

        } else if (word == "version") {
            ret = 1;                            // enable version'ed code; unsupported feature are ignored.
            break;

        } else if (lhs) {
            if (!op) {
                op = word;                      // operators
                continue;
            }
        } else {
            op = "";
            continue;
        }

        // comparisions
        switch (op) {
        case "==":      // equals
            if (lhs == "&background") {         // word contains "base"
                ret = (0 != strstr(word, base));

            } else if (lhs == "&t_Co") {
                if (strstr(word, "256")) {
                    ret = (colordepth == 256);
                } else if (strstr(word, "88")) {
                    ret = (colordepth == 88);
                } else if (strstr(word, "16")) {
                    ret = (colordepth == 16);
                } else if (strstr(word, "8")) {
                    ret = (colordepth == 8);
                }
            }
            break;
        case "==#":     // strcasecmp
            if (lhs == "&background") {         // word contains "base"
                ret = (0 != strstr(lower(word), lower(base)));
            }
            break;
        case "!=":      // non-equals
            if (lhs == "&background") {         // word doesnt-contains "base"
                ret = (0 == strstr(word, base));

            } else if (lhs == "&t_Co") {
                if (strstr(word, "256")) {
                    ret = (colordepth != 256);
                } else if (strstr(word, "88")) {
                    ret = (colordepth != 88);
                } else if (strstr(word, "16")) {
                    ret = (colordepth != 16);
                } else if (strstr(word, "8")) {
                    ret = (colordepth != 8);
                }
            }
            break;
        case ">=":      // greater-then-or-equals
            if (lhs == "&t_Co") {
                if (strstr(word, "256")) {
                    ret = (colordepth >= 256);
                } else if (strstr(word, "88")) {
                    ret = (colordepth >= 88);
                } else if (strstr(word, "16")) {
                    ret = (colordepth >= 16);
                } else if (strstr(word, "8")) {
                    ret = (colordepth >= 8);
                }
            }
            break;
        }
        dprintf("vim: ==> %s %s %s := %d\n", lhs, op, word, ret);
    }
    list_reset(parts);
    return ret;
}


/*  Function:           vim_colorscheme
 *      VIM colorscheme.
 *
 *  Parameters:
 *      label - Colorscheme name.
 *      colors - Supported colors (8, 16, 88 or 256), otherwise -1.
 *      base - Optional base color scheme "dark" or "light"; current if omitted.
 *      spec - highlight command list.
 *      asgui - TRUE utilise gui colors, FALSE terminal, otherwise -1 dynamic.
 *
 *  Returns:
 *      Imported color count.
 */

int
vim_colorscheme(string label, int colors, ~string base, list spec, int asgui)
{
    string background;                          // scheme background.
    int colordepth, done, links, ignored;
    declare value;

    // verify colors against current display environment
    get_term_feature(TF_COLORDEPTH, colordepth);
    if (colors > 0) {
        if (colordepth != colors) {
            message("%s: color depth %d not available (%d)", label, colors, colordepth);
            return -1;
        }
    }

    if (-1 == asgui) {                          // dynamic
        asgui = (colordepth > 256 ? TRUE : FALSE);
    }

    if (0 == strlen(base)) {                    // dark or light base
        int isdark;
        get_term_feature(TF_SCHEMEDARK, isdark);
        base = (isdark ? "dark" : "light");
    }

    set_term_feature(TF_COLORSCHEME, label);
    background = base;

    // load schema
    while (list_each(spec, value) >= 0) {
        const list parts =
            tokenize(value, " ", TOK_DOUBLE_QUOTES|TOK_PRESERVE_QUOTES|TOK_WHITESPACE|TOK_COLLAPSE_MULTIPLE|TOK_TRIM);

        switch (parts[0]) {
        case "hi":
        case "hi!":
        case "highlight":
        case "highlight!":       // hi[ghtlight] command
            if (length_of_list(parts) >= 2) {
                if ("clear" == parts[1]) {
                    //
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

                    if (0 == strlen(groupname)) // scheme=dark|light|default
                        set_color("scheme=" + background);

                    groupclear(groupname);

                } else if ("default" == parts[1] && length_of_list(parts) >= 3 && "link" == parts[2]) {
                    //
                    //  [default] link {from-group} {to-group}
                    //      Setup an attribute link.
                    //
                    //  [default] link {from-group} NONE
                    //      Remove an attribute link.
                    //
                    string link;

                    if (5 == length_of_list(parts)) {
                        string id1 = attrmap(parts[3]), id2 = attrmap(parts[4]);

                        if (strlen(id1) && strlen(id2)) {
                            sprintf(link, "%s=none:sticky@%s", id1, id2);
                            set_color(link);
                            ++links;
                        }
                    }
                    if (!link) ++ignored;

                } else if ("link" == parts[1]) {
                    string link;

                    if (4 == length_of_list(parts)) {
                        string id1 = attrmap(parts[2]), id2 = attrmap(parts[3]);

                        if (strlen(id1) && strlen(id2)) {
                            sprintf(link, "%s=none:link@%s", id1, id2);
                            set_color(link);
                            ++links;
                        }
                    }
                    if (!link) ++ignored;

                } else {
                    //
                    //  [default] {group-name} key=arg
                    //      State an attributes color specification.
                    //
                    int baseidx = ("default" == parts[1] ? 2 : 1);
                    string fg, bg, sf, ident = attrmap(parts[baseidx]);
                    int i;

                    if (ident) {
                        for (i = baseidx + 1; i < length_of_list(parts); ++i) {
                            const list parts2 =
                                    tokenize(parts[i], "=", TOK_DOUBLE_QUOTES|TOK_WHITESPACE|TOK_TRIM);

                            if (length_of_list(parts2) >= 2) {
                                string val = parts2[1];

                                if (0 == asgui) {
                                    //  cterm={attr-list}
                                    //      Color termination attribute list.
                                    //      One or more of the following comma seperated values (without spaces).
                                    //
                                    //          bold, underline, undercurl, none, reverse, inverse, italic, standout
                                    //
                                    //  ctermfg={color}
                                    //      Color terminal foreground color.
                                    //
                                    //  ctermbg={color}
                                    //      Color terminal background color.
                                    //
                                    switch (parts2[0]) {
                                    case "cterm":   sf = val; break;
                                    case "ctermfg": fg = val; break;
                                    case "ctermbg": bg = val; break;
                                    }

                                } else {
                                    //  gui={attr-list}
                                    //      Color termination attribute list.
                                    //      One or more of the following comma seperated values (without spaces).
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
                                    switch (parts2[0]) {
                                    case "gui":     sf = val; break;
                                    case "guifg":   fg = val; break;
                                    case "guibg":   bg = val; break;
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
                                    bg = "bg";                  // current background (TODO: NONE=transparent)
                                }

                                if ("frame" == ident)
                                    sf = "";                    // ignore underline etc

                                set_color_pair(ident, fg, bg, sf);
                                break;
                            }
                            ++done;
                        }

                    } else {
                        dprintf("%s: attribute: %s=unknown", label, parts[baseidx]);
                        ++ignored;
                    }
                }
            }
            break;

        case "let":
        case "set":              // set command
            //
            //  set bg=[dark|light]/
            //  set background=[dark|light]/
            //      When set to "dark", use colors that look good on a dark background.
            //      When set to "light", try to use colors that look good on a light background.
            //      Any other value is illegal.
            //
            //  let g:colors_name = 'duckbones'
            //      Scheme name.
            //
            if (length_of_list(parts) >= 2) {
                const list parts2 =
                    tokenize(parts[1], "=", TOK_DOUBLE_QUOTES|TOK_WHITESPACE|TOK_TRIM);

                if (length_of_list(parts2) >= 2) {
                    if ("bg" == parts2[0] || "background" == parts2[0]) {
                        const string arg = parts2[1];

                        if (0 == strcasecmp("dark", arg) || 0 == strcasecmp("light", arg) || 0 == strcasecmp("default", arg)) {
                            background = arg;
                        }
                    }
                }
            }
            break;

        case "syntax":          // syntax command
            //
            //  syntax reset
            //      Resets the colors for:
            //
            //          Comment, Constant, Special, Identifier, Statement, PreProc, Type, Underline, Ignore
            //
            //      according to the selected background (dark or light).
            //      Links that point to these colors (String, Number, etc.) are also reset.
            //
            if (length_of_list(parts) >= 2) {
                if (parts[1] == "reset") {
                    syntaxreset();
                }
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

        default:                // unknown
            dprintf("%s: attribute: %s=unknown", label, value);
            ++ignored;
            break;
        }
    }

    if (done > 0) {
        if (colors > 0) {
            message("%s:%d(%d): links=%d, ignored=%d", label, colors, done, links, ignored);
        } else {
            message("%s(%d): links=%d, ignored=%d", label, done, links, ignored);
        }
    }
    return done;
}


/*  Function:           groupclear
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
static void
groupclear(string name)
{
    const int all = (0 == strlen(name));

    // Constant [constant]
    //     *Constant        any constant
    //      String          a string constant: "this is a string"
    //      Character       a character constant: 'c', '\n'
    //      Number          a number constant: 234, 0xff
    //      Boolean         a boolean constant: TRUE, false
    //      Float           a floating point constant: 2.3e10
    //
    if (all || 0 == strcasecmp(name, "Constant")) {
        const string link = "=clear:link@constant";
        set_color("string" + link);
        set_color("character" + link);
        set_color("number" + link);
        set_color("boolean" + link);
        set_color("float" + link);
    }

    // Identifier [n/a]
    //      *Identifier     any variable name
    //       Function       function name (also: methods for classes)
    //
    if (all || 0 == strcasecmp(name, "Identifier")) {
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
    if (all || 0 == strcasecmp(name, "Statement")) {
        const string link = "=clear:link@code";
        set_color("keyword" + link);
        set_color("keyword_extension" + link);
        set_color("keyword_definition" + link);
        set_color("keyword_conditional" + link);
        set_color("keyword_repeat" + link);
        set_color("keyword_exception" + link);
        set_color("keyword_label" + link);
    }

    //  PreProc [preprocessor]
    //      *PreProc        generic Preprocessor
    //       Include        preprocessor #include
    //       Define         preprocessor #define
    //       Macro          same as Define
    //       PreCondit      preprocessor #if, #else, #endif, etc.
    //
    if (all || 0 == strcasecmp(name, "PreProc")) {
        const string link = "=clear:link@preprocessor";
        set_color("preprocessor_include" + link);
        set_color("preprocessor_define" + link);
           set_color("preprocessor_keyword" + link);
        set_color("preprocessor_conditional" + link);
    }

    // Type [storageclass]
    //      *Type           int, long, char, etc.
    //      StorageClass    static, register, volatile, etc.
    //      Structure       struct, union, enum, etc.
    //      Typedef         A typedef
    //
    if (all || 0 == strcasecmp(name, "Type")) {
        const string link = "=clear:link@keyword_type";
        set_color("keyword_storageclass" + link);
        set_color("keyword_structure" + link);
        set_color("keyword_typedef" + link);
    }

    // Special [standout]
    //     *Special         any special symbol
    //      SpecialChar     special character in a constant
    //      Tag             you can use CTRL-] on this
    //      Delimiter       character that needs attention
    //      SpecialComment  special things inside a comment
    //      Debug           debugging statements
    //
    if (all || 0 == strcasecmp(name, "Special")) {
        const string link = "=clear:link@standout";
        set_color("constant_standout" + link);
        set_color("tag" + link);
        set_color("delimiter" + link);
        set_color("comment_standout" + link);
        set_color("keyword_debug" + link);
    }
}


static void
syntaxlink(string from, string to)
{
    from = attrmap(from);
    if (from) {
        to = attrmap(to);
        if (to) {
            set_color(from + "=clear:link@" + to);
        }
    }
}


static void
syntaxreset(void)
{
    // TODO: review
    syntaxlink("Boolean", "Constant");
    syntaxlink("Character", "Constant");
    syntaxlink("Conditional", "Statement");
    syntaxlink("Debug", "Special");
    syntaxlink("Define", "PreProc");
    syntaxlink("Delimiter", "Special");
    syntaxlink("Exception", "Statement");
    syntaxlink("Float", "Constant");
  //syntaxlink("Function", "Identifier");
    syntaxlink("Include", "PreProc");
    syntaxlink("Keyword", "Statement");
    syntaxlink("Label", "Statement");
    syntaxlink("Macro", "PreProc");
    syntaxlink("Number", "Constant");
    syntaxlink("Operator", "Statement");
    syntaxlink("PreCondit", "PreProc");
    syntaxlink("Repeat", "Statement");
    syntaxlink("SpecialChar", "Special");
    syntaxlink("SpecialComment", "Special");
    syntaxlink("StorageClass", "Type");
    syntaxlink("String", "Constant");
    syntaxlink("Structure", "Type");
    syntaxlink("Tag", "Special");
    syntaxlink("Typedef", "Type");
}


/*  Function:           attrmap
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
attrmap(string attr)
{
    int idx;

    if ((idx = re_search(SF_NOT_REGEXP|SF_IGNORE_CASE, "=" + attr + "=", scheme_colors)) >= 0) {
        return scheme_colors[idx + 1];
    }
    return "";
}


/*
 * Basic unit-testing -----
 *
 */

static void
condtest(string line, string base, int colordepth, int ret)
{
    const list parts =
        tokenize(line, " ", TOK_DOUBLE_QUOTES|TOK_PRESERVE_QUOTES|TOK_WHITESPACE|TOK_COLLAPSE_MULTIPLE|TOK_TRIM);

    int t_ret = condition(parts);
    dprintf("vimtest: [%s] %s/%s <%s> := %d", (t_ret == ret ? "OK" : "BAD"), base, colordepth, line);
}


void
vim_condition_tests()
{
    condtest("if &background == \"dark\"",   "dark", -1, 1);
    condtest("if &background ==# 'dark'",    "DARK", -1, 1);
    condtest("if &background == \"dark\"",   "light", -1, 0);
    condtest("if &background ==# 'dark'",    "light", -1, 0);

    condtest("if &background == \"light\"",  "light", -1, 1);
    condtest("if &background ==# 'light'",   "Light", -1, 1);
    condtest("if &background == \"light\"",  "dark", -1, 0);
    condtest("if &background ==# 'light'",   "dark", -1, 0);

    condtest("if has(\"gui_running\")", "", 256, 0);
    condtest("if has(\"gui_running\")", "", 1024, 1);

    condtest("if exists(\"syntax_on\")", "", -1, 1);

    condtest("if version > 700", "", -1, 1);

    condtest("if &t_Co == 16, 88, 256", "", 8, 0);
    condtest("if &t_Co == 16, 88, 256", "", 16, 1);
    condtest("if &t_Co == 16, 88, 256", "", 88, 1);
    condtest("if &t_Co == 16, 88, 256", "", 256, 1);

    condtest("if &t_Co >= 16, 88, 256", "", 15, 0);
    condtest("if &t_Co >= 16, 88, 256", "", 16, 1);
    condtest("if &t_Co >= 16, 88, 256", "", 17, 1);
    condtest("if &t_Co >= 16, 88, 256", "", 88, 1);
    condtest("if &t_Co >= 16, 88, 256", "", 89, 1);
    condtest("if &t_Co >= 16, 88, 256", "", 256, 1);
    condtest("if &t_Co >= 16, 88, 256", "", 257, 1);

    condtest("if has(\"gui_running\") || &t_Co == 88 || &t_Co == 256", "", 16, 0);
    condtest("if has(\"gui_running\") || &t_Co == 88 || &t_Co == 256", "", 88, 1);
    condtest("if has(\"gui_running\") || &t_Co == 88 || &t_Co == 256", "", 256, 1);
}

/*eof*/
