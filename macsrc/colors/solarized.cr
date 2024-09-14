/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: solarized.cr,v 1.6 2024/08/04 11:42:44 cvsuser Exp $
 * solarized coloriser, GRIEF port -- **experimental**.
 *
 * Original:
 *  Name:     Solarized vim colorscheme
 *  Author:   Ethan Schoonover <es@ethanschoonover.com>
 *  URL:      http://ethanschoonover.com/solarized
 *              (see this url for latest release & screenshots)
 *  License:  OSI approved MIT license (see end of this file)
 *
 *  Created:  In the middle of the night
 *  Modified: 2011 May 05
 *
 *    Solarized is a carefully designed selective contrast colorscheme with dual
 *    light and dark modes that runs in both GUI, 256 and 16 color modes.
 *
 *    Solarized is a sixteen color palette (eight monotones, eight accent colors) designed for
 *    use with terminal and gui applications. It has several unique properties. I designed this
 *    colorscheme with both precise CIELAB lightness relationships and a refined set of hues
 *    based on fixed color wheel relationships. It has been tested extensively in real world use
 *    on color calibrated displays (as well as uncalibrated/intentionally miscalibrated displays)
 *    and in a variety of lighting conditions.
 *
 *    See the homepage above for screenshots and details.
 *
 * Usage:
 *
 *    colorscheme solarized [--mode=dark|light] [options]
 *    solarized_options
 *
 * Options:
 *
 *    o --termcolors=<number>
 *
 *      This is set to 16 by default, meaning that Solarized will attempt to use the standard 16
 *      colors of your terminal emulator. You will need to set those colors to the correct
 *      Solarized values either manually or by importing one of the many colorscheme available
 *      for popular terminal emulators and Xdefaults.
 *
 *    o --termtrans=yes/no
 *
 *      If you use a terminal emulator with a transparent background and Solarized isn't
 *      displaying the background color transparently, set this to 1 and Solarized will use the
 *      default (transparent) background of the terminal emulator. urxvt required this in my
 *      testing; iTerm2 did not.
 *
 *      Note that on Mac OS X Terminal.app, solarized_termtrans is set to 1 by default as this is
 *      almost always the best option. The only exception to this is if the working terminfo file
 *      supports 256 colors (xterm-256color).
 *
 *    o --degrade=yes/no
 *
 *      For test purposes only; forces Solarized to use the 256 degraded color mode to test the
 *      approximate color values for accuracy.
 *
 *    o --bold=yes/no, underline=yes/no, italic=yes/no
 *
 *      If you wish to stop Solarized from displaying bold, underlined or italicized typefaces,
 *      simply assign a zero value to the appropriate variable, for example: let
 *      'solarized_italic=0';
 *
 *    o --contrast=<normal,high,low>
 *
 *      Stick with normal! It's been carefully tested. Setting this option to high or low does
 *      use the same Solarized palette but simply shifts some values up or down in order to
 *      expand or compress the tonal range displayed.
 *
 *    o --visibility=<normal,high,low>
 *
 *      Special characters such as trailing whitespace, tabs, newlines, when displayed using :set
 *      list can be set to one of three levels depending on your needs. Default value is normal
 *      with high and low options.
 */

#include "../grief.h"

// Italic
static int      terminal_italic = 1;
static list     terms_italic = {
                    "rxvt",
                    "xterm-rxvt",
                    "xterm-gnome",
                    "gnome-terminal",
                    "xconsole-dos",
                    "xgrief"
                    };

// For reference only, terminals are known to be incomptible.
//  terminals that are in neither list need to be tested.
static string   terms_noitalic = {
                    "iTerm.app",
                    "Apple_Terminal"
                    };

// Options
static string   solarized_mode          = "default";
static int      solarized_termtrans_default = 0;
static int      solarized_termtrans     = 0;
static int      solarized_degrade       = 0;
static int      solarized_bold          = 1;
static int      solarized_underline     = 1;
static int      solarized_italic        = 1;
static int      solarized_termcolors    = 16;
static string   solarized_contrast      = "normal";
static string   solarized_visibility    = "normal";
static string   solarized_diffmode      = "normal";


