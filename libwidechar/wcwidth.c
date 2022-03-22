/*
 *  wcwidth - determine columns needed for a wide character.
 */

#include "widechar.h"

int
Wcwidth(WChar_t wch)
{
	return ucs_width(wch);
}
