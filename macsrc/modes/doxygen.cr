/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: doxygen.cr,v 1.5 2014/10/22 02:34:33 ayoung Exp $
 * doxygen markup mode.
 *
 *
 *
 */

#include "../grief.h"

static list keywords = {
    "a",
    "addindex",
    "addtogroup",
    "anchor",
    "arg",
    "attention",
    "author",
    "authors",
    "b",
    "brief",
    "bug",
    "c",
    "callgraph",
    "callergraph",
    "category",
    "cite",
    "class",
    "code",
    "cond",
    "condnot",
    "copybrief",
    "copydetails",
    "copydoc",
    "copyright",
    "date",
    "def",
    "defgroup",
    "deprecated",
    "details",
    "dir",
    "dontinclude",
    "dot",
    "dotfile",
    "e",
    "else",
    "elseif",
    "em",
    "endcode",
    "endcond",
    "enddot",
    "endhtmlonly",
    "endif",
    "endinternal",
    "endlatexonly",
    "endlink",
    "endmanonly",
    "endmsc",
    "endrtfonly",
    "endverbatim",
    "endxmlonly",
    "enum",
    "example",
    "exception",
    "extends",
    "f$",
    "f[",
    "f]",
    "f{",
    "f}",
    "file",
    "fn",
    "headerfile",
    "hideinitializer",
    "htmlinclude",
    "htmlonly",
    "if",
    "ifnot",
    "image",
    "implements",
    "include",
    "includelineno",
    "ingroup",
    "internal",
    "invariant",
    "interface",
    "latexonly",
    "li",
    "line",
    "link",
    "mainpage",
    "manonly",
    "memberof",
    "msc",
    "mscfile",
    "n",
    "name",
    "namespace",
    "nosubgrouping",
    "note",
    "overload",
    "p",
    "package",
    "page",
    "par",
    "paragraph",
    "param",
    "post",
    "pre",
    "private",
    "privatesection",
    "property",
    "protected",
    "protectedsection",
    "protocol",
    "public",
    "publicsection",
    "ref",
    "related",
    "relates",
    "relatedalso",
    "relatesalso",
    "remark",
    "remarks",
    "result",
    "return",
    "returns",
    "retval",
    "rtfonly",
    "sa",
    "section",
    "see",
    "short",
    "showinitializer",
    "since",
    "skip",
    "skipline",
    "snippet",
    "struct",
    "subpage",
    "subsection",
    "subsubsection",
    "tableofcontents",
    "test",
    "throw",
    "throws",
    "todo",
    "tparam",
    "typedef",
    "union",
    "until",
    "var",
    "verbatim",
    "verbinclude",
    "version",
    "warning",
    "weakgroup",
    "xmlonly",
    "xrefitem"+
    "$",
    "@",
    "\\",
    "&",
    "~",
    "<",
    ">",
    "#",
    "%",
    "\"",
    ".",
    "::",

    // commands included for Qt compatibility ---
    //  The following commands are supported to remain compatible to the Qt class browser generator.
    //
    //  Do not use these commands in your own documentation.
    //
    "annotatedclasslist",
    "classhierarchy",
    "define",
    "functionindex",
    "header",
    "headerfilelist",
    "inherit",
    "l",
    "postheader"
    };


void
doxygen_keyword()
{
    string word;
    int dflg;

    dflg = debug(0);
    while (list_each(keywords, word) >= 0) {
        define_keywords(SYNK_MARKUP, "\\"+word+",@" + word);
    }
    debug(dflg, FALSE);

    syntax_rule("[ \t][\\\\@]\\c[a-z${}&~<>#%.:\\[\\]]+",
        "group=comment,markup:comment_standout");
}
/*end*/
