/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: unittest.cr,v 1.1 2025/01/17 08:44:53 cvsuser Exp $
 * unittest
 *
 *
 */

#include "grief.h"

extern int regress();

int
unittest()
{
    // externals
    autoload("demos/regress",
        "regress");
    autoload("region",
        "cut");
    autoload("misc",
        "gsub");

    // execute
    set_macro_history(NULL, "unittest");
    int ret = regress();
    error("unittest=%d", ret);
    return ret;
}

//end

