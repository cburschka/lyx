#! /usr/bin/env perl
# -*- mode: perl; -*-

package lyxStatus;

use strict;

our(@EXPORT, @ISA);

BEGIN {
  use Exporter   ();
  @ISA       = qw(Exporter);
  @EXPORT    = qw(initLyxStack checkLyxLine closeLyxStack diestack);
}

# Prototypes
sub initLyxStack($$$);
sub diestack($);
sub closeLyxStack();
sub setMatching($);
sub getMatching();
sub checkForEndBlock($);
sub newMatch(%);
sub getSearch($);
sub getFileType($);
sub getFileIdx($);
sub getExt($);
sub getResult($);
sub checkForHeader($);
sub checkForPreamble($);
sub checkForLayoutStart($);
sub checkForInsetStart($);
sub checkForLatexCommand($);
sub checkLyxLine($);

my @stack = ();			# list of HASH-Arrays
my $rFont = {};
my $useNonTexFont = "true";
my $inputEncoding = undef;

# The elements are:
# type (layout, inset, header, preamble, ...)
# name
# matching list of matching spes
#      search: regular expression
#      ext: list of extensions needed for the full path of the file spec
#      filetype: one of prefix_only,replace_only,copy_only,prefix_for_list,interpret
#      fileidx: index into the resulting array, defining the filename
#      result: conatenation of the elements should reflect the parsed line
#              but first set the modified value into $result->[$fileidx]
#              numerical value will be replaced with appropriate matching group value

sub initLyxStack($$$)
{
  $rFont = $_[0];
  if ($_[1] eq "systemF") {
    $useNonTexFont = "true";
  }
  else {
    $useNonTexFont = "false";
    $inputEncoding = $_[2];
  }
  $stack[0] = { type => "Starting"};
}

sub diestack($)
{
  my ($msg) = @_;
  # Print stack
  print "Called stack\n";
  my @call_stack = ();
  for my $depth ( 0 .. 100) {
    #my ($pkg, $file, $line, $subname, $hasargs, $wantarray) = caller($depth)
    my @stack = caller($depth);
    last if ($stack[0] ne "main");
    push(@call_stack, \@stack);
  }
  for my $depth ( 0 .. 100) {
    last if (! defined($call_stack[$depth]));
    my $subname = $call_stack[$depth]->[3];
    my $line = $call_stack[$depth]->[2];
    print "($depth) $subname()";
    if ($depth > 0) {
      my $oldline = $call_stack[$depth-1]->[2];
      print ":$oldline";
    }
    print " called from ";
    if (defined($call_stack[$depth+1])) {
      my $parent = $call_stack[$depth+1]->[3];
      print "$parent():$line\n";
    }
    else {
      my $file = $call_stack[$depth]->[1];
      print "\"$file\":$line\n";
    }
  }
  die($msg);
}

sub closeLyxStack()
{
  diestack("Stack not OK") if ($stack[0]->{type} ne "Starting");
}

sub setMatching($)
{
  my ($match) = @_;

  $stack[0]->{"matching"} = $match;
}

sub getMatching()
{
  return($stack[0]->{"matching"});
}

###########################################################
#
sub checkForEndBlock($)
{
  my ($l) = @_;

  for my $et ( qw( layout inset preamble header)) {
    if ($l =~ /^\\end_$et$/) {
      diestack("Not in $et") if ($stack[0]->{type} ne "$et");
      #print "End $et\n";
      shift(@stack);
      return(1);
    }
  }
  return(0);
}

sub newMatch(%)
{
  my %elem = @_;

  if (! defined($elem{"ext"})) {
    $elem{"ext"} = "";
  }
  if (! defined($elem{"filetype"})) {
    $elem{"filetype"} = "prefix_only";
  }
  if (! defined($elem{"fileidx"})) {
    $elem{"fileidx"} = 1;
  }
  diestack("No result defined") if (! defined($elem{"result"}));
  return(\%elem);
}

sub getSearch($)
{
  my ($m) = @_;

  return($m->{"search"});
}

sub getFileType($)
{
  my ($m) = @_;

  return($m->{"filetype"});
}

sub getFileIdx($)
{
  my ($m) = @_;

  return($m->{"fileidx"});
}

sub getExt($)
{
  my ($m) = @_;

  return($m->{"ext"});
}

sub getResult($)
{
  my ($m) = @_;

  return($m->{"result"});
}

sub checkForHeader($)
{
  my ($l) = @_;

  if ($l =~ /^\\begin_header\s*$/) {
    my %selem = ();
    $selem{type} = "header";
    $selem{name} = $1;
    unshift(@stack, \%selem);
    my @rElems = ();
    $rElems[0] = newMatch("search" => '^\\\\master\s+(.*\.lyx)',
			   "filetype" => "prefix_only",
			   "result" => ["\\master ", ""]);
    if (keys %{$rFont}) {
      for my $ff ( keys %{$rFont}) {
	# fontentry of type '\font_roman default'
	my $elem = newMatch("search" => '^\\\\font_' . $ff . '\s+[^"]*\s*$',
			     "filetype" => "replace_only",
			     "result" => ["\\font_$ff ", $rFont->{$ff}]);
	# fontentry of type '\font_roman "default"'
	my $elem1 = newMatch("search" => '^\\\\font_' . $ff . '\s+"[^"]*"\s*$',
			     "filetype" => "replace_only",
			     "result" => ["\\font_$ff \"", $rFont->{$ff}, '"']);
	# fontentry of type '\font_roman "default" "default"'
	my $elem2 = newMatch("search" => '^\\\\font_' . $ff . '\s+"(.*)"\s+"default"\s*$',
			     "filetype" => "replace_only",
			     "result" => ["\\font_$ff ", '"', "1", '" "', $rFont->{$ff}, '"']);
	push(@rElems, $elem, $elem1, $elem2);
      }
    }
    my $elemntf = newMatch("search" => '^\\\\use_non_tex_fonts\s+(false|true)',
			    "filetype" => "replace_only",
			    "result" => ["\\use_non_tex_fonts $useNonTexFont"]);
    push(@rElems, $elemntf);
    if (defined($inputEncoding)) {
      my $inputenc = newMatch("search" =>  '^\\\\inputencoding\s+(' . $inputEncoding->{search} . ')',
			      "filetype" => "replace_only",
			      "result" => ["\\inputencoding " . $inputEncoding->{out}]);
      push(@rElems, $inputenc);
    }
    setMatching(\@rElems);
    return(1);
  }
  return(0);
}

sub checkForPreamble($)
{
  my ($l) = @_;

  if ($l =~ /^\\begin_preamble\s*$/) {
    my %selem = ();
    $selem{type} = "preamble";
    $selem{name} = $1;
    unshift(@stack, \%selem);
    my $rElem = newMatch("ext" => [".eps", ".png"],
			  "search" => '^\\\\(photo|ecvpicture)(.*\{)(.*)\}',
			  "fileidx" => 3,
			  "result" => ["\\", "1", "2", "3", "}"]);
    #
    # Remove comments from preamble
    my $comments = newMatch("search" => '^([^%]*)([%]+)([^%]*)$',
    	                    "filetype" => "replace_only",
			    "result" => ["1", "2"]);
    setMatching([$rElem, $comments]);
    return(1);
  }
  return(0);
}

sub checkForLayoutStart($)
{
  my ($l) = @_;

  if ($l =~ /^\\begin_layout\s+(.*)$/) {
    #print "started layout\n";
    my %selem = ();
    $selem{type} = "layout";
    $selem{name} = $1;
    unshift(@stack, \%selem);
    if ($selem{name} =~ /^(Picture|Photo)$/ ) {
      my $rElem = newMatch("ext" => [".eps", ".png"],
			    "search" => '^(.+)',
			    "result" => ["", "", ""]);
      setMatching([$rElem]);
    }
    return(1);
  }
  return(0);
}

sub checkForInsetStart($)
{
  my ($l) = @_;

  if ($l =~ /^\\begin_inset\s+(.*)$/) {
    #print "started inset\n";
    my %selem = ();
    $selem{type} = "inset";
    $selem{name} = $1;
    unshift(@stack, \%selem);
    if ($selem{name} =~ /^(Graphics|External)$/) {
      my $rElem = newMatch("search" => '^\s+filename\s+(.+)$',
			    "filetype" => "copy_only",
			    "result" => ["\tfilename ", "", ""]);
      setMatching([$rElem]);
    }
    return(1);
  }
  return(0);
}

sub checkForLatexCommand($)
{
  my ($l) = @_;

  if ($stack[0]->{type} eq "inset") {
    if ($l =~ /^LatexCommand\s+([^\s]+)\s*$/) {
      my $param = $1;
      if ($stack[0]->{name} =~ /^CommandInset\s+bibtex$/) {
	if ($param eq "bibtex") {
	  my $rElem1 = newMatch("ext" => ".bib",
				 "filetype" => "prefix_for_list",
				 "search" => '^bibfiles\s+\"(.+)\"',
				 "result" => ["bibfiles \"", "1", "\""]);
	  my $rElem2 = newMatch("ext" => ".bst",
				 "filetype" => "prefix_for_list",
				 "search" => '^options\s+\"(.+)\"',
				 "result" => ["options \"", "1", "\""]);
	  setMatching([$rElem1, $rElem2]);
	}
      }
      elsif ($stack[0]->{name} =~ /^CommandInset\s+include$/) {
	if ($param =~ /^(verbatiminput\*?|lstinputlisting)$/) {
	  my $rElem = newMatch("search" => '^filename\s+\"(.+)\"',
				"filetype" => "copy_only",
				"result" => ["filename \"", "", "\""]);
	  setMatching([$rElem]);
	}
	elsif ($param =~ /^(include|input)$/) {
	  my $rElem = newMatch("search" => '^filename\s+\"(.+)\"',
				"filetype" => "interpret",
				"result" => ["filename \"", "", "\""]);
	  setMatching([$rElem]);
	}
      }
    }
  }
  return(0);
}

#
# parse the given line
# returns a hash with folloving values
#    found:  1 if line matched some regex
#    fileidx: index into result
#    ext: list of possible extensions to use for a valid file
#    filelist: list of found file-pathes (may be more then one, e.g. in bibfiles spec)
#    separator: to be used while concatenating the filenames
#    filetype: prefix_only,replace_only,copy_only,interpret
#              same as before, but without 'prefix_for_list'
sub checkLyxLine($)
{
  my ($l) = @_;

  return({"found" => 0}) if (checkForHeader($l));
  return({"found" => 0}) if (checkForPreamble($l));
  return({"found" => 0}) if (checkForEndBlock($l));
  return({"found" => 0}) if (checkForLayoutStart($l));
  return({"found" => 0}) if (checkForInsetStart($l));
  return({"found" => 0}) if (checkForLatexCommand($l));
  if (defined($stack[0])) {
    my $rMatch = getMatching();
    for my $m ( @{$rMatch}) {
      my $search = getSearch($m);
      if ($l =~ /$search/) {
	my @matches = ($1, $2, $3, $4);
	my $filetype = getFileType($m);
	my @result2 = @{getResult($m)};

	for my $r (@result2) {
	  if ($r =~ /^\d$/) {
	    $r = $matches[$r-1];
	  }
	}
	if ($filetype eq "replace_only") {
	  # No filename needed
	  my %result = ("found" => 1,
			"filetype" => $filetype,
			"result" => \@result2);
	  return(\%result);
	}
	else {
	  my $fileidx = getFileIdx($m);
	  my $filename = $matches[$fileidx-1];
	  if ($filename !~ /^\.*$/) {
	    my %result = ("found" => 1,
			  "fileidx" => $fileidx,
			  "ext" => getExt($m),
			  "result" => \@result2);
	    if ($filetype eq "prefix_for_list") {
	      # bibfiles|options in CommandInset bibtex
	      my @filenames = split(',', $filename);
	      $result{"separator"} = ",";
	      $result{"filelist"} = \@filenames;
	      $result{"filetype"} = "prefix_only";
	    }
	    else {
	      $result{"separator"} = "";
	      $result{"filelist"} = [$filename];
	      $result{"filetype"} = $filetype;
	    }
	    return(\%result);
	  }
	}
      }
    }
  }
  return({"found" => 0});
}

1;
