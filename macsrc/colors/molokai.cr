/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: molokai.cr,v 1.5 2024/10/08 11:40:03 cvsuser Exp $
 * molokai coloriser, GRIEF port.
 *
 * Original:
 *  Name: molokai.vim
 *  Author: Tomas Restrepo <tomas@winterdom DOT com>
 *  Source: https://github.com/tomasr/molokai
 *
 *  Note: Based on the Monokai theme for TextMate
 *  by Wimer Hazenberg and its darker variant
 *  by Hamish Stuart Macpherson
 */

#include "../grief.h"

static list                 /*vim style coloriser specification*/
molokai_gui = {
        "set background=dark",
        "hi clear",

        "hi Boolean "                     +"guifg=#AE81FF",
        "hi Character "                   +"guifg=#E6DB74",
        "hi Number "                      +"guifg=#AE81FF",
        "hi String "                      +"guifg=#E6DB74",
        "hi Conditional "                 +"guifg=#F92672               gui=bold",
        "hi Constant "                    +"guifg=#AE81FF               gui=bold",
        "hi Cursor "                      +"guifg=#000000 guibg=#F8F8F0",
        "hi iCursor "                     +"guifg=#000000 guibg=#F8F8F0",
        "hi Debug "                       +"guifg=#BCA3A3               gui=bold",
        "hi Define "                      +"guifg=#66D9EF",
        "hi Delimiter "                   +"guifg=#8F8F8F",
        "hi DiffAdd "                     +"guibg=#13354A",
        "hi DiffChange "                  +"guifg=#89807D guibg=#4C4745",
        "hi DiffDelete "                  +"guifg=#960050 guibg=#1E0010",
        "hi DiffText "                    +"guibg=#4C4745 gui=italic,bold",

        "hi Directory "                   +"guifg=#A6E22E               gui=bold",
        "hi Error "                       +"guifg=#E6DB74 guibg=#1E0010",
        "hi ErrorMsg "                    +"guifg=#F92672 guibg=#232526 gui=bold",
        "hi Exception "                   +"guifg=#A6E22E               gui=bold",
        "hi Float "                       +"guifg=#AE81FF",
        "hi FoldColumn "                  +"guifg=#465457 guibg=#000000",
        "hi Folded "                      +"guifg=#465457 guibg=#000000",
        "hi Function "                    +"guifg=#A6E22E",
        "hi Identifier "                  +"guifg=#FD971F",
        "hi Ignore "                      +"guifg=#808080 guibg=bg",
        "hi IncSearch "                   +"guifg=#C4BE89 guibg=#000000",

        "hi Keyword "                     +"guifg=#F92672               gui=bold",
        "hi Label "                       +"guifg=#E6DB74               gui=none",
        "hi Macro "                       +"guifg=#C4BE89               gui=italic",
        "hi SpecialKey "                  +"guifg=#66D9EF               gui=italic",

        "hi MatchParen "                  +"guifg=#000000 guibg=#FD971F gui=bold",
        "hi ModeMsg "                     +"guifg=#E6DB74",
        "hi MoreMsg "                     +"guifg=#E6DB74",
        "hi Operator "                    +"guifg=#F92672",

        // complete menu
        "hi Pmenu "                       +"guifg=#66D9EF guibg=#000000",
        "hi PmenuSel "                    +"guibg=#808080",
        "hi PmenuSbar "                   +"guibg=#080808",
        "hi PmenuThumb "                  +"guifg=#66D9EF",

        "hi PreCondit "                   +"guifg=#A6E22E               gui=bold",
        "hi PreProc "                     +"guifg=#A6E22E",
        "hi Question "                    +"guifg=#66D9EF",
        "hi Repeat "                      +"guifg=#F92672               gui=bold",
        "hi Search "                      +"guifg=#000000 guibg=#FFE792",

        // marks
        "hi SignColumn "                  +"guifg=#A6E22E guibg=#232526",
        "hi SpecialChar "                 +"guifg=#F92672               gui=bold",
        "hi SpecialComment "              +"guifg=#7E8E91               gui=bold",
        "hi Special "                     +"guifg=#66D9EF guibg=bg      gui=italic",
        "hi SpellBad "                    +"guisp=#FF0000 gui=undercurl",
        "hi SpellCap "                    +"guisp=#7070F0 gui=undercurl",
        "hi SpellLocal "                  +"guisp=#70F0F0 gui=undercurl",
        "hi SpellRare "                   +"guisp=#FFFFFF gui=undercurl",

        "hi Statement "                   +"guifg=#F92672               gui=bold",
        "hi StatusLine "                  +"guifg=#455354 guibg=fg",
        "hi StatusLineNC "                +"guifg=#808080 guibg=#080808",
        "hi StorageClass "                +"guifg=#FD971F               gui=italic",
        "hi Structure "                   +"guifg=#66D9EF",
        "hi Tag "                         +"guifg=#F92672               gui=italic",
        "hi Title "                       +"guifg=#ef5939",
        "hi Todo "                        +"guifg=#FFFFFF guibg=bg      gui=bold",

        "hi Typedef "                     +"guifg=#66D9EF",
        "hi Type "                        +"guifg=#66D9EF               gui=none",
        "hi Underlined "                  +"guifg=#808080               gui=underline",

        "hi VertSplit "                   +"guifg=#808080 guibg=#080808 gui=bold",
        "hi VisualNOS "                   +"guibg=#403D3D",
        "hi Visual "                      +"guibg=#403D3D",
        "hi WarningMsg "                  +"guifg=#FFFFFF guibg=#333333 gui=bold",
        "hi WildMenu "                    +"guifg=#66D9EF guibg=#000000",

        "hi TabLineFill "                 +"guifg=#1B1D1E guibg=#1B1D1E",
        "hi TabLine "                     +"guibg=#1B1D1E guifg=#808080 gui=none",

   // if s:molokai_original == 1
   //   "hi Normal "                      +"guifg=#F8F8F2 guibg=#272822",
   //   "hi Comment "                     +"guifg=#75715E",
   //   "hi CursorLine "                  +"guibg=#3E3D32",
   //   "hi CursorLineNr "                +"guifg=#FD971F               gui=none",
   //   "hi CursorColumn "                +"guibg=#3E3D32",
   //   "hi ColorColumn "                 +"guibg=#3B3A32",
   //   "hi LineNr "                      +"guifg=#BCBCBC guibg=#3B3A32",
   //   "hi NonText "                     +"guifg=#75715E",
   //   "hi SpecialKey "                  +"guifg=#75715E",
   // else
        "hi Normal "                      +"guifg=#F8F8F2 guibg=#1B1D1E",
        "hi Comment "                     +"guifg=#7E8E91",
        "hi CursorLine "                  +"guibg=#293739",
        "hi CursorLineNr "                +"guifg=#FD971F               gui=none",
        "hi CursorColumn "                +"guibg=#293739",
        "hi ColorColumn "                 +"guibg=#232526",
        "hi LineNr "                      +"guifg=#465457 guibg=#232526",
        "hi NonText "                     +"guifg=#465457",
        "hi SpecialKey "                  +"guifg=#465457"
        };

