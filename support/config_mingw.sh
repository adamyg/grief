#!/bin/sh
# $Id: config_mingw.sh,v 1.5 2024/05/28 12:39:07 cvsuser Exp $
# MingW32 configuration .. example
#
#   o Build requirements
#
#       mingw-get install \
#           mingw32-binutils \
#           mingw32-gcc
#           msys-wget
#
#   o Development requirements
#
#       mingw-get install \
#           msys-autoconf \
#           msys-perl
#
#       --with-warnings
#

x_usage="\
Usage: $0 [OPTION]

Options:
     --help         display this help and exit.
     --base <path>  installation base directory (default /usr/local)
     --home         install within home directory

"

x_base=/usr/local

while test $# -ne 0; do
	case $1 in
	--base)	x_base="$2";
		shift
		;;

	--home)	x_base="$HOME";
		;;

	--help)	echo "$x_usage";
		exit $?
		;;

	--)	shift
		break
		;;

	-*)	echo "$0: invalid option: $1" >&2
		exit 1
		;;

	*)	break;;
	esac
	shift
done

./configure \
	--prefix=$x_base/grief \
	--datarootdir=$x_base/grief \
	--sysconfdir=$x_base/grief \
	--with-termlib=yes \
	--with-dlmalloc \
	$*

#end
