/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: rxvt.cr,v 1.3 2014/10/22 02:34:41 ayoung Exp $
 * rxvt support
 *
 *
 */

#include "tty.h"
#include "tty_xterm.h"

void
main()
{
    load_macro("tty/xterm_rxvt", FALSE);        /* redirect */
}


void
rxvt_cygwin()
{
}


void
rxvt_native()
{
}

