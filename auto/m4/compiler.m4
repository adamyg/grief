dnl $Id: compiler.m4,v 1.3 2024/05/02 14:34:31 cvsuser Exp $
dnl misc compiler check
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl

#   Localised AC_CANONICAL_HOST, with host specific functionality
#
AC_DEFUN([CF_CANONICAL_HOST],[
	AC_CANONICAL_HOST

	case $host_os in
	mingw*)
		AC_CHECK_HEADERS([windows.h winsock2.h])
		AC_CHECK_HEADERS([wincrypt.h winioctl.h],[],[],[
#ifdef HAVE_WINDOWS_H
#include <windows.h>	//rereq
#endif])
		RC=windres
		AC_SUBST([RC])
		;;
	msys*)
		# See: http://www.mingw.org/wiki/IncludePathHOWTO
		#
		# MSYS installation directory maps to root (/) or (/usr) add $MSYS/include
		# and $MSYS/lib being the standard path for msys-xxx packages.
		#
		# see: mingw-get list, examples include libarchive and libmagic.
		#
		# Note, MSYS and Mingw packages connot be mixed as a result of which
		# runtime is utilised.
		#
		CFLAGS="$CFLAGS -I/usr/include"
		CXXFLAGS="$CXXFLAGS -I/usr/include"
		LDFLAGS="$LDFLAGS -L/usr/lib"
		;;
	esac
])


#   Test if the compiler supports -Wno-unused-result.
#
#   Newer ubuntu's turn on -D_FORTIFY_SOURCE=2, enabling __attribute__((warn_unused_result))
#   for things like write(), which we don't care about.
#
AC_DEFUN([CF_WARNING_UNUSED_RESULT],[
	AC_CACHE_CHECK([if the compiler supports -Wno-unused-result],
		cf_cv_warning_no_unused_result,[
			cf_save_CFLAGS="$CFLAGS"
			CFLAGS="$CFLAGS -Wno-error -Wno-unused-result"
			# gcc doesn't warn about unknown flags unless it's
			# also warning for some other purpose, hence the
			# divide-by-0.  (We use -Wno-error to make sure the
			# divide-by-0 doesn't cause this test to fail!)
			AC_COMPILE_IFELSE([AC_LANG_PROGRAM(, return 1/0)],
				cf_cv_warning_no_unused_result=yes,
				cf_cv_warning_no_unused_result=no)
			CFLAGS="$cf_save_CFLAGS"])

	if test "x$cf_warning_no_unused_result" = "xyes" ; then
		AC_DEFINE([HAVE_WARNING_NO_UNUSED_RESULT], 1)
	fi
])


#   Test linker mapfile generation abilities
#
#
AC_DEFUN([CF_LDMAPFILE],[
	AC_SUBST(LDMAPFILE)
	AX_CHECK_LINK_FLAG([-Xlinker -Map=linker.map],[
		if test -f linker.map; then
			LDMAPFILE="-Xlinker -Map=\$(MAPFILE)"
			rm -f linker.map
		fi
		])
])


#   GCC compiler warnings
#
#
AC_DEFUN([CF_GCC_WARNINGS],[
	AX_CHECK_COMPILE_FLAG([-Wbounded], [CFLAGS="$CFLAGS -Wbounded"])
	AX_CHECK_COMPILE_FLAG([-Winit-self], [CFLAGS="$CFLAGS -Winit-self"])
	AX_CHECK_COMPILE_FLAG([-Wwrite-strings], [CFLAGS="$CFLAGS -Wwrite-strings"])
	AX_CHECK_COMPILE_FLAG([-Wdiv-by-zero], [CFLAGS="$CFLAGS -Wdiv-by-zero"])
	AX_CHECK_COMPILE_FLAG([-Wsometimes-uninitialized], [CFLAGS="$CFLAGS -Wsometimes-uninitialized"])

	AC_ARG_VAR([CWARN], [define to compilation flags for generating extra warnings])
	AX_CHECK_COMPILE_FLAG([-Wall], [CWARN="$CWARN -Wall"])
	AX_CHECK_COMPILE_FLAG([-Wextra], [CWARN="$CWARN -Wextra"])

	AX_CHECK_COMPILE_FLAG([-Wbad-function-cast], [CWARN="$CWARN -Wbad-function-cast"])
	AX_CHECK_COMPILE_FLAG([-Wcast-align], [CWARN="$CWARN -Wcast-align"])
	AX_CHECK_COMPILE_FLAG([-Wcast-qual], [CWARN="$CWARN -Wcast-qual"])
	AX_CHECK_COMPILE_FLAG([-Wchar-subscripts], [CWARN="$CWARN -Wchar-subscripts"])
	AX_CHECK_COMPILE_FLAG([-Wcomment], [CWARN="$CWARN -Wcomment"])
	AX_CHECK_COMPILE_FLAG([-Wfloat-equal], [CWARN="$CWARN -Wfloat-equal"])
	AX_CHECK_COMPILE_FLAG([-Wformat=2], [CWARN="$CWARN -Wformat=2"])
	AX_CHECK_COMPILE_FLAG([-Wimplicit], [CWARN="$CWARN -Wimplicit"])
	AX_CHECK_COMPILE_FLAG([-Wmissing-declarations], [CWARN="$CWARN -Wmissing-declarations"])
	AX_CHECK_COMPILE_FLAG([-Wmissing-prototypes], [CWARN="$CWARN -Wmissing-prototypes"])
	AX_CHECK_COMPILE_FLAG([-Wnormalized=id], [CWARN="$CWARN -Wnormalized=id"])
	AX_CHECK_COMPILE_FLAG([-Woverride-init], [CWARN="$CWARN -Woverride-init"])
	AX_CHECK_COMPILE_FLAG([-Wparentheses], [CWARN="$CWARN -Wparentheses"])
	AX_CHECK_COMPILE_FLAG([-Wpointer-arith], [CWARN="$CWARN -Wpointer-arith"])
	AX_CHECK_COMPILE_FLAG([-Wredundant-decls], [CWARN="$CWARN -Wredundant-decls"])
	AX_CHECK_COMPILE_FLAG([-Wstrict-prototypes], [CWARN="$CWARN -Wstrict-prototypes"])
	AX_CHECK_COMPILE_FLAG([-Wswitch-enum], [CWARN="$CWARN -Wswitch-enum"])
	AX_CHECK_COMPILE_FLAG([-Wvariable-decl], [CWARN="$CWARN -Wvariable-decl"])
])

dnl
