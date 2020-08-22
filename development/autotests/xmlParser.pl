#! /usr/bin/env perl

use strict;
use XML::Parser;

my $p1 = XML::Parser->new(Style => 'Debug', ErrorContext => 2, Namespaces => 1);
for my $arg (@ARGV) {
  $p1->parsefile($arg);
}

exit(0);
