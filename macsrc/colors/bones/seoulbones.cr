/* -*- mode: cr; indent-width: 4; -*- */
// $Id: seoulbones.cr,v 1.3 2024/10/27 06:09:51 cvsuser Exp $
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

static void seoulbones_dark(void);
static void seoulbones_light(void);


void
main(void)
{
    module("csbones");
}


static int
seoulbones(int dark)
{
    if (dark) {
        seoulbones_dark();
    } else {
        seoulbones_light();
    }
    return 0;
}


static void
seoulbones_dark(void)
{
//  let s:italics = (&t_ZH != '' && &t_ZH != '[7m') || has('gui_running') || has('nvim')
//  let g:terminal_color_0  = '#4B4B4B'
//  let g:terminal_color_1  = '#E388A3'
//  let g:terminal_color_2  = '#98BD99'
//  let g:terminal_color_3  = '#FFDF9B'
//  let g:terminal_color_4  = '#97BDDE'
//  let g:terminal_color_5  = '#A5A6C5'
//  let g:terminal_color_6  = '#6FBDBE'
//  let g:terminal_color_7  = '#DDDDDD'
//  let g:terminal_color_8  = '#6C6465'
//  let g:terminal_color_9  = '#EB99B1'
//  let g:terminal_color_10 = '#8FCD92'
//  let g:terminal_color_11 = '#FFE5B3'
//  let g:terminal_color_12 = '#A2C8E9'
//  let g:terminal_color_13 = '#B2B3DA'
//  let g:terminal_color_14 = '#6BCACB'
//  let g:terminal_color_15 = '#A8A8A8'

    list x;

    x += "set background=dark";
    x += "hi clear";
    x += "hi Normal guifg=#DDDDDD guibg=#4B4B4B guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link ModeMsg Normal";
    x += "hi Bold guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi BufferVisible guifg=#E5E5E5 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleIndex guifg=#E5E5E5 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleSign guifg=#E5E5E5 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi CocMarkdownLink guifg=#6FBDBE guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi ColorColumn guifg=NONE guibg=#857556 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link LspReferenceRead ColorColumn";
    x += "hi! link LspReferenceText ColorColumn";
    x += "hi! link LspReferenceWrite ColorColumn";
    x += "hi Comment guifg=#719871 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi Conceal guifg=#969696 guibg=NONE guisp=NONE gui=bold,italic cterm=bold,italic";
    x += "hi Constant guifg=#A3A3A3 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Character Constant";
    x += "hi! link Float Constant";
    x += "hi! link TroubleSource Constant";
    x += "hi! link WhichKeyValue Constant";
    x += "hi! link helpOption Constant";
    x += "hi Cursor guifg=#4B4B4B guibg=#E2E2E2 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursor Cursor";
    x += "hi CursorLine guifg=NONE guibg=#525252 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocMenuSel CursorLine";
    x += "hi! link CursorColumn CursorLine";
    x += "hi CursorLineNr guifg=#DDDDDD guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Delimiter guifg=#9B9B9B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownLinkTextDelimiter Delimiter";
    x += "hi! link NotifyERRORIcon DiagnosticError";
    x += "hi! link NotifyERRORTitle DiagnosticError";
    x += "hi DiagnosticHint guifg=#A5A6C5 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyDEBUGIcon DiagnosticHint";
    x += "hi! link NotifyDEBUGTitle DiagnosticHint";
    x += "hi! link NotifyTRACEIcon DiagnosticHint";
    x += "hi! link NotifyTRACETitle DiagnosticHint";
    x += "hi DiagnosticInfo guifg=#97BDDE guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyINFOIcon DiagnosticInfo";
    x += "hi! link NotifyINFOTitle DiagnosticInfo";
    x += "hi DiagnosticOk guifg=#98BD99 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignError guifg=#E388A3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorSign DiagnosticSignError";
    x += "hi DiagnosticSignHint guifg=#A5A6C5 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocHintSign DiagnosticSignHint";
    x += "hi DiagnosticSignInfo guifg=#97BDDE guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocInfoSign DiagnosticSignInfo";
    x += "hi DiagnosticSignOk guifg=#98BD99 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignWarn guifg=#FFDF9B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningSign DiagnosticSignWarn";
    x += "hi DiagnosticUnderlineError guifg=NONE guibg=NONE guisp=#E388A3 gui=undercurl cterm=undercurl";
    x += "hi! link CocErrorHighlight DiagnosticUnderlineError";
    x += "hi DiagnosticUnderlineHint guifg=NONE guibg=NONE guisp=#A5A6C5 gui=undercurl cterm=undercurl";
    x += "hi! link CocHintHighlight DiagnosticUnderlineHint";
    x += "hi DiagnosticUnderlineInfo guifg=NONE guibg=NONE guisp=#97BDDE gui=undercurl cterm=undercurl";
    x += "hi! link CocInfoHighlight DiagnosticUnderlineInfo";
    x += "hi DiagnosticUnderlineOk guifg=NONE guibg=NONE guisp=#98BD99 gui=undercurl cterm=undercurl";
    x += "hi DiagnosticUnderlineWarn guifg=NONE guibg=NONE guisp=#FFDF9B gui=undercurl cterm=undercurl";
    x += "hi! link CocWarningHighlight DiagnosticUnderlineWarn";
    x += "hi DiagnosticVirtualTextError guifg=#E388A3 guibg=#5F5155 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorVirtualText DiagnosticVirtualTextError";
    x += "hi DiagnosticVirtualTextHint guifg=#A5A6C5 guibg=#535461 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextInfo guifg=#97BDDE guibg=#525558 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextOk guifg=#98BD99 guibg=#515651 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextWarn guifg=#FFDF9B guibg=#575451 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningVirtualText DiagnosticVirtualTextWarn";
    x += "hi! link DiagnosticDeprecated DiagnosticWarn";
    x += "hi! link DiagnosticUnnecessary DiagnosticWarn";
    x += "hi! link NotifyWARNIcon DiagnosticWarn";
    x += "hi! link NotifyWARNTitle DiagnosticWarn";
    x += "hi DiffAdd guifg=NONE guibg=#406742 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffChange guifg=NONE guibg=#466177 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffDelete guifg=NONE guibg=#82505E guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffText guifg=#DDDDDD guibg=#5D809B guisp=NONE gui=NONE cterm=NONE";
    x += "hi Directory guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Error guifg=#E388A3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticError Error";
    x += "hi! link ErrorMsg Error";
    x += "hi FlashBackdrop guifg=#8B8B8B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FlashLabel guifg=#DDDDDD guibg=#648BA9 guisp=NONE gui=NONE cterm=NONE";
    x += "hi FloatBorder guifg=#989898 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FoldColumn guifg=#868686 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Folded guifg=#BBBBBB guibg=#636363 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Function guifg=#DFDFC1 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TroubleNormal Function";
    x += "hi! link TroubleText Function";
    x += "hi FzfLuaBufFlagAlt guifg=#97BDDE guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufFlagCur guifg=#FFDF9B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufNr guifg=#98BD99 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderBind guifg=#98BD99 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderText guifg=#FFDF9B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaLiveSym guifg=#FFDF9B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaPathColNr guifg=#AEAEAE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaPathLineNr FzfLuaPathColNr";
    x += "hi FzfLuaTabMarker guifg=#98BD99 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaTabTitle guifg=#6FBDBE guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi GitSignsAdd guifg=#98BD99 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterAdd GitSignsAdd";
    x += "hi GitSignsChange guifg=#97BDDE guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterChange GitSignsChange";
    x += "hi GitSignsDelete guifg=#E388A3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterDelete GitSignsDelete";
    x += "hi IblIndent guifg=#575757 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IblScope guifg=#6F6F6F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Identifier guifg=#DDDDDD guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IncSearch guifg=#4B4B4B guibg=#DCDCE8 guisp=NONE gui=bold cterm=bold";
    x += "hi! link CurSearch IncSearch";
    x += "hi Italic guifg=NONE guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi Keyword guifg=#DC8CA3 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Exception Keyword";
    x += "hi LineNr guifg=#868686 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocCodeLens LineNr";
    x += "hi! link LspCodeLens LineNr";
    x += "hi! link SignColumn LineNr";
    x += "hi LspInlayHint guifg=#9E898C guibg=#525252 guisp=NONE gui=NONE cterm=NONE";
    x += "hi MoreMsg guifg=#98BD99 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Question MoreMsg";
    x += "hi! link NnnNormalNC NnnNormal";
    x += "hi! link NnnVertSplit NnnWinSeparator";
    x += "hi NonText guifg=#7C7C7C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link EndOfBuffer NonText";
    x += "hi! link Whitespace NonText";
    x += "hi NormalFloat guifg=NONE guibg=#5C5C5C guisp=NONE gui=NONE cterm=NONE";
    x += "hi Number guifg=#F7E0B3 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Boolean Number";
    x += "hi Pmenu guifg=NONE guibg=#5C5C5C guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSbar guifg=NONE guibg=#818181 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSel guifg=NONE guibg=#6F6F6F guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuThumb guifg=NONE guibg=#A0A0A0 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PreProc guifg=#D590A3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Search guifg=#DDDDDD guibg=#8283AD guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocSearch Search";
    x += "hi! link MatchParen Search";
    x += "hi! link QuickFixLine Search";
    x += "hi! link Sneak Search";
    x += "hi SneakLabelMask guifg=#A5A6C5 guibg=#A5A6C5 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Special guifg=#BCBCD3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link WhichKeyGroup Special";
    x += "hi SpecialComment guifg=#8B8B8B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownUrl SpecialComment";
    x += "hi SpecialKey guifg=#7C7C7C guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi SpellBad guifg=#D291A3 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi! link CocSelectedText SpellBad";
    x += "hi SpellCap guifg=#D291A3 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi! link SpellLocal SpellCap";
    x += "hi SpellRare guifg=#D291A3 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi Statement guifg=#97BDDE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaBufName Statement";
    x += "hi! link WhichKey Statement";
    x += "hi StatusLine guifg=#DDDDDD guibg=#5E5E5E guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLine StatusLine";
    x += "hi! link WinBar StatusLine";
    x += "hi StatusLineNC guifg=#E5E5E5 guibg=#555555 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLineFill StatusLineNC";
    x += "hi! link WinBarNC StatusLineNC";
    x += "hi String guifg=#ABC4DB guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi TabLineSel guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link BufferCurrent TabLineSel";
    x += "hi Title guifg=#DDDDDD guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Todo guifg=NONE guibg=NONE guisp=NONE gui=bold,underline cterm=bold,underline";
    x += "hi Type guifg=#AEAEAE guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link helpSpecial Type";
    x += "hi! link markdownCode Type";
    x += "hi Underlined guifg=NONE guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi Visual guifg=NONE guibg=#777777 guisp=NONE gui=NONE cterm=NONE";
    x += "hi WarningMsg guifg=#FFDF9B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticWarn WarningMsg";
    x += "hi! link gitcommitOverflow WarningMsg";
    x += "hi WhichKeySeparator guifg=#868686 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi WildMenu guifg=#4B4B4B guibg=#A5A6C5 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link SneakLabel WildMenu";
    x += "hi WinSeparator guifg=#868686 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link VertSplit WinSeparator";
    x += "hi diffAdded guifg=#98BD99 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffChanged guifg=#97BDDE guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffFile guifg=#FFDF9B guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffIndexLine guifg=#FFDF9B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffLine guifg=#A5A6C5 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffNewFile guifg=#98BD99 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffOldFile guifg=#E388A3 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffRemoved guifg=#E388A3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi helpHyperTextEntry guifg=#AEAEAE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi helpHyperTextJump guifg=#BBBBBB guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi lCursor guifg=#4B4B4B guibg=#8E8E8E guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursorNC lCursor";
    x += "hi markdownLinkText guifg=#BBBBBB guibg=NONE guisp=NONE gui=underline cterm=underline";

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


static void
seoulbones_light(void)
{
//  let g:terminal_color_0 = '#E2E2E2'
//  let g:terminal_color_1 = '#DC5284'
//  let g:terminal_color_2 = '#628562'
//  let g:terminal_color_3 = '#C48562'
//  let g:terminal_color_4 = '#0084A3'
//  let g:terminal_color_5 = '#896788'
//  let g:terminal_color_6 = '#008586'
//  let g:terminal_color_7 = '#555555'
//  let g:terminal_color_8 = '#BFBABB'
//  let g:terminal_color_9 = '#BE3C6D'
//  let g:terminal_color_10 = '#487249'
//  let g:terminal_color_11 = '#A76B48'
//  let g:terminal_color_12 = '#006F89'
//  let g:terminal_color_13 = '#7F4C7E'
//  let g:terminal_color_14 = '#006F70'
//  let g:terminal_color_15 = '#777777'

    list x;

    x += "set background=light";
    x += "hi clear";
    x += "hi Normal guifg=#555555 guibg=#E2E2E2 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link ModeMsg Normal";
    x += "hi Bold guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi BufferVisible guifg=#818181 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleIndex guifg=#818181 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleSign guifg=#818181 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi CocMarkdownLink guifg=#008586 guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi ColorColumn guifg=NONE guibg=#E2B8A6 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link LspReferenceRead ColorColumn";
    x += "hi! link LspReferenceText ColorColumn";
    x += "hi! link LspReferenceWrite ColorColumn";
    x += "hi Comment guifg=#628562 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi Conceal guifg=#777777 guibg=NONE guisp=NONE gui=bold,italic cterm=bold,italic";
    x += "hi Constant guifg=#7C7C7C guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Character Constant";
    x += "hi! link Float Constant";
    x += "hi! link TroubleSource Constant";
    x += "hi! link WhichKeyValue Constant";
    x += "hi! link helpOption Constant";
    x += "hi Cursor guifg=#E2E2E2 guibg=#555555 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursor Cursor";
    x += "hi CursorLine guifg=NONE guibg=#DADADA guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocMenuSel CursorLine";
    x += "hi! link CursorColumn CursorLine";
    x += "hi CursorLineNr guifg=#555555 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Delimiter guifg=#7C7C7C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownLinkTextDelimiter Delimiter";
    x += "hi! link NotifyERRORIcon DiagnosticError";
    x += "hi! link NotifyERRORTitle DiagnosticError";
    x += "hi DiagnosticHint guifg=#896788 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyDEBUGIcon DiagnosticHint";
    x += "hi! link NotifyDEBUGTitle DiagnosticHint";
    x += "hi! link NotifyTRACEIcon DiagnosticHint";
    x += "hi! link NotifyTRACETitle DiagnosticHint";
    x += "hi DiagnosticInfo guifg=#0084A3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyINFOIcon DiagnosticInfo";
    x += "hi! link NotifyINFOTitle DiagnosticInfo";
    x += "hi DiagnosticOk guifg=#628562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignError guifg=#DC5284 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorSign DiagnosticSignError";
    x += "hi DiagnosticSignHint guifg=#896788 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocHintSign DiagnosticSignHint";
    x += "hi DiagnosticSignInfo guifg=#0084A3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocInfoSign DiagnosticSignInfo";
    x += "hi DiagnosticSignOk guifg=#628562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignWarn guifg=#C48562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningSign DiagnosticSignWarn";
    x += "hi DiagnosticUnderlineError guifg=NONE guibg=NONE guisp=#DC5284 gui=undercurl cterm=undercurl";
    x += "hi! link CocErrorHighlight DiagnosticUnderlineError";
    x += "hi DiagnosticUnderlineHint guifg=NONE guibg=NONE guisp=#896788 gui=undercurl cterm=undercurl";
    x += "hi! link CocHintHighlight DiagnosticUnderlineHint";
    x += "hi DiagnosticUnderlineInfo guifg=NONE guibg=NONE guisp=#0084A3 gui=undercurl cterm=undercurl";
    x += "hi! link CocInfoHighlight DiagnosticUnderlineInfo";
    x += "hi DiagnosticUnderlineOk guifg=NONE guibg=NONE guisp=#628562 gui=undercurl cterm=undercurl";
    x += "hi DiagnosticUnderlineWarn guifg=NONE guibg=NONE guisp=#C48562 gui=undercurl cterm=undercurl";
    x += "hi! link CocWarningHighlight DiagnosticUnderlineWarn";
    x += "hi DiagnosticVirtualTextError guifg=#DC5284 guibg=#E9D1D7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorVirtualText DiagnosticVirtualTextError";
    x += "hi DiagnosticVirtualTextHint guifg=#896788 guibg=#E8D0E7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextInfo guifg=#0084A3 guibg=#C2DBE8 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextOk guifg=#628562 guibg=#A3E8A3 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextWarn guifg=#C48562 guibg=#E9D3CA guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningVirtualText DiagnosticVirtualTextWarn";
    x += "hi! link DiagnosticDeprecated DiagnosticWarn";
    x += "hi! link DiagnosticUnnecessary DiagnosticWarn";
    x += "hi! link NotifyWARNIcon DiagnosticWarn";
    x += "hi! link NotifyWARNTitle DiagnosticWarn";
    x += "hi DiffAdd guifg=NONE guibg=#AEDEAE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffChange guifg=NONE guibg=#C0D5E0 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffDelete guifg=NONE guibg=#E5CBD1 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffText guifg=#555555 guibg=#99B5C3 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Directory guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Error guifg=#DC5284 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticError Error";
    x += "hi! link ErrorMsg Error";
    x += "hi FlashBackdrop guifg=#868686 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FlashLabel guifg=#555555 guibg=#00C9F6 guisp=NONE gui=NONE cterm=NONE";
    x += "hi FloatBorder guifg=#6A6A6A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FoldColumn guifg=#919191 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Folded guifg=#4B4B4B guibg=#BBBBBB guisp=NONE gui=NONE cterm=NONE";
    x += "hi Function guifg=#6C6B20 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TroubleNormal Function";
    x += "hi! link TroubleText Function";
    x += "hi FzfLuaBufFlagAlt guifg=#0084A3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufFlagCur guifg=#C48562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufNr guifg=#628562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderBind guifg=#628562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderText guifg=#C48562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaLiveSym guifg=#C48562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaPathColNr guifg=#6D4C52 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaPathLineNr FzfLuaPathColNr";
    x += "hi FzfLuaTabMarker guifg=#628562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaTabTitle guifg=#008586 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi GitSignsAdd guifg=#628562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterAdd GitSignsAdd";
    x += "hi GitSignsChange guifg=#0084A3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterChange GitSignsChange";
    x += "hi GitSignsDelete guifg=#DC5284 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterDelete GitSignsDelete";
    x += "hi IblIndent guifg=#D4D4D4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IblScope guifg=#ABABAB guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Identifier guifg=#555555 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IncSearch guifg=#E2E2E2 guibg=#9E779D guisp=NONE gui=bold cterm=bold";
    x += "hi! link CurSearch IncSearch";
    x += "hi Italic guifg=NONE guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi Keyword guifg=#CA6284 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Exception Keyword";
    x += "hi LineNr guifg=#919191 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocCodeLens LineNr";
    x += "hi! link LspCodeLens LineNr";
    x += "hi! link SignColumn LineNr";
    x += "hi LspInlayHint guifg=#9C868A guibg=#DDDDDD guisp=NONE gui=NONE cterm=NONE";
    x += "hi MoreMsg guifg=#628562 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Question MoreMsg";
    x += "hi! link NnnNormalNC NnnNormal";
    x += "hi! link NnnVertSplit NnnWinSeparator";
    x += "hi NonText guifg=#A6A6A6 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link EndOfBuffer NonText";
    x += "hi! link Whitespace NonText";
    x += "hi NormalFloat guifg=NONE guibg=#CFCFCF guisp=NONE gui=NONE cterm=NONE";
    x += "hi Number guifg=#896500 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Boolean Number";
    x += "hi Pmenu guifg=NONE guibg=#C9C9C9 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSbar guifg=NONE guibg=#9E9E9E guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSel guifg=NONE guibg=#B0B0B0 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuThumb guifg=NONE guibg=#F3F3F3 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PreProc guifg=#BE6A84 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Search guifg=#555555 guibg=#CBB1CA guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocSearch Search";
    x += "hi! link MatchParen Search";
    x += "hi! link QuickFixLine Search";
    x += "hi! link Sneak Search";
    x += "hi SneakLabelMask guifg=#896788 guibg=#896788 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Special guifg=#755F74 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link WhichKeyGroup Special";
    x += "hi SpecialComment guifg=#868686 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownUrl SpecialComment";
    x += "hi SpecialKey guifg=#A6A6A6 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi SpellBad guifg=#C66484 guibg=NONE guisp=#DC5284 gui=undercurl cterm=undercurl";
    x += "hi! link CocSelectedText SpellBad";
    x += "hi SpellCap guifg=#C66484 guibg=NONE guisp=#DF6B91 gui=undercurl cterm=undercurl";
    x += "hi! link SpellLocal SpellCap";
    x += "hi SpellRare guifg=#C66484 guibg=NONE guisp=#C48562 gui=undercurl cterm=undercurl";
    x += "hi Statement guifg=#0084A3 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaBufName Statement";
    x += "hi! link WhichKey Statement";
    x += "hi StatusLine guifg=#555555 guibg=#C4C4C4 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLine StatusLine";
    x += "hi! link WinBar StatusLine";
    x += "hi StatusLineNC guifg=#818181 guibg=#D4D4D4 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLineFill StatusLineNC";
    x += "hi! link WinBarNC StatusLineNC";
    x += "hi String guifg=#4A7587 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi TabLineSel guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link BufferCurrent TabLineSel";
    x += "hi Title guifg=#555555 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Todo guifg=NONE guibg=NONE guisp=NONE gui=bold,underline cterm=bold,underline";
    x += "hi Type guifg=#6D4C52 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link helpSpecial Type";
    x += "hi! link markdownCode Type";
    x += "hi Underlined guifg=NONE guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi Visual guifg=NONE guibg=#CCCCCC guisp=NONE gui=NONE cterm=NONE";
    x += "hi WarningMsg guifg=#C48562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticWarn WarningMsg";
    x += "hi! link gitcommitOverflow WarningMsg";
    x += "hi WhichKeySeparator guifg=#919191 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi WildMenu guifg=#E2E2E2 guibg=#896788 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link SneakLabel WildMenu";
    x += "hi WinSeparator guifg=#919191 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link VertSplit WinSeparator";
    x += "hi diffAdded guifg=#628562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffChanged guifg=#0084A3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffFile guifg=#C48562 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffIndexLine guifg=#C48562 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffLine guifg=#896788 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffNewFile guifg=#628562 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffOldFile guifg=#DC5284 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffRemoved guifg=#DC5284 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi helpHyperTextEntry guifg=#6D4C52 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi helpHyperTextJump guifg=#6D6D6D guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi lCursor guifg=#E2E2E2 guibg=#747474 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursorNC lCursor";
    x += "hi markdownLinkText guifg=#6D6D6D guibg=NONE guisp=NONE gui=underline cterm=underline";

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
