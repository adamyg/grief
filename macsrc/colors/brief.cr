/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: brief.cr,v 1.9 2024/07/11 10:42:49 cvsuser Exp $
 * Brief/Borland coloriser.
 *
 *
 */

#include "../grief.h"

static list
brief16_colors = {              /*basic brief coloriser specification*/
        "scheme=default",
        "background=dark-blue",
        "normal=light-white",
        "select=light-cyan",
        "message=light-green",
        "error=red",
        "hilite=magenta",
        "hilite_fg=light-white",
        "standout=light-cyan",
        "frame=white",
        "whitespace=white,light-red",

        "spell=red,underline",
        "todo=brown",
        "code=white",
        "string=light-magenta",
        "operator=yellow",
        "number=light-magenta",
        "delimiter=cyan",
        "word=yellow",

        "preprocessor=light-green",
        "preprocessor_keyword=light-green",
        "keyword=white"
        };

void
colorscheme_brief(void)
{
        set_color(brief16_colors);
}
