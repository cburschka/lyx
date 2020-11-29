#! /usr/bin/env perl
# -*- mode: perl; -*-
#
# file useSystemFonts.pl
# 1.) Copies lyx-files to another location
# 2.) While copying,
#   2a.) searches for relative references to files and
#        replaces them with absolute ones
#   2b.) Changes default fonts to use non-tex-fonts
#
# Syntax: perl useSystemFonts.pl sourceFile destFile format
# Each param represents a path to a file
# sourceFile: full path to a lyx file
# destFile: destination path
#   Each subdocument will be copied into a subdirectory of dirname(destFile)
# format: any string of the form '[a-zA-Z0-9]+', e.g. pdf5
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

BEGIN {
  use File::Spec;
  my $p = File::Spec->rel2abs( __FILE__ );
  $p =~ s/[\/\\]?[^\/\\]+$//;
  unshift(@INC, "$p");
}
use File::Basename;
use File::Path;
use File::Copy "cp";
use File::Temp qw/ :POSIX /;
use lyxStatus;

# Prototypes
sub printCopiedDocuments($);
sub interpretedCopy($$$$);
sub copyFoundSubdocuments($);
sub copyJob($$);
sub isrelativeFix($$$);
sub isrelative($$$);
sub createTemporaryFileName($$$);
sub copyJobPending($$);
sub addNewJob($$$$$);
sub addFileCopyJob($$$$$);
sub getNewNameOf($$);
sub getlangs($$);
sub simplifylangs($);
sub getLangEntry();

# convert lyx file to be compilable with xetex

my ($source, $dest, $format, $fontT, $encodingT, $languageFile, $rest) = @ARGV;
my %encodings = ();      # Encoding with TeX fonts, depending on language tag

diestack("Too many arguments") if (defined($rest));
diestack("Sourcefilename not defined") if (! defined($source));
diestack("Destfilename not defined") if (! defined($dest));
diestack("Format (e.g. pdf4) not defined") if (! defined($format));
diestack("Font type (e.g. texF) not defined") if (! defined($fontT));
diestack("Encoding (e.g. ascii) not defined") if (! defined($encodingT));

$source = File::Spec->rel2abs($source);
$dest = File::Spec->rel2abs($dest);

my %font = ();
my $lang = "main";
if ($source =~ /\/([a-z][a-z](_[A-Z][A-Z])?)[\/_]/) {
  $lang = $1;
}

my $inputEncoding = undef;
if ($fontT eq "systemF") {
}
elsif ($encodingT ne "default") {
  # set input encoding to the requested value
  $inputEncoding = {
        "search" => '.*', # this will be substituted from '\inputencoding'-line
	"out" => $encodingT,
    };
}
elsif (0) { # set to '1' to enable setting of inputencoding
  # use tex font here
  my %encoding = ();
  if (defined($languageFile)) {
    # The 2 lines below does not seem to have any effect
    #&getlangs($languageFile, \%encoding);
    #&simplifylangs(\%encoding);
  }
  if ($format =~ /^(pdf4)$/) { # xelatex
    # set input encoding to 'ascii' always
    $inputEncoding = {
      "search" => '.*', # this will be substituted from '\inputencoding'-line
      "out" => "ascii",
    };
  }
  elsif ($format =~ /^(dvi3|pdf5)$/) { # (dvi)?lualatex
    # when to set input encoding to 'ascii'?
    if (defined($encoding{$lang})) {
      $inputEncoding = {
	"search" => '.*', # this will be substituted from '\inputencoding'-line
	"out" => $encoding{$lang},
      };
    }
  }
}

my $sourcedir = dirname($source);
my $destdir = dirname($dest);
if (! -d $destdir) {
  diestack("could not make dir \"$destdir\"") if (! mkpath $destdir);
}

my $destdirOfSubdocuments;
{
  my ($name, $pat, $suffix) = fileparse($source, qr/\.[^.]*/);
  my $ext = $format . "-$lang";
  $name =~ s/[%_]/-/g;
  $destdirOfSubdocuments = "$destdir/tmp-$ext" . "-$name"; # Global var, something TODO here
}

