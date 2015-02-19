/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ftplist.c,v 1.3 2014/11/24 23:50:02 ayoung Exp $
 * FTP LIST command reply parser.
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

#if HAVE_CONFIG_H
#include "config.h"
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

#include "fetch.h"
#include "common.h"

#if defined(LOCAL_MAIN) && (0)
#define DLIST(__x)	printf __x;
#define DLIST2(__x)
#else
#define DLIST(__x)
#define DLIST2(__x)
#endif

static int		getmode(const char *umode, int *endp);
static int		getnetware(const char *umode, int mode, int *endp);
static long		getnumeric(const char *buf, int len);
static int		getmonth(const char *buf, int len);
static time_t		mtime(int year, int mon, int day, int hour, int min);

/*
 *  fetch_list_entry ---
 *      Parse a LIST command reply, which may be one of a number of supported
 *      directory formats both current and legacy.
 *
 *          o ELPF
 *          o UNIX/Netware (ls, ls -lF)
 *          o WIN32
 *          o VMS/Multinet (single line only)
 *          o DOS
 */
int
fetch_list_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf)
{
	int ret = 0;

	while (' ' == *buf) ++buf;
	if (0 == *buf || 0 == strncasecmp(buf, "total ", 6)) {
		/* "Total of 11 Files, 10966 Blocks." (VMS) */
		/* "total 14786" (UNIX) */
		return 0;
	}

	switch (*buf) {
	case '+':	/* EPLF */
		return fetch_eplf_entry(ue, base, us, buf);

	case '-':	/* UNIX/NetWare */
	case 'b': case 'c': case 'd':
	case 'l': case 'p': case 's':
	case 'w': case 'D':
		if ((ret = fetch_unix_entry(ue, base, us, buf)) >= 0) {
			return ret;
		}
		break;

	case '0':	/* WIN32 */
	case '1': case '2': case '3':
	case '4': case '5': case '6':
	case '7': case '8': case '9':
		if ((ret = fetch_win32_entry(ue, base, us, buf)) >= 0) {
			return ret;
		}
		break;

	default:
		break;
	}
			/* VMS/Multinet */
	if ((ret = fetch_mnet_entry(ue, base, us, buf)) >= 0) {
		return ret;
	}

			/* DOS */
	if ((ret = fetch_dos_entry(ue, base, us, buf)) >= 0) {
		return ret;
	}

	/* Some useless lines, safely ignored: */
	/*  "DISK$ANONFTP:[ANONYMOUS]" (VMS) */
	/*  "Directory DISK$PCSA:[ANONYM]" (VMS) */

	return 0;
}


/*
 *  fetch_unix_entry ---
 *      Parse a UNIX style directory entry.
 *
 *  Examples:
 *
 *      o UNIX-style listing, without inum and without blocks:
 *
 *       "-rw-r--r--   1 root     other        531 Jan 29 03:26 README"
 *       "dr-xr-xr-x   2 root     other        512 Apr  8  1994 etc"
 *       "dr-xr-xr-x   2 root     512 Apr  8  1994 etc"
 *       "lrwxrwxrwx   1 root     other          7 Jan 25 00:17 bin -> usr/bin"
 *
 *       "dr-xr-xr-x   2 root     other        512 Apr  8  2013 etc2/"
 *       "-r-xr-xr-x   1 root     other          7 Jan 25 12:01 bintool*
 *
 *      o Produced by Microsoft's FTP servers for Windows:
 *
 *       "----------   1 owner    group         1803128 Jul 10 10:18 ls-lR.Z"
 *       "d---------   1 owner    group               0 May  9 19:45 Softlib"
 *
 *      o WFTPD for MSDOS:
 *
 *       "-rwxrwxrwx   1 noone    nogroup      322 Aug 19  1996 message.ftp"
 *
 *      o NetPresenz for the Mac:
 *
 *       "-------r--         326  1391972  1392298 Nov 22  1995 MegaPhone.sit"
 *       "drwxrwxr-x               folder        2 May 10  1996 network"
 *
 *      o NetWare (old/newer):
 *
 *       "d[RWCEMFA] supervisor            512       Jan 16 18:53    login"
 *       "-[RWCEMFA] rhesus             214059       Oct 20 15:27    cx.exe"
 *
 *       "d [R----F--] supervisor            512       Jan 16 18:53    login"
 *       "- [R----F--] rhesus             214059       Oct 20 15:27    cx.exe"
 *
 *  Source:
 *      Modified copy of ftpparse.c
 *      http://cr.yp.to/ftpparse.html, 20001223, D. J. Bernstein, djb@cr.yp.to
 */
