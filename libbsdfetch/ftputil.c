/* -*- mode: c; indent-width: 4; -*- */
/* $Id: ftputil.c,v 1.1 2014/11/24 23:58:09 ayoung Exp $
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

#include <libstr.h>
#include "fetch.h"
#include "common.h"


const char *
fetchDebugTime(time_t timestamp, char *buffer, unsigned buflen)
{
	static const char *mnames[] = {
	    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
	    };
#if defined(HAVE_GMTIME_R)
	struct tm ltm, *tm = gmtime_r(&timestamp, &ltm);
#else
	struct tm *tm = gmtime(&timestamp);
#endif

	snprintf(buffer, buflen, "%04d %s %02d  %02d:%02d:%02d",
	    tm->tm_year + 1900, mnames[tm->tm_mon], tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	buffer[buflen-1] = 0;
	return buffer;
}


const char *
fetchDebugMode(unsigned mode, char *buffer, unsigned buflen)
{
	if (buflen <= 10) return "n/a";

	/* type */
	if (S_ISDIR(mode)) {
		buffer[0] = ('d');		/* directory */

#if defined(S_ISCHR)
	} else if (S_ISCHR(mode)) {
		buffer[0] = ('c');		/* character device */
#endif
#if defined(S_ISBLK)
	} else if (S_ISBLK(mode)) {
		buffer[0] = ('b');		/* block device */
#endif
#if defined(S_ISLNK)
	} else if (S_ISLNK(mode)) {
		buffer[0] = ('l');		/* link */
#endif
#if defined(S_ISFIFO)
	} else if (S_ISFIFO(mode)) {
		buffer[0] = ('p');		/* fifo/pipe */
#endif
#if defined(S_ISSOCK)
	} else if (S_ISSOCK(mode)) {
		buffer[0] = ('s');		/* sockets */
#endif
#if defined(S_ISNAM)
	} else if (S_ISNAM(mode)) {
		buffer[0] = ('n');		/* name */
#endif
#if defined(S_ISDOOR)
	} else if (S_ISDOOR(mode)) {
		buffer[0] = ('D');		/* door */
#endif
	} else {
		buffer[0] = '-';
	}

	/* permissions */
#if !defined(S_IWGRP)				/* MINGW32 etc */
#define S_IRGRP		S_IRUSR
#define S_IWGRP		S_IWUSR
#define S_IXGRP		S_IXUSR
#endif
#if !defined(S_IROTH)
#define S_IROTH		S_IRUSR
#define S_IWOTH		S_IWUSR
#define S_IXOTH		S_IXUSR
#endif
	buffer[1] = (mode & S_IRUSR ? 'r' : '-');
	buffer[2] = (mode & S_IWUSR ? 'w' : '-');
	buffer[3] = (mode & S_IXUSR ? 'x' : '-');
	buffer[4] = (mode & S_IRGRP ? 'r' : '-');
	buffer[5] = (mode & S_IWGRP ? 'w' : '-');
	buffer[6] = (mode & S_IXGRP ? 'x' : '-');
	buffer[7] = (mode & S_IROTH ? 'r' : '-');
	buffer[8] = (mode & S_IWOTH ? 'w' : '-');
	buffer[9] = (mode & S_IXOTH ? 'x' : '-');
	buffer[10] = '\0';

	/* sticky bits */
#if defined(S_ISUID) && (S_ISUID)
	if (mode & S_ISUID) {
		buffer[3] = (buffer[3] == 'x') ? 's' : 'S';
	}
#endif
#if defined(S_ISGID) && (S_ISGID)
	if (mode & S_ISGID) {
		buffer[6] = (buffer[6] == 'x') ? 's' : 'S';
	}
#endif
#if defined(S_ISVTX) && (S_ISVTX)
	if (mode & S_ISVTX) {
		buffer[9] = (buffer[9] == 'x') ? 't' : 'T';
	}
#endif
	return buffer;
}


const char *
fetchDebugSize(off_t size, char *buffer, unsigned buflen)
{
#define KBYTES		1024
	static const char *units[] = {
	    "Kb", "Mb", "Gb", "Tb", "Pb", "Eb"
	    };

	if (size >= KBYTES) {
		double sz = ((double)size) / KBYTES;
		int unit = 0;

		while (sz >= KBYTES && unit < (int)(sizeof(units)/sizeof(units[0]))) {
			sz /= KBYTES; ++unit;
		}
		snprintf(buffer, buflen, "%.2f%s", sz, units[unit]);
	} else {
		snprintf(buffer, buflen, "%d", (int)size);
	}
	buffer[buflen-1] = 0;
	return buffer;
}


