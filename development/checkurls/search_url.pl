#! /usr/bin/env perl
# -*- mode: perl; -*-
#
# file search_url.pl
# script to search for url's in lyxfiles
# and testing their validity.
#
# Syntax: search_url.pl [(filesToScan|(ignored|reverted|extra|selected)URLS)={path_to_control]*
# Param value is a path to a file containing list of xxx:
# filesToScan={xxx = lyx-file-names to be scanned for}
# ignoredURLS={xxx = urls that are discarded from test}
# revertedURLS={xxx = urls that should fail, to test the test with invalid urls}
# extraURLS={xxx = urls which should be also checked}
#
# This file is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this software; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Copyright (c) 2013 Kornel Benko <kornel@lyx.org>
#           (c) 2013 Scott Kostyshak <skotysh@lyx.org>

use strict;

BEGIN  {
  use File::Spec;
  my $p = File::Spec->rel2abs(__FILE__);
  $p =~ s/[\/\\]?[^\/\\]+$//;
  unshift(@INC, "$p");
}

use CheckURL;
use Try::Tiny;
use locale;
use POSIX qw(locale_h);

setlocale(LC_CTYPE, "");
setlocale(LC_MESSAGES, "en_US.UTF-8");

# Prototypes
sub printNotUsedURLS($\%);
sub readUrls($\%);
sub parse_file($ );
sub handle_url($$$ );
##########

my %URLS = ();
my %ignoredURLS = ();
my %revertedURLS = ();
my %extraURLS = ();
my %selectedURLS = ();
my $summaryFile = undef;

my $checkSelectedOnly = 0;
for my $arg (@ARGV) {
  die("Bad argument \"$arg\"") if ($arg !~ /=/);
  my ($type,$val) = split("=", $arg);
  if ($type eq "filesToScan") {
    #The file should be a list of files to search in
    if (open(FLIST, $val)) {
      while (my $l = <FLIST>) {
	chomp($l);
	parse_file($l);
      }
      close(FLIST);
    }
  }
  elsif ($type eq "ignoredURLS") {
    readUrls($val, %ignoredURLS);
  }
  elsif ($type eq "revertedURLS") {
    readUrls($val, %revertedURLS);
  }
  elsif ($type eq "extraURLS") {
    readUrls($val,  %extraURLS);
  }
  elsif ($type eq "selectedURLS") {
    $checkSelectedOnly = 1;
    readUrls($val,  %selectedURLS);
  }
  elsif ($type eq "summaryFile") {
    if (open(SFO, '>', "$val")) {
      $summaryFile = $val;
    }
  }
  else {
    die("Invalid argument \"$arg\"");
  }
}

my @urls = sort keys %URLS, keys %extraURLS;
my $errorcount = 0;

my $URLScount = 0;

for my $u (@urls) {
  if (defined($selectedURLS{$u})) {
    ${selectedURLS}{$u}->{count} += 1;
  }
  if (defined($ignoredURLS{$u})) {
    $ignoredURLS{$u}->{count} += 1;
    next;
  }
  next if ($checkSelectedOnly && ! defined($selectedURLS{$u}));
  $URLScount++;
  print "Checking '$u': ";
  my ($res, $prnt, $outSum);
  try {
    $res = check_url($u);
    if ($res) {
      print "Failed\n";
      $prnt = "";
      $outSum = 1;
    }
    else {
      $prnt = "OK\n";
      $outSum = 0;
    }
  }
  catch {
    $prnt = "Failed, caught error: $_\n";
    $outSum = 1;
    $res = 700;
  };
  printx("$prnt", $outSum);
  my $printSourceFiles = 0;
  my $err_txt = "Error url:";

  if ($res || $checkSelectedOnly) {
    $printSourceFiles = 1;
  }
  if ($res && defined($revertedURLS{$u})) {
    $err_txt = "Failed url:";
  }
  $res = ! $res if (defined($revertedURLS{$u}));
  if ($res || $checkSelectedOnly) {
    printx("$err_txt \"$u\"\n", $outSum);
  }
  if ($printSourceFiles) {
    if (defined($URLS{$u})) {
      for my $f(sort keys %{$URLS{$u}}) {
	my $lines = ":" . join(',', @{$URLS{$u}->{$f}});
	printx("  $f$lines\n", $outSum);
      }
    }
    if ($res ) {
      $errorcount++;
    }
  }
}

