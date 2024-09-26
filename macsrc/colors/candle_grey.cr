/* -*- mode: cr; indent-width: 4; -*- */
// $Id: candle_grey.cr,v 1.2 2024/08/04 11:42:44 cvsuser Exp $
// candle-grey, GriefEdit port - beta.
//
// A dark monochrome colorscheme with a hint of color
// Source: https://github.com/aditya-azad/candle-grey
//
// Colors used
//
//    #0D0D0D
//    #404040
//    #8C8C8C
//    #F2F2F2
//    #D99962
//
// MIT License
//
// Copyright (c) 2020-present Aditya Azad
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

void
colorscheme_candle_grey(~ list args)
{
    list x;

    UNUSED(args);

    x += "set background=dark";
    x += "hi clear";
    x += "syntax reset";

    // --------------------------------
    // Editor settings
    // --------------------------------
    x += "hi Normal          ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Cursor          ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi CursorLine      ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi LineNr          ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";
    x += "hi CursorLineNR    ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";

    // -----------------
    // - Number column -
    // -----------------
    x += "hi CursorColumn    ctermfg=NONE     ctermbg=Black    cterm=NONE guifg=NONE    guibg=#0D0D0D gui=NONE";
    x += "hi FoldColumn      ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";
    x += "hi SignColumn      ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";
    x += "hi Folded          ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";

    // -------------------------
    // - Window/Tab delimiters -
    // -------------------------
    x += "hi VertSplit       ctermfg=Black    ctermbg=Black    cterm=NONE guifg=#0D0D0D guibg=#0D0D0D gui=NONE";
    x += "hi ColorColumn     ctermfg=Black    ctermbg=Black    cterm=NONE guifg=#0D0D0D guibg=#0D0D0D gui=NONE";
    x += "hi TabLine         ctermfg=Black    ctermbg=Black    cterm=NONE guifg=#0D0D0D guibg=#0D0D0D gui=NONE";
    x += "hi TabLineFill     ctermfg=Black    ctermbg=Black    cterm=NONE guifg=#0D0D0D guibg=#0D0D0D gui=NONE";
    x += "hi TabLineSel      ctermfg=Black    ctermbg=Black    cterm=NONE guifg=#0D0D0D guibg=#0D0D0D gui=NONE";

    // -------------------------------
    // - File Navigation / Searching -
    // -------------------------------
    x += "hi Directory       ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Search          ctermfg=White    ctermbg=210      cterm=NONE guifg=#0D0D0D guibg=#D99962 gui=NONE";
    x += "hi IncSearch       ctermfg=White    ctermbg=210      cterm=NONE guifg=#0D0D0D guibg=#D99962 gui=NONE";

    // -----------------
    // - Prompt/Status -
    // -----------------
    x += "hi StatusLine      ctermfg=210      ctermbg=Black    cterm=NONE guifg=#D99962 guibg=#0D0D0D gui=NONE";
    x += "hi StatusLineNC    ctermfg=Black    ctermbg=Black    cterm=NONE guifg=#0D0D0D guibg=#0D0D0D gui=NONE";
    x += "hi WildMenu        ctermfg=210      ctermbg=Black    cterm=NONE guifg=#D99962 guibg=#0D0D0D gui=NONE";
    x += "hi Question        ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";
    x += "hi Title           ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi ModeMsg         ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";
    x += "hi MoreMsg         ctermfg=210      ctermbg=Black    cterm=NONE guifg=#D99962 guibg=#0D0D0D gui=NONE";

    // --------------
    // - Visual aid -
    // --------------
    x += "hi MatchParen      ctermfg=210      ctermbg=DarkGray cterm=NONE guifg=#D99962 guibg=#404040 gui=NONE";
    x += "hi Visual          ctermfg=White    ctermbg=DarkGray cterm=NONE guifg=#F2F2F2 guibg=#404040 gui=NONE";
    x += "hi VisualNOS       ctermfg=White    ctermbg=DarkGray cterm=NONE guifg=#F2F2F2 guibg=#404040 gui=NONE";
    x += "hi NonText         ctermfg=Black    ctermbg=Black    cterm=NONE guifg=#0D0D0D guibg=#0D0D0D gui=NONE";

    x += "hi Todo            ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Underlined      ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Error           ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";
    x += "hi ErrorMsg        ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";
    x += "hi WarningMsg      ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";
    x += "hi Ignore          ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";
    x += "hi SpecialKey      ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";
    x += "hi WhiteSpaceChar  ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";
    x += "hi WhiteSpace      ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";

    // --------------------------------
    // Variable types
    // --------------------------------
    x += "hi Constant        ctermfg=Gray     ctermbg=Black    cterm=NONE guifg=#8C8C8C guibg=#0D0D0D gui=NONE";
    x += "hi String          ctermfg=Gray     ctermbg=Black    cterm=NONE guifg=#8C8C8C guibg=#0D0D0D gui=NONE";
    x += "hi StringDelimiter ctermfg=Gray     ctermbg=Black    cterm=NONE guifg=#8C8C8C guibg=#0D0D0D gui=NONE";
    x += "hi Character       ctermfg=Gray     ctermbg=Black    cterm=NONE guifg=#8C8C8C guibg=#0D0D0D gui=NONE";
    x += "hi Number          ctermfg=Gray     ctermbg=Black    cterm=NONE guifg=#8C8C8C guibg=#0D0D0D gui=NONE";
    x += "hi Boolean         ctermfg=Gray     ctermbg=Black    cterm=NONE guifg=#8C8C8C guibg=#0D0D0D gui=NONE";
    x += "hi Float           ctermfg=Gray     ctermbg=Black    cterm=NONE guifg=#8C8C8C guibg=#0D0D0D gui=NONE";

    x += "hi Identifier      ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Function        ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";

    // --------------------------------
    // Language constructs
    // --------------------------------
    x += "hi Statement       ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Conditional     ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Repeat          ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Label           ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Operator        ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Keyword         ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Exception       ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Comment         ctermfg=DarkGray ctermbg=Black    cterm=NONE guifg=#404040 guibg=#0D0D0D gui=NONE";

    x += "hi Special         ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi SpecialChar     ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Tag             ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Delimiter       ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi SpecialComment  ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Debug           ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";

    // ----------
    // - C like -
    // ----------
    x += "hi PreProc         ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Include         ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Define          ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Macro           ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi PreCondit       ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";

    x += "hi Type            ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi StorageClass    ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Structure       ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi Typedef         ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";

    // --------------------------------
    // Diff
    // --------------------------------
    x += "hi DiffAdd         ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi DiffChange      ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi DiffDelete      ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi DiffText        ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";

    // --------------------------------
    // Completion menu
    // --------------------------------
    x += "hi Pmenu           ctermfg=Gray     ctermbg=Black    cterm=NONE guifg=#8C8C8C guibg=#0D0D0D gui=NONE";
    x += "hi PmenuSel        ctermfg=Gray     ctermbg=Black    cterm=NONE guifg=#8C8C8C guibg=#0D0D0D gui=NONE";
    x += "hi PmenuSbar       ctermfg=Gray     ctermbg=Black    cterm=NONE guifg=#8C8C8C guibg=#0D0D0D gui=NONE";
    x += "hi PmenuThumb      ctermfg=Gray     ctermbg=Black    cterm=NONE guifg=#8C8C8C guibg=#0D0D0D gui=NONE";

    // --------------------------------
    // Spelling
    // --------------------------------
    x += "hi SpellBad        ctermfg=210      ctermbg=Black    cterm=NONE guifg=#D99962 guibg=#0D0D0D gui=NONE";
    x += "hi SpellCap        ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi SpellLocal      ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";
    x += "hi SpellRare       ctermfg=White    ctermbg=Black    cterm=NONE guifg=#F2F2F2 guibg=#0D0D0D gui=NONE";

    vim_colorscheme("candle_grey", 0, NULL, x, -1);
}

//end
