/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: twilight.cr,v 1.1 2014/11/24 03:56:39 ayoung Exp $
 * twilight coloriser, GRIEF port.
 *
 *
 */

#include "../grief.h"

#define grey_blue           "#8a9597"
#define light_grey_blue     "#a0a8b0"
#define dark_grey_blue      "#34383c"
#define mid_grey_blue       "#64686c"
#define beige               "#ceb67f"
#define light_orange        "#ebc471"
#define yellow              "#e3d796"
#define violet              "#a999ac"
#define green               "#a2a96f"
#define lightgreen          "#c2c98f"
#define red                 "#d08356"
#define cyan                "#74dad9"
#define darkgrey            "#1a1a1a"
#define grey                "#303030"
#define lightgrey           "#605958"
#define white               "#fffedc"

static list                 /*vim style coloriser specification*/
twilight_colors[] = {
        "set background=dark",
        "hi clear",

        "hi CursorLine      guibg=#262626",
        "hi CursorColumn    guibg=#262626",
        "hi MatchParen      guifg=white guibg=#80a090 gui=bold",

        // Tabpages
        "hi TabLine         guifg=#a09998 guibg=#202020 gui=underline",
        "hi TabLineFill     guifg=#a09998 guibg=#202020 gui=underline",
        "hi TabLineSel      guifg=#a09998 guibg=#404850 gui=underline",

        //  P-Menu (auto-completion)
        "hi Pmenu           guifg=#605958 guibg=#303030 gui=underline",
        "hi PmenuSel        guifg=#a09998 guibg=#404040 gui=underline",
        "hi Visual          guibg=#404040",
        "hi Cursor          guibg=#b0d0f0",

        "hi Normal          guifg="+white           +" guibg="+darkgrey,
        "hi Underlined      guifg="+white           +" guibg="+darkgrey         +" gui=underline",
        "hi NonText         guifg="+lightgrey       +" guibg="+grey,
        "hi SpecialKey      guifg="+grey            +" guibg="+darkgrey,

        "hi LineNr          guifg="+mid_grey_blue   +" guibg="+dark_grey_blue   +" gui=none",
        "hi StatusLine      guifg="+white           +" guibg="+grey             +" gui=italic,underline",
        "hi StatusLineNC    guifg="+lightgrey       +" guibg="+grey             +" gui=italic,underline",
        "hi VertSplit       guifg="+grey            +" guibg="+grey             +" gui=none",

        "hi Folded          guifg="+grey_blue       +" guibg="+dark_grey_blue   +" gui=none",
        "hi FoldColumn      guifg="+grey_blue       +" guibg="+dark_grey_blue   +" gui=none",
        "hi SignColumn      guifg="+grey_blue       +" guibg="+dark_grey_blue   +" gui=none",

        "hi Comment         guifg="+mid_grey_blue   +" guibg="+darkgrey         +" gui=italic",
        "hi TODO            guifg="+grey_blue       +" guibg="+darkgrey         +" gui=italic,bold",

        "hi Title           guifg="+red             +" guibg="+darkgrey         +" gui=underline",

        "hi Constant        guifg="+red             +" guibg="+darkgrey         +" gui=none",
        "hi String          guifg="+green           +" guibg="+darkgrey         +" gui=none",
        "hi Special         guifg="+lightgreen      +" guibg="+darkgrey         +" gui=none",

        "hi Identifier      guifg="+grey_blue       +" guibg="+darkgrey         +" gui=none",
        "hi Statement       guifg="+beige           +" guibg="+darkgrey         +" gui=none",
        "hi Conditional     guifg="+beige           +" guibg="+darkgrey         +" gui=none",
        "hi Repeat          guifg="+beige           +" guibg="+darkgrey         +" gui=none",
        "hi Structure       guifg="+beige           +" guibg="+darkgrey         +" gui=none",
        "hi Function        guifg="+violet          +" guibg="+darkgrey         +" gui=none",

        "hi PreProc         guifg="+grey_blue       +" guibg="+darkgrey         +" gui=none",
        "hi Operator        guifg="+light_orange    +" guibg="+darkgrey         +" gui=none",
        "hi Type            guifg="+yellow          +" guibg="+darkgrey         +" gui=italic",

        "hi Identifier      guifg=#7587a6",

        "hi Structure       guifg=#9B859D gui=underline",
        "hi Function        guifg=#dad085",
        "hi Statement       guifg=#7187a1 gui=NONE",
        "hi PreProc         guifg=#8fbfdc ",        //gui=underline",
        "hi Operator        guifg=#a07020",
        "hi Repeat          guifg=#906040 gui=underline",
        "hi Type            guifg=#708090",
        "hi Type            guifg=#f9ee98 gui=NONE",
        "hi NonText         guifg=#808080 guibg=#303030",
        "hi Macro           guifg=#a0b0c0 gui=underline",

        // Tabs, trailing spaces, etc (lcs)
        "hi SpecialKey      guifg=#808080 guibg=#343434",
        "hi TooLong         guibg=#ff0000 guifg=#f8f8f8",
        "hi Search          guifg=#606000 guibg=#c0c000 gui=bold",
        "hi Directory       guifg=#dad085 gui=NONE",
        "hi Error           guibg=#602020"
        };

void
colorscheme_twilight(void)
{
    vim_colorscheme("twilight", 256, NULL, twilight_colors, TRUE);
}
/*end*/
