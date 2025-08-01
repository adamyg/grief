#include <edidentifier.h>
__CIDENT_RCSID(gr_w32_strptime_c,"$Id: w32_strptime.c,v 1.8 2025/06/28 11:07:20 cvsuser Exp $")

/* -*- mode: c; indent-width: 4; -*- */
/*
 * Copyright (c) 1997, 1998, 2005, 2008 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code was contributed to The NetBSD Foundation by Klaus Klein.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *  $OpenBSD: strptime.c,v 1.16 2014/02/13 23:16:03 millert Exp $
 *  $NetBSD: strptime.c,v 1.12 1998/01/20 21:39:40 mycroft Exp $
 * ==native-notice==
 */

#include "win32_internal.h"
#include "tzfile.h"

#include <sys/utypes.h>
#include <unistd.h>

#include <ctype.h>
#include <locale.h>
#include <string.h>
#include <time.h>

/*
 * We do not implement alternate representations. However, we always
 * check whether a given modifier is allowed for a certain conversion.
 */
#define _ALT_E			0x01
#define _ALT_O			0x02
#define _LEGAL_ALT(x)		{ if (alt_format & ~(x)) return (0); }

/*
 * We keep track of some of the fields we set in order to compute missing ones.
 */
#define FIELD_TM_MON		(1 << 0)
#define FIELD_TM_MDAY		(1 << 1)
#define FIELD_TM_WDAY		(1 << 2)
#define FIELD_TM_YDAY		(1 << 3)
#define FIELD_TM_YEAR		(1 << 4)

static char gmt[] =		{ "GMT" };
#ifdef TM_ZONE
static char utc[] =		{ "UTC" };
#endif

/* RFC-822/RFC-2822 */
static const char * const nast[5] = {
	"EST",    "CST",    "MST",    "PST",    "\0\0\0"
};
static const char * const nadt[5] = {
	"EDT",    "CDT",    "MDT",    "PDT",    "\0\0\0"
};

