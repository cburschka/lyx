#! /usr/bin/env perl
# -*- mode: perl; -*-

# file GetOptions.pm
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING
# or at http://www.lyx.org/about/licence.php
#
# author Kornel Benko
# Full author contact details are available in the file CREDITS
# or at https://www.lyx.org/Credits
#
# Used as wrapper for Getopt::Mixed
# as
#    use GetOptions;
#    ...
#    my %optionsDef = (
#    ...
#    );
#    my %options = %{&handleOptions(\%optionsDef)};

package GetOptions;

use strict;
our(@EXPORT, @ISA);

sub handleOptions($);

BEGIN {
  use Exporter   ();
  @ISA        = qw(Exporter);
  @EXPORT     = qw(handleOptions);
}

use warnings;
use Getopt::Mixed;

sub makeOpts();            # Create option spec for Getopt::Mixed::init()
sub makeHelp();            # Create help-string to describe options

# Following fields for a parameter can be defined:
# fieldname:         Name of entry in %options
# type:              [:=][sif], ':' = optional, '=' = required, 's' = string, 'i' = integer, 'f' = float
# alias:             reference to a list of aliases e.g. ["alias1", "alias2", ... ]
# listsep:           Separator for multiple data
# comment:           Parameter description

my %optionsDef = ();
#option|param|type|aliases|comment
my $helpFormat = "  %-8s|%-9s|%-7s|%-17s|%s\n";

sub handleOptions($)
{
  if (ref($_[0]) eq "ARRAY") {
    for (my $i = 0; defined($_[0]->[$i]); $i++) {
      my $rO = $_[0]->[$i];
      $optionsDef{$rO->[0]} = $rO->[1];
      $optionsDef{$rO->[0]}->{Sort} = $i+1;
    }
  }
  else {
    %optionsDef = %{$_[0]};
  }
  $optionsDef{h}->{fieldname} = "help";
  $optionsDef{h}->{alias} = ["help"];
  $optionsDef{h}->{Sort} = 0;
  $optionsDef{v}->{fieldname} = "verbose";
  $optionsDef{v}->{alias} = ["verbose"];
  $optionsDef{v}->{Sort} = 0;

  my %options = ("help" => 0);
  my $opts = &makeOpts();

  Getopt::Mixed::init($opts);
  while( my( $option, $value, $pretty ) = Getopt::Mixed::nextOption()) {
    if (defined($optionsDef{$option})) {
      my $fieldname = $optionsDef{$option}->{fieldname};
      if ($option eq "h") {
        print "Syntax: $0 options xxxx ...\n";
        print "Available options:\n";
        printf($helpFormat, "option", "param", "type", "aliases", "comment");
        print "  " . "-" x 90 . "\n";
        my $optx = &makeHelp();
        print "$optx";
        $options{$fieldname} = 1;
      }
      else {
        if (defined($optionsDef{$option}->{listsep})) {
          my @list = split($optionsDef{$option}->{listsep}, $value);
          $options{$fieldname} = \@list;
        }
        else {
          $options{$fieldname} = $value;
        }
      }
    }
  }

  Getopt::Mixed::cleanup();
  if (exists($options{verbose})) {
    printf("Found following options:\n    %-16soptvalue\n", "option");
    print "    " . "-" x 32 . "\n";
    for my $k (sort keys %options) {
      if (defined($options{$k})) {
        printf("    %-16s%s\n", $k, $options{$k});
      }
      else {
        print "    $k\n";
      }
    }
  }
  if ($options{help}) {
    exit 0;
  }
  return \%options;
}

#############################################################

# Create option spec for Getopt::Mixed::init()
sub makeOpts()
{
  my $first = 1;
  my $opts = "";
  for my $ex (sort keys %optionsDef) {
    my $e = $optionsDef{$ex};
    if (! $first) {
      $opts .= " ";
    }
    $first = 0;
    $opts .= $ex;
    if (defined($e->{type})) {
      my $tp = $e->{type};
      $opts .= $tp;
    }
    if (defined($e->{alias})) {
      for my $a (@{$e->{alias}}) {
        $opts .= " $a>$ex";
      }
    }
  }
  return($opts);
}

# Create help-string to describe options
sub makeHelp()
{
  my $opts = "";
  my %modifier = (
    ":" => "optional",
    "=" => "required",
    "s" => "string",
    "i" => "integer",
    "f" => "float",
      );
  for my $ex (sort {$optionsDef{$a}->{Sort} <=> $optionsDef{$b}->{Sort};} keys %optionsDef) {
    my $e = $optionsDef{$ex};
    my $type = "";
    my $needed = "";
    my $partype = "";
    my $aliases = "";
    my $comment = "";
    if (defined($e->{type})) {
      my $tp = $e->{type};
      if ($tp =~ /^([:=])([sif])$/) {
        $needed = $modifier{$1};
        $partype = $modifier{$2};
      }
      else {
        print "wrong option type: $tp\n";
        exit(1);
      }
    }
    if (defined($e->{alias})) {
      $aliases = join(',', @{$e->{alias}});
    }
    if (defined($e->{comment})) {
      $comment = $e->{comment};
    }
    $opts .= sprintf($helpFormat, $ex, $needed, $partype, $aliases, $comment);
  }
  return($opts);
}

#############################################################
1;

