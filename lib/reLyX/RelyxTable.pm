# This file is part of reLyX
# Copyright (c) 1998-9 Amir Karger karger@post.harvard.edu
# You are free to use and modify this code under the terms of
# the GNU General Public Licence version 2 or later.

package RelyxTable;

# This is a package to read LaTeX tables and print out LyX tables


# We declare here the sub-packages found in this package.
# This allows the parser to understand "indirect object" form of subroutines
{
package RelyxTable::Table;
package RelyxTable::Column;
package RelyxTable::Row;
}

use strict;

# Variables used by other packages
use vars qw(@table_array $TableBeginString $TableEndString);
# @table_array is the list of all arrays
# $TableBeginString is a string to write during one pass so that a later
#     pass knows to put the table info there
# $TableEndString is written at the end of the table so that we know
#     the table is done
$TableBeginString = '%%%%%Insert reLyX table here!';
$TableEndString =   '%%%%%End of reLyX table!';

# Debugging on?
my $debug_on;

# Are we currently inside a table?
# If we are, return the table
sub in_table {
    return "" unless defined(@table_array); # no tables exist
    my $thistable = $table_array[-1];
    if ($thistable->{"active"}) {
        return (bless $thistable, "RelyxTable::Table");
    } else {
        return "";
    }
}


# Global variables###############
# LyX' enums corresponding to table alignments
my %TableAlignments = ("l" => 2, "r" => 4, "c" => 8);
# LyX' enums corresponding to multicol types
#    normal (non-multicol) cell, beginning of a multicol, part of a multicol
my %MulticolumnTypes = ("normal" => 0, "begin" => 1, "part" => 2);

# Subroutines used by tables and rows, e.g.
sub parse_cols {
# parse a table's columns' description
# Returns an array where each element is one column description
# arg0 is the description -- a Text::TeX::Group
    my $groupref = shift;
    my (@cols, @new_cols);
    my ($tok, $description, $i);

    # tokens in the group, not including '{' and '}'
    my @group = $groupref->contents;

    # Loop over the token(s) in the group
    my $first = ""; my $tempfirst;
    while (@group) {

	$tok = shift(@group);
	# Each $tok will consist of /^[clr|]*[p*@]?$/
	# (Except first may have | and/or @ expressions before it)
	# p*@ will end the $tok since after it comes a group in braces
	# @ will be a TT::Token, everything else will be in TT::Text
	$description = $tok->print;

	# Chop off left lines for first column if any
	($tempfirst = $description) =~ s/(\|*).*/$1/;
	if ($#cols == -1) { # |'s before any column description
	    $first .= $tempfirst;
	} else {
	    $cols[-1] .= $tempfirst; # add it to end of current col
	}

	# Greedy searches, so only 0th column can possibly have left line
	@new_cols = ($description =~ /[clr]\|*/g);
	push @cols, @new_cols;

	# parse a p or * or @ if necessary
	# use exact_print in case there's weird stuff in the @ descriptions
	$description = substr($description,-1);
	if ($description eq 'p') {
	    $tok = shift(@group);
	    my $pdes = $description . $tok->exact_print; # "p{foo}"
	    push @cols, $pdes;

	} elsif ($description eq '@') {
	    $tok = shift(@group);
	    my $atdes = $description . $tok->exact_print;
	    if ($#cols == -1) { # it's an @ before any column description
	        $first .= $atdes;
	    } else {
		$cols[-1] .= $atdes; # add it to end of current col
	    }

	} elsif ($description eq '*') {

	    $tok = shift(@group); # TT::Group with number of repeats in it
	    my $rep = $tok->contents->print;
	    $tok = shift(@group); # Group to repeat $rep times
	    @new_cols = &parse_cols($tok);
	    foreach $i (1 .. $rep) {
		push @cols, @new_cols;
	    }
	}
    } # end loop over description tokens

    # this handles description like {|*{3}{c}}
    $cols[0] = $first . $cols[0];

    return @cols;
} # end sub parse_cols

################################################################################
# This package handles tables for reLyX

