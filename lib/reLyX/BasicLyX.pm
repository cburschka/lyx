# This file is part of reLyX
# Copyright (c) 1998-9 Amir Karger karger@post.harvard.edu
# You are free to use and modify this code under the terms of
# the GNU General Public Licence version 2 or later.

package BasicLyX;
# This package includes subroutines to translate "clean" LaTeX to LyX.
# It translates only "basic" stuff, which means it doesn't do class-specific
#     things. (It uses the TeX parser Text::TeX)
use strict;

use RelyxTable; # Handle LaTeX tables
use RelyxFigure; # Handle LaTeX figures
use Verbatim;   # Copy stuff verbatim

use vars qw($bibstyle_insert_string $bibstyle_file $Begin_Inset_Include);
$bibstyle_insert_string = "%%%%%Insert bibliographystyle file here!";
$bibstyle_file = "";
$Begin_Inset_Include = "\\begin_inset Include";

#################### PACKAGE-WIDE VARIABLES ###########################
my $debug_on; # is debugging on?

######
#Next text starts a new paragraph?
# $INP = 0 for plain text not starting a new paragraph
# $INP = 1 for text starting a new paragraph, so that we write a new
#    \layout command and renew any font changes for the new paragraph
# Starts as 1 cuz if the first text in the document is plain text
#    (not, e.g., in a \title command) it will start a new paragraph
my $IsNewParagraph = 1;
my $OldINP; #Save $IsNewParagraph during footnotes

# Some layouts may have no actual text in them before the next layout
# (e.g. slides). Pending Layout is set when we read a command that puts us
# in a new layout. If we get some regular text to print out, set it to false.
# But if we get to another layout command, first print out the command to
# start the pending layout.
my $PendingLayout = 0;

# HACK to protect spaces within optional argument to \item
my $protect_spaces = 0;

# $MBD = 1 if we're in a list, but haven't seen an '\item' command
#    In that case, we may need to start a nested "Standard" paragraph
my $MayBeDeeper = 0;
my $OldMBD; #Save $MBD during footnotes -- this should very rarely be necessary!

# Stack to take care of environments like Enumerate, Quote
# We need a separate stack for footnotes, which have separate layouts etc.
# Therefore we use a reference to an array, not just an array
my @LayoutStack = ("Standard"); #default if everything else pops off
my $CurrentLayoutStack = \@LayoutStack;

# Status of various font commands
# Every font characteristic (family, series, etc.) needs a stack, because
#    there may be nested font commands, like \textsf{blah \texttt{blah} blah}
# CurrentFontStatus usually points to the main %FontStatus hash, but
#     when we're in a footnote, it will point to a temporary hash
my %FontStatus = (
    '\emph' => ["default"],
    '\family' => ["default"],
    '\series' => ["default"],
    '\shape' => ["default"],
    '\bar' => ["default"],
    '\size' => ["default"],
    '\noun' => ["default"],
);
my $CurrentFontStatus = \%FontStatus;

# Currently aligning paragraphs right, left, or center?
my $CurrentAlignment = "";
my $OldAlignment; # Save $AS during footnotes

# Global variables for copying tex stuff
my $tex_mode_string; # string we accumulate tex mode stuff in
my @tex_mode_tokens; # stack of tokens which required calling copy_latex_known

# LyX strings to start and end TeX mode
my $start_tex_mode = "\n\\latex latex \n";
my $end_tex_mode = "\n\\latex default \n";

# String to write before each item
my $item_preface = "";

#############  INFORMATION ABOUT LATEX AND LYX   #############################
# LyX translations of LaTeX font commands
my %FontTransTable = (
   # Font commands
   '\emph' => "\n\\emph on \n",
   '\underline' => "\n\\bar under \n",
   '\underbar' => "\n\\bar under \n", # plain tex equivalent of underline?
   '\textbf' => "\n\\series bold \n",
   '\textmd' => "\n\\series medium \n",
   '\textsf' => "\n\\family sans \n",
   '\texttt' => "\n\\family typewriter \n",
   '\textrm' => "\n\\family roman \n",
   '\textsc' => "\n\\shape smallcaps \n",
   '\textsl' => "\n\\shape slanted \n",
   '\textit' => "\n\\shape italic \n",
   '\textup' => "\n\\shape up \n",
   '\noun' => "\n\\noun on \n", # LyX abstraction of smallcaps

# Font size commands
   '\tiny' => "\n\\size tiny \n",
   '\scriptsize' => "\n\\size scriptsize \n",
   '\footnotesize' => "\n\\size footnotesize \n",
   '\small' => "\n\\size small \n",
   '\normalsize' => "\n\\size default \n",
   '\large' => "\n\\size large \n",
   '\Large' => "\n\\size Large \n",
   '\LARGE' => "\n\\size LARGE \n",
   '\huge' => "\n\\size huge \n",
   '\Huge' => "\n\\size Huge \n",
   # This doesn't work yet!
   #'\textnormal' => "\n\\series medium \n\\family roman \n\\shape up \n",
);

# Things LyX implements as "Floats"
my %FloatTransTable = (
   # Footnote, Margin note
   '\footnote' => "\n\\begin_float footnote \n",
   '\thanks' => "\n\\begin_float footnote \n", # thanks is same as footnote
   '\marginpar' => "\n\\begin_float margin \n",
);
# Environments that LyX implements as "floats"
my %FloatEnvTransTable = (
    "table" => "\n\\begin_float tab \n",
    "table*" => "\n\\begin_float wide-tab \n",
    "figure" => "\n\\begin_float fig \n",
    "figure*" => "\n\\begin_float wide-fig \n",
);

# Simple LaTeX tokens which are turned into small pieces of LyX text
my %TextTokenTransTable = (
    # LaTeX escaped characters
    '\_' => '_',
    '\%' => '%',
    '\$' => '$',
    '\&' => '&',
    '\{' => '{',
    '\}' => '}',
    '\#' => '#',
    '\~' => '~',
    '\^' => '^',
    # \i and \j are used in accents. LyX doesn't recognize them in plain
    #    text. Hopefully, this isn't a problem.
    '\i' => '\i',
    '\j' => '\j',

    # Misc simple LaTeX tokens
    '~'      => "\n\\protected_separator \n",
    '@'      => "@", # TeX.pm considers this a token, but it's not really
    '\@'     => "\\SpecialChar \\@",
    '\ldots' => "\n\\SpecialChar \\ldots\{\}\n",
    '\-'     => "\\SpecialChar \\-\n",
    '\LaTeX' => "LaTeX",
    '\LaTeXe' => "LaTeX2e",
    '\TeX'    => "TeX",
    '\LyX'    => "LyX",
    '\lyxarrow' => "\\SpecialChar \\menuseparator\n",
    '\hfill'  => "\n\\hfill \n",
    '\noindent'        => "\n\\noindent \n",
    '\textbackslash'   => "\n\\backslash \n",
    '\textgreater'     => ">",
    '\textless'        => "<",
    '\textbar'         => "|",
    '\textasciitilde'  => "~",
);

# LyX translations of some plain LaTeX text (TeX parser won't recognize it
#     as a Token, so we need to translate the Text::TeX::Text token.)
my %TextTransTable = (
    # Double quotes
    "``" => "\n\\begin_inset Quotes eld\n\\end_inset \n\n",
    "''" => "\n\\begin_inset Quotes erd\n\\end_inset \n\n",

    # Tokens that don't start with a backslash, so parser won't recognize them
    # (LyX doesn't support them, so we just print them in TeX mode)
    '?`' => "$start_tex_mode?`$end_tex_mode",
    '!`' => "$start_tex_mode!`$end_tex_mode",
);