static void
solarized_colorscheme(int dark)
{
        string termname;
        int colordepth, gui_running = 0;        // TODO: TF_GUI
        list colors;

        // depth >= 88
        get_term_feature(TF_NAME, termname);
        get_term_feature(TF_COLORDEPTH, colordepth);
        if (colordepth < 8) {
                error("solarized, color depth not supported");
                return;
        }

        // terminal specific modes
        if (gui_running) {
                terminal_italic = 1;            // TODO: could refactor to not require this at all
                solarized_termtrans_default = 0;
        } else {
                string term;

                terminal_italic = 0;            // terminals will be guilty until proven compatible
                while (list_each(terms_italic, term) >= 0) {
                        if (termname == term) {
                                terminal_italic = 1;
                        }
                }
                if (termname == "apple_terminal") {
                        solarized_termtrans_default = 1;
                }
        }

        if (-1 == solarized_termtrans) {
                solarized_termtrans = solarized_termtrans_default;
        }

        // " GUI & CSApprox hexadecimal palettes"{{{
        // " ---------------------------------------------------------------------
        // "
        // " Set both gui and terminal color values in separate conditional statements
        // " Due to possibility that CSApprox is running (though I suppose we could just
        // " leave the hex values out entirely in that case and include only cterm colors)
        // " We also check to see if user has set solarized (force use of the
        // " neutral gray monotone palette component)
        //
        // SOLARIZED  HEX      16/8  TERMCOL   XTERM/HEX    L*A*B       sRGB         HSB
        // ---------  -------  ----  -------   -----------  ----------  -----------  -----------
        // base03     #002b36   8/4  brblack   234 #1c1c1c  15 -12 -12    0  43  54  193 100  21
        // base02     #073642   0/4  black     235 #262626  20 -12 -12    7  54  66  192  90  26
        // base01     #586e75  10/7  brgreen   240 #4e4e4e  45 -07 -07   88 110 117  194  25  46
        // base00     #657b83  11/7  bryellow  241 #585858  50 -07 -07  101 123 131  195  23  51
        // base0      #839496  12/6  brblue    244 #808080  60 -06 -03  131 148 150  186  13  59
        // base1      #93a1a1  14/4  brcyan    245 #8a8a8a  65 -05 -02  147 161 161  180   9  63
        // base2      #eee8d5   7/7  white     254 #d7d7af  92 -00  10  238 232 213   44  11  93
        // base3      #fdf6e3  15/7  brwhite   230 #ffffd7  97  00  10  253 246 227   44  10  99
        // yellow     #b58900   3/3  yellow    136 #af8700  60  10  65  181 137   0   45 100  71
        // orange     #cb4b16   9/3  brred     166 #d75f00  50  50  55  203  75  22   18  89  80
        // red        #dc322f   1/1  red       160 #d70000  50  65  45  220  50  47    1  79  86
        // magenta    #d33682   5/5  magenta   125 #af005f  50  65 -05  211  54 130  331  74  83
        // violet     #6c71c4  13/5  brmagenta  61 #5f5faf  50  15 -45  108 113 196  237  45  77
        // blue       #268bd2   4/4  blue       33 #0087ff  55 -10 -45   38 139 210  205  82  82
        // cyan       #2aa198   6/6  cyan       37 #00afaf  60 -35 -05   42 161 152  175  74  63
        // green      #859900   2/2  green      64 #5f8700  60 -20  65  133 153   0   68 100  60
        //
        string base03, base02, base01, base00, base0,  base1, base2, base3;
        string yellow, orange, red, magenta, violet, blue, cyan, green;
        string vmode, back;

        if (gui_running && solarized_degrade == 0) {
                vmode       = "gui";
                base03      = "#002b36";
                base02      = "#073642";
                base01      = "#586e75";
                base00      = "#657b83";
                base0       = "#839496";
                base1       = "#93a1a1";
                base2       = "#eee8d5";
                base3       = "#fdf6e3";
                yellow      = "#b58900";
                orange      = "#cb4b16";
                red         = "#dc322f";
                magenta     = "#d33682";
                violet      = "#6c71c4";
                blue        = "#268bd2";
                cyan        = "#2aa198";
           // " green       = "#859900";        // " original
                green       = "#719e07";        // " experimental

        } else if (gui_running && solarized_degrade == 1) {
                // " These colors are identical to the 256 color mode. They may be viewed
                // " while in gui mode via "solarized_degrade=1", though this is not
                // " recommened and is for testing only.
                vmode       = "gui";
                base03      = "#1c1c1c";
                base02      = "#262626";
                base01      = "#4e4e4e";
                base00      = "#585858";
                base0       = "#808080";
                base1       = "#8a8a8a";
                base2       = "#d7d7af";
                base3       = "#ffffd7";
                yellow      = "#af8700";
                orange      = "#d75f00";
                red         = "#af0000";
                magenta     = "#af005f";
                violet      = "#5f5faf";
                blue        = "#0087ff";
                cyan        = "#00afaf";
                green       = "#5f8700";

        } else if (solarized_termcolors != 256 && colordepth >= 16) {
                vmode       = "cterm";
                base03      = "8";
                base02      = "0";
                base01      = "10";
                base00      = "11";
                base0       = "12";
                base1       = "14";
                base2       = "7";
                base3       = "15";
                yellow      = "3";
                orange      = "9";
                red         = "1";
                magenta     = "5";
                violet      = "13";
                blue        = "4";
                cyan        = "6";
                green       = "2";

        } else if (solarized_termcolors == 256) {
                vmode       = "cterm";
                base03      = "233";            //GRIEF, was "234";
                base02      = "235";
                base01      = "239";
                base00      = "240";
                base0       = "244";
                base1       = "245";
                base2       = "187";
                base3       = "230";
                yellow      = "136";
                orange      = "166";
                red         = "124";
                magenta     = "125";
                violet      = "61";
                blue        = "33";
                cyan        = "37";
                green       = "64";

        } else {
                vmode       = "cterm";
                base03      = "DarkGray";
                base02      = "Black";
                base01      = "LightGreen";
                base00      = "LightYellow";
                base0       = "LightBlue";
                base1       = "LightCyan";
                base2       = "LightGray";
                base3       = "White";
                yellow      = "DarkYellow";
                orange      = "LightRed";
                red         = "DarkRed";
                magenta     = "DarkMagenta";
                violet      = "LightMagenta";
                blue        = "DarkBlue";
                cyan        = "DarkCyan";
                green       = "DarkGreen";
        }

        // "}}}
        // " Formatting options and null values for passthrough effect "{{{
        // " ---------------------------------------------------------------------
        string none         = "NONE";
      //string t_none       = "NONE";
      //string n            = "NONE";
        string c            = ",undercurl";
        string r            = ",reverse";
        string s            = ",standout";
        string ou           = "";
      //string ob           = "";
        string b;
        string bb;
        string u;
        string i;

        // "}}}
        // " Background value based on termtrans setting "{{{
        // " ---------------------------------------------------------------------
        if (gui_running || solarized_termtrans == 0) {
                back        = base03;
        } else {
                back        = "NONE";
        }

        // "}}}
        // " Alternate light scheme "{{{
        // " ---------------------------------------------------------------------
        if (0 == dark) {
                string temp03 = base03;
                string temp02 = base02;
                string temp01 = base01;
                string temp00 = base00;
                base03 = base3;
                base02 = base2;
                base01 = base1;
                base00 = base0;
                base0  = temp00;
                base1  = temp01;
                base2  = temp02;
                base3  = temp03;
                if (back != "NONE") {
                        back = base03;
                }
        }

        // "}}}
        // " Optional contrast schemes "{{{
        // " ---------------------------------------------------------------------
        if (solarized_contrast == "high") {
                base01      = base00;
                base00      = base0;
                base0       = base1;
                base1       = base2;
                base2       = base3;
        } else if (solarized_contrast == "low") {
                back        = base02;
                ou          = ",underline";
        }

        // "}}}
        // " Overrides dependent on user specified values and environment "{{{
        // " ---------------------------------------------------------------------
        if (solarized_bold == 0 || colordepth == 8) {
                b           = "";
                bb          = ",bold";
        } else {
                b           = ",bold";
                bb          = "";
        }

        if (solarized_underline == 0) {
                u           = "";
        } else {
                u           = ",underline";
        }

        if (solarized_italic == 0 || terminal_italic == 0) {
                i           = "";
        } else {
                i           = ",italic";
        }

        // "}}}
        // " Highlighting primitives"{{{
        // " ---------------------------------------------------------------------

        string bg_none      = " "+vmode+"bg="+none;
        string bg_back      = " "+vmode+"bg="+back;
        string bg_base03    = " "+vmode+"bg="+base03;
        string bg_base02    = " "+vmode+"bg="+base02;
        string bg_base01    = " "+vmode+"bg="+base01;
        string bg_base00    = " "+vmode+"bg="+base00;
        string bg_base0     = " "+vmode+"bg="+base0;
      //string bg_base1     = " "+vmode+"bg="+base1;
        string bg_base2     = " "+vmode+"bg="+base2;
      //string bg_base3     = " "+vmode+"bg="+base3;
      //string bg_green     = " "+vmode+"bg="+green;
      //string bg_yellow    = " "+vmode+"bg="+yellow;
      //string bg_orange    = " "+vmode+"bg="+orange;
      //string bg_red       = " "+vmode+"bg="+red;
      //string bg_magenta   = " "+vmode+"bg="+magenta;
      //string bg_violet    = " "+vmode+"bg="+violet;
      //string bg_blue      = " "+vmode+"bg="+blue;
      //string bg_cyan      = " "+vmode+"bg="+cyan;

        string fg_none      = " "+vmode+"fg="+none;
      //string fg_back      = " "+vmode+"fg="+back;
        string fg_base03    = " "+vmode+"fg="+base03;
        string fg_base02    = " "+vmode+"fg="+base02;
        string fg_base01    = " "+vmode+"fg="+base01;
        string fg_base00    = " "+vmode+"fg="+base00;
        string fg_base0     = " "+vmode+"fg="+base0;
        string fg_base1     = " "+vmode+"fg="+base1;
        string fg_base2     = " "+vmode+"fg="+base2;
      //string fg_base3     = " "+vmode+"fg="+base3;
        string fg_green     = " "+vmode+"fg="+green;
        string fg_yellow    = " "+vmode+"fg="+yellow;
        string fg_orange    = " "+vmode+"fg="+orange;
        string fg_red       = " "+vmode+"fg="+red;
        string fg_magenta   = " "+vmode+"fg="+magenta;
        string fg_violet    = " "+vmode+"fg="+violet;
        string fg_blue      = " "+vmode+"fg="+blue;
        string fg_cyan      = " "+vmode+"fg="+cyan;

        string fmt_none     = " "+vmode+"=NONE"     +" term=NONE";
        string fmt_bold     = " "+vmode+"=NONE"+b   +" term=NONE"+b;
      //string fmt_bldi     = " "+vmode+"=NONE"+b   +" term=NONE"+b;
        string fmt_undr     = " "+vmode+"=NONE"+u   +" term=NONE"+u;
        string fmt_undb     = " "+vmode+"=NONE"+u+b +" term=NONE"+u+b;
      //string fmt_undi     = " "+vmode+"=NONE"+u   +" term=NONE"+u;
        string fmt_uopt     = " "+vmode+"=NONE"+ou  +" term=NONE"+ou;
        string fmt_curl     = " "+vmode+"=NONE"+c   +" term=NONE"+c;
        string fmt_ital     = " "+vmode+"=NONE"+i   +" term=NONE"+i;
        string fmt_stnd     = " "+vmode+"=NONE"+s   +" term=NONE"+s;
        string fmt_revr     = " "+vmode+"=NONE"+r   +" term=NONE"+r;
      //string fmt_revb     = " "+vmode+"=NONE"+r+b +" term=NONE"+r+b;

        // " revbb (reverse bold for bright colors) is only set to actual bold in low
        //       " color terminals (t_co=8, such as OS X Terminal.app) and should only be used
        //       " with colors 8-15.
        string fmt_revbb    = " "+vmode+"=NONE"+r+bb+" term=NONE"+r+bb;
        string fmt_revbbu   = " "+vmode+"=NONE"+r+bb+u+" term=NONE"+r+bb+u;

        string sp_none, sp_back, sp_base03, sp_base02, sp_base01, sp_base00, sp_base0,
                sp_base1, sp_base2, sp_base3, sp_green, sp_yellow, sp_orange, sp_red,
                sp_magenta, sp_violet, sp_blue, sp_cyan;

        if (gui_running) {
                sp_none     = " guisp="+none;
                sp_back     = " guisp="+back;
                sp_base03   = " guisp="+base03;
                sp_base02   = " guisp="+base02;
                sp_base01   = " guisp="+base01;
                sp_base00   = " guisp="+base00;
                sp_base0    = " guisp="+base0;
                sp_base1    = " guisp="+base1;
                sp_base2    = " guisp="+base2;
                sp_base3    = " guisp="+base3;
                sp_green    = " guisp="+green;
                sp_yellow   = " guisp="+yellow;
                sp_orange   = " guisp="+orange;
                sp_red      = " guisp="+red;
                sp_magenta  = " guisp="+magenta;
                sp_violet   = " guisp="+violet;
                sp_blue     = " guisp="+blue;
                sp_cyan     = " guisp="+cyan;

        } else {
                sp_none     = "";
                sp_back     = "";
                sp_base03   = "";
                sp_base02   = "";
                sp_base01   = "";
                sp_base00   = "";
                sp_base0    = "";
                sp_base1    = "";
                sp_base2    = "";
                sp_base3    = "";
                sp_green    = "";
                sp_yellow   = "";
                sp_orange   = "";
                sp_red      = "";
                sp_magenta  = "";
                sp_violet   = "";
                sp_blue     = "";
                sp_cyan     = "";
        }

        // "}}}
        // " Basic highlighting"{{{
        // " ---------------------------------------------------------------------
        // " note that link syntax to avoid duplicate configuration doesn't work with the
        // " exe compiled formats

        colors += "set background="+(dark ? "dark":"light");
        colors += "hi clear";

        colors += "hi! Normal"         +fmt_none   +fg_base0  +bg_back;

        colors += "hi! Comment"        +fmt_ital   +fg_base01 +bg_none;
            // "       *Comment         any comment

        colors += "hi! Constant"       +fmt_none   +fg_cyan   +bg_none;
            // "       *Constant        any constant
            // "        String          a string constant: "this is a string"
            // "        Character       a character constant: 'c', '\n'
            // "        Number          a number constant: 234, 0xff
            // "        Boolean         a boolean constant: TRUE, false
            // "        Float           a floating point constant: 2.3e10

        colors += "hi! Identifier"     +fmt_none   +fg_blue   +bg_none;
            // "       *Identifier      any variable name
            // "        Function        function name (also: methods for classes)

        colors += "hi! Statement"      +fmt_none   +fg_green  +bg_none;
            // "       *Statement       any statement
            // "        Conditional     if, then, else, endif, switch, etc.
            // "        Repeat          for, do, while, etc.
            // "        Label           case, default, etc.
            // "        Operator        "sizeof", "+", "*", etc.
            // "        Keyword         any other keyword
            // "        Exception       try, catch, throw

        colors += "hi! PreProc"        +fmt_none   +fg_orange +bg_none;
            // "       *PreProc         generic Preprocessor
            // "        Include         preprocessor #include
            // "        Define          preprocessor #define
            // "        Macro           same as Define
            // "        PreCondit       preprocessor #if, #else, #endif, etc.

        colors += "hi! Type"          +fmt_none   +fg_yellow +bg_none;
            // "       *Type            int, long, char, etc.
            // "        StorageClass    static, register, volatile, etc.
            // "        Structure       struct, union, enum, etc.
            // "        Typedef         A typedef

        colors += "hi! Special"        +fmt_none   +fg_red    +bg_none;
            // "       *Special         any special symbol
            // "        SpecialChar     special character in a constant
            // "        Tag             you can use CTRL-] on this
            // "        Delimiter       character that needs attention
            // "        SpecialComment  special things inside a comment
            // "        Debug           debugging statements

        colors += "hi! Underlined"     +fmt_none   +fg_violet +bg_none;
            // "       *Underlined      text that stands out, HTML links

        colors += "hi! Ignore"         +fmt_none   +fg_none   +bg_none;
            // "       *Ignore          left blank, hidden  |hl-Ignore|

        colors += "hi! Error"          +fmt_bold   +fg_red    +bg_none;
            // "       *Error           any erroneous construct

        colors += "hi! Todo"           +fmt_bold   +fg_magenta+bg_none;
            // "       *Todo            anything that needs extra attention; mostly the
            // "                        keywords TODO FIXME and XXX

        // "}}}
        // " Extended highlighting "{{{
        // " ---------------------------------------------------------------------
     if (solarized_visibility=="high") {
        colors += "hi! SpecialKey"     +fmt_revr   +fg_red    +bg_none;
        colors += "hi! NonText"        +fmt_bold   +fg_red    +bg_none;
     } else if (solarized_visibility=="low") {
        colors += "hi! SpecialKey"     +fmt_bold   +fg_base02 +bg_none;
        colors += "hi! NonText"        +fmt_bold   +fg_base02 +bg_none;
     } else {
        colors += "hi! SpecialKey"     +fmt_bold   +fg_base00 +bg_base02;
        colors += "hi! NonText"        +fmt_bold   +fg_base00 +bg_none;
     }
        colors += "hi! StatusLine"     +fmt_none   +fg_base1  +bg_base02 +fmt_revbb;
        colors += "hi! StatusLineNC"   +fmt_none   +fg_base00 +bg_base02 +fmt_revbb;
        colors += "hi! Visual"         +fmt_none   +fg_base01 +bg_base03 +fmt_revbb;
        colors += "hi! Directory"      +fmt_none   +fg_blue   +bg_none;
        colors += "hi! ErrorMsg"       +fmt_revr   +fg_red    +bg_none;
        colors += "hi! IncSearch"      +fmt_stnd   +fg_orange +bg_none;
        colors += "hi! Search"         +fmt_revr   +fg_yellow +bg_none;
        colors += "hi! MoreMsg"        +fmt_none   +fg_blue   +bg_none;
        colors += "hi! ModeMsg"        +fmt_none   +fg_blue   +bg_none;
        colors += "hi! LineNr"         +fmt_none   +fg_base01 +bg_base02;
        colors += "hi! Question"       +fmt_bold   +fg_cyan   +bg_none;
     if (gui_running || colordepth > 8) {
        colors += "hi! VertSplit"      +fmt_none   +fg_base00 +bg_base00;
     } else {
        colors += "hi! VertSplit"      +fmt_revbb  +fg_base00 +bg_base02;
     }
        colors += "hi! Title"          +fmt_bold   +fg_orange +bg_none;
        colors += "hi! VisualNOS"      +fmt_stnd   +fg_none   +bg_base02 +fmt_revbb;
        colors += "hi! WarningMsg"     +fmt_bold   +fg_red    +bg_none;
        colors += "hi! WildMenu"       +fmt_none   +fg_base2  +bg_base02 +fmt_revbb;
        colors += "hi! Folded"         +fmt_undb   +fg_base0  +bg_base02 +sp_base03;
        colors += "hi! FoldColumn"     +fmt_none   +fg_base0  +bg_base02;
     if (solarized_diffmode=="high") {
        colors += "hi! DiffAdd"        +fmt_revr   +fg_green  +bg_none;
        colors += "hi! DiffChange"     +fmt_revr   +fg_yellow +bg_none;
        colors += "hi! DiffDelete"     +fmt_revr   +fg_red    +bg_none;
        colors += "hi! DiffText"       +fmt_revr   +fg_blue   +bg_none;
     } else if (solarized_diffmode=="low") {
        colors += "hi! DiffAdd"        +fmt_undr   +fg_green  +bg_none   +sp_green;
        colors += "hi! DiffChange"     +fmt_undr   +fg_yellow +bg_none   +sp_yellow;
        colors += "hi! DiffDelete"     +fmt_bold   +fg_red    +bg_none;
        colors += "hi! DiffText"       +fmt_undr   +fg_blue   +bg_none   +sp_blue;
     } else {
      if (gui_running) {
        colors += "hi! DiffAdd"        +fmt_bold   +fg_green  +bg_base02 +sp_green;
        colors += "hi! DiffChange"     +fmt_bold   +fg_yellow +bg_base02 +sp_yellow;
        colors += "hi! DiffDelete"     +fmt_bold   +fg_red    +bg_base02;
        colors += "hi! DiffText"       +fmt_bold   +fg_blue   +bg_base02 +sp_blue;
      } else {
        colors += "hi! DiffAdd"        +fmt_none   +fg_green  +bg_base02 +sp_green;
        colors += "hi! DiffChange"     +fmt_none   +fg_yellow +bg_base02 +sp_yellow;
        colors += "hi! DiffDelete"     +fmt_none   +fg_red    +bg_base02;
        colors += "hi! DiffText"       +fmt_none   +fg_blue   +bg_base02 +sp_blue;
      }
     }
        colors += "hi! SignColumn"     +fmt_none   +fg_base0;
        colors += "hi! Conceal"        +fmt_none   +fg_blue   +bg_none;
        colors += "hi! SpellBad"       +fmt_curl   +fg_none   +bg_none    +sp_red;
        colors += "hi! SpellCap"       +fmt_curl   +fg_none   +bg_none    +sp_violet;
        colors += "hi! SpellRare"      +fmt_curl   +fg_none   +bg_none    +sp_cyan;
        colors += "hi! SpellLocal"     +fmt_curl   +fg_none   +bg_none    +sp_yellow;
        colors += "hi! Pmenu"          +fmt_none   +fg_base0  +bg_base02  +fmt_revbb;
        colors += "hi! PmenuSel"       +fmt_none   +fg_base01 +bg_base2   +fmt_revbb;
        colors += "hi! PmenuSbar"      +fmt_none   +fg_base2  +bg_base0   +fmt_revbb;
        colors += "hi! PmenuThumb"     +fmt_none   +fg_base0  +bg_base03  +fmt_revbb;
        colors += "hi! TabLine"        +fmt_undr   +fg_base0  +bg_base02  +sp_base0;
        colors += "hi! TabLineFill"    +fmt_undr   +fg_base0  +bg_base02  +sp_base0;
        colors += "hi! TabLineSel"     +fmt_undr   +fg_base01 +bg_base2   +sp_base0  +fmt_revbbu;
        colors += "hi! CursorColumn"   +fmt_none   +fg_none   +bg_base02;
        colors += "hi! CursorLine"     +fmt_uopt   +fg_none   +bg_base02  +sp_base1;
        colors += "hi! ColorColumn"    +fmt_none   +fg_none   +bg_base02;
        colors += "hi! Cursor"         +fmt_none   +fg_base03 +bg_base0;
        colors += "hi! link lCursor Cursor";
        colors += "hi! MatchParen"     +fmt_bold   +fg_red    +bg_base01;

        // "}}}
        // " vim syntax highlighting "{{{
        // " ---------------------------------------------------------------------
     // colors += "hi! vimLineComment"    +fg_base01   +bg_none   +fmt_ital;
     // "hi! link vimComment Comment
     // "hi! link vimLineComment Comment
     // colors += "hi! link vimVar Identifier";
     // colors += "hi! link vimFunc Function";
     // colors += "hi! link vimUserFunc Function";
     // colors += "hi! link helpSpecial Special";
     // colors += "hi! link vimSet Normal";
     // colors += "hi! link vimSetEqual Normal";
     // colors += "hi! vimCommentString"  +fmt_none    +fg_violet +bg_none;
     // colors += "hi! vimCommand"        +fmt_none    +fg_yellow +bg_none;
     // colors += "hi! vimCmdSep"         +fmt_bold    +fg_blue   +bg_none;
     // colors += "hi! helpExample"       +fmt_none    +fg_base1  +bg_none;
     // colors += "hi! helpOption"        +fmt_none    +fg_cyan   +bg_none;
     // colors += "hi! helpNote"          +fmt_none    +fg_magenta+bg_none;
     // colors += "hi! helpVim"           +fmt_none    +fg_magenta+bg_none;
     // colors += "hi! helpHyperTextJump" +fmt_undr    +fg_blue   +bg_none;
     // colors += "hi! helpHyperTextEntry"+fmt_none    +fg_green  +bg_none;
     // colors += "hi! vimIsCommand"      +fmt_none    +fg_base00 +bg_none;
     // colors += "hi! vimSynMtchOpt"     +fmt_none    +fg_yellow +bg_none;
     // colors += "hi! vimSynType"        +fmt_none    +fg_cyan   +bg_none;
     // colors += "hi! vimHiLink"         +fmt_none    +fg_blue   +bg_none;
     // colors += "hi! vimHiGroup"        +fmt_none    +fg_blue   +bg_none;
     // colors += "hi! vimGroup"          +fmt_undb    +fg_blue   +bg_none;

        // "}}}
        // " diff highlighting "{{{
        // " ---------------------------------------------------------------------
     // colors += "hi! link diffAdded Statement";
     // colors += "hi! link diffLine Identifier";

        // "}}}
        // " git & gitcommit highlighting "{{{
     // "git
     // "exe "hi! gitDateHeader"
     // "exe "hi! gitIdentityHeader"
     // "exe "hi! gitIdentityKeyword"
     // "exe "hi! gitNotesHeader"
     // "exe "hi! gitReflogHeader"
     // "exe "hi! gitKeyword"
     // "exe "hi! gitIdentity"
     // "exe "hi! gitEmailDelimiter"
     // "exe "hi! gitEmail"
     // "exe "hi! gitDate"
     // "exe "hi! gitMode"
     // "exe "hi! gitHashAbbrev"
     // "exe "hi! gitHash"
     // "exe "hi! gitReflogMiddle"
     // "exe "hi! gitReference"
     // "exe "hi! gitStage"
     // "exe "hi! gitType"
     // "exe "hi! gitDiffAdded"
     // "exe "hi! gitDiffRemoved"
     // "gitcommit
     // "exe "hi! gitcommitSummary"
     // colors += "hi! gitcommitComment"      +fmt_ital     +fg_base01    +bg_none;
     // colors += "hi! link gitcommitUntracked gitcommitComment";
     // colors += "hi! link gitcommitDiscarded gitcommitComment";
     // colors += "hi! link gitcommitSelected  gitcommitComment";
     // colors += "hi! gitcommitUnmerged"     +fmt_bold     +fg_green     +bg_none;
     // colors += "hi! gitcommitOnBranch"     +fmt_bold     +fg_base01    +bg_none;
     // colors += "hi! gitcommitBranch"       +fmt_bold     +fg_magenta   +bg_none;
     // colors += "hi! link gitcommitNoBranch gitcommitBranch";
     // colors += "hi! gitcommitDiscardedType"+fmt_none     +fg_red       +bg_none;
     // colors += "hi! gitcommitSelectedType" +fmt_none     +fg_green     +bg_none;
     // "exe "hi! gitcommitUnmergedType";
     // "exe "hi! gitcommitType";
     // "exe "hi! gitcommitNoChanges";
     // "exe "hi! gitcommitHeader";
     // colors += "hi! gitcommitHeader"       +fmt_none     +fg_base01    +bg_none;
     // colors += "hi! gitcommitUntrackedFile"+fmt_bold     +fg_cyan      +bg_none;
     // colors += "hi! gitcommitDiscardedFile"+fmt_bold     +fg_red       +bg_none;
     // colors += "hi! gitcommitSelectedFile" +fmt_bold     +fg_green     +bg_none;
     // colors += "hi! gitcommitUnmergedFile" +fmt_bold     +fg_yellow    +bg_none;
     // colors += "hi! gitcommitFile"         +fmt_bold     +fg_base0     +bg_none;
     // colors += "hi! link gitcommitDiscardedArrow gitcommitDiscardedFile";
     // colors += "hi! link gitcommitSelectedArrow  gitcommitSelectedFile";
     // colors += "hi! link gitcommitUnmergedArrow  gitcommitUnmergedFile";
     // "exe "hi! gitcommitArrow"
     // "exe "hi! gitcommitOverflow"
     // "exe "hi! gitcommitBlank"

        // " }}}
        // " html highlighting "{{{
        // " ---------------------------------------------------------------------
     // colors += "hi! htmlTag"           +fmt_none +fg_base01 +bg_none;
     // colors += "hi! htmlEndTag"        +fmt_none +fg_base01 +bg_none;
     // colors += "hi! htmlTagN"          +fmt_bold +fg_base1  +bg_none;
     // colors += "hi! htmlTagName"       +fmt_bold +fg_blue   +bg_none;
     // colors += "hi! htmlSpecialTagName"+fmt_ital +fg_blue   +bg_none;
     // colors += "hi! htmlArg"           +fmt_none +fg_base00 +bg_none;
     // colors += "hi! javaScript"        +fmt_none +fg_yellow +bg_none;

        // "}}}
        // " perl highlighting "{{{
        // " ---------------------------------------------------------------------
     // colors += "hi! perlHereDoc"    . s:fg_base1  +bg_back   +fmt_none;
     // colors += "hi! perlVarPlain"   . s:fg_yellow +bg_back   +fmt_none;
     // colors += "hi! perlStatementFileDesc". s:fg_cyan+bg_back+fmt_none;

        // "}}}
        // " tex highlighting "{{{
        // " ---------------------------------------------------------------------
     // colors += "hi! texStatement"   . s:fg_cyan   +bg_back   +fmt_none;
     // colors += "hi! texMathZoneX"   . s:fg_yellow +bg_back   +fmt_none;
     // colors += "hi! texMathMatcher" . s:fg_yellow +bg_back   +fmt_none;
     // colors += "hi! texMathMatcher" . s:fg_yellow +bg_back   +fmt_none;
     // colors += "hi! texRefLabel"    . s:fg_yellow +bg_back   +fmt_none;

        // "}}}
        // " ruby highlighting "{{{
        // " ---------------------------------------------------------------------
     // colors += "hi! rubyDefine"     . s:fg_base1  +bg_back   +fmt_bold;
     // "rubyInclude
     // "rubySharpBang
     // "rubyAccess
     // "rubyPredefinedVariable
     // "rubyBoolean
     // "rubyClassVariable
     // "rubyBeginEnd

     // "rubyRepeatModifier
     // colors += "hi! link rubyArrayDelimiter    Special";     // " [ , , ]
     // "rubyCurlyBlock  { , , }
     // colors += "hi! link rubyClass             Keyword";
     // colors += "hi! link rubyModule            Keyword";
     // colors += "hi! link rubyKeyword           Keyword";
     // colors += "hi! link rubyOperator          Operator";
     // colors += "hi! link rubyIdentifier        Identifier";
     // colors += "hi! link rubyInstanceVariable  Identifier";
     // colors += "hi! link rubyGlobalVariable    Identifier";
     // colors += "hi! link rubyClassVariable     Identifier";
     // colors += "hi! link rubyConstant          Type";

        // "}}}
        // " haskell syntax highlighting"{{{
        // " ---------------------------------------------------------------------
        //" For use with syntax/haskell.vim : Haskell Syntax File
        //" http://www.vim.org/scripts/script.php?script_id=3034
        //" See also Steffen Siering's github repository:
        //" http://github.com/urso/dotrc/blob/master/vim/syntax/haskell.vim
        //" ---------------------------------------------------------------------
        //"
        //" Treat True and False specially, see the plugin referenced above
     // hs_highlight_boolean=1;
     // " highlight delims, see the plugin referenced above
     // hs_highlight_delimiters=1;

     // colors += "hi! cPreCondit"                     +fg_orange +bg_none   +fmt_none;

     // colors += "hi! VarId"                          +fg_blue   +bg_none   +fmt_none;
     // colors += "hi! ConId"                          +fg_yellow +bg_none   +fmt_none;
     // colors += "hi! hsImport"                       +fg_magenta+bg_none   +fmt_none;
     // colors += "hi! hsString"                       +fg_base00 +bg_none   +fmt_none;

     // colors += "hi! hsStructure"                    +fg_cyan   +bg_none   +fmt_none;
     // colors += "hi! hs_hlFunctionName"              +fg_blue   +bg_none;
     // colors += "hi! hsStatement"                    +fg_cyan   +bg_none   +fmt_none;
     // colors += "hi! hsImportLabel"                  +fg_cyan   +bg_none   +fmt_none;
     // colors += "hi! hs_OpFunctionName"              +fg_yellow +bg_none   +fmt_none;
     // colors += "hi! hs_DeclareFunction"             +fg_orange +bg_none   +fmt_none;
     // colors += "hi! hsVarSym"                       +fg_cyan   +bg_none   +fmt_none;
     // colors += "hi! hsType"                         +fg_yellow +bg_none   +fmt_none;
     // colors += "hi! hsTypedef"                      +fg_cyan   +bg_none   +fmt_none;
     // colors += "hi! hsModuleName"                   +fg_green  +bg_none   +fmt_undr;
     // colors += "hi! hsModuleStartLabel"             +fg_magenta+bg_none   +fmt_none;
     // colors += "hi! link hsImportParams     Delimiter";
     // colors += "hi! link hsDelimTypeExport  Delimiter";
     // colors += "hi! link hsModuleStartLabel hsStructure";
     // colors += "hi! link hsModuleWhereLabel hsModuleStartLabel";

        //" following is for the haskell-conceal plugin
        //" the first two items don't have an impact, but better safe
     //   exe "hi! hsNiceOperator"     +fg_cyan   +bg_none   +fmt_none
     //   exe "hi! hsniceoperator"     +fg_cyan   +bg_none   +fmt_none

        // "}}}
        // " pandoc markdown syntax highlighting "{{{
        // " ---------------------------------------------------------------------

        // "PandocHiLink pandocNormalBlock
     // colors += "hi! pandocTitleBlock"               +fg_blue   +bg_none   +fmt_none;
     // colors += "hi! pandocTitleBlockTitle"          +fg_blue   +bg_none   +fmt_bold;
     // colors += "hi! pandocTitleComment"             +fg_blue   +bg_none   +fmt_bold;
     // colors += "hi! pandocComment"                  +fg_base01 +bg_none   +fmt_ital;
     // colors += "hi! pandocVerbatimBlock"            +fg_yellow +bg_none   +fmt_none;
     // colors += "hi! link pandocVerbatimBlockDeep         pandocVerbatimBlock";
     // colors += "hi! link pandocCodeBlock                 pandocVerbatimBlock";
     // colors += "hi! link pandocCodeBlockDelim            pandocVerbatimBlock";
     // colors += "hi! pandocBlockQuote"               +fg_blue   +bg_none   +fmt_none;
     // colors += "hi! pandocBlockQuoteLeader1"        +fg_blue   +bg_none   +fmt_none;
     // colors += "hi! pandocBlockQuoteLeader2"        +fg_cyan   +bg_none   +fmt_none;
     // colors += "hi! pandocBlockQuoteLeader3"        +fg_yellow +bg_none   +fmt_none;
     // colors += "hi! pandocBlockQuoteLeader4"        +fg_red    +bg_none   +fmt_none;
     // colors += "hi! pandocBlockQuoteLeader5"        +fg_base0  +bg_none   +fmt_none;
     // colors += "hi! pandocBlockQuoteLeader6"        +fg_base01 +bg_none   +fmt_none;
     // colors += "hi! pandocListMarker"               +fg_magenta+bg_none   +fmt_none;
     // colors += "hi! pandocListReference"            +fg_magenta+bg_none   +fmt_undr;

        // " Definitions
        // " ---------------------------------------------------------------------
     // fg_pdef = fg_violet;
     // colors += "hi! pandocDefinitionBlock"              +fg_pdef  +bg_none  +fmt_none;
     // colors += "hi! pandocDefinitionTerm"               +fg_pdef  +bg_none  +fmt_stnd;
     // colors += "hi! pandocDefinitionIndctr"             +fg_pdef  +bg_none  +fmt_bold;
     // colors += "hi! pandocEmphasisDefinition"           +fg_pdef  +bg_none  +fmt_ital;
     // colors += "hi! pandocEmphasisNestedDefinition"     +fg_pdef  +bg_none  +fmt_bldi;
     // colors += "hi! pandocStrongEmphasisDefinition"     +fg_pdef  +bg_none  +fmt_bold;
     // colors += "hi! pandocStrongEmphasisNestedDefinition"   +fg_pdef+bg_none+fmt_bldi;
     // colors += "hi! pandocStrongEmphasisEmphasisDefinition" +fg_pdef+bg_none+fmt_bldi;
     // colors += "hi! pandocStrikeoutDefinition"          +fg_pdef  +bg_none  +fmt_revr;
     // colors += "hi! pandocVerbatimInlineDefinition"     +fg_pdef  +bg_none  +fmt_none;
     // colors += "hi! pandocSuperscriptDefinition"        +fg_pdef  +bg_none  +fmt_none;
     // colors += "hi! pandocSubscriptDefinition"          +fg_pdef  +bg_none  +fmt_none;

        // " Tables
        // " ---------------------------------------------------------------------
     // fg_ptable = fg_blue;
     // colors += "hi! pandocTable"                        +fg_ptable+bg_none  +fmt_none;
     // colors += "hi! pandocTableStructure"               +fg_ptable+bg_none  +fmt_none;
     // colors += "hi! link pandocTableStructureTop             pandocTableStructre";
     // colors += "hi! link pandocTableStructureEnd             pandocTableStructre";
     // colors += "hi! pandocTableZebraLight"              +fg_ptable+bg_base03+fmt_none;
     // colors += "hi! pandocTableZebraDark"               +fg_ptable+bg_base02+fmt_none;
     // colors += "hi! pandocEmphasisTable"                +fg_ptable+bg_none  +fmt_ital;
     // colors += "hi! pandocEmphasisNestedTable"          +fg_ptable+bg_none  +fmt_bldi;
     // colors += "hi! pandocStrongEmphasisTable"          +fg_ptable+bg_none  +fmt_bold;
     // colors += "hi! pandocStrongEmphasisNestedTable"    +fg_ptable+bg_none  +fmt_bldi;
     // colors += "hi! pandocStrongEmphasisEmphasisTable"  +fg_ptable+bg_none  +fmt_bldi;
     // colors += "hi! pandocStrikeoutTable"               +fg_ptable+bg_none  +fmt_revr;
     // colors += "hi! pandocVerbatimInlineTable"          +fg_ptable+bg_none  +fmt_none;
     // colors += "hi! pandocSuperscriptTable"             +fg_ptable+bg_none  +fmt_none;
     // colors += "hi! pandocSubscriptTable"               +fg_ptable+bg_none  +fmt_none;

        // " Headings
        // " ---------------------------------------------------------------------
     // fg_phead = fg_orange;
     // colors += "hi! pandocHeading"                      +fg_phead +bg_none+fmt_bold;
     // colors += "hi! pandocHeadingMarker"                +fg_yellow+bg_none+fmt_bold;
     // colors += "hi! pandocEmphasisHeading"              +fg_phead +bg_none+fmt_bldi;
     // colors += "hi! pandocEmphasisNestedHeading"        +fg_phead +bg_none+fmt_bldi;
     // colors += "hi! pandocStrongEmphasisHeading"        +fg_phead +bg_none+fmt_bold;
     // colors += "hi! pandocStrongEmphasisNestedHeading"  +fg_phead +bg_none+fmt_bldi;
     // colors += "hi! pandocStrongEmphasisEmphasisHeading"+fg_phead +bg_none+fmt_bldi;
     // colors += "hi! pandocStrikeoutHeading"             +fg_phead +bg_none+fmt_revr;
     // colors += "hi! pandocVerbatimInlineHeading"        +fg_phead +bg_none+fmt_bold;
     // colors += "hi! pandocSuperscriptHeading"           +fg_phead +bg_none+fmt_bold;
     // colors += "hi! pandocSubscriptHeading"             +fg_phead +bg_none+fmt_bold;

        // " Links
        // " ---------------------------------------------------------------------
     // colors += "hi! pandocLinkDelim"                +fg_base01 +bg_none   +fmt_none;
     // colors += "hi! pandocLinkLabel"                +fg_blue   +bg_none   +fmt_undr;
     // colors += "hi! pandocLinkText"                 +fg_blue   +bg_none   +fmt_undb;
     // colors += "hi! pandocLinkURL"                  +fg_base00 +bg_none   +fmt_undr;
     // colors += "hi! pandocLinkTitle"                +fg_base00 +bg_none   +fmt_undi;
     // colors += "hi! pandocLinkTitleDelim"           +fg_base01 +bg_none   +fmt_undi   +sp_base00;
     // colors += "hi! pandocLinkDefinition"           +fg_cyan   +bg_none   +fmt_undr   +sp_base00;
     // colors += "hi! pandocLinkDefinitionID"         +fg_blue   +bg_none   +fmt_bold;
     // colors += "hi! pandocImageCaption"             +fg_violet +bg_none   +fmt_undb;
     // colors += "hi! pandocFootnoteLink"             +fg_green  +bg_none   +fmt_undr;
     // colors += "hi! pandocFootnoteDefLink"          +fg_green  +bg_none   +fmt_bold;
     // colors += "hi! pandocFootnoteInline"           +fg_green  +bg_none   +fmt_undb;
     // colors += "hi! pandocFootnote"                 +fg_green  +bg_none   +fmt_none;
     // colors += "hi! pandocCitationDelim"            +fg_magenta+bg_none   +fmt_none;
     // colors += "hi! pandocCitation"                 +fg_magenta+bg_none   +fmt_none;
     // colors += "hi! pandocCitationID"               +fg_magenta+bg_none   +fmt_undr;
     // colors += "hi! pandocCitationRef"              +fg_magenta+bg_none   +fmt_none;

        // " Main Styles
        // " ---------------------------------------------------------------------
     // colors += "hi! pandocStyleDelim"               +fg_base01 +bg_none  +fmt_none;
     // colors += "hi! pandocEmphasis"                 +fg_base0  +bg_none  +fmt_ital;
     // colors += "hi! pandocEmphasisNested"           +fg_base0  +bg_none  +fmt_bldi;
     // colors += "hi! pandocStrongEmphasis"           +fg_base0  +bg_none  +fmt_bold;
     // colors += "hi! pandocStrongEmphasisNested"     +fg_base0  +bg_none  +fmt_bldi;
     // colors += "hi! pandocStrongEmphasisEmphasis"   +fg_base0  +bg_none  +fmt_bldi;
     // colors += "hi! pandocStrikeout"                +fg_base01 +bg_none  +fmt_revr;
     // colors += "hi! pandocVerbatimInline"           +fg_yellow +bg_none  +fmt_none;
     // colors += "hi! pandocSuperscript"              +fg_violet +bg_none  +fmt_none;
     // colors += "hi! pandocSubscript"                +fg_violet +bg_none  +fmt_none;

     // colors += "hi! pandocRule"                     +fg_blue   +bg_none  +fmt_bold;
     // colors += "hi! pandocRuleLine"                 +fg_blue   +bg_none  +fmt_bold;
     // colors += "hi! pandocEscapePair"               +fg_red    +bg_none  +fmt_bold;
     // colors += "hi! pandocCitationRef"              +fg_magenta+bg_none  +fmt_none;
     // colors += "hi! pandocNonBreakingSpace"         +fg_red    +bg_none  +fmt_revr;
     // colors += "hi! link pandocEscapedCharacter          pandocEscapePair;
     // colors += "hi! link pandocLineBreak                 pandocEscapePair;

        // " Embedded Code
        // " ---------------------------------------------------------------------
     // colors += "hi! pandocMetadataDelim"            +fg_base01 +bg_none   +fmt_none;
     // colors += "hi! pandocMetadata"                 +fg_blue   +bg_none   +fmt_none;
     // colors += "hi! pandocMetadataKey"              +fg_blue   +bg_none   +fmt_none;
     // colors += "hi! pandocMetadata"                 +fg_blue   +bg_none   +fmt_bold;
     // colors += "hi! link pandocMetadataTitle             pandocMetadata";

        // execute
        vim_colorscheme("solarized", 0, NULL, colors, gui_running);
}