{
    package RelyxTable::Table;
    # Table class
    # Fields:
    #    columns - array containing references to RelyxTable::Columns
    #    rows    - array containing references to RelyxTable::Rows
    #    active  - are we currently reading this table?
    # Fields for printout
    #	 is_long_table
    #	 rotate
    #	 endhead
    #	 end_first_head
    #	 endfoot
    #	 end_last_foot


# Subroutines to read and create the table
    sub new {
    # 'new' takes an argument containing the LaTeX table description string,
    #    which is a Text::TeX::Group token

	my $class = shift; # should be "table"
	my $description = shift;
	my $thistable;
	# This seems like a convenient place to declare this...
	$debug_on= (defined($main::opt_d) && $main::opt_d);

	# Initialize fields - including ones we don't support yet
	$thistable->{"is_long_table"} = 0;
	$thistable->{"rotate"} = 0;
	$thistable->{"endhead"} = 0;
	$thistable->{"end_first_head"} = 0;
	$thistable->{"endfoot"} = 0;
	$thistable->{"end_last_foot"} = 0;
	$thistable->{"active"} = 1;

	bless $thistable, $class;

	# Parse the column descriptions: return an array, where each
	#    element is a (regular text) single column description
	my @cols = &RelyxTable::parse_cols($description);
	my $colref;
	my $col_description;
	foreach $col_description (@cols) {
	    $colref = new RelyxTable::Column $col_description;
	    push @{$thistable->{"columns"}}, $colref;
	}
	# put the table into the table array
	push @RelyxTable::table_array, $thistable;


	# Now that it's blessed, put the 0th row into the table 
	$thistable->addrow;

	return $thistable;
    } # end sub new

    sub addrow {
    # add a row to the table
    # Since we're starting the row, we're in the 0th column
	my $thistable = shift;
	my $row = new RelyxTable::Row;
	push (@{$thistable->{"rows"}}, $row);

	# Also initialize the cells for this row
	my $col;
	foreach $col (@{$thistable->{"columns"}}) {
	    push (@{$row->{"cells"}}, RelyxTable::Cell->new($row, $col));
	}
    } # end sub addrow

    sub nextcol {
    # Go to next column - this just involves calling RT::Row->nextcol
    #    on the current row
	my $thistable = shift;
	my $row = $thistable->current_row;
	$row->nextcol;
    } # end of sub nextcol

    sub hcline {
    # interpret an '\hline' or '\cline' command
    # (It's cline if there's an arg1)
    # hline:
    # Add a bottom line to the row *before* the current row, unless it's
    #    the top row. In that case, add a top line to the current (top) row
    # Change the row and all the cells that make up the row
    # cline:
    # Change the cells from the row in the range given in arg1
	my $thistable = shift;
	my $range = shift;
	my $is_cline = defined($range);
	my ($rownum, $line_str, $lastrow, $cell);

	if ($lastrow = $thistable->numrows - 1) { # not top row
	    $rownum = $lastrow - 1;
	    $line_str = "bottom_line";
	} else {
	    $rownum = $lastrow;
	    $line_str = "top_line";
	}

	my $row = $thistable->{"rows"}[$rownum];
	# Add a row line (only) if it's a \hline command
	unless ($is_cline) {
	    $row->{"$line_str"} +=1;
	    if (defined($main::opt_d) && $row->{"$line_str"} == 2) {
	        print "\nToo many \\hline's";
	    }
	}

	# Figure out which rows to change
	my ($r1, $r2);
	if ($is_cline) {
	    $range =~ /(\d+)-(\d+)/ or warn "weird \\cline range";
	    # LaTeX numbers columns from 1, we number from 0
	    ($r1, $r2) = ($1 - 1, $2 - 1);
	} else {
	    $r1 = 0;
	    $r2 = $thistable->numcols - 1;
	}

	my $i;
	foreach $i ($r1 .. $r2) {
	    $cell = $row->{"cells"}[$i];
	    $cell->{"$line_str"} +=1; # change the cells in the row
	}
    } # end sub hline

    sub multicolumn {
    # interpret a \multicolumn command
    # This really just needs to call RT::Row->multicolumn for the correct row
	my $thistable = shift;
        my $row = $thistable->current_row;
	$row->multicolumn(@_);
    } # end sub multicolumn

    sub done_reading {
    # Finished reading a table
        my $thistable = shift;
	# If we just had \hlines at the end, it's not a real row
	# But if numcols==1, curr_col *has* to be zero!
	# HACK HACK HACK. If numcols==1 but we need to subtract a row, we
	# won't know until LastLyX. At that point, we'll subtract a row.
        my $row = $thistable->current_row;
	if ($thistable->numcols > 1 && $row->{"curr_col"} == 0) {
	    pop @{$thistable->{"rows"}}
	}

	# We're no longer reading this table
	$thistable->{"active"} = 0;

	if ($debug_on) {
	    print "\nDone with table ",$#RelyxTable::table_array,", which has ",
		$thistable->numrows," rows and ",
		$thistable->numcols," columns";
	    print"\nNumber of rows may be 1 too high" if $thistable->numcols==1;
	}
    } # end sub done_reading

# Subroutines to print out the table once it's created
    sub print_info {
        # print the header information for this table
	my $thistable = shift;
        my $to_print = "";
	$to_print .= "\n\\LyXTable\nmulticol5\n";
	my @arr = ($thistable->numrows,
		    $thistable->numcols,
		    $thistable->{"is_long_table"},
		    $thistable->{"rotate"},
		    $thistable->{"endhead"},
		    $thistable->{"end_first_head"},
		    $thistable->{"endfoot"},
		    $thistable->{"end_last_foot"}
		  );
	$to_print .= join(" ",@arr);
	$to_print .= "\n";

	# Print row info
	my $row;
	foreach $row (@{$thistable->{"rows"}}) {
	    $to_print .= $row->print_info;
	}

	# Print column info
	my $col;
	foreach $col (@{$thistable->{"columns"}}) {
	    $to_print .= $col->print_info;
	}
		   
	# Print cell info
	my $cell;
	foreach $row (@{$thistable->{"rows"}}) {
	    my $count = 0;
	    foreach $col (@{$thistable->{"columns"}}) {
	        $cell = $row->{"cells"}[$count];
		$count++;
	        $to_print .= $cell->print_info;
	    }
	}

	$to_print .= "\n";

	return $to_print;
    } # end sub print_info

# Convenient subroutines
    sub numrows {
	my $thistable = shift;
        return $#{$thistable->{"rows"}} + 1;
    } # end sub numrows

    sub numcols {
	my $thistable = shift;
        return $#{$thistable->{"columns"}} + 1;
    } # end sub numrows

    sub current_row {
    # Return the current row blessed as an RT::Row
        my $thistable = shift;
	my $row = $thistable->{"rows"}[-1];
	bless $row, "RelyxTable::Row"; #... and return it
    } # end sub current_row

} # end package RelyxTable::Table

