/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: gruvbox.cr,v 1.3 2024/07/06 18:03:26 cvsuser Exp $
 * gruvbox coloriser, GRIEF port.
 *
 * Original:
 *  Name: gruvbox.vim
 *  Description: Retro groove color scheme for Vim
 *  Author: morhetz <morhetz@gmail.com>
 *  Source: https://github.com/morhetz/gruvbox
 *  License: MIT/X11
 *  Last Modified: 10 Nov 2014
 *
 * Description:
 *  Retro groove color scheme for Vim.
 *
 *  Gruvbox is heavily inspired by badwolf, jellybeans and solarized.
 *
 *  Designed as a bright theme with pastel 'retro groove' colors and light/dark mode
 *  switching in the way of solarized. The main focus when developing Gruvbox is to keep
 *  colors easily distinguishable, contrast enough and still pleasant for the eyes.
 *
 */

#include "../grief.h"

static int              gruvbox_bold=1;
static int              gruvbox_italic=1;
static int              gruvbox_undercurl=1;
static int              gruvbox_underline=1;
static int              gruvbox_italicize_comments=1;
static int              gruvbox_italicize_strings=0;
static int              gruvbox_termcolors=256;
static int              gruvbox_invert_indent_guides=0;
static string           gruvbox_hls_cursor="orange";
static string           gruvbox_sign_column="dark1";
static string           gruvbox_color_column="dark1";
static string           gruvbox_vert_split="dark2";
static int              gruvbox_invert_signs=0;
static int              gruvbox_invert_selection=1;
static string           gruvbox_contrast="medium";
static string           gruvbox_contrast_dark="medium";
static string           gruvbox_contrast_light="medium";
static int              gruvbox_invert_tabline=0;

static string
HL(string group, declare fg, ~declare bg, ~string gui, ~declare sp)
{
        string histring = "hi " + group + " ";

        // fg
        if (is_string(fg)) {
                if (fg == "fg") {
                        histring += "guifg=fg ctermfg=fg ";
                } else if (fg == "bg") {
                        histring += "guifg=bg ctermfg=bg ";
                } else if (fg == "none") {
                        histring += "guifg=NONE ctermfg=NONE ";
                }
        } else if (is_list(fg) && length_of_list(fg)) {
                histring += "guifg=#" + fg[0] + " ctermfg=" + fg[1] + " ";
        }

        // bg
        if (is_string(bg)) {
                if (bg == "bg") {
                        histring += "guibg=bg ctermbg=bg ";
                } else if (is_string(fg) && fg == "fg") {
                        histring += "guibg=fg ctermbg=fg ";
                } else if (bg == "none") {
                        histring += "guibg=NONE ctermbg=NONE ";
                }
        } else if (is_list(bg) && length_of_list(bg)) {
                histring += "guibg=#" + bg[0] + " ctermbg=" + bg[1] + " ";
        } else {
                histring += "guibg=NONE ctermbg=NONE ";
        }

        // Hotfixing #24;
        // TODO: get rid of this spaghetti
        if (is_string(gui) && strlen(gui)) {
                if (gui == "none") {
                        histring += "gui=NONE cterm=NONE ";
                } else if (gui == "italic" && gruvbox_italic == 0) {
                        histring += "gui=NONE cterm=NONE ";
                } else if (gui == "bold" && gruvbox_bold == 0) {
                        histring += "gui=NONE cterm=NONE ";
                } else if (gui == "bold,inverse" && gruvbox_bold == 0) {
                        histring += "gui=inverse cterm=inverse ";
                } else if (gui == "undercurl" && gruvbox_undercurl == 0) {
                        histring += "gui=NONE cterm=NONE ";
                } else if (gui == "underline" && gruvbox_underline == 0) {
                        histring += "gui=NONE cterm=NONE ";
                } else if (gui == "bold,italic") {
                        if (gruvbox_italic == 0 && gruvbox_bold == 0) {
                                histring += "gui=NONE cterm=NONE ";
                        } else if (gruvbox_italic == 0) {
                                histring += "gui=bold cterm=bold ";
                        } else if (gruvbox_bold == 0) {
                                histring += "gui=italic cterm=italic ";
                        } else {
                                histring += "gui=" + gui + " cterm=" + gui + " ";
                        }
                } else if (gui == "bold,underline") {
                        if (gruvbox_underline == 0 && gruvbox_bold == 0) {
                                histring += "gui=NONE cterm=NONE ";
                        } else if (gruvbox_underline == 0) {
                                histring += "gui=bold cterm=bold ";
                        } else if (gruvbox_bold == 0) {
                                histring += "gui=underline cterm=underline ";
                        } else {
                                histring += "gui=" + gui + " cterm=" + gui + " ";
                        }
                } else if (gui == "underline,italic") {
                        if (gruvbox_underline == 0 && gruvbox_italic == 0) {
                                histring += "gui=NONE cterm=NONE ";
                        } else if (gruvbox_underline == 0) {
                                histring += "gui=italic cterm=italic ";
                        } else if (gruvbox_italic == 0) {
                                histring += "gui=underline cterm=underline ";
                        } else {
                                histring += "gui=" + gui + " cterm=" + gui + " ";
                        }
                } else if (gui == "bold,underline,italic") {
                        if (gruvbox_italic == 0 && gruvbox_bold == 0) {
                                histring += "gui=underline cterm=underline ";
                        } else if (gruvbox_italic == 0) {
                                histring += "gui=bold,underline cterm=bold,underline ";
                        } else if (gruvbox_bold == 0) {
                                histring += "gui=italic,underline cterm=italic,underline ";
                        } else {
                                histring += "gui=" + gui + " cterm=" + gui + " ";
                        }
                } else {
                        histring += "gui=" + gui + " cterm=" + gui + " ";
                }
        } else {
                histring += "gui=NONE cterm=NONE ";
        }

        // sp
        if (is_string(sp)) {
                if (sp == "none") {
                        histring += "guisp=NONE ";
                }
        } else if (is_list(sp) && length_of_list(sp)) {
                histring += "guisp=#" + sp[0] + " ";
        }

        return histring;
}


