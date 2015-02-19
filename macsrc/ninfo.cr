/* $Id: ninfo.cr,v 1.14 2014/10/27 23:28:25 ayoung Exp $
 * System help/information interpreter.
 *
 *
 */

#include "grief.h"

static void         ninfo_exec(string filename, list index_list);

/*
 *  List construction:
 *
 *          "section", "file", line, NULL
 *      or  "section", "file", line, list
 */
#define NINFOSIZE   4                           /* ninfo record elements */
#define NINFONAME   0
#define NINFOFILE   1
#define NINFOLINE   2
#define NINFOLIST   3


/*  Function:       ninfo
 *      System information command
 *
 */
void
ninfo(string name)
{
    /*
     *  derive HELP location.
     */
    if (getenv("GRHELP") == "" && getenv("BHELP") == "") {
        error("GRHELP/BHELP environment variable not set.");
        return;
    }

    if (name == "") {
        name = "user";                          /* default, top level 'user' manual */
    }

    /*
     *  load and retrieve the index.
     */
    load_macro(help_resolve(name + GREXTENSION), FALSE);

    declare ninfolst =
        execute_macro(name + "_ninfoindex");

    if (! is_list(ninfolst)) {
        error("NINFO definition invalid.");
        return;
    }

    ninfo_exec(name + ".hlp", ninfolst);
}


/*  Function:       ninfo_exec
 *      NINFO execute.
 *
 */
static void
ninfo_exec(string filename, list ninfolst)
{
    list    items, actions;
    string  name, item;
    declare var;
    int     llen, lwidth;
    int     i;

    UNUSED(filename);

    if ((lwidth = strlen(ninfolst)) < 38) {
        lwidth = 38;
    }
    llen = length_of_list(ninfolst);

    for (i = 0; i < llen; i += NINFOSIZE) {     /* section, file, line, list */

        name = ninfolst[i + NINFONAME];
        var = ninfolst[i + NINFOLIST];

        if (is_list(var)) {                     /* sub menu available */
            sprintf(item, "%s%*s =>", name, (lwidth - strlen(name)) + 2, "");

                                                /* centre headers */
        } else if (re_search(SF_BRIEF, "<\\[\\[", name) > 0) {
            sprintf(item, "%*s%s", ((lwidth + 4) - strlen(name))/2, "", name);

        } else {                                /* standard items */
            sprintf(item, "%s%*s  ", name, (lwidth - strlen(name)) + 2, "");
        }

        items += item;
        actions += "_ninfo_select";
    }

    select_list("GRIEF Help", "<Back/Space> prev, <Enter/Right> select",
        1, items, SEL_NORMAL, NULL, actions, 0, "_ninfo_keylist");
}


/*  Function:       _ninfo_keylist
 *      Additional select buffer keys, for sub buffers.
 *
 */
void
_ninfo_keylist(void)
{
    assign_to_key("<Backspace",  "sel_esc");
    assign_to_key("<Space>",     "sel_esc");
    assign_to_key("<Right>",     "sel_list");
    assign_to_key("<Left>",      "sel_esc");
}


/*  Function:         _ninfo_select
 *      NINFO menu item section.
 *
 */
void
_ninfo_select()
{
    extern  list ninfolst;
    extern  string filename;
    int     pos;
    declare var;

    inq_position(pos); --pos;                   /* determine associated item */
    var = ninfolst[(pos * NINFOSIZE) + NINFOLIST];

    if (is_list(var)) {
        ninfo_exec(filename, var);              /* process sub menu. */

    } else {                                    /* display item. */
        string file = ninfolst[(pos * NINFOSIZE) + NINFOFILE];
        int line = ninfolst[(pos * NINFOSIZE) + NINFOLINE];

        help_display(file, "", line);
    }
}


/*
 *  Local Variables: ***
 *  mode: cr ***
 *  indent-width: 4 ***
 *  End: ***
 */
