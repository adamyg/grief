/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: sonofobsidian.cr,v 1.3 2014/11/25 04:44:50 ayoung Exp $
 * Son-of-Obsidian coloriser, GRIEF port.
 *
 *
 */

#include "../grief.h"

static list                 /*vim style coloriser specification*/
soo_256_cterm[] = {
        "set background=dark",
        "hi clear",

        "hi Constant"       +" ctermfg=149",
        "hi String"         +" ctermfg=208",
        "hi Character"      +" ctermfg=149",
        "hi Number"         +" ctermfg=208",
        "hi Boolean"        +" ctermfg=208",
        "hi Float"          +" ctermfg=208",
        "hi Comment"        +" ctermfg=66",
        "hi Identifier"     +" ctermfg=15",
        "hi Function"       +" ctermfg=15",
        "hi Statement"      +" ctermfg=149"                                 +" cterm=bold",
        "hi Conditional"    +" ctermfg=149"                                 +" cterm=bold",
        "hi Repeat"         +" ctermfg=149"                                 +" cterm=bold",
        "hi Label"          +" ctermfg=149"                                 +" cterm=bold",
        "hi Operator"       +" ctermfg=149",
        "hi Keyword"        +" ctermfg=149"                                 +" cterm=bold",
        "hi Exception"      +" ctermfg=170",
        "hi PreProc"        +" ctermfg=170",
        "hi Include"        +" ctermfg=170",
        "hi Define"         +" ctermfg=170",
        "hi Macro"          +" ctermfg=170",
        "hi PreCondit"      +" ctermfg=170",
        "hi Type"           +" ctermfg=149",
        "hi StorageClass"   +" ctermfg=149",
        "hi Structure"      +" ctermfg=110",
        "hi Typedef"        +" ctermfg=149",
        "hi Special"        +" ctermfg=15",
        "hi SpecialChar"    +" ctermfg=15",
        "hi Tag"            +" ctermfg=149",
        "hi Delimiter"      +" ctermfg=15",
        "hi SpecialComment" +" ctermfg=15",
        "hi Debug"          +" ctermfg=15",
        "hi Underlined"     +" ctermfg=110"                                 +" cterm=underline",
        "hi Ignore"         +" ctermfg=15",
        "hi Error"          +" ctermfg=15"          +" ctermbg=9"           +" cterm=bold",
        "hi Todo"           +" ctermfg=15"          +" ctermbg=149",

        "hi Cursor"         +" ctermfg=white"       +" ctermbg=red",
        "hi CursorLine"     +" ctermfg=none"        +" ctermbg=236",
        "hi CursorColumn"   +" ctermfg=none"        +" ctermbg=236",
        "hi Directory"      +" ctermfg=149",
        "hi ErrorMsg"       +" ctermfg=15"          +" ctermbg=9"           +" cterm=bold",
        "hi LineNr"         +" ctermfg=245"         +" ctermbg=238",
        "hi MatchParen"     +" ctermfg=0"           +" ctermbg=149"         +" cterm=bold",
        "hi ModeMsg"        +" ctermfg=15",
        "hi MoreMsg"        +" ctermfg=149",
        "hi NonText"        +" ctermfg=239",
        "hi Normal"         +" ctermfg=253"         +" ctermbg=235"         +" cterm=bold",
//      "hi Normal"         +" ctermfg=253"         +" ctermbg=none"        +" cterm=bold",
        "hi Question"       +" ctermfg=149",
        "hi SpecialKey"     +" ctermfg=149",
        "hi Title"          +" ctermfg=170",
        "hi VertSplit"      +" ctermfg=0"           +" ctermbg=240"         +" cterm=reverse",
        "hi WarningMsg"     +" ctermfg=9",
        "hi WildMenu"       +" ctermfg=15"          +" ctermbg=236",

        "hi TabLine"        +" ctermfg=0"           +" ctermbg=240"         +" cterm=bold,reverse",
        "hi TabLineFill"    +" ctermfg=0"           +" ctermbg=240"         +" cterm=bold,reverse",
        "hi TabLineSel"     +" ctermfg=0"           +" ctermbg=240"         +" cterm=bold,reverse",

        "hi Pmenu"          +" ctermfg=252"         +" ctermbg=75",
        "hi PmenuSel"       +" ctermfg=none"        +" ctermbg=75",

        "hi StatusLine"     +" ctermfg=111"         +" ctermbg=0"           +" cterm=reverse",
        "hi StatusLineNC"   +" ctermfg=238"         +" ctermbg=245"         +" cterm=reverse",

        "hi DiffAdd"        +" ctermfg=NONE"        +" ctermbg=239",
        "hi DiffChange"     +" ctermfg=NONE"        +" ctermbg=170",
        "hi DiffDelete"     +" ctermfg=239"         +" ctermbg=66"          +" cterm=bold",
        "hi DiffText"       +" ctermfg=15"                                  +" cterm=bold",

        "hi Visual"         +" ctermfg=15"          +" ctermbg=239",
        "hi VisualNOS"      +" ctermfg=15"          +" ctermbg=239",

        "hi Folded"         +" ctermfg=244"         +" ctermbg=235",
        "hi FoldColumn"     +" ctermfg=15"          +" ctermbg=237",

        "hi IncSearch"      +" ctermfg=15"          +" ctermbg=149",
        "hi Search"         +" ctermfg=15"          +" ctermbg=149",
        };


void
colorscheme_sonofobsidian(void)
{
        vim_colorscheme("sonofobsidian", 256, NULL, soo_256_cterm, FALSE);
}


void
colorscheme_soo(void)
{
        colorscheme_sonofobsidian();
}

