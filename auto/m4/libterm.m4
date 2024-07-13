dnl $Id: libterm.m4,v 1.12 2024/07/13 17:17:17 cvsuser Exp $
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

AC_DEFUN([LIBTERM_CHECK_CONFIG],[

	AC_MSG_RESULT([determining term lib, --with-termlib options [ncursesw, ncurses, pdcurses, tinfo, curses, termcap, termlib]])
	AC_ARG_VAR([CURSES_CFLAGS], [preprocessor flags for Curses, e.g. -I/usr/include/ncursesw])
	AC_ARG_WITH(ncurses,
	    [  --with-ncurses          use ncurses library], with_termlib=ncurses)

	AC_ARG_WITH(ncursesw,
	    [  --with-ncursesw         use ncursesw library], with_termlib=ncursesw)

	AC_ARG_WITH(pdcurses,
	    [  --with-pdcurses         use pdcurses library], with_termlib=pdcurses)

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
	if test -n "$CURSES_CFLAGS"; then
		dnl TODO: pkg-check
		CFLAGS="$cf_saved_CFLAGS $CURSES_CFLAGS"
	fi

	AC_SUBST(TERMLIB)
	cf_save_LIBS="$LIBS"
	termlib_name=""
	termlib_cv_terminfo=no
	termlib_cv_termcap=no

	dnl basic headers
	dnl
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

	dnl library selection
	dnl
	if test -n "$with_termlib"; then
		AC_MSG_RESULT([termcap library ... $with_termlib])
		termlib_name=$with_termlib
		AC_MSG_CHECKING(for linking with $with_termlib library)
		AC_LINK_IFELSE([AC_LANG_PROGRAM([[]], [[]])], AC_MSG_RESULT(OK), AC_MSG_ERROR(FAILED))

	else
		AC_MSG_RESULT([no termlib options, checking for suitable terminal library])

		dnl Selection rules/
		dnl
		dnl     o Newer versions of ncursesw/ncurses are preferred over anything,
		dnl          Note: older versions of ncurses have bugs hence we assume the latest (5.5 +).
		dnl     o also allow the smaller ncurses tinfo library
		dnl     o Digital Unix (OSF1) should use curses.
		dnl     o SCO Openserver prefer termlib.
		dnl
		case "`uname -s 2>/dev/null`" in
			OSF1|SCO_SV)	termlibs="ncursesw ncurses tinfo curses termlib termcap";;
			*)		termlibs="ncursesw ncurses tinfo termlib termcap curses";;
		esac

		for libname in $termlibs; do
			AC_CHECK_LIB(${libname}, tgetent,,)
			if test "x$cf_save_LIBS" != "x$LIBS"; then
				dnl It's possible that a library is found but it doesn't work, for example
				dnl shared library that cannot be found; compile and run a test program to be sure
				dnl
				AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_TERMCAP_H
# include <termcap.h>
#endif
#ifdef HAVE_TERM_H
# include <term.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#endif]],[[
	char *s; s=(char *)tgoto("%p1%d", 0, 1);
	return 0;]])],
					[res="OK"], [res="FAIL"], [res="FAIL"])
				LIBS="$cf_save_LIBS"
				if test "$res" = "OK"; then
					termlib_name=$libname
					break
				fi
				AC_MSG_RESULT($libname library is not usable)
			fi
		done

		if test -z "$termlib_name"; then
			AC_MSG_RESULT(no terminal library found)
		fi
	fi

	if test -n "$termlib_name"; then
		if test "x$termlib_name" = "xtermlib"; then
			AC_MSG_NOTICE([using internal terminal interface library])

		elif test "x$termlib_name" = "xyes"; then
			AC_MSG_NOTICE([using internal terminal interface library])

		else
			TERMLIB="-l$termlib_name"
			LIBS="$LIBS $TERMLIB"

			AC_MSG_CHECKING([for setupterm and tigetxxx()])
			AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_CURSES_H
#include <curses.h>
#endif
#ifdef HAVE_TERM_H
#include <term.h>
#endif]],
				[[setupterm("terminalwontexist",0,0); tigetstr("str"); tigetnum("num"); tigetflag("flag");]])],
				[termlib_cv_terminfo=yes; AC_MSG_RESULT(yes)],
				AC_MSG_RESULT(no))

			AC_MSG_CHECKING([for tgetent() and tgetxxx()])
			AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_CURSES_H