# Things that LyX translates as "LatexCommand"s
# E.g., \ref{foo} ->'\begin_inset LatexCommand \ref{foo}\n\n\end_inset \n'
# (Some take arguments, others don't)
my @LatexCommands = map {"\\$_"} qw(ref pageref label cite bibliography
	                         index printindex tableofcontents
                                 listofalgorithms listoftables listoffigures);
my @IncludeCommands = map {"\\$_"} qw(input include);
# Included postscript files
# LyX 1.0 can't do \includegraphics*!
my @GraphicsCommands = map {"\\$_"} qw(epsf epsffile epsfbox
                                       psfig epsfig includegraphics);

# Accents. Most of these take an argument -- the thing to accent
# (\l and \L are handled as InsetLatexAccents, so they go here too)
my $AccentTokens = "\\\\[`'^#~=.bcdHklLrtuv\"]";

# Environments which describe justifying (converted to LyX \align commands)
#    and the corresponding LyX commands
my %AlignEnvironments = (
    "center" => "\n\\align center \n",
    "flushright" => "\n\\align right \n",
    "flushleft" => "\n\\align left \n",
);

# Some environments' begin commands take an extra argument
# Print string followed by arg for each item in the list, or ignore the arg ("")
my %ExtraArgEnvironments = (
    "thebibliography" => "",
    "lyxlist" =>'\labelwidthstring ',
    "labeling" =>'\labelwidthstring ', # koma script list
);

# Math environments are copied verbatim
my $MathEnvironments = "(math|equation|displaymath|eqnarray(\\*)?)";
# ListLayouts may have standard paragraphs nested inside them.
my $ListLayouts = "Itemize|Enumerate|Description";

#####################   PARSER INVOCATION   ##################################
sub call_parser {
# This subroutine calls the TeX parser & translator
# Before it does that, it does lots of setup work to get ready for parsing.
# Arg0 is the file to read (clean) LaTeX from
# Arg1 is the file to write LyX to
# Arg2 is the file to read layouts from (e.g., in LYX_DIR/layouts/foo.layout)

    my ($InFileName, $OutFileName) = (shift,shift);

# Before anything else, set the package-wide variables based on the
#    user-given flags
    # opt_d is set to 1 if '-d' option given, else (probably) undefined
    $debug_on = (defined($main::opt_d) && $main::opt_d);

    # Hash of tokens passed to the TeX parser
    # Many values are $Text::TeX::Tokens{'\operatorname'}, which has
    #    Type=>report_args and count=>1
    # Note that we don't have to bother putting in tokens which will be simply
    #    translated (e.g., from %TextTokenTransTable).
    my %MyTokens = ( 
	'{' => $Text::TeX::Tokens{'{'},
	'}' => $Text::TeX::Tokens{'}'},
	'\begin' => $Text::TeX::Tokens{'\begin'},
	'\end' => $Text::TeX::Tokens{'\end'},

	# Lots of other commands will be made by ReadCommands:Merge
	# by reading the LaTeX syntax file

	# Font sizing commands (local)
	'\tiny' => {Type => 'local'},
	'\small' => {Type => 'local'},
	'\scriptsize' => {Type => 'local'},
	'\footnotesize' => {Type => 'local'},
	'\small' => {Type => 'local'},
	'\normalsize' => {Type => 'local'},
	'\large' => {Type => 'local'},
	'\Large' => {Type => 'local'},
	'\LARGE' => {Type => 'local'},
	'\huge' => {Type => 'local'},
	'\Huge' => {Type => 'local'},

	# Tokens to ignore (which make a new paragraph)
	# Just pretend they actually ARE new paragraph markers!
	'\maketitle' => {'class' => 'Text::TeX::Paragraph'},
    );
    
    # Now add to MyTokens all of the commands that were read from the
    #    commands file by package ReadCommands
    &ReadCommands::Merge(\%MyTokens);

# Here's the actual subroutine. The above is all preparation
    # Output LyX file
    my $zzz = $debug_on ? " ($InFileName --> $OutFileName)\n" : "... ";
    print STDERR "Translating$zzz";
    open (OUTFILE,">$OutFileName");

    # Open the file to turn into LyX.
    my $infile = new Text::TeX::OpenFile $InFileName,
	'defaultact' => \&basic_lyx,
	'tokens' => \%MyTokens;

    # Process what's left of the file (everything from \begin{document})
    $infile->process;

    # Last line of the LyX file
    print OUTFILE "\n\\the_end\n";
    close OUTFILE;
    #warn "Done with basic translation\n";
    return;
} # end subroutine call_parser

