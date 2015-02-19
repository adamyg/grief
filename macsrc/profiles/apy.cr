/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: apy.cr,v 1.21 2014/10/22 02:34:36 ayoung Exp $
 * User configuration.
 *
 *
 */

#include "../grief.h"

void
main(void)
{
    save_state();                               /* turn on full state saving */

    set_backup_option(BK_ASK, NULL, 10*1024);   /* prompt for backup >= 10Mb */

    display_mode(DC_ROSUFFIX|DC_MODSUFFIX);     /* add read-only/modified suffix */

                                                /* control attributes */
    set_ctrl_state(WCTRLO_CLOSE_BTN,   WCTRLS_ENABLE);
    set_ctrl_state(WCTRLO_ZOOM_BTN,    WCTRLS_ENABLE);
    set_ctrl_state(WCTRLO_VERT_SCROLL, WCTRLS_ENABLE);
    set_ctrl_state(WCTRLO_HORZ_SCROLL, WCTRLS_ENABLE);

    load_macro("scrollfixed");                  /* scroll without cursor movement */
        // <Ctrl-Up-Arrow>
        // <Ctrl-Down-Arrow>
        // <Alt-Up-Arrow>
        // <Alt-Down-Arrow>

    load_macro("short");                        /* short command set */
}
/*eof*/
