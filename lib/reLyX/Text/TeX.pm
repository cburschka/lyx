package Text::TeX;

# This file is copyright (c) 1997-8 Ilya Zakharevich
# Modifications for reLyX by Amir Karger
# You are free to use and modify this code under the terms of
# the GNU General Public Licence version 2 or later.

#
#use strict;
#use vars qw($VERSION @ISA @EXPORT);

#require Exporter;
#require # AutoLoader;	# To quiet AutoSplit.

# @ISA = qw
# (Exporter AutoLoader);
# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.
@EXPORT = qw(
	
);
$VERSION = '0.01';


# Preloaded methods go here.

# Does not deal with verbatims
# Spaces are treated bad.

#####################      GENERAL NOTES      ##################################
# Each package describes a different sort of token.
# Packages:
#    Chunk        - default, just used as an ISA
#    Text         - plain text, made up of TT::$usualtokenclass stuff
#    Paragraph    - new paragraph starting (cuz you got \n\n in a latex file)
#    Token        - simple token, like ~ or \blah
#    EndLocal     - pseudotoken meaning that the scope of a local command (like
#                   '\large') has ended
#    BegArgsToken - pseudotoken which takes one or more arguments, like \section
#    ArgToken     - pseudotoken returned in between arguments to a BegArgsToken
#    EndArgsToken - pseudotoken returned after we finish getting arguments
#                   to a BegArgsToken
#    LookAhead    - a special kind of EndArgsToken when you want to look ahead
#    BegArgsTokenLookedAhead - special kind of BegArgsToken (see man page)
#    Begin::Group - Beginning of a group, i.e., '{'
#    End::Group   - End of a group, i.e., '}'
#    Begin::Group::Args - begin group but get args first, i.e., '\begin'
#    End::Group::Args   - end group but get args first, i.e., '\end'
#    SelfMatch    - e.g., '$'. Matches itself, but otherwise like a Begin::Group
#    Separator    - e.g., '&' (not used in reLyX)
#    Comment      - (not used in reLyX)
# 
# The main package is TT::OpenFile. It contains the subroutines that do
#    most of the parsing work. TT::GetParagraph does some stuff too, but
#    it's not a token you'd expect the code to return
#
# Package subroutines (other than 'new'):
#    refine - takes a token to a more specific kind of token type
#               e.g., '{' goes from TT::Token to TT::Begin::Group
#    digest - extra actions to do once you've eaten the token.
#               e.g., eating arguments of \begin, or popping various
#               stacks when you get to an End::Group
#    print  - how to print the token (e.g., the text making up the token)
#    exact_print - print the token exactly as it appeared in the file.
#               Usually involves adding whitespace
#
# Token and pseudotokens have some more subs:
#    base_token   - the token this token is created from. It's the token
#                   itself for a Token, but not for pseudotokens
#    token_name   - the name of the base_token
#
# Token structure:
# $tok->[0] will usually be the word (e.g., '\blah') the parser read
#      For pseudotokens, it's something more complicated
#      (some tokens, like Paragraph have nothing there, though)
# $tok->[1] will be any comment (usually ignored)
# $tok->[2] will be the exact thing the parser read (usu. [0] plus whitespace)
# $tok->[3] stores arguments for Begin::Group::Args and End::Group::Args
# $tok->[4] stores pointer to beginning token for End::Group::Args
#    A TT::Group is a reference to an array of tokens. Often (but not always),
# the first and last groups are Begin::Group and End::Group tokens respectively.

# Pseudotokens are objects, one of whose fields is a reference to the token
# that created the pseudotoken
# BegArgToken, ArgToken, EndArgToken pseudotokens:
# $tok->[0][0] - token (e.g. a TT::Token) that begins this group
# $tok->[0][1] - number of arguments that that token takes
# $tok->[0][2] - (found only in ArgToken) number of arguments to the token
#                that have been read so far
################################################################################

########################   GLOBAL VARIABLES   ##################################
# Sorts of text you find in a LaTeX file. For matching
$notusualtoks = "\\\\" . '\${}^_~&@%'; # Why \\\\? double interpretation!
$notusualtokenclass = "[$notusualtoks]";
$usualtokenclass = "[^$notusualtoks]";

# Original $macro wouldn't recognize, e.g., '\section*'. Added '\*?' - Ak
# (Had to add it for \section and \\ separately.)
#    \" or \frac, e.g. Note that it eats whitespace AFTER the token. This is
# correct LaTeX behavior, but if text follows such a macro, and you just
# print out the macro & then the text, they will run together.
$macro = '\\\\(?:[^a-zA-Z]\*?|([a-zA-Z]+\*?)\s*)'; # Has one level of grouping
#$macro = '\\\\(?:[^a-zA-Z]|([a-zA-Z]+)\s*)'; # Contains one level of grouping

# active is a backslashed macro or $$ (same as \[) or ^^ followed by a char
#    (^^A means ASCII(1), e.g. See the TeXbook) or a special character like ~
$active = "$macro|\\\$\\\$|\\^\\^.|$notusualtokenclass"; # 1 level of grouping

# In TeX, ^joe is equivalent to ^{j}oe, so sometimes we use tokenpattern
#     instead of multitokenpattern to get just one character
$tokenpattern = "($usualtokenclass)|$active"; # Two levels of grouping
$multitokenpattern = "($usualtokenclass+)|$active"; # Two levels of grouping

# Note: In original (CPAN) version, $commentpattern had "". It needs ''
# or otherwise '\s' gets translated to 's'
$commentpattern = '(?:%.*\n\s*)+'; #one or more comment lines
$whitespaceAndComment = '\s*(%.*\n[ \t]*)+';

# matches either nothing OR an argument in brackets ($1 doesn't include [])
$optionalArgument = "(?:\\[([^]]*)\\])?"; # Contains one level of grouping

# These tokens are built from other tokens, so they're pseudotokens
#    (except BegArgsToken actually does have text!?)
for (qw(Text::TeX::ArgToken Text::TeX::BegArgsToken Text::TeX::EndArgsToken )) {
  $pseudo{$_} = 1;
}

# More global variables can be found at the end of the file
# E.g., the main Tokens hash

#######################   Token Packages   #####################################
{
  package Text::TeX::Comment;
  $ignore = 1;
}

{
  package Text::TeX::Chunk;
  sub refine {}
  sub digest {}
  sub collect {$_[0]->[0]}
  sub new {
    my $class = shift;
    bless [@_], $class;
  }
  sub print {$_[0]->[0]}
  # exact_print prints the *exact* text read, including whitespace
  #     (but not including comments...)
  sub exact_print {$_[0]->[2]}
  # print the comment that came before a token
  sub comment {$_[0]->[1]}

}

