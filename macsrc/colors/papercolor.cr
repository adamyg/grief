/* -*- mode: cr; indent-width: 4; -*- */
// $Id: papercolor.cr,v 1.3 2024/08/04 11:42:44 cvsuser Exp $
// papercolor coloriser, GriefEdit port.
//
// Theme: PaperColor
// Description : The original PaperColor Theme, inspired by Google Material Design.
//
// Version: 0.9.x
// Author: Nikyle Nguyen <NLKNguyen@MSN.com>
// License: MIT
// Source: http://github.com/NLKNguyen/papercolor-theme
//
// The MIT License (MIT)
//
// Copyright (c) 2015-2020 Nikyle Nguyen
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "../grief.h"

// ============================ THEMES ====================================

// Theme can have 'light' and/or 'dark' color palette.
//
// Color values can be HEX and/or 256-color. Use empty string '' if not provided.
// Only color00 -> color15 are required. The rest are optional.
//

static int
acquire_theme()
{
    int h = create_dictionary();

    h.NO_CONVERSION = 1;
    h.TEST_256_COLOR_CONSISTENCY = 1;
    h.original_source = "http://github.com/NLKNguyen/papercolor-theme";
    h.description = "The original PaperColor Theme, inspired by Google Material Design";
    h.allow_bold = 1;

    return h;
}


static int
palette_light()
{
    int s = create_dictionary();

    s.color00 =                 quote_list("#eeeeee", "255");
    s.color01 =                 quote_list("#af0000", "124");
    s.color02 =                 quote_list("#008700", "28");
    s.color03 =                 quote_list("#5f8700", "64");
    s.color04 =                 quote_list("#0087af", "31");
    s.color05 =                 quote_list("#878787", "102");
    s.color06 =                 quote_list("#005f87", "24");
    s.color07 =                 quote_list("#444444", "238");
    s.color08 =                 quote_list("#bcbcbc", "250");
    s.color09 =                 quote_list("#d70000", "160");
    s.color10 =                 quote_list("#d70087", "162");
    s.color11 =                 quote_list("#8700af", "91");
    s.color12 =                 quote_list("#d75f00", "166");
    s.color13 =                 quote_list("#d75f00", "166");
    s.color14 =                 quote_list("#005faf", "25");
    s.color15 =                 quote_list("#005f87", "24");
    s.color16 =                 quote_list("#0087af", "31");
    s.color17 =                 quote_list("#008700", "28");
    s.cursor_fg =               quote_list("#eeeeee", "255");
    s.cursor_bg =               quote_list("#005f87", "24");
    s.cursorline =              quote_list("#e4e4e4", "254");
    s.cursorcolumn =            quote_list("#e4e4e4", "254");
    s.cursorlinenr_fg =         quote_list("#af5f00", "130");
    s.cursorlinenr_bg =         quote_list("#eeeeee", "255");
    s.popupmenu_fg =            quote_list("#444444", "238");
    s.popupmenu_bg =            quote_list("#d0d0d0", "252");
    s.search_fg =               quote_list("#444444", "238");
    s.search_bg =               quote_list("#ffff5f", "227");
    s.incsearch_fg =            quote_list("#ffff5f", "227");
    s.incsearch_bg =            quote_list("#444444", "238");
    s.linenumber_fg =           quote_list("#b2b2b2", "249");
    s.linenumber_bg =           quote_list("#eeeeee", "255");
    s.vertsplit_fg =            quote_list("#005f87", "24");
    s.vertsplit_bg =            quote_list("#eeeeee", "255");
    s.statusline_active_fg =    quote_list("#e4e4e4", "254");
    s.statusline_active_bg =    quote_list("#005f87", "24");
    s.statusline_inactive_fg =  quote_list("#444444", "238");
    s.statusline_inactive_bg =  quote_list("#d0d0d0", "252");
    s.todo_fg =                 quote_list("#00af5f", "35");
    s.todo_bg =                 quote_list("#eeeeee", "255");
    s.error_fg =                quote_list("#af0000", "124");
    s.error_bg =                quote_list("#ffd7ff", "225");
    s.matchparen_bg =           quote_list("#c6c6c6", "251");
    s.matchparen_fg =           quote_list("#005f87", "24");
    s.visual_fg =               quote_list("#eeeeee", "255");
    s.visual_bg =               quote_list("#0087af", "31");
    s.folded_fg =               quote_list("#0087af", "31");
    s.folded_bg =               quote_list("#afd7ff", "153");
    s.wildmenu_fg =             quote_list("#444444", "238");
    s.wildmenu_bg =             quote_list("#ffff00", "226");
    s.spellbad =                quote_list("#ffafd7", "218");
    s.spellcap =                quote_list("#ffffaf", "229");
    s.spellrare =               quote_list("#afff87", "156");
    s.spelllocal =              quote_list("#d7d7ff", "189");
    s.diffadd_fg =              quote_list("#008700", "28");
    s.diffadd_bg =              quote_list("#afffaf", "157");
    s.diffdelete_fg =           quote_list("#af0000", "124");
    s.diffdelete_bg =           quote_list("#ffd7ff", "225");
    s.difftext_fg =             quote_list("#0087af", "31");
    s.difftext_bg =             quote_list("#ffffd7", "230");
    s.diffchange_fg =           quote_list("#444444", "238");
    s.diffchange_bg =           quote_list("#ffd787", "222");
    s.tabline_bg =              quote_list("#005f87", "24O");
    s.tabline_active_fg =       quote_list("#444444", "238");
    s.tabline_active_bg =       quote_list("#e4e4e4", "254");
    s.tabline_inactive_fg =     quote_list("#eeeeee", "255");
    s.tabline_inactive_bg =     quote_list("#0087af", "31");
    s.buftabline_bg =           quote_list("#005f87", "24");
    s.buftabline_current_fg =   quote_list("#444444", "238");
    s.buftabline_current_bg =   quote_list("#e4e4e4", "254");
    s.buftabline_active_fg =    quote_list("#eeeeee", "255");
    s.buftabline_active_bg =    quote_list("#005faf", "25");
    s.buftabline_inactive_fg =  quote_list("#eeeeee", "255");
    s.buftabline_inactive_bg =  quote_list("#0087af", "31");

    return s;
}


static int
palette_dark()
{
    int s = create_dictionary();

    s.color00 =                 quote_list("#1c1c1c", "234");
    s.color01 =                 quote_list("#af005f", "125");
    s.color02 =                 quote_list("#5faf00", "70");
    s.color03 =                 quote_list("#d7af5f", "179");
    s.color04 =                 quote_list("#5fafd7", "74");
    s.color05 =                 quote_list("#808080", "244");
    s.color06 =                 quote_list("#d7875f", "173");
    s.color07 =                 quote_list("#d0d0d0", "252");
    s.color08 =                 quote_list("#585858", "240");
    s.color09 =                 quote_list("#5faf5f", "71");
    s.color10 =                 quote_list("#afd700", "148");
    s.color11 =                 quote_list("#af87d7", "140");
    s.color12 =                 quote_list("#ffaf00", "214");
    s.color13 =                 quote_list("#ff5faf", "205");
    s.color14 =                 quote_list("#00afaf", "37");
    s.color15 =                 quote_list("#5f8787", "66");
    s.color16 =                 quote_list("#5fafd7", "74");
    s.color17 =                 quote_list("#d7af00", "178");
    s.cursor_fg =               quote_list("#1c1c1c", "234");
    s.cursor_bg =               quote_list("#c6c6c6", "251");
    s.cursorline =              quote_list("#303030", "236");
    s.cursorcolumn =            quote_list("#303030", "236");
    s.cursorlinenr_fg =         quote_list("#ffff00", "226");
    s.cursorlinenr_bg =         quote_list("#1c1c1c", "234");
    s.popupmenu_fg =            quote_list("#c6c6c6", "251");
    s.popupmenu_bg =            quote_list("#303030", "236");
    s.search_fg =               quote_list("#000000", "16");
    s.search_bg =               quote_list("#00875f", "29");
    s.incsearch_fg =            quote_list("#00875f", "29");
    s.incsearch_bg =            quote_list("#000000", "16");
    s.linenumber_fg =           quote_list("#585858", "240");
    s.linenumber_bg =           quote_list("#1c1c1c", "234");
    s.vertsplit_fg =            quote_list("#5f8787", "66");
    s.vertsplit_bg =            quote_list("#1c1c1c", "234");
    s.statusline_active_fg =    quote_list("#1c1c1c", "234");
    s.statusline_active_bg =    quote_list("#5f8787", "66");
    s.statusline_inactive_fg =  quote_list("#bcbcbc", "250");
    s.statusline_inactive_bg =  quote_list("#3a3a3a", "237");
    s.todo_fg =                 quote_list("#ff8700", "208");
    s.todo_bg =                 quote_list("#1c1c1c", "234");
    s.error_fg =                quote_list("#af005f", "125");
    s.error_bg =                quote_list("#5f0000", "52");
    s.matchparen_bg =           quote_list("#4e4e4e", "239");
    s.matchparen_fg =           quote_list("#c6c6c6", "251");
    s.visual_fg =               quote_list("#000000", "16");
    s.visual_bg =               quote_list("#8787af", "103");
    s.folded_fg =               quote_list("#d787ff", "177");
    s.folded_bg =               quote_list("#5f005f", "53");
    s.wildmenu_fg =             quote_list("#1c1c1c", "234");
    s.wildmenu_bg =             quote_list("#afd700", "148");
    s.spellbad =                quote_list("#5f0000", "52");
    s.spellcap =                quote_list("#5f005f", "53");
    s.spellrare =               quote_list("#005f00", "22");
    s.spelllocal =              quote_list("#00005f", "17");
    s.diffadd_fg =              quote_list("#87d700", "112");
    s.diffadd_bg =              quote_list("#005f00", "22");
    s.diffdelete_fg =           quote_list("#af005f", "125");
    s.diffdelete_bg =           quote_list("#5f0000", "52");
    s.difftext_fg =             quote_list("#5fffff", "87");
    s.difftext_bg =             quote_list("#008787", "30");
    s.diffchange_fg =           quote_list("#d0d0d0", "252");
    s.diffchange_bg =           quote_list("#005f5f", "23");
    s.tabline_bg =              quote_list("#262626", "235");
    s.tabline_active_fg =       quote_list("#121212", "233");
    s.tabline_active_bg =       quote_list("#00afaf", "37");
    s.tabline_inactive_fg =     quote_list("#bcbcbc", "250");
    s.tabline_inactive_bg =     quote_list("#585858", "240");
    s.buftabline_bg =           quote_list("#262626", "235");
    s.buftabline_current_fg =   quote_list("#121212", "233");
    s.buftabline_current_bg =   quote_list("#00afaf", "37");
    s.buftabline_active_fg =    quote_list("#00afaf", "37");
    s.buftabline_active_bg =    quote_list("#585858", "240");
    s.buftabline_inactive_fg =  quote_list("#bcbcbc", "250");
    s.buftabline_inactive_bg =  quote_list("#585858", "240");

    return s;
}

// ============================ THEME REGISTER =================================

static int
acquire_pallette(int dark)
{
    if (dark) return palette_dark();
    return palette_light();
}


// ============================ OPTION HANDLER =================================

// Generate Them Option Variables:

static void
generate_theme_option_variables(int h)
{
    UNUSED(h);

#if (TODO)

    // 0. All possible theme option names must be registered here
    list available_theme_options = {
	    'allow_bold',
	    'allow_italic',
	    'transparent_background',
	    };

    // 1. Generate variables and set to default value
    for l:option in l:available_theme_options
	let s:{'themeOpt_' . l:option} = 0
    endfor

    let s:themeOpt_override = {}	// special case, this has to be a dictionary

    // 2. Reassign value to the above variables based on theme settings

    // 2.1 In case the theme has top-level options
    if has_key(s:selected_theme, 'options')
	let l:theme_options = s:selected_theme['options']
	for l:opt_name in keys(l:theme_options)
	    let s:{'themeOpt_' . l:opt_name} = l:theme_options[l:opt_name]
	    // echo 's:themeOpt_' . l:opt_name . ' = ' . s:{'themeOpt_' . l:opt_name}
	endfor
    endif

    // 2.2 In case the theme has specific variant options
    if has_key(s:selected_theme[s:selected_variant], 'options')
	let l:theme_options = s:selected_theme[s:selected_variant]['options']
	for l:opt_name in keys(l:theme_options)
	    let s:{'themeOpt_' . l:opt_name} = l:theme_options[l:opt_name]
	    // echo 's:themeOpt_' . l:opt_name . ' = ' . s:{'themeOpt_' . l:opt_name}
	endfor
    endif


    // 3. Reassign value to the above variables which the user customizes
    // Part of user-config options
    let s:theme_options = {}
    if has_key(s:options, 'theme')
	let s:theme_options = s:options['theme']
    endif

    // 3.1 In case user sets for a theme without specifying which variant
    if has_key(s:theme_options, s:theme_name)
	let l:theme_options = s:theme_options[s:theme_name]
	for l:opt_name in keys(l:theme_options)
	    let s:{'themeOpt_' . l:opt_name} = l:theme_options[l:opt_name]
	    // echo 's:themeOpt_' . l:opt_name . ' = ' . s:{'themeOpt_' . l:opt_name}
	endfor
    endif


    // 3.2 In case user sets for a specific variant of a theme

    // Create the string that the user might have set for this theme variant
    // for example, 'default.dark'
    let l:specific_theme_variant = s:theme_name . '.' . s:selected_variant

    if has_key(s:theme_options, l:specific_theme_variant)
	let l:theme_options = s:theme_options[l:specific_theme_variant]
	for l:opt_name in keys(l:theme_options)
	    let s:{'themeOpt_' . l:opt_name} = l:theme_options[l:opt_name]
	    // echo 's:themeOpt_' . l:opt_name . ' = ' . s:{'themeOpt_' . l:opt_name}
	endfor
    endif

#endif

}


