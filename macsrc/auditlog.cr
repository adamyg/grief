/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: auditlog.cr,v 1.8 2014/11/27 15:54:09 ayoung Exp $
 * Audit log.
 *
 *
 */

#include "grief.h"

#define AUDITLOG        "Audit.log"
#define DIR_DEPTH       10

static int              audit_buf = -1;
static string           audit_file;


void
auditlog(declare mode, declare file)
{
    if (is_string(mode)) {
        mode = lower(mode) == "yes" ? 1 : 0;

    } else if (get_parm(0, mode) <= 0) {
        mode = 1;
    }

    if (mode) {
        if (audit_buf < 0) {
            /*
             *  walk up the directory tree looking for the audit-log file, allowing
             *  the user to enter macro from any sub-directory of the software.
             */
            int depth;

            if (is_string(file) && strlen(file)) {
                audit_file = file;
            } else {
                string dir;

                dir = "";                       /* Current directory */
                for (depth = 0; depth < DIR_DEPTH; ++depth) {
                    file = dir + AUDITLOG;
                    if (exist(file)) {
                        break;
                    }
                    dir = "../" + dir;          /* Go up a directory level */
                }
                audit_file = "";
            }

            if (depth >= DIR_DEPTH ||
                    (audit_buf = create_buffer("auditlog", file, 1)) < 0) {
                error("Couldn't locate/open the audit log.");
                return;
            }

            set_buffer_flags(audit_buf, BF_SYSBUF|BF_NO_UNDO, ~BF_BACKUP);
        }

        reregister_macro(REG_FILE_WRITTEN, "::trigger");
        message("Audit Log enabled.");

    } else {
        if (audit_buf >= 0) {
            unregister_macro(REG_FILE_WRITTEN, "::trigger");
            delete_buffer(audit_buf);
            audit_buf = -1;
        }
        message("Audit Log disabled.");
    }
}


/*  Function:       trigger
        auditlog REG_FILE_WRITTEN trigger.

    Parameters:
        file -          File.

    Returns:
        nothing
 */
static void
trigger(string file)
{
    int buf, year, day, hour, min, sec;
    string mon;

    /*autosave. if so ignore trigger */
    if (inq_macro("autosave_state") > 0) {
        if (autosave_state()) {
            return;
        }
    }
    if (audit_buf < 0) {
        return;
    }

    /*build audit record and export */
    buf = inq_buffer(file);
    if (buf <= 0 || buf == audit_buf || inq_system(buf)) {
        return;
    }

    localtime(NULL, year, NULL, day, mon, NULL, hour, min, sec);
    save_excursion();
    set_buffer(audit_buf);
    clear_buffer();
    top_of_buffer();
    insertf("%02d-%3.3s-%04d %02d:%02d:%02d %s", day, mon, year, hour, min, sec, file);
    write_buffer(NULL, WRITE_APPEND|WRITE_NOTRIGGER);
    restore_excursion();
}


/*  Function:       _griset_auditlog
        grinit auditlog configuration importer

    Parameters:
        arg - Configuration specification.

    Syntax:
        [yes|no] [mode=yes|no] [log=file-name]

    Returns:
        nothing
 */
void
_griset_auditlog(string arg)
{
    string mode, file;
    list args;
    int i;

    args = split (arg, " =", 1);
    for (i = 0; i < length_of_list(args); i += 2) {
        if (args[i] == "yes") {                 /* yes */
            mode = "yes";
            --i;

        } else if (args[i] == "no") {           /* no */
            mode = "no";
            --i;

        } else if (args[i] == "mode") {         /* mode=yes|no */
            mode = args[i+1];

        } else if (args[i] == "log") {          /* log=file-name */
            file = args[i+1];
        }
    }

    auditlog(mode, file);
}


/*  Function:       _griget_auditlog
      grinit auditlog configuration exporter

    Parameters:
        none

    Returns:
        string - grinit configuration entry
 */
string
_griget_auditlog(void)
{
    string mode;

    mode = "mode=";
    if (audit_buf >= 0) {
        mode += "yes";
        if (strlen(audit_file)) {
            audit_buf += "log=" + audit_file;
        }
    } else {
        mode += "no";
    }
    return (mode);
}
/*eof*/
