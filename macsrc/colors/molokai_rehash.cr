/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: molokai_rehash.cr,v 1.2 2014/11/27 15:54:14 ayoung Exp $
 * molokai-rehash coloriser, GRIEF port.
 *
 * Original:
 *  Name: molokai.vim
 *  Author: Tomas Restrepo <tomas@winterdom.com>
 *  Source: https://github.com/tomasr/molokai
 *
 *  Note: Based on the Monokai theme for TextMate
 *        by Wimer Hazenberg and its darker variant
 *        by Hamish Stuart Macpherson
 */

#include "../grief.h"

static list                 /*vim style coloriser specification*/
molokai_256_cteam_rehash = {
        "hi Normal          ctermfg=252 ctermbg=234",
        "hi CursorLine                  ctermbg=236   cterm=none",
        "hi CursorLineNr    ctermfg=208               cterm=none",

        "hi Boolean         ctermfg=141",
        "hi Character       ctermfg=222",
        "hi Number          ctermfg=141",
        "hi String          ctermfg=222",
        "hi Conditional     ctermfg=197               cterm=bold",
        "hi Constant        ctermfg=141               cterm=bold",

        "hi DiffDelete      ctermfg=125 ctermbg=233",

        "hi Directory       ctermfg=154               cterm=bold",
        "hi Error           ctermfg=222 ctermbg=233",
        "hi Exception       ctermfg=154               cterm=bold",
        "hi Float           ctermfg=141",
        "hi Function        ctermfg=154",
        "hi Identifier      ctermfg=208",

        "hi Keyword         ctermfg=197               cterm=bold",
        "hi Operator        ctermfg=197",
        "hi PreCondit       ctermfg=154               cterm=bold",
        "hi PreProc         ctermfg=154",
        "hi Repeat          ctermfg=197               cterm=bold",

        "hi Statement       ctermfg=197               cterm=bold",
        "hi Tag             ctermfg=197",
        "hi Title           ctermfg=203",
        "hi Visual                      ctermbg=238",

        "hi Comment         ctermfg=244",
        "hi LineNr          ctermfg=239 ctermbg=235",
        "hi NonText         ctermfg=239",
        "hi SpecialKey      ctermfg=239"
        };

extern void
colorscheme_molokai();

void
colorscheme_molokai_rehash(void)
{
        require("colors/molokai");
        colorscheme_molokai();
        vim_colorscheme("molokai_rehash", 256, NULL, molokai_256_cteam_rehash, FALSE);
}
/*end*/