// Check If Theme Has Hint:
//
// Brief:
//    Function to Check if the selected theme and variant has a hint
//
// Details:
//    A hint is a known key that has value 1
//    It is not part of theme design but is used for technical purposes
//
// Example:
//    If a theme has hint 'NO_CONVERSION', then we can assume that every
//    color value is a complete pair, so we don't have to check.

// Set Overriding Colors:

static void
set_overriding_colors(int s, int h)
{
    UNUSED(h);
    UNUSED(s);

#if (TODO)

    if *h.NO_CONVERSION) {

        // s:convert_colors will not do anything, so we take care of conversion
        // for the overriding colors that need to be converted

        if (h.mode == MODE_GUI_COLOR) {

            // if GUI color is not provided, convert from 256 color that must be available
            if !empty(s:themeOpt_override)
                load_256_to_GUI_converter()
            endif

            for l:color in keys(s:themeOpt_override)
                let l:value = s:themeOpt_override[l:color]
                if l:value[0] == ''
                    let l:value[0] = s:to_HEX[l:value[1]]
                endif
                let s:palette[l:color] = l:value
            endfor

        } else if (h.mode == MODE_256_COLOR) {

            // if 256 color is not provided, convert from GUI color that must be available
            if !empty(s:themeOpt_override)
                load_GUI_to_256_converter()
            endif

            for l:color in keys(s:themeOpt_override)
                let l:value = s:themeOpt_override[l:color]
                if l:value[1] == ''
                    let l:value[1] = s:to_256(l:value[0])
                endif
                let s:palette[l:color] = l:value
            endfor
        }

    } else {

        // simply set the colors and let s:convert_colors() take care of conversion

        for l:color in keys(s:themeOpt_override)
            let s:palette[l:color] = s:themeOpt_override[l:color]
        endfor
    }

#endif
}


// Generate Language Option Variables:

// Brief:
//    Function to generate language option variables so that there is no need to
//    look up from the dictionary every time the option value is checked in the
//    function s:apply_syntax_highlightings()
//
// Require:
//    s:options <dictionary> user options
//
// Require Optionally:
//    g:PaperColor_Theme_Options  <dictionary>  user option config in .vimrc
//
// Expose:
//    s:langOpt_[LANGUAGE]__[OPTION]  <any>   variables for language options
//
// Example:
//     g:PaperColor_Theme_Options has something like this:
//       'language': {
//       \   'python': {
//       \     'highlight_builtins': 1
//       \   }
//       }
//    The following variable will be generated:
//    s:langOpt_python__highlight_builtins = 1
//

static void
generate_language_option_variables(int h)
{
    UNUSED(h);

#if (TODO)

    // Possible theme option names must be registered here
    list available_language_options = {
            'c__highlight_builtins',
            'cpp__highlight_standard_library',
            'python__highlight_builtins',
            'haskell__no_bold_types'
            };

    // Generate variables and set to default value
    for l:option in l:available_language_options
        let s:{'langOpt_' . l:option} = 0
    endfor

    // Part of user-config options
    if has_key(s:options, 'language')
        let l:language_options = s:options['language']
        // echo l:language_options
        for l:lang in keys(l:language_options)
            let l:options = l:language_options[l:lang]
            // echo l:lang
            // echo l:options
            for l:option in keys(l:options)
                  let s:{'langOpt_' . l:lang . '__' . l:option} = l:options[l:option]
                  // echo 's:langOpt_' . l:lang . '__' . l:option . ' = ' . l:options[l:option]
            endfor
        endfor

    endif

#endif
}

// =========================== COLOR CONVERTER =================================

// ========================== ENVIRONMENT ADAPTER ==============================

// Set Format Attributes:

static void
set_format_attributes(int s, int h)
{
    extern int colordepth;

    UNUSED(h);

    // These are the default

    if (colordepth >= 256) {
	s.ft_bold	  = " cterm=bold gui=bold " + " cterm=bold ";
        s.ft_none         = " cterm=none gui=none " + " cterm=none ";
        s.ft_reverse      = " cterm=reverse gui=reverse " + " cterm=reverse ";
        s.ft_italic       = " cterm=italic gui=italic " + " cterm=italic ";
        s.ft_italic_bold  = " cterm=italic,bold gui=italic,bold " + " cterm=italic,bold ";

    } else {
        s.ft_bold         = "";
        s.ft_none         = " cterm=none ";
        s.ft_reverse      = " cterm=reverse ";
        s.ft_italic       = "";
        s.ft_italic_bold  = "";
    }

    // Unless instructed otherwise either by theme setting or user overriding

    //TODO
}


// Convert Colors If Needed:

static void
convert_colors(int s, int h)
{
    UNUSED(s);
    UNUSED(h);

    //TODO
}


// ============================ COLOR POPULARIZER ===============================

// Helper:
// -------
// Function to dynamically generate variables that store the color strings
// for setting highlighting. Each color name will have 2 variables with prefix
//
// s:fg_ and s:bg_.
//
// For example:
//
// if a:color_name is 'Normal' and a:color_value is ['#000000", "0', 'Black");
// the following 2 variables will be created:
//
//   s:fg_Normal that stores the string ' guifg=#000000 '
//   s:bg_Normal that stores the string ' guibg=#000000 '
//
// Depending on the color mode, ctermfg and ctermbg will be either 0 or Black
//
// Rationale:
// The whole purpose is for speed. We generate these ahead of time so that we
// don't have to do look up or do any if-branch when we set the highlightings.
//

static void
pset(int s, string color_name, list rich_color, string term_color)
{
    extern int colordepth;

    if (colordepth >= 256) {
        set_property(s, "fg_"+color_name, " guifg="+rich_color[0] + " ctermfg="+rich_color[1]);
        set_property(s, "bg_"+color_name, " guibg="+rich_color[0] + " ctermbg="+rich_color[1]);
//TODO  set_property(s, "sp_"+color_name, " guisp="+rich_color[0]);
        set_property(s, "sp_"+color_name, "");

    } else {
        set_property(s, "fg_"+color_name, " ctermfg="+term_color);
        set_property(s, "bg_"+color_name, " ctermbg="+term_color);
        set_property(s, "sp_"+color_name, "");
    }
}


static list
pget(int s, string name, ~list defvalue)
{
    declare value = get_property(s, name);
    if (is_null(value)) return defvalue;
    return value;
}

// ========================== ENVIRONMENT ADAPTER ==============================

// Set Color Variables:

static void
set_color_variables(int s)
{
    // Color value format: Array [<GUI COLOR/HEX >, <256-Base>, <16-Base>]
    // 16-Base is terminal"s native color palette that can be alternated through
    // the terminal settings. The 16-color names are according to `:h cterm-colors`

    // BASIC COLORS:
    // color00-15 are required by all themes.
    // These are also how the terminal color palette for the target theme should be.
    // See README for theme design guideline
    //
    // An example format of the below variable's value: ['#262626', '234', 'Black']
    // Where the 1st value is HEX color for GUI Vim, 2nd value is for 256-color terminal,
    // and the color name on the right is for 16-color terminal (the actual terminal colors
    // can be different from what the color names suggest). See :h cterm-colors
    //
    // Depending on the provided color palette and current Vim, the 1st and 2nd
    // parameter might not exist, for example, on 16-color terminal, the variables below
    // only store the color names to use the terminal color palette which is the only
    // thing available therefore no need for GUI-color or 256-color.

    list color00 = pget(s, "color00");
    list color01 = pget(s, "color01");
    list color02 = pget(s, "color02");
    list color03 = pget(s, "color03");
    list color04 = pget(s, "color04");
    list color05 = pget(s, "color05");
    list color06 = pget(s, "color06");
    list color07 = pget(s, "color07");
    list color08 = pget(s, "color08");
    list color09 = pget(s, "color09");
    list color10 = pget(s, "color10");
    list color11 = pget(s, "color11");
    list color12 = pget(s, "color12");
    list color13 = pget(s, "color13");
    list color14 = pget(s, "color14");
    list color15 = pget(s, "color15");

    pset(s, "background", color00, "Black");
    pset(s, "negative", color01, "DarkRed");
    pset(s, "positive", color02, "DarkGreen");
    pset(s, "olive", color03, "DarkYellow");
    pset(s, "neutral", color04, "DarkBlue");
    pset(s, "comment", color05, "DarkMagenta");
    pset(s, "navy", color06, "DarkCyan");
    pset(s, "foreground", color07, "LightGray");

    pset(s, "nontext", color08, "DarkGray");
    pset(s, "red", color09, "LightRed");
    pset(s, "pink", color10, "LightGreen");
    pset(s, "purple", color11, "LightYellow");
    pset(s, "accent", color12, "LightBlue");
    pset(s, "orange", color13, "LightMagenta");
    pset(s, "blue", color14, "LightCyan");
    pset(s, "highlight", color15, "White");

    // Note: special case for FoldColumn grous. I want to get rid of this case.
    pset(s, "transparent", quote_list(color00[0], "none"), "none");

    // EXTENDED COLORS:
    //
    // From here on, all colors are optional and must have default values (3rd parameter of the
    // `get` command) that point to the above basic colors in case the target theme doesn"t
    // provide the extended colors. The default values should be reasonably sensible.
    // The terminal color must be provided also.

    pset(s, "aqua", pget(s, "color16", color14), "LightCyan");
    pset(s, "green", pget(s, "color17", color13), "LightMagenta");
    pset(s, "wine", pget(s, "color18", color11), "LightYellow");

    // LineNumber: when set number
    pset(s, "linenumber_fg", pget(s, "linenumber_fg", color08), "DarkGray");
    pset(s, "linenumber_bg", pget(s, "linenumber_bg", color00), "Black");

    // Vertical Split: when there are more than 1 window side by side, ex: <C-W><C-V>
    pset(s, "vertsplit_fg", pget(s, "vertsplit_fg", color15), "White");
    pset(s, "vertsplit_bg", pget(s, "vertsplit_bg", color00), "Black");

    // Statusline: when set status=2
    pset(s, "statusline_active_fg", pget(s, "statusline_active_fg", color00), "Black");
    pset(s, "statusline_active_bg", pget(s, "statusline_active_bg", color15), "White");
    pset(s, "statusline_inactive_fg", pget(s, "statusline_inactive_fg", color07), "LightGray");
    pset(s, "statusline_inactive_bg", pget(s, "statusline_inactive_bg", color08), "DarkGray");

    // Cursor: in normal mode
    pset(s, "cursor_fg", pget(s, "cursor_fg", color00), "Black");
    pset(s, "cursor_bg", pget(s, "cursor_bg", color07), "LightGray");

    pset(s, "cursorline", pget(s, "cursorline", color00), "Black");

    // CursorColumn: when set cursorcolumn
    pset(s, "cursorcolumn", pget(s, "cursorcolumn", color00), "Black");

    // CursorLine Number: when set cursorline number
    pset(s, "cursorlinenr_fg", pget(s, "cursorlinenr_fg", color13), "LightMagenta");
    pset(s, "cursorlinenr_bg", pget(s, "cursorlinenr_bg", color00), "Black");

    // Popup Menu: when <C-X><C-N> for autocomplete
    pset(s, "popupmenu_fg", pget(s, "popupmenu_fg", color07), "LightGray");
    pset(s, "popupmenu_bg", pget(s, "popupmenu_bg", color08), "DarkGray"); // TODO: double check this, might resolve an issue

    // Search: ex: when * on a word
    pset(s, "search_fg", pget(s, "search_fg", color00), "Black");
    pset(s, "search_bg", pget(s, "search_bg", color15), "Yellow");

    // IncSearch: ex: during a search
    pset(s, "incsearch_fg", pget(s, "incsearch_fg", color00), "Black");
    pset(s, "incsearch_bg", pget(s, "incsearch_bg", color15), "Yellow");

    // Todo: ex: TODO
    pset(s, "todo_fg", pget(s, "todo_fg", color05), "LightYellow");
    pset(s, "todo_bg", pget(s, "todo_bg", color00), "Black");

    // Error: ex: turn spell on and have invalid words
    pset(s, "error_fg", pget(s, "error_fg", color01), "DarkRed");
    pset(s, "error_bg", pget(s, "error_bg", color00), "Black");

    // Match Parenthesis: selecting an opening/closing pair and the other one will be highlighted
    pset(s, "matchparen_fg", pget(s, "matchparen_fg", color00), "LightMagenta");
    pset(s, "matchparen_bg", pget(s, "matchparen_bg", color05), "Black");

    // Visual:
    pset(s, "visual_fg", pget(s, "visual_fg", color08), "Black");
    pset(s, "visual_bg", pget(s, "visual_bg", color07), "White");

    // Folded:
    pset(s, "folded_fg", pget(s, "folded_fg", color00), "Black");
    pset(s, "folded_bg", pget(s, "folded_bg", color05), "DarkYellow");

    // WildMenu: Autocomplete command, ex: :color <tab><tab>
    pset(s, "wildmenu_fg", pget(s, "wildmenu_fg", color00), "Black");
    pset(s, "wildmenu_bg", pget(s, "wildmenu_bg", color06), "LightGray");

    // Spelling: when spell on and there are spelling problems like this for example: papercolor. a vim color scheme
    pset(s, "spellbad", pget(s, "spellbad", color04), "DarkRed");
    pset(s, "spellcap", pget(s, "spellcap", color05), "DarkMagenta");
    pset(s, "spellrare", pget(s, "spellrare", color06), "DarkYellow");
    pset(s, "spelllocal", pget(s, "spelllocal", color01), "DarkBlue");

    // Diff:
    pset(s, "diffadd_fg", pget(s, "diffadd_fg", color00), "Black");
    pset(s, "diffadd_bg", pget(s, "diffadd_bg", color02), "DarkGreen");

    pset(s, "diffdelete_fg", pget(s, "diffdelete_fg", color00), "Black");
    pset(s, "diffdelete_bg", pget(s, "diffdelete_bg", color04), "DarkRed");

    pset(s, "difftext_fg", pget(s, "difftext_fg", color00), "Black");
    pset(s, "difftext_bg", pget(s, "difftext_bg", color06), "DarkYellow");

    pset(s, "diffchange_fg", pget(s, "diffchange_fg", color00), "Black");
    pset(s, "diffchange_bg", pget(s, "diffchange_bg", color14), "LightYellow");

    // Tabline: when having tabs, ex: :tabnew
    pset(s, "tabline_bg", pget(s, "tabline_bg", color00), "Black");
    pset(s, "tabline_active_fg", pget(s, "tabline_active_fg", color07), "LightGray");
    pset(s, "tabline_active_bg", pget(s, "tabline_active_bg", color00), "Black");
    pset(s, "tabline_inactive_fg", pget(s, "tabline_inactive_fg", color07), "Black");
    pset(s, "tabline_inactive_bg", pget(s, "tabline_inactive_bg", color08), "DarkMagenta");

    // Plugin: BufTabLine https://github.com/ap/vim-buftabline
    pset(s, "buftabline_bg", pget(s, "buftabline_bg", color00), "Black");
    pset(s, "buftabline_current_fg", pget(s, "buftabline_current_fg", color07), "LightGray");
    pset(s, "buftabline_current_bg", pget(s, "buftabline_current_bg", color05), "DarkMagenta");
    pset(s, "buftabline_active_fg", pget(s, "buftabline_active_fg", color07), "LightGray");
    pset(s, "buftabline_active_bg", pget(s, "buftabline_active_bg", color12), "LightBlue");
    pset(s, "buftabline_inactive_fg", pget(s, "buftabline_inactive_fg", color07), "LightGray");
    pset(s, "buftabline_inactive_bg", pget(s, "buftabline_inactive_bg", color00), "Black");

#if (TODO)

    // Neovim terminal colors https://neovim.io/doc/user/nvim_terminal_emulator.html#nvim-terminal-emulator-configuration
    // TODO: Fix this
//  let g:terminal_color_0  = color00[0]
//  let g:terminal_color_1  = color01[0]
//  let g:terminal_color_2  = color02[0]
//  let g:terminal_color_3  = color03[0]
//  let g:terminal_color_4  = color04[0]
//  let g:terminal_color_5  = color05[0]
//  let g:terminal_color_6  = color06[0]
//  let g:terminal_color_7  = color07[0]
//  let g:terminal_color_8  = color08[0]
//  let g:terminal_color_9  = color09[0]
//  let g:terminal_color_10 = color10[0]
//  let g:terminal_color_11 = color11[0]
//  let g:terminal_color_12 = color12[0]
//  let g:terminal_color_13 = color13[0]
//  let g:terminal_color_14 = color14[0]
//  let g:terminal_color_15 = color15[0]
//
//  // Vim 8"s :terminal buffer ANSI colors
//  if (has_terminal) {
//      let g:terminal_ansi_colors = [color00[0], color01[0], color02[0], color03[0],
//          \ color04[0], color05[0], color06[0], color07[0], color08[0], color09[0],
//          \ color10[0], color11[0], color12[0], color13[0], color14[0], color15[0]]
//  }

#endif
}