##########################   MAIN TRANSLATOR SUBROUTINE   #####################
sub basic_lyx {
# This subroutine is called by Text::TeX::process each time subroutine
#     eat returns a value.
# Argument 0 is the return value from subroutine eat
# Argument 1 is the Text::TeX::OpenFile (namely, $TeXfile)
    my $eaten = shift;
    my $fileobject = shift;

    # This handles most but maybe not all comments
    # THere shouldn't be any if we've run CleanTeX.pl
    print "Comment: ",$eaten->comment if defined $eaten->comment && $debug_on;

    my $type = ref($eaten);
    print "$type " if $debug_on;

    # This loop is basically just a switch. However, making it a for
    #    (1) makes $_ = $type (convenient)
    #    (2) allows us to use things like next and last
    TYPESW: for ($type) {

        # some pre-case work
        s/^Text::TeX:://o or die "unknown token?!";
	my ($dummy, $tok);
	my ($thistable);

	# The parser puts whitespace before certain kinds of tokens along
	# with that token. If that happens, save a space
	my $pre_space = ""; # whitespace before a token
	if (/BegArgsToken|^Token|::Group$/) {
	    $dummy = $eaten->exact_print;
	    # Only set prespace if we match something
	    #    We wouldn't want it to be more than one space, since that's
	    # illegal in LyX. Also, replace \t or \n with ' ' since they are
	    # ignored in LyX. Hopefully, this won't lead to anything worse
	    # than some lines with >80 chars
	    #    Finally, don't put a space at the beginning of a new paragraph
	    if (($dummy =~ /^\s+/) && !$IsNewParagraph) {
		$pre_space = " ";
	    }
	}

        # Handle blank lines.
        if (m/^Paragraph$/o) {
	    # $INP <>0 means We will need a new layout command
	    $IsNewParagraph = 1;

	    # $MBD means start a begin_deeper within list environments
	    #    unless there's an \item command
	    $MayBeDeeper = 1;

            last TYPESW;
        }

	# If, e.g., there's just a comment in this token, don't do anything
	# This actually shouldn't happen if CleanTeX has already removed them
	last TYPESW if !defined $eaten->print;
        
        # Handle LaTeX tokens
        if (/^Token$/o) {

	    my $name = $eaten->token_name; # name of the token, e.g., "\large"
	    print "'$name' " if $debug_on;

	    # Tokens which turn into a bit of LyX text
	    if (exists $TextTokenTransTable{$name}) {
	        &CheckForNewParagraph; #Start new paragraph if necessary

		my $to_print = $TextTokenTransTable{$name};

		# \@ has to be specially handled, because it depends on
		# the character AFTER the \@
		if ($name eq '\@') {
		    my $next = $fileobject->eatGroup(1);
		    my $ch="";
		    $ch = $next->print or warn "\@ confused me!\n";
		    if ($ch eq '.') {
			# Note: \@ CAN'T have pre_space before it
			print OUTFILE "$to_print$ch\n";
			print "followed by $ch" if $debug_on;
		    } else {
		       warn "LyX (or LaTeX) can't handle '$ch' after $name\n";
			print OUTFILE $ch;
		    }

		} else { # it's not \@
		    # Print the translated text (include preceding whitespace)
		    print OUTFILE "$pre_space$to_print";
		} # end special handling for \@

	    # Handle tokens that LyX translates as a "LatexCommand" inset
	    } elsif (grep {$_ eq $name} @LatexCommands) {
		&CheckForNewParagraph; #Start new paragraph if necessary
	        print OUTFILE "$pre_space\n\\begin_inset LatexCommand ",
		               $name,
			      "\n\n\\end_inset \n\n";

	    # Math -- copy verbatim until you're done
	    } elsif ($name eq '\(' || $name eq '\[') {
		print "\nCopying math beginning with '$name'\n" if $debug_on;
		# copy everything until end text
		$dummy = &Verbatim::copy_verbatim($fileobject, $eaten);
		$dummy = &fixmath($dummy); # convert '\sp' to '^', etc.

		&CheckForNewParagraph; # math could be first thing in a par
		print OUTFILE "$pre_space\n\\begin_inset Formula $name ";
		print $dummy if $debug_on;
		print OUTFILE $dummy;

	    } elsif ($name eq '\)' || $name eq '\]') {
	        # end math
		print OUTFILE "$name\n\\end_inset \n\n";
		print "\nDone copying math ending with '$name'" if $debug_on;

	    # Items in list environments
	    } elsif ($name eq '\item') {
		
		# What if we had a nested "Standard" paragraph?
		# Then write \end_deeper to finish the standard layout
		#     before we write the new \layout ListEnv command
		if ($$CurrentLayoutStack[-1] eq "Standard") {
		    pop (@$CurrentLayoutStack); # take "Standard" off the stack
		    print OUTFILE "\n\\end_deeper ";
		    print "\nCurrent Layout Stack: @$CurrentLayoutStack"
		          if $debug_on;
		} # end deeper if

		# Upcoming text (the item) will be a new paragraph, 
		#    requiring a new layout command based on whichever
		#    kind of list environment we're in
		$IsNewParagraph = 1;

		# But since we had an \item command, DON'T nest a
		#    deeper "Standard" paragraph in the list
		$MayBeDeeper = 0;

		# Check for an optional argument to \item
		# If so, within the [] we need to protect spaces
		# TODO: In fact, for description, if there's no [] or
		# there's an empty [], then we need to write a ~, since LyX
		# will otherwise make the next word the label
		# If it's NOT a description & has a [] then we're stuck!
		# They need to fix the bullets w/in lyx!
		if (($dummy = $fileobject->lookAheadToken) &&
		    ($dummy =~ /\s*\[/)) {
		    $fileobject->eatFixedString('\['); # eat the [
		    $protect_spaces = 1;
		}

		# Special lists (e.g. List layout) have to print something
		# before each item. In that case, CFNP and print it
		if ($item_preface) {
		    &CheckForNewParagraph;
		    print OUTFILE $item_preface;
		}

	    # Font sizing commands
	    # (Other font commands are TT::BegArgsTokens because they take
	    #     arguments. Font sizing commands are 'local' TT::Tokens)
	    } elsif (exists $FontTransTable{$name}) {
		my $command = $FontTransTable{$name}; #e.g., '\size large'

		if (! $IsNewParagraph) {
		    print OUTFILE "$pre_space$command";
		} #otherwise, wait until we've printed the new paragraph command

		# Store the current font change
		($dummy = $command) =~ s/\s*(\S+)\s+(\w+)\s*/$1/;
		die "Font command error" if !exists $$CurrentFontStatus{$dummy};
		push (@{$CurrentFontStatus->{$dummy}}, $2);
		print "\nCurrent $dummy Stack: @{$CurrentFontStatus->{$dummy}}"
		      if $debug_on;

	    # Table stuff
	    } elsif ($name eq '&') {
		if ($thistable = &RelyxTable::in_table) {
		    print OUTFILE "\n\\newline \n";
		    $thistable->nextcol;
		} else {warn "& is illegal outside a table!"}

	    } elsif ($name eq '\\\\' || $name eq '\\newline') {
		&CheckForNewParagraph; # could be at beginning of par?
                print OUTFILE "\n\\newline \n";

		# If we're in a table, \\ means add a row to the table
		# Note: if we're on the last row of the table, this extra
		# row will get deleted later. This hack is necessary, because
		# we don't know while reading when the last row is!
		if ($thistable = &RelyxTable::in_table) {
		    $thistable->addrow;
		}

	    } elsif ($name eq '\hline') {
		if ($thistable = &RelyxTable::in_table) {
		    # hcline does hline if no arg is given
		    $thistable->hcline;
		} else {warn "\\hline is illegal outside a table!"}

	    # Figures

	    } elsif ($name =~ /^\\epsf[xy]size$/) {
		# We need to eat '=' followed by EITHER more text OR
		# one (or more?!) macros describing a TeX size
	        my $arg = $fileobject->eatMultiToken;
		my $length = $arg->print;
		$length =~ s/^\s*=\s*// or warn "weird '$name' command!";

		# If there's no "cm" or other letters in $length, the next token
		# ought to be something like \textwidth. Then it will be empty
		# or just have numbers in it.
		# This is bugprone. Hopefully not too many people use epsf!
		if ($length =~ /^[\d.]*\s*$/) {
		    my $next = $fileobject->eatMultiToken;
		    $length .= $next->print;
		}
		$length =~ s/\s*$//; # may have \n at end

		# If we can't parse the command, print it in tex mode
		&RelyxFigure::parse_epsfsize($name, $length) or 
		    &print_tex_mode("$name=$length");

	    # Miscellaneous...

	    } elsif ($name =~ /\\verb.*?/) {
	        my $dummy = &Verbatim::copy_verb($fileobject, $eaten);
		print "\nCopying $name in TeX mode: " if $debug_on;
		&print_tex_mode ($dummy);

	    # Otherwise it's an unknown token, which must be copied
	    #     in TeX mode, along with its arguments, if any
	    } else {
		if (defined($eaten->relyx_args($fileobject))) {
		   &copy_latex_known($eaten, $fileobject);
		} else { # it's not in MyTokens
		    &copy_latex_unknown($eaten, $fileobject);
		}
	    }

            last TYPESW;
        }
        
	# Handle tokens that take arguments, like \section{},\section*{}
	if (/^BegArgsToken$/) {
	    my $name = $eaten->token_name;
	    print "$name" if $debug_on;

	    # Handle things that LyX translates as a "LatexCommand" inset
	    if (grep {$_ eq $name} @LatexCommands) {
		&CheckForNewParagraph; #Start new paragraph if necessary

	        print OUTFILE "$pre_space\n\\begin_inset LatexCommand ";

		#    \bibliography gets handled as a LatexCommand inset, but
		# it's a special case, cuz LyX syntax expects "\BibTeX"
		# instead of "\bibliography" (I have no idea why), and because
		# we have to print extra stuff
		#    Because we might not have encountered the
		# \bibliographystyle command yet, we write
		# "insert bibstyle here", and replace that string
		# with the actual bibliographystyle argument in
		# LastLyX (i.e., the next pass through the file)
		if ($name eq "\\bibliography") {
		    print OUTFILE "\\BibTeX[", $bibstyle_insert_string, "]";
		} else {
		    print OUTFILE "$name";
		}

		# \cite takes an optional argument, e.g.
		my $args = $eaten->relyx_args ($fileobject);
		while ($args =~ s/^o//) {
		    my $tok = $fileobject->eatOptionalArgument;
		    my $dummy = $tok->exact_print;
		    print OUTFILE $dummy;
		}

		print OUTFILE "\{";
	        last TYPESW; # skip to the end of the switch
	    }

	    if (grep {$_ eq $name} @IncludeCommands) {
		&CheckForNewParagraph; #Start new paragraph if necessary
	        print OUTFILE "$pre_space\n$Begin_Inset_Include $name\{";
	        last TYPESW; # skip to the end of the switch
	    }

	    # This is to handle cases where _ is used, say, in a filename.
	    # When _ is used in math mode, it'll be copied by the math mode
	    # copying subs. Here we handle cases where it's used in non-math.
	    # Examples are filenames for \include & citation labels.
	    # (It's illegal to use it in regular LaTeX text.)
	    if ($name eq "_") {
	       print OUTFILE $eaten->exact_print;
	       last TYPESW;
	    }

	    # Sectioning and Title environments (using a LyX \layout command)
	    if (exists $ReadCommands::ToLayout->{$name}) {
		&ConvertToLayout($name);
		last TYPESW; #done translating

	    # Font characteristics
	    } elsif (exists $FontTransTable{$name}) {
		my $dum2;
	        my $command = $FontTransTable{$name};
	        ($dummy, $dum2) = ($command =~ /(\S+)\s+(\w+)/);

		# HACK so that "\emph{hi \emph{bye}}" yields unemph'ed "bye"
		if ( ($dummy eq "\\emph") && 
		     ($CurrentFontStatus->{$dummy}->[-1] eq "on")) {
		       $dum2 = "default"; # "on" instead of default?
		       $command =~ s/on/default/;
		}

		# If we're about to start a new paragraph, save writing
		#    this command until *after* we write '\layout Standard'
		if (! $IsNewParagraph) {
		    print OUTFILE "$pre_space$command";
		}

		# Store the current font change
		die "Font command error" if !exists $$CurrentFontStatus{$dummy};
		push (@{$CurrentFontStatus->{$dummy}}, $dum2);


	    # Handle footnotes and margin notes
	    # Make a new font table & layout stack which will be local to the 
	    #    footnote or marginpar
	    } elsif (exists $FloatTransTable{$name}) {
	        my $command = $FloatTransTable{$name};

		# Open the footnote
		print OUTFILE "$pre_space$command";

		# Make $CurrentFontStatus point to a new (anonymous) font table
	        $CurrentFontStatus =  {
		    '\emph' => ["default"],
		    '\family' => ["default"],
		    '\series' => ["default"],
		    '\shape' => ["default"],
		    '\bar' => ["default"],
		    '\size' => ["default"],
		    '\noun' => ["default"],
		};

		# And make $CurrentLayoutStack point to a new (anon.) stack
		$CurrentLayoutStack = ["Standard"];

		# Store whether we're at the end of a paragraph or not
		#    for when we get to end of footnote AND 
		# Note that the footnote text will be starting a new paragraph
		# Also store the current alignment (justification)
		$OldINP = $IsNewParagraph; $OldMBD = $MayBeDeeper;
		$OldAlignment = $CurrentAlignment;
		$IsNewParagraph = 1;
		$MayBeDeeper = 0; #can't be deeper at beginning of footnote
		$CurrentAlignment = "";

	    # Accents
	    } elsif ($name =~ m/^$AccentTokens$/) {
		&CheckForNewParagraph; # may be at the beginning of a par

		print OUTFILE "$pre_space\n",'\i ',$name,'{'
	    
	    # Included Postscript Figures
	    # Currently, all postscript including commands take one
	    # required argument and 0 to 2 optional args, so we can
	    # clump them together in one else.
	    } elsif (grep {$_ eq $name} @GraphicsCommands) {
		&CheckForNewParagraph; # may be at the beginning of a par
		my $arg1 = $fileobject->eatOptionalArgument;
		# arg2 is a token of an empty string for most commands
		my $arg2 = $fileobject->eatOptionalArgument;
		my $arg3 = $fileobject->eatRequiredArgument;
		my $save = $arg1->exact_print . $arg2->exact_print .
		           $arg3->exact_print;

		# Parse and put figure into LyX file
		# Print it verbatim if we didn't parse correctly
		my $thisfig = new RelyxFigure::Figure;
		if ($thisfig->parse_pscommand($name, $arg1, $arg2, $arg3)) {
		    print OUTFILE $thisfig->print_info;
		} else {
		    &print_tex_mode($eaten->exact_print . $save);
		}

	    # Tables

	    } elsif ($name eq "\\multicolumn") {
		if ($thistable = &RelyxTable::in_table) {
		    # the (simple text) first arg.
		    $dummy = $fileobject->eatRequiredArgument->contents->print;
		    my $group = $fileobject->eatRequiredArgument;
		    $thistable->multicolumn($dummy, $group);
		} else {warn "\\multicolumn is illegal outside a table!"}

	    } elsif ($name eq '\cline') {
		if ($thistable = &RelyxTable::in_table) {
		    # the (simple text) first arg.
		    $dummy = $fileobject->eatRequiredArgument->contents->print;
		    # sub hcline does cline if an arg is given
		    $thistable->hcline($dummy);
		} else {warn "\\cline is illegal outside a table!"}

	    # Bibliography

	    } elsif ($name eq '\bibliographystyle') {
		$tok = $fileobject->eatRequiredArgument;
		$bibstyle_file = "";
		# There may be >1 token in the {}, e.g. "{a_b}" -> 3 tokens
		my @toks = $tok->contents;
		foreach $tok (@toks) {
		    # kludge: CleanTeX adds {} after _
		    $tok = $tok->contents if ref($tok) eq "Text::TeX::Group";
		    $bibstyle_file .= $tok->print;
		}
		print "\nBibliography style file is $bibstyle_file"if $debug_on;

	    # LyX \bibitem actually looks just like LaTeX bibitem, except
	    # it's in a Bibliography par & there must be a space after the
	    # bibitem command. Note we need to explicitly print the braces...
	    } elsif ($name eq "\\bibitem") {
		$IsNewParagraph=1; # \bibitem always starts new par. in LyX
		&CheckForNewParagraph;

		$tok = $fileobject->eatOptionalArgument;
	        print OUTFILE "$name ", $tok->exact_print, "{";

	    # Miscellaneous

	    # ensuremath -- copy verbatim until you're done
	    # but add \( and \)
	    # Note that we'll only get here if the command is NOT in math mode
	    } elsif ($name eq '\ensuremath') {
		print "\nCopying math beginning with '$name'\n" if $debug_on;
		my $tok = $fileobject->eatGroup; # eat math expression
		my $dummy = $tok->exact_print;
		$dummy =~ s/\{(.*)\}/$1/;
		$dummy = &fixmath($dummy); # convert '\sp' to '^', etc.

		&CheckForNewParagraph; # math could be first thing in a par
		print OUTFILE "$pre_space\n\\begin_inset Formula \\( ";
		print $dummy if $debug_on;
		print OUTFILE $dummy;

	        # end math
		print OUTFILE "\\)\n\\end_inset \n\n";
		print "\nDone copying math" if $debug_on;

	    # Token in the ReadCommands command list that basic_lyx doesn't
	    #    know how to handle
	    } else {
		&copy_latex_known($eaten,$fileobject);
	    } # end big if

	    # Exit the switch
	    last TYPESW;
	}

	# ArgTokens appear when we've used eatRequiredArgument
	if (/^ArgToken$/) {
	    # If we're copying a recognized but untranslatable token in tex mode
	    my $tok = $tex_mode_tokens[-1] || 0;
	    if ($eaten->base_token == $tok) {
		&copy_latex_known($eaten,$fileobject);
	    }
        
	    last TYPESW;
	}

	if (/^EndArgsToken$/) {
	    # If we're copying a recognized but untranslatable token in tex mode
	    my $tok = $tex_mode_tokens[-1] || 0;
	    if ($eaten->base_token eq $tok) {
		&copy_latex_known($eaten,$fileobject);
	        last TYPESW;
	    }

	    my $name = $eaten->token_name;
	    print "$name" if $debug_on;

	    # Handle things that LyX translates as a "LatexCommand" inset
	    # or "Include" insets
	    if (grep {$_ eq $name} @LatexCommands, @IncludeCommands) {
	        print OUTFILE "\}\n\n\\end_inset \n\n";

	    } elsif (exists $ReadCommands::ToLayout->{$name}) {
		&EndLayout($name);

	    # Font characteristics
	    # Pop the current FontStatus stack for a given characteristic
	    #    and give the new command (e.g., \emph default)
	    } elsif (exists $FontTransTable{$name}) {
	        my $command = $FontTransTable{$name};
	        ($dummy) = ($command =~ /(\S+)\s+\w+/);
		pop @{$CurrentFontStatus->{$dummy}};
		$command = "\n$dummy $CurrentFontStatus->{$dummy}->[-1] \n";
		print OUTFILE "$command";

	    # Footnotes and marginpars
	    } elsif (exists $FloatTransTable{$name}) {
	        print OUTFILE "\n\\end_float \n\n";

		# Reset the layout stack and font status table pointers to
		#    point to the global stack/table again, instead of the
		#    footnote-specific stack/table
		$CurrentFontStatus = \%FontStatus;
		$CurrentLayoutStack = \@LayoutStack;

		# We need to reissue any font commands (but not layouts)
		foreach $dummy (keys %$CurrentFontStatus) {
		    if ($CurrentFontStatus->{$dummy}->[-1] ne "default") {
			print OUTFILE $FontTransTable{$dummy};
		    }
		}

		# Same paragraph status as we had before the footnote started
		$IsNewParagraph = $OldINP; $MayBeDeeper = $OldMBD;
		$CurrentAlignment = $OldAlignment;

	    } elsif ($name =~ m/^$AccentTokens$/) {
		print OUTFILE "}\n";
	    
	    } elsif ($name eq "\\bibitem") {
	        print OUTFILE "}\n";
	    } # End if on $name

	    # Exit main switch
	    last TYPESW;
	} # end if EndArgsToken

	# Handle END of scope of local commands like \large
	if (/^EndLocal$/) {
	    my $name = $eaten->token_name; #cmd we're ending, e.g.,\large
	    print $name if $debug_on;

	    if (exists $FontTransTable{$name}) {
		my $command = $FontTransTable{$name};
		($dummy = $command) =~ s/\s*(\S*)\s+(\w+)\s*/$1/; #e.g., '\size'
		die "Font command error" if !exists $$CurrentFontStatus{$dummy};
		# TT::OF->check_presynthetic returns local commands FIFO!
		# So pop font stack, but warn if we pop the wrong thing
		warn " font confusion?" if
			   pop @{$CurrentFontStatus->{$dummy}} ne $2;
		print "\nCurrent $dummy Stack: @{$CurrentFontStatus->{$dummy}}"
		      if $debug_on;
		my $newfont = $CurrentFontStatus->{$dummy}->[-1];
		$command = "\n$dummy $newfont\n";
		print OUTFILE "$command";

	    } else {
	        warn "Unknown EndLocal token!\n";
	    }

	    last TYPESW;
	}

	# We don't print { or }, but we make sure that the spacing is correct
	# Handle '{'
	if (/^Begin::Group$/) {
	    print OUTFILE "$pre_space";
	    last TYPESW;
	}

	# Handle '{'
	if (/^End::Group$/) {
	    print OUTFILE "$pre_space";
	    last TYPESW;
	}

        # Handle \begin{foo}
        if (/^Begin::Group::Args$/) {
	    print $eaten->print," " if $debug_on; # the string "\begin{foo}"
	    my $env = $eaten->environment;
	    
	    # Any environment found in the layouts files
	    if (exists $ReadCommands::ToLayout->{$env}) {
	        &ConvertToLayout($env);

		# Some environments have an extra argument. In that case,
		# print the \layout command (cuz these environments always
		# start new pars). Then either print the extra arg or
		# ignore it (depending on the environment).
		if (exists $ExtraArgEnvironments{$env}) {
		    # Should be just one token in the arg.
		    my $arg = $fileobject->eatBalanced->contents->print;

		    if ($ExtraArgEnvironments{$env}) { #print it
			print "\nArgument $arg to $env environment"
								if $debug_on;
			$item_preface = $ExtraArgEnvironments{$env} . $arg."\n";

		    } else { #ignore it
			print "\nIgnoring argument '$arg' to $env environment"
								if $debug_on;
		    }
		} # end if for reading extra args to \begin command

	    # Math environments
	    } elsif ($env =~ /^$MathEnvironments$/o) {
		&CheckForNewParagraph; # may be beginning of paragraph
		my $begin_text = $eaten->print;
		print "\nCopying math beginning with '$begin_text'\n"
		                                              if $debug_on;
		print OUTFILE "\n\\begin_inset Formula $begin_text ";
		$dummy = &Verbatim::copy_verbatim($fileobject, $eaten);
		$dummy = &fixmath($dummy); # convert '\sp' to '^', etc.
		print $dummy if $debug_on;
		print OUTFILE $dummy;

	    # Alignment environments
	    } elsif (exists $AlignEnvironments{$env}) {
		# Set it to the command which creates this alignment
	        $CurrentAlignment = $AlignEnvironments{$env};
		($dummy) = ($CurrentAlignment =~ /\S+\s+(\w+)/);
		print "\nNow $dummy-aligning text " if $debug_on;

		# alignment environments automatically start a new paragraph
		$IsNewParagraph = 1;

	    # Environments lyx translates to floats
	    } elsif (exists $FloatEnvTransTable{$env}) {
		# this really only matters if it's at the very
		# beginning of the doc.
		&CheckForNewParagraph;

		$tok = $fileobject->eatOptionalArgument;
		if ($tok && defined ($dummy = $tok->print) && $dummy) {
		    print "\nIgnoring float placement '$dummy'" if $debug_on;
		}
	        my $command = $FloatEnvTransTable{$env};

		# Open the table/figure
		print OUTFILE "$command";

	    # table
	    } elsif ($env =~ /^tabular$/) { # don't allow tabular* or ctabular
		# Table must start a new paragraph
		$IsNewParagraph = 1; $MayBeDeeper = 1;
		# We want to print table stuff *after* a \layout Standard
		&CheckForNewParagraph;

		# Since table info needs to come *before* the table content,
		#    put a line in the output file saying that the *next*
		#    reLyX pass needs to put the table info there
		print OUTFILE "\n$RelyxTable::TableBeginString\n";

		# Read and ignore an optional argument [t] or [b]
		$tok = $fileobject->eatOptionalArgument;
		if ($tok && defined ($dummy = $tok->print) && $dummy) {
		    print "\nIgnoring positioning arg '$dummy'" if $debug_on;
		}

	        # Read the argument into a TT::Group
		#   (that group may contain groups, e.g. for {clp{10cm}}
		$tok = $fileobject->eatGroup;
		new RelyxTable::Table $tok;

	    # \begin document
	    } elsif ($env eq "document") {
		# do nothing
	        #print "\nStarting to translate actual document" if $debug_on;

	    # Special environments to copy as regular text (-r option).
	    # Do this by copying the \begin & \end command in TeX mode
	    # (\Q\E around $env allows *'s in environment names!)
	    } elsif (grep /^\Q$env\E$/, @ReadCommands::regular_env) {
		print "\nCopying $env environment as regular text\n"
		                                              if $debug_on;
		$dummy = $eaten->print; # \begin{env}, ignore initial whitespace
		&print_tex_mode($dummy);

	    # otherwise, it's an unknown environment
	    # In that case, copy everything up to the \end{env}
	    #    Save stuff in global tex_mode_string so we can print it
	    # when we read & handle the \end{env}
	    } else {

		print "\nUnknown environment $env" if $debug_on;
		$tex_mode_string = "";
		# print "\begin{env}
		# For reLyXskip env, don't print the \begin & \end commands!
		$tex_mode_string .= $eaten->exact_print 
		                unless $env eq "reLyXskip";
		$tex_mode_string .=&Verbatim::copy_verbatim($fileobject,$eaten);
	    }

            last TYPESW;
        }
        
        # Handle \end{foo}
        if (/^End::Group::Args$/) {
	    print $eaten->print," " if $debug_on; # the string "\end{foo}"
	    my $env = $eaten->environment;

	    # End of list or quote/verse environment
	    # OR special environment given with -t option
	    if (exists $ReadCommands::ToLayout->{$env}) {
	        &EndLayout($env);
		$item_preface = ""; # reset when at end of List env.

	    # End of math environments
	    } elsif ($env =~ /^$MathEnvironments$/o) {
		print OUTFILE "\\end{$env}\n\\end_inset \n\n";
		print "\nDone copying math environment '$env'" if $debug_on;

	    } elsif (exists $AlignEnvironments{$env}) {
		# Back to block alignment
	        $CurrentAlignment = "";
		print "\nBack to block alignment" if $debug_on;

		# assume that \end should end a paragraph
		# This isn't correct LaTeX, but LyX can't align part of a par
		$IsNewParagraph = 1;

	    # Environments lyx translates to floats
	    } elsif (exists $FloatEnvTransTable{$env}) {
	        print OUTFILE "\n\\end_float \n\n";

	    # table
	    } elsif ($env =~ /tabular$/) { # don't allow tabular*
	        if ($thistable = &RelyxTable::in_table) {
		    $thistable->done_reading;
		    print OUTFILE "\n$RelyxTable::TableEndString\n";
		} else {warn "found \\end{tabular} when not in table!"}

		# Anything after a table will be a new paragraph
		$IsNewParagraph = 1; $MayBeDeeper = 1;

	    } elsif ($env eq "document") {
	        print "\nDone with document!" if $debug_on;

	    # "regular" environment given with -r option
	    } elsif (grep /^\Q$env\E$/, @ReadCommands::regular_env) {
		$dummy = $eaten->print; # \end{env}, ignore initial whitespace
		&print_tex_mode($dummy);

		# Next stuff will be new env.
		$IsNewParagraph = 1;

	    # End of unknown environments. We're already in TeX mode
	    } else {
		# Add \end{env} (including initial whitespace) to string
		# For reLyXskip environment, don't print \begin & \end commands!
		$tex_mode_string .= $eaten->exact_print
		                       unless $env eq "reLyXskip";
		# Now print it
		&print_tex_mode($tex_mode_string);
		print "Done copying unknown environment '$env'" if $debug_on;
	    }

            last TYPESW;

        }

	# Note for text handling: we have to do lots of stuff to handle
	# spaces in (as close as possible to) the same way that LaTeX does
	#    LaTeX considers all whitespace to be the same, so basically, we
	# convert each clump of whitespace to one space. Unfortunately, there
	# are special cases, like whitespace at the beginning/end of a par,
	# which we have to get rid of to avoid extra spaces in the LyX display.
	#    \n at the end of a paragraph must be considered like a space,
	# because the next line might begin with a token like \LyX. But
	# if the next line starts with \begin, say, then an extra space will be
	# generated in the LyX file. Oh well. It doesn't affect the dvi file.
	if (/^Text$/) {
	    my $outstr = $eaten->print; # the actual text

	    # don't bother printing whitespace
	    #    Note: this avoids the problem of extra whitespace generating
	    # extra Text::TeX::Paragraphs, which would generate extra
	    # \layout commands
	    last TYPESW if $outstr =~ /^\s+$/;

	    # whitespace at beginning of a paragraph is meaningless
	    # e.g. \begin{foo}\n hello \end{foo} shouldn't print the \n
	    # (Note: check IsNewParagraph BEFORE calling &CFNP, which zeros it)
	    my $replace = $IsNewParagraph ? "" : " ";
	    $outstr =~ s/^\s+/$replace/;

	    # Only write '\layout Standard' once per paragraph
	    &CheckForNewParagraph;

	    # \n\n signals end of paragraph, so get rid of it (and any space
	    # before it)
	    $outstr =~ s/\s*\n\n$//;

	    # Print the LaTeX text to STDOUT
	    print "'$outstr'" if $debug_on;

	    # LyX *ignores* \n and \t, whereas LaTeX considers them just
	    # like a space.
	    #    Also, many spaces are equivalent to one space in LaTeX
	    # (But LyX misleadingly displays them on screen, so get rid of them)
	    $outstr =~ s/\s+/ /g;

	    # protect spaces in an optional argument if necessary
	    # Put a SPACE after the argument for List, Description layouts
	    if ($protect_spaces) {
		$dummy = $TextTokenTransTable{'~'};

		# This will not handle brackets in braces!
		if ($outstr =~ /\]/) { # protect spaces only *until* the bracket
		    my $tempstr = $`;
		    my $tempstr2 = $';
		    # Note that any \t's have been changed to space already
		    $tempstr =~ s/ /$dummy/g;

		    # Print 1 space after the argument (which finished with ])
		    # Don't print 2 (i.e. remove leading space from $tempstr2)
		    # don't print the bracket
		    $tempstr2 =~ s/^ //;
		    $outstr = "$tempstr $tempstr2";
		    $protect_spaces = 0; # Done with optional argument
		} else { # protect all spaces, since we're inside brackets
		    $outstr =~ s/ /$dummy/g;
		}
	    } # end special stuff for protected spaces

	    # Translate any LaTeX text that requires special LyX handling
	    foreach $dummy (keys %TextTransTable) {
		$outstr =~ s/\Q$dummy\E/$TextTransTable{$dummy}/g;
	    }

	    # "pretty-print" the string. It's not perfect, since there may
	    # be text in the OUTFILE before $outstr, but it will keep from
	    # having REALLY long lines.
	    # Try to use approximately the same word-wrapping as LyX does:
	    # - before space after a period, except at end of string
	    # - before first space after column seventy-one
	    # - after 80'th column
	    while (1) {
	        $outstr =~ s/\. (?!$)/.\n /      or
		$outstr =~ s/(.{71,79}?) /$1\n / or
		$outstr =~ s/(.{80})(.)/$1\n$2/ or
		last; # exit loop if string is < 79 characters
	    }

	    # Actually print the text
	    print OUTFILE "$outstr";
	    last TYPESW;
	} # end TT::Text handling

	# The default action - this should never happen!
	print("I don't know ",$eaten->print) if $debug_on;

    } # end for ($type)

    print "\n" if $debug_on;

} #end sub basic_lyx

#########################  TEX MODE  SUBROUTINES  #########################

# This subroutine copies and prints a latex token and its arguments if any.
# This sub is only needed if the command was not found in the syntax file
# Use exact_print to preserve, e.g., whitespace after macros
sub copy_latex_unknown {
    my $eaten = shift;
    my $fileobject = shift;
    my $outstr = $eaten->exact_print;
    my ($dummy, $tok, $count);

# Copy the actual word. Copy while you've still got
#     arguments. Assume all args must be in the same paragraph
#     (There could be new paragraphs *within* args)
    #    We can't use copy_verbatim (unless we make it smarter) because
    # it would choke on nested braces
    print "\nUnknown token: '",$eaten->print,"': Copying in TeX mode\n"
                                                         if $debug_on;
    my $dum2;
    while (($dum2 = $fileobject->lookAheadToken) &&
	   ($dum2 =~ /^[[{]$/)) {
	if ($dum2 eq '[') { #copy optional argument - assume it's simple
	    $tok = $fileobject->eatOptionalArgument;
	    $outstr .= $tok->exact_print; # also print brackets & whitespace
	} else {
	    $count = 0;
	    EAT: { #copied from eatBalanced, but allow paragraphs
	        die unless defined ($tok = $fileobject->eatMultiToken);
		$outstr.="\n",redo EAT if ref($tok) eq "Text::TeX::Paragraph";
		$dummy = $tok->exact_print;
		$outstr .= $dummy;
		# Sometimes, token will be '\n{', e.g.
		$count++ if $dummy =~ /^\s*\{$/; # add a layer of nesting
		$count-- if $dummy =~ /^\s*\}$/; # end one layer of nesting
		redo EAT if $count; #don't dump out until all done nesting
	    } #end EAT block
	} # end if $dummy = [{

    } #end while
    # Add {} after macro if it's followed by '}'. Otherwise, {\foo}bar
    #     will yield \foobar when LyX creates LaTeX files
    $outstr.="{}" if $outstr=~/\\[a-zA-Z]+$/ && $dum2 eq '}';

    # Print it out in TeX mode
    &print_tex_mode($outstr);

    print "\nDone copying unknown token" if $debug_on;
} # end sub copy_latex_unknown

# Copy an untranslatable latex command whose syntax we know, along with its
# arguments
#    The command itself, optional arguments, and untranslatable
# arguments get copied in TeX mode. However, arguments which contain regular
# LaTeX will get translated by reLyX. Do that by printing what you have so
# far in TeX mode, leaving this subroutine, continuing with regular reLyX
# translating, and then returning here when you reach the ArgToken or
# EndArgsToken at the end of the translatable argument.
#    We need to keep a stack of the tokens that brought us here, because
# you might have nested commands (e.g., \mbox{hello \fbox{there} how are you}
sub copy_latex_known {
    my ($eaten, $fileobject) = (shift,shift);
    my $type = ref($eaten);
    $type =~ s/^Text::TeX::// or die "unknown token?!";

    # token itself for TT::Token, TT::BegArgsToken,
    # Corresponding BegArgsToken for ArgToken,EndArgsToken
    my $temp_start = $eaten->base_token;

# Initialize tex mode copying
    if ($type eq "BegArgsToken" or $type eq "Token") {
	print "\nCopying untranslatable token '",$eaten->print,
	                              "' in TeX mode" if $debug_on;
	push @tex_mode_tokens, $temp_start;

	# initialize the string of stuff we're copying
	$tex_mode_string = $eaten->exact_print;
    } # Start tex copying?

# Handle arguments
    # This token's next arguments -- returns a string matching /o*[rR]?/
    my $curr_args = $eaten->next_args($fileobject);

    if ($type eq "EndArgsToken" or $type eq "ArgToken") {
	# Print ending '}' for the group we just finished reading
	$tex_mode_string .= '}';
    }

    # If there could be optional arguments next, copy them
    while ($curr_args =~ s/^o// && $fileobject->lookAheadToken eq '[') {
	my $opt = $fileobject->eatOptionalArgument;
	$tex_mode_string .= $opt->exact_print;
    }
    $curr_args =~ s/^o*//; # Some OptArgs may not have appeared

    if ($type eq "BegArgsToken" or $type eq "ArgToken") {
	# Print beginning '{' for the group we're about to read
	$tex_mode_string .= '{';
    }

    # Now copy the next required argument, if any
    # Copy it verbatim (r), or translate it as regular LaTeX (R)?
    if ($curr_args =~ s/^r//) {
	my $group = $fileobject->eatRequiredArgument;
	my $temp = $group->exact_print;
	# Remove braces. They're put in explicitly
	$temp =~ s/\{(.*)\}/$1/; # .* is greedy
	$tex_mode_string .= $temp;

    } elsif ($curr_args =~ s/^R//) {
	print "\n" if $debug_on;
	&print_tex_mode($tex_mode_string);
	$tex_mode_string = "";
	print "\nTranslating this argument for ",$temp_start->print,
	      " as regular LaTeX" if $debug_on;

    } else { # anything but '' is weird
	warn "weird arg $curr_args to ",$temp_start->print,"\n" if $curr_args;
    }

# Finished tex mode copying
    if ($type eq "Token" or $type eq "EndArgsToken") {

	# Add {} to plain tokens followed by { or }. Otherwise {\foo}bar
	# and \foo{bar} yield \foobar in the LaTeX files created by LyX
	my $dummy;
	if ($type eq "Token" and
	        $dummy=$fileobject->lookAheadToken and
	        $dummy =~ /[{}]/)
	{
	    $tex_mode_string .= '{}';
	}

	# Print out the string
	print "\n" if $debug_on;
	&print_tex_mode($tex_mode_string);
	$tex_mode_string = "";

        # We're done with this token
	pop(@tex_mode_tokens);

	my $i = $type eq "Token" ? "" : " and its arguments";
	my $j = $temp_start->print;
	print "\nDone copying untranslatable token '$j'$i in TeX mode"
	                                                  if $debug_on;
    } # end tex copying?
} # end sub copy_latex_known

# Print a string in LyX "TeX mode"
#    The goal of this subroutine is to create a block of LyX which will be
# translated exactly back to the original when LyX creates its temporary LaTeX
# file prior to creating a dvi file.
#    Don't print \n\n at the end of the string... instead just set the new
# paragraph flag. Also, don't print whitespace at the beginning of the string.
# Print nothing if it's the beginning of a paragraph, or space otherwise.
# These two things avoid extra C-Enter's in the LyX file
sub print_tex_mode {
    my $outstr = shift;

    print "'$outstr'" if $debug_on;

    # Handle extra \n's (& spaces) at beginning & end of string
    my $str_ends_par = ($outstr =~ s/\n{2,}$//);
    if ($IsNewParagraph) {
        $outstr =~ s/^\s+//;  # .e.g, $outstr follows '\begin{quote}'
    } else {
	# Any whitespace is equivalent to one space in LaTeX
        $outstr =~ s/^\s+/ /; # e.g. $outstr follows "\LaTeX{}" or "Hi{}"
    }

    # Check whether string came right at the beginning of a new paragraph
    # This *might* not be necessary. Probably can't hurt.
    &CheckForNewParagraph;

    # Get into TeX mode
    print OUTFILE "$start_tex_mode";

    # Do TeX mode translation;
    $outstr =~ s/\\/\n\\backslash /g;
    # don't translate \n in '\n\backslash' that we just made!
    $outstr =~ s/\n(?!\\backslash)/\n\\newline \n/g;

    # Print the actual token + arguments if any
    print OUTFILE $outstr;

    # Get OUT of LaTeX mode (and end nesting if nec.)
    print OUTFILE "$end_tex_mode";
    $IsNewParagraph = $str_ends_par;

    return;
} # end sub print_tex_mode

############################  LAYOUT  SUBROUTINES  ###########################

sub CheckForNewParagraph {
# This subroutine makes sure we only write \layout command once per paragraph
#    It's mostly necessary cuz 'Standard' layout is the default in LaTeX;
#    there is no command which officially starts a standard environment
# If we're in a table, new layouts aren't allowed, so just return
# If $IsNewParagraph is 0, it does nothing
# If $INP==1, It starts a new paragraph
# If $CurrentAlignment is set, it prints the alignment command for this par.
# If $MayBeDeeper==1 and we're currently within a list environment,
#    it starts a "deeper" Standard paragraph
    my $dummy; 
    my $layout = $$CurrentLayoutStack[-1];

    return if &RelyxTable::in_table;

    if ($IsNewParagraph) {
	# Handle standard text within a list environment specially
	if ($MayBeDeeper) {
	    if ($layout =~ /^$ListLayouts$/o) {
		push (@$CurrentLayoutStack, "Standard");
		print "\nCurrent Layout Stack: @$CurrentLayoutStack\n"
		                                     if $debug_on;
		print OUTFILE "\n\\begin_deeper ";
		$layout = "Standard";
	    }
	    $MayBeDeeper = 0; # Don't test again until new paragraph
	}

	# Print layout command itself
	print OUTFILE "\n\\layout $layout\n\n";
	print OUTFILE $CurrentAlignment if $CurrentAlignment;

	# Now that we've written the command, it's no longer a new paragraph
	$IsNewParagraph = 0;

	# And we're no longer waiting to see if this paragraph is empty
	$PendingLayout = 0;

	# When you write a new paragraph, reprint any font commands
	foreach $dummy (keys %$CurrentFontStatus) {
	    my $currf = $CurrentFontStatus->{$dummy}->[-1];
	    if ($currf ne "default") {
		print OUTFILE "\n$dummy $currf \n";
	    }
	}
    } # end if $INP
} # end sub CheckForNewParagraph

sub ConvertToLayout {
# This subroutine begins a new layout, pushing onto the layout stack, nesting
# if necessary. It doesn't actually write the \layout command -- that's
# done by CheckForNewParagraph.
#    The subroutine assumes that it's being passed an environment name or macro
# which is known and which creates a known layout
#    It uses the ToLayout hash (created by the ReadCommands module) which
# gives the LyX layout for a given LaTeX command or environment
# Arg0 is the environment or macro
    my $name = shift;

    my $layoutref = $ReadCommands::ToLayout->{$name};
    my $layout = $layoutref->{'layout'};
    my $keepempty = $layoutref->{'keepempty'};
    my $dummy = ($name =~ /^\\/ ? "macro" : "environment");
    print "\nChanging $dummy $name to layout $layout" if $debug_on;

    # Nest if the layout stack has more than just "Standard" in it
    if ($#{$CurrentLayoutStack} > 0) {
	# Die here for sections & things that can't be nested!
	print " Nesting!" if $debug_on;
	print OUTFILE "\n\\begin_deeper ";
    }

    # If we still haven't printed the *previous* \layout command because that
    # environment is empty, print it now! (This happens if an environment
    # is nested inside a keepempty, like slide.)
    &CheckForNewParagraph if $PendingLayout;

    # Put the new layout onto the layout stack
    push @$CurrentLayoutStack, $layout;
    print "\nCurrent Layout Stack: @$CurrentLayoutStack" if $debug_on;

    # Upcoming text will be new paragraph, needing a new layout cmd
    $IsNewParagraph = 1;

    # Test for nested "Standard" paragraph in upcoming text?
    # Some environments can nest. Sections & Title commands can't
    $MayBeDeeper = $layoutref->{"nestable"};

    # We haven't yet read any printable stuff in the new paragraph
    # If this is a layout that's allowed to be empty, prepare for an
    # empty paragraph
    $PendingLayout = $keepempty;

} # end sub ConvertToLayout

sub EndLayout {
# This subroutine ends a layout, popping the layout stack, etc.
#    The subroutine assumes that it's being passed an environment name or macro
# which is known and which creates a known layout
#    It uses the ToLayout hash (created by the ReadCommands module) which
# gives the LyX layout for a given LaTeX command or environment
# Arg0 is the environment or macro
    my $name = shift;

    my $layoutref = $ReadCommands::ToLayout->{$name};
    my $layout = $layoutref->{'layout'};
    my $dummy = ($name =~ /^\\/ ? "macro" : "environment");
    print "\nEnding $dummy $name (layout $layout)" if $debug_on;

    # If we still haven't printed the *previous* \layout command because that
    # environment is empty, print it now! Before we pop the stack!
    # This happens for a totally empty, non-nesting environment,
    # like hollywood.sty's fadein
    &CheckForNewParagraph if $PendingLayout;

    my $mylayout = pop (@$CurrentLayoutStack);

    # If a standard paragraph was the last thing in a list, then
    #     we need to end_deeper and then pop the actual list layout
    # This should only happen if the Standard layout was nested
    #    in a nestable layout. We don't check.
    if ($mylayout eq "Standard") {
	print OUTFILE "\n\\end_deeper ";
	print " End Standard Nesting!" if $debug_on;
	$mylayout = pop (@$CurrentLayoutStack);
    }

    # The layout we popped off the stack had better match the
    #    environment (or macro) we're ending!
    if ($mylayout ne $layout) { die "Problem with Layout Stack!\n"};
    print "\nCurrent Layout Stack: @$CurrentLayoutStack" if $debug_on;

    # If we're finishing a nested layout, we need to end_deeper
    # This should only happen if the layout was nested
    #    in a nestable layout. We don't check.
    # Note that if we're nested in a list environment and the
    #     NEXT paragraph is Standard, then we'll have an extra
    #     \end_deeper \begin_deeper in the LyX file. It's sloppy
    #     but it works, and LyX will get rid of it when it
    #     resaves the file.
    if ($#{$CurrentLayoutStack} > 0) {
	print " End Nesting!" if $debug_on;
	print OUTFILE "\n\\end_deeper ";
    }

    # Upcoming text will be new paragraph, needing a new layout cmd
    $IsNewParagraph = 1;

    # Test for nested "Standard" paragraph in upcoming text?
    # Some environments can nest. Sections & Title commands can't
    $MayBeDeeper = $layoutref->{"nestable"};
} # end sub EndLayout

#######################  MISCELLANEOUS SUBROUTINES  ###########################
sub fixmath {
# Translate math commands LyX doesn't support into commands it does support
    my $input = shift;
    my $output = "";

    while ($input =~ s/
	    (.*?)    # non-token matter ($1)
	    (\\      # token ($2) is a backslash followed by ...
	        ( ([^A-Za-z] \*?) |    # non-letter (and *) ($4) OR
		  ([A-Za-z]+ \*?)   )  # letters (and *) ($5)
	    )//xs) # /x to allow whitespace/comments, /s to copy \n's
    { 
	$output .= $1;
	my $tok = $2;
	if (exists $ReadCommands::math_trans{$tok}) {
	    $tok = $ReadCommands::math_trans{$tok};
	    # add ' ' in case we had, e.g., \|a, which would become \Verta
	    # Only need to do it in those special cases
	    $tok .= ' ' if
	            defined $4 && $tok =~ /[A-Za-z]$/ && $input =~ /^[A-Za-z]/;
	}
	$output .= $tok;
    }
    $output .= $input; # copy what's left in $input

    return $output;
}

1; # return true to calling subroutine

