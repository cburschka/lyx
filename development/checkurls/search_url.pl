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

$ENV{LANG} = "en";
$ENV{LANGUAGE} = "en";

my %URLS = ();
my %ignoredURLS = ();
my %revertedURLS = ();
my %extraURLS = ();
my %selectedURLS = ();

my $checkSelectedOnly = 0;
for my $arg (@ARGV) {
  die("Bad argument \"$arg\"") if ($arg !~ /=/);
  my ($type,$val) = split("=", $arg);
  if ($type eq "filesToScan") {
    #The file should be a list of files to search in
    if (open(FLIST, $val)) {
      while (my $l = <FLIST>) {
	chomp($l);
	&parse_file($l);
      }
      close(FLIST);
    }
  }
  elsif ($type eq "ignoredURLS") {
    &readUrls($val, \%ignoredURLS);
  }
  elsif ($type eq "revertedURLS") {
    &readUrls($val, \%revertedURLS);
  }
  elsif ($type eq "extraURLS") {
    &readUrls($val,  \%extraURLS);
  }
  elsif ($type eq "selectedURLS") {
    $checkSelectedOnly = 1;
    &readUrls($val,  \%selectedURLS);
  }
  else {
    die("Invalid argument \"$arg\"");
  }
}

my @urls = sort keys %URLS, keys %extraURLS;
my $errorcount = 0;

my $URLScount = 0;

for my $u (@urls) {
  if (defined($ignoredURLS{$u})) {
    $ignoredURLS{$u} += 1;
    next;
  }
  next if ($checkSelectedOnly && ! defined(${selectedURLS}{$u}));
  if (defined(${selectedURLS}{$u})) {
    ${selectedURLS}{$u} += 1;
  }
  $URLScount++;
  print "Checking '$u'";
  my $res = &check_url($u);
  if ($res) {
    print ": Failed\n";
  }
  else {
    print ": OK\n";
  }
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
    print "$err_txt \"$u\"\n";
  }
  if ($printSourceFiles) {
    if (defined($URLS{$u})) {
      for my $f(sort keys %{$URLS{$u}}) {
	print "  $f\n";
      }
    }
    if ($res ) {
      $errorcount++;
    }
  }
}

&printNotUsedURLS("Ignored", \%ignoredURLS);
&printNotUsedURLS("Selected", \%selectedURLS);

print "\n$errorcount URL-tests failed out of $URLScount\n\n";
exit($errorcount);

###############################################################################

sub printNotUsedURLS($$)
{
  my ($txt, $rURLS) = @_;
  my @msg = ();
  for my $u ( sort keys %{$rURLS}) {
    if ($rURLS->{$u} < 2) {
      push(@msg, $u);
    }
  }
  if (@msg) {
    print "\n$txt URLs not found in sources: " . join(' ',@msg) . "\n";
  }
}

sub readUrls($$)
{
  my ($file, $rUrls) = @_;

  die("Could not read file $file") if (! open(ULIST, $file));
  while (my $l = <ULIST>) {
    $l =~ s/[\r\n]+$//;		# remove eol
    $l =~ s/\s*\#.*$//;		# remove comment
    next if ($l eq "");
    $rUrls->{$l} = 1;
  }
  close(ULIST);
}

sub parse_file($)
{
  my($f) = @_;
  my $status = "out";		# outside of URL

  return if ($f =~ /\/attic\//);
  if(open(FI, $f)) {
    while(my $l = <FI>) {
      $l =~ s/[\r\n]+$//;	#  Simulate chomp
      if($status eq "out") {
	# searching for "\begin_inset Flex URL"
	if($l =~ /^\s*\\begin_inset\s+Flex\s+URL\s*$/) {
	  $status = "ininset";
	}
      }
      else {
	if($l =~ /^\s*\\end_(layout|inset)\s*$/) {
	  $status = "out";
	}
	else {
	  if($l =~ /\s*([a-z]+:\/\/.+)\s*$/) {
	    my $url = $1;
	    $status = "out";
	    &handle_url($url, $f);
	  }
	}
      }
    }
    close(FI);
  }
}

sub handle_url($$)
{
  my($url, $f) = @_;

  if(!defined($URLS{$url})) {
    $URLS{$url} = {};
  }
  $URLS{$url}->{$f} = 1;
}
