/* -*- mode: cr; indent-width: 4; -*- */
// $Id: forestbones.cr,v 1.4 2024/10/27 06:09:51 cvsuser Exp $
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

static void forestbones_dark(void);
static void forestbones_light(void);


void
main(void)
{
    module("csbones");
}


static int
forestbones(int dark)
{
    if (dark) {
        forestbones_dark();
    } else {
        forestbones_light();
    }
    return 0;
}


static void
forestbones_dark(void)
{
//  let s:italics = (&t_ZH != '' && &t_ZH != '[7m') || has('gui_running') || has('nvim')
//  let g:terminal_color_0  = '#2C343A'
//  let g:terminal_color_1  = '#E67C7F'
//  let g:terminal_color_2  = '#A9C181'
//  let g:terminal_color_3  = '#DDBD7F'
//  let g:terminal_color_4  = '#7FBCB4'
//  let g:terminal_color_5  = '#D69AB7'
//  let g:terminal_color_6  = '#83C193'
//  let g:terminal_color_7  = '#E7DCC4'
//  let g:terminal_color_8  = '#45525C'
//  let g:terminal_color_9  = '#ED9294'
//  let g:terminal_color_10 = '#B0CE7B'
//  let g:terminal_color_11 = '#EDC77A'
//  let g:terminal_color_12 = '#7AC9C0'
//  let g:terminal_color_13 = '#E5A7C4'
//  let g:terminal_color_14 = '#7DD093'
//  let g:terminal_color_15 = '#B2A790'
    list x;

    x += "set background=dark";
    x += "hi clear";
    x += "hi Normal guifg=#E7DCC4 guibg=#2C343A guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link ModeMsg Normal";
    x += "hi Bold guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi BufferVisible guifg=#EDE5D4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleIndex guifg=#EDE5D4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleSign guifg=#EDE5D4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi CocMarkdownLink guifg=#83C193 guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi ColorColumn guifg=NONE guibg=#685A41 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link LspReferenceRead ColorColumn";
    x += "hi! link LspReferenceText ColorColumn";
    x += "hi! link LspReferenceWrite ColorColumn";
    x += "hi Comment guifg=#6E7B85 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi Conceal guifg=#9F9580 guibg=NONE guisp=NONE gui=bold,italic cterm=bold,italic";
    x += "hi Constant guifg=#ADA28B guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Character Constant";
    x += "hi! link Float Constant";
    x += "hi! link String Constant";
    x += "hi! link TroubleSource Constant";
    x += "hi! link WhichKeyValue Constant";
    x += "hi! link helpOption Constant";
    x += "hi Cursor guifg=#2C343A guibg=#EBE2CF guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursor Cursor";
    x += "hi CursorLine guifg=NONE guibg=#313A41 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocMenuSel CursorLine";
    x += "hi! link CursorColumn CursorLine";
    x += "hi CursorLineNr guifg=#E7DCC4 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Delimiter guifg=#7B8E9D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownLinkTextDelimiter Delimiter";
    x += "hi! link NotifyERRORIcon DiagnosticError";
    x += "hi! link NotifyERRORTitle DiagnosticError";
    x += "hi DiagnosticHint guifg=#D69AB7 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyDEBUGIcon DiagnosticHint";
    x += "hi! link NotifyDEBUGTitle DiagnosticHint";
    x += "hi! link NotifyTRACEIcon DiagnosticHint";
    x += "hi! link NotifyTRACETitle DiagnosticHint";
    x += "hi DiagnosticInfo guifg=#7FBCB4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyINFOIcon DiagnosticInfo";
    x += "hi! link NotifyINFOTitle DiagnosticInfo";
    x += "hi DiagnosticOk guifg=#A9C181 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignError guifg=#E67C7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorSign DiagnosticSignError";
    x += "hi DiagnosticSignHint guifg=#D69AB7 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocHintSign DiagnosticSignHint";
    x += "hi DiagnosticSignInfo guifg=#7FBCB4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocInfoSign DiagnosticSignInfo";
    x += "hi DiagnosticSignOk guifg=#A9C181 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignWarn guifg=#DDBD7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningSign DiagnosticSignWarn";
    x += "hi DiagnosticUnderlineError guifg=NONE guibg=NONE guisp=#E67C7F gui=undercurl cterm=undercurl";
    x += "hi! link CocErrorHighlight DiagnosticUnderlineError";
    x += "hi DiagnosticUnderlineHint guifg=NONE guibg=NONE guisp=#D69AB7 gui=undercurl cterm=undercurl";
    x += "hi! link CocHintHighlight DiagnosticUnderlineHint";
    x += "hi DiagnosticUnderlineInfo guifg=NONE guibg=NONE guisp=#7FBCB4 gui=undercurl cterm=undercurl";
    x += "hi! link CocInfoHighlight DiagnosticUnderlineInfo";
    x += "hi DiagnosticUnderlineOk guifg=NONE guibg=NONE guisp=#A9C181 gui=undercurl cterm=undercurl";
    x += "hi DiagnosticUnderlineWarn guifg=NONE guibg=NONE guisp=#DDBD7F gui=undercurl cterm=undercurl";
    x += "hi! link CocWarningHighlight DiagnosticUnderlineWarn";
    x += "hi DiagnosticVirtualTextError guifg=#E67C7F guibg=#443939 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorVirtualText DiagnosticVirtualTextError";
    x += "hi DiagnosticVirtualTextHint guifg=#D69AB7 guibg=#42393D guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextInfo guifg=#7FBCB4 guibg=#393C3C guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextOk guifg=#A9C181 guibg=#3A3C38 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextWarn guifg=#DDBD7F guibg=#3D3B38 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningVirtualText DiagnosticVirtualTextWarn";
    x += "hi! link DiagnosticDeprecated DiagnosticWarn";
    x += "hi! link DiagnosticUnnecessary DiagnosticWarn";
    x += "hi! link NotifyWARNIcon DiagnosticWarn";
    x += "hi! link NotifyWARNTitle DiagnosticWarn";
    x += "hi DiffAdd guifg=NONE guibg=#3E482D guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffChange guifg=NONE guibg=#304946 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffDelete guifg=NONE guibg=#643839 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffText guifg=#E7DCC4 guibg=#456763 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Directory guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Error guifg=#E67C7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticError Error";
    x += "hi! link ErrorMsg Error";
    x += "hi FlashBackdrop guifg=#6E7B85 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FlashLabel guifg=#E7DCC4 guibg=#4B726D guisp=NONE gui=NONE cterm=NONE";
    x += "hi FloatBorder guifg=#798B9A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FoldColumn guifg=#667783 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Folded guifg=#9CB4C6 guibg=#424D55 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Function guifg=#E7DCC4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TroubleNormal Function";
    x += "hi! link TroubleText Function";
    x += "hi FzfLuaBufFlagAlt guifg=#7FBCB4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufFlagCur guifg=#DDBD7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufNr guifg=#A9C181 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderBind guifg=#A9C181 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderText guifg=#DDBD7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaLiveSym guifg=#DDBD7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaPathColNr guifg=#90A6B7 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaPathLineNr FzfLuaPathColNr";
    x += "hi FzfLuaTabMarker guifg=#A9C181 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaTabTitle guifg=#83C193 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi GitSignsAdd guifg=#A9C181 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterAdd GitSignsAdd";
    x += "hi GitSignsChange guifg=#7FBCB4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterChange GitSignsChange";
    x += "hi GitSignsDelete guifg=#E67C7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterDelete GitSignsDelete";
    x += "hi IblIndent guifg=#394147 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IblScope guifg=#515B63 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Identifier guifg=#C6BAA0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IncSearch guifg=#2C343A guibg=#DFB2C7 guisp=NONE gui=bold cterm=bold";
    x += "hi! link CurSearch IncSearch";
    x += "hi Italic guifg=NONE guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi LineNr guifg=#667783 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocCodeLens LineNr";
    x += "hi! link LspCodeLens LineNr";
    x += "hi! link SignColumn LineNr";
    x += "hi LspInlayHint guifg=#6B8292 guibg=#313A41 guisp=NONE gui=NONE cterm=NONE";
    x += "hi MoreMsg guifg=#A9C181 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Question MoreMsg";
    x += "hi! link NnnNormalNC NnnNormal";
    x += "hi! link NnnVertSplit NnnWinSeparator";
    x += "hi NonText guifg=#5D6D78 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link EndOfBuffer NonText";
    x += "hi! link Whitespace NonText";
    x += "hi NormalFloat guifg=NONE guibg=#3B464E guisp=NONE gui=NONE cterm=NONE";
    x += "hi Number guifg=#E7DCC4 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Boolean Number";
    x += "hi Pmenu guifg=NONE guibg=#3B464E guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSbar guifg=NONE guibg=#606F7B guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSel guifg=NONE guibg=#4E5B65 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuThumb guifg=NONE guibg=#8296A5 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PreProc guifg=#83C193 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Search guifg=#E7DCC4 guibg=#9E5179 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocSearch Search";
    x += "hi! link MatchParen Search";
    x += "hi! link QuickFixLine Search";
    x += "hi! link Sneak Search";
    x += "hi SneakLabelMask guifg=#D69AB7 guibg=#D69AB7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Special guifg=#B5AA92 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link WhichKeyGroup Special";
    x += "hi SpecialComment guifg=#6E7B85 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownUrl SpecialComment";
    x += "hi SpecialKey guifg=#5D6D78 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi SpellBad guifg=#D48688 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi! link CocSelectedText SpellBad";
    x += "hi SpellCap guifg=#D48688 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi! link SpellLocal SpellCap";
    x += "hi SpellRare guifg=#D48688 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi Statement guifg=#A9C181 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaBufName Statement";
    x += "hi! link WhichKey Statement";
    x += "hi StatusLine guifg=#E7DCC4 guibg=#3E4850 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLine StatusLine";
    x += "hi! link WinBar StatusLine";
    x += "hi StatusLineNC guifg=#EDE5D4 guibg=#353F46 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLineFill StatusLineNC";
    x += "hi! link WinBarNC StatusLineNC";
    x += "hi TabLineSel guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link BufferCurrent TabLineSel";
    x += "hi Title guifg=#E7DCC4 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Todo guifg=NONE guibg=NONE guisp=NONE gui=bold,underline cterm=bold,underline";
    x += "hi Type guifg=#7FBCB4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link helpSpecial Type";
    x += "hi! link markdownCode Type";
    x += "hi Underlined guifg=NONE guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi Visual guifg=NONE guibg=#615B51 guisp=NONE gui=NONE cterm=NONE";
    x += "hi WarningMsg guifg=#DDBD7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticWarn WarningMsg";
    x += "hi! link gitcommitOverflow WarningMsg";
    x += "hi WhichKeySeparator guifg=#667783 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi WildMenu guifg=#2C343A guibg=#D69AB7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link SneakLabel WildMenu";
    x += "hi WinSeparator guifg=#667783 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link VertSplit WinSeparator";
    x += "hi diffAdded guifg=#A9C181 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffChanged guifg=#7FBCB4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffFile guifg=#DDBD7F guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffIndexLine guifg=#DDBD7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffLine guifg=#D69AB7 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffNewFile guifg=#A9C181 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffOldFile guifg=#E67C7F guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffRemoved guifg=#E67C7F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi helpHyperTextEntry guifg=#90A6B7 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi helpHyperTextJump guifg=#C6BAA0 guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi lCursor guifg=#2C343A guibg=#978D79 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursorNC lCursor";
    x += "hi markdownLinkText guifg=#C6BAA0 guibg=NONE guisp=NONE gui=underline cterm=underline";

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
forestbones_light(void)
{
//  let g:terminal_color_0 = '#FAF3E1'
//  let g:terminal_color_1 = '#F85550'
//  let g:terminal_color_2 = '#8DA200'
//  let g:terminal_color_3 = '#DEA000'
//  let g:terminal_color_4 = '#3A94C4'
//  let g:terminal_color_5 = '#DF69BA'
//  let g:terminal_color_6 = '#36A87E'
//  let g:terminal_color_7 = '#4F5B62'
//  let g:terminal_color_8 = '#DBC988'
//  let g:terminal_color_9 = '#E6271C'
//  let g:terminal_color_10 = '#758700'
//  let g:terminal_color_11 = '#B98500'
//  let g:terminal_color_12 = '#297CA6'
//  let g:terminal_color_13 = '#CA43A3'
//  let g:terminal_color_14 = '#258C67'
//  let g:terminal_color_15 = '#6E7F88'

    list x;

    x += "set background=light";
    x += "hi clear";
    x += "hi Normal guifg=#4F5B62 guibg=#FAF3E1 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link ModeMsg Normal";
    x += "hi Bold guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi BufferVisible guifg=#758690 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleIndex guifg=#758690 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleSign guifg=#758690 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi CocMarkdownLink guifg=#36A87E guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi ColorColumn guifg=NONE guibg=#E9CDAD guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link LspReferenceRead ColorColumn";
    x += "hi! link LspReferenceText ColorColumn";
    x += "hi! link LspReferenceWrite ColorColumn";
    x += "hi Comment guifg=#9A9071 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi Conceal guifg=#6E7F88 guibg=NONE guisp=NONE gui=bold,italic cterm=bold,italic";
    x += "hi Constant guifg=#73848D guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Character Constant";
    x += "hi! link Float Constant";
    x += "hi! link String Constant";
    x += "hi! link TroubleSource Constant";
    x += "hi! link WhichKeyValue Constant";
    x += "hi! link helpOption Constant";
    x += "hi Cursor guifg=#FAF3E1 guibg=#4F5B62 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursor Cursor";
    x += "hi CursorLine guifg=NONE guibg=#F6EBC8 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocMenuSel CursorLine";
    x += "hi! link CursorColumn CursorLine";
    x += "hi CursorLineNr guifg=#4F5B62 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Delimiter guifg=#92865B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownLinkTextDelimiter Delimiter";
    x += "hi! link NotifyERRORIcon DiagnosticError";
    x += "hi! link NotifyERRORTitle DiagnosticError";
    x += "hi DiagnosticHint guifg=#DF69BA guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyDEBUGIcon DiagnosticHint";
    x += "hi! link NotifyDEBUGTitle DiagnosticHint";
    x += "hi! link NotifyTRACEIcon DiagnosticHint";
    x += "hi! link NotifyTRACETitle DiagnosticHint";
    x += "hi DiagnosticInfo guifg=#3A94C4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyINFOIcon DiagnosticInfo";
    x += "hi! link NotifyINFOTitle DiagnosticInfo";
    x += "hi DiagnosticOk guifg=#8DA200 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignError guifg=#F85550 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorSign DiagnosticSignError";
    x += "hi DiagnosticSignHint guifg=#DF69BA guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocHintSign DiagnosticSignHint";
    x += "hi DiagnosticSignInfo guifg=#3A94C4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocInfoSign DiagnosticSignInfo";
    x += "hi DiagnosticSignOk guifg=#8DA200 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignWarn guifg=#DEA000 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningSign DiagnosticSignWarn";
    x += "hi DiagnosticUnderlineError guifg=NONE guibg=NONE guisp=#F85550 gui=undercurl cterm=undercurl";
    x += "hi! link CocErrorHighlight DiagnosticUnderlineError";
    x += "hi DiagnosticUnderlineHint guifg=NONE guibg=NONE guisp=#DF69BA gui=undercurl cterm=undercurl";
    x += "hi! link CocHintHighlight DiagnosticUnderlineHint";
    x += "hi DiagnosticUnderlineInfo guifg=NONE guibg=NONE guisp=#3A94C4 gui=undercurl cterm=undercurl";
    x += "hi! link CocInfoHighlight DiagnosticUnderlineInfo";
    x += "hi DiagnosticUnderlineOk guifg=NONE guibg=NONE guisp=#8DA200 gui=undercurl cterm=undercurl";
    x += "hi DiagnosticUnderlineWarn guifg=NONE guibg=NONE guisp=#DEA000 gui=undercurl cterm=undercurl";
    x += "hi! link CocWarningHighlight DiagnosticUnderlineWarn";
    x += "hi DiagnosticVirtualTextError guifg=#F85550 guibg=#F2E5E5 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorVirtualText DiagnosticVirtualTextError";
    x += "hi DiagnosticVirtualTextHint guifg=#DF69BA guibg=#F2E4EC guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextInfo guifg=#3A94C4 guibg=#E0E9F2 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextOk guifg=#8DA200 guibg=#E1EFB0 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextWarn guifg=#DEA000 guibg=#F2E6DA guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningVirtualText DiagnosticVirtualTextWarn";
    x += "hi! link DiagnosticDeprecated DiagnosticWarn";
    x += "hi! link DiagnosticUnnecessary DiagnosticWarn";
    x += "hi! link NotifyWARNIcon DiagnosticWarn";
    x += "hi! link NotifyWARNTitle DiagnosticWarn";
    x += "hi DiffAdd guifg=NONE guibg=#DDE7BD guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffChange guifg=NONE guibg=#DCE3EB guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffDelete guifg=NONE guibg=#EEDFDF guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffText guifg=#4F5B62 guibg=#B0C3D4 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Directory guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Error guifg=#F85550 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticError Error";
    x += "hi! link ErrorMsg Error";
    x += "hi FlashBackdrop guifg=#9A9071 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FlashLabel guifg=#4F5B62 guibg=#9ACFF8 guisp=NONE gui=NONE cterm=NONE";
    x += "hi FloatBorder guifg=#7C724D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FoldColumn guifg=#A99B6A guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Folded guifg=#5A5236 guibg=#DAC98B guisp=NONE gui=NONE cterm=NONE";
    x += "hi Function guifg=#4F5B62 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TroubleNormal Function";
    x += "hi! link TroubleText Function";
    x += "hi FzfLuaBufFlagAlt guifg=#3A94C4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufFlagCur guifg=#DEA000 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufNr guifg=#8DA200 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderBind guifg=#8DA200 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderText guifg=#DEA000 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaLiveSym guifg=#DEA000 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaPathColNr guifg=#635934 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaPathLineNr FzfLuaPathColNr";
    x += "hi FzfLuaTabMarker guifg=#8DA200 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaTabTitle guifg=#36A87E guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi GitSignsAdd guifg=#8DA200 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterAdd GitSignsAdd";
    x += "hi GitSignsChange guifg=#3A94C4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterChange GitSignsChange";
    x += "hi GitSignsDelete guifg=#F85550 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterDelete GitSignsDelete";
    x += "hi IblIndent guifg=#F0E2B6 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IblScope guifg=#C6B88C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Identifier guifg=#63727A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IncSearch guifg=#FAF3E1 guibg=#DF69BA guisp=NONE gui=bold cterm=bold";
    x += "hi! link CurSearch IncSearch";
    x += "hi Italic guifg=NONE guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi LineNr guifg=#A99B6A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocCodeLens LineNr";
    x += "hi! link LspCodeLens LineNr";
    x += "hi! link SignColumn LineNr";
    x += "hi LspInlayHint guifg=#A1935F guibg=#F7EED1 guisp=NONE gui=NONE cterm=NONE";
    x += "hi MoreMsg guifg=#8DA200 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Question MoreMsg";
    x += "hi! link NnnNormalNC NnnNormal";
    x += "hi! link NnnVertSplit NnnWinSeparator";
    x += "hi NonText guifg=#C0B079 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link EndOfBuffer NonText";
    x += "hi! link Whitespace NonText";
    x += "hi NormalFloat guifg=NONE guibg=#F0DC99 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Number guifg=#4F5B62 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Boolean Number";
    x += "hi Pmenu guifg=NONE guibg=#E9D795 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSbar guifg=NONE guibg=#B7A874 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSel guifg=NONE guibg=#CFBE83 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuThumb guifg=NONE guibg=#FCF9F1 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PreProc guifg=#36A87E guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Search guifg=#4F5B62 guibg=#EEBADB guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocSearch Search";
    x += "hi! link MatchParen Search";
    x += "hi! link QuickFixLine Search";
    x += "hi! link Sneak Search";
    x += "hi SneakLabelMask guifg=#DF69BA guibg=#DF69BA guisp=NONE gui=NONE cterm=NONE";
    x += "hi Special guifg=#6E7F88 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link WhichKeyGroup Special";
    x += "hi SpecialComment guifg=#9A9071 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownUrl SpecialComment";
    x += "hi SpecialKey guifg=#C0B079 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi SpellBad guifg=#E16966 guibg=NONE guisp=#F85550 gui=undercurl cterm=undercurl";
    x += "hi! link CocSelectedText SpellBad";
    x += "hi SpellCap guifg=#E16966 guibg=NONE guisp=#F86B68 gui=undercurl cterm=undercurl";
    x += "hi! link SpellLocal SpellCap";
    x += "hi SpellRare guifg=#E16966 guibg=NONE guisp=#DEA000 gui=undercurl cterm=undercurl";
    x += "hi Statement guifg=#8DA200 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaBufName Statement";
    x += "hi! link WhichKey Statement";
    x += "hi StatusLine guifg=#4F5B62 guibg=#E3D191 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLine StatusLine";
    x += "hi! link WinBar StatusLine";
    x += "hi StatusLineNC guifg=#758690 guibg=#F3E2AA guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLineFill StatusLineNC";
    x += "hi! link WinBarNC StatusLineNC";
    x += "hi TabLineSel guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link BufferCurrent TabLineSel";
    x += "hi Title guifg=#4F5B62 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Todo guifg=NONE guibg=NONE guisp=NONE gui=bold,underline cterm=bold,underline";
    x += "hi Type guifg=#3A94C4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link helpSpecial Type";
    x += "hi! link markdownCode Type";
    x += "hi Underlined guifg=NONE guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi Visual guifg=NONE guibg=#D3DFE6 guisp=NONE gui=NONE cterm=NONE";
    x += "hi WarningMsg guifg=#DEA000 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticWarn WarningMsg";
    x += "hi! link gitcommitOverflow WarningMsg";
    x += "hi WhichKeySeparator guifg=#A99B6A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi WildMenu guifg=#FAF3E1 guibg=#DF69BA guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link SneakLabel WildMenu";
    x += "hi WinSeparator guifg=#A99B6A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link VertSplit WinSeparator";
    x += "hi diffAdded guifg=#8DA200 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffChanged guifg=#3A94C4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffFile guifg=#DEA000 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffIndexLine guifg=#DEA000 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffLine guifg=#DF69BA guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffNewFile guifg=#8DA200 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffOldFile guifg=#F85550 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffRemoved guifg=#F85550 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi helpHyperTextEntry guifg=#635934 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi helpHyperTextJump guifg=#63727A guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi lCursor guifg=#FAF3E1 guibg=#697982 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursorNC lCursor";
    x += "hi markdownLinkText guifg=#63727A guibg=NONE guisp=NONE gui=underline cterm=underline";

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
