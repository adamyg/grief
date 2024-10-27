/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: cscheme.cr,v 1.5 2024/10/27 06:09:44 cvsuser Exp $
 * base16 based colorscheme loader, GriefEdit port - experimental.
 *
 * Original Source: https://github.com/chriskempson/base16-vim
 *
 *    Base16 aims to group similar language constructs with a single colour.
 *    For example, floats, ints, and doubles would belong to the same colour group.
 *    The colours for the default theme were chosen to be easily separable, but scheme designers should
 *    pick whichever colours they desire, e.g. base0B (green by default) could be replaced with red.
 *    There are, however, some general guidelines below that stipulate which base0B should be used
 *    to highlight each construct when designing templates for editors.
 *
 *    Since describing syntax highlighting can be tricky, please see base16-vim and base16-textmate for reference.
 *    Though it should be noted that each editor will have some discrepancies due the fact that editors
 *    generally have different syntax highlighting engines.
 *
 *    Colours base00 to base07 are typically variations of a shade and run from darkest to lightest.
 *    These colours are used for foreground and background, status bars, line highlighting and such.
 *    colours base08 to base0F are typically individual colours used for types, operators, names and variables.
 *    In order to create a dark theme, colours base00 to base07 should span from dark to light.
 *    For a light theme, these colours should span from light to dark.
 *
 *       base00 - Default Background
 *       base01 - Lighter Background (Used for status bars, line number and folding marks)
 *       base02 - Selection Background
 *       base03 - Comments, Invisibles, Line Highlighting
 *       base04 - Dark Foreground (Used for status bars)
 *       base05 - Default Foreground, Caret, Delimiters, Operators
 *       base06 - Light Foreground (Not often used)
 *       base07 - Light Background (Not often used)
 *       base08 - Variables, XML Tags, Markup Link Text, Markup Lists, Diff Deleted
 *       base09 - Integers, Boolean, Constants, XML Attributes, Markup Link Url
 *       base0A - Classes, Markup Bold, Search Text Background
 *       base0B - Strings, Inherited Class, Markup Code, Diff Inserted
 *       base0C - Support, Regular Expressions, Escape Characters, Markup Quotes
 *       base0D - Functions, Methods, Attribute IDs, Headings
 *       base0E - Keywords, Storage, Selector, Markup Italic, Diff Changed
 *       base0F - Deprecated, Opening/Closing Embedded Language Tags, e.g. <?php ?>
 *
 *   Hexadecimal color values should not be preceded by a "#".
 */

#include "../../grief.h"
#include "../util/rgbmap.h"

extern list base16_schemes();

void
main(void)
{
    require("colors/util/rgbmap");
    require("colors/base16/schemes");
    module("base16");
}


// Highlight function
static string
hi(string group, string guifg, string guibg, string ctermfg, string ctermbg, string attr, string guisp)
{
    extern int truecolor;
    string s;

    s = "hi " + group;
    if (truecolor) {
        if (guifg) s += " guifg=#" + guifg;
        if (guibg) s += " guibg=#" + guibg;
        if (attr)  s += " gui=" + attr;

    } else {
        if (ctermfg) s += " ctermfg=" + ctermfg;
        if (ctermbg) s += " ctermbg=" + ctermbg;
        if (attr) s += " cterm=" + attr;
    }
    UNUSED(guisp);
    return s;
}


