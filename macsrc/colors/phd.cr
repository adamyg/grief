/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: phd.cr,v 1.2 2014/11/27 15:54:14 ayoung Exp $
 * PhD coloriser, GRIEF port.
 *
 * Original:
 *  Name: phd.vim
 *  Author: Henning Hasemann
 *  Link: https://github.com/vim-scripts/phd
 *  Features: dark
 */

#include "../grief.h"

#define white               "white"
#define darkgrey            "darkgrey"

#define violet              "#99699c"
#define light_violet        "#a989ac"
#define yellow              "#e3d756"
#define red                 "#d07346"
#define darkgreen           "#80a050"
#define green               "#99bf52"
#define lightgreen          "#e2e9af"
#define darkblue            "#32698f"
#define blue                "#5299bf"
#define lightblue           "#72b9bf"
#define light_orange        "#fbd461"
//  orange = "#f0a000"
//  orange = "#b08060"
#define orange              "#bba401"
#define grey                "#808080"

#define text                "#e0e0e0"
#define text_hl             "#ffffff"
#define text_minor          "#909090"
#define text_very_minor     "#304050"

#define greyed              "#687898"

#define popup_bg            "#101010"
#define popup_bg_hl         "#000000"
#define popup_fg            text;
#define popup_fg_hl         text_hl;

#define bg                  "#061229"
#define bg_hl_soft          "#1e293e"
#define bg_hl               "#324454"
#define bg_minor            "#030a17"

#define structure           "#76cc68"
#define constant1           "#c08040"
#define constant2           "#d2852b"
#define constant3           "#f2a54b"
#define control1            "#f9f7a4"
#define control2            "#c9b794"
#define border1_bg          bg_hl
#define border1_fg          text_minor
#define func                "#96b2cc"

static list                 /*vim style coloriser specification*/
phd_gui = {
        "set background=dark",
        "hi clear",

        // Tabpages
        "hi TabLineSel      guifg="+text_hl         +" guibg="+border1_bg   +" gui=underline",
        "hi TabLine         guifg="+text_minor      +" guibg="+bg           +" gui=underline",
        "hi TabLineFill     guifg="+text_minor      +" guibg="+bg           +" gui=none",

        // P-Menu (auto-completion)
        "hi Pmenu           guifg=#605958"          +" guibg=#101418"       +" gui=none",
        "hi PmenuSel        guifg=#a09998"          +" guibg=#404040"       +" gui=underline",
        "hi CursorLine      guibg="+bg_hl_soft                              +" gui=none",
        "hi CursorColumn    guibg="+bg_hl_soft                              +" gui=none",
        "hi MatchParen      guifg="+text_hl         +" guibg="+bg_hl        +" gui=bold",

        "hi Pmenu           guifg="+text            +" guibg="+popup_bg     +" gui=none",
        "hi PmenuSel        guifg="+text_hl         +" guibg="+popup_bg_hl  +" gui=bold",
        "hi PmenuSbar"                              +" guibg="+popup_bg_hl,
        "hi PmenuThumb      guifg="+text,

        "hi Visual          guibg="+bg_hl,

        "hi Cursor          guifg=NONE"             +" guibg=#586068",

        "hi Normal          guifg="+text            +" guibg="+bg,
        "hi Underlined      guifg="+white           +" guibg="+darkgrey     +" gui=underline",
        "hi NonText         guifg="+text_very_minor +" guibg="+bg,
        "hi SpecialKey      guifg="+text_very_minor +" guibg="+bg,

        "hi LineNr          guifg="+border1_fg      +" guibg="+border1_bg   +" gui=none",
        "hi StatusLine      guifg="+text_hl         +" guibg="+border1_bg   +" gui=underline",
        "hi StatusLineNC    guifg="+text_minor      +" guibg="+border1_bg   +" gui=underline",
        "hi VertSplit       guifg="+border1_bg      +" guibg="+border1_bg   +" gui=none",

        "hi Folded          guifg="+text_minor      +" guibg="+border1_bg   +" gui=none",
        "hi FoldColumn      guifg="+text_minor      +" guibg="+border1_bg   +" gui=none",
        "hi SignColumn      guifg="+text_minor      +" guibg="+border1_bg   +" gui=none",

        "hi Comment         guifg="+greyed          +" guibg="+bg           +" gui=none",
        "hi TODO            guifg="+greyed          +" guibg="+bg           +" gui=bold",

        "hi Title           guifg="+red             +" guibg="+bg           +" gui=underline",

        "hi Constant        guifg="+constant1       +" guibg="+bg           +" gui=none",
        "hi String          guifg="+constant2       +" guibg="+bg           +" gui=none",
        "hi Special         guifg="+constant3       +" guibg="+bg           +" gui=none",

        "hi Identifier      guifg="+control1        +" guibg="+bg           +" gui=none",
        "hi Statement       guifg="+control2        +" guibg="+bg           +" gui=none",
        "hi Conditional     guifg="+grey            +" guibg="+bg           +" gui=bold",
        "hi Repeat          guifg="+light_orange    +" guibg="+bg           +" gui=bold",
        "hi Structure       guifg="+structure       +" guibg="+bg           +" gui=none",
        "hi Function        guifg="+func            +" guibg="+bg           +" gui=none",

        "hi PreProc         guifg="+light_violet    +" guibg="+bg           +" gui=none",
        "hi Define          guifg="+light_violet    +" guibg="+bg           +" gui=none",
        "hi Operator        guifg="+light_orange    +" guibg="+bg           +" gui=none",
        "hi Type            guifg="+yellow          +" guibg="+bg           +" gui=none",

        "hi Macro           guifg=#a0b0c0           gui=underline",

        // Tabs, trailing spaces, etc (lcs)
        "hi SpecialKey      guifg=#808080"          +" guibg=#343434",
        "hi TooLong         guibg=#ff0000"          +" guifg=#f8f8f8",

        "hi Search          guifg=#606000"          +" guibg=#c0c000"       +" gui=bold",

        "hi Directory       guifg=#dad085"                                  +" gui=NONE",
        "hi Error           guibg=#602020",
        };


void
colorscheme_phd(void)
{
    vim_colorscheme("phd", 256, NULL, phd_gui, TRUE);
}
/*end*/