static list apply_basic(int s, int h);
static list apply_vim(int s);
static list apply_make(int s);
static list apply_c(int s);
static list apply_lex(int s);
static list apply_asm(int s);
static list apply_shell(int s);
static list apply_markup(int s);
static list apply_py(int s);
static list apply_java(int s);
static list apply_js(int s);
static list apply_go(int s);
static list apply_stap(int s);
static list apply_dtrace(int s);
static list apply_plantuml(int s);
static list apply_haskell(int s);
static list apply_sql(int s);
static list apply_octave(int s);
static list apply_ruby(int s);
static list apply_fortran(int s);
static list apply_algol(int s);
static list apply_r(int s);
static list apply_xxd(int s);
static list apply_php(int s);
static list apply_perl(int s);
static list apply_pascal(int s);
static list apply_lua(int s);
static list apply_clojure(int s);
static list apply_docker(int s);
static list apply_ngx(int s);
static list apply_ymal(int s);
static list apply_qml(int s);
static list apply_dosini(int s);
static list apply_mail(int s);
static list apply_xml(int s);
static list apply_elixir(int s);
static list apply_erlang(int s);
static list apply_cucumber(int s);
static list apply_ada(int s);
static list apply_cobol(int s);
static list apply_sed(int s);
static list apply_awk(int s);
static list apply_elm(int s);
static list apply_ps(int s);
static list apply_fsharp(int s);
static list apply_asn1(int s);
static list apply_netrw(int s);
static list apply_nerdtree(int s);
static list apply_tagbar(int s);
static list apply_diff(int s);
static list apply_spell(int s);
static list apply_indent(int s);
static list apply_startify(int s);
static list apply_signify(int s);
static list apply_git(int s);
static list apply_coc(int s);


static string
HI(string group, string a, ~ string b, ~ string c, ~ string d)
{
    string val;
    val = "hi " + group + a + b + c + d;
    return val;
}


static void
apply_syntax_highlightings(int s, int h)
{
    list x;

    x += apply_basic(s, h);
    x += apply_vim(s);
    x += apply_make(s);
    x += apply_c(s);
    x += apply_lex(s);
    x += apply_asm(s);
    x += apply_shell(s);
    x += apply_markup(s);
    x += apply_py(s);
    x += apply_java(s);
    x += apply_js(s);
    x += apply_go(s);
    x += apply_stap(s);
    x += apply_dtrace(s);
    x += apply_plantuml(s);
    x += apply_haskell(s);
    x += apply_sql(s);
    x += apply_octave(s);
    x += apply_ruby(s);
    x += apply_fortran(s);
    x += apply_algol(s);
    x += apply_r(s);
    x += apply_xxd(s);
    x += apply_php(s);
    x += apply_perl(s);
    x += apply_pascal(s);
    x += apply_lua(s);
    x += apply_clojure(s);
    x += apply_docker(s);
    x += apply_ngx(s);
    x += apply_ymal(s);
    x += apply_qml(s);
    x += apply_dosini(s);
    x += apply_mail(s);
    x += apply_xml(s);
    x += apply_elixir(s);
    x += apply_erlang(s);
    x += apply_cucumber(s);
    x += apply_ada(s);
    x += apply_cobol(s);
    x += apply_sed(s);
    x += apply_awk(s);
    x += apply_elm(s);
    x += apply_ps(s);
    x += apply_fsharp(s);
    x += apply_asn1(s);
    x += apply_netrw(s);
    x += apply_nerdtree(s);
    x += apply_tagbar(s);
    x += apply_diff(s);
    x += apply_spell(s);
    x += apply_indent(s);
    x += apply_startify(s);
    x += apply_signify(s);
    x += apply_git(s);
    x += apply_coc(s);

    vim_colorscheme(h.SCHEME, 0, NULL, x, h.GUI);
}


static list
apply_basic(int s, int h)
{
    list x;

    x += "hi clear";
    x += "syntax reset";

    // Apply Syntax Highlightings:
    // -------------------------------------------------------------------------------------------------

//TODO
//  if (h.themeOpt_transparent_background) {
//      x += HI("Normal", s:fg_foreground);
//
//      //" Switching between dark & light variant through `set background`
//      //" NOTE: Handle background switching right after `Normal` group because of
//      //" God-know-why reason. Not doing this way had caused issue before
//
//      if (h.DARK) {
//          x += "set background=dark";
//      } else {
//          x += "set background=light";
//      }
//
//      x += HI("NonText", s,fg_nontext);
//      x += HI("LineNr", s,fg_linenumber_fg);
//      x += HI("Conceal", s,fg_linenumber_fg);
//      x += HI("VertSplit", s,fg_vertsplit_fg, s,ft_none);
//      x += HI("FoldColumn", s,fg_folded_fg, s,bg_transparent, s,ft_none);
//
//  } else {
        x += HI("Normal", s.fg_foreground, s.bg_background);

        // Switching between dark & light variant through `set background`
        if (h.DARK) {
            x += "set background=dark";
            x += HI("EndOfBuffer", s.fg_cursor_fg, s.ft_none);
        } else {
            x += "set background=light";
        }

        x += HI("NonText", s.fg_nontext, s.bg_background);
        x += HI("LineNr", s.fg_linenumber_fg, s.bg_linenumber_bg);
        x += HI("Conceal", s.fg_linenumber_fg, s.bg_linenumber_bg);
        x += HI("VertSplit", s.fg_vertsplit_bg, s.bg_vertsplit_fg);
        x += HI("FoldColumn", s.fg_folded_fg, s.bg_background, s.ft_none);
//  }

    x += HI("Cursor", s.fg_cursor_fg, s.bg_cursor_bg);
    x += HI("SpecialKey", s.fg_nontext);
    x += HI("Search", s.fg_search_fg, s.bg_search_bg);
    x += HI("IncSearch", s.fg_incsearch_fg, s.bg_incsearch_bg);
    x += HI("StatusLine", s.fg_statusline_active_bg, s.bg_statusline_active_fg);
    x += HI("StatusLineNC", s.fg_statusline_inactive_bg, s.bg_statusline_inactive_fg);
    x += HI("StatusLineTerm", s.fg_statusline_active_bg, s.bg_statusline_active_fg);
    x += HI("StatusLineTermNC", s.fg_statusline_inactive_bg, s.bg_statusline_inactive_fg);
    x += HI("Visual", s.fg_visual_fg, s.bg_visual_bg);
    x += HI("Directory", s.fg_blue);
    x += HI("ModeMsg", s.fg_olive);
    x += HI("MoreMsg", s.fg_olive);
    x += HI("Question", s.fg_olive);
    x += HI("WarningMsg", s.fg_pink);
    x += HI("MatchParen", s.fg_matchparen_fg, s.bg_matchparen_bg);
    x += HI("Folded", s.fg_folded_fg, s.bg_folded_bg);
    x += HI("WildMenu", s.fg_wildmenu_fg, s.bg_wildmenu_bg, s.ft_bold);

//  if (version >= 700) {
        x += HI("CursorLine", s.bg_cursorline, s.ft_none);
//      if (s.mode == s.MODE_16_COLOR) {
//          x += HI("CursorLineNr", s.fg_cursorlinenr_fg, s.bg_cursorlinenr_bg);
//      } else {
            x += HI("CursorLineNr", s.fg_cursorlinenr_fg, s.bg_cursorlinenr_bg, s.ft_none);
//      }
        x += HI("CursorColumn", s.bg_cursorcolumn, s.ft_none);
        x += HI("PMenu", s.fg_popupmenu_fg, s.bg_popupmenu_bg, s.ft_none);
        x += HI("PMenuSel", s.fg_popupmenu_fg, s.bg_popupmenu_bg, s.ft_reverse);
//      if (s.themeOpt_transparent_background) {
//          x += HI("SignColumn", s.fg_green, s.ft_none);
//      } else {
            x += HI("SignColumn", s.fg_green, s.bg_background, s.ft_none);
//      }
//  }
//  if (version >= 703) {
        x += HI("ColorColumn", s.bg_cursorcolumn, s.ft_none);
//  }

    x += HI("TabLine", s.fg_tabline_inactive_fg, s.bg_tabline_inactive_bg, s.ft_none);
    x += HI("TabLineFill", s.fg_tabline_bg, s.bg_tabline_bg, s.ft_none);
    x += HI("TabLineSel", s.fg_tabline_active_fg, s.bg_tabline_active_bg, s.ft_none);

    x += HI("BufTabLineCurrent", s.fg_buftabline_current_fg, s.bg_buftabline_current_bg, s.ft_none);
    x += HI("BufTabLineActive", s.fg_buftabline_active_fg, s.bg_buftabline_active_bg, s.ft_none);
    x += HI("BufTabLineHidden", s.fg_buftabline_inactive_fg, s.bg_buftabline_inactive_bg, s.ft_none);
    x += HI("BufTabLineFill", s.bg_buftabline_bg, s.ft_none);

    // Standard Group Highlighting:
    x += HI("Comment", s.fg_comment, s.ft_italic);

    x += HI("Constant", s.fg_orange);
    x += HI("String", s.fg_olive);
    x += HI("Character", s.fg_olive);
    x += HI("Number", s.fg_orange);
    x += HI("Boolean", s.fg_green, s.ft_bold);
    x += HI("Float", s.fg_orange);

    x += HI("Identifier", s.fg_navy);
    x += HI("Function", s.fg_foreground);

    x += HI("Statement", s.fg_pink, s.ft_none);
    x += HI("Conditional", s.fg_purple, s.ft_bold);
    x += HI("Repeat", s.fg_purple, s.ft_bold);
    x += HI("Label", s.fg_blue);
    x += HI("Operator", s.fg_aqua, s.ft_none);
    x += HI("Keyword", s.fg_blue);
    x += HI("Exception", s.fg_red);

    x += HI("PreProc", s.fg_blue);
    x += HI("Include", s.fg_red);
    x += HI("Define", s.fg_blue);
    x += HI("Macro", s.fg_blue);
    x += HI("PreCondit", s.fg_aqua);

    x += HI("Type", s.fg_pink, s.ft_bold);
    x += HI("StorageClass", s.fg_navy, s.ft_bold);
    x += HI("Structure", s.fg_blue, s.ft_bold);
    x += HI("Typedef", s.fg_pink, s.ft_bold);

    x += HI("Special", s.fg_foreground);
    x += HI("SpecialChar", s.fg_foreground);
    x += HI("Tag", s.fg_green);
    x += HI("Delimiter", s.fg_aqua);
    x += HI("SpecialComment", s.fg_comment, s.ft_bold);
    x += HI("Debug", s.fg_orange);

    x += HI("Error", s.fg_error_fg, s.bg_error_bg);
    x += HI("Todo", s.fg_todo_fg, s.bg_todo_bg, s.ft_bold);

    x += HI("Title", s.fg_comment);
    x += HI("Global", s.fg_blue);

    return x;
}


