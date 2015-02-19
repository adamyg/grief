/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: signos.cr,v 1.3 2014/10/27 23:28:28 ayoung Exp $
 * Signal support.
 *
 *
 */

#include "grief.h"

void
signos(void)
{
    string desc, manifest;
    list signos;
    int i;

    for (i = 0; i <= 99; ++i) {
        desc = strsignal(i, manifest, TRUE);
        if (manifest != "SIGUNKNOWN") {
            signos += format("%-5d %-22s %s", i, manifest, desc);
        }
    }

    select_list("signal number chart", "", 1, signos);
}

/*end*/
