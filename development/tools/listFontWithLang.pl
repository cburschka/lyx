#! /usr/bin/env perl
# -*- mode: perl; -*-

# file listFontWithLang.pl
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING
# or at http://www.lyx.org/about/licence.php
#
# author Kornel Benko
# Full author contact details are available in the file CREDITS
# or at https://www.lyx.org/Credits
#
# Usage: listFontWithLang.pl <options>
#   Displays installed system font names selected by <options>
#   Option-strings with more that 1 char need be prefixed by '--'
#
# Option to get list of options: -h
# 
# Some equivalencies for instance with option -n
#	-n arial
#	-N=arial
#	--nAme=Arial
#	--name arial
# Options and option-parameter are case insensitive

BEGIN {
    use File::Spec;
    my $p = File::Spec->rel2abs( __FILE__ );
    $p =~ s/[\/\\]?[^\/\\]+$//;
    unshift(@INC, $p);
}

use strict;
use warnings;
use GetOptions;

sub convertlang($);
sub extractlist($$$);	# my ($l, $islang, $txt, $rres) = @_;
sub getIndex($$);
sub getVal($$$);	# my ($l, $txtval, $txtlang) = @_;
sub getproperties($$$$);
sub correctstyle($);

# Following fields for a parameter can be defined:
# fieldname:         Name of entry in %options
# type:              [:=][sif], ':' = optional, '=' = required, 's' = string, 'i' = integer, 'f' = float
# alias:             reference to a list of aliases e.g. ["alias1", "alias2", ... ]
# listsep:           Separator for multiple data
# comment:           Parameter description
my %optionsDef = (
  # help + verbose already handled in 'GetOptions'
  "l"       => {fieldname => "Lang",
                type => "=s", alias=>["lang"],
                comment => "Comma separated list of desired languages"},
  "n"       => {fieldname => "FontName", listsep => ',',
                type => "=s", alias => ["name"],
                comment => "Select font-names matching these (comma separated) regexes"},
  "nn"      => {fieldname => "NFontName",
                type => "=s", listsep => ',',
                comment => "Select font-names NOT matching these (comma separated) regexes"},
  "ps"      => {fieldname => "Scripts", alias => ["printscripts"],
                comment => "Print supported scripts"},
  "pl"      => {fieldname => "PrintLangs", alias => ["printlangs"],
                comment => "Print supported languages"},
  "pp"      => {fieldname => "PrintProperties", alias => ["printproperties"],
                comment => "Print properties from weight, slant and width"},
  "pf"      => {fieldname => "PrintFiles", alias => ["printfiles"],
                comment => "Print font file names"},
  "p"       => {fieldname => "Property",
                type => "=s", listsep => ',',
                comment => "Select fonts with properties matching these (comma separated) regexes"},
  "np"      => {fieldname => "NProperty",
                type => "=s", listsep => ',',
                comment => "Select fonts with properties NOT matching these (comma separated) regexes"},
  "pw"      => {fieldname => "PrintWarnings",
                comment => "Print warnings about discarded/overwritten fonts, conflicting styles"},
);
my %options = %{&handleOptions(\%optionsDef)};

$options{Lang} = "" if (! defined($options{Lang}));

#############################################################

my @langs = split(',', $options{Lang});
for my $lg (@langs) {
  $lg = &convertlang($lg);
}

my $cmd = "fc-list";
if (defined($langs[0])) {
  $cmd .= " :lang=" . join(',', @langs);
}

my $format = "foundry=\"%{foundry}\" postscriptname=\"%{postscriptname}\" fn=\"%{fullname}\" fnl=\"%{fullnamelang}\" family=\"%{family}\" flang=\"%{familylang}\" style=\"%{style}\" stylelang=\"%{stylelang}\"";

