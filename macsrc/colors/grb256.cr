/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: grb256.cr,v 1.5 2024/08/04 11:42:44 cvsuser Exp $
 * grb256 coloriser, GRIEF port.
 *
 * Original:
 *  Name: grb256.vim
 *  Source: https://github.com/garybernhardt/dotfiles/blob/master/.vim/colors
 *
 *  GRB256 is a color scheme based on ir_black by Gary Bernhardt.
 *  It feels really nice especially for Ruby on Rails development.
 */

#include "../grief.h"

static list                 /*vim style coloriser specification*/
grb256_cteam = {
        "hi Comment         ctermfg=darkgray",

        "hi StatusLine      ctermbg=darkgrey  ctermfg=white",
        "hi StatusLineNC    ctermbg=black     ctermfg=lightgrey",
        "hi VertSplit       ctermbg=black     ctermfg=lightgrey",
        "hi LineNr          ctermfg=darkgray",

        "hi CursorLine      guifg=NONE        guibg=#121212     gui=NONE      ctermfg=NONE       ctermbg=234    cterm=NONE",
        "hi Function        guifg=#FFD2A7     guibg=NONE        gui=NONE      ctermfg=yellow     ctermbg=NONE   cterm=NONE",
        "hi Visual          guifg=NONE        guibg=#262D51     gui=NONE      ctermfg=NONE       ctermbg=236    cterm=NONE",

        "hi Error           guifg=NONE        guibg=NONE        gui=undercurl ctermfg=16         ctermbg=red    cterm=NONE     guisp=#FF6C60",
        "hi ErrorMsg        guifg=white       guibg=#FF6C60     gui=BOLD      ctermfg=16         ctermbg=red    cterm=NONE",
        "hi WarningMsg      guifg=white       guibg=#FF6C60     gui=BOLD      ctermfg=16         ctermbg=red    cterm=NONE",
        "hi SpellBad        guifg=white       guibg=#FF6C60     gui=BOLD      ctermfg=16         ctermbg=160    cterm=NONE",

        "hi Operator        guifg=#6699CC     guibg=NONE        gui=NONE      ctermfg=lightblue  ctermbg=NONE   cterm=NONE",

        "hi DiffAdd         term=reverse      cterm=bold        ctermbg=lightgreen ctermfg=16",
        "hi DiffChange      term=reverse      cterm=bold        ctermbg=lightblue ctermfg=16",
        "hi DiffText        term=reverse      cterm=bold        ctermbg=lightgray ctermfg=16",
        "hi DiffDelete      term=reverse      cterm=bold        ctermbg=lightred ctermfg=16",

        "hi PmenuSel        ctermfg=16        ctermbg=156"
        };


extern void colorscheme_ir_black();

void
colorscheme_grb256(void)
{
        require("colors/ir_black");
        colorscheme_ir_black();
        vim_colorscheme("grb256", 256, NULL, grb256_cteam, FALSE);
}

/*end*/