{
  package Text::TeX::Token;
  @ISA = ('Text::TeX::Chunk');

  sub refine {
    my $self = shift;
    return undef unless defined $self->[0];
    my $txt = shift;
    my $type;
    if (defined ($tok = $txt->{tokens}->{$self->[0]}) 
	and defined $tok->{class}) {
      bless $self, $tok->{class};
    }
  } # end sub refine

  # Name of the token. Same as print for Token, but ArgToken and
  # EndArgsToken, e.g., print nothing!
  sub token_name {
      my $tok = shift->base_token;
      return $tok->print;
  }

  sub base_token {
  # For pseudotokens, this sub is more complicated, but a token is just a token.
      return shift;
  }

  # return the syntax argument created by reLyX
  # Return "" if relyx_args is empty, i.e., if the token takes no args
  # Return undef if relyx_args doesn't exist, i.e., if the token is unknown
  sub relyx_args {
      warn "not enough args to Text::TeX::relyx_args" unless @_==2;
      my ($tok,$object) = (shift, shift);
      my $name;

      # Test copied from TT::OpenFile::eat
      if (defined ($name = $tok->token_name)) {
	  #print "$name is defined\n";
	  if (defined ($entry = $object->{"tokens"}->{$name})) {
	      #print "Entry in MyTokens is defined\n";
	      if (exists ($entry->{"relyx_args"})) { # even if it's empty...
		  #print "the args are '",$entry->{"relyx_args"},"'\n";
		  return $entry->{"relyx_args"}
	      }
	  }
      }

      # else...
      #print "did not exist";
      return undef;
  } # end sub relyx_args

  sub next_args {
  # Return the next argument(s) expected by this token.
  # For regular Tokens: /^o*$/. 
  # For BegArgsTokens and ArgTokens: /^o*[rR]$/
  # For EndArgsTokens: /^o*/. (in case opt args come after last required arg)
    my ($eaten,$fileobject) = (shift,shift);

    # Get the number & type of arguments of this token == /^[or]*$/
    # If it takes no args, just return
    # Will also return if curr_args is called for plain Text for some reason
    my $syntax = $eaten->relyx_args($fileobject) or return "";

    # If it takes just optional args, return them (it's a plain Token)
    return $syntax if $syntax =~ /^o+$/;

    # Number of arguments we've already read (== 0 for BegArgsToken)
    # Note that we only get here for Beg/EndArgsToken or ArgToken
    my $arg_num = $eaten->args_done;

    # Split args into single "argument sets", each of which is 0 or more
    # optional arguments followed by 0 or 1 required argument.
    @args = ($syntax =~ /o*[rR]?/g);
    push (@args,""); # necessary for EndArgsToken if $syntax ends with "r"

    # Now return the n'th argument set
    #    e.g., if 0 args have been eaten, return the 0th element of @args,
    # which is the first argument
    return $args[$arg_num];
  } # end sub curr_args
} # end package Text::TeX::Token

{
  package Text::TeX::BegArgsToken;
  @ISA = ('Text::TeX::Token');
  sub print {
      my $tok = shift->base_token; # Token this pseudotoken was made from
      return $tok->print;
  }

  sub exact_print {
      my $tok = shift->base_token;
      return $tok->exact_print;
  }
  
  # How many arguments we've read already.
  # Obviously zero before we've begun to read the arguments
  sub args_done {return 0}

  sub base_token { return shift->[0]->[0] }
  sub comment { return shift->base_token->comment }
}

{
  package Text::TeX::ArgToken;
  @ISA = ('Text::TeX::Token');
  # This token isn't made from actual text, so it prints nothing
  sub print {return ''}
  sub exact_print {return ''}

  # How many arguments we've read already.
  # Luckily, this number is stored in the ArgToken token
  sub args_done { return shift->[0]->[2] }

  sub base_token { return shift->[0]->[0] }
}

{
  package Text::TeX::EndArgsToken;
  @ISA = ('Text::TeX::Token');
  # This token isn't made because of real text, so it prints nothing
  sub print {return ''}
  sub exact_print {return ''}

  # How many arguments we've read already.
  # Obviously the total number of arguments, since we're done
  sub args_done {return shift->[0]->[1]}
  sub base_token { return shift->[0]->[0] }
}

{
  package Text::TeX::EndLocal;
  @ISA = ('Text::TeX::Token');
  # No text in this token
  sub print {return ''}
  sub exact_print {return ''}
  sub base_token { return shift->[0] }
}

{
  package Text::TeX::Group;
  sub new {shift; my $in = shift; bless $in}
  sub print {
    local @arr; #arr becomes global for called subroutines
    foreach (@{ $_[0] }) {
      push(@arr, $_->print);
    }
    "`" . join("',`", @arr) . "'";
  }

  # exact_print prints w/out the quotes
  sub exact_print {
    local @arr; #arr becomes global for called subroutines
    foreach (@{ $_[0] }) {
      push(@arr, $_->exact_print);
    }
    join("", @arr); # ... and return it
  }

  # Not created straight from LaTeX, so it'll never have a comment
  # (although comments can be in the subtokens in the group)
  sub comment {undef}

  # Return what's in the group, i.e. strip out the '{' and '}' tokens
  # if they exist. Return an array of tokens or just one token
  sub contents {
      #strip off TT::Begin::Group and TT::End::Group from beginning and end
      # if they exist. eatBalanced will return Tokens, so don't worry about
      # stripping too much from a group like {{foo} bar}. And eatGroup
      # will return Begin::Group, Group, End::Group, so after stripping one,
      # don't have to worry about stripping another.
      $group = shift;
      if (ref($group->[0] ) eq "Text::TeX::Begin::Group" and
          ref($group->[-1]) eq "Text::TeX::End::Group")
      {
	  shift @$group;
	  pop @$group;
      }

      if (wantarray) {
          return @$group;
      } elsif (!@$group) { # group was '{}'
          return new Text::TeX::Token '','',''; # send back an empty token
      } else {
          warn "Text::TeX -- more than one token in group!" if $#$group > 1;
          return $$group[0];
      }
  }
}

{
  package Text::TeX::End::Group;
  @ISA = ('Text::TeX::Chunk');
  sub new {shift; my $in = shift; bless \$in}
  sub digest {			# 0: the token, 1: text object
    # If there are any EndLocal tokens in $txt->{presynthetic}, do them first
    # See TT::OpenFile::check_presynthetic for details
    return if $_[1]->check_presynthetic($_[0]);	# May change $_[0]
    my $wa = $_[1]->curwaitforaction;
    my $w = $_[1]->popwait;
    warn "Expecting `$w', got `$_[0][0]'=`$_[0][0][0]' in `$ {$_[1]->{paragraph}}'" 
      if $w ne $_[0]->[0];
    &$wa if defined $wa; # i.e., do $txt->{waitforactions}[-1] if it exists
  }
}

{
  package Text::TeX::End::Group::Args;
  @ISA = ('Text::TeX::End::Group');

  sub digest {			# 0: the token, 1: text object
    # If there are any EndLocal tokens in $txt->{presynthetic}, do them first
    #    (Lamport p. 27 says \em is ended by '\end{blah}', not just '}')
    # check_presynthetic will put the End::Group::Args token into pending_in
    #    so it'll be read on the next pass through eat. Since sub digest will
    #    be called again on this token, don't read the argument to \end{}
    #    on the first call to sub digest
    # See TT::OpenFile::check_presynthetic for details
    return if $_[1]->check_presynthetic($_[0]);	# May change $_[0]

    my $Token = $_[1]->{tokens}->{$_[0]->[0]};
    my $count = $Token->{eatargs};
    my ($tok, @arr);
    # Read environment you're ending (just like in Begin::Group::Args)
    while ($count--) {
      $tok = $_[1]->eatGroup(1);
      if (@$tok == 3 and $tok->[0]->[0] eq '{') { # Special case for {\a}
	$tok = $tok->[1];
      }
      push(@arr,$tok);
    }
    #$_[0]->[0] .= ' ' . join ' ', map $_->[0], @arr;
    $_[0]->[3] = \@arr;
    my $s = $_[1]->starttoken;

    # like TT::End::Group
    my $wa = $_[1]->curwaitforaction;
    my $w = $_[1]->popwait;
    # If you got '}' when you wanted '\end'
    warn "Expecting `$w', got $_[0]->[0] in `$ {$_[1]->{paragraph}}'" 
      if $w ne $_[0]->[0];
    # If you got \end{foo} when you wanted \end{bar}
    if ($Token->{selfmatch} and $s->environment ne $_[0]->environment) {
      warn "Expecting `$w" , "{", $s->environment,"}', got $_[0]->[0]",
	"{", $_[0]->environment , "} in `$ {$_[1]->{paragraph}}'";
    }

    # If there was a waitforaction then do it now
    &$wa if defined $wa;
    $_[0]->[4] = $s;		# Put the start data into the token
  }

  sub print { # need special print to print name of environment
      my $obj = $_[0];
      my $env = $obj->environment; # assume we've already digested it
      # Use the method for printing a regular old token, but append env. name
      return $obj->SUPER::print . "{$env}";
  }

  sub exact_print {
      my $obj = $_[0];
      my $env = $obj->environment; # assume we've already digested it
      # Use the method for printing a regular old token, but append env. name
      return $obj->SUPER::exact_print . "{$env}";
  }

  sub environment {
  # this group's environment
      return $_[0]->[3]->[0]->[0];
  }
} # end package TT::End::Group::Args

