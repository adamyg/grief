# $Id: acx_nanosleep.m4,v 1.3 2013/04/05 19:52:09 ayoung Exp $
# Check for support for nanosleep.
# -*- mode: Autoconf; tabs: 8; -*-
#
#       Defined in <time.h>, yet on some systems, such as Solaris, you need an additional
#       library to be linked.
#
#       acx_nanosleep_ok if nanosleep is supported; in that case, NANOSLEEP_LIBS
#       is set to whatever libraries are needed to support nanosleep.
#

AC_DEFUN([ACX_NANOSLEEP],[
	AC_MSG_CHECKING(if nanosleep requires any libraries)
	AC_LANG_SAVE
	AC_LANG_C

	acx_nanosleep_ok="no"
	NANOSLEEP_LIBS=

	# For most folks, this should just work, unless Mingw32
	AC_TRY_LINK([
#ifdef __MINGW32__
#include <windows.h> /*Sleep*/
#include <pthread.h> /*timespec*/
#endif
#include <time.h>],[
  static struct timespec ts = {1, 0};
#ifdef __MINGW32__
  Sleep((ts.tv_sec * 1000.0) + (ts.tv_nsec / 1000.0));
#else
  nanosleep(&ts, NULL);
#endif],[acx_nanosleep_ok=yes])

	# Solaris, -lrt
	if test "x$acx_nanosleep_ok" != "xyes"; then
		cf_save_LIBS="$LIBS"
		LIBS="$LIBS -lrt"
		AC_TRY_LINK([#include <time.h>],
			[static struct timespec ts; nanosleep(&ts, NULL);],
			[acx_nanosleep_ok=yes])
		if test "x$acx_nanosleep_ok" = "xyes"; then
			NANOSLEEP_LIBS="-lrt"
		fi
		LIBS="$cf_save_LIBS"
	fi

	if test "x$acx_nanosleep_ok" != "xyes"; then
		AC_MSG_ERROR([cannot find the nanosleep function])
	else
		AC_MSG_RESULT(${NANOSLEEP_LIBS:-no})
	fi
	AC_LANG_RESTORE
])dnl