static list
CO(string color)
{
        extern list dark0, dark1, dark2, dark3, dark4,
                medium, light0, light1, light2, light3, light4, light4_256,
                red, green, yellow, blue, purple, aqua, orange;

        switch (color) {
        case "dark0":      return dark0;
        case "dark1":      return dark1;
        case "dark2":      return dark2;
        case "dark3":      return dark3;
        case "dark4":      return dark4;
        case "medium":     return medium;
        case "light0":     return light0;
        case "light1":     return light1;
        case "light2":     return light2;
        case "light3":     return light3;
        case "light4":     return light4;
        case "light4_256": return light4_256;
        case "red":        return red;
        case "green":      return green;
        case "yellow":     return yellow;
        case "blue":       return blue;
        case "purple":     return purple;
        case "aqua":       return aqua;
        case "orange":     return orange;
        }
        return dark0;
}


static void
gruvbox_colorscheme(void)
{
        list colors = {
                "hi clear"
                };
        int colordepth, is_dark;

        get_term_feature(TF_COLORDEPTH, colordepth);
        get_term_feature(TF_SCHEMEDARK, is_dark);
        if (colordepth < 256) {
                error("gruvbox, color depth not supported");
                return;
        }

        // scheme
        list dark0, dark1, dark2, dark3, dark4;
        list medium;
        list light0, light1, light2, light3, light4, light4_256;
        list red, green, yellow, blue, purple, aqua, orange;

        if (is_dark) {
                dark0  = quote_list("282828", 235);             // 40-40-40
                dark1  = quote_list("3c3836", 237);             // 60-56-54
                dark2  = quote_list("504945", 239);             // 80-73-69
                dark3  = quote_list("665c54", 241);             // 102-92-84
                dark4  = quote_list("7c6f64", 243);             // 124-111-100

                medium = quote_list("928374", 245);             // 146-131-116

                light0 = quote_list("fdf4c1", 229);             // 253-244-193
                light1 = quote_list("ebdbb2", 223);             // 235-219-178
                light2 = quote_list("d5c4a1", 250);             // 213-196-161
                light3 = quote_list("bdae93", 248);             // 189-174-147
                light4 = quote_list("a89984", 246);             // 168-153-132

                light4_256
                       = quote_list("a89984", 246);             // 168-153-132

                red    = quote_list("fb4934", 167);             // 251-73-52
                green  = quote_list("b8bb26", 142);             // 184-187-38
                yellow = quote_list("fabd2f", 214);             // 250-189-47
                blue   = quote_list("83a598", 109);             // 131-165-152
                purple = quote_list("d3869b", 175);             // 211-134-155
                aqua   = quote_list("8ec07c", 108);             // 142-192-124
                orange = quote_list("fe8019", 208);             // 254-128-25

                if (gruvbox_termcolors == 16) {
                        dark0  = quote_list("282828", 0);
                        light4 = quote_list("a89984", 7);
                        medium = quote_list("928374", 8);
                        red    = quote_list("fb4934", 9);
                        green  = quote_list("b8bb26", 10);
                        yellow = quote_list("fabd2f", 11);
                        blue   = quote_list("83a598", 12);
                        purple = quote_list("d3869b", 13);
                        aqua   = quote_list("8ec07c", 14);
                        light1 = quote_list("ebdbb2", 15);
                }

                if (gruvbox_contrast == "soft") {
                        dark0  = quote_list("32302f", 236);     // 50-48-47
                }

                if (gruvbox_contrast == "hard") {
                        dark0  = quote_list("1d2021", 234);     // 29-32-33
                }

                if (gruvbox_contrast_dark == "soft") {
                        dark0  = quote_list("32302f", 236);     // 50-48-47
                }

                if (gruvbox_contrast_dark == "hard") {
                        dark0  = quote_list("1d2021", 234);     // 29-32-33
                }
        } else {
                dark0  = quote_list("fbf1c7", 229);             // 251-241-199
                dark1  = quote_list("ebdbb2", 223);             // 235-219-178
                dark2  = quote_list("d5c4a1", 250);             // 213-196-161
                dark3  = quote_list("bdae93", 248);             // 189-174-147
                dark4  = quote_list("a89984", 246);             // 168-153-132

                medium = quote_list("928374", 244);             // 146-131-116

                light0 = quote_list("282828", 235);             // 40-40-40
                light1 = quote_list("3c3836", 237);             // 60-56-54
                light2 = quote_list("504945", 239);             // 80-73-69
                light3 = quote_list("665c54", 241);             // 102-92-84
                light4 = quote_list("7c6f64", 243);             // 124-111-100

                light4_256 = quote_list("7c6f64", 243);         // 124-111-100

                red    = quote_list("9d0006", 88);              // 157-0-6
                green  = quote_list("79740e", 100);             // 121-116-14
                yellow = quote_list("b57614", 136);             // 181-118-20
                blue   = quote_list("076678", 24);              // 7-102-120
                purple = quote_list("8f3f71", 96);              // 143-63-113
                aqua   = quote_list("427b58", 66);              // 66-123-88
                orange = quote_list("af3a03", 130);             // 175-58-3

                if (gruvbox_termcolors == 16) {
                        dark0  = quote_list("fbf1c7", 0);
                        light4 = quote_list("7c6f64", 7);
                        medium = quote_list("928374", 8);
                        red    = quote_list("9d0006", 9);
                        green  = quote_list("79740e", 10);
                        yellow = quote_list("b57614", 11);
                        blue   = quote_list("076678", 12);
                        purple = quote_list("8f3f71", 13);
                        aqua   = quote_list("427b58", 14);
                        light1 = quote_list("3c3836", 15);
                }

                if (gruvbox_contrast == "soft") {
                        dark0  = quote_list("f2e5bc", 228);     // 242-229-188
                }

                if (gruvbox_contrast == "hard") {
                        dark0  = quote_list("f9f5d7", 230);     // 249-245-215
                }

                if (gruvbox_contrast_light == "soft") {
                        dark0  = quote_list("f2e5bc", 228);     // 242-229-188
                }

                if (gruvbox_contrast_light == "hard") {
                        dark0  = quote_list("f9f5d7", 230);     // 249-245-215
                }
        }


        // Vanilla colorscheme ---------------------------------------------------------
        // General UI: {{{

        // Normal text
        colors += HL("Normal", light1, dark0);

        // Correct background (see issue #7):
        // --- Problem with changing between dark and light on 256 color terminal
        // --- https://github.com/morhetz/gruvbox/issues/7
        if (is_dark) {
                colors += "set background=dark";
        } else {
                colors += "set background=light";
        }

        // Screen line that the cursor is
        colors += HL("CursorLine", "none", dark1);

        // Screen column that the cursor is
        colors += HL("CursorColumn", "none", dark1);

        // Tab pages line filler
        // Active tab page label
        // Not active tab page label
        if (gruvbox_invert_tabline == 0) {
                colors += HL("TabLineFill", dark4, "bg");
                colors += HL("TabLineSel", "bg", dark4, "bold");
                colors += HL("TabLine", dark4, "bg");
        } else {
                colors += HL("TabLineFill", "bg", dark4);
                colors += HL("TabLineSel", dark4, "bg", "bold");
                colors += HL("TabLine", "bg", dark4);
        }

        // Match paired bracket under the cursor
        colors += HL("MatchParen", "none", dark3, "bold");

        // Highlighted screen columns
        colors += HL("ColorColumn", "none", CO(gruvbox_color_column));

        // Concealed element: \lambda → λ
        colors += HL("Conceal", blue, "none");

        // Line number of CursorLine
        colors += HL("CursorLineNr", yellow, dark1);

        colors += HL("NonText", dark2);
        colors += HL("SpecialKey", dark2);

        if (gruvbox_invert_selection == 0) {
                colors += HL("Visual", "none", dark2);
                colors += HL("VisualNOS", "none", dark2);
        } else {
                colors += HL("Visual", "none",  dark3, "inverse");
                colors += HL("VisualNOS", "none",  dark3, "inverse");
        }

        colors += HL("Search", dark0, yellow);
        colors += HL("IncSearch", dark0, CO(gruvbox_hls_cursor));

        colors += HL("Underlined", blue, "none", "underline");

        colors += HL("StatusLine", dark4, dark0, "bold,inverse");
        colors += HL("StatusLineNC", dark2, light4, "bold,inverse");

        // The column separating vertically split windows
        colors += HL("VertSplit", light4, CO(gruvbox_vert_split));

        // Current match in wildmenu completion
        colors += HL("WildMenu", blue, dark2, "bold");

        // Directory names, special names in listing
        colors += HL("Directory", green, "none", "bold");

        // Titles for output from :set all, :autocmd, etc.
        colors += HL("Title", green, "none", "bold");

        // Error messages on the command line
        colors += HL("ErrorMsg", "bg", red, "bold");
        // More prompt: -- More --
        colors += HL("MoreMsg", yellow, "none", "bold");
        // Current mode message: -- INSERT --
        colors += HL("ModeMsg", yellow, "none", "bold");
        // "Press enter" prompt and yes/no questions
        colors += HL("Question", orange, "none", "bold");
        // Warning messages
        colors += HL("WarningMsg", red, "none", "bold");

        // }}}
        // Gutter: {{{

        // Line number for :number and :# commands
        colors += HL("LineNr", dark4);

        // Column where signs are displayed
        colors += HL("SignColumn", "none", CO(gruvbox_sign_column));

        // Line used for closed folds
        colors += HL("Folded", medium, dark1, "italic");
        // Column where folds are displayed
        colors += HL("FoldColumn", medium, dark1);

        // }}}
        // Cursor: {{{

        // Character under cursor
        colors += HL("Cursor", "none", "none", "inverse");
        // Visual mode cursor, selection
        colors += HL("vCursor", "none", "none", "inverse");
        // Input moder cursor
        colors += HL("iCursor", "none", "none", "inverse");
        // Language mapping cursor
        colors += HL("lCursor", "none", "none", "inverse");

        // }}}
        // Syntax Highlightin {{{

        colors += HL("Special", orange);
        if (gruvbox_italicize_comments == 0) {
                colors += HL("Comment", medium, "none");
        } else {
                colors += HL("Comment", medium, "none", "italic");
        }
        colors += HL("Todo","fg", "bg", "bold");
        colors += HL("Error", "bg", red, "bold");

        // Generic statement
        colors += HL("Statement", red);
        // if, then, } else {, }, swicth, etc.
        colors += HL("Conditional", red);
        // for, do, while, etc.
        colors += HL("Repeat", red);
        // case, default, etc.
        colors += HL("Label", red);
        // try, catch, throw
        colors += HL("Exception", red);
        // sizeof, "+", "*", etc.
        colors += "hi link Operator Normal";
        // Any other keyword
        colors += HL("Keyword", red);

        // Variable name
        colors += HL("Identifier", blue);
        // Function name
        colors += HL("Function", green, "none", "bold");

        // Generic preprocessor
        colors += HL("PreProc", aqua);
        // Preprocessor #include
        colors += HL("Include", aqua);
        // Preprocessor #define
        colors += HL("Define", aqua);
        // Same as Define
        colors += HL("Macro", aqua);
        // Preprocessor #if, #} else {, #}, etc.
        colors += HL("PreCondit", aqua);

        // Generic constant
        colors += HL("Constant",purple);
        // Character constant: "c", "/n"
        colors += HL("Character", purple);
        // String constant: "this is a string"
        if (gruvbox_italicize_strings == 0) {
                colors += HL("String", green);
        } else {
                colors += HL("String", green, "none", "italic");
        }
        // Boolean constant: TRUE, false
        colors += HL("Boolean", purple);
        // Number constant: 234, 0xff
        colors += HL("Number", purple);
        // Floating point constant: 2.3e10
        colors += HL("Float", purple);

        // Generic type
        colors += HL("Type", yellow);
        // static, register, volatile, etc
        colors += HL("StorageClass", orange);
        // struct, union, enum, etc.
        colors += HL("Structure", aqua);
        // typedef
        colors += HL("Typedef", yellow);

        // }}}
        // Completion Menu: {{{

        // Popup menu: normal item
        colors += HL("Pmenu", light1, dark2);
        // Popup menu: selected item
        colors += HL("PmenuSel", dark2, blue, "bold");
        // Popup menu: scrollbar
        colors += HL("PmenuSbar", "none", dark2);
        // Popup menu: scrollbar thumb
        colors += HL("PmenuThumb", "none", dark4);

        // }}}
        // Diffs: {{{

        colors += HL("DiffDelete", dark0, red);
        colors += HL("DiffAdd", dark0, green);
        // "colors += HL("DiffChange", dark0, blue);
        // "colors += HL("DiffText", dark0, yellow);

        // Alternative setting
        colors += HL("DiffChange", dark0, aqua);
        colors += HL("DiffText", dark0, yellow);

        // }}}
        // Spellin {{{

        // Not capitalised word
        colors += HL("SpellCap", "none", "none", "undercurl", red);
        // Not recognized word
        colors += HL("SpellBad", "none", "none", "undercurl", blue);
        // Wrong spelling for selected region
        colors += HL("SpellLocal", "none", "none", "undercurl", aqua);
        // Rare word
        colors += HL("SpellRare", "none", "none", "undercurl", purple);

        // }}}

        // Plugin specific -------------------------------------------------------------
        // EasyMotion: {{{

        // colors += "hi! link EasyMotionTarget Search";
        // colors += "hi! link EasyMotionShade Comment";

        // }}}
        // Sneak: {{{

        // colors += "hi! link SneakPluginTarget Search";
        // colors += "hi! link SneakStreakTarget Search";
        // colors += HL("SneakStreakMask", yellow, yellow);
        // hi! link SneakStreakStatusLine Search

        // }}}
        // Indent Guides: {{{
        //
        // let indent_guides_auto_colors = 0
        //
        // if (gruvbox_invert_indent_guides == 0
        //      colors += HL("IndentGuidesOdd", "bg", dark2);
        //      colors += HL("IndentGuidesEven", "bg", dark1);
        // } else {
        //      colors += HL("IndentGuidesOdd", "bg", dark2, "inverse");
        //      colors += HL("IndentGuidesEven", "bg", dark3, "inverse");
        // }
        //
        // }}}
        // IndentLine: {{{
        //
        // string indentLine_color_term = dark2[1];
        // string indentLine_color_gui = "#" + dark2[0];
        //
        // }}}
        // Rainbow Parentheses: {{{
        //
        // if (!exists("rbpt_colorpairs");
        //      let rbpt_colorpairs =
        //              \ [
        //                      \ {blue, "#458588"], {"magenta", "#b16286"],
        //                      \ {red,  "#cc241d"], {"166",     "#d65d0e"]
        //              \ ]
        // }
        //
        // let rainbow_guifgs   = [ "#d65d0e", "#cc241d", "#b16286", "#458588" ]
        // let rainbow_ctermfgs = [ "166", red, "magenta", blue ]
        //
        // if (!exists("rainbow_conf");
        //      let rainbow_conf = {}
        // }
        // if (!has_key(rainbow_conf, "guifgs");
        //      let rainbow_conf{"guifgs");= rainbow_guifgs
        // }
        // if (!has_key(rainbow_conf, "ctermfgs");
        //      let rainbow_conf{"ctermfgs");= rainbow_ctermfgs
        // }
        //
        // let niji_dark_colours = rbpt_colorpairs
        // let niji_light_colours = rbpt_colorpairs
        //
        // "}}}
        // GitGutter: {{{
        //
        // if (gruvbox_invert_signs == 0) {
        //      colors += HL("GitGutterAdd", green, gruvbox_sign_column);
        //      colors += HL("GitGutterChange", aqua, gruvbox_sign_column);
        //      colors += HL("GitGutterDelete", red, gruvbox_sign_column);
        //      colors += HL("GitGutterChangeDelete", aqua, gruvbox_sign_column);
        // } else {
        //      colors += HL("GitGutterAdd", green, gruvbox_sign_column, "inverse");
        //      colors += HL("GitGutterChange", aqua, gruvbox_sign_column, "inverse");
        //      colors += HL("GitGutterDelete", red, gruvbox_sign_column, "inverse");
        //      colors += HL("GitGutterChangeDelete", aqua, gruvbox_sign_column, "inverse");
        // }
        //
        // }}}
        // Signify: {{{
        //
        // if (gruvbox_invert_signs == 0) {
        //      colors += HL("SignifySignAdd", green, gruvbox_sign_column);
        //      colors += HL("SignifySignChange ", aqua, gruvbox_sign_column);
        //      colors += HL("SignifySignDelete", red, gruvbox_sign_column);
        // } else {
        //      colors += HL("SignifySignAdd", green, gruvbox_sign_column, "inverse");
        //      colors += HL("SignifySignChange ", aqua, gruvbox_sign_column, "inverse");
        //      colors += HL("SignifySignDelete", red, gruvbox_sign_column, "inverse");
        // }
        //
        // }}}
        // Syntastic: {{{
        //
        // colors += HL("SyntasticError", "none", "none", "undercurl", red);
        // colors += HL("SyntasticWarning", "none", "none", "undercurl", yellow);
        //
        // if (gruvbox_invert_signs == 0
        //      colors += HL("SyntasticErrorSign", red, gruvbox_sign_column);
        //      colors += HL("SyntasticWarningSign", yellow, gruvbox_sign_column);
        // } else {
        //      colors += HL("SyntasticErrorSign", red, gruvbox_sign_column, "inverse");
        //      colors += HL("SyntasticWarningSign", yellow, gruvbox_sign_column, "inverse");
        // }
        //
        // }}}
        // Signature: {{{
        //
        // if (gruvbox_invert_signs == 0
        //      colors += HL("SignatureMarkerText", purple, gruvbox_sign_column);
        //      colors += HL("SignatureMarkText", blue, gruvbox_sign_column);
        // } else {
        //      colors += HL("SignatureMarkerText", purple, gruvbox_sign_column, "inverse");
        //      colors += HL("SignatureMarkText", blue, gruvbox_sign_column, "inverse");
        // }
        //
        // let SignatureMarkerTextHL=""SignatureMarkerText""
        // let SignatureMarkTextHL=""SignatureMarkText""
        //
        // }}}
        // ShowMarks: {{{
        //
        // if (gruvbox_invert_signs == 0
        //      colors += HL("ShowMarksHLl", blue, gruvbox_sign_column);
        //      colors += HL("ShowMarksHLu", blue, gruvbox_sign_column);
        //      colors += HL("ShowMarksHLo", blue, gruvbox_sign_column);
        //      colors += HL("ShowMarksHLm", blue, gruvbox_sign_column);
        // } else {
        //      colors += HL("ShowMarksHLl", blue, gruvbox_sign_column, "inverse");
        //      colors += HL("ShowMarksHLu", blue, gruvbox_sign_column, "inverse");
        //      colors += HL("ShowMarksHLo", blue, gruvbox_sign_column, "inverse");
        //      colors += HL("ShowMarksHLm", blue, gruvbox_sign_column, "inverse");
        // }
        //
        // }}}
        // CtrlP: {{{
        //
        // colors += HL("CtrlPMatch", yellow);
        // colors += HL("CtrlPNoEntries", red);
        // colors += HL("CtrlPPrtBase", dark2);
        // colors += HL("CtrlPPrtCursor", blue);
        // colors += HL("CtrlPLinePre", dark2);
        //
        // colors += HL("CtrlPMode1", blue, dark2, "bold");
        // colors += HL("CtrlPMode2", dark0, blue, "bold");
        // colors += HL("CtrlPStats", light4, dark2, "bold");
        //
        // }}}
        // Startify: {{{
        //
        // colors += HL("StartifyBracket", light3);
        // colors += HL("StartifyFile", light0);
        // colors += HL("StartifyNumber", blue);
        // colors += HL("StartifyPath", medium);
        // colors += HL("StartifySlash", medium);
        // colors += HL("StartifySection", yellow);
        // colors += HL("StartifySpecial", dark2);
        // colors += HL("StartifyHeader", orange);
        // colors += HL("StartifyFooter", dark2);
        //
        // }}}
        // Vimshell: {{{
        //
        // let vimshell_escape_colors = map(split(
        //      \ "dark4 red green yellow blue purple aqua light4 " .
        //      \ "dark0 red green orange blue purple aqua light0"
        //      \ ), ""#" + s:gb[v:val][0]");
        //
        // }}}
        //
        // Filetype specific -----------------------------------------------------------
        // Diff: {{{

        colors += HL("diffAdded", green);
        colors += HL("diffRemoved", red);
        colors += HL("diffChanged", aqua);

        colors += HL("diffFile", orange);
        colors += HL("diffNewFile", yellow);

        colors += HL("diffLine", blue);

        // }}}
        // Html: {{{
        //
        // colors += HL("htmlTag", blue);
        // colors += HL("htmlEndTag", blue);
        //
        // colors += HL("htmlTagName", aqua, "none", "bold");
        // colors += HL("htmlArg", aqua);
        //
        // colors += HL("htmlScriptTag", purple);
        // colors += HL("htmlTagN", light1);
        // colors += HL("htmlSpecialTagName", aqua, "none", "bold");
        //
        // colors += HL("htmlLink", light4, "none", "underline");
        //
        // colors += HL("htmlSpecialChar", orange);
        //
        // colors += HL("htmlBold", "fg", "bg", "bold");
        // colors += HL("htmlBoldUnderline", "fg", "bg", "bold,underline");
        // colors += HL("htmlBoldItalic", "fg", "bg", "bold,italic");
        // colors += HL("htmlBoldUnderlineItalic", "fg", "bg", "bold,underline,italic");
        //
        // colors += HL("htmlUnderline", "fg", "bg", "underline");
        // colors += HL("htmlUnderlineItalic", "fg", "bg", "underline,italic");
        // colors += HL("htmlItalic", "fg", "bg", "italic");
        //
        // }}}
        // Xml: {{{
        //
        // colors += HL("xmlTag", blue);
        // colors += HL("xmlEndTag", blue);
        // colors += HL("xmlTagName", blue);
        // colors += HL("xmlEqual", blue);
        // colors += HL("docbkKeyword", aqua, "none", "bold");
        //
        // colors += HL("xmlDocTypeDecl", medium);
        // colors += HL("xmlDocTypeKeyword", purple);
        // colors += HL("xmlCdataStart", medium);
        // colors += HL("xmlCdataCdata", purple);
        // colors += HL("dtdFunction", medium);
        // colors += HL("dtdTagName", purple);
        //
        // colors += HL("xmlAttrib", aqua);
        // colors += HL("xmlProcessingDelim", medium);
        // colors += HL("dtdParamEntityPunct", medium);
        // colors += HL("dtdParamEntityDPunct", medium);
        // colors += HL("xmlAttribPunct", medium);
        //
        // colors += HL("xmlEntity", orange);
        // colors += HL("xmlEntityPunct", orange);
        // }}}
        // Vim: {{{
        //
        // if (gruvbox_italicize_comments == 0
        //      colors += HL("vimCommentTitle", light4_256, "none", "bold");
        // } else {
        //      colors += HL("vimCommentTitle", light4_256, "none", "bold,italic");
        // }
        // colors += HL("vimNotation", orange);
        // colors += HL("vimBracket", orange);
        // colors += HL("vimMapModKey", orange);
        // colors += HL("vimFuncSID", light3);
        // colors += HL("vimSetSep", light3);
        // colors += HL("vimSep", light3);
        // colors += HL("vimContinue", light3);
        //
        // }}}
        // Clojure: {{{
        //
        // colors += HL("clojureKeyword", blue);
        // colors += HL("clojureCond", orange);
        // colors += HL("clojureSpecial", orange);
        // colors += HL("clojureDefine", orange);
        //
        // colors += HL("clojureFunc", yellow);
        // colors += HL("clojureRepeat", yellow);
        // colors += HL("clojureCharacter", aqua);
        // colors += HL("clojureStringEscape", aqua);
        // colors += HL("clojureException", red);
        //
        // colors += HL("clojureRegexp", aqua);
        // colors += HL("clojureRegexpEscape", aqua);
        // colors += HL("clojureRegexpCharClass", light3, "none", "bold");
        // colors += HL("clojureRegexpMod", light3, "none", "bold");
        // colors += HL("clojureRegexpQuantifier", light3, "none", "bold");
        //
        // colors += HL("clojureParen", light3);
        // colors += HL("clojureAnonArg", yellow);
        // colors += HL("clojureVariable", blue);
        // colors += HL("clojureMacro", orange);
        //
        // colors += HL("clojureMeta", yellow);
        // colors += HL("clojureDeref", yellow);
        // colors += HL("clojureQuote", yellow);
        // colors += HL("clojureUnquote", yellow);
        //
        // }}}
        // C: {{{
        //
        // colors += HL("cOperator", purple);
        // colors += HL("cStructure", orange);
        //
        // }}}
        // Python: {{{
        //
        // colors += HL("pythonBuiltin", orange);
        // colors += HL("pythonBuiltinObj", orange);
        // colors += HL("pythonBuiltinFunc", orange);
        // colors += HL("pythonFunction", aqua);
        // colors += HL("pythonDecorator", red);
        // colors += HL("pythonInclude", blue);
        // colors += HL("pythonImport", blue);
        // colors += HL("pythonRun", blue);
        // colors += HL("pythonCoding", blue);
        // colors += HL("pythonOperator", red);
        // colors += HL("pythonExceptions", purple);
        // colors += HL("pythonBoolean", purple);
        // colors += HL("pythonDot", light3);
        //
        // }}}
        // CSS: {{{
        //
        // colors += HL("cssBraces", blue);
        // colors += HL("cssFunctionName", yellow);
        // colors += HL("cssIdentifier", orange);
        // colors += HL("cssClassName", green);
        // colors += HL("cssColor", blue);
        // colors += HL("cssSelectorOp", blue);
        // colors += HL("cssSelectorOp2", blue);
        // colors += HL("cssImportant", green);
        // colors += HL("cssVendor", light1);
        //
        // colors += HL("cssTextProp", aqua);
        // colors += HL("cssAnimationProp", aqua);
        // colors += HL("cssUIProp", yellow);
        // colors += HL("cssTransformProp", aqua);
        // colors += HL("cssTransitionProp", aqua);
        // colors += HL("cssPrintProp", aqua);
        // colors += HL("cssPositioningProp", yellow);
        // colors += HL("cssBoxProp", aqua);
        // colors += HL("cssFontDescriptorProp", aqua);
        // colors += HL("cssFlexibleBoxProp", aqua);
        // colors += HL("cssBorderOutlineProp", aqua);
        // colors += HL("cssBackgroundProp", aqua);
        // colors += HL("cssMarginProp", aqua);
        // colors += HL("cssListProp", aqua);
        // colors += HL("cssTableProp", aqua);
        // colors += HL("cssFontProp", aqua);
        // colors += HL("cssPaddingProp", aqua);
        // colors += HL("cssDimensionProp", aqua);
        // colors += HL("cssRenderProp", aqua);
        // colors += HL("cssColorProp", aqua);
        // colors += HL("cssGeneratedContentProp", aqua);
        //
        // }}}
        // JavaScript: {{{
        //
        // colors += HL("javaScriptBraces", orange);
        // colors += HL("javaScriptFunction", aqua);
        // colors += HL("javaScriptIdentifier", red);
        // colors += HL("javaScriptMember", blue);
        // colors += HL("javaScriptNumber", purple);
        // colors += HL("javaScriptNull", purple);
        // colors += HL("javaScriptParens", light3);
        //
        // }}}
        // CoffeeScript: {{{
        //
        // colors += HL("coffeeExtendedOp", light3);
        // colors += HL("coffeeSpecialOp", light3);
        // colors += HL("coffeeCurly", orange);
        // colors += HL("coffeeParen", light3);
        // colors += HL("coffeeBracket", orange);
        //
        // }}}
        // Ruby: {{{
        //
        // colors += HL("rubyStringDelimiter", green);
        // colors += HL("rubyInterpolationDelimiter", aqua);
        //
        // }}}
        // ObjectiveC: {{{
        //
        // colors += HL("objcTypeModifier", red);
        // colors += HL("objcDirective", blue);
        //
        // }}}
        // Go: {{{
        //
        // colors += HL("goDirective", aqua);
        // colors += HL("goConstants", purple);
        // colors += HL("goDeclaration", red);
        // colors += HL("goDeclType", blue);
        // colors += HL("goBuiltins", orange);
        //
        // }}}
        // Lua: {{{
        //
        // colors += HL("luaIn", red);
        // colors += HL("luaFunction", aqua);
        // colors += HL("luaTable", orange);
        //
        // }}}
        // MoonScript: {{{
        //
        // colors += HL("moonSpecialOp", light3);
        // colors += HL("moonExtendedOp", light3);
        // colors += HL("moonFunction", light3);
        // colors += HL("moonObject", yellow);
        //
        // }}}
        // Java: {{{
        //
        // colors += HL("javaAnnotation", blue);
        // colors += HL("javaDocTags", aqua);
        // colors += "hi! link javaCommentTitle vimCommentTitle";
        // colors += HL("javaParen", light3);
        // colors += HL("javaParen1", light3);
        // colors += HL("javaParen2", light3);
        // colors += HL("javaParen3", light3);
        // colors += HL("javaParen4", light3);
        // colors += HL("javaParen5", light3);
        // colors += HL("javaOperator", orange);
        //
        // colors += HL("javaVarArg", green);
        //
        // }}}
        // Elixir: {{{
        //
        // colors += "hi! link elixirDocString Comment";
        //
        // colors += HL("elixirStringDelimiter", green);
        // colors += HL("elixirInterpolationDelimiter", aqua);
        //
        // }}}
        // Scala: {{{
        //
        // NB: scala vim syntax file is kinda horrible
        // colors += HL("scalaNameDefinition", light1);
        // colors += HL("scalaCaseFollowing", light1);
        // colors += HL("scalaCapitalWord", light1);
        // colors += HL("scalaTypeExtension", light1);
        //
        // colors += HL("scalaKeyword", red);
        // colors += HL("scalaKeywordModifier", red);
        //
        // colors += HL("scalaSpecial", aqua);
        // colors += HL("scalaOperator", light1);
        //
        // colors += HL("scalaTypeDeclaration", yellow);
        // colors += HL("scalaTypeTypePostDeclaration", yellow);
        //
        // colors += HL("scalaInstanceDeclaration", light1);
        // colors += HL("scalaInterpolation", aqua);

        vim_colorscheme("gruvbox", 256, NULL, colors, -1);
}


