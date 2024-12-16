/* -*- mode: cr; indent-width: 4; -*- */
// $Id: kanagawabones.cr,v 1.3 2024/10/27 06:09:51 cvsuser Exp $
// zenbones coloriser collection, GriefEdit port - beta.
//
// Source: https://github.com/zenbones-theme
//
// MIT License
//
// Copyright (c) 2022 Michael Chris Lopez
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

#include "../../grief.h"

extern void bones_scheme_set(list x);

static void zenbones_dark(void);
static void zenbones_light(void);


void
main(void)
{
    module("csbones");
}


static int
zenbones(int dark)
{
    if (dark) {
        zenbones_dark();
        return 0;
    }
    return -1;
}


static void
zenbones_dark(void)
{
//  let s:italics = (&t_ZH != '' && &t_ZH != '[7m') || has('gui_running') || has('nvim')
//
//  let g:terminal_color_0  = '#1F1F28'
//  let g:terminal_color_1  = '#E46A78'
//  let g:terminal_color_2  = '#98BC6D'
//  let g:terminal_color_3  = '#E5C283'
//  let g:terminal_color_4  = '#7EB3C9'
//  let g:terminal_color_5  = '#957FB8'
//  let g:terminal_color_6  = '#7EB3C9'
//  let g:terminal_color_7  = '#DDD8BB'
//  let g:terminal_color_8  = '#3C3C51'
//  let g:terminal_color_9  = '#EC818C'
//  let g:terminal_color_10 = '#9EC967'
//  let g:terminal_color_11 = '#F1C982'
//  let g:terminal_color_12 = '#7BC2DF'
//  let g:terminal_color_13 = '#A98FD2'
//  let g:terminal_color_14 = '#7BC2DF'
//  let g:terminal_color_15 = '#A8A48D'

    list x;

    x += "set background=dark";
    x += "hi clear";
    x += "hi Normal guifg=#DDD8BB guibg=#1F1F28 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link ModeMsg Normal";
    x += "hi Bold guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi BufferVisible guifg=#E9E3C5 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleIndex guifg=#E9E3C5 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleSign guifg=#E9E3C5 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi CocMarkdownLink guifg=#7EB3C9 guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi ColorColumn guifg=NONE guibg=#514531 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link LspReferenceRead ColorColumn";
    x += "hi! link LspReferenceText ColorColumn";
    x += "hi! link LspReferenceWrite ColorColumn";
    x += "hi Comment guifg=#696977 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi Conceal guifg=#95917D guibg=NONE guisp=NONE gui=bold,italic cterm=bold,italic";
    x += "hi Constant guifg=#A29E89 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Character Constant";
    x += "hi! link Float Constant";
    x += "hi! link String Constant";
    x += "hi! link TroubleSource Constant";
    x += "hi! link WhichKeyValue Constant";
    x += "hi! link helpOption Constant";
    x += "hi Cursor guifg=#1F1F28 guibg=#E6E0C2 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursor Cursor";
    x += "hi CursorLine guifg=NONE guibg=#272732 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocMenuSel CursorLine";
    x += "hi! link CursorColumn CursorLine";
    x += "hi CursorLineNr guifg=#DDD8BB guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Delimiter guifg=#7D7D8D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownLinkTextDelimiter Delimiter";
    x += "hi! link NotifyERRORIcon DiagnosticError";
    x += "hi! link NotifyERRORTitle DiagnosticError";
    x += "hi DiagnosticHint guifg=#957FB8 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyDEBUGIcon DiagnosticHint";
    x += "hi! link NotifyDEBUGTitle DiagnosticHint";
    x += "hi! link NotifyTRACEIcon DiagnosticHint";
    x += "hi! link NotifyTRACETitle DiagnosticHint";
    x += "hi DiagnosticInfo guifg=#7EB3C9 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyINFOIcon DiagnosticInfo";
    x += "hi! link NotifyINFOTitle DiagnosticInfo";
    x += "hi DiagnosticOk guifg=#98BC6D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignError guifg=#E46A78 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorSign DiagnosticSignError";
    x += "hi DiagnosticSignHint guifg=#957FB8 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocHintSign DiagnosticSignHint";
    x += "hi DiagnosticSignInfo guifg=#7EB3C9 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocInfoSign DiagnosticSignInfo";
    x += "hi DiagnosticSignOk guifg=#98BC6D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignWarn guifg=#E5C283 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningSign DiagnosticSignWarn";
    x += "hi DiagnosticUnderlineError guifg=NONE guibg=NONE guisp=#E46A78 gui=undercurl cterm=undercurl";
    x += "hi! link CocErrorHighlight DiagnosticUnderlineError";
    x += "hi DiagnosticUnderlineHint guifg=NONE guibg=NONE guisp=#957FB8 gui=undercurl cterm=undercurl";
    x += "hi! link CocHintHighlight DiagnosticUnderlineHint";
    x += "hi DiagnosticUnderlineInfo guifg=NONE guibg=NONE guisp=#7EB3C9 gui=undercurl cterm=undercurl";
    x += "hi! link CocInfoHighlight DiagnosticUnderlineInfo";
    x += "hi DiagnosticUnderlineOk guifg=NONE guibg=NONE guisp=#98BC6D gui=undercurl cterm=undercurl";
    x += "hi DiagnosticUnderlineWarn guifg=NONE guibg=NONE guisp=#E5C283 gui=undercurl cterm=undercurl";
    x += "hi! link CocWarningHighlight DiagnosticUnderlineWarn";
    x += "hi DiagnosticVirtualTextError guifg=#E46A78 guibg=#2E2626 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorVirtualText DiagnosticVirtualTextError";
    x += "hi DiagnosticVirtualTextHint guifg=#957FB8 guibg=#29272D guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextInfo guifg=#7EB3C9 guibg=#262829 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextOk guifg=#98BC6D guibg=#272826 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextWarn guifg=#E5C283 guibg=#292826 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningVirtualText DiagnosticVirtualTextWarn";
    x += "hi! link DiagnosticDeprecated DiagnosticWarn";
    x += "hi! link DiagnosticUnnecessary DiagnosticWarn";
    x += "hi! link NotifyWARNIcon DiagnosticWarn";
    x += "hi! link NotifyWARNTitle DiagnosticWarn";
    x += "hi DiffAdd guifg=NONE guibg=#2A331F guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffChange guifg=NONE guibg=#22333A guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffDelete guifg=NONE guibg=#47272A guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffText guifg=#DDD8BB guibg=#364F59 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Directory guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Error guifg=#E46A78 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticError Error";
    x += "hi! link ErrorMsg Error";
    x += "hi FlashBackdrop guifg=#696977 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FlashLabel guifg=#DDD8BB guibg=#3C5965 guisp=NONE gui=NONE cterm=NONE";
    x += "hi FloatBorder guifg=#7B7B8B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FoldColumn guifg=#646476 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Folded guifg=#A5A5B0 guibg=#383846 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Function guifg=#DDD8BB guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TroubleNormal Function";
    x += "hi! link TroubleText Function";
    x += "hi FzfLuaBufFlagAlt guifg=#7EB3C9 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufFlagCur guifg=#E5C283 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufNr guifg=#98BC6D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderBind guifg=#98BC6D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderText guifg=#E5C283 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaLiveSym guifg=#E5C283 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaPathColNr guifg=#9797A5 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaPathLineNr FzfLuaPathColNr";
    x += "hi FzfLuaTabMarker guifg=#98BC6D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaTabTitle guifg=#7EB3C9 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi GitSignsAdd guifg=#98BC6D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterAdd GitSignsAdd";
    x += "hi GitSignsChange guifg=#7EB3C9 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterChange GitSignsChange";
    x += "hi GitSignsDelete guifg=#E46A78 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterDelete GitSignsDelete";
    x += "hi IblIndent guifg=#2D2D37 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IblScope guifg=#484856 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Identifier guifg=#BBB79E guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IncSearch guifg=#1F1F28 guibg=#AE9FCA guisp=NONE gui=bold cterm=bold";
    x += "hi! link CurSearch IncSearch";
    x += "hi Italic guifg=NONE guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi LineNr guifg=#646476 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocCodeLens LineNr";
    x += "hi! link LspCodeLens LineNr";
    x += "hi! link SignColumn LineNr";
    x += "hi LspInlayHint guifg=#6D6D8C guibg=#272732 guisp=NONE gui=NONE cterm=NONE";
    x += "hi MoreMsg guifg=#98BC6D guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Question MoreMsg";
    x += "hi! link NnnNormalNC NnnNormal";
    x += "hi! link NnnVertSplit NnnWinSeparator";
    x += "hi NonText guifg=#58586A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link EndOfBuffer NonText";
    x += "hi! link Whitespace NonText";
    x += "hi NormalFloat guifg=NONE guibg=#31313F guisp=NONE gui=NONE cterm=NONE";
    x += "hi Number guifg=#DDD8BB guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Boolean Number";
    x += "hi Pmenu guifg=NONE guibg=#31313F guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSbar guifg=NONE guibg=#5D5D6F guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSel guifg=NONE guibg=#484759 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuThumb guifg=NONE guibg=#858594 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Search guifg=#DDD8BB guibg=#614A82 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocSearch Search";
    x += "hi! link MatchParen Search";
    x += "hi! link QuickFixLine Search";
    x += "hi! link Sneak Search";
    x += "hi SneakLabelMask guifg=#957FB8 guibg=#957FB8 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Special guifg=#ADA992 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link WhichKeyGroup Special";
    x += "hi SpecialComment guifg=#696977 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownUrl SpecialComment";
    x += "hi SpecialKey guifg=#58586A guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi SpellBad guifg=#D0777F guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi! link CocSelectedText SpellBad";
    x += "hi SpellCap guifg=#D0777F guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi! link SpellLocal SpellCap";
    x += "hi SpellRare guifg=#D0777F guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi Statement guifg=#DDD8BB guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaBufName Statement";
    x += "hi! link PreProc Statement";
    x += "hi! link WhichKey Statement";
    x += "hi StatusLine guifg=#DDD8BB guibg=#363644 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLine StatusLine";
    x += "hi! link WinBar StatusLine";
    x += "hi StatusLineNC guifg=#E9E3C5 guibg=#292934 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLineFill StatusLineNC";
    x += "hi! link WinBarNC StatusLineNC";
    x += "hi TabLineSel guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link BufferCurrent TabLineSel";
    x += "hi Title guifg=#DDD8BB guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Todo guifg=NONE guibg=NONE guisp=NONE gui=bold,underline cterm=bold,underline";
    x += "hi Type guifg=#9797A5 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link helpSpecial Type";
    x += "hi! link markdownCode Type";
    x += "hi Underlined guifg=NONE guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi Visual guifg=NONE guibg=#49473E guisp=NONE gui=NONE cterm=NONE";
    x += "hi WarningMsg guifg=#E5C283 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticWarn WarningMsg";
    x += "hi! link gitcommitOverflow WarningMsg";
    x += "hi WhichKeySeparator guifg=#646476 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi WildMenu guifg=#1F1F28 guibg=#957FB8 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link SneakLabel WildMenu";
    x += "hi WinSeparator guifg=#646476 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link VertSplit WinSeparator";
    x += "hi diffAdded guifg=#98BC6D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffChanged guifg=#7EB3C9 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffFile guifg=#E5C283 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffIndexLine guifg=#E5C283 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffLine guifg=#957FB8 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffNewFile guifg=#98BC6D guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffOldFile guifg=#E46A78 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffRemoved guifg=#E46A78 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi helpHyperTextEntry guifg=#9797A5 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi helpHyperTextJump guifg=#BBB79E guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi lCursor guifg=#1F1F28 guibg=#8F8C78 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursorNC lCursor";
    x += "hi markdownLinkText guifg=#BBB79E guibg=NONE guisp=NONE gui=underline cterm=underline";

#if (0)
    if !s:italics
        x += "hi Boolean gui=NONE cterm=NONE";
        x += "hi Character gui=NONE cterm=NONE";
        x += "hi Comment gui=NONE cterm=NONE";
        x += "hi Constant gui=NONE cterm=NONE";
        x += "hi Float gui=NONE cterm=NONE";
        x += "hi Number gui=NONE cterm=NONE";
        x += "hi SpecialKey gui=NONE cterm=NONE";
        x += "hi String gui=NONE cterm=NONE";
        x += "hi TroubleSource gui=NONE cterm=NONE";
        x += "hi WhichKeyValue gui=NONE cterm=NONE";
        x += "hi diffNewFile gui=NONE cterm=NONE";
        x += "hi diffOldFile gui=NONE cterm=NONE";
        x += "hi helpOption gui=NONE cterm=NONE";
    endif
#endif

    bones_scheme_set(x);
}

//end
