dnl $Id: libterm.m4,v 1.25 2024/07/18 19:11:04 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl     Terminal support library checks
dnl
dnl  Usage:
dnl
dnl     #if defined(HAVE_LIBNCURSESW)
dnl     #elif defined(HAVE_LIBNCURSES)
dnl     #elif defined(HAVE_LIBCURSES)
dnl     #endif
dnl
dnl     #if defined HAVE_NCURSESW_CURSES_H
dnl     #  include <ncursesw/curses.h>
dnl     #  include <ncursesw/termcap.h>
dnl     #  include <ncursesw/term.h>
dnl     #elif defined HAVE_NCURSESW_H
dnl     #  include <ncursesw.h>
dnl     #  if defined(HAVE_TERMCAP_H)
dnl     #      include <termcap.h>
dnl     #  endif
dnl     #  if defined(HAVE_TERM_H)
dnl     #      include <term.h>
dnl     #  endif
dnl     #elif defined HAVE_NCURSES_CURSES_H
dnl     #  include <ncurses/curses.h>
dnl     #  include <ncurses/termcap.h>
dnl     #  include <ncurses/term.h>
dnl     #elif defined HAVE_NCURSES_H
dnl     #  include <ncurses.h>
dnl     #  if defined(HAVE_TERMCAP_H)
dnl     #      include <termcap.h>
dnl     #  endif
dnl     #  if defined(HAVE_TERM_H)
dnl     #      include <term.h>
dnl     #  endif
dnl     #else
dnl     #  error "missing ncurses" ..
dnl     #endif
dnl
dnl	CURSES_CFLAGS
dnl	CURSES_LDFLAGS
dnl	TERMLIB
dnl

AC_DEFUN([CF_LIBTERM_CHECK_TERMINFO],[
	AC_MSG_CHECKING([for terminfo setupterm(), tigetxxx() and tparm()])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[
extern int setupterm(char *, int, int *);
extern int tigetflag(char *);
extern int tigetnum(char *);
extern char *tigetstr(char *);
extern char *tparm(const char *str, ...);
]],[[
	int err = 0;
	setupterm((char *)"nonexistentterminal",1,(int *)&err);
	tigetflag((char *)"flg");
	tigetnum((char *)"num");
	tigetstr((char *)"str");
	tparm("%p1%d", 1, (void *)0);
]])],
		[cf_result=yes],[cf_result=no])
])dnl


AC_DEFUN([CF_LIBTERM_CHECK_TERMCAP],[
	AC_MSG_CHECKING([for termcap tgetxxx() and tgoto()])
	AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_CURSES_H
#include <curses.h>
#endif
#ifdef HAVE_TERMCAP_H
#include <termcap.h>
#endif
#ifdef HAVE_TERM_H
#include <term.h>
#endif
]],[[
	char buffer[1024 * 2];
	char *str = (char *)0;
	tgetent(buffer, "nonexistentterminal");
	tgetflag((char *)"FF");
	tgetnum((char *)"NN");
	tgetstr((char *)"SS", &str);
	tgoto("%p1%d", 0, 1);
]])],
		[cf_result=yes],[cf_result=no])
])dnl


