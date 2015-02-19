#!/bin/sh
#
# Generate local Changelog
#

if [ -f ChangeLog ]; then
	chmod +w ChangeLog
fi

perl EXTRA/cvs2cl.pl --tags --branches --revisions --day-of-week

if [ -f ChangeLog.bak ]; then
	rm ChangeLog.bak
fi

