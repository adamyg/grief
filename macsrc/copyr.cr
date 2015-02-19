/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: copyr.cr,v 1.8 2014/10/27 23:28:19 ayoung Exp $
 * Copyright notice.
 *
 *
 */

#include "grief.h"

#if defined(__PROTOTYPES__)
static void     insert_subst(string old, string new);
#endif

static list     months =
    {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };


void
ccopyr()
{
    int old_insert_mode, line, column, d, m, y;
    string file_name, ext, description, buf;

    old_insert_mode = insert_mode(1);
    inq_position(line, column);
    top_of_buffer();
    refresh();

    top_of_buffer();
    inq_names(NULL, ext, file_name);
    file_name = substr(file_name, rindex(file_name, "/") + 1);
    insert("/* $" + "Id:$\n");
    insert(" * " + file_name + "\n");
    date(y, m, d);
    sprintf(buf, "%d %s %d", d, months[m - 1], y);
    insert(" * Created: " + buf + "\n");
    insert(" *\n * $" + "Log$\n");
    insert(" *\n *\n *\n */\n\n");

    top_of_buffer();
    re_search(NULL, "mcm");
    get_parm(0, description, "Enter description: ", 50);
    delete_char(3);

    insert(description);
    insert_mode(old_insert_mode);
    goto_old_line(line);
    beginning_of_line();
    move_rel(0, column);
}


void
revision()
{
    string date_string, initials, comment, ok;
    int year, month, day, i;

    top_of_buffer();
    if (re_search(NULL, "$Id") <= 0) {
        get_parm(NULL, ok, "Generate module header (y/n) ? ");
        if (ok != "y") {
            return;
        }
        ccopyr();
    }

    top_of_buffer();
    if (re_search(NULL, "Revision") <= 0) {
        get_parm(NULL, ok, "Generate revision history (y/n) ? ");
        if (ok != "y") {
            return;
        }
    }

    re_search(NULL, "{SCCS}|{sccs}|{\\@(#)}");

    end_of_line();
    insert("\n\n * Revision History\n * ================\n *\n */\n");
    re_search(SF_BACKWARDS, "Revision");
    re_search(SF_NOT_REGEXP, "*/");
    move_rel(-1);
    end_of_line();
    date(year, month, day);
    sprintf(date_string, "\n *  %d-%d-%d", day, month, year);
    insert(date_string);
    i = 17 - strlen(date_string);
    insert(" ", i);

    get_parm(0, initials, "Enter your initials: ", 6);
    insert(initials);
    insert(" ", 8 - strlen(initials));
    insert(": ");
    get_parm(0, comment, "Enter comment: ", 40);
    insert(comment);
}


void
fn()
{
    string f;
    string name;
    int i;

    beginning_of_line();

    f = compress(trim(read()));
    i = index(f, "(");
    name = substr(f, 1, i - 1);
    f = substr(f, i + 1);
    insert("/* X\n");
    insert_subst("X", name);
    insert(" * \n");
    insert(" */\n");

    up(); up(); up();
    set_top_of_window();
    down();
    end_of_line();
}


static void
insert_subst(string old, string new)
{
    up();
    re_search(NULL, old);
    delete_char(strlen(new));
    insert(new);
    down();
    beginning_of_line();
}

/*eof*/