if (exists($options{Scripts})) {
  $format .= " script=\"%{capability}\"";
}
if (exists($options{PrintLangs}) || defined($langs[0])) {
  $format .= " lang=\"%{lang}\"";
}
if (exists($options{PrintProperties}) || defined($options{Property})) {
  $format .= " weight=%{weight} slant=%{slant} width=%{width} spacing=%{spacing}";
}
$format .= " file=\"%{file}\" abcd\\n";
$cmd .= " -f '$format'";
#print "$cmd\n";

my %ftypes = (
  # Dummy internal map
  0 => "Serif",
  100 => "Sans",
  "default" => "Serif",
);

my %weights = (
  0 => "Thin",
  40 => "Extralight",
  50 => "Light",
  55 => "Semilight",
  75 => "Book",
  80 => "Regular",
  100 => "Medium",
  180 => "Semibold",
  200 => "Bold",
  205 => "Extrabold",
  210 => "Black",
);

my %slants = (
  0 => "Roman",
  100 => "Italic",
  110 => "Oblique",
);

my %widths = (
  50 => "Ultracondensed",
  63 => "Extracondensed",
  75 => "Condensed",
  87 => "Semicondensed",
  100 => "Normal",
  113 => "Semiexpanded",
  125 => "Expanded",
  150 => "Extraexpanded",
  200 => "Ultraexpanded",
);

my %spacings = (
  0 => "Proportional",
  90 => "Dual",
  100 => "Mono",
  110 => "Charcell",
  "default" => "Proportional",
);

# Build reverse mappings, (not needed yet)
for my $txt (qw(ftypes weights slants widths spacings)) {
  my $map;
  eval "\$map = \\%$txt";
  for my $key (keys %{$map}) {
    next if ($key !~ /^\d+$/);
    my $value = lc($map->{$key});
    $map->{"r"}->{$value} = $key;
  }
}

# key:= fontname
#     subkey foundry
#            subfoundry
my %collectedfonts = ();
my %fontpriority = (
  otf => 0,                # type 2, opentype CFF (Compact Font Format)
  ttc => 1.1,              # type 1 (True Type font Collection)
  ttf => 1.2,              # type 1 (True Type Font)
  woff=> 1.3,              # type 1 (Web Open Font Format)
  t1  => 1.4,              # type 1 (postscript)
  pfb => 1.5,              # type 1 (Printer Font Binary)
  pfa => 1.6,              # type 1 (Printer Font Ascii)
  pcf => 5,                # Bitmap (Packaged Collaboration Files)?
);
my $nexttype = 6;

