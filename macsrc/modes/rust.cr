/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: rust.cr,v 1.1 2024/08/01 14:08:10 cvsuser Exp $
 * GRIEF/rust syntax definition mode.
 *
 *
 */

#include "../grief.h"
#include "../mode.h"

#define MODENAME "rust"

void
main(void)
{
    /*
     *  Syntax lexer/
     *      utilised during basic line pre-processing.
     */
    create_syntax(MODENAME);
    syntax_token(SYNT_COMMENT,      "/*", "*/");
    syntax_token(SYNT_COMMENT,      "//");
  //syntax_token(SYNT_CHARACTER,    "\'");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_LINECONT,     "\\");
  //syntax_token(SYNT_PREPROCESSOR, "#");
    syntax_token(SYNT_BRACKET,      "([{<", ")]}>");
    syntax_token(SYNT_KEYWORD,      "a-zA-Z_", "0-9a-zA-Z_");
    syntax_token(SYNT_WORD,         "a-zA-Z_");

    /*
     *  Options/
     *      SYNF_COMMENTS_CSTYLE
     *          Ignore leading white-space on comments.
     *
     *      SYNF_COMMENTS_QUOTE
     *          Allow quoting of comment terminator.
     *
     *      SYNF_LINECONT_WS
     *          Continuation, allows trailing white-space.
     */
    set_syntax_flags(SYNF_COMMENTS_CSTYLE|SYNF_COMMENTS_QUOTE|SYNF_LINECONT_WS);

    /*
     *  Syntax engine rules
     */
                                                // comments (open, block, block-unmatched and eol)
    syntax_rule("/\\*.*$", "spell,todo:comment");
    syntax_rule("/\\*.*\\*/", "spell,todo,quick:comment");
    syntax_rule("//.*$", "spell,todo:comment");

    syntax_rule("\\*/", "quick:alert");         // unmatched block comment.

                                                // keywords and preprocessor directives
    syntax_rule("[A-Za-z_][A-Za-z_0-9]*", "keyword,directive:normal");

                                                // numeric constants
    syntax_rule("[0-9]+(\\.[0-9]*)?([Ee][-+]?[0-9]*)?", "number");
    syntax_rule("0[xX][0-9A-Fa-f]+[LlUu]*", "number");

        // strings
    syntax_rule("\"(\\\\.|[^\\\"])*\"", "string");
    syntax_rule("\"(\\\\.|[^\\\\\"])*[^\\\\ \\\"\t\n]+", "string");

    syntax_rule("r#\"(\\\\.|[^\\\"])*\"#", "string");   // raw-string
    syntax_rule("br#\"(\\\\.|[^\\\"])*\"#", "string");  // raw-byte-string
    syntax_rule("c\"(\\\\.|[^\\\"])*\"#", "string");    // c-string
    syntax_rule("cr\"(\\\\.|[^\\\"])*\"#", "string");   // raw-c-string

        // characters
    syntax_rule("\'[^\']\'", "character");              // character
    syntax_rule("b\'[^\']\'", "character");             // byte
    syntax_rule("\'\\\\[\\\']\'", "character");         // '\\', '\''
    syntax_rule("\'\\u{A-Za-z0-9]+}\'", "character");   // '\u{xxxx}'

        // delimiters/operators
    syntax_rule("[()\\[\\]{},;.?:]", "delimiter");
    syntax_rule("[-%" + "+/&*=<>|!~^]+", "operator");

        // macros
        // +!
        // $+

    syntax_rule("\\\\[ \t]+$", "whitespace");   // trailing white-space after continuation

    // string elements
    syntax_rule("\\\\x[A-Za-z0-9][A-Za-z0-9]",  // hexadecimal escapes \xAA.
        "group=string:operator");

    syntax_rule("\\\\x[^A-Za-z0-9]",            // illegal hexadecimal escapes \xXX.
        "group=string:alert");
    syntax_rule("\\\\x[A-Za-z0-9][^A-Za-z0-9]",
        "group=string:alert");

    syntax_rule("\\\\u{A-Za-z0-9]+}",           // unicode character escapes \u{XXXX}
        "group=string:operator");

    syntax_rule("\\\\u{[^A-Za-z0-9]+}?",        // illegal unicode character escapes.
        "group=string:alert");

    syntax_rule("\\\\[0-7]+",                   // octal escapes.
        "group=string:operator");

    syntax_rule("\\\\[0-7]+",                   // binary escapes.
        "group=string:operator");

    syntax_rule("\\\\['\"?\\\\abefnrtv]",       // character escapes (includes known extensions).
        "group=string:operator");

    syntax_rule("\\\\[^'\"?\\\\abefnrtvux0]",   // illegal character escapes.
        "group=string:alert");

    syntax_rule("\\\\",                         // omitted character escapes.
        "group=string:alert");

    syntax_rule("{{",                           // escaped format.
        "group=string,quick:operator");

    syntax_rule("}}",                           // escaped format.
        "group=string,quick:operator");

    syntax_rule("{[^}]+}",                      // format
        "group=string,quick:operator");

    syntax_build(__COMPILETIME__);              // build and auto-cache.

    /*
     *  Keywords
     *
     *      "as,break,const,continue,crate,else,enum,extern,false,fn,for,if,impl,in,let,loop,match,mod,move,mut,pub,ref,return,self,Self,static,struct,super,trait,true,type,unsafe,use,where,while
     *      "async,await,dyn"
     *
     *      "abstract,become,box,do,final,macro,override,priv,typeof,unsized,virtual,yield"
     *      "try"
     */
    define_keywords(SYNK_PRIMARY,
        "break,continue,crate,extern,for,impl,in,let,mod,pub,return,self,Self,super,trait,unsafe,use,where");
    define_keywords(SYNK_PRIMARY,
        "async,await");

    define_keywords(SYNK_PRIMARY,
        "abstract,become,box,do,final,macro,override,priv,typeof,unsized,virtual,yield");
    define_keywords(SYNK_PRIMARY,
        "try");

        define_keywords(SYNK_STRUCTURE,
            "struct,enum,union,static,dyn");
        define_keywords(SYNK_STORAGECLASS,
            "fn,type");
        define_keywords(SYNK_STORAGECLASS,
            "move,mut,ref,static,const");
        define_keywords(SYNK_CONDITIONAL,
            "match,if,else");
        define_keywords(SYNK_REPEAT,
            "loop,while");
        define_keywords(SYNK_OPERATOR,
            "as");

    define_keywords(SYNK_TYPE,
        "bool,char,isize,usize");
    define_keywords(SYNK_TYPE,
        "i8,i16,i32,i64,u8,u16,u32,u64,isize,usize,f32,f64");

    define_keywords(SYNK_TYPE,
        "str,String,Vec,Option,Result");

    define_keywords(SYNK_BOOLEAN,
        "false,true");
    define_keywords(SYNK_CONSTANT,
        "Some,None,Ok,Err");

    define_keywords(SYNK_TODO,
        "XXX,TODO,FIXME,DEPRECATED,MAGIC,HACK,NB,NOTE,SAFETY");
}


/*
 *  Modeline/package support
 */
string
_rust_mode(void)
{
    return "rs";                                /* return package extension */
}


string
_rust_highlight_first(void)
{
    attach_syntax(MODENAME);                    /* attach colorizer */
    return "";
}

/*end*/

