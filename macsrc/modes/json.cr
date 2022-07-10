/* -*- mode: cr; indent-width: 4; -*-
 * $Id: json.cr,v 1.1 2022/07/10 13:08:02 cvsuser Exp $
 * json mode.
 *
 *
 */

#include "../grief.h"

#define MODENAME "json"

void
main()
{
    create_syntax(MODENAME);

    //  Syntax lexer/
    //      utilised during basic line pre-processing.
    //
    syntax_token(SYNT_COMMENT,      "/*", "*/");
    syntax_token(SYNT_COMMENT,      "//");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_BRACKET,      "[{", "]}");
    syntax_token(SYNT_OPERATOR,     ",");

    set_syntax_flags(SYNF_COMMENTS_CSTYLE);

    //  DFA based syntax engine/
    //      used to built a DFA based lexer/parser, which are generally faster.
    //
    syntax_rule("\".*\"[ \t]*:", "keyword");

    syntax_rule("(true)|(false)|(null)", "quick:constant"); // special words
    syntax_rule("[A-Za-z][A-Za-z0-9_]*", "alert"); // unexpected bare words.

    syntax_rule("[()\\[\\]{}]", "delimiter");
    syntax_rule(",", "operator");

    syntax_rule("\"(\\\\.|[^\\\"])*\"", "string");

    syntax_rule("[0-9]+(\\.[0-9]*)?([Ee][-+]?[0-9]*)?", "number");
    syntax_rule("[0-9]+", "number");

    syntax_rule("/\\*.*$", "spell,todo:comment");
    syntax_rule("/\\*.*\\*/", "spell,todo,quick:comment");
    syntax_rule("//.*$", "spell,todo:comment");

    syntax_rule("/[^*/].*$", "alert");          // invalid eol comment.
    syntax_rule("\\*/", "quick:alert");         // unmatched block comment.

    // string elements
    syntax_rule("\\\\u\\x\\x\\x\\x",            // universal character name, escapes.
        "group=string,quick:constant_standout");

    syntax_rule("\\\\U\\x\\x\\x\\x\\x\\x\\x\\x",
        "group=string,quick:constant_standout");

    syntax_rule("\\\\[uU][A-Za-z0-9]+",         // illegal unicode character escapes.
        "group=string:alert");

    syntax_rule("\\\\",                         // omitted character escapes.
        "group=string:alert");

    syntax_build(__COMPILETIME__);              /* build and auto-cache */
}


string
_json_mode()
{
    return MODENAME;                            /* return package extension */
}


string
_json_highlight_first()
{
    attach_syntax(MODENAME);                    /* attach colorizer */
    return "";
}

/*end*/

