/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: zenburn.cr,v 1.5 2014/11/24 03:56:39 ayoung Exp $
 * zenburn coloriser, GRIEF port.
 *
 *  Original author Jani Numinen in 2002
 *  see http://slinky.imukuppi.org/zenburnpage
 *
 *
 */

#include "../grief.h"

static list                 /*vim style coloriser specification*/
zenburn_256_cterm[] = {
        "set background=dark",
        "hi clear",

        "hi Boolean"        +" ctermfg=181",
        "hi Character"      +" ctermfg=181"                                 +" cterm=bold",
        "hi Comment"        +" ctermfg=108",
        "hi Conditional"    +" ctermfg=223"                                 +" cterm=bold",
        "hi Constant"       +" ctermfg=181"                                 +" cterm=bold",
        "hi Cursor"         +" ctermfg=233"         +" ctermbg=109"         +" cterm=bold",
        "hi Debug"          +" ctermfg=181"                                 +" cterm=bold",
        "hi Define"         +" ctermfg=223"                                 +" cterm=bold",
        "hi Delimiter"      +" ctermfg=245",
        "hi DiffAdd"        +" ctermfg=66 "         +" ctermbg=237"         +" cterm=bold",
        "hi DiffChange"     +" ctermbg=236",
        "hi DiffDelete"     +" ctermfg=236"         +" ctermbg=238",
        "hi DiffText"       +" ctermfg=217"         +" ctermbg=237"         +" cterm=bold",
        "hi Directory"      +" ctermfg=109"                                 +" cterm=bold",
        "hi ErrorMsg"       +" ctermfg=115"         +" ctermbg=236"         +" cterm=bold",
        "hi Exception"      +" ctermfg=249"                                 +" cterm=bold",
        "hi Float"          +" ctermfg=251",
        "hi Function"       +" ctermfg=228",
        "hi Identifier"     +" ctermfg=223",
        "hi IncSearch"      +" ctermbg=228"         +" ctermfg=238",
        "hi Keyword"        +" ctermfg=223"                                 +" cterm=bold",
        "hi Label"          +" ctermfg=187"                                 +" cterm=underline",
        "hi LineNr"         +" ctermfg=248"         +" ctermbg=233",
        "hi Macro"          +" ctermfg=223"                                 +" cterm=bold",
        "hi ModeMsg"        +" ctermfg=223"                                 +" cterm=none",
        "hi MoreMsg"        +" ctermfg=15 "                                 +" cterm=bold",
        "hi Number"         +" ctermfg=116",
        "hi Operator"       +" ctermfg=230",
        "hi PreCondit"      +" ctermfg=180"                                 +" cterm=bold",
        "hi PreProc"        +" ctermfg=223"                                 +" cterm=bold",
        "hi Question"       +" ctermfg=15 "                                 +" cterm=bold",
        "hi Repeat"         +" ctermfg=223"                                 +" cterm=bold",
        "hi Search"         +" ctermfg=230"         +" ctermbg=236",
        "hi SpecialChar"    +" ctermfg=181"                                 +" cterm=bold",
        "hi SpecialComment" +" ctermfg=181"                                 +" cterm=bold",
        "hi Special"        +" ctermfg=181",
        "hi SpecialKey"     +" ctermfg=151",
        "hi Statement"      +" ctermfg=187"         +" ctermbg=234"         +" cterm=none",
        "hi StatusLine"     +" ctermfg=236"         +" ctermbg=186",
        "hi StatusLineNC"   +" ctermfg=235"         +" ctermbg=108",
        "hi StorageClass"   +" ctermfg=249"                                 +" cterm=bold",
        "hi String"         +" ctermfg=174",
        "hi Structure"      +" ctermfg=229"                                 +" cterm=bold",
        "hi Tag"            +" ctermfg=181"                                 +" cterm=bold",
        "hi Title"          +" ctermfg=7  "         +" ctermbg=234"         +" cterm=bold",
        "hi Todo"           +" ctermfg=108"         +" ctermbg=234"         +" cterm=bold",
        "hi Typedef"        +" ctermfg=253"                                 +" cterm=bold",
        "hi Type"           +" ctermfg=187"                                 +" cterm=bold",
        "hi Underlined"     +" ctermfg=188"         +" ctermbg=234"         +" cterm=bold",
        "hi VertSplit"      +" ctermfg=236"         +" ctermbg=65",
        "hi VisualNOS"      +" ctermfg=236"         +" ctermbg=210"         +" cterm=bold",
        "hi WarningMsg"     +" ctermfg=15 "         +" ctermbg=236"         +" cterm=bold",
        "hi WildMenu"       +" ctermbg=236"         +" ctermfg=194"         +" cterm=bold",
//      "hi SpellLocal"     +" ctermfg=14 "         +" ctermbg=237",
//      "hi SpellBad"       +" ctermfg=9  "         +" ctermbg=237",
//      "hi SpellCap"       +" ctermfg=12 "         +" ctermbg=237",
//      "hi SpellRare"      +" ctermfg=13 "         +" ctermbg=237",
        "hi Spell"          +" ctermfg=108"                                 +" cterm=underline",
        "hi PMenu"          +" ctermfg=248"         +" ctermbg=0",
        "hi PMenuSel"       +" ctermfg=223"         +" ctermbg=235",
        };

void
colorscheme_zenburn(void)
{
    vim_colorscheme("zenburn", 256, NULL, zenburn_256_cterm, FALSE);
}
/*end*/
