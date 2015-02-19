/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: window.cr,v 1.6 2014/10/27 23:28:34 ayoung Exp $
 * Profile when GRIEF is running under a windowing system.
 *
 *  It avoids us having to define key bindings, because they're built into GRIEF or configurable
 *  via the standard resource management scheme. We just need to tell it simple things like we
 *  have color.
 *
 *
 */

#include "tty.h"

void
main()
{
    set_term_feature(TF_COLOR, TRUE);
}

/*end*/
