# -*- mode: perl; -*-
package parsePoLine;

use strict;

our(@EXPORT, @ISA);

BEGIN {
    use Exporter   ();
    @ISA       = qw(Exporter);
    @EXPORT    = qw(parse_po_file getLineSortedKeys);
}

# Prototypes
sub parse_po_file($\%);
sub parse_po_line($$$$$ );
sub getLineSortedKeys(\%);
############

my ($status, $foundline, $msgid, $msgstr, $fuzzy);


my $alternative = 0;
my @entry = ();
my %entries = ();

sub parse_po_file($\%)
{
  $alternative = 0;
  @entry = ();
  %entries = ();

  my @result = ();
  my $resindex = 0;
  my ($file, $rMessages) = @_;
  if (open(FI, '<', $file)) {
    $status = "normal";
    $fuzzy = 0;
    my $lineno = 0;
    while (my $line = <FI>) {
      $lineno++;
      parse_po_line($line, $lineno, $rMessages, \@result, \$resindex);
      push(@entry, $line);

    }
    parse_po_line("", $lineno + 1, $rMessages, \@result, \$resindex);
    my @entr1 = @entry;
    $result[$resindex] = ["zzzzzzzzzzzz", \@entr1];
    close(FI);
  }
  return(@result);
}

sub parse_po_line($$$$$)
{
  my ($line, $lineno, $rMessages, $rresult, $rresindex) = @_;
  chomp($line);

  if ($status eq "normal") {
    if ($line =~ /^#, fuzzy/) {
      $fuzzy = 1;
    }
    elsif ($line =~ s/^msgid\s+//) {
      die("line alternate") if ($alternative);
      $foundline = $lineno;
      $status = "msgid";
      $msgid = "";
      parse_po_line($line, $lineno, $rMessages, $rresult, $rresindex);
    }
    elsif ($line =~ s/^\#\~ msgid\s+//) {
      $alternative = 1;
      $foundline = $lineno;
      $status = "msgid";
      $msgid = "";
      parse_po_line($line, $lineno, $rMessages, $rresult, $rresindex);
    }
  }
  elsif ($status eq "msgid") {
    if ($line =~ /^\s*"(.*)"\s*/) {
      $msgid .= $1;
    }
    elsif ($line =~ /^\#\~\s*"(.*)"\s*/) {
      die("line not alternate") if (! $alternative);
      $msgid .= $1;
    }
    elsif ($line =~ s/^msgstr\s+//) {
      $alternative = 0;
      $status = "msgstr";
      $msgstr = "";
      parse_po_line($line, $lineno, $rMessages, $rresult, $rresindex);
    }
    elsif ($line =~ s/^\#\~ msgstr\s+//) {
      $alternative = 1;
      $status = "msgstr";
      $msgstr = "";
      parse_po_line($line, $lineno, $rMessages, $rresult, $rresindex);
    }
  }
  elsif ($status eq "msgstr") {
    if ($line =~ /^\s*"(.*)"\s*/) {
      $msgstr .= $1;
    }
    elsif ($line =~ /^\#\~\s+"(.*)"\s*/) {
      die("line not alternate") if (! $alternative);
      $msgstr .= $1;
    }
    else {
      if (!defined($entries{$msgid})) {
	my @entr1 = @entry;
	$rresult->[${$rresindex}] = [$msgid, \@entr1];
	$entries{$msgid} = $msgstr;
      }
      else {
	if ($alternative) {
	  print "duplicated alternate entry: \"$msgid\"\n";
	}
	else {
	  print "duplicated entry: \"$msgid\"\n";
	}
	print " on line:          $foundline\n";
	print " original on line: $rMessages->{$msgid}->{line}\n\n";
      }
      @entry = ();
      $rMessages->{$msgid}->{line} = $foundline;
      $rMessages->{$msgid}->{fuzzy} = $fuzzy;
      $rMessages->{$msgid}->{msgstr} = $msgstr;
      $rMessages->{$msgid}->{alternative} = $alternative;
      $rMessages->{$msgid}->{entryidx} = ${$rresindex};
      ${$rresindex} = ${$rresindex}+1;
      $fuzzy = 0;
      $status = "normal";
    }
  }
  else {
    die("invalid status");
  }
}

sub getLineSortedKeys(\%)
{
  my ($rMessages) = @_;

  return sort {$rMessages->{$a}->{line} <=> $rMessages->{$b}->{line};} keys %{$rMessages};
}

1;


__END__

=pod

=encoding utf8

=head1 NAME

parsePoLine

=head1 SYNOPSIS

  use parsePoLine; #imports functions 'parse_po_file() and getLineSortedKeys()'

  my %Messages = ();
  my @entries = parse_po_file("sk.po", %Messages);

=head1 DESCRIPTION

This is used to parse a single po-file.

Results:
    %Messages The keys in this hash are the msgid-strings
              the value is reference to a hash wit following values:
        msgstr:      the translated string
        line:        the line-no in the po-file
        fuzzy:       boolean, if the string is fuzzy
        alternative: if set, so this entry is part of help-strings
        entryidx:    The index in the correspondig @entries array
    @entries  List of references to 2-valued arrays
        [0]:         msgid-string
        [1]:         The sequence of lines from the po-file
                     belonging to this entry.

To print the whole po-file:
    for my $entry (@entries) {
      print @{$entry->[1]};
    }

To get the index to a known $msgid:
    my $entriesidx = $Messages{$msgid}->{entryidx};

=head1 AUTHOR

Kornel Benko <Kornel.Benko@berlin.de>
