# This file is part of reLyX
# Copyright (c) 1998-9 Amir Karger karger@post.harvard.edu
# You are free to use and modify this code under the terms of
# the GNU General Public Licence version 2 or later.

package CleanTeX;
# This package prepares a LaTeX file for translation to LyX
# - Translates some local commands (e.g., {\em blah} to {\emph{blah}})
# - Prepares math mode stuff for LyX. LyX reads LaTeX math mode directly,
#      so reLyX can basically copy all math mode exactly, but LyX is a
#      bit stricter than LaTeX. E.g., translate 'x^2' -> 'x^{2}
# - Removes optional arguments if LyX doesn't understand them, e.g. \\

use strict;

use Verbatim;

######
# Global variables
my $last_eaten; # last token we ate

# List of commands for which LyX doesn't support the optional argument
my @DeleteOptArg = map {"\\$_"} qw(\\ \\*
              chapter section subsection subsubsection paragraph subparagraph
	      );

my $debug_on; # was -d option given?

#########################   PARSER INVOCATION   ################################
sub call_parser {
# This subroutine opens the TeX parser and processes the file.
# Arg0 is the name of the input TeX file
# Arg1 is the name of the output "clean" file

    my ($InFileName, $OutFileName) = (shift,shift);

    $debug_on = (defined($main::opt_d) && $main::opt_d);
    my $zzz=$debug_on ? " TeX file ($InFileName --> $OutFileName)\n" :"... ";
    print STDERR "Cleaning$zzz";
    open (OUTFILE, ">$OutFileName") or die "problem opening $OutFileName: $!\n";

# Create the list of tokens for the parser
# Parts of the token list are swiped from TeX.pm
    my %MyTokens = ( '{' => $Text::TeX::Tokens{'{'},
		     '}' => $Text::TeX::Tokens{'}'},
		     '$' => $Text::TeX::Tokens{'$'},
		     '$$' => $Text::TeX::Tokens{'$$'},
		     '\begin' => $Text::TeX::Tokens{'\begin'},
		     '\end' => $Text::TeX::Tokens{'\end'},
		   );

    # Put local tokens, like \em, into %MyTokens
    #Note: \cal is "local", although it's found in math mode
    # (The "map" just puts a backslash in front of each word in the list)
    my @LocalTokens = qw (em rm bf tt sf sc sl it
			 rmfamily ttfamily sffamily mdseries bfseries
			upshape itshape slshape scshape cal
			); 
    foreach (@LocalTokens) {
	$MyTokens{"\\$_"} = $Text::TeX::Tokens{'\em'}
    }
    # Now add any commands
    &ReadCommands::Merge(\%MyTokens);

# Create the fileobject
    my $file = new Text::TeX::OpenFile 
	   $InFileName,
	   'defaultact' => \&clean_tex,
	   'tokens' => \%MyTokens;

# Now actually process the file
    $file->process;
    close OUTFILE;
    #warn "Done cleaning TeX file\n";
} # end sub call_parser


