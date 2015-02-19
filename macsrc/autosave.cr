/* -*- indent-width: 4; -*- */
/* $Id: autosave.cr,v 1.11 2014/10/27 23:28:16 ayoung Exp $
 *
 *
 *
 */

#include "grief.h"

static list         asvd_list;                  /* List of files which dont want to be autosaved */
static list         asv_list;                   /* List of file-names of auto-saved files which
                                                 * need to be deleted when we exit */

static int          asv_ext = FALSE;            /* TRUE if we have to strip off extension */
static int          asv_idle_time;

static int          asv_state = FALSE;          /* TRUE if autosave is active */

static string       asv_prefix;                 /* Filename prefix -- O/S dependent */
static string       asv_suffix;                 /* Filename suffix */


/*
 *  Following says how long to wait before resetting the backup
 *  flags for the currently loaded buffers
 */
static int          rebackup_time;


/*
 *  Following is the period of the rebackup timeout. Default is 2 hours
 */
static int          rebackup_period = 120;


void
main()
{
#if defined(VMS)
    asv_prefix = "ASV-";
    asv_suffix = "";
#elif defined(MSDOS)
    asv_prefix = "";
    asv_suffix = ".asv";
    asv_ext = TRUE;
#else
    asv_prefix = "@";
    asv_suffix = "@";
#endif
}


/*
 *  _griset_autosave ---
 *      Function called on startup to setup the autosave timeout values
 *      from the '.grinit' file of the form <idle> <rebackup>, for
 *      example "60 120". First argument is the idle time to set <idle>,
 *      with the second argument being the rebackup timeout <rebackup>.
 */
void
_griset_autosave(string arg)
{
    list l;

    if (!first_time())
        return;

    if (arg == "")
        arg = "60 120";                         // default
    l = split(arg, " ", 1);
    set_idle_default(l[0]);
    rebackup_period = l[1];
    if (rebackup_period) {
        rebackup_time = rebackup_period * 60 + time();
    }
    register_macro(REG_IDLE, "autosave_files");
    register_macro(REG_EXIT, "autosave_exit");
}


/*
 *  _griget_autosave ---
 *      Executed on exit to return the autosave timeout values for the .grinit file.
 */
string
_griget_autosave(void)
{
    return "" + inq_idle_default() + " " + rebackup_period;
}


void
autosave_disable(string filename)
{
    asvd_list += filename;
}


void
autosave_files(void)
{
    int     curbuf, slash, i, num_written, h, m, s, time_now, time_since_last, nextbuf;
    int     rebackup = FALSE;
    string  filename, bufnam;

    /* Check to see if user has typed anything recently. */
    if (inq_idle_time() < inq_idle_default()) {
        return;
    }

    /* Check to see if its at least idle time since last key press */
    time(h, m, s);
    time_now = ((h * 60) + m) * 60 + s;
    time_since_last = time_now - asv_idle_time;
    if (time_since_last < inq_idle_default()) {
        return;
    }
    asv_idle_time = time_now;

    /*
     *  If the rebackup timeout has expired then set the buffer backup flag. This forces the
     *  buffer to be backed up again. (Normally backing up of buffers only happens the first time
     *  it is saved).
     */
//  int odebug = debug(0xffff, FALSE);

    if (time() >= rebackup_time) {
        rebackup = TRUE;
        rebackup_time = time() + (rebackup_period * 60);
    }
    curbuf = inq_buffer();
    save_position();

    asv_state = TRUE;                           /* TRUE if autosave is active */

    num_written = 0;
    while (1) {

        nextbuf = next_buffer(TRUE);
        set_buffer(nextbuf);
        if (rebackup) {                         /* reback trigger */
            set_buffer_flags(NULL, BF_BACKUP);
        }

        if (inq_modified() &&                   /* modified, non-system and auto-saved enabled */
                0 == inq_system() &&
                inq_buffer_flags(NULL, "autosave")) {
            /*
             *  Check to see if file occurs in disabled list
             */
            inq_names(filename, NULL, bufnam);

            if (re_search(SF_NOT_REGEXP, filename, asvd_list) < 0) {
                /*
                 *  Create auto-save file-name
                 */
                slash = rindex(filename, CRISP_SLASH);
                filename = substr(filename, 1, slash) + asv_prefix + substr(filename, slash + 1);

                if (asv_ext) {                  /* remove extension */
                    i = rindex(filename, ".");
                    if (i > rindex(filename, CRISP_SLASH)) {
                        filename = substr(filename, 1, i - 1);
                    }
                }
                filename += asv_suffix;
                message("Autosave %s..", bufnam);
                ++num_written;

                /* Delete file we are going to write first --
                 *      this ensures under VMS that we don't end up with a million
                 *      and one versions of the autosave file
                 */
                remove(filename);
                write_buffer(filename);

                if (re_search(SF_NOT_REGEXP, filename, asv_list) < 0) {
                    asv_list += filename;
                }
            }
        }

        if (nextbuf == curbuf) {
            break;
        }
    }

    asv_state = FALSE;                          /* TRUE if autosave is active */

    set_buffer(curbuf);
    restore_position();
    if (num_written) {
        message("Autosaved %d file%s.", num_written, num_written == 1 ? "" : "s");
    }
//  debug(odebug, FALSE);
}


/* Function:        autosave_exit
 *    This macro is called when we are about to exit. It is responsible for
 *    deleting any of the autosave files we may have accumulated
 *
 * Returns:
 *    nothing
 */
void
autosave_exit(void)
{
    int n;

    for (n = length_of_list(asv_list); n-- > 0;) {
        remove(asv_list[n]);
    }
}


/*  Function:       autosave_state
 *      Determine whether autosave is currently active.
 *
 *  Returns:
 *      *TRUE* is autosave is active, otherwise *FALSE*.
 */
int
autosave_state(void)
{
    return asv_state;
}

/*eof*/
