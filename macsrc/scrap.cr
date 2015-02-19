/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: scrap.cr,v 1.6 2014/10/27 23:28:26 ayoung Exp $
 * Named scrap interface.
 *
 *
 *
 */

#include "grief.h"

static int          scrap_set(string msg);
static int          scrap_bufid(string sname);
static void         scrap_keys(void);
static string       scrap_dialog();


/*
 *  cut_named_scrap ---
 *      Named scrap cut operator.
 */
int
cut_named_scrap(void)
{
    if (scrap_set("Cut to named scrap: ") >= 0) {
        /*
         *  Allow the replacement macro to be called pretending user
         *  directly got there from the keyboard.
         */
        set_calling_name(inq_called());
        return cut();
    }
    return -1;
}


/*
 *  copy_named_scrap ---
 *      Named scrap copy operator.
 */
int
copy_named_scrap(void)
{
    if (scrap_set("Copy to named scrap: ") >= 0) {
        set_calling_name(inq_called());
        return copy();
    }
    return -1;
}


/*
 *  paste_named_scrap ---
 *      Named scrap paste operator.
 */
int
paste_named_scrap(void)
{
    if (scrap_set("Paste named scrap: ") >= 0) {
        set_calling_name(inq_called());
        return paste();
    }
    return -1;
}


/*
 *  scrap_set ---
 *      Retrieve the named scrap buffer.
 */
static int
scrap_set(string msg)
{
    string sname;
    int bufid;

    if (get_parm(NULL, sname, msg) <= 0 || 0 == strlen(sname)) {
        return -1;
    }

    if (-1 == (bufid = scrap_bufid(sname))) {
        bufid = create_buffer("/SCRAP/" + sname, NULL, TRUE);
    }

    if (bufid >= 0) {
        set_scrap_info(NULL, NULL, bufid);
    }

    return bufid;
}


/*
 *  scrap_bufid ---
 *      Retrieve the buffer associated with the named scrap buffer.
 */
static int
scrap_bufid(string sname)
{
    int idx, curbuf, bufid;

    if ((idx = index(sname, " ")) > 0) {
        sname = substr(sname, 1, idx - 1);
    }

    curbuf = inq_buffer();
    while ((bufid = next_buffer(TRUE)) != curbuf) {
        set_buffer(bufid);
        if (inq_buffer_flags() & BF_SCRAPBUF) {
            string fname;

            inq_names(fname);
            if (sname == basename(fname)) {
                set_buffer(curbuf);
                return bufid;
            }
        }
    }
    set_buffer(curbuf);
    return -1;
}


/*
 *  compl_scrap ---
 *      Named scrap command line completion, executed when the user
 *      hits <Tab> at the named scrap prompts.
 */
string
compl_scrap(string name)
{
    string sname = scrap_dialog();

    message("");
    return (strlen(sname) ? sname : name);
}


/*
 *  scrap_dialog ---
 *      Named scrap buffer management dialog.
 */
static string
scrap_dialog()
{
    int dialog = dialog_create( make_list(
        DLGA_TITLE,                 "Named Scrap",
        DLGA_CALLBACK,              "::scrap_cb",

        DLGC_CONTAINER,
            DLGA_ATTACH_LEFT,
            DLGC_CONTAINER,
                DLGA_ATTACH_RIGHT,
                DLGA_ALIGN_NE,
                DLGC_LIST_BOX,
                    DLGA_ALIGN_E,
                    DLGA_NAME,      "buffers",
                    DLGA_ROWS,      8,
                    DLGA_COLS,      53,         // "10 [40]"
                    DLGA_HOTKEY,    'b',        // Alt-B
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGA_ATTACH_TOP,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,         "&Select",
                DLGA_NAME,          "select",
                DLGA_ALLOW_FILLX,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,         "&Delete",
                DLGA_NAME,          "delete",
                DLGA_ALLOW_FILLX,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,         "E&xit",
                DLGA_NAME,          "exit",
                DLGA_ALLOW_FILLX,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,         "&Help",
                DLGA_NAME,          "help",
                DLGA_ALLOW_FILLX,
        DLGC_END
        ));

    string sname;
    int ret;

    if ((ret = dialog_run(dialog)) > 0) {
        inq_names(sname, NULL, NULL, ret - 1);
        sname = basename(sname);
    }
    dialog_delete(dialog);
    return sname;
}


/*
 *  scrap_cb ---
 *      Named scrap dialog callback.
 */
static int
scrap_cb(int ident, string name, int p1, int p2)
{
    extern int erridx;

    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_INIT: {
            list buffers;
            int curbuf, bufid;

            curbuf = inq_buffer();
            while ((bufid = next_buffer(TRUE)) != curbuf) {
                set_buffer(bufid);
                if (inq_buffer_flags() & BF_SCRAPBUF) {
                    string fname, sample, item;

                    inq_names(fname);
                    save_position();
                    top_of_buffer();
                    sample = trim(read());
                    if (strlen(sample) > 40) {
                        sample = substr(sample, 1, 37) + "...";
                    }
                    restore_position();
                    sprintf(item, "%-10.10s [%s]", basename(fname), sample);
                    buffers += item;
                    set_buffer(bufid);
                }
            }
            widget_set(NULL, "buffers", buffers, DLGA_LBELEMENTS);
            set_buffer(curbuf);
        }
        break;
    case DLGE_BUTTON:
        switch(name) {
        case "select": {
                const int item =
                    widget_get(NULL, "buffers", DLGA_LBACTIVE);

                if (item >= 0) {
                    string value =
                        widget_get(NULL, "buffers", DLGA_LBTEXT, item);
                    int bufid = scrap_bufid(value);

                    if (bufid >= 0) {
                        dialog_exit(bufid + 1);
                    }
                }
            }
            break;
        case "delete": {
                const int item =
                    widget_get(NULL, "buffers", DLGA_LBACTIVE);

                if (item >= 0) {
                    string value =
                        widget_get(NULL, "buffers", DLGA_LBTEXT, item);
                    int bufid = scrap_bufid(value);

                    if (bufid >= 0) {
                        if (bufid == inq_scrap()) {
                            error("Cannot delete the current scrap.");
                        } else {
                            widget_set(NULL, "buffers", DLGA_LBREMOVE, item);
                            delete_buffer(bufid);
                            message("Scrap deleted.");
                        }
                    }
                }
            }
            break;
        case "exit":
            dialog_exit();
            break;
        case "help":
            execute_macro("explain \"Named Scrap\"");
            break;
        }
    }
    return TRUE;
}

/*end*/