static void
build_dictionary(list colors, int s)
{
    extern int colordepth;

    // GUI color definitions
    s.gui00 = colors[0x00];
    s.gui01 = colors[0x01];
    s.gui02 = colors[0x02];
    s.gui03 = colors[0x03];
    s.gui04 = colors[0x04];
    s.gui05 = colors[0x05];
    s.gui06 = colors[0x06];
    s.gui07 = colors[0x07];
    s.gui08 = colors[0x08];
    s.gui09 = colors[0x09];
    s.gui0A = colors[0x0A];
    s.gui0B = colors[0x0B];
    s.gui0C = colors[0x0C];
    s.gui0D = colors[0x0D];
    s.gui0E = colors[0x0E];
    s.gui0F = colors[0x0F];

    // Terminal color definitions
    if (colordepth >= 16) {
        s.cterm00     = "0";
        s.cterm03     = "8";
        s.cterm05     = "7";
        s.cterm07     = "15";
        s.cterm08     = "1";
        s.cterm0A     = "3";
        s.cterm0B     = "2";
        s.cterm0C     = "6";
        s.cterm0D     = "4";
        s.cterm0E     = "5";
        if (colordepth > 16) {
            s.cterm01 = "18";
            s.cterm02 = "19";
            s.cterm04 = "20";
            s.cterm06 = "21";
            s.cterm09 = "16";
            s.cterm0F = "17";
        } else {
            s.cterm01 = "10";
            s.cterm02 = "11";
            s.cterm04 = "12";
            s.cterm06 = "13";
            s.cterm09 = "9";
            s.cterm0F = "14";
        }
    } else {
        s.cterm00 = RGBMap(colors[0x00], colordepth);
        s.cterm01 = RGBMap(colors[0x01], colordepth);
        s.cterm02 = RGBMap(colors[0x02], colordepth);
        s.cterm03 = RGBMap(colors[0x03], colordepth);
        s.cterm04 = RGBMap(colors[0x04], colordepth);
        s.cterm05 = RGBMap(colors[0x05], colordepth);
        s.cterm06 = RGBMap(colors[0x06], colordepth);
        s.cterm07 = RGBMap(colors[0x07], colordepth);
        s.cterm08 = RGBMap(colors[0x08], colordepth);
        s.cterm09 = RGBMap(colors[0x09], colordepth);
        s.cterm0A = RGBMap(colors[0x0A], colordepth);
        s.cterm0B = RGBMap(colors[0x0B], colordepth);
        s.cterm0C = RGBMap(colors[0x0C], colordepth);
        s.cterm0D = RGBMap(colors[0x0D], colordepth);
        s.cterm0E = RGBMap(colors[0x0E], colordepth);
        s.cterm0F = RGBMap(colors[0x0F], colordepth);
    }
}


