#! /usr/bin/env perl

# file diff_po.pl
# script to compare changes between translation files before merging them
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
# author: Kornel Benko, kornel@lyx.org
#
# TODO: Search for good correlations of deleted and inserted string
# using Text::Levenshtein or Algorithm::Diff

use strict;
use Term::ANSIColor qw(:constants);

my ($status, $foundline, $msgid, $msgstr, $fuzzy);

my %Messages = ();              # Used for original po-file
my %newMessages = ();           # new po-file
my %Untranslated = ();          # inside new po-file
my %Fuzzy = ();                 # inside new po-file
my $result = 0;                 # exit value
my $printlines = 0;
my @names = ();

# 
while(defined($ARGV[0])) {
  last if ($ARGV[0] !~ /^\-/);
  my $param = shift(@ARGV);
  if ($param eq "-L") {
    my $name = shift(@ARGV);
    push(@names, $name);
  }
}
if (! defined($names[0])) {
  push(@names, "original");
}
if (! defined($names[1])) {
  push(@names, "new");
}

if (@ARGV != 2) {
  die('"', join('" "', @names, @ARGV) . "\" Expected exactly 2 parameters");
}

&check($names[0], $ARGV[0]);
&check($names[1], $ARGV[1]);

&parse_po_file($ARGV[0], \%Messages);
&parse_po_file($ARGV[1], \%newMessages);

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

&printExtraMessages("fuzzy", \%Fuzzy);
&printExtraMessages("untranslated", \%Untranslated);

exit($result);

sub check($$)
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

sub parse_po_file($$)
{
  my ($file, $rMessages) = @_;
  if (open(FI, '<', $file)) {
    $status = "normal";
    $fuzzy = 0;
    my $lineno = 0;
    while (my $line = <FI>) {
      $lineno++;
      &parse_po_line($line, $lineno, $rMessages);
    }
    &parse_po_line("", $lineno + 1, $rMessages);
    close(FI);
  }
}

sub parse_po_line($$$)
{
  my ($line, $lineno, $rMessages) = @_;
  chomp($line);

  if ($status eq "normal") {
    if ($line =~ /^#, fuzzy/) {
      $fuzzy = 1;
    }
    elsif ($line =~ s/^msgid\s+//) {
      $foundline = $lineno;
      $status = "msgid";
      $msgid = "";
      &parse_po_line($line);
    }
  }
  elsif ($status eq "msgid") {
    if ($line =~ /^\s*"(.*)"\s*/) {
      $msgid .= $1;
    }
    elsif ($line =~ s/^msgstr\s+//) {
      $status = "msgstr";
      $msgstr = "";
      &parse_po_line($line);
    }
  }
  elsif ($status eq "msgstr") {
    if ($line =~ /^\s*"(.*)"\s*/) {
      $msgstr .= $1;
    }
    else {
      if ($msgid ne "") {
	$rMessages->{$msgid}->{line} = $foundline;
	$rMessages->{$msgid}->{fuzzy} = $fuzzy;
	$rMessages->{$msgid}->{msgstr} = $msgstr;
      }
      $fuzzy = 0;
      $status = "normal";
    }
  }
  else {
    die("invalid status");
  }
}

sub getLineSortedKeys($)
{
  my ($rMessages) = @_;

  return sort {$rMessages->{$a}->{line} <=> $rMessages->{$b}->{line};} keys %{$rMessages};
}

sub printExtraMessages($$)
{
  my ($type, $rExtra) = @_;
  my @UntranslatedKeys = sort { $a <=> $b;} keys %{$rExtra};

  if (@UntranslatedKeys > 0) {
    print "Still " . 0 + @UntranslatedKeys . " $type messages found in $ARGV[1]\n";
    for my $l (@UntranslatedKeys) {
      print "> line $l: \"" . $rExtra->{$l} . "\"\n"; 
    }
  }
}