#######################   MAIN TRANSLATING SUBROUTINE   ########################
# Routine called by the TeX-parser to perform token-processing.
sub clean_tex {
    my($eaten,$txt) = (shift,shift);
    my ($outstr, $type);

    # Sub translate is given a string and one of the translation tables below.
    # It returns the translation, or just the string if there's no translation
    # Translation table for TT::Begin::Group tokens
    my %begtranstbl = (
			'$' => '\(', # LyX math mode doesn't
			'$$' => '\[', # understand \$ or $$
			);

    # Translation table for TT::End::Group tokens
    my %endtranstbl = (
			   '$' => '\)',
			   '$$' => '\]',
		       );

    # Translation table for TT::Token tokens whose translations should
    #    NOT have whitespace after them! See sub translate...
    #   Note that tokens of type TT::EndLocal are always translated to '}'. So,
    #   any token defined as a local token *must* be translated to something
    #   with a '{' (e.g., '\em' -> '\emph{') or we'll have mismatched braces
    my %no_ws_transtbl = (
			'\em' => '\emph{',
			'\rm' => '\textrm{',
			'\bf' => '\textbf{',
			'\tt' => '\texttt{',
			'\sf' => '\textsf{',
			'\sc' => '\textsc{',
			'\sl' => '\textsl{',
			'\it' => '\textit{',
			'\rmfamily' => '\textrm{',
			'\ttfamily' => '\texttt{',
			'\sffamily' => '\textsf{',
			'\mdseries' => '\textmd{',
			'\bfseries' => '\textbf{',
			'\upshape' => '\textup{',
			'\itshape' => '\textit{',
			'\slshape' => '\textsl{',
			'\scshape' => '\textsc{',
			'\cal' => '\mathcal{',
			);


    # a faux "switch" statement.  sets $_ for later use in pattern
    # matching.
    $type = ref($eaten);
    $type =~ s/^Text::TeX::// or die "Non-Text::TeX object";
    my $printstr = ""; # default for undefined printstrs etc.
    SWITCH: for ($type) {
	   # Handle blank lines.
	   if (/Paragraph/) {
	       last SWITCH;
	   }

	   # Handle the end of a local font command - insert a '}'
	   if (/EndLocal/) {
	       # we could just say $printstr='}'
	       $printstr = &translate('}', \%endtranstbl);
	       last SWITCH;
	   }
	   
	   # $eaten->exact_print is undefined for previous environments
	   $outstr = $eaten->exact_print;
	   if (! defined $outstr) { # comment at end of paragraph
	       warn "Weird undefined token $eaten!" unless $eaten->comment;
	       last SWITCH;
	    }
	   
	   # Handle LaTeX tokens
	   if (/^Token$/) {
	       my $realtok = $eaten->print; # w/out whitespace
	       # If a comment is its own paragraph, print nothing
	       last SWITCH unless defined($realtok);
	       # Special handling for \verb and \verb*
	       if ($realtok =~ /^\\verb\*?/) {
	           $printstr = &Verbatim::copy_verb($txt,$eaten);
		   last SWITCH;
	       }

	       # Translate token if necessary, or just print it
	       # "no_ws" is HACK to remove whitespace, so '\em ' -> '\emph{'
	       $printstr = &translate($outstr, \%no_ws_transtbl, "no_ws");

	       # Ignore optional argument(s) if necessary
	       $printstr .= &handle_opt_args($eaten,$txt);

	       last SWITCH;
	   }

	   # Tokens taking arguments, like '^'
	   # ADD '{' if there isn't one before the argument!
	   # TODO can we check whether the command is \label, \include
	   # and not add the braces in that case?
	   if (/^BegArgsToken$/) {
	       $printstr = $outstr;

	       # Ignore optional argument(s) if necessary
	       $printstr .= &handle_opt_args($eaten,$txt);

	       # Add beginning brace before the 1st argument if there isn't one
	       my $tok = $txt->lookAheadToken;
	       $printstr .= '{' unless ($tok =~ /\{/);
	       last SWITCH;
	   }

	   # End of one argument, beginning of next
	   # Note: by default ArgToken,EndArgsToken print nothing
	   # ADD '}' if there isn't one after the last argument
	   # Then read and print any optional arguments which may exist
	   #    between this argument the next (we must do this here or we would
	   #    add a '{' before an optional argument!)
	   # ADD '{' if there isn't one before the next argument!
	   # (just like we do in BegArgsToken and EndArgsToken)
	   if (/^ArgToken$/) {
	       $printstr = $outstr; # = ''

	       # Add '}' after the argument that ended if necessary
	       $printstr .= '}' unless $last_eaten->print eq "\}";

	       # Eat and print any optional arguments
	       $printstr .= &handle_opt_args($eaten,$txt);

	       # Add '{' before the next argument if necessary
	       my $tok = $txt->lookAheadToken;
	       $printstr .= '{' unless ($tok =~ /\{/);
	       last SWITCH;
	   }

	   # End of tokens taking arguments, like '^'
	   #     ADD '}' if there isn't one after the last argument, i.e., 
	   # if the previous token *wasn't* a '}'
	   #     Kludge: for TeX style \input command ("\input foo" with no
	   # braces) we need to read the whole filename, but parser will have
	   # read only one char. So read in the rest of the filename before
	   # printing the '}'.
	   if (/^EndArgsToken$/) {
	       $printstr = $outstr; # = ''

	       unless ($last_eaten->print eq "\}") {
		   my $s = $eaten->base_token;
		   if ($s->print eq "\\input") {
		       my $t = $txt->lookAheadToken;
		       # For one-char filename (a.tex) do nothing
		       if ($t =~ /^[\w.\-]/) {
			   my $u = $txt->eatMultiToken;
			   $t = $u->print;
			   $t =~ s/\s+//g;
			   $printstr .= $t;
			}
			# TeX \input always adds .tex ending
			$printstr .= ".tex";
		    }

		   $printstr .= '}';
		}

	       # Don't bother eating optional args coming after the last
	       # required arg: they'll just be copied as text
	       last SWITCH;
	   }
	   
	   # Handle opening groups, like '{' and '$'.
	   if (/Begin::Group$/) {
	       $printstr = &translate($outstr,\%begtranstbl);
	       last SWITCH;
	   }
	   
	   # Handle closing groups, like '}' and '$'.
	   if (/End::Group$/) {
	       $printstr = &translate($outstr, \%endtranstbl);
	       last SWITCH;
	   }

	   if (/Begin::Group::Args/) {
	       my $env = $eaten->environment;
	       $printstr = $outstr;
	       if ($env eq "verbatim" || $env eq "reLyXskip") {
		   # copy everything up to "\end{foo}"
	           $printstr .= &Verbatim::copy_verbatim($txt, $eaten);
	       }
	       last SWITCH;
	   }
	   
	   if  (/End::Group::Args/) {
	       $printstr = $outstr;
	       last SWITCH;
	   }

	   if (/Text/) {
	       $printstr = $outstr;
	       last SWITCH;
	   }

	   # The default action - print the string.
	   $printstr = $outstr;
    } # end SWITCH:for ($type)
    
    # Actually print the string
    if (defined $printstr) { 
	print OUTFILE $printstr;
	$last_eaten = $eaten; #save for next time
    } else {warn "Undefined printstr";}

} # end sub clean_tex

####################   TRANSLATOR SUBROUTINES    ###############################
sub translate {
# Replace a string (possibly with whitespace around it) with another
# Arg0 is a string, Arg1 is a reference to a hash containing translations
# If a token not in the table is passed in, do nothing
# If Arg2 is defined AND the token is known, then remove whitespace from
#     the end of the translated token. This is a HACK to do '\em ' -> '\emph{'
# Return the string, possibly modified
    my ($tokstr, $transref) = (shift, shift);
    my $remove_ws = shift;
    my %transtable = %$transref;

    # remove whitespace from the string (since transtable doesn't have it)
    my $stripstr = $tokstr;
    $stripstr =~ s/^\s*(\S+)\s*$/$1/ or warn "couldn't strip token";
    if ( exists $transtable{$stripstr} ) {
         # use \Q or \, (, $, and [ will be misinterpreted
        $tokstr =~ s/\Q$stripstr\E/$transtable{$stripstr}/;

	# remove whitespace?
	if (defined $remove_ws) {
	    $tokstr =~ s/\s*$//;
	}
    }

    return $tokstr;
}

sub handle_opt_args {
# read and concatenate OR IGNORE optional arguments
# Arg0 is a BegArgsToken or ArgToken
    my ($eaten,$fileobject) = (shift,shift);
    my $outstr = "";

    # If at end of paragraph, don't bother looking for optArgs
    return "" unless $fileobject->lookAheadToken;

    # Get the next argument(s) expected for this token == /^o*[rR]?$/
    # If there are no args expected, just return
    my $curr_args = $eaten->next_args($fileobject) or return "";

    # Now print or ignore any optional arguments
    # If there's an 'r' in curr_args, we're done for now
    my $foo;
    my $token_name = $eaten->token_name; # (needed for EndArgsToken, e.g.)
    while ($curr_args =~ s/^o//) {
        my $opt = $fileobject->eatOptionalArgument;
	# Print any initial space before the optional argument
        if ($foo = $opt->exact_print) {
	    if ($foo =~ /^(\s+)/) {
	        $outstr .= $1;
	    }
	}

	# Print the argument or ignore it
        if ($opt->print) {
	    if (grep /^\Q$token_name\E$/, @DeleteOptArg) {
	        print "Optional argument '",$opt->print,
		       "' to macro $token_name ignored\n";
	    } else {
	        $outstr .= "[" . $opt->print . "]";
	    }
        } # Was an optional argument found?
    }

    return $outstr;
} # end sub handle_opt_args

1; # return true value to calling program