if (open(FI,  "$cmd |")) {
 NXTLINE: while (my $l = <FI>) {
    chomp($l);
    while ($l !~ /abcd$/) {
      $l .= <FI>;
      chomp($l);
    }
    my $file = "";
    my $fonttype;
    if ($l =~ /file=\"([^\"]+)\"/) {
      $file = $1;
      #next if ($file !~ /\.(otf|ttf|pfa|pfb|pcf|ttc)$/i);
      if ($file !~ /\.([a-z0-9]{2,5})$/i) {
        print "Unhandled extension for file $file\n";
        next;
      }
      $fonttype = lc($1);
      if (! defined($fontpriority{$fonttype})) {
        print "Added extension $fonttype for file $file\n";
        $fontpriority{$fonttype} = $nexttype;
        $nexttype++;
      }
    }
    my $nfound = 0;
    my %usedlangs = ();
    if ($l =~ / lang=\"([^\"]+)\"/) {
      my @ll = split(/\|/, $1);
      for my $lx (@ll) {
	$usedlangs{&convertlang($lx)} = 1;
      }
    }

    for my $lang (@langs) {
      next NXTLINE if (! defined($usedlangs{$lang}));
    }
    next if ($nfound);
    my $style = &getVal($l, "style", "stylelang");
    $style =~ s/^\\040//;
    my $fullname = &getVal($l, "fn", "fnl");
    my $postscriptname = "";
    if ($l =~ /postscriptname=\"([^\"]+)\"/) {
      $postscriptname = $1;
    }
    my $family = &getVal($l, "family", "flang");
    $family =~ s/\\040/\-/;
    my $fontname;
    if (length($family) < 3) {
      if (length($postscriptname) < 2) {
        $fontname = $fullname;
      }
      else {
        $fontname = $postscriptname;
      }
    }
    else {
      $fontname = "$family $style";
    }
    if (defined($options{NFontName})) {
      for my $fn (@{$options{NFontName}}) {
        next NXTLINE if ($fontname =~ /$fn/i);
      }
    }
    if (defined($options{FontName})) {
      for my $fn (@{$options{FontName}}) {
        next NXTLINE if ($fontname !~ /$fn/i);
      }
    }
    my $props = "";
    my @errors = ();
    if (exists($options{PrintProperties}) || defined($options{Property}) || defined($options{NProperty})) {
      my $properties = getproperties($l, $family, $style, \@errors);
      if (defined($options{Property})) {
        for my $pn (@{$options{Property}}) {
          next NXTLINE if ($properties !~ /$pn/i);
        }
      }
      if (defined($options{NProperty})) {
        for my $pn (@{$options{NProperty}}) {
          next NXTLINE if ($properties =~ /$pn/i);
        }
      }
      if (exists($options{PrintProperties})) {
        $props .= "($properties)";
      }
    }

    if (exists($options{PrintLangs})) {
      $props .= '(' . join(',', sort keys %usedlangs) . ')';
    }
    if (exists($options{Scripts})) {
      if ($l =~ / script=\"([^\"]+)\"/) {
	my @scripts = split(/\s+/, $1);
	for my $ent (@scripts) {
	  $ent =~ s/^\s*otlayout://;
	  $ent = lc($ent);
	}
	$props .= '(' . join(',', @scripts) . ')';
      }
    }
    my $foundry = "";
    if ($l =~ /foundry=\"([^\"]+)\"/) {
      $foundry = $1;
      $foundry =~ s/^\s+//;
      $foundry =~ s/\s+$//;
    }
    if (defined($collectedfonts{$fontname}->{$foundry}->{errors})) {
      # Apparently not the first one, so add some info
      my $oldfonttype = $collectedfonts{$fontname}->{$foundry}->{fonttype};
      if (defined($errors[0])) {
        push(@{$collectedfonts{$fontname}->{$foundry}->{errors}}, @errors);
      }
      if ($fontpriority{$oldfonttype} > $fontpriority{$fonttype}) {
        push(@{$collectedfonts{$fontname}->{$foundry}->{errors}}, "Warning: overwriting old info for file: " . $collectedfonts{$fontname}->{$foundry}->{file});
      }
      else {
        push(@{$collectedfonts{$fontname}->{$foundry}->{errors}}, "Warning: discarding new info for file: $file");
        next;
      }
    }
    else {
      $collectedfonts{$fontname}->{$foundry}->{errors} = \@errors;
    }
    $collectedfonts{$fontname}->{$foundry}->{props} = $props;
    $collectedfonts{$fontname}->{$foundry}->{file} = $file;
    $collectedfonts{$fontname}->{$foundry}->{fonttype} = $fonttype;
  }
  close(FI);
}

for my $fontname (sort keys %collectedfonts) {
  my @foundries = sort keys %{$collectedfonts{$fontname}};
  my $printfoundries = 0;
  if (defined($foundries[1])) {
    $printfoundries = 1;
  }
  for my $foundry (@foundries) {
    if (exists($options{PrintWarnings})) {
      for my $err (@{$collectedfonts{$fontname}->{$foundry}->{errors}}) {
        print "$err\n";
      }
    }
    print "Font : $fontname";
    if ($printfoundries && ($foundry ne "")) {
      print " \[$foundry\]";
    }
    print $collectedfonts{$fontname}->{$foundry}->{props};
    if (exists($options{PrintFiles})) {
      print ": " . $collectedfonts{$fontname}->{$foundry}->{file} . "\n";
    }
    else {
      print "\n";
    }
  }
}

