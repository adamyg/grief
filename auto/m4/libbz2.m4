dnl $Id: libbz2.m4,v 1.1 2017/01/30 04:03:27 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     libbz2 options
dnl

AC_DEFUN([CF_WITH_LIBBZ2],[
	AC_MSG_CHECKING(whether to enable use of bz2)
	AC_ARG_WITH(bz2,
		[  --with-bz2              enable libbz2 use (default=yes)],
		[case "$withval" in
			yes)  with_libbz2=yes;;
			no)   with_libbz2=no;;
			*)    AC_MSG_ERROR(bad value ${withval} for --with-libbz2);;
		esac],
		[with_libbz2=yes])
	AC_MSG_RESULT($with_libbz2)
	if test "$with_libbz2" = no; then
		AC_DEFINE(NO_LIBBZ2, 1, [Define to disable bz2 functionality])
	fi

	AC_ARG_WITH(bz2-h,
		[  --with-bz2-h=DIR        libbz2 header location],
		[LIBBZ2_H_DIR=$withval; with_libbz2=yes])

	AC_ARG_WITH(bz2-lib,
		[  --with-bz2-lib=DIR      libbz2 library location],
		[LIBBZ2_LIB_DIR=$withval; with_libbz2=yes])

	if test "$with_libbz2" = yes; then
		CF_LIB_BZ2_IMPL
	fi
])

AC_DEFUN([CF_LIB_BZ2_IMPL],[
	if test -n "$LIBBZ2_H_DIR"; then
		AC_MSG_NOTICE([Searching using explicit <bzlib.h> path $LIBBZ2_H_DIR])
		AC_CHECK_HEADERS($LIBBZ2_H_DIR/bzlib,[
			CFLAGS="$CFLAGS -I$LIBBZ2_H_DIR"
			libbz2_header=yes],
			[AC_MSG_ERROR([<bzlib.> not found at specified location])])
	else
		AC_CHECK_HEADERS(bzlib.h,[libbz2_header=yes])
	fi

	LIBBZ2=
	if test "$libbz2_header" = yes; then
		AC_CHECK_LIB(bz2, BZ2_bzCompress, have_libbz2=yes)
		if test "$have_libbz2" = yes; then
			AC_DEFINE(HAVE_LIBBZ2, 1, [Define if libbz2 is available])
			LIBBZ2="-lbz2"
		fi
	fi
	AC_SUBST(LIBBZ2)
])

dnl end
