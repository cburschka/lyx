# This file is part of reLyX
# Copyright (c) 1998-9 Amir Karger karger@post.harvard.edu
# You are free to use and modify this code under the terms of
# the GNU General Public Licence version 2 or later.

package LastLyX;
# This package is the last step in creating a LyX file. It:
#  - rejoins the lyx preamble to the rest of the file
#  - adds some information (see below)
#  - determines whether reLyX needs to translate any other files (e.g.,
#    files included in an \include command)
#  - creates a '.lyx' file
#
# reLyX may not have enough information during the previous pass. In that case,
# it puts a "marker" in the temporary file it writes, and stores the missing
# information---when it does come upon it---in a global variable.
# So during this pass, if we see any such markers, we replace them with the
# necessary information. Examples of this include:
# - header information for tables
# - name of the bibliography style file to use

use strict;
use RelyxTable; # handle LaTeX tables
use File::Basename;

my $debug_on; # was -d option given?

sub last_lyx {
# Arg0 is input file name
# Arg1 is output file name (foo.lyx)
# Arg2 is a string containing the entire preamble

    my ($InFileName, $OutFileName, $LyXPreamble) = (shift, shift, shift);
    $debug_on = (defined($main::opt_d) && $main::opt_d);
    my $zzz=$debug_on ? " LyX file ($InFileName --> $OutFileName)\n" :"... ";
    print STDERR "Writing$zzz";
    open (INFILE, "<$InFileName") or die "problem opening $InFileName: $!\n";
    open (OUTFILE,">$OutFileName") or die "problem opening $OutFileName: $!\n";

    # Print the preamble
    print OUTFILE $LyXPreamble;

    # Now print out the rest of the LyX file
    # Some lines have to be changed somewhat
    # Otherwise just print all lines as they appear.
    #    TODO In the future, we could buffer text, and then get rid of extra
    # '\latex default \latex latex' or '\end_deeper \begin_deeper' pieces
    # created by the translator
    while (<INFILE>) {
	if (/$RelyxTable::TableBeginString/o) {
	    # Write out the header information for the table
	    $_ = &print_table;

	} elsif (/$BasicLyX::bibstyle_insert_string/o) {
	    # Replace the "insert bibstyle file here" with the actual file name
	    
	    my $ins = $BasicLyX::bibstyle_insert_string;
	    my $fil = $BasicLyX::bibstyle_file;
	    if ($fil) {
		s/$ins/$fil/;
	    } else {
		warn("Don't know which bibliographystyle file to use!\n".
		 "Replace '$ins' in the LyX file with the bibstyle file\n");
	    }

	} elsif (/^\Q$BasicLyX::Begin_Inset_Include\E/o) {
	    # tell main:: we need to translate an included (or inputted) file
	    m/\{(.*)\}\s*$/ or warn "weird Include command $_";
	    my $fil = $1;
	    # Change relative path to absolute path if necessary
	    my $abs_fil = &main::abs_file_name($fil);
	    print "Adding $abs_fil to file list\n" if $debug_on;
	    push @main::File_List, $abs_fil;

	    # include file.lyx, not file.tex!
	    my ($basename, $path, $suffix)=fileparse($fil, @main::Suffix_List);
	    $suffix = "" unless defined $suffix;
	    $path .= '/' unless $path =~ /\/$/;
	    my $newfile;
	    if ($main::opt_o) { # all files go to outputdir; no path nec.
		$newfile = "$basename.lyx";
	    } else { # keep relative path, e.g. Just change suffix
		($newfile = $fil) =~ s/$suffix/.lyx/;
	    }
	    s/\Q{$fil}\E/{$newfile}/;
	} # end special if for table, bibstyle, include

	print OUTFILE $_;
    }

    close INFILE; close OUTFILE;
    #warn "Done writing LyX file!\n";
} # end sub last_lyx

sub print_table {
# Print a table, from TableBeginString to TableEndString
# Also (kind of a hack) remove the last \newline in a table, if any,
#    since it causes LyX to seg fault.
    my $to_print=""; # string to collect the table in
    my $thistable = shift(@RelyxTable::table_array);

    # Collect the whole table in $to_print
    my $line;
    while (($line = <INFILE>) !~ /$RelyxTable::TableEndString/o) {
        $to_print .= $line;
    }

    # Remove the last \newline, if it was created by a \\ \hline
    # at the end of a table. (Note: according to Lamport, \\ at the end of
    # a table is *illegal* unless followed by an \hline)
    #    If it was created due to an empty cell at the end of the table, though
    # (latex table "a & b \\ c &", e.g.) then we need to keep it!
    # HACK HACK HACK. If it's a one-column table, then we couldn't
    # know while reading the table that the last row was empty, so we
    # couldn't pop the last row then. So do it now. Yuck.
    if ($thistable->numcols==1) {
	$to_print =~ s/\\newline(?=\s*$)// && pop @{$thistable->{"rows"}} 
    } elsif ($thistable->{"rows"}[$thistable->numrows -1]->{"bottom_line"}) {
	$to_print =~ s/\\newline(?=\s*$)//;
    }

    # Put the header information at the top
    # We have to do this *after* reading the table because of the
    # one-column table hack. Yuck.
    $to_print = $thistable->print_info . $to_print;

    return $to_print;
} # end sub print_table

1; # return true to main package
