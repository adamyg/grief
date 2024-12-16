#!/bin/sh
#
gcc -I.. -I../../include -DHAVE_CONFIG_H -DLOCAL_MAIN -Wall -o tokenize ../m_tokenize.c

#end