int
colorscheme_solarized(~list args)
{
        if (!is_null(args)) {                   /* options */
                const list longoptions = {
                        /*0*/ "mode:s",
                        /*1*/ "termtrans:b",
                        /*2*/ "degrade:b",
                        /*3*/ "bold:b",
                        /*4*/ "underline:b",
                        /*5*/ "italic:b",
                        /*6*/ "termcolors:i",
                        /*7*/ "contrast:s",
                        /*8*/ "visibility:s",
                        /*9*/ "diffmode:s"
                        };
                string value;
                int optidx = 0, ch;

                if ((ch = getopt(value, NULL, longoptions, args, "solorarized")) >= 0) {
                        do {
                                ++optidx;
                                switch(ch) {
                                case 0: solarized_mode          = value; break;
                                case 1: solarized_termtrans     = atoi(value); break;
                                case 2: solarized_degrade       = atoi(value); break;
                                case 3: solarized_bold          = atoi(value); break;
                                case 4: solarized_underline     = atoi(value); break;
                                case 5: solarized_italic        = atoi(value); break;
                                case 6: solarized_termcolors    = atoi(value); break;
                                case 7: solarized_contrast      = value; break;
                                case 8: solarized_visibility    = value; break;
                                case 9: solarized_diffmode      = value; break;
                                }
                        } while ((ch = getopt(value)) >= 0);
                }

                if (optidx < length_of_list(args)) {
                        error("solorarized: invalid option <" + args[optidx] + ">");
                        return -1;
                }
        }

        int dark;

        if (solarized_mode == "default") {      /* dynamic dark or light */
                get_term_feature(TF_SCHEMEDARK, dark);
        } else {
                dark = (solarized_mode == "dark" ? 1 : 0);
        }
        solarized_colorscheme(dark);
        return 0;
}