static list                 /*vim style coloriser specification*/
molokai_cteam = {
        "set background=dark",
        "hi clear",

   // molokai_original == 1
   //   "hi Normal "                      +"ctermbg=234",
   //   "hi CursorLine "                  +"              ctermbg=235  cterm=none",
   //   "hi CursorLineNr "                +"ctermfg=208                cterm=none",
   // else
        "hi Normal "                      +"ctermfg=252  ctermbg=233",
        "hi CursorLine "                  +"ctermbg=234  cterm=none",
        "hi CursorLineNr "                +"ctermfg=208                cterm=none",

        "hi Boolean "                     +"ctermfg=135",
        "hi Character "                   +"ctermfg=144",
        "hi Number "                      +"ctermfg=135",
        "hi String "                      +"ctermfg=144",
        "hi Conditional "                 +"ctermfg=161               cterm=bold",
        "hi Constant "                    +"ctermfg=135               cterm=bold",
        "hi Cursor "                      +"ctermfg=16  ctermbg=253",
        "hi Debug "                       +"ctermfg=225               cterm=bold",
        "hi Define "                      +"ctermfg=81",
        "hi Delimiter "                   +"ctermfg=241",

        "hi DiffAdd "                     +"ctermbg=24",
        "hi DiffChange "                  +"ctermfg=181 ctermbg=239",
        "hi DiffDelete "                  +"ctermfg=162 ctermbg=53",
        "hi DiffText "                    +"ctermbg=102 cterm=bold",

        "hi Directory "                   +"ctermfg=118               cterm=bold",
        "hi Error "                       +"ctermfg=219 ctermbg=89",
        "hi ErrorMsg "                    +"ctermfg=199 ctermbg=16    cterm=bold",
        "hi Exception "                   +"ctermfg=118               cterm=bold",
        "hi Float "                       +"ctermfg=135",
        "hi FoldColumn "                  +"ctermfg=67  ctermbg=16",
        "hi Folded "                      +"ctermfg=67  ctermbg=16",
        "hi Function "                    +"ctermfg=118",
        "hi Identifier "                  +"ctermfg=208               cterm=none",
        "hi Ignore "                      +"ctermfg=244 ctermbg=232",
        "hi IncSearch "                   +"ctermfg=193 ctermbg=16",

        "hi keyword "                     +"ctermfg=161               cterm=bold",
        "hi Label "                       +"ctermfg=229               cterm=none",
        "hi Macro "                       +"ctermfg=193",
        "hi SpecialKey "                  +"ctermfg=81",

        "hi MatchParen "                  +"ctermfg=233 ctermbg=208   cterm=bold",
        "hi ModeMsg "                     +"ctermfg=229",
        "hi MoreMsg "                     +"ctermfg=229",
        "hi Operator "                    +"ctermfg=161",

        // complete menu
        "hi Pmenu "                       +"ctermfg=81  ctermbg=16",
        "hi PmenuSel "                    +"ctermfg=255 ctermbg=242",
        "hi PmenuSbar "                   +"            ctermbg=232",
        "hi PmenuThumb "                  +"            ctermfg=81",

        "hi PreCondit "                   +"ctermfg=118               cterm=bold",
        "hi PreProc "                     +"ctermfg=118",
        "hi Question "                    +"ctermfg=81",
        "hi Repeat "                      +"ctermfg=161               cterm=bold",
        "hi Search "                      +"ctermfg=0   ctermbg=222   cterm=NONE",

        // marks column
        "hi SignColumn "                  +"ctermfg=118 ctermbg=235",
        "hi SpecialChar "                 +"ctermfg=161               cterm=bold",
        "hi SpecialComment "              +"ctermfg=245               cterm=bold",
        "hi Special "                     +"ctermfg=81",
        "hi SpellBad "                    +"            ctermbg=52",
        "hi SpellCap "                    +"            ctermbg=17",
        "hi SpellLocal "                  +"            ctermbg=17",
        "hi SpellRare "                   +"ctermfg=none ctermbg=none cterm=reverse",

        "hi Statement "                   +"ctermfg=161               cterm=bold",
        "hi StatusLine "                  +"ctermfg=238 ctermbg=253",
        "hi StatusLineNC "                +"ctermfg=244 ctermbg=232",
        "hi StorageClass "                +"ctermfg=208",
        "hi Structure "                   +"ctermfg=81",
        "hi Tag "                         +"ctermfg=161",
        "hi Title "                       +"ctermfg=166",
        "hi Todo "                        +"ctermfg=231 ctermbg=232   cterm=bold",

        "hi Typedef "                     +"ctermfg=81",
        "hi Type "                        +"ctermfg=81                cterm=none",
        "hi Underlined "                  +"ctermfg=244               cterm=underline",

        "hi VertSplit "                   +"ctermfg=244 ctermbg=232   cterm=bold",
        "hi VisualNOS "                   +"            ctermbg=238",
        "hi Visual "                      +"            ctermbg=235",
        "hi WarningMsg "                  +"ctermfg=231 ctermbg=238   cterm=bold",
        "hi WildMenu "                    +"ctermfg=81  ctermbg=16",

        "hi Comment "                     +"ctermfg=59",
        "hi CursorColumn "                +"            ctermbg=236",
        "hi ColorColumn "                 +"            ctermbg=236",
        "hi LineNr "                      +"ctermfg=250 ctermbg=236",
        "hi NonText "                     +"ctermfg=59",

        "hi SpecialKey "                  +"ctermfg=59",
        };

int
colorscheme_molokai(void)
{
        const string scheme = "molokai";
        int colordepth, truecolor;

        get_term_feature(TF_COLORDEPTH, colordepth);
        get_term_feature(TF_TRUECOLOR, truecolor);
        if (colordepth < 256) {
                error("%s: color depth not supported", scheme);
                return -1;
        }

        if (truecolor) {
                vim_colorscheme("molokai", 0, NULL, molokai_gui, TRUE);
        } else {
                vim_colorscheme("molokai", 256, NULL, molokai_cteam, FALSE);
        }
        return 0;
}

/*end*/
