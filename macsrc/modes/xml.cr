/* -*- mode: cr; indent-width: 4; -*-
 * $Id: xml.cr,v 1.1 2022/08/10 13:06:32 cvsuser Exp $
 * xml mode.
 *
 *
 */

#include "../grief.h"

#define MODENAME "xml"

void
main()
{
    create_syntax(MODENAME);

    //  Syntax lexer/
    //      utilised during basic line preprocessing.
    //
    syntax_token(SYNT_COMMENT,      "<!--", "-->");     // open/close, attr=comment
    syntax_token(SYNT_STRING,       "\"");
    syntax_token(SYNT_LITERAL,      "\'");
    syntax_token(SYNT_QUOTE,        "\\");
    syntax_token(SYNT_BRACKET,      "<", ">");          // attr=delimiter
    syntax_token(SYNT_WORD,         "&A-Za-z");

    //  Special Characters in XML/
    //
    //      &lt;        <, less than
    //      &gt;        >, greater than
    //      &amp;       &, ampersand
    //      &apos;      ', apostrophe
    //      &quot;      ", quotation mark
    //
    //  Plus html specials.
    //
    define_keywords(SYNK_PRIMARY,
        "&gt,&lt");

    define_keywords(SYNK_PRIMARY,
        "&ETH,&amp,&eth");

    define_keywords(SYNK_PRIMARY,
        "&Auml,&Euml,&Iuml,&Ouml,&Uuml,&auml,&euml,&iuml,&nbsp,&ouml,&quot,&uuml,&yuml");

    define_keywords(SYNK_PRIMARY,
        "&AElig,&Acirc,&Aring,&Ecirc,&Icirc,&Ocirc,&THORN,&Ucirc,&acirc,&aelig,&aring,&ecirc,&icirc,&ocirc,&szlig,&thorn,&ucirc");

    define_keywords(SYNK_PRIMARY,
        "&Aacute,&Agrave,&Atilde,&Ccedil,&Eacute,&Egrave,&Iacute,&Igrave,&Ntilde,&Oacute,&Ograve,&Oslash,&Otilde,&Uacute,&Ugrave,&Yacute,"+
        "&aacute,&agrave,&atilde,&ccedil,&eacute,&egrave,&iacute,&igrave,&ntilde,&oacute,&ograve,&oslash,&otilde,&uacute,&ugrave,&yacute");

    // TODO/DFA
    //
    //      <\?xml \?>  attr=constant 
    //      <\? \?>     attr=code
    //      < >         attr=tag
    //
}


string
_xml_mode()
{
    return MODENAME;                            /* return package extension */
}


string
_xml_highlight_first()
{
    attach_syntax(MODENAME);                    /* attach colorizer */
    return "";
}

/*end*/


