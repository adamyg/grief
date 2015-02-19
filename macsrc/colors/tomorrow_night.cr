/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: tomorrow_night.cr,v 1.2 2014/11/25 04:44:50 ayoung Exp $
 * Tomorrow_Night coloriser, GRIEF port.
 *
 * Original:
 *  Name: Tomorrow_Night.vim
 *  Maintainer: http://chriskempson.com
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
colorscheme_tomorrow_night(void)
{
        string foreground = "c5c8c6";
        string background = "1d1f21";
        string selection  = "373b41";
        string line       = "282a2e";
        string comment    = "969896";
        string red        = "cc6666";
        string orange     = "de935f";
        string yellow     = "f0c674";
        string green      = "b5bd68";
        string aqua       = "8abeb7";
        string blue       = "81a2be";
        string purple     = "b294bb";
        string window     = "4d5057";
        list   colors[] = {
            "set background=dark",
            "hi clear"
            };
        int colordepth;

        // Depth >= 88
        get_term_feature(TF_COLORDEPTH, colordepth);
        if (colordepth < 88) {
                error("Tomorrow-Night, color depth not supported");
                return;
        }
        if (colordepth <= 256) {
                background = "303030";
                window     = "5e5e5e";
                line       = "3a3a3a";
                selection  = "585858";
        }

        // Vim Highlighting
        colors += HI("Normal", foreground, background, "");
        colors += HI("LineNr", selection, "", "");
        colors += HI("NonText", selection, "", "");
        colors += HI("SpecialKey", selection, "", "");
        colors += HI("Search", background, yellow, "");
        colors += HI("TabLine", window, foreground, "reverse");
        colors += HI("TabLineFill", window, foreground, "reverse");
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
        colors += HI("FoldColumn", "", background, "");
        colors += HI("CursorLine", "", line, "none");
        colors += HI("CursorColumn", "", line, "none");
        colors += HI("PMenu", foreground, selection, "none");
        colors += HI("PMenuSel", foreground, selection, "reverse");
        colors += HI("SignColumn", "", background, "none");
        colors += HI("ColorColumn", "", line, "none");

        //  Standard Highlighting
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

        //  Vim Highlighting
        colors += HI("vimCommand", red, "", "none");

        //  C Highlighting
        colors += HI("cType", yellow, "", "");
        colors += HI("cStorageClass", purple, "", "");
        colors += HI("cConditional", purple, "", "");
        colors += HI("cRepeat", purple, "", "");

        //  PHP Highlighting
        colors += HI("phpVarSelector", red, "", "");
        colors += HI("phpKeyword", purple, "", "");
        colors += HI("phpRepeat", purple, "", "");
        colors += HI("phpConditional", purple, "", "");
        colors += HI("phpStatement", purple, "", "");
        colors += HI("phpMemberSelector", foreground, "", "");

        //  Ruby Highlighting
        colors += HI("rubySymbol", green, "", "");
        colors += HI("rubyConstant", yellow, "", "");
        colors += HI("rubyAccess", yellow, "", "");
        colors += HI("rubyAttribute", blue, "", "");
        colors += HI("rubyInclude", blue, "", "");
        colors += HI("rubyLocalVariableOrMethod", orange, "", "");
        colors += HI("rubyCurlyBlock", orange, "", "");
        colors += HI("rubyStringDelimiter", green, "", "");
        colors += HI("rubyInterpolationDelimiter", orange, "", "");
        colors += HI("rubyConditional", purple, "", "");
        colors += HI("rubyRepeat", purple, "", "");
        colors += HI("rubyControl", purple, "", "");
        colors += HI("rubyException", purple, "", "");

        //  Python Highlighting
        colors += HI("pythonInclude", purple, "", "");
        colors += HI("pythonStatement", purple, "", "");
        colors += HI("pythonConditional", purple, "", "");
        colors += HI("pythonRepeat", purple, "", "");
        colors += HI("pythonException", purple, "", "");
        colors += HI("pythonFunction", blue, "", "");
        colors += HI("pythonPreCondit", purple, "", "");
        colors += HI("pythonRepeat", aqua, "", "");
        colors += HI("pythonExClass", orange, "", "");

        //  JavaScript Highlighting
        colors += HI("javaScriptBraces", foreground, "", "");
        colors += HI("javaScriptFunction", purple, "", "");
        colors += HI("javaScriptConditional", purple, "", "");
        colors += HI("javaScriptRepeat", purple, "", "");
        colors += HI("javaScriptNumber", orange, "", "");
        colors += HI("javaScriptMember", orange, "", "");
        colors += HI("javascriptNull", orange, "", "");
        colors += HI("javascriptGlobal", blue, "", "");
        colors += HI("javascriptStatement", red, "", "");

        //  HTML Highlighting
        colors += HI("htmlTag", red, "", "");
        colors += HI("htmlTagName", red, "", "");
        colors += HI("htmlArg", red, "", "");
        colors += HI("htmlScriptTag", red, "", "");

        //  Diff Highlighting
        colors += HI("diffAdded", green, "", "");
        colors += HI("diffRemoved", red, "", "");

        //  ShowMarks Highlighting
        colors += HI("ShowMarksHLl", orange, background, "none");
        colors += HI("ShowMarksHLo", purple, background, "none");
        colors += HI("ShowMarksHLu", yellow, background, "none");
        colors += HI("ShowMarksHLm", aqua, background, "none");

        //  Cucumber Highlighting
        colors += HI("cucumberGiven", blue, "", "");
        colors += HI("cucumberGivenAnd", blue, "", "");

        //  Go Highlighting
        colors += HI("goDirective", purple, "", "");
        colors += HI("goDeclaration", purple, "", "");
        colors += HI("goStatement", purple, "", "");
        colors += HI("goConditional", purple, "", "");
        colors += HI("goConstants", orange, "", "");
        colors += HI("goTodo", yellow, "", "");
        colors += HI("goDeclType", blue, "", "");
        colors += HI("goBuiltins", purple, "", "");

        //  Lua Highlighting
        colors += HI("luaStatement", purple, "", "");
        colors += HI("luaRepeat", purple, "", "");
        colors += HI("luaCondStart", purple, "", "");
        colors += HI("luaCondElseif", purple, "", "");
        colors += HI("luaCond", purple, "", "");
        colors += HI("luaCondEnd", purple, "", "");

        //  Clojure highlighting
        colors += HI("clojureConstant", orange, "", "");
        colors += HI("clojureBoolean", orange, "", "");
        colors += HI("clojureCharacter", orange, "", "");
        colors += HI("clojureKeyword", green, "", "");
        colors += HI("clojureNumber", orange, "", "");
        colors += HI("clojureString", green, "", "");
        colors += HI("clojureRegexp", green, "", "");
        colors += HI("clojureParen", aqua, "", "");
        colors += HI("clojureVariable", yellow, "", "");
        colors += HI("clojureCond", blue, "", "");
        colors += HI("clojureDefine", purple, "", "");
        colors += HI("clojureException", red, "", "");
        colors += HI("clojureFunc", blue, "", "");
        colors += HI("clojureMacro", blue, "", "");
        colors += HI("clojureRepeat", blue, "", "");
        colors += HI("clojureSpecial", purple, "", "");
        colors += HI("clojureQuote", blue, "", "");
        colors += HI("clojureUnquote", blue, "", "");
        colors += HI("clojureMeta", blue, "", "");
        colors += HI("clojureDeref", blue, "", "");
        colors += HI("clojureAnonArg", blue, "", "");
        colors += HI("clojureRepeat", blue, "", "");
        colors += HI("clojureDispatch", blue, "", "");

        //  Scala highlighting
        colors += HI("scalaKeyword", purple, "", "");
        colors += HI("scalaKeywordModifier", purple, "", "");
        colors += HI("scalaOperator", blue, "", "");
        colors += HI("scalaPackage", red, "", "");
        colors += HI("scalaFqn", foreground, "", "");
        colors += HI("scalaFqnSet", foreground, "", "");
        colors += HI("scalaImport", purple, "", "");
        colors += HI("scalaBoolean", orange, "", "");
        colors += HI("scalaDef", purple, "", "");
        colors += HI("scalaVal", purple, "", "");
        colors += HI("scalaVar", aqua, "", "");
        colors += HI("scalaClass", purple, "", "");
        colors += HI("scalaObject", purple, "", "");
        colors += HI("scalaTrait", purple, "", "");
        colors += HI("scalaDefName", blue, "", "");
        colors += HI("scalaValName", foreground, "", "");
        colors += HI("scalaVarName", foreground, "", "");
        colors += HI("scalaClassName", foreground, "", "");
        colors += HI("scalaType", yellow, "", "");
        colors += HI("scalaTypeSpecializer", yellow, "", "");
        colors += HI("scalaAnnotation", orange, "", "");
        colors += HI("scalaNumber", orange, "", "");
        colors += HI("scalaDefSpecializer", yellow, "", "");
        colors += HI("scalaClassSpecializer", yellow, "", "");
        colors += HI("scalaBackTick", green, "", "");
        colors += HI("scalaRoot", foreground, "", "");
        colors += HI("scalaMethodCall", blue, "", "");
        colors += HI("scalaCaseType", yellow, "", "");
        colors += HI("scalaLineComment", comment, "", "");
        colors += HI("scalaComment", comment, "", "");
        colors += HI("scalaDocComment", comment, "", "");
        colors += HI("scalaDocTags", comment, "", "");
        colors += HI("scalaEmptyString", green, "", "");
        colors += HI("scalaMultiLineString", green, "", "");
        colors += HI("scalaUnicode", orange, "", "");
        colors += HI("scalaString", green, "", "");
        colors += HI("scalaStringEscape", green, "", "");
        colors += HI("scalaSymbol", orange, "", "");
        colors += HI("scalaChar", orange, "", "");
        colors += HI("scalaXml", green, "", "");
        colors += HI("scalaConstructorSpecializer", yellow, "", "");
        colors += HI("scalaBackTick", blue, "", "");

        vim_colorscheme("Tomorrow-Night", 0, NULL, colors, -1);
}
/*end*/

