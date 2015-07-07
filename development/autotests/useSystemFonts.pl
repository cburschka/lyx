#! /usr/bin/env perl
# -*- mode: perl; -*-
#
# file useSystemFonts.pl
# 1.) Copies lyx-files to another location
# 2.) While copying,
#   2a.) searches for relative references to files and
#        replaces them with absolute ones
#   2b.) In order to be able to compile with luatex or xetex
#        changes default fonts to use non-tex-fonts instead
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
sub createTemporaryFileName($$);
sub copyJobPending($$);
sub addNewJob($$$$$);
sub addFileCopyJob($$$$);
sub getNewNameOf($$);

# convert lyx file to be compilable with xetex

my ($source, $dest, $format, $fontT, $rest) = @ARGV;

diestack("Too many arguments") if (defined($rest));
diestack("Sourcefilename not defined") if (! defined($source));
diestack("Destfilename not defined") if (! defined($dest));
diestack("Format (e.g. pdf4) not defined") if (! defined($format));
diestack("Font type (e.g. texF) not defined") if (! defined($fontT));

$source = File::Spec->rel2abs($source);
$dest = File::Spec->rel2abs($dest);

my %font = ();
my $lang = "main";
if ($source =~ /\/([a-z][a-z](_[A-Z][A-Z])?)\//) {
  $lang = $1;
}
if ($fontT eq "systemF") {
  if ($lang =~ /^(he|el|ru|uk|main)$/) {
    $font{roman} = "FreeSans";
    $font{sans} = "FreeSans";
    $font{typewriter} = "FreeSans";
  }
  elsif ($lang eq "fa") {
    $font{roman} = "FreeFarsi";
    $font{sans} = "FreeFarsi";
    $font{typewriter} = "FreeFarsi Monospace";
  }
  elsif ($lang eq "zh_CN") {
    $font{roman} = "WenQuanYi Micro Hei";
    $font{sans} = "WenQuanYi Micro Hei";
    $font{typewriter} = "WenQuanYi Micro Hei";
  }
  elsif ($lang eq "ko" ) {
    $font{roman} = "NanumGothic"; # NanumMyeongjo, NanumGothic Eco, NanumGothicCoding
    $font{sans} = "NanumGothic";
    $font{typewriter} = "NanumGothic";
  }
  elsif ($lang eq "ar" ) {
    # available in 'fonts-sil-scheherazade' package
    $font{roman} = "Scheherazade";
    $font{sans} = "Scheherazade";
    $font{typewriter} = "Scheherazade";
  }
  else {
    # default system fonts
    $font{roman} = "FreeSans";
    $font{sans} = "FreeSans";
    $font{typewriter} = "FreeSans";
  }
}
else {
  # use tex font here
}

my $sourcedir = dirname($source);
my $destdir = dirname($dest);
if (! -d $destdir) {
  diestack("could not make dir \"$destdir\"") if (! mkdir $destdir);
}

my $destdirOfSubdocuments;
{
  my ($name, $pat, $suffix) = fileparse($source, qr/\.[^.]*/);
  my $ext = $format . "_$lang";
  $destdirOfSubdocuments = "$destdir/tmp_$ext" . "_$name"; # Global var, something TODO here
}

if(-d $destdirOfSubdocuments) {
  rmtree($destdirOfSubdocuments);
}
mkdir($destdirOfSubdocuments);	#  for possibly included files

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

  initLyxStack(\%font, $fontT);

  while (my $l = <FI>) {
    chomp($l);
    my $rStatus = checkLyxLine($l);
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
	    }
	    else {
	      my ($newname, $res1);
	      ($newname, $res1) = addFileCopyJob("$sourcedir/$f$ext",
						  "$destdirOfSubdocuments",
						  $rStatus->{"filetype"},
						  $rFiles);
	      print "Added ($res1) file \"$sourcedir/$f$ext\" to be copied to \"$newname\"\n";
	      if ($ext ne "") {
		$newname =~ s/$ext$//;
	      }
	      $f = $newname;
	      $res += $res1;
	    }
	  }
	}
	if ($foundrelative) {
	  $rF->[$fidx] = join($separator, @{$filelist});
	  $l = join('', @{$rF});
	}
      }
    }
    print FO "$l\n";
  }
  close(FI);
  close(FO);

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

sub createTemporaryFileName($$)
{
  my ($source, $destdir) = @_;

  # get the basename to be used for the template
  my ($name, $path, $suffix) = fileparse($source, qr/\.[^.]*/);
  #print "source = $source, name = $name, path = $path, suffix = $suffix\n";
  my $template = "xx_$name" . "_";
  my $fname = File::Temp::tempnam($destdir, $template);

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

sub addFileCopyJob($$$$)
{
  my ($source, $destdirOfSubdocuments, $filetype, $rFiles) = @_;
  my ($res, $newname) = (0, undef);
  my $rJob = $rFiles->{$source};

  my $hashname = $type2hash{$filetype};
  if (! defined($hashname)) {
    diestack("unknown filetype \"$filetype\"");
  }
  if (!defined($rJob->{$hashname})) {
    addNewJob($source,
	       createTemporaryFileName($source, $destdirOfSubdocuments),
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
