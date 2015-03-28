#! /usr/bin/env perl
# -*- mode: perl; -*-

# How to use:

use File::Temp tempdir;

my $template = "$ARGV[0]/AbC_XXXXXX";
my $res  = tempdir($template, CLEANUP => 0);

print "$res";

exit(0);