if(-d $destdirOfSubdocuments) {
  rmtree($destdirOfSubdocuments);
}
mkpath($destdirOfSubdocuments);	#  for possibly included files

my %IncludedFiles = ();
my %type2hash = (
  "copy_only" => "copyonly",
  "interpret" => "interpret");

addNewJob($source, $dest, "interpret", {}, \%IncludedFiles);

copyFoundSubdocuments(\%IncludedFiles);

#printCopiedDocuments(\%IncludedFiles);

exit(0);
###########################################################

sub printCopiedDocuments($)
{
  my ($rFiles) = @_;
  for my $k (keys %{$rFiles}) {
    my $rJob = $rFiles->{$k};
    for my $j ( values %type2hash) {
      if (defined($rJob->{$j})) {
	print "$j: $k->$rJob->{$j}, " . $rJob->{$j . "copied"} . "\n";
      }
    }
  }
}

sub interpretedCopy($$$$)
{
  my ($source, $dest, $destdirOfSubdocuments, $rFiles) = @_;
  my $sourcedir = dirname($source);
  my $res = 0;

  diestack("could not read \"$source\"") if (!open(FI, $source));
  diestack("could not write \"$dest\"") if (! open(FO, '>', $dest));

  initLyxStack(\%font, $fontT, $inputEncoding);

  my $fi_line_no = 0;
  my @path_errors = ();
  while (my $l = <FI>) {
    $fi_line_no += 1;
    $l =~ s/[\n\r]+$//;
    #chomp($l);
    my $rStatus = checkLyxLine($l, $sourcedir);
    if ($rStatus->{found}) {
      my $rF = $rStatus->{result};
      if ($rStatus->{"filetype"} eq "replace_only") {
	# e.g. if no files involved (font chage etc)
	$l = join('', @{$rF});
      }
      else {
	my $filelist = $rStatus->{filelist};
	my $fidx = $rStatus->{fileidx};
	my $separator = $rStatus->{"separator"};
	my $foundrelative = 0;
	for my $f (@{$filelist}) {
	  my @isrel = isrelative($f,
				  $sourcedir,
				  $rStatus->{ext});
	  if ($isrel[0]) {
	    $foundrelative = 1;
	    my $ext = $isrel[1];
	    if ($rStatus->{"filetype"} eq "prefix_only") {
	      $f = getNewNameOf("$sourcedir/$f", $rFiles);
	      if ($format eq "docbook5") {
		$rF->[1] = join(',', @{$filelist});
		$l =  join('', @$rF);
	      }
	    }
	    else {
	      my ($newname, $res1);
              my @extlist = ();
              if (ref($rStatus->{ext}) eq "ARRAY") {
                my @extlist = @{$rStatus->{ext}};
                my $created = 0;
                for my $extx (@extlist) {
                  if (-e "$sourcedir/$f$extx") {
                    ($newname, $res1) = addFileCopyJob("$sourcedir/$f$extx",
                                                       "$destdirOfSubdocuments",
                                                       $rStatus->{"filetype"},
                                                       $rFiles, $created);
                    print "Added ($res1) file \"$sourcedir/$f$extx\" to be copied to \"$newname\"\n";
                    if (!$created && $extx ne "") {
                      $newname =~ s/$extx$//;
                    }
                    $created = 1;
                  }
                }
                print "WARNING: No prefixed file.(" . join('|', @extlist) . ") seens to exist, at \"$source:$fi_line_no\"\n" if (!$created);
              }
              else {
	      ($newname, $res1) = addFileCopyJob("$sourcedir/$f$ext",
						  "$destdirOfSubdocuments",
						  $rStatus->{"filetype"},
                                                   $rFiles, 0);
	      print "Added ($res1) file \"$sourcedir/$f$ext\" to be copied to \"$newname\"\n";
	      if ($ext ne "") {
		$newname =~ s/$ext$//;
	      }
              }
	      $f = $newname;
	      $res += $res1;
	    }
	  }
	  else {
	    if (! -e "$f") {
	      # Non relative (e.g. with absolute path) file should exist
	      if ($rStatus->{"filetype"} eq "interpret") {
		# filetype::interpret should be interpreted by lyx or latex and therefore emit error
		# We prinnt a warning instead
		print "WARNING: Interpreted file \"$f\" not found, at \"$source:$fi_line_no\"\n";
	      }
	      elsif ($rStatus->{"filetype"} eq "prefix_only") {
		# filetype::prefix_only should be interpreted by latex
		print "WARNING: Prefixed file \"$f\" not found, at \"$source:$fi_line_no\"\n";
	      }
	      else {
		# Collect the path-error-messages
		push(@path_errors, "File \"$f(" . $rStatus->{"filetype"} . ")\" not found, at \"$source:$fi_line_no\"");
	      }
	    }
	  }
	}
	if ($foundrelative && $rStatus->{"filetype"} !~ /^(prefix_for_list|prefix_only)$/) {
          # The result can be relative too
          # but, since prefix_for_list does no copy, we have to use absolute paths
          # to address files inside the source dir
          my @rel_list = ();
          for my $fr (@{$filelist}) {
            push(@rel_list, File::Spec->abs2rel($fr, $destdir));
          }
          $rF->[$fidx] = join($separator, @rel_list);
	  $l = join('', @{$rF});
	}
      }
    }
    print FO "$l\n";
  }
  close(FI);
  close(FO);
  if (@path_errors > 0) {
    for my $entry (@path_errors) {
      print "ERROR: $entry\n";
    }
    diestack("Aborted because of path errors in \"$source\"");
  }

  closeLyxStack();
  return($res);
}

