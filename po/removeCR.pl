#! /usr/bin/env perl
# -*- mode: perl; -*-

# file removeCR.pl
#
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author: Kornel Benko, kornel@lyx.org
#
# Remove eventually added CR's from po-file after merging from a Windows OS.
#
# Usage: removeCR.pl *.po
#
use strict;
use warnings;

use File::Temp qw/ tempfile tempdir /;
use File::Copy qw(move);

sub removeCR($);

for my $file (@ARGV) {
  if ($file =~ /\b[a-z][a-z](_[A-Z][A-Z])?\.po$/) {
    removeCR($file);
  }
  else {
    print "Not handled $file\n";
  }
}

exit(0);

sub removeCR($)
{
  my ($file) = @_;

  print "Checking file $file ... ";

  if (open(FI, $file)) {
    my $found = 0;
    my $fh =  File::Temp->new(UNLINK => 0);
    my $tmpname = $fh->filename;
    while (my $l = <FI>) {
      while ($l =~ s/\r//) {
	$found = 1;
      }
      print $fh $l;
    }
    close(FI);
    close($fh);
    if ($found) {
      print "differs from $tmpname --> rewriting\n";
      move($tmpname, $file);
    }
    else {
      print "Ok\n";
      unlink($tmpname);
    }
  }
  else {
    print "Could not read $file\n";
  }
}