AC_DEFUN([LIBTERM_CHECK_CONFIG],[

	AC_MSG_RESULT([determining term lib, --with-termlib options [ncursesw, ncurses, tinfo, curses, termcap, termlib]])
	AC_ARG_VAR([CURSES_CFLAGS], [preprocessor flags for Curses, e.g. -I/usr/include/ncursesw])
	AC_ARG_VAR([CURSES_LDFLAGS], [linker flags for Curses, e.g. -L/usr/pkg/lib])
	AC_ARG_WITH(ncurses,
	    [  --with-ncurses          use ncurses library], with_termlib=ncurses)

	AC_ARG_WITH(ncursesw,
	    [  --with-ncursesw         use ncursesw library], with_termlib=ncursesw)

	AC_ARG_WITH(tinfo,
	    [  --with-tinfo            use tinfo library], with_termlib=tinfo)

	AC_ARG_WITH(curses,
	    [  --with-curses           use curses library],  with_termlib=curses)

	AC_ARG_WITH(termcap,
	    [  --with-termcap          use termcap library], with_termlib=termcap)

	AC_ARG_WITH(termlib,
	    [  --with-termlib=library  use names library for terminal support],)

	AC_SUBST(CURSES_CFLAGS)
	cf_save_CFLAGS="$CFLAGS"

	AC_SUBST(TERMLIB)
	cf_save_LIBS="$LIBS"
	cf_libterm_name=""
	cf_libterm_cv_terminfo=no
	cf_libterm_cv_termcap=no
	cf_result=""

	dnl
	dnl basic headers
	dnl
	AC_CHECK_TOOL([PKG_CONFIG], [pkg-config])
	AC_CHECK_HEADERS(curses.h)
	AC_CHECK_HEADERS(termcap.h)
	AC_CHECK_HEADERS(term.h, [], [], [
#if HAVE_CURSES_H
#include <curses.h>
#endif
#if HAVE_TERMCAP_H
#include <termcap.h>
#endif
#include <term.h>
])

	dnl
	dnl library selection
	dnl
	CFLAGS="$cf_saved_CFLAGS $CURSES_CFLAGS"
	if test -n "$with_termlib"; then
		AC_MSG_CHECKING([for linking with $with_termlib library])
		TERMLIB="-l${cf_libterm_name}"
		LIBS="$cf_save_LIBS $TERMLIB"
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])],
			AC_MSG_RESULT(yes), AC_MSG_ERROR(no))
		AC_MSG_RESULT([using terminal library... $TERMLIB])
		cf_libterm_name=$with_termlib

	else
		AC_MSG_RESULT([no termlib options, checking for suitable terminal library])
		CFLAGS="$cf_saved_CFLAGS $CURSES_CFLAGS"

		dnl Selection rules/
		dnl
		dnl     o newer versions of ncursesw/ncurses are preferred over anything else,
		dnl        Note: older versions of ncurses have bugs hence we assume the latest (5.5 +).
		dnl     o smaller ncurses tinfo library.
		dnl     o otherwise termlib/curses.
		dnl
		case "`uname -s 2>/dev/null`" in
			OSF1|SCO_SV)	termlibs="ncursesw ncurses tinfo curses termlib termcap";;
			*)		termlibs="ncursesw ncurses tinfo termlib curses termcap";;
		esac

		TERMLIB=""
		cf_check_LIBS="$cf_save_LIBS"
		if test -n "$CURSES_LDFLAGS"; then
			CF_APPEND_TEXT(cf_check_LIBS,$CURSES_LDFLAGS)
		fi

		for libname in $termlibs; do

			LIBS="$cf_check_LIBS"

			if test "$libname" = "ncursesw" || test "$libname" = "ncurses"; then
				dnl
				dnl libncurses[w]
				dnl
				AC_CHECK_LIB($libname, setupterm)
				if test "x$cf_check_LIBS" = "x$LIBS"; then
					if test -z "$CURSES_LDFLAGS" && test -n "$PKG_CONFIG"; then
						AC_MSG_CHECKING([whether pkg-config information available])
						cf_pkg_config=`$PKG_CONFIG $libname --libs-only-L --libs-only-other 2>/dev/null`
						if test $? = 0 && test -n "$cf_pkg_config"; then
							AC_MSG_RESULT([$cf_pkg_config])
							CF_APPEND_TEXT(LIBS,$cf_pkg_config)
							AS_UNSET(ac_cv_lib_${libname}_setupterm)
							AC_CHECK_LIB($libname, setupterm, [], [LIBS="$cf_check_LIBS"])
							if test "x$cf_check_LIBS" != "x$LIBS"; then
								TERMLIB="$cf_pk_config"
							fi
						else
							AC_MSG_RESULT([none])
						fi
					fi
				fi

				if test "x$cf_check_LIBS" != "x$LIBS"; then
					CF_LIBTERM_CHECK_TERMINFO
					if test "$cf_result" = "yes"; then
						AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <stdio.h>
