dnl $Id: liblzma.m4,v 1.1 2017/01/30 04:03:27 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     liblzma options
dnl

AC_DEFUN([CF_WITH_LIBLZMA],[
	AC_MSG_CHECKING(whether to enable use of lzma)
	AC_ARG_WITH(lzma,
		[  --with-lzma             enable liblzma use (default=yes)],
		[case "$withval" in
			yes)  with_liblzma=yes;;
			no)   with_liblzma=no;;
			*)    AC_MSG_ERROR(bad value ${withval} for --with-liblzma);;
		esac],
		[with_liblzma=yes])
	AC_MSG_RESULT($with_liblzma)
	if test "$with_liblzma" = no; then
		AC_DEFINE(NO_LIBLZMA, 1,
				[Define to disable lzma functionality])
	fi

	AC_ARG_WITH(lzma-h,
		[  --with-lzma-h=DIR       liblzma header location],
		[LIBLZMA_H_DIR=$withval; with_liblzma=yes])

	AC_ARG_WITH(lzma-lib,
		[  --with-lzma-lib=DIR     liblzma library location],
		[LIBLZMA_LIB_DIR=$withval; with_liblzma=yes])

	if test "$with_liblzma" = yes; then
		CF_LIB_LZMA_IMPL
	fi
])

AC_DEFUN([CF_LIB_LZMA_IMPL],[
	if test -n "$LIBLZMA_H_DIR"; then
		AC_MSG_NOTICE([Searching using explicit <lzma.h> path $LIBLZMA_H_DIR])
		AC_CHECK_HEADERS($LIBLZMA_H_DIR/lzma,h,[
			CFLAGS="$CFLAGS -I$LIBLZMA_H_DIR"
			liblzma_header=yes],
			[AC_MSG_ERROR([<lzma.h> not found at specified location])])
	else
		AC_CHECK_HEADERS(lzma.h,[liblzma_header=yes])
	fi

	LIBLZMA=
	if test "$liblzma_header" = yes; then
		AC_CHECK_LIB(lzma, lzma_code, have_liblzma=yes)
		if test "$have_liblzma" = yes; then
			AC_DEFINE(HAVE_LIBLZMA, 1, [Define if liblzma is available])
			LIBLZMA="-llzma"
		fi
	fi
	AC_SUBST(LIBLZMA)
])

dnl end
