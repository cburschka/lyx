#! /usr/bin/env perl
# -*- mode: perl; -*-
#
# file diff_po.pl
# script to compare changes between translation files before merging them
#
# Examples of usage:
# ./diff_po.pl cs.po.old cs.po
# svn diff -r38367 --diff-cmd ./diff_po.pl cs.po
# git difftool --extcmd=./diff_po.pl sk.po
# ./diff_po.pl -r HEAD~100 cs.po	#fetch git revision and compare
# ./diff_po.pl -r39229 cs.po		#fetch svn revision and compare
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
# Copyright (c) 2010-2013 Kornel Benko, kornel@lyx.org
#
# TODO:
# 1.) Search for good correlations of deleted <==> inserted string
#     using Text::Levenshtein or Algorithm::Diff

BEGIN {
    use File::Spec;
    my $p = File::Spec->rel2abs( __FILE__ );
    $p =~ s/[\/\\]?[^\/\\]+$//;
    unshift(@INC, "$p");
}

use strict;
use parsePoLine;
use Term::ANSIColor qw(:constants);
use File::Temp;
use Cwd;

my ($status, $foundline, $msgid, $msgstr, $fuzzy);

my %Messages = ();              # Used for original po-file
my %newMessages = ();           # new po-file
my %Untranslated = ();          # inside new po-file
my %Fuzzy = ();                 # inside new po-file
my $result = 0;                 # exit value
my $printlines = 1;
my @names = ();

# Check first, if called as standalone program for git
if ($ARGV[0] =~ /^-r(.*)/) {
  my $rev = $1;
  shift(@ARGV);
  if ($rev eq "") {
    $rev = shift(@ARGV);
  }
  for my $argf (@ARGV) {
    my $baseargf;
    my $filedir;
    if ($argf =~ /^(.*)\/([^\/]+)$/) {
      $baseargf = $2;
      $filedir = $1;
      chdir($filedir);	# set working directory for the repo-command
    }
    else {
      $baseargf = $argf;
      $filedir = ".";
    }
    $filedir = getcwd();
    my ($repo, $level) = &searchRepo($filedir);
    my $relargf = $baseargf;	# argf relative to the top-most repo directory
    my $topdir;
    if (defined($level)) {
      my $abspathpo = $filedir;	# directory of the po-file
      $topdir = $abspathpo;
      #print "Level = $level, abs path = $abspathpo\n";
      while ($level > 0) {
	$topdir =~ s/\/([^\/]+)$//;
	$relargf = "$1/$relargf";
	$level--;
	#print "Level = $level, topdir = $topdir, rel path = $relargf\n";
      }
      chdir($topdir);
    }
    else {
      print "Could not find the repo-type\n";
      exit(-1);
    }
    #check po-file
    &check_po_file_readable($baseargf, $relargf);
    if ($repo eq ".git") {
      my @args = ();
      my $tmpfile = File::Temp->new();
      $rev = &getrev($repo, $rev);
      push(@args, "-L", $argf . "    (" . $rev . ")");
      push(@args, "-L", $argf . "    (local copy)");
      print "git show $rev:$relargf\n";
      open(FI, "git show $rev:$relargf|");
      $tmpfile->unlink_on_destroy( 1 );
      while(my $l = <FI>) {
	print $tmpfile $l;
      }
      close(FI);
      $tmpfile->seek( 0, SEEK_END );		# Flush()
      push(@args, $tmpfile->filename, $argf);
      print "===================================================================\n";
      &diff_po(@args);
    }
    elsif ($repo eq ".svn") {
      # program svnversion needed here
      $rev = &getrev($repo, $rev);
      # call it again indirectly
      my @cmd = ("svn", "diff", "-r$rev", "--diff-cmd", $0, $relargf);
      print "cmd = " . join(' ', @cmd) . "\n";
      system(@cmd);
    }
    elsif ($repo eq ".hg") {
      # for this to work, one has to edit ~/.hgrc
      # Insert there
      #     [extensions]
      #     hgext.extdiff =
      #
      $rev = &getrev($repo, $rev);
      my @cmd = ("hg", "extdiff", "-r", "$rev", "-p", $0, $relargf);
      print "cmd = " . join(' ', @cmd) . "\n";
      system(@cmd);
    }
  }
}
else {
  &diff_po(@ARGV);
}

