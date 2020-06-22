@echo off
rem
rem Watcom C/C++
rem
if not defined GNUWIN32 (
        set GNUWIN32=\devl\gnuwin32
)
if not defined PERL (
        set PERL=perl
)
%PERL% makelib.pl --gnuwin32=%GNUWIN32% wc %1 %2 %3 %4

