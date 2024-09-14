/* -*- mode: cr; indent-width: 4; -*- */
// $Id: lucius.cr,v 1.5 2024/07/06 18:03:27 cvsuser Exp $
// lucius coloriser, GriefEdit port - beta.
//
// ============================================================================
// Name:     Lucius vim color scheme
// Author:   Jonathan Filip <jfilip1024@gmail.com>
// Version:  8.1.7
// Source:   https://github.com/jonathanfilip/lucius
// ----------------------------------------------------------------------------
//
// Light and dark color scheme for GUI and 256 color terminal.
//
// There are several options available to customize the color scheme to your
// own tastes. This is particularly useful when you have to work in different
// environments at different times (home, work, day, night, etc).
//
// The GUI and 256 color terminal versions of this color scheme are identical.
//
// Presets:
//
// There are several presets available that will set all the options for you.
// There are screenshots of each preset below:
//
//    * lucius_dark (dark default): http://i.imgur.com/LsZbF.png
//    * lucius_darkHighContrast: http://i.imgur.com/e70i9.png
//    * lucius_darkLowContrast: http://i.imgur.com/Hmw8s.png
//
//    * lucius_black: http://i.imgur.com/iD4ri.png
//    * lucius_blackHighContrast: http://i.imgur.com/lHvTJ.png
//    * lucius_blackLowContrast: http://i.imgur.com/oZLkg.png
//
//    * lucius_light (light default): http://i.imgur.com/soYD8.png
//    * lucius_lightLowContrast: http://i.imgur.com/95I86.png
//
//    * lucius_white: http://i.imgur.com/wDzkz.png
//    * lucius_whiteLowContrast: http://i.imgur.com/jlUf4.png
//
// Options [reference only]:
//
//  The presets available cover most of the options. You can, however, customize
//  things by setting the following variables yourself:
//
//  lucius_style (default: 'dark')
//
//    Set this option to either 'light' or 'dark' for your desired color scheme.
//    It has the same effect as setting the background.
//
//  lucius_contrast (default: 'normal')
//
//    This option determines the contrast to use for text/ui elements. It can be
//    set to 'low', 'normal', or 'high'. At this time there is no 'high' for the
//    light scheme.
//
//  lucius_contrast_bg (default: 'normal')
//
//    Setting this option makes the background a higher contrast. Current settings
//    are 'normal' and 'high'.
//
//  lucius_use_bold (default: 1)
//
//    Setting this will cause the color scheme to use bold fonts for some items.
//
//  lucius_use_underline (default: 1)
//
//    Setting this will cause the color scheme to use underlined fonts for some
//    items.
//
//  lucius_no_term_bg (default: 0)
//
//    Setting this will cause the color scheme to not set a background color in
//    the terminal (useful for transparency or terminals with different background
//    colors).
//
// License:
//
//  Copyright (c) 2015 Jonathan Filip
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//
// ============================================================================

#include "../grief.h"
#include "rgbmap.h"

void
main(void)
{
    require("colors/rgbmap");
}


// ============================================================================
// Functions:
// ============================================================================

static void
HI(int s, string name, ~string fg, ~string bg, ~string sp)
{
    declare value = get_property(s, name);

    if (is_null(value))
        value = "hi " + name;

    if (fg != "") value += " guifg=" + fg;
    if (bg != "") value += " guibg=" + bg;

    UNUSED(sp);
//  if (sp != "") value += " guisp=" + sp;

    set_property(s, name, value);
}


static string
color_map(string rgb)
{
    extern int colordepth;
    return RGBMap(rgb, colordepth);
}


static void
AddCterm(int s, string name, ~string extra)
{
    declare value = get_property(s, name);

    if (is_null(value))
        return;

    // foreground
    if (0 == strstr(value, "ctermfg=")) {
        const int guifg = strstr(value, "guifg=");

        if (guifg) {                            // guifg=fg|bg|NONE|#rrggbb
            value += " ctermfg=" + color_map(substr(value, guifg + 6, 7));
        }
    }

    // background
    if (0 == strstr(value, "ctermbg=")) {
        const int guibg = strstr(value, "guibg=");

        if (guibg) {                            // guibg=fg|bg|NONE|#rrggbb
            value += " ctermbg=" + color_map(substr(value, guibg + 6, 7));
        }
    }

    // additional; optional
    value += extra;

    set_property(s, name, value);
}


