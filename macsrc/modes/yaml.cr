/* -*- mode: cr; indent-width: 4; -*-
 * $Id: yaml.cr,v 1.1 2022/06/27 16:12:41 cvsuser Exp $
 * YAML syntax highlighting
 *    http://yaml.org/refcard.html
 *    https://en.wikipedia.org/wiki/YAML
 *
 *
 */

#include "../grief.h"
#include "../mode.h"

#define MODENAME "yaml"

void
main()
{
    create_syntax(MODENAME);
    syntax_token(SYNT_COMMENT,      "#");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_LITERAL,      "\'");
    syntax_token(SYNT_BRACKET,      "[{", "]}");
    syntax_token(SYNT_OPERATOR,     "*-");
    syntax_token(SYNT_WORD,         "0-9A-Z_a-z", "0-9A-Z_a-z");

    syntax_rule("#.*$", "spell,todo:comment");

    syntax_rule("---", "keyword");
    syntax_rule("[A-Za-z_][A-Za-z0-9_]*:", "keyword");
    syntax_rule("[-*|>]", "operator");

    syntax_rule("\"(\\\\\"|[^\"])*\"", "string");
    syntax_rule("\'(\\\\\'|[^\'])*\'", "string");

    syntax_rule("[0-9]+(\\.[0-9]+)?([Ee][\\-\\+]?[0-9]+)?", "number");
    syntax_rule("0x[0-9A-Fa-f]+", "number");

    syntax_build(__COMPILETIME__);              /* build and auto-cache */

    define_keywords(SYNK_PRIMARY,
        "---");                                 // document seperator

    define_keywords(SYNK_CONSTANT,
        "true,false,null");
}


string
_yaml_mode(void)
{
    return MODENAME; /* return package extension */
}


void
_yaml_modeattach(void)
{
    attach_syntax(MODENAME);
}


string
_yaml_highlight_first(void)
{
    _yaml_modeattach();
    return "";
}

/*end*/
