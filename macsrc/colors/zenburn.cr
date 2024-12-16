/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: zenburn.cr,v 1.16 2024/12/14 10:04:23 cvsuser Exp $
 * zenburn coloriser, GriefEdit port.
 *
 * References:
 *  https://en.wikipedia.org/wiki/Wikipedia:Zenburn
 *  https://kippura.org/zenburnpage/
 *
 */

#include "../grief.h"

static list
zenburn_spec[] = {

        //[0]   |[1]                            |[2]    |[3]    |[4]                |[5]        |[6]        |[7]                |[8]            |
        //Mode  |Highlight group                | CTFG  | CTBG  | CTAttributes      | GUIFG     | GUIBG     | GUIAttributes     |GUISubscript   |
        //------|-------------------------------|-------|-------|-------------------|-----------|-----------|-------------------|---------------|

            // High contrast

        { 2|4,  "Normal",                       188,    234,    NULL,               "#d7d7d7",  "#1f1f1f",  NULL,               NULL            },
        { 2,    "NonText",                      238,    NULL,   NULL,               "#404040",  NULL,       "bold",             NULL            },
        { 2,    "CursorLine",                   NULL,   233,    NULL,               NULL,       "#121212",  "bold",             NULL            },
        { 2,    "CursorColumn",                 NULL,   235,    NULL,               NULL,       "#2b2b2b",  NULL,               NULL            },
        { 2,    "LineNr",                       248,    233,    NULL,               "#9fafaf",  "#161616",  NULL,               NULL            },
        { 2,    "Visual",                       236,    210,    "bold",             "#333333",  "#0f0f0f",  "bold,underline",   NULL            },
        { 2,    "VisualNOS",                    236,    210,    "bold",             "#333333",  "#0f0f0f",  "bold,underline",   NULL            },

            // Normal contrast

        { 1|4,  "Normal",                       188,    237,    NULL,               "#d7d7d7",  "#3a3a3a",  NULL,               NULL            },
        { 1,    "NonText",                      240,    NULL,   NULL,               "#5b605e",  NULL,       "bold",             NULL            },
        { 1,    "CursorLine",                   NULL,   238,    NULL,               NULL,       "#434443",  NULL,               NULL            },
        { 1,    "CursorColumn",                 NULL,   239,    NULL,               NULL,       "#4f4f4f",  NULL,               NULL            },
        { 1,    "LineNr",                       248,    233,    NULL,               "#9fafaf",  "#262626",  NULL,               NULL            },
        { 1,    "Visual",                       235,    32,     "bold",             "#333333",  "#0087d7",  "bold",             NULL            },
        { 1,    "VisualNOS",                    235,    32,     "bold",             "#333333",  "#0087d7",  "bold",             NULL            },

            // Context-less

        { 0,    "Boolean",                      181,    NULL,   NULL,               "#dca3a3",  NULL,       NULL,               NULL            },
        { 0,    "Character",                    181,    NULL,   "bold",             "#dca3a3",  NULL,       "bold",             NULL            },
        { 0,    "Comment",                      108,    NULL,   NULL,               "#7f9f7f",  NULL,       "italic",           NULL            },
        { 0,    "Conditional",                  223,    NULL,   "bold",             "#f0dfaf",  NULL,       "bold",             NULL            },
        { 0,    "Constant",                     181,    NULL,   "bold",             "#dca3a3",  NULL,       "bold",             NULL            },
        { 0,    "Cursor",                       233,    109,    "bold",             "#000d18",  "#8faf9f",  "bold",             NULL            },
        { 0,    "Debug",                        181,    NULL,   "bold",             "#bca3a3",  NULL,       "bold",             NULL            },
        { 0,    "Define",                       223,    NULL,   "bold",             "#ffcfaf",  NULL,       "bold",             NULL            },
        { 0,    "Delimiter",                    245,    NULL,   NULL,               "#8f8f8f",  NULL,       NULL,               NULL            },
        { 0,    "DiffAdd",                      66,     237,    "bold",             "#709080",  "#313c36",  "bold",             NULL            },
        { 0,    "DiffChange",                   NULL,   236,    NULL,               NULL,       "#333333",  NULL,               NULL            },
        { 0,    "DiffDelete",                   236,    238,    NULL,               "#333333",  "#464646",  NULL,               NULL            },
        { 0,    "DiffText",                     217,    237,    "bold",             "#ecbcbc",  "#41363c",  "bold",             NULL            },
        { 0,    "Directory",                    109,    NULL,   "bold",             "#9fafaf",  NULL,       "bold",             NULL            },
        { 0,    "ErrorMsg",                     115,    236,    "bold",             "#80d4aa",  "#2f2f2f",  "bold",             NULL            },
        { 0,    "Exception",                    249,    NULL,   "bold",             "#c3bf9f",  NULL,       "bold",             NULL            },
        { 0,    "Float",                        251,    NULL,   NULL,               "#c0bed1",  NULL,       NULL,               NULL            },
        { 0,    "FoldColumn",                   109,    233,    NULL,               "#93b3a3",  "#3f4040",  NULL,               NULL            },
        { 0,    "Folded",                       109,    233,    NULL,               "#93b3a3",  "#3f4040",  NULL,               NULL            },
        { 0,    "Function",                     228,    NULL,   NULL,               "#efef8f",  NULL,       NULL,               NULL            },
        { 0,    "Identifier",                   223,    NULL,   NULL,               "#efdcbc",  NULL,       NULL,               NULL            },
        { 0,    "IncSearch",                    228,    238,    NULL,               "#385f38",  "#f8f893",  NULL,               NULL            },
        { 0,    "Keyword",                      223,    NULL,   "bold",             "#f0dfaf",  NULL,       "bold",             NULL            },
        { 0,    "Label",                        187,    NULL,   "underline",        "#dfcfaf",  NULL,       "underline",        NULL            },
        { 0,    "Macro",                        223,    NULL,   "bold",             "#ffcfaf",  NULL,       "bold",             NULL            },
        { 0,    "ModeMsg",                      223,    NULL,   "none",             "#ffcfaf",  NULL,       "none",             NULL            },
        { 0,    "MoreMsg",                      15,     NULL,   "bold",             "#ffffff",  NULL,       "bold",             NULL            },
        { 0,    "Number",                       116,    NULL,   NULL,               "#8cd0d3",  NULL,       NULL,               NULL            },
        { 0,    "Operator",                     230,    NULL,   NULL,               "#f0efd0",  NULL,       NULL,               NULL            },
        { 0,    "PreCondit",                    180,    NULL,   "bold",             "#dfaf8f",  NULL,       "bold",             NULL            },
        { 0,    "PreProc",                      223,    NULL,   "bold",             "#ffcfaf",  NULL,       "bold",             NULL            },
        { 0,    "Question",                     15,     NULL,   "bold",             "#ffffff",  NULL,       "bold",             NULL            },
        { 0,    "Repeat",                       223,    NULL,   "bold",             "#ffd7a7",  NULL,       "bold",             NULL            },
        { 0,    "Search",                       230,    236,    NULL,               "#ffffe0",  "#284f28",  NULL,               NULL            },
        { 0,    "Special",                      181,    NULL,   NULL,               "#cfbfaf",  NULL,       NULL,               NULL            },
        { 0,    "SpecialChar",                  181,    NULL,   "bold",             "#dca3a3",  NULL,       "bold",             NULL            },
        { 0,    "SpecialComment",               181,    NULL,   "bold",             "#82a282",  NULL,       "bold",             NULL            },
        { 0,    "SpecialKey",                   151,    NULL,   NULL,               "#9ece9e",  NULL,       NULL,               NULL            },
        { 0,    "Spell",                        108,    NULL,   "underline",        "#dc8c6c",  NULL,       "underline",        NULL            },
        { 0,    "SpellBad",                     9,      237,    "undercurl",        "#dc8c6c",  NULL,       "undercurl",        "#bc6c4c"       },
        { 0,    "SpellCap",                     12,     237,    NULL,               "#8c8cbc",  NULL,       NULL,               "#6c6c9c"       },
        { 0,    "SpellLocal",                   14,     237,    NULL,               "#9ccc9c",  NULL,       NULL,               "#7cac7c"       },
        { 0,    "SpellRare",                    13,     237,    NULL,               "#bc8cbc",  NULL,       NULL,               "#bc6c9c"       },
        { 0,    "Statement",                    187,    234,    "none",             "#e3ceab",  NULL,       "none",             NULL            },
        { 0,    "StatusLine",                   236,    186,    NULL,               "#313633",  "#ccdc90",  NULL,               NULL            },
        { 0,    "StatusLineNC",                 235,    108,    NULL,               "#2e3330",  "#88b090",  NULL,               NULL            },
        { 0,    "StorageClass",                 249,    NULL,   "bold",             "#c3bf9f",  NULL,       "bold",             NULL            },
        { 0,    "String",                       174,    NULL,   NULL,               "#cc9393",  NULL,       NULL,               NULL            },
        { 0,    "Structure",                    229,    NULL,   "bold",             "#efefaf",  NULL,       "bold",             NULL            },
        { 0,    "Tag",                          181,    NULL,   "bold",             "#e89393",  NULL,       "bold",             NULL            },
        { 0,    "Title",                        7,      234,    "bold",             "#efefef",  NULL,       "bold",             NULL            },
        { 0,    "Todo",                         108,    234,    "bold",             "#dfdfdf",  "bg",       "bold",             NULL            },
        { 0,    "Type",                         187,    NULL,   "bold",             "#dfdfbf",  NULL,       "bold",             NULL            },
        { 0,    "Typedef",                      253,    NULL,   "bold",             "#dfe4cf",  NULL,       "bold",             NULL            },
        { 0,    "Underlined",                   188,    234,    "underline" ,       "#dcdccc",  NULL,       "underline",        NULL            },
    //  { 0,    "VertSplit",                    236,    65,     NULL,               "#2e3330",  "#688060",  NULL,               NULL            },
        { 0,    "Frame",  /*GRIEF*/             188,    237,    NULL,               "#2e3330",  "#688060",  NULL,               NULL            },

        { 0,    "WarningMsg",                   15,     236,    "bold",             "#ffffff",  "#333333",  "bold",             NULL            },
        { 0,    "WildMenu",                     236,    194,    "bold",             "#cbecd0",  "#2c302d",  "underline",        NULL            },

        { 0,    "Error",                        228,    95,     "bold",             "#e37170",  "#3d3535",  NULL,               NULL            },
        { 0,    "HTMLLink",                     181,    NULL,   "underline",        NULL,       NULL,       NULL,               NULL            },

            // High contrast

    //  { 2,    "Include",                                      NULL,               "#dfaf8f",  NULL,       "bold",             NULL            },
    //  { 2,    "Ignore",                       238             NULL,               "#545a4f",  NULL,       NULL,               NULL            },
        { 2,    "PMenu",                        248,    0,      NULL,               "#ccccbc",  "#242424",  NULL,               NULL            },
        { 2,    "PMenuSel",                     223,    235,    NULL,               "#ccdc90",  "#353a37",  "bold",             NULL            },
    //  { 2,    "PmenuSbar                                      NULL,               "#000000",  "#2e3330",  NULL,               NULL            },
    //  { 2,    "PMenuThumb                                     NULL,               "#040404",  "#a0afa0",  NULL,               NULL            },
    //  { 2,    "MatchParen                                     NULL,               "#f0f0c0",  "#383838",  "bold"              NULL            },
    //  { 2,    "SignColumn                                     NULL,               "#9fafaf",  "#181818",  "bold"              NULL            },
        { 2,    "FoldColumn",                   109,    233,    NULL,               "#161616",  NULL,       NULL,               NULL            },
        { 2,    "Folded",                       109,    233,    NULL,               "#161616",  NULL,       NULL,               NULL            },
        { 2,    "TabLine",                      108,    236,    "none",             "#88b090",  "#313633",  "none",             NULL            },
        { 2,    "TabLineSel",                   186,    235,    "bold",             "#ccd990",  "#222222",  NULL,               NULL            },
        { 2,    "TabLineFill",                  236,    236,    NULL,               "#88b090",  "#313633",  "none",             NULL            },
        { 2,    "SpecialKey",                   152,    NULL,   NULL,               NULL,       "#242424",  NULL,               NULL            },

            // Normal contrast

    //  { 1,    "Include",                                      NULL,               "#dfaf8f",  NULL,       "bold",             NULL            },
    //  { 1,    "Ignore",                       240             NULL,               "#545a4f",  NULL,       NULL,               NULL            },
        { 1,    "PMenu",                        248,    0,      NULL,               "#9f9f9f",  "#2c2e2e",  NULL,               NULL            },
        { 1,    "PMenuSel",                     223,    235,    NULL,               "#d0d0a0",  "#242424",  "bold",             NULL            },
    //  { 1,    "PmenuSbar                                      NULL,               "#000000",  "#2e3330",  NULL,               NULL            },
    //  { 1,    "PMenuThumb                                     NULL,               "#040404",  "#a0afa0",  NULL,               NULL            },
    //  { 1,    "MatchParen                                     NULL,               "#b2b2a0",  "#2e2e2e",  "bold",             NULL            },
    //  { 1,    "SignColumn                                     NULL,               "#9fafaf",  "#343434",  "bold",             NULL            },
        { 1,    "FoldColumn",                   109,    236,    NULL,               NULL,       "#333333",  NULL,               NULL            },
        { 1,    "Folded",                       109,    236,    NULL,               NULL,       "#333333",  NULL,               NULL            },
        { 1,    "TabLine",                      187,    235,    "none",             "#d0d0b8",  "#222222",  "none",             NULL            },
        { 1,    "TabLineSel",                   229,    236,    "bold",             "#f0f0b0",  "#333333",  "bold",             NULL            },
        { 1,    "TabLineFill",                  233,    233,    NULL,               "#dccdcc",  "#101010",  "none",             NULL            },
        { 1,    "SpecialKey",                   151,    NULL,   NULL,               NULL,       "#444444",  NULL,               NULL            },

            // Links

//TODO
//      { -1,   NULL,                           "Class", "Function" },
//      { -1,   NULL,                           "Import", "PythonInclude" },
//      { -1,   NULL,                           "Member", "Function" },
//      { -1,   NULL,                           "GlobalVariable", "Normal" },
//      { -1,   NULL,                           "GlobalConstant", "Constant" },
//      { -1,   NULL,                           "EnumerationValue", "Float" },
//      { -1,   NULL,                           "EnumerationName", "Identifier" },
//      { -1,   NULL,                           "DefinedName", "WarningMsg" },
//      { -1,   NULL,                           "LocalVariable", "WarningMsg" },
//      { -1,   NULL,                           "Structure", "WarningMsg" },
//      { -1,   NULL,                           "Union", "WarningMsg" },
        };

