# This file is part of reLyX
# Copyright (c) 1998-9 Amir Karger karger@post.harvard.edu
# You are free to use and modify this code under the terms of
# the GNU General Public Licence version 2 or later.

package RelyxFigure;

# This is a package to read LaTeX figures and print out LyX figures


# We declare here the sub-packages found in this package.
# This allows the parser to understand "indirect object" form of subroutines
#{
#package RelyxTable::Table;
#package RelyxTable::Column;
#package RelyxTable::Row;
#}

use strict;

# Variables used by other packages
use vars qw($EpsfYsize $EpsfXsize %HW_types);

# Debugging on?
my $debug_on;

# This means "display in monochrome" and "do translations", but who cares?
# It's just here because this is the default that LyX outputs.
my $Default_Flags = 9;

# Names for width_type & height_type fields.
%HW_types = (
    "def"     => 0,
    "cm"     => 1,
    "in"     => 2,
    "per_page" => 3,  # percent of the page
    "per_col"  => 4,  # percent of a column (illegal for height_type)
);

# Parse \epsfxsize or \epsfysize command
# Return 0 if we can't convert the length (in which case we have to type
# the \epsf[xy]size command in tex mode).
sub parse_epsfsize {
    # Command & length have already been "stringified" (they're not tokens)
    my ($command, $length) = (shift,shift);
    if ($command eq '\\epsfxsize') {
        $EpsfXsize = $length;
	my @dummy = &convert_length($EpsfXsize) || return 0;
    } elsif ($command eq '\\epsfysize') {
        $EpsfYsize = $length;
	my @dummy = &convert_length($EpsfYsize) || return 0;
    }

    return 1;
} # end sub RelyxFIgure::Figure::parse_epsfig

sub convert_length {
    # test if it's a valid LyX width/height.
    # (But assume a person won't try to set width to \textheight,
    # and that they won't have negative or otherwise weird lengths)
    # Then convert to width & width_type (or height/height_type)
    # Return empty list on error
    my $size = shift;

    # A length can be (optional plus followed by) (num)(unit) where 
    # num is a float number (possibly with European command) and unit
    # is a size unit, either in,cm,pt etc. or \textwidth etc.
    my %unit_convert = (  # 1 inch = 25.4 mm
        "mm" => 25.4, "pt" => 72.27, "bp" => 72, "pc" => 72.27/12,
	"dd" => 72.27*1157/1238, "cc" => 72.27*1157/(1238*12),
    );
    my ($number, $type);
    if ($size =~ /^\+?([\d.,]+)(cm|in)$/) {
        ($number, $type) = ($1, $2);
	$number =~ s/,/./; # Allow european numbers!
	# print "length is $number '$type'";
    } elsif ($size =~ /^\+?([\d.,]+)(mm|pt|bp|pc|dd|cc)$/) {
        ($number, $type) = ($1, $2);
	$number =~ s/,/./;
	$number = $number / $unit_convert{$type};
	$type = "in";
    } elsif ($size =~ /^\+?([\d.,]*)\s*\\text(height|width)$/) {
        $number = $1 || 1;
	$number =~ s/,/./;
	$number *= 100;
	$type = "per_page";
    } elsif ($size =~ /^\+?([\d.,]*)\s*\\columnwidth$/) {
        $number = $1 || 1;
	$number =~ s/,/./;
	$number *= 100;
	$type = "per_col";
    } else {
	print "\ncannot translate length '$size' in Figure; ",
	      "copying in TeX mode\n" if $debug_on;
    }

    if ($number) {
        return ($number, $type);
    } else {
	return ();
    }
} # end sub convert_length

sub parse_keyval {
# Parse a string containing comma-separated "key=value" pairs
# Compare the keys with a list of known keys.
# If we know all keys, return a hash containing keys/values
# Else return undef.
    my ($string, @known_keys) = @_;
    my @fields = split(/\s*,\s*/,$string);
    my %fighash;
    foreach (@fields) { # split "key=val" into fighash{key}=val
	my ($key,$val) = split(/\s*=\s*/,$_);
	$val = "" unless defined $val; # e.g., 'clip='
	$fighash{$key} = $val;

	unless (grep /^$key$/, @known_keys)  {
	    print "\nUntranslatable key '$key' to figure;",
		  " copying in TeX mode\n" if $debug_on;
	    return undef;
	}
    }

    return \%fighash;
} # end sub parse_keyval



