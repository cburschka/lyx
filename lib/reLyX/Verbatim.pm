#######################  VERBATIM COPYING SUBROUTINES  ########################
# This file is part of reLyX. 
# Copyright (c) 1998-9 Amir Karger karger@post.harvard.edu
# You are free to use and modify this code under the terms of
# the GNU General Public Licence version 2 or later.
#
# Subs for copying stuff verbatim from a TeX file instead of parsing it.
# These subs use the rather low-level TT:OpenFile::paragraph method, because
# the higher level methods assume you've parsed, and verbatim stuff might be
# parsed normally.

package Verbatim;
use strict;

sub copy_verb {
# This subroutine handles a \verb token. Text is guaranteed to be on one line.
# \verb must be followed by a non-letter, then copy anything until the next
# occurrence of that character.
    my ($fileobject, $token) = @_;
    my $verb = $token->exact_print; # "\verb" or "\verb*"
    my $textref = $fileobject->paragraph;
    # eat e.g., !text $\% text!
    if ($$textref =~ s/^([^A-Za-z*]).*?\1//) {
        $verb .= $&;
    } else { warn "unable to parse \\verb"; $verb .="||" }
    return $verb;
}

sub copy_verbatim {
# This subroutine eats text verbatim until a certain text is reached
# The end text itself is not eaten; this is necessary so that
#    environments are properly nested (otherwise, TeX.pm complains)
# It returns a string containing the text
#
# Arg 0 is the Text::TeX::OpenFile file object, arg 1 is the beginning token
    my $fileobject = shift;
    my $begin_token = shift;
    my %endtokentbl = (  '\(' => '\)' , '\[' => '\]'  );

    my $type = ref($begin_token);
    $type =~ s/^Text::TeX:://o or die "unknown token type $type?!";

# Figure out beginning & end text of this token or environment
# Beginning text so we know if you have an environment nested within itself
# End text so we know when to finish copying OR when to 'pop' a level
#    if an environment is nested within itself
# Because the searches will generally be matching expressions with backslashes
#    and other meta-characters, we put \Q\E around (pieces of) the expressions
    my ($begin_text, $end_text);
    if ($type =~ /^Token$/) {  # \( or \[
	$begin_text = $begin_token->print; # e.g., '\('
        die "unknown begin_text" unless exists $endtokentbl{$begin_text};
	$end_text = "\Q$endtokentbl{$begin_text}\E";
	# actually, begin_text shouldn't be nec. since you can't nest math
	$begin_text = "\Q$begin_text\E"; # quote slashes, etc.

    } elsif (/^Begin::Group::Args$/) {    # \begin{foo}
	# \s* to allow, e.g., '\begin {foo}'
	$begin_text = $begin_token->print;
	$begin_text = "\Q$begin_text\E";
	$begin_text =~ s/begin/begin\\s*/;
        ($end_text = $begin_text) =~ s/begin/end/;

    } else {
        die "copy_verbatim called with unknown token type $type!";
    }
    #print "\nsub copy_verbatim going to copy until $end_text\n" if $debug_on;

# Actual copying
    my $textref; # reference to stuff we read in to print
    my $to_print = ""; #text to print
    # we're automatically "nested" once since we had the original \begin
    my $nest_count = 1;

    # (Eat and) Print out paragraphs until you find $end_text
    # paragraph returns "" if it's time to get a new paragraph -- if that
    # happens, we want to continue, but we can't dereference $textref
    #    Call paragraph with an argument so that it gets a new paragraph if
    # it gets to the end of a paragraph
    #    Allow nesting of this environment!
    while (defined ($textref = $fileobject->paragraph(1))) {
	next unless $textref; # new paragraph; keep going

	# If we match begin or end text, eat everything up to it
	# Make sure to handle (nested) begin & end texts in order, so we can
	#    differentiate \begin \begin \end \end from \begin \end \begin \end
	if ($$textref =~ /$end_text/ && $` !~ /$begin_text/) {
	    # Note that $` will be from the last *successful* match,
	    #    namely the end_text match
	    --$nest_count;
	    $to_print .= $`; # print until the \end command
	    if ($nest_count) {
		$to_print .= $&; # print the end text too
		$$textref = $'; # leave the rest in the paragraph
	    } else {
		# Leave end text (and anything after it) for TeX.pm
		$$textref = $& . $';
		last; # done copying since there's no more nesting
	    }

	# If we match beginning text, we have a nested environment
	} elsif ($$textref =~ /$begin_text/ && $` !~ /$end_text/) {
	    $to_print .= $`; # print up to and
	    $to_print .= $&; # INCLUDING the begin text
	    $$textref = $'; # leave the rest in the paragraph
	    ++$nest_count;

	# If we didn't match begin OR end text, just eat the whole paragraph
	} else {
	    $to_print .= $$textref;
	    $$textref = "";
        } # end if $$textref
    } #end while
    die "eof without finding matching text $end_text" if (!defined $textref);

    # return the string
    #print "Exiting sub copy_verbatim\n" if $debug_on;
    return $to_print;
} # end copy_verbatim

1; # return true to calling routine