exit(0);
#################################################################################
sub convertlang($)
{
  my ($ilang) = @_;
  if ($ilang =~ /^\s*([a-z]+)([\-_]([a-z]+))?\s*$/i) {
    my ($x, $y) = ($1, $3);
    if (defined($y)) {
      $ilang = lc($x) . '-' . lc($y);
    }
    else {
      $ilang = lc($x);
    }
  }
  return($ilang);
}

sub extractlist($$$)
{
  my ($l, $islang, $txt, $rres) = @_;
  my @res = ();
  if ($l =~ /$txt=\"([^\"]+)\"/) {
    @{res} = split(',', $1);
    if ($islang) {
      for my $lg (@res) {
	$lg = &convertlang($lg);
      }
    }
  }
  @{$rres} = @res;
}

sub getIndex($$)
{
  my ($lang, $rlangs) = @_;
  for (my $i = 0; defined($rlangs->[$i]); $i++) {
    return $i if ($rlangs->[$i] eq $lang);
  }
  return(-1);
}

sub getVal($$$)
{
  my ($l, $txtval, $txtlang) = @_;
  my @values = ();
  my @langs = ();
  &extractlist($l, 0, $txtval, \@values);
  return("") if (! defined($values[0]));
  &extractlist($l, 1, $txtlang, \@langs);
  my $i = &getIndex("en", \@langs);
  return ($values[$i]) if ($i >= 0);
  return($values[0]);
}

sub getsinglevalue($$$)
{
  my ($l, $txt, $rMap, $rget) = @_;
  my $val;
  if ($l =~ / $txt=(\d+)/) {
    $val = $1;
    # Search for nearest value to $val
    if (defined($rMap->{$val})) {
      return($rMap->{$val});
    }
    my $maxv = -1;
    my $minv = 1000;
    for my $key (keys %{$rMap}) {
      next if ($key !~ /^\d+$/);
      my $diff = abs($key - $val);
      if ($diff < $minv) {
        $maxv = $key;
        $minv = $diff;
      }
      elsif ($diff == $minv) {
        if ($key > $maxv) {
          $maxv = $key;
        }
      }
    }
    if (! defined($rMap->{$maxv})) {
      print "ERROR2: txt=$txt, val=$val\n";
      exit(-2);
    }
    if ($val > $maxv) {
      return($rMap->{$maxv} . "+$minv");
    }
    else {
      return($rMap->{$maxv} . "-$minv");
    }
  }
  else {
    return(undef);
  }
}

sub addTxt($$)
{
  my ($txt, $val) = @_;
  return("$txt($val)");
}

sub getftype($$)
{
  my ($family, $style) = @_;
  if ("$family" =~ /arial|helvet/i) {
    return($ftypes{100}); # Sans Serif
  }
  elsif ($family =~ /(sans)[-_ ]?(serif)?/i) {
    return($ftypes{100}); # Sans Serif
  }
  elsif ($family =~ /serif/i) {
    return($ftypes{0});    # Serif
  }
  else {
    return(undef);
  }
}

sub getweight($$)
{
  my ($family, $style) = @_;
  my $result = undef;
  for my $key (keys %weights) {
    next if ($key !~ /^\d+$/);
    my $val = $weights{$key};
    for my $info ($style, $family) {
      if ($info =~ /\b$val\b/i) {
        if ($val eq "Regular") {
          $result = $val;    # It may refer to width
        }
        else {
          return($val);
        }
      }
    }
  }
  return($result);
}

sub getwidth($$)
{
  my ($family, $style) = @_;
  my $result = undef;
  for my $key (keys %widths) {
    next if ($key !~ /^\d+$/);
    for my $info ($style, $family) {
      if ($info =~ /\b$widths{$key}\b/i) {
        return($widths{$key});
      }
      if ($info =~ /\bRegular\b/) {
        if (!defined($result)) {
          $result = $widths{100};
        }
      }
    }
  }
  return($result);
}

