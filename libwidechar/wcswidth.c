/*
 *  wcswidth - determine columns needed for a fixed-size wide-character string.
 */

#include "widechar.h"

int
Wcswidth(const WChar_t *s, size_t n)
{
	int w, q;

	w = 0;
	while (n && *s) {
		q = ucs_width(*s);
		if (q == -1)
			return (-1);
		w += q;
		s++;
		n--;
	}

	return w;
}
