dnl $Id: clang.m4,v 1.2 2024/05/02 14:34:31 cvsuser Exp $
dnl clang compiler check
dnl -*- mode: autoconf; tab-width: 8; -*-
dnl
dnl

AC_DEFUN([AX_COMPILER_IS_CLANG],[
	AC_CACHE_CHECK([compiler is clang], [ac_cv_compiler_is_clang], [
		if $CXX --version | grep -q -i 'clang.*llvm' ; then
			ac_cv_compiler_is_clang=yes
		else
			ac_cv_compiler_is_clang=no
		fi
	])
])

AC_DEFUN([AX_QUNUSED_ARGUMENTS_FLAG],[
	AC_CACHE_CHECK([for the compiler flag "-Qunused-arguments"], [ax_cv_qunused_arguments_flag],[

	CXXFLAGS_SAVED=$CXXFLAGS
	CXXFLAGS="$CXXFLAGS -Qunused-arguments"
	export CXXFLAGS

	AC_OALANG_PUSH(C++)
	AC_COMPILE_IFELSE([],
		[ax_cv_qunused_arguments_flag="yes"],
		[ax_cv_qunused_arguments_flag="no"])
	AC_LANG_POP
	CXXFLAGS="$CXXFLAGS_SAVED"
	])

	QUNUSED_ARGUMENTS=""
	if test x"$ax_cv_qunused_arguments_flag" = xyes ; then
		QUNUSED_ARGUMENTS="-Qunused-arguments"
	fi
	AC_SUBST(QUNUSED_ARGUMENTS)
])

AX_QUNUSED_ARGUMENTS_FLAG
AX_COMPILER_IS_CLANG

dnl
