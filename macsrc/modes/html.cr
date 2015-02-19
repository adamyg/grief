/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: html.cr,v 1.11 2014/10/22 02:34:33 ayoung Exp $
 * HTML support and indenting mode.
 *
 *
 */

#include "../grief.h"

#define MIN_ABBREV 1

int         _html_min_abbrev,
            _html_keyboard,
            _sgml_keyboard;

#define MODENAME "html"


void
main()
{
    create_syntax(MODENAME);

    syntax_token(SYNT_COMMENT,      "<!-", "-->");
    syntax_token(SYNT_PREPROCESSOR, "#");
    syntax_token(SYNT_HTML,         "<", ">");
    syntax_token(SYNT_BRACKET,      "<", ">");
    syntax_token(SYNT_WORD,         "A-Za-z&");

    /* keywords */
    define_keywords(0,
        "&gt&lt", 3);

    define_keywords(0,
        "&ETH&amp&eth", 4);

    define_keywords(0,
        "&Auml&Euml&Iuml&Ouml&Uuml&auml&euml&iuml&nbsp&ouml&quot"+
        "&uuml&yuml", 5);

    define_keywords(0,
        "&AElig&Acirc&Aring&Ecirc&Icirc&Ocirc&THORN&Ucirc&acirc"+
        "&aelig&aring&ecirc&icirc&ocirc&szlig&thorn&ucirc", 6);

    define_keywords(0,
        "&Aacute&Agrave&Atilde&Ccedil&Eacute&Egrave&Iacute&Igrave"+
        "&Ntilde&Oacute&Ograve&Oslash&Otilde&Uacute&Ugrave&Yacute"+
        "&aacute&agrave&atilde&ccedil&eacute&egrave&iacute&igrave"+
        "&ntilde&oacute&ograve&oslash&otilde&uacute&ugrave&yacute", 7);

    /* Template editor keyboard */
    load_indent();
    keyboard_push();
    assign_to_key("<Tab>",          "_slide_in");
    assign_to_key("<Shift-Tab>",    "_slide_out");
    assign_to_key("<<>",            "_html_electric_open");
    assign_to_key("<\">",           "_html_electric_dquote");
    assign_to_key("<'>",            "_html_electric_quote");
    assign_to_key("<Space>",        "_html_abbrev");
    _sgml_keyboard = inq_keyboard();
    assign_to_key("<>>",            "_html_abbrev");
    _html_keyboard = inq_keyboard();
    keyboard_pop(1);
}


/*
 * Modeline/package support
 */
string
_html_mode()
{
    return "html";                              /* return package extension */
}


string
_html_highlight_first()
{
    attach_syntax(MODENAME);
    return "";
}


string
_html_template_first()
{
    _html_min_abbrev = MIN_ABBREV;
    use_local_keyboard(_html_keyboard);
    return "";
}


string
_html_smart_first()
{
    return _html_template_first();
}


string
_html_regular_first()
{
    return _html_template_first();
}


void
_html_electric_open()
{
    insert("<>");
    move_rel(0, -1);
}


void
_html_electric_quote()
{
    insert("''");
    move_rel(0, -1);
}


void
_html_electric_dquote()
{
    insert("\"\"");
    move_rel(0, -1);
}


