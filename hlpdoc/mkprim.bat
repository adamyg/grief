@echo off
set PERL5LIB=c:/perl/lib
perl ./makehelp.pl --topic -I ./src/library.txt.src -F ../hlpsrc/features -F ../hlpsrc/cshelp prim

