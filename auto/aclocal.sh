#!/bin/sh
#
#   rebuild aclocal.m4
#
aclocal -I cf -I m4 --install --verbose >aclocal.out 2>&1
