#! /usr/bin/perl -w

# this is a corny script to check that qtarch hasn't fscked up
# the initialisation order of the widgets. perl hackers are free
# to improve this

# usage: dlg/checkinitorder.pl *data.C

$found{"this"} = 1;
$found{"FALSE"} = 1; 
while (<>) {
	# yes, this is a broken regexp, but the sources are machine generated 
	if (/= new / && ! /Layout/) {
		( $varname, $parent) = split('=');
		# print "$varname: $parent"; 
		$varname =~ s/^\s*//;
		$varname =~ s/\s*$//;
		$found{$varname} = 1;
		$parent =~ s/^.*\(\s*//;
		$parent =~ s/\s*,.*$//;
		chomp($parent);
		if (!defined($found{$parent})) {
			print "Widget $varname constructed with ",
			      "uninitialised parent $parent !\n";
		}
	}
};
