# This file is part of reLyX. 
# Copyright (c) 1998-9 Amir Karger karger@post.harvard.edu
# You are free to use and modify this code under the terms of
# the GNU General Public Licence version 2 or later.

package MakePreamble;
# This package reads a LaTeX preamble (everything before \begin{document}
#    and translates it to LaTeX.

use strict;

my $debug_on; # package-wide variable set if -d option is given
my $true_class; # which textclass to use (if -c option is given)

sub split_preamble {
# Split the file into two files, one with just the preamble
# Simply copy stuff verbatim, saving translation for later
# Skip the LyX-generated portion of the preamble, if any. (Otherwise,
#     when you try to "view dvi" in LyX, there will be two copies of the
#     preamble matter, which will break LyX.
    my ($InFileName, $PreambleName, $OutFileName) = (shift, shift, shift);
    my ($a, $b, $c);
    my $forget=0;

    # Was -d option given?
    $debug_on = (defined($main::opt_d) && $main::opt_d);
    # -c option?
    $true_class = defined($main::opt_c) ? $main::opt_c : "";
    my $zzz = $debug_on 
        ? " from LaTeX file $InFileName into $PreambleName and $OutFileName" 
	: "";
    warn "Splitting Preamble$zzz\n";

    open (INFILE, "<$InFileName") or die "problem opening $InFileName: $!\n";
    open (PREAMBLE, ">$PreambleName") or
                die "problem opening $PreambleName: $!\n";
    open (OUTFILE, ">$OutFileName") or die "problem opening $OutFileName: $!\n";

    # Copy everything up to "\begin{document}" into the preamble
    while (<INFILE>) {
        if (s/\Q\begin{document}\E//) {
	    ($a, $b, $c) = ($`, $&, $'); # save for later
	    last;
	}
	# In real life, there should never be Textclass specific stuff unless
	# there is also LyX specific. But it can't hurt to test for both.
        $forget=1 if m/%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% LyX specific/ ||
                     m/%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% Textclass specific/;
        print PREAMBLE $_ unless $forget;
        $forget=0 if m/%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% User specified/;
    }
    die "Didn't find \\begin{document} command!" unless defined $b;
    # There *could* be a preamble command on the same line as \begin{doc}!
    print PREAMBLE $a;
    print "Put preamble into file $PreambleName\n" if $debug_on;

    # Copy everything else into another file
    # Ignore anything after '\end{document}
    print OUTFILE $b, $c; #'\begin{document}' plus anything else on the line
    while (<INFILE>) {
        print OUTFILE $_;
	last if $_ =~ /^[^%]*\Q\end{document}\E/;
    }

    print "Put rest of file into $OutFileName\n" if $debug_on;
    close INFILE; close OUTFILE; close PREAMBLE;
    #warn "Done splitting preamble\n";
} #end sub split_preamble



# Actually translate a LaTeX preamble (in a file) into a LyX preamble.
# Input:
# First argument is the name of the file containing *only* the preamble
# Second argument is the LyX format (e.g., "2.15")
#
# Output: document class, LyX Preamble, LaTeX preamble
#    LyX premable contains the LyX options etc.
#    LaTeX preamble is stuff that doesn't translate into LyX options)
#
sub translate_preamble {
    my $PreambleName = shift;
    my $Format = shift;
    $debug_on = (defined($main::opt_d) && $main::opt_d);
    my $zzz=$debug_on ? " from $PreambleName" :"";
    warn "Creating LyX preamble$zzz\n";
    open (PREAMBLE, "<$PreambleName");

    # Translate optional args to \documentclass to LyX preamble statements
    my %Option_Trans_Table = (
        "10pt"           => "\\paperfontsize 10",
        "11pt"           => "\\paperfontsize 11",
        "12pt"           => "\\paperfontsize 12",

	"letterpaper"    => "\\papersize letterpaper",
	"legalpaper"     => "\\papersize legalpaper",
	"executivepaper" => "\\papersize executivepaper",
	"a4paper"        => "\\papersize a4paper",
	"a5paper"        => "\\papersize a5paper",
	"b5paper"        => "\\papersize b5paper",

	"twoside"        => "\\papersides 2",
	"oneside"        => "\\papersides 1",

	"landscape"      => "\\paperorientation landscape",

	"onecolumn"      => "\\papercolumns 1",
	"twocolumn"      => "\\papercolumns 2",
    );

    # This is the string in which we're concatenating everything we translate
    my $LyX_Preamble = "";

# It would be nice to read the defaults.lyx file but for now we don't bother
#my $default_dir = "templates/";
#my $default_name = "defaults.lyx";
#my $default_file = $dot_lyxdir . $default_dir . $default_name;
#if (-e $default_file) {
#    print "Going to open default LyX file $default_file\n";
#    open (DEFAULT_LYX,$default_file) or warn "can't find default lyx file!";
#	if ($fmt =~ /^\s*\\lyxformat [0-9.]+/) {
#	    print $fmt
#	} else {
#	    die "Need \\lyxformat command in first line of default lyx file!";
#	}
#       AFTER printing the preamble, we'd print other commands from defaults.lyx
#	while (($line = <DEFAULT_LYX>) !~ /^.*\\layout/) {
#	    if ($line !~ /^\s*(\#|\\textclass\W)/) { #already printed this line
#		print OUTFILE $line;
#	    }
#	}
#}

    # Write first two lines of the lyx file
    my ($nm, $dt);
    # whoami is needed on Win32, because getlong/getpwuid aren't implemented
    # I'd rather use internal Perl functions when possible, though
    $nm = (eval {getlogin || getpwuid($<)}) || `whoami`;
    chomp($nm); # whomai returns "foo\n"
    $dt = localtime;
    $LyX_Preamble .= "\#This file was created by <$nm> $dt\n";
    $LyX_Preamble .= "\#LyX 1.0 (C) 1995-1999 Matthias Ettrich " .
                    "and the LyX Team\n";

    # Print \lyxformat.
    $LyX_Preamble .= "\\lyxformat $Format\n";

    # Ignore everything up to the \documentclass
    my $ignore = "";
    while (<PREAMBLE>) {
        next if /^\s*$/ || /^\s*%/; # skip whitespace or comment
	# could just allow 'documentclass' and not support 2.09
	last if s/^\s*\\document(style|class)//;
	$ignore .= $_; # concatenate the ignored text
    } # end while
    warn "Uncommented text before \\documentclass command ignored!\n"if $ignore;
    print "Ignored text was\n------\n$ignore------\n" if $debug_on && $ignore;

    # concatenate all the extra options until the required argument to 
    # \documentclass, which will be in braces
    until (eof(PREAMBLE) || /\{/) {
        my $instr = <PREAMBLE>;
	$instr =~ s/\s*%.*$//; # get rid of comments
	chomp; # remove the \n on $_ prior to concatenating
	$_ .= $instr;
    } # end loop; check the now longer $_ for a brace

    # Read optional arguments, which are now guaranteed to be on one line
    # (They're read here, but printed AFTER the \\textclass command)
    #    Note: the below pattern match is *always* successful, so $& is always
    # set, but it will be '' if there are no optional arguments. $1 will be
    # whatever was inside the brackets (i.e., not including the [])
    my $extra_options;
    if (defined($_)) {
	s/^\s*//; # there might be space before args
	s/$Text::TeX::optionalArgument//;
	$extra_options = $1 if $&;
    }

    # Read the document class, in braces, then convert it to a textclass
    # However, if the user input a different class with the -c option, use that
    s/\s*\{(\w+)\}//;
    my $class = $1;
    $class = $true_class if $true_class; # override from -c option
    die "no document class in file, no -c option given\n" unless $class;
    $LyX_Preamble .= "\\textclass $class\n";
    print "document class is $class\n" if $debug_on;

    # Analyze the extra options, and remove those we know about
    if ($extra_options) {
	my $op;
	foreach $op (keys %Option_Trans_Table) {
	    $extra_options =~ s/\b$op\b// && do {
	        $LyX_Preamble .= "$Option_Trans_Table{$op}\n";
		print "Document option $op\n" if $debug_on;
	    }
	}
	$extra_options =~ s/^,+|,+(?=,)|,+$//g; # extra commas
    }
    # Convert any remaining options to an \options command
    if ($extra_options) {
	$LyX_Preamble .= "\\options $extra_options\n";
	print "extra options: $extra_options\n" if $debug_on;
    }

    # Now copy the whole preamble into the preamble of the LyX document
    #     (unless there is no preamble)
    # Everything until the end of filehandle PREAMBLE is preamble matter
    my $Latex_Preamble = $_; # there COULD be a preamble command on same line
    my $write_preamble = (! /^\s*$/ && ! /^\s*%/);
    while (<PREAMBLE>) {
	$Latex_Preamble .= $_;
	# write an official preamble if we find anything that's not comment
	$write_preamble ||= (! /^\s*$/ && ! /^\s*%/);
    }

    if ($write_preamble) {
	$Latex_Preamble =~ s/^\s*//;
	print "LaTeX preamble, consists of:\n$Latex_Preamble" if $debug_on;
	$Latex_Preamble = "\\begin_preamble\n$Latex_Preamble\\end_preamble\n";
	print "End of LaTeX preamble\n" if $debug_on;
    } else {
        $Latex_Preamble = ""; #just comments, whitespace. Ignore them
    }

    #warn "Done creating LyX Preamble\n";
    return ($class, $LyX_Preamble, $Latex_Preamble);
}

1; # return true value to calling program
