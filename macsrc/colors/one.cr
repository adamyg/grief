/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: one.cr,v 1.5 2024/10/07 16:23:06 cvsuser Exp $

   Source: https://github.com/rakr/vim-one/
   Version: 1.1.1-pre

   Light and dark vim/style colorscheme, shamelessly stolen from atom (another excellent text editor).
   One supports true colors and falls back gracefully and automatically if your environment does not support this feature.

   Author: Ramzi Akremi

   =================================================================================

   The MIT License (MIT)

   Copyright (c) 2016 Ramzi Akremi

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

   =================================================================================
*/

#include "../grief.h"
#include "util/rgbmap.h"

void
main(void)
{
    require("colors/util/rgbmap");
}


string
rbg_map(string rgb)
{
    extern int colordepth;
    int r,g,b;

    sscanf(rgb, "#%2x%2x%2x", r, g, b);
    return RGBPaletteIndex(r, g, b, colordepth);
}


// Highlight function
static string
X(string group, ~list fg, ~list bg, ~string attr)
{
    extern int colordepth;
    string s, guisp;

    s = "hi " + group;

    if (colordepth < 256) {
        if (! is_null(fg)) {
            if (length_of_list(fg)) {
                s += " guifg=" + fg[0];
                s += " ctermfg=" + rbg_map(fg[0]);
                if (length_of_list(fg) >= 3) {
                    guisp = fg[2];
                }
            }
        }

        if (! is_null(bg)) {
            if (length_of_list(bg)) {
                s += " guibg=" + bg[0];
                s += " ctermbg=" + rbg_map(bg[0]);
                if (length_of_list(bg) >= 3) {
                    guisp = bg[2];
                }
            }
        }

    } else {
        if (! is_null(fg)) {
            if (length_of_list(fg)) {
                s += " guifg=" + fg[0];
                s += " ctermfg=" + fg[1];
                if (length_of_list(fg) >= 3) {
                    guisp = fg[2];
                }
            }
        }

        if (! is_null(bg)) {
            if (length_of_list(bg)) {
                s += " guibg=" + bg[0];
                s += " ctermbg=" + bg[1];
                if (length_of_list(bg) >= 3) {
                    guisp = bg[2];
                }
            }
        }
    }

    if (is_string(attr)) {
        if (attr) {
            s += " gui=" + attr;
            s += " cterm=" + attr;
        }
    }

    // TODO: "guisp" is used for undercurl and strikethrough.
    //  if (guisp)
    //      s += " guisp=" + guisp;

    return s;
}


