@echo off
rem
rem Visual C/C++ 2005/2008/2010
rem
echo Configure: Microsoft Visual Studio C/C++ 2010

rem perl: assume within path
if not defined PERL (
        set PERL=perl
)

rem iscc: command line interface
if not defined INNO (
        set INNO="C:/Program Files (x86)/Inno Setup 5/iscc"
)

%PERL% ./support/config_windows.pl makelib.pl --inno=%INNO% %* vc2010