int
fetch_unix_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf)
{
	struct url_stat sb = {0};
	int year = -1, mon, day, hour, min, size = 0;
	int i, j, state = 1;
	const char *name = NULL;
	int namelen = 0;

	i = j = 0;
	if ((sb.mode = getmode(buf, &j)) < 0) {
		return -1;
	}

	for (; buf[j]; ++j) {
		if ((buf[j] == ' ') && (buf[j - 1] != ' ')) {
			switch (state) {
			case 1:     /* permissions */
				state = 2;
				break;
			case 2:     /* nlink */
				state = 3;
				if ((j - i) == 6 && 0 == strncmp(buf + i, "folder", 6))
					state = 4;	/*NetPresenz*/
				break;
			case 3:     /* uid or possible size */
				size = getnumeric(buf + i, j - i);
				state = 4;
				break;
			case 4:     /* gid or possible month or size  */
				mon = getmonth(buf + i, j - i);
				if (mon >= 0)
					state = 6;
				else {
					size = getnumeric(buf + i, j - i);
					state = 5;
				}
				break;
			case 5:     /* month, otherwise possible size */
				mon = getmonth(buf + i, j - i);
				if (mon >= 0)
					state = 6;
				else
					size = getnumeric(buf + i, j - i);
				break;
			case 6:     /* have size and month */
				day = (int)getnumeric(buf + i, j - i);
				state = 7;
				break;
			case 7:     /* have size, month, day */
				if ((j - i == 4) && (buf[i + 1] == ':')) {
					hour = (int)getnumeric(buf + i, 1);
					min = (int)getnumeric(buf + i + 2, 2);
					sb.mtime = mtime(-1, mon, day, hour, min);

				} else if ((j - i == 5) && (buf[i + 2] == ':')) {
					hour = (int)getnumeric(buf + i, 2);
					min = (int)getnumeric(buf + i + 3, 2);
					sb.mtime = mtime(-1, mon, day, hour, min);

				} else if (j - i >= 4) {
					year = (int)getnumeric(buf + i, j - i);
					sb.mtime = mtime(year, mon, day, 0, 0);

				} else {
					return -1;
				}

				name = buf + j + 1;
				namelen = strlen(name);
				state = 8;
				break;
			case 8:
				break;
			}
			i = j + 1;
			while (buf[i] && (buf[i] == ' ')) ++i;
		}
	}

	if (state != 8)
		return -1;

	/* link, redirection */
	if ('l' == *buf)
		for (i = 0; i + 3 < namelen; ++i)
			if (' ' == name[i] && 0 == strncmp(name + i, " -> ", 4)) {
				namelen = i;
				break;
			}

	/* Name */
	while (' ' == *name) {
		++name; --namelen;
	}

	if (namelen > 1) {			/* filter 'ls -F' */
		switch (name[namelen - 1]) {
		case '/':	/* directory */
			if (sb.mode & S_IFDIR)
				--namelen;
			break;
		case '@':	/* symlink */
#if defined(S_IFLNK) && (S_IFLNK)
			if (sb.mode & S_IFLNK)
#endif
				--namelen;
			break;
		case '*':	/* executeable */
			if (sb.mode & (S_IXUSR|S_IXGRP|S_IXOTH))
				--namelen;
			break;
		case '=':	/* socket */
		case '%':	/* whiteout */
		case '|':	/* fifo */
			--namelen;
			break;
		}
	}

	/* build */
	sb.size  = size;
	sb.atime = 0;
	if (us) *us = sb;
	if (ue && base) {
		char fn[PATH_MAX] = {0};
		strncpy(fn, name, sizeof(fn)-1);
		return fetch_add_entry2(ue, base, fn, &sb, 0);
	} else if (base) {
		base->doc = malloc(namelen + 1);
		memcpy(base->doc, name, namelen);
		base->doc[namelen] = 0;
	}
	return 0;
}