sub copyFoundSubdocuments($)
{
  my ($rFiles) = @_;
  my $res = 0;
  do {
    $res = 0;
    my %copylist = ();

    for my $filename (keys  %{$rFiles}) {
      next if (! copyJobPending($filename, $rFiles));
      $copylist{$filename} = 1;
    }
    for my $f (keys %copylist) {
      # Second loop needed, because here $rFiles may change
      my ($res1, @destfiles) = copyJob($f, $rFiles);
      $res += $res1;
      for my $destfile (@destfiles) {
	print "res1 = $res1 for \"$f\" to be copied to $destfile\n";
      }
    }
  } while($res > 0);		#  loop, while $rFiles changed
}

sub copyJob($$)
{
  my ($source, $rFiles) = @_;
  my $sourcedir = dirname($source);
  my $res = 0;
  my @dest = ();

  for my $k (values %type2hash) {
    if ($rFiles->{$source}->{$k}) {
      if (! $rFiles->{$source}->{$k . "copied"}) {
        $rFiles->{$source}->{$k . "copied"} = 1;
        my $dest = $rFiles->{$source}->{$k};
        push(@dest, $dest);
        if ($k eq "copyonly") {
          diestack("Could not copy \"$source\" to \"$dest\"") if (! cp($source, $dest));
        }
        else {
          interpretedCopy($source, $dest, $destdirOfSubdocuments, $rFiles);
        }
        $res += 1;
      }
    }
  }
  return($res, @dest);
}

# Trivial check
sub isrelativeFix($$$)
{
  my ($f, $sourcedir, $ext) = @_;

  return(1, $ext) if  (-e "$sourcedir/$f$ext");
  return(0,0);
}

sub isrelative($$$)
{
  my ($f, $sourcedir, $ext) = @_;

  if (ref($ext) eq "ARRAY") {
    for my $ext2 (@{$ext}) {
      my @res = isrelativeFix($f, $sourcedir, $ext2);
      if ($res[0]) {
	return(@res);
      }
    }
    return(0,0);
  }
  else {
    return(isrelativeFix($f, $sourcedir, $ext));
  }
}

my $oldfname = "";

