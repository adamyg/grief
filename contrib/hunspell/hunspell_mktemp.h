/*
 *  hunspell mkdtemp/mkstemp implementation
 */
 
#include <stdio.h>

#if defined(__cplusplus)
extern "C" {
#endif

FILE *hunspell_fdopen(int fd, const char *mode);
int hunspell_mkstemp(char *templ);
char *hunspell_mkdtemp(char *templ);

#if defined(__MINGW32__)
#define fdopen(__a,__b)     hunspell_fdopen(__a,__b)
#else
#if !defined(WIN32)
#define WIN32
#endif
#define fdopen(__a,__b)     _fdopen(__a,__b)
#endif

#if !defined(__WATCOMC__)
#define mkstemp(__templ)    hunspell_mkstemp(__templ)
#endif

#define mkdtemp(__templ)    hunspell_mkdtemp(__templ)

#if defined(__cplusplus)
}
#endif

/*end*/


