/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: inkpot.cr,v 1.2 2014/11/27 15:54:12 ayoung Exp $
 * inkpot coloriser, GRIEF port.
 *
 * Original:
 *    Name:       inkpot.vim
 *    Maintainer: Ciaran McCreesh <ciaran.mccreesh@googlemail.com>
 *    Homepage:   http://github.com/ciaranm/inkpot/
 *
 *    This should work in the GUI, rxvt-unicode (88 colour mode) and xterm (256
 *    colour mode). It won't work in 8/16 colour terminals.
 */

#include "../grief.h"

// To use a black background, inkpot_black_background = 1;
int inkpot_black_background = 0;


// map a urxvt cube number to an xterm-256 cube number
static int
M(int a)
{
//  return strpart("0135", a, 1) + 0;
    switch(a) {
    case 0: return 0;
    case 1: return 1;
    case 2: return 3;
    case 3: return 5;
    }
    return 0;
}


// map a urxvt colour to an xterm-256 colour
static string
X(int a)
{
    extern int colordepth;

    if (colordepth == 88) {
        return a;

    } else {
        if (a == 8) {
            return 237;

        } else if (a < 16) {
            return a;
        } else if (a > 79) {
            return 232 + (3 * (a - 80));
        } else {
            int b =  a - 16;
            int x =  b % 4;
            int y = (b / 4) % 4;
            int z = (b / 16);

            return 16 + M(x) + (6 * M(y)) + (36 * M(z));
        }
    }
}


