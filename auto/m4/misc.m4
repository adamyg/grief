dnl $Id: misc.m4,v 1.8 2020/06/03 16:59:01 cvsuser Exp $
dnl Process this file with autoconf to produce a configure script.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl ---------------------------------------------------------------------------
dnl Check if the file-system supports mixed-case filenames.  If we are able to
dnl create a lowercase name and see it as uppercase, it doesnt support that.
dnl

AC_DEFUN([CF_MIXEDCASE_FILENAMES],[
	AC_CACHE_CHECK(if filesystem supports mixed case filenames,cf_cv_mixedcase,[
		rm -f conftest CONFTEST
		echo test >conftest
		if test -f CONFTEST ; then
			cf_cv_mixedcase=no
		else
			cf_cv_mixedcase=yes
		fi
		rm -f conftest CONFTEST
	])
	test "$cf_cv_mixedcase" = yes && \
		AC_DEFINE([MIXEDCASE_FILENAMES], 1, [supports mixed case filenames.])
])dnl


dnl ---------------------------------------------------------------------------
dnl Provide a value for the $PATH and similar separator
dnl

AC_DEFUN([CF_PATHSEP],[
	case $cf_cv_system_name in
	os2*)	PATHSEP=';'  ;;
	*)	PATHSEP=':'  ;;
	esac
ifelse($1,,,[$1=$PATHSEP])
	AC_SUBST(PATHSEP)
])dnl


dnl ---------------------------------------------------------------------------
dnl Add checks for large file support.
dnl

AC_DEFUN([CF_LARGEFILE],[
	ifdef([AC_FUNC_FSEEKO],[
		AC_SYS_LARGEFILE
		if test "$enable_largefile" != no ; then
			AC_FUNC_FSEEKO

			# Normally we would collect these definitions in the config.h,
			# but (like _XOPEN_SOURCE), some environments rely on having these
			# defined before any of the system headers are included.  Another
			# case comes up with C++, e.g., on AIX the compiler compiles the
			# header files by themselves before looking at the body files it is
			# told to compile.  For ncurses, those header files do not include
			# the config.h
			test "$ac_cv_sys_large_files"      != no && CPPFLAGS="$CPPFLAGS -D_LARGE_FILES "
			test "$ac_cv_sys_largefile_source" != no && CPPFLAGS="$CPPFLAGS -D_LARGEFILE_SOURCE "
			test "$ac_cv_sys_file_offset_bits" != no && CPPFLAGS="$CPPFLAGS -D_FILE_OFFSET_BITS=$ac_cv_sys_file_offset_bits "

			AC_CACHE_CHECK(whether to use struct dirent64, cf_cv_struct_dirent64,[
				AC_TRY_COMPILE([
				#include <sys/types.h>
				#include <dirent.h>
				],[
				/* if transitional largefile support is setup, this is true */
				extern struct dirent64 * readdir(DIR *);
				struct dirent64 *x = readdir((DIR *)0);
				struct dirent *y = readdir((DIR *)0);
				int z = x - y;
				],
				[cf_cv_struct_dirent64=yes],
				[cf_cv_struct_dirent64=no])
			])
			test "$cf_cv_struct_dirent64" = yes && \
				AC_DEFINE([HAVE_STRUCT_DIRENT64], 1, [struct dirent64.])
		fi
	])
])dnl


dnl ---------------------------------------------------------------------------
dnl Check for a working mkstemp.  This creates two files, checks that they are
dnl successfully created and distinct (AmigaOS apparently fails on the last).

