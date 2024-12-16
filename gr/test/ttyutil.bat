@echo off
if not "%1" == "gcc" cl  -I.. -I../../libw32 -I../../include -DHAVE_CONFIG_H -DKBPROTOCOLS_TEST -MDd ttyutil_test.c ../ttyutil.c
if     "%1" == "gcc" gcc -I.. -I../../libw32 -I../../include -DHAVE_CONFIG_H -DTTYUTIL_TEST -Wall -o ttyutil ttyutil_test.c ../ttyutil.c ../../lib.gcc/debug/libmisc.a
