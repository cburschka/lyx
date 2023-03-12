#! /usr/bin/env perl
# -*- mode: perl; -*-

# This script does a line by line comparison of two tex files. Ignore LyX-version info

use File::Slurp qw(read_file);

my $file1_name = shift;
my $file2_name = shift;

my @file1 = read_file($file1_name);
my @file2 = read_file($file2_name);
chomp @file1;
chomp @file2;

my $line_count = 0;


my @diffs;

foreach my $file1_line (@file1) {
	$line_count++;
	next if ($line_count == 1); # Skip the first line with the version

	my $file2_line = $file2[$line_count-1];

	if ($file1_line ne $file2_line) {
		push @diffs, {
			line => $line_count,
			file1 => $file1_line,
			file2 => $file2_line,
		};
	}
}

my $diff_output = '';
foreach $diff (@diffs) {
	$diff_output .= $diff->{line} . ' - ' . $diff->{file1} . ' | ' . $diff->{file2} . "\n";
}

if ($diff_output) {
	die "Differences found!\n$diff_output\n";
}

exit(0);
