#!/usr/bin/perl -w
#
# Main code for reLyX - the LaTeX to LyX translator
#
# reLyX is Copyright (c) 1998-9 Amir Karger karger@post.harvard.edu
# You are free to use and modify this code under the terms of
# the GNU General Public Licence version 2 or later.
#
# This code usually gets called by the reLyX wrapper executable
#
# $Id: reLyXmain.pl,v 1.4 2001/08/31 07:54:05 jamatos Exp $
#

require 5.002; # Perl 5.001 doesn't work. Perl 4 REALLY doesn't work.

# Standard Perl Library modules
use strict; # must define all variables, no barewords, no hard references

# Variables from other files, like Getopt::Std, OR vars that need to be global
# $opt_f etc. - command line options set by Getop::Std::getopts
# $dot_lyxdir - the user's personal .lyx directory
# $Success    - keep track of whether reLyX finished successfully or died
# @File_List  - the list of files to translate. Initially this is just
#               the file input by the user, but \input or \include commands
#               will add to it.
use vars qw($opt_c $opt_d $opt_f $opt_h $opt_n $opt_o $opt_p $opt_r $opt_s
            $dot_lyxdir
            $Success
            @File_List
            @Suffix_List
            $LyXFormat
	   );
use Cwd; # getcwd etc.
use Getopt::Std; # read in command-line options
use File::Basename; # &basename and &dirname

# Modules containing subroutines for reLyX
# Note that @INC must include the directory that these modules are in. The
# reLyX wrapper takes care of that.
use Text::TeX; # TeX parser package
use ReadCommands; # package to read LaTeX commands' syntax from a file
use MakePreamble; # package to split off LaTeX preamble & translate it to LyX
use CleanTeX; # package to clean TeX file for Lyxifying
use BasicLyX; # package to translate clean TeX to Basic LyX
use LastLyX; # package to print out LyX file once all translation is done


# Hack to allow running reLyX without the wrapper
if (!defined($lyxdir)) {$lyxdir = "/usr/local/share/lyx"}
if (!defined($lyxname)) {$lyxname = "lyx"}
# there's a use vars in the wrapper, so we only need these if running w/out it
use vars qw($lyxdir $lyxname);
 
# variables that a user might want to change
@Suffix_List = '\.(ltx|latex|tex)'; # allowed suffixes for LaTeX file
$LyXFormat = "2.15"; #What to print in \lyxformat command in .lyx file
my $syntaxname = "syntax.default"; # name of the default syntax file
$dot_lyxdir = $ENV{'HOME'} . "/.$lyxname"; # personal .lyx directory

# This variable tells us if the program died or exited happily
BEGIN{$Success = 0}

##############################################################################
# MAIN
#

# Print welcome message including version info
my $version_info = '$Date: 2001/08/31 07:54:05 $'; # RCS puts checkin date here
$version_info =~ s&.*?(\d+/\d+/\d+).*&$1&; # take out just the date info
warn "reLyX, the LaTeX to LyX translator. Revision date $version_info\n\n";

# Usage information
my $Usage_Short = <<"ENDSHORTUSAGE";


USAGE:
$0 [ -c textclass ] [ -fd ] [ -o outputdir ]
         [ -r renv1[,renv2...]] [ -s sfile1[,sfile2...]] inputfile

$0 -p -c textclass [ -fd ] [ -o outputdir ]
         [ -r renv1[,renv2...]] [ -s sfile1[,sfile2...]] inputfile(s)

$0 -h   (to get more usage information)

ENDSHORTUSAGE

my $Usage_Long = $Usage_Short . <<"ENDLONGUSAGE";
    -c    which textclass this file is (required with -p)
             Overrides \\documentclass command, if one exists
    -d    print lots of debug information & save temporary files
    -f    force destruction of existing lyx file or temporary files
    -h    print this message and quit
    -n    convert a noweb file to a literate document (requires -c)
    -o    output all LyX files to directory "outputdir"
             Otherwise, LyX file is created in directory the LaTeX file is in
    -p    translate LaTeX fragments or include files (requires -c)
             I.e., files without \\documentclass commands
    -r    give reLyX a (list of) regular environment(s)
    -s    give reLyX a (list of) additional syntax file(s) to read

man reLyX for lots of usage information



ENDLONGUSAGE

# Read command line
# Get Options: set $opt_f etc. based on command line options
getopts('c:dfhno:pr:s:') or die "Illegal option!$Usage_Short";
if ($opt_h) {print $Usage_Long; $Success=1; exit}
die "No LaTeX file was input on the command line$Usage_Short" unless @ARGV;

