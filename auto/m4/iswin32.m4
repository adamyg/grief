dnl $Id: iswin32.m4,v 1.1 2024/07/12 18:22:15 cvsuser Exp $
dnl ISWIN32=yes/no.
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl


dnl ---------------------------------------------------------------------------
dnl CF_ISWIN32 - configure ISWIN32
dnl

AC_DEFUN([CF_ISWIN32],[
	AC_REQUIRE([CF_CANONICAL_HOST])
	AC_MSG_CHECKING(for win32-target)
	case "$host_os" in
		*cygwin*)
			ISWIN32=yes
			;;
		*mingw*)
			ISWIN32=yes
			;;
		*)
			ISWIN32=no
			;;
	esac
	AC_MSG_RESULT([$ISWIN32])
	AC_SUBST([ISWIN32])
])dnl