static void
one(string scheme, int dark, int italic)
{
    int gui_running = 0;   // TODO: TF_GUI

    list kmono_1;
    list kmono_2;
    list kmono_3;
    list kmono_4;
    list khue_1;
    list khue_2;
    list khue_3;
    list khue_4;
    list khue_5;
    list khue_5_2;
    list khue_6;
    list khue_6_2;
    list ksyn_bg;
    list ksyn_gutter;
    list ksyn_cursor;
    list ksyn_accent;
    list ksyn_accent_2;
    list kvertsplit;
    list kspecial_grey;
    list kvisual_grey;
    list kpmenu;

    // Color definition
    // --------------------------------------------------------
    if (dark) {
        kmono_1         = quote_list("#abb2bf", "145");
        kmono_2         = quote_list("#828997", "102");
        kmono_3         = quote_list("#5c6370", "59");
        kmono_4         = quote_list("#4b5263", "59");

        khue_1          = quote_list("#56b6c2", "73",   "cyan");
        khue_2          = quote_list("#61afef", "75",   "blue");
        khue_3          = quote_list("#c678dd", "176",  "purple");
        khue_4          = quote_list("#98c379", "114",  "green");

        khue_5          = quote_list("#e06c75", "168",  "red 1");
        khue_5_2        = quote_list("#be5046", "130",  "red 2");

        khue_6          = quote_list("#d19a66", "173",  "orange 1");
        khue_6_2        = quote_list("#e5c07b", "180",  "orange 2");

        ksyn_bg         = quote_list("#282c34", "16");
        ksyn_gutter     = quote_list("#636d83", "60");
        ksyn_cursor     = quote_list("#2c323c", "16");
        ksyn_accent     = quote_list("#528bff", "69");

        kvertsplit      = quote_list("#181a1f", "233");
        kspecial_grey   = quote_list("#3b4048", "16");
        kvisual_grey    = quote_list("#3e4452", "17");
        kpmenu          = quote_list("#333841", "16");

    } else {
        kmono_1         = quote_list("#494b53", "23");
        kmono_2         = quote_list("#696c77", "60");
        kmono_3         = quote_list("#a0a1a7", "145");
        kmono_4         = quote_list("#c2c2c3", "250");

        khue_1          = quote_list("#0184bc", "31",   "cyan");
        khue_2          = quote_list("#4078f2", "33",   "blue");
        khue_3          = quote_list("#a626a4", "127",  "purple");
        khue_4          = quote_list("#50a14f", "71",   "green");

        khue_5          = quote_list("#e45649", "166",  "red 1");
        khue_5_2        = quote_list("#ca1243", "160",  "red 2");

        khue_6          = quote_list("#986801", "94",   "orange 1");
        khue_6_2        = quote_list("#c18401", "136",  "orange 2");

        ksyn_bg         = quote_list("#fafafa", "255");
        ksyn_gutter     = quote_list("#9e9e9e", "247");
        ksyn_cursor     = quote_list("#f0f0f0", "254");

        ksyn_accent     = quote_list("#526fff", "63");
        ksyn_accent_2   = quote_list("#0083be", "31");

        kvertsplit      = quote_list("#e7e9e1", "188");
        kspecial_grey   = quote_list("#d3d3d3", "251");
        kvisual_grey    = quote_list("#d0d0d0", "251");
        kpmenu          = quote_list("#dfdfdf", "253");
    }

    const string kitalic = (italic ? "italic" : "");
    list ksyn_fg = kmono_1;
    list x;

    x += "hi clear";
    x += "syntax reset";

    // Vim editor color --------------------------------------------------------
    x += X("Normal",                            ksyn_fg,        ksyn_bg,        NULL);
    x += X("bold",                              NULL,           NULL,           "bold");
    x += X("ColorColumn",                       NULL,           ksyn_cursor,    NULL);
    x += X("Conceal",                           kmono_4,        ksyn_bg,        NULL);
    x += X("Cursor",                            NULL,           ksyn_accent,    NULL);
    x += X("CursorIM",                          NULL,           NULL,           NULL);
    x += X("CursorColumn",                      NULL,           ksyn_cursor,    NULL);
    x += X("CursorLine",                        NULL,           ksyn_cursor,    "none");
    x += X("Directory",                         khue_2,         NULL,           NULL);
    x += X("ErrorMsg",                          khue_5,         ksyn_bg,        "none");
    x += X("VertSplit",                         ksyn_cursor,    ksyn_cursor,    "none");
    x += X("Folded",                            ksyn_fg,        ksyn_bg,        "none");
    x += X("FoldColumn",                        kmono_3,        ksyn_cursor,    NULL);
    x += X("IncSearch",                         khue_6,         NULL,           NULL);
    x += X("LineNr",                            kmono_4,        NULL,           NULL);
    x += X("CursorLineNr",                      ksyn_fg,        ksyn_cursor,    "none");
    x += X("MatchParen",                        khue_5,         ksyn_cursor,    "underline,bold");
    x += X("Italic",                            NULL,           NULL,           kitalic);
    x += X("ModeMsg",                           ksyn_fg,        NULL,           NULL);
    x += X("MoreMsg",                           ksyn_fg,        NULL,           NULL);
    x += X("NonText",                           kmono_3,        NULL,           "none");
    x += X("PMenu",                             NULL,           kpmenu,         NULL);
    x += X("PMenuSel",                          NULL,           kmono_4,        NULL);
    x += X("PMenuSbar",                         NULL,           ksyn_bg,        NULL);
    x += X("PMenuThumb",                        NULL,           kmono_1,        NULL);
    x += X("Question",                          khue_2,         NULL,           NULL);
    x += X("Search",                            ksyn_bg,        khue_6_2,       NULL);
    x += X("SpecialKey",                        kspecial_grey,  NULL,           "none");
    x += X("Whitespace",                        kspecial_grey,  NULL,           "none");
    x += X("StatusLine",                        ksyn_fg,        ksyn_cursor,    "none");
    x += X("StatusLineNC",                      kmono_3,        NULL,           NULL);
    x += X("TabLine",                           kmono_2,        kvisual_grey,   "none");
    x += X("TabLineFill",                       kmono_3,        kvisual_grey,   "none");
    x += X("TabLineSel",                        ksyn_bg,        khue_2,         NULL);
    x += X("Title",                             ksyn_fg,        NULL,           "bold");
    x += X("Visual",                            NULL,           kvisual_grey,   NULL);
    x += X("VisualNOS",                         NULL,           kvisual_grey,   NULL);
    x += X("WarningMsg",                        khue_5,         NULL,           NULL);
    x += X("TooLong",                           khue_5,         NULL,           NULL);
    x += X("WildMenu",                          ksyn_fg,        kmono_3,        NULL);
    x += X("SignColumn",                        NULL,           ksyn_bg,        NULL);
    x += X("Special",                           khue_2,         NULL,           NULL);

    // Vim Help highlighting ---------------------------------------------------
    x += X("helpCommand",                       khue_6_2,       NULL,           NULL);
    x += X("helpExample",                       khue_6_2,       NULL,           NULL);
    x += X("helpHeader",                        kmono_1,        NULL,           "bold");
    x += X("helpSectionDelim",                  kmono_3,        NULL,           NULL);

    // Standard syntax highlighting --------------------------------------------
    x += X("Comment",                           kmono_3,        NULL,           kitalic);
    x += X("Constant",                          khue_4,         NULL,           NULL);
    x += X("String",                            khue_4,         NULL,           NULL);
    x += X("Character",                         khue_4,         NULL,           NULL);
    x += X("Number",                            khue_6,         NULL,           NULL);
    x += X("Boolean",                           khue_6,         NULL,           NULL);
    x += X("Float",                             khue_6,         NULL,           NULL);
    x += X("Identifier",                        khue_5,         NULL,           "none");
    x += X("Function",                          khue_2,         NULL,           NULL);
    x += X("Statement",                         khue_3,         NULL,           "none");
    x += X("Conditional",                       khue_3,         NULL,           NULL);
    x += X("Repeat",                            khue_3,         NULL,           NULL);
    x += X("Label",                             khue_3,         NULL,           NULL);
    x += X("Operator",                          ksyn_accent,    NULL,           "none");
    x += X("Keyword",                           khue_5,         NULL,           NULL);
    x += X("Exception",                         khue_3,         NULL,           NULL);
    x += X("PreProc",                           khue_6_2,       NULL,           NULL);
    x += X("Include",                           khue_2,         NULL,           NULL);
    x += X("Define",                            khue_3,         NULL,           "none");
    x += X("Macro",                             khue_3,         NULL,           NULL);
    x += X("PreCondit",                         khue_6_2,       NULL,           NULL);
    x += X("Type",                              khue_6_2,       NULL,           "none");
    x += X("StorageClass",                      khue_6_2,       NULL,           NULL);
    x += X("Structure",                         khue_6_2,       NULL,           NULL);
    x += X("Typedef",                           khue_6_2,       NULL,           NULL);
    x += X("Special",                           khue_2,         NULL,           NULL);
    x += X("SpecialChar",                       NULL,           NULL,           NULL);
    x += X("Tag",                               NULL,           NULL,           NULL);
    x += X("Delimiter",                         NULL,           NULL,           NULL);
    x += X("SpecialComment",                    NULL,           NULL,           NULL);
    x += X("Debug",                             NULL,           NULL,           NULL);
    x += X("Underlined",                        NULL,           NULL,           "underline");
    x += X("Ignore",                            NULL,           NULL,           NULL);
    x += X("Error",                             khue_5,         ksyn_bg,        "bold");
    x += X("Todo",                              khue_3,         ksyn_bg,        NULL);

    // Diff highlighting -------------------------------------------------------
    x += X("DiffAdd",                           khue_4,         kvisual_grey,   NULL);
    x += X("DiffChange",                        khue_6,         kvisual_grey,   NULL);
    x += X("DiffDelete",                        khue_5,         kvisual_grey,   NULL);
    x += X("DiffText",                          khue_2,         kvisual_grey,   NULL);
    x += X("DiffAdded",                         khue_4,         kvisual_grey,   NULL);
    x += X("DiffFile",                          khue_5,         kvisual_grey,   NULL);
    x += X("DiffNewFile",                       khue_4,         kvisual_grey,   NULL);
    x += X("DiffLine",                          khue_2,         kvisual_grey,   NULL);
    x += X("DiffRemoved",                       khue_5,         kvisual_grey,   NULL);

    // Asciidoc highlighting ---------------------------------------------------
    x += X("asciidocListingBlock",              kmono_2,        NULL,           NULL);

    // C/C++ highlighting ------------------------------------------------------
    x += X("cInclude",                          khue_3,         NULL,           NULL);
    x += X("cPreCondit",                        khue_3,         NULL,           NULL);
    x += X("cPreConditMatch",                   khue_3,         NULL,           NULL);

    x += X("cType",                             khue_3,         NULL,           NULL);
    x += X("cStorageClass",                     khue_3,         NULL,           NULL);
    x += X("cStructure",                        khue_3,         NULL,           NULL);
    x += X("cOperator",                         khue_3,         NULL,           NULL);
    x += X("cStatement",                        khue_3,         NULL,           NULL);
    x += X("cTODO",                             khue_3,         NULL,           NULL);
    x += X("cConstant",                         khue_6,         NULL,           NULL);
    x += X("cSpecial",                          khue_1,         NULL,           NULL);
    x += X("cSpecialCharacter",                 khue_1,         NULL,           NULL);
    x += X("cString",                           khue_4,         NULL,           NULL);

    x += X("cppType",                           khue_3,         NULL,           NULL);
    x += X("cppStorageClass",                   khue_3,         NULL,           NULL);
    x += X("cppStructure",                      khue_3,         NULL,           NULL);
    x += X("cppModifier",                       khue_3,         NULL,           NULL);
    x += X("cppOperator",                       khue_3,         NULL,           NULL);
    x += X("cppAccess",                         khue_3,         NULL,           NULL);
    x += X("cppStatement",                      khue_3,         NULL,           NULL);
    x += X("cppConstant",                       khue_5,         NULL,           NULL);
    x += X("cCppString",                        khue_4,         NULL,           NULL);

    // Cucumber highlighting ---------------------------------------------------
    x += X("cucumberGiven",                     khue_2,         NULL,           NULL);
    x += X("cucumberWhen",                      khue_2,         NULL,           NULL);
    x += X("cucumberWhenAnd",                   khue_2,         NULL,           NULL);
    x += X("cucumberThen",                      khue_2,         NULL,           NULL);
    x += X("cucumberThenAnd",                   khue_2,         NULL,           NULL);
    x += X("cucumberUnparsed",                  khue_6,         NULL,           NULL);
    x += X("cucumberFeature",                   khue_5,         NULL,           "bold");
    x += X("cucumberBackground",                khue_3,         NULL,           "bold");
    x += X("cucumberScenario",                  khue_3,         NULL,           "bold");
    x += X("cucumberScenarioOutline",           khue_3,         NULL,           "bold");
    x += X("cucumberTags",                      kmono_3,        NULL,           "bold");
    x += X("cucumberDelimiter",                 kmono_3,        NULL,           "bold");

    // CSS/Sass highlighting ---------------------------------------------------
    x += X("cssAttrComma",                      khue_3,         NULL,           NULL);
    x += X("cssAttributeSelector",              khue_4,         NULL,           NULL);
    x += X("cssBraces",                         kmono_2,        NULL,           NULL);
    x += X("cssClassName",                      khue_6,         NULL,           NULL);
    x += X("cssClassNameDot",                   khue_6,         NULL,           NULL);
    x += X("cssDefinition",                     khue_3,         NULL,           NULL);
    x += X("cssFontAttr",                       khue_6,         NULL,           NULL);
    x += X("cssFontDescriptor",                 khue_3,         NULL,           NULL);
    x += X("cssFunctionName",                   khue_2,         NULL,           NULL);
    x += X("cssIdentifier",                     khue_2,         NULL,           NULL);
    x += X("cssImportant",                      khue_3,         NULL,           NULL);
    x += X("cssInclude",                        kmono_1,        NULL,           NULL);
    x += X("cssIncludeKeyword",                 khue_3,         NULL,           NULL);
    x += X("cssMediaType",                      khue_6,         NULL,           NULL);
    x += X("cssProp",                           khue_1,         NULL,           NULL);
    x += X("cssPseudoClassId",                  khue_6,         NULL,           NULL);
    x += X("cssSelectorOp",                     khue_3,         NULL,           NULL);
    x += X("cssSelectorOp2",                    khue_3,         NULL,           NULL);
    x += X("cssStringQ",                        khue_4,         NULL,           NULL);
    x += X("cssStringQQ",                       khue_4,         NULL,           NULL);
    x += X("cssTagName",                        khue_5,         NULL,           NULL);
    x += X("cssAttr",                           khue_6,         NULL,           NULL);

    x += X("sassAmpersand",                     khue_5,         NULL,           NULL);
    x += X("sassClass",                         khue_6_2,       NULL,           NULL);
    x += X("sassControl",                       khue_3,         NULL,           NULL);
    x += X("sassExtend",                        khue_3,         NULL,           NULL);
    x += X("sassFor",                           kmono_1,        NULL,           NULL);
    x += X("sassProperty",                      khue_1,         NULL,           NULL);
    x += X("sassFunction",                      khue_1,         NULL,           NULL);
    x += X("sassId",                            khue_2,         NULL,           NULL);
    x += X("sassInclude",                       khue_3,         NULL,           NULL);
    x += X("sassMedia",                         khue_3,         NULL,           NULL);
    x += X("sassMediaOperators",                kmono_1,        NULL,           NULL);
    x += X("sassMixin",                         khue_3,         NULL,           NULL);
    x += X("sassMixinName",                     khue_2,         NULL,           NULL);
    x += X("sassMixing",                        khue_3,         NULL,           NULL);

    x += X("scssSelectorName",                  khue_6_2,       NULL,           NULL);

    // Elixir highlighting------------------------------------------------------
    x += "hi link elixirModuleDefine Define";
    x += X("elixirAlias",                       khue_6_2,       NULL,           NULL);
    x += X("elixirAtom",                        khue_1,         NULL,           NULL);
    x += X("elixirBlockDefinition",             khue_3,         NULL,           NULL);
    x += X("elixirModuleDeclaration",           khue_6,         NULL,           NULL);
    x += X("elixirInclude",                     khue_5,         NULL,           NULL);
    x += X("elixirOperator",                    khue_6,         NULL,           NULL);

    //  Git and git related plugins highlighting--------------------------------
    x += X("gitcommitComment",                  kmono_3,        NULL,           NULL);
    x += X("gitcommitUnmerged",                 khue_4,         NULL,           NULL);
    x += X("gitcommitOnBranch",                 NULL,           NULL,           NULL);
    x += X("gitcommitBranch",                   khue_3,         NULL,           NULL);
    x += X("gitcommitDiscardedType",            khue_5,         NULL,           NULL);
    x += X("gitcommitSelectedType",             khue_4,         NULL,           NULL);
    x += X("gitcommitHeader",                   NULL,           NULL,           NULL);
    x += X("gitcommitUntrackedFile",            khue_1,         NULL,           NULL);
    x += X("gitcommitDiscardedFile",            khue_5,         NULL,           NULL);
    x += X("gitcommitSelectedFile",             khue_4,         NULL,           NULL);
    x += X("gitcommitUnmergedFile",             khue_6_2,       NULL,           NULL);
    x += X("gitcommitFile",                     NULL,           NULL,           NULL);

    x += "hi link gitcommitNoBranch gitcommitBranch";
    x += "hi link gitcommitUntracked gitcommitComment";
    x += "hi link gitcommitDiscarded gitcommitComment";
    x += "hi link gitcommitSelected gitcommitComment";
    x += "hi link gitcommitDiscardedArrow gitcommitDiscardedFile";
    x += "hi link gitcommitSelectedArrow gitcommitSelectedFile";
    x += "hi link gitcommitUnmergedArrow gitcommitUnmergedFile";

    x += X("SignifySignAdd",                    khue_4,         NULL,           NULL);
    x += X("SignifySignChange",                 khue_6_2,       NULL,           NULL);
    x += X("SignifySignDelete",                 khue_5,         NULL,           NULL);

    x += "hi link GitGutterAdd SignifySignAdd";
    x += "hi link GitGutterChange SignifySignChange";
    x += "hi link GitGutterDelete SignifySignDelete";

    x += X("diffAdded",                         khue_4,         NULL,           NULL);
    x += X("diffRemoved",                       khue_5,         NULL,           NULL);

    // Go highlighting ---------------------------------------------------------
    x += X("goDeclaration",                     khue_3,         NULL,           NULL);
    x += X("goField",                           khue_5,         NULL,           NULL);
    x += X("goMethod",                          khue_1,         NULL,           NULL);
    x += X("goType",                            khue_3,         NULL,           NULL);
    x += X("goUnsignedInts",                    khue_1,         NULL,           NULL);

    // Haskell highlighting ----------------------------------------------------
    x += X("haskellDeclKeyword",                khue_2,         NULL,           NULL);
    x += X("haskellType",                       khue_4,         NULL,           NULL);
    x += X("haskellWhere",                      khue_5,         NULL,           NULL);
    x += X("haskellImportKeywords",             khue_2,         NULL,           NULL);
    x += X("haskellOperators",                  khue_5,         NULL,           NULL);
    x += X("haskellDelimiter",                  khue_2,         NULL,           NULL);
    x += X("haskellIdentifier",                 khue_6,         NULL,           NULL);
    x += X("haskellKeyword",                    khue_5,         NULL,           NULL);
    x += X("haskellNumber",                     khue_1,         NULL,           NULL);
    x += X("haskellString",                     khue_1,         NULL,           NULL);

    // HTML highlighting -------------------------------------------------------
    x += X("htmlArg",                           khue_6,         NULL,           NULL);
    x += X("htmlTagName",                       khue_5,         NULL,           NULL);
    x += X("htmlTagN",                          khue_5,         NULL,           NULL);
    x += X("htmlSpecialTagName",                khue_5,         NULL,           NULL);
    x += X("htmlTag",                           kmono_2,        NULL,           NULL);
    x += X("htmlEndTag",                        kmono_2,        NULL,           NULL);

    x += X("MatchTag",                          khue_5,         ksyn_cursor,    "underline,bold");

    // JavaScript highlighting -------------------------------------------------
    x += X("coffeeString",                      khue_4,         NULL,           NULL);

    x += X("javaScriptBraces",                  kmono_2,        NULL,           NULL);
    x += X("javaScriptFunction",                khue_3,         NULL,           NULL);
    x += X("javaScriptIdentifier",              khue_3,         NULL,           NULL);
    x += X("javaScriptNull",                    khue_6,         NULL,           NULL);
    x += X("javaScriptNumber",                  khue_6,         NULL,           NULL);
    x += X("javaScriptRequire",                 khue_1,         NULL,           NULL);
    x += X("javaScriptReserved",                khue_3,         NULL,           NULL);

    // httpk//github.com/pangloss/vim-javascript
    x += X("jsArrowFunction",                   khue_3,         NULL,           NULL);
    x += X("jsBraces",                          kmono_2,        NULL,           NULL);
    x += X("jsClassBraces",                     kmono_2,        NULL,           NULL);
    x += X("jsClassKeywords",                   khue_3,         NULL,           NULL);
    x += X("jsDocParam",                        khue_2,         NULL,           NULL);
    x += X("jsDocTags",                         khue_3,         NULL,           NULL);
    x += X("jsFuncBraces",                      kmono_2,        NULL,           NULL);
    x += X("jsFuncCall",                        khue_2,         NULL,           NULL);
    x += X("jsFuncParens",                      kmono_2,        NULL,           NULL);
    x += X("jsFunction",                        khue_3,         NULL,           NULL);
    x += X("jsGlobalObjects",                   khue_6_2,       NULL,           NULL);
    x += X("jsModuleWords",                     khue_3,         NULL,           NULL);
    x += X("jsModules",                         khue_3,         NULL,           NULL);
    x += X("jsNoise",                           kmono_2,        NULL,           NULL);
    x += X("jsNull",                            khue_6,         NULL,           NULL);
    x += X("jsOperator",                        khue_3,         NULL,           NULL);
    x += X("jsParens",                          kmono_2,        NULL,           NULL);
    x += X("jsStorageClass",                    khue_3,         NULL,           NULL);
    x += X("jsTemplateBraces",                  khue_5_2,       NULL,           NULL);
    x += X("jsTemplateVar",                     khue_4,         NULL,           NULL);
    x += X("jsThis",                            khue_5,         NULL,           NULL);
    x += X("jsUndefined",                       khue_6,         NULL,           NULL);
    x += X("jsObjectValue",                     khue_2,         NULL,           NULL);
    x += X("jsObjectKey",                       khue_1,         NULL,           NULL);
    x += X("jsReturn",                          khue_3,         NULL,           NULL);

    // httpk//github.com/othree/yajs.vim
    x += X("javascriptArrowFunc",               khue_3,         NULL,           NULL);
    x += X("javascriptClassExtends",            khue_3,         NULL,           NULL);
    x += X("javascriptClassKeyword",            khue_3,         NULL,           NULL);
    x += X("javascriptDocNotation",             khue_3,         NULL,           NULL);
    x += X("javascriptDocParamName",            khue_2,         NULL,           NULL);
    x += X("javascriptDocTags",                 khue_3,         NULL,           NULL);
    x += X("javascriptEndColons",               kmono_3,        NULL,           NULL);
    x += X("javascriptExport",                  khue_3,         NULL,           NULL);
    x += X("javascriptFuncArg",                 kmono_1,        NULL,           NULL);
    x += X("javascriptFuncKeyword",             khue_3,         NULL,           NULL);
    x += X("javascriptIdentifier",              khue_5,         NULL,           NULL);
    x += X("javascriptImport",                  khue_3,         NULL,           NULL);
    x += X("javascriptObjectLabel",             kmono_1,        NULL,           NULL);
    x += X("javascriptOpSymbol",                khue_1,         NULL,           NULL);
    x += X("javascriptOpSymbols",               khue_1,         NULL,           NULL);
    x += X("javascriptPropertyName",            khue_4,         NULL,           NULL);
    x += X("javascriptTemplateSB",              khue_5_2,       NULL,           NULL);
    x += X("javascriptVariable",                khue_3,         NULL,           NULL);

    // JSON highlighting -------------------------------------------------------
    x += X("jsonCommentError",                  kmono_1,        NULL,           NULL);
    x += X("jsonKeyword",                       khue_5,         NULL,           NULL);
    x += X("jsonQuote",                         kmono_3,        NULL,           NULL);
    x += X("jsonTrailingCommaError",            khue_5,         NULL,           "reverse");
    x += X("jsonMissingCommaError",             khue_5,         NULL,           "reverse");
    x += X("jsonNoQuotesError",                 khue_5,         NULL,           "reverse");
    x += X("jsonNumError",                      khue_5,         NULL,           "reverse");
    x += X("jsonString",                        khue_4,         NULL,           NULL);
    x += X("jsonBoolean",                       khue_3,         NULL,           NULL);
    x += X("jsonNumber",                        khue_6,         NULL,           NULL);
    x += X("jsonStringSQError",                 khue_5,         NULL,           "reverse");
    x += X("jsonSemicolonError",                khue_5,         NULL,           "reverse");

    // Markdown highlighting ---------------------------------------------------
    x += X("markdownUrl",                       kmono_3,        NULL,           NULL);
    x += X("markdownBold",                      khue_6,         NULL,           "bold");
    x += X("markdownItalic",                    khue_6,         NULL,           "bold");
    x += X("markdownCode",                      khue_4,         NULL,           NULL);
    x += X("markdownCodeBlock",                 khue_5,         NULL,           NULL);
    x += X("markdownCodeDelimiter",             khue_4,         NULL,           NULL);
    x += X("markdownHeadingDelimiter",          khue_5_2,       NULL,           NULL);
    x += X("markdownH1",                        khue_5,         NULL,           NULL);
    x += X("markdownH2",                        khue_5,         NULL,           NULL);
    x += X("markdownH3",                        khue_5,         NULL,           NULL);
    x += X("markdownH3",                        khue_5,         NULL,           NULL);
    x += X("markdownH4",                        khue_5,         NULL,           NULL);
    x += X("markdownH5",                        khue_5,         NULL,           NULL);
    x += X("markdownH6",                        khue_5,         NULL,           NULL);
    x += X("markdownListMarker",                khue_5,         NULL,           NULL);

    // Perl highlighting -------------------------------------------------------
    x += X("perlFunction",                      khue_3,         NULL,           NULL);
    x += X("perlMethod",                        ksyn_fg,        NULL,           NULL);
    x += X("perlPackageConst",                  khue_3,         NULL,           NULL);
    x += X("perlPOD",                           kmono_3,        NULL,           NULL);
    x += X("perlSubName",                       ksyn_fg,        NULL,           NULL);
    x += X("perlSharpBang",                     kmono_3,        NULL,           NULL);
    x += X("perlSpecialString",                 khue_4,         NULL,           NULL);
    x += X("perlVarPlain",                      khue_2,         NULL,           NULL);
    x += X("podCommand",                        kmono_3,        NULL,           NULL);

    // PHP highlighting --------------------------------------------------------
    x += X("phpClass",                          khue_6_2,       NULL,           NULL);
    x += X("phpFunction",                       khue_2,         NULL,           NULL);
    x += X("phpFunctions",                      khue_2,         NULL,           NULL);
    x += X("phpInclude",                        khue_3,         NULL,           NULL);
    x += X("phpKeyword",                        khue_3,         NULL,           NULL);
    x += X("phpParent",                         kmono_3,        NULL,           NULL);
    x += X("phpType",                           khue_3,         NULL,           NULL);
    x += X("phpSuperGlobals",                   khue_5,         NULL,           NULL);

    // Pug (Formerly Jade) highlighting ----------------------------------------
    x += X("pugAttributesDelimiter",            khue_6,         NULL,           NULL);
    x += X("pugClass",                          khue_6,         NULL,           NULL);
    x += X("pugDocType",                        kmono_3,        NULL,           kitalic);
    x += X("pugTag",                            khue_5,         NULL,           NULL);

    // PureScript highlighting -------------------------------------------------
    x += X("purescriptKeyword",                 khue_3,         NULL,           NULL);
    x += X("purescriptModuleName",              ksyn_fg,        NULL,           NULL);
    x += X("purescriptIdentifier",              ksyn_fg,        NULL,           NULL);
    x += X("purescriptType",                    khue_6_2,       NULL,           NULL);
    x += X("purescriptTypeVar",                 khue_5,         NULL,           NULL);
    x += X("purescriptConstructor",             khue_5,         NULL,           NULL);
    x += X("purescriptOperator",                ksyn_fg,        NULL,           NULL);

    // Python highlighting -----------------------------------------------------
    x += X("pythonImport",                      khue_3,         NULL,           NULL);
    x += X("pythonBuiltin",                     khue_1,         NULL,           NULL);
    x += X("pythonStatement",                   khue_3,         NULL,           NULL);
    x += X("pythonParam",                       khue_6,         NULL,           NULL);
    x += X("pythonEscape",                      khue_5,         NULL,           NULL);
    x += X("pythonSelf",                        kmono_2,        NULL,           kitalic);
    x += X("pythonClass",                       khue_2,         NULL,           NULL);
    x += X("pythonOperator",                    khue_3,         NULL,           NULL);
    x += X("pythonEscape",                      khue_5,         NULL,           NULL);
    x += X("pythonFunction",                    khue_2,         NULL,           NULL);
    x += X("pythonKeyword",                     khue_2,         NULL,           NULL);
    x += X("pythonModule",                      khue_3,         NULL,           NULL);
    x += X("pythonStringDelimiter",             khue_4,         NULL,           NULL);
    x += X("pythonSymbol",                      khue_1,         NULL,           NULL);

    // Ruby highlighting -------------------------------------------------------
    x += X("rubyBlock",                         khue_3,         NULL,           NULL);
    x += X("rubyBlockParameter",                khue_5,         NULL,           NULL);
    x += X("rubyBlockParameterList",            khue_5,         NULL,           NULL);
    x += X("rubyCapitalizedMethod",             khue_3,         NULL,           NULL);
    x += X("rubyClass",                         khue_3,         NULL,           NULL);
    x += X("rubyConstant",                      khue_6_2,       NULL,           NULL);
    x += X("rubyControl",                       khue_3,         NULL,           NULL);
    x += X("rubyDefine",                        khue_3,         NULL,           NULL);
    x += X("rubyEscape",                        khue_5,         NULL,           NULL);
    x += X("rubyFunction",                      khue_2,         NULL,           NULL);
    x += X("rubyGlobalVariable",                khue_5,         NULL,           NULL);
    x += X("rubyInclude",                       khue_2,         NULL,           NULL);
    x += X("rubyIncluderubyGlobalVariable",     khue_5,         NULL,           NULL);
    x += X("rubyInstanceVariable",              khue_5,         NULL,           NULL);
    x += X("rubyInterpolation",                 khue_1,         NULL,           NULL);
    x += X("rubyInterpolationDelimiter",        khue_5,         NULL,           NULL);
    x += X("rubyKeyword",                       khue_2,         NULL,           NULL);
    x += X("rubyModule",                        khue_3,         NULL,           NULL);
    x += X("rubyPseudoVariable",                khue_5,         NULL,           NULL);
    x += X("rubyRegexp",                        khue_1,         NULL,           NULL);
    x += X("rubyRegexpDelimiter",               khue_1,         NULL,           NULL);
    x += X("rubyStringDelimiter",               khue_4,         NULL,           NULL);
    x += X("rubySymbol",                        khue_1,         NULL,           NULL);

    // Spelling highlighting ---------------------------------------------------
    x += X("SpellBad",                          NULL,           ksyn_bg,        "undercurl");
    x += X("SpellLocal",                        NULL,           ksyn_bg,        "undercurl");
    x += X("SpellCap",                          NULL,           ksyn_bg,        "undercurl");
    x += X("SpellRare",                         NULL,           ksyn_bg,        "undercurl");

    // Vim highlighting --------------------------------------------------------
    x += X("vimCommand",                        khue_3,         NULL,           NULL);
    x += X("vimCommentTitle",                   kmono_3,        NULL,           "bold");
    x += X("vimFunction",                       khue_1,         NULL,           NULL);
    x += X("vimFuncName",                       khue_3,         NULL,           NULL);
    x += X("vimHighlight",                      khue_2,         NULL,           NULL);
    x += X("vimLineComment",                    kmono_3,        NULL,           kitalic);
    x += X("vimParenSep",                       kmono_2,        NULL,           NULL);
    x += X("vimSep",                            kmono_2,        NULL,           NULL);
    x += X("vimUserFunc",                       khue_1,         NULL,           NULL);
    x += X("vimVar",                            khue_5,         NULL,           NULL);

    //XML highlighting --------------------------------------------------------
    x += X("xmlAttrib",                         khue_6_2,       NULL,           NULL);
    x += X("xmlEndTag",                         khue_5,         NULL,           NULL);
    x += X("xmlTag",                            khue_5,         NULL,           NULL);
    x += X("xmlTagName",                        khue_5,         NULL,           NULL);

    // ZSH highlighting --------------------------------------------------------
    x += X("zshCommands",                       ksyn_fg,        NULL,           NULL);
    x += X("zshDeref",                          khue_5,         NULL,           NULL);
    x += X("zshShortDeref",                     khue_5,         NULL,           NULL);
    x += X("zshFunction",                       khue_1,         NULL,           NULL);
    x += X("zshKeyword",                        khue_3,         NULL,           NULL);
    x += X("zshSubst",                          khue_5,         NULL,           NULL);
    x += X("zshSubstDelim",                     kmono_3,        NULL,           NULL);
    x += X("zshTypes",                          khue_3,         NULL,           NULL);
    x += X("zshVariableDef",                    khue_6,         NULL,           NULL);

    // Rust highlighting -------------------------------------------------------
    x += X("rustExternCrate",                   khue_5,         NULL,           "bold");
    x += X("rustIdentifier",                    khue_2,         NULL,           NULL);
    x += X("rustDeriveTrait",                   khue_4,         NULL,           NULL);
    x += X("SpecialComment",                    kmono_3,        NULL,           NULL);
    x += X("rustCommentLine",                   kmono_3,        NULL,           NULL);
    x += X("rustCommentLineDoc",                kmono_3,        NULL,           NULL);
    x += X("rustCommentLineDocError",           kmono_3,        NULL,           NULL);
    x += X("rustCommentBlock",                  kmono_3,        NULL,           NULL);
    x += X("rustCommentBlockDoc",               kmono_3,        NULL,           NULL);
    x += X("rustCommentBlockDocError",          kmono_3,        NULL,           NULL);

    // man highlighting --------------------------------------------------------
    x += "hi link manTitle String";
    x += X("manFooter",                         kmono_3,        NULL,           NULL);

    // ALE (Asynchronous Lint Engine) highlighting -----------------------------
    x += X("ALEWarningSign",                    khue_6_2,       NULL,           NULL);
    x += X("ALEErrorSign",                      khue_5,         NULL,           NULL);

    // Neovim NERDTree Background fix ------------------------------------------
    x += X("NERDTreeFile",                      ksyn_fg,        NULL,           NULL);

    vim_colorscheme(scheme, 0, NULL, x, gui_running);
}