void
colorscheme_inkpot(void)
{
        list colors = {
                "set background=dark",
                "hi clear"
                };
        int colordepth;

        // Depth >= 88
        get_term_feature(TF_COLORDEPTH, colordepth);
        if (colordepth < 88) {
                error("inkpot, color depth not supported");
                return;
        }

        // Select scheme
        if (colordepth > 256) {

            if (0 == inkpot_black_background) {
                colors += "hi Normal         gui=NONE   guifg=#cfbfad   guibg=#1e1e27";
            } else {
                colors += "hi Normal         gui=NONE   guifg=#cfbfad   guibg=#000000";
            }

                colors += "hi CursorLine                                guibg=#2e2e37";

                colors += "hi IncSearch      gui=BOLD   guifg=#303030   guibg=#cd8b60";
                colors += "hi Search         gui=NONE   guifg=#303030   guibg=#ad7b57";
                colors += "hi ErrorMsg       gui=BOLD   guifg=#ffffff   guibg=#ce4e4e";
                colors += "hi WarningMsg     gui=BOLD   guifg=#ffffff   guibg=#ce8e4e";
                colors += "hi ModeMsg        gui=BOLD   guifg=#7e7eae   guibg=NONE";
                colors += "hi MoreMsg        gui=BOLD   guifg=#7e7eae   guibg=NONE";
                colors += "hi Question       gui=BOLD   guifg=#ffcd00   guibg=NONE";

                colors += "hi StatusLine     gui=BOLD   guifg=#b9b9b9   guibg=#3e3e5e";
                colors += "hi User1          gui=BOLD   guifg=#00ff8b   guibg=#3e3e5e";
                colors += "hi User2          gui=BOLD   guifg=#7070a0   guibg=#3e3e5e";
                colors += "hi StatusLineNC   gui=NONE   guifg=#b9b9b9   guibg=#3e3e5e";
                colors += "hi VertSplit      gui=NONE   guifg=#b9b9b9   guibg=#3e3e5e";

                colors += "hi WildMenu       gui=BOLD   guifg=#eeeeee   guibg=#6e6eaf";

                colors += "hi MBENormal                 guifg=#cfbfad   guibg=#2e2e3f";
                colors += "hi MBEChanged                guifg=#eeeeee   guibg=#2e2e3f";
                colors += "hi MBEVisibleNormal          guifg=#cfcfcd   guibg=#4e4e8f";
                colors += "hi MBEVisibleChanged         guifg=#eeeeee   guibg=#4e4e8f";

                colors += "hi DiffText       gui=NONE   guifg=#ffffcd   guibg=#4a2a4a";
                colors += "hi DiffChange     gui=NONE   guifg=#ffffcd   guibg=#306b8f";
                colors += "hi DiffDelete     gui=NONE   guifg=#ffffcd   guibg=#6d3030";
                colors += "hi DiffAdd        gui=NONE   guifg=#ffffcd   guibg=#306d30";

                colors += "hi Cursor         gui=NONE   guifg=#404040   guibg=#8b8bff";
                colors += "hi lCursor        gui=NONE   guifg=#404040   guibg=#8fff8b";
                colors += "hi CursorIM       gui=NONE   guifg=#404040   guibg=#8b8bff";

                colors += "hi Folded         gui=NONE   guifg=#cfcfcd   guibg=#4b208f";
                colors += "hi FoldColumn     gui=NONE   guifg=#8b8bcd   guibg=#2e2e2e";

                colors += "hi Directory      gui=NONE   guifg=#00ff8b   guibg=NONE";
                colors += "hi LineNr         gui=NONE   guifg=#8b8bcd   guibg=#2e2e2e";
                colors += "hi NonText        gui=BOLD   guifg=#8b8bcd   guibg=NONE";
                colors += "hi SpecialKey     gui=BOLD   guifg=#ab60ed   guibg=NONE";
                colors += "hi Title          gui=BOLD   guifg=#af4f4b   guibg=NONE";
                colors += "hi Visual         gui=NONE   guifg=#eeeeee   guibg=#4e4e8f";

                colors += "hi Comment        gui=NONE   guifg=#cd8b00   guibg=NONE";
                colors += "hi Constant       gui=NONE   guifg=#ffcd8b   guibg=NONE";
                colors += "hi String         gui=NONE   guifg=#ffcd8b   guibg=#404040";
                colors += "hi Error          gui=NONE   guifg=#ffffff   guibg=#6e2e2e";
                colors += "hi Identifier     gui=NONE   guifg=#ff8bff   guibg=NONE";
                colors += "hi Ignore         gui=NONE";
                colors += "hi Number         gui=NONE   guifg=#f0ad6d   guibg=NONE";
                colors += "hi PreProc        gui=NONE   guifg=#409090   guibg=NONE";
                colors += "hi Special        gui=NONE   guifg=#c080d0   guibg=NONE";
                colors += "hi SpecialChar    gui=NONE   guifg=#c080d0   guibg=#404040";
                colors += "hi Statement      gui=NONE   guifg=#808bed   guibg=NONE";
                colors += "hi Todo           gui=BOLD   guifg=#303030   guibg=#d0a060";
                colors += "hi Type           gui=NONE   guifg=#ff8bff   guibg=NONE";
                colors += "hi Underlined     gui=BOLD   guifg=#df9f2d   guibg=NONE";
                colors += "hi TaglistTagName gui=BOLD   guifg=#808bed   guibg=NONE";

                colors += "hi perlSpecialMatch   gui=NONE guifg=#c080d0   guibg=#404040";
                colors += "hi perlSpecialString  gui=NONE guifg=#c080d0   guibg=#404040";

                colors += "hi cSpecialCharacter  gui=NONE guifg=#c080d0   guibg=#404040";
                colors += "hi cFormat            gui=NONE guifg=#c080d0   guibg=#404040";

                colors += "hi doxygenBrief                 gui=NONE guifg=#fdab60   guibg=NONE";
                colors += "hi doxygenParam                 gui=NONE guifg=#fdd090   guibg=NONE";
                colors += "hi doxygenPrev                  gui=NONE guifg=#fdd090   guibg=NONE";
                colors += "hi doxygenSmallSpecial          gui=NONE guifg=#fdd090   guibg=NONE";
                colors += "hi doxygenSpecial               gui=NONE guifg=#fdd090   guibg=NONE";
                colors += "hi doxygenComment               gui=NONE guifg=#ad7b20   guibg=NONE";
                colors += "hi doxygenSpecial               gui=NONE guifg=#fdab60   guibg=NONE";
                colors += "hi doxygenSpecialMultilineDesc  gui=NONE guifg=#ad600b   guibg=NONE";
                colors += "hi doxygenSpecialOnelineDesc    gui=NONE guifg=#ad600b   guibg=NONE";

                colors += "hi Pmenu          gui=NONE   guifg=#eeeeee   guibg=#4e4e8f";
                colors += "hi PmenuSel       gui=BOLD   guifg=#eeeeee   guibg=#2e2e3f";
                colors += "hi PmenuSbar      gui=BOLD   guifg=#eeeeee   guibg=#6e6eaf";
                colors += "hi PmenuThumb     gui=BOLD   guifg=#eeeeee   guibg=#6e6eaf";

                colors += "hi SpellBad       gui=undercurl guisp=#cc6666";
                colors += "hi SpellRare      gui=undercurl guisp=#cc66cc";
                colors += "hi SpellLocal     gui=undercurl guisp=#cccc66";
                colors += "hi SpellCap       gui=undercurl guisp=#66cccc";

                colors += "hi MatchParen     gui=NONE   guifg=#cfbfad   guibg=#4e4e8f";

        } else {

            if (0 == inkpot_black_background) {
                colors += "hi Normal         cterm=NONE   ctermfg=" + X(79) + " ctermbg=" + X(80);
            } else {
                colors += "hi Normal         cterm=NONE   ctermfg=" + X(79) + " ctermbg=" + X(16);
            }

                colors += "hi IncSearch      cterm=BOLD   ctermfg=" + X(80) + " ctermbg=" + X(73);
                colors += "hi Search         cterm=NONE   ctermfg=" + X(80) + " ctermbg=" + X(52);
                colors += "hi ErrorMsg       cterm=BOLD   ctermfg=" + X(16) + " ctermbg=" + X(48);
                colors += "hi WarningMsg     cterm=BOLD   ctermfg=" + X(16) + " ctermbg=" + X(68);
                colors += "hi ModeMsg        cterm=BOLD   ctermfg=" + X(38) + " ctermbg=" + "NONE";
                colors += "hi MoreMsg        cterm=BOLD   ctermfg=" + X(38) + " ctermbg=" + "NONE";
                colors += "hi Question       cterm=BOLD   ctermfg=" + X(52) + " ctermbg=" + "NONE";

                colors += "hi StatusLine     cterm=BOLD   ctermfg=" + X(85) + " ctermbg=" + X(81);
                colors += "hi User1          cterm=BOLD   ctermfg=" + X(28) + " ctermbg=" + X(81);
                colors += "hi User2          cterm=BOLD   ctermfg=" + X(39) + " ctermbg=" + X(81);
                colors += "hi StatusLineNC   cterm=NONE   ctermfg=" + X(84) + " ctermbg=" + X(81);
                colors += "hi VertSplit      cterm=NONE   ctermfg=" + X(84) + " ctermbg=" + X(81);

                colors += "hi WildMenu       cterm=BOLD   ctermfg=" + X(87) + " ctermbg=" + X(38);

                colors += "hi MBENormal                   ctermfg=" + X(85) + " ctermbg=" + X(81);
                colors += "hi MBEChanged                  ctermfg=" + X(87) + " ctermbg=" + X(81);
                colors += "hi MBEVisibleNormal            ctermfg=" + X(85) + " ctermbg=" + X(82);
                colors += "hi MBEVisibleChanged           ctermfg=" + X(87) + " ctermbg=" + X(82);

                colors += "hi DiffText       cterm=NONE   ctermfg=" + X(79) + " ctermbg=" + X(34);
                colors += "hi DiffChange     cterm=NONE   ctermfg=" + X(79) + " ctermbg=" + X(17);
                colors += "hi DiffDelete     cterm=NONE   ctermfg=" + X(79) + " ctermbg=" + X(32);
                colors += "hi DiffAdd        cterm=NONE   ctermfg=" + X(79) + " ctermbg=" + X(20);

                colors += "hi Folded         cterm=NONE   ctermfg=" + X(79) + " ctermbg=" + X(35);
                colors += "hi FoldColumn     cterm=NONE   ctermfg=" + X(39) + " ctermbg=" + X(80);

                colors += "hi Directory      cterm=NONE   ctermfg=" + X(28) + " ctermbg=" + "NONE";
                colors += "hi LineNr         cterm=NONE   ctermfg=" + X(39) + " ctermbg=" + X(80);
                colors += "hi NonText        cterm=BOLD   ctermfg=" + X(39) + " ctermbg=" + "NONE";
                colors += "hi SpecialKey     cterm=BOLD   ctermfg=" + X(55) + " ctermbg=" + "NONE";
                colors += "hi Title          cterm=BOLD   ctermfg=" + X(48) + " ctermbg=" + "NONE";
                colors += "hi Visual         cterm=NONE   ctermfg=" + X(79) + " ctermbg=" + X(38);

                colors += "hi Comment        cterm=NONE   ctermfg=" + X(52) + " ctermbg=" + "NONE";
                colors += "hi Constant       cterm=NONE   ctermfg=" + X(73) + " ctermbg=" + "NONE";
                colors += "hi String         cterm=NONE   ctermfg=" + X(73) + " ctermbg=" + X(81);
                colors += "hi Error          cterm=NONE   ctermfg=" + X(79) + " ctermbg=" + X(32);
                colors += "hi Identifier     cterm=NONE   ctermfg=" + X(53) + " ctermbg=" + "NONE";
                colors += "hi Ignore         cterm=NONE";
                colors += "hi Number         cterm=NONE   ctermfg=" + X(69) + " ctermbg=" + "NONE";
                colors += "hi PreProc        cterm=NONE   ctermfg=" + X(25) + " ctermbg=" + "NONE";
                colors += "hi Special        cterm=NONE   ctermfg=" + X(55) + " ctermbg=" + "NONE";
                colors += "hi SpecialChar    cterm=NONE   ctermfg=" + X(55) + " ctermbg=" + X(81);
                colors += "hi Statement      cterm=NONE   ctermfg=" + X(27) + " ctermbg=" + "NONE";
                colors += "hi Todo           cterm=BOLD   ctermfg=" + X(16) + " ctermbg=" + X(57);
                colors += "hi Type           cterm=NONE   ctermfg=" + X(71) + " ctermbg=" + "NONE";
                colors += "hi Underlined     cterm=BOLD   ctermfg=" + X(77) + " ctermbg=" + "NONE";
                colors += "hi TaglistTagName cterm=BOLD   ctermfg=" + X(39) + " ctermbg=" + "NONE";

                colors += "hi Pmenu          cterm=NONE   ctermfg=" + X(87) + " ctermbg=" + X(82);
                colors += "hi PmenuSel       cterm=BOLD   ctermfg=" + X(87) + " ctermbg=" + X(38);
                colors += "hi PmenuSbar      cterm=BOLD   ctermfg=" + X(87) + " ctermbg=" + X(39);
                colors += "hi PmenuThumb     cterm=BOLD   ctermfg=" + X(87) + " ctermbg=" + X(39);

                colors += "hi SpellBad       cterm=NONE   ctermbg=" + X(32);
                colors += "hi SpellRare      cterm=NONE   ctermbg=" + X(33);
                colors += "hi SpellLocal     cterm=NONE   ctermbg=" + X(36);
                colors += "hi SpellCap       cterm=NONE   ctermbg=" + X(21);
                colors += "hi MatchParen     cterm=NONE   ctermbg=" + X(14) + "ctermfg="  + X(25);
        }

        vim_colorscheme("inkpot", 0, NULL, colors, -1);
}
/*end*/



