/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: add_int.cr,v 1.2 2020/04/20 23:13:53 cvsuser Exp $
 *
 * LUA/DukTape/Grunch comparision benchmark
 * See:
 *  https://github.com/svaarala/duktape/tree/master/tests/perf
 *  https://wiki.duktape.org
 */

#include "../grief.h"

#if defined(__REGISTERS__)
#define REGISTER register                       /* 1/4/2020 */
#else
#pragma message("WARNING: registers not available ...")
#define REGISTER
#endif

void
benchmark_add_int(void)
{
    REGISTER int i;
    REGISTER int x = 123;
    REGISTER int t;

//  for (i = 0; i < 1e7; ++i) {
    for (i = 0; i < 1e6; ++i) {
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
    }
}

 /*end*/
