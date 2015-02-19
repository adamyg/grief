@echo off
set PERL5LIB=c:/perl/lib
perl ./makehelp.pl -I src/library.txt.src -F ../hlpsrc/features -F ../hlpsrc/cshelp mdoc