extern const char *curses_version(void);
]],[[
	/*routine specific to ncurses*/
	const char *v = curses_version();
	if (v) printf("%s... ", v);
	return (v == 0);
]])],
							[cf_result=yes],[cf_result=no])
					fi
					AC_MSG_RESULT($cf_result)
				fi
				LIBS="$cf_save_LIBS"
				if test "$cf_result" = "yes"; then
					cf_libterm_cv_terminfo=yes
					cf_libterm_name=$libname
					break
				fi

			else if test "$libname" = "tinfo"; then
				dnl
				dnl libtinfo
				dnl
				AC_CHECK_LIB($libname, setupterm)
				if test "x$cf_check_LIBS" != "x$LIBS"; then
					CF_LIBTERM_CHECK_TERMINFO
					AC_MSG_RESULT($cf_result)
				fi
				LIBS="$cf_save_LIBS"
				if test "$cf_result" = "yes"; then
					cf_libterm_cv_terminfo=yes
					cf_libterm_name=$libname
					break
				fi

			else
				dnl
				dnl curses/termcap/termlib
				dnl
				AC_CHECK_LIB($libname, tgetent)
				if test "x$cf_check_LIBS" = "x$LIBS" && test "$libname" = "curses"; then
					AC_MSG_CHECKING(if we need both curses and termcap libraries)
					LIBS="$cf_check_LIBS -lcurses -ltermcap"
					AC_LINK_IFELSE([AC_LANG_PROGRAM([[
extern int tgetent(char *, const char *);
]],[[
	char buffer[1024 * 2];
	tgetent(buffer, "nonexistentterminal");
]])],
						[cf_result=yes; TERMLIB="$CURSES_LDFLAGS -lcurses -ltermcap"; AC_MSG_RESULT(yes)],
						[cf_result=no; LIBS="$cf_check_LIBS"; AC_MSG_RESULT(no)])
					AC_MSG_RESULT([yes])
				fi
				if test "x$cf_check_LIBS" != "x$LIBS"; then
					CF_LIBTERM_CHECK_TERMCAP
					AC_MSG_RESULT($cf_result)
				fi
				LIBS="$cf_save_LIBS"
				if test "$cf_result" = "yes"; then
					cf_libterm_cv_termcap=yes
					cf_libterm_name=$libname
					break
				fi
			fi; fi

			AC_MSG_RESULT($libname library is not usable)
		done

		if test -n "$cf_libterm_name"; then
			if test -z "$TERMLIB"; then
				TERMLIB="$CURSES_LDFLAGS"
				CF_APPEND_TEXT(TERMLIB,"-l${cf_libterm_name}")
			fi
			LIBS="$LIBS $TERMLIB"
		else
			AC_MSG_RESULT(no terminal library found)
		fi
	fi

	if test -n "$cf_libterm_name"; then
		if test "x$cf_libterm_name" = "xtermlib" || test "x$cf_libterm_name" = "xyes"; then
			AC_MSG_NOTICE([using internal terminal interface library])

		else
			if test "$cf_libterm_cv_terminfo" != "yes"; then
				CF_LIBTERM_CHECK_TERMINFO
				AC_MSG_RESULT($cf_result)
				cf_libterm_cv_terminfo=$cf_result
			fi

			if test "$cf_libterm_cv_termcap" != "yes"; then
				CF_LIBTERM_CHECK_TERMCAP
				AC_MSG_RESULT($cf_result)
				cf_libterm_cv_termcap=$cf_result
			fi

			if test "$cf_libterm_cv_terminfo" != "yes"; then
				if test "$cf_libterm_cv_termcap" != "yes"; then
					AC_MSG_ERROR([
    You need to install a suitable terminal library; for example ncurses.
    alternatively specify the name of the library with --with-termlib=<lib>.])
				fi
			fi
		fi
		AC_MSG_RESULT([using terminal library... $TERMLIB])
	fi

	dnl
	dnl library features
	dnl
	if test "$cf_libterm_cv_terminfo" = "no"; then
		AC_CACHE_CHECK([whether we talk terminfo], cf_libterm_cv_terminfo, [
			AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_TERMCAP_H
# include <termcap.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#endif]],[[
	char *s = (char *)tgoto("%p1%d", 0, 1);
	return (0 == strcmp(s == 0 ? "" : s, "1"));
]])],
		[cf_libterm_cv_terminfo=no],
		[cf_libterm_cv_terminfo=yes],
			[AC_MSG_ERROR(cross-compiling: please set 'cf_libterm_cv_terminfo')])])
	fi

	dnl
	dnl library specific resources
	dnl
	dnl  ncursesw
	dnl  ncurses / ncurses_g (debug ncurses, diagnostic to trace)
	dnl  tinfo(*)
	dnl  curses
	dnl  termcap(*)
	dnl  termlib(*)
	dnl
	dnl  (*) should in theory work, yet not generally tested.
	dnl
	if test "x$cf_libterm_cv_terminfo" = "xyes"; then
		AC_DEFINE([HAVE_TERMINFO], 1, [terminfo interface.])
	fi
	if test "x$cf_libterm_cv_termcap" = "xyes"; then
		AC_DEFINE([HAVE_TERMCAP], 1, [termcap interface.])
	fi

	cf_libterm_ncurses_headers=
	if test "x$cf_libterm_name" = "xncursesw"; then
		AC_DEFINE([HAVE_LIBNCURSESW], 1, [enable libncursesw support.])
		AC_CHECK_LIB(ncursesw, main)
		AC_CHECK_LIB(ncursesw_g, main)
		cf_libterm_ncurses_headers=ncursesw

	elif test "x$cf_libterm_name" = "xncurses"; then
		AC_DEFINE([HAVE_LIBNCURSES], 1, [enable libncurses support.])
		AC_CHECK_LIB(ncurses, main)
		AC_CHECK_LIB(ncurses_g, main)
		cf_libterm_ncurses_headers=ncurses
	fi

	dnl additional search directories
	dnl main plus base, without trailing ncurses/w package name

	cf_libterm_includes="/usr/local/include"
	if test -z "$CURSES_CFLAGS" && test -n "$PKG_CONFIG"; then
		AC_MSG_CHECKING([whether pkg-config information available])
		cf_pkg_config=`$PKG_CONFIG $cf_libterm_name --cflags-only-I 2>/dev/null`
		if test $? = 0 && test -n "$cf_pkg_config"; then
			AC_MSG_RESULT([$cf_pkg_config])
			cf_libterm_includes=""
			for cf_config in $cf_pkg_config; do

				cf_include=${cf_config#-I}
				if test "$cf_include" = "$cf_config"; then
					continue
				fi

				cf_result=yes
				for cf_config in $cf_libterm_includes; do
					if test $cf_config = $cf_include; then
						cf_result=no
						break
					fi
				done
				if test $cf_result = yes; then
					CF_APPEND_TEXT(cf_libterm_includes,$cf_include)
				fi
			done
		else
			AC_MSG_RESULT([none])
		fi
	fi

	dnl package headers
	if test -n "$cf_libterm_ncurses_headers"; then

		dnl Newer versions of ncurses only publish ncurses.h supporting both char and wchar_t interfaces,
		dnl yet dependent on packaging/host the following may exist.
		dnl
		dnl    ncursesw/curses.h
		dnl    ncursesw.h
		dnl    ncurses/curses.h
		dnl    ncurses.h
		dnl
		dnl Note: Allow alternative installation under "/usr/local/include"

		cf_have_ncurses_h=no
		if test "x$cf_libterm_ncurses_headers" = "xncursesw"; then
			dnl
			dnl ncursesw/curses.h
			dnl
			AC_CHECK_HEADERS(ncursesw/curses.h, [cf_have_ncurses_h=yesa], [cf_have_ncurses_h=no])

			if test "x$cf_have_ncurses_h" = "xno" && test -z "$CURSES_CFLAGS" ; then
				AC_MSG_NOTICE([checking secondary ncurses directories])
				for cf_include in $cf_libterm_includes; do
					if test "$cf_include" != "${cf_include%/ncurses*}"; then
						continue
					fi
					CFLAGS="$cf_saved_CFLAGS -I$cf_include"
					AS_UNSET(ac_cv_header_ncursesw_curses_h)
					AC_CHECK_HEADER(ncursesw/curses.h, [cf_have_ncurses_h=yesa], [cf_have_ncurses_h=no])
					if test "x$cf_have_ncurses_h" = "xyesa"; then
						CURSES_CFLAGS="-I$cf_include"
						break
					fi
					CFLAGS="$cf_saved_CFLAGS"
				done
			fi

			if test "x$cf_have_ncurses_h" = "xyesa"; then
				AC_CHECK_HEADERS(ncursesw/nc_alloc.h, [have_nc_alloc_h])
				AC_CHECK_HEADERS(ncursesw/nomacros.h, [have_nomacros_h])
				AC_CHECK_HEADERS(ncursesw/termcap.h)
				AC_CHECK_HEADERS(ncursesw/term.h)
			fi

			dnl
			dnl ncursesw.h
			dnl
			if test "x$cf_have_ncurses_h" = "xno"; then
				AC_CHECK_HEADERS(ncursesw.h, [cf_have_ncurses_h=yesb], [cf_have_ncurses_h=no])
				if test "x$cf_have_ncurses_h" = "xno" && test -z "$CURSES_CFLAGS" ; then
					AC_MSG_NOTICE([checking secondary ncurses directories])
					for cf_include in $cf_libterm_includes; do
						CFLAGS="$cf_saved_CFLAGS -I$cf_include"
						AS_UNSET(ac_cv_header_ncursesw_h)
						AC_CHECK_HEADERS(ncursesw.h, [cf_have_ncurses_h=yesb], [cf_have_ncurses_h=no])
						if test "x$cf_have_ncurses_h" = "xyesb"; then
							CURSES_CFLAGS="-I$cf_include"
							break
						fi
						CFLAGS="$cf_saved_CFLAGS"
					done
				fi
			fi

			if test "x$cf_have_ncurses_h" = "xno"; then
				AC_MSG_RESULT([checking for common ncurses header])
				cf_libterm_ncurses_headers=ncurses
			fi
		fi

		if test "x$cf_libterm_ncurses_headers" = "xncurses"; then

			dnl
			dnl ncurses/curses.h
			dnl
			AC_CHECK_HEADERS(ncurses/curses.h, [cf_have_ncurses_h=yesc], [cf_have_ncurses_h=no])

			if test "x$cf_have_ncurses_h" = "xno" && test -z "$CURSES_CFLAGS" ; then
				AC_MSG_RESULT([checking secondary ncurses directories])
				for cf_include in $cf_libterm_includes; do
					if test "$cf_include" != "${cf_include%/ncurses*}"; then
						continue
					fi
					CFLAGS="$cf_saved_CFLAGS -I$cf_include"
					AS_UNSET(ac_cv_header_ncurses_curses_h)
					AC_CHECK_HEADERS(ncurses/curses.h, [cf_have_ncurses_h=yesc], [cf_have_ncurses_h=no])
					if test "x$cf_have_ncurses_h" = "xyesc"; then
						CURSES_CFLAGS="-I$cf_include"
						break
					fi
					CFLAGS="$cf_saved_CFLAGS"
				done
			fi

			if test "x$cf_have_ncurses_h" = "xyesc" ; then
				AC_CHECK_HEADERS(ncurses/nc_alloc.h, [have_nc_alloc_h])
				AC_CHECK_HEADERS(ncurses/nomacros.h, [have_nomacros_h])
				AC_CHECK_HEADERS(ncurses/termcap.h)
				AC_CHECK_HEADERS(ncurses/term.h)
			fi

			dnl
			dnl ncurses.h
			dnl
			AC_CHECK_HEADERS(ncurses.h, [cf_have_ncurses_h=yesd], [cf_have_ncurses_h=no])

			if test "x$cf_have_ncurses_h" = "xno" && test -z "$CURSES_CFLAGS" ; then
				AC_MSG_NOTICE([checking secondary ncurse directories])
				for cf_include in $cf_libterm_includes; do
					CFLAGS="$cf_saved_CFLAGS -I$cf_include"
					AS_UNSET(ac_cv_header_ncurses_h)
					AC_CHECK_HEADERS(ncurses.h, [cf_have_ncurses_h=yesd], [cf_have_ncurses_h=no])
					if test "x$cf_have_ncurses_h" = "xyesd"; then
						CURSES_CFLAGS="-I$cf_include"
						break
					fi
					CFLAGS="$cf_saved_CFLAGS"
				done
			fi
		fi

		if test "x$cf_have_ncurses_h" = "xno" ; then
			if test "x$cf_libterm_name" = "xncursesw"; then
				AC_MSG_WARN([could not find ncursesw/curses.h, ncursesw.h nor ncurses/curses.h or ncurses.h])
			else
				AC_MSG_WARN([could not find ncurses/curses.h or ncurses.h])
			fi
		else
			if test "x$have_nc_alloc_h" = "x"; then
				AC_CHECK_HEADERS(nc_alloc.h)
			fi
			if test "x$have_nomacros_h" = "x"; then
				AC_CHECK_HEADERS(nomacros.h)
			fi
		fi

	else if test "x$cf_libterm_name" = "xtinfo"; then
		AC_DEFINE([HAVE_LIBTINFO], 1, [enable libtinfo support.])
		AC_CHECK_HEADERS(ncurses/termcap.h ncurses/term.h)
		AC_CHECK_LIB(tinfo, main)

	else if test "x$cf_libterm_name" = "xcurses"; then
		AC_DEFINE([HAVE_LIBCURSES], 1, [enable libcurses support.])
		AC_CHECK_HEADERS(curses.h)
		AC_CHECK_LIB(curses, main)

	else if test "x$cf_libterm_name" = "xtermcap"; then
		AC_DEFINE([HAVE_LIBTERMCAP], 1, [enable libtermcap support.])
		AC_CHECK_HEADERS(term.h termcap.h)

	else if test "x$cf_libterm_name" = "xtermlib"; then
		AC_DEFINE([HAVE_LIBTERMLIB], 1, [enable libtermlib support.])
		AC_CHECK_HEADERS(edtermcap.h)

	fi; fi; fi; fi; fi

	if test "$cf_libterm_cv_termcap" = "yes"; then
		if test -n "$cf_libterm_name"; then
			AC_CACHE_CHECK([what tgetent() returns for an unknown terminal], cf_libterm_cv_tgent, [
				AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_TERMCAP_H
#include <termcap.h>
#endif
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
]],[[
	char buffer[1024 * 2];
	int res = tgetent(buffer, "nonexistentterminal");
	exit(res != 0);
]])],
					[cf_libterm_cv_tgent=zero],
					[cf_libterm_cv_tgent=non-zero],
				[AC_MSG_ERROR(failed to compile test program.)])
				])

			if test "x$cf_libterm_cv_tgent" = "xzero"; then
				AC_DEFINE([TGETENT_ZERO_ERR], 0, [tgetent() return code.])
			fi
		fi
	fi

	AC_MSG_CHECKING(whether termcap.h contains ospeed)
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_TERMCAP_H
#include <termcap.h>
#endif]], [[ospeed = 20000;]])],
		AC_MSG_RESULT(yes); AC_DEFINE([HAVE_OSPEED], 1, [extern ospeed available.]),
		[AC_MSG_RESULT(no)
		AC_MSG_CHECKING(whether ospeed can be extern)
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_TERMCAP_H
#include <termcap.h>
#endif
extern short ospeed;]], [[ospeed = 20000;]])],
			AC_MSG_RESULT(yes); AC_DEFINE([OSPEED_EXTERN], 1, [extern ospeed needed.]),
			AC_MSG_RESULT(no))]
		)

	AC_MSG_CHECKING([whether termcap.h contains UP, BC and PC])
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_TERMCAP_H
#include <termcap.h>
#endif]], [[if (UP == 0 && BC == 0) PC = 1]])],
		AC_MSG_RESULT(yes); AC_DEFINE([HAVE_UP_BC_PC], 1, [extern UP BC and PC available.]),
		[AC_MSG_RESULT(no)
			AC_MSG_CHECKING([whether UP, BC and PC can be extern])
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_TERMCAP_H
#include <termcap.h>
#endif
extern char *UP, *BC, PC;]], [[if (UP == 0 && BC == 0) PC = 1;]])],
			AC_MSG_RESULT(yes); AC_DEFINE([UP_BC_PC_EXTERN], 1, [extern UP BC and PC needed.]),
			AC_MSG_RESULT(no))]
		)

	AC_MSG_CHECKING(whether tputs() uses outfuntype)
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_TERMCAP_H
#include <termcap.h>
#endif]], [[extern int xx(); tputs("test", 1, (outfuntype)xx)]])],
			[AC_MSG_RESULT(yes); AC_DEFINE([HAVE_OUTFUNTYPE], 1, [typedef outfuntype available.])],
			[AC_MSG_RESULT(no); AC_MSG_CHECKING(determining tputs() function final argument type)
			AC_EGREP_CPP([tputs.*[(][ \\\t]*char[ \\\t]*[)]],[
#ifdef HAVE_TERM_H
#include <term.h>
#endif
#ifdef HAVE_CURSES_H
#include <curses.h>
#endif
			], [AC_MSG_RESULT(char); AC_DEFINE([TPUTS_TAKES_CHAR], 1, [tputs character interface.])],
				[AC_MSG_RESULT(not char, int assumed);
				])
		])

	CFLAGS=$cf_save_CFLAGS
	LIBS=$cf_save_LIBS
])dnl