sub getslant($$)
{
  my ($family, $style) = @_;
  for my $key (keys %slants) {
    next if ($key !~ /^\d+$/);
    if ($style =~ /\b$slants{$key}\b/i) {
      return($slants{$key});
    }
  }
  return(undef);
}

sub getspacing($$)
{
  my ($family, $style) = @_;
  for my $key (keys %spacings) {
    next if ($key !~ /^\d+$/);
    if ($style =~ /\b$spacings{$key}\b/i) {
      return($spacings{$key});
    }
  }
  if ("$family $style" =~ /[-_ ](mono|typewriter|cursor)\b/i) {
    return($spacings{100}); # Mono
  }
  else {
    return(undef);
  }
}

sub getproperties($$$$)
{
  my ($l, $family, $style, $rerrors) = @_;
  my $newstyle = &correctstyle($style);
  my $newfam = &correctstyle($family);
  my @properties = ();

  for my $txt (qw(ftype weight width slant spacing)) {
    my ($map, $rget);
    eval("\$map = " . '\%' . $txt . 's');
    eval('$rget = \&' . "get$txt");
    my $val2 = getsinglevalue($l, $txt, $map);
    my $val1 = $rget->($newfam, $newstyle);
    my $val;
    if (defined($val2) && defined($val1) && ($val2 ne $val1)) {
      push(@{$rerrors}, "Family($family),Style($style): Values for $txt ($val1 != $val2) differ, selecting internal $txt($val2)");
      $val = $val2;
    }
    elsif (! defined($val2)) {
      $val = $val1;
    }
    else {
      $val = $val2;
    }
    if (defined($val)) {
      push(@properties, &addTxt($txt,$val));
    }
    else {
      if (defined($map->{"default"})) {
        push(@properties, &addTxt($txt,$map->{"default"}));
      }
      else {
        push(@{$rerrors}, "Undefined value for $txt");
      }
    }
  }
  return(join(' ', @properties));
}

sub correctstyle($)
{
  my ($style) = @_;
  $style =~ s/^\\040//;
  $style =~ s/^\s*\d+\s*//;
  $style =~ s/\s*\d+$//;
  $style =~ s/italic/ Italic/i;
  $style =~ s/oblique/ Oblique/i;
  $style =~ s/[\-_]/ /g;
  $style =~ s/\breg\b/Regular/i;
  $style =~ s/\bregita(lic)?\b/Regular Italic/i;
  $style =~ s/\bregobl(ique)?\b/Regular Oblique/i;
  $style =~ s/medium/Medium /i;
  $style =~ s/\bmedita(lic)?\b/Medium Italic/i;
  $style =~ s/\bmedobl(ique)?\b/Medium Oblique/i;
  $style =~ s/\bmed\b/Medium /i;
  $style =~ s/\bdemi\b/SemiBold/i;
  $style =~ s/\bex(pd|t)\b/Expanded/i;
  $style =~ s/semi ?cond(ensed)?/SemiCondensed/i;
  $style =~ s/[sd]emi ?(bold|bd|bol)/SemiBold/i;
  $style =~ s/semi ?(expanded|extended|expd)/SemiExpanded/i;
  $style =~ s/[sd]emi ?light/SemiLight/i;
  $style =~ s/ultra ?(expanded|extended|expd)/UltraExpanded/i;
  $style =~ s/light/Light /i;
  $style =~ s/\blt\b/Light /i;
  $style =~ s/(ultra|extra)(light|lt)/ExtraLight /i;
  $style =~ s/\bheavy\b/Extrabold/i;
  $style =~ s/\bhairline\b/Extralight/i;
  $style =~ s/\bcond\b/Condensed/i;
  $style =~ s/(roman)?slanted/ Italic/i;
  $style =~ s/\bslant\b/Italic/i;
  $style =~ s/\b(SC|Small(caps(alt)?)?)\b/SmallCaps/i;
  $style =~ s/w3 mono/Dual/i;
  $style =~ s/Regul[ea]r/Regular/i;
  $style =~ s/  +/ /g;
  return($style);
}