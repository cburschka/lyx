#! /usr/bin/env perl
# -*- mode: perl; -*-

use strict;

my @stack = ();
my $depth = 0;
my $input = $ARGV[0];
my $line = 0;
if (open(FI, $input)) {
  while (my $l = <FI>) {
    chomp($l);
    $line++;
    if ($l =~ /^\s*\\begin_([a-z]+)/) {
      $stack[$depth] = $1;
      $depth++;
    }
    elsif ($l =~ /^\s*\\(index|branch)\s/) {
      # does not start with e.g. \begin_index, but ends with \end_index!!
      $stack[$depth] = $1;
      $depth++;
    }
    elsif ($l =~ /^\s*\\end_([a-z]+)/) {
      my $expect = $1;
      if ($depth > 0) {
	if ($stack[$depth-1] eq $expect) {
	  $depth--;
	}
	else {
	  print "expected \\end_$stack[$depth-1], got \\end_$expect instead at $input:$line\n";
	  exit(-1);
	}
      }
      else {
	print "got \\end_$expect, but depth is already 0 at $input:$line\n";
	exit(-2);
      }
    }
  }
}
exit(0);
