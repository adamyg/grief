/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: add_float.cr,v 1.3 2023/03/06 15:28:18 cvsuser Exp $
 *
 * LUA/DukTape/Grunch comparison benchmark
 */

#include "../grief.h"

#if defined(__REGISTERS__)
#define REGISTER register                       /* 1/4/2020 */
#else
#pragma message("WARNING: registers not available ...")
#define REGISTER
#endif

void
benchmark_add_float(void)
{
    REGISTER int i;
    REGISTER float x = 123.1;
    REGISTER float y = 234.2;
    REGISTER float t;

//  for (i = 0; i < 1e7; i++) {
    for (i = 0; i < 1e6; ++i) {
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
    }
}

 /*end*/
