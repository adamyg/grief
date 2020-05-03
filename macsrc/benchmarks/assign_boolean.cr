/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: assign_boolean.cr,v 1.2 2020/04/20 23:13:53 cvsuser Exp $
 *
 * LUA/DukTape/Grunch comparision benchmark
 */

#include "../grief.h"

#if defined(__REGISTERS__)
#define REGISTER register                       /* 1/4/2020 */
#else
#pragma message("WARNING: registers not available ...")
#define REGISTER
#endif

void
benchmark_assign_boolean(void)
{
    REGISTER int i;
    REGISTER int t;

//  for (i = 0; i < 1e7; i++) {
    for (i = 0; i < 1e6; ++i) {
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
    }
}

 /*end*/