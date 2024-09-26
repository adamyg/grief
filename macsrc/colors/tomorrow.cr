/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: tomorrow.cr,v 1.3 2024/08/04 11:42:44 cvsuser Exp $
 * Tomorrow coloriser, GriefEdit port - beta.
 *
 * Original:
 *  Name: Tomorrow.vim
 *  https://github.com/chriskempson
 */

#include "../grief.h"
#include "rgbmap.h"


void
main(void)
{
        require("colors/rgbmap");
}


static string
HI(string group, string fg, string bg, string at)
{
        extern int colordepth;
        string val;

        val = "hi " + group;
        if (fg != "") val += " guifg=#" + fg + " ctermfg=" + RGBMap(fg, colordepth);
        if (bg != "") val += " guibg=#" + bg + " ctermbg=" + RGBMap(bg, colordepth);
        if (at != "") val += " gui=" + at + " cterm=" + at;
        return val;
}


void
colorscheme_tomorrow(void)
{
        const string scheme = "Tomorrow";

        string foreground = "4d4d4c";
        string background = "fafafa";
        string selection  = "d6d6d6";
        string line       = "efefef";
        string comment    = "8e908c";
        string red        = "c82829";
        string orange     = "f5871f";
        string yellow     = "eab700";
        string green      = "718c00";
        string aqua       = "3e999f";
        string blue       = "4271ae";
        string purple     = "8959a8";
        string window     = "efefef";

        list colors[] = {
                "set background=dark",
                "hi clear",
                "syntax reset"
                };
        int colordepth;

        // Depth >= 88
        get_term_feature(TF_COLORDEPTH, colordepth);
        if (colordepth < 88) {
                error("%s: color depth not supported", scheme);
                return;
        }

        // Vim Highlighting
        colors += HI("Normal", foreground, background, "");
        colors += "highlight LineNr term=bold cterm=NONE ctermfg=DarkGrey ctermbg=NONE gui=NONE guifg=DarkGrey guibg=NONE";
        colors += HI("NonText", selection, "", "");
        colors += HI("SpecialKey", selection, "", "");
        colors += HI("Search", foreground, yellow, "");
        colors += HI("TabLine", foreground, background, "reverse");
        colors += HI("StatusLine", window, yellow, "reverse");
        colors += HI("StatusLineNC", window, foreground, "reverse");
        colors += HI("VertSplit", window, window, "none");
        colors += HI("Visual", "", selection, "");
        colors += HI("Directory", blue, "", "");
        colors += HI("ModeMsg", green, "", "");
        colors += HI("MoreMsg", green, "", "");
        colors += HI("Question", green, "", "");
        colors += HI("WarningMsg", red, "", "");
        colors += HI("MatchParen", "", selection, "");
        colors += HI("Folded", comment, background, "");
        colors += HI("FoldColumn", comment, background, "");
//      if version >= 700
                colors += HI("CursorLine", "", line, "none");
                colors += HI("CursorColumn", "", line, "none");
                colors += HI("PMenu", foreground, selection, "none");
                colors += HI("PMenuSel", foreground, selection, "reverse");
                colors += HI("SignColumn", "", background, "none");
//      end
//      if version >= 703
                colors += HI("ColorColumn", "", line, "none");
//      end

        // Standard Highlighting
        colors += HI("Comment", comment, "", "");
        colors += HI("Todo", comment, background, "");
        colors += HI("Title", comment, "", "");
        colors += HI("Identifier", red, "", "none");
        colors += HI("Statement", foreground, "", "");
        colors += HI("Conditional", foreground, "", "");
        colors += HI("Repeat", foreground, "", "");
        colors += HI("Structure", purple, "", "");
        colors += HI("Function", blue, "", "");
        colors += HI("Constant", orange, "", "");
        colors += HI("String", green, "", "");
        colors += HI("Special", foreground, "", "");
        colors += HI("PreProc", purple, "", "");
        colors += HI("Operator", aqua, "", "none");
        colors += HI("Type", blue, "", "none");
        colors += HI("Define", purple, "", "none");
        colors += HI("Include", blue, "", "");
      //colors += HI("Ignore", "666666", "", "");

        // Vim Highlighting
        colors += HI("vimCommand", red, "", "none");

        // C Highlighting
        colors += HI("cType", yellow, "", "");
        colors += HI("cStorageClass", purple, "", "");
        colors += HI("cConditional", purple, "", "");
        colors += HI("cRepeat", purple, "", "");

        // PHP Highlighting
        colors += HI("phpVarSelector", red, "", "");
        colors += HI("phpKeyword", purple, "", "");
        colors += HI("phpRepeat", purple, "", "");
        colors += HI("phpConditional", purple, "", "");
        colors += HI("phpStatement", purple, "", "");
        colors += HI("phpMemberSelector", foreground, "", "");

        // Ruby Highlighting
        colors += HI("rubySymbol", green, "", "");
        colors += HI("rubyConstant", yellow, "", "");
        colors += HI("rubyAttribute", blue, "", "");
        colors += HI("rubyInclude", blue, "", "");
        colors += HI("rubyLocalVariableOrMethod", orange, "", "");
        colors += HI("rubyCurlyBlock", orange, "", "");
        colors += HI("rubyStringDelimiter", green, "", "");
        colors += HI("rubyInterpolationDelimiter", orange, "", "");
        colors += HI("rubyConditional", purple, "", "");
        colors += HI("rubyRepeat", purple, "", "");

        // Python Highlighting
        colors += HI("pythonInclude", purple, "", "");
        colors += HI("pythonStatement", purple, "", "");
        colors += HI("pythonConditional", purple, "", "");
        colors += HI("pythonRepeat", purple, "", "");
        colors += HI("pythonException", purple, "", "");
        colors += HI("pythonFunction", blue, "", "");

        // Go Highlighting
        colors += HI("goStatement", purple, "", "");
        colors += HI("goConditional", purple, "", "");
        colors += HI("goRepeat", purple, "", "");
        colors += HI("goException", purple, "", "");
        colors += HI("goDeclaration", blue, "", "");
        colors += HI("goConstants", yellow, "", "");
        colors += HI("goBuiltins", orange, "", "");

        // CoffeeScript Highlighting
        colors += HI("coffeeKeyword", purple, "", "");
        colors += HI("coffeeConditional", purple, "", "");

        // JavaScript Highlighting
        colors += HI("javaScriptBraces", foreground, "", "");
        colors += HI("javaScriptFunction", purple, "", "");
        colors += HI("javaScriptConditional", purple, "", "");
        colors += HI("javaScriptRepeat", purple, "", "");
        colors += HI("javaScriptNumber", orange, "", "");
        colors += HI("javaScriptMember", orange, "", "");

        // HTML Highlighting
        colors += HI("htmlTag", red, "", "");
        colors += HI("htmlTagName", red, "", "");
        colors += HI("htmlArg", red, "", "");
        colors += HI("htmlScriptTag", red, "", "");

        // Diff Highlighting
        colors += HI("diffAdded", green, "", "");
        colors += HI("diffRemoved", red, "", "");

        vim_colorscheme(scheme, 0, NULL, colors, -1);
}

//end
