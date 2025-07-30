dnl $Id: libicu.m4,v 1.4 2025/07/03 08:40:04 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl  libicu options
dnl
dnl  Usage:
dnl     CF_WITH_LIBICU(<version>)
dnl

AC_DEFUN([CF_WITH_LIBICU],[
	AC_MSG_CHECKING(whether to enable use of icu)
	AC_ARG_WITH(icu,
		[  --with-icu              enable libicu use (default=no)],
		[case "$withval" in
			yes)  with_libicu=yes;;
			no)   with_libicu=no;;
			*)    AC_MSG_ERROR(bad value ${withval} for --with-libicu);;
		esac],
		[with_libicu=no])
	AC_MSG_RESULT($with_libicu)
	if test "$with_libicu" = yes; then
		AC_DEFINE(NEED_LIBICU, 1,
			[Define to enable ICU functionality])
	fi

	AC_ARG_WITH(icu-h,
		[  --with-icu-h=DIR        libicu header location],
		[LIBICU_H_DIR=$withval; with_libicu=yes])

	AC_ARG_WITH(icu-lib,
		[  --with-icu-lib=DIR      libicu library location],
		[LIBICU_LIB_DIR=$withval; with_libicu=yes])

	if test "$with_libicu" = yes; then
		CF_LIB_ICU($1)
	fi
])


dnl  ICU, populate the following
dnl
dnl     HAVE_LIBICU
dnl     LIBICU_CFLAGS
dnl     LIBICU_CXXFLAGS
dnl     LIBICU
dnl
dnl  Usage:
dnl     CF_LIB_ICU(<version>)
dnl

AC_DEFUN([CF_LIB_ICU],[
	LIBICU_VERSION=""
	LIBICU_CFLAGS=""
	LIBICU_CXXFLAGS=""
	LIBICU=""

	if test -z "$have_icu_lib" ; then
		if test -z "$ICU_CONFIG" ; then
			AC_PATH_PROG(ICU_CONFIG, icu-config, no)
		fi
		if test "$ICU_CONFIG" = "no" ; then
			AC_CHECK_TOOL([PKG_CONFIG], [pkg-config])
			if test x"$PKG_CONFIG" != x""; then
				if $PKG_CONFIG --exists icu-uc 2>/dev/null; then
					LIBICU_VERSION=`$PKG_CONFIG --modversion icu-uc 2>/dev/null`
				else
					LIBICU_VERSION=0
				fi

				AC_MSG_CHECKING(for ICU verion >= $1)
					LIBICU_VERSION_CHECK=`expr $LIBICU_VERSION \>\= $1`
				AC_MSG_RESULT($LIBICU_VERSION)

				if test "$LIBICU_VERSION_CHECK" = "1" ; then
					AC_MSG_CHECKING(LIBICU_CFLAGS)
					LIBICU_CFLAGS=`$PKG_CONFIG --cflags icu-i18n 2>/dev/null`
					AC_MSG_RESULT($LIBICU_CFLAGS)

					AC_MSG_CHECKING(LIBICU_CXXFLAGS)
					LIBICU_CXXFLAGS=`$PKG_CONFIG --cflags icu-i18n 2>/dev/null`
					AC_MSG_RESULT($LIBICU_CXXFLAGS)

					AC_MSG_CHECKING(LIBICU)
					LIBICU=`$PKG_CONFIG --libs-only-l icu-i18n 2>/dev/null`
					AC_MSG_RESULT($LIBICU)

					have_icu_lib=yes
				else
					echo "*** ICU not available"
				fi
			else
				echo "*** neither icu-config or pkg-config could not be found within your path"
				echo "*** see http://ibm.com/software/globalization/icu/"
			fi
		else
			LIBICU_VERSION=`$ICU_CONFIG --version`
			AC_MSG_CHECKING(for ICU verion >= $1)
				LIBICU_VERSION_CHECK=`expr $LIBICU_VERSION \>\= $1`
			AC_MSG_RESULT($LIBICU_VERSION)

			if test "$LIBICU_VERSION_CHECK" = "1" ; then
				AC_MSG_CHECKING(LIBICU_CFLAGS)
				LIBICU_CFLAGS=`$ICU_CONFIG --cflags`
				AC_MSG_RESULT($LIBICU_CFLAGS)

				AC_MSG_CHECKING(LIBICU_CXXFLAGS)
				LIBICU_CXXFLAGS=`$ICU_CONFIG --cxxflags`
				AC_MSG_RESULT($LIBICU_CXXFLAGS)

				AC_MSG_CHECKING(LIBICU)
				LIBICU=`$ICU_CONFIG --ldflags`
				AC_MSG_RESULT($LIBICU)

				have_icu_lib=yes
			else
				echo "*** ICU version is old, please update"
				echo "*** see http://ibm.com/software/globalization/icu/"
			fi
		fi
	fi

dnl	if test -z "$have_icu_lib" ; then
dnl		if test -z "$PKG_CONFIG"; then
dnl			AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
dnl		fi
dnl		if test "$PKG_CONFIG" != "no" ; then
dnl			if $PKG_CONFIG --exists icu; then
dnl				LIBICU=`$PKG_CONFIG --libs-only-L icu`
dnl				have_icu_lib=yes
dnl			fi
dnl		fi
dnl	fi

	if test "$have_icu_lib" = yes ; then
		AC_DEFINE(HAVE_LIBICU, 1,
			[Define if you have ICU installed.])
	else
		AC_MSG_ERROR([Library requirements (ICU) not met.])
	fi

	AC_SUBST(ICU_CONFIG)
	AC_SUBST(LIBICU_VERSION)
	AC_SUBST(LIBICU_CFLAGS)
	AC_SUBST(LIBICU_CXXFLAGS)
	AC_SUBST(LIBICU)
])dnl
