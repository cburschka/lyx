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

sub sexit($);		      # Print synax and exit
sub readPatterns($);	      # Process patterns file
sub processLogFile($);	      #
sub convertPattern($);	      # check for regex, comment
sub convertSimplePattern($);  # escape some chars, (e.g. ']' ==> '\]')
sub printInvalid($$);	      # display lines which should not match

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
  my ($pat) = @_;
  if ($pat eq "") {
    return("");
  }
  return $pat if ($pat =~ /^Comment:/);
  if ($pat =~ s/^((Err)?Regex):\s+//) {
    # PassThrough variant
    return($1 . ":" . $pat);
  }
  elsif ($pat =~ s/^((Err)?Simple):\s+//) {
    my $ermark = $2;
    $ermark = "" if (!defined($ermark));
    return $ermark . "Regex:" . &convertSimplePattern($pat);
  }
  else {
    # This should not happen.
    return undef;
  }
}

sub convertSimplePattern($)
{
  # Convert all chars '[]()+'
  my ($pat) = @_;
  if ($pat eq "") {
    return("");
  }
  if ($pat =~ /^(.*)(\\n)(.*)$/) {
    # do not convert '\n'
    my ($first, $found, $third) = ($1, $2, $3);
    $first = &convertSimplePattern($first);
    $third = &convertSimplePattern($third);
    return("$first$found$third");
  }
  if ($pat =~ /^(.*)([\[\]\(\)\+\^\{\}\\])(.*)$/) {
    my ($first, $found, $third) = ($1, $2, $3);
    $first = &convertSimplePattern($first);
    $third = &convertSimplePattern($third);
    return($first . "\\$found" . $third);
  }
  # Substitue white spaces
  while ($pat =~ s/[\s]+/\\s\+/) {};
  return($pat);
}

sub readPatterns($)
{
  my ($patfile) = @_;

  my $errors = 0;
  if (open(FP, $patfile)) {
    my $line = 0;
    while (my $p = <FP>) {
      $line++;
      chomp($p);
      $p = &convertPattern($p);
      if (defined($p)) {
	push(@patterns, $p) if ($p ne "");
      }
      else {
	print "Wrong entry in patterns-file at line $line\n";
	$errors++;
      }
    }
    close(FP);
  }
  if ($errors > 0) {
    exit(1);
  }
}

sub processLogFile($)
{
  my ($log) = @_;
  my $found;
  my $errors = 1;
  my @savedlines = ();
  my $readsavedlines = 0;
  my $savedline;
  my $comment = "";
  if (open(FL, $log)) {
    $errors = 0;
    my $line = 0;
    my @ErrPatterns = ();
    my $minprevlines = 0;
    for my $pat (@patterns) {
      if ($pat =~ /^Comment:\s*(.*)$/) {
	$comment = $1;
	$comment =~ s/\s+$//;
	if ($comment ne "") {
	  print "............ $comment ..........\n";
	}
	next;
      }
      if ($pat =~ /^(Err)?Regex:(.*)$/) {
	my ($type, $regex) = ($1, $2);
	next if ($regex eq "");
	if (defined($type)) {
	  # This regex should not apply until next 'found line'
	  my $erlines = () = $regex =~ /\\n/g;
	  $minprevlines = $erlines if ($erlines > $minprevlines);
	  push(@ErrPatterns, $regex);
	  next;
	}
	else {
	  # This is the pattern which we are looking for
	  $pat = $regex;
	}
      }
      #print "Searching for \"$pat\"\n";
      $found = 0;
      my $invalidmessages = 0;
      my $prevlines = () = $pat =~ /\\n/g; # Number of lines in pattern
      $prevlines = $minprevlines if ($prevlines < $minprevlines);
      my @prevl = ();
      for (my $i = 0; $i <= $prevlines; $i++) {
	push(@prevl, "\n");
      }
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
	for (my $i = 0; $i < $prevlines; $i++) {
	  $prevl[$i] = $prevl[$i+1];
	}
	$prevl[$prevlines] = $l;
	my $check = join("", @prevl);
	$line++;
	if ($check =~ /$pat/) {
	  my $fline = $line - $prevlines;
	  print "$fline:\tfound \"$pat\"\n";
	  $found = 1;
	  # Do not search in already found area
	  for (my $i = 0; $i <= $prevlines; $i++) {
	    $prevl[$i] = "\n";
	  }
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
	  # Check for not wanted patterns
	  for my $ep (@ErrPatterns) {
	    if ($check =~ /$ep/) {
	      $errors++;
	      if ($invalidmessages++ < 10) {
		my $fline = $line - $prevlines;
		&printInvalid($fline, $check);
	      }
	      last;
	    }
	  }
	}
      }
      if (! $found) {
	$errors++;
	print "\tNOT found \"$pat\" in remainder of file\n";
	$readsavedlines = 1;
      }
      @ErrPatterns = ();	# clean search for not wanted patterns
      $minprevlines = 0;
    }
    close(FL);
  }
  return($errors);
}

sub printInvalid($$)
{
  my ($line, $check) = @_;
  my @chk = split(/\n/, $check);
  print("$line:\tInvalid match: " . shift(@chk) . "\n");
  for my $l (@chk) {
    print("\t\t\t$l\n");
  }
}