int
colorscheme_one(~list args)
{
    string scheme = "one";
    int colordepth = -1, dark = -1, italic = 1;

    if (! is_null(args)) {
        /* options */
        const list longoptions = {
            "colors:d",
            "mode:s",
            "dark",
            "light",
            "italic:b",
            };
        string value;
        int optidx = 0, ch;

        if ((ch = getopt(value, NULL, longoptions, args, scheme)) >= 0) {
            do {
                ++optidx;
                switch(ch) {
                case 0: // colordepth
                    colordepth = atoi(value);
                    break;
                case 1: // mode
                    if (value == "default") {   /* dynamic dark or light */
                        dark = -1;
                    } else {
                        dark = (value == "dark" ? 1 : 0);
                    }
                    break;
                case 2: // dark
                    dark = 1;
                    break;
                case 3: // light
                    dark = 0;
                    break;
                case 4: // italic
                    italic = value;
                    break;
                default:
                    error("%s: %s", scheme, value);
                    return -1;
                }
            } while ((ch = getopt(value)) >= 0);
        }

        if (optidx < length_of_list(args)) {
            error("%s: invalid option <%s>", scheme, args[optidx]);
            return -1;
        }
    }

    if (colordepth <= 0) {
         get_term_feature(TF_COLORDEPTH, colordepth);
    }
    if (colordepth < 88) {
        error("%s: color depth not supported", scheme);
        return -1;
    }

    if (dark == -1) {
        get_term_feature(TF_SCHEMEDARK, dark);  /* default */
    }

    one(scheme, dark, italic);
    return 0;
}

//end
