/* -*- mode: cr; indent-width: 4; -*- */
// $Id: zenbones.cr,v 1.4 2024/10/27 06:09:51 cvsuser Exp $
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
    } else {
        zenbones_light();
    }
    return 0;
}


static void
zenbones_dark(void)
{
//  let g:terminal_color_0  = '#1C1917'
//  let g:terminal_color_1  = '#DE6E7C'
//  let g:terminal_color_2  = '#819B69'
//  let g:terminal_color_3  = '#B77E64'
//  let g:terminal_color_4  = '#6099C0'
//  let g:terminal_color_5  = '#B279A7'
//  let g:terminal_color_6  = '#66A5AD'
//  let g:terminal_color_7  = '#B4BDC3'
//  let g:terminal_color_8  = '#403833'
//  let g:terminal_color_9  = '#E8838F'
//  let g:terminal_color_10 = '#8BAE68'
//  let g:terminal_color_11 = '#D68C67'
//  let g:terminal_color_12 = '#61ABDA'
//  let g:terminal_color_13 = '#CF86C1'
//  let g:terminal_color_14 = '#65B8C1'
//  let g:terminal_color_15 = '#888F94'

    list x;

    x += "set background=dark";
    x += "hi clear";
    x += "hi Normal guifg=#B4BDC3 guibg=#1C1917 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link ModeMsg Normal";
    x += "hi Bold guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi BufferVisible guifg=#CAD0D4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleIndex guifg=#CAD0D4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleSign guifg=#CAD0D4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi CocMarkdownLink guifg=#66A5AD guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi ColorColumn guifg=NONE guibg=#55392C guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link LspReferenceRead ColorColumn";
    x += "hi! link LspReferenceText ColorColumn";
    x += "hi! link LspReferenceWrite ColorColumn";
    x += "hi Comment guifg=#6E6763 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi Conceal guifg=#797F84 guibg=NONE guisp=NONE gui=bold,italic cterm=bold,italic";
    x += "hi Constant guifg=#868C91 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Character Constant";
    x += "hi! link Float Constant";
    x += "hi! link String Constant";
    x += "hi! link TroubleSource Constant";
    x += "hi! link WhichKeyValue Constant";
    x += "hi! link helpOption Constant";
    x += "hi Cursor guifg=#1C1917 guibg=#C4CACF guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursor Cursor";
    x += "hi CursorLine guifg=NONE guibg=#25211F guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocMenuSel CursorLine";
    x += "hi! link CursorColumn CursorLine";
    x += "hi CursorLineNr guifg=#B4BDC3 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Delimiter guifg=#867A74 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownLinkTextDelimiter Delimiter";
    x += "hi! link NotifyERRORIcon DiagnosticError";
    x += "hi! link NotifyERRORTitle DiagnosticError";
    x += "hi DiagnosticHint guifg=#B279A7 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyDEBUGIcon DiagnosticHint";
    x += "hi! link NotifyDEBUGTitle DiagnosticHint";
    x += "hi! link NotifyTRACEIcon DiagnosticHint";
    x += "hi! link NotifyTRACETitle DiagnosticHint";
    x += "hi DiagnosticInfo guifg=#6099C0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyINFOIcon DiagnosticInfo";
    x += "hi! link NotifyINFOTitle DiagnosticInfo";
    x += "hi DiagnosticOk guifg=#819B69 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignError guifg=#DE6E7C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorSign DiagnosticSignError";
    x += "hi DiagnosticSignHint guifg=#B279A7 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocHintSign DiagnosticSignHint";
    x += "hi DiagnosticSignInfo guifg=#6099C0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocInfoSign DiagnosticSignInfo";
    x += "hi DiagnosticSignOk guifg=#819B69 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignWarn guifg=#B77E64 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningSign DiagnosticSignWarn";
    x += "hi DiagnosticUnderlineError guifg=NONE guibg=NONE guisp=#DE6E7C gui=undercurl cterm=undercurl";
    x += "hi! link CocErrorHighlight DiagnosticUnderlineError";
    x += "hi DiagnosticUnderlineHint guifg=NONE guibg=NONE guisp=#B279A7 gui=undercurl cterm=undercurl";
    x += "hi! link CocHintHighlight DiagnosticUnderlineHint";
    x += "hi DiagnosticUnderlineInfo guifg=NONE guibg=NONE guisp=#6099C0 gui=undercurl cterm=undercurl";
    x += "hi! link CocInfoHighlight DiagnosticUnderlineInfo";
    x += "hi DiagnosticUnderlineOk guifg=NONE guibg=NONE guisp=#819B69 gui=undercurl cterm=undercurl";
    x += "hi DiagnosticUnderlineWarn guifg=NONE guibg=NONE guisp=#B77E64 gui=undercurl cterm=undercurl";
    x += "hi! link CocWarningHighlight DiagnosticUnderlineWarn";
    x += "hi DiagnosticVirtualTextError guifg=#DE6E7C guibg=#272020 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorVirtualText DiagnosticVirtualTextError";
    x += "hi DiagnosticVirtualTextHint guifg=#B279A7 guibg=#252024 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextInfo guifg=#6099C0 guibg=#202223 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextOk guifg=#819B69 guibg=#212220 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextWarn guifg=#B77E64 guibg=#242120 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningVirtualText DiagnosticVirtualTextWarn";
    x += "hi! link DiagnosticDeprecated DiagnosticWarn";
    x += "hi! link DiagnosticUnnecessary DiagnosticWarn";
    x += "hi! link NotifyWARNIcon DiagnosticWarn";
    x += "hi! link NotifyWARNTitle DiagnosticWarn";
    x += "hi DiffAdd guifg=NONE guibg=#232D1A guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffChange guifg=NONE guibg=#1D2C36 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffDelete guifg=NONE guibg=#3E2225 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffText guifg=#B4BDC3 guibg=#324757 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Directory guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Error guifg=#DE6E7C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticError Error";
    x += "hi! link ErrorMsg Error";
    x += "hi FlashBackdrop guifg=#6E6763 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FlashLabel guifg=#B4BDC3 guibg=#315167 guisp=NONE gui=NONE cterm=NONE";
    x += "hi FloatBorder guifg=#837771 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FoldColumn guifg=#685F5A guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Folded guifg=#AFA099 guibg=#393431 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Function guifg=#B4BDC3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TroubleNormal Function";
    x += "hi! link TroubleText Function";
    x += "hi FzfLuaBufFlagAlt guifg=#6099C0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufFlagCur guifg=#B77E64 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufNr guifg=#819B69 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderBind guifg=#819B69 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderText guifg=#B77E64 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaLiveSym guifg=#B77E64 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaPathColNr guifg=#A1938C guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaPathLineNr FzfLuaPathColNr";
    x += "hi FzfLuaTabMarker guifg=#819B69 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaTabTitle guifg=#66A5AD guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi GitSignsAdd guifg=#819B69 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterAdd GitSignsAdd";
    x += "hi GitSignsChange guifg=#6099C0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterChange GitSignsChange";
    x += "hi GitSignsDelete guifg=#DE6E7C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterDelete GitSignsDelete";
    x += "hi IblIndent guifg=#2B2725 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IblScope guifg=#494341 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Identifier guifg=#979FA4 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IncSearch guifg=#1C1917 guibg=#BF8FB5 guisp=NONE gui=bold cterm=bold";
    x += "hi! link CurSearch IncSearch";
    x += "hi Italic guifg=NONE guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi LineNr guifg=#685F5A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocCodeLens LineNr";
    x += "hi! link LspCodeLens LineNr";
    x += "hi! link SignColumn LineNr";
    x += "hi LspInlayHint guifg=#79675E guibg=#25211F guisp=NONE gui=NONE cterm=NONE";
    x += "hi MoreMsg guifg=#819B69 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Question MoreMsg";
    x += "hi! link NnnNormalNC NnnNormal";
    x += "hi! link NnnVertSplit NnnWinSeparator";
    x += "hi NonText guifg=#5C534F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link EndOfBuffer NonText";
    x += "hi! link Whitespace NonText";
    x += "hi NormalFloat guifg=NONE guibg=#302B29 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Number guifg=#B4BDC3 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Boolean Number";
    x += "hi Pmenu guifg=NONE guibg=#302B29 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSbar guifg=NONE guibg=#615853 guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSel guifg=NONE guibg=#4A433F guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuThumb guifg=NONE guibg=#8E817B guisp=NONE gui=NONE cterm=NONE";
    x += "hi Search guifg=#B4BDC3 guibg=#65435E guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocSearch Search";
    x += "hi! link MatchParen Search";
    x += "hi! link QuickFixLine Search";
    x += "hi! link Sneak Search";
    x += "hi SneakLabelMask guifg=#B279A7 guibg=#B279A7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Special guifg=#8D9499 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link WhichKeyGroup Special";
    x += "hi SpecialComment guifg=#6E6763 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownUrl SpecialComment";
    x += "hi SpecialKey guifg=#5C534F guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi SpellBad guifg=#CB7A83 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi! link CocSelectedText SpellBad";
    x += "hi SpellCap guifg=#CB7A83 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi! link SpellLocal SpellCap";
    x += "hi SpellRare guifg=#CB7A83 guibg=NONE guisp=NONE gui=undercurl cterm=undercurl";
    x += "hi Statement guifg=#B4BDC3 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaBufName Statement";
    x += "hi! link PreProc Statement";
    x += "hi! link WhichKey Statement";
    x += "hi StatusLine guifg=#B4BDC3 guibg=#352F2D guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLine StatusLine";
    x += "hi! link WinBar StatusLine";
    x += "hi StatusLineNC guifg=#CAD0D4 guibg=#272321 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLineFill StatusLineNC";
    x += "hi! link WinBarNC StatusLineNC";
    x += "hi TabLineSel guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link BufferCurrent TabLineSel";
    x += "hi Title guifg=#B4BDC3 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Todo guifg=NONE guibg=NONE guisp=NONE gui=bold,underline cterm=bold,underline";
    x += "hi Type guifg=#A1938C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link helpSpecial Type";
    x += "hi! link markdownCode Type";
    x += "hi Underlined guifg=NONE guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi Visual guifg=NONE guibg=#3D4042 guisp=NONE gui=NONE cterm=NONE";
    x += "hi WarningMsg guifg=#B77E64 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticWarn WarningMsg";
    x += "hi! link gitcommitOverflow WarningMsg";
    x += "hi WhichKeySeparator guifg=#685F5A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi WildMenu guifg=#1C1917 guibg=#B279A7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link SneakLabel WildMenu";
    x += "hi WinSeparator guifg=#685F5A guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link VertSplit WinSeparator";
    x += "hi diffAdded guifg=#819B69 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffChanged guifg=#6099C0 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffFile guifg=#B77E64 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffIndexLine guifg=#B77E64 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffLine guifg=#B279A7 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffNewFile guifg=#819B69 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffOldFile guifg=#DE6E7C guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffRemoved guifg=#DE6E7C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi helpHyperTextEntry guifg=#A1938C guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi helpHyperTextJump guifg=#979FA4 guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi lCursor guifg=#1C1917 guibg=#797F84 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursorNC lCursor";
    x += "hi markdownLinkText guifg=#979FA4 guibg=NONE guisp=NONE gui=underline cterm=underline";

#if (0)
    if !s:italics
        highlight Boolean gui=NONE cterm=NONE
        highlight Character gui=NONE cterm=NONE
        highlight Comment gui=NONE cterm=NONE
        highlight Constant gui=NONE cterm=NONE
        highlight Float gui=NONE cterm=NONE
        highlight Number gui=NONE cterm=NONE
        highlight SpecialKey gui=NONE cterm=NONE
        highlight String gui=NONE cterm=NONE
        highlight TroubleSource gui=NONE cterm=NONE
        highlight WhichKeyValue gui=NONE cterm=NONE
        highlight diffNewFile gui=NONE cterm=NONE
        highlight diffOldFile gui=NONE cterm=NONE
        highlight helpOption gui=NONE cterm=NONE
    endif
#endif

    bones_scheme_set(x);
}