/*
 *  fetch_win32_entry ---
 *      Parse a WIN32/NT style directory entry.
 *
 *  Examples:
 *
 *      "04-27-00  09:09PM       <DIR>          licensed"
 *      "07-18-00  10:16AM       <DIR>          pub"
 *      "04-14-00  03:47PM                  589 readme.htm"
 */
int
fetch_win32_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf)
{
	struct url_stat sb = {0};
	int year = -1, mon, day, hour, min, size = 0;
	char am_pm[3];

	/* MM-DD-YY */
	while (' ' == *buf) ++buf;
	if ((*buf < '0') || (*buf > '9') ||
		    3 != sscanf(buf, "%2d-%2d-%2d", &mon, &day, &year)) {
		return -1;
	}
	while (*buf && ' ' != *buf) ++buf;

	/* HH:MMXX */
        while (' ' == *buf) ++buf;
	if (3 != sscanf(buf, "%2d:%2d%2s", &hour, &min, am_pm) ||
		    ('A' != am_pm[0] && 'P' != am_pm[0]) || 'M' != am_pm[1]) {
		return -1;
	}
	while (*buf && ' ' != *buf) ++buf;
	if ('P' == am_pm[0]) hour += 12;
	else if (12 == hour) hour = 0;

	/* <DIR> or size */
	sb.mode = S_IRWXU | S_IRWXG | S_IRWXO;
	while (' ' == *buf) ++buf;
	if (*buf == '<') {
		if (0 == strncmp(buf, "<DIR>", 5)) {
			sb.mode |= S_IFDIR;
			buf += 5;
		} else if (0 == strncmp(buf, "<JUNCTION>", 10)) {
			sb.mode |= S_IFLNK;
			buf += 10;
		} else {
			return -1;
		}
	} else if (*buf >= '0' && *buf <= '9') {
		sb.mode |= S_IFREG;
		size = (off_t)strtol(buf, (char **)&buf, 10);
	} else {
		return -1;
	}

	/* Name */
	while (' ' == *buf) ++buf;

	/* build */
	sb.size  = size;
	sb.mtime = mtime(year, mon, day, hour, min);
	sb.atime = 0;
	if (us) *us = sb;
	if (ue && base) {
		return fetch_add_entry2(ue, base, buf, &sb, 0);
	} else if (base) {
		base->doc = strdup(buf);
	}
	return 0;
}


/*
 *  fetch_dos_entry ---
 *      Parse a DOS style directory entry.
 *
 *  Examples:
 *
 *      ".               <DIR>           11-16-97        12:31"
 *      "..              <DIR>           11-16-97        12:36"
 *      "INSTALL         <DIR>           10-16-97        12:17"
 *      "DESIGN.DOC           12264      05-11-98        11:20"
 *      "README.TXT            2045      05-10-95        11:01"
 *
 *   and
 *
 *      "DESIGN.DOC      12264      May 11 1997 14:20   A"
 *      "README.TXT       2045      May 10 1995 11:01"
 *
 *  Note:
 *      o MM/DD/YY separator may be '/' or '-'.
 *      o YY >= 80 1980... otherwise 20..
 */
