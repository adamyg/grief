dnl $Id: libmalloc.m4,v 1.5 2024/05/02 14:34:32 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     malloc library and system memory interface support
dnl

AC_DEFUN([LIBMALLOC_CHECK_CONFIG],[
	AC_MSG_NOTICE([determining memory allocation support])

	AC_CHECK_HEADERS(sys/mman.h)
	AC_CHECK_FUNCS(mmap mprotect mremap)
	AC_CHECK_FUNCS(madvise posix_madvise)

	AC_SUBST(LIBMALLOC)
	with_libmalloc="no"

	AC_MSG_CHECKING(--with-malloclib options)
	AC_ARG_WITH(dlmalloc,
	    [  --with-dlmalloc         use Doug Lees dlmalloc library], with_libmalloc=dlmalloc)

	AC_ARG_WITH(nedmalloc,
	    [  --with-nedmalloc        use nedmalloc library],          with_libmalloc=nedmalloc)

	AC_ARG_WITH(dbmalloc,
	    [  --with-dbmalloc         use Conor Cahills library],      with_libmalloc=dbmalloc)

	AC_ARG_WITH(tcmalloc,
	    [  --with-tcmalloc         use Googles tcmalloc library],   with_libmalloc=tcmalloc)

	AC_ARG_WITH(malloclib,
	    [  --with-malloclib=lib    use optional 'lib' (or libmalloc) instead of system malloc],
			with_libmalloc=$witheval)

	AC_MSG_RESULT($with_libmalloc)

	if test -n "$with_libmalloc"; then
		cf_save_LIBS="$LIBS"
		malloclib_name=$with_libmalloc

		if test "x$malloclib_name" = "xdlmalloc"; then
			AC_DEFINE([HAVE_LIBDLMALLOC], 1, [Have libdlmalloc])
			LIBMALLOC="-ldlmalloc"
			AC_CHECK_HEADERS(dlmalloc.h)

		else if test "x$malloclib_name" = "xnedmalloc"; then
			AC_DEFINE([HAVE_LIBNEDMALLOC], 1, [Have libnedmalloc])
			LIBMALLOC="-lnedmalloc"
			AC_CHECK_HEADERS(nedmalloc.h)

		else if test "x$malloclib_name" = "xdbmalloc"; then
			AC_DEFINE([HAVE_LIBDBMALLOC], 1, [Have libdbmalloc])
			LIBMALLOC="-ldbmalloc"

		else if test "x$malloclib_name" = "xtcmalloc"; then
			AC_DEFINE([HAVE_LIBTCMALLOC], 1, [Have libtcmalloc])
			LIBMALLOC="-ltcmalloc"

		else if test "x$malloclib_name" != "xno"; then
			AC_DEFINE([HAVE_LIBMALLOC], "$malloclib_name", [malloc library name])
			LIBMALLOC="-l$malloclib_name"

		fi; fi; fi; fi; fi

		LIBS="$cf_save_LIBS"
	fi
])dnl


dnl     Malloc options and diagnostics
dnl
dnl             HAVE_MALLOC_STATS
dnl             HAVE_MALINFO
dnl             HAVE_MALOPT
dnl
dnl             HAVE_STRUCT_MALLINFO_HBLKS
dnl             HAVE_STRUCT_MALLINFO_KEEPCOST
dnl             HAVE_STRUCT_MALLINFO_TREEOVERHEAD
dnl             HAVE_STRUCT_MALLINFO_GRAIN
dnl             HAVE_STRUCT_MALLINFO_ALLOCATED
dnl

AC_DEFUN([CF_MALLOC_OPT],[
	AC_CHECK_FUNCS(\
		malloc_stats \
		mallinfo \
		mallopt)

#       AC_SEARCH_LIBS(mallinfo,malloc,[
#               AC_DEFINE([HAVE_MALLINFO],[1],[Define if mallinfo() is available on this platform.])])
#       AC_CHECK_TYPES([struct mallinfo],,, [#include <malloc.h>])
#       AC_CHECK_MEMBER([struct mallinfo.hblks])
#       AC_CHECK_MEMBER([struct mallinfo.keepcost])
#       AC_CHECK_MEMBER([struct mallinfo.treeoverhead])
#       AC_CHECK_MEMBER([struct mallinfo.grain])
#       AC_CHECK_MEMBER([struct mallinfo.allocated])
])

dnl end