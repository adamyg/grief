/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: colorlabels.cr,v 1.9 2024/10/07 16:23:06 cvsuser Exp $
 * Color label support.
 *
 *
 */

#include "grief.h"

extern list             color_labels;           // global color labels.

void                    colorlabels(void);

static void             cl_keys(void);
static void             cl_up(void);
static void             cl_down(void);
static void             cl_pgup(void);
static void             cl_pgdn(void);
static void             cl_enter(void);

static void             rgb_init(void);
static void             rgb_load(string file);

static list             xterm_colors = {
    "black",
    "red3",
    "green3",
    "yellow3",
    "blue3",
    "magenta3",
    "cyan3",
    "gray90",
    "gray50",
    "red",
    "green",
    "yellow",
    "rgb:5c/5c/ff",
    "magenta",
    "cyan",
    "white"
    };

void
main(void)
{
    if (0 == length_of_list(color_labels)) {    // load once
        rgb_init();
    }
}


void
colorlabels(void)
{
    select_list("Color Labels", "select or <esc>", -2, color_labels, SEL_CENTER,
                        NULL, NULL, 0, inq_module() + "::cl_keys");
}


static void
cl_keys(void)
{
    assign_to_key("<Enter>",        "::cl_enter");
    assign_to_key("<Up>",           "::cl_up");
    assign_to_key("<Down>",         "::cl_down");
    assign_to_key("<Wheel-Up>",     "::cl_up");
    assign_to_key("<Wheel-Down>",   "::cl_down");
    assign_to_key("<PgUp>",         "::cl_pgup");
    assign_to_key("<PgDn>",         "::cl_pgdn");
    cl_enter();
}


static void
cl_up(void)
{
    sel_up();
    cl_enter();
}


static void
cl_down(void)
{
    sel_down();
    cl_enter();
}


static void
cl_pgup(void)
{
    sel_pgup();
    cl_enter();
}


static void
cl_pgdn(void)
{
    sel_pgdn();
    cl_enter();
}


static void
cl_enter(void)
{
    int idx;

    inq_position(idx); --idx; idx *= 2;
    if (idx < length_of_list(color_labels)) {
        message("%s: %s", color_labels[idx], color_labels[idx+1]);
    }
}


//
//  Load RGB name mapping table.
//
static void
rgb_init(void)
{
    const list paths = {                        // rgb color table
        "${X11ROOT}/rgb.txt",
        "/etc/X11/rgb.txt",
        "/usr/share/X11/rgb.txt",
        "/usr/X11R6/lib/X11/rgb.txt",
        "${GRPATH}/colors/rgb.txt"              // FIXME - must split
        };
    string path;

    for (list_reset(paths); list_each(paths, path) >= 0;) {
        path = expandpath(path, TRUE);
        if (exist(path)) {
            rgb_load(path);
            return;
        }
    }
    load_macro("colors/util/rgb256");           // default
}

static void
rgb_load(string file)
{
    int buf, curbuf = inq_buffer();
    int dlevel = debug(0, FALSE);

    if ((buf = create_buffer("-colorscheme-rgb-", file, TRUE)) >= 0) {
        string line, name;
        int r, g, b;

        set_buffer(buf);
        top_of_buffer();
        do {
            //  255 250 250     snow
            //  248 248 255     ghost white
            //  248 248 255     GhostWhite
            //   :   :   :          :
            //
            line = read();
            if (4 == sscanf(line, "%d %d %d %[^\n]\n", r, g, b, name)) {
                if (strlen(name) >= 3 &&
                        r >= 0 && r <= 255 && g >= 0 && g <= 255 && b >= 0 && b <= 255) {
                    sprintf(line, "#%02X%02X%02X", r, g, b);
                    color_labels += name;
                    color_labels += line;
                }
            }
        } while (down());
        set_buffer(curbuf);
        delete_buffer(buf);
    }
    debug(dlevel, FALSE);
}

/*end*/
