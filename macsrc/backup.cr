/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: backup.cr,v 1.9 2014/10/27 23:28:17 ayoung Exp $
 * Backup support.
 *
 *
 */

/*  EMAC:   Single or Numbered Backups

    If you choose to have a single backup file (this is the default), the backup file's
    name is constructed by appending `~' to the file name being edited; thus, the
    backup file for `eval.c' would be `eval.c~'.

    If you choose to have a series of numbered backup files, backup file names are made
    by appending `.~', the number, and another `~' to the original file name. Thus, the
    backup files of `eval.c' would be called `eval.c.~1~', `eval.c.~2~', and so on,
    through names like `eval.c.~259~' and beyond.

    If protection stops you from writing backup files under the usual names, the backup
    file is written as `%backup%~' in your home directory. Only one such file can
    exist, so only the most recently made such backup is available.

    The choice of single backup or numbered backups is controlled by the variable
    version-control. Its possible values are

        t           Make numbered backups.

        nil         Make numbered backups for files that have numbered
                    backups already. Otherwise, make single backups.

        never       Do not in any case make numbered backups; always make
                    single backups.

    You can set version-control locally in an individual buffer to control the making
    of backups for that buffer's file. For example, Rmail mode locally sets
    version-control to never to make sure that there is only one backup for an Rmail
    file. See section Local Variables.

    If you set the environment variable VERSION_CONTROL, to tell various GNU utilities
    what to do with backup files, Emacs also obeys the environment variable by setting
    the Lisp variable version-control accordingly at startup. If the environment
    variable's value is `t' or `numbered', then version-control becomes t; if the value
    is `nil' or `existing', then version-control becomes nil; if it is `never' or
    `simple', then version-control becomes never.

    For files managed by a version control system (see section Version Control), the
    variable vc-make-backup-files determines whether to make backup files. By default,
    it is nil, since backup files are redundant when you store all the previous
    versions in a version control system. See section VC Workfile Handling.
**/

#include "grief.h"

#define DELIMIT         "|"

#if defined(__PROTOTYPES__)
static int              recover_dialog(void);
#endif

static list             bk_grinit;              // grinit configuration

static list             bk_roots;

static list             bk_options = {
    /*name,         setting*/
    "dir",          BK_DIR,
    "versions",     BK_VERSIONS,
    "ask",          BK_ASK,
    "dont",         BK_DONT,
    "prefix",       BK_PREFIX,
    "suffix",       BK_SUFFIX
    };


void
main(void)
{                                               // XXX - work-around until fixed
    register_macro(REG_NEW, "::newfile");
//  register_macro(REG_RENAME, "::renamefile");
}


/*  Function:           newfile
        REG_NEW trigger .. parse the backup configuration (if any) setting any buffer
        specific characteristics.

**/
static void
newfile(void)
{
    string name, dir;
    int i, k, o;

    if (length_of_list(bk_roots) <= 0) {
        return;
    }

    if (first_time()) {                         // sort by length
        bk_roots = sort_list(bk_roots, "::sortroots");
    }

    inq_names(name);
    dir = dirname(name);

    for (i = 0; i < length_of_list(bk_roots); ++i) {
        list params = bk_roots[i];

        if (strfilecmp(params[0], dir, strlen(params[0])) == 0) {
            for (k = 1, o = 1; k < length_of_list(params); ++k, o += 2) {
                declare param = params[k];

                if (!is_null(param)) {          // XXX - is_null requires a ref
                    set_backup_option(bk_options[o], inq_buffer(), param);
                }
            }
            break;
        }
    }
}


static int
sortroots(/*const*/ list a, /*const*/ list b)
{
    int na = strlen(a[0]);
    int nb = strlen(b[0]);

    if (na == nb)
        return 0;
    if (na > nb)
        return -1;
    return 1;
}


/*  Function:           _griset_backup
        'backup' configuration parser

**/
void
_griset_backup(string arg)
{
    list args;
    int i, k;

    arg = trim(arg);
    push(bk_grinit, arg);
    args = split(arg, " =", 1);

    if (length_of_list(args) <= 0) {
        return;                                 // backup:
    }

    if (args[0] == "root") {
        /*
         *  Specific backup options for a given 'root'.
         */
        list params;
        string root;

        root = args[1];                         // root, first argument
        realpath(root, root);
        if (strlen(root) > 1) {
            rtrim(root, "/\\");                 // remove trailing delimiter (if any)
        }
        params[0] = root;

        for (i = 2; i < length_of_list(args); i += 2) {
            if ((k = re_search(NULL, "<" + args[i] + ">", bk_options)) >= 0)  {
                params[(k/2)+1] = args[i+1];
            }
        }

        push(bk_roots, params);

    } else {
        /*
         *  Default backup options.
         */
        for (i = 0; i < length_of_list(args); i += 2) {
            if ((k = re_search(NULL, "<" + args[i] + ">", bk_options)) >= 0)  {
                set_backup_option(bk_options[k+1], NULL, args[i+1]);
            }
        }
    }
}


