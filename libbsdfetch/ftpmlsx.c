/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ftpmlsx.c,v 1.6 2024/07/13 09:34:10 cvsuser Exp $
 * FTP MLSx command reply parser.
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
 
#define _BSD_SOURCE     /* implied origin */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if defined(__linux__)  /* _BSD_SOURCE has been deprecated, glibc >= 2.2 */
#if defined(_BSD_SOURCE)
#define _DEFAULT_SOURCE
#endif
#endif

#include <sys/stat.h>
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

static int		mlsx_subopt(char **optionp, const char * const *tokens, char **valuep);
static time_t		mlsx_time(const char *timestamp);
static int		mlsx_mode(const char *umode);

/*
 *  fetch_mlsx_entry ---
 *      Parse a FTP MLSD/MLST command reply.
 */
int
fetch_mlsx_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *name)
{
	static const char * const mlsxfacts[] = {

#define MLSD_OPTTYPE		0
#define MLSD_OPTSIZE		1
#define MLSD_OPTMODIFY		3
#define MLSD_OPTCREATE		4
#define MLSD_OPTUNIQUE		5
#define MLSD_OPTPERM		6
#define MLSD_OPTUNIXMODE	10

		/*0 */ "type",			/* entry type. */
		/*1 */ "size",			/* size in octets. */
		/*2 */ "sizd",			/* directory size in octets. */
		/*3 */ "modify",		/* last modification time. */
		/*4 */ "create",		/* creation time. */
		/*5 */ "unique",		/* unique id of file/directory. */
		/*6 */ "perm",			/* file permissions, whether read, write, execute is allowed for the login id. */
		/*7 */ "lang",			/* language of the file name per IANA registry. */
		/*8 */ "media-type",		/* MIME media-type of file contents per IANA registry. */
		/*9 */ "charset",		/* character set per IANA registry (if not UTF-8). */
		/*10*/ "unix.mode",
		/*11*/ "unix.owner",
		/*12*/ "unix.group",
		/*13*/ "unix.chgd",
		/*14*/ "unix.uid",
		/*15*/ "unix.gid",
		NULL
		};
	char *option = (char *)name, *value = NULL;
	struct url_stat sb = {0};
	int umode = -1, mode = 0;

	while (option && *option) {
		const int fact = mlsx_subopt(&option, mlsxfacts, &value);

	     /* ED_TRACE2((" fact[%2d] %s=<%s>\n", fact, (fact >= 0 ? mlsxfacts[fact] : "n/a"), (value ? value : ""))) */
		switch (fact) {
		case MLSD_OPTTYPE:
			if (value) {
				if (0 == str_icmp(value, "file")) {
					mode = S_IFREG;
				} else if (0 == str_icmp(value, "dir") ||
						0 == str_icmp(value, "cdir") ||
						0 == str_icmp(value, "pdir")) {
					mode = S_IFDIR;
#if defined(S_IFLNK) && (S_IFLNK)
				} else if (0 == str_icmp(value, "OS.unix=symlink") ||
						0 == str_icmp(value, "OS.unix=slink")) {
					mode = S_IFLNK;
#endif
				}
			}
			break;
		case MLSD_OPTSIZE:
			sb.size = strtoul(value, NULL, 10);
			break;
		case MLSD_OPTMODIFY:
			sb.mtime = mlsx_time(value);
			break;
		case MLSD_OPTCREATE:
			sb.atime = mlsx_time(value);
			break;
		case MLSD_OPTPERM:
			while (*value) {
			case 'r': mode |= S_IRUSR; break;
			case 'w': mode |= S_IWUSR; break;
			}
			break;
		case MLSD_OPTUNIXMODE:
			umode = mlsx_mode(value);
			break;
		default:
			if (!*option) {		/* EOS */
				if (value && (umode > 0 || mode)) {
					sb.mode = (umode > 0 ? umode : mode);
					if (us) *us = sb;
					if (ue && base) {
						return fetch_add_entry2(ue, base, value, &sb, 0);
					} else if (base) {
						base->doc = strdup(name);
					}
					return 0;
				}
			}
			break;
		}
	}
	return 0;
}