{
  package Text::TeX::Begin::Group::Args;
  @ISA = ('Text::TeX::Begin::Group');

  sub digest {			# 0: the token, 1: text object
    my $Token = $_[1]->{tokens}->{$_[0]->[0]};
    my $count = $Token->{eatargs};
    my ($tok, @arr);
    # Read the arguments, e.g., read "{blah}" for "\begin{blah}"
    while ($count--) {
      $tok = $_[1]->eatGroup(1);
      if (@$tok == 3 and $tok->[0]->[0] eq '{') { # Special case for {\a}
	$tok = $tok->[1];
      }
      push(@arr,$tok);
    }
    # $_[0]->[0] .= ' ' . join ' ', map $_->[0], @arr;
    $_[0]->[3] = \@arr;
    $_[0]->SUPER::digest($_[1]); # i.e. do Begin::Group stuff (pushwait)
  }

  sub print { # need special print to print name of environment
      my $obj = $_[0];
      my $env = $obj->environment; # assume we've already digested it
      # Use the method for printing a regular old token, but append env. name
      return $obj->SUPER::print . "{$env}";
  }

  sub exact_print {
      my $obj = $_[0];
      my $env = $obj->environment; # assume we've already digested it
      # Use the method for printing a regular old token, but append env. name
      return $obj->SUPER::exact_print . "{$env}";
  }

  sub environment {
  # this group's environment
      return $_[0]->[3]->[0]->[0];
  }
} # end package TT::Begin::Group::Args

{
  package Text::TeX::Begin::Group;
  @ISA = ('Text::TeX::Chunk');
  # 0: the token, 1: text object
  sub digest {
      my ($tok, $txt) = (shift, shift);
      # $dummy = the anonymous hash associated with this token in the %Tokens
      my $dummy = $txt->{tokens}->{$tok->[0]};

      # see if this group requires different actions
      my $newaction; # action to do while parsing this group
      my $waitaction; # action to do when you hit the matching End::Group
      undef $waitaction; undef $newaction;
      if (defined $dummy) {
          if (exists $dummy->{newaction}) {
	      $newaction = $dummy->{newaction};
	  }
	  if (exists $dummy->{waitaction}) {
	      $waitaction = $dummy->{waitaction};
	  }
      }

      # push stuff onto stacks for this group
      $txt->pushwait($tok, $newaction, $waitaction);
  }
}

{
  package Text::TeX::SelfMatch;
  @ISA = ('Text::TeX::Chunk');
  sub refine {
  # This subroutine is never used. See sub digest below
    if ($_[1]->curwait eq $_[0]->[0]) {  #if you match what you're waiting for
      bless $_[0], Text::TeX::End::Group;
    } else { #you need to BE matched
      bless $_[0], Text::TeX::Begin::Group;
    }
  }
  # 0: the token, 1: text object
  # Unfortunately, this sub IS necessary, because originally, a '$' (e.g.)
  #    is type TT::Token. Calling refine calls Chunk::refine, which blesses
  #    it to SelfMatch, but then SelfMatch::refine is never called! -Ak
  sub digest {			# XXXX Should not be needed?
    # curwait returns undefined if not waiting for anything
    if (defined ($cwt = $_[1]->curwait) && $cwt eq $_[0]->[0]) { 
      bless $_[0], Text::TeX::End::Group;
      $_[0]->Text::TeX::End::Group::digest($_[1]);
    } else {
      bless $_[0], Text::TeX::Begin::Group;
      $_[1]->pushwait($_[0]);
    }
  }
}

@Text::TeX::Text::ISA = ('Text::TeX::Chunk');
@Text::TeX::Paragraph::ISA = ('Text::TeX::Chunk');
@Text::TeX::BegArgsTokenLookedAhead::ISA = ('Text::TeX::BegArgsToken');
@Text::TeX::LookAhead::ISA = ('Text::TeX::EndArgsToken');
@Text::TeX::Separator::ISA = ('Text::TeX::Chunk');

########################   MAIN CODE   #########################################
{
  package Text::TeX::GetParagraph;
  # Get a new paragraph from the LaTeX file
  # Get stuff until a non-empty line which follows an empty line
  sub new {
    shift; 
    my $file = shift;
    my $fh;
    $fh = $ {$file->{fhs}}[-1] if @{$file->{fhs}};
    return undef if (not defined $fh or eof($fh)) and $file->{readahead} eq "";

    # See below: every time we call GetParagraph, we read one extra (non-empty)
    #    line, which we store in readahead for next time
    my $string = $file->{readahead};
    $file->{readahead} = ""; #default in case eof($fh) or !defined($fh)

    if (defined $fh) { # i.e., if eof($fh) just return readahead from last time
      # Read until an empty line (or eof)
      while (defined ($in = <$fh>)  && ($in =~ /\S/)) { # $in undefined at eof
	$string .= $in;
      }
      # $in has the empty line we just read in. Add it for verbatim copying
      $string .= $in if defined $in; # add whitespace

      # Now read until NON-empty line (or eof)
      while (defined ($in = <$fh>) && ($in !~ /\S/)) {
	$string .= $in;
      }

      # Next time, the paragraph will begin with the non-empty line we just read
      $file->{readahead} = $in if defined $in; # readahead stays "" at eof
    }

    bless \$string; # ... and return it
  }
}


