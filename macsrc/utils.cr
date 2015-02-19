/* $Id: utils.cr,v 1.16 2014/10/22 02:34:25 ayoung Exp $
 * Various utility macros.
 *
 *      rw
 *      write_buffer_as
 *      write_buffers
 *      wildcard_erase
 *
 *
 */

#include "grief.h"

/*
 *  rw ---
 *      Make the current file read/write
 */
void
rw()
{
    string filename;
    int ok, mode;

    if (0 == (inq_buffer_flags() & BF_READONLY)) {
        ok = 1;                                 /* already */

    } else if (inq_buffer_flags() & BF_NEW_FILE) {
        set_buffer_flags(NULL, NULL, ~BF_READONLY);
        ok = 1;

    } else {
        inq_names(filename);
        if (stat(filename, NULL, NULL, NULL, NULL, mode) != 0) {
            message("rw stat : (%s)", strerror());

        } else {
            mode |= 0600;                       /* user read/write */
            if (chmod(filename, mode) != 0) {
                message("rw chmod : (%s)", strerror());

            } else {                            /* buffer */
                set_buffer_flags(NULL, NULL, ~BF_READONLY);
                ok = 1;
            }
        }
    }

    if (ok) {
        message("File now writable.");
    }
}


/*
 *  write_buffer_as( [name] ) ---
 *      Save the current buffer using an alternative name.
 */
int
write_buffer_as()
{
    string name, old_name;
    int ret;

    inq_names(old_name);
    if (get_parm(0, name, "Save as: ", NULL, old_name) <= 0 ||
            name == "" || old_name == name ) {
        message("");                            /* abort or same */
        ret = 0;

    } else if ((ret = output_file(name)) >= 0) {
        inq_names(name);                        /* full name */
        set_calling_name("");
        ret = write_buffer();
        if (ret > 0) {
            message("Saved buffer: %s", name);
        } else {
            output_file(old_name);              /* restore old name */
        }
    }
    return ret;
}


/*
 *  write_buffers() ---
 *      Save all current modified buffers.
 */
int
write_buffers()
{
    string filenam, bufnam;
    int curbuf, thisbuf;
    int fileno;

    fileno = 0;
    curbuf = thisbuf = inq_buffer();
    do {
        if (! inq_system() && inq_modified()) {
            inq_names(filenam, NULL, bufnam);
            message("Saving %s..", bufnam);
            write_buffer(filenam);              /* XXX - error handling */
            ++fileno;
        }
        thisbuf = next_buffer(TRUE);
        set_buffer(thisbuf);
    } while (thisbuf != curbuf);

    if (0 == fileno) {
        message("No modified buffers.");
    } else {
        message("Wrote %d modified buffer%s.", fileno, (fileno > 1 ? "s" : ""));
    }
    return fileno;
}


/*
 *  wildcard_erase ---
 *      Erase files by wildcard.
 */
void
wildcard_erase()
{
    int     i, c, llen;
    string  arg;
    list    files_to_erase;

    if (get_parm(0, arg, "Erase spec: ", NULL, "") < 0) {
        return;
    }

    message("Gathering files...");
    files_to_erase = file_glob(arg);
    llen = length_of_list(files_to_erase);
    if (! llen) {
        message("No matching files.");
    } else {                                    /* XXX - need "You are sure" */
        for (c = i = 0; i < llen;){
            arg = files_to_erase[i++];
            message("Erasing %s", arg);
            if (remove(arg) >= 0) {
                ++c;
            }
        }

        if (c == llen) {
            message("%d files successfully erased.", llen);
        } else {
            message("%d of %d files NOT erased.", llen - c, llen);
        }
    }
}

/*
 *  Local Variables: ***
 *  mode: cr ***
 *  indent-width: 4 ***
 *  End: ***
 */
