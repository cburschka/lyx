#! /usr/bin/env perl
# -*- mode: perl; -*-
#
# file searchPatterns.pl
# Uses patterns-file to consecutively process given tex-file
# Command succedes if each pattern matches the file content in given order
#
# How to use:
#
# searchPatterns.pl patterns=<name of file with patterns> log=<name of file to check against>

use strict;
use warnings;

sub sexit($);			# Print synax and exit
sub readPatterns($);		# Process patterns file
sub processLogFile($);
sub convertPattern($);		# escape some chars, (e.g. ']' ==> '\]')

my %options = (
  "log" => undef,
  "patterns" => undef,
    );

my @patterns = ();

for my $arg (@ARGV) {
  if ($arg eq "-help") {
    &sexit(0);
  }
  if ($arg =~ /^([^=]+)=(.+)$/) {
    my ($what, $val) = ($1, $2);
    if (exists($options{$what})) {
      if (defined($options{$what})) {
	print "Value for \"$what\" already defined\n";
	&sexit(1);
      }
      $options{$what} = $val;
    }
    else {
      print "Unknown param \"$what\"\n";
      &sexit(1);
    }
  }
  else {
    print "Wrong param syntax for \"$arg\"\n";
    &sexit(1);
  }
}

for my $k (keys %options) {
  if (! defined($options{$k})) {
    &sexit(1);
  }
  if (! -r $options{$k}) {
    print "File \"$options{$k}\" is not readable\n";
    &sexit(1);
  }
}

# Read patterns
&readPatterns($options{"patterns"});
if (&processLogFile($options{"log"}) > 0) {
  print "Errors occured, exiting\n";
  exit(1);
}

exit(0);

sub syntax()
{
  print "Syntax:\n";
  print " $0";
  for my $k (keys %options) {
    print " $k=<filename>";
  }
  print "\n";
}

sub sexit($)
{
  my ($exval) = @_;
  &syntax();
  exit($exval);
}

sub convertPattern($)
{
  # Convert all chars '[]()+'
  my ($pat) = @_;
  if ($pat eq "") {
    return("");
  }
  if ($pat =~ /^(.*)([\[\]\(\)\+\^\{\}])(.*)$/) {
    my ($first, $found, $third) = ($1, $2, $3);
    $first = &convertPattern($first);
    $third = &convertPattern($third);
    return($first . "\\$found" . $third);
  }
  # Substitue white spaces
  while ($pat =~ s/[\s]+/\\s\+/) {};
  return($pat);
}

sub readPatterns($)
{
  my ($patfile) = @_;

  if (open(FP, $patfile)) {
    while (my $p = <FP>) {
      chomp($p);
      $p = &convertPattern($p);
      push(@patterns, $p);
    }
    close(FP);
  }
}

sub processLogFile($)
{
  my ($log) = @_;
  my $prevl = "\n";

  my $found;
  my $errors = 1;
  my @savedlines = ();
  my $readsavedlines = 0;
  my $savedline;
  if (open(FL, $log)) {
    $errors = 0;
    my $line = 0;
    for my $pat (@patterns) {
      #print "Searching for \"$pat\"\n";
      $found = 0;
      my @lines = ();
      if ($readsavedlines) {
	# Last regex not found
	@lines = @savedlines;
	@savedlines = ();
	$line = $savedline;
      }
      else {
	$savedline = $line;
      }
      while (1) {
	my $l;
	if ($readsavedlines) {
	  $l = shift(@lines);
	}
	else {
	  $l = <FL>;
	}
	last if (! $l);
	my $check = $prevl . $l;
	$prevl = $l;
	$line++;
	if ($check =~ /$pat/) {
	  print "$line:\tfound \"$pat\"\n";
	  $found = 1;
	  $prevl = "\n";	# Don't search this line again
	  if ($readsavedlines) {
	    @savedlines = @lines;
	  }
	  else {
	    @savedlines = ();
	  }
	  $savedline = $line;
	  last;
	}
	else {
	  push(@savedlines, $l);
	}
      }
      if (! $found) {
	$errors++;
	print "\tNOT found \"$pat\" in remainder of file\n";
	$readsavedlines = 1;
      }
    }
    close(FL);
  }
  return($errors);
}
