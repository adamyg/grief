/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: errnos.cr,v 1.5 2014/10/27 23:28:21 ayoung Exp $
 * errno support.
 *
 *
 *
 */

#include "grief.h"

void
errnos(void)
{
    string desc, manifest;
    list errnos;
    int i;

    for (i = 0; i <= 499; ++i) {                // unix
        desc = strerror(i, manifest, TRUE);
        if (manifest != "EUNKNOWNERR") {
            errnos += format("%-5d %-22s %s", i, manifest, desc);
        }
    }

    for (i = 10000; i <= 11999; ++i) {          // windows, socket errno's 10000+
        desc = strerror(i, manifest, TRUE);
        if (manifest != "EUNKNOWNERR") {
            errnos += format("%-5d %-22s %s", i, manifest, desc);
        }
    }

    select_list("errno chart", "", 1, errnos);
}

/*end*/
