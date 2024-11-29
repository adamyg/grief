@echo off
if not "%1" == "gcc" cl  -I.. -I../../libw32 -I../../include -DHAVE_CONFIG_H -DLOCAL_MAIN -MDd -Fetokenize.exe ../m_tokenize.c -DNEEDS_GETOPT ../../libmisc/getopt.c ../../libmisc/getopt_common.c
if     "%1" == "gcc" gcc -I.. -I../../include -DHAVE_CONFIG_H -DLOCAL_MAIN -Wall -o tokenize ../m_tokenize.c