static void
zenbones_light(void)
{
//  let g:terminal_color_0 = '#F0EDEC'
//  let g:terminal_color_1 = '#A8334C'
//  let g:terminal_color_2 = '#4F6C31'
//  let g:terminal_color_3 = '#944927'
//  let g:terminal_color_4 = '#286486'
//  let g:terminal_color_5 = '#88507D'
//  let g:terminal_color_6 = '#3B8992'
//  let g:terminal_color_7 = '#2C363C'
//  let g:terminal_color_8 = '#CFC1BA'
//  let g:terminal_color_9 = '#94253E'
//  let g:terminal_color_10 = '#3F5A22'
//  let g:terminal_color_11 = '#803D1C'
//  let g:terminal_color_12 = '#1D5573'
//  let g:terminal_color_13 = '#7B3B70'
//  let g:terminal_color_14 = '#2B747C'
//  let g:terminal_color_15 = '#4F5E68'

    list x;

    x += "set background=light";
    x += "hi clear";
    x += "hi Normal guifg=#2C363C guibg=#F0EDEC guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link ModeMsg Normal";
    x += "hi Bold guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi BufferVisible guifg=#596A76 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleIndex guifg=#596A76 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi BufferVisibleSign guifg=#596A76 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi CocMarkdownLink guifg=#3B8992 guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi ColorColumn guifg=NONE guibg=#E6C5BD guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link LspReferenceRead ColorColumn";
    x += "hi! link LspReferenceText ColorColumn";
    x += "hi! link LspReferenceWrite ColorColumn";
    x += "hi Comment guifg=#948985 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi Conceal guifg=#4F5E68 guibg=NONE guisp=NONE gui=bold,italic cterm=bold,italic";
    x += "hi Constant guifg=#556570 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Character Constant";
    x += "hi! link Float Constant";
    x += "hi! link String Constant";
    x += "hi! link TroubleSource Constant";
    x += "hi! link WhichKeyValue Constant";
    x += "hi! link helpOption Constant";
    x += "hi Cursor guifg=#F0EDEC guibg=#2C363C guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursor Cursor";
    x += "hi CursorLine guifg=NONE guibg=#E9E4E2 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocMenuSel CursorLine";
    x += "hi! link CursorColumn CursorLine";
    x += "hi CursorLineNr guifg=#2C363C guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Delimiter guifg=#8E817B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownLinkTextDelimiter Delimiter";
    x += "hi! link NotifyERRORIcon DiagnosticError";
    x += "hi! link NotifyERRORTitle DiagnosticError";
    x += "hi DiagnosticHint guifg=#88507D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyDEBUGIcon DiagnosticHint";
    x += "hi! link NotifyDEBUGTitle DiagnosticHint";
    x += "hi! link NotifyTRACEIcon DiagnosticHint";
    x += "hi! link NotifyTRACETitle DiagnosticHint";
    x += "hi DiagnosticInfo guifg=#286486 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyINFOIcon DiagnosticInfo";
    x += "hi! link NotifyINFOTitle DiagnosticInfo";
    x += "hi DiagnosticOk guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignError guifg=#A8334C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorSign DiagnosticSignError";
    x += "hi DiagnosticSignHint guifg=#88507D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocHintSign DiagnosticSignHint";
    x += "hi DiagnosticSignInfo guifg=#286486 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocInfoSign DiagnosticSignInfo";
    x += "hi DiagnosticSignOk guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticSignWarn guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningSign DiagnosticSignWarn";
    x += "hi DiagnosticUnderlineError guifg=NONE guibg=NONE guisp=#A8334C gui=undercurl cterm=undercurl";
    x += "hi! link CocErrorHighlight DiagnosticUnderlineError";
    x += "hi DiagnosticUnderlineHint guifg=NONE guibg=NONE guisp=#88507D gui=undercurl cterm=undercurl";
    x += "hi! link CocHintHighlight DiagnosticUnderlineHint";
    x += "hi DiagnosticUnderlineInfo guifg=NONE guibg=NONE guisp=#286486 gui=undercurl cterm=undercurl";
    x += "hi! link CocInfoHighlight DiagnosticUnderlineInfo";
    x += "hi DiagnosticUnderlineOk guifg=NONE guibg=NONE guisp=#4F6C31 gui=undercurl cterm=undercurl";
    x += "hi DiagnosticUnderlineWarn guifg=NONE guibg=NONE guisp=#944927 gui=undercurl cterm=undercurl";
    x += "hi! link CocWarningHighlight DiagnosticUnderlineWarn";
    x += "hi DiagnosticVirtualTextError guifg=#A8334C guibg=#EFDFE0 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorVirtualText DiagnosticVirtualTextError";
    x += "hi DiagnosticVirtualTextHint guifg=#88507D guibg=#EFDEEB guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextInfo guifg=#286486 guibg=#D9E4EF guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextOk guifg=#4F6C31 guibg=#C9EEAB guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiagnosticVirtualTextWarn guifg=#944927 guibg=#EFDFDC guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningVirtualText DiagnosticVirtualTextWarn";
    x += "hi! link DiagnosticDeprecated DiagnosticWarn";
    x += "hi! link DiagnosticUnnecessary DiagnosticWarn";
    x += "hi! link NotifyWARNIcon DiagnosticWarn";
    x += "hi! link NotifyWARNTitle DiagnosticWarn";
    x += "hi DiffAdd guifg=NONE guibg=#CBE5B8 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffChange guifg=NONE guibg=#D4DEE7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffDelete guifg=NONE guibg=#EBD8DA guisp=NONE gui=NONE cterm=NONE";
    x += "hi DiffText guifg=#2C363C guibg=#A9BED1 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Directory guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Error guifg=#A8334C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticError Error";
    x += "hi! link ErrorMsg Error";
    x += "hi FlashBackdrop guifg=#948985 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FlashLabel guifg=#2C363C guibg=#8FCAF6 guisp=NONE gui=NONE cterm=NONE";
    x += "hi FloatBorder guifg=#786D68 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FoldColumn guifg=#A4968F guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Folded guifg=#564E4A guibg=#CDC2BC guisp=NONE gui=NONE cterm=NONE";
    x += "hi Function guifg=#2C363C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TroubleNormal Function";
    x += "hi! link TroubleText Function";
    x += "hi FzfLuaBufFlagAlt guifg=#286486 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufFlagCur guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaBufNr guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderBind guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaHeaderText guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaLiveSym guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaPathColNr guifg=#6A5549 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaPathLineNr FzfLuaPathColNr";
    x += "hi FzfLuaTabMarker guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi FzfLuaTabTitle guifg=#3B8992 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi GitSignsAdd guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterAdd GitSignsAdd";
    x += "hi GitSignsChange guifg=#286486 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterChange GitSignsChange";
    x += "hi GitSignsDelete guifg=#A8334C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterDelete GitSignsDelete";
    x += "hi IblIndent guifg=#E1DCDA guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IblScope guifg=#BEB1AA guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi Identifier guifg=#44525B guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi IncSearch guifg=#F0EDEC guibg=#C074B2 guisp=NONE gui=bold cterm=bold";
    x += "hi! link CurSearch IncSearch";
    x += "hi Italic guifg=NONE guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi LineNr guifg=#A4968F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocCodeLens LineNr";
    x += "hi! link LspCodeLens LineNr";
    x += "hi! link SignColumn LineNr";
    x += "hi LspInlayHint guifg=#A38C80 guibg=#EBE7E6 guisp=NONE gui=NONE cterm=NONE";
    x += "hi MoreMsg guifg=#4F6C31 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Question MoreMsg";
    x += "hi! link NnnNormalNC NnnNormal";
    x += "hi! link NnnVertSplit NnnWinSeparator";
    x += "hi NonText guifg=#BBABA3 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link EndOfBuffer NonText";
    x += "hi! link Whitespace NonText";
    x += "hi NormalFloat guifg=NONE guibg=#DDD6D3 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Number guifg=#2C363C guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Boolean Number";
    x += "hi Pmenu guifg=NONE guibg=#DAD3CF guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSbar guifg=NONE guibg=#B2A39B guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuSel guifg=NONE guibg=#C4B6AF guisp=NONE gui=NONE cterm=NONE";
    x += "hi PmenuThumb guifg=NONE guibg=#F7F6F5 guisp=NONE gui=NONE cterm=NONE";
    x += "hi Search guifg=#2C363C guibg=#DEB9D6 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocSearch Search";
    x += "hi! link MatchParen Search";
    x += "hi! link QuickFixLine Search";
    x += "hi! link Sneak Search";
    x += "hi SneakLabelMask guifg=#88507D guibg=#88507D guisp=NONE gui=NONE cterm=NONE";
    x += "hi Special guifg=#4F5E68 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link WhichKeyGroup Special";
    x += "hi SpecialComment guifg=#948985 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownUrl SpecialComment";
    x += "hi SpecialKey guifg=#BBABA3 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi SpellBad guifg=#974352 guibg=NONE guisp=#A8334C gui=undercurl cterm=undercurl";
    x += "hi! link CocSelectedText SpellBad";
    x += "hi SpellCap guifg=#974352 guibg=NONE guisp=#C13C58 gui=undercurl cterm=undercurl";
    x += "hi! link SpellLocal SpellCap";
    x += "hi SpellRare guifg=#974352 guibg=NONE guisp=#944927 gui=undercurl cterm=undercurl";
    x += "hi Statement guifg=#2C363C guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaBufName Statement";
    x += "hi! link PreProc Statement";
    x += "hi! link WhichKey Statement";
    x += "hi StatusLine guifg=#2C363C guibg=#D6CDC9 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLine StatusLine";
    x += "hi! link WinBar StatusLine";
    x += "hi StatusLineNC guifg=#596A76 guibg=#E1DCD9 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLineFill StatusLineNC";
    x += "hi! link WinBarNC StatusLineNC";
    x += "hi TabLineSel guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link BufferCurrent TabLineSel";
    x += "hi Title guifg=#2C363C guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi Todo guifg=NONE guibg=NONE guisp=NONE gui=bold,underline cterm=bold,underline";
    x += "hi Type guifg=#6A5549 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link helpSpecial Type";
    x += "hi! link markdownCode Type";
    x += "hi Underlined guifg=NONE guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi Visual guifg=NONE guibg=#CBD9E3 guisp=NONE gui=NONE cterm=NONE";
    x += "hi WarningMsg guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticWarn WarningMsg";
    x += "hi! link gitcommitOverflow WarningMsg";
    x += "hi WhichKeySeparator guifg=#A4968F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi WildMenu guifg=#F0EDEC guibg=#88507D guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link SneakLabel WildMenu";
    x += "hi WinSeparator guifg=#A4968F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link VertSplit WinSeparator";
    x += "hi diffAdded guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffChanged guifg=#286486 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffFile guifg=#944927 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffIndexLine guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi diffLine guifg=#88507D guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi diffNewFile guifg=#4F6C31 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffOldFile guifg=#A8334C guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi diffRemoved guifg=#A8334C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi helpHyperTextEntry guifg=#6A5549 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi helpHyperTextJump guifg=#44525B guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi lCursor guifg=#F0EDEC guibg=#4D5C65 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursorNC lCursor";
    x += "hi markdownLinkText guifg=#44525B guibg=NONE guisp=NONE gui=underline cterm=underline";

#if (0)
    if !s:italics
        highlight Boolean gui=NONE cterm=NONE
        highlight Character gui=NONE cterm=NONE
        highlight Comment gui=NONE cterm=NONE
        highlight Constant gui=NONE cterm=NONE
        highlight Float gui=NONE cterm=NONE
        highlight Number gui=NONE cterm=NONE
        highlight SpecialKey gui=NONE cterm=NONE
        highlight String gui=NONE cterm=NONE
        highlight TroubleSource gui=NONE cterm=NONE
        highlight WhichKeyValue gui=NONE cterm=NONE
        highlight diffNewFile gui=NONE cterm=NONE
        highlight diffOldFile gui=NONE cterm=NONE
        highlight helpOption gui=NONE cterm=NONE
    endif
#endif

    bones_scheme_set(x);
}

//end