int
fetch_dos_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf)
{
	struct url_stat sb = {0};
	int year = -1, mon, day, hour, min, size = 0;
	char month[4];
	const char *name;
	int namelen = 0;

	/* Name */
	while (' ' == *buf) ++buf;
	name = buf;
	while (*buf && ' ' != *buf) {
		++buf; ++namelen;
	}

	/* <DIR> or size */
	sb.mode = S_IRWXU | S_IRWXG | S_IRWXO;
	while (' ' == *buf) ++buf;
	if (*buf == '<') {
		if (0 == strncmp(buf, "<DIR>", 5)) {
			sb.mode |= S_IFDIR;
			buf += 5;
		} else {
			return -1;
		}
	} else if (*buf >= '0' && *buf <= '9') {
		sb.mode |= S_IFREG;
		size = (off_t)strtol(buf, (char **)&buf, 10);
	} else {
		return -1;
	}

	/* MM-DD-YY, MM/DD/YY or MMM DD YYYY */
	while (' ' == *buf) ++buf;
	if (*buf >= '0' && *buf <= '9') {
		if (3 != sscanf(buf, "%2d-%2d-%2d", &mon, &day, &year) &&
		    3 != sscanf(buf, "%2d/%2d/%2d", &mon, &day, &year)) {
			return -1;
		}
		year += (year >= 80 ? 1900 : 2000);
		buf += 8;
		while (*buf && ' ' != *buf) ++buf;
	} else {
		if (3 != sscanf(buf, "%3s %2d %4d", month, &day, &year) ||
			    (mon = getmonth(month, 3)) < 0) {
			return -1;
		}
		buf += 11;
	}

	/* HH:MM */
	if (2 != sscanf(buf, "%2d:%2d", &hour, &min)) {
		return -1;
	}

	/* build */
	sb.size  = size;
	sb.mtime = mtime(year, mon, day, hour, min);
	sb.atime = 0;
	if (us) *us = sb;
	if (ue && base) {
		char fn[PATH_MAX] = {0};
		strncpy(fn, name, sizeof(fn)-1);
		return fetch_add_entry2(ue, base, fn, &sb, 0);
	} else if (base) {
		base->doc = malloc(namelen + 1);
		memcpy(base->doc, name, namelen);
		base->doc[namelen] = 0;
	}
	return 0;
}


/*
 *  fetch_mnet_entry ---
 *      Parse a MultiNet style directory entry.
 *
 *  Examples:
 *
 *      o MultiNet (some spaces removed from examples)
 *
 *          "00README.TXT;1      2 30-DEC-1996 17:44 [SYSTEM] (RWED,RWED,RE,RE)"
 *          "CORE.DIR;1          1  8-SEP-1996 16:09 [SYSTEM] (RWE,RWE,RE,RE)"
 *
 *      o Non-MutliNet VMS
 *
 *          "CII-MANUAL.TEX;1  213/216  29-JAN-1996 03:33:12  [ANONYMOU,ANONYMOUS]   (RWED,RWED,,)"
 *
 *      o CMU/VMS-IP FTP
 *
 *          "[VMSSERV.FILES]ALARM.DIR;1 1/3 5-MAR-1993 18:09"
 *
 *  Source:
 *      Modified copy of ftpparse.c
 *      http://cr.yp.to/ftpparse.html, 20001223, D. J. Bernstein, djb@cr.yp.to
 */
