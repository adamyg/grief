/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: distinguished.cr,v 1.4 2024/08/04 11:42:44 cvsuser Exp $
 * distinguished coloriser, GRIEF port.
 *
 * Original:
 *  Author: Kim Silkebækken <kim.silkebaekken+vim@gmail.com>
 *  Source repository: https://github.com/Lokaltog/vim-distinguished
 *
 */

#include "../grief.h"

static list
distinguished_specs = {

        //[0]                              |[1]    |[2]    |[3]              | || |[4]      |[5]       |[6]              |
        //|Highlight group                 |  CTFG |  CTBG |    CTAttributes | || |   GUIFG |    GUIBG |   GUIAttributes |
        //|--------------------------------|-------|-------|-----------------| || |---------|----------|-----------------|

        { "Normal"                         ,    231,     16,             NULL,      "ffffff",  "000000",             NULL },
        { "Visual"                         ,    240,    253,             NULL,      "585858",  "dadada",             NULL },

        { "Cursor"                         ,   NULL,   NULL,             NULL,      "ffffff",  "dd4010",             NULL },
        { "lCursor"                        ,   NULL,   NULL,             NULL,      "ffffff",  "89b6e2",             NULL },

        { "CursorLine"                     ,   NULL,    236,             NULL,          NULL,  "3a3a3a",             NULL },
        { "CursorLineNr"                   ,    231,    240,             NULL,      "ffffff",  "585858",             NULL },
        { "CursorColumn"                   ,    231,    237,             NULL,      "ffffff",  "3a3a3a",             NULL },

        { "Folded"                         ,    249,    234,             NULL,      "b2b2b2",  "1c1c1c",             NULL },
        { "FoldColumn"                     ,    243,    234,             NULL,      "767676",  "1c1c1c",             NULL },
        { "SignColumn"                     ,    231,    233,           "bold",      "ffffff",  "121212",           "bold" },
        { "ColorColumn"                    ,    NULL,   233,             NULL,          NULL,  "262626",             NULL },

        { "StatusLine"                     ,    231,    236,           "bold",      "ffffff",  "303030",           "bold" },
        { "StatusLineNC"                   ,    244,    232,             NULL,      "808080",  "080808",             NULL },

        { "LineNr"                         ,    243,    235,             NULL,      "767676",  "262626",             NULL },
        { "VertSplit"                      ,    240,   NULL,             NULL,      "585858",  "1c1c1c",             NULL },

        { "WildMenu"                       ,    234,    231,             NULL,      "1c1c1c",  "ffffff",             NULL },
        { "Directory"                      ,    143,   NULL,           "bold",      "afaf5f",      NULL,           "bold" },
        { "Underlined"                     ,    130,   NULL,             NULL,      "af5f00",      NULL,             NULL },

        { "Question"                       ,     74,   NULL,           "bold",      "5fafd7",      NULL,           "bold" },
        { "MoreMsg"                        ,    214,   NULL,           "bold",      "ffaf00",      NULL,           "bold" },
        { "WarningMsg"                     ,    202,   NULL,           "bold",      "ff5f00",      NULL,           "bold" },
        { "ErrorMsg"                       ,    196,   NULL,           "bold",      "ff0000",      NULL,           "bold" },

        { "Comment"                        ,    243,    233,             NULL,      "767676",  "121212",             NULL },
        { "vimCommentTitleLeader"          ,    250,    233,             NULL,      "bcbcbc",  "121212",             NULL },
        { "vimCommentTitle"                ,    250,    233,             NULL,      "bcbcbc",  "121212",             NULL },
        { "vimCommentString"               ,    245,    233,             NULL,      "8a8a8a",  "121212",             NULL },

        { "TabLine"                        ,    231,    238,             NULL,      "ffffff",  "444444",             NULL },
        { "TabLineSel"                     ,    255,   NULL,           "bold",      "eeeeee",      NULL,           "bold" },
        { "TabLineFill"                    ,    240,    238,             NULL,      "585858",  "444444",             NULL },
        { "TabLineNumber"                  ,    160,    238,           "bold",      "d70000",  "444444",           "bold" },
        { "TabLineClose"                   ,    245,    238,           "bold",      "8a8a8a",  "444444",           "bold" },

        { "SpellCap"                       ,    231,     31,           "bold",      "ffffff",  "0087af",           "bold" },

        { "SpecialKey"                     ,    239,   NULL,             NULL,      "4e4e4e",      NULL,             NULL },
        { "NonText"                        ,     88,   NULL,             NULL,      "870000",      NULL,             NULL },
        { "MatchParen"                     ,    231,     25,           "bold",      "ffffff",  "005faf",           "bold" },

        { "Constant"                       ,    137,   NULL,           "bold",      "af875f",      NULL,           "bold" },
        { "Special"                        ,    150,   NULL,             NULL,      "afd787",      NULL,             NULL },
        { "Identifier"                     ,     66,   NULL,           "bold",      "5f8787",      NULL,           "bold" },
        { "Statement"                      ,    186,   NULL,           "bold",      "d7d787",      NULL,           "bold" },
        { "PreProc"                        ,    247,   NULL,             NULL,      "9e9e9e",      NULL,             NULL },
        { "Type"                           ,     67,   NULL,           "bold",      "5f87af",      NULL,           "bold" },
        { "String"                         ,    143,   NULL,             NULL,      "afaf5f",      NULL,             NULL },
        { "Number"                         ,    173,   NULL,             NULL,      "d7875f",      NULL,             NULL },
        { "Define"                         ,    173,   NULL,             NULL,      "d7875f",      NULL,             NULL },
        { "Error"                          ,    208,    124,             NULL,      "ff8700",  "af0000",             NULL },
        { "Function"                       ,    179,   NULL,             NULL,      "d7af5f",      NULL,             NULL },
        { "Include"                        ,    173,   NULL,             NULL,      "d7875f",      NULL,             NULL },
        { "PreCondit"                      ,    173,   NULL,             NULL,      "d7875f",      NULL,             NULL },
        { "Keyword"                        ,    173,   NULL,             NULL,      "d7875f",      NULL,             NULL },
        { "Search"                         ,    231,    131,             NULL,      "000000",  "ffff5f", "underline,bold" },
        { "Title"                          ,    231,   NULL,             NULL,      "ffffff",      NULL,             NULL },
        { "Delimiter"                      ,    246,   NULL,             NULL,      "949494",      NULL,             NULL },
        { "StorageClass"                   ,    187,   NULL,             NULL,      "d7d7af",      NULL,             NULL },
        { "Operator"                       ,    180,   NULL,             NULL,      "d7af87",      NULL,             NULL },

        { "TODO"                           ,    228,     94,           "bold",      "ffff87",  "875f00",           "bold" },

        { "SyntasticWarning"               ,    220,     94,             NULL,      "ffff87",  "875f00",           "bold" },
        { "SyntasticError"                 ,    202,     52,             NULL,      "ffff87",  "875f00",           "bold" },

        { "Pmenu"                          ,    248,    240,             NULL,      "a8a8a8",  "585858",             NULL },
        { "PmenuSel"                       ,    253,    245,             NULL,      "dadada",  "8a8a8a",             NULL },
        { "PmenuSbar"                      ,    253,    248,             NULL,      "dadada",  "a8a8a8",             NULL },

        { "phpEOL"                         ,    245,   NULL,             NULL,      "dadada",      NULL,             NULL },
        { "phpStringDelim"                 ,     94,   NULL,             NULL,      "875f00",      NULL,             NULL },
        { "phpDelimiter"                   ,    160,   NULL,             NULL,      "d70000",      NULL,             NULL },
        { "phpFunctions"                   ,    221,   NULL,           "bold",      "ffd75f",      NULL,           "bold" },
        { "phpBoolean"                     ,    172,   NULL,           "bold",      "d78700",      NULL,           "bold" },
        { "phpOperator"                    ,    215,   NULL,             NULL,      "ffaf5f",      NULL,             NULL },
        { "phpMemberSelector"              ,    138,   NULL,           "bold",      "af8787",      NULL,           "bold" },
        { "phpParent"                      ,    227,   NULL,             NULL,      "ffff5f",      NULL,             NULL },

        { "PHPClassTag"                    ,    253,   NULL,             NULL,      "dadada",      NULL,             NULL },
        { "PHPInterfaceTag"                ,    253,   NULL,             NULL,      "dadada",      NULL,             NULL },
        { "PHPFunctionTag"                 ,    222,   NULL,           "bold",      "ffd787",      NULL,           "bold" },

        { "pythonDocString"                ,    240,    233,             NULL,      "585858",  "121212",             NULL },
        { "pythonDocStringTitle"           ,    245,    233,             NULL,      "dadada",  "121212",             NULL },
        { "pythonRun"                      ,     65,   NULL,             NULL,      "5f875f",      NULL,             NULL },
        { "pythonBuiltinObj"               ,     67,   NULL,           "bold",      "5f87af",      NULL,           "bold" },
        { "pythonSelf"                     ,    250,   NULL,           "bold",      "bcbcbc",      NULL,           "bold" },
        { "pythonFunction"                 ,    179,   NULL,           "bold",      "d7af5f",      NULL,           "bold" },
        { "pythonClass"                    ,    221,   NULL,           "bold",      "ffd75f",      NULL,           "bold" },
        { "pythonExClass"                  ,    130,   NULL,             NULL,      "af5f00",      NULL,             NULL },
        { "pythonException"                ,    130,   NULL,           "bold",      "af5f00",      NULL,           "bold" },
        { "pythonOperator"                 ,    186,   NULL,             NULL,      "d7d787",      NULL,             NULL },
        { "pythonPreCondit"                ,    152,   NULL,           "bold",      "afd7d7",      NULL,           "bold" },
        { "pythonDottedName"               ,    166,   NULL,             NULL,      "d75f00",      NULL,             NULL },
        { "pythonDecorator"                ,    124,   NULL,           "bold",      "af0000",      NULL,           "bold" },

        { "PythonInterfaceTag"             ,    109,   NULL,             NULL,      "87afaf",      NULL,             NULL },
        { "PythonClassTag"                 ,    221,   NULL,             NULL,      "ffd75f",      NULL,             NULL },
        { "PythonFunctionTag"              ,    109,   NULL,             NULL,      "87afaf",      NULL,             NULL },
        { "PythonVariableTag"              ,    253,   NULL,             NULL,      "dadada",      NULL,             NULL },
        { "PythonMemberTag"                ,    145,   NULL,             NULL,      "afafaf",      NULL,             NULL },

        { "CTagsImport"                    ,    109,   NULL,             NULL,      "87afaf",      NULL,             NULL },
        { "CTagsClass"                     ,    221,   NULL,             NULL,      "ffd75f",      NULL,             NULL },
        { "CTagsFunction"                  ,    109,   NULL,             NULL,      "87afaf",      NULL,             NULL },
        { "CTagsGlobalVariable"            ,    253,   NULL,             NULL,      "dadada",      NULL,             NULL },
        { "CTagsMember"                    ,    145,   NULL,             NULL,      "afafaf",      NULL,             NULL },

        { "xmlTag"                         ,    149,   NULL,           "bold",      "afd75f",      NULL,           "bold" },
        { "xmlTagName"                     ,    250,   NULL,             NULL,      "bcbcbc",      NULL,             NULL },
        { "xmlEndTag"                      ,    209,   NULL,           "bold",      "ff875f",      NULL,           "bold" },

        { "cssImportant"                   ,    166,   NULL,           "bold",      "d75f00",      NULL,           "bold" },

        { "DiffAdd"                        ,    112,     22,             NULL,      "87d700",  "005f00",             NULL },
        { "DiffChange"                     ,    220,     94,             NULL,      "ffd700",  "875f00",             NULL },
        { "DiffDelete"                     ,    160,   NULL,             NULL,      "d70000",      NULL,             NULL },
        { "DiffText"                       ,    220,     94,   "reverse,bold",      "ffd700",  "875f00",   "reverse,bold" },

        { "diffLine"                       ,     68,   NULL,           "bold",      "5f87d7",      NULL,           "bold" },
        { "diffFile"                       ,    242,   NULL,             NULL,      "6c6c6c",      NULL,             NULL },
        { "diffNewFile"                    ,    242,   NULL,             NULL,      "6c6c6c",      NULL,             NULL }
        };


void
colorscheme_distinguished(void)
{
    list distinguished_colors = {
        "set background=dark",
        "hi clear"
        };
    list spec;

    while (list_each(distinguished_specs, spec) >= 0) {
        string val;

        val = "hi " + spec[0];
        if (typeof(spec[1]) != "NULL") val += format(" ctermfg=%d", spec[1]);
        if (typeof(spec[2]) != "NULL") val += format(" ctermbg=%d", spec[2]);
        if (typeof(spec[3]) != "NULL") val += format(  " cterm=%s", spec[3]);
        if (typeof(spec[4]) != "NULL") val += format( " guifg=#%s", spec[4]);
        if (typeof(spec[5]) != "NULL") val += format( " guibg=#%s", spec[5]);
        if (typeof(spec[6]) != "NULL") val += format(    " gui=%s", spec[6]);
        distinguished_colors += val;
    }

    vim_colorscheme("distinguished", 0, NULL, distinguished_colors, -1);
}
/*end*/
