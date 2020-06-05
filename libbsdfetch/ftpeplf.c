/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ftpeplf.c,v 1.5 2020/06/05 23:45:01 cvsuser Exp $
 * FTP EPLF command reply parser.
 *
 *
 * This file is part of the GRIEF Editor.
 *
 * The GRIEF Editor is free software: you can redistribute it
 * and/or modify it under the terms of the GRIEF Editor License.
 *
 * The GRIEF Editor is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * License for more details.
 * ==end==
 */
 
#define _BSD_SOURCE     /* implied orgin */
 
#if HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(__linux__)  /* _BSD_SOURCE has been deprecated, glibc >= 2.2 */
#if defined(_BSD_SOURCE)
#define _DEFAULT_SOURCE
#endif
#endif

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#if defined(HAVE_INTTYPES_H) || defined(NETBSD)
#include <inttypes.h>
#endif
#include <stdarg.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <libstr.h>
#include "fetch.h"
#include "common.h"

#if defined(LOCAL_MAIN) && (0)
#define DELPF(__x)	printf __x;
#else
#define DELPF(__x)
#endif

/*
 *  fetch_eplf_entry ---
 *      Parse a FTP EPLF command reply; an EPLF response to LIST is a series of lines,
 *      each line specifying one file.
 *
 *      Each line contains
 *
 *          a plus sign (\053);
 *          a series of facts about the file;
 *          a tab (\011);
 *          an abbreviated pathname; and \015\012.
 *
 *  Example:
 *      "+m838255902,/,\treadme.txt"
 *      "+m843956783,r,s510376,\tarchive.tgz"
 *
 *  Reference:
 *      http://cr.yp.to/ftp/list/eplf.html
 */
int
fetch_eplf_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *name)
{
#if defined(HAVE_STRTOULL)
	typedef unsigned long long unsigned_long;
	#define STRTOUL strtoull
#else
	typedef unsigned long unsigned_long;
	#define STRTOUL strtoul
#endif
	struct url_stat sb = {0};
	int ch;

	while (' ' == name[0]) {
		++name;
	}

	if ('+' != *name++) {
		return -1;
	}

	while (0 != (ch = *name++)) {
		const char *sep = strchr(name, ',');

		switch(ch) {
		case 'r':	/* RETR may succeed */
			sb.mode |= S_IFREG;
			break;
		case '/':	/* CWD may succeed */
			sb.mode |= S_IFDIR;
			break;
		case 'u':	/* chmod */
			if ('p' == *name) {
				unsigned_long mode;
				char *endp = NULL;

				if ((mode = STRTOUL(name, &endp, 8)) > 0 && endp == sep) {
					sb.mode |= (unsigned)mode;
				}
			}
			break;

		case 's': {	/* size */
				unsigned_long size;
				char *endp = NULL;

				if ((size = STRTOUL(name, &endp, 10)) > 0 && endp == sep) {
					sb.size = (off_t)size;
				}
			}
			break;

		case 'm': {	/* mtime */
				unsigned_long mtime;
				char *endp = NULL;

				if ((mtime = STRTOUL(name, &endp, 10)) > 0 && endp == sep) {
					sb.mtime = (time_t)mtime;
				}
			}
			break;

		case 'i':	/* file identifier */
			break;

		case '\t':	/* trailing file-name */
			if (*name && NULL == sep) {
				if (sb.mode) {
					sb.atime = 0;
					if (us) *us = sb;
					if (ue && base) {
						return fetch_add_entry2(ue, base, name, &sb, 0);
					} else if (base) {
						base->doc = strdup(name);
					}
					return 0;
				}
			}
			break;
		}
		if (NULL == sep) break;
		name = sep + 1;
	}
	return -1;
}
/*end*/
