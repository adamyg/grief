@echo off
if not "%1" == "gcc" cl  -I.. -I../../libw32 -I../../include -DHAVE_CONFIG_H -DKBPROTOCOLS_TEST -MDd regdfa_test.c ../regdfa.c
if     "%1" == "gcc" gcc -I.. -I../../include -DHAVE_CONFIG_H -DLOCAL_MAIN -Wall -o regdfa regdfa_test.c ../regdfa.c
