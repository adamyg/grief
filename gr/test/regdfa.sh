#!/bin/sh
#
gcc -I.. -I../../include -DHAVE_CONFIG_H -DLOCAL_MAIN -Wall -o regdfa \
	regdfa_test.c ../regdfa.c

#end