static void
AddSpCterm(int s, string name, ~string extra)
{
    UNUSED(s, name, extra);
    //TODO
}


// ============================================================================
// Text Groups:
// ============================================================================

static list normal_items = {
    "ColorColumn", "Comment", "Conceal", "Constant", "Cursor", "CursorColumn",
    "CursorIM", "CursorLine", "CursorLineNr", "DiffAdd", "DiffChange",
    "DiffDelete", "Directory", "Error", "ErrorMsg", "Identifier",
    "IncSearch", "LineNr", "MatchParen", "ModeMsg", "MoreMsg",
    "NonText", "Pmenu", "PmenuSbar", "PmenuSel",
    "PmenuThumb", "PreProc", "Question", "Search", "SignColumn",
    "Special", "SpecialKey", "Statement", "StatusLineNC", "TabLine",
    "TabLineFill", "Todo", "Type", "VertSplit", "Visual",
    "WarningMsg", "WildMenu"
    };

static list bold_items = {
    "DiffText", "FoldColumn", "Folded", "StatusLine", "TabLineSel",
    "Title", "CursorLineNr",
    };

static list alternative_bold_items = {
    "Identifier", "PreProc", "Statement", "Special", "Constant", "Type"
    };

static list underline_items = {
    "Underlined", "VisualNOS"
    };

static list undercurl_items = {
    "SpellBad", "SpellCap", "SpellLocal", "SpellRare"
    };


