/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: dayone.cr,v 1.5 2025/01/09 16:35:28 cvsuser Exp $
 * day-one execution.
 *
 *
 */

#include "grief.h"

static int      dialog;                         // dialog handle

static void     dayone_dialog(void);
static void     dayone_callback(int ident, string name, int p1, int p2);

static int      resource_empty(void);
static int      resource_prime(void);

static int      gr_prime(string home);
static int      grinit_prime(string home);
static int      grrc_prime(string home);


void
main(void)
{
}


//  Function:   dayone
//      GRIEF configuration function for new users.
//
//  Return:
//      Returns 0 of non dayone, otherwise -1=terminate or 1=Continue
//
int
dayone(~ string arg)
{
#define DAYONE_TERM     -1
#define DAYONE_CONT     1

    string grinit = inq_grinit();

    if (arg == "" || arg != "--force") {        // resource checks?
        if (getenv("GRFLAGS") != "" || GRRC != "" || exist(grinit)) {
            return 0;                           // up-to-date
        }
    }

    if (dialog == 0) {                          // build dialog
        dayone_dialog();
    }

    return dialog_run(dialog);                  // -1=terminate, 1=continue
}


static void
dayone_dialog(void)
{
    list dayone_text = make_list(               // Splash message
            "You are seeing this message because you have no GRIEF startup files",
            "present within your home directory (~) nor environment settings.",
            "",
            "The config file ~/" + GRRC_FILE + " plus the equivalent legacy environment",
            "variable GRFLAGS and the profile file " + GRINIT_FILE + " define operational",
            "settings. You may apply these to customise the editor, optionals:",
            "",
            " (C) Create ~/" + GRRC_FILE + " and " + GRINIT_FILE + " using the default recommended",
            "     configuration plus the local working directory and exit.",
            "     You can review and edit, if so desired.",
            "",
            " (I) Ignore and continue the edit session. You may manually create",
            "     one or more of the required startup elements. Note, unless",
            "     created you shall be prompted again.",
            "",
            " (X) Exit, creating an empty ~/" + GRRC_FILE + " file containing just a comment.",
            "     That prevents this function being run again on the next run.",
            "",
            " (Q) Quit and do nothing, prompt again next time."
            );

   int maj, min, edit, rel;
   string appver;

   version(maj, min, edit, rel);
   sprintf(appver, "%s v%d.%d.%d.%d - Configuration", APPNAME, maj, min, edit, rel);

    dialog =
        dialog_create( make_list(
            DLGA_TITLE,                   appver,
            DLGA_CALLBACK,                "::dayone_callback",

            DLGC_CONTAINER,
                DLGA_ATTACH_TOP,
                DLGC_LIST_BOX,
                    DLGA_ROWS,            20, // limit to 25x80
                    DLGA_COLS,            68,
                    DLGA_LBDUPLICATES,    TRUE,
                    DLGA_LBELEMENTS,      dayone_text,
                    DLGA_ALLOW_FILLX,
                    DLGA_GREYED,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_BOTTOM,
                DLGA_ALIGN_CENTER,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,           "&Create",
                    DLGA_NAME,            "create",
                    DLGA_ATTACH_LEFT,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,           "&Ignore",
                    DLGA_NAME,            "ignore",
                    DLGA_ATTACH_LEFT,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,           "e&Xit",
                    DLGA_NAME,            "exit",
                    DLGA_ATTACH_LEFT,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,           "&Quit",
                    DLGA_NAME,            "quit",
                    DLGA_ATTACH_LEFT,
                    DLGA_DEFAULT_FOCUS,
                    DLGA_DEFAULT_BUTTON,
            DLGC_END
            ));
}


static void
dayone_callback(int ident, string name, int p1, int p2)
{
    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_INIT:
        break;
    case DLGE_BUTTON:
        switch(name) {
        case "create":
            if (0 == resource_prime()) {
               dialog_exit(DAYONE_TERM);
            }
            break;
        case "ignore":
            dialog_exit(DAYONE_CONT);
            break;
        case "exit":
            if (0 == resource_empty()) {
               dialog_exit(DAYONE_TERM);
            }
            break;
        case "quit":
            dialog_exit(DAYONE_TERM);
            break;
        }
        break;
    }
}


