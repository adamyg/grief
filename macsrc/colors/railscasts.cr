/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: railscasts.cr,v 1.6 2024/10/08 11:40:03 cvsuser Exp $
 * railscasts coloriser, GRIEF port.
 *
 * Original:
 *  Name: railscast.vim
 *  Maintainer: Josh O'Rourke <joshorourke@me.com>
 *  License: public domain
 *
 *    A GUI Only port of the RailsCasts TextMate theme [1] to Vim.
 *    Some parts of this theme were borrowed from the well-documented Lucius theme [2].
 *
 *    [1] http://railscasts.com/about
 *    [2] http://www.vim.org/scripts/script.php?script_id=2536
 *
 */

#include "../grief.h"

static list                 /*vim style coloriser specification*/
railscasts_colors = {
        "set background=dark",
        "hi clear",

        //  Colors
        //      Brown       #BC9458
        //      Dark Blue   #6D9CBE
        //      Dark Green  #519F50
        //      Dark Orange #CC7833
        //      Light Blue  #D0D0FF
        //      Light Green #A5C261
        //      Tan         #FFC66D
        //
        "hi Normal          guifg=#E6E1DC guibg=#2B2B2B ctermfg=white ctermbg=234",
        "hi Cursor          guifg=#000000 guibg=#FFFFFF ctermfg=0 ctermbg=15",
        "hi CursorLine      guibg=#333435 ctermbg=235 cterm=NONE",
        "hi Search          guibg=#5A647E ctermfg=NONE ctermbg=236 cterm=underline",
        "hi Visual          guibg=#5A647E ctermbg=60",
        "hi LineNr          guifg=#888888 ctermfg=242",
        "hi StatusLine      guibg=#414243 gui=NONE guifg=#E6E1DC",
        "hi StatusLineNC    guibg=#414243 gui=NONE",
        "hi VertSplit       guibg=#414243 gui=NONE guifg=#414243",
        "hi CursorLineNr    guifg=#bbbbbb ctermfg=248",
        "hi ColorColumn     guibg=#333435 ctermbg=235",

        //  Folds
        "hi Folded          guifg=#F6F3E8 guibg=#444444 gui=NONE",

        //  Invisible Characters
        "hi NonText         guifg=#777777 gui=NONE",
        "hi SpecialKey      guifg=#777777 gui=NONE",

        //  Misc
        "hi Directory       guifg=#A5C261 gui=NONE",

        //  Popup Menu
        "hi Pmenu           guifg=#F6F3E8 guibg=#444444 gui=NONE",
        "hi PmenuSel        guifg=#000000 guibg=#A5C261 gui=NONE",
        "hi PMenuSbar       guibg=#5A647E gui=NONE",
        "hi PMenuThumb      guibg=#AAAAAA gui=NONE",

        //  RubyComment
        "hi Comment         guifg=#BC9458 gui=italic ctermfg=137",
        "hi Todo            guifg=#BC9458 guibg=NONE gui=italic ctermfg=94",
        "hi Constant        guifg=#6D9CBE ctermfg=73",
        "hi Define          guifg=#CC7833 ctermfg=173",
        "hi Delimiter       guifg=#519F50",
        "hi Error           guifg=#FFFFFF guibg=#990000 ctermfg=221 ctermbg=88",
        "hi Function        guifg=#FFC66D gui=NONE ctermfg=221 cterm=NONE",
        "hi Identifier      guifg=#D0D0FF gui=NONE ctermfg=73 cterm=NONE",
        "hi Include         guifg=#CC7833 gui=NONE ctermfg=173 cterm=NONE",
        "hi Keyword         guifg=#CC7833 ctermfg=172 cterm=NONE",
        "hi Macro           guifg=#CC7833 gui=NONE ctermfg=172",
        "hi Number          guifg=#A5C261 ctermfg=107",
        "hi PreCondit       guifg=#CC7833 gui=NONE ctermfg=172 cterm=NONE",
        "hi PreProc         guifg=#CC7833 gui=NONE ctermfg=103",
        "hi Statement       guifg=#CC7833 gui=NONE ctermfg=172 cterm=NONE",
        "hi String          guifg=#A5C261 ctermfg=107",
        "hi Title           guifg=#FFFFFF ctermfg=15",
        "hi Type            guifg=#DA4939 gui=NONE",

        "hi DiffAdd         guifg=#E6E1DC guibg=#144212",
        "hi DiffDelete      guifg=#E6E1DC guibg=#660000",

    //  "hi link            htmlTag xmlTag",
    //  "hi link            htmlTagName xmlTagName",
    //  "hi link            htmlEndTag xmlEndTag",

    //  "hi xmlTag          guifg=#E8BF6A",
    //  "hi xmlTagName      guifg=#E8BF6A",
    //  "hi xmlEndTag       guifg=#E8BF6A"
        };

void
colorscheme_railscasts(void)
{
        vim_colorscheme("railscasts", 256, NULL, railscasts_colors, TRUE);
}

/*end*/
