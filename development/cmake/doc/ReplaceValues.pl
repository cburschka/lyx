#! /usr/bin/env perl

use strict;

# Syntax: ReplaceValues.pl [<var1>=<Subst1> [<var2>=<Subst> ...]] <Inputfile> [<Inputfile> ...]


# Parse Arguments for strings to substitute

my %Subst = ();

for my $arg (@ARGV) {
    if ($arg =~ /^([^=]+)=(.*)$/) {
	$Subst{$1} = $2;
    }
    else {
	# $arg should be filename here
	&SubstituteDataInFile($arg);
    }
}

exit(0);

#################################################################
sub SubstituteDataInFile($)
    { my ($InFile) = @_;
      open(FI, '<', $InFile) || die("Could not read \"$InFile\"");
      while (my $l = <FI>) {
	  print &SubstituteDataInLine($l);
      }
      close(FI);
  }

sub SubstituteDataInLine($)
	{my ($line) = @_;
	 my $result = $line;
	 for my $k (keys %Subst) {
	    while ($result =~ s/\b$k\b/$Subst{$k}/) {
	    }
	 }
	 return($result);
     }
