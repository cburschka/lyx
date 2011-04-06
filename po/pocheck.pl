#! /usr/bin/perl -w

# file pocheck.pl
#
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author: Michael Gerz, michael.gerz@teststep.org
#

use strict;
use warnings;
use Getopt::Std;

my $usage = <<EOT;
pocheck.pl [-acmpqst] po_file [po_file] ...

This script performs some consistency checks on po files. 

We check for everything listed here, unless one or more of these 
options is given, in which case we checks only for those requested.
-a: Check arguments, like %1\$s
-c: Check for colons at end
-m: Check for menu shortcuts
-p: Check for period at end
-q: Check Qt shortcuts
-s: Check for space at end
-t: Check for uniform translation
These options can be given with or without other options.
-f: Ignore fuzzy translations
-w: Only report summary total of errors
-i: Silent mode, report only errors
EOT

my %options;
getopts(":hacfmpqstwi", \%options);

if (defined($options{h})) { 
  print $usage; 
  exit 0; 
}

my $only_total = defined($options{w});
delete $options{w} if $only_total;
my $ignore_fuzzy = defined($options{f});
delete $options{f} if $ignore_fuzzy;
my $silent_mode = defined($options{i});
delete $options{i} if $silent_mode;

my $check_args = (!%options or defined($options{a}));
my $check_colons = (!%options or defined($options{c}));
my $check_spaces = (!%options or defined($options{m}));
my $check_periods = (!%options or defined($options{p}));
my $check_qt = (!%options or defined($options{q}));
my $check_menu = (!%options or defined($options{s}));
my $check_trans = (!%options or defined($options{t}));

my %trans;