static list
apply_vim(int s)
{
    list x;

    // Neovim (LSP) diagnostics
//  if (isnvim) {
//      x += HI("LspDiagnosticsDefaultError", s.fg_error_fg, s.bg_error_bg
//      x += HI("LspDiagnosticsDefaultWarning", s.fg_todo_fg, s.bg_todo_bg, s.ft_bold
//      x += HI("LspDiagnosticsDefaultInformation", s.fg_todo_fg, s.bg_todo_bg, s.ft_bold
//      x += HI("LspDiagnosticsDefaultHint", s.fg_todo_fg, s.bg_todo_bg, s.ft_bold
//
//      x += HI("LspDiagnosticsUnderlineError cterm=undercurl gui=undercurl", s.sp_error_fg
//      x += HI("LspDiagnosticsUnderlineWarning cterm=undercurl gui=undercurl", s.sp_todo_fg
//      x += HI("LspDiagnosticsUnderlineInformation cterm=undercurl gui=undercurl", s.sp_todo_fg
//      x += HI("LspDiagnosticsUnderlineHint cterm=undercurl gui=undercurl", s.sp_todo_fg
//
//      x += "hi! link DiagnosticError LspDiagnosticsDefaultError";
//      x += "whi! link DiagnosticWarn LspDiagnosticsDefaultWarning";
//      x += "hi! link DiagnosticInfo LspDiagnosticsDefaultInformation";
//      x += "hi! link DiagnosticHint LspDiagnosticsDefaultHint";
//
//      x += "hi! link DiagnosticUnderlineError LspDiagnosticsUnderlineError";
//      x += "hi! link DiagnosticUnderlineWarn LspDiagnosticsUnderlineWarning";
//      x += "hi! link DiagnosticUnderlineInfo LspDiagnosticsUnderlineInformation";
//      x += "hi! link DiagnosticUnderlineHint LspDiagnosticsUnderlineHint";
//  }

    // Extension
    // VimL Highlighting
    x += HI("vimCommand", s.fg_pink);
    x += HI("vimVar", s.fg_navy);
    x += HI("vimFuncKey", s.fg_pink);
    x += HI("vimFunction", s.fg_blue, s.ft_bold);
    x += HI("vimNotFunc", s.fg_pink);
    x += HI("vimMap", s.fg_red);
    x += HI("vimAutoEvent", s.fg_aqua, s.ft_bold);
    x += HI("vimMapModKey", s.fg_aqua);
    x += HI("vimFuncName", s.fg_purple);
    x += HI("vimIsCommand", s.fg_foreground);
    x += HI("vimFuncVar", s.fg_aqua);
    x += HI("vimLet", s.fg_red);
    x += HI("vimContinue", s.fg_aqua);
    x += HI("vimMapRhsExtend", s.fg_foreground);
    x += HI("vimCommentTitle", s.fg_comment, s.ft_italic_bold);
    x += HI("vimBracket", s.fg_aqua);
    x += HI("vimParenSep", s.fg_aqua);
    x += HI("vimNotation", s.fg_aqua);
    x += HI("vimOper", s.fg_foreground);
    x += HI("vimOperParen", s.fg_foreground);
    x += HI("vimSynType", s.fg_purple);
    x += HI("vimSynReg", s.fg_pink, s.ft_none);
    x += HI("vimSynRegion", s.fg_foreground);
    x += HI("vimSynMtchGrp", s.fg_pink);
    x += HI("vimSynNextgroup", s.fg_pink);
    x += HI("vimSynKeyRegion", s.fg_green);
    x += HI("vimSynRegOpt", s.fg_blue);
    x += HI("vimSynMtchOpt", s.fg_blue);
    x += HI("vimSynContains", s.fg_pink);
    x += HI("vimGroupName", s.fg_foreground);
    x += HI("vimGroupList", s.fg_foreground);
    x += HI("vimHiGroup", s.fg_foreground);
    x += HI("vimGroup", s.fg_navy, s.ft_bold);
    x += HI("vimOnlyOption", s.fg_blue);

    return x;
}


static list
apply_make(int s)
{
    list x;

    // Makefile Highlighting
    x += HI("makeIdent", s.fg_blue);
    x += HI("makeSpecTarget", s.fg_olive);
    x += HI("makeTarget", s.fg_red);
    x += HI("makeStatement", s.fg_aqua, s.ft_bold);
    x += HI("makeCommands", s.fg_foreground);
    x += HI("makeSpecial", s.fg_orange, s.ft_bold);

    // CMake Highlighting (Builtin)
    x += HI("cmakeStatement", s.fg_blue);
    x += HI("cmakeArguments", s.fg_foreground);
    x += HI("cmakeVariableValue", s.fg_pink);

    // CMake Highlighting (Plugin: https.//github.com/pboettch/vim-cmake-syntax)
    x += HI("cmakeCommand", s.fg_blue);
    x += HI("cmakeCommandConditional", s.fg_purple, s.ft_bold);
    x += HI("cmakeKWset", s.fg_orange);
    x += HI("cmakeKWvariable_watch", s.fg_orange);
    x += HI("cmakeKWif", s.fg_orange);
    x += HI("cmakeArguments", s.fg_foreground);
    x += HI("cmakeKWproject", s.fg_pink);
    x += HI("cmakeGeneratorExpressions", s.fg_orange);
    x += HI("cmakeGeneratorExpression", s.fg_aqua);
    x += HI("cmakeVariable", s.fg_pink);
    x += HI("cmakeProperty", s.fg_aqua);
    x += HI("cmakeKWforeach", s.fg_aqua);
    x += HI("cmakeKWunset", s.fg_aqua);
    x += HI("cmakeKWmacro", s.fg_aqua);
    x += HI("cmakeKWget_property", s.fg_aqua);
    x += HI("cmakeKWset_tests_properties", s.fg_aqua);
    x += HI("cmakeKWmessage", s.fg_aqua);
    x += HI("cmakeKWinstall_targets", s.fg_orange);
    x += HI("cmakeKWsource_group", s.fg_orange);
    x += HI("cmakeKWfind_package", s.fg_aqua);
    x += HI("cmakeKWstring", s.fg_olive);
    x += HI("cmakeKWinstall", s.fg_aqua);
    x += HI("cmakeKWtarget_sources", s.fg_orange);

    return x;
}


static list
apply_c(int s)
{
    list x;

    // C Highlighting
    x += HI("cType", s.fg_pink, s.ft_bold);
    x += HI("cFormat", s.fg_olive);
    x += HI("cStorageClass", s.fg_navy, s.ft_bold);
    x += HI("cBoolean", s.fg_green, s.ft_bold);
    x += HI("cCharacter", s.fg_olive);
    x += HI("cConstant", s.fg_green, s.ft_bold);
    x += HI("cConditional", s.fg_purple, s.ft_bold);
    x += HI("cSpecial", s.fg_olive, s.ft_bold);
    x += HI("cDefine", s.fg_blue);
    x += HI("cNumber", s.fg_orange);
    x += HI("cPreCondit", s.fg_aqua);
    x += HI("cRepeat", s.fg_purple, s.ft_bold);
    x += HI("cLabel", s.fg_aqua);
 // x += HI("cAnsiFunction", s.fg_aqua, s.ft_bold);
 // x += HI("cAnsiName", s.fg_pink);
    x += HI("cDelimiter", s.fg_blue);
 // x += HI("cBraces", s.fg_foreground);
 // x += HI("cIdentifier", s.fg_blue, s.bg_pink);
 // x += HI("cSemiColon", s.bg_blue);
    x += HI("cOperator", s.fg_aqua);
 // x += HI("cStatement", s.fg_pink);
 // x += HI("cTodo", s.fg_comment, s.ft_bold);
 // x += HI("cStructure", s.fg_blue, s.ft_bold);
    x += HI("cCustomParen", s.fg_foreground);
 // x += HI("cCustomFunc", s.fg_foreground);
 // x += HI("cUserFunction", s.fg_blue, s.ft_bold);
    x += HI("cOctalZero", s.fg_purple, s.ft_bold);
//TODO
//  if (h.langOpt_c__highlight_builtins == 1) {
        x += HI("cFunction", s.fg_blue);
//  } else {
//      x += HI("cFunction", s.fg_foreground);
//  }

    // CPP highlighting
    x += HI("cppBoolean", s.fg_green, s.ft_bold);
    x += HI("cppSTLnamespace", s.fg_purple);
    x += HI("cppSTLexception", s.fg_pink);
    x += HI("cppSTLfunctional", s.fg_foreground, s.ft_bold);
    x += HI("cppSTLiterator", s.fg_foreground, s.ft_bold);
    x += HI("cppExceptions", s.fg_red);
    x += HI("cppStatement", s.fg_blue);
    x += HI("cppStorageClass", s.fg_navy, s.ft_bold);
    x += HI("cppAccess", s.fg_orange, s.ft_bold);
//TODO
//  if (h.langOpt_cpp__highlight_standard_library == 1) {
        x += HI("cppSTLconstant", s.fg_green, s.ft_bold);
        x += HI("cppSTLtype", s.fg_pink, s.ft_bold);
        x += HI("cppSTLfunction", s.fg_blue);
        x += HI("cppSTLios", s.fg_olive, s.ft_bold);
//  } else {
//      x += HI("cppSTLconstant", s.fg_foreground);
//      x += HI("cppSTLtype", s.fg_foreground);
//      x += HI("cppSTLfunction", s.fg_foreground);
//      x += HI("cppSTLios", s.fg_foreground);
//  }
    // x += HI("cppSTL", s.fg_blue

    // Rust highlighting
    x += HI("rustKeyword", s.fg_pink);
    x += HI("rustModPath", s.fg_blue);
    x += HI("rustModPathSep", s.fg_blue);
    x += HI("rustLifetime", s.fg_purple);
    x += HI("rustStructure", s.fg_aqua, s.ft_bold);
    x += HI("rustAttribute", s.fg_aqua, s.ft_bold);
    x += HI("rustPanic", s.fg_olive, s.ft_bold);
    x += HI("rustTrait", s.fg_blue, s.ft_bold);
    x += HI("rustEnum", s.fg_green, s.ft_bold);
    x += HI("rustEnumVariant", s.fg_green);
    x += HI("rustSelf", s.fg_orange);
    x += HI("rustSigil", s.fg_aqua, s.ft_bold);
    x += HI("rustOperator", s.fg_aqua, s.ft_bold);
    x += HI("rustMacro", s.fg_olive, s.ft_bold);
    x += HI("rustMacroVariable", s.fg_olive);
    x += HI("rustAssert", s.fg_olive, s.ft_bold);
    x += HI("rustConditional", s.fg_purple, s.ft_bold);
}