int
colorscheme_zenburn(~list args)
{
    const string scheme = "zenburn";
    int contrast = 1;                           // normal=1, high=2
    int colordepth = -1;
    int black = 0;

    // options
    if (! is_null(args)) {
        const list longoptions = {
            /*0*/ "colors:n+",                  // color depth
            /*1*/ "contrast:s",                 // "normal", "high"
            /*2*/ "black_bg",                   // black background
            };
        int optidx = 0, ch;
        string value;

        if ((ch = getopt(value, NULL, longoptions, args, scheme)) >= 0) {
            do {
                ++optidx;
                switch(ch) {
                case 0: // colordepth
                    colordepth = atoi(value);
                    break;
                case 1: // contrast=normal|high
                    if (value == "normal") {
                        contrast = 1;
                    } else if (value == "high") {
                        contrast = 2;
                    } else {
                        error("%s: invalid contrast <%s>", scheme, value);
                        return -1;
                    }
                    break;
                case 2: // black_bg
                    black = 1;
                    break;
                default:
                    error("%s: %s", scheme, value);
                    return -1;
                }
            } while ((ch = getopt(value)) >= 0);
        }

        if (optidx < length_of_list(args)) {
            if (args[optidx] == "show") {
                message("%s: by %s, GriefEdit version", scheme, "Jonathan Filip");
            } else {
                error("%s: unexpected option <%s>", scheme, args[optidx]);
            }
            return -1;
        }
    }

    if (colordepth <= 0) {
         get_term_feature(TF_COLORDEPTH, colordepth);
    }
    if (colordepth < 256) {
        error("%s: color depth not supported", scheme);
        return -1;
    }

    // build
    list zenburn_colors = {
        "set background=dark",
        "hi clear",
        "syntax reset"
        };
    list spec;

    while (list_each(zenburn_spec, spec) >= 0) {
        const int mode = spec[0];
        string val;

        if (mode == 0 || (mode & contrast)) {
            //
            //  colors
            //
            val = "hi " + spec[1];
            if (typeof(spec[2]) != "NULL") val += format(" ctermfg=%d", spec[2]);
            if (typeof(spec[3]) != "NULL") {
                if (0 == (mode & 4) || 0 == black) { // black background
                    val += format(" ctermbg=%d", spec[3]);
                }
            }
            if (typeof(spec[4]) != "NULL") val += format(  " cterm=%s", spec[4]);
            if (typeof(spec[5]) != "NULL") val += format(  " guifg=%s", spec[5]);
            if (typeof(spec[6]) != "NULL") {
                if (0 == (mode & 4) || 0 == black) { // black background
                    val += format( " guibg=%s", spec[6]);
                }
            }
            if (typeof(spec[7]) != "NULL") val += format(    " gui=%s", spec[7]);
            zenburn_colors += val;

        } else if (mode == -1) {
            //
            //  links
            //
            val = "hi link " + spec[1] + " " + spec[2];
            zenburn_colors += val;
        }
    }

    vim_colorscheme("zenburn", 256, NULL, zenburn_colors, -1);
}

/*end*/
