dnl $Id: libclang.m4,v 1.3 2024/05/02 14:34:31 cvsuser Exp $
dnl libclang support
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl

AC_DEFUN([CF_WITH_LIBCLANG],[
	AC_MSG_CHECKING(whether to enable use of clang)

	AC_ARG_ENABLE(libclang,
		[  --disable-libclang     disable use of libclang.],[with_libclang=no],)
	AC_ARG_WITH(clang,
		[  --with-libclang        enable libclang use (default=yes)],
		[case "$withval" in
			yes)  with_libclang=yes;;
			no)   with_libclang=no;;
			*)    AC_MSG_ERROR(bad value ${withval} for --with-libclang);;
		esac],
		[with_libclang=yes])

	AC_ARG_WITH(clang-h,
		[  --with-clang-h=DIR      libclang header location],
		[LIBCLANG_H_DIR=$withval; with_libclang=yes])

	AC_ARG_WITH(clang-lib,
		[  --with-clang-lib=DIR    libclang library location],
		[LIBCLANG_LIB_DIR=$withval; with_libclang=yes])

	LIBCLANG=   
	if test "$with_libclang" = yes; then
		CF_LIB_CLANG
	fi
	AC_SUBST(HAVE_LIBCLANG)
	AC_SUBST(HAVE_CLANG_INDEX_H)
	AC_SUBST(LIBCLANG)
])


dnl ---------------------------------------------------------------------------
dnl

AC_DEFUN([CF_LIB_CLANG],[
	LIBCLANG=
   
	if test -n "$LIBCLANG_H_DIR"; then
		AC_MSG_NOTICE([searching using explicit clang include path $LIBCLANG_H_DIR])
		AC_CHECK_HEADERS("$LIBCLANG_H_DIR/clang-c/Index.h",[
			CFLAGS="$CFLAGS -I$LIBCLANG_H_DIR"
			libclang_header=yes],
			[AC_MSG_ERROR([clang-c/Index.h not found a specified location])])
	fi

	if test "$with_libclang" != "no"; then
		AC_CHECK_HEADERS("clang-c/Index.h",[libclang_header=yes])
		if test "$ac_cv_header_clang_c_Index_h" = "yes"; then
			if test -n "$LIBCLANG_LIB_DIR"; then
				cf_save_LIBS="$LIBS"
				LIBS="$LIBS -L$LIBCLANG_LIB_DIR"
				AC_MSG_NOTICE([searching using explicit clang lib path $LIBCLANG_H_DIR])
				AC_CHECK_LIB(clang, clang_visitChildren, LIBCLANG="-L$LIBCLANG_LIBDIR -lclang")
				LIBS="$cf_save_LIBS"
			else
				AC_CHECK_LIB(clang, clang_visitChildren, LIBCLANG="-lclang")
			fi
			if test "$ac_cv_lib_clang_clang_visitChildren" = "yes"; then
				AC_DEFINE(HAVE_LIBCLANG, 1, [libclang.])
			else
				LIBCLANG=
			fi
		fi 
	fi
	AC_SUBST(LIBCLANG)
])dnl
