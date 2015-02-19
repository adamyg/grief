/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: change.cr,v 1.14 2014/10/27 23:28:18 ayoung Exp $
 * changelog file.
 *
 *
 */

#include "grief.h"


/*
 *  Default filename for the change-log file
 */
#define CHANGES_FILE        "Changes"
#define CHANGELOG_FILE      "ChangeLog"
#define TODO_FILE           "ToDo"


/*
 *  Maximum nesting of sub-directory from top level of source code. We need this
 *  because we want to access the change log file by referring to it as
 *  ../../../$CHANGE_FILE because its easier to walk up the tree than down from the root
 */
#define DIR_DEPTH           10


/*  Function:   findchange
 *      Locate the specified file within the directory tree.
 */
static int
findchange(string fname, ~string)
{
    string dir = "./";
    string changepath;
    int depth;

    /* Firstly we need to walk up the directory tree looking for the
     * change-log file. We do this to allow user to enter macro from any
     * sub-directory of the software.
     */
    for (depth = 0; depth < DIR_DEPTH; depth++) {
        changepath = dir + fname;

        if (exist(changepath)) {                /* abs name */
            put_parm(1, changepath);
            return depth;
        }

        changepath += ".txt";                   /* also allow .txt versions */
        if (exist(changepath)) {
            put_parm(1, changepath);
            return depth;
        }

        dir = "../" + dir;                      /* Go up a directory level */
    }
    return -1;
}


/* User calls this macro to log a change to a file. The
 * changelog file follows the style of the GNU/FSF ChangeLogs
 */
static void
editchange(string changepath)
{
    string line, day, month, user;
    int day_num, year, hr, min, sec;
    string buf;

    if (edit_file(changepath) == -1)
        return;

    /*
     *  Now generate a header line
     */
    date(year, NULL, day_num, month, day);
    time(hr, min, sec);
    sprintf(buf, "%s %s %2d %02d:%02d:%02d %d ",
        substr(day, 1, 3), substr(month, 1, 3), day_num, hr, min, sec, year);

    if ("" == (user = inq_username()))
        user = "<Dont know>";
    buf += user;

    /*
     *  Locate the start of the Change history
     *
     *      TODO - assumes english locale
     */
    top_of_buffer();
    line = read();
    while (1) {                             /* Find top date stamp */
        if (re_search(SF_IGNORE_CASE,
                "<{Mon}|{Tue}|{Wed}|{Thu}|{Fri}|{Sat}|{Sun}", line) == 1) {
            break;
        }

        down();
        if (inq_position()) {               /* End-Of-Buffer */
            top_of_buffer();
            line = read();
            break;
        }
        line = read();
    }

    /*
     *  If the line in the file and the line we've just generated are
     *  different then insert the new line
     */
    if (substr(buf, 1, 11) != substr(line, 1, 11)) {
        insert(buf);
        insert("\n\n\n\n");
        up();
        up();
        insert("\t* ");
    } else {
        down();
        insert("\n\n");
        up();
        insert("\t* ");
    }
}


static void
do_change(string fname)
{
    string changefile;

    if (findchange(fname, changefile) < 0) {
        error("Could not locate a %s file image.", fname);

    } else {
        editchange(changefile);
    }
}


void
change()
{
    int depth1, depth2;
    string changes, changelog;

    depth1 = findchange(CHANGES_FILE, changes);
    depth2 = findchange(CHANGELOG_FILE, changelog);

//  dprintf("depth: %d/%d, change: %s/%s", depth1, depth2, changes, changelog);

    if (depth1 < 0 && depth2 < 0) {
        error("Could not neither %s nor %s.", CHANGES_FILE, CHANGELOG_FILE);

    } else {
        if (depth1 == depth2) {
            error("Both '%s' and '%s' located at same scope.", CHANGES_FILE, CHANGELOG_FILE);

        } else if (depth2 < 0 || (depth1 >= 0 && depth1 < depth2)) {
            editchange(changes);

        } else {
            editchange(changelog);
        }
    }
}


void
changes()
{
    do_change(CHANGES_FILE);
}


void
changelog()
{
    do_change(CHANGELOG_FILE);
}


void
todo()
{
    do_change(TODO_FILE);
}

/*eof*/
