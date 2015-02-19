dnl $Id: libx11.m4,v 1.3 2013/04/05 19:52:10 ayoung Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     X11 build options
dnl

AC_DEFUN([CF_WITH_LIBX11],[
	AC_MSG_CHECKING(whether to enable X11)
	AC_ARG_WITH(magic,
		[  --with-x11              enable libx11 (default=yes)],
		[case "$withval" in
			yes)  cf_with_libx11=yes;;
			no)   cf_with_libx11=no;;
			*)    AC_MSG_ERROR(bad value ${withval} for --with-libx11);;
		esac],
		[cf_with_libx11=yes])
	AC_MSG_RESULT($cf_with_libx11)

	AC_SUBST(LIBX11)
	LIBX11=""

	if test "$cf_with_libx11" = yes; then
		CF_CHECK_LIBX11
		CF_CHECK_LIBXSHAPE
	fi
])


dnl ---------------------------------------------------------------------------
dnl libX11 and friends
dnl
dnl     CF_WITH_LIBX11 implementation
dnl
dnl         CF_CHECK_LIBX11         Primary X11 funcionality
dnl         CF_CHECK_LIBXSHAPE      libXshape extension
dnl

AC_DEFUN([CF_CHECK_LIBX11],[

	# libX11 implementation check
	AC_CHECK_HEADERS([X11/Xlib.h],[
		AC_CHECK_LIB([X11], XCreateWindow,[
			cf_have_libx11=yes
			AC_DEFINE([HAVE_LIBX11], 1)
			LIBX11="-lX11"
		])
	])

	# libXft option
	if test "x$cf_have_libx11" = xyes; then
		AC_MSG_CHECKING(whether to enable Xft libraries)
		AC_ARG_WITH(xft,
			[  --with-xft              enable Xft (default=yes)],
			[case "$withval" in
				yes)  cf_with_libxft=yes;;
				no)   cf_with_libxft=no;;
				*)    AC_MSG_ERROR(bad value ${withval} for --with-libxft);;
			esac],[cf_with_libxft=yes])
		AC_MSG_RESULT($cf_with_libxft)
	fi

	# libXft implementation check
	if test "x$cf_with_libxft" = xyes; then
		AC_CHECK_PROG(cf_freetype_config, freetype-config, yes, no)

		cf_save_CPPFLAGS="$CPPFLAGS"
		if test x"$cf_freetype_config" = xyes; then
			# assumption libfreetype is underlying
			cf_add_cppflags="`freetype-config --cflags`"
		elif test -d "/usr/X11R6/include/freetype2"; then
			cf_add_cppflags="-I/usr/X11R6/include/freetype2"
		elif test -d "/usr/include/freetype2"; then
			cf_add_cppflags="-I/usr/include/freetype2"
		fi

		CPPFLAGS="$CPPFLAGS $cf_add_cppflags"
		AC_CHECK_HEADERS([X11/Xft/Xft.h],[
			AC_DEFINE([HAVE_XFT_XFT_H], 1)
			AC_CHECK_LIB([Xft], XftFontOpenPattern, [
				AC_DEFINE([HAVE_LIBXFT], 1)
				LIBX11="$LIBX11 -lXft"]
			)],[
			AC_MSG_RESULT([Xft.h include error, freetype-dev package maybe required])
			CPPFLAGS="$cf_save_CPPFLAGS"],[
#if defined(HAVE_X11_XLIB_H)
#include <X11/Xlib.h>			/*prereq on Solaris*/
#include <X11/Xutil.h>
#endif
#include <X11/Xft/Xft.h>
		])
	fi

	# XkbBell option
	if test "x$cf_have_libx11" = xyes; then
		AC_MSG_CHECKING(whether to enable Xkb extensions)
		AC_ARG_WITH(xkb,
			[  --with-xkb              enable Xkb extensions (default=yes)],
			[case "$withval" in
				yes)  cf_with_xkbbell=yes;;
				no)   cf_with_xkbbell=no;;
				*)    AC_MSG_ERROR(bad value ${withval} for --with-xkb);;
			esac],[cf_with_xkbbell=yes])
		AC_MSG_RESULT($cf_with_xkbbell)
	fi

	# Xext implementation check
	if test x"$cf_with_xkbbell" = xyes; then
		AC_CHECK_HEADERS([X11/XKBlib.h],[
			AC_DEFINE([HAVE_XFT_XLBLIB_H], 1)
		cf_save_LIBS=$LIBS
		LIBS="$LIBS -lX11"
			AC_CHECK_LIB([xkbfile], [XkbBell],[
			AC_TRY_LINK([
#include <X11/Xlib.h>
#include <X11/XKBlib.h>],
			[Display *d;
			Window w;
			XkbBell(d, w, 0, 0);],
				[cf_cv_xkbbell=yes
				 AC_DEFINE([HAVE_XKBBELL], 1)
				 LIBX11="$LIBX11 -lxkbfile"],
						[cf_cv_xkbbell=no
				])
			])
			LIBS=$cf_save_LIBS],[],[
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
		])
		LIBS=$cf_save_LIBS
	fi
])


AC_DEFUN([CF_CHECK_LIBXSHAPE],[
	AC_REQUIRE([CF_CHECK_LIBX11])

	if test "x$cf_have_libx11" = xyes; then
		AC_MSG_CHECKING(whether to enable Xext extensions)
		AC_ARG_WITH(xext,
			[  --with-xshape           enable XShape (default=yes)],
			[case "$withval" in
				yes)  cf_with_libxext=yes;;
				no)   cf_with_libxext=no;;
				*)    AC_MSG_ERROR(bad value ${withval} for --with-libxext);;
			esac],[cf_with_libxext=yes])
		AC_MSG_RESULT($cf_with_libxext)

	fi	#HAVE_LIBX11

	# Xext implementation check
	if test x"$cf_with_libxext" = xyes; then
		AC_CHECK_LIB([Xext], [XShapeCombineShape], [
			AC_MSG_CHECKING([for X11/extensions/shape.h])
			AC_TRY_LINK([
#if defined(HAVE_X11_XLIB_H)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif
#include <X11/extensions/shape.h>],
			[long foo = ShapeSet;],
				[cf_cv_libxext=yes
				 AC_DEFINE([HAVE_X11_EXTENSIONS_SHAPE_H], 1)
				 AC_DEFINE([HAVE_LIBXEXT], 1)
				 LIBX11="$LIBX11 -lXext"],
				[cf_cv_libxext=no])
			AC_MSG_RESULT($cf_cv_libxext)
		])
	fi
])

dnl end