################################################################################

{
# Column class
package RelyxTable::Column;

# Fields:
#    alignment - left, right, or center (l, r, or c)
#    right_line- How many lines this column has to its right
#    left_line - How many lines this column has to its left
#                (only first column can have left lines!)
#    pwidth    - width argument to a 'p' alignment command -- e.g., 10cm
#    special   - special column description that lyx can't handle

    sub new {
	my $class = shift;
	my $description = shift;
	my $col;

	# Initially zero everything, since we set different 
	# fields for @ and non-@ columns
	$col->{"alignment"} = "c";  # default
	$col->{"left_line"} = 0;
	$col->{"right_line"} = 0;
	$col->{"pwidth"} = "";
	$col->{"special"} = "";

	# Any special (@) column should be handled differently
	if ($description =~ /\@/) {
	   # Just put the whole description in "special" field --- this
	   # corresponds the the "extra" field in LyX table popup
	   # Note that LyX ignores alignment, r/l lines for a special column
	   $col->{"special"} = $description;
	   print "\n'$description' column won't display WYSIWYG in LyX\n"
	                                                    if $debug_on;

	# It's not a special @ column
	} else {

	    # left line?
	    $description =~ s/^\|*//;
	    $col->{"left_line"} = length($&);

	    # main column description
	    $description =~ s/^[clrp]//;
	    if ($& eq "p") {
		$description =~ s/^\{(.+)\}//; # eat the width
		$col->{"pwidth"} = $1; # width without braces
		# note: alignment is not applicable for 'p' columns
	    } else {
		$col->{"alignment"} = $&;
	    }

	    # right line?
	    $description =~ s/^\|*//;
	    $col->{"right_line"} = length($&);
	}

	bless $col, $class; #... and return it
    } # end sub new

    sub print_info {
    # print out header information for this column
    # Note that we need to put "" around pwidth and special for multicol5 format
	my $col = shift;
        my $to_print = "";
	my @arr = ($TableAlignments{$col->{"alignment"}},
		      $col->{"left_line"},
		      $col->{"right_line"},
		      '"' . $col->{"pwidth"} . '"',
		      '"' . $col->{"special"} . '"'
		    );
	$to_print .= join(" ",@arr);
	$to_print .= "\n";
		   
	return $to_print;
    }
} # end package RelyxTable::Column

################################################################################

