#!/usr/bin/perl -w
# Note! If your perl isn't in /usr/bin/perl, change the line above.
# (Or say "perl Doc_toc.pl" instead of just "Doc_toc.pl")
#
# This script creates a "master table of contents" for a set of LyX docs.
# It does so by going through the files and printing out all of the
# chapter, section, and sub(sub)section headings out. (It numbers the
# sections sequentially; hopefully noone's using Section* in the docs.)

# ./Doc_toc.pl TOC_top/TOC_top.lyx Intro.lyx FAQ.lyx Tutorial.lyx UserGuide.lyx Extended.lyx Customization.lyx  >TOC.lyx 
# The script tries hard to ignore Footnotes, Notes; ERT and labels in section 
# headings. There is even some code to handle nested insets, but no doubt it 
# can break on some files. The emphasis has been put on working with current 
# docs.
 
use strict;

my $Usage = <<ENDUSAGE;
    $0 Toc_Top_File Doc [Doc ...]

    * Docs are LyX docs to be TOC-ified
    * All information up to and including the TOC doc title is in a
      language-specific file Toc_Top_File

ENDUSAGE

my $printing = 0; # are we currently supposed to be copying input to STDOUT?
my @LyX_classes = qw (Chapter Section Subsection Subsubsection);
my $Toc_Top_File = shift;

# Get document header from language-specific TOC_top
open(HEAD, "<$Toc_Top_File") or die $!;
while (<HEAD>) {
    last if /\\the_end/; # don't print last line!
    print;
}
close HEAD;

# loop through files; print out headers
my @matches;
my $level;
my $match = "";
my $sub_match;
my $ignore = 0; # ignore lines (e.g. footnote in a heading)
my @sec_counter;

while (<>) {
    # first few lines of the file
    unless (@matches) {
	if (/^\\textclass (\w+)/) {
	    # Hopefully it's book, report, or article
	    my $class = $1;
	    @matches = @LyX_classes;
	    # Article doesn't have Chapters
	    if ($class eq "article") {
		shift @matches;
	    }
	    $match = $matches[0];
	    $sub_match = $matches[1];
	}

	next;
    }

    # Footnotes in a section heading could confuse things!
    # And don't bother printing out section labels.
    if (/^\\begin_float footnote/ 
        || /^\\begin_inset OptArg/
        || /^\\begin_inset (Foot|Note|ERT|LatexCommand \\label)/) { 
	$ignore++;
    } 
    elsif ($ignore && /^\\begin_inset/ ) {
	$ignore++; 
    }
    # Unset ignoring. But note that end_inset could be the end of another
    # kind of inset!
    if ($ignore && /^\\end_(float|inset)/) {
	$ignore--;
	next; # don't print out \end_float line
    }
    next if $ignore;

    # Now actually handle title & section headings
    if (/^\\layout\s+(\w+)/) {
	my $layout = $1;
	my $found = 0; # did we find the start of a heading?
    
	if ($layout eq "Title") {
	    $found = 1;
	    @sec_counter = scalar(@matches) x (0); # (re)start section counters
	    print "\\layout Section*\n";

	} else {
	    my $level;
	    foreach $level (0 .. $#matches) {
		if ($layout eq $matches[$level]) {

		    # If this is the first subsection of this level,
		    # start nesting. Top level (level 0) needs no nesting
		    if ($level && !$sec_counter[$level]) {
			    print "\\begin_deeper\n";
		    }

		    # A new section ends any sub- or subsubsections
		    # below it, etc.
		    # (Make sure to do this before calling &section_number!
		    my $sublevel;
		    foreach $sublevel ($level+1 .. $#sec_counter) {
			if ($sec_counter[$sublevel]) {
			    print "\\end_deeper\n";
			    $sec_counter[$sublevel] = 0;
			}
		    }

		    $found = 1;
		    $sec_counter[$level]++;
		    my $sec = &section_number (@sec_counter);
		    print "\\layout Description\n$sec ";
		    last; # don't need to try any more matches
		}
	    }
	}

	# If we found "Title" or one of the section layouts, then we're
	# in a section header. Otherwise, this \layout command ENDS any section
	# we might have been in before.
	$printing = $found;

    # Rare case of a section header w/ no text after it at the end of a file
    } elsif (/^\\the_end/) {
	$printing = 0; # obviously we're not in the section header any more!
    } else {
	print if $printing;
    }

    # Cleanup at the end of each file
    if (eof) {
	# Finish nesting subsections
	# I.e., if we had any subsections in this section, end subsectioning
	my $level;
	foreach $level (1 .. $#sec_counter) {
	    if ($sec_counter[$level]) {
		print "\\end_deeper\n";
	    }
	}
	# So that on next file we check whether it's an article or book
	@matches = ();
    }

}

print "\n\\the_end\n";

# Make a (nested) section number
# Input is current section numbers
sub section_number {
    my $number = shift; # Highest level section counter
    my $index;
    foreach $index (@_) {
        if ($index) {
	    $number .= ".$index";
	} else {
	    # Done creating the number
	    last;
	}
    }
    return $number;
}
