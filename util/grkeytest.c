/*
 *  console key-test
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(_WIN32)

#if !defined(_WIN32_WINNT)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#elif (_WIN32_WINNT < 0x601)
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x601
#endif
#undef WINVER
#define WINVER _WIN32_WINNT
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
