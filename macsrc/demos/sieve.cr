/* -*- mode: cr; indent-width: 4; -*- */
/* $Id: sieve.cr,v 1.7 2014/10/22 02:34:30 ayoung Exp $
 * Sieve of Eratosthenes -- prime numbers
 *
 *  This macro isn't really that useful but its a demonstration
 *  of how slow (exponentially) list processing is, or
 *  conversely, how fast list processing is as I do optimisations
 *  to the internal code of CRISP.
 *
 *
 */

#include "../grief.h"

list            calc_primes(int nprimes);

void
sieve()
{
    int nprimes;
    list bucket;
    int i, col;
    string buf;

    if (get_parm(0, nprimes, "Number of primes: ") <= 0 || nprimes <= 0)
        nprimes = 100;
    edit_file("Primes");
    clear_buffer();

    bucket = calc_primes(nprimes);

    for (i = 1; i < nprimes; i += 2)
        if (bucket[i]) {
            sprintf(buf, "%5d ", i);
            insert(buf);
            inq_position(NULL, col);
            if (col > 60) {
                insert("\n");
            }
        }
    message("Finished.");
}


/*
 *  calc_primes ---
 *
 *      Following macro returns a list of prime numbers. Called by
 *      performance macro as well as by sieve above.
 */
list
calc_primes(int nprimes)
{
    int i, j;
    list bucket;

    message("Clearing numbers...");
    for (i = 0; i < nprimes; i++) {
        bucket[i] = TRUE;
    }

    message("Casting out 2's....");
    for (j = 2; j < nprimes; j += 2) {
        bucket[j] = FALSE;
    }

    for (i = 3; i < nprimes; i += 2) {
        /*
         *  %p means only print maximum of one message per second.
         */
        message("%pCasting out %d's....", i);
        for (j = 2 * i; j < nprimes; j += i)
            bucket[j] = FALSE;
    }
    return bucket;
}
