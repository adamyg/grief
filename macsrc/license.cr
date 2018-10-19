/* -*- indent-width: 4; -*- */
/* $Id: license.cr,v 1.9 2018/10/01 21:05:01 cvsuser Exp $
 * License information.
 *
 *
 */

#include "grief.h"

#define LICENSE_FILE    "license.txt"
#define LICENSE_SOURCE  "http://griefedit.sourceforge.net/license.txt"
#define LICENSE_TEXT    1001

static list             license_import(void);

static int              dialog;                 // dialog handle


void
main()
{
    list license_text = {
            "",
            "   Glorious Reconfigurable Interactive Editing Facility",
            "",
            "                __________  ________________",
            "               / ____/ __ \\/  _/ ____/ ____/",
            "              / / __/ /_/ // // __/ / /_",
            "             / /_/ / _, _// // /___/ __/",
            "             \\____/_/ |_/___/_____/_/",
            "",
            "         1000111 1110010 1101001 1100101 1100110",
            "",
            "Copyright (c) 1998 - 2018, Adam Young.",
            "All Rights Reserved.",
            "",
            "Derived from Crisp2.2, by Paul Fox, 1991.",
            "",
            "Please help publish and sponsor " + APPNAME + " development !",
            "",
            APPNAME + " is open software: you can use, redistribute it",
            "and/or modify it under the terms of the " + APPNAME + " License.",
            "",
            "THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND",
            "ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE",
            "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE",
            "ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE",
            "FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL",
            "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS",
            "OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)",
            "HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT",
            "LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY",
            "OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF",
            "SUCH DAMAGE.",
            "",
            "           ---------------------------------------------------",
            };
    license_text += license_import();

    int maj, min, edit;
    string verbuf;

    version(maj, min, edit);
    sprintf(verbuf, "%s v%d.%d.%d", APPNAME, maj, min, edit);

    dialog =
        dialog_create( make_list(
            DLGA_TITLE,                         verbuf,
            DLGA_CALLBACK,                      "::license_callback",

            DLGC_CONTAINER,
                DLGA_ATTACH_TOP,
                DLGC_LIST_BOX,
                    DLGA_ROWS,                  14,
                    DLGA_COLS,                  78,
                    DLGA_LBDUPLICATES,          TRUE,
                    DLGA_LBELEMENTS,            license_text,
                    DLGA_ALLOW_FILLX,
            DLGC_END,

            DLGC_CONTAINER,
                DLGA_ATTACH_BOTTOM,
                DLGA_ALIGN_CENTER,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Accept",
                    DLGA_NAME,                  "accept",
                    DLGA_ATTACH_LEFT,
                    DLGA_DEFAULT_FOCUS,
                    DLGA_DEFAULT_BUTTON,
                DLGC_PUSH_BUTTON,
                    DLGA_LABEL,                 "&Cancel",
                    DLGA_NAME,                  "cancel",
                    DLGA_ATTACH_LEFT,
            DLGC_END
            ));
}


static list
license_import(void)
{
    const int curbuf = inq_buffer();
    string source = help_resolve(LICENSE_FILE);
    string line;
    list text;

    push(text, "");
    if (0 == access(source, 0) &&
            edit_file(EDIT_SYSTEM, source) != -1) {
        while ((line = read()) != "") {         // while (!eof)
            rtrim(line, "\r\n\t ");             // remove new-line
            push(text, line);
            down();
        }
        set_buffer(curbuf);
        attach_buffer(curbuf);
    } else {
        push(text, "unable to locate license \"" + LICENSE_FILE + "\"");
        push(text, "");
        push(text, "please consult " + LICENSE_SOURCE);
    }
    push(text, "");
    return text;
}


void
license(void)
{
    dialog_run(dialog);
}


static void
license_callback(int ident, string name, int p1, int p2)
{
    UNUSED(ident, p2);
    switch (p1) {
    case DLGE_INIT:
        break;
    case DLGE_BUTTON:
        switch(name) {
        case "accept":
            dialog_exit();
            break;
        case "cancel":
            dialog_exit();
            exit();
            break;
        }
        break;
    }
}

/*eof*/
