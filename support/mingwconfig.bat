@echo off
rem
rem mingW - experimental
rem
if not defined GNUWIN32 (
	set GNUWIN32=k:\devl\gnuwin32
)
if not defined PERL (
	set PERL=perl
)
%PERL% makelib.pl --gnuwin32=%GNUWIN32% mingw %1 %2 %3 %4

