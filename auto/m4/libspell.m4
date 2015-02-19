dnl $Id: libspell.m4,v 1.3 2013/03/23 00:07:31 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     spell library support
dnl           none
dnl           enchant
dnl           hunspell
dnl           aspell
dnl
AC_DEFUN([LIBSPELL_CHECK_CONFIG],[
	AC_MSG_NOTICE([determining spell checker support])

	LIBSPELL=
	libspell_name="no"
	AC_SUBST(LIBSPELL)

	AC_MSG_CHECKING(--with-spelllib options)
	AC_ARG_WITH(nopell,
	    [  --without-spell         disable spell],
			with_libspell=none)

	AC_ARG_WITH(huspell,
	    [  --with-enchant          enchant library],
			with_libspell=enchant)

	AC_ARG_WITH(huspell,
	    [  --with-hunspell         hunspell library],
			with_libspell=hunspell)

	AC_ARG_WITH(aspell,
	    [  --with-aspell           aspell library],
			with_libspell=aspell)

	AC_ARG_WITH(spelllib,
	    [  --with-spelllib=lib     use optional 'lib'],
			with_libspell=$witheval)

	AC_MSG_RESULT($with_libspell)

	if test -n "$with_libspell"; then
		spelllib_name=$with_libspell
	else
		AC_MSG_RESULT([no spelllib options, checking for suitable spell library])

		cf_save_LIBS="$LIBS"
		spelllibs="enchant hunspell aspell"
		for libname in $spelllibs; do
			AC_CHECK_LIB(${libname}, main)
			if test "x$cf_save_LIBS" != "x$LIBS"; then
				spelllib_name=$libname
				break
			fi
		done

		if test -z "$spelllib_name"; then
			AC_MSG_RESULT(no spell library found)
		fi
		LIBS="$cf_save_LIBS"
	fi

	dnl
	dnl library specific resources
	dnl
	if test "x$spelllib_name" = "xenchant"; then
		AC_CHECK_HEADERS(enchant/enchant.h)
		AC_CHECK_HEADERS(enchant/enchant++.h)
		if test "x$ac_cv_header_enchant_enchant_h" = "x"; then
			AC_CHECK_HEADERS(enchant.h)
			AC_CHECK_HEADERS(enchant++.h)
		fi
		AC_DEFINE([HAVE_LIBENCHANT], 1, [Have libenchant -- spell library interface])
		LIBSPELL="-lenchant"

	else if test "x$spelllib_name" = "xhunspell"; then
		AC_CHECK_HEADERS(hunspell/hunspell.h)
		AC_CHECK_HEADERS(hunspell/hunspell.hxx)
		if test "x$ac_cv_header_hunspell_hunspell_h" = "x"; then
			AC_CHECK_HEADERS(hunspell.h)
			AC_CHECK_HEADERS(hunspell.hxx)
		fi
		AC_DEFINE([HAVE_LIBHUSPELL], 1, [Have libhunspell])
		LIBSPELL="-lhunspell"

	else if test "x$spelllib_name" = "xaspell"; then
		AC_CHECK_HEADERS(aspell/aspell.h)
		if test "x$ac_cv_header_aspell_aspell_h" = "x"; then
			AC_CHECK_HEADERS(aspell.h)
		fi
		AC_DEFINE([HAVE_LIBASPELL], 1, [Have libaspell])
		LIBSPELL="-laspell"

	else if test "x$spelllib_name" != "x"; then
		if test "x$spelllib_name" != "xnone"; then
			AC_DEFINE([HAVE_LIBSPELL], "$spelllib_name", [spell library name])
			LIBSPELL="-l$spelllib_name"
		fi

	fi; fi; fi; fi
])dnl






