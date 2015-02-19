/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: linenumbers.cr,v 1.2 2014/11/27 17:47:18 ayoung Exp $
 * Linenumbers configuration.
 *
 *
 */

#include "grief.h"

#pragma autoload("_griset_linenumbers")

static void         newfile(void);

static int          lineno_default = 0;
static list         lineno_yes = { "cfg", "ini" };
static list         lineno_no;


void
main(void)
{
    reregister_macro(REG_NEW, "::newfile");
}


/*  Function:       newfile
        newfile event.

    Parameters:
        none

    Returns:
        nothing
 */
static void
newfile(void)
{
    string ext;

    inq_names(NULL, ext);
    if (strlen(ext)) {
        int state = lineno_default;

        if (state) {
            if (re_search(SF_IGNORE_CASE, "<" + quote_regexp(ext) + ">", lineno_no) >= 0) {
                state = 0;
            }
        } else {
            if (re_search(SF_IGNORE_CASE, "<" + quote_regexp(ext) + ">", lineno_yes) >= 0) {
                state = 1;
            }
        }

        if (state) {
            set_buffer_flags(NULL, "line_numbers", NULL);
        }
    }
}


/*  Function:       _griset_linenumbers
        grinit linenumbers configuration importer; controls the display of
        line-numbers for the specified source types.

    Parameters:
        arg - Configuration specification.

    Syntax:
        default=[yes|no] extension,[extension]:[yes|no]

    Returns:
        nothing
 */
void
_griset_linenumbers(string arg)
{
    list parts = tokenize(arg, " \t", TOK_COLLAPSE_MULTIPLE|TOK_TRIM);
    string part, ext;
    int sep;

    lineno_default = 0;
    lineno_yes = NULL;
    lineno_no = NULL;

    while (list_each(parts, part) >= 0) {

        if (characterat(part, 1) == '.') {      /* extension list */

            if ((sep = rindex(part, ':')) > 0) {
                list extensions = split(substr(part, 1, sep - 1), ".");
                int state = (0 == strcasecmp(substr(part, sep + 1), "yes") ? 1 : 0);

                if (state) {
                    while (list_each(extensions, ext) >= 0) {
                        lineno_yes += ext;
                    }
                } else {
                    while (list_each(extensions, ext) >= 0) {
                        lineno_no += ext;
                    }
                }
            }

        } else {                                /* options tag=value */
            if ((sep = index(part, '=')) > 0) {
                string tag = substr(part, 1, sep - 1),
                    val = substr(part, sep + 1);

                if ("default" == tag) {
                    lineno_default =
                        (0 == strcasecmp(val, "yes") ? 1 : 0);
                }
            }
        }
    }
}
/*end*/
