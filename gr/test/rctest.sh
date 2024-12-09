#!/bin/sh
#
gcc -I.. -I../../include -DHAVE_CONFIG_H -Wall -o rc_test \
	rc_test.c ../argrc.c

#end
