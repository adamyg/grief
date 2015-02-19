/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: clarity.cr,v 1.3 2014/11/27 15:54:11 ayoung Exp $
 * clarity coloriser, GRIEF port.
 *
 *
 */

#include "../grief.h"

static list                 /*vim style coloriser specification*/
clarity_gui = {
        "set background=dark",
        "hi clear",
        "hi Normal          gui=NONE guifg=LightBlue2   guibg=#1F3055",
        "hi Comment         gui=NONE guifg=Grey62       guibg=bg",
        "hi PreProc         gui=NONE guifg=Salmon       guibg=bg",
        "hi Precondit       gui=NONE guifg=Khaki3       guibg=bg",
        "hi Identifier      gui=NONE guifg=Khaki3       guibg=bg",
        "hi Type            gui=BOLD guifg=Orange       guibg=bg",
        "hi StorageClass    gui=BOLD guifg=Cornsilk2    guibg=bg",
        "hi Todo            gui=BOLD guifg=#1F3055      guibg=White",
        "hi NonText         gui=NONE guifg=#334C51      guibg=SteelBlue4",
        "hi LineNr          gui=NONE guifg=HoneyDew2    guibg=Grey25",
        "hi StatusLineNC    gui=NONE guifg=Grey80       guibg=LightBlue4",
        "hi StatusLine      gui=NONE guifg=DarkBlue     guibg=#FFFFCA",
        "hi IncSearch       gui=NONE guifg=Black        guibg=#FFE568",
        "hi Search          gui=UNDERLINE,BOLD          guifg=#FFE568 guibg=bg",
        "hi Cursor          gui=NONE guifg=Grey50       guibg=#FFE568",
        "hi CursorIM        gui=NONE guifg=Grey50       guibg=#FFE568",
        "hi Title           gui=BOLD guifg=OliveDrab3   guibg=bg",
        "hi WarningMsg      gui=BOLD guifg=White        guibg=Red4",
        "hi String          gui=NONE guifg=Grey80       guibg=bg",
        "hi Number          gui=NONE guifg=OliveDrab2   guibg=bg",
        "hi Constant        gui=NONE guifg=#ACEDAB      guibg=bg",
        "hi Visual          gui=BOLD guifg=White        guibg=bg",
        "hi Directory       gui=NONE guifg=PeachPuff    guibg=bg",
        "hi DiffAdd         gui=NONE guifg=white        guibg=SeaGreen",
        "hi DiffChange      gui=BOLD guifg=white        guibg=Blue",
        "hi DiffDelete      gui=NONE guifg=Grey40       guibg=Grey20",
        "hi DiffText        gui=BOLD guifg=HoneyDew1    guibg=FireBrick",
        "hi Typedef         gui=NONE guifg=Cornsilk     guibg=bg",
        "hi Define          gui=NONE guifg=White        guibg=bg",
        "hi Tag             gui=NONE guifg=LightBlue2   guibg=bg",
        "hi Debug           gui=BOLD guifg=Green        guibg=bg",
        "hi Special         gui=NONE guifg=NavajoWhite  guibg=bg",
        "hi SpecialChar     gui=NONE guifg=NavajoWhite  guibg=bg",
        "hi Delimiter       gui=NONE guifg=NavajoWhite  guibg=bg",
        "hi SpecialComment  gui=NONE guifg=NavajoWhite3 guibg=bg",
        "hi Conditional     gui=BOLD guifg=Wheat2       guibg=bg",
        "hi Statement       gui=BOLD guifg=Pink3        guibg=bg",
        "hi WildMenu        gui=NONE guifg=White        guibg=FireBrick",
        "hi browseSuffixes  gui=NONE guifg=Cornsilk3    guibg=bg"
        };

void
colorscheme_clarity(void)
{
        vim_colorscheme("clarity", 256, NULL, clarity_gui, TRUE);
}
/*end*/

