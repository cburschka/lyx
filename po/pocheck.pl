#! /usr/bin/perl -w

# file pocheck.pl
#
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author: Michael Gerz, michael.gerz@teststep.org
#
# This script performs some consistency checks on po files:
#
#   1. Uniform translation of messages that are identical except
#      for capitalization, shortcuts, and shortcut notation.
#   2. Usage of the following elements in both the original and
#      the translated message (or no usage at all):
#      shortcuts ("&" and "|..."), trailing space, trailing colon
#
# Invocation:
#    pocheck.pl po_file po_file ...

foreach $pofilename ( @ARGV )
{
  print "Processing po file '$pofilename'...\n";

  open( INPUT, "<$pofilename" )
    || die "Cannot read po file '$pofilename'";
  @pofile = <INPUT>;
  close( INPUT );

  undef( %trans );
  keys( %trans ) = 10000;

  $noOfLines = $#pofile;

  $warn = 0;

  $i = 0;
  while ($i <= $noOfLines) {
    ( $msgid ) = ( $pofile[$i] =~ m/^msgid "(.*)"/ );
    $i++;
    next unless $msgid;
    
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
    @argstrs = ( $msgid =~ m/%(\d)\$s/g );
    if (@argstrs) {
      $num = 0;
      foreach $arg (@argstrs) { $num = $arg if $arg > $num; }
      if ($num <= 0) { 
        print "Problem finding arguments in:\n    $msgid!\n";
        $warn++;
      } else {
        foreach $i (1..$num) {
          $arg = "%$i\\\$s"; 
          if ( $msgstr !~ m/$arg/ ) {
            print "Missing argument `$arg'\n  '$msgid' ==> '$msgstr'\n";
            $warn++;
          }
        }
      }
    }

    # Check colon at the end of a message
    if ( ( $msgid =~ m/: *(\|.*)?$/ ) != ( $msgstr =~ m/: *(\|.*)?$/ ) ) {
      print( "Missing or unexpected colon:\n" );
      print( "  '$msgid' => '$msgstr'\n" );
      $warn++;
    }

    # Check period at the end of a message; uncomment code if you are paranoid
    #if ( ( $msgid =~ m/\. *(\|.*)?$/ ) != ( $msgstr =~ m/\. *(\|.*)?$/ ) ) {
    #  print( "Missing or unexpected period:\n" );
    #  print( "  '$msgid' => '$msgstr'\n" );
    #  $warn++;
    #}

    # Check space at the end of a message
    if ( ( $msgid =~ m/  *?(\|.*)?$/ ) != ( $msgstr =~ m/  *?(\|.*)?$/ ) ) {
      print( "Missing or unexpected space:\n" );
      print( "  '$msgid' => '$msgstr'\n" );
      $warn++;
    }

    # Check for "&" shortcuts
    if ( ( $msgid =~ m/&[^ ]/ ) != ( $msgstr =~ m/&[^ ]/ ) ) {
      print( "Missing or unexpected Qt shortcut:\n" );
      print( "  '$msgid' => '$msgstr'\n" );
      $warn++;
    }

    # Check for "|..." shortcuts
    if ( ( $msgid =~ m/\|[^ ]/ ) != ( $msgstr =~ m/\|[^ ]/ ) ) {
      print( "Missing or unexpected menu shortcut:\n" );
      print( "  '$msgid' => '$msgstr'\n" );
      $warn++;
    }
    
    # we now collect these translations in a hash.
    # this will allow us to check below if we have translated
    # anything more than one way.
    $msgid_clean  = lc($msgid);
    $msgstr_clean = lc($msgstr);

    $msgid_clean  =~ s/(.*)\|.*?$/$1/;  # strip menu shortcuts
    $msgstr_clean =~ s/(.*)\|.*?$/$1/;
    $msgid_clean  =~ s/&([^ ])/$1/;     # strip Qt shortcuts
    $msgstr_clean =~ s/&([^ ])/$1/;

    # this is a hash of hashes. the keys of the outer hash are
    # cleaned versions of ORIGINAL strings. the keys of the inner hash 
    # are the cleaned versions of their TRANSLATIONS. The value for the 
    # inner hash is an array of the orignal string and translation.
    $trans{$msgid_clean}{$msgstr_clean} = [ $msgid, $msgstr ];
  }

  foreach $msgid ( keys %trans ) {
    # so $ref is a reference to the inner hash.
    $ref = $trans{$msgid};
    # @msgstrkeys is an array of the keys of that inner hash.
    @msgstrkeys = keys %$ref;

    # do we have more than one such key?
    if ( $#msgstrkeys > 0 ) {
      print( "Different translations for '$msgid':\n" );
      foreach $msgstr ( @msgstrkeys ) {
        print( "  '" . $trans{$msgid}{$msgstr}[0] . "' => '" . $trans{$msgid}{$msgstr}[1] . "'\n" );
      }
      $warn++;
    }
  }

  print( "\nTotal number of warnings: $warn\n\n" );
}