static list
apply_lex(int s)
{
    list x;

    // Lex highlighting
    x += HI("lexCFunctions", s.fg_foreground);
    x += HI("lexAbbrv", s.fg_purple);
    x += HI("lexAbbrvRegExp", s.fg_aqua);
    x += HI("lexAbbrvComment", s.fg_comment);
    x += HI("lexBrace", s.fg_navy);
    x += HI("lexPat", s.fg_aqua);
    x += HI("lexPatComment", s.fg_comment);
    x += HI("lexPatTag", s.fg_orange);
 // x += HI("lexPatBlock", s.fg_foreground, s.ft_bold);
    x += HI("lexSlashQuote", s.fg_foreground);
    x += HI("lexSep", s.fg_foreground);
    x += HI("lexStartState", s.fg_orange);
    x += HI("lexPatTagZone", s.fg_olive, s.ft_bold);
    x += HI("lexMorePat", s.fg_olive, s.ft_bold);
    x += HI("lexOptions", s.fg_olive, s.ft_bold);
    x += HI("lexPatString", s.fg_olive);

    // Yacc highlighting
    x += HI("yaccNonterminal", s.fg_navy);
    x += HI("yaccDelim", s.fg_orange);
    x += HI("yaccInitKey", s.fg_aqua);
    x += HI("yaccInit", s.fg_navy);
    x += HI("yaccKey", s.fg_purple);
    x += HI("yaccVar", s.fg_aqua);

    return x;
}


static list
apply_asm(int s)
{
    list x;

    // NASM highlighting
    x += HI("nasmStdInstruction", s.fg_navy);
    x += HI("nasmGen08Register", s.fg_aqua);
    x += HI("nasmGen16Register", s.fg_aqua);
    x += HI("nasmGen32Register", s.fg_aqua);
    x += HI("nasmGen64Register", s.fg_aqua);
    x += HI("nasmHexNumber", s.fg_purple);
    x += HI("nasmStorage", s.fg_aqua, s.ft_bold);
    x += HI("nasmLabel", s.fg_pink);
    x += HI("nasmDirective", s.fg_blue, s.ft_bold);
    x += HI("nasmLocalLabel", s.fg_orange);

    // GAS highlighting
    x += HI("gasSymbol", s.fg_pink);
    x += HI("gasDirective", s.fg_blue, s.ft_bold);
    x += HI("gasOpcode_386_Base", s.fg_navy);
    x += HI("gasDecimalNumber", s.fg_purple);
    x += HI("gasSymbolRef", s.fg_pink);
    x += HI("gasRegisterX86", s.fg_blue);
    x += HI("gasOpcode_P6_Base", s.fg_navy);
    x += HI("gasDirectiveStore", s.fg_foreground, s.ft_bold);

    // MIPS highlighting
    x += HI("mipsInstruction", s.fg_pink);
    x += HI("mipsRegister", s.fg_navy);
    x += HI("mipsLabel", s.fg_aqua, s.ft_bold);
    x += HI("mipsDirective", s.fg_purple, s.ft_bold);

    return x;
}


static list
apply_shell(int s)
{
    list x;

    // Shell/Bash highlighting
    x += HI("bashStatement", s.fg_foreground, s.ft_bold);
    x += HI("shDerefVar", s.fg_aqua, s.ft_bold);
    x += HI("shDerefSimple", s.fg_aqua);
    x += HI("shFunction", s.fg_orange, s.ft_bold);
    x += HI("shStatement", s.fg_foreground);
    x += HI("shLoop", s.fg_purple, s.ft_bold);
    x += HI("shQuote", s.fg_olive);
    x += HI("shCaseEsac", s.fg_aqua, s.ft_bold);
    x += HI("shSnglCase", s.fg_purple, s.ft_none);
    x += HI("shFunctionOne", s.fg_navy);
    x += HI("shCase", s.fg_navy);
    x += HI("shSetList", s.fg_navy);
    // @see Dockerfile Highlighting section for more sh*

    // PowerShell Highlighting
    x += HI("ps1Type", s.fg_green, s.ft_bold);
    x += HI("ps1Variable", s.fg_navy);
    x += HI("ps1Boolean", s.fg_navy, s.ft_bold);
    x += HI("ps1FunctionInvocation", s.fg_pink);
    x += HI("ps1FunctionDeclaration", s.fg_pink);
    x += HI("ps1Keyword", s.fg_blue, s.ft_bold);
    x += HI("ps1Exception", s.fg_red);
    x += HI("ps1Operator", s.fg_aqua, s.ft_bold);
    x += HI("ps1CommentDoc", s.fg_purple);
    x += HI("ps1CDocParam", s.fg_orange);

    return x;
}


static list
apply_markup(int s)
{
    list x;

    // HTML Highlighting
    x += HI("htmlTitle", s.fg_green, s.ft_bold);
    x += HI("htmlH1", s.fg_green, s.ft_bold);
    x += HI("htmlH2", s.fg_aqua, s.ft_bold);
    x += HI("htmlH3", s.fg_purple, s.ft_bold);
    x += HI("htmlH4", s.fg_orange, s.ft_bold);
    x += HI("htmlTag", s.fg_comment);
    x += HI("htmlTagName", s.fg_wine);
    x += HI("htmlArg", s.fg_pink);
    x += HI("htmlEndTag", s.fg_comment);
    x += HI("htmlString", s.fg_blue);
    x += HI("htmlScriptTag", s.fg_comment);
    x += HI("htmlBold", s.fg_foreground, s.ft_bold);
    x += HI("htmlItalic", s.fg_comment, s.ft_italic);
    x += HI("htmlBoldItalic", s.fg_navy, s.ft_italic_bold);
 // x += HI("htmlLink", s.fg_blue, s.ft_bold
    x += HI("htmlTagN", s.fg_wine, s.ft_bold);
    x += HI("htmlSpecialTagName", s.fg_wine);
    x += HI("htmlComment", s.fg_comment, s.ft_italic);
    x += HI("htmlCommentPart", s.fg_comment, s.ft_italic);

    // CSS Highlighting
    x += HI("cssIdentifier", s.fg_pink);
    x += HI("cssPositioningProp", s.fg_foreground);
    x += HI("cssNoise", s.fg_foreground);
    x += HI("cssBoxProp", s.fg_foreground);
    x += HI("cssTableAttr", s.fg_purple);
    x += HI("cssPositioningAttr", s.fg_navy);
    x += HI("cssValueLength", s.fg_orange);
    x += HI("cssFunctionName", s.fg_blue);
    x += HI("cssUnitDecorators", s.fg_aqua);
    x += HI("cssColor", s.fg_blue, s.ft_bold);
    x += HI("cssBraces", s.fg_pink);
    x += HI("cssBackgroundProp", s.fg_foreground);
    x += HI("cssTextProp", s.fg_foreground);
    x += HI("cssDimensionProp", s.fg_foreground);
    x += HI("cssClassName", s.fg_pink);

    // Markdown Highlighting
    x += HI("markdownHeadingRule", s.fg_pink, s.ft_bold);
    x += HI("markdownH1", s.fg_pink, s.ft_bold);
    x += HI("markdownH2", s.fg_orange, s.ft_bold);
    x += HI("markdownBlockquote", s.fg_pink);
    x += HI("markdownCodeBlock", s.fg_olive);
    x += HI("markdownCode", s.fg_olive);
    x += HI("markdownLink", s.fg_blue, s.ft_bold);
    x += HI("markdownUrl", s.fg_blue);
    x += HI("markdownLinkText", s.fg_pink);
    x += HI("markdownLinkTextDelimiter", s.fg_purple);
    x += HI("markdownLinkDelimiter", s.fg_purple);
    x += HI("markdownCodeDelimiter", s.fg_blue);

    x += HI("mkdCode", s.fg_olive);
    x += HI("mkdLink", s.fg_blue, s.ft_bold);
    x += HI("mkdURL", s.fg_comment);
    x += HI("mkdString", s.fg_foreground);
    x += HI("mkdBlockQuote", s.fg_pink);
    x += HI("mkdLinkTitle", s.fg_pink);
    x += HI("mkdDelimiter", s.fg_aqua);
    x += HI("mkdRule", s.fg_pink);

    // reStructuredText Highlighting
    x += HI("rstSections", s.fg_pink, s.ft_bold);
    x += HI("rstDelimiter", s.fg_pink, s.ft_bold);
    x += HI("rstExplicitMarkup", s.fg_pink, s.ft_bold);
    x += HI("rstDirective", s.fg_blue);
    x += HI("rstHyperlinkTarget", s.fg_green);
    x += HI("rstExDirective", s.fg_foreground);
    x += HI("rstInlineLiteral", s.fg_olive);
    x += HI("rstInterpretedTextOrHyperlinkReference", s.fg_blue);

    return x;
}


static list
apply_py(int s)
{
    list x;

    // Python Highlighting
    x += HI("pythonImport", s.fg_pink, s.ft_bold);
    x += HI("pythonExceptions", s.fg_red);
    x += HI("pythonException", s.fg_purple, s.ft_bold);
    x += HI("pythonInclude", s.fg_red);
    x += HI("pythonStatement", s.fg_pink);
    x += HI("pythonConditional", s.fg_purple, s.ft_bold);
    x += HI("pythonRepeat", s.fg_purple, s.ft_bold);
    x += HI("pythonFunction", s.fg_aqua, s.ft_bold);
    x += HI("pythonPreCondit", s.fg_purple);
    x += HI("pythonExClass", s.fg_orange);
    x += HI("pythonOperator", s.fg_purple, s.ft_bold);
    x += HI("pythonBuiltin", s.fg_foreground);
    x += HI("pythonDecorator", s.fg_orange);

    x += HI("pythonString", s.fg_olive);
    x += HI("pythonEscape", s.fg_olive, s.ft_bold);
    x += HI("pythonStrFormatting", s.fg_olive, s.ft_bold);

    x += HI("pythonBoolean", s.fg_green, s.ft_bold);
    x += HI("pythonBytesEscape", s.fg_olive, s.ft_bold);
    x += HI("pythonDottedName", s.fg_purple);
    x += HI("pythonStrFormat", s.fg_foreground);
//TODO
//  if (h.langOpt_python__highlight_builtins == 1) {
        x += HI("pythonBuiltinFunc", s.fg_blue);
        x += HI("pythonBuiltinObj", s.fg_red);
//  } else {
//      x += HI("pythonBuiltinFunc", s.fg_foreground);
//      x += HI("pythonBuiltinObj", s.fg_foreground);
//  }

    return x;
}


static list
apply_java(int s)
{
    list x;

    // Java Highlighting
    x += HI("javaExternal", s.fg_pink);
    x += HI("javaAnnotation", s.fg_orange);
    x += HI("javaTypedef", s.fg_aqua);
    x += HI("javaClassDecl", s.fg_aqua, s.ft_bold);
    x += HI("javaScopeDecl", s.fg_blue, s.ft_bold);
    x += HI("javaStorageClass", s.fg_navy, s.ft_bold);
    x += HI("javaBoolean", s.fg_green, s.ft_bold);
    x += HI("javaConstant", s.fg_blue);
    x += HI("javaCommentTitle", s.fg_wine);
    x += HI("javaDocTags", s.fg_aqua);
    x += HI("javaDocComment", s.fg_comment);
    x += HI("javaDocParam", s.fg_foreground);
    x += HI("javaStatement", s.fg_pink);

    // JavaScript Highlighting
    x += HI("javaScriptBraces", s.fg_blue);
    x += HI("javaScriptParens", s.fg_blue);
    x += HI("javaScriptIdentifier", s.fg_pink);
    x += HI("javaScriptFunction", s.fg_blue, s.ft_bold);
    x += HI("javaScriptConditional", s.fg_purple, s.ft_bold);
    x += HI("javaScriptRepeat", s.fg_purple, s.ft_bold);
    x += HI("javaScriptBoolean", s.fg_green, s.ft_bold);
    x += HI("javaScriptNumber", s.fg_orange);
    x += HI("javaScriptMember", s.fg_navy);
    x += HI("javaScriptReserved", s.fg_navy);
    x += HI("javascriptNull", s.fg_comment, s.ft_bold);
    x += HI("javascriptGlobal", s.fg_foreground);
    x += HI("javascriptStatement", s.fg_pink);
    x += HI("javaScriptMessage", s.fg_foreground);
    x += HI("javaScriptMember", s.fg_foreground);

    // TypeScript Highlighting
    x += HI("typescriptDecorators", s.fg_orange);
    x += HI("typescriptLabel", s.fg_purple, s.ft_bold);

    return x;
}


