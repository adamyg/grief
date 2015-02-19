/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: defaultuser.cr,v 1.1 2014/10/22 02:34:36 ayoung Exp $
 * Default user.
 *
 *
 */

#include "../grief.h"

void
main(void)
{
    set_backup_option(BK_ASK, NULL, 10*1024);   /* prompt for backup >= 10Mb */

    display_mode(DC_ROSUFFIX|DC_MODSUFFIX);     /* add read-only/modified suffix */

                                                /* control attributes */
    set_ctrl_state(WCTRLO_VERT_SCROLL, WCTRLS_ENABLE);
    set_ctrl_state(WCTRLO_HORZ_SCROLL, WCTRLS_ENABLE);

    load_macro("scrollfixed");                  /* scroll without cursor movement */
        // <Ctrl-Up-Arrow>
        // <Ctrl-Down-Arrow>
        // <Alt-Up-Arrow>
        // <Alt-Down-Arrow>
}