static int
mlsx_subopt(char **optionp, const char * const *tokens, char **valuep)
{
#define OPTDELIMIT		';'

	register int cnt;
	register char *p;
	char *subval;

	subval = *valuep = NULL;
	if (! optionp || ! *optionp) {
		return -1;
	}

	for (p = *optionp; *p && (OPTDELIMIT == *p || ' ' == *p || '\t' == *p); ++p) {
		/*continue*/;
	}

	if (! *p) {
		*optionp = p;
		return -1;			/* EOF */
	}

	/* save the start of the token, and skip the rest of the token. */
	for (subval = p; *++p && *p != OPTDELIMIT && *p != '=' && *p != ' ' && *p != '\t';) {
		/*continue*/;
	}

	if (*p) {
		if ('=' == *p) {		/* token=value */
			*p = '\0';
			for (*valuep = ++p; *p && *p != OPTDELIMIT && *p != ' ' && *p != '\t'; ++p)
				/*continue*/;
			if (*p) *p++ = '\0';
		} else {
			*p++ = '\0';
		}
		
		for (; *p && (OPTDELIMIT == *p || ' ' == *p || '\t' == *p ); ++p) {
			/*continue*/;		/* skip white-space/delimiter after this token. */
		}
	}

	/* match */
	*optionp = p;				/* set optionp for next round */

	for (cnt = 0; *tokens; ++tokens, ++cnt) {
		if (0 == str_icmp(subval, *tokens)) {
			return cnt;		/* token index, case insensitive token */
		}
	}

	*valuep = subval;			/* value, represents non-matched token */
	return -2;				/* non-matching */
}


static time_t
mlsx_time(const char *timestamp)
{
	struct tm tm = {0};
	unsigned year = 0;
	time_t t;

	if (sscanf(timestamp, "%04u%02u%02u%02u%02u%02u",
			&year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6) {
		return -1;
	}
	--tm.tm_mon;
	tm.tm_year = year - 1900;
	tm.tm_isdst = -1;
	t = timegm(&tm);
	return (t == (time_t)-1 ? time(NULL) : t);
}


static int
mlsx_mode(const char *umode)
{
	int mode = 0;

	if (strlen(umode) < 10) return -1;

	/* type */
	switch (umode[0]) {
	case 'd': /* 'd' - directory. */
		mode = S_IFDIR;
		break;
	case 'l': /* 'l - link. */
#if defined(S_IFLNK)
		mode = S_IFLNK;
#else
		mode = S_IFREG;
#endif
		break;
	case '-': /* '-' - normal. */
		mode = S_IFREG;
		break;
	default:
		/* 'c' - character-device. */
		/* 'b' - block-device. */
		/* 'p' - fifo/pipe. */
		/* 's' - sockets. */
		/* 'n' - name. */
		/* 'D' - door. */
		return -1;
	}

	/* user (rwx) */
	switch(umode[1]) {
	case 'r': case 'R':
		mode |= S_IRUSR;
		break;
	case '-':
		break;
	default:
		return -1;
	}

	switch(umode[2]) {
	case 'w': case 'W':
		mode |= S_IWUSR;
		break;
	case '-':
		break;
	default:
		return -1;
	}

	switch(umode[3]) {
	case 'x': case 'X':
		mode |= S_IXUSR;
		break;
	case 's':
		mode |= S_IXUSR|S_ISUID;
		break;
	case 'S':
		mode |= S_ISUID;
		break;
	case '-':
		break;
	default:
		return -1;
	}

	/* group (rwx) */
	switch(umode[4]) {
	case 'r': case 'R':
		mode |= S_IRGRP;
		break;
	case '-':
		break;
	default:
		return -1;
	}

	switch(umode[5]) {
	case 'w': case 'W':
		mode |= S_IWGRP;
		break;
	case '-':
		break;
	default:
		return -1;
	}

	switch(umode[6]) {
	case 'x': case 'X':
		mode |= S_IXGRP;
		break;
	case 's':
		mode |= S_IXGRP|S_ISGID;
		break;
	case 'S':
		mode |= S_ISGID;
		break;
	case '-':
		break;
	default:
		return -1;
	}

	/* others (rwx) */
	switch(umode[7]) {
	case 'r': case 'R':
		mode |= S_IROTH;
		break;
	case '-':
		break;
	default:
		return -1;
	}

	switch(umode[8]) {
	case 'w': case 'W':
		mode |= S_IWOTH;
		break;
	case '-':
		break;
	default:
		return -1;
	}

#if !defined(S_ISVTX)
#define S_ISVTX		0
#endif

	switch(umode[9]) {
	case 'x': case 'X':
		mode |= S_IXOTH;
		break;
	case 't':
		mode |= S_IXOTH|S_ISVTX;
		break;
	case 'T':
		mode |= S_ISVTX;
		break;
	case '-':
		break;
	default:
		return -1;
	}

	return mode;
}
/*end*/
