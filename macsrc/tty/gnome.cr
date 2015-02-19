/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: gnome.cr,v 1.1 2011/11/03 23:42:01 cvsuser Exp $
 * gnome terminal support
 *
 *      gnome
 *      gnome-256color
 *      gnome-2007
 *      gnome-2008
 *      gnome-fc5
 *      gnome-rh62
 *      gnome-rh80
 *      gnome-rh90
 *
 */

#include "tty.h"
#include "tty_xterm.h"

void
main()
{
    load_macro("tty/xterm_gnome", FALSE);       /* redirect */
}

