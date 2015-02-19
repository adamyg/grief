#ifndef FMT_SCALED_H_INCLUDED
#define FMT_SCALED_H_INCLUDED

#define FMT_SCALED_STRSIZE      7               /* minus sign, 4 digits, suffix, null byte */

__CBEGIN_DECLS

extern int              scan_scaled(char *scaled, long long *result);
extern int              fmt_scaled(long long number, char *result);

__CEND_DECLS

#endif /*FMT_SCALED_H_INCLUDED*/


