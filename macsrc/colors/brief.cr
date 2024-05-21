/* -*- mode: cr; indent-width: 8; -*- */
/* $Id: brief.cr,v 1.8 2024/05/20 17:21:07 cvsuser Exp $
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
        "string=light-magenta",
        "operator=yellow",
        "number=light-magenta",
        "comment=white:italic",
        "preprocessor=light-green",
        "preprocessor_keyword=light-green",
        "whitespace=white,light-red",
        "delimiter=cyan",
        "code=white",
        "word=yellow",
        "todo=brown",
        "spell=red,underline"
        };

void
colorscheme_brief(void)
{
        set_color(brief16_colors);
}
