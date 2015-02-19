/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: jellybeans.cr,v 1.2 2014/11/27 15:54:13 ayoung Exp $
 * jellybean coloriser, GRIEF port.
 *
 * Original:
 *    Vim color file
 *
 *         __       _ _       _
 *         \ \  ___| | |_   _| |__   ___  __ _ _ __  ___
 *          \ \/ _ \ | | | | |  _ \ / _ \/ _  |  _ \/ __|
 *       /\_/ /  __/ | | |_| | |_| |  __/ |_| | | | \__ \
 *       \___/ \___|_|_|\__  |____/ \___|\____|_| |_|___/
 *                       \___/
 *
 *          "A colorful, dark color scheme for Vim."
 *
 *    File:         jellybeans.vim
 *    URL:          github.com/nanotech/jellybeans.vim
 *    Scripts URL:  vim.org/scripts/script.php?script_id=2555
 *    Maintainer:   NanoTech (nanotech.nanotechcorp.net)
 *    Version:      1.6~git
 *    Last Change:  January 15th, 2012
 *    License:      MIT
 *    Contributors: Daniel Herbert (pocketninja)
 *                  Henry So, Jr. <henryso@panix.com>
 *                  David Liang <bmdavll at gmail dot com>
 *                  Rich Healey (richo)
 *                  Andrew Wong (w0ng)
 *
 *    Copyright (c) 2009-2012 NanoTech
 *
 *    Permission is hereby granted, free of charge, to any per-
 *    son obtaining a copy of this software and associated doc-
 *    umentation  files  (the “Software”), to deal in the Soft-
 *    ware without restriction,  including  without  limitation
 *    the rights to use, copy, modify, merge, publish, distrib-
 *    ute, sublicense, and/or sell copies of the Software,  and
 *    to permit persons to whom the Software is furnished to do
 *    so, subject to the following conditions:
 *
 *    The above copyright notice  and  this  permission  notice
 *    shall  be  included in all copies or substantial portions
 *    of the Software.
 *
 *    THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY
 *    KIND,  EXPRESS  OR  IMPLIED, INCLUDING BUT NOT LIMITED TO
 *    THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICU‐
 *    LAR  PURPOSE  AND  NONINFRINGEMENT. IN NO EVENT SHALL THE
 *    AUTHORS OR COPYRIGHT HOLDERS BE  LIABLE  FOR  ANY  CLAIM,
 *    DAMAGES  OR OTHER LIABILITY, WHETHER IN AN ACTION OF CON‐
 *    TRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CON‐
 *    NECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *    THE SOFTWARE.
 */

#include "../grief.h"

static string   jellybeans_background_color     = "151515";
static int      jellybeans_use_lowcolor_black   = 1;
static int      jellybeans_background_color_256 = 233;


// Returns the palette index to approximate the 'rrggbb' hex string
static string
RGB(string rgb)
{
        return format("#%s", rgb);              // utilise GRIEF's internal rgb mapping
}


// Sets the highlighting for the given group
static string
X(string group, string fg, string bg, string attr, string lcfg, string lcbg)
{
        extern int low_color;
        string histring = "hi " + group + " ";

        // colors
        if (low_color) {
                int fge = ("" == lcfg);
                int bge = ("" == lcbg);

                if (!fge && !bge) {
                        histring += " ctermfg="+lcfg+" ctermbg="+lcbg;

                } else if (!fge && bge) {
                        histring += " ctermfg="+lcfg+" ctermbg=NONE";

                } else if (fge && !bge) {
                        histring += " ctermfg=NONE ctermbg="+lcbg;
                }
        } else {
                int fge = ("" == fg);
                int bge = ("" == bg);

                if (!fge && !bge) {
                        histring += " guifg=#"+fg+" guibg=#"+bg+" ctermfg="+RGB(fg)+" ctermbg="+RGB(bg);

                } else if (!fge && bge) {
                        histring += " guifg=#"+fg+" guibg=NONE ctermfg="+RGB(fg)+" ctermbg=NONE";

                } else if (fge && !bge) {
                        histring += " guifg=NONE guibg=#"+bg+" ctermfg=NONE ctermbg="+RGB(bg);
                }
        }

        // attribute
        if (attr == "") {
                histring += " gui=none cterm=none";
        } else {
                string noitalic = re_translate(NULL, "italic", "", attr);
                if ("" == noitalic) {
                        noitalic = "none";
                }
                histring += " gui="+attr+" cterm="+noitalic;
        }

        return histring;
}