static list
apply_js(int s)
{
    list x;

    // @target https.//github.com/pangloss/vim-javascript
    x += HI("jsImport", s.fg_pink, s.ft_bold);
    x += HI("jsExport", s.fg_pink, s.ft_bold);
    x += HI("jsModuleAs", s.fg_pink, s.ft_bold);
    x += HI("jsFrom", s.fg_pink, s.ft_bold);
    x += HI("jsExportDefault", s.fg_pink, s.ft_bold);
    x += HI("jsFuncParens", s.fg_blue);
    x += HI("jsFuncBraces", s.fg_blue);
    x += HI("jsParens", s.fg_blue);
    x += HI("jsBraces", s.fg_blue);
    x += HI("jsNoise", s.fg_blue);

    // Jsx Highlighting);
    // @target https.//github.com/MaxMEllon/vim-jsx-pretty);
    x += HI("jsxTagName", s.fg_wine);
    x += HI("jsxComponentName", s.fg_wine);
    x += HI("jsxAttrib", s.fg_pink);
    x += HI("jsxEqual", s.fg_comment);
    x += HI("jsxString", s.fg_blue);
    x += HI("jsxCloseTag", s.fg_comment);
    x += HI("jsxCloseString", s.fg_comment);
    x += HI("jsxDot", s.fg_wine);
    x += HI("jsxNamespace", s.fg_wine);
    x += HI("jsxPunct", s.fg_comment);

    // Json Highlighting
    // @target https.//github.com/elzr/vim-json
    x += HI("jsonKeyword", s.fg_blue);
    x += HI("jsonString", s.fg_olive);
    x += HI("jsonQuote", s.fg_comment);
    x += HI("jsonNoise", s.fg_foreground);
    x += HI("jsonKeywordMatch", s.fg_foreground);
    x += HI("jsonBraces", s.fg_foreground);
    x += HI("jsonNumber", s.fg_orange);
    x += HI("jsonNull", s.fg_purple, s.ft_bold);
    x += HI("jsonBoolean", s.fg_green, s.ft_bold);
    x += HI("jsonCommentError", s.fg_pink, s.bg_background);

    return x;
}


static list
apply_go(int s)
{
    list x;

    // Go Highlighting
    x += HI("goDirective", s.fg_red);
    x += HI("goDeclaration", s.fg_blue, s.ft_bold);
    x += HI("goStatement", s.fg_pink);
    x += HI("goConditional", s.fg_purple, s.ft_bold);
    x += HI("goConstants", s.fg_orange);
    x += HI("goFunction", s.fg_orange);
 // x += HI("goTodo", s.fg_comment, s.ft_bold);
    x += HI("goDeclType", s.fg_blue);
    x += HI("goBuiltins", s.fg_purple);

    return x;
}


static list
apply_stap(int s)
{
    list x;

    // Systemtap Highlighting
  // x += HI("stapBlock", s.fg_comment, s.ft_none
    x += HI("stapComment", s.fg_comment, s.ft_none);
    x += HI("stapProbe", s.fg_aqua, s.ft_bold);
    x += HI("stapStat", s.fg_navy, s.ft_bold);
    x += HI("stapFunc", s.fg_foreground);
    x += HI("stapString", s.fg_olive);
    x += HI("stapTarget", s.fg_navy);
    x += HI("stapStatement", s.fg_pink);
    x += HI("stapType", s.fg_pink, s.ft_bold);
    x += HI("stapSharpBang", s.fg_comment);
    x += HI("stapDeclaration", s.fg_pink);
    x += HI("stapCMacro", s.fg_blue);

    return x;
}


static list
apply_dtrace(int s)
{
    list x;

    // DTrace Highlighting
    x += HI("dtraceProbe", s.fg_blue);
    x += HI("dtracePredicate", s.fg_purple, s.ft_bold);
    x += HI("dtraceComment", s.fg_comment);
    x += HI("dtraceFunction", s.fg_foreground);
    x += HI("dtraceAggregatingFunction", s.fg_blue, s.ft_bold);
    x += HI("dtraceStatement", s.fg_navy, s.ft_bold);
    x += HI("dtraceIdentifier", s.fg_pink);
    x += HI("dtraceOption", s.fg_pink);
    x += HI("dtraceConstant", s.fg_orange);
    x += HI("dtraceType", s.fg_pink, s.ft_bold);

    return x;
}


static list
apply_plantuml(int s)
{
    list x;

    // PlantUML Highlighting
    x += HI("plantumlPreProc", s.fg_orange, s.ft_bold);
    x += HI("plantumlDirectedOrVerticalArrowRL", s.fg_pink);
    x += HI("plantumlDirectedOrVerticalArrowLR", s.fg_pink);
    x += HI("plantumlString", s.fg_olive);
    x += HI("plantumlActivityThing", s.fg_purple);
    x += HI("plantumlText", s.fg_navy);
    x += HI("plantumlClassPublic", s.fg_olive, s.ft_bold);
    x += HI("plantumlClassPrivate", s.fg_red);
    x += HI("plantumlColonLine", s.fg_orange);
    x += HI("plantumlClass", s.fg_navy);
    x += HI("plantumlHorizontalArrow", s.fg_pink);
    x += HI("plantumlTypeKeyword", s.fg_blue, s.ft_bold);
    x += HI("plantumlKeyword", s.fg_pink, s.ft_bold);

    x += HI("plantumlType", s.fg_blue, s.ft_bold);
    x += HI("plantumlBlock", s.fg_pink, s.ft_bold);
    x += HI("plantumlPreposition", s.fg_orange);
    x += HI("plantumlLayout", s.fg_blue, s.ft_bold);
    x += HI("plantumlNote", s.fg_orange);
    x += HI("plantumlLifecycle", s.fg_aqua);
    x += HI("plantumlParticipant", s.fg_foreground, s.ft_bold);

    return x;
}


static list
apply_haskell(int s)
{
    list x;

    // Haskell Highlighting
//TODO
//  if (h.langOpt_haskell__no_bold_types == 1) {
        x += HI("haskellType", s.fg_aqua);
//  } else {
//      x += HI("haskellType", s.fg_aqua, s.ft_bold);
//  }
    x += HI("haskellIdentifier", s.fg_orange, s.ft_bold);
    x += HI("haskellOperators", s.fg_pink);
    x += HI("haskellWhere", s.fg_foreground, s.ft_bold);
    x += HI("haskellDelimiter", s.fg_aqua);
    x += HI("haskellImportKeywords", s.fg_pink);
    x += HI("haskellStatement", s.fg_purple, s.ft_bold);

    return x;
}


static list
apply_sql(int s)
{
    list x;

    // SQL/MySQL Highlighting
    x += HI("sqlStatement", s.fg_pink, s.ft_bold);
    x += HI("sqlType", s.fg_blue, s.ft_bold);
    x += HI("sqlKeyword", s.fg_pink);
    x += HI("sqlOperator", s.fg_aqua);
    x += HI("sqlSpecial", s.fg_green, s.ft_bold);

    x += HI("mysqlVariable", s.fg_olive, s.ft_bold);
    x += HI("mysqlType", s.fg_blue, s.ft_bold);
    x += HI("mysqlKeyword", s.fg_pink);
    x += HI("mysqlOperator", s.fg_aqua);
    x += HI("mysqlSpecial", s.fg_green, s.ft_bold);

    return x;
}


static list
apply_octave(int s)
{
    list x;

    // Octave/MATLAB Highlighting
    x += HI("octaveVariable", s.fg_foreground);
    x += HI("octaveDelimiter", s.fg_pink);
    x += HI("octaveQueryVar", s.fg_foreground);
    x += HI("octaveSemicolon", s.fg_purple);
    x += HI("octaveFunction", s.fg_navy);
    x += HI("octaveSetVar", s.fg_blue);
    x += HI("octaveUserVar", s.fg_foreground);
    x += HI("octaveArithmeticOperator", s.fg_aqua);
    x += HI("octaveBeginKeyword", s.fg_purple, s.ft_bold);
    x += HI("octaveElseKeyword", s.fg_purple, s.ft_bold);
    x += HI("octaveEndKeyword", s.fg_purple, s.ft_bold);
    x += HI("octaveStatement", s.fg_pink);

    return x;
}


static list
apply_ruby(int s)
{
    list x;

    // Ruby Highlighting
    x += HI("rubyModule", s.fg_navy, s.ft_bold);
    x += HI("rubyClass", s.fg_pink, s.ft_bold);
    x += HI("rubyPseudoVariable", s.fg_comment, s.ft_bold);
    x += HI("rubyKeyword", s.fg_pink);
    x += HI("rubyInstanceVariable", s.fg_purple);
    x += HI("rubyFunction", s.fg_foreground, s.ft_bold);
    x += HI("rubyDefine", s.fg_pink);
    x += HI("rubySymbol", s.fg_aqua);
    x += HI("rubyConstant", s.fg_blue);
    x += HI("rubyAccess", s.fg_navy);
    x += HI("rubyAttribute", s.fg_green);
    x += HI("rubyInclude", s.fg_red);
    x += HI("rubyLocalVariableOrMethod", s.fg_orange);
    x += HI("rubyCurlyBlock", s.fg_foreground);
    x += HI("rubyCurlyBlockDelimiter", s.fg_aqua);
    x += HI("rubyArrayDelimiter", s.fg_aqua);
    x += HI("rubyStringDelimiter", s.fg_olive);
    x += HI("rubyInterpolationDelimiter", s.fg_orange);
    x += HI("rubyConditional", s.fg_purple, s.ft_bold);
    x += HI("rubyRepeat", s.fg_purple, s.ft_bold);
    x += HI("rubyControl", s.fg_purple, s.ft_bold);
    x += HI("rubyException", s.fg_purple, s.ft_bold);
    x += HI("rubyExceptional", s.fg_purple, s.ft_bold);
    x += HI("rubyBoolean", s.fg_green, s.ft_bold);

    return x;
}


static list
apply_fortran(int s)
{
    list x;

    // Fortran Highlighting
    x += HI("fortranUnitHeader", s.fg_blue, s.ft_bold);
    x += HI("fortranIntrinsic", s.fg_blue, s.bg_background, s.ft_none);
    x += HI("fortranType", s.fg_pink, s.ft_bold);
    x += HI("fortranTypeOb", s.fg_pink, s.ft_bold);
    x += HI("fortranStructure", s.fg_aqua);
    x += HI("fortranStorageClass", s.fg_navy, s.ft_bold);
    x += HI("fortranStorageClassR", s.fg_navy, s.ft_bold);
    x += HI("fortranKeyword", s.fg_pink);
    x += HI("fortranReadWrite", s.fg_aqua, s.ft_bold);
    x += HI("fortranIO", s.fg_navy);
    x += HI("fortranOperator", s.fg_aqua, s.ft_bold);
    x += HI("fortranCall", s.fg_aqua, s.ft_bold);
    x += HI("fortranContinueMark", s.fg_green);

    return x;
}


static list
apply_algol(int s)
{
    list x;

    // ALGOL Highlighting (Plugin: https.//github.com/sterpe/vim-algol68)
    x += HI("algol68Statement", s.fg_blue, s.ft_bold);
    x += HI("algol68Operator", s.fg_aqua, s.ft_bold);
    x += HI("algol68PreProc", s.fg_green);
    x += HI("algol68Function", s.fg_blue);

    return x;
}


static list
apply_r(int s)
{
    list x;

    // R Highlighting
    x += HI("rType", s.fg_blue);
    x += HI("rArrow", s.fg_pink);
    x += HI("rDollar", s.fg_blue);

    return x;
}


static list
apply_xxd(int s)
{
    list x;

    // XXD Highlighting
    x += HI("xxdAddress", s.fg_navy);
    x += HI("xxdSep", s.fg_pink);
    x += HI("xxdAscii", s.fg_pink);
    x += HI("xxdDot", s.fg_aqua);

    return x;
}


static list
apply_php(int s)
{
    list x;

    // PHP Highlighting
    x += HI("phpIdentifier", s.fg_foreground);
    x += HI("phpVarSelector", s.fg_pink);
    x += HI("phpKeyword", s.fg_blue);
    x += HI("phpRepeat", s.fg_purple, s.ft_bold);
    x += HI("phpConditional", s.fg_purple, s.ft_bold);
    x += HI("phpStatement", s.fg_pink);
    x += HI("phpAssignByRef", s.fg_aqua, s.ft_bold);
    x += HI("phpSpecialFunction", s.fg_blue);
    x += HI("phpFunctions", s.fg_blue);
    x += HI("phpComparison", s.fg_aqua);
    x += HI("phpBackslashSequences", s.fg_olive, s.ft_bold);
    x += HI("phpMemberSelector", s.fg_blue);
    x += HI("phpStorageClass", s.fg_purple, s.ft_bold);
    x += HI("phpDefine", s.fg_navy);
    x += HI("phpIntVar", s.fg_navy, s.ft_bold);

    return x;
}


static list
apply_perl(int s)
{
    list x;

    // Perl Highlighting
    x += HI("perlFiledescRead", s.fg_green);
    x += HI("perlMatchStartEnd", s.fg_pink);
    x += HI("perlStatementFlow", s.fg_pink);
    x += HI("perlStatementStorage", s.fg_pink);
    x += HI("perlFunction", s.fg_pink, s.ft_bold);
    x += HI("perlMethod", s.fg_foreground);
    x += HI("perlStatementFiledesc", s.fg_orange);
    x += HI("perlVarPlain", s.fg_navy);
    x += HI("perlSharpBang", s.fg_comment);
    x += HI("perlStatementInclude", s.fg_aqua, s.ft_bold);
    x += HI("perlStatementScalar", s.fg_purple);
    x += HI("perlSubName", s.fg_aqua, s.ft_bold);
    x += HI("perlSpecialString", s.fg_olive, s.ft_bold);

    return x;
}


