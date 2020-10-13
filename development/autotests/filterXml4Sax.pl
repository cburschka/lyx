#! /usr/bin/env perl

use strict;
use File::Temp qw/ tempfile tempdir /;

sub convert($);
sub handlePara($);

die("No xml file specified") if (! defined($ARGV[0]));
my $f = $ARGV[0];
die("Bad extension of $f") if ($f !~ /\.xml$/);
die("Could not read $f") if (!open(FI, $f));
my ($fh, $filename) = tempfile("tempXXXX", SUFFIX => '.xml', UNLINK => 0);
while (my $l = <FI>) {
  chomp($l);
  $l = convert($l);
  print $fh "$l\n";
}
close(FI);
close($fh);
my $err = 0;
my @errors = ();
if (open(FI, "xmllint --sax $filename|")) {
  while (my $l = <FI>) {
    print $l;
  }
}
else {
  $err = 1;
  @errors = ("Could not run xmllint\n");
}
unlink($filename);
if ($err > 0) {
  die(join('', @errors));
}
exit(0);

#########################################################################
sub convert($)
{
  my ($l) = @_;
  if ($l =~ /^(.*)\<(\/?[a-zA-Z]+(:[a-zA-Z]+)?)([^\>\<]*)\>(.*)$/) {
    my ($prev,$tag,$para,$rest) = ($1,$2,$4,$5);
    $prev = &convert($prev);
    $rest = &convert($rest);
    if ($para !~ /^\s*\/?$/) {
      if ($para !~ /^\s+[a-z]+(:[a-z]+)?\s*=/) {
	$para =~ s/[^a-z_]/_/g;
      }
      else {
	$para = " " . &handlePara($para);
      }
    }
    if ($para =~ s/\s*\/$//) {
      return "$prev<$tag$para\>\</$tag\>$rest";
    }
    else {
      return "$prev<$tag$para>$rest";
    }
  }
  else {
    return($l);
  }
}

sub handlePara($)
{
  my ($para) = @_;
  if ($para =~ /^\s*([a-z]+(:[a-z]+)?)\s*=\s*(.*)$/) {
    my $val;
    my ($p, $rest) = ($1, $3);
    if ($rest =~ /^(\'([^\']|\\\')*\')(.*)$/) {
      $val = $1;
      $rest = $3;
    }
    elsif ($rest =~ /^(\"([^\"]|\\\")*\")(.*)$/) {
      $val = $1;
      $rest = $3;
    }
    elsif ($rest =~ /^([^\s]+)(.*)$/) {
      my $val1 = $1;
      $rest = $2;
      $val1 =~ s/([\"\'\\])/\\$1/g;
      $val = '"' . $val1 . '"';
    }
    else {
      die("param error for rest = $rest");
    }
    if ($rest !~ /^\s*$/) {
      return "$p=$val " . &handlePara($rest);
    }
    else {
      return "$p=$val";
    }
  }
  return $para;
}
