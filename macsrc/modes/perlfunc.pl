# -*- mode: perl; -*-
# -------------------------------------------------------------
#  Run this code snippet thru 'perl -x' to extract the function names from
#
#  'perlfunc' - keywords 'if', 'else', etc. added by hand
#
#..

use strict;

@ARGV = 'perldoc -u perlfunc|';

while (<>) { 
    /^=head2\s+Alphabetical/ and last
}	                                        # cue up

                                                # keywords
my  %kw = map { $_ => length } map { /^=item\s+([a-z\d]+)/ } <>;  

    #
    # standard keywords + carp/croak (which everyone always uses)
    #
for (qw(if else elsif for foreach unless until while carp croak)) {
    $kw{$_} = length
}
delete @kw{ grep { /^dbm/ } keys %kw };	        # obsolete

my @list;		                        # store sorted keywords by length

$list[$kw{$_}] .= $_  for ( sort keys %kw );

splice @list, 0, (my $n = 2);	                # keywords with < 2 letters are useless

for (@list) {
    defined and length
        and print "define_keywords(n, $n, \"$_\"\n);\n";
    $n++
}
__END__
