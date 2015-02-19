dnl $Id: libthread.m4,v 1.3 2013/03/23 00:33:04 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     threading support
dnl

AC_DEFUN([CF_WITH_THREADS],[
	AC_SUBST([LIBTHREAD])

	ACX_NANOSLEEP
	ACX_PTHREAD([
		AC_CHECK_HEADERS(pthread.h)

		AC_MSG_RESULT([-  Threading:])
		AC_MSG_RESULT([-         Compiler: $PTHREAD_CC])
		AC_MSG_RESULT([-           CFLAGS: $PTHREAD_CFLAGS])
		AC_MSG_RESULT([-        Libraries: $PTHREAD_LIBS $NANOSLEEP_LIBS])
		AC_MSG_RESULT([-])

		CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
		CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
		LDFLAGS="$LDFLAGS $PTHREAD_CFLAGS"
		LIBTHREAD="$PTHREAD_LIBS $NANOSLEEP_LIBS"
	],[
		AC_MSG_WARN([Compiling without POSIX threads support])
		AC_CHECK_HEADERS(thread.h)
		LIBTHREAD="$NANOSLEEP_LIB"
	])
])



