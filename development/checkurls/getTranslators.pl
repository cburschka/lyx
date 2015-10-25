#! /usr/bin/env perl
# -*- mode: perl; -*-

# file getTranslators.pl
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
# Copyright (c) 2015 Kornel Benko <kornel@lyx.org>

use strict;

package IdentityParse;
use base "HTML::Parser";
use locale;
use POSIX qw(locale_h);
use feature 'fc';

setlocale(LC_CTYPE, "");
setlocale(LC_MESSAGES, "en_US.UTF-8");
$ENV{LC_ALL} = "C";		# set language for the output of command 'msgfmt'

# Prototypes
sub insertEntry(\%$$);
sub start($$$$$);
sub text($$);
sub end($$$);
sub actual_tag();
sub convertlang($);
sub scanPoFile($);
sub row_init(\%);
sub row_valid(\%);
sub phantomscript();

my $p = new IdentityParse;
$p->strict_names(0);

my $lyxurl = "http://www.lyx.org/I18n-trunk";
my $podir = "./po";	# script starts in top binary dir
my $jsfile = "$podir/lyxtranslators.js";

my %langs = (			# translation to make language spec unique 
  "pt" => "Portuguese",
  "pt_BR" => "Portuguese (Brazilian)",
  "ar" => "Arabic",
  "bg" => "Bulgarian",
  "ca" => "Catalan",
  "cs" => "Czech",
  "da" => "Danish",
  "de" => "German",
  "el" => "Greek",
  "en" => "English",
  "es" => "Spanish",
  "eu" => "Basque",
  "fi" => "Finnish",
  "fr" => "French",
  "gl" => "Galician",
  "he" => "Hebrew",
  "hu" => "Hungarian",
  "ia" => "Interlingua",
  "id" => "Indonesian",
  "it" => "Italian",
  "ja" => "Japanese",
  "ko" => "Korean",
  "sk" => "Slovak",
  "nb" => "Norwegian (Bokmål)",
  "nl" => "Dutch",
  "nn" => "Norwegian (Nynorsk)",
  "pl" => "Polish",
  "ro" => "Romanian",
  "ru" => "Russian",
  "sr" => "Serbian",
  "sv" => "Swedish",
  "tr" => "Turkish",
  "uk" => "Ukrainian",
  "zh_CN" => "Chinese (simplified)",
  "zh_TW" => "Chinese (traditional)",
  "Simplified" => "Chinese (simplified)",
  "Simplified Chinese" => "Chinese (simplified)",
  "Traditional Chinese" => "Chinese (traditional)",
    );

open(FO, '>', "$jsfile");
print FO &phantomscript();
close(FO);

my %status = ();
my @tag = ();			# stack for active html-tags
my %page_row = ();			# entries for mail, name pofile, language (in the actual row or po-file)
my @translation_entries = qw(language mail name pofile);

my %list = ();			# collected list of rows, entry key is language

my $errors = 0;
if (open(my $fh, "phantomjs $jsfile|")) {
  $p->parse_file($fh);
  print "Parsed \"$lyxurl\"\n";
}
else {
  $errors++;	# cannot parse html file
  print "ERROR: Program \"phantomjs\" to parse \"$lyxurl\" could not be executed\n";
}

if (opendir(DI, $podir)) {
  my $po_count = 0;
  while (my $po = readdir(DI)) {
    if ($po =~ /\.po$/) {
      my $res = &scanPoFile("$po");
      if ($res == 0) {
	print "No valid entry found in \"$po\"\n";
	$errors++;
      }
      else {
	$po_count += $res;
      }
    }
  }
  closedir(DI);
  if ($po_count < 1) {
    print "ERROR: No correct po-files in directory \"$podir\" found\n";
    $errors++;
  }
  else {
    print "Found $po_count po-files with valid translator entry\n";
  }
}
else {
  print "Directory for po-files ($podir) missing\n";
  $errors++;	# PO directory not found, so cannot check
}

for my $lang (sort keys %list) {
  for my $rentry (@{$list{$lang}}) {
    my $prefix = sprintf("(%03d%) ", "$rentry->{fract}");
    if (defined($rentry->{error})) {
      $errors++;
      $prefix .= sprintf("%-24s", "$rentry->{error}:");
    }
    else {
      $prefix .= sprintf("%24s", "");
    }
    my $msg = sprintf("%-24s%-10s", "$lang:", "$rentry->{po},");
    print "$prefix$msg mail = \"$rentry->{name}\" <$rentry->{mail}>\n";
  }
}

if ($errors > 0) {
  exit(1);
}
else {
  exit(0);
}
###################################################################
# Insert collected row values in %list
sub insertEntry(\%$$)
{
  my ($rrow, $cycle, $fract) = @_;

  # Convert mangled mail
  $rrow->{mail} =~ s/ pound /\@/;
  $rrow->{mail} =~ s/ dot /\./g;
  $rrow->{mail} =~ s/ underscore /_/g;
  my %entry = ();
  my $language = $rrow->{language};
  $entry{mail} = $rrow->{mail};
  $entry{name} = $rrow->{name};
  $entry{po} = $rrow->{pofile};
  $entry{cycle} = $cycle;
  # Check, if entry already exists
  if (! defined($list{$language})) {
    $list{$language} = [];
  }
  my $found = 0;
  my $empty = 1;
  my $name1 = undef;
  for my $rentry (@{$list{$language}}) {
    $empty = 0;
    if ($cycle == 2) {
      if (! defined($rentry->{fract})) {
	$rentry->{fract} = $fract;
      }
    }
    next if (fc($rentry->{mail}) ne fc($rrow->{mail}));	# char case does not matter in mail strings
    next if ($rentry->{po} ne $rrow->{pofile});
    $name1 = $rentry->{name};
    $found = 1;
    last;
  }
  if ($cycle == 1) {
    push(@{$list{$language}}, \%entry);
  }
  else {
    $entry{fract} = $fract;
    if ($empty) {
      if ($fract > 40) {
	$entry{error} = "Missing in page";
	push(@{$list{$language}}, \%entry);
      }
    }
    elsif (! $found) {
      $entry{error} = "Different mail in po";
      push(@{$list{$language}}, \%entry);
    }
    else {
      # found, but maybe incorrect name?
      if ($name1 ne $rrow->{name}) {
	$entry{error} = "Different name in po";
	push(@{$list{$language}}, \%entry);
      }
    }
  }
}

