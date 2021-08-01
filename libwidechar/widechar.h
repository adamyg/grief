/*
 *  Compat wide character.
 *
 *  The wchar_t type is an implementation-defined wide character type. For example under the
 *  Microsoft compiler, it represents a 16-bit wide character used to store Unicode encoded as
 *  UTF-16LE, the native character type on Windows operating systems.  Characters that cannot b
 *  represented in just one wide character are represented in a Unicode pair by using the
 *  Unicode surrogate pair feature.
 *
 *  Whereas other implementations including this use UTF-32 need not manage wide-character strings
 *  without special handling for surrogates.
 *
 *  Furthermore the behaviour of several standard functions are influenced by the current locale.
 *
 *  These functions provide uniform behaviour independent of run-time environment and the locale.
 */

#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

    //#define MB_LEN_MAX 6
typedef int WChar_t;

#ifdef __cplusplus
extern "C" {
#endif

WChar_t *   Wcscat(WChar_t *s1, const WChar_t *s2);
WChar_t *   Wcschr(const WChar_t *s, WChar_t c);
int         Wcscmp(const WChar_t *s1, const WChar_t *s2);
WChar_t *   Wcscpy(WChar_t *s1, const WChar_t *s2);
size_t      Wcslen(const WChar_t *s);
WChar_t *   Wcsncat(WChar_t *s1, const WChar_t *s2, size_t n);
int         Wcsncmp(const WChar_t *s1, const WChar_t *s2, size_t n);
WChar_t *   Wcsncpy(WChar_t *s1, const WChar_t *s2, size_t n);
WChar_t *   Wcspbrk(const WChar_t *s, const WChar_t *set);
WChar_t *   Wcsrchr(const WChar_t *s, WChar_t c);
WChar_t *   Wcstok(WChar_t * s, const WChar_t * delim, WChar_t ** last);

int         Wcscasecmp(const WChar_t *s1, const WChar_t *s2);
int         Wcsncasecmp(const WChar_t *s1, const WChar_t *s2, size_t n);

double      Wcstod(const WChar_t *nptr, WChar_t **endptr);

WChar_t *   Wmemchr(const WChar_t *s, WChar_t c, size_t n);
int         Wmemcmp(const WChar_t *s1, const WChar_t *s2, size_t n);
WChar_t *   Wmemcpy(WChar_t *d, const WChar_t *s, size_t n);
WChar_t *   Wmemmove(WChar_t *d, const WChar_t *s, size_t n);
WChar_t *   Wmemset(WChar_t *s, WChar_t c, size_t n);

size_t      Wcslcat(WChar_t *dst, const WChar_t *src, size_t dsize);
size_t      Wcslcpy(WChar_t *dst, const WChar_t *src, size_t dsize);
size_t      Wcsspn(const WChar_t *s, const WChar_t *set);
int         Wcswidth(const WChar_t *s, size_t n);

int         Wcfromutf8(const char *str, WChar_t *wc);
int         Wctoutf8(WChar_t wch, char *buf, int buflen);

int         Wcsfromutf8(const char *str, WChar_t *buf, int buflen);
int         Wcstoutf8(const WChar_t *str, char *buf, int buflen);

int         Wcwidth(WChar_t ucs);
int         Wcswidth(const WChar_t *s, size_t n);

int         Wvsnprintf(WChar_t *str, size_t size, const char *format, va_list args);
int         Wsnprintf(WChar_t *str, size_t size, const char *format, ...);
int         Wasprintf(WChar_t **ret, const char *format, ...);

int         utf8_width(const void *src, const void *cpend);
int         utf8_nwidth(const void *src, size_t len);
int         utf8_swidth(const void *src);

int         ucs_width_set(const char *label);
const char *ucs_width_version(void);
int         ucs_width(int32_t ucs);

#ifdef __cplusplus
}
#endif

/*end*/