# Make each file in the file list an absolute file name (e.g., staring with '/')
@File_List = map {&abs_file_name($_)} @ARGV;

# If noweb files, then we need to pre-process each file
if (defined $opt_n) {
    die "-n option requires -c!$Usage_Short" unless defined $opt_c;
    foreach (@File_List) {
        system("noweb2lyx", "-pre", "$_", "$_.pre$$");
        $_ .= ".pre$$";
    }
}

# -p option allows multiple input files
if (defined $opt_p) {
    die "-p option requires -c!$Usage_Short" unless defined $opt_c;
} else {
    die "Only one input file allowed unless using -p option$Usage_Short"
                    unless @ARGV == 1;
}

# Make sure outputdir given with -o option is valid
# Make it an absolute path, too!
if (defined($opt_o)) {
    die "directory $opt_o doesn't exist!\n$Usage_Short"
                                 unless defined(-d $opt_o);
    die "$opt_o isn't a directory!\n$Usage_Short" unless -d $opt_o;
    $opt_o = &my_fast_abs_path($opt_o);
    # may not have trailing slash.
    $opt_o .= '/' unless $opt_o =~ /\/$/;
}

# Read file(s) containing LaTeX commands and their syntax
# Read personal syntax.default, or system-wide if there isn't a personal one
# Then read other syntax files, given by the -s option
my $default_file = "$dot_lyxdir/reLyX/$syntaxname";
if (! -e $default_file) {
    $default_file = "$lyxdir/reLyX/$syntaxname";
    die "cannot find default syntax file $default_file" unless -e $default_file;
}
my @syntaxfiles = ($default_file);
push (@syntaxfiles, (split(/,/,$opt_s))) if defined $opt_s;
&ReadCommands::read_syntax_files(@syntaxfiles);

########### Main loop over files (include files will be added to @File_List)
my $count=0; #number of files we've done
my @deletelist = (); # files to delete when we're done (if no '-d' flag)
my $Doc_Class; # LaTeX documentclass
my $LyX_Preamble = "";   # LyX Preamble not including Latex preamble part
my $Latex_Preamble = "";
my $File;
while ($File = shift(@File_List)) {
    # TODO we should always die (or something) if temp files exist & !opt_f
    my $OutFileName;
    my $filenum = 0; #numbering for temporary files

    # May need to add ".tex" to input file name.
    #    PathBase is the output file name without ".lyx". It's used for building
    # temporary files' file names, too.
    #    Finally, make sure the file is valid, directory is writable, etc.
    # Sub returns (undef, undef, undef) if something goes wrong.
    my ($InFileDir, $InFileName, $PathBase) = &test_file($File);

    # Change to the input file's directory, so that if the input file has an
    # \include{../foo} in it, we'll find the included file.
    # Did something go wrong?
    unless (defined($InFileDir) && chdir($InFileDir)) {
	# main file must be ok; for included files or file fragments, just warn
	next if ($count || $opt_p);
	die "\n"; # printed error already
    }

    # OK. Start parsing the file!
    print STDERR "In Directory $InFileDir\n" if $opt_d;
    print STDERR "($InFileName: ";

    # Read preamble and calculate document class if necessary
    # (It's necessary when we're doing the first file.)
    unless ($count) {

	if ($opt_p) { # it's a partial file
	    $Doc_Class = $opt_c;
	} else {
	    # Split the preamble off of the rest of the file
	    my $PreambleName = $PathBase . ".relyx" . ++$filenum;
	    $OutFileName = $PathBase . ".relyx" . ++$filenum;
	    push @deletelist, $OutFileName, $PreambleName;
	    &MakePreamble::split_preamble($InFileName,
	                            $PreambleName, $OutFileName);
	    $InFileName = $OutFileName;

	    # Now read and translate the LaTeX preamble into LyX
	    # Return document's class so we know which layout file(s) to read
	    # Also return LyX preamble in a string, for later
	    ($Doc_Class, $LyX_Preamble, $Latex_Preamble) =
		 &MakePreamble::translate_preamble($PreambleName, $LyXFormat);

	} # end partial file if

	# Read file(s) containing the valid LyX layouts for this documentclass
	# and their LaTeX command/environment equivalents
	&ReadCommands::read_layout_files($Doc_Class);
    }

    # Clean the TeX file (not including its preamble)
    $OutFileName = $PathBase . ".relyx" . ++$filenum;
    push @deletelist, $OutFileName;
    &CleanTeX::call_parser($InFileName, $OutFileName);

    # Now convert basic constructs in the cleaned TeX file to LyX constructs
    $InFileName = $OutFileName;
    $OutFileName = $PathBase . ".relyx" . ++$filenum;
    push @deletelist, $OutFileName;
    &BasicLyX::call_parser($InFileName, $OutFileName);

    # Final cleanup step for noweb files
    if (defined $opt_n) {
        $InFileName = $OutFileName;
        $OutFileName = $PathBase . ".relyx" . ++$filenum;
        push @deletelist, $OutFileName;
        system("noweb2lyx", "-post", $InFileName, $OutFileName);
    }

    # Finally, print out the actual LyX file including the preamble
    # For the *first* file, print out the LaTeX preamble too
    $InFileName = $OutFileName;
    if (defined $opt_n) {
        push @deletelist, $PathBase;
        $PathBase =~ s/.pre$$//;
    }
    $OutFileName = $PathBase . ".lyx";
    my $preamble = $count ? $LyX_Preamble : $LyX_Preamble . $Latex_Preamble;
    &LastLyX::last_lyx($InFileName, $OutFileName, $preamble);

    warn ")\n";

} continue {
    $count++;

} # END MAIN WHILE LOOP

