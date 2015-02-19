/* -*- mode: cr; indent-width: 4; -*-
 * $Id: ansi.cr,v 1.3 2014/10/27 23:28:32 ayoung Exp $
 * ANSI support.
 *
 *
 */

#include "../grief.h"

string
_ansi_mode()
{
   return "ansi";
}

string
_ansi_highlight_first()
{
    set_buffer_flags(NULL, "ansi");
    return "";
}