static void
lucius(string scheme, int dark, string contrast, string contrast_bg)
{
    int s = create_dictionary();
    list x;

    // ============================================================================
    // Color Definitions:
    // ============================================================================

    x += "hi clear";
    x += "set background=" + (dark ? "dark" : "light");
    x += "syntax reset";

    // ----------------------------------------------------------------------------
    // 'Normal' Colors:
    // ----------------------------------------------------------------------------

    string fg, bg;

    if (dark == 0) { // light
        if (contrast == "high") {
            fg = "#000000";
        } else if (contrast == "low") {
            fg = "#626262";
        } else {
            fg = "#444444";
        }
    } else { // dark
        if (contrast == "high") {
            fg = "#eeeeee";
        } else if (contrast == "low") {
            fg = "#bcbcbc";
        } else {
            fg = "#d7d7d7";
        }
    }

    if (dark == 0) { // light
        if (contrast_bg == "high") {
            bg = "#ffffff";
        } else {
            bg = "#eeeeee";
        }
    } else { // dark
        if (contrast_bg == "high") {
            bg = "#121212";
        } else {
            bg = "#303030";
        }
    }

    // ----------------------------------------------------------------------------
    // Extra setup
    // ----------------------------------------------------------------------------

//  if (no_term_bg == 1) {
//      x += "Normal ctermbg=NONE";
//      x += "hi Normal guifg=" + fg + " guibg=" + bg + " ctermfg=" + color_map(fg) + " ctermbg=NONE";

//  } else {
        x += "hi Normal guifg=" + fg + " guibg=" + bg + " ctermfg=" + color_map(fg) + " ctermbg=" + color_map(bg);
//  }

    // ----------------------------------------------------------------------------
    // Text Markup:
    // ----------------------------------------------------------------------------

    if (dark == 0) {
        HI(s, "NonText",                  "#afafd7",      NULL);
        HI(s, "SpecialKey",               "#afd7af",      NULL);
        if (contrast == "low") {
            HI(s, "Comment",              "#9e9e9e",      NULL);
            HI(s, "Conceal",              "#9e9e9e",      NULL);
            HI(s, "Constant",             "#d78700",      NULL);
            HI(s, "Directory",            "#00af87",      NULL);
            HI(s, "Identifier",           "#00af00",      NULL);
            HI(s, "PreProc",              "#00afaf",      NULL);
            HI(s, "Special",              "#af00af",      NULL);
            HI(s, "Statement",            "#0087d7",      NULL);
            HI(s, "Title",                "#0087d7",      NULL);
            HI(s, "Type",                 "#0087af",      NULL);
        } else {
            HI(s, "Comment",              "#808080",      NULL);
            HI(s, "Conceal",              "#808080",      NULL);
            HI(s, "Constant",             "#af5f00",      NULL);
            HI(s, "Directory",            "#00875f",      NULL);
            HI(s, "Identifier",           "#008700",      NULL);
            HI(s, "PreProc",              "#008787",      NULL);
            HI(s, "Special",              "#870087",      NULL);
            HI(s, "Statement",            "#005faf",      NULL);
            HI(s, "Title",                "#005faf",      NULL);
            HI(s, "Type",                 "#005f87",      NULL);
        }
    } else {
        HI(s, "NonText",                  "#5f5f87",      NULL);
        HI(s, "SpecialKey",               "#5f875f",      NULL);
        if (contrast == "low") {
            HI(s, "Comment",              "#6c6c6c",      NULL);
            HI(s, "Conceal",              "#6c6c6c",      NULL);
            HI(s, "Constant",             "#afaf87",      NULL);
            HI(s, "Directory",            "#87af87",      NULL);
            HI(s, "Identifier",           "#87af5f",      NULL);
            HI(s, "PreProc",              "#5faf87",      NULL);
            HI(s, "Special",              "#af87af",      NULL);
            HI(s, "Statement",            "#5fafd7",      NULL);
            HI(s, "Title",                "#00afd7",      NULL);
            HI(s, "Type",                 "#5fafaf",      NULL);
        } else if (contrast == "high") {
            HI(s, "Comment",              "#8a8a8a",      NULL);
            HI(s, "Conceal",              "#8a8a8a",      NULL);
            HI(s, "Constant",             "#ffffd7",      NULL);
            HI(s, "Directory",            "#d7ffd7",      NULL);
            HI(s, "Identifier",           "#d7ffaf",      NULL);
            HI(s, "PreProc",              "#afffd7",      NULL);
            HI(s, "Special",              "#ffd7ff",      NULL);
            HI(s, "Statement",            "#afffff",      NULL);
            HI(s, "Title",                "#87d7ff",      NULL);
            HI(s, "Type",                 "#afffff",      NULL);
        } else {
            HI(s, "Comment",              "#808080",      NULL);
            HI(s, "Conceal",              "#808080",      NULL);
            HI(s, "Constant",             "#d7d7af",      NULL);
            HI(s, "Directory",            "#afd7af",      NULL);
            HI(s, "Identifier",           "#afd787",      NULL);
            HI(s, "PreProc",              "#87d7af",      NULL);
            HI(s, "Special",              "#d7afd7",      NULL);
            HI(s, "Statement",            "#87d7ff",      NULL);
            HI(s, "Title",                "#5fafd7",      NULL);
            HI(s, "Type",                 "#87d7d7",      NULL);
        }
    }

    // ----------------------------------------------------------------------------
    // Highlighting:
    // ----------------------------------------------------------------------------

    HI(s, "Cursor",                       bg,             NULL);
    HI(s, "CursorColumn",                 "NONE",         NULL);
    HI(s, "CursorIM",                     bg,             NULL);
    HI(s, "CursorLine",                   "NONE",         NULL);
    HI(s, "Visual",                       "NONE",         NULL);
    HI(s, "VisualNOS",                    fg,             "NONE");
    if (dark == 0) {
        HI(s, "CursorColumn",             NULL,           "#dadada");
        HI(s, "CursorLine",               NULL,           "#dadada");
        HI(s, "IncSearch",                fg,             "#5fd7d7");
        HI(s, "MatchParen",               "NONE",         "#5fd7d7");
        HI(s, "Search",                   fg,             "#ffaf00");
        HI(s, "Visual",                   NULL,           "#afd7ff");
        if (contrast == "low") {
            HI(s, "Cursor",               NULL,           "#87afd7");
            HI(s, "CursorIM",             NULL,           "#87afd7");
            HI(s, "Error",                "#d70000",      "#ffd7d7");
            HI(s, "Todo",                 "#af8700",      "#ffffaf");
        } else {
            HI(s, "Cursor",               NULL,           "#5f87af");
            HI(s, "CursorIM",             NULL,           "#5f87af");
            HI(s, "Error",                "#af0000",      "#d7afaf");
            HI(s, "Todo",                 "#875f00",      "#ffffaf");
        }
    } else {
        HI(s, "CursorColumn",             NULL,           "#444444");
        HI(s, "CursorLine",               NULL,           "#444444");
        HI(s, "IncSearch",                bg,             NULL);
        HI(s, "MatchParen",               fg,             "#87af00");
        HI(s, "Search",                   bg,             NULL);
        HI(s, "Visual",                   NULL,           "#005f87");
        if (contrast == "low") {
            HI(s, "Cursor",               NULL,           "#5f87af");
            HI(s, "CursorIM",             NULL,           "#5f87af");
            HI(s, "Error",                "#d75f5f",      "#870000");
            HI(s, "IncSearch",            NULL,           "#00afaf");
            HI(s, "Search",               NULL,           "#d78700");
            HI(s, "Todo",                 "#afaf00",      "#5f5f00");
        } else if (contrast == "high") {
            HI(s, "Cursor",               NULL,           "#afd7ff");
            HI(s, "CursorIM",             NULL,           "#afd7ff");
            HI(s, "Error",                "#ffafaf",      "#af0000");
            HI(s, "IncSearch",            NULL,           "#87ffff");
            HI(s, "Search",               NULL,           "#ffaf5f");
            HI(s, "Todo",                 "#ffff87",      "#87875f");
        } else {
            HI(s, "Cursor",               NULL,           "#87afd7");
            HI(s, "CursorIM",             NULL,           "#87afd7");
            HI(s, "Error",                "#ff8787",      "#870000");
            HI(s, "IncSearch",            NULL,           "#5fd7d7");
            HI(s, "Search",               NULL,           "#d78700");
            HI(s, "Todo",                 "#d7d75f",      "#5f5f00");
        }
    }


    // ----------------------------------------------------------------------------
    // Messages:
    // ----------------------------------------------------------------------------

    HI(s, "Question",                     fg,             NULL);
    if (dark == 0) {
        if (contrast == "low") {
            HI(s, "ErrorMsg",             "#d70000",      NULL);
            HI(s, "ModeMsg",              "#0087ff",      NULL);
            HI(s, "MoreMsg",              "#0087ff",      NULL);
            HI(s, "WarningMsg",           "#d78700",      NULL);
        } else {
            HI(s, "ErrorMsg",             "#af0000",      NULL);
            HI(s, "ModeMsg",              "#005faf",      NULL);
            HI(s, "MoreMsg",              "#005faf",      NULL);
            HI(s, "WarningMsg",           "#af5f00",      NULL);
        }
    } else {
        if (contrast == "low") {
            HI(s, "ErrorMsg",             "#d75f5f",      NULL);
            HI(s, "ModeMsg",              "#87afaf",      NULL);
            HI(s, "MoreMsg",              "#87afaf",      NULL);
            HI(s, "WarningMsg",           "#af875f",      NULL);
        } else if (contrast == "high") {
            HI(s, "ErrorMsg",             "#ff8787",      NULL);
            HI(s, "ModeMsg",              "#afffff",      NULL);
            HI(s, "MoreMsg",              "#afffff",      NULL);
            HI(s, "WarningMsg",           "#ffaf87",      NULL);
        } else {
            HI(s, "ErrorMsg",             "#ff5f5f",      NULL);
            HI(s, "ModeMsg",              "#afd7d7",      NULL);
            HI(s, "MoreMsg",              "#afd7d7",      NULL);
            HI(s, "WarningMsg",           "#d7875f",      NULL);
        }
    }


    // ----------------------------------------------------------------------------
    // UI:
    // ----------------------------------------------------------------------------

    HI(s, "ColorColumn",                  "NONE",         NULL);
    HI(s, "Pmenu",                        bg,             NULL);
    HI(s, "PmenuSel",                     fg,             NULL);
    HI(s, "PmenuThumb",                   fg,             NULL);
    HI(s, "StatusLine",                   bg,             NULL);
    HI(s, "TabLine",                      bg,             NULL);
    HI(s, "TabLineSel",                   fg,             NULL);
    HI(s, "WildMenu",                     fg,             NULL);
    if (dark == 0) {
        HI(s, "ColorColumn",              NULL,           "#e4e4e4");
        HI(s, "CursorLineNr",             "#626262",      "#dadada");
        HI(s, "FoldColumn",               NULL,           "#bcbcbc");
        HI(s, "Folded",                   NULL,           "#bcbcbc");
        HI(s, "LineNr",                   "#9e9e9e",      "#dadada");
        HI(s, "PmenuSel",                 NULL,           "#afd7ff");
        HI(s, "SignColumn",               NULL,           "#d0d0d0");
        HI(s, "StatusLineNC",             "#dadada",      NULL);
        HI(s, "TabLineFill",              "#dadada",      NULL);
        HI(s, "VertSplit",                "#e4e4e4",      NULL);
        HI(s, "WildMenu",                 NULL,           "#afd7ff");
        if (contrast == "low") {
            HI(s, "FoldColumn",           "#808080",      NULL);
            HI(s, "Folded",               "#808080",      NULL);
            HI(s, "Pmenu",                NULL,           "#9e9e9e");
            HI(s, "PmenuSbar",            "#9e9e9e",      "#626262");
            HI(s, "PmenuThumb",           NULL,           "#9e9e9e");
            HI(s, "SignColumn",           "#808080",      NULL);
            HI(s, "StatusLine",           NULL,           "#9e9e9e");
            HI(s, "StatusLineNC",         NULL,           "#9e9e9e");
            HI(s, "TabLine",              NULL,           "#9e9e9e");
            HI(s, "TabLineFill",          NULL,           "#9e9e9e");
            HI(s, "TabLineSel",           NULL,           "#afd7ff");
            HI(s, "VertSplit",            NULL,           "#9e9e9e");
        } else {
            HI(s, "FoldColumn",           "#626262",      NULL);
            HI(s, "Folded",               "#626262",      NULL);
            HI(s, "Pmenu",                NULL,           "#808080");
            HI(s, "PmenuSbar",            "#808080",      "#444444");
            HI(s, "PmenuThumb",           NULL,           "#9e9e9e");
            HI(s, "SignColumn",           "#626262",      NULL);
            HI(s, "StatusLine",           NULL,           "#808080");
            HI(s, "StatusLineNC",         NULL,           "#808080");
            HI(s, "TabLine",              NULL,           "#808080");
            HI(s, "TabLineFill",          NULL,           "#808080");
            HI(s, "TabLineSel",           NULL,           "#afd7ff");
            HI(s, "VertSplit",            NULL,           "#808080");
        }
    } else {
        HI(s, "ColorColumn",              NULL,           "#3a3a3a");
        HI(s, "CursorLineNr",             "#9e9e9e",      "#444444");
        HI(s, "FoldColumn",               NULL,           "#4e4e4e");
        HI(s, "Folded",                   NULL,           "#4e4e4e");
        HI(s, "LineNr",                   "#626262",      "#444444");
        HI(s, "PmenuSel",                 NULL,           "#005f87");
        HI(s, "SignColumn",               NULL,           "#4e4e4e");
        HI(s, "StatusLineNC",             "#4e4e4e",      NULL);
        HI(s, "TabLineFill",              "#4e4e4e",      NULL);
        HI(s, "VertSplit",                "#626262",      NULL);
        HI(s, "WildMenu",                 NULL,           "#005f87");
        if (contrast == "low") {
            HI(s, "FoldColumn",           "#a8a8a8",      NULL);
            HI(s, "Folded",               "#a8a8a8",      NULL);
            HI(s, "Pmenu",                NULL,           "#8a8a8a");
            HI(s, "PmenuSbar",            "#8a8a8a",      "#bcbcbc");
            HI(s, "PmenuThumb",           NULL,           "#585858");
            HI(s, "SignColumn",           "#8a8a8a",      NULL);
            HI(s, "StatusLine",           NULL,           "#8a8a8a");
            HI(s, "StatusLineNC",         NULL,           "#8a8a8a");
            HI(s, "TabLine",              NULL,           "#8a8a8a");
            HI(s, "TabLineFill",          NULL,           "#8a8a8a");
            HI(s, "TabLineSel",           NULL,           "#005f87");
            HI(s, "VertSplit",            NULL,           "#8a8a8a");
        } else if (contrast == "high") {
            HI(s, "FoldColumn",           "#c6c6c6",      NULL);
            HI(s, "Folded",               "#c6c6c6",      NULL);
            HI(s, "Pmenu",                NULL,           "#bcbcbc");
            HI(s, "PmenuSbar",            "#bcbcbc",      "#dadada");
            HI(s, "PmenuThumb",           NULL,           "#8a8a8a");
            HI(s, "SignColumn",           "#bcbcbc",      NULL);
            HI(s, "StatusLine",           NULL,           "#bcbcbc");
            HI(s, "StatusLineNC",         NULL,           "#bcbcbc");
            HI(s, "TabLine",              NULL,           "#bcbcbc");
            HI(s, "TabLineFill",          NULL,           "#bcbcbc");
            HI(s, "TabLineSel",           NULL,           "#0087af");
            HI(s, "VertSplit",            NULL,           "#bcbcbc");
        } else {
            HI(s, "FoldColumn",           "#bcbcbc",      NULL);
            HI(s, "Folded",               "#bcbcbc",      NULL);
            HI(s, "Pmenu",                NULL,           "#b2b2b2");
            HI(s, "PmenuSbar",            "#b2b2b2",      "#d0d0d0");
            HI(s, "PmenuThumb",           NULL,           "#808080");
            HI(s, "SignColumn",           "#b2b2b2",      NULL);
            HI(s, "StatusLine",           NULL,           "#b2b2b2");
            HI(s, "StatusLineNC",         NULL,           "#b2b2b2");
            HI(s, "TabLine",              NULL,           "#b2b2b2");
            HI(s, "TabLineFill",          NULL,           "#b2b2b2");
            HI(s, "TabLineSel",           NULL,           "#005f87");
            HI(s, "VertSplit",            NULL,           "#b2b2b2");
        }
    }


    // ----------------------------------------------------------------------------
    // Diff:
    // ----------------------------------------------------------------------------

    HI(s, "DiffAdd",                      fg,             NULL);
    HI(s, "DiffChange",                   fg,             NULL);
    HI(s, "DiffDelete",                   fg,             NULL);

    if (dark == 0) {
        HI(s, "DiffAdd",                  NULL,           "#afd7af");
        HI(s, "DiffChange",               NULL,           "#d7d7af");
        HI(s, "DiffDelete",               NULL,           "#d7afaf");
        HI(s, "DiffText",                 NULL,           "#d7d7af");
        if (contrast == "low") {
            HI(s, "DiffText",             "#ff8700",      NULL);
        } else {
            HI(s, "DiffText",             "#d75f00",      NULL);
        }
    } else {
        HI(s, "DiffAdd",                  NULL,           "#5f875f");
        HI(s, "DiffChange",               NULL,           "#87875f");
        HI(s, "DiffDelete",               NULL,           "#875f5f");
        HI(s, "DiffText",                 NULL,           "#87875f");
        if (contrast == "low") {
            HI(s, "DiffText",             "#d7d75f",      NULL);
        } else {
            HI(s, "DiffText",             "#ffff87",      NULL);
        }
    }


    // ----------------------------------------------------------------------------
    // Spelling:
    // ----------------------------------------------------------------------------

    if (dark == 0) {
        HI(s, "SpellBad",                 NULL,           NULL, "#d70000");
        HI(s, "SpellCap",                 NULL,           NULL, "#00afd7");
        HI(s, "SpellLocal",               NULL,           NULL, "#d7af00");
        HI(s, "SpellRare",                NULL,           NULL, "#5faf00");
    } else {
        HI(s, "SpellBad",                 NULL,           NULL, "#ff5f5f");
        HI(s, "SpellCap",                 NULL,           NULL, "#5fafd7");
        HI(s, "SpellLocal",               NULL,           NULL, "#d7af5f");
        HI(s, "SpellRare",                NULL,           NULL, "#5faf5f");
    }


    // ----------------------------------------------------------------------------
    // Miscellaneous:
    // ----------------------------------------------------------------------------

    HI(s, "Ignore",                       bg,             NULL);
    HI(s, "Underlined",                   fg,             NULL);


    // ============================================================================
    // Cterm Colors, plus Text Emphasis
    // ============================================================================

    string item;

    while (list_each(normal_items, item) >= 0) {
        if (re_search(NULL, "^" + item + "$", alternative_bold_items) >= 0) {
            AddCterm(s, item, " gui=bold cterm=bold term=none");
        } else {
            AddCterm(s, item);
        }
    }

//  if (use_bold) {
        while (list_each(bold_items, item) >= 0) {
            AddCterm(s, item, " gui=bold cterm=bold term=none");
        }

//  } else {
//      while (list_each(bold_items, value) >= 0) {
//          AddCterm(s, item);
//      }
//  }

//  if (use_underline) {
        while (list_each(underline_items, item) >= 0) {
            AddCterm(s, item, " gui=underline cterm=underline term=none");
        }

        while (list_each(undercurl_items, item) >= 0) {
            AddSpCterm(s, item, " gui=undercurl cterm=undercurl term=none");
        }

//  } else {
//      while ((idx = list_each(underline_items, value)) >= 0) {
//          AddCterm(s, item);
//      }
//      while (list_each(undercurl_items, item) >= 0) {
//          AddSpCterm(s, item);
//      }
//  }


    // ============================================================================
    // Alternative Bold Definitions:
    // ============================================================================

//  for item in alternative_bold_items
//      let temp_gui_fg = synIDattr(synIDtrans(hlID('" . item . "')), 'fg', 'gui')"
//      let temp_cterm_fg = synIDattr(synIDtrans(hlID('" . item . "')), 'fg', 'cterm')"
//      HI(s, B" . item . " guifg=" . temp_gui_fg . " ctermfg=" . temp_cterm_fg . " gui=bold cterm=bold term=none"
//  }

    x += dict_list(s, FALSE /*values*/);
    delete_dictionary(s);

    // ============================================================================
    // Plugin Specific Colors:
    // ============================================================================

    // Tagbar:
    x += "link TagbarAccessPublic Constant";
    x += "link TagbarAccessProtected Type";
    x += "link TagbarAccessPrivate PreProc";

    // Vimwiki:
    x += "link VimwikiHeader1 BIdentifier";
    x += "link VimwikiHeader2 BPreProc";
    x += "link VimwikiHeader3 BStatement";
    x += "link VimwikiHeader4 BSpecial";
    x += "link VimwikiHeader5 BConstant";
    x += "link VimwikiHeader6 BType";

    // CoC:
    x += "hi link CocErrorSign ErrorMsg";
    x += "hi link CocErrorFloat Pmenu";
    x += "hi link CocWarningSign WarningMsg";
    x += "hi link CocWarningFloat Pmenu";
    x += "hi link CocInfoSign MoreMsg";
    x += "hi link CocInfoFloat Pmenu";
    x += "hi link CocHintFloat Directory";
    x += "hi link CocHintFloat Pmenu";

    vim_colorscheme(scheme, 0, NULL, x, -1);
}


