dnl $Id: libthread.m4,v 1.5 2024/07/13 18:28:49 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     threading support
dnl

AC_DEFUN([CF_WITH_THREADS],[
	AC_SUBST([LIBTHREAD])

	dnl threads
	AC_CHECK_HEADERS(threads.h)
	AC_CHECK_LIB(stdthreads, thrd_create, [
		AC_DEFINE(HAVE_STDTHREADS, 1, [Define if we have libstdthreads])
		THREADS_LIBS="-lstdthreads"
		],[THREADS_LIBS=""])

	dnl pthread
	ACX_NANOSLEEP
	AX_PTHREAD([
		AC_CHECK_HEADERS(pthread.h)
		CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
		CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
		LDFLAGS="$LDFLAGS $PTHREAD_CFLAGS"
		LIBTHREAD="$THREADS_LIBS $PTHREAD_LIBS $NANOSLEEP_LIBS"
	],[
		AC_MSG_WARN([Compiling without POSIX threads support])
		AC_CHECK_HEADERS(thread.h)
		LIBTHREAD="$THREADS_LIBS $NANOSLEEP_LIB"
	])

	AC_MSG_RESULT([-  Threading:])
	AC_MSG_RESULT([-         Compiler: $PTHREAD_CC])
	AC_MSG_RESULT([-           CFLAGS: $PTHREAD_CFLAGS])
	AC_MSG_RESULT([-        Libraries: $LIBTHREAD])
	AC_MSG_RESULT([-])

])
dnl
