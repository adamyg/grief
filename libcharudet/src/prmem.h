#ifndef prmem_h_included
#define prmem_h_included

#include <stdlib.h>

#if !defined(PR_Malloc)
void *
PR_Malloc(size_t len)
{
    return malloc(len);
}
#endif

#define PR_FREEIF(p) \
    do { if (p) free(p); } while(0)

#endif  /*prmem_h_included*/