{
package RelyxTable::Row;
# Fields:
#    top_line    - does this row have a top line?
#    bottom_line - does this row have a bottom line?
#    curr_col    - which column we're currently dealing with
#    cells       - array containing references to this row's cells

    sub new {
	my $class = shift;
	my $row;
	$row->{"top_line"} = 0;
	$row->{"bottom_line"} = 0;
	$row->{"is_cont_row"} = 0;
	$row->{"newpage"} = 0;
	$row->{"curr_col"} = 0;

	bless $row, $class;
    } # end sub new

    sub nextcol {
    # Go to next column on the current row
	my $row = shift;
	my $i = $row->{"curr_col"};
	$i++;

	# What if it was a multicolumn?
	$i++ while ${$row->{"cells"}}[$i]->{"multicolumn"} eq "part";

	$row->{"curr_col"} = $i;
    } # end of sub nextcol

    sub multicolumn {
    # interpret a \multicolumn command
    # Arg0 is the row that the multicolumn is in
    # Arg 1 is the first argument to \multicolumn, simply a number (no braces)
    # Arg 2 is the second argument, which is a TT::Group column specification
        my $row = shift;
	my ($num_cols, $coldes) = (shift, shift);

	# parse_cols warns about @{} expressions, which aren't WYSIWYG
	# and turns the description into a simple string
	my @dum = &RelyxTable::parse_cols($coldes);
	# LaTeX multicolumn description can only describe one column...
	warn "Strange multicolumn description $coldes" if $#dum;
	my $description = $dum[0];

	# Set the first cell
	my $firstcell = $row->{"curr_col"};
	my $cell = $row->{"cells"}[$firstcell];
	$cell->{"multicolumn"} = "begin";
	# Simple descriptions use alignment field, others use special
	#    Special isn't WYSIWYG in LyX -- currently, LyX can't display
	#    '|' or @{} stuff in multicolumns
	if ($description =~ /^[clr]$/) {
	    $cell->{"alignment"} = $description;
	} else {
	    $cell->{"special"} = $description;
	    print "\n'$description' multicolumn won't display WYSIWYG in LyX\n"
	                                                 if $debug_on;
	}

	# Set other cells
	my $i;
	foreach $i (1 .. $num_cols-1) {
	    $cell = $row->{"cells"}[$firstcell + $i];
	    $cell->{"multicolumn"} = "part";
	}

    } # end sub multicolumn

    sub print_info {
    # print information for this column
	my $row = shift;
        my $to_print = "";
	my @arr = ($row->{"top_line"},
		        $row->{"bottom_line"},
			$row->{"is_cont_row"},
			$row->{"newpage"}
		    );
	$to_print .= join(" ",@arr);
	$to_print .= "\n";
		   
	return $to_print;
    } # end sub print_info

} # end package RelyxTable::Row

################################################################################

{
package RelyxTable::Cell;
# Fields:
#    multicolumn - 0 (regular cell), 1 (beg. of multicol), 2 (part of multicol)
#    alignment   - alignment of this cell
#    top_line    - does the cell have a line on the top?
#    bottom_line - does the cell have a line on the bottom?
#    has_cont_row- 
#    rotate      - rotate cell?
#    line_breaks - cell has line breaks in it (???)
#    special     - does this multicol have a special description (@ commands?)
#    pwidth      - pwidth of this cell for a parbox command (for linebreaks)

    sub new {
    # args 1 and 2 are the parent row and column of this cell
	my $class = shift;
	my ($parent_row, $parent_col) = (shift, shift);
	my $cell;
	$cell->{"multicolumn"} = "normal"; # by default, it isn't a multicol
	$cell->{"alignment"} = "l"; # doesn't really matter: will be reset soon
	$cell->{"top_line"} = 0;
	$cell->{"bottom_line"} = 0;
	$cell->{"has_cont_row"} = 0;
	$cell->{"rotate"} = 0;
	$cell->{"line_breaks"} = 0;
	$cell->{"special"} = "";
	$cell->{"pwidth"} = "";

	# Have to bless $cell here, so that we can call methods on it
        bless $cell, $class;

	# The cell should inherit characteristics from its parent row & col
	$cell->row_inherit($parent_row);
	$cell->col_inherit($parent_col);

	return $cell;
    } # end sub new

    sub row_inherit {
    # Inherit fields from parent row
        my ($cell, $row) = (shift, shift);
	$cell->{"top_line"} = $row->{"top_line"};
	$cell->{"bottom_line"} = $row->{"bottom_line"};
    } # end sub row_inherit

    sub col_inherit {
    # Inherit field(s) from parent column
        my ($cell, $col) = (shift, shift);
	$cell->{"alignment"} = $col->{"alignment"};
    }

    sub print_info {
    # print information for this cell
    # Note that we need to put "" around pwidth and special for multicol5 format
	my $cell = shift;
        my $to_print = "";
	my @arr = ($MulticolumnTypes{$cell->{"multicolumn"}},
	                $TableAlignments{$cell->{"alignment"}},
	                $cell->{"top_line"},
		        $cell->{"bottom_line"},
			$cell->{"has_cont_row"},
			$cell->{"rotate"},
			$cell->{"line_breaks"},
		      '"' . $cell->{"special"} . '"',
		      '"' . $cell->{"pwidth"} . '"',
		    );
	$to_print .= join(" ",@arr);
	$to_print .= "\n";
		   
	return $to_print;
    }
} # end package RelyxTable::Cell

1; # return "true" to calling routine