static list
apply_pascal(int s)
{
    list x;

    // Pascal Highlighting
    x += HI("pascalType", s.fg_pink, s.ft_bold);
    x += HI("pascalStatement", s.fg_blue, s.ft_bold);
    x += HI("pascalPredefined", s.fg_pink);
    x += HI("pascalFunction", s.fg_foreground);
    x += HI("pascalStruct", s.fg_navy, s.ft_bold);
    x += HI("pascalOperator", s.fg_aqua, s.ft_bold);
    x += HI("pascalPreProc", s.fg_green);
    x += HI("pascalAcces", s.fg_navy, s.ft_bold);

    return x;
}


static list
apply_lua(int s)
{
    list x;

    // Lua Highlighting
    x += HI("luaFunc", s.fg_foreground);
    x += HI("luaIn", s.fg_blue, s.ft_bold);
    x += HI("luaFunction", s.fg_pink);
    x += HI("luaStatement", s.fg_blue);
    x += HI("luaRepeat", s.fg_blue, s.ft_bold);
    x += HI("luaCondStart", s.fg_purple, s.ft_bold);
    x += HI("luaTable", s.fg_aqua, s.ft_bold);
    x += HI("luaConstant", s.fg_green, s.ft_bold);
    x += HI("luaElse", s.fg_purple, s.ft_bold);
    x += HI("luaCondElseif", s.fg_purple, s.ft_bold);
    x += HI("luaCond", s.fg_purple, s.ft_bold);
    x += HI("luaCondEnd", s.fg_purple);

    return x;
}


static list
apply_clojure(int s)
{
    list x;

    // Clojure highlighting:
    x += HI("clojureConstant", s.fg_blue);
    x += HI("clojureBoolean", s.fg_orange);
    x += HI("clojureCharacter", s.fg_olive);
    x += HI("clojureKeyword", s.fg_pink);
    x += HI("clojureNumber", s.fg_orange);
    x += HI("clojureString", s.fg_olive);
    x += HI("clojureRegexp", s.fg_purple);
    x += HI("clojureRegexpEscape", s.fg_pink);
    x += HI("clojureParen", s.fg_aqua);
    x += HI("clojureVariable", s.fg_olive);
    x += HI("clojureCond", s.fg_blue);
    x += HI("clojureDefine", s.fg_blue, s.ft_bold);
    x += HI("clojureException", s.fg_red);
    x += HI("clojureFunc", s.fg_navy);
    x += HI("clojureMacro", s.fg_blue);
    x += HI("clojureRepeat", s.fg_blue);
    x += HI("clojureSpecial", s.fg_blue, s.ft_bold);
    x += HI("clojureQuote", s.fg_blue);
    x += HI("clojureUnquote", s.fg_blue);
    x += HI("clojureMeta", s.fg_blue);
    x += HI("clojureDeref", s.fg_blue);
    x += HI("clojureAnonArg", s.fg_blue);
    x += HI("clojureRepeat", s.fg_blue);
    x += HI("clojureDispatch", s.fg_aqua);

    return x;
}


static list
apply_docker(int s)
{
    list x;

    // Dockerfile Highlighting
    // @target https.//github.com/docker/docker/tree/master/contrib/syntax/vim
    x += HI("dockerfileKeyword", s.fg_blue);
    x += HI("shDerefVar", s.fg_purple, s.ft_bold);
    x += HI("shOperator", s.fg_aqua);
    x += HI("shOption", s.fg_navy);
    x += HI("shLine", s.fg_foreground);
    x += HI("shWrapLineOperator", s.fg_pink);

    return x;
}


static list
apply_ngx(int s)
{
    list x;

    // NGINX Highlighting
    // @target https.//github.com/evanmiller/nginx-vim-syntax
    x += HI("ngxDirectiveBlock", s.fg_pink, s.ft_bold);
    x += HI("ngxDirective", s.fg_blue, s.ft_none);
    x += HI("ngxDirectiveImportant", s.fg_blue, s.ft_bold);
    x += HI("ngxString", s.fg_olive);
    x += HI("ngxVariableString", s.fg_purple);
    x += HI("ngxVariable", s.fg_purple, s.ft_none);

    return x;
}


static list
apply_ymal(int s)
{
    list x;

    // Yaml Highlighting
    x += HI("yamlBlockMappingKey", s.fg_blue);
    x += HI("yamlKeyValueDelimiter", s.fg_pink);
    x += HI("yamlBlockCollectionItemStart", s.fg_pink);

    return x;
}


static list
apply_qml(int s)
{
    list x;

    // Qt QML Highlighting
    x += HI("qmlObjectLiteralType", s.fg_pink);
    x += HI("qmlReserved", s.fg_purple);
    x += HI("qmlBindingProperty", s.fg_navy);
    x += HI("qmlType", s.fg_navy);

    return x;
}


static list
apply_dosini(int s)
{
    list x;

    // Dosini Highlighting
    x += HI("dosiniHeader", s.fg_pink);
    x += HI("dosiniLabel", s.fg_blue);

    return x;
}


static list
apply_mail(int s)
{
    list x;

    // Mail highlighting
    x += HI("mailHeaderKey", s.fg_blue);
    x += HI("mailHeaderEmail", s.fg_purple);
    x += HI("mailSubject", s.fg_pink);
    x += HI("mailHeader", s.fg_comment);
    x += HI("mailURL", s.fg_aqua);
    x += HI("mailEmail", s.fg_purple);
    x += HI("mailQuoted1", s.fg_olive);
    x += HI("mailQuoted2", s.fg_navy);

    return x;
}


static list
apply_xml(int s)
{
    list x;

    // XML Highlighting
    x += HI("xmlProcessingDelim", s.fg_pink);
    x += HI("xmlString", s.fg_olive);
    x += HI("xmlEqual", s.fg_orange);
    x += HI("xmlAttrib", s.fg_navy);
    x += HI("xmlAttribPunct", s.fg_pink);
    x += HI("xmlTag", s.fg_blue);
    x += HI("xmlTagName", s.fg_blue);
    x += HI("xmlEndTag", s.fg_blue);
    x += HI("xmlNamespace", s.fg_orange);

    return x;
}


static list
apply_elixir(int s)
{
    list x;

    // Elixir Highlighting
    // @target https.//github.com/elixir-lang/vim-elixir
    x += HI("elixirAlias", s.fg_blue, s.ft_bold);
    x += HI("elixirAtom", s.fg_navy);
    x += HI("elixirVariable", s.fg_navy);
    x += HI("elixirUnusedVariable", s.fg_foreground, s.ft_bold);
    x += HI("elixirInclude", s.fg_purple);
    x += HI("elixirStringDelimiter", s.fg_olive);
    x += HI("elixirKeyword", s.fg_purple, s.ft_bold);
    x += HI("elixirFunctionDeclaration", s.fg_aqua, s.ft_bold);
    x += HI("elixirBlockDefinition", s.fg_pink);
    x += HI("elixirDefine", s.fg_pink);
    x += HI("elixirStructDefine", s.fg_pink);
    x += HI("elixirPrivateDefine", s.fg_pink);
    x += HI("elixirModuleDefine", s.fg_pink);
    x += HI("elixirProtocolDefine", s.fg_pink);

    x += HI("elixirImplDefine", s.fg_pink);
    x += HI("elixirModuleDeclaration", s.fg_aqua, s.ft_bold);
    x += HI("elixirDocString", s.fg_olive);
    x += HI("elixirDocTest", s.fg_green, s.ft_bold);

    return x;
}


static list
apply_erlang(int s)
{
    list x;

    // Erlang Highlighting
    x += HI("erlangBIF", s.fg_purple, s.ft_bold);
    x += HI("erlangBracket", s.fg_pink);
    x += HI("erlangLocalFuncCall", s.fg_foreground);
    x += HI("erlangVariable", s.fg_foreground);
    x += HI("erlangAtom", s.fg_navy);
    x += HI("erlangAttribute", s.fg_blue, s.ft_bold);
    x += HI("erlangRecordDef", s.fg_blue, s.ft_bold);
    x += HI("erlangRecord", s.fg_blue);
    x += HI("erlangRightArrow", s.fg_blue, s.ft_bold);
    x += HI("erlangStringModifier", s.fg_olive, s.ft_bold);
    x += HI("erlangInclude", s.fg_blue, s.ft_bold);
    x += HI("erlangKeyword", s.fg_pink);
    x += HI("erlangGlobalFuncCall", s.fg_foreground);

    return x;
}


static list
apply_cucumber(int s)
{
    list x;

    // Cucumber Highlighting
    x += HI("cucumberFeature", s.fg_blue, s.ft_bold);
    x += HI("cucumberBackground", s.fg_pink, s.ft_bold);
    x += HI("cucumberScenario", s.fg_pink, s.ft_bold);
    x += HI("cucumberGiven", s.fg_orange);
    x += HI("cucumberGivenAnd", s.fg_blue);
    x += HI("cucumberThen", s.fg_orange);
    x += HI("cucumberThenAnd", s.fg_blue);
    x += HI("cucumberWhen", s.fg_purple, s.ft_bold);
    x += HI("cucumberScenarioOutline", s.fg_pink, s.ft_bold);
    x += HI("cucumberExamples", s.fg_aqua);
    x += HI("cucumberTags", s.fg_aqua);
    x += HI("cucumberPlaceholder", s.fg_aqua);

    return x;
}


static list
apply_ada(int s)
{
    list x;

    // Ada Highlighting
    x += HI("adaInc", s.fg_aqua, s.ft_bold);
    x += HI("adaSpecial", s.fg_aqua, s.ft_bold);
    x += HI("adaKeyword", s.fg_pink);
    x += HI("adaBegin", s.fg_pink);
    x += HI("adaEnd", s.fg_pink);
    x += HI("adaTypedef", s.fg_navy, s.ft_bold);
    x += HI("adaAssignment", s.fg_aqua, s.ft_bold);
    x += HI("adaAttribute", s.fg_green);

    return x;
}


static list
apply_cobol(int s)
{
    list x;

    // COBOL Highlighting
    x += HI("cobolMarker", s.fg_comment, s.bg_cursorline);
    x += HI("cobolLine", s.fg_foreground);
    x += HI("cobolReserved", s.fg_blue);
    x += HI("cobolDivision", s.fg_pink, s.ft_bold);
    x += HI("cobolDivisionName", s.fg_pink, s.ft_bold);
    x += HI("cobolSection", s.fg_navy, s.ft_bold);
    x += HI("cobolSectionName", s.fg_navy, s.ft_bold);
    x += HI("cobolParagraph", s.fg_purple);
    x += HI("cobolParagraphName", s.fg_purple);
    x += HI("cobolDeclA", s.fg_purple);
    x += HI("cobolDecl", s.fg_green);
    x += HI("cobolCALLs", s.fg_aqua, s.ft_bold);
    x += HI("cobolEXECs", s.fg_aqua, s.ft_bold);

    return x;
}


static list
apply_sed(int s)
{
    list x;

    // GNU sed highlighting
    x += HI("sedST", s.fg_purple, s.ft_bold);
    x += HI("sedFlag", s.fg_purple, s.ft_bold);
    x += HI("sedRegexp47", s.fg_pink);
    x += HI("sedRegexpMeta", s.fg_blue, s.ft_bold);
    x += HI("sedReplacement47", s.fg_olive);
    x += HI("sedReplaceMeta", s.fg_orange, s.ft_bold);
    x += HI("sedAddress", s.fg_pink);
    x += HI("sedFunction", s.fg_aqua, s.ft_bold);
    x += HI("sedBranch", s.fg_green, s.ft_bold);
    x += HI("sedLabel", s.fg_green, s.ft_bold);

    return x;
}


static list
apply_awk(int s)
{
    list x;

    // GNU awk highlighting
    x += HI("awkPatterns", s.fg_pink, s.ft_bold);
    x += HI("awkSearch", s.fg_pink);
    x += HI("awkRegExp", s.fg_blue, s.ft_bold);
    x += HI("awkCharClass", s.fg_blue, s.ft_bold);
    x += HI("awkFieldVars", s.fg_green, s.ft_bold);
    x += HI("awkStatement", s.fg_blue, s.ft_bold);
    x += HI("awkFunction", s.fg_blue);
    x += HI("awkVariables", s.fg_green, s.ft_bold);
    x += HI("awkArrayElement", s.fg_orange);
    x += HI("awkOperator", s.fg_foreground);
    x += HI("awkBoolLogic", s.fg_foreground);
    x += HI("awkExpression", s.fg_foreground);
    x += HI("awkSpecialPrintf", s.fg_olive, s.ft_bold);

    return x;
}


static list
apply_elm(int s)
{
    list x;

    // Elm highlighting
    x += HI("elmImport", s.fg_navy);
    x += HI("elmAlias", s.fg_aqua);
    x += HI("elmType", s.fg_pink);
    x += HI("elmOperator", s.fg_aqua, s.ft_bold);
    x += HI("elmBraces", s.fg_aqua, s.ft_bold);
    x += HI("elmTypedef", s.fg_blue, s.ft_bold);
    x += HI("elmTopLevelDecl", s.fg_green, s.ft_bold);

    return x;
}