static int
boolean(string value)
{
        if (0 == strcasecmp(value, "y", 1) || 0 == strcasecmp(value, "on") || 0 == strcasecmp(value, "true")) {
                return 1;
        } else if (0 == strcasecmp(value, "n", 1) || 0 == strcasecmp(value, "off") || 0 == strcasecmp(value, "false")) {
                return 0;
        }
        return atoi(value);
}


void
colorscheme_gruvbox(~list args)
{
        if (!is_null(args)) {                   /* options */
                const list longoptions = {
                        /*0 */ "bold:s",
                        /*1 */ "italic:s",
                        /*2 */ "undercurl:s",
                        /*3 */ "underline:s",
                        /*4 */ "italicize_comments:s",
                        /*5 */ "italicize_strings:s",
                        /*6 */ "invert_indent_guides:s",
                        /*7 */ "termcolors:i",
                        /*8 */ "hls_cursor:s",
                        /*9 */ "sign_column:s",
                        /*10*/ "color_column:s",
                        /*11*/ "vert_split:s",
                        /*12*/ "contrast:s",
                        /*13*/ "contrast_dark:s",
                        /*14*/ "contrast_light:s",
                        /*15*/ "invert_signs:s",
                        /*16*/ "invert_selection:s",
                        /*17*/ "invert_tabline:s",
                        };
                string value;
                int ch;

                if ((ch = getopt(value, NULL, longoptions, args, "gruvbox")) >= 0) {
                        do {
                                switch(ch) {
                                case 0:  gruvbox_bold                 = boolean(value); break;
                                case 1:  gruvbox_italic               = boolean(value); break;
                                case 2:  gruvbox_undercurl            = boolean(value); break;
                                case 3:  gruvbox_underline            = boolean(value); break;
                                case 4:  gruvbox_italicize_comments   = boolean(value); break;
                                case 5:  gruvbox_italicize_strings    = boolean(value); break;
                                case 6:  gruvbox_invert_indent_guides = boolean(value); break;
                                case 7:  gruvbox_termcolors           = atoi(value); break;
                                case 8:  gruvbox_hls_cursor           = value; break;
                                case 9:  gruvbox_sign_column          = value; break;
                                case 10: gruvbox_color_column         = value; break;
                                case 11: gruvbox_vert_split           = value; break;
                                case 12: gruvbox_contrast             = value; break;
                                case 13: gruvbox_contrast_dark        = value; break;
                                case 14: gruvbox_contrast_light       = value; break;
                                case 15: gruvbox_invert_signs         = boolean(value); break;
                                case 16: gruvbox_invert_selection     = boolean(value); break;
                                case 17: gruvbox_invert_tabline       = boolean(value); break;
                                }
                        } while ((ch = getopt(value)) >= 0);
                }
        }

        gruvbox_colorscheme();
}


