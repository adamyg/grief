/* -*- mode: cr; indent-width: 4; -*- */
// $Id: vimbones.cr,v 1.3 2024/10/27 06:09:51 cvsuser Exp $
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

static void vimbones_light(void);


void
main(void)
{
    module("csbones");
}


static int
vimbones(int dark)
{
    if (0 == dark) {
        vimbones_light();
        return 0;
    }
    return -1;
}


static void
vimbones_light(void)
{
//  let s:italics = (&t_ZH != '' && &t_ZH != '[7m') || has('gui_running') || has('nvim')
//  let g:terminal_color_0  = '#F0F0CA'
//  let g:terminal_color_1  = '#A8334C'
//  let g:terminal_color_2  = '#4F6C31'
//  let g:terminal_color_3  = '#944927'
//  let g:terminal_color_4  = '#286486'
//  let g:terminal_color_5  = '#88507D'
//  let g:terminal_color_6  = '#3B8992'
//  let g:terminal_color_7  = '#353535'
//  let g:terminal_color_8  = '#C6C6A3'
//  let g:terminal_color_9  = '#94253E'
//  let g:terminal_color_10 = '#3F5A22'
//  let g:terminal_color_11 = '#803D1C'
//  let g:terminal_color_12 = '#1D5573'
//  let g:terminal_color_13 = '#7B3B70'
//  let g:terminal_color_14 = '#2B747C'
//  let g:terminal_color_15 = '#5C5C5C'

    list x;

    x += "set background=light";
    x += "hi  clear";
    x += "hi  Normal guifg=#353535 guibg=#F0F0CA guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link ModeMsg Normal";
    x += "hi  Bold guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi  BufferVisible guifg=#686868 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  BufferVisibleIndex guifg=#686868 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  BufferVisibleSign guifg=#686868 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  CocMarkdownLink guifg=#3B8992 guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi  ColorColumn guifg=NONE guibg=#E6C5BD guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link LspReferenceRead ColorColumn";
    x += "hi! link LspReferenceText ColorColumn";
    x += "hi! link LspReferenceWrite ColorColumn";
    x += "hi  Comment guifg=#8C8C7C guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi  Conceal guifg=#5C5C5C guibg=NONE guisp=NONE gui=bold,italic cterm=bold,italic";
    x += "hi  Constant guifg=#636363 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Character Constant";
    x += "hi! link Float Constant";
    x += "hi! link String Constant";
    x += "hi! link TroubleSource Constant";
    x += "hi! link WhichKeyValue Constant";
    x += "hi! link helpOption Constant";
    x += "hi  Cursor guifg=#F0F0CA guibg=#353535 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursor Cursor";
    x += "hi  CursorLine guifg=NONE guibg=#E7E8C3 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocMenuSel CursorLine";
    x += "hi! link CursorColumn CursorLine";
    x += "hi  CursorLineNr guifg=#353535 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi  Delimiter guifg=#85856F guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownLinkTextDelimiter Delimiter";
    x += "hi! link NotifyERRORIcon DiagnosticError";
    x += "hi! link NotifyERRORTitle DiagnosticError";
    x += "hi  DiagnosticHint guifg=#88507D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyDEBUGIcon DiagnosticHint";
    x += "hi! link NotifyDEBUGTitle DiagnosticHint";
    x += "hi! link NotifyTRACEIcon DiagnosticHint";
    x += "hi! link NotifyTRACETitle DiagnosticHint";
    x += "hi  DiagnosticInfo guifg=#286486 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link NotifyINFOIcon DiagnosticInfo";
    x += "hi! link NotifyINFOTitle DiagnosticInfo";
    x += "hi  DiagnosticOk guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  DiagnosticSignError guifg=#A8334C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorSign DiagnosticSignError";
    x += "hi  DiagnosticSignHint guifg=#88507D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocHintSign DiagnosticSignHint";
    x += "hi  DiagnosticSignInfo guifg=#286486 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocInfoSign DiagnosticSignInfo";
    x += "hi  DiagnosticSignOk guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  DiagnosticSignWarn guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningSign DiagnosticSignWarn";
    x += "hi  DiagnosticUnderlineError guifg=NONE guibg=NONE guisp=#A8334C gui=undercurl cterm=undercurl";
    x += "hi! link CocErrorHighlight DiagnosticUnderlineError";
    x += "hi  DiagnosticUnderlineHint guifg=NONE guibg=NONE guisp=#88507D gui=undercurl cterm=undercurl";
    x += "hi! link CocHintHighlight DiagnosticUnderlineHint";
    x += "hi  DiagnosticUnderlineInfo guifg=NONE guibg=NONE guisp=#286486 gui=undercurl cterm=undercurl";
    x += "hi! link CocInfoHighlight DiagnosticUnderlineInfo";
    x += "hi  DiagnosticUnderlineOk guifg=NONE guibg=NONE guisp=#4F6C31 gui=undercurl cterm=undercurl";
    x += "hi  DiagnosticUnderlineWarn guifg=NONE guibg=NONE guisp=#944927 gui=undercurl cterm=undercurl";
    x += "hi! link CocWarningHighlight DiagnosticUnderlineWarn";
    x += "hi  DiagnosticVirtualTextError guifg=#A8334C guibg=#EFDFE0 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocErrorVirtualText DiagnosticVirtualTextError";
    x += "hi  DiagnosticVirtualTextHint guifg=#88507D guibg=#EFDEEB guisp=NONE gui=NONE cterm=NONE";
    x += "hi  DiagnosticVirtualTextInfo guifg=#286486 guibg=#D9E4EF guisp=NONE gui=NONE cterm=NONE";
    x += "hi  DiagnosticVirtualTextOk guifg=#4F6C31 guibg=#C9EEAB guisp=NONE gui=NONE cterm=NONE";
    x += "hi  DiagnosticVirtualTextWarn guifg=#944927 guibg=#EFDFDC guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocWarningVirtualText DiagnosticVirtualTextWarn";
    x += "hi! link DiagnosticDeprecated DiagnosticWarn";
    x += "hi! link DiagnosticUnnecessary DiagnosticWarn";
    x += "hi! link NotifyWARNIcon DiagnosticWarn";
    x += "hi! link NotifyWARNTitle DiagnosticWarn";
    x += "hi  DiffAdd guifg=NONE guibg=#CBE5B8 guisp=NONE gui=NONE cterm=NONE";
    x += "hi  DiffChange guifg=NONE guibg=#D4DEE7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi  DiffDelete guifg=NONE guibg=#EBD8DA guisp=NONE gui=NONE cterm=NONE";
    x += "hi  DiffText guifg=#353535 guibg=#A9BED1 guisp=NONE gui=NONE cterm=NONE";
    x += "hi  Directory guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi  Error guifg=#A8334C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticError Error";
    x += "hi! link ErrorMsg Error";
    x += "hi  FlashBackdrop guifg=#8C8C7C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  FlashLabel guifg=#353535 guibg=#8FCAF6 guisp=NONE gui=NONE cterm=NONE";
    x += "hi  FloatBorder guifg=#71715E guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  FoldColumn guifg=#9A9A81 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi  Folded guifg=#515143 guibg=#C6C6A6 guisp=NONE gui=NONE cterm=NONE";
    x += "hi  Function guifg=#353535 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TroubleNormal Function";
    x += "hi! link TroubleText Function";
    x += "hi  FzfLuaBufFlagAlt guifg=#286486 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  FzfLuaBufFlagCur guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  FzfLuaBufNr guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  FzfLuaHeaderBind guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  FzfLuaHeaderText guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  FzfLuaLiveSym guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  FzfLuaPathColNr guifg=#5B5B42 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaPathLineNr FzfLuaPathColNr";
    x += "hi  FzfLuaTabMarker guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  FzfLuaTabTitle guifg=#3B8992 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  GitSignsAdd guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterAdd GitSignsAdd";
    x += "hi  GitSignsChange guifg=#286486 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterChange GitSignsChange";
    x += "hi  GitSignsDelete guifg=#A8334C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link GitGutterDelete GitSignsDelete";
    x += "hi  IblIndent guifg=#DFDFC2 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  IblScope guifg=#B5B59D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  Identifier guifg=#505050 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  IncSearch guifg=#F0F0CA guibg=#C074B2 guisp=NONE gui=bold cterm=bold";
    x += "hi! link CurSearch IncSearch";
    x += "hi  Italic guifg=NONE guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi  LineNr guifg=#9A9A81 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocCodeLens LineNr";
    x += "hi! link LspCodeLens LineNr";
    x += "hi! link SignColumn LineNr";
    x += "hi  LspInlayHint guifg=#929274 guibg=#EAEAC6 guisp=NONE gui=NONE cterm=NONE";
    x += "hi  MoreMsg guifg=#4F6C31 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link Question MoreMsg";
    x += "hi! link NnnNormalNC NnnNormal";
    x += "hi! link NnnVertSplit NnnWinSeparator";
    x += "hi  NonText guifg=#B0B093 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link EndOfBuffer NonText";
    x += "hi! link Whitespace NonText";
    x += "hi  NormalFloat guifg=NONE guibg=#D9D9B7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi  Number guifg=#2A6535 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi! link Boolean Number";
    x += "hi  Pmenu guifg=NONE guibg=#D6D6B5 guisp=NONE gui=NONE cterm=NONE";
    x += "hi  PmenuSbar guifg=NONE guibg=#A7A78D guisp=NONE gui=NONE cterm=NONE";
    x += "hi  PmenuSel guifg=NONE guibg=#BABB9D guisp=NONE gui=NONE cterm=NONE";
    x += "hi  PmenuThumb guifg=NONE guibg=#F9F9D2 guisp=NONE gui=NONE cterm=NONE";
    x += "hi  PreProc guifg=#35663D guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  Search guifg=#353535 guibg=#DEB9D6 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link CocSearch Search";
    x += "hi! link MatchParen Search";
    x += "hi! link QuickFixLine Search";
    x += "hi! link Sneak Search";
    x += "hi  SneakLabelMask guifg=#88507D guibg=#88507D guisp=NONE gui=NONE cterm=NONE";
    x += "hi  Special guifg=#5C5C5C guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link WhichKeyGroup Special";
    x += "hi  SpecialComment guifg=#8C8C7C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link markdownUrl SpecialComment";
    x += "hi  SpecialKey guifg=#B0B093 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi  SpellBad guifg=#974352 guibg=NONE guisp=#A8334C gui=undercurl cterm=undercurl";
    x += "hi! link CocSelectedText SpellBad";
    x += "hi  SpellCap guifg=#974352 guibg=NONE guisp=#C13C58 gui=undercurl cterm=undercurl";
    x += "hi! link SpellLocal SpellCap";
    x += "hi  SpellRare guifg=#974352 guibg=NONE guisp=#944927 gui=undercurl cterm=undercurl";
    x += "hi  Statement guifg=#156A29 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link FzfLuaBufName Statement";
    x += "hi! link WhichKey Statement";
    x += "hi  StatusLine guifg=#353535 guibg=#D1D1B0 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLine StatusLine";
    x += "hi! link WinBar StatusLine";
    x += "hi  StatusLineNC guifg=#686868 guibg=#DFDFBC guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TabLineFill StatusLineNC";
    x += "hi! link WinBarNC StatusLineNC";
    x += "hi  TabLineSel guifg=NONE guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi! link BufferCurrent TabLineSel";
    x += "hi  Title guifg=#353535 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi  Todo guifg=NONE guibg=NONE guisp=NONE gui=bold,underline cterm=bold,underline";
    x += "hi  Type guifg=#5B5B42 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link helpSpecial Type";
    x += "hi! link markdownCode Type";
    x += "hi  Underlined guifg=NONE guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi  Visual guifg=NONE guibg=#D7D7D7 guisp=NONE gui=NONE cterm=NONE";
    x += "hi  WarningMsg guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link DiagnosticWarn WarningMsg";
    x += "hi! link gitcommitOverflow WarningMsg";
    x += "hi  WhichKeySeparator guifg=#9A9A81 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  WildMenu guifg=#F0F0CA guibg=#88507D guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link SneakLabel WildMenu";
    x += "hi  WinSeparator guifg=#9A9A81 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link VertSplit WinSeparator";
    x += "hi  diffAdded guifg=#4F6C31 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  diffChanged guifg=#286486 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  diffFile guifg=#944927 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi  diffIndexLine guifg=#944927 guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  diffLine guifg=#88507D guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi  diffNewFile guifg=#4F6C31 guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi  diffOldFile guifg=#A8334C guibg=NONE guisp=NONE gui=italic cterm=italic";
    x += "hi  diffRemoved guifg=#A8334C guibg=NONE guisp=NONE gui=NONE cterm=NONE";
    x += "hi  helpHyperTextEntry guifg=#5B5B42 guibg=NONE guisp=NONE gui=bold cterm=bold";
    x += "hi  helpHyperTextJump guifg=#505050 guibg=NONE guisp=NONE gui=underline cterm=underline";
    x += "hi  lCursor guifg=#F0F0CA guibg=#595959 guisp=NONE gui=NONE cterm=NONE";
    x += "hi! link TermCursorNC lCursor";
    x += "hi  markdownLinkText guifg=#505050 guibg=NONE guisp=NONE gui=underline cterm=underline";

#if (0)
    if !s:italics
        x += "hi  Boolean gui=NONE cterm=NONE";
        x += "hi  Character gui=NONE cterm=NONE";
        x += "hi  Comment gui=NONE cterm=NONE";
        x += "hi  Constant gui=NONE cterm=NONE";
        x += "hi  Float gui=NONE cterm=NONE";
        x += "hi  Number gui=NONE cterm=NONE";
        x += "hi  SpecialKey gui=NONE cterm=NONE";
        x += "hi  String gui=NONE cterm=NONE";
        x += "hi  TroubleSource gui=NONE cterm=NONE";
        x += "hi  WhichKeyValue gui=NONE cterm=NONE";
        x += "hi  diffNewFile gui=NONE cterm=NONE";
        x += "hi  diffOldFile gui=NONE cterm=NONE";
        x += "hi  helpOption gui=NONE cterm=NONE";
    endif
#endif

    bones_scheme_set(x);
}

//endif
