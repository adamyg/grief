/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: cr.cr,v 1.16 2022/07/10 13:08:02 cvsuser Exp $
 * GRIEF/Crisp syntax definition mode.
 *
 *
 */

#include "../grief.h"
#include "../mode.h"

#define MODENAME "grief"

void
main(void)
{
    string chars = "abcedfghijklmnopqrstuvwxyz";
    list kwl;
    string kw;
    int ll, l, klen;
    int dflag;

    /*
     *  Syntax lexer/
     *      utilised during basic line pre-processing.
     */
    create_syntax(MODENAME);
    syntax_token(SYNT_COMMENT,      "/*", "*/");
    syntax_token(SYNT_COMMENT,      "//");
    syntax_token(SYNT_CHARACTER,    "\'");
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_LINECONT,     "\\");
    syntax_token(SYNT_PREPROCESSOR, "#");
    syntax_token(SYNT_BRACKET,      "([{", ")]}");
    syntax_token(SYNT_WORD,         "0-9a-zA-Z_");

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
     *  Advanced syntax engine/
     *      Used to built a DFA based lexer/parser, which are generally faster.
     */
    syntax_rule("^[ \t]*#", "preproc:preprocessor");

                                                // comments (open, block, block-unmatched and eol)
    syntax_rule("/\\*.*$", "spell,todo:comment");
    syntax_rule("/\\*.*\\*/", "spell,todo,quick:comment");
    syntax_rule("//.*$", "spell,todo:comment");

    syntax_rule("/[^*/].*$", "alert");          // invalid eol comment.
    syntax_rule("\\*/", "quick:alert");         // unmatched block comment.

                                                // keywords and preprocessor directives
    syntax_rule("[A-Za-z_][A-Za-z_0-9]*", "keyword,directive:normal");

                                                // numeric constants
    syntax_rule("[0-9]+(\\.[0-9]*)?([Ee][-+]?[0-9]*)?", "number");
    syntax_rule("0[xX][0-9A-Fa-f]+[LlUu]*", "number");
    syntax_rule("[0-9]+[LlUu]*", "number");

                                                // strings (block, open/continued)
    syntax_rule("\"(\\\\.|[^\\\"])*\"", "string");
    syntax_rule("\"(\\\\.|[^\\\\\"])*[^\\\\ \\\"\t\n]+", "string");

    syntax_rule("\'\\\\[\\\']\'", "character"); // '\\', '\''
    syntax_rule("\'[^\']+\'", "character");     // 'x[xxxx]'

    // delimiters/operators
    syntax_rule("[()\\[\\]{},;.?:]", "delimiter");
    syntax_rule("[-%" + "+/&*=<>|!~^]+", "operator");

    syntax_rule("\\\\[ \t]+$", "whitespace");   // trailing white-space after continuation

    // string elements
    syntax_rule("\\\\x[A-Za-z0-9]+",            // hexadecimal escapes.
        "group=string:operator");

    syntax_rule("\\\\x[^A-Za-z0-9]",            // illegal hexadecimal escapes.
        "group=string:alert");

    syntax_rule("\\\\u[A-Za-z0-9]+",            // unicode character escapes.
        "group=string:operator");

    syntax_rule("\\\\u[^A-Za-z0-9]+",           // illegal unicode character escapes.
        "group=string:alert");

    syntax_rule("\\\\[0-7]+",                   // octal escapes.
        "group=string:operator");

    syntax_rule("\\\\['\"?\\\\abefnrtv]",       // character escapes (includes known extensions).
        "group=string:operator");

    syntax_rule("\\\\[^'\"?\\\\abefnrtvux0]",   // illegal character escapes.
        "group=string:alert");

    syntax_rule("\\\\",                         // omitted character escapes.
        "group=string:alert");

    syntax_rule("%%",                           // escaped format.
        "group=string,quick:operator");

    syntax_rule("%" + "\\[[^\\]]\\]",           // regular expressions, character-set.
        "group=string:operator");
                                                // format specifications (includes c99 and 'W' extensions).
    syntax_rule("%" + "[-+ #0']?[0-9]*\\.[0-9]*[hlL]*[bdiuoxXDOUfeEgGcCsSWpn]",
        "group=string,quick:operator");

    syntax_rule("%" + "[-+ #0']?[0-9]*[hlL]*[bdiuoxXDOUfeEgGcCsSWpn]",
        "group=string,quick:operator");

    syntax_rule("%" + "[-+ #0']?[*]?\\.[*]?[hlL]*[bdiuoxXDOUfeEgGcCsSWpn]",
        "group=string,quick:operator");

    syntax_rule("%" + "[-+ #0']?[*]?[hlL]*[bdiuoxXDOUfeEgGcCsSWpn]",
        "group=string,quick:operator");

    syntax_rule("%" + "[^ \"]+",                // non-standard character escapes.
        "group=string:alert");

    syntax_build(__COMPILETIME__);              // build and auto-cache.

    /*
     *  Keywords, plus crunch (only) keywords.
     */
    dflag = debug(-1);
    if (dflag) debug(0);                        // disable debug, result is far 2 verbose.

    kwl = command_list(TRUE);                   // keywords only.
    if ((ll = length_of_list(kwl)) > 0)
        for (l = 0; l < ll; l++) {
            kw = kwl[l];
            klen = strlen(kw);
            if (klen > 2 || (klen == 2 && index(chars, substr(kw, 1, 1)))) {
                define_keywords(SYNK_PRIMARY, kw, klen);
            }
        }

    /* crunch only */
    define_keywords(SYNK_PRIMARY,       "case,else,enum,void", -4);
    define_keywords(SYNK_PRIMARY,       "local", -5);
    define_keywords(SYNK_PRIMARY,       "static", -6);
    define_keywords(SYNK_PRIMARY,       "default", -7);
    define_keywords(SYNK_PRIMARY,       "catch,finally");

    /* pre-preprocessor */
    define_keywords(SYNK_PREPROCESSOR,  "line", -4);
    define_keywords(SYNK_PREPROCESSOR,  "error,undef", -5);
    define_keywords(SYNK_PREPROCESSOR,  "pragma", -6);
    define_keywords(SYNK_PREPROCESSOR,  "defined", -7);
    define_keywords(SYNK_PREPROCESSOR,
        "__DATE__,__FILE__,__FUNCTION__,__LINE__,__TIME__");
    define_keywords(SYNK_PREPROCESSOR_DEFINE, "define", -6);
    define_keywords(SYNK_PREPROCESSOR_INCLUDE, "include", -7);
    define_keywords(SYNK_PREPROCESSOR_CONDITIONAL, "if", -2);
    define_keywords(SYNK_PREPROCESSOR_CONDITIONAL, "elif,else", -4);
    define_keywords(SYNK_PREPROCESSOR_CONDITIONAL, "endif,ifdef,undef", -5);
    define_keywords(SYNK_PREPROCESSOR_CONDITIONAL, "ifndef", -6);
    define_keywords(SYNK_PREPROCESSOR_CONDITIONAL, "defined", -7);

    define_keywords(SYNK_CONSTANT,
        "__COMPILETIME__,__PROTOTYPES__,GRUNCH,GRUNCH_VERSION");

    define_keywords(SYNK_TODO,
        "XXX,TODO,FIXME,DEPRECATED,MAGIC,HACK");

    if (dflag) debug(dflag, FALSE);             // restore debug.

    /*
     *  prereq's
     */
    load_indent();
    load_compile();
}


/*
 *  Modeline/package support
 */
string
_cr_mode(void)
{
    extern string _c_mode(void);

    _c_mode();                                  /* make sure it is loaded */
    return "cr";
}


string
_cr_highlight_first(void)
{
    attach_syntax(MODENAME);
    return "";
}


list
_cr_hier_list(void)
{
    extern list _c_hier_list(void);

    return _c_hier_list();
}


string
_cr_template_first(void)
{
    extern string _c_template_first(void);

    return _c_template_first();
}


string
_cr_smart_first(void)
{
    extern string _c_smart_first(void);

    return _c_smart_first();
}

/*end*/