static list
apply_ps(int s)
{
    list x;

    // Purescript highlighting
    x += HI("purescriptModuleKeyword", s.fg_navy);
    x += HI("purescriptImportKeyword", s.fg_navy);
    x += HI("purescriptModuleName", s.fg_pink);
    x += HI("purescriptOperator", s.fg_aqua, s.ft_bold);
    x += HI("purescriptType", s.fg_pink);
    x += HI("purescriptTypeVar", s.fg_navy);
    x += HI("purescriptStructure", s.fg_blue, s.ft_bold);
    x += HI("purescriptLet", s.fg_blue, s.ft_bold);
    x += HI("purescriptFunction", s.fg_green, s.ft_bold);
    x += HI("purescriptDelimiter", s.fg_aqua, s.ft_bold);
    x += HI("purescriptStatement", s.fg_purple, s.ft_bold);
    x += HI("purescriptConstructor", s.fg_pink);
    x += HI("purescriptWhere", s.fg_purple, s.ft_bold);

    return x;
}


static list
apply_fsharp(int s)
{
    list x;

    // F# highlighting
    x += HI("fsharpTypeName", s.fg_pink);
    x += HI("fsharpCoreClass", s.fg_pink);
    x += HI("fsharpType", s.fg_pink);
    x += HI("fsharpKeyword", s.fg_blue, s.ft_bold);
    x += HI("fsharpOperator", s.fg_aqua, s.ft_bold);
    x += HI("fsharpBoolean", s.fg_green, s.ft_bold);
    x += HI("fsharpFormat", s.fg_foreground);
    x += HI("fsharpLinq", s.fg_blue);
    x += HI("fsharpKeyChar", s.fg_aqua, s.ft_bold);
    x += HI("fsharpOption", s.fg_orange);
    x += HI("fsharpCoreMethod", s.fg_purple);
    x += HI("fsharpAttrib", s.fg_orange);
    x += HI("fsharpModifier", s.fg_aqua);
    x += HI("fsharpOpen", s.fg_red);

    return x;
}


static list
apply_asn1(int s)
{
    list x;

    // ASN.1 highlighting
    x += HI("asnExternal", s.fg_green, s.ft_bold);
    x += HI("asnTagModifier", s.fg_purple);
    x += HI("asnBraces", s.fg_aqua, s.ft_bold);
    x += HI("asnDefinition", s.fg_foreground);
    x += HI("asnStructure", s.fg_blue);
    x += HI("asnType", s.fg_pink);
    x += HI("asnTypeInfo", s.fg_aqua, s.ft_bold);
    x += HI("asnFieldOption", s.fg_purple);

    return x;
}


static list
apply_netrw(int s)
{
    list x;

    // Plugin: Netrw
    x += HI("netrwVersion", s.fg_red);
    x += HI("netrwList", s.fg_pink);
    x += HI("netrwHidePat", s.fg_olive);
    x += HI("netrwQuickHelp", s.fg_blue);
    x += HI("netrwHelpCmd", s.fg_blue);
    x += HI("netrwDir", s.fg_aqua, s.ft_bold);
    x += HI("netrwClassify", s.fg_pink);
    x += HI("netrwExe", s.fg_green);
    x += HI("netrwSuffixes", s.fg_comment);
    x += HI("netrwTreeBar", s.fg_linenumber_fg);

    return x;
}


static list
apply_nerdtree(int s)
{
    list x;

    // Plugin: NERDTree
    x += HI("NERDTreeUp", s.fg_comment);
    x += HI("NERDTreeHelpCommand", s.fg_pink);
    x += HI("NERDTreeHelpTitle", s.fg_blue, s.ft_bold);
    x += HI("NERDTreeHelpKey", s.fg_pink);
    x += HI("NERDTreeHelp", s.fg_foreground);
    x += HI("NERDTreeToggleOff", s.fg_red);
    x += HI("NERDTreeToggleOn", s.fg_green);
    x += HI("NERDTreeDir", s.fg_blue, s.ft_bold);
    x += HI("NERDTreeDirSlash", s.fg_pink);
    x += HI("NERDTreeFile", s.fg_foreground);
    x += HI("NERDTreeExecFile", s.fg_green);
    x += HI("NERDTreeOpenable", s.fg_aqua, s.ft_bold);
    x += HI("NERDTreeClosable", s.fg_pink);

    return x;
}


static list
apply_tagbar(int s)
{
    list x;

    // Plugin: Tagbar
    x += HI("TagbarHelpTitle", s.fg_blue, s.ft_bold);
    x += HI("TagbarHelp", s.fg_foreground);
    x += HI("TagbarKind", s.fg_pink);
    x += HI("TagbarSignature", s.fg_aqua);

    return x;
}


static list
apply_diff(int s)
{
    list x;

    // Plugin: Vimdiff
    x += HI("DiffAdd", s.fg_diffadd_fg, s.bg_diffadd_bg, s.ft_none);
    x += HI("DiffChange", s.fg_diffchange_fg, s.bg_diffchange_bg, s.ft_none);
    x += HI("DiffDelete", s.fg_diffdelete_fg, s.bg_diffdelete_bg, s.ft_none);
    x += HI("DiffText", s.fg_difftext_fg, s.bg_difftext_bg, s.ft_none);

    // Plugin: vim-gitgutter
    x += HI("GitGutterAdd", s.fg_diffadd_fg);
    x += HI("GitGutterChange", s.fg_diffchange_fg);
    x += HI("GitGutterDelete", s.fg_diffdelete_fg);
    x += HI("GitGutterAddLine", s.fg_diffadd_fg, s.bg_diffadd_bg, s.ft_none);
    x += HI("GitGutterChangeLine", s.fg_diffchange_fg, s.bg_diffchange_bg, s.ft_none);
    x += HI("GitGutterDeleteLine", s.fg_diffdelete_fg, s.bg_diffdelete_bg, s.ft_none);

    // Plugin: AGit
    x += HI("agitHead", s.fg_green, s.ft_bold);
    x += HI("agitHeader", s.fg_olive);
    x += HI("agitStatAdded", s.fg_diffadd_fg);
    x += HI("agitStatRemoved", s.fg_diffdelete_fg);
    x += HI("agitDiffAdd", s.fg_diffadd_fg);
    x += HI("agitDiffRemove", s.fg_diffdelete_fg);
    x += HI("agitDiffHeader", s.fg_pink);
    x += HI("agitDiff", s.fg_foreground);
    x += HI("agitDiffIndex", s.fg_purple);
    x += HI("agitDiffFileName", s.fg_aqua);
    x += HI("agitLog", s.fg_foreground);
    x += HI("agitAuthorMark", s.fg_olive);
    x += HI("agitDateMark", s.fg_comment);
    x += HI("agitHeaderLabel", s.fg_aqua);
    x += HI("agitDate", s.fg_aqua);
    x += HI("agitTree", s.fg_pink);
    x += HI("agitRef", s.fg_blue, s.ft_bold);
    x += HI("agitRemote", s.fg_purple, s.ft_bold);
    x += HI("agitTag", s.fg_orange, s.ft_bold);

    return x;
}


static list
apply_spell(int s)
{
    list x;

    // Plugin: Spell Checking
    x += HI("SpellBad", s.fg_foreground, s.bg_spellbad);
    x += HI("SpellCap", s.fg_foreground, s.bg_spellcap);
    x += HI("SpellRare", s.fg_foreground, s.bg_spellrare);
    x += HI("SpellLocal", s.fg_foreground, s.bg_spelllocal);

    return x;
}


static list
apply_indent(int s)
{
    list x;

    // Plugin: Indent Guides
    x += HI("IndentGuidesOdd", s.bg_background);
    x += HI("IndentGuidesEven", s.bg_cursorline);

    return x;
}


static list
apply_startify(int s)
{
    list x;

    // Plugin: Startify
    x += HI("StartifyFile", s.fg_blue, s.ft_bold);
    x += HI("StartifyNumber", s.fg_orange);
    x += HI("StartifyHeader", s.fg_comment);
    x += HI("StartifySection", s.fg_pink);
    x += HI("StartifyPath", s.fg_foreground);
    x += HI("StartifySlash", s.fg_navy);
    x += HI("StartifyBracket", s.fg_aqua);
    x += HI("StartifySpecial", s.fg_aqua);

    return x;
}


static list
apply_signify(int s)
{
    list x;

    // Plugin: Signify
    x += HI("SignifyLineChange", s.fg_diffchange_fg);
    x += HI("SignifySignChange", s.fg_diffchange_fg);
    x += HI("SignifyLineAdd", s.fg_diffadd_fg);
    x += HI("SignifySignAdd", s.fg_diffadd_fg);
    x += HI("SignifyLineDelete", s.fg_diffdelete_fg);
    x += HI("SignifySignDelete", s.fg_diffdelete_fg);

    return x;
}


static list
apply_git(int s)
{
    list x;

    // Git commit message
    x += HI("gitcommitSummary", s.fg_blue);
    x += HI("gitcommitHeader", s.fg_green, s.ft_bold);
    x += HI("gitcommitSelectedType", s.fg_blue);
    x += HI("gitcommitSelectedFile", s.fg_pink);
    x += HI("gitcommitUntrackedFile", s.fg_diffdelete_fg);
    x += HI("gitcommitBranch", s.fg_aqua, s.ft_bold);
    x += HI("gitcommitDiscardedType", s.fg_diffdelete_fg);
    x += HI("gitcommitDiff", s.fg_comment);

    x += HI("diffFile", s.fg_blue);
    x += HI("diffSubname", s.fg_comment);
    x += HI("diffIndexLine", s.fg_comment);
    x += HI("diffAdded", s.fg_diffadd_fg);
    x += HI("diffRemoved", s.fg_diffdelete_fg);
    x += HI("diffLine", s.fg_orange);
    x += HI("diffBDiffer", s.fg_orange);
    x += HI("diffNewFile", s.fg_comment);

    return x;
}


static list
apply_coc(int s)
{
    list x;

    // Pluging: CoC
    x += HI("CocFloating", s.fg_popupmenu_fg, s.bg_popupmenu_bg, s.ft_none);
    x += HI("CocErrorFloat", s.fg_popupmenu_fg, s.bg_popupmenu_bg, s.ft_none);
    x += HI("CocWarningFloat", s.fg_popupmenu_fg, s.bg_popupmenu_bg, s.ft_none);
    x += HI("CocInfoFloat", s.fg_popupmenu_fg, s.bg_popupmenu_bg, s.ft_none);
    x += HI("CocHintFloat", s.fg_popupmenu_fg, s.bg_popupmenu_bg, s.ft_none);

    x += HI("CocErrorHighlight", s.fg_foreground, s.bg_spellbad);
    x += HI("CocWarningHighlight", s.fg_foreground, s.bg_spellcap);
    x += HI("CocInfoHighlight", s.fg_foreground, s.bg_spellcap);
    x += HI("CocHintHighlight", s.fg_foreground, s.bg_spellcap);

    x += HI("CocErrorSign", s.fg_error_fg, s.bg_error_bg);
    x += HI("CocWarningSign", s.fg_todo_fg, s.bg_todo_bg, s.ft_bold);
    x += HI("CocInfoSign", s.fg_todo_fg, s.bg_todo_bg, s.ft_bold);
    x += HI("CocHintSign", s.fg_todo_fg, s.bg_todo_bg, s.ft_bold);

    return x;
}


static list
papercolor(string scheme, int dark)
{
    int h = acquire_theme();

    h.SCHEME = scheme;
    h.DARK = dark;
    h.GUI = 0;

    generate_theme_option_variables(h);
    generate_language_option_variables(h);

    int s = acquire_pallette(dark);

    set_format_attributes(s, h);
    set_overriding_colors(s, h);

    convert_colors(s, h);
    set_color_variables(s);

    apply_syntax_highlightings(s, h);

    delete_dictionary(h);
    delete_dictionary(s);
}


int
colorscheme_papercolor(~list args)
{
    string scheme = "papercolor";
    int colordepth = -1, dark = -1;

    if (! is_null(args)) {
        /* options */
        const list longoptions = {
            "colors:d",
            "mode:s",
            "dark",
            "light",
            };
        string value;
        int optidx = 0, ch;

        if ((ch = getopt(value, NULL, longoptions. args. scheme)) >= 0) {
            do {
                ++optidx;
                switch(ch) {
                case 0: // colordepth
                    colordepth = atoi(value);
                    break;
                case 1: // mode
                    if (value == "default") {   /* dynamic dark or light */
                        dark = -1;
                    } else {
                        dark = (value == "dark" ? 1 : 0);
                    }
                    break;
                case 2: // dark
                    dark = 1;
                    break;
                case 3: // light
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
                message("%s: by %s. %s", scheme, "Nikyle Nguyen et al.", "https.//github.com/NLKNguyen/papercolor-theme/");
            } else {
                error("%s: invalid option <%s>", scheme, args[optidx]);
            }
            return -1;
        }
    }

    if (colordepth <= 0) {
         get_term_feature(TF_COLORDEPTH, colordepth);
    }
    if (colordepth != 16 && colordepth != 88 && colordepth != 256) {
        error("%s. color depth not supported", scheme);
        return -1;
    }

    if (dark == -1) {
        get_term_feature(TF_SCHEMEDARK, dark); /* default */
    }

    papercolor(scheme, dark);
    return 0;
}

//end
