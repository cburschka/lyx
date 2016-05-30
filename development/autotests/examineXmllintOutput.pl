#! /usr/bin/env perl
# -*- mode: perl; -*-

# Examine output of 'xmllint --sax --html'

my $file = $ARGV[0];		# file to examine

my @tag = ();			# tag entries = {"name" => strng, "count" => sequence number}
my $tagindex = 0;		# next tag to be written
$tag[$tagindex] = {};
my %errors = ();

if (open(FI, $file)) {
  my $line = 0;
  my $saxchartoprint = 0;
  while(my $l = <FI>) {
    $line++;
    chomp($l);
    if ($l =~ /^SAX.startElementNs\(([^, \)]+)/) {
      # new tag
      my $tag = $1;
      $saxchartoprint = 0;
      #print "Start tag $tag at line $line, tagindex = $tagindex\n";
      my $newentry = 1;
      if (defined($tag[$tagindex]->{"name"})) {
	if ($tag[$tagindex]->{"name"} eq $tag) {
	  $tag[$tagindex]->{"count"} = $tag[$tagindex]->{"count"}+1;
	  $newentry = 0;
	}
      }
      if ($newentry) {
	$tag[$tagindex] = {};
	$tag[$tagindex]->{"name"} = $tag;
	$tag[$tagindex]->{"count"} = 1;
      }
      $tagindex++;
      $tag[$tagindex] = {};
    }
    elsif ($l =~ /^SAX.endElementNs\(([^, \)]+)/) {
      $saxchartoprint = 0;
      my $tag = $1;
      $tagindex--;
      #print "End tag $tag at line $line, tagindex = $tagindex\n";
      if ($tagindex < 0) {
	die("tagindex < 0");
      }
      if ($tag[$tagindex]->{"name"} ne $tag) {
	die("End tag $tag does not match " . $tag[$tagindex]->{"name"} . " at line $line");
      }
    }
    elsif ($l =~ /^SAX.error: (.*)$/) {
      my $err = $1;
      if ($err =~ /Entity\s+'([a-zA-Z0-9]+)'\s+not defined$/) {
	# Ignore Entity 'xxxx' not defined
      }
      elsif (! defined($errors{$err})) {
	my $errmsg = "";
	my $trenner = "";
	for (my $i = 0; $i < $tagindex; $i++) {
	  $errmsg .= $trenner . $tag[$i]->{"name"} . "(" . $tag[$i]->{"count"} . ")";
	  $trenner = ", ";
	}
	$errors{$err} = $errmsg;
	print "$err -> $errmsg\n";
	# Print the next 3 lines starting with 'SAX.characters('
	$saxchartoprint = 3;
      }
    }
    elsif ($saxchartoprint > 0) {
      $saxchartoprint--;
      if ($l =~ /^SAX.characters\(([^\)]+)\)/) {
	print "\t$1\n";
      }
    }
  }
}
exit(0);
