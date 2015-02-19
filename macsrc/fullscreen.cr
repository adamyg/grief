/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: fullscreen.cr,v 1.2 2015/02/19 00:17:41 ayoung Exp $
 * full-screen (Minimum-Maximum) console windows
 *
 *
 */

#include "grief.h"

static void
fullscreen_onexit()
{
    ega(-2);
}

void
main(void)
{
    register_macro(REG_EXIT, "::fullscreen_onexit");
}

void
fullscreen(void)
{
    ega(-1);
}