exit($result);
#########################################################

# This routine builds n-th parent-path
# E.g. &buildParentDir("abc", 1) --> "abc/.."
#      &buildParentDir("abc", 4) --> "abc/../../../.."
sub buildParentDir($$)
{
  my ($dir, $par) = @_;
  if ($par > 0) {
    return &buildParentDir("$dir/..", $par-1);
  }
  else {
    return $dir;
  }
}

# Tries up to 10 parent levels to find the repo-type
# Returns the repo-type
sub searchRepo($)
{
  my ($dir) = @_;
  for my $parent ( 0 .. 10 ) {
    my $f = &buildParentDir($dir, $parent);
    for my $s (".git", ".svn", ".hg") {
      if (-d "$f/$s") {
	#print "Found repo on level $parent\n";
	return ($s, $parent);
      }
    }
  }
  return("");	# not found
}

sub diff_po($$)
{
  my @args = @_;
  %Messages = ();
  %newMessages = ();
  %Untranslated = ();
  %Fuzzy = ();
  @names = ();
  while(defined($args[0])) {
    last if ($args[0] !~ /^\-/);
    my $param = shift(@args);
    if ($param eq "-L") {
      my $name = shift(@args);
      push(@names, $name);
    }
  }
  if (! defined($names[0])) {
    push(@names, "original");
  }
  if (! defined($names[1])) {
    push(@names, "new");
  }

  if (@args != 2) {
    die("names = \"", join('" "', @names) . "\"... args = \"" . join('" "', @args) . "\" Expected exactly 2 parameters");
  }

  &check_po_file_readable($names[0], $args[0]);
  &check_po_file_readable($names[1], $args[1]);

  &parse_po_file($args[0], \%Messages);
  &parse_po_file($args[1], \%newMessages);

  my @MsgKeys = &getLineSortedKeys(\%newMessages);

  print RED "<<< \"$names[0]\"\n", RESET;
  print GREEN ">>> \"$names[1]\"\n", RESET;
  for my $k (@MsgKeys) {
    if ($newMessages{$k}->{msgstr} eq "") {
      # this is still untranslated string
      $Untranslated{$newMessages{$k}->{line}} = $k;
    }
    elsif ($newMessages{$k}->{fuzzy}) {
      #fuzzy string
      $Fuzzy{$newMessages{$k}->{line}} = $k;
    }
    if (exists($Messages{$k})) {
      &printIfDiff($k, $Messages{$k}, $newMessages{$k});
      delete($Messages{$k});
      delete($newMessages{$k});
    }
  }

  if (0) {
    @MsgKeys = sort keys %Messages, keys %newMessages;
    for my $k (@MsgKeys) {
      if (defined($Messages{$k})) {
	$result |= 8;
	print "deleted message\n";
	print "< line = " . $Messages{$k}->{line} . "\n" if ($printlines);
	print RED "< fuzzy = " . $Messages{$k}->{fuzzy} . "\n", RESET;
	print RED "< msgid = \"$k\"\n", RESET;
	print RED "< msgstr = \"" . $Messages{$k}->{msgstr} . "\"\n", RESET;
      }
      if (defined($newMessages{$k})) {
	$result |= 16;
	print "new message\n";
	print "> line = " . $newMessages{$k}->{line} . "\n" if ($printlines);
	print GREEN "> fuzzy = " . $newMessages{$k}->{fuzzy} . "\n", RESET;
	print GREEN "> msgid = \"$k\"\n", RESET;
	print GREEN "> msgstr = \"" . $newMessages{$k}->{msgstr} . "\"\n", RESET;
      }
    }
  }
  else {
    @MsgKeys = &getLineSortedKeys(\%Messages);
    for my $k (@MsgKeys) {
      $result |= 8;
      print "deleted message\n";
      print "< line = " . $Messages{$k}->{line} . "\n" if ($printlines);
      print RED "< fuzzy = " . $Messages{$k}->{fuzzy} . "\n", RESET;
      print RED "< msgid = \"$k\"\n", RESET;
      print RED "< msgstr = \"" . $Messages{$k}->{msgstr} . "\"\n", RESET;
    }

    @MsgKeys = &getLineSortedKeys(\%newMessages);
    for my $k (@MsgKeys) {
      $result |= 16;
      print "new message\n";
      print "> line = " . $newMessages{$k}->{line} . "\n" if ($printlines);
      print GREEN "> fuzzy = " . $newMessages{$k}->{fuzzy} . "\n", RESET;
      print GREEN "> msgid = \"$k\"\n", RESET;
      print GREEN "> msgstr = \"" . $newMessages{$k}->{msgstr} . "\"\n", RESET;
    }
  }
  &printExtraMessages("fuzzy", \%Fuzzy, \@names);
  &printExtraMessages("untranslated", \%Untranslated, \@names);
}

