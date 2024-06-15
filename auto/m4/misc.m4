dnl $Id: misc.m4,v 1.15 2024/06/15 06:33:17 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl


dnl ---------------------------------------------------------------------------
dnl CF_CHECK_LIBBFD
dnl

AC_DEFUN([CF_CHECK_LIBBFD],[

	AC_CHECK_LIB(bfd, bfd_init, [
		AC_DEFINE(HAVE_LIBBFD, 1,
			[Define to enable libbfd support])
		AC_CHECK_HEADERS(bfd.h)
		CF_ADD_EXTRALIBS("-lbfd")

		dnl --- bfd_section_flags
		AC_MSG_CHECKING(for bfd_section_flags)
		AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <bfd.h>
asection *s;
],[return bfd_section_flags(s) == 0;])],
		[with_bfd_section_flags=yes],
		[with_bfd_section_flags=no])
		AC_MSG_RESULT([$with_bfd_section_flags])
		if test "$with_bfd_section_flags" = yes; then
			AC_DEFINE(HAVE_BFD_SECTION_FLAGS, 1,
				[Define for newer libbdf API support])
		fi

		dnl --- bfd_section_size
		AC_MSG_CHECKING(for bfd_section_size)
		AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <bfd.h>
asection *s;
],[return bfd_section_size(s) == 0;])],
		[with_bfd_section_size=yes],
		[with_bfd_section_size=no])
		AC_MSG_RESULT([$with_bfd_section_size])
		if test "$with_bfd_section_size" = yes; then
			AC_DEFINE(HAVE_BFD_SECTION_SIZE, 1,
				[Define for newer libbdf API support])
		fi

		dnl --- bfd_section_vma
		AC_MSG_CHECKING(for bfd_section_vma)
		AC_LINK_IFELSE([AC_LANG_PROGRAM([
#include <bfd.h>
asection *s;
],[return bfd_section_vma(s) == 0;])],
		[with_bfd_section_vma=yes],
		[with_bfd_section_vma=no])
		AC_MSG_RESULT([$with_bfd_section_vma])
		if test "$with_bfd_section_vma" = yes; then
			AC_DEFINE(HAVE_BFD_SECTION_VMA, 1,
				[Define for newer libbdf API support])
		fi

	], [], [$EXTRALIBS])

	AC_SUBST([HAVE_BFD_SECTION_FLAGS])
	AC_SUBST([HAVE_BFD_SECTION_SIZE])
	AC_SUBST([HAVE_BFD_SECTION_VMA])

])dnl


dnl ---------------------------------------------------------------------------
dnl CF_ADD_EXTRALIBS
dnl
dnl Add a lib to current LIBS if it is not already there.
dnl

AC_DEFUN([CF_ADD_EXTRALIBS],[
	EXTRALIBS_addsave=[$EXTRALIBS]
	if test "x$EXTRALIBS_addsave" != "x" ; then
		flag=`echo "$1" | sed 's/-/\\\-/g'`
		if test -z "`echo \"${EXTRALIBS}\" | grep \"${flag}\"`" ; then
			EXTRALIBS="$EXTRALIBS_addsave $1"
		fi
	else
		EXTRALIBS="$1"
	fi
])dnl

dnl ---------------------------------------------------------------------------
dnl Check if we have either a function or macro for additional ctypes
dnl
dnl   isascii     (extension)
dnl   isblank     (c99)
dnl

AC_DEFUN([CF_FUNC_ISASCII],[
	AC_MSG_CHECKING(for isascii)
	AC_CACHE_VAL(cf_cv_have_isascii,[
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <ctype.h>]],[[int x = isascii(' ')]])],
			[cf_cv_have_isascii=yes],
			[cf_cv_have_isascii=no])
		])dnl
	AC_MSG_RESULT($cf_cv_have_isascii)
	test "$cf_cv_have_isascii" = yes && \
		AC_DEFINE([HAVE_ISASCII], 1, [isascii() available.])
])dnl

AC_DEFUN([CF_FUNC_ISBLANK],[
	AC_MSG_CHECKING(for isblank)
	AC_CACHE_VAL(cf_cv_have_isblank,[
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <ctype.h>]],[[int x = isblank(' ')]])],
			[cf_cv_have_isblank=yes],
			[cf_cv_have_isblank=no])
		])dnl
	AC_MSG_RESULT($cf_cv_have_isblank)
	test "$cf_cv_have_isblank" = yes && \
		AC_DEFINE([HAVE_ISBLANK], 1, [isblank() available.])
])dnl

AC_DEFUN([CF_FUNC_ISCSYM],[
	AC_MSG_CHECKING(for iscsym)
	AC_CACHE_VAL(cf_cv_have_iscsym,[
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <ctype.h>]],[[int x = iscsym(' ')]])],
			[cf_cv_have_iscsym=yes],
			[cf_cv_have_iscsym=no])
		])dnl
	AC_MSG_RESULT($cf_cv_have_iscsym)
	test "$cf_cv_have_iscsym" = yes && \
		AC_DEFINE([HAVE_ISCSYM], 1, [iscsym() available.])
])dnl


dnl ---------------------------------------------------------------------------
dnl Check for a working mkstemp.  This creates two files, checks that they are
dnl successfully created and distinct (AmigaOS apparently fails on the last).

AC_DEFUN([CF_FUNC_MKSTEMP],[
	AC_CACHE_CHECK(for working mkstemp, cf_cv_func_mkstemp,[
	rm -f conftest*
	AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>]],[[
	char *tmpl = "conftestXXXXXX";
	char name[2][80];
	int n;
	int result = 0;
	int fd;
	struct stat sb;

	umask(077);
	for (n = 0; n < 2; ++n) {
		strcpy(name[n], tmpl);
		if ((fd = mkstemp(name[n])) >= 0) {
			if (!strcmp(name[n], tmpl)
			|| stat(name[n], &sb) != 0
			|| (sb.st_mode & S_IFMT) != S_IFREG
			|| (sb.st_mode & 077) != 0) {
				result = 1;
			}
			close(fd);
		}
	}
	if (result == 0	&& !strcmp(name[0], name[1]))
		result = 1;
	exit(result);]])],
			[cf_cv_func_mkstemp=yes],
			[cf_cv_func_mkstemp=no],
			[cf_cv_func_mkstemp=no]
		[AC_CHECK_FUNC(mkstemp)])
	])
	if test "$cf_cv_func_mkstemp" = yes ; then
		AC_DEFINE([HAVE_MKSTEMP], 1, [mkstemp().])
	fi
])dnl


dnl ---------------------------------------------------------------------------
dnl nl_langinfo and CODESET
dnl

AC_DEFUN([CF_LANGINFO_CODESET],[
	AC_CACHE_CHECK([for nl_langinfo and CODESET], cf_cv_langinfo_codeset,[
		AC_RUN_IFELSE([AC_LANG_PROGRAM([[#include <langinfo.h>]],[[char* cs = nl_langinfo(CODESET);]])],
			[cf_cv_langinfo_codeset=yes],
			[cf_cv_langinfo_codeset=no])
		])
	if test $cf_cv_langinfo_codeset = yes; then
		AC_DEFINE(HAVE_LANGINFO_CODESET, 1,
			[Define if you have <langinfo.h> and nl_langinfo(CODESET).])
	fi
])dnl

dnl -- end