foreach my $pofilename ( @ARGV ) {
  my %bad;
  if (!$silent_mode) {
    print "Processing po file '$pofilename'...\n";
  }

  open( INPUT, "<$pofilename" )
    || die "Cannot read po file '$pofilename'";
  my @pofile = <INPUT>;
  close( INPUT );

  undef( %trans );
  keys( %trans ) = 10000;

  my $noOfLines = $#pofile;

  my $warn = 0;

  my $i = 0;
  my ($msgid, $msgstr, $more);

  while ($i <= $noOfLines) {
    my $linenum = $i;
    ( $msgid ) = ( $pofile[$i] =~ m/^msgid "(.*)"/ );
    $i++;
    next unless $msgid;
    if ($ignore_fuzzy) {
      my $previous = $pofile[$i - 2];
      next if $previous =~ m/#,.*fuzzy/;
    }
    
    # some msgid's are more than one line long, so add those.
    while ( ( $more ) = $pofile[$i] =~ m/^"(.*)"/ ) {
      $msgid = $msgid . $more;
      $i++;
    }
    
    # now look for the associated msgstr.
    until ( ( $msgstr ) = ( $pofile[$i] =~ m/^msgstr "(.*)"/ ) ) { $i++; };
    $i++;
    # again collect any extra lines.
    while ( ( $i <= $noOfLines ) &&
            ( ( $more ) = $pofile[$i] =~ m/^"(.*)"/ ) ) {
      $msgstr = $msgstr . $more;
      $i++;
    }

    # nothing to do if one of them is empty. 
    # (surely that is always $msgstr?)
    next if ($msgid eq "" or $msgstr eq "");

    # Check for matching %1$s, etc.
      if ($check_args) {
      my @argstrs = ( $msgid =~ m/%(\d)\$s/g );
      if (@argstrs) {
        my $n = 0;
        foreach my $arg (@argstrs) { $n = $arg if $arg > $n; }
        if ($n <= 0) { 
          print "$pofilename, line $linenum: Problem finding arguments in:\n    $msgid!\n"
            unless $only_total;
          ++$bad{"Missing arguments"};
          $warn++;
        } else {
          foreach my $i (1..$n) {
            my $arg = "%$i\\\$s"; 
            if ( $msgstr !~ m/$arg/ ) {
              print "$pofilename, line $linenum: Missing argument `$arg'\n  '$msgid' ==> '$msgstr'\n"
                unless $only_total;
              ++$bad{"Missing arguments"};
              $warn++;
            }
          }
        }
      }
    }

    if ($check_colons) {
      # Check colon at the end of a message
      if ( ( $msgid =~ m/: *(\|.*)?$/ ) != ( $msgstr =~ m/: *(\|.*)?$/ ) ) {
        print "Line $linenum: Missing or unexpected colon:\n  '$msgid' => '$msgstr'\n"
          unless $only_total;
        ++$bad{"Bad colons"};
        $warn++;
      }
    }

    if ($check_periods) {
      # Check period at the end of a message; uncomment code if you are paranoid
      if ( ( $msgid =~ m/\. *(\|.*)?$/ ) != ( $msgstr =~ m/\. *(\|.*)?$/ ) ) {
       print "Line $linenum: Missing or unexpected period:\n  '$msgid' => '$msgstr'\n"
        unless $only_total;
      ++$bad{"Bad periods"};
       $warn++;
      }
    }

    if ($check_spaces) {
      # Check space at the end of a message
      if ( ( $msgid =~ m/  *?(\|.*)?$/ ) != ( $msgstr =~ m/  *?(\|.*)?$/ ) ) {
        print "Line $linenum: Missing or unexpected space:\n  '$msgid' => '$msgstr'\n"
          unless $only_total;
        ++$bad{"Bad spaces"};
        $warn++;
      }
    }

    if ($check_qt) {
      # Check for "&" shortcuts
      if ( ( $msgid =~ m/&[^ ]/ ) != ( $msgstr =~ m/&[^ ]/ ) ) {
        print "Line $linenum: Missing or unexpected Qt shortcut:\n  '$msgid' => '$msgstr'\n"
          unless $only_total;
        ++$bad{"Bad Qt shortcuts"};
        $warn++;
      }
    }

    if ($check_menu) {
      # Check for "|..." shortcuts
      if ( ( $msgid =~ m/\|[^ ]/ ) != ( $msgstr =~ m/\|[^ ]/ ) ) {
        print "Line $linenum: Missing or unexpected menu shortcut:\n  '$msgid' => '$msgstr'\n"
          unless $only_total;
        ++$bad{"Bad menu shortcuts"};
        $warn++;
      }
    }
    
    next unless $check_trans;
    
    # we now collect these translations in a hash.
    # this will allow us to check below if we have translated
    # anything more than one way.
    my $msgid_clean  = lc($msgid);
    my $msgstr_clean = lc($msgstr);

    $msgid_clean  =~ s/(.*)\|.*?$/$1/;  # strip menu shortcuts
    $msgstr_clean =~ s/(.*)\|.*?$/$1/;
    $msgid_clean  =~ s/&([^ ])/$1/;     # strip Qt shortcuts
    $msgstr_clean =~ s/&([^ ])/$1/;

    # this is a hash of hashes. the keys of the outer hash are
    # cleaned versions of ORIGINAL strings. the keys of the inner hash 
    # are the cleaned versions of their TRANSLATIONS. The value for the 
    # inner hash is an array of the orignal string and translation.
    $trans{$msgid_clean}{$msgstr_clean} = [ $msgid, $msgstr, $linenum ];
  }

  if ($check_trans) {
    foreach $msgid ( keys %trans ) {
      # so $ref is a reference to the inner hash.
      my $ref = $trans{$msgid};
      # @msgstrkeys is an array of the keys of that inner hash.
      my @msgstrkeys = keys %$ref;

      # do we have more than one such key?
      if ( $#msgstrkeys > 0 ) {
        if (!$only_total) {
          print "Different translations for '$msgid':\n";
          foreach $msgstr ( @msgstrkeys ) {
            print "Line $ref->{$msgstr}[2]: '" . 
              $ref->{$msgstr}[0] . "' => '" . 
              $ref->{$msgstr}[1] . "'\n";
          }
        }
        ++$bad{"Inconsistent translations"};
        $warn++;
      }
    }
  }
  if (!$silent_mode) {
    if ($warn) {
      while (my ($k, $v) = each %bad) { print "$k: $v\n"; }
      if (scalar(keys %bad) > 1) {
        print "Total warnings: $warn\n";
      }
    } else {
      print "No warnings!\n";
    }
    print "\n";
  }
}
