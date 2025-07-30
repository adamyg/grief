/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: assign_literal.cr,v 1.4 2025/01/10 15:12:25 cvsuser Exp $
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
benchmark_assign_literal(void)
{
    REGISTER int i;
    REGISTER declare t;
    REGISTER int this = -1;

//  for (i = 0; i < 10000000; i++) {
    for (i = 0; i < 1000000; ++i) {
        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;
        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;

        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;
        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;

        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;
        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;

        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;
        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;

        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;
        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;

        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;
        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;

        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;
        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;

        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;
        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;

        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;
        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;

        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;
        t = true;
        t = false;
        t = /*void*/ NULL;
        t = NULL;
        t = this;
    }
}

/*end*/
