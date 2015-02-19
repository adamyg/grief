/* -*- mode: cr; indent-width: 4; -*-
 * $Id: java.cr,v 1.11 2014/10/27 23:28:33 ayoung Exp $
 * 'java' programming language mode.
 *
 *
 *
 */

#include "../grief.h"

static list java_hier_list =
    {
    "Action                 Operator(s)",
    "call, member           (),[]",
    "negation/increment     ! ~- ++ --",
    "multiply/divide        * / %",
    "addition/subtraction   + -",
    "bitwise shift          << >> >>>",
    "comparison             < <= > >=",
    "equality               == !=",
    "bitwiseAND             &",
    "bitwise XOR            ^",
    "bitwise OR             |",
    "logical AND            &&",
    "logical OR             ||",
    "conditional            ?:",
    "assignment             = += -= *= /= %= <<= >>= >>>= &= ^= |=",
    "comma                  ,"
    };

#define MODENAME        "JAVA"

void
main()
{
    create_syntax(MODENAME);

    syntax_token(SYNT_COMMENT,      "/*", "*/");
    syntax_token(SYNT_COMMENT,      "//");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_PREPROCESSOR, "#");
    syntax_token(SYNT_CHARACTER,    "\'");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");
    syntax_token(SYNT_OPERATOR,     "-%+/&*=<>|!~^");
    syntax_token(SYNT_DELIMITER,    ",;.?:");
    syntax_token(SYNT_WORD,         "0-9a-zA-Z_");
    syntax_token(SYNT_NUMERIC,      "-+0-9a-fA-F.xXL");

    set_syntax_flags(SYNF_COMMENTS_CSTYLE);

    define_keywords(SYNK_PRIMARY,   "doif", 2);
    define_keywords(SYNK_PRIMARY,   "forintnewtry", 3);
    define_keywords(SYNK_PRIMARY,   "bytecasecharelselongthisvoid", 4);
    define_keywords(SYNK_PRIMARY,   "breakcatchclassfinalfloatshortsuperthrowwhile", 5);
    define_keywords(SYNK_PRIMARY,   "doubleimportnativepublicreturnstaticswitchthrows", 6);
    define_keywords(SYNK_PRIMARY,   "booleandefaultextendsfinallypackageprivatevirtual", 7);
    define_keywords(SYNK_PRIMARY,   "abstractcontinuevolatile", 8);
    define_keywords(SYNK_PRIMARY,   "interfaceprotected", 9);
    define_keywords(SYNK_PRIMARY,   "implementsinstanceof", 10);
    define_keywords(SYNK_PRIMARY,   "synchronized", 12);

    define_keywords(SYNK_PRIMARY,   "var", 3);
    define_keywords(SYNK_PRIMARY,   "castgotonullresttrue", 4);
    define_keywords(SYNK_PRIMARY,   "constfalseinnerouter", 5);
    define_keywords(SYNK_PRIMARY,   "future", 6);
    define_keywords(SYNK_PRIMARY,   "byvaluegeneric", 7);
    define_keywords(SYNK_PRIMARY,   "finalizeoperator", 8);
    define_keywords(SYNK_PRIMARY,   "transient", 9);
}


/*
 *  Modeline/package support
 */
string
_java_mode()
{
    return "java";                              /* return package extension */
}


list
_java_hier_list()
{
    return java_hier_list;
}


string
_java_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}

/*end*/
