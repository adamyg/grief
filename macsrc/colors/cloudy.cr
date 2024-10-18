/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: cloudy.cr,v 1.4 2024/10/07 16:14:59 cvsuser Exp $
 * cloudy coloriser, GRIEF port.
 *
 * Original:
 *  Name: cloudy.vim
 *  Author: iyerns <iyerns AT gmail DOT com>
 *  Last Change: 25 August 2005
 *  Version:1.0
 */

#include "../grief.h"

static list                 /*vim style coloriser specification*/
cloudy_colors = {
        "set background=light",
        "hi clear",

        "hi Normal "                      +"guifg=#ffffcc guibg=#003366",
        "hi LineNr "                      +"guifg=white guibg=#003366",
        "hi Statusline "                  +"gui=none guibg=#0099cc guifg=#ffffff",
        "hi StatuslineNC "                +"gui=none guibg=#003399 guifg=#ffffff",

        "hi Title "                       +"guifg=black guibg=white gui=BOLD",
        "hi lCursor "                     +"guibg=Cyan guifg=NONE",

        "hi Comment "                     +"gui=NONE guifg=#cccccc",
        "hi Operator "                    +"guifg=#ff0000",
        "hi Delimiter  "                  +"guifg=black",

        "hi Identifier "                  +"guifg=#33ff99 gui=NONE",

        "hi Statement "                   +"guifg=#cc9966 gui=NONE",
        "hi TypeDef "                     +"guifg=#c000c8 gui=NONE",
        "hi Type "                        +"guifg=#ccffff gui=NONE",
        "hi Boolean "                     +"guifg=#ff00aa gui=NONE",

        "hi String "                      +"guifg=#99ccff gui=NONE",
        "hi Number "                      +"guifg=#66ff66 gui=NONE",
        "hi Constant "                    +"guifg=#f0f080 gui=NONE",

        "hi Function "                    +"gui=NONE guifg=#fffcfc",
        "hi PreProc "                     +"guifg=#ffff00 gui=NONE",
        "hi Define "                      +"gui=bold guifg=#f0f0f0",
        "hi Special "                     +"gui=none guifg=#cccccc",
        "hi BrowseDirectory "             +"gui=bold guifg=#FFFF00",

        "hi Keyword "                     +"guifg=#ff8088 gui=NONE",
        "hi Search "                      +"gui=NONE guibg=#ffff00 guifg=#330000",
        "hi IncSearch "                   +"gui=NONE guifg=#fcfcfc guibg=#8888ff",
        "hi SpecialKey "                  +"gui=NONE guifg=#fcfcfc guibg=#8888ff",
        "hi NonText "                     +"gui=NONE guifg=#fcfcfc",
        "hi Directory "                   +"gui=NONE guifg=#999900",
        "hi browseDirectory "             +"gui=NONE guifg=#00F0FF"
        };

void
colorscheme_cloudy(void)
{
        vim_colorscheme("cloudy", 0, NULL, cloudy_colors, TRUE);
}

/*end*/
