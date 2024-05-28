#!/bin/sh
#set -e

srcdir=`dirname $0`
ACLOCAL_FLAGS="-I ${srcdir}/m4 ${ACLOCAL_FLAGS}"

fail() {
	status=$?
	echo "Last command failed with status $status in directory $(pwd)."
	echo "Aborting"
	exit $status
}

# Refresh GNU autotools toolchain: libtool
echo "Removing libtool cruft"
rm -f ltmain.sh ../ltmain.sh
rm -f ../config.guess ../config.sub

echo "Running libtoolize"
(glibtoolize --version) < /dev/null > /dev/null 2>&1 && LIBTOOLIZE=glibtoolize || LIBTOOLIZE=libtoolize
$LIBTOOLIZE --copy --force --no-warn || fail

# Refresh GNU autotools toolchain: autoheader
	#echo "Removing aclocal cruft"
	#rm -f aclocal.m4
	#echo "Running aclocal $ACLOCAL_FLAGS"
	#aclocal $ACLOCAL_FLAGS || fail

# Refresh GNU autotools toolchain: aclocal
	#echo "Removing autoheader cruft"
	#rm -f config.h.in src/config.h.in
	#echo "Running autoheader"
	#autoheader || fail

# Autoupdate config.sub and config.guess, from GNU CVS
WGET=`which wget`
if [ "x$WGET" != "x" ]; then
	echo "Autoupdate config.sub and config.guess (y/n)?"
	read IN
	if [ "$IN" = "y" ] || [ "$IN" = "Y" ]; then
		rm -f config.guess config.sub
		wget -O tmpfile http://savannah.gnu.org/cgi-bin/viewcvs/*checkout*/config/config/config.guess
		mv tmpfile config.guess
		wget -O tmpfile http://savannah.gnu.org/cgi-bin/viewcvs/*checkout*/config/config/config.sub
		mv tmpfile config.sub
	fi
else
	echo "Could not autoupdate config.sub and config.guess"
fi

# Refresh GNU autotools toolchain: automake
	#echo "Removing automake cruft"
	#rm -f depcomp install-sh missing mkinstalldirs
	#rm -f ../depcomp ../install-sh ../missing ../mkinstalldirs
	#rm -f stamp-h*
	#echo "Running automake"
	#touch config.rpath
	#automake-1.16 --add-missing --copy --gnu 2>/dev/null
	#rm -f config.rpath

# Refresh GNU autotools toolchain: autoconf
echo "Removing autoconf cruft"
rm -f configure
rm -rf autom4te*.cache/
echo "Running autoconf"
autoconf

#end