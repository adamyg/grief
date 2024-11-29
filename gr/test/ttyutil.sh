#!/bin/sh
gcc -I.. -I../../include -DHAVE_CONFIG_H -DTTYUTIL_TEST -Wall -o ttyutil \
	ttyutil_test.c ../ttyutil.c ../../lib.gcc/debug/libmisc.a

#end
