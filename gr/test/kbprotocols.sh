#!/bin/sh
gcc -I.. -I../../include -DHAVE_CONFIG_H -DKBPROTOCOLS_TEST -Wall -g -o kbprotocols_test \
	kbprotocols_test.c ../kbname.c ../kbprotocols.c ../kbsequence.c ../kbwin32.c ../ttyutil.c ../../lib.gcc/debug/libtrie.a ../../lib.gcc/debug/libmisc.a

#end
