@echo off
rem
rem Visual C/C++ 2005/2008/2010
rem
if not defined GNUWIN32 (
        set GNUWIN32=\devl\gnuwin32
)
if not defined PERL (
        set PERL=perl
)
%PERL% makelib.pl --gnuwin32=%GNUWIN32% --icu=auto vc2010 %1 %2 %3 %4

