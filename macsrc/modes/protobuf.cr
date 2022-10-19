/* -*- mode: cr; indent-width: 4; -*-
 * $Id: protobuf.cr,v 1.1 2022/06/26 15:38:14 cvsuser Exp $
 * 'protobuf' programming languages mode.
 *
 *
 */

#include "../grief.h"

#define MODENAME "protobuf"

void
main()
{
    create_syntax(MODENAME);

    syntax_token(SYNT_COMMENT,      "//");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_LITERAL,      "\'");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");
    syntax_token(SYNT_DELIMITER,    ",;:");
    syntax_token(SYNT_OPERATOR,     "=");
    syntax_token(SYNT_WORD,         "0-9A-Z_a-z");
    syntax_token(SYNT_NUMERIC,      "-+.0-9_xa-fA-F");

    define_keywords(SYNK_PRIMARY,
        "enum,"+
        "extend,"+
        "extensions,"+
        "import,"+
        "map,"+
        "message,"+
        "oneof,"+
        "option,"+
        "optional,"+
        "package,"+
        "public,"+
        "repeated,"+
        "required,"+
        "reserved,"+
        "returns,"+
        "rpc,"+
        "service,"+
        "stream"
        );

    define_keywords(SYNK_TYPE,
        "double,"+
        "float,"+
        "int32,"+
        "int64,"+
        "uint32,"+
        "uint64,"+
        "sint32,"+
        "sint64,"+
        "fixed32,"+
        "fixed64,"+
        "sfixed32,"+
        "sfixed64,"+
        "bool,"+
        "string,"+
        "bytes"
        );

    set_syntax_flags(SYNF_HILITE_LINECONT);
}


/*
 *  Modeline support
 */
string
_protobuf_mode()
{
    return MODENAME;
}


/*
 *  Package support
 */
string
_protobuf_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}

/*end*/