AC_DEFUN([CF_FUNC_MKSTEMP],[
	AC_CACHE_CHECK(for working mkstemp, cf_cv_func_mkstemp,[
	rm -f conftest*
	AC_TRY_RUN([
	#include <sys/types.h>
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include <sys/stat.h>
	int main()
	{
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
		if (result == 0
		&& !strcmp(name[0], name[1]))
			result = 1;
		exit(result);
	}],[cf_cv_func_mkstemp=yes],[cf_cv_func_mkstemp=no],
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
	AC_CACHE_CHECK([for nl_langinfo and CODESET], cf_cv_langinfo_codeset,
		[AC_TRY_LINK([#include <langinfo.h>],
		[char* cs = nl_langinfo(CODESET);],
		cf_cv_langinfo_codeset=yes,
		cf_cv_langinfo_codeset=no)
	])
	if test $cf_cv_langinfo_codeset = yes; then
		AC_DEFINE(HAVE_LANGINFO_CODESET, 1,
			[Define if you have <langinfo.h> and nl_langinfo(CODESET).])
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
		AC_TRY_LINK([#include <ctype.h>],[int x = isascii(' ')],
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
		AC_TRY_LINK([#include <ctype.h>],[int x = isblank(' ')],
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
		AC_TRY_LINK([#include <ctype.h>],[int x = iscsym(' ')],
		[cf_cv_have_iscsym=yes],
		[cf_cv_have_iscsym=no])
	])dnl
	AC_MSG_RESULT($cf_cv_have_iscsym)
	test "$cf_cv_have_iscsym" = yes && \
		AC_DEFINE([HAVE_ISCSYM], 1, [iscsym() available.])
+])dnl

dnl ---------------------------------------------------------------------------
dnl Check if the C compiler supports "inline".
dnl
dnl     CF_C_INLINE(<define>,<inline_argument>)
dnl

AC_DEFUN([CF_C_INLINE],[
	cf_c_inline_define=$1
	AC_C_INLINE
	if test "$ac_cv_c_inline" != no ; then
		$1=inline
		if test "$INTEL_COMPILER" = yes
		then
			:
		elif test "$GCC" = yes
		then
			AC_CACHE_CHECK(if gcc supports options to tune inlining,cf_cv_gcc_inline,[
			cf_save_CFLAGS=$CFLAGS
			CFLAGS="$CFLAGS --param max-inline-insns-single=$2"
			AC_TRY_COMPILE([inline int foo(void) { return 1; }],
				[${cf_cv_main_return:-return} foo()],
				[cf_cv_gcc_inline=yes],
				[cf_cv_gcc_inline=no])
			CFLAGS=$cf_save_CFLAGS
			])
			if test "$cf_cv_gcc_inline" = yes ; then
				CF_ADD_CFLAGS([--param max-inline-insns-single=$2])
			fi
		fi
	fi

	if test "$cf_c_inline_define" = yes ; then
		if test "$ac_cv_c_inline" != no ; then
			AC_DEFINE(HAVE_INLINE, 1, [Have inline keyword])
			AC_SUBST(HAVE_INLINE)
		else
			AC_TRY_COMPILE([__inline int foo(void) { return 1; }],
				[${cf_cv_main_return:-return} foo()],
				[cf_cv_c_inline=yes],
				[cf_cv_c_inline=no])
			if test "$ac_cv_c_inline" != no ; then
				AC_DEFINE(HAVE___INLINE, 1, [Have __inline keyword])
				AC_SUBST(HAVE___INLINE)
			fi
		fi
	fi
])dnl


dnl ---------------------------------------------------------------------------
dnl Check for memmove, or a bcopy that can handle overlapping copy.
dnl
dnl If neither is found define NEED_MEMMOVE
dnl

AC_DEFUN([CF_FUNC_MEMMOVE],[
	AC_CHECK_FUNC(memmove,,[
	AC_CHECK_FUNC(bcopy,[
		AC_CACHE_CHECK(if bcopy does overlapping moves,cf_cv_good_bcopy,[
			AC_TRY_RUN([
	int main() {
		static char data[] = "abcdefghijklmnopqrstuwwxyz";
		char temp[40];
		bcopy(data, temp, sizeof(data));
		bcopy(temp+10, temp, 15);
		bcopy(temp+5, temp+15, 10);
		${cf_cv_main_return:-return} (strcmp(temp, "klmnopqrstuwwxypqrstuwwxyz"));
	}
			],
			[cf_cv_good_bcopy=yes],
			[cf_cv_good_bcopy=no],
			[cf_cv_good_bcopy=unknown])
			])
		],[cf_cv_good_bcopy=no])
		if test "$cf_cv_good_bcopy" != yes ; then
			AC_DEFINE([NEED_MEMMOVE], 1, [need external memmove().])
		fi
	])
])dnl


dnl ---------------------------------------------------------------------------
dnl Check for existence of workable nanosleep() function.  Some systems, e.g.,
dnl AIX 4.x, provide a non-working version.
dnl

AC_DEFUN([CF_FUNC_NANOSLEEP],[
	AC_CACHE_CHECK(if nanosleep really works,cf_cv_func_nanosleep,[
	AC_TRY_RUN([
	#include <stdio.h>
	#include <errno.h>
	#include <time.h>

	#ifdef HAVE_SYS_TIME_H
	#include <sys/time.h>
	#endif

	int main() {
		struct timespec ts1, ts2;
		int code;
		ts1.tv_sec  = 0;
		ts1.tv_nsec = 750000000;
		ts2.tv_sec  = 0;
		ts2.tv_nsec = 0;
		errno = 0;
		code = nanosleep(&ts1, &ts2); /* on failure errno is ENOSYS. */
		${cf_cv_main_return:-return}(code != 0);
	}
	],
		[cf_cv_func_nanosleep=yes],
		[cf_cv_func_nanosleep=no],
		[cf_cv_func_nanosleep=unknown])])

	test "$cf_cv_func_nanosleep" = "yes" && \
		AC_DEFINE([HAVE_NANOSLEEP], 1, [nonosleep() available.])
])


dnl ---------------------------------------------------------------------------
dnl CF_ADD_CFLAGS
dnl
dnl	Copy non-preprocessor flags to $CFLAGS, preprocessor flags to $CPPFLAGS
dnl	The second parameter if given makes this macro verbose.
dnl
dnl	Put any preprocessor definitions that use quoted strings in $EXTRA_CPPFLAGS,
dnl	to simplify use of $CPPFLAGS in compiler checks, etc., that are easily
dnl	confused by the quotes (which require backslashes to keep them usable).
dnl

AC_DEFUN([CF_ADD_CFLAGS],[
	cf_fix_cppflags=no
	cf_new_cflags=
	cf_new_cppflags=
	cf_new_extra_cppflags=

	for cf_add_cflags in $1
	do
	case $cf_fix_cppflags in
	no)
		case $cf_add_cflags in #(vi
		-undef|-nostdinc*|-I*|-D*|-U*|-E|-P|-C) #(vi
			case $cf_add_cflags in
			-D*)
				cf_tst_cflags=`echo ${cf_add_cflags} |sed -e 's/^-D[[^=]]*='\''\"[[^"]]*//'`

				test "${cf_add_cflags}" != "${cf_tst_cflags}" \
				&& test -z "${cf_tst_cflags}" \
				&& cf_fix_cppflags=yes

				if test $cf_fix_cppflags = yes ; then
					cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"
					continue
				elif test "${cf_tst_cflags}" = "\"'" ; then
					cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"
					continue
				fi
				;;
			esac
			case "$CPPFLAGS" in
			*$cf_add_cflags) #(vi
				;;
			*) #(vi
				cf_new_cppflags="$cf_new_cppflags $cf_add_cflags"
				;;
			esac
			;;
		*)
			cf_new_cflags="$cf_new_cflags $cf_add_cflags"
			;;
		esac
		;;
	yes)
		cf_new_extra_cppflags="$cf_new_extra_cppflags $cf_add_cflags"

		cf_tst_cflags=`echo ${cf_add_cflags} |sed -e 's/^[[^"]]*"'\''//'`

		test "${cf_add_cflags}" != "${cf_tst_cflags}" \
		&& test -z "${cf_tst_cflags}" \
		&& cf_fix_cppflags=no
		;;
	esac
	done

	if test -n "$cf_new_cflags" ; then
		ifelse($2,,,[CF_VERBOSE(add to \$CFLAGS $cf_new_cflags)])
		CFLAGS="$CFLAGS $cf_new_cflags"
	fi

	if test -n "$cf_new_cppflags" ; then
		ifelse($2,,,[CF_VERBOSE(add to \$CPPFLAGS $cf_new_cppflags)])
		CPPFLAGS="$cf_new_cppflags $CPPFLAGS"
	fi

	if test -n "$cf_new_extra_cppflags" ; then
		ifelse($2,,,[CF_VERBOSE(add to \$EXTRA_CPPFLAGS $cf_new_extra_cppflags)])
		EXTRA_CPPFLAGS="$cf_new_extra_cppflags $EXTRA_CPPFLAGS"
	fi
	AC_SUBST(EXTRA_CPPFLAGS)
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
dnl CF_ADD_LIBS
dnl
dnl Add a lib to current LIBS if it is not already there.
dnl

AC_DEFUN([CF_ADD_LIBS],[
	LIBS_addsave=[$LIBS]
	if test "x$LIBS_addsave" != "x" ; then
		flag=`echo "$1" | sed 's/-/\\\-/g'`
		if test -z "`echo \"${LIBS}\" | grep \"${flag}\"`" ; then
			LIBS="$LIBS_addsave $1"
		fi
	else
		LIBS="$1"
	fi
])dnl


dnl ---------------------------------------------------------------------------
dnl Find version of gcc
dnl

AC_DEFUN([CF_GCC_VERSION],[
	AC_REQUIRE([AC_PROG_CC])
	GCC_VERSION=none
	if test "$GCC" = yes ; then
		AC_MSG_CHECKING(version of $CC)
		GCC_VERSION="`${CC} --version| sed -e '2,$d' -e 's/^.*(GCC) //' -e 's/^[[^0-9.]]*//' -e 's/[[^0-9.]].*//'`"
		test -z "$GCC_VERSION" && GCC_VERSION=unknown
		AC_MSG_RESULT($GCC_VERSION)
	fi
])dnl

dnl ---------------------------------------------------------------------------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
dnl

AC_DEFUN([CF_UPPER],[
	$1=`echo "$2" | sed y%abcdefghijklmnopqrstuvwxyz./-%ABCDEFGHIJKLMNOPQRSTUVWXYZ___%`
])

dnl ---------------------------------------------------------------------------
dnl Test for availability of useful gcc __attribute__ directives to quiet
dnl compiler warnings.
dnl
dnl Defines the following:
dnl     GCC_SCANF
dnl     GCC_PRINTF
dnl     GCC_UNUSED
dnl     GCC_NORETURN
dnl     GCC_SCANFLIKE
dnl     GCC_PRINTFLIKE
dnl

AC_DEFUN([CF_GCC_ATTRIBUTES],[

	if test "$GCC" = yes; then
		cat > conftest.i <<EOF
#ifndef GCC_PRINTF
#define GCC_PRINTF 0
#endif
#ifndef GCC_SCANF
#define GCC_SCANF 0
#endif
#ifndef GCC_NORETURN
#define GCC_NORETURN /* nothing */
#endif
#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif
EOF

		if test "$GCC" = yes; then
			AC_CHECKING([for $CC __attribute__ directives])
			cat > conftest.$ac_ext <<EOF
#line __oline__ "${as_me:-configure}"
#include "confdefs.h"
#include "conftest.h"
#include "conftest.i"
#if GCC_PRINTF
#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#else
#define GCC_PRINTFLIKE(fmt,var) /*nothing*/
#endif
#if GCC_SCANF
#define GCC_SCANFLIKE(fmt,var)  __attribute__((format(scanf,fmt,var)))
#else
#define GCC_SCANFLIKE(fmt,var)  /*nothing*/
#endif
extern void wow(char *,...)   GCC_SCANFLIKE(1,2);
extern void oops(char *,...)  GCC_PRINTFLIKE(1,2) GCC_NORETURN;
extern void foo(void)         GCC_NORETURN;
int main(int argc GCC_UNUSED, char *argv[[]] GCC_UNUSED) { return 0; }
EOF

			cf_printf_attribute=no
			cf_scanf_attribute=no

			for cf_attribute in scanf printf unused noreturn; do
				CF_UPPER(cf_ATTRIBUTE,$cf_attribute)
				cf_directive="__attribute__(($cf_attribute))"
				echo "checking for $CC $cf_directive" 1>&AC_FD_CC

				case $cf_attribute in #(vi
				printf) #(vi
					cf_printf_attribute=yes
					cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
					;;
				scanf) #(vi
					cf_scanf_attribute=yes
					cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE 1
EOF
					;;
				*) #(vi
	    				cat >conftest.h <<EOF
#define GCC_$cf_ATTRIBUTE $cf_directive
EOF
					;;
				esac

				if AC_TRY_EVAL(ac_compile); then
					test -n "$verbose" && AC_MSG_RESULT(... $cf_attribute)
					cat conftest.h >>confdefs.h
					case $cf_attribute in #(vi
					printf) #(vi
						if test "$cf_printf_attribute" = no ; then
							cat >>confdefs.h <<EOF
#define GCC_PRINTFLIKE(fmt,var) /* nothing */
EOF
						else
							cat >>confdefs.h <<EOF
#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
EOF
						fi
						;;
					scanf) #(vi
						if test "$cf_scanf_attribute" = no ; then
							cat >>confdefs.h <<EOF
#define GCC_SCANFLIKE(fmt,var) /* nothing */
EOF
						else
							cat >>confdefs.h <<EOF
#define GCC_SCANFLIKE(fmt,var)  __attribute__((format(scanf,fmt,var)))
EOF
						fi
						;;
					esac
				fi
			done
		else
			fgrep define conftest.i >>confdefs.h
		fi
		rm -rf conftest*
	fi
])dnl


