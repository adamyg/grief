#include <edidentifier.h>
__CIDENT_RCSID(gr_m_random_c,"$Id: m_random.c,v 1.10 2025/02/07 03:03:21 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/* $Id: m_random.c,v 1.10 2025/02/07 03:03:21 cvsuser Exp $
 * Random primitives.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */

#include <editor.h>
#include <math.h>

#include "m_random.h"

#include "accum.h"                              /* acc_...() */
#include "eval.h"                               /* isa_/get_..() */

static int              x_rand_init = FALSE;
static char             x_rand_state[256];

static void             bsd_srandom(unsigned int x);
static char *           bsd_initstate(unsigned int seed, char *arg_state, size_t n);
static char *           bsd_setstate(const char *arg_state);
static long             bsd_random(void);

#define SRAND(__seed)   bsd_srandom(__seed)
#define RAND()          bsd_random()


/*  Function:           do_rand
 *      rand primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: rand - Generate a random number

        int
        rand([int upper])

    Macro Description:
        The 'rand()' primitive computes a sequence of pseudo-random
        integers in the range 0 to 'RAND_MAX'.

        'rand' will by default produce a sequence of numbers that can be
        duplicated by calling srand() with '1' as the seed.

        The 'srand' primitive can be use to set/reset to random seed
        plus modify the generator table depth. This implementation uses
        a non-linear additive feedback random number generator employing
        a default table of size 31 long integers to return successive
        pseudo-random numbers in the range from 0 to (2**31)-1.

        The period of this random number generator is very large,
        approximately 16*((2**31)-1).

    Macro Parameters:
        upper - Optional integer stating the upper range of the returned
            random number, if omitted 2^32.

    Macro Returns:
        The 'rand' primitive returns a random number in the range
        '0..2^31' or '0..upper' if a positive 'upper' value is stated.

    Macro See Also:
        srand
 */
void
do_rand(void)                   /* int ([int upper]) */
{
    accint_t value;

    if (! x_rand_init) {
     /* SRAND((unsigned int)time(NULL)); */
        SRAND(1);
        x_rand_init = TRUE;
    }

 /* value =  RAND() << 21; */
 /* value += RAND() << 10; */
 /* value += RAND() >> 3; */
 /* value &= 0x7fffffffL; */

    value = RAND();
    if (value < 0) value *= -1;                 /* guard */

    if (isa_integer(1)) {                       /* upper limit */
        const int upper = get_xinteger(1, 0);
        if (upper > 0) {
            value = value % upper;
        }
    }
    acc_assign_int(value);
}


/*  Function:           do_srand
 *      srand primitive.
 *
 *  Parameters:
 *      none.
 *
 *  Returns:
 *      nothing.
 *
 *<<GRIEF>>
    Macro: srand - Seed the random number generator.

        int
        srand([int seed = time()], [int depth])

    Macro Description:
        The 'srand()' primitive initialises the random number generator
        based on the given seeds.

        The 'seed' is used to prime the generator running at the
        specified 'depth'.

        By default, the package runs with 128 bytes of state information
        and generates far better random numbers than a linear
        congruential generator. If the amount of state information is
        less than 32 bytes, a simple linear congruential R.N.G. is used.

    Macro Parameters:
        seed - Basic initial primer, if omitted defaults to the current
            value of <time>.

        depth - Optional integer depth controlling how sophisticated the
            random number generator shall be.

            Current "optimal" values for the amount of state information
            are '8', '32', '64', '128', and '256' bytes; other amounts
            will be rounded down to the nearest known amount. Using less
            than 8 bytes will cause an error.

    Macro Returns:
        The 'srand()' primitive returns
        0 on success, otherwise -1 on
        error.

    Macro See Also:
        rand
 */
void
do_srand(void)                  /* int ([int seed = time()], [int depth]) */
{
    unsigned int seed;
    int ret = 0;

    x_rand_init = TRUE;

    if (isa_integer(1)) {
        seed = (unsigned int)get_integer(1);
    } else {
        seed = (unsigned int)time(NULL);
    }

    if (isa_integer(2)) {                       /* extension, optional size 8, 32, 64, 128 or 256 */
        /*
        //  The initstate() routine allows a state array, passed in as an argument, to be initialized
        //  for future use. The size of the state array (in bytes) is used by initstate() to decide
        //  how sophisticated a random number generator it should use the more state, the better the
        //  random numbers will be.
        //
        //  Current "optimal" values for the amount of state information are 8, 32, 64, 128, and
        //  256 bytes; other amounts will be rounded down to the nearest known amount. Using less
        //  than 8 bytes will cause an error.
        //
        //  The seed for the initialization (which specifies a starting point for the random
        //  number sequence, and provides for restarting at the same point) is also an argument.
        */
        size_t n = (size_t)get_integer(2);

        if (n < 8) n = 8;                       /* lower bound */
        else if (n > 256) n = 256;              /* upper */

        if (NULL == bsd_initstate(seed, x_rand_state, n)) {
            ret = -1;
        }
    } else {
        SRAND(seed);                            /* seed uses current depth, default 128 */
    }

    acc_assign_int(ret);
}


/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * random.c:
 *
 * An improved random number generation package.  In addition to the standard
 * rand()/srand() like interface, this package also has a special state info
 * interface.  The initstate() routine is called with a seed, an array of
 * bytes, and a count of how many bytes are being passed in; this array is
 * then initialized to contain information for random number generation with
 * that much state information.  Good sizes for the amount of state
 * information are 32, 64, 128, and 256 bytes.  The state can be switched by
 * calling the setstate() routine with the same array as was initiallized
 * with initstate().  By default, the package runs with 128 bytes of state
 * information and generates far better random numbers than a linear
 * congruential generator.  If the amount of state information is less than
 * 32 bytes, a simple linear congruential R.N.G. is used.
 *
 * Internally, the state information is treated as an array of int32_t; the
 * zeroeth element of the array is the type of R.N.G. being used (small
 * integer); the remainder of the array is the state information for the
 * R.N.G.  Thus, 32 bytes of state information will give 7 int32_ts worth of
 * state information, which will allow a degree seven polynomial.  (Note:
 * the zeroeth word of state information also has some other information
 * stored in it -- see setstate() for details).
 *
 * The random number generation technique is a linear feedback shift register
 * approach, employing trinomials (since there are fewer terms to sum up that
 * way).  In this approach, the least significant bit of all the numbers in
 * the state table will act as a linear feedback shift register, and will
 * have period 2^deg - 1 (where deg is the degree of the polynomial being
 * used, assuming that the polynomial is irreducible and primitive).  The
 * higher order bits will have longer periods, since their values are also
 * influenced by pseudo-random carries out of the lower bits.  The total
 * period of the generator is approximately deg*(2**deg - 1); thus doubling
 * the amount of state information has a vast influence on the period of the
 * generator.  Note: the deg*(2**deg - 1) is an approximation only good for
 * large deg, when the period of the shift register is the dominant factor.
 * With deg equal to seven, the period is actually much longer than the
 * 7*(2**7 - 1) predicted by this formula.
 */

#define TYPE_0          0               /* linear congruential */
#define BREAK_0         8
#define DEG_0           0
#define SEP_0           0

#define TYPE_1          1               /* x**7 + x**3 + 1 */
#define BREAK_1         32
#define DEG_1           7
#define SEP_1           3

#define TYPE_2          2               /* x**15 + x + 1 */
#define BREAK_2         64
#define DEG_2           15
#define SEP_2           1

#define TYPE_3          3               /* x**31 + x**3 + 1 */
#define BREAK_3         128
#define DEG_3           31
#define SEP_3           3

#define TYPE_4          4               /* x**63 + x + 1 */
#define BREAK_4         256
#define DEG_4           63
#define SEP_4           1

/*
 * Array versions of the above information to make code run faster --
 * relies on fact that TYPE_i == i.
 */
#define MAX_TYPES       5               /* max number of types above */

static int      degrees[MAX_TYPES] =    { DEG_0, DEG_1, DEG_2, DEG_3, DEG_4 };
static int      seps [MAX_TYPES] =      { SEP_0, SEP_1, SEP_2, SEP_3, SEP_4 };

/*
 * Initially, everything is set up as if from:
 *
 *      initstate(1, &randtbl, 128);
 *
 * Note that this initialization takes advantage of the fact that srandom()
 * advances the front and rear pointers 10*rand_deg times, and hence the
 * rear pointer which starts at 0 will also end up at zero; thus the zeroeth
 * element of the state information, which contains info about the current
 * position of the rear pointer is just
 *
 *      MAX_TYPES * (rptr - state) + TYPE_3 == TYPE_3.
 */

static int32_t randtbl[DEG_3 + 1] = {
        TYPE_3,
        0x991539b1, 0x16a5bce3, 0x6774a4cd, 0x3e01511e, 0x4e508aaa, 0x61048c05,
        0xf5500617, 0x846b7115, 0x6a19892c, 0x896a97af, 0xdb48f936, 0x14898454,
        0x37ffd106, 0xb58bff9c, 0x59e17104, 0xcf918a49, 0x09378c83, 0x52c7a471,
        0x8d293ea9, 0x1f4fc301, 0xc3db71be, 0x39b44e1c, 0xf8a44ef9, 0x4c8b80b1,
        0x19edc328, 0x87bf4bdd, 0xc9b240e5, 0xe9ee4b1b, 0x4382aee7, 0x535b6b41,
        0xf3bec5da,
};

/*
 * fptr and rptr are two pointers into the state info, a front and a rear
 * pointer.  These two pointers are always rand_sep places aparts, as they
 * cycle cyclically through the state information.  (Yes, this does mean we
 * could get away with just one pointer, but the code for random() is more
 * efficient this way).  The pointers are left positioned as they would be
 * from the call
 *
 *      initstate(1, randtbl, 128);
 *
 * (The position of the rear pointer, rptr, is really 0 (as explained above
 * in the initialization of randtbl) because the state table pointer is set
 * to point to randtbl[1] (as explained below).
 */
static int32_t *        fptr            = &randtbl[SEP_3 + 1];
static int32_t *        rptr            = &randtbl[1];

/*
 * The following things are the pointer to the state information table, the
 * type of the current generator, the degree of the current polynomial being
 * used, and the separation between the two pointers.  Note that for efficiency
 * of random(), we remember the first location of the state information, not
 * the zeroeth.  Hence it is valid to access state[-1], which is used to
 * store the type of the R.N.G.  Also, we remember the last location, since
 * this is more efficient than indexing every time to find the address of
 * the last element to see if the front and rear pointers have wrapped.
 */
static int32_t *        state           = &randtbl[1];
static int32_t *        end_ptr         = &randtbl[DEG_3 + 1];
static int              rand_type       = TYPE_3;
static int              rand_deg        = DEG_3;
static int              rand_sep        = SEP_3;

/*
 * srandom:
 *
 * Initialize the random number generator based on the given seed.  If the
 * type is the trivial no-state-information type, just remember the seed.
 * Otherwise, initializes state[] based on the given "seed" via a linear
 * congruential generator.  Then, the pointers are set to known locations
 * that are exactly rand_sep places apart.  Lastly, it cycles the state
 * information a given number of times to get rid of any initial dependencies
 * introduced by the L.C.R.N.G.  Note that the initialization of randtbl[]
 * for default usage relies on values produced by this routine.
 */
static void
bsd_srandom(unsigned int x)
{
        int i;
        int32_t test;
        div_t val;

        if (rand_type == TYPE_0)
                state[0] = x;
        else {
                state[0] = x;
                for (i = 1; i < rand_deg; i++) {
                        /*
                         * Implement the following, without overflowing 31 bits:
                         *
                         *      state[i] = (16807 * state[i - 1]) % 2147483647;
                         *
                         *      2^31-1 (prime) = 2147483647 = 127773*16807+2836
                         */
                        val = div(state[i-1], 127773);
                        test = 16807 * val.rem - 2836 * val.quot;
                        state[i] = test + (test < 0 ? 2147483647 : 0);
                }
                fptr = &state[rand_sep];
                rptr = &state[0];
                for (i = 0; i < 10 * rand_deg; i++)
                        (void)bsd_random();
        }
}

/*
 * initstate:
 *
 * Initialize the state information in the given array of n bytes for future
 * random number generation.  Based on the number of bytes we are given, and
 * the break values for the different R.N.G.'s, we choose the best (largest)
 * one we can and set things up for it.  srandom() is then called to
 * initialize the state information.
 *
 * Note that on return from srandom(), we set state[-1] to be the type
 * multiplexed with the current value of the rear pointer; this is so
 * successive calls to initstate() won't lose this information and will be
 * able to restart with setstate().
 *
 * Note: the first thing we do is save the current state, if any, just like
 * setstate() so that it doesn't matter when initstate is called.
 *
 * Returns a pointer to the old state.
 */
static char *
bsd_initstate(unsigned int seed, char *arg_state, size_t n)
{
        char *ostate = (char *)(&state[-1]);

        if (rand_type == TYPE_0)
                state[-1] = rand_type;
        else
                state[-1] = MAX_TYPES * (int32_t)(rptr - state) + rand_type;
        if (n < BREAK_0)
                return(NULL);
        if (n < BREAK_1) {
                rand_type = TYPE_0;
                rand_deg  = DEG_0;
                rand_sep  = SEP_0;
        } else if (n < BREAK_2) {
                rand_type = TYPE_1;
                rand_deg  = DEG_1;
                rand_sep  = SEP_1;
        } else if (n < BREAK_3) {
                rand_type = TYPE_2;
                rand_deg  = DEG_2;
                rand_sep  = SEP_2;
        } else if (n < BREAK_4) {
                rand_type = TYPE_3;
                rand_deg  = DEG_3;
                rand_sep  = SEP_3;
        } else {
                rand_type = TYPE_4;
                rand_deg  = DEG_4;
                rand_sep  = SEP_4;
        }
        state = &(((int32_t *)arg_state)[1]);   /* first location */
        end_ptr = &state[rand_deg];             /* must set end_ptr before srandom */
        SRAND(seed);
        if (rand_type == TYPE_0)
                state[-1] = rand_type;
        else
                state[-1] = MAX_TYPES*((int32_t)(rptr - state)) + rand_type;
        return(ostate);
}

/*
 * setstate:
 *
 * Restore the state from the given state array.
 *
 * Note: it is important that we also remember the locations of the pointers
 * in the current state information, and restore the locations of the pointers
 * from the old state information.  This is done by multiplexing the pointer
 * location into the zeroeth word of the state information.
 *
 * Note that due to the order in which things are done, it is OK to call
 * setstate() with the same state as the current state.
 *
 * Returns a pointer to the old state information.
 */
static char *
bsd_setstate(const char *arg_state)
{
        int32_t *new_state = (int32_t *)arg_state;
        int32_t type = new_state[0] % MAX_TYPES;
        int32_t rear = new_state[0] / MAX_TYPES;
        char *ostate = (char *)(&state[-1]);

        if (rand_type == TYPE_0)
                state[-1] = rand_type;
        else
                state[-1] = MAX_TYPES * (int32_t)(rptr - state) + rand_type;
        switch(type) {
        case TYPE_0:
        case TYPE_1:
        case TYPE_2:
        case TYPE_3:
        case TYPE_4:
                rand_type = type;
                rand_deg = degrees[type];
                rand_sep = seps[type];
                break;
        default:
                return(NULL);
        }
        state = &new_state[1];
        if (rand_type != TYPE_0) {
                rptr = &state[rear];
                fptr = &state[(rear + rand_sep) % rand_deg];
        }
        end_ptr = &state[rand_deg];             /* set end_ptr too */
        return(ostate);
}

/*
 * random:
 *
 * If we are using the trivial TYPE_0 R.N.G., just do the old linear
 * congruential bit.  Otherwise, we do our fancy trinomial stuff, which is
 * the same in all the other cases due to all the global variables that have
 * been set up.  The basic operation is to add the number at the rear pointer
 * into the one at the front pointer.  Then both pointers are advanced to
 * the next location cyclically in the table.  The value returned is the sum
 * generated, reduced to 31 bits by throwing away the "least random" low bit.
 *
 * Note: the code takes advantage of the fact that both the front and
 * rear pointers can't wrap on the same call by not testing the rear
 * pointer if the front one has wrapped.
 *
 * Returns a 31-bit random number.
 */
static long
bsd_random(void)
{
        int32_t i;

        if (rand_type == TYPE_0)
                i = state[0] = (state[0] * 1103515245 + 12345) & 0x7fffffff;
        else {
                *fptr += *rptr;
                i = (*fptr >> 1) & 0x7fffffff;  /* chucking least random bit */
                if (++fptr >= end_ptr) {
                        fptr = state;
                        ++rptr;
                } else if (++rptr >= end_ptr)
                        rptr = state;
        }
        return((long)i);
}

/*end*/