sub check_po_file_readable($$)
{
  my ($spec, $filename) = @_;

  if (! -e $filename ) {
    die("$spec po file does not exist");
  }
  if ( ! -f $filename ) {
    die("$spec po file is not regular");
  }
  if ( ! -r $filename ) {
    die("$spec po file is not readable");
  }
}

# Diff of one corresponding entry
sub printDiff($$$$)
{
  my ($k, $nk, $rM, $rnM) = @_;
  print "diffline = " . $rM->{line} . "," . $rnM->{line} . "\n" if ($printlines);
  print "  msgid = \"$k\"\n";
  if ($rM->{fuzzy} eq $rnM->{fuzzy}) {
    print "  fuzzy = " . $rM->{fuzzy} . "\n" if ($printlines);
  }
  else {
    print RED "< fuzzy = " . $rM->{fuzzy} . "\n", RESET;
  }
  print RED "< msgstr = " . $rM->{msgstr} . "\n", RESET;
  if ($k ne $nk) {
    print GREEN "> msgid = \"$nk\"\n", RESET;
  }
  if ($rM->{fuzzy} ne $rnM->{fuzzy}) {
    print GREEN "> fuzzy = " . $rnM->{fuzzy} . "\n", RESET;
  }
  print GREEN "> msgstr = " . $rnM->{msgstr} . "\n", RESET;
  print "\n";
}

sub printIfDiff($$$)
{
  my ($k, $rM, $rnM) = @_;
  my $doprint = 0;
  $doprint = 1 if ($rM->{fuzzy} != $rnM->{fuzzy});
  $doprint = 1 if ($rM->{msgstr} ne $rnM->{msgstr});
  if ($doprint) {
    $result |= 4;
    &printDiff($k, $k, $rM, $rnM);
  }
}

sub printExtraMessages($$$)
{
  my ($type, $rExtra, $rNames) = @_;
  #print "file1 = $rNames->[0], file2 = $rNames->[1]\n";
  my @UntranslatedKeys = sort { $a <=> $b;} keys %{$rExtra};

  if (@UntranslatedKeys > 0) {
    print "Still " . 0 + @UntranslatedKeys . " $type messages found in $rNames->[0]\n";
    for my $l (@UntranslatedKeys) {
      print "> line $l: \"" . $rExtra->{$l} . "\"\n";
    }
  }
}

#
# get repository dependent revision representation
sub getrev($$)
{
  my ($repo, $rev) = @_;
  my $revnum;

  if ($rev eq "HEAD") {
    $revnum = 0;
  }
  else {
    return $rev if ($rev !~ /^(-|HEAD[-~])(\d+)$/);
    $revnum = $2;
  }
  if ($repo eq ".hg") {
    return "-$revnum";
  }
  elsif ($repo eq ".git") {
    return("HEAD~$revnum");
  }
  elsif ($repo eq ".svn") {
    if (open(VI, "svnversion |")) {
      while (my $r1 = <VI>) {
	chomp($r1);
	if ($r1 =~ /^((\d+):)?(\d+)M?$/) {
	  $rev = $3-$revnum;
	}
      }
      close(VI);
    }
    return $rev;
  }
  return $rev;
}