// ============================================================================
// Preset Commands:
// ============================================================================

#define LUCIUS_DEFAULT  -1
#define LUCIUS_LIGHT 0
#define LUCIUS_DARK 1

/*
 * lucius_set ---
 *    Lucuis colorscheme
 *
 *    dark - default dark/light: default=-1, dark=1, light=0
 *    contrast - default contrast; "normal", "low" or "high"
 *    contrast_bg - default contrast_bg "normal" or "high"
 *    args - optional arguments
 *
 */
static int
lucius_set(int dark, string contrast, string contrast_bg, ~list args)
{
    string scheme = "lucius";
    int colordepth = -1;

    if (! is_null(args)) {
        /* options */
        const list longoptions = {
            /*0*/ "colors:n+",
            /*1*/ "mode:s",               // "dark" or "light"
            /*2*/ "contrast:s",           // "normal", "low" or "high"
            /*3*/ "contrast_bg:s",        // "normal" or "high"
            /*4*/ "dark",                 // mode=dark [shortcut]
            /*5*/ "light"                 // mode=light [shortcut]
            };
        int optidx = 0, ch;
        string value;

        if ((ch = getopt(value, NULL, longoptions, args, scheme)) >= 0) {
            do {
                ++optidx;
                switch(ch) {
                case 0: // colordepth
                    colordepth = atoi(value);
                    break;
                case 1: // mode
                    if (value == "default") { /* dynamic dark or light */
                        dark = -1;
                    } else {
                        dark = (value == "dark" ? 1 : 0);
                    }
                    break;
                case 2: // contrast=normal|low|high
                    if (value != "normal" && value != "low" && value != "high") {
                        error("%s: invalid contrast <%s>", scheme, value);
                        return -1;
                    }
                    contrast = value;
                    break;
                case 3: // contrast_bg=normal|high
                    if (value != "normal" && value != "high") {
                        error("%s: invalid contrast_bg <%s>", scheme, value);
                        return -1;
                    }
                    contrast_bg = value;
                    break;
                case 4: // dark
                    dark = 1;
                    break;
                case 5: // light
                    dark = 0;
                    break;
                default:
                    error("%s: %s", scheme, value);
                    return -1;
                }
            } while ((ch = getopt(value)) >= 0);
        }

        if (optidx < length_of_list(args)) {
            if (args[optidx] == "show") {
                message("%s: by %s, GriefEdit version", scheme, "Jonathan Filip");
            } else {
                error("%s: unexpected option <%s>", scheme, args[optidx]);
            }
            return -1;
        }
    }

    if (colordepth <= 0) {
         get_term_feature(TF_COLORDEPTH, colordepth);
    }
    if (colordepth != 16 && colordepth != 88 && colordepth != 256) {
        error("%s: color depth not supported", scheme);
        return -1;
    }

    if (dark == -1) {
        get_term_feature(TF_SCHEMEDARK, dark); /* default */
    }

    lucius(scheme, dark, contrast, contrast_bg);
    return 0;
}


