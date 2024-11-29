#!/bin/sh
gcc -I.. -I../../include -DHAVE_CONFIG_H -DLOCAL_MAIN -Wall -o ttyrgb \
	ttyrgb_test.c ../ttyrgb.c ../../lib.gcc/debug/libmisc.a

#end