int
fetch_mnet_entry(struct url_list *ue, struct url *base, struct url_stat *us, const char *buf)
{
	struct url_stat sb = {0};
	int year = -1, mon, day, hour, min, size = 0;
	char month[4] = {0};
	const char *colon = NULL, *name;
	int namelen;

	/* name */
	while (' ' == *buf) ++buf;
	for (name = buf; *name; ++name) {
		/*
		 *  Validate against MS legal character spec [A-Z0-9$.-_~].
		 */
		const unsigned ch = *((unsigned char *)name);

		if (';' == ch) {
			colon = name;
			break;
		} else if (strchr("$.-_~[]", ch) ||
			     (isalnum(ch) && ch == toupper(ch))) {
			continue;
		}
		return -1;
	}
	if (!colon)
		return -1;
	name = buf;
	namelen = colon - buf;
	if ('[' == *name) {			/* consume directory */
		for (++name, --namelen; name < colon;) {
			--namelen;
			if (']' == *name++)
				break;
		}
		if (!namelen)
			return -1;
	}

	/* type */
	sb.mode = S_IRWXU | S_IRWXG | S_IRWXO;
	if (namelen > 4 && 0 == strncmp(colon - 4, ".DIR", 4)) {
		sb.mode |= S_IFDIR;
		namelen -= 4;
	} else {
		sb.mode |= S_IFREG;
	}
	buf = colon;
	while (*buf && *buf != ' ') ++buf;

	/* size */
	while (*buf == ' ') ++buf;
	if (1 != sscanf(buf, "%d", &size) || size < 0) {
		return -1;
	}
	while (*buf && ' ' != *buf) ++buf;
	size <<= 9;				/* 512 byte blocks */

	/* DD-MMM-YYYY */
	if (5 != sscanf(buf, "%2d-%3s-%d %2d:%2d", &day, month, &year, &hour, &min) ||
		    (mon = getmonth(month, 3)) < 0) {
		return -1;
	}

	/* push result */
	sb.size  = size;
	sb.mtime = mtime(year, mon, day, hour, min);
	sb.atime = 0;
	if (us) *us = sb;
	if (ue && base) {
		char fn[PATH_MAX] = {0};
		strncpy(fn, name, sizeof(fn)-1);
		return fetch_add_entry2(ue, base, fn, &sb, 0);
	} else if (base) {
		base->doc = malloc(namelen + 1);
		memcpy(base->doc, name, namelen);
		base->doc[namelen] = 0;
	}
	return 0;
}


/*
 *  Decode a unix permission specifiation.
 */
