/* -*- mode: cr; indent-width: 4; -*- */
// $Id: rosebones.cr,v 1.3 2024/10/27 06:09:51 cvsuser Exp $
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

static void rosebones_dark(void);
static void rosebones_light(void);


void
main(void)
{
    module("csbones");
}


static int
rosebones(int dark)
{
    if (dark) {
        rosebones_dark();
    } else {
        rosebones_light();
    }
    return 0;
}


static void
rosebones_dark(void)
{
//  let s:italics = (&t_ZH != '' && &t_ZH != '[7m') || has('gui_running') || has('nvim')
//  let g:terminal_color_0  = '#1A1825'
//  let g:terminal_color_1  = '#EB7193'
//  let g:terminal_color_2  = '#317490'
//  let g:terminal_color_3  = '#F6C074'
//  let g:terminal_color_4  = '#9CCFD8'
//  let g:terminal_color_5  = '#C4A7E7'
//  let g:terminal_color_6  = '#9CCFD8'
//  let g:terminal_color_7  = '#E1D4D4'
//  let g:terminal_color_8  = '#3A3651'
//  let g:terminal_color_9  = '#F289A4'
//  let g:terminal_color_10 = '#358DAF'
//  let g:terminal_color_11 = '#F9CA8E'
//  let g:terminal_color_12 = '#94DAE6'
//  let g:terminal_color_13 = '#CEB3EF'
//  let g:terminal_color_14 = '#94DAE6'
//  let g:terminal_color_15 = '#BF9B99'

    list x;

    x += "set background=dark";
    x += "hi clear";
    x += "hi Normal guifg=#E1D4D4 guibg=#1A1825 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link ModeMsg Normal";
    x += "hi Bold guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi BufferVisible guifg=#E9E0E0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleIndex guifg=#E9E0E0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleSign guifg=#E9E0E0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi CocMarkdownLink guifg=#9CCFD8 guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi ColorColumn guifg=NONE guibg=#4C3D2C guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link LspReferenceRead ColorColumn";
    x += "hi! link LspReferenceText ColorColumn";
    x += "hi! link LspReferenceWrite ColorColumn";
    x += "hi Comment guifg=#69657E guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi Conceal guifg=#B48583 guibg=NONE guisp=NONE gui=bold,italic cterm=bold,italic";
    x += "hi Constant guifg=#BC9493 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Character Constant";
    x += "hi! link Float Constant";
    x += "hi! link String Constant";
    x += "hi! link TroubleSource Constant";
    x += "hi! link WhichKeyValue Constant";
    x += "hi! link helpOption Constant";
    x += "hi Cursor guifg=#1A1825 guibg=#E7DDDD guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursor Cursor";
    x += "hi CursorLine guifg=NONE guibg=#222030 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocMenuSel CursorLine";
    x += "hi! link CursorColumn CursorLine";
    x += "hi CursorLineNr guifg=#E1D4D4 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Delimiter guifg=#7D7997 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownLinkTextDelimiter Delimiter";
    x += "hi! link NotifyERRORIcon DiagnosticError";
    x += "hi! link NotifyERRORTitle DiagnosticError";
    x += "hi DiagnosticHint guifg=#C4A7E7 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyDEBUGIcon DiagnosticHint";
    x += "hi! link NotifyDEBUGTitle DiagnosticHint";
    x += "hi! link NotifyTRACEIcon DiagnosticHint";
    x += "hi! link NotifyTRACETitle DiagnosticHint";
    x += "hi DiagnosticInfo guifg=#9CCFD8 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyINFOIcon DiagnosticInfo";
    x += "hi! link NotifyINFOTitle DiagnosticInfo";
    x += "hi DiagnosticOk guifg=#317490 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignError guifg=#EB7193 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorSign DiagnosticSignError";
    x += "hi DiagnosticSignHint guifg=#C4A7E7 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocHintSign DiagnosticSignHint";
    x += "hi DiagnosticSignInfo guifg=#9CCFD8 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocInfoSign DiagnosticSignInfo";
    x += "hi DiagnosticSignOk guifg=#317490 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignWarn guifg=#F6C074 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningSign DiagnosticSignWarn";
    x += "hi DiagnosticUnderlineError guifg=NONE guibg=NONE guisp=#EB7193 gui=undercurl cterm=undercurl";
    x += "hi! link CocErrorHighlight DiagnosticUnderlineError";
    x += "hi DiagnosticUnderlineHint guifg=NONE guibg=NONE guisp=#C4A7E7 gui=undercurl cterm=undercurl";
    x += "hi! link CocHintHighlight DiagnosticUnderlineHint";
    x += "hi DiagnosticUnderlineInfo guifg=NONE guibg=NONE guisp=#9CCFD8 gui=undercurl cterm=undercurl";
    x += "hi! link CocInfoHighlight DiagnosticUnderlineInfo";
    x += "hi DiagnosticUnderlineOk guifg=NONE guibg=NONE guisp=#317490 gui=undercurl cterm=undercurl";
    x += "hi DiagnosticUnderlineWarn guifg=NONE guibg=NONE guisp=#F6C074 gui=undercurl cterm=undercurl";
    x += "hi! link CocWarningHighlight DiagnosticUnderlineWarn";
    x += "hi DiagnosticVirtualTextError guifg=#EB7193 guibg=#262021 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorVirtualText DiagnosticVirtualTextError";
    x += "hi DiagnosticVirtualTextHint guifg=#C4A7E7 guibg=#232126 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextInfo guifg=#9CCFD8 guibg=#202222 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextOk guifg=#317490 guibg=#202223 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextWarn guifg=#F6C074 guibg=#232120 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningVirtualText DiagnosticVirtualTextWarn";
    x += "hi! link DiagnosticDeprecated DiagnosticWarn";
    x += "hi! link DiagnosticUnnecessary DiagnosticWarn";
    x += "hi! link NotifyWARNIcon DiagnosticWarn";
    x += "hi! link NotifyWARNTitle DiagnosticWarn";
    x += "hi DiffAdd guifg=NONE guibg=#1D2C34 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffChange guifg=NONE guibg=#1C2D2F guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffDelete guifg=NONE guibg=#3D2229 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffText guifg=#E1D4D4 guibg=#30484C guisp=NONE gui=NONE cterm=NONE";
    x += "hi Directory guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Error guifg=#EB7193 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticError Error";
    x += "hi! link ErrorMsg Error";
    x += "hi FlashBackdrop guifg=#69657E guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FlashLabel guifg=#E1D4D4 guibg=#3B5155 guisp=NONE gui=NONE cterm=NONE";
    x += "hi FloatBorder guifg=#7A7695 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FoldColumn guifg=#625D7F guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Folded guifg=#A4A1B7 guibg=#353248 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Function guifg=#E1D4D4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TroubleNormal Function";
    x += "hi! link TroubleText Function";
    x += "hi FzfLuaBufFlagAlt guifg=#9CCFD8 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufFlagCur guifg=#F6C074 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufNr guifg=#317490 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderBind guifg=#317490 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderText guifg=#F6C074 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaLiveSym guifg=#F6C074 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaPathColNr guifg=#9693AC guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaPathLineNr FzfLuaPathColNr";
    x += "hi FzfLuaTabMarker guifg=#317490 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaTabTitle guifg=#9CCFD8 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi GitSignsAdd guifg=#317490 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterAdd GitSignsAdd";
    x += "hi GitSignsChange guifg=#9CCFD8 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterChange GitSignsChange";
    x += "hi GitSignsDelete guifg=#EB7193 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterDelete GitSignsDelete";
    x += "hi IblIndent guifg=#282635 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IblScope guifg=#454258 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Identifier guifg=#CAB0AF guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IncSearch guifg=#1A1825 guibg=#B48DE0 guisp=NONE gui=bold cterm=bold";
    x += "hi! link CurSearch IncSearch";
    x += "hi Italic guifg=NONE guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi LineNr guifg=#625D7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocCodeLens LineNr";
    x += "hi! link LspCodeLens LineNr";
    x += "hi! link SignColumn LineNr";
    x += "hi LspInlayHint guifg=#6C6593 guibg=#222030 guisp=NONE gui=NONE cterm=NONE";
    x += "hi MoreMsg guifg=#317490 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Question MoreMsg";
    x += "hi! link NnnNormalNC NnnNormal";
    x += "hi! link NnnVertSplit NnnWinSeparator";
    x += "hi NonText guifg=#565172 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link EndOfBuffer NonText";
    x += "hi! link Whitespace NonText";
    x += "hi NormalFloat guifg=NONE guibg=#2D2A3D guisp=NONE gui=NONE cterm=NONE";
    x += "hi Number guifg=#E1D4D4 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Boolean Number";
    x += "hi Pmenu guifg=NONE guibg=#2D2A3D guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSbar guifg=NONE guibg=#5A5578 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSel guifg=NONE guibg=#45415D guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuThumb guifg=NONE guibg=#84809D guisp=NONE gui=NONE cterm=NONE";
    x += "hi Search guifg=#E1D4D4 guibg=#673592 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocSearch Search";
    x += "hi! link MatchParen Search";
    x += "hi! link QuickFixLine Search";
    x += "hi! link Sneak Search";
    x += "hi SneakLabelMask guifg=#C4A7E7 guibg=#C4A7E7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Special guifg=#9CCFD8 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link WhichKeyGroup Special";
    x += "hi SpecialComment guifg=#69657E guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownUrl SpecialComment";
    x += "hi SpecialKey guifg=#565172 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi SpellBad guifg=#D67E95 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi! link CocSelectedText SpellBad";
    x += "hi SpellCap guifg=#D67E95 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi! link SpellLocal SpellCap";
    x += "hi SpellRare guifg=#D67E95 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi Statement guifg=#317490 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link FzfLuaBufName Statement";
    x += "hi! link PreProc Statement";
    x += "hi! link WhichKey Statement";
    x += "hi StatusLine guifg=#E1D4D4 guibg=#312E43 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLine StatusLine";
    x += "hi! link WinBar StatusLine";
    x += "hi StatusLineNC guifg=#E9E0E0 guibg=#242232 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLineFill StatusLineNC";
    x += "hi! link WinBarNC StatusLineNC";
    x += "hi TabLineSel guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link BufferCurrent TabLineSel";
    x += "hi Title guifg=#E1D4D4 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Todo guifg=NONE guibg=NONE guisp=NONE gui=bold,underline cterm=bold,underline";
    x += "hi Type guifg=#DFDEF1 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link helpSpecial Type";
    x += "hi! link markdownCode Type";
    x += "hi Underlined guifg=NONE guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi Visual guifg=NONE guibg=#523A39 guisp=NONE gui=NONE cterm=NONE";
    x += "hi WarningMsg guifg=#F6C074 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticWarn WarningMsg";
    x += "hi! link gitcommitOverflow WarningMsg";
    x += "hi WhichKeySeparator guifg=#625D7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi WildMenu guifg=#1A1825 guibg=#C4A7E7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link SneakLabel WildMenu";
    x += "hi WinSeparator guifg=#625D7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link VertSplit WinSeparator";
    x += "hi diffAdded guifg=#317490 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffChanged guifg=#9CCFD8 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffFile guifg=#F6C074 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffIndexLine guifg=#F6C074 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffLine guifg=#C4A7E7 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffNewFile guifg=#317490 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffOldFile guifg=#EB7193 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffRemoved guifg=#EB7193 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi helpHyperTextEntry guifg=#9693AC guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi helpHyperTextJump guifg=#CAB0AF guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi lCursor guifg=#1A1825 guibg=#B27F7C guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursorNC lCursor";
    x += "hi markdownLinkText guifg=#CAB0AF guibg=NONE guisp=NONE gui=underline cterm=underline";

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
rosebones_light(void)
{
//  let g:terminal_color_0 = '#FBF6F0'
//  let g:terminal_color_1 = '#B5637A'
//  let g:terminal_color_2 = '#286A84'
//  let g:terminal_color_3 = '#EC9D33'
//  let g:terminal_color_4 = '#5795A0'
//  let g:terminal_color_5 = '#917BA9'
//  let g:terminal_color_6 = '#5795A0'
//  let g:terminal_color_7 = '#724341'
//  let g:terminal_color_8 = '#E8C48B'
//  let g:terminal_color_9 = '#A54A66'
//  let g:terminal_color_10 = '#1C5970'
//  let g:terminal_color_11 = '#C68223'
//  let g:terminal_color_12 = '#407D88'
//  let g:terminal_color_13 = '#855AAC'
//  let g:terminal_color_14 = '#407D88'
//  let g:terminal_color_15 = '#A4635F'

    list x;

    x += "set background=light";
    x += "hi clear";
    x += "hi Normal guifg=#724341 guibg=#FBF6F0 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link ModeMsg Normal";
    x += "hi Bold guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi BufferVisible guifg=#AE6966 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleIndex guifg=#AE6966 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleSign guifg=#AE6966 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi CocMarkdownLink guifg=#5795A0 guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi ColorColumn guifg=NONE guibg=#EACFBD guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link LspReferenceRead ColorColumn";
    x += "hi! link LspReferenceText ColorColumn";
    x += "hi! link LspReferenceWrite ColorColumn";
    x += "hi Comment guifg=#A18E72 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi Conceal guifg=#A4635F guibg=NONE guisp=NONE gui=bold,italic cterm=bold,italic";
    x += "hi Constant guifg=#AB6763 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Character Constant";
    x += "hi! link Float Constant";
    x += "hi! link String Constant";
    x += "hi! link TroubleSource Constant";
    x += "hi! link WhichKeyValue Constant";
    x += "hi! link helpOption Constant";
    x += "hi Cursor guifg=#FBF6F0 guibg=#724341 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursor Cursor";
    x += "hi CursorLine guifg=NONE guibg=#F7ECE0 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocMenuSel CursorLine";
    x += "hi! link CursorColumn CursorLine";
    x += "hi CursorLineNr guifg=#724341 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Delimiter guifg=#9B835D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownLinkTextDelimiter Delimiter";
    x += "hi! link NotifyERRORIcon DiagnosticError";
    x += "hi! link NotifyERRORTitle DiagnosticError";
    x += "hi DiagnosticHint guifg=#917BA9 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyDEBUGIcon DiagnosticHint";
    x += "hi! link NotifyDEBUGTitle DiagnosticHint";
    x += "hi! link NotifyTRACEIcon DiagnosticHint";
    x += "hi! link NotifyTRACETitle DiagnosticHint";
    x += "hi DiagnosticInfo guifg=#5795A0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyINFOIcon DiagnosticInfo";
    x += "hi! link NotifyINFOTitle DiagnosticInfo";
    x += "hi DiagnosticOk guifg=#286A84 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignError guifg=#B5637A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorSign DiagnosticSignError";
    x += "hi DiagnosticSignHint guifg=#917BA9 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocHintSign DiagnosticSignHint";
    x += "hi DiagnosticSignInfo guifg=#5795A0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocInfoSign DiagnosticSignInfo";
    x += "hi DiagnosticSignOk guifg=#286A84 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignWarn guifg=#EC9D33 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningSign DiagnosticSignWarn";
    x += "hi DiagnosticUnderlineError guifg=NONE guibg=NONE guisp=#B5637A gui=undercurl cterm=undercurl";
    x += "hi! link CocErrorHighlight DiagnosticUnderlineError";
    x += "hi DiagnosticUnderlineHint guifg=NONE guibg=NONE guisp=#917BA9 gui=undercurl cterm=undercurl";
    x += "hi! link CocHintHighlight DiagnosticUnderlineHint";
    x += "hi DiagnosticUnderlineInfo guifg=NONE guibg=NONE guisp=#5795A0 gui=undercurl cterm=undercurl";
    x += "hi! link CocInfoHighlight DiagnosticUnderlineInfo";
    x += "hi DiagnosticUnderlineOk guifg=NONE guibg=NONE guisp=#286A84 gui=undercurl cterm=undercurl";
    x += "hi DiagnosticUnderlineWarn guifg=NONE guibg=NONE guisp=#EC9D33 gui=undercurl cterm=undercurl";
    x += "hi! link CocWarningHighlight DiagnosticUnderlineWarn";
    x += "hi DiagnosticVirtualTextError guifg=#B5637A guibg=#F4E8EB guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorVirtualText DiagnosticVirtualTextError";
    x += "hi DiagnosticVirtualTextHint guifg=#917BA9 guibg=#EEE9F3 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextInfo guifg=#5795A0 guibg=#DAEFF3 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextOk guifg=#286A84 guibg=#E2ECF3 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextWarn guifg=#EC9D33 guibg=#F4E9E2 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningVirtualText DiagnosticVirtualTextWarn";
    x += "hi! link DiagnosticDeprecated DiagnosticWarn";
    x += "hi! link DiagnosticUnnecessary DiagnosticWarn";
    x += "hi! link NotifyWARNIcon DiagnosticWarn";
    x += "hi! link NotifyWARNTitle DiagnosticWarn";
    x += "hi DiffAdd guifg=NONE guibg=#DDE7ED guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffChange guifg=NONE guibg=#D6E9ED guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffDelete guifg=NONE guibg=#F0E2E5 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffText guifg=#724341 guibg=#A8C9D1 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Directory guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Error guifg=#B5637A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticError Error";
    x += "hi! link ErrorMsg Error";
    x += "hi FlashBackdrop guifg=#A18E72 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FlashLabel guifg=#724341 guibg=#81D9E9 guisp=NONE gui=NONE cterm=NONE";
    x += "hi FloatBorder guifg=#877150 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FoldColumn guifg=#B69A6E guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Folded guifg=#605038 guibg=#E7C48E guisp=NONE gui=NONE cterm=NONE";
    x += "hi Function guifg=#724341 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TroubleNormal Function";
    x += "hi! link TroubleText Function";
    x += "hi FzfLuaBufFlagAlt guifg=#5795A0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufFlagCur guifg=#EC9D33 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufNr guifg=#286A84 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderBind guifg=#286A84 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderText guifg=#EC9D33 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaLiveSym guifg=#EC9D33 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaPathColNr guifg=#6D5937 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaPathLineNr FzfLuaPathColNr";
    x += "hi FzfLuaTabMarker guifg=#286A84 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaTabTitle guifg=#5795A0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi GitSignsAdd guifg=#286A84 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterAdd GitSignsAdd";
    x += "hi GitSignsChange guifg=#5795A0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterChange GitSignsChange";
    x += "hi GitSignsDelete guifg=#B5637A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterDelete GitSignsDelete";
    x += "hi IblIndent guifg=#F0E3D4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IblScope guifg=#D3B790 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Identifier guifg=#935855 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IncSearch guifg=#FBF6F0 guibg=#A18EB6 guisp=NONE gui=bold cterm=bold";
    x += "hi! link CurSearch IncSearch";
    x += "hi Italic guifg=NONE guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi LineNr guifg=#B69A6E guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocCodeLens LineNr";
    x += "hi! link LspCodeLens LineNr";
    x += "hi! link SignColumn LineNr";
    x += "hi LspInlayHint guifg=#AF9263 guibg=#F8EFE5 guisp=NONE gui=NONE cterm=NONE";
    x += "hi MoreMsg guifg=#286A84 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Question MoreMsg";
    x += "hi! link NnnNormalNC NnnNormal";
    x += "hi! link NnnVertSplit NnnWinSeparator";
    x += "hi NonText guifg=#CEAF7E guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link EndOfBuffer NonText";
    x += "hi! link Whitespace NonText";
    x += "hi NormalFloat guifg=NONE guibg=#F1DDC3 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Number guifg=#724341 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Boolean Number";
    x += "hi Pmenu guifg=NONE guibg=#EED7B7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSbar guifg=NONE guibg=#C5A778 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSel guifg=NONE guibg=#DEBC88 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuThumb guifg=NONE guibg=#FEFCFA guisp=NONE gui=NONE cterm=NONE";
    x += "hi Search guifg=#724341 guibg=#D1C9DC guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocSearch Search";
    x += "hi! link MatchParen Search";
    x += "hi! link QuickFixLine Search";
    x += "hi! link Sneak Search";
    x += "hi SneakLabelMask guifg=#917BA9 guibg=#917BA9 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Special guifg=#5795A0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link WhichKeyGroup Special";
    x += "hi SpecialComment guifg=#A18E72 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownUrl SpecialComment";
    x += "hi SpecialKey guifg=#CEAF7E guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi SpellBad guifg=#A66B7B guibg=NONE guisp=#B5637A gui=undercurl cterm=undercurl";
    x += "hi! link CocSelectedText SpellBad";
    x += "hi SpellCap guifg=#A66B7B guibg=NONE guisp=#C27187 gui=undercurl cterm=undercurl";
    x += "hi! link SpellLocal SpellCap";
    x += "hi SpellRare guifg=#A66B7B guibg=NONE guisp=#EC9D33 gui=undercurl cterm=undercurl";
    x += "hi Statement guifg=#286A84 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link FzfLuaBufName Statement";
    x += "hi! link PreProc Statement";
    x += "hi! link WhichKey Statement";
    x += "hi StatusLine guifg=#724341 guibg=#ECD0A9 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLine StatusLine";
    x += "hi! link WinBar StatusLine";
    x += "hi StatusLineNC guifg=#AE6966 guibg=#F3E3CF guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLineFill StatusLineNC";
    x += "hi! link WinBarNC StatusLineNC";
    x += "hi TabLineSel guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link BufferCurrent TabLineSel";
    x += "hi Title guifg=#724341 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Todo guifg=NONE guibg=NONE guisp=NONE gui=bold,underline cterm=bold,underline";
    x += "hi Type guifg=#57527A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link helpSpecial Type";
    x += "hi! link markdownCode Type";
    x += "hi Underlined guifg=NONE guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi Visual guifg=NONE guibg=#EADDDC guisp=NONE gui=NONE cterm=NONE";
    x += "hi WarningMsg guifg=#EC9D33 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticWarn WarningMsg";
    x += "hi! link gitcommitOverflow WarningMsg";
    x += "hi WhichKeySeparator guifg=#B69A6E guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi WildMenu guifg=#FBF6F0 guibg=#917BA9 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link SneakLabel WildMenu";
    x += "hi WinSeparator guifg=#B69A6E guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link VertSplit WinSeparator";
    x += "hi diffAdded guifg=#286A84 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffChanged guifg=#5795A0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffFile guifg=#EC9D33 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffIndexLine guifg=#EC9D33 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffLine guifg=#917BA9 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffNewFile guifg=#286A84 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffOldFile guifg=#B5637A guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffRemoved guifg=#B5637A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi helpHyperTextEntry guifg=#6D5937 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi helpHyperTextJump guifg=#935855 guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi lCursor guifg=#FBF6F0 guibg=#9D5F5B guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursorNC lCursor";
    x += "hi markdownLinkText guifg=#935855 guibg=NONE guisp=NONE gui=underline cterm=underline";

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
