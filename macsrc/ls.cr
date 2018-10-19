/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: ls.cr,v 1.11 2018/10/01 21:05:01 cvsuser Exp $
 * Directory & File manager
 *
 *
 */

#include "grief.h"

/*
 *  How to display size of file.
 */
#define SIZE_BYTES      0
#define SIZE_KBYTES     1
#define SIZE_COOK       2

#define SYNTAX          "ls_coloriser"

static int              dm_size = SIZE_COOK;
static int              dm_num = 0;

/*
 *  Current nesting level of popups. Used so we can avoid exiting
 *  macro when we select the '..' entry in the first popup.
 */
static int              ls_level = 0;

/*
 *  main ---
 *      Run-time initialisation.
 */
void
main(void)
{
    create_syntax(SYNTAX);
    syntax_rule("^[ ].*[/]",        "lsdirectory");
    syntax_rule("^[*][^ ]+",        "lsexecute");
    syntax_rule("^[@][^ ]+",        "lssymlink");
    syntax_rule("^[|][^ ]+",        "lspipe");
    syntax_rule("^[!][^ ]+",        "lserror");
    syntax_rule("^[-=S+][^ ]+",     "lsspecial");
    syntax_rule("^[ ][^ ]+",        "lsnormal");
    syntax_build(__COMPILETIME__);
}


/*
 *  ls ---
 *      This macro is a sort of interactive 'ls' extending the
 *      facilities of the filename completion popup menus.
 */
void
ls(string dir)
{
    int buf, win, curbuf = inq_buffer();
    string cwd, name, t_size;
    int dirs, size, mtime, ctime, mode;

    /*
     *  If no directory specified, use current one
     */
    getwd(NULL, cwd);
    if (strlen(dir)) {
        cd(dir);
        getwd(NULL, dir);                       /* in case dir access error */
    } else {
        dir = cwd;                              /* use current working dir */
    }

    /*
     *  Match on all files. This forces us to match on files starting with '.'
     *  Get info on each file in current directory.
     */
    if ((buf = create_buffer("ls-" + dm_num++, NULL, TRUE)) < 0) {
        return;
    }
    set_buffer(buf);
    attach_syntax(SYNTAX);
    set_buffer_title(buf, dir);

    file_pattern("?*");
    while (find_file(name, size, mtime, ctime, mode)) {
        if (SIZE_COOK == dm_size) {
            if (size < 9999999) {
                sprintf(t_size, "%7d", size);
            } else if ((size /= 1024) < 999999) {
                sprintf(t_size, "%6dK", size);
            } else if ((size /= 1024) < 999999) {
                sprintf(t_size, "%6dM", size);
            } else {
                sprintf(t_size, "%6dG", size/1024);
            }
        } else {
            sprintf(t_size, "%7d",
                dm_size == SIZE_BYTES ? size : ((size / 1024) + ((size & 1023) != 0)));
        }

        if (S_IFDIR & mode) {
            if (substr(name, 1, 1) == ".") {
                if (".." != name) {
                    continue;                   /* current directory */
                }
            }
            top_of_buffer();
            insertf(" %-20.20s %s %s\n", name+"/", t_size, mode_string(mode));
            end_of_buffer();
            ++dirs;
        } else {
            insertf("%s%-20.20s %s %s\n",
                substr(mode_string(mode, cwd+"/"+name, TRUE), 1, 1), name, t_size, mode_string(mode));
        }
    }

    /*
     *  Sort and create popup window.
     */
    delete_line();
    sort_buffer(NULL, "::ls_sort", 1, dirs);
    sort_buffer(NULL, "::ls_sort", dirs + 1);

    set_buffer(curbuf);
    win = sized_window(inq_lines(buf)+1, inq_line_length(buf)+1, "ls");
    select_buffer(buf, win, SEL_NORMAL, inq_module() + "::ls_keys");
    delete_buffer(buf);
    cd(cwd);
}


static int
ls_sort(int alen, string astr, int blen, string bstr)
{
    UNUSED(alen, blen);
    return strcasecmp(substr(astr, 2), substr(bstr, 2));
}


static void
ls_keys()
{
    assign_to_key("<Enter>", "::ls_walk");
}


/*  Function:       ls_walk
 *      Called when user hits <Enter> on a file name. If its a directory walk into it
 *      by recursing, so that the other window stays on the screen. If its an upper
 *      layer directory (i.e. ..), then force current macro to return.
 *
 *  Parameters:
 *      none
 *
 *  Returns:
 *      nothing
 */
static void
ls_walk()
{
    extern int window_offset;
    extern int top_line;
    string s;
    int i;

    s = ltrim(read());
    if ((i = index(s, '/')) > 1) {
        s = substr(s, 1, i);
        ++top_line;
        window_offset += 10;
        ++ls_level;
        ls("./" + s);
        --ls_level;
        window_offset -= 10;
        --top_line;
    }
}

/*end*/