void
colorscheme_jellybeans(void)
{
        int colordepth, gui_running = 0;        // TODO: TF_GUI
        list colors = {
                "set background=dark",
                "hi clear"
                };

        // depth >= 88
        get_term_feature(TF_COLORDEPTH, colordepth);
        if (colordepth < 88) {
                error("jellybeans, color depth not supported");
                return;
        }

        // scheme
        int low_color;
        string termBlack;

        if (gui_running || colordepth == 88 || colordepth == 256) {
                low_color = 0;                  // gui
        } else {
                low_color = 1;                  // cterm
        }

        if (0 == jellybeans_use_lowcolor_black) {
                termBlack = "Grey";
        } else {
                termBlack = "Black";
        }

        colors += X("Normal","e8e8d3",jellybeans_background_color,"","White","");
        colors += X("CursorLine","","1c1c1c","","",termBlack);
        colors += X("CursorColumn","","1c1c1c","","",termBlack);
        colors += X("MatchParen","ffffff","556779","bold","","DarkCyan");

        colors += X("TabLine","000000","b0b8c0","italic","",termBlack);
        colors += X("TabLineFill","9098a0","","","",termBlack);
        colors += X("TabLineSel","000000","f0f0f0","italic,bold",termBlack,"White");

        // " Auto-completion
        colors += X("Pmenu","ffffff","606060","","White",termBlack);
        colors += X("PmenuSel","101010","eeeeee","",termBlack,"White");

        colors += X("Visual","","404040","","",termBlack);
        colors += X("Cursor",jellybeans_background_color,"b0d0f0","","","");

        colors += X("LineNr","605958",jellybeans_background_color,"none",termBlack,"");
        colors += X("CursorLineNr","ccc5c4","","none","White","");
        colors += X("Comment","888888","","italic","Grey","");
        colors += X("Todo","c7c7c7","","bold","White",termBlack);

        colors += X("StatusLine","000000","dddddd","italic","","White");
        colors += X("StatusLineNC","ffffff","403c41","italic","White","Black");
        colors += X("VertSplit","777777","403c41","",termBlack,termBlack);
        colors += X("WildMenu","f0a0c0","302028","","Magenta","");

        colors += X("Folded","a0a8b0","384048","italic",termBlack,"");
        colors += X("FoldColumn","535D66","1f1f1f","","",termBlack);
        colors += X("SignColumn","777777","333333","","",termBlack);
        colors += X("ColorColumn","","000000","","",termBlack);

        colors += X("Title","70b950","","bold","Green","");

        colors += X("Constant","cf6a4c","","","Red","");
        colors += X("Special","799d6a","","","Green","");
        colors += X("Delimiter","668799","","","Grey","");

        colors += X("String","99ad6a","","","Green","");
        colors += X("StringDelimiter","556633","","","DarkGreen","");

        colors += X("Identifier","c6b6ee","","","LightCyan","");
        colors += X("Structure","8fbfdc","","","LightCyan","");
        colors += X("Function","fad07a","","","Yellow","");
        colors += X("Statement","8197bf","","","DarkBlue","");
        colors += X("PreProc","8fbfdc","","","LightBlue","");

        colors += "hi! link Operator Structure";

        colors += X("Type","ffb964","","","Yellow","");
        colors += X("NonText","606060",jellybeans_background_color,"",termBlack,"");

        colors += X("SpecialKey","444444","1c1c1c","",termBlack,"");

        colors += X("Search","f0a0c0","302028","underline","Magenta","");

        colors += X("Directory","dad085","","","Yellow","");
        colors += X("ErrorMsg","","902020","","","DarkRed");
        colors += "hi! link Error ErrorMsg";
        colors += "hi! link MoreMsg Special";
        colors += X("Question","65C254","","","Green","");

        // " Spell Checking

        colors += X("SpellBad","","902020","underline","","DarkRed");
        colors += X("SpellCap","","0000df","underline","","Blue");
        colors += X("SpellRare","","540063","underline","","DarkMagenta");
        colors += X("SpellLocal","","2D7067","underline","","Green");

        // " Diff

        colors += "hi! link diffRemoved Constant";
        colors += "hi! link diffAdded String";

        // " VimDiff

        colors += X("DiffAdd","D2EBBE","437019","","White","DarkGreen");
        colors += X("DiffDelete","40000A","700009","","DarkRed","DarkRed");
        colors += X("DiffChange","","2B5B77","","White","DarkBlue");
        colors += X("DiffText","8fbfdc","000000","reverse","Yellow","");

        // " PHP

        colors += "hi! link phpFunctions Function";
        colors += X("StorageClass","c59f6f","","","Red","");
        colors += "hi! link phpSuperglobal Identifier";
        colors += "hi! link phpQuoteSingle StringDelimiter";
        colors += "hi! link phpQuoteDouble StringDelimiter";
        colors += "hi! link phpBoolean Constant";
        colors += "hi! link phpNull Constant";
        colors += "hi! link phpArrayPair Operator";
        colors += "hi! link phpOperator Normal";
        colors += "hi! link phpRelation Normal";
        colors += "hi! link phpVarSelector Identifier";

        // " Python

        colors += "hi! link pythonOperator Statement";

        // " Ruby

        colors += "hi! link rubySharpBang Comment";
        colors += X("rubyClass","447799","","","DarkBlue","");
        colors += X("rubyIdentifier","c6b6fe","","","Cyan","");
        colors += "hi! link rubyConstant Type";
        colors += "hi! link rubyFunction Function";

        colors += X("rubyInstanceVariable","c6b6fe","","","Cyan","");
        colors += X("rubySymbol","7697d6","","","Blue","");
        colors += "hi! link rubyGlobalVariable rubyInstanceVariable";
        colors += "hi! link rubyModule rubyClass";
        colors += X("rubyControl","7597c6","","","Blue","");

        colors += "hi! link rubyString String";
        colors += "hi! link rubyStringDelimiter StringDelimiter";
        colors += "hi! link rubyInterpolationDelimiter Identifier";

        colors += X("rubyRegexpDelimiter","540063","","","Magenta","");
        colors += X("rubyRegexp","dd0093","","","DarkMagenta","");
        colors += X("rubyRegexpSpecial","a40073","","","Magenta","");

        colors += X("rubyPredefinedIdentifier","de5577","","","Red","");

        // " Erlang

        colors += "hi! link erlangAtom rubySymbol";
        colors += "hi! link erlangBIF rubyPredefinedIdentifier";
        colors += "hi! link erlangFunction rubyPredefinedIdentifier";
        colors += "hi! link erlangDirective Statement";
        colors += "hi! link erlangNode Identifier";

        // " JavaScript

        colors += "hi! link javaScriptValue Constant";
        colors += "hi! link javaScriptRegexpString rubyRegexp";

        // " CoffeeScript

        colors += "hi! link coffeeRegExp javaScriptRegexpString";

        // " Lua

        colors += "hi! link luaOperator Conditional";

        // " C

        colors += "hi! link cFormat Identifier";
        colors += "hi! link cOperator Constant";

        // " Objective-C/Cocoa

        colors += "hi! link objcClass Type";
        colors += "hi! link cocoaClass objcClass";
        colors += "hi! link objcSubclass objcClass";
        colors += "hi! link objcSuperclass objcClass";
        colors += "hi! link objcDirective rubyClass";
        colors += "hi! link objcStatement Constant";
        colors += "hi! link cocoaFunction Function";
        colors += "hi! link objcMethodName Identifier";
        colors += "hi! link objcMethodArg Normal";
        colors += "hi! link objcMessageName Identifier";

        // " Vimscript

        colors += "hi! link vimOper Normal";

        // " Debugger.vim

        colors += X("DbgCurrent","DEEBFE","345FA8","","White","DarkBlue");
        colors += X("DbgBreakPt","","4F0037","","","DarkMagenta");

        // " vim-indent-guides

        colors += X("IndentGuidesOdd","","232323","","","");
        colors += X("IndentGuidesEven","","1b1b1b","","","");

        // " Plugins, etc.

        colors += "hi! link TagListFileName Directory";
        colors += X("PreciseJumpTarget","B9ED67","405026","","White","Green");

        // " Manual overrides for 256-color terminals. Dark colors auto-map badly.
        if (0 == low_color) {
                colors += "hi StatusLineNC ctermbg=235";
                colors += "hi Folded ctermbg=236";
                colors += "hi FoldColumn ctermbg=234";
                colors += "hi SignColumn ctermbg=236";
                colors += "hi CursorColumn ctermbg=234";
                colors += "hi CursorLine ctermbg=234";
                colors += "hi SpecialKey ctermbg=234";
                colors += "hi NonText ctermbg="+jellybeans_background_color_256;
                colors += "hi LineNr ctermbg="+jellybeans_background_color_256;
                colors += "hi DiffText ctermfg=81";
                colors += "hi Normal ctermbg="+jellybeans_background_color_256;
                colors += "hi DbgBreakPt ctermbg=53";
                colors += "hi IndentGuidesOdd ctermbg=235";
                colors += "hi IndentGuidesEven ctermbg=234";
        }

        vim_colorscheme("jellybeans", 0, NULL, colors, (!low_color ? TRUE : FALSE));
}
/*end*/