/*  Function:           _griget_backup
        'backup' configuration retrieve interface.

**/
list
_griget_backup(void)
{
    return bk_grinit;                           // return "global" setting
}


/*  Function:           recover
        recover command, allowing the selection and recovery of a files backup image.

    Notes:
        file-name

        Version Time-Date stamp
            :
            :
            :
            :

                < ok >  < restore > < view > < help >

**/
void
recover(void)
{
    int dialog, ret;

    /* build list of backup versions */

    /* create dialog */
    dialog = recover_dialog();
    ret = dialog_run(dialog);
    dialog_delete(dialog);

    /* restore (if required) */
            /* TODO */
}


static int
recover_dialog(void)
{
    int dialog =  dialog_create( quote_list(
        DLGA_TITLE,                             "File Recovery",
        DLGA_CALLBACK,                          "::recover_callback",
//      DLAG_ALLOW_MOVE,
//      DLAG_ALLOW_RESIZE,

        DLGC_CONTAINER,
            DLGA_ATTACH_LEFT,
            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_ALIGN_NW,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Buffer:",
                    DLGA_ALIGN_W,
                    DLGA_COLS,                  10,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Time:",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Source:",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_VALUE,                 "Versions:",
                    DLGA_ALIGN_W,
//              DLGC_LABEL,
//                  DLGA_VALUE,                 "Images:",
//                  DLGA_ALIGN_W,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_RIGHT,
                DLGA_ALIGN_NE,
                DLGC_EDIT_FIELD,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "buffer",
                    DLGA_ROWS,                  1,
                    DLGA_GREYED,
                    DLGA_ALLOW_FILLX,
                DLGC_EDIT_FIELD,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "mtime",
                    DLGA_ROWS,                  1,
                    DLGA_GREYED,
                    DLGA_ALLOW_FILLX,
                DLGC_EDIT_FIELD,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "source",
                    DLGA_ROWS,                  1,
                    DLGA_GREYED,
                    DLGA_ALLOW_FILLX,
                DLGC_EDIT_FIELD,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "versions",
                    DLGA_ROWS,                  1,
                    DLGA_GREYED,
                    DLGA_ALLOW_FILLX,
                DLGC_LIST_BOX,
                    DLGA_ALIGN_E,
                    DLGA_NAME,                  "images",
                    DLGA_ROWS,                  10,
                    DLGA_COLS,                  32,
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_RIGHT,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,                     "&Restore",
                DLGA_NAME,                      "restore",
                DLGA_ALLOW_FILLX,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,                     "&View",
                DLGA_NAME,                      "view",
                DLGA_ALLOW_FILLX,
//          DLGC_PUSH_BUTTON,
//              DLGA_LABEL,                     "D&iff",
//              DLGA_NAME,                      "diff",
//              DLGA_ALLOW_FILLX,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,                     "&Done",
                DLGA_NAME,                      "done",
                DLGA_ALLOW_FILLX,
                DLGA_CANCEL_BUTTON,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,                     "&Help",
                DLGA_NAME,                      "help",
                DLGA_ALLOW_FILLX,
        DLGC_END
        ));

    return dialog;
}


static void
recover_callback(int ident, string name, int p1, int p2)
{
    UNUSED(ident, p2);

    switch (p1) {
    case DLGE_INIT: {
            string dir = inq_backup_option(BK_DIR, inq_buffer());
            string versions = inq_backup_option(BK_VERSIONS, inq_buffer());
            string path, buffer;
            int mtime, v;

            inq_names(buffer);
            buffer = basename(buffer);

            widget_set(NULL, "buffer", buffer);
            widget_set(NULL, "source", dir);
            widget_set(NULL, "versions", versions);

            /* scan backup tree. */
            for (v = 0; v <= versions; ++v) {
                if (v == 0) {
                    sprintf(path, "%s/%s", dir, buffer);
                } else {
                    sprintf(path, "%s/%d/%s", dir, v, buffer);
                }

                if (stat(path, NULL, mtime) == 0) {
                    sprintf(path, "%s", strftime(NULL, mtime));
                    widget_set(NULL, "images", path);
                }
            }
        }
        break;

    case DLGE_BUTTON:
        switch (name) {
        case "recover":
            dialog_exit(1);
            break;
        case "done":
            dialog_exit(-1);
            break;
        case "help":
            execute_macro("explain recover");
            break;
        default:
            break;
        }
        break;
    }
}

/*end*/
