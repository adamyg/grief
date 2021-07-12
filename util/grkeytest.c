/*
 *  console key-test
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(_WIN32)
#include "w32keytest.c"

#else
#include <stdio.h>

int
main(void)
{
    printf("not implemented\n");
    return 3;
}

#endif
