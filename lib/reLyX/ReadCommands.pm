# This file is part of reLyX
# Copyright (c) 1998-9 Amir Karger karger@post.harvard.edu
# You are free to use and modify this code under the terms of
# the GNU General Public Licence version 2 or later.

package ReadCommands;
# Read a file containing LaTeX commands and their syntax
# Also, read a file containing LyX layouts and their LaTeX equivalents

use strict; 

# Variables needed by other modules
# ToLayout is a ref to a hash which contains LyX layouts & LaTeX equivalents
# regular_env is a list of environments that have "reLyXable" LaTeX in them
# math_trans is a hash of math commands and what they're translated to
use vars qw($ToLayout @regular_env %math_trans);

#    CommandHash is the hash of LaTeX commands and their syntaxes which
# this package builds.
my %CommandHash;

# Name of the environment containing names of regular environments :)
my $regenv_name = "reLyXre";
# Name of environment containing translations of math commands
my $math_trans_name = "reLyXmt";
# Variables set when in the above environments
my ($in_regular_env, $in_math_trans);
my @Environments = qw($regenv_name $math_trans_name);

# This word in a command's argument (in the syntax file) means that reLyX
# should translate that argument as regular LaTeX
my $Translate_Word = "translate";

#########################  READ COMMAND SYNTAX  ################################
sub read_syntax_files {
# This subroutine calls the TeX parser & translator to read LaTeX syntax file(s)
#    It sets the list of "regular" environments, environments which aren't known
# by reLyX, but which contain text that reLyX can translate.
#    It also reads math commands which should be translated (e.g., \sp -> ^)
#
# @_ contains the syntax file(s) to read

    my @syntaxfiles = @_;

# Before anything else, set the package-wide variables based on the
#    user-given flags
    # opt_d is set to 1 if '-d' option given, else (probably) undefined
    my $debug_on = (defined($main::opt_d) && $main::opt_d);

    # opt_r is a group of environments to copy like regular text
    # If opt_r wasn't given, then there are no such special environments
    @regular_env = (defined $main::opt_r ? (split(/,/, $main::opt_r)) : () );

    # The only important commands to pass are \begin and \end, so that the
    # parser realizes they start/end environments, as opposed to being
    # regular old tokens.
    my %MyTokens = ( '{' => $Text::TeX::Tokens{'{'},
                     '}' => $Text::TeX::Tokens{'}'},
                     '\begin' => $Text::TeX::Tokens{'\begin'},
                     '\end' => $Text::TeX::Tokens{'\end'},
    );
    
    my $InFileName;
    foreach $InFileName (@syntaxfiles) {
	die "could not find syntax file $InFileName" unless -e $InFileName;
	my $zzz=$debug_on ? "from $InFileName " :"";
	warn "Reading LaTeX command syntax $zzz\n";

	# Open the file to turn into LyX.
	my $infile = new Text::TeX::OpenFile $InFileName,
	    'defaultact' => \&read_commands,
	    'tokens' => \%MyTokens;

	# When we start (each file), we're not reading regular environments yet
	$in_regular_env = 0;

	# Process the file
	$infile->process;
    }

    if ($debug_on) {
	print "Regular environments: @regular_env\n";
	my @mathkeys = keys(%math_trans);
	print "     Math command     |  translation\n" if @mathkeys;
	foreach (@mathkeys) { printf("%20s       %s\n",$_,$math_trans{$_}) }
    }

    #warn "Done reading commands\n";
    return;
} # end subroutine call_parser

