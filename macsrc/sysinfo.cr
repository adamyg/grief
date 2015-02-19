/* -*- indent-width: 4; -*- */
/* $Id: sysinfo.cr,v 1.15 2014/10/27 23:28:29 ayoung Exp $
 * System information Dialog.
 *
 *
 */

#include "grief.h"


void
sysinfo()
{
    int dialog = dialog_create(make_list(
        DLGA_TITLE,                     "System Info",
        DLGA_CALLBACK,                  "::sysinfo_cb",

        DLGC_CONTAINER,
            DLGC_CONTAINER,
                DLGA_ATTACH_LEFT,
                DLGA_ALIGN_NW,
                DLGC_LABEL,
                    DLGA_LABEL,         "version",
                    DLGA_COLS,          12,
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "compiled",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "build",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "machtype",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "uname",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "home",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "profile",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "grinit",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "tmpdir",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "username",
                    DLGA_ALIGN_W,
                DLGC_LABEL,
                    DLGA_LABEL,         "hostname",
                    DLGA_ALIGN_W,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_RIGHT,
                DLGA_ALIGN_NE,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "version",
                    DLGA_ROWS,          1,
                    DLGA_COLS,          60,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "compiled",
                    DLGA_ROWS,          1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "build",
                    DLGA_ROWS,          1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "machtype",
                    DLGA_ROWS,          1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "uname",
                    DLGA_ROWS,          1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "home",
                    DLGA_ROWS,          1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "profile",
                    DLGA_ROWS,          1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "grinit",
                    DLGA_ROWS,          1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "tmpdir",
                    DLGA_ROWS,          1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "username",
                    DLGA_ROWS,          1,
                    DLGA_ALLOW_FILLX,
                DLGC_LABEL,
                    DLGA_ALIGN_E,
                    DLGA_NAME,          "hostname",
                    DLGA_ROWS,          1,
                    DLGA_ALLOW_FILLX,
            DLGC_END,
        DLGC_END,

        DLGC_CONTAINER,
            DLGC_PUSH_BUTTON,
                DLGA_LABEL,             "&Done",
                DLGA_NAME,              "done",
                DLGA_ALLOW_FILLX,
                DLGA_DEFAULT_BUTTON,

        DLGC_END
        ));

    dialog_run(dialog);
    dialog_delete(dialog);
}


static void
sysinfo_cb(int ident, string name, int p1, int p2)
{
    UNUSED(p2);

    switch (p1) {
    case DLGE_INIT: {
            int maj, min, edit, rel, cmver;
            string machtype, compiled, build;
            string usysname, unodename, uversion, urelease, umachine;
            string buf;

            version(maj, min, edit, rel, machtype, compiled, cmver, NULL, build);
            sprintf(buf, "%s v%d.%d.%d (%d)", APPNAME, maj, min, edit, cmver);
            widget_set(ident, "version", buf);
            widget_set(ident, "compiled", compiled);
            widget_set(ident, "build", build);
            widget_set(ident, "machtype", machtype);

            uname(usysname, unodename, uversion, urelease, umachine);
            sprintf(buf, "%s %s %s %s %s", usysname, unodename, urelease, uversion, umachine);
            widget_set(ident, "uname", buf);

            widget_set(ident, "home", inq_home());
            widget_set(ident, "profile", inq_profile());
            widget_set(ident, "grinit", inq_grinit());
            widget_set(ident, "tmpdir", inq_tmpdir());
            widget_set(ident, "username", inq_username());
            widget_set(ident, "hostname", inq_hostname());
        }
        break;

    case DLGE_BUTTON:
        if (name == "done") {
            dialog_exit();
        }
        break;
    }
}

/*end*/