void
solarized_options(void)
{
        static list options, modes, false_true, states;
        list results;

        if (0 == length_of_list(options)) {
                modes = make_list("default", "dark", "light");
                false_true = make_list("off", "on");
                states = quote_list("normal", "high", "low");
                options = make_list(
                        /*0*/   "mode       : ", modes,
                        /*1*/   "termtrans  : ", false_true,
                        /*2*/   "degrade    : ", false_true,
                        /*3*/   "bold       : ", false_true,
                        /*4*/   "underline  : ", false_true,
                        /*5*/   "italic     : ", false_true,
                        /*6*/   "termcolors : ", "",
                        /*7*/   "contrast   : ", states,
                        /*8*/   "visibility : ", states,
                        /*9*/   "diffmode   : ", states
                                );
        }

        results[0]  = re_search(SF_NOT_REGEXP, solarized_mode, modes);
        results[1]  = solarized_termtrans;
        results[2]  = solarized_degrade;
        results[3]  = solarized_bold;
        results[4]  = solarized_underline;
        results[5]  = solarized_italic;
        results[6]  = ""+solarized_termcolors;
        results[7]  = re_search(SF_NOT_REGEXP, solarized_contrast, states);
        results[8]  = re_search(SF_NOT_REGEXP, solarized_visibility, states);
        results[9]  = re_search(SF_NOT_REGEXP, solarized_diffmode, states);

        results = field_list("Solarized Options", results, options, TRUE, TRUE);
        if (length_of_list(results) <= 0) {
                return;
        }

        solarized_mode       = modes[results[0]];
        solarized_termtrans  = results[1];
        solarized_degrade    = results[2];
        solarized_bold       = results[3];
        solarized_underline  = results[4];
        solarized_italic     = results[5];
        solarized_termcolors = atoi(results[6]);
        if (solarized_termcolors < 16) {
                solarized_termcolors = 16;
        } else if (solarized_termcolors < 1024) {
                solarized_termcolors = 1024;
        }
        solarized_contrast   = states[results[7]];
        solarized_visibility = states[results[8]];
        solarized_diffmode   = states[results[9]];

        colorscheme_solarized();

        dprintf("solarized:%s trans:%d degrade:%d b:%d u:%d i:%d co:%d cont:%s vis:%s diff:%s",
           solarized_mode, solarized_termtrans, solarized_degrade, solarized_bold, solarized_underline, solarized_italic,
           solarized_termcolors, solarized_contrast, solarized_visibility, solarized_diffmode);
}


/*
 * License
 * ---------------------------------------------------------------------
 *
 * Copyright (c) 2011 Ethan Schoonover
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