# Cleanup
unless ($opt_d) {
    warn "Deleting temp files\n";
    unlink @deletelist;
}

$Success = 1;
exit;

# If we "die", output a sad message
END{
    if ($Success) {
	warn "Finished successfully!\n";
    } else {
	warn "Exited due to fatal Error!\n";
    }
}

##################### SUBROUTINES  ###########################################
# Input: File (including absolute path)
# Output: input file directory (absolute path)
#         input file name (".tex" added if necessary) without path
#         PathBase, the output file name (including path) without ".lyx" ending
# Returns (undef, undef, undef) if something breaks
#
# Only allow certain suffixes
#    Test for things, like writability of output directory, existence of
# .lyx file we would be creating...
sub test_file {
    my $File = shift;
    my @return_error = (undef, undef, undef);

    # Get file names, set up for making different temporary files
    # fileparse_set_fstype("MSDOS") for DOS support!
    my ($in_basename, $in_path, $suffix) = fileparse($File, @Suffix_List);
    #$path .= '/' unless $path =~ /\/$/; # fix BUG in perl5.002 fileparse!

    # Try adding .tex to filename if you can't find the file the user input
    unless (-e $File) {
        if (! $suffix) { # didn't have a valid suffix. Try adding one
	    if (-e "$File.tex") {
		$suffix = ".tex";
	    } else {
	 	warn "\nCan't find input file $File or $File.tex\n";
		return @return_error;
	    }
	} else { # it had a valid suffix, but the file doesn't exist
	    warn "\nCan't find input file $File\n";
	    return @return_error;
	}
    }
    my $in_filename = $in_basename . $suffix;

    # Make sure directory is valid
    #    Note that we chdir to an input file's directory before translating it.
    # Therefore, unless the -o option is given, we want to output files in '.'
    #
    # TODO if file foo.tex includes a/bar.tex and b/bar.tex, then with the
    # -o option things will get ugly. We could test for that and create a name
    # (like relyx-1-12345-bar.tex) for the second (and later!) bar.tex file(s)
    my $out_path = defined $opt_o ? $opt_o : "./";
    unless (-w $out_path) { # Note: "" isn't writable!
	warn "\nDirectory $out_path isn't writable!\n";
	return @return_error;
    }
    $out_path =~ s(^./)(); # "foo" is more readable than "./foo"
    # This will be used for creating LyX file as well as temp files
    my $PathBase = $out_path . $in_basename;

    # Check for files that already exist
    my $lname = $PathBase . ".lyx";
    if (-e $lname) {
	if ($opt_f) {
	    warn "Will overwrite file $lname\n" if $opt_d;
	} else {
	    warn "\nLyX file $lname already exists. Use -f to overwrite\n";
	    return @return_error;
	}
    }

    return ($in_path, $in_filename, $PathBase);
} # end sub test_file


sub abs_file_name {
    my $File = shift;
    my ($basename, $path, $suffix) = fileparse($File, @Suffix_List);
    my $realpath = &my_fast_abs_path($path);
    # add / at end
    $realpath .= '/' unless $realpath =~ /\/$/; 
    my $name = "$realpath$basename$suffix";
    return $name;
}


# Stole this from Cwd.pm. Can't use the Cwd:: function cuz it's not in 5.003
# I could test $] >5.004 or something, but why bother?
sub my_fast_abs_path {
    my $cwd = fastcwd();
    my $path = shift || '.';
    chdir($path) || die "Cannot chdir to $path:$!";
    my $realpath = fastcwd();
    chdir($cwd)  || die "Cannot chdir back to $cwd:$!";
    $realpath;
}


1; # return "true" to the wrapper