void
gruvbox_options(void)
{
        static list options, false_true, colors, contrasts;
        list results;

        if (0 == length_of_list(options)) {
                false_true = quote_list("off", "on");
                colors = quote_list("dark0", "dark1", "dark2", "dark3", "dark4",
                            "medium", "light0", "light1", "light2", "light3", "light4", "light4_256",
                                "red", "green", "yellow", "blue", "purple", "aqua", "orange");
                contrasts = quote_list("soft", "medium", "hard");
                options = make_list(
                                /*0 */ "bold                : ", false_true,
                                /*1 */ "italic              : ", false_true,
                                /*2 */ "undercurl           : ", false_true,
                                /*3 */ "underline           : ", false_true,
                                /*4 */ "italicize_comments  : ", false_true,
                                /*5 */ "italicize_strings   : ", false_true,
                                /*6 */ "invert_indent_guides: ", false_true,
                                /*7 */ "termcolors          : ", "",
                                /*8 */ "hls_cursor          : ", colors,
                                /*9 */ "sign_column         : ", colors,
                                /*10*/ "color_column        : ", colors,
                                /*11*/ "vert_split          : ", colors,
                                /*12*/ "contrast            : ", contrasts,
                                /*13*/ "contrast_dark       : ", contrasts,
                                /*14*/ "contrast_light      : ", contrasts,
                                /*15*/ "invert_signs        : ", false_true,
                                /*16*/ "invert_selection    : ", false_true,
                                /*17*/ "invert_tabline      : ", false_true
                                );
        }

        results[0 ] = gruvbox_bold;
        results[1 ] = gruvbox_italic;
        results[2 ] = gruvbox_undercurl;
        results[3 ] = gruvbox_underline;
        results[4 ] = gruvbox_italicize_comments;
        results[5 ] = gruvbox_italicize_strings;
        results[6 ] = gruvbox_invert_indent_guides;
        results[7 ] = ""+gruvbox_termcolors;
        results[8 ] = re_search(SF_NOT_REGEXP, gruvbox_hls_cursor, colors);
        results[9 ] = re_search(SF_NOT_REGEXP, gruvbox_sign_column, colors);
        results[10] = re_search(SF_NOT_REGEXP, gruvbox_color_column, colors);
        results[11] = re_search(SF_NOT_REGEXP, gruvbox_vert_split, colors);
        results[12] = re_search(SF_NOT_REGEXP, gruvbox_contrast, contrasts);
        results[13] = re_search(SF_NOT_REGEXP, gruvbox_contrast_dark, contrasts);
        results[14] = re_search(SF_NOT_REGEXP, gruvbox_contrast_light, contrasts);
        results[15] = gruvbox_invert_signs;
        results[16] = gruvbox_invert_selection;
        results[17] = gruvbox_invert_tabline;

        results = field_list("Gruvbox Options", results, options, TRUE, TRUE);
        if (length_of_list(results) <= 0) {
                return;
        }

        gruvbox_bold             = results[0 ];
        gruvbox_italic           = results[1 ];
        gruvbox_undercurl        = results[2 ];
        gruvbox_underline        = results[3 ];
        gruvbox_italicize_comments = results[4 ];
        gruvbox_italicize_strings = results[5 ];
        gruvbox_invert_indent_guides = results[6 ];
        gruvbox_termcolors       = atoi(results[7 ]);
        if (gruvbox_termcolors < 16) {
                gruvbox_termcolors = 16;
        } else if (gruvbox_termcolors < 1024) {
                gruvbox_termcolors = 1024;
        }
        gruvbox_hls_cursor       = colors[results[8 ]];
        gruvbox_sign_column      = colors[results[9 ]];
        gruvbox_color_column     = colors[results[10]];
        gruvbox_vert_split       = colors[results[11]];
        gruvbox_contrast         = contrasts[results[12]];
        gruvbox_contrast_dark    = contrasts[results[13]];
        gruvbox_contrast_light   = contrasts[results[14]];
        gruvbox_invert_signs     = results[15];
        gruvbox_invert_selection = results[16];
        gruvbox_invert_tabline   = results[17];

        colorscheme_gruvbox();
}

/*end*/