static list
html_abbrev_list =
{
    /* match, completioNULL, rel col shift, rel line shift */
    /* Template */
    "HTML",       "HTML>\n<HEAD>\n<TITLE></TITLE>\n<H1></H1>\n</HEAD>\n<BODY>\n</BODY>\n</HTML>\n", 7, -6,

    /* Sections */
    "HEAD",       "HEAD>\n<TITLE></TITLE>\n</HEAD>",  7, -2,
    "BODY",       "BODY>\n\n</BODY>",  0, -2,

    /* Structure */
    "ADDRESS",    "ADDRESS></ADDRESS>", 0, -10,
    "BLOCKQUOTE", "BLOCKQUOTE>\n\n</BLOCKQUOTE>", 0, -2,
    "CODE",       "CODE>\n</CODE>\n", 0, -2,
    "FORM",       "FORM ACTION=\"\">\n</FORM>\n", 14, -2,
    "SELECT",     "SELECT NAME=\"\">\n</SELECT>\n", 14, -2,
    "H1",         "H1></H1>", -5, 0,
    "H2",         "H2></H2>", -5, 0,
    "H3",         "H3></H3>", -5, 0,
    "H4",         "H4></H4>", -5, 0,
    "H5",         "H5></H5>", -5, 0,
    "H6",         "H6></H6>", -5, 0,
    "PRE",        "PRE>\n\n</PRE>", -6, -1,
    "P",          "P>\n\n</P>", -4, -1,
    "HR",         "HR>\n", 0, 0,

    /* Lists */
    "DL",         "DL>\n<DT></DT>\n<DD></DD>\n</DL>\n", 4, -3,
    "OL",         "OL>\n<LI></LI>\n</OL>\n", 4, -2,
    "UL",         "UL>\n<LI></LI>\n</UL>\n", 4, -2,
    "DIR",        "DIR>\n<LI></LI>\n</DIR>\n", 4, -2,
    "MENU",       "MENU>\n<LI></LI>\n</MENU>\n", 4, -2,
    "DD",         "DD></DD>", -5, 0,
    "DT",         "DT></DT>", -5, 0,
    "LI",         "LI></LI>", -5, 0,

    /* Misc */
    "A H",        "A HREF=\"\"></A>", -6, 0,
    "A N",        "A NAME=\"\"></A>", -6, 0,
    "AHREF",      "A HREF=\"\"></A>", -6, 0,
    "ANAME",      "A NAME=\"\"></A>", -6, 0,
    "BR",         "BR>\n", 0, 0,
    "IMG",        "IMG SRC=\"\">", -2, 0,

    /* Effects */
    "B",          "B></B>", -4, 0,
    "I",          "I></I>", -4, 0,
    "U",          "U></U>", -4, 0,
    "TT",         "TT></TT>", -5, 0,
    "EM",         "EM></EM>", -5, 0,
    "BIG",        "BIG></BIG>", -6, 0,
    "KBD",        "KBD></KBD>", -6, 0,
    "VAR",        "VAR></VAR>", -6, 0,
    "CITE",       "CITE></CITE>", -7, 0,
    "CODE",       "CODE></CODE>", -7, 0,
    "SAMP",       "SAMP></SAMP>", -7, 0,
    "SMALL",      "SMALL></SMALL>", -8, 0,
    "STRONG",     "STRONG></STRONG>", -9, 0,
    "CENTER",     "CENTER></CENTER>", -9, 0,

    /* Comment */
    "!--",        "!--  --!>", -5, 0
};


void
_html_abbrev()
{
    string rd = read(1);

    if (rd == "\n" || rd == ">")
    {
        int cur_line, cur_col;

        inq_position(cur_line, cur_col);
        if (re_search(SF_BACKWARDS|SF_NOT_REGEXP, "<") > 0)
        {
            int c2r_line, c2r_col;

            inq_position(c2r_line, c2r_col);
            if (c2r_line == cur_line && move_rel(0, 1))
            {
                string line = trim(read());
                int len;

                len = index(line, ">");
                if (len > 1)
                    line = substr(line, 1, --len);

                len = strlen(line);
                if (len >= _html_min_abbrev)
                {
                    int loc = 0;
                    int got = -1;
                    int amb = 0;

                    len = length_of_list(html_abbrev_list) / 4;
                    while (loc < len) {
                        int at = loc * 4;

                        if (re_search(SF_NOT_REGEXP|SF_IGNORE_CASE, line, html_abbrev_list[at]) == 1) {
                            if (upper(line) == html_abbrev_list[at]) {
                                /*Exact match gets an immediate hit*/
                                got = loc;
                                amb = 0;
                                break;
                            }
                            if (got != -1)
                                amb = 1;        /* Ambiguous (not enough letters) */
                            got = loc;
                        }
                        loc++;
                    }

                    if (got != -1 && !amb)
                    {
                        int at = got * 4;
                        string completion;

                     /* message("%s->%s", line, html_abbrev_list[at]); */
                        delete_char(cur_col - c2r_col);
                        completion = html_abbrev_list[at + 1];
                        insert(completion);
                        move_rel(html_abbrev_list[at+3], html_abbrev_list[at+2]);
                        return;
                    }
                }
            }
        }
        move_abs(cur_line, cur_col);
    }

    self_insert();
}