sub read_commands {
# This subroutine is called by Text::TeX::process
# Arg0 is the token we just ate
# Arg1 is the file object we're reading from
#
#    We create a hash, where each command is a key. The value is just a string
# of zero or more 'o' and 'r' characters. Each 'o' stands for an optional
# argument, 'r' stands for a required argument. 'R' stands for a required
# argument whose text will be regular LaTeX, e.g., the argument to \mbox{}
#    In addition, the $regenv_name environment contains
# regular environments, like those input with the -r option.
#    Note that if a command is found more than once, then it wil be overwritten.
# This is a feature. This way, a user-defined syntax file can overwrite the
# argument list found in the default syntax file.
    my ($token,$fileobject) = (shift,shift);

    my $type = ref($token);
    $type =~ s/^Text::TeX::// or die "unknown token type $type from Text::TeX";
    #print $token->exact_print, unless $type eq "Paragraph";
    #print $token->comment if $token->comment;

    # Because there's no token list, ALL tokens will be
    #    Paragraph, Text, or Token
    SWITCH: for ($type) {
       # Handle blank lines.
        if (/Paragraph/) {
	    # don't do anything
	    last SWITCH;

        } elsif (/^Token/) {
	    # Comment in its own paragraph... skip
	    last SWITCH unless defined($token->print);

	    if ($in_math_trans) { # read translations of math commands
	        my $key = $token->print;
		# Translation is whatever's in the argument to the token
		# (There might be multiple tokens in there)
		my @vals = $fileobject->eatBalanced->contents;
		my $val = join ("", map {$_->exact_print} @vals);
		$math_trans{$key} = $val;
	    
	    } else { # regular portion of syntax file
		my ($dum2);
		my $args = "";
		# read while there are arguments
		while (($dum2 = $fileobject->lookAheadToken) &&
		       ($dum2 =~ /^[[{]$/)) {
		    if ($dum2 eq '[') { #eat optional argument - assumed simple
			$fileobject->eatOptionalArgument;
			$args .= "o";
		    } else {
			my $tok = $fileobject->eatBalanced or warn "bad group";
			if ($tok->exact_print eq $Translate_Word) {
			    $args .= "R";
			} else {
			    $args .= "r";
			}
		    } # end if $dummy = [{
		} # done reading command
		$CommandHash{$token->print} = $args;
	    } # in math trans env or regular token?

	    last SWITCH;

        } elsif (/^Begin::Group::Args/) {
	    my $env = $token->environment;
	    CASE: {
		$in_regular_env = 1, last CASE if $env eq $regenv_name;
		$in_math_trans = 1,  last CASE if $env eq $math_trans_name;
		warn "Unknown environment $env in syntax file";
	    }

        } elsif (/^End::Group::Args/) {
	    my $env = $token->environment;
	    CASE: {
		$in_regular_env = 0, last CASE if $env eq $regenv_name;
		$in_math_trans = 0,  last CASE if $env eq $math_trans_name;
		warn "Unknown environment $env in syntax file";
	    }

        } elsif (/^Text/) {
	    # don't do anything unless we're reading environments
	    if ($in_regular_env) {
		my @new_envs = (split(/\s+/, $token->print));
		@new_envs = grep ($_, @new_envs); # remove empty elements
		push @regular_env,@new_envs;
	    }
	    last SWITCH;
        } else {
	    die "unexpected token type $type";
	}

    } # end SWITCH

} # end sub read_commands

sub Merge {
#    This sub creates a token list (which could be used to call a Text::TeX
# parser) from %CommandHash, and merges it with the input token list
#    If a command takes any required arguments, it will be a report_args,
# but if it just takes an optional argument, it can stay a regular old token.
# In either case, we insert a new field, "relyx_args", into the token list,
# which is the expected order of arguments for that command. Even if there
# are no args, we insert an empty relyx_args, so that we can differentiate
# between a truly unknown token and a known token which takes no args.
#
# We don't allow a new command to override a command that already exists in
# OldHash, i.e., one that was defined explicitly in the calling sub.
#
# Arg0 is a (reference to an) existing token list
    my $OldHashRef = shift;

    foreach (keys %CommandHash) {
	my $val = $CommandHash{$_};
	my ($entry, $count, @foo);
        if (!exists $OldHashRef->{$_}) {
	    if ($count = scalar(@foo = ($val =~ /r/gi))) {
		# TeX.pm will make this a TT::BegArgsToken and $count-1
		#    TT::ArgTokens, followed by a TT::EndArgsToken
		$OldHashRef->{$_} = {"Type" => 'report_args',
		                     "count" => $count,
				     "relyx_args" => $val};
	    } else { # only non-required args
		# Make it a regular TT::Token, but write relyx_args
		#    (even if $val is "")
		$OldHashRef->{$_} = {"relyx_args" => $val};
	    }
	}
    } # end foreach

} # end sub Merge

############################  READ LAYOUTS  ####################################
sub read_layout_files {
# This subroutine reads a textclass-specific layout file and all files
# included in that file.
#    It sets up the layout hash table. For each environment, it describes which
# layout that environment refers to. It does the same for macros which
# begin LyX layouts (e.g., \section)
#    If we read a command that's not already in CommandHash, it means that this
# layout has some commands that aren't in syntax.default. If so, we ASSUME
# that the command takes just one required argument, and put it in
# CommandHash, so that &Merge will eventually put these commands into the
# token lists.
#
# TODO: we actually need to allow more sophisticated stuff. E.g. \foilhead
# is converted to Foilhead or ShortFoilHead (foils.layout) depending on whether
# the command has "r" or "or" arguments. Reading LatexParam (if it exists)
# can help us with this.
# Default is "r". Just unshift other args as you read them, since latexparam
# is put in between macro & the argument
# TODO: We need to store ToLayout s.t. we can have > 1 layout per command.
# Maybe by default just have one layout, but if (ref(layout)) then read
# more args & thereby figure out which layout?
# 
# Arg0 is the name of the documentclass
    use FileHandle;
    use File::Basename;
    my $doc_class = shift;
    my @filestack;
    my $fh;
    my $line;
    # ToLayout{latexname} stores layout; so ReversHash{layout} = latexname
    my %ReverseHash;
    my $debug_on = (defined($main::opt_d) && $main::opt_d);

    # look for layout file in $HOME/.lyx first, then system layouts directory
    my $searchname = "$doc_class.layout";
    my @searchdirs = ();
    my $personal_layout = "$main::dot_lyxdir/layouts";
    push(@searchdirs,$personal_layout) if -e $personal_layout;
    my $system_layout = "$main::lyxdir/layouts";
    # I guess this won't exist if running reLyX without installing...
    # Of course, in that case, this will probably break
    push(@searchdirs,$system_layout) if -e $system_layout;
    my @foundfiles = grep(-e "$_/$searchname", @searchdirs) or
          die "Cannot find layout file $searchname in dir(s) @searchdirs";
    my $LayoutFileName = "$foundfiles[0]/$searchname"; # take first one we found

    $fh = new FileHandle;
    $fh->open ("<$LayoutFileName");
    my $zzz=$debug_on ? "$LayoutFileName" :"";
    warn "Reading layout file $zzz\n";
    push @filestack, $fh;

    # Read the layout file!
    my ($lyxname, $latexname, $latextype, $latexparam, $keepempty);
    my $fname;
    while() {
	# Read a line. If eof, pop the filestack to return to the file
	#    that included this file *or* finish if the stack's empty
        unless (defined ($line = <$fh>)) {
	    $fh->close;
	    pop @filestack;
	    last unless ($#filestack+1); # finish when stack is empty
	    $fh = $filestack[-1];
	    next; # read another line from the "calling" file
	}

	# Skip blank lines
	next if $line =~ /^\s*$/;

	# Split the line. Use limit 2 since there may be whitespace in 2nd term
	my ($field_name, $field_stuff) = split(' ', $line, 2);
	$field_name = lc($field_name); # LyX is case insensitive for fields
	if (defined($field_stuff)) {
	    $field_stuff =~ s/^\"(.*)\"/$1/;
	    chomp ($field_stuff);
	    # Since split is limited to 2 fields, there may be extra whitespace
	    # at end. LyX breaks on a "\layout Abstract " command!
	    $field_stuff =~ s/\s*$//;
	}

	# This set of ifs deals with lines outside a style definition
	if ($field_name eq "style") { # start a style definition
	    $lyxname = $field_stuff;
	    # Styles in LyX have spaces, but _ in layout files
	    $lyxname =~ s/_/ /g;
	    $latexname  = ""; # make sure these variables are unset
	    $latextype  = "";
	    $latexparam = "";
	    $keepempty = 0;
	} elsif ($field_name eq "input") { #include a file
	    $searchname = $field_stuff;
	    @foundfiles = grep(-e "$_/$searchname", @searchdirs) or
	      die "Cannot find layout file $searchname in dir(s) @searchdirs";
	    $fname = "$foundfiles[0]/$searchname"; # take first one we found
	    $fh = new FileHandle;
	    push @filestack, $fh;
	    $fh->open("<$fname");
	    print "Reading included layout file $fname\n" if $debug_on;
	}

	next unless $lyxname; # not w/in style definition

	# This set of ifs deals with lines within a Style definition
	if ($field_name eq "latexname") {
	    $latexname = $field_stuff;
	    next;
	} elsif ($field_name eq "latexparam") {
	    #$dum = $field_stuff;
	    $latexparam = $field_stuff;
	    next;
	} elsif ($field_name eq "latextype") {
	    $latextype = $field_stuff;
	    next;
	} elsif ($field_name eq "keepempty") {
	    $keepempty = $field_stuff;
	    next;
	} elsif ($field_name eq "copystyle") { # copy an existing style
	    # "if" is necessary in case someone tries "CopyStyle Standard"
	    if (exists $ReverseHash{$field_stuff}) {
		my $layref = $ReverseHash{$field_stuff};
		$latexname  = $layref->{"name"};
		$latextype  = $layref->{"type"};
		$latexparam = $layref->{"param"};
		$keepempty = $layref->{"keepempty"};
	    }

	# When you get to the end of a definition, create hash table entries
	#    (if you've found the right information)
	} elsif ($field_name eq "end") {

	    if ($latextype and $latexname) {
		# Create reverse hash entry (needed for CopyStyle)
		# Do it before making modifications to $latexname, e.g.
		$ReverseHash{$lyxname} = {"name"  => $latexname,
		                          "type"  => $latextype,
					  "param" => $latexparam,
					  "keepempty" => $keepempty,
					  };

		my ($nest, $skip) = (0,0);
		for ($latextype) { # make $_=$latextype
		    if (/^Command/) {
			# Macros need a '\' before them. Environments don't
			$latexname = '\\' . $latexname;

			# Create the command if it wasn't in syntax.default
			unless (exists $CommandHash{$latexname}) {
			    $CommandHash{$latexname} = "r";
			}

		    } elsif (/^Environment/) {
			$nest = 1;
		    } elsif (/Item_Environment/i || /List_Environment/) {
			$nest = 1;

		    # layout Standard has LatexType Paragraph. It shouldn't
		    #    have any hash entry at all
		    } elsif (/^Paragraph$/) {
		        $skip = 1;
		    } else {
			warn "unknown LatexType $latextype" . 
			     "for $latexname (layout $lyxname)!\n";
		    }
	    # Handle latexparam, if any
#	    if ($dum =~ s/^"(.*)"/$1/) { # newer layout file syntax
#		while ($dum =~ /^[[{]/) {
#		    $dum =~ s/\[.*?\]// && ($latexargs = "o$latexargs") or
#		    $dum =~ s/\{.*?\}// && ($latexargs = "r$latexargs");
#		}
#		warn "leftover LatexParam stuff $dum" if $dum;
#	    } else { # 0.12.0
#		if ($latextype eq "Command") {optarg}
#		else {req. arg}
#	    }
		} #end for

		# Create the hash entry
		unless ($skip) {
		    $ToLayout->{$latexname} = {"layout" => $lyxname,
						"nestable" => $nest,
						"keepempty" => $keepempty};
		}

		# Now that we've finished the style, unset $lyxname so that
		#     we'll skip lines until the next style definition
		$lyxname = "";
	    } # end if ($latextype and $latexname)
	} # end if on $line
	    
    } #end while

## Print every known layout
#    print "     LatexName            Layout        Keepempty?\n";
#    foreach (sort keys %$ToLayout) {
#        printf "%20s%15s     %1d\n",$_,$ToLayout->{$_}{'layout'},
#    	      $ToLayout->{$_}{'keepempty'};
#    };
    #warn "Done reading layout files\n";
    return;
} # end sub read_layout_files



1; # return TRUE to calling routine