{
    package RelyxFigure::Figure;
    # reLyX figure class
    # Fields:
    #    file        - file name
    #    width       - width
    #    height      - height
    #    width_type  - is width in cm, % of pagewidth, etc.?
    #    height_type - is height in cm, % of pagewidth, etc.?
    #    angle       - rotate fig through angle
    #    flags       - various flags for LyX display. Not important

    sub new {
        my $class = shift;
	my $thisfig;
	$thisfig->{'file'} = "";
	$thisfig->{'width'} = 0;
	$thisfig->{'height'} = 0;
	$thisfig->{'width_type'} = "def";
	$thisfig->{'height_type'} = "def";
	$thisfig->{'angle'} = 0;
	$thisfig->{'flags'} = $Default_Flags;
        # This seems like a convenient place to declare this...
        $debug_on= (defined($main::opt_d) && $main::opt_d);

	bless $thisfig, $class;
    } # end sub RelyxFigure::Figure::new


    sub parse_pscommand {
    # this sub is given the various arguments to a command & adds that
    # information to the figure object.
    # Return 0 if it can't read the command, or if LyX can't handle it. Else 1.
    #
    # command is the name of the postscript command
    # optargs are optional arguments (TT:Tokens). For many of the commands,
    #    only one optarg is allowed. And of course, they may be empty tokens.
    # reqarg is usually the filename (for (e)psfig, it's more)
    #
    # Currently, LyX can't handle bounding box, so we always return 0 if one
    # is given.
        my ($self, $command, $optarg1, $optarg2, $reqarg) = @_;
	my (@known_keys, $filename, $keyval_string, %fighash);

	for ($command) {
            if (/^\\epsf(file|box)?$/) {
		# syntax: \epsffile[bounding box]{filename.eps}
		# epsffile is an older version of epsfbox
		return 0 if $optarg1->print; # bounding box was given. Panic!
		$filename = &recursive_print($reqarg);
		# fighash key shouldn't exist if no size was given
		$fighash{'height'} = $RelyxFigure::EpsfYsize 
		    if $RelyxFigure::EpsfYsize;
		$fighash{'width'} = $RelyxFigure::EpsfXsize
		    if $RelyxFigure::EpsfXsize;
		# Once you use \epsf[xy]size, they get reset
		$RelyxFigure::EpsfXsize = $RelyxFigure::EpsfYsize = "";
		$keyval_string = ""; # no key/value pairs for this command

	    } elsif (/^\\e?psfig$/) {
		# "silent" key doesn't do anything
		@known_keys = qw(file figure rotate angle height width silent);
		$keyval_string = &recursive_print($reqarg);
		my $fighashref = 
		        &RelyxFigure::parse_keyval($keyval_string, @known_keys);
		return 0 unless defined $fighashref; # found unknown key...
		%fighash = %$fighashref;

		$filename = $fighash{'file'} || $fighash{'figure'} || warn
		    "no filename found in figure argument '$keyval_string'";

	    } elsif (/^\\includegraphics$/) {
		# 0 optargs can be either graphics or graphicx. Doesn't
		# matter, matter, it's just the filename.
		#    1 optarg can either be graphicx (if arg contains '=') or
		# graphics (optarg is upper right; lower left is 0,0).
		#    2 optargs is graphics with bounding box.

		# Optional arguments are always returned as single tokens,
		# not groups. So we can use the print method instead of
		# recursive_print.
		$keyval_string = $optarg1->print;
		if ($keyval_string) {
		    if ($keyval_string =~ /=/) { # graphicx form
			$keyval_string =~ s/\[(.*)\]/$1/; # remove '[' and ']'
			@known_keys = qw(rotate angle height width);
			my $fighashref = &RelyxFigure::parse_keyval(
			                $keyval_string, @known_keys);
			return 0 unless defined $fighashref; # found unknown key
			%fighash = %$fighashref;

		    } else { # graphics form with bounding box
			print "\nLyX cannot support bounding box; ",
			      "copying Figure in TeX mode\n" if $debug_on;
			return 0;
		    }
		}

		$filename = &recursive_print($reqarg);

	    }
	} # end switch on command name

	# Now set fields in the Figure object
	$self->{'file'} = $filename;
	$self->{'angle'} = $fighash{'rotate'} if exists $fighash{'rotate'};
	$self->{'angle'} = $fighash{'angle'} if exists $fighash{'angle'};
	if (exists $fighash{'height'}) {
	    my @heights = &RelyxFigure::convert_length($fighash{'height'});
	    return 0 unless @heights; # unconvertable length!
	    ($self->{'height'},$self->{'height_type'}) = @heights;
	}
	if (exists $fighash{'width'}) {
	    my @widths = &RelyxFigure::convert_length($fighash{'width'});
	    return 0 unless @widths; # unconvertable length!
	    ($self->{'width'},$self->{'width_type'}) = @widths;
	}

	return 1; # if we got here, we parsed correctly and LyX can handle it.
    } # end sub RelyxFigure::Figure::parse_pscommand

    sub recursive_print {
	# if arg is a group, print its contents (i.e., ignore '{' and '}')
	# otherwise, print arg
	my $tok = shift;
	my $filename = "";
	my $type = ref($tok);
	$type =~ s/Text::TeX::// or warn "weird group";
	if ($type eq 'Group') {
	    foreach ($tok->contents) {$filename .= &recursive_print($_)}
	} else {
	    $filename .= $tok->exact_print
	}
	return $filename;
    }

    sub print_info {
        # LyX figure command -- return what to print; don't actually print it
        my $self = shift;

	my $to_print = "\n\\begin_inset Figure\n";
	# (LyX fig. command has eps size here. But we don't know that, so
	# we dont print it out.)

	$to_print .= "file $self->{'file'}\n";
	my ($size, $type) = ("","");
	($size, $type) = ($self->{'width'}, 
	                  $RelyxFigure::HW_types{$self->{'width_type'}});
	$to_print .= "width $type $size\n" if $size;
	($size, $type) = ("","");
	($size, $type) = ($self->{'height'}, 
	                  $RelyxFigure::HW_types{$self->{'height_type'}});
	$to_print .= "height $type $size\n" if $size;
	$to_print .= "angle $self->{'angle'}\n" if $self->{'angle'};
	$to_print .= "flags $self->{'flags'}\n";

	$to_print .= "\n\\end_inset \n\n";
        
    } # end sub RelyxFigure::Figure::print

} # end of package RelyxFigure::Figure

1; # return true to calling package
