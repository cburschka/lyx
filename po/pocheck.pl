#! /usr/bin/perl -w

# file pocheck.pl
#
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author: Michael Schmitt, michael.schmitt@teststep.org
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
    if ( ( $msgid ) = ( $pofile[$i] =~ m/^msgid "(.*)"/ ) ) {
      $i++;
      while ( ( $more ) = $pofile[$i] =~ m/^"(.*)"/ ) {
        $msgid = $msgid . $more;
        $i++;
      }

      until ( ( $msgstr ) = ( $pofile[$i] =~ m/^msgstr "(.*)"/ ) ) { $i++; };
      $i++;
      while ( ( $i <= $noOfLines ) &&
              ( ( $more ) = $pofile[$i] =~ m/^"(.*)"/ ) ) {
        $msgstr = $msgstr . $more;
        $i++;
      }

      if ( $msgid ne "" && $msgstr ne "" ) {

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

        # Check for "|..." shortcut(s)
        if ( ( $msgid =~ m/\|[^ ]/ ) != ( $msgstr =~ m/\|[^ ]/ ) ) {
          print( "Missing or unexpected xforms shortcut:\n" );
          print( "  '$msgid' => '$msgstr'\n" );
          $warn++;
        }

        $msgid_clean  = lc($msgid);
        $msgstr_clean = lc($msgstr);

        $msgid_clean  =~ s/(.*)\|.*?$/$1/;  # strip xforms shortcuts
        $msgstr_clean =~ s/(.*)\|.*?$/$1/;
        $msgid_clean  =~ s/&([^ ])/$1/;     # strip Qt shortcuts
        $msgstr_clean =~ s/&([^ ])/$1/;

        $trans{$msgid_clean}{$msgstr_clean} = [ $msgid, $msgstr ];
      }
    } else {
      $i++;
    }
  }

  foreach $msgid ( keys %trans ) {
    $ref = $trans{$msgid};
    @msgstrkeys = keys %$ref;

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
