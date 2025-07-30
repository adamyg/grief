/*
 *  UTF-8 support
 */

#include "widechar.h"
#include "../libchartable/libchartable.h"

#include <string.h>
#include <assert.h>

// Multibyte string width.
int
utf8_width(const void *src, const void *cpend)
{
	int width, result = 0;
	const void *end;
	int32_t wch;

	while (src < cpend) {
		if ((end = charset_utf8_decode_safe(src, cpend, &wch)) > src &&
				(width = ucs_width(wch)) >= 0) {
			result += width;
			src = end;
			continue;
		}
		break;
	}
	return result;
}


// Multibyte string width.
int
utf8_nwidth(const void *src, size_t len)
{
	return utf8_width(src, (const char *)src + len);
}


// Multibyte string width.
int
utf8_swidth(const void *src)
{
	return utf8_width(src, (const char *)src + strlen(src));
}


// Convert multibyte string to wide-character string.
int
Wcsfromutf8(const char *mbstr, WChar_t *buf, int buflen)
{
	int remaining = buflen;
	WChar_t *cursor = buf;
	const char *cend;
	int32_t ch;

	assert(mbstr && buf && buflen > 0);
	for (--remaining /*nul*/; remaining && *mbstr;) {
		if ((cend = charset_utf8_decode(mbstr, mbstr + 6, &ch)) <= mbstr || ch <= 0) {
			break;
		}
		*cursor++ = ch;
		--remaining;
		mbstr = cend;
	}
        assert(cursor < (buf + buflen));
	*cursor = 0;
	return (int)(cursor - buf);
}


// Convert wide-character string to multibyte string.
int
Wcstoutf8(const WChar_t *wstr, char *buf, int buflen)
{
	WChar_t ch;
	int len = 0;

	if (NULL == buf) {
		assert(wstr);
		while (0 != (ch = *wstr++)) {
			len += charset_utf8_length(ch);
		}
	} else {
		int remaining = buflen;
		char *cursor = buf;

		assert(wstr && buflen > 0);
		for (--remaining /*nul*/; remaining && 0 != (ch = *wstr++);) {
			const int t_len = charset_utf8_length(ch);
			if (t_len > remaining) {
				break;
			}
			cursor += charset_utf8_encode(ch, cursor);
			remaining -= t_len;
		}
		assert(cursor < (buf + buflen));
		*cursor = 0;
		len = (int)(cursor - buf);
	}
	return len;
}


// Convert multibyte character to wide-character.
int
Wcfromutf8(const char *mbstr, WChar_t *wc)
{
	const char *cend;
	int32_t ch;

	assert(mbstr && wc);
	if ((cend = charset_utf8_decode(mbstr, mbstr + 6, &ch)) <= mbstr || ch <= 0) {
		return -1;
	}
	*wc = ch;
	return (int)(cend - mbstr);
}


// Convert wide-character character to multibyte string.
int
Wctoutf8(WChar_t wch, char *buf, int buflen)
{
	const int len = charset_utf8_length(wch);
	if (len > buflen) {
		return -1;
	}
	return charset_utf8_encode(wch, buf);
}