sub createTemporaryFileName($$$)
{
  my ($source, $destdir, $created) = @_;

  # get the basename to be used for the template
  my ($name, $path, $suffix) = fileparse($source, qr/\.[^.]*/);
  #print "source = $source, name = $name, path = $path, suffix = $suffix\n";
  my $template = "xx-$name" . "-";
  my $fname;
  if (! $created) {
    $fname = File::Temp::tempnam($destdir, $template);
    $oldfname = $fname;
  }
  else {
    $fname = $oldfname;
  }

  # Append extension from source
  if ($suffix ne "") {
    $fname .= "$suffix";
  }
  return($fname);
}

# Check, if file not copied yet
sub copyJobPending($$)
{
  my ($f, $rFiles) = @_;
  for my $t (values %type2hash) {
    if (defined($rFiles->{$f}->{$t})) {
      return 1 if (! $rFiles->{$f}->{$t . "copied"});
    }
  }
  return 0;
}

sub addNewJob($$$$$)
{
  my ($source, $newname, $hashname, $rJob, $rFiles) = @_;

  $rJob->{$hashname} = $newname;
  $rJob->{$hashname . "copied"} = 0;
  $rFiles->{$source} = $rJob;
}

sub addFileCopyJob($$$$$)
{
  my ($source, $destdirOfSubdocuments, $filetype, $rFiles, $created) = @_;
  my ($res, $newname) = (0, undef);
  my $rJob = $rFiles->{$source};

  my $hashname = $type2hash{$filetype};
  if (! defined($hashname)) {
    diestack("unknown filetype \"$filetype\"");
  }
  if (!defined($rJob->{$hashname})) {
    addNewJob($source,
               createTemporaryFileName($source, $destdirOfSubdocuments, $created),
	       "$hashname", $rJob, $rFiles);
    $res = 1;
  }
  $newname = $rJob->{$hashname};
  return($newname, $res);
}

sub getNewNameOf($$)
{
  my ($f, $rFiles) = @_;
  my $resultf = $f;

  if (defined($rFiles->{$f})) {
    for my $t (values %type2hash) {
      if (defined($rFiles->{$f}->{$t})) {
	$resultf = $rFiles->{$f}->{$t};
	last;
      }
    }
  }
  return($resultf);
}

sub getlangs($$)
{
  my ($languagefile, $rencoding) = @_;

  if (open(FI, $languagefile)) {
    while (my $l = <FI>) {
      if ($l =~ /^Language/) {
        my ($lng, $enc) = &getLangEntry();
        if (defined($lng)) {
          $rencoding->{$lng} = $enc;
        }
      }
    }
    close(FI);
  }
}

sub simplifylangs($)
{
  my ($rencoding) = @_;
  my $base = "";
  my $enc = "";
  my $differ = 0;
  my @klist = ();
  my @klist2 = ();
  for my $k (reverse sort keys %{$rencoding}) {
    my @tag = split('_', $k);
    if ($tag[0] eq $base) {
      push(@klist, $k);
      if ($rencoding->{$k} ne $enc) {
	$differ = 1;
      }
    }
    else {
      # new base, check that old base was OK
      if ($base ne "") {
	if ($differ == 0) {
	  $rencoding->{$base} = $enc;
	  push(@klist2, @klist);
	}
      }
      @klist = ($k);
      $base = $tag[0];
      $enc = $rencoding->{$k};
      $differ = 0;
    }
  }
  if ($base ne "") {
    # close handling for last entry too
    if ($differ == 0) {
      $rencoding->{$base} = $enc;
      push(@klist2, @klist);
    }
  }
  for my $k (@klist2) {
    delete($rencoding->{$k});
  }
}

sub getLangEntry()
{
  my ($lng, $enc) = (undef, undef);
  while (my $l = <FI>) {
    chomp($l);
    if ($l =~ /^\s*Encoding\s+([^ ]+)\s*$/) {
      $enc = $1;
    }
    elsif ($l =~ /^\s*LangCode\s+([^ ]+)\s*$/) {
      $lng = $1;
    }
    elsif ($l =~ /^\s*End\s*$/) {
      last;
    }
  }
  if (defined($lng) && defined($enc)) {
    return($lng, $enc);
  }
  else {
    return(undef, undef);
  }
}