if (%URLS) {
  printNotUsedURLS("Ignored", %ignoredURLS);
  printNotUsedURLS("Selected", %selectedURLS);
  printNotUsedURLS("KnownInvalid", %extraURLS);
}

print "\n$errorcount URL-tests failed out of $URLScount\n\n";
if (defined($summaryFile)) {
  close(SFO);
}
exit($errorcount);

###############################################################################
sub printx($$)
{
  my ($txt, $outSum) = @_;
  print "$txt";
  if ($outSum && defined($summaryFile)) {
    print SFO "$txt";
  }
}

sub printNotUsedURLS($\%)
{
  my ($txt, $rURLS) = @_;
  my @msg = ();
  for my $u ( sort keys %{$rURLS}) {
    if ($rURLS->{$u}->{count} < 2) {
      my @submsg = ();
      for my $f (sort keys %{$rURLS->{$u}}) {
	next if ($f eq "count");
	push(@submsg, "$f:" . $rURLS->{$u}->{$f});
      }
      push(@msg, "\n  $u\n    " . join("\n    ", @submsg) . "\n");
    }
  }
  if (@msg) {
    print "\n$txt URLs not found in sources: " . join(' ',@msg) . "\n";
  }
}

sub readUrls($\%)
{
  my ($file, $rUrls) = @_;

  die("Could not read file $file") if (! open(ULIST, $file));
  my $line = 0;
  while (my $l = <ULIST>) {
    $line++;
    $l =~ s/[\r\n]+$//;		# remove eol
    $l =~ s/\s*\#.*$//;		# remove comment
    next if ($l eq "");
    if (! defined($rUrls->{$l} )) {
      $rUrls->{$l} = {$file => $line, count => 1};
    }
  }
  close(ULIST);
}

sub parse_file($)
{
  my($f) = @_;
  my $status = "out";		# outside of URL/href

  return if ($f =~ /\/attic\//);
  if(open(FI, $f)) {
    my $line = 0;
    while(my $l = <FI>) {
      $line++;
      $l =~ s/[\r\n]+$//;	#  Simulate chomp
      if ($status eq "out") {
	# searching for "\begin_inset Flex URL"
	if($l =~ /^\s*\\begin_inset\s+Flex\s+URL\s*$/) {
	  $status = "inUrlInset";
	}
	elsif ($l =~ /^\s*\\begin_inset\s+CommandInset\s+href\s*$/) {
	  $status = "inHrefInset";
	}
	else {
	  # Outside of url, check also
	  if ($l =~ /"((ftp|http|https):\/\/[^ ]+)"/) {
	    my $url = $1;
	    handle_url($url, $f, "x$line");
	  }
	}
      }
      else {
	if($l =~ /^\s*\\end_(layout|inset)\s*$/) {
	  $status = "out";
	}
	elsif ($status eq "inUrlInset") {
	  if ($l =~ /\s*([a-z]+:\/\/.+)\s*$/) {
	    my $url = $1;
	    $status = "out";
	    handle_url($url, $f, "u$line");
	  }
	}
	elsif ($status eq "inHrefInset") {
	  if ($l =~ /^target\s+"([a-z]+:\/\/[^ ]+)"$/) {
	    my $url = $1;
	    $status = "out";
	    handle_url($url, $f, "h$line");
	  }
	}
      }
    }
    close(FI);
  }
}

sub handle_url($$$)
{
  my($url, $f, $line) = @_;

  if(!defined($URLS{$url})) {
    $URLS{$url} = {};
  }
  if(!defined($URLS{$url}->{$f})) {
    $URLS{$url}->{$f} = [];
  }
  push(@{$URLS{$url}->{$f}}, $line);
}