#######################################################################
# Routines called from parse_file(): start(), text(), end().
sub start($$$$$)
{
  my ($self, $tag, $attr, $attrseq, $origtext) = @_;

  push(@tag, $tag);
  if ($tag eq "tr") {		# new table row
    &row_init(\%page_row);
    for my $k (keys %status) {
      $status{$k} = 0;
    }
  }
  $status{"Tag_" . $tag} =  $status{"Tag_" . $tag} + 1;

  if ($tag eq "a") {
    if (defined($attr->{class})) {
      if ($attr->{class} eq "urllink") {
	if ($status{Tag_td} == 6) {
	  if ($attr->{href} =~ /^mailto:(.*)$/) {
	    $page_row{mail} = $1;
	    $page_row{mail} =~ s/\%20/ /g;
	  }
	}
	elsif ($status{Tag_td} == 1) {
	  if ($attr->{href} =~ /f=po\/([a-z][a-z](_[A-Z][A-Z])?\.po)$/) {
	    $page_row{pofile} = $1;
	  }
	}
      }
    }
  }
}

sub text($$)
{
  my ($self, $text) = @_;

  if ($status{Tag_td} == 1) {
    if (&actual_tag() eq "a") {
      if ($text =~ /^[A-Z][a-z]+( .+)?$/) {
	$page_row{language} = &convertlang($text);
      }
    }
  }
  if ($status{Tag_td} == 6) {
    if (&actual_tag() eq "a") {
      $page_row{name} .= $text;	# '.=' because text can be splitted
    }
    elsif (&actual_tag() eq "td") { # name without associated e-mail
      $page_row{name} .= $text;
    }
  }
}

sub end($$$)
{
  my ($self, $tag, $origtext) = @_;

  while (my $t = pop(@tag)) {
    last if ($t eq $tag);
  }
  if ($tag eq "tr") {
    # check row entry for completeness
    return if (! &row_valid(\%page_row));
    &insertEntry(\%page_row, 1);
  }
}

sub actual_tag()
{
  return undef if (@tag == 0);
  return($tag[$#tag]);
}

sub convertlang($)
{
  my ($ilang) = @_;
  $ilang =~ s/\s+$//;
  if (defined($langs{$ilang})) {
    return($langs{$ilang});
  }
  else {
    return($ilang);
  }
}

sub scanPoFile($)
{
  my ($pofile) = @_;
  my %po_row;
  my ($translated, $fuzzy, $untranslated) = (0, 0, 0);

  if (open(FM, "msgfmt -c --statistics $podir/$pofile 2>&1 |")) {
    while (my $l = <FM>) {
      if ($l =~ s/^(\d+)\s+translated messages.\s*//) {
	$translated = $1;
      }
      if ($l =~ s/^(\d+)\s+fuzzy translations.\s*//) {
	$fuzzy = $1;
      }
      if ($l =~ s/^(\d+)\s+untranslated messages//) {
	$untranslated = $1;
      }
    }
    close(FM);
  }
  return 0 if ($translated == 0);
  my $fract = int(($translated * 100)/($translated+$fuzzy+$untranslated));
  if (open(FI, "$podir/$pofile")) {
    &row_init(\%po_row);
    $po_row{pofile} = $pofile;
    my $i = 0;
    while (my $l = <FI>) {
      last if ($l =~ /^"[A-Z].*:/);
    }
    my $inserted = 0;
    while (my $l = <FI>) {
      last if ($l !~ /^"/);
      chomp($l);
      if ($l =~ s/^"Last-Translator:\s//) {
	while ($l !~ />\\n"$/) {
	  $l =~ s/"$//;
	  my $extraline = <FI>;
	  chomp($extraline);
	  $extraline =~ s/^"//;
	  $l .= $extraline
	}
	if ($l =~ /^([^<]*)<([^>]*)>/) {	# allow empty mail
	  $po_row{mail} = $2;
	  ($po_row{name} = $1) =~ s/\s+$//;
	  $i += 2;
	}
      }
      elsif ($l =~/^"Language:\s*([^\\]+)\\n/) {
	$po_row{language} = &convertlang($1);
	$i += 1;
      }
      if (&row_valid(\%po_row)) {
	&insertEntry(\%po_row, 2, $fract);
	$inserted++;
	last;
      }
    }
    close(FI);
    return($inserted);
  }
  else {
    return(0);
  }
}

###########################################################
# handling of row entries

sub row_init(\%)
{
  my ($rrow) = @_;
  %{$rrow} = ();
  $rrow->{mail} = "";		# Allow for empty mail
}

sub row_valid(\%)
{
  my ($rrow) = @_;
  for my $k (@translation_entries) {
    return 0 if (! defined($rrow->{$k}));
  }
  return(1);
}

# used by phantomjs command to output the refered html page
sub phantomscript()
{
  return "var page = require(\"webpage\").create();
var url = \"$lyxurl\";
page.open(url,
 function (status) {
  var f = function () {
   var html = page.evaluate(function () { return document.documentElement.innerHTML });
   console.log(html);
   phantom.exit();
  };
  setTimeout(f, 5000);
 }
);
";
}
