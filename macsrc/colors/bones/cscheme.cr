/* -*- mode: cr; indent-width: 4; -*- */
// $Id: cscheme.cr,v 1.7 2024/10/27 06:09:51 cvsuser Exp $
// zenbones coloriser collection, GriefEdit port - beta.
//
// Usage:
//
//    colorscheme bones-zen[bones] [options]
//
// Collection:
//
//    Colorscheme       Description
//    ------------------------------------------------------------
//    zenwritten        Zero hue and saturation version
//    neobones          Inspired by neovim.io
//    vimbones          Inspired by vim.org
//    rosebones         Inspired by Rose Pine
//    forestbones       Inspired by Everforest
//    nordbones         Inspired by Nord
//    tokyobones        Inspired by Tokyo Night
//    seoulbones        Inspired by Seoul256
//    duckbones         Inspired by Spaceduck
//    zenbones          Inspired by Zenburn
//    kanagawabones     Inspired by Kanagawa
//    ------------------------------------------------------------
//
// Options:
//    --colors=<depeth>
//    --mode=dark|light
//    --light
//    --dark
//

#include "../../grief.h"
#include "../util/rgbmap.h"

static list bones_list = {
    "zenwritten    - Zero hue and saturation version",
    "neobones      - Inspired by neovim.io",
    "vimbones      - Inspired by vim.org",
    "rosebones     - Inspired by Roses Pine",
    "forestbones   - Inspired by Everforest",
    "nordbones     - Inspired by Nord",
    "tokyobones    - Inspired by Tokyo Night",
    "seoulbones    - Inspired by Seoul256",
    "duckbones     - Inspired by Spaceduck",
    "zenbones      - Inspired by Zenburn",
    "kanagawabones - Inspired by Kanagawa",
    };


void
main(void)
{
    require("colors/util/rgbmap");              // RRGGBB support
    module("csbones");
}


string
colorschemepkg_bones(~string scheme, ~list args)
{
    string package = "bones";
    int colordepth = -1, dark = -1;

    // Options
    if (! is_null(args)) {
        /* options */
        const list longoptions = {
            /*0*/ "colors:n+",
            /*1*/ "mode:s",                     // "dark" or "light"
            /*2*/ "dark",                       // mode=dark [shortcut]
            /*3*/ "light"                       // mode=light [shortcut]
            };
        int optidx = 0, ch;
        string value;

        if ((ch = getopt(value, NULL, longoptions, args, package)) >= 0) {
            ++optidx;
            do {
                switch(ch) {
                case 0: // colordepth
                    colordepth = atoi(value);
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
            error("%s: unexpected option <%s>", package, args[optidx]);
            return "";
        }
    }

    if (colordepth <= 0) {
         get_term_feature(TF_COLORDEPTH, colordepth);
    }

    if (colordepth < 256 && colordepth != 16 && colordepth != 88) {
        error("%s: color depth not supported", package);
        return "";
    }

    if (dark == -1) {
        get_term_feature(TF_SCHEMEDARK, dark);  // default
    }

    // Resolve package-name

    string pkgname = scheme;

    if (pkgname == "") {                        // prompt selection
        const int sel = select_list("Bones", "Select Scheme", 1, bones_list, SEL_NORMAL);

        if (sel <= 0) {
            return "";
        }

        pkgname = bones_list[sel - 1];
        pkgname = substr(pkgname, 1, index(pkgname, " ") - 1);
        scheme  = re_translate(SF_BRIEF, "bones$", "", pkgname);

    } else {
        if (strrstr(pkgname, "bones") == 0 )    // short-hand, "zen" = "zenbones"
            if (strrstr(pkgname, "written") == 0)
            {
                pkgname += "bones";
            }
    }

    // Load package

    if (require("colors/bones/" + pkgname) >= 0) {
        string name = inq_module() + "::" + pkgname;
        int ret = execute_macro(name, dark);
        if (ret == 0) {
            return "bones-" + scheme;           // return package
        }
        error("%s: scheme not supported '%s'", package, scheme);
    } else {
        error("%s: unavailable to locate '%s'", package, scheme);
    }
    return "";
}


void
bones_scheme_set(list x)
{
    extern string scheme;
    extern int colordepth;

    //
    // Load scheme
    //
//  if (colordepth > 256) {                     // leverage gui colors
        vim_colorscheme(scheme, 0, NULL, x, TRUE);
        return;
//  }

    //
    // Map RGB colors to terminal
    //
    string line, word;
    list c;

    while (list_each(x, line) >= 0) {
        const list words = tokenize(line, " ", TOK_TRIM);

        switch (words[0]) {
        case "hi":
        case "hi!":
        case "highlight":
        case "highlight!":       // hi[ghtlight] command
            if (length_of_list(words) > 2) {
                int ctermfg = 1, ctermbg = 1;

                if (re_search(NULL, "^ctermfg=", words) >= 0) {
                    ctermfg = 0;                // available omit mapping
                }

                if (re_search(NULL, "^ctermbg=", words) >= 0) {
                    ctermbg = 0;                // available omit mapping
                }

                if (ctermfg || ctermbg) {
                    line = "";                  // build line

                    while (list_each(words, word) >= 0) {
                        //
                        //  guifg={color}
                        //      Color terminal foreground color.
                        //
                        //  guibg={color}
                        //      Color terminal background color.
                        //
                        const list parts = tokenize(word, "=", TOK_TRIM);

                        if (length_of_list(parts) == 2) {
                            line += parts[0] + "=" + parts[1];

                            if (strcasecmp(parts[1], "NONE")) {
                                switch (parts[0]) {
                                case "guifg":
                                    if (ctermfg) {
                                        line += " ctermfg=" + RGBMap(parts[1], colordepth);
                                    }
                                    break;
                                case "guibg":
                                    if (ctermbg) {
                                        line += " ctermbg=" + RGBMap(parts[1], colordepth);
                                    }
                                    break;
                                default:
                                    break;
                                }
                            }

                        } else {
                            line += word;
                        }
                        line += " ";
                    }
                    c += rtrim(line);
                }
            }
            break;
        default:
            break;
        }
        c += line;
    }

    vim_colorscheme(scheme, 0, NULL, c, -1);
}

//end
