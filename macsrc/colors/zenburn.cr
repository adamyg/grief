/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: zenburn.cr,v 1.7 2022/08/10 15:44:58 cvsuser Exp $
 * zenburn coloriser, GRIEF port.
 *
 *  Original author Jani Numinen in 2002
 *  see http://slinky.imukuppi.org/zenburnpage
 *
 *
 */

#include "../grief.h"

static list
zenburn_spec[] = {

        //[0]                               |[1]    |[2]    |[3]              | || |[4]      |[5]       |[6]              |
        //|Highlight group                  |  CTFG |  CTBG |    CTAttributes | || |   GUIFG |    GUIBG |   GUIAttributes |
        //|---------------------------------|-------|-------|-----------------| || |---------|----------|-----------------|

        { "Boolean",                        181,    NULL,   NULL                },
        { "Character",                      181,    NULL,   "bold"              },
        { "Comment",                        108,    NULL,   NULL                },
        { "Conditional",                    223,    NULL,   "bold"              },
        { "Constant",                       181,    NULL,   "bold"              },
        { "Cursor",                         233,    109,    "bold"              },
        { "Debug",                          181,    NULL,   "bold"              },
        { "Define",                         223,    NULL,   "bold"              },
        { "Delimiter",                      245,    NULL,   NULL                },
        { "DiffAdd",                        66,     237,    "bold"              },
        { "DiffChange",                     236,    NULL,   NULL                },
        { "DiffDelete",                     236,    238,    NULL                },
        { "DiffText",                       217,    237,    "bold"              },
        { "Directory",                      109,    NULL,   "bold"              },
        { "ErrorMsg",                       115,    236,    "bold"              },
        { "Exception",                      249,    NULL,   "bold"              },
        { "Float",                          251,    NULL,   NULL                },
        { "Function",                       228,    NULL,   NULL                },
        { "Identifier",                     223,    NULL,   NULL                },
        { "IncSearch",                      228,    238,    NULL                },
        { "Keyword",                        223,    NULL,   "bold"              },
        { "Label",                          187,    NULL,   "underline"         },
        { "LineNr",                         248,    233,    NULL                },
        { "Macro",                          223,    NULL,   "bold"              },
        { "ModeMsg",                        223,    NULL,   "none"              },
        { "MoreMsg",                        15,     NULL,   "bold"              },
        { "Number",                         116,    NULL,   NULL                },
        { "Operator",                       230,    NULL,   NULL                },
        { "PreCondit",                      180,    NULL,   "bold"              },
        { "PreProc",                        223,    NULL,   "bold"              },
        { "Question",                       15,     NULL,   "bold"              },
        { "Repeat",                         223,    NULL,   "bold"              },
        { "Search",                         230,    236,    NULL                },
        { "SpecialChar",                    181,    NULL,   "bold"              },
        { "SpecialComment",                 181,    NULL,   "bold"              },
        { "Special",                        181,    NULL,   NULL                },
        { "SpecialKey",                     151,    NULL,   NULL                },
        { "Statement",                      187,    234,    "none"              },
        { "StatusLine",                     236,    186,    NULL                },
        { "StatusLineNC",                   235,    108,    NULL                },
        { "StorageClass",                   249,    NULL,   "bold"              },
        { "String",                         174,    NULL,   NULL                },
        { "Structure",                      229,    NULL,   "bold"              },
        { "Tag",                            181,    NULL,   "bold"              },
        { "Title",                          7,      234,    "bold"              },
        { "Todo",                           108,    234,    "bold"              },
        { "Typedef",                        253,    NULL,   "bold"              },
        { "Type",                           187,    NULL,   "bold"              },
        { "Underlined",                     188,    234,    "bold"              },
        { "VertSplit",                      236,    65,     NULL                },
        { "VisualNOS",                      236,    210,    "bold"              },
        { "WarningMsg",                     15,     236,    "bold"              },
        { "WildMenu",                       236,    194,    "bold"              },
//      { "SpellBad",                       9,      237,    NULL                },
//      { "SpellCap",                       12,     237,    NULL                },
//      { "SpellLocal",                     14,     237,    NULL                },
//      { "SpellRare",                      13,     237,    NULL                },
        { "Spell",                          108,    NULL,   "underline"         },

        { "PMenu",                          248,    0,      NULL                },
        { "PMenuSel",                       223,    235,    NULL                },

//      { "TabLine",                        187,    235,    "none"              },
//      { "TabLineSel",                     229,    236,    "bold"              },
//      { "TabLineFill",                    188,    233,    "none"              },

        { "HTMLLink",                       181,    NULL,   "underline"         },

        { "Error",                          167,    236,    "bold"              },
//      { "Include",
//      { "Label",                                                              },
//      { "Ignore",                                                             },

        // Tag support
//      { NULL,                             "Class", "Function",                NULL },
//      { NULL,                             "Import", "PythonInclude",          NULL },
//      { NULL,                             "Member", "Function",               NULL },
//      { NULL,                             "GlobalVariable", "Normal",         NULL },
//      { NULL,                             "GlobalConstant", "Constant",       NULL },
//      { NULL,                             "EnumerationValue", "Float",        NULL },
//      { NULL,                             "EnumerationName", "Identifier",    NULL },
//      { NULL,                             "DefinedName", "WarningMsg",        NULL },
//      { NULL,                             "LocalVariable", "WarningMsg",      NULL },
//      { NULL,                             "Structure", "WarningMsg",          NULL },
//      { NULL,                             "Union", "WarningMsg",              NULL }
        };

void
colorscheme_zenburn(void)
{
    list zenburn_colors = {
        "set background=dark",
        "hi clear"
        };
    list spec;

    while (list_each(zenburn_spec, spec) >= 0) {
        string val;

        if (typeof(spec[1]) == "NULL") {
            // links
            val = "hi link " + spec[1] + " " + spec[2];

        } else {
            // colors
            val = "hi " + spec[0];
            if (typeof(spec[1]) != "NULL") val += format(" ctermfg=%d", spec[1]);
            if (typeof(spec[2]) != "NULL") val += format(" ctermbg=%d", spec[2]);
            if (typeof(spec[3]) != "NULL") val += format(  " cterm=%s", spec[3]);
//          if (typeof(spec[4]) != "NULL") val += format( " guifg=#%s", spec[4]);
//          if (typeof(spec[5]) != "NULL") val += format( " guibg=#%s", spec[5]);
//          if (typeof(spec[6]) != "NULL") val += format(    " gui=%s", spec[6]);
        }
        zenburn_colors += val;
    }

    vim_colorscheme("zenburn", 256, NULL, zenburn_colors, -1);
}

/*end*/
