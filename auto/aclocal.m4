# $Id: aclocal.m4,v 1.21 2024/07/12 18:22:36 cvsuser Exp $
# -*- mode: Autoconf; tabs: 8; -*-
# local packages
#

###

m4_include([cf/CF_ADD_CFLAGS])
m4_include([cf/CF_ADD_LIBS])

m4_include([cf/CF_FUNC_MEMMOVE])
m4_include([cf/CF_FUNC_VASPRINTF])
m4_include([cf/CF_FUNC_NANOSLEEP])

m4_include([cf/CF_C_INLINE])
m4_include([cf/CF_C11_NORETURN])
m4_include([cf/CF_VA_COPY])

m4_include([cf/CF_CLANG_COMPILER])
m4_include([cf/CF_INTEL_COMPILER])

m4_include([cf/CF_GCC_ATTRIBUTES])
m4_include([cf/CF_GCC_VERSION])
m4_include([cf/CF_GCC_WARNINGS])
m4_include([cf/CF_GNU_SOURCE])

m4_include([cf/CF_LARGEFILE])
m4_include([cf/CF_TYPE_SIGACTION])
m4_include([cf/CF_MIXEDCASE_FILENAMES])
m4_include([cf/CF_PATHSEP])

m4_include([cf/CF_APPEND_TEXT])
m4_include([cf/CF_ARG_DISABLE])
m4_include([cf/CF_ARG_ENABLE])
m4_include([cf/CF_ARG_OPTION])
m4_include([cf/CF_ARG_WITH])
m4_include([cf/CF_CONST_X_STRING])
m4_include([cf/CF_MSG_LOG])
m4_include([cf/CF_REMOVE_CFLAGS])
m4_include([cf/CF_REMOVE_DEFINE])
m4_include([cf/CF_REMOVE_LIB])
m4_include([cf/CF_RESTORE_XTRA_FLAGS])
m4_include([cf/CF_SAVE_XTRA_FLAGS])
m4_include([cf/CF_UPPER])
m4_include([cf/CF_VERBOSE])

###

m4_include([m4/environ.m4])
m4_include([m4/acx_nanosleep.m4])
m4_include([m4/ax_compiler_vendor.m4])
m4_include([m4/ax_compiler_version.m4])
m4_include([m4/ax_check_compile_flag.m4])
m4_include([m4/ax_check_link_flag.m4])
m4_include([m4/ax_check_openssl.m4])
m4_include([m4/ax_pthread.m4])
m4_include([m4/ax_recursive_eval.m4])

m4_include([m4/libtool.m4])
m4_include([m4/ltoptions.m4])
m4_include([m4/ltsugar.m4])
m4_include([m4/ltversion.m4])
m4_include([m4/lt~obsolete.m4])

m4_include([m4/libthread.m4])
m4_include([m4/compiler.m4])
m4_include([m4/libarchive.m4])
m4_include([m4/libapriconv.m4])
m4_include([m4/libclang.m4])
m4_include([m4/libcurl.m4])
m4_include([m4/libenca.m4])
m4_include([m4/libiconv.m4])
m4_include([m4/libicu.m4])
m4_include([m4/liblzma.m4])
m4_include([m4/libbz2.m4])
m4_include([m4/libz.m4])
m4_include([m4/libmagic.m4])
m4_include([m4/libmalloc.m4])
m4_include([m4/libm.m4])
m4_include([m4/libspell.m4])
m4_include([m4/libterm.m4])
m4_include([m4/libx11.m4])
m4_include([m4/longlong.m4])
m4_include([m4/ssize_t.m4])
m4_include([m4/string.m4])
m4_include([m4/misc.m4])
m4_include([m4/iswin32.m4])

#
