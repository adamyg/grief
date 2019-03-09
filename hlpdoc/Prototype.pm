# $Id: Prototype.pm,v 1.2 2018/10/01 21:19:56 cvsuser Exp $
###############################################################################
#
#   Class: Prototype
#
###############################################################################

use strict;

package Prototype;

use constant BEFORE     => 0;
use constant NAME       => 1;
use constant AFTER      => 2;
use constant PARAM      => 3;


#   Function:   New
#       Creates and returns a new object.
#
#   Parameters:
#       leading - Elements before parameters.
#       trailing - Elements after parameters.
#
#   Returns:
#       A <Prototype> object.
#
sub
New                     #($self, $leading, $trailing)
{
    my ($self, $leading, $trailing) = @_;
    my ($before, $name) =
            $self->__ParseLeading($self->__Clean($leading));
    my $object = [];

    $object->[BEFORE] = $before;
    if ($name) {
        $object->[NAME] = $name;
        $object->[AFTER] = $self->__ParseTrailing($trailing);
    } else {
        $object->[NAME] = '';
        $object->[AFTER] = $self->__Clean($trailing);
    }
    $object->[PARAM] = [ ];

    bless $object, $self;
    return $object;
}


#   Function:  Print
#       Generate a formatted.
#
#   Parameters:
#       none
#
#   Returns:
#       String contained formatted prototype.
#
sub
Print                   #([framed])
{
    my ($self, $framed) = @_;
    my $ret = '';

    $ret .= '.Dl ' . ("_" x 50) . "\n"
        if (defined $framed && $framed);

    if ($self->Name()) {
        $ret .= '.Ft ' . $self->Before() . "\n";
        $ret .= '.Fo ' . $self->Name() . "\n";
        foreach my $arg ($self->Parameters()) {
            $arg =~ s{\"}{\\*q}g;
            $ret .= '.Fa "' . $arg . "\"\n";
        }
        $ret .= '.Fc ' . "\n";

    } else {
        $ret .= '\fB';
        $ret .= $self->Before();
        foreach ($self->Parameters()) {
            $ret .= ' ' . $_;
        }
        $ret .= $self->After();
        $ret .= '\fR';
        $ret .= "\n";
    }

    $ret .= '.Dl ' . ("_" x 50) . "\n"
        if (defined $framed && $framed);

    return $ret;
}


#   Function:   ParsePrototype
#       Parses the prototype and returns it as a <Prototype> object.
#
#   Parameters:
#       prototype - The text prototype.
#       isClass - Class overriden, if enabled allows '{ .. }' processing.
#
#   Returns:
#       A <Prototype> object.
#
sub
ParsePrototype          #(prototype, [isClass])
{
    my ($self, $prototype, $isClass) = @_;

    # Parse the parameters out of the prototype.

    my @tokens = ($prototype =~ /([^\(\)\[\]\{\}\<\>\'\"\,\;]+|.)/g);

    my $parameter;
    my @parameterLines;

    my @symbolStack;
    my $finishedParameters;

    my ($beforeParameters, $afterParameters);

    foreach my $token (@tokens) {

        if ($finishedParameters) {
            $afterParameters .= $token;

        } elsif ($symbolStack[-1] eq '\'' || $symbolStack[-1] eq '"') {
            if ($symbolStack[0] eq '(' || ($isClass && $symbolStack[0] eq '{')) {
                $parameter .= $token;
            } else {
                $beforeParameters .= $token;
            };

            pop @symbolStack
                if ($token eq $symbolStack[-1]);

        } elsif ($token =~ /^[\(\[\{\<\'\"]$/) {
            if ($symbolStack[0] eq '(' || ($isClass && $symbolStack[0] eq '{')) {
                $parameter .= $token;
            } else {
                $beforeParameters .= $token;
            };

            push @symbolStack, $token;

        } elsif ( ($token eq ')' && $symbolStack[-1] eq '(') ||
                  ($token eq ']' && $symbolStack[-1] eq '[') ||
                  ($token eq '}' && $symbolStack[-1] eq '{') ||
                  ($token eq '>' && $symbolStack[-1] eq '<') ) {

            if ($symbolStack[0] eq '(') {
                if ($token eq ')' && scalar @symbolStack == 1) {
                    push @parameterLines, $parameter
                        if ($parameter ne ' ');

                    $finishedParameters = 1;
                    $afterParameters .= $token;

                } else {
                    $parameter .= $token;
                };

            } elsif ($isClass && $symbolStack[0] eq '{') {
                if ($token eq '}' && scalar @symbolStack == 1) {
                    push @parameterLines, $parameter
                        if ($parameter ne ' ');

                    $finishedParameters = 1;
                    $afterParameters .= $token;

                } else {
                    $parameter .= $token;
                };

            } else {
                $beforeParameters .= $token;
            };

            pop @symbolStack;

        } elsif ($token eq ',' || $token eq ';') {
            if ($symbolStack[0] eq '(' || ($isClass && $symbolStack[0] eq '{')) {
                if (scalar @symbolStack == 1) {
                    push @parameterLines, $parameter . $token;
                    $parameter = undef;

                } else {
                    $parameter .= $token;
                };

            } else {
                $beforeParameters .= $token;
            };

        } else {
            if ($symbolStack[0] eq '(' || ($isClass && $symbolStack[0] eq '{')) {
                $parameter .= $token;

            } else {
                $beforeParameters .= $token;
            };
        };
    };

    foreach my $part (\$beforeParameters, \$afterParameters) {
        $$part =~ s/^ //;
        $$part =~ s/ $//;
    };

    my $object = Prototype->New($beforeParameters, $afterParameters);

    # Parse the actual parameters.

    foreach my $parameterLine (@parameterLines) {
##      $object->AddParameter($self->ParseParameterLine($parameterLine));
        $object->AddParameter($parameterLine);
    };

    return $object;
};


sub
__Clean                 #(text)
{
    my ($self, $text) = @_;

    $text =~ s/^\s+//g;
    $text =~ s/\s+$//g;
    $text =~ s/\s+/ /g;

    return $text;
}


sub
__ParseLeading          #(leading)
{
    my ($self, $leading) = @_;

    $leading = $self->__Clean($leading);

    return ($1, $2)
        if ($leading =~ /^(.*?)([A-Z0-9_]+)\s*[\(\{]\s*$/i);

    return ($leading, '');
}


sub
__ParseTrailing         #(trailing)
{
    my ($self, $trailing) = @_;

    $trailing = $self->__Clean($trailing);

    return "$1 $2"
        if ($trailing =~ /^(.*)[\)\}](.*)$/);

    return $trailing;
}


sub
__ParseArgument         #(argument)
{
    my ($self, $argument) = @_;

    $argument = $self->__Clean($argument);

    return $1
        if ($argument =~ /^(.*),\s*$/);

    return $argument;
}



#   Function:   AddParameter
#       Adds a <Parameter> to the list.
#
sub
AddParameter            #(parameter)
{
    my ($self, $parameter) = @_;

    $parameter = $self->__ParseArgument($parameter);

    push @{$self->[PARAM]}, $parameter
        if ($parameter);
};


#   Function:   Before
#       Retrieve the element before the parameters list.
#
sub
Before                  #()
{
    my ($self) = @_;
    return $self->[BEFORE];
};


#   Function:   Name
#       Retrieve the name component.
#
sub
Name                    #()
{
    my ($self) = @_;
    return $self->[NAME];
};


#   Function:   After
#       Retrieve the element after the parameters list.
#
sub
After                   #()
{
    my ($self) = @_;
    return $self->[AFTER];
};


#   Function:   Parameter
#       Retrieve the list of <Parameter> objects.
#
sub
Parameters              #()
{
    my ($self) = @_;
    return @{$self->[PARAM]};
};

1;