//  Function: resource_empty
//      Exit mode, creates a basic ~/.grrc file containing just a comment, preventing this function
//      being run again on the next run.
//
//  Returns:
//      0 on success, otherwise -1.
//
static int
resource_empty(void)
{
    string home;

    home = inq_home();                          // users home directory.
    if (! home) {
        error("Couldn't resolve home directory.");
        return -1;
    }
    return grrc_prime(home);
}


//  Function: grrc_prime
//      Create mode, generate ~/.grrc and ~/.grinit using the default recommended configuration
//      plus the local working directory and exit.
//
//  Returns:
//      0 on success, otherwise -1.
//
static int
resource_prime(void)
{
    string home;

    home = inq_home();                          // users home directory.
    if (! home) {
        error("Couldn't resolve home directory.");
        return -1;
    }

    if (gr_prime(home) == 0) {                  // ~/.gr
        if (grinit_prime(home) == 0) {          // ~/.grinit
            if (grrc_prime(home) == 0) {        // ~/.grrc
                return 0;
            }
        }
    }
    return -1;
}


//  Function: gr_prime
//      Generate local working directory tree under ~/.gr.
//
//  Returns:
//      0 on success, otherwise -1.
//
static int
gr_prime(string home)
{
    string grdir;
    int mode;

    // working directory tree
    //
    //  ~/.gr
    //  ~/.gr/restore
    //  ~/.gr/backup
    //
    grdir = file_canon(home + "/.gr");          // ~/.gr
    if (0 == stat(grdir, NULL, NULL, NULL, NULL, mode)) {
        if (0 == (S_IFDIR & mode)) {
            error("Working path <%s> is not a directory.", grdir);
            return -1;
        }
    } else {
        if (mkdir(grdir) != 0) {
            error("Couldn't create working directory <%s>.", grdir);
            return -1;
        }
    }

    mkdir(grdir + "/backup");
    mkdir(grdir + "/restore");

    return 0;
}


//  Function: grrc_prime
//      Generate a default ~/.grinit, by importing the default template.
//
//  Returns:
//      0 on success, otherwise -1.
//
static int
grinit_prime(string home)
{
    int curbuf, rcbuf, ret = 0;
    string grexample, grinit;

    grinit = file_canon(home + "/" + GRINIT_FILE);
    grexample = help_resolve("grinit_example");

    curbuf = inq_buffer();
    if ((rcbuf = create_buffer("-grinit-", grexample, TRUE)) < 0) {
        error("Couldn't import <%s>.", grexample);
        return -1;
    }

    if (exist(grinit)) {
        rename(grinit, grinit + ".sav");
    }

    set_buffer(rcbuf);
    set_buffer_flags(NULL, NULL, ~BF_READONLY); /* readable */
    if (write_buffer(grinit, WRITE_NOTRIGGER) < 0) {
        error("Couldn't create <%s>.", grinit);
        ret = -1;
    }
    set_buffer(curbuf);
    delete_buffer(rcbuf);

    return ret;
}


//  Function: grrc_prime
//      Creates a default ~/.grrc file containing just a comment.
//
//  Output:
//      # GRIEF resource file; see "gr --help" for available options
//      # created: 08-Dec-2024 21:19:47
//
//      # Example:
//      #--dark     # enable dark scheme
//
//      # end
//
//  Returns:
//      0 on success, otherwise -1.
//
static int
grrc_prime(string home)
{
    int curbuf, rcbuf, ret = 0;
    string grrc;
    int year, day, hour, min, sec;
    string mon;

    grrc = file_canon(home + "/" + GRRC_FILE);

    curbuf = inq_buffer();
    if ((rcbuf = create_buffer("-grrc-", NULL, TRUE)) < 0) {
        error("Couldn't create <%s>.", grrc);
        return -1;
    }

    set_buffer(rcbuf);
    localtime(NULL, year, NULL, day, mon, NULL, hour, min, sec);

    insertf("# GRIEF resource file; see \"gr --help\" for available options\n");
    insertf("# created: %02d-%3.3s-%04d %02d:%02d:%02d\n\n", day, mon, year, hour, min, sec);
    insertf("# Example option:\n#--dark\t\t# enable dark scheme\n\n");
    insertf("# end\n");

    if (exist(grrc)) {
        rename(grrc, grrc + ".sav");
    }

    if (write_buffer(grrc, WRITE_NOTRIGGER) < 0) {
        error("Couldn't create <%s>.", grrc);
        ret = -1;
    }

    set_buffer(curbuf);
    delete_buffer(rcbuf);
    return ret;
}

//end
