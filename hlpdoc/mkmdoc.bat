@echo off
set PERL5LIB=c:/perl/lib
perl ./makehelp.pl -I src/library.txt.src -F ./mansrc/features -F ./mansrc/cshelp mdoc