static const int mon_lengths[2][MONSPERYEAR] = {
	{ 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
	{ 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

static	int _conv_num(const unsigned char **, int *, int, int);
static	int leaps_thru_end_of(const int y);
static	char *_strptime(const char *, const char *, struct tm *, int);
static	const unsigned char *_find_string(const unsigned char *, int *, const char * const *, const char * const *, int);

/*
//  NAME
//
//      strptime - date and time conversion
//
//  SYNOPSIS
//
//      #include <time.h>
//
//      char *strptime(const char *restrict buf, const char *restrict format,
//                  struct tm *restrict tm); [Option End]
//
//  DESCRIPTION
//
//      The strptime() function shall convert the character string pointed to by buf to values
//      which are stored in the tm structure pointed to by tm, using the format specified by
//      format.
//
//      The format is composed of zero or more directives. Each directive is composed of one of
//      the following: one or more white-space characters (as specified by isspace()); an
//      ordinary character (neither '%' nor a white-space character); or a conversion
//      specification. Each conversion specification is composed of a '%' character followed by a
//      conversion character which specifies the replacement required. The application shall
//      ensure that there is white-space or other non-alphanumeric characters between any two
//      conversion specifications. The following conversion specifications are supported:
//
//      %a  The day of the week, using the locale's weekday names; either the abbreviated or full
//          name may be specified.
//
//      %A  Equivalent to %a.
//
//      %b  The month, using the locale's month names; either the abbreviated or full name may be
//          specified.
//
//      %B  Equivalent to %b.
//
//      %c  Replaced by the locale's appropriate date and time representation.
//
//      %C  The century number [00,99]; leading zeros are permitted but not required.
//
//      %d  The day of the month [01,31]; leading zeros are permitted but not required.
//
//      %D  The date as %m / %d / %y.
//
//      %e  Equivalent to %d.
//
//      %h  Equivalent to %b.
//
//      %H  The hour (24-hour clock) [00,23]; leading zeros are permitted but not required.
//
//      %I  The hour (12-hour clock) [01,12]; leading zeros are permitted but not required.
//
//      %j  The day number of the year [001,366]; leading zeros are permitted but not required.
//
//      %m  The month number [01,12]; leading zeros are permitted but not required.
//
//      %M  The minute [00,59]; leading zeros are permitted but not required.
//
//      %n  Any white space.
//
//      %p  The locale's equivalent of a.m or p.m.
//
//      %r  12-hour clock time using the AM/PM notation if t_fmt_ampm is not an empty string in the
//          LC_TIME portion of the current locale; in the POSIX locale, this shall be equivalent
//          to %I : %M : %S %p.
//
//      %R  The time as %H : %M.
//
//      %S  The seconds [00,60]; leading zeros are permitted but not required.
//
//      %t  Any white space.
//
//      %T  The time as %H : %M : %S.
//
//      %U  The week number of the year (Sunday as the first day of the week) as a decimal number
//              [00, 53]; leading zeros are permitted but not required.
//
//      %w  The weekday as a decimal number [0, 6], with 0 representing Sunday; leading zeros are
//          permitted but not required.
//
//      %W  The week number of the year (Monday as the first day of the week) as a decimal
//              number [00, 53]; leading zeros are permitted but not required.
//
//      %x  The date, using the locale's date format.
//
//      %X  The time, using the locale's time format.
//
//      %y  The year within century. When a century is not otherwise specified, values in the range
//          [69, 99] shall refer to years 1969 to 1999 inclusive, and values in the range [00, 68] shall
//          refer to years 2000 to 2068 inclusive; leading zeros shall be permitted but shall not be
//          required.
//
//          Note: It is expected that in a future version of IEEE Std 1003.1-2001 the default
//          century inferred from a 2-digit year will change. (This would apply to all commands
//          accepting a 2-digit year as input.)
//
//      %Y  The year, including the century (for example, 1988).
//
//      %%  Replaced by %.
//
//      Modified Conversion Specifiers
//
//      Some conversion specifiers can be modified by the E and O modifier characters to indicate
//      that an alternative format or specification should be used rather than the one normally used
//      by the unmodified conversion specifier. If the alternative format or specification does not
//      exist in the current locale, the behavior shall be as if the unmodified conversion
//      specification were used.
//
//      %Ec The locale's alternative appropriate date and time representation.
//
//      %EC The name of the base year (period) in the locale's alternative representation.
//
//      %Ex The locale's alternative date representation.
//
//      %EX The locale's alternative time representation.
//
//      %Ey The offset from %EC (year only) in the locale's alternative representation.
//
//      %EY The full alternative year representation.
//
//      %Od The day of the month using the locale's alternative numeric symbols; leading zeros
//          are permitted but not required.
//
//      %Oe Equivalent to %Od.
//
//      %OH The hour (24-hour clock) using the locale's alternative numeric symbols.
//
//      %OI The hour (12-hour clock) using the locale's alternative numeric symbols.
//
//      %Om The month using the locale's alternative numeric symbols.
//
//      %OM The minutes using the locale's alternative numeric symbols.
//
//      %OS The seconds using the locale's alternative numeric symbols.
//
//      %OU The week number of the year (Sunday as the first day of the week) using the locale's
//          alternative numeric symbols.
//
//      %Ow The number of the weekday (Sunday=0) using the locale's alternative numeric symbols.
//
//      %OW The week number of the year (Monday as the first day of the week) using the locale's
//          alternative numeric symbols.
//
//      %Oy The year (offset from %C ) using the locale's alternative numeric symbols.
//
//      A conversion specification composed of white-space characters is executed by scanning input
//      up to the first character that is not white-space (which remains unscanned), or until no more
//      characters can be scanned.
//
//      A conversion specification that is an ordinary character is executed by scanning the next
//      character from the buffer. If the character scanned from the buffer differs from the one
//      comprising the directive, the directive fails, and the differing and subsequent characters
//      remain unscanned.
//
//      A series of conversion specifications composed of %n, %t, white-space characters, or any
//      combination is executed by scanning up to the first character that is not white space (which
//      remains unscanned), or until no more characters can be scanned.
//
//      Any other conversion specification is executed by scanning characters until a character
//      matching the next directive is scanned, or until no more characters can be scanned. These
//      characters, except the one matching the next directive, are then compared to the locale
//      values associated with the conversion specifier. If a match is found, values for the
//      appropriate tm structure members are set to values corresponding to the locale information.
//      Case is ignored when matching items in buf such as month or weekday names. If no match is
//      found, strptime() fails and no more characters are scanned.
//
//  RETURN VALUE
//
//      Upon successful completion, strptime() shall return a pointer to the character following the
//      last character parsed. Otherwise, a null pointer shall be returned.
//
//  ERRORS
//
//      No errors are defined.
*/

LIBW32_API char *
w32_strptime(const char *buf, const char *fmt, struct tm *tm)
{
	return(_strptime(buf, fmt, tm, 1));
}

static char *
_strptime(const char *buf, const char *fmt, struct tm *tm, int initialize)
{
	unsigned char c;
	const unsigned char *bp, *ep;
//	size_t len;
	int alt_format, i, offs;
	int neg = 0;
	static int century, relyear, fields;

#if (TODO)
#define _ctloc(x)		(_CurrentTimeLocale->x)
	_locale_t _CurrentTimeLocale = _get_current_locale();
#endif

	if (initialize) {
		century = TM_YEAR_BASE;
		relyear = -1;
		fields = 0;
	}

	bp = (unsigned char *)buf;
	while ((c = *fmt) != '\0') {
		/* Clear `alternate' modifier prior to new conversion. */
		alt_format = 0;

		/* Eat up white-space. */
		if (isspace(c)) {
			while (isspace(*bp))
				bp++;

			fmt++;
			continue;
		}
				
		if ((c = *fmt++) != '%')
			goto literal;


again:		switch (c = *fmt++) {
		case '%':	/* "%%" is converted to "%". */
literal:
		if (c != *bp++)
			return (NULL);

		break;

		/*
		 * "Alternative" modifiers. Just set the appropriate flag
		 * and start over again.
		 */
		case 'E':	/* "%E?" alternative conversion modifier. */
			_LEGAL_ALT(0);
			alt_format |= _ALT_E;
			goto again;

		case 'O':	/* "%O?" alternative conversion modifier. */
			_LEGAL_ALT(0);
			alt_format |= _ALT_O;
			goto again;
			
		/*
		 * "Complex" conversion rules, implemented through recursion.
		 */
//		case 'c':	/* Date and time, using the locale's format. */
//			_LEGAL_ALT(_ALT_E);
//			if (!(bp = _strptime(bp, _ctloc(d_t_fmt), tm, 0)))
//				return (NULL);
//			break;

		case 'D':	/* The date as "%m/%d/%y". */
			_LEGAL_ALT(0);
			if (!(bp = (unsigned char *)_strptime((const char *)bp, "%m/%d/%y", tm, 0)))
				return (NULL);
			break;

		case 'F':	/* The date as "%Y-%m-%d". */
			_LEGAL_ALT(0);
			if (!(bp = (unsigned char *)_strptime((const char *)bp, "%Y-%m-%d", tm, 0)))
				return (NULL);
			continue;

		case 'R':	/* The time as "%H:%M". */
			_LEGAL_ALT(0);
			if (!(bp = (unsigned char *)_strptime((const char *)bp, "%H:%M", tm, 0)))
				return (NULL);
			break;

		case 'r':	/* The time as "%I:%M:%S %p". */
			_LEGAL_ALT(0);
			if (!(bp = (unsigned char *)_strptime((const char *)bp, "%I:%M:%S %p", tm, 0)))
				return (NULL);
			break;

		case 'T':	/* The time as "%H:%M:%S". */
			_LEGAL_ALT(0);
			if (!(bp = (unsigned char *)_strptime((const char *)bp, "%H:%M:%S", tm, 0)))
				return (NULL);
			break;

//		case 'X':	/* The time, using the locale's format. */
//			_LEGAL_ALT(_ALT_E);
//			if (!(bp = (unsigned char *)_strptime((const char *)bp, _ctloc(t_fmt), tm, 0)))
//				return (NULL);
//			break;

//		case 'x':	/* The date, using the locale's format. */
//			_LEGAL_ALT(_ALT_E);
//			if (!(bp = (unsigned char *)_strptime((const char *)bp, _ctloc(d_fmt), tm, 0)))
//				return (NULL);
//			break;

		/*
		 * "Elementary" conversion rules.
		 */
//		case 'A':	/* The day of week, using the locale's form. */
//		case 'a':
//			_LEGAL_ALT(0);
//			for (i = 0; i < 7; i++) {
//				/* Full name. */
//				len = strlen(_ctloc(day[i]));
//				if (strncasecmp(_ctloc(day[i]), bp, len) == 0)
//					break;
//
//				/* Abbreviated name. */
//				len = strlen(_ctloc(abday[i]));
//				if (strncasecmp(_ctloc(abday[i]), bp, len) == 0)
//					break;
//			}
//
//			/* Nothing matched. */
//			if (i == 7)
//				return (NULL);
//
//			tm->tm_wday = i;
//			bp += len;
//			fields |= FIELD_TM_WDAY;
//			break;

//		case 'B':	/* The month, using the locale's form. */
//		case 'b':
//		case 'h':
//			_LEGAL_ALT(0);
//			for (i = 0; i < 12; i++) {
//				/* Full name. */
//				len = strlen(_ctloc(mon[i]));
//				if (strncasecmp(_ctloc(mon[i]), bp, len) == 0)
//					break;
//
//				/* Abbreviated name. */
//				len = strlen(_ctloc(abmon[i]));
//				if (strncasecmp(_ctloc(abmon[i]), bp, len) == 0)
//					break;
//			}
//
//			/* Nothing matched. */
//			if (i == 12)
//				return (NULL);
//
//			tm->tm_mon = i;
//			bp += len;
//			fields |= FIELD_TM_MON;
//			break;

		case 'C':	/* The century number. */
			_LEGAL_ALT(_ALT_E);
			if (!(_conv_num(&bp, &i, 0, 99)))
				return (NULL);

			century = i * 100;
			break;

		case 'd':	/* The day of month. */
		case 'e':
			_LEGAL_ALT(_ALT_O);
			if (!(_conv_num(&bp, &tm->tm_mday, 1, 31)))
				return (NULL);
			fields |= FIELD_TM_MDAY;
			break;

		case 'k':	/* The hour (24-hour clock representation). */
			_LEGAL_ALT(0);
			/* FALLTHROUGH */
		case 'H':
			_LEGAL_ALT(_ALT_O);
			if (!(_conv_num(&bp, &tm->tm_hour, 0, 23)))
				return (NULL);
			break;

		case 'l':	/* The hour (12-hour clock representation). */
			_LEGAL_ALT(0);
			/* FALLTHROUGH */
		case 'I':
			_LEGAL_ALT(_ALT_O);
			if (!(_conv_num(&bp, &tm->tm_hour, 1, 12)))
				return (NULL);
			break;

		case 'j':	/* The day of year. */
			_LEGAL_ALT(0);
			if (!(_conv_num(&bp, &tm->tm_yday, 1, 366)))
				return (NULL);
			tm->tm_yday--;
			fields |= FIELD_TM_YDAY;
			break;

		case 'M':	/* The minute. */
			_LEGAL_ALT(_ALT_O);
			if (!(_conv_num(&bp, &tm->tm_min, 0, 59)))
				return (NULL);
			break;

		case 'm':	/* The month. */
			_LEGAL_ALT(_ALT_O);
			if (!(_conv_num(&bp, &tm->tm_mon, 1, 12)))
				return (NULL);
			tm->tm_mon--;
			fields |= FIELD_TM_MON;
			break;

//		case 'p':	/* The locale's equivalent of AM/PM. */
//			_LEGAL_ALT(0);
//			/* AM? */
//			len = strlen(_ctloc(am_pm[0]));
//			if (strncasecmp(_ctloc(am_pm[0]), bp, len) == 0) {
//				if (tm->tm_hour > 12)	/* i.e., 13:00 AM ?! */
//					return (NULL);
//				else if (tm->tm_hour == 12)
//					tm->tm_hour = 0;
//
//				bp += len;
//				break;
//			}
//			/* PM? */
//			len = strlen(_ctloc(am_pm[1]));
//			if (strncasecmp(_ctloc(am_pm[1]), bp, len) == 0) {
//				if (tm->tm_hour > 12)	/* i.e., 13:00 PM ?! */
//					return (NULL);
//				else if (tm->tm_hour < 12)
//					tm->tm_hour += 12;
//
//				bp += len;
//				break;
//			}
//
//			/* Nothing matched. */
//			return (NULL);

		case 'S':	/* The seconds. */
			_LEGAL_ALT(_ALT_O);
			if (!(_conv_num(&bp, &tm->tm_sec, 0, 61)))
				return (NULL);
			break;

		case 'U':	/* The week of year, beginning on sunday. */
		case 'W':	/* The week of year, beginning on monday. */
			_LEGAL_ALT(_ALT_O);
			/*
			 * XXX This is bogus, as we can not assume any valid
			 * information present in the tm structure at this
			 * point to calculate a real value, so just check the
			 * range for now.
			 */
			 if (!(_conv_num(&bp, &i, 0, 53)))
				return (NULL);
			 break;

		case 'w':	/* The day of week, beginning on sunday. */
			_LEGAL_ALT(_ALT_O);
			if (!(_conv_num(&bp, &tm->tm_wday, 0, 6)))
				return (NULL);
			fields |= FIELD_TM_WDAY;
			break;

		case 'u':	/* The day of week, monday = 1. */
			_LEGAL_ALT(_ALT_O);
			if (!(_conv_num(&bp, &i, 1, 7)))
				return (NULL);
			tm->tm_wday = i % 7;
			fields |= FIELD_TM_WDAY;
			continue;

		case 'g':	/* The year corresponding to the ISO week
				 * number but without the century.
				 */
			if (!(_conv_num(&bp, &i, 0, 99)))
				return (NULL);				
			continue;

		case 'G':	/* The year corresponding to the ISO week
				 * number with century.
				 */
			do
				bp++;
			while (isdigit(*bp));
			continue;

		case 'V':	/* The ISO 8601:1988 week number as decimal */
			if (!(_conv_num(&bp, &i, 0, 53)))
				return (NULL);
			continue;

		case 'Y':	/* The year. */
			_LEGAL_ALT(_ALT_E);
			if (!(_conv_num(&bp, &i, 0, 9999)))
				return (NULL);

			relyear = -1;
			tm->tm_year = i - TM_YEAR_BASE;
			fields |= FIELD_TM_YEAR;
			break;

		case 'y':	/* The year within the century (2 digits). */
			_LEGAL_ALT(_ALT_E | _ALT_O);
			if (!(_conv_num(&bp, &relyear, 0, 99)))
				return (NULL);
			break;

		case 'Z':
			WIN32_TZSET();
			if (strncmp((const char *)bp, gmt, 3) == 0) {
				tm->tm_isdst = 0;
#ifdef TM_GMTOFF
				tm->TM_GMTOFF = 0;
#endif
#ifdef TM_ZONE
				tm->TM_ZONE = gmt;
#endif
				bp += 3;
			} else {
				ep = _find_string(bp, &i,
#if defined(_MSC_VER) && (_MSC_VER >= 1900)
						 (const char * const *)_tzname,
#else
						 (const char * const *)tzname,
#endif
						  NULL, 2);
				if (ep != NULL) {
					tm->tm_isdst = i;
#ifdef TM_GMTOFF
					tm->TM_GMTOFF = -(timezone);
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = tzname[i];
#endif
				}
				bp = ep;
			}
			continue;

		case 'z':
			/*
			 * We recognize all ISO 8601 formats:
			 * Z	= Zulu time/UTC
			 * [+-]hhmm
			 * [+-]hh:mm
			 * [+-]hh
			 * We recognize all RFC-822/RFC-2822 formats:
			 * UT|GMT
			 *          North American : UTC offsets
			 * E[DS]T = Eastern : -4 | -5
			 * C[DS]T = Central : -5 | -6
			 * M[DS]T = Mountain: -6 | -7
			 * P[DS]T = Pacific : -7 | -8
			 *          Military
			 * [A-IL-M] = -1 ... -9 (J not used)
			 * [N-Y]  = +1 ... +12
			 */
			while (isspace(*bp))
				bp++;

			switch (*bp++) {
			case 'G':
				if (*bp++ != 'M')
					return NULL;
				/*FALLTHROUGH*/
			case 'U':
				if (*bp++ != 'T')
					return NULL;
				/*FALLTHROUGH*/
			case 'Z':
				tm->tm_isdst = 0;
#ifdef TM_GMTOFF
				tm->TM_GMTOFF = 0;
#endif
#ifdef TM_ZONE
				tm->TM_ZONE = utc;
#endif
				continue;
			case '+':
				neg = 0;
				break;
			case '-':
				neg = 1;
				break;
			default:
				--bp;
				ep = _find_string(bp, &i, nast, NULL, 4);
				if (ep != NULL) {
#ifdef TM_GMTOFF
					tm->TM_GMTOFF = -5 - i;
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = __UNCONST(nast[i]);
#endif
					bp = ep;
					continue;
				}
				ep = _find_string(bp, &i, nadt, NULL, 4);
				if (ep != NULL) {
					tm->tm_isdst = 1;
#ifdef TM_GMTOFF
					tm->TM_GMTOFF = -4 - i;
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = __UNCONST(nadt[i]);
#endif
					bp = ep;
					continue;
				}

				if ((*bp >= 'A' && *bp <= 'I') ||
				    (*bp >= 'L' && *bp <= 'Y')) {
#ifdef TM_GMTOFF
					/* Argh! No 'J'! */
					if (*bp >= 'A' && *bp <= 'I')
						tm->TM_GMTOFF =
						    ('A' - 1) - (int)*bp;
					else if (*bp >= 'L' && *bp <= 'M')
						tm->TM_GMTOFF = 'A' - (int)*bp;
					else if (*bp >= 'N' && *bp <= 'Y')
						tm->TM_GMTOFF = (int)*bp - 'M';
#endif
#ifdef TM_ZONE
					tm->TM_ZONE = NULL; /* XXX */
#endif
					bp++;
					continue;
				}
				return NULL;
			}
			offs = 0;
			for (i = 0; i < 4; ) {
				if (isdigit(*bp)) {
					offs = offs * 10 + (*bp++ - '0');
					i++;
					continue;
				}
				if (i == 2 && *bp == ':') {
					bp++;
					continue;
				}
				break;
			}
			switch (i) {
			case 2:
				offs *= 100;
				break;
			case 4:
				i = offs % 100;
				if (i >= 60)
					return NULL;
				/* Convert minutes into decimal */
				offs = (offs / 100) * 100 + (i * 50) / 30;
				break;
			default:
				return NULL;
			}
			if (neg)
				offs = -offs;
			tm->tm_isdst = 0;	/* XXX */
#ifdef TM_GMTOFF
			tm->TM_GMTOFF = offs;
#endif
#ifdef TM_ZONE
			tm->TM_ZONE = NULL;	/* XXX */
#endif
			continue;

		/*
		 * Miscellaneous conversions.
		 */
		case 'n':	/* Any kind of white-space. */
		case 't':
			_LEGAL_ALT(0);
			while (isspace(*bp))
				bp++;
			break;


		default:	/* Unknown/unsupported conversion. */
			return (NULL);
		}


	}

	/*
	 * We need to evaluate the two digit year spec (%y)
	 * last as we can get a century spec (%C) at any time.
	 */
	if (relyear != -1) {
		if (century == TM_YEAR_BASE) {
			if (relyear <= 68)
				tm->tm_year = relyear + 2000 - TM_YEAR_BASE;
			else
				tm->tm_year = relyear + 1900 - TM_YEAR_BASE;
		} else {
			tm->tm_year = relyear + century - TM_YEAR_BASE;
		}
		fields |= FIELD_TM_YEAR;
	}

	/* Compute some missing values when possible. */
	if (fields & FIELD_TM_YEAR) {
		const int year = tm->tm_year + TM_YEAR_BASE;
		const int *mon_lens = mon_lengths[isleap(year)];
		if (!(fields & FIELD_TM_YDAY) &&
		    (fields & FIELD_TM_MON) && (fields & FIELD_TM_MDAY)) {
			tm->tm_yday = tm->tm_mday - 1;
			for (i = 0; i < tm->tm_mon; i++)
				tm->tm_yday += mon_lens[i];
			fields |= FIELD_TM_YDAY;
		}
		if (fields & FIELD_TM_YDAY) {
			int days = tm->tm_yday;
			if (!(fields & FIELD_TM_WDAY)) {
				tm->tm_wday = EPOCH_WDAY +
				    ((year - EPOCH_YEAR) % DAYSPERWEEK) *
				    (DAYSPERNYEAR % DAYSPERWEEK) +
				    leaps_thru_end_of(year - 1) -
				    leaps_thru_end_of(EPOCH_YEAR - 1) +
				    tm->tm_yday;
				tm->tm_wday %= DAYSPERWEEK;
				if (tm->tm_wday < 0)
					tm->tm_wday += DAYSPERWEEK;
			}
			if (!(fields & FIELD_TM_MON)) {
				tm->tm_mon = 0;
				while (tm->tm_mon < MONSPERYEAR && days >= mon_lens[tm->tm_mon])
					days -= mon_lens[tm->tm_mon++];
			}
			if (!(fields & FIELD_TM_MDAY))
				tm->tm_mday = days + 1;
		}
	}

	return ((char *)bp);
}


static int
_conv_num(const unsigned char **buf, int *dest, int llim, int ulim)
{
	int result = 0;
	int rulim = ulim;

	if (**buf < '0' || **buf > '9')
		return (0);

	/* we use rulim to break out of the loop when we run out of digits */
	do {
		result *= 10;
		result += *(*buf)++ - '0';
		rulim /= 10;
	} while ((result * 10 <= ulim) && rulim && **buf >= '0' && **buf <= '9');

	if (result < llim || result > ulim)
		return (0);

	*dest = result;
	return (1);
}

static const unsigned char *
_find_string(const unsigned char *bp, int *tgt, const char * const *n1,
		const char * const *n2, int c)
{
	int i;
	unsigned int len;

	/* check full name - then abbreviated ones */
	for (; n1 != NULL; n1 = n2, n2 = NULL) {
		for (i = 0; i < c; i++, n1++) {
			len = (int)strlen(*n1);
			if (strncasecmp(*n1, (const char *)bp, len) == 0) {
				*tgt = i;
				return bp + len;
			}
		}
	}

	/* Nothing matched */
	return NULL;
}

static int
leaps_thru_end_of(const int y)
{
	return (y >= 0) ? (y / 4 - y / 100 + y / 400) :
		-(leaps_thru_end_of(-(y + 1)) + 1);
}