#include <curses.h>
#endif
#ifdef HAVE_TERM_H
#include <term.h>
#endif]],
				[[char buffer[10000]; char *area = (char *)0;
int res = tgetent(buffer, "terminalwontexist"); tgetstr("str", &area); tgetnum("num"); tgetflag("flag");]])],
				[termlib_cv_termcap=yes; AC_MSG_RESULT(yes)],
				AC_MSG_RESULT(no))

			if test "$termlib_cv_terminfo" != "yes"; then
				if test "$termlib_cv_termcap" != "yes"; then
					AC_MSG_ERROR([
    You need to install a suitable terminal library; for example ncurses.
    alternatively specify the name of the library with --with-termlib=<lib>.])
				fi
			fi
		fi
	fi
	TERMLIB="-l$termlib_name"

	dnl library features
	dnl
	if test "$termlib_cv_terminfo" = "no"; then
		AC_CACHE_CHECK([whether we talk terminfo], termlib_cv_terminfo, [
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
    char *s; s=(char *)tgoto("%p1%d", 0, 1);
    exit(!strcmp(s==0 ? "" : s, "1"));
    return 0;
]])],
		[termlib_cv_terminfo=no],
		[termlib_cv_terminfo=yes],
			[AC_MSG_ERROR(cross-compiling: please set 'termlib_cv_terminfo')])])
	fi

	dnl library specific resources
	dnl
	dnl	ncursesw
	dnl	ncurses / ncurses_g
	dnl	pdcurses(*)
	dnl	tinfo(*)
	dnl	curses
	dnl	termcap
	dnl	termlib
	dnl
	dnl	(*) should in theory work, yet fully tested.
	dnl
	if test "x$termlib_cv_terminfo" = "xyes"; then
		AC_DEFINE([HAVE_TERMINFO], 1, [terminfo interface.])
	fi
	if test "x$termlib_cv_termcap" = "xyes"; then
		AC_DEFINE([HAVE_TERMCAP], 1, [termcap interface.])
	fi

	cf_need_ncurses_headers=
	if test "x$termlib_name" = "xncursesw"; then
		AC_DEFINE([HAVE_LIBNCURSESW], 1, [enable libncursesw support.])
		AC_CHECK_LIB(ncursesw, main)
		AC_CHECK_LIB(ncursesw_g, main)
		cf_need_ncurses_headers=ncursesw

	else if test "x$termlib_name" = "xncurses"; then
		AC_DEFINE([HAVE_LIBNCURSES], 1, [enable libncurses support.])
		AC_CHECK_LIB(ncurses, main)
		AC_CHECK_LIB(ncurses_g, main)
		cf_need_ncurses_headers=ncurses
	fi; fi

	dnl package headers
	if test -n "$cf_need_ncurses_headers"; then

		dnl Newer versions of ncurses only publish ncurses.h supporting both char and wchar_t interfaces,
		dnl yet dependent on packaging/host the following may exist.
		dnl
		dnl    ncursesw/curses.h
		dnl    ncursesw.h
		dnl    ncurses/curses.h
		dnl    ncurses.h
		dnl
		dnl Note: Allow alternative installation under "/usr/local/include"

		if test "x$cf_need_ncurses_headers" = "xncursesw"; then

			dnl ncursesw/curses.h
			AC_CHECK_HEADERS(ncursesw/curses.h, [cf_have_ncurses_h=yesa], [cf_have_ncurses_h=no])
			if test "x$cf_have_ncurses_h" = "xno" ; then
				if test "x$CURSES_CFLAGS" = "x" ; then
					CFLAGS="$cf_saved_CFLAGS -I/usr/local/include"
					$as_unset ac_cv_header_ncursesw_curses_h
					AC_MSG_NOTICE([using /usr/local/include])
					AC_CHECK_HEADERS(ncursesw/curses.h, [cf_have_ncurses_h=yesa], [cf_have_ncurses_h=no])
					if test x$have_ncursesw_h = xyesa ; then
						CURSES_CFLAGS="-I/usr/local/include"
					else
						CFLAGS="$cf_saved_CFLAGS"
					fi
				fi
			fi
			if test "x$cf_have_ncurses_h" = "xyesa" ; then
				AC_CHECK_HEADERS(ncursesw/nc_alloc.h, [have_nc_alloc_h])
				AC_CHECK_HEADERS(ncursesw/nomacros.h, [have_nomacros_h])
				AC_CHECK_HEADERS(ncursesw/termcap.h)
				AC_CHECK_HEADERS(ncursesw/term.h)
			fi

			dnl ncursesw.h
			if test "x$cf_have_ncurses_h" = "xno" ; then
				AC_CHECK_HEADERS(ncursesw.h, [cf_have_ncurses_h=yesb], [cf_have_ncurses_h=no])
				if test "x$cf_have_ncurses_h" = "xno" ; then
					if test "x$CURSES_CFLAGS" = "x" ; then
						CFLAGS="$cf_saved_CFLAGS -I/usr/local/include"
						$as_unset ac_cv_header_ncursesw_h
						AC_MSG_NOTICE([using /usr/local/include])
						AC_CHECK_HEADERS(ncursesw.h, [cf_have_ncurses_h=yesb], [cf_have_ncurses_h=no])
						if test x$have_ncursesw_h = xyesb ; then
							CURSES_CFLAGS="-I/usr/local/include"
						else
							CFLAGS="$cf_saved_CFLAGS"
						fi
					fi
				fi
			fi
			if test "x$cf_have_ncurses_h" = "xno" ; then
				cf_need_ncurses_headers=ncurses
			fi
		fi

		if test "x$cf_need_ncurses_headers" = "xncurses"; then

			dnl ncurses/curses.h
			AC_CHECK_HEADERS(ncurses/curses.h, [cf_have_ncurses_h=yesc], [cf_have_ncurses_h=no])
			if test "x$cf_have_ncurses_h" = "xno" ; then
				if test "x$CURSES_CFLAGS" = "x" ; then
					CFLAGS="$cf_saved_CFLAGS -I/usr/local/include"
					$as_unset ac_cv_header_ncurses_curses_h
					AC_MSG_NOTICE([using /usr/local/include])
					AC_CHECK_HEADERS(ncurses/curses.h, [cf_have_ncurses_h=yesc], [cf_have_ncurses_h=no])
					if test x$cf_have_ncurses_h = xyesc ; then
						CURSES_CFLAGS="-I/usr/local/include"
					else
						CFLAGS="$cf_saved_CFLAGS"
					fi
				fi
			fi
			if test "x$cf_have_ncurses_h" = "xyesc" ; then
				AC_CHECK_HEADERS(ncurses/nc_alloc.h, [have_nc_alloc_h])
				AC_CHECK_HEADERS(ncurses/nomacros.h, [have_nomacros_h])
				AC_CHECK_HEADERS(ncurses/termcap.h)
				AC_CHECK_HEADERS(ncurses/term.h)
			fi

			dnl ncurses.h
			AC_CHECK_HEADERS(ncurses.h, [cf_have_ncurses_h=yesd], [cf_have_ncurses_h=no])
			if test "x$cf_have_ncurses_h" = "xno" ; then
				if test "x$CURSES_CFLAGS" = "x" ; then
					CFLAGS="$cf_saved_CFLAGS -I/usr/local/include"
					$as_unset ac_cv_header_ncurses_h
					AC_MSG_NOTICE([using /usr/local/include])
					AC_CHECK_HEADERS(ncurses.h, [cf_have_ncurses_h=yesd], [cf_have_ncurses_h=no])
					if test x$cf_have_ncurses_h = xyesd ; then
						CURSES_CFLAGS="-I/usr/local/include"
					else
						CFLAGS="$cf_saved_CFLAGS"
					fi
				fi
			fi
		fi

		if test "x$cf_have_ncurses_h" = "xno" ; then
			if test "x$termlib_name" = "xncursesw"; then
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

	else if test "x$termlib_name" = "xpdcurses"; then
		AC_DEFINE([HAVE_LIBPDCURSES], 1, [enable libpdcurses support.])
		AC_CHECK_HEADERS(pdcurses.h pdterm.h curses.h term.h)
		AC_CHECK_LIB(pdcurses, main)

	else if test "x$termlib_name" = "xtinfo"; then
		AC_DEFINE([HAVE_LIBTINFO], 1, [enable libtinfo support.])
		AC_CHECK_HEADERS(ncurses/termcap.h ncurses/term.h)
		AC_CHECK_LIB(tinfo, main)

	else if test "x$termlib_name" = "xcurses"; then
		AC_DEFINE([HAVE_LIBCURSES], 1, [enable libcurses support.])
		AC_CHECK_HEADERS(curses.h)
		AC_CHECK_LIB(curses, main)

	else if test "x$termlib_name" = "xtermcap"; then
		AC_DEFINE([HAVE_LIBTERMCAP], 1, [enable libtermcap support.])
		AC_CHECK_HEADERS(term.h termcap.h)

	else if test "x$termlib_name" = "xtermlib"; then
		AC_DEFINE([HAVE_LIBTERMLIB], 1, [enable libtermlib support.])
		AC_CHECK_HEADERS(edtermcap.h)

	fi; fi; fi; fi; fi; fi

	if test "$termlib_cv_termcap" = "yes"; then
		if test -n "$termlib_name"; then
			AC_CACHE_CHECK([what tgetent() returns for an unknown terminal], termlib_cv_tgent, [
				AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_TERMCAP_H
#include <termcap.h>
#endif
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif]],[[
    char s[10000];
    int res = tgetent(s, "terminalwontexist");
    exit(res != 0);
    return 0;
]])],
			    [termlib_cv_tgent=zero],
			    [termlib_cv_tgent=non-zero],
				[AC_MSG_ERROR(failed to compile test program.)])
			    ])

			if test "x$termlib_cv_tgent" = "xzero"; then
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