int
colorscheme_lucius(~list args)
{
    return lucius_set(LUCIUS_DEFAULT, "normal", "normal", args);
}

///////////

int
colorscheme_lucius_light(~list args)
{
    return lucius_set(LUCIUS_LIGHT, "normal", "normal", args);
}

int
colorscheme_lucius_lightLowContrast(~list args)
{
    return lucius_set(LUCIUS_LIGHT, "low", "normal", args);
}

int
colorscheme_lucius_lightHighContrast(~list args)
{
    return lucius_set(LUCIUS_LIGHT, "high", "normal", args);
}

///////////

int
colorscheme_lucius_white(~list args)
{
    return lucius_set(LUCIUS_LIGHT, "normal", "high", args);
}

int
colorscheme_Lucius_whiteHighContrast(~list args)
{
    return lucius_set(LUCIUS_LIGHT, "high", "high", args);
}

int
colorscheme_lucius_whiteLowContrast(~list args)
{
    return lucius_set(LUCIUS_LIGHT, "low", "high", args);
}

///////////

int
colorscheme_lucius_dark(~list args)
{
   return lucius_set(LUCIUS_DARK, "normal", "normal", args);
}

int
colorscheme_lucius_darkLowContrast(~list args)
{
   return lucius_set(LUCIUS_DARK, "low", "normal", args);
}

int
colorscheme_lucius_darkHighContrast(~list args)
{
   return lucius_set(LUCIUS_DARK, "high", "normal", args);
}

///////////

int
colorscheme_lucius_black(~list args)
{
    return lucius_set(LUCIUS_DARK, "normal", "high", args);
}

int
colorscheme_lucius_blackLowContrast(~list args)
{
    return lucius_set(LUCIUS_DARK, "low", "high", args);
}

int
colorscheme_lucius_blackHighContrast(~list args)
{
    return lucius_set(LUCIUS_DARK, "high", "high", args);
}

//end