static void
base16_generate(string scheme, list colors)
{
    int s = create_dictionary();
    list x;

    build_dictionary(colors, s);

    x += "hi clear";
    x += "syntax reset";

    // Vim editor colors
    x += hi("Normal",                   s.gui05, s.gui00, s.cterm05, s.cterm00, "", "");
    x += hi("Bold",                     "", "", "", "", "bold", "");
    x += hi("Debug",                    s.gui08, "", s.cterm08, "", "", "");
    x += hi("Directory",                s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("Error",                    s.gui00, s.gui08, s.cterm00, s.cterm08, "", "");
    x += hi("ErrorMsg",                 s.gui08, s.gui00, s.cterm08, s.cterm00, "", "");
    x += hi("Exception",                s.gui08, "", s.cterm08, "", "", "");
    x += hi("FoldColumn",               s.gui0C, s.gui01, s.cterm0C, s.cterm01, "", "");
    x += hi("Folded",                   s.gui03, s.gui01, s.cterm03, s.cterm01, "", "");
    x += hi("IncSearch",                s.gui01, s.gui09, s.cterm01, s.cterm09, "none", "");
    x += hi("Italic",                   "", "", "", "", "none", "");
    x += hi("Macro",                    s.gui08, "", s.cterm08, "", "", "");
    x += hi("MatchParen",               "", s.gui03, "", s.cterm03,  "", "");
    x += hi("ModeMsg",                  s.gui0B, "", s.cterm0B, "", "", "");
    x += hi("MoreMsg",                  s.gui0B, "", s.cterm0B, "", "", "");
    x += hi("Question",                 s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("Search",                   s.gui01, s.gui0A, s.cterm01, s.cterm0A,  "", "");
    x += hi("Substitute",               s.gui01, s.gui0A, s.cterm01, s.cterm0A, "none", "");
    x += hi("SpecialKey",               s.gui03, "", s.cterm03, "", "", "");
    x += hi("TooLong",                  s.gui08, "", s.cterm08, "", "", "");
    x += hi("Underlined",               s.gui08, "", s.cterm08, "", "", "");
    x += hi("Visual",                   "", s.gui02, "", s.cterm02, "", "");
    x += hi("VisualNOS",                s.gui08, "", s.cterm08, "", "", "");
    x += hi("WarningMsg",               s.gui08, "", s.cterm08, "", "", "");
    x += hi("WildMenu",                 s.gui08, s.gui0A, s.cterm08, "", "", "");
    x += hi("Title",                    s.gui0D, "", s.cterm0D, "", "none", "");
    x += hi("Conceal",                  s.gui0D, s.gui00, s.cterm0D, s.cterm00, "", "");
    x += hi("Cursor",                   s.gui00, s.gui05, s.cterm00, s.cterm05, "", "");
    x += hi("NonText",                  s.gui03, "", s.cterm03, "", "", "");
    x += hi("LineNr",                   s.gui03, s.gui01, s.cterm03, s.cterm01, "", "");
    x += hi("SignColumn",               s.gui03, s.gui01, s.cterm03, s.cterm01, "", "");
    x += hi("StatusLine",               s.gui04, s.gui02, s.cterm04, s.cterm02, "none", "");
    x += hi("StatusLineNC",             s.gui03, s.gui01, s.cterm03, s.cterm01, "none", "");
    x += hi("VertSplit",                s.gui02, s.gui02, s.cterm02, s.cterm02, "none", "");
    x += hi("ColorColumn",              "", s.gui01, "", s.cterm01, "none", "");
    x += hi("CursorColumn",             "", s.gui01, "", s.cterm01, "none", "");
    x += hi("CursorLine",               "", s.gui01, "", s.cterm01, "none", "");
    x += hi("CursorLineNr",             s.gui04, s.gui01, s.cterm04, s.cterm01, "", "");
    x += hi("QuickFixLine",             "", s.gui01, "", s.cterm01, "none", "");
    x += hi("PMenu",                    s.gui05, s.gui01, s.cterm05, s.cterm01, "none", "");
    x += hi("PMenuSel",                 s.gui01, s.gui05, s.cterm01, s.cterm05, "", "");
    x += hi("TabLine",                  s.gui03, s.gui01, s.cterm03, s.cterm01, "none", "");
    x += hi("TabLineFill",              s.gui03, s.gui01, s.cterm03, s.cterm01, "none", "");
    x += hi("TabLineSel",               s.gui0B, s.gui01, s.cterm0B, s.cterm01, "none", "");

    // Standard syntax highlighting
    x += hi("Boolean",                  s.gui09, "", s.cterm09, "", "", "");
    x += hi("Character",                s.gui08, "", s.cterm08, "", "", "");
    x += hi("Comment",                  s.gui03, "", s.cterm03, "", "", "");
    x += hi("Conditional",              s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("Constant",                 s.gui09, "", s.cterm09, "", "", "");
    x += hi("Define",                   s.gui0E, "", s.cterm0E, "", "none", "");
    x += hi("Delimiter",                s.gui0F, "", s.cterm0F, "", "", "");
    x += hi("Float",                    s.gui09, "", s.cterm09, "", "", "");
    x += hi("Function",                 s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("Identifier",               s.gui08, "", s.cterm08, "", "none", "");
    x += hi("Include",                  s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("Keyword",                  s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("Label",                    s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("Number",                   s.gui09, "", s.cterm09, "", "", "");
    x += hi("Operator",                 s.gui05, "", s.cterm05, "", "none", "");
    x += hi("PreProc",                  s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("Repeat",                   s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("Special",                  s.gui0C, "", s.cterm0C, "", "", "");
    x += hi("SpecialChar",              s.gui0F, "", s.cterm0F, "", "", "");
    x += hi("Statement",                s.gui08, "", s.cterm08, "", "", "");
    x += hi("StorageClass",             s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("String",                   s.gui0B, "", s.cterm0B, "", "", "");
    x += hi("Structure",                s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("Tag",                      s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("Todo",                     s.gui0A, s.gui01, s.cterm0A, s.cterm01, "", "");
    x += hi("Type",                     s.gui0A, "", s.cterm0A, "", "none", "");
    x += hi("Typedef",                  s.gui0A, "", s.cterm0A, "", "", "");

    // C highlighting
    x += hi("cOperator",                s.gui0C, "", s.cterm0C, "", "", "");
    x += hi("cPreCondit",               s.gui0E, "", s.cterm0E, "", "", "");

    // C# highlighting
    x += hi("csClass",                  s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("csAttribute",              s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("csModifier",               s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("csType",                   s.gui08, "", s.cterm08, "", "", "");
    x += hi("csUnspecifiedStatement",   s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("csContextualStatement",    s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("csNewDecleration",         s.gui08, "", s.cterm08, "", "", "");

    // CSS highlighting
    x += hi("cssBraces",                s.gui05, "", s.cterm05, "", "", "");
    x += hi("cssClassName",             s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("cssColor",                 s.gui0C, "", s.cterm0C, "", "", "");

    // Diff highlighting
    x += hi("DiffAdd",                  s.gui0B, s.gui01,  s.cterm0B, s.cterm01, "", "");
    x += hi("DiffChange",               s.gui03, s.gui01,  s.cterm03, s.cterm01, "", "");
    x += hi("DiffDelete",               s.gui08, s.gui01,  s.cterm08, s.cterm01, "", "");
    x += hi("DiffText",                 s.gui0D, s.gui01,  s.cterm0D, s.cterm01, "", "");
    x += hi("DiffAdded",                s.gui0B, s.gui00,  s.cterm0B, s.cterm00, "", "");
    x += hi("DiffFile",                 s.gui08, s.gui00,  s.cterm08, s.cterm00, "", "");
    x += hi("DiffNewFile",              s.gui0B, s.gui00,  s.cterm0B, s.cterm00, "", "");
    x += hi("DiffLine",                 s.gui0D, s.gui00,  s.cterm0D, s.cterm00, "", "");
    x += hi("DiffRemoved",              s.gui08, s.gui00,  s.cterm08, s.cterm00, "", "");

    // Git highlighting
    x += hi("gitcommitOverflow",        s.gui08, "", s.cterm08, "", "", "");
    x += hi("gitcommitSummary",         s.gui0B, "", s.cterm0B, "", "", "");
    x += hi("gitcommitComment",         s.gui03, "", s.cterm03, "", "", "");
    x += hi("gitcommitUntracked",       s.gui03, "", s.cterm03, "", "", "");
    x += hi("gitcommitDiscarded",       s.gui03, "", s.cterm03, "", "", "");
    x += hi("gitcommitSelected",        s.gui03, "", s.cterm03, "", "", "");
    x += hi("gitcommitHeader",          s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("gitcommitSelectedType",    s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("gitcommitUnmergedType",    s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("gitcommitDiscardedType",   s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("gitcommitBranch",          s.gui09, "", s.cterm09, "", "bold", "");
    x += hi("gitcommitUntrackedFile",   s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("gitcommitUnmergedFile",    s.gui08, "", s.cterm08, "", "bold", "");
    x += hi("gitcommitDiscardedFile",   s.gui08, "", s.cterm08, "", "bold", "");
    x += hi("gitcommitSelectedFile",    s.gui0B, "", s.cterm0B, "", "bold", "");

    // GitGutter highlighting
    x += hi("GitGutterAdd",             s.gui0B, s.gui01, s.cterm0B, s.cterm01, "", "");
    x += hi("GitGutterChange",          s.gui0D, s.gui01, s.cterm0D, s.cterm01, "", "");
    x += hi("GitGutterDelete",          s.gui08, s.gui01, s.cterm08, s.cterm01, "", "");
    x += hi("GitGutterChangeDelete",    s.gui0E, s.gui01, s.cterm0E, s.cterm01, "", "");

    // HTML highlighting
    x += hi("htmlBold",                 s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("htmlItalic",               s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("htmlEndTag",               s.gui05, "", s.cterm05, "", "", "");
    x += hi("htmlTag",                  s.gui05, "", s.cterm05, "", "", "");

    // JavaScript highlighting
    x += hi("javaScript",               s.gui05, "", s.cterm05, "", "", "");
    x += hi("javaScriptBraces",         s.gui05, "", s.cterm05, "", "", "");
    x += hi("javaScriptNumber",         s.gui09, "", s.cterm09, "", "", "");
    // pangloss/vim-javascript highlighting
    x += hi("jsOperator",               s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("jsStatement",              s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("jsReturn",                 s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("jsThis",                   s.gui08, "", s.cterm08, "", "", "");
    x += hi("jsClassDefinition",        s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("jsFunction",               s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("jsFuncName",               s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("jsFuncCall",               s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("jsClassFuncName",          s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("jsClassMethodType",        s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("jsRegexpString",           s.gui0C, "", s.cterm0C, "", "", "");
    x += hi("jsGlobalObjects",          s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("jsGlobalNodeObjects",      s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("jsExceptions",             s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("jsBuiltins",               s.gui0A, "", s.cterm0A, "", "", "");

    // LSP highlighting
    x += hi("LspDiagnosticsDefaultError", s.gui08, "", s.cterm08, "", "", "");
    x += hi("LspDiagnosticsDefaultWarning", s.gui09, "", s.cterm09, "", "", "");
    x += hi("LspDiagnosticsDefaultHnformation", s.gui05, "", s.cterm05, "", "", "");
    x += hi("LspDiagnosticsDefaultHint", s.gui03, "", s.cterm03, "", "", "");

    // Mail highlighting
    x += hi("mailQuoted1",              s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("mailQuoted2",              s.gui0B, "", s.cterm0B, "", "", "");
    x += hi("mailQuoted3",              s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("mailQuoted4",              s.gui0C, "", s.cterm0C, "", "", "");
    x += hi("mailQuoted5",              s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("mailQuoted6",              s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("mailURL",                  s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("mailEmail",                s.gui0D, "", s.cterm0D, "", "", "");

    // Markdown highlighting
    x += hi("markdownCode",             s.gui0B, "", s.cterm0B, "", "", "");
    x += hi("markdownError",            s.gui05, s.gui00, s.cterm05, s.cterm00, "", "");
    x += hi("markdownCodeBlock",        s.gui0B, "", s.cterm0B, "", "", "");
    x += hi("markdownHeadingDelimiter", s.gui0D, "", s.cterm0D, "", "", "");

    // NERDTree highlighting
    x += hi("NERDTreeDirSlash",         s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("NERDTreeExecFile",         s.gui05, "", s.cterm05, "", "", "");

    // PHP highlighting
    x += hi("phpMemberSelector",        s.gui05, "", s.cterm05, "", "", "");
    x += hi("phpComparison",            s.gui05, "", s.cterm05, "", "", "");
    x += hi("phpParent",                s.gui05, "", s.cterm05, "", "", "");
    x += hi("phpMethodsVar",            s.gui0C, "", s.cterm0C, "", "", "");

    // Python highlighting
    x += hi("pythonOperator",           s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("pythonRepeat",             s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("pythonInclude",            s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("pythonStatement",          s.gui0E, "", s.cterm0E, "", "", "");

    // Ruby highlighting
    x += hi("rubyAttribute",            s.gui0D, "", s.cterm0D, "", "", "");
    x += hi("rubyConstant",             s.gui0A, "", s.cterm0A, "", "", "");
    x += hi("rubyInterpolationDelimiter", s.gui0F, "", s.cterm0F, "", "", "");
    x += hi("rubyRegexp",               s.gui0C, "", s.cterm0C, "", "", "");
    x += hi("rubySymbol",               s.gui0B, "", s.cterm0B, "", "", "");
    x += hi("rubyStringDelimiter",      s.gui0B, "", s.cterm0B, "", "", "");

    // SASS highlighting
    x += hi("sassidChar",               s.gui08, "", s.cterm08, "", "", "");
    x += hi("sassClassChar",            s.gui09, "", s.cterm09, "", "", "");
    x += hi("sassInclude",              s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("sassMixing",               s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("sassMixinName",            s.gui0D, "", s.cterm0D, "", "", "");

    // Signify highlighting
    x += hi("SignifySignAdd",           s.gui0B, s.gui01, s.cterm0B, s.cterm01, "", "");
    x += hi("SignifySignChange",        s.gui0D, s.gui01, s.cterm0D, s.cterm01, "", "");
    x += hi("SignifySignDelete",        s.gui08, s.gui01, s.cterm08, s.cterm01, "", "");

    // Spelling highlighting
    x += hi("SpellBad",                 "", "", "", "", "undercurl", s.gui08);
    x += hi("SpellLocal",               "", "", "", "", "undercurl", s.gui0C);
    x += hi("SpellCap",                 "", "", "", "", "undercurl", s.gui0D);
    x += hi("SpellRare",                "", "", "", "", "undercurl", s.gui0E);

    // Startify highlighting
    x += hi("StartifyBracket",          s.gui03, "", s.cterm03, "", "", "");
    x += hi("StartifyFile",             s.gui07, "", s.cterm07, "", "", "");
    x += hi("StartifyFooter",           s.gui03, "", s.cterm03, "", "", "");
    x += hi("StartifyHeader",           s.gui0B, "", s.cterm0B, "", "", "");
    x += hi("StartifyNumber",           s.gui09, "", s.cterm09, "", "", "");
    x += hi("StartifyPath",             s.gui03, "", s.cterm03, "", "", "");
    x += hi("StartifySection",          s.gui0E, "", s.cterm0E, "", "", "");
    x += hi("StartifySelect",           s.gui0C, "", s.cterm0C, "", "", "");
    x += hi("StartifySlash",            s.gui03, "", s.cterm03, "", "", "");
    x += hi("StartifySpecial",          s.gui03, "", s.cterm03, "", "", "");

    // Java highlighting
    x += hi("javaOperator",             s.gui0D, "", s.cterm0D, "", "", "");

    vim_colorscheme(scheme, 0, NULL, x, -1);
    delete_dictionary(s);
}


/*
 *  base16_yaml ---
 *      Load a yaml formatter base16 color scheme.
 *
 *  Scheme Files:
 *      Scheme files have the following structure:
 *
 *       scheme: "Scheme Name"
 *       author: "Scheme Author"
 *       base00: "000000"
 *       base01: "111111"
 *       base02: "222222"
 *       base03: "333333"
 *       base04: "444444"
 *       base05: "555555"
 *       base06: "666666"
 *       base07: "777777"
 *       base08: "888888"
 *       base09: "999999"
 *       base0A: "aaaaaa"
 *       base0B: "bbbbbb"
 *       base0C: "cccccc"
 *       base0D: "dddddd"
 *       base0E: "eeeeee"
 *       base0F: "ffffff"
 */
static int
base16_yaml(string scheme)
{
    int buf, curbuf = inq_buffer();
    string name, author;
    list colors;

    // Load yaml
    message("loading: %s", scheme);
    if ((buf = create_buffer("-colorscheme-base16-", scheme, TRUE)) < 0) {
        return -1;
    }
    set_buffer(buf);
    top_of_buffer();
    do {
        string ln, data;
        int idx;

        ln = compress(read(), TRUE);            // line, compress white-space and trimmed.
        if (ln) {
            if (2 == sscanf(ln, "base%x: \"%[0-9a-fA-F]\"", idx, data)) {
                if (idx < 16) {                 // RRGGBB
                    colors[idx] = data;
                }
            } else {
                if (2 == sscanf(ln, "scheme: \"%[^\"]\"", data)) {
                    name = data;                // scheme name
                } else if (2 == sscanf(ln, "author: \"%[^\"]\"", data)) {
                    author = data;              // details of authors
                }
            }
        }
    } while (down());
    set_buffer(curbuf);
    delete_buffer(buf);

    // Apply
    if (strlen(name) && length_of_list(colors) == 16) {
        message("applying: base16 %s (%s)", name, author);
        base16_generate(format("base16-%s", scheme), colors);
        return 0;
    }
    return -1;
}


/*
 *  base16_scheme ---
 *      Load a base16 color scheme.
 *
 *  Scheme List:
 *
 *      list = {
 *          "scheme", "Scheme Name",
 *          "author", "Scheme Author",
 *          "00", 0x000000,
 *          "01", 0x111111,
 *          "02", 0x222222,
 *          "03", 0x333333,
 *          "04", 0x444444,
 *          "05", 0x555555,
 *          "06", 0x666666,
 *          "07", 0x777777,
 *          "08", 0x888888,
 *          "09", 0x999999,
 *          "0A", 0xaaaaaa,
 *          "0B", 0xbbbbbb,
 *          "0C", 0xcccccc,
 *          "0D", 0xdddddd,
 *          "0E", 0xeeeeee,
 *          "0F", 0xffffff
 *      }
 */
static int
base16_scheme(string scheme)
{
    string function;
    declare def;
    string name, author;
    list colors;
    int idx, i;

    // Load scheme definition
    function = inq_module() + "::def_" + scheme;
    def = execute_macro(function);
    if (is_list(def)) {
        for (i = 0; i < length_of_list(def); i += 2) {
            string tag = def[i];                // tag[0], value[1]

            if (tag == "scheme") {
                name = def[i + 1];              // scheme name
            } else if (tag == "author") {
                author = def[i + 1];            // details of authors
            } else {
                if (1 == sscanf(tag, "%x", idx)) {
                    if (idx < 16) {             // RRGGBB
                        colors[idx] = format("%06x", def[i + 1]);
                    }
                }
            }
        }
    }

    // Apply
    if (strlen(name) && length_of_list(colors) == 16) {
        message("applying: base16 %s (%s)", name, author);
        base16_generate(format("base16-%s", scheme), colors);
        return 0;
    }
    return 0;
}


string
colorschemepkg_base16(~string scheme, ~list args)
{
    string package = "base16";
    int colordepth = -1, truecolor = 0, dark = -1;

    if (! is_null(args)) {
        /* options */
        const list longoptions = {
            "colors.d",
            "mode:s",
            "dark",
            "light"
            };
        string value;
        int optidx = 0, ch;

        if ((ch = getopt(value, NULL, longoptions, args, package)) >= 0) {
            do {
                ++optidx;
                switch(ch) {
                case 0: // colordepth
                    if (value == "truecolor") {
                        colordepth = 256;
                        truecolor = 1;
                    } else {
                        colordepth = atoi(value);
                        truecolor = 0;
                    }
                    break;
                case 1: // mode
                    if (value == "default") {   // dynamic dark or light
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
                default:
                    error("%s: %s", package, value);
                    return "";
                }
            } while ((ch = getopt(value)) >= 0);
        }

        if (optidx < length_of_list(args)) {
            error("%s: invalid option <%s>", package, args[optidx]);
            return "";
        }
    }

    if (colordepth <= 0) {
         get_term_feature(TF_COLORDEPTH, colordepth);
         get_term_feature(TF_TRUECOLOR, truecolor);
    }

    if (colordepth < 8) {
        error("%s: color depth not supported", package);
        return "";
    }

    if (dark == -1) {
        get_term_feature(TF_SCHEMEDARK, dark);  // default
    }

    // Resolve package-name

    if (scheme == "") {                         // prompt selection
        list schemes = base16_schemes();
        const int sel = select_list(package, "Select Scheme", -2, schemes, SEL_NORMAL);

        if (sel <= 0) {
            return "";
        }
        scheme = schemes[((sel - 1) * 2) + 1];
    }

    // Apply scheme

    int ext = strrstr(scheme, ".yaml");         // "xxxx.yaml"
    if (ext && ext == (strlen(scheme) - 4)) {
        if (-1 == base16_yaml(scheme)) {
            error("%s: unable to load scheme '%s'", package, scheme);
            return "";
        }
    } else {
        if (0 != base16_scheme(scheme)) {
            error("%s: unavailable to locate '%s'", package, scheme);
            return "";
        }
    }
    return "base16-" + scheme;
}

//end