{
  package Text::TeX::OpenFile;

  $refgen = "TeXOpenFile0000";

  sub new {
# Description of OpenFile object:
# readahead - every time we read a paragraph we read one extra token. This 
#             token goes into 'readahead' and is prepended to the next paragraph
#             we read
# paragraph - stores the paragraph we're currently parsing
# actions   - what to do. TT::OpenFile->process calls the function pointed
#             to by actions on each token it eats
# tokens    - reference to a hash describing all tokens that the parser
#             should recognize
# presynthetic - holds pseudotokens to deliver before a block ends.
#             Specifically, it holds EndLocal tokens, so that we know to end
#             a command like \em just before the '}' which ends a group
# synthetic - holds pseudotokens to deliver after block ends - specifically,
#             it holds ArgToken (and EndArgsToken) tokens, which it returns
#             in between arguments (and after all arguments) to a command.
#             (also holds LookAhead tokens, which are like EndArgsTokens)
# pending_in - pseudotokens for input. Stuff is put here from synthetic or
#             from pending_out, and if there's something in pending_in, sub
#             eat doesn't bother eating a new token
# pending_out - pseudotokens for output -- stuff put here from presynthetic
#             If there's anything in pending_out it gets returned or put into
#             pending_in, and sub eat doesn't bother eating a new token
    shift; my $file = shift; my %opt = @_;
    if (defined $file) {
       ++$refgen;
       open("::$refgen",$file) || die "Cannot open $file: $!";
       die "End of file `$file' during opening" if eof("::$refgen");
    }
    my $fhs = defined $file ? ["::$refgen"] : [];
    bless {  fhs => $fhs, 
	     readahead => ($opt{string} || ""), 
	     files => [$file],
	     "paragraph" => undef, 
	     "tokens" => ($opt{tokens} || \%Text::TeX::Tokens),
	     waitfors => [], options => \%opt,
	     waitforactions => [],
	     defaultacts => [$opt{defaultact}],	# The last element is
                                                # the default action
                                                # for next deeper
                                                # level
	     actions => [defined $opt{action} ? 
			 $opt{action} : 
			 $opt{defaultact}],
	     waitargcounts => [0],
	     pending_out => [],
	     pending_in => [],
	     synthetic => [[]],
	     presynthetic => [[]],
	   };
  }
  sub DESTROY {
    my $in = shift; my $i = 0;
    for (@{$in->{fhs}}) {
      close($_)
	|| die "Cannot close $ {$in->{files}}[$i]: $!";
      $i++;
    }
  }

# Return the paragraph we're currently reading
#    If called with an argument, get a new paragraph at end of par, otherwise
# don't. (Useful for looking ahead without affecting the file we're reading)
# Either way, return nothing at end of par.
  sub paragraph {
    my $in = shift;
    my $get_paragraph = defined(shift);
    #print "ep.in=$in\n";

    # Return something if not at end of par
    if ($in->{"paragraph"} and $ {$in->{"paragraph"}} ne "") {
      $in->{"paragraph"};
    # Done with all files and readahead?
    } elsif (@{$in->{fhs}} and eof($ {$in->{fhs}}[-1]) and !$in->{readahead}) {
      undef;
    # No files and done with readahead?
    } elsif (!@{$in->{fhs}} and $in->{readahead} eq '') {
      undef;
    } else {
      if ($get_paragraph) {
	  #warn "getting new\n";
	  $in->{"paragraph"} = new Text::TeX::GetParagraph $in;
      }
      return "";
    }
  }

# pushwait means don't do stuff you've got waiting (like EndLocal tokens)
#    until you're done with something else
# If Arg2 exists, then the upcoming group will have it as its action
# If Arg3 exists, then we'll do it when we get to the end of the upcoming group
  sub pushwait {		# 0: text object, 1: token, 2: ????
    push(@{ $_[0]->{starttoken} }, $_[1]);
    push(@{ $_[0]->{waitfors} }, $_[0]->{tokens}{$_[1]->[0]}{waitfor});
    push(@{ $_[0]->{actions} }, 
	 defined $_[2] ? $_[2] : $_[0]->{defaultacts}[-1]);
    push(@{ $_[0]->{waitforactions} }, $_[3]);
    push(@{ $_[0]->{synthetic} }, []);
    push(@{ $_[0]->{presynthetic} }, []); # so that a local argument won't
                                 # finish at end of the nested group
  }

# You've finished a group, so pop all the stuff pushwait pushed on
  sub popwait {
    if ($#{ $_[0]->{waitfors} } < 0) {
      warn "Got negative depth"; return;
    }
    my $rest = pop(@{ $_[0]->{synthetic} });
    warn "Not enough arguments" if @$rest;
    $rest = pop(@{ $_[0]->{presynthetic} });
    warn "Presynthetic events remaining" if @$rest;
    pop(@{ $_[0]->{starttoken} });
    pop(@{ $_[0]->{actions} });
    pop(@{ $_[0]->{waitforactions} });
    pop(@{ $_[0]->{waitfors} });
  }

# If there's anything in synthetic, pop it, reverse it, push it onto pending_out
  sub popsynthetic {
    my $rest = $ { $_[0]->{synthetic} }[-1];
    if (@$rest) {
      push @{ $_[0]->{pending_out} }, reverse @{ pop @$rest };
    } 
  }

  sub pushsynthetic {		# Add new list of events to do *after* the
                                # next end of group.
    my $rest = $ { shift->{synthetic} }[-1];
    push @$rest, [@_];
  }

  sub addpresynthetic {		# Add to the list of events to do *before*
                                # the next end of group $uplevel above.
    my ($txt) = (shift);
    my $rest = $ { $txt->{presynthetic} }[-1];
    push @$rest, @_;
#    if (@$rest) {
#      push @{ @$rest->[-1] }, @_;
#    } else {
#      push @$rest, [@_];
#    }
  }

# If anything exists in presynthetic[-1], pop it and CHANGE $_[1] to that.
#    Push $_[1] AND (reverse of) anything else in presynthetic[-1] onto
#    pending_in so that we do it before any more tokens are read.
# Otherwise, just return false.
# BUG?! I don't understand why we do reverse. It makes stuff come out FIFO!
  sub check_presynthetic {	# 0: text, 1: end token. Returns true on success
    if (@{ $_[0]->{presynthetic}[-1] }) {
      my $rest = $_[0]->{presynthetic}[-1];
      my $next = pop @$rest;
      push @{ $_[0]->{pending_in} }, $_[1], (reverse @$rest);
      $#$rest = -1;		# Delete them
      $_[1] = $next;
      return 1;
    }
  }
  

  sub curwait {
  # return what we're currently waiting for. Returns undef if not waiting
    my $ref = $_[0]->{waitfors}; $$ref[-1];
  }

  sub curwaitforaction {
    my $ref = $_[0]->{waitforactions}; $$ref[-1];
  }

  sub starttoken {
    my $ref = $_[0]->{starttoken}; $$ref[-1];
  }

  # These are default bindings. You probably should override it.

# Eat '[blah]' or nothing. Brackets aren't returned in token's [0]
#    but they are returned in [2], so exact_print will print them.
  sub eatOptionalArgument {
    # Call with no arg. Don't get new paragraph if at end of par
    my $in = shift->paragraph;
    return undef unless defined $in;
    my $comment = ( $$in =~ s/^\s*($Text::TeX::commentpattern)//o );
    if ($$in =~ s/^\s*$Text::TeX::optionalArgument//o) {
      new Text::TeX::Token $1, $comment, $&;
    } else {
      warn "No optional argument found";
      if ($comment) {new Text::TeX::Token undef, $comment}
      else {undef}
    } 
  }

# eat {blah} when it's an argument to a BegArgsToken.
# Returns a TT::Group of refined tokens
#    This sub calls popsynthetic, so an ArgToken or EndArgsToken will be
# popped from synthetic into pending_in. This means that the ArgToken or
# EndArgsToken will be the next token returned by sub eat!
  sub eatRequiredArgument {
      my $txt = shift;
      my $group = $txt->eatGroup(@_);
      $txt->popsynthetic;
      return $group;
  }

  sub eatFixedString {
    # Call with no arg. Don't get new paragraph if at end of par
    my $in = shift->paragraph;
    return undef unless defined $in;
    my $str = shift;
    my ($comment) = ( $$in =~ s/^\s*($Text::TeX::commentpattern)//o );
    if ($$in =~ s/^\s*$str//) {new Text::TeX::Token $&, $comment, $&}
    else {
      warn "String `$str' expected, not found";
      if ($comment) {new Text::TeX::Token undef, $comment}
      else {undef}
    } 
  }

# Eat '{blah}'. Braces aren't returned. Stuff is returned as a Group,
#   where each member is an (unrefined) TT::Text or Token
  sub eatBalanced {
    my $txt = shift;
    my ($in);
    warn "Did not get `{' when expected", return undef
      unless defined ($in = $txt->eatFixedString('{')) && defined ($in->[0]);
    $txt->eatBalancedRest;
  }

# Eat 'blah}'
  sub eatBalancedRest {
    my $txt = shift;
    my ($count,$in,@in) = (1);
  EAT:
    {
      warn "Unfinished balanced next", last EAT 
	unless defined ($in = $txt->eatMultiToken) && defined $in->[0];
      push(@in,$in);
      $count++,redo if $in->[0] eq '{';
      $count-- if $in->[0] eq '}';
      # if !$count, remove '}' you just read and exit, else keep going
      pop(@in), last EAT unless $count;
      redo EAT;
    }
    bless \@in, 'Text::TeX::Group';
  }

# Eat stuff, either a token or a group (within {})
#    Tokens will be refined.
#    Braces ARE in the group
  sub eatGroup {		# If arg2==1 will eat exactly one
                                # group, otherwise a group or a
                                # multitoken.
    my $txt = shift;
    local ($in,$r,@in); #Note, this is a stupid way to name variables -Ak
    if (defined ($in[0] = $txt->eatMultiToken(shift)) and defined $in[0]->[0]) {
      $in[0]->refine($txt);
      if (ref $in[0] ne 'Text::TeX::Begin::Group') {
	return $in[0];
      } else { #it is the beginning of a group. So recurse until End::Group
	while (defined ($r=ref($in = $txt->eatGroup)) # Eat many groups
	       && $r ne 'Text::TeX::End::Group') {
	  push(@in,$in);
	}
	if (defined $r) {push(@in,$in)}
	else {warn "Uncompleted group"}
      } # end if Begin::Group
    } else {
      warn "Got nothing when argument expected";
      return undef;
    }
    bless \@in, 'Text::TeX::Group';
  }

  sub eatUntil {		# We suppose that the text to match
				# fits in a paragraph 
    my $txt = shift;
    my $m = shift;
    my ($in,@in);
    while ( (!defined $txt->{'paragraph'} || $ {$txt->{'paragraph'}} !~ /$m/)
	   && defined ($in = $txt->eatGroup(1))) {
      push(@in,@$in);
    }
    ($ {$txt->{'paragraph'}} =~ s/$m//) || warn "Delimiter `$m' not found";
    bless \@in, 'Text::TeX::Group';
  }

# return next token without eating it. Return '' if end of paragraph
  sub lookAheadToken {		# If arg2, will eat one token - WHY!? -Ak
    my $txt = shift;
    # Call paragraph with no argument to say we're "just looking"
    my $in = $txt->paragraph;
    return '' unless $in;	# To be able to match without warnings
    my $comment = undef;
    if ($$in =~ 
	/^(?:\s*)(?:$Text::TeX::commentpattern)?($Text::TeX::tokenpattern)/o) {
      if (defined $2) {return $1} #if 1 usualtokenclass char, return it ($1==$2)
      elsif (defined $3) {return "\\$3"} # Multiletter (\[a-zA-Z]+)
      elsif (defined $1) {return $1} # \" or notusualtokenclass
    }
    return '';
  }
  
# This is the main subroutine for eating a token.
# It returns a token as either TT::Text or TT::Token.
# Or it returns TT::Paragraph if it had to read a new paragraph in the TeX file.
  sub eatMultiToken {		# If arg2, will eat one token
    my $txt = shift;
    # call paragraph with an arg so it gets new paragraph if necessary
    my $in = $txt->paragraph(1);
    return undef unless defined $in;
    return new Text::TeX::Paragraph unless $in; #i.e., if it's a new paragraph
    my $comment = undef;
    # eat a comment that comes before the token we're about to read
    $comment = $2 if $$in =~ s/^(\s*)($Text::TeX::commentpattern)/$1/o;
    my $nomulti = shift; #if arg2, eat one token
    # Eat text or a token
    # Cannot use if () BLOCK, because $& is local.
    $got = $$in =~ s/^\s*($Text::TeX::tokenpattern)//o	if $nomulti;
    $got = $$in =~ s/^\s*($Text::TeX::multitokenpattern)//o	unless $nomulti;
    # $1 = \[^a-zA-Z] or special char like ~
    # $2 = regular text. Return $& to include leading space!
    # $3 = [a-zA-Z]+ which followed a backslash, i.e., a 'multiletter' command
    if ($got and defined $2) {new Text::TeX::Text $&, $comment, $&}
    elsif ($got and defined $3) {new Text::TeX::Token "\\$3", $comment, $&}
    elsif ($got and defined $1) {new Text::TeX::Token $1, $comment, $&}
    elsif ($comment) {new Text::TeX::Token undef, $comment, ""}
    else {undef}
  }

# This is the main subroutine for eating the file.
# It eats tokens and returns them. Sometimes it also returns pseudotokens.
# Basic rundown:
#  - if there's stuff in pending_out, return it
#  - otherwise get stuff from pending_in OR eat a new token
#  - refine the token, then digest it
# (- pop stuff from synthetic into pending_out for next time UNLESS
#      you read a new command that takes arguments. E.g. x^\sqrt)
#  - return the token unless it's special & has a 'type'
#  - based on the type, set up one or more tokens to be handled later
#    so that, e.g., type 'report_args' returns BegArgsToken, followed
#    later by some number of ArgToken's, followed by an EndArgsToken
#
#    LookAhead tokens can be used for _^. If you have x^a_b, the EndArgsToken
# for the ^ will be changed to a LookAhead, which notes that a _ is next.
# The _ has a BegArgsLookedAhead token instead of BegArgsToken. If anything
# other than _ or ^ follows the argument to the LookAhead token (for example,
# x^2+b, a regular old EndArgsToken is returned for the ^. reLyX doesn't use
# the LookAhead functionality. (phew!)
  sub eat {
    my $txt = shift;
    if ( @{ $txt->{pending_out} } ) {
      my $out = pop @{ $txt->{pending_out} };
      # E.g., if you have x^\sqrt2 -- when you pop and return the \sqrt
      # EndArgsToken, you need to make sure the ^ EndArgsToken falls out next.
      #    But if pending_out is an ArgToken, *don't* pop the next thing 
      # (next ArgToken or EndArgsToken) out of synthetic yet
      # Most often, synthetic will be empty, so popsynthetic will do nothing
      $txt->popsynthetic if ref($out) eq 'Text::TeX::EndArgsToken';
      if (ref $out eq 'Text::TeX::LookAhead') {
	my $in = $txt->lookAheadToken;
	if (defined ($res = $out->[0][2]{$in})) {
	  push @{$out->[0]}, $in, $res;
	  # actually eat what you looked ahead
	  $in = $txt->eatMultiToken(1);	# XXXX may be wrong if next
                                        # token needs to be eaten in
                                        # the style `multi', like \left.
	  # Put it at beginning of pending_in so we do E.g., EndLocals first
	  splice @{ $txt->{pending_in} }, 
	    0, 0, (bless \$in, 'Text::TeX::LookedAhead');
	  return $out;
	} else {
	  return bless $out, 'Text::TeX::EndArgsToken';
	}
      } else {
	return $out;
      }
    } # end if pending_out

    # We didn't get & return stuff from pending_out. So try to get stuff
    #    from pending_in. If there's nothing there, eat a new token.
    my $in = pop @{ $txt->{pending_in} };
    my $after_lookahead;
    if (defined $in) {
      # after_lookahead is true if we got a LookedAhead token from pending_out
      #    because we looked ahead when there was a LookAhead token
      $in = $$in, $after_lookahead = 1 
	if ref $in eq 'Text::TeX::LookedAhead';
    } else {
      my $one;
      # This will happen if we did pushsynthetic on the last token.
      # That happened for report_args tokens, i.e., things that require
      #     arguments. \frac, e.g., will read either a character or
      #     a token *or* the '{' that begins a group, then popsynthetic below.
      # \frac puts *two* tokens in {synthetic} so $one will be set TWICE
      $one = 1 if @{ $txt->{synthetic}[-1] }; # Need to eat a group.
      $in = $txt->eatMultiToken($one);
    }
    return undef unless defined $in;
    $in->refine($txt);
    $in->digest($txt);
    my ($Token, $type, @arr);
    unless (defined $in
            && defined $in->[0] 
	    && $in->[0] =~ /$Text::TeX::active/o
	    && defined ( $Token = $txt->{tokens}->{$in->[0]} )
	    && exists ($Token->{"Type"})
	    ) {
	$txt->popsynthetic;
	return $in;
    }
    $type = $Token->{Type};
    $txt->popsynthetic unless $type eq 'report_args';

    # If the token is special enough that it's got a 'type', do more stuff
    my $out = $in;
    if ($type eq 'action') {
#      return &{$Token->{sub}}($in);
      return &{$Token->{'sub'}}($in); #Without 's it breaks strict refs -Ak
    } elsif ($type eq 'argmask') {
      # eatWithMask;		# ????
    } elsif ($type eq 'args') {
      # Args eaten already
    } elsif ($type eq 'local') {
      $txt->addpresynthetic(new Text::TeX::EndLocal $in);
    } elsif ($type eq 'report_args') {
      my $count = $Token->{count};
      my $ordinal = $count;
      my $res;
      if ($res = $Token->{lookahead}) {
	$txt->pushsynthetic(new Text::TeX::LookAhead [$in, $count, $res]);
      } else {
	# This will fall out after we read all the args this token needs
	$txt->pushsynthetic(new Text::TeX::EndArgsToken [$in, $count]);	
      }
      # One of these tokens will fall out after we finish each arg (except last)
      # Push on 3,2,1, so that when we *popsynthetic*, 1 will come off first
      # followed by 2, 3
      # ArgToken->[0][2] will then be the number of args read so far for
      # the token held in ArgToken->[0][0]
      while (--$ordinal) {
	$txt->pushsynthetic(new Text::TeX::ArgToken [$in, $count, $ordinal]);
      }
      if ($after_lookahead) {
	$out = new Text::TeX::BegArgsTokenLookedAhead [$in, $count];
      } else {
	$out = new Text::TeX::BegArgsToken [$in, $count];
      }
    } else {
      warn "Format of token data unknown for `", $in->[0], "'"; 
    }
    return $out;
  }
  
  sub report_arg {
    my $n = shift;
    my $max = shift;
    my $act = shift;
    my $lastact = shift;
    if ($n == $max) {
      &$lastact($n);
    } else {
      &$act($n,$max);
    }
  }

  sub eatDefine {
    my $txt = shift;
    my ($args, $body);
    warn "No `{' found after defin", return undef 
      unless $args = $txt->eatUntil('{');
    warn "Argument list @$args too complicated", return undef 
      unless @$args == 1 && $$args[0] =~ /^(\ \#\d)*$/;
    warn "No `}' found after defin", return undef 
      unless $body = $txt->eatBalancedRest;
    #my @args=split(/(\#[\d\#])/,$$);       # lipa
  }
  
# This is the main subroutine called by parsing programs. Basically, it
#     keeps eating tokens, then calling $txt->actions on that token
  sub process {
    my ($txt, $eaten, $act) = (shift);
    while (defined ($eaten = $txt->eat)) {
      if (defined ($act = $txt->{actions}[-1])) {
	&$act($eaten,$txt);
      }
    }
  }
} #END Text::TeX::OpenFile

#####################    MORE GLOBAL STUFF    ##################################
%super_sub_lookahead = qw( ^ 1 _ 0 \\sb 0 \\sp 1 \\Sp 1 \\Sb 0 );

# class => 'where to bless to', Type => how to process
# eatargs => how many args to swallow before digesting

%Tokens = (
  '{' => {'class' => 'Text::TeX::Begin::Group', 'waitfor' => '}'},
  '}' => {'class' => 'Text::TeX::End::Group'},
  "\$" => {'class' => 'Text::TeX::SelfMatch', waitfor => "\$"},
  '$$' => {'class' => 'Text::TeX::SelfMatch', waitfor => '$$'},
  '\begin' => {class => 'Text::TeX::Begin::Group::Args', 
	       eatargs => 1, 'waitfor' => '\end', selfmatch => 1},
  '\end' => {class => 'Text::TeX::End::Group::Args', eatargs => 1, selfmatch => 1},
  '\left' => {class => 'Text::TeX::Begin::Group::Args', 
	       eatargs => 1, 'waitfor' => '\right'},
  '\right' => {class => 'Text::TeX::End::Group::Args', eatargs => 1},
  '\frac' => {Type => 'report_args', count => 2},
  '\sqrt' => {Type => 'report_args', count => 1},
  '\text' => {Type => 'report_args', count => 1},
  '\operatorname' => {Type => 'report_args', count => 1},
  '\operatornamewithlimits' => {Type => 'report_args', count => 1},
  '^' => {Type => 'report_args', count => 1, 
	  lookahead => \%super_sub_lookahead },
  '_' => {Type => 'report_args', count => 1, 
	  lookahead => \%super_sub_lookahead },
  '\em' => {Type => 'local'},
  '\bold' => {Type => 'local'},
  '\it' => {Type => 'local'},
  '\rm' => {Type => 'local'},
  '\mathcal' => {Type => 'local'},
  '\mathfrak' => {Type => 'local'},
  '\mathbb' => {Type => 'local'},
  '\\\\' => {'class' => 'Text::TeX::Separator'},
  '&' => {'class' => 'Text::TeX::Separator'},
);

##############   I NEVER USE ANYTHING BELOW THIS LINE!! -Ak   ##################
{
  my $i = 0;
  @symbol = (
       (undef) x 8,		# 1st row
       (undef) x 8,
       (undef) x 8,		# 2nd row
       (undef) x 8,
       undef, undef, '\forall', undef, '\exists', undef, undef, '\???', # 3rd: symbols
       (undef) x 8,
       (undef) x 8,     # 4th: numbers and symbols
       (undef) x 8,
       '\???', ( map {"\\$_"} 
		 qw(Alpha Beta Chi Delta Epsilon Phi Gamma 
		 Eta Iota vartheta Kappa Lambda Mu Nu Omicron 
		 Pi Theta Rho Sigma Tau Ypsilon varsigma Omega
		 Xi Psi Zeta)), undef, '\therefore', undef, '\perp', undef,
       undef, ( map {"\\$_"} 
	        qw(alpha beta chi delta varepsilon phi gamma
		   eta iota varphi kappa lambda mu nu omicron
		   pi theta rho sigma tau ypsilon varpi omega
		   xi psi zeta)), undef, undef, undef, undef, undef,
       (undef) x 8,		# 9st row
       (undef) x 8,
       (undef) x 8,		# 10nd row
       (undef) x 8,
       undef, undef, undef, '\leq', undef, '\infty', undef, undef, # 11th row
       undef, undef, undef, undef, '\from', undef, '\to', undef,
       '\circ', '\pm', undef, '\geq', '\times', undef, '\partial', '\bullet', # 12th row
       undef, '\neq', '\equiv', '\approx', '\dots', '\mid', '\hline', undef,
       '\Aleph', undef, undef, undef, '\otimes', '\oplus', '\empty', '\cap', # 13th row
       '\cup', undef, undef, undef, undef, undef, '\in', '\notin',
       undef, '\nabla', undef, undef, undef, '\prod', undef, '\cdot', # 14th row
       undef, '\wedge', '\vee', undef, undef, undef, undef, undef,
       undef, '\<', undef, undef, undef, '\sum', undef, undef, # 15th row
       (undef) x 8,
       undef, '\>', '\int', (undef) x 5, # 16th row
       (undef) x 8,
      );
  for (@symbol) {
    $xfont{$_} = ['symbol', chr($i)] if defined $_;
    $i++;
  }
}

# This list was autogenerated by the following script:
# Some handediting is required since MSSYMB.TEX is obsolete.

## Usage is like:
##		extract_texchar.pl  PLAIN.TEX MSSYMB.TEX
##$family = shift;

#%fonts = (2 => "cmsy", 3 => "cmex", '\\msx@' => msam, '\\msy@' => msbm, );

#while (defined ($_ = <ARGV>)) {
#  $list{$fonts{$2}}[hex $3] = $1
#    if /^\s*\\mathchardef(\\\w+)=\"\d([23]|\\ms[xy]\@)([\da-fA-F]+)\s+/o;
#}

#for $font (keys %list) {
#  print "\@$font = (\n  ";
#  for $i (0 .. $#{$list{$font}}/8) {
#    print join ', ', map {packit($_)} @{$list{$font}}[ 8*$i .. 8*$i+7 ];
#    print ",\n  ";
#  }
#  print ");\n\n";
#}

#sub packit {
#  my $cs = shift;
#  if (defined $cs) {
#    #$cs =~ s/\\\\/\\\\\\\\/g;
#    "'$cs'";
#  } else {
#    'undef';
#  }
#}

@cmsy = (
  undef, '\cdotp', '\times', '\ast', '\div', '\diamond', '\pm', '\mp',
  '\oplus', '\ominus', '\otimes', '\oslash', '\odot', '\bigcirc', '\circ', '\bullet',
  '\asymp', '\equiv', '\subseteq', '\supseteq', '\leq', '\geq', '\preceq', '\succeq',
  '\sim', '\approx', '\subset', '\supset', '\ll', '\gg', '\prec', '\succ',
  '\leftarrow', '\rightarrow', '\uparrow', '\downarrow', '\leftrightarrow', '\nearrow', '\searrow', '\simeq',
  '\Leftarrow', '\Rightarrow', '\Uparrow', '\Downarrow', '\Leftrightarrow', '\nwarrow', '\swarrow', '\propto',
  '\prime', '\infty', '\in', '\ni', '\bigtriangleup', '\bigtriangledown', '\not', '\mapstochar',
  '\forall', '\exists', '\neg', '\emptyset', '\Re', '\Im', '\top', '\perp',
  '\aleph', undef, undef, undef, undef, undef, undef, undef,
  undef, undef, undef, undef, undef, undef, undef, undef,
  undef, undef, undef, undef, undef, undef, undef, undef,
  undef, undef, undef, '\cup', '\cap', '\uplus', '\wedge', '\vee',
  '\vdash', '\dashv', undef, undef, undef, undef, undef, undef,
  '\langle', '\rangle', '\mid', '\parallel', undef, undef, '\setminus', '\wr',
  undef, '\amalg', '\nabla', '\smallint', '\sqcup', '\sqcap', '\sqsubseteq', '\sqsupseteq',
  undef, '\dagger', '\ddagger', undef, '\clubsuit', '\diamondsuit', '\heartsuit', '\spadesuit',
  );

@cmex = (
  undef, undef, undef, undef, undef, undef, undef, undef, # 0-7
  undef, undef, undef, undef, undef, undef, undef, undef, # 8-15
  undef, undef, undef, undef, undef, undef, undef, undef, # 16-23
  undef, undef, undef, undef, undef, undef, undef, undef, # 24-31
  undef, undef, undef, undef, undef, undef, undef, undef, # 32-39
  undef, undef, undef, undef, undef, undef, undef, undef, # 40-47
  undef, undef, undef, undef, undef, undef, undef, undef, # 48-55
  undef, undef, undef, undef, undef, undef, undef, undef, # 56-64
  undef, undef, undef, undef, undef, undef, '\bigsqcup', undef,	# 64-71
  '\ointop', undef, '\bigodot', undef, '\bigoplus', undef, '\bigotimes', undef,	# 72-79
  '\sum', '\prod', '\intop', '\bigcup', '\bigcap', '\biguplus', '\bigwedge', '\bigvee',	# 80-87
  undef, undef, undef, undef, undef, undef, undef, undef,
  '\coprod', undef, undef, undef, undef, undef, undef, undef,
  );

@msam = (
  '\boxdot', '\boxplus', '\boxtimes', '\square', '\blacksquare', '\centerdot', '\lozenge', '\blacklozenge',
  '\circlearrowright', '\circlearrowleft', '\rightleftharpoons', '\leftrightharpoons', '\boxminus', '\Vdash', '\Vvdash', '\vDash',
  '\twoheadrightarrow', '\twoheadleftarrow', '\leftleftarrows', '\rightrightarrows', '\upuparrows', '\downdownarrows', '\upharpoonright', '\downharpoonright',
  '\upharpoonleft', '\downharpoonleft', '\rightarrowtail', '\leftarrowtail', '\leftrightarrows', '\rightleftarrows', '\Lsh', '\Rsh',
  '\rightsquigarrow', '\leftrightsquigarrow', '\looparrowleft', '\looparrowright', '\circeq', '\succsim', '\gtrsim', '\gtrapprox',
  '\multimap', '\therefore', '\because', '\doteqdot', '\triangleq', '\precsim', '\lesssim', '\lessapprox',
  '\eqslantless', '\eqslantgtr', '\curlyeqprec', '\curlyeqsucc', '\preccurlyeq', '\leqq', '\leqslant', '\lessgtr',
  '\backprime', undef, '\risingdotseq', '\fallingdotseq', '\succcurlyeq', '\geqq', '\geqslant', '\gtrless',
  '\sqsubset', '\sqsupset', '\vartriangleright', '\vartriangleleft', '\trianglerighteq', '\trianglelefteq', '\bigstar', '\between',
  '\blacktriangledown', '\blacktriangleright', '\blacktriangleleft', undef, undef, '\vartriangle', '\blacktriangle', '\triangledown',
  '\eqcirc', '\lesseqgtr', '\gtreqless', '\lesseqqgtr', '\gtreqqless', '\yen', '\Rrightarrow', '\Lleftarrow',
  '\checkmark', '\veebar', '\barwedge', '\doublebarwedge', '\angle', '\measuredangle', '\sphericalangle', '\varpropto',
  '\smallsmile', '\smallfrown', '\Subset', '\Supset', '\Cup', '\Cap', '\curlywedge', '\curlyvee',
  '\leftthreetimes', '\rightthreetimes', '\subseteqq', '\supseteqq', '\bumpeq', '\Bumpeq', '\lll', '\ggg',
  '\ulcorner', '\urcorner', '\circledR', '\circledS', '\pitchfork', '\dotplus', '\backsim', '\backsimeq',
  '\llcorner', '\lrcorner', '\maltese', '\complement', '\intercal', '\circledcirc', '\circledast', '\circleddash',
  );

@msbm = (
  '\lvertneqq', '\gvertneqq', '\nleq', '\ngeq', '\nless', '\ngtr', '\nprec', '\nsucc',
  '\lneqq', '\gneqq', '\nleqslant', '\ngeqslant', '\lneq', '\gneq', '\npreceq', '\nsucceq',
  '\precnsim', '\succnsim', '\lnsim', '\gnsim', '\nleqq', '\ngeqq', '\precneqq', '\succneqq',
  '\precnapprox', '\succnapprox', '\lnapprox', '\gnapprox', '\nsim', '\ncong', undef, undef,
  '\varsubsetneq', '\varsupsetneq', '\nsubseteqq', '\nsupseteqq', '\subsetneqq', '\supsetneqq', '\varsubsetneqq', '\varsupsetneqq',
  '\subsetneq', '\supsetneq', '\nsubseteq', '\nsupseteq', '\nparallel', '\nmid', '\nshortmid', '\nshortparallel',
  '\nvdash', '\nVdash', '\nvDash', '\nVDash', '\ntrianglerighteq', '\ntrianglelefteq', '\ntriangleleft', '\ntriangleright',
  '\nleftarrow', '\nrightarrow', '\nLeftarrow', '\nRightarrow', '\nLeftrightarrow', '\nleftrightarrow', '\divideontimes', '\varnothing',
  '\nexists', undef, undef, undef, undef, undef, undef, undef,
  undef, undef, undef, undef, undef, undef, undef, undef,
  undef, undef, undef, undef, undef, undef, undef, undef,
  undef, undef, undef, undef, undef, undef, undef, undef,
  undef, undef, undef, undef, undef, undef, '\mho', '\eth',
  '\eqsim', '\beth', '\gimel', '\daleth', '\lessdot', '\gtrdot', '\ltimes', '\rtimes',
  '\shortmid', '\shortparallel', '\smallsetminus', '\thicksim', '\thickapprox', '\approxeq', '\succapprox', '\precapprox',
  '\curvearrowleft', '\curvearrowright', '\digamma', '\varkappa', undef, '\hslash', '\hbar', '\backepsilon',
  );

# Temporary workaround against Tk's \n (only cmsy contains often-used \otimes):

$cmsy[ord "\n"] = undef;

for $font (qw(cmsy cmex msam msbm)) {
  for $num (0 .. $#{$font}) {
    $xfont{$$font[$num]} = [$font, chr($num)] if defined $$font[$num];
  }
}

%aliases = qw(
	      \int \intop \oint \ointop \restriction \upharpoonright
	      \Doteq \doteqdot \doublecup \Cup \doublecap \Cap
	      \llless \lll \gggtr \ggg \lnot \neg \land \wedge
	      \lor \vee \le \leq \ge \geq \owns \ni \gets \leftarrow
	      \to \rightarrow \< \langle \> \rangle \| \parallel
	     );

for $from (keys %aliases) {
  $xfont{$from} = $xfont{$aliases{$from}} if exists $xfont{$aliases{$from}};
}


# Autoload methods go after =cut, and are processed by the autosplit program.

1;
__END__

=head1 NAME

Text::TeX -- Perl module for parsing of C<TeX>.

=head1 SYNOPSIS

  use Text::TeX;

  sub report {
    my($eaten,$txt) = (shift,shift);
    print "Comment: `", $eaten->[1], "'\n" if defined $eaten->[1];
    print "@{$txt->{waitfors}} ", ref $eaten, ": `", $eaten->[0], "'";
    if (defined $eaten->[3]) {
      my @arr = @{ $eaten->[3] };
      foreach (@arr) {
	print " ", $_->print;
      }
    }
    print "\n";
  }

  my $file = new Text::TeX::OpenFile 'test.tex',
    'defaultact' => \&report;
  $file->process;

=head1 DESCRIPTION

A new C<TeX> parser is created by

  $file = new Text::TeX::OpenFile $filename, attr1 => $val1, ...;

$filename may be C<undef>, in this case the text to parse may be
specified in the attribute C<string>.

Recognized attributes are:

=over 12

=item C<string>

contains the text to parse before parsing $filename.

=item C<defaultact>

denotes a procedure to submit C<output tokens> to.

=item C<tokens>

gives a hash of C<descriptors> for C<input token>. A sane default is
provided.

=back

A call to the method C<process> launches the parser.

=head2 Tokenizer

When the parser is running, it processes input stream by splitting it
into C<input tokens> using some I<heuristics> similar to the actual
rules of TeX tokenizer. However, since it does not use I<the exact
rules>, the resulting tokens may be wrong if some advanced TeX command
are used, say, the character classes are changed.

This should not be of any concern if the stream in question is a
"user" file, but is important for "packages".

=head2 Digester

The processed C<input tokens> are handled to the digester, which
handles them according to the provided C<tokens> attribute.

=head2 C<tokens> attribute

This is a hash reference which describes how the C<input tokens>
should be handled. A key to this hash is a literal like C<^> or
C<\fraction>. A value should be another hash reference, with the
following keys recognized:

=over 7

=item class

Into which class to bless the token. Several predefined classes are
provided. The default is C<Text::TeX::Token>.

=item Type

What kind of special processing to do with the input after the
C<class> methods are called. Recognized C<Type>s are:

=over 10

=item report_args

When the token of this C<Type> is encountered, it is converted into
C<Text::Tex::BegArgsToken>. Then the arguments are processed as usual,
and an C<output token> of type C<Text::Tex::ArgToken> is inserted
between them. Finally, after all the arguments are processed, an
C<output token> C<Text::Tex::EndArgsToken> is inserted.

The first element of these simulated C<output tokens> is an array
reference with the first element being the initial C<output token>
which generated this sequence. The second element of the internal
array is the number of arguments required by the C<input token>. The
C<Text::Tex::ArgToken> token has a third element, which is the ordinal
of the argument which ends immediately before this token.

If requested, a token C<Text::Tex::LookAhead> may be returned instead
of C<Text::Tex::EndArgsToken>. The additional elements of
C<$token->[0]> are: the reference to the corresponding C<lookahead>
attribute, the relevant key (text of following token) and the
corresponding value.

In such a case the input token which was looked-ahead would generate
an output token of type C<Text::Tex::BegArgsTokenLookedAhead> (if it
usually generates C<Text::Tex::BegArgsToken>).

=item local

Means that these macro introduces a local change, which should be
undone at the end of enclosing block. At the end of the block an
output event C<Text::TeX::EndLocal> is delivered, with C<$token->[0]>
being the output token for the I<local> event starting.

Useful for font switching. 

=back

=back

Some additional keys may be recognized by the code for the particular
C<class>.

=over 12

=item C<count>

number of arguments to the macro.

=item C<waitfor>

gives the matching token for a I<starting delimiter> token.

=item C<eatargs>

number of tokens to swallow literally and put into the relevant slot
of the C<output token>. The surrounding braces are stripped.

=item C<selfmatch>

is used with C<eatargs==1>. Denotes that the matching token is also
C<eatargs==1>, and the swallowed tokens should coinside (like with
C<\begin{blah} ... \end{blah}>).

=item C<lookahead>

is a hash with keys being texts of tokens which need to be treated
specially after the end of arguments for the current token. If the
corresponding text follows the token indeed, a token
C<Text::Tex::LookAhead> is returned instead of
C<Text::Tex::EndArgsToken>.

=back

=head2 Symbol font table

The hash %Text::TeX::xfont contains the translation table from TeX
tokens into the corresponding font elements. The values are array
references of the form C<[fontname, char]>, Currently the only font
supported is C<symbol>.

=cut

=head1 AUTHOR

Ilya Zakharevich, ilya@math.ohio-state.edu

=head1 SEE ALSO

perl(1).

=cut