static int
getmode(const char *umode, int *endp)
{
	int mode = 0;

	/* type */
	switch (umode[0]) {
	case 'd': /* 'd' - directory. */
		mode = S_IFDIR;
		break;
#if defined(S_IFLNK) && (S_IFLNK)
	case 'l': /* 'l - link. */
		mode = S_IFLNK;
		break;
#endif
	case '-': /* '-' - normal. */
		mode = S_IFREG;
		break;
	default:
		 /* 'c' - character-device. */
		 /* 'b' - block-device. */
		 /* 'p' - fifo/pipe. */
		 /* 's' - sockets. */
		 /* 'n' - name. */
		 /* 'w' - whiteout. */
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
	case '[':
		if (getnetware(umode+1, mode, endp) >= 0) {
			*endp = 8;
			return mode;
		}
		return -1;
	case ' ':
		if ('[' == umode[2]) {
			if (getnetware(umode+2, mode, endp) >= 0) {
				return mode;
			}
		}
		return -1;
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

	*endp = 10;
	DLIST2(("getmode(%.*s)=%o\n", *endp, umode, mode))
	return mode;
}


static int
getnetware(const char *umode, int mode, int *endp)
{
	switch(umode[0]) {
	case '[':
		break;
	default:
		return -1;
	}

	switch(umode[1]) {
	case 'R': case 'W': case 'C': case 'E': case 'M': case 'F': case 'A': case '-':
		break;
	default:
		return -1;
	}

	switch(umode[2]) {
	case 'R': case 'W': case 'C': case 'E': case 'M': case 'F': case 'A': case '-':
		break;
	default:
		return -1;
	}

	switch(umode[3]) {
	case 'R': case 'W': case 'C': case 'E': case 'M': case 'F': case 'A': case '-':
		break;
	default:
		return -1;
	}

	switch(umode[4]) {
	case 'R': case 'W': case 'C': case 'E': case 'M': case 'F': case 'A': case '-':
		break;
	default:
		return -1;
	}

	switch(umode[5]) {
	case 'R': case 'W': case 'C': case 'E': case 'M': case 'F': case 'A': case '-':
		break;
	default:
		return -1;
	}

	switch(umode[6]) {
	case 'R': case 'W': case 'C': case 'E': case 'M': case 'F': case 'A': case '-':
		break;
	default:
		return -1;
	}

	switch(umode[7]) {
	case 'R': case 'W': case 'C': case 'E': case 'M': case 'F': case 'A': case '-':
		break;
	default:
		return -1;
	}

	switch(umode[8]) {
	case ']':
		*endp = 9;
		break;
	case 'R': case 'W': case 'C': case 'E': case 'M': case 'F': case 'A': case '-':
		switch(umode[9]) {
		case ']':
			*endp = 10;
			break;
		default:
			return -1;
		}
		break;
	default:
		return -1;
	}

	DLIST2(("getnetware(%.*s)=%o\n", *endp, umode, mode))
	return mode;
}


/*
 *  Decode a numeric value.
 */
static long
getnumeric(const char *buf, int len)
{
	unsigned char c;
	long val = 0;

	while (0 != (c = *((unsigned char *)buf++)) && --len >= 0) {
		if (c >= '0' && c <= '9')  {
			val *= 10;
			val += (c - '0');
			continue;
		}
		return -1;
	}
	return val;
}


/*
 *  Decode month name.
 */
static int
getmonth(const char *buf, int len)
{
	static const struct {
	    const char *val;
	    size_t len;
	} mnames[] = {
#define MNAME(__m)	{ __m, sizeof(__m)-1 }
	    MNAME("January"),
	    MNAME("February"),
	    MNAME("March"),
	    MNAME("April"),
	    MNAME("May"),
	    MNAME("June"),
	    MNAME("July"),
	    MNAME("August"),
	    MNAME("September"),
	    MNAME("October"),
	    MNAME("November"),
	    MNAME("December")
#undef  MNAME
	    };
	unsigned m;

	for (m = 0; m < 12; ++m) {
		if ((len == mnames[m].len && 0 == strcasecmp(buf, mnames[m].val)) ||
		    (3 == len && 0 == strncasecmp(buf, mnames[m].val, 3))) {
			return m + 1;
		}
	}
	return -1;
}


/*
 *  Make time
 */
static time_t
mtime(int year, int mon, int day, int hour, int min)
{
	struct tm tm = {0};
	time_t t;

	DLIST2(("mtime(%ld/%02ld/%02ld, %02ld:%02ld:%02ld\n", year, mon, day, hour, min, sec))
	if (year < 100) {
		if (-1 == year) {
			const time_t currtime = time(NULL);
#if defined(HAVE_GMTIME_R)
			struct tm *ctm = gmtime_r(&currtime, &tm);
#else
			struct tm *ctm = gmtime(&currtime);
#endif

			year = ctm->tm_year + 1900;
			if (mon > (ctm->tm_mon + 1)) {
				--year;		/* previous year */
			}
		} else {
			year += (year >= 70 ? 1900 : 2000);
		}
	}
	tm.tm_sec   = 0;
	tm.tm_min   = min;
	tm.tm_hour  = hour;
	tm.tm_mday  = day;
	tm.tm_mon   = mon - 1;
	tm.tm_year  = year - 1900;
	tm.tm_isdst = -1;
	t = timegm(&tm);

	DLIST2((" = (%04d/%02d/%02d, %02d:%02d:%02d) : %ld\n", \
	    tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, (long)t))
	return t;
}


#if defined(LOCAL_MAIN)
/*
 *  local test framework.
 */
#include <stdio.h>
#include <stdlib.h>

#include "ftpeplf.c"
#include "ftputil.c"


int
fetch_add_entry(struct url_list *ue, struct url *base, const char *name, int pre_quoted)
{
	return -1;
}


void
main(void)
{
	static const char *tests[] = {
	    /*
	     *  Examples taken from:
	     *    ftpparse.c (http://cr.yp.to/ftpparse.html)
	     *    mirror.pl
	     */
	    "+m838255902,/,\tDIR",
	    "+m843956783,r,s510376,\tarchive.tgz",

	    "-rw-r--r--   1 root     other        531 Jan 29 03:26 README",
	    "dr-xr-xr-x   2 root     other        512 Apr  8  1994 etc",
	    "dr-xr-xr-x   2 root     512 Apr  8  1994 etc",
	    "lrwxrwxrwx   1 root     other          7 Jan 25 00:17 bin -> usr/bin",

	    "dr-xr-xr-x   2 root     other        512 Apr  8  2013 etc2/",
	    "-r-xr-xr-x   1 root     other          7 Jan 25 12:01 bintool*",

	    "----------   1 owner    group         1803128 Jul 10 10:18 ls-lR.Z",
	    "d---------   1 owner    group               0 May  9 19:45 Softlib",
	    "-rwxrwxrwx   1 noone    nogroup      322 Aug 19  1996 message.ftp",
	    "drwx-wx-wt   2 root     wheel        512 Jul  1 02:15 incoming",
	    "drwxr-xr-x   2 0        0            512 May 28 22:17 etc",

	    "-------r--         326  1391972  1392298 Nov 22  1995 MegaPhone.sit",
	    "drwxrwxr-x               folder        2 May 10  1996 network",

	    "d[RWCEMFA] supervisor            512       Jan 16 18:53    login",
	    "-[RWCEMFA] rhesus             214059       Oct 20 15:27    cx.exe",

	    "d [R----F--] supervisor            512       Jan 16 18:53    login",
	    "- [R----F--] rhesus             214059       Oct 20 15:27    cx.exe",

	    "00README.TXT;1      2 30-DEC-1996 17:44 [SYSTEM] (RWED,RWED,RE,RE)",
	    "CORE.DIR;1          1  8-SEP-1996 16:09 [SYSTEM] (RWE,RWE,RE,RE)",
	    "CII-MANUAL.TEX;1  213/216  29-JAN-1996 03:33:12  [ANONYMOU,ANONYMOUS] (RWED,RWED,,)",
	    "FOO.BAR;1 4 5-MAR-1993 18:09:01.12",
	    "[VMSSERV.FILES]ALARM.DIR;1 1/3 5-MAR-1993 18:09",

	    "04-27-00  09:09PM       <DIR>          licensed",
	    " 7-18-00  10:16AM       <DIR>          pub",
	    " 4-14-00  03:47PM                  589 readme.htm",
	    "07-21-00  01:19PM                95077 Enjoy the Good Life.jpg",

	    "INSTALL         <DIR>           10-16-97   12:17",
	    "DESIGN.DOC           12264      05-11-98   11:20",
	    "README.TXT            2045      05/10/95   11:01",
	    "DESIGN.DOC      12264      May 11 1997 14:20   A",
	    NULL
	    };
	char MB[12], SB[32], TB[80];
	const char *test;
	unsigned i;

	for (i = 0; NULL != (test = tests[i]); ++i) {
		struct url base = {0};
		struct url_stat us = {0};
		const int ret =
			fetch_list_entry(NULL, &base, &us, test);

		if (ret >= 0) {
			printf("%s\n %d = %s, %8.8s, %s [%s]\n\n", test, ret,
				fetchDebugMode(us.mode, TB, sizeof(TB)),
					fetchDebugSize(us.size, SB, sizeof(SB)),
					fetchDebugTime(us.mtime, MB, sizeof(MB)),
					(base.doc?base.doc:"(null)"));
		} else {
			printf("%s\n %d = n/a\n\n", test, ret);
		}
	}
}
#endif  /*LOCAL_MAIN*/
/*end*/
