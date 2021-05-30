# -*- mode: perl; -*-
package prefTest;

our(@EXPORT, @ISA);
BEGIN {
  use Exporter   ();
  @ISA        = qw(Exporter);
  @EXPORT     = qw(getSubstitutes getConverters applyChanges);
}

sub getSubstitutes($$@);
sub getConverters($$$);
sub getConverter($$);
sub getNext($);
sub writeConverters($$);
sub createConvLine($$);

sub getSubstitutes($$@)
{
  my ($rAllowedKeys, $rSubst, @args) = @_;

  my @ctestpars = ();
  my $ctestparams = 0;
  for my $arg (@args) {
    if ($ctestparams) {
      push(@ctestpars, $arg);
    }
    else {
      if ($arg =~ /^([^=]+)=(.*)$/) {
	my $key = $1;
	my $value = $2;
	my $valid = 0;
	if (defined($rAllowedKeys->{$key})) {
	  if (ref($rAllowedKeys->{$key}) eq "ARRAY") {
	    for my $val (@{$rAllowedKeys->{$key}}) {
	      if ($val eq $value) {
		$valid = 1;
		last;
	      }
	    }
	  }
	  elsif ($rAllowedKeys->{$key} eq "integer") {
	    if ($value =~ /^\d+$/) {
	      $valid = 1;
	    }
	  }
	  elsif ($rAllowedKeys->{$key} eq "string") {
	    $valid = 1;
	  }
	}
	if ($valid) {
	  $rSubst->{$key} = [$value, 0];
	}
	else {
	  die("invalid key or value specified in \"$arg\"");
	}
      }
      else {
	$ctestparams = 1;
	push(@ctestpars, $arg);
      }
    }
  }
  return(@ctestpars);
}

sub getConverters($$$)
{
  my ($userdir, $rConverter, $add) = @_;

  if (open(FI, "$userdir/lyxrc.defaults")) {
    while (my $l = <FI>) {
      if ($l =~ s/^\s*\\converter\s+//) {
	my $entry = &getConverter($l, $add);
	if (defined($entry)) {
	  $rConverter->{$entry->[0]} = $entry->[1];
	}
      }
    }
    close(FI);
  }
}

sub getConverter($$)
{
  my ($l, $add) = @_;
  chomp($l);
  my ($from, $to, $cmd, $par);
  ($l, $from) = getNext($l);
  return undef if ($from !~ /(ps|tex|dvi)$/);
  ($l, $to) = getNext($l);
  return undef if ($to !~ /^((dvi3?|pdf[23456]?)(log)?)$/);
  my ($checkfor, $substitute, $extrapar);
  if ($from =~ /tex/) {
    $checkfor = qr/\s+\-shell\-(escape|restricted)/;
    $substitute = "-shell-escape";
    $extrapar = qr/^latex/;
  }
  elsif ($from =~ /ps$/) {
    $checkfor = qr/\s+\-dALLOWPSTRANSPARENCY/;
    $substitute = "-dALLOWPSTRANSPARENCY";
    $extrapar = qr/hyperref-driver=dvips/;
  }
  else {
    $checkfor = qr/\s+-i\s+dvipdfmx-unsafe.cfg/;
    $substitute = "-i dvipdfmx-unsafe.cfg";
    $extrapar = qr/^hyperref-driver=dvipdfm/;
  }
  ($l, $cmd) = getNext($l);
  if ($add) {
    if ($cmd !~ $checkfor) {
      if ($cmd =~ /^(\S+)\s*(.*)$/) {
	$cmd = "$1 $substitute $2";
	$cmd =~ s/\s+$//;
      }
    }
  }
  else {
    $cmd =~ s/$checkfor//;
  }
  ($l, $par) = getNext($l);
  return undef if ($par !~ $extrapar);
  my $key = "\"$from\" \"$to\"";
  if ($add) {
    return([$key, [$cmd, $par]]);
  }
  else {
    # Removes entry from prefs
    return([$key, [$cmd, $par, 1]]);
  }
}

sub getNext($)
{
  my ($l) = @_;
  my $val = undef;

  if ($l =~ /^\s*\"([^\"]*)\"\s*(.*)$/) {
    $val = $1;
    $l = $2;
  }
  elsif ($l =~ /^\s*(\S+)\s*(.*)$/) {
    $val = $1;
    $l = $2;
  }
  else {
    exit(7);
  }
  return($l, $val);
}

sub writeConverters($$)
{
  my ($fo, $rConverter) = @_;
  for my $key (sort keys %{$rConverter}) {
    if (! defined($rConverter->{$key}->[2])) {
      $rConverter->{$key}->[2] = 1;
      my $l = &createConvLine($key, $rConverter);
      print $fo $l;
    }
  }
}

sub createConvLine($$)
{
  my ($key, $rConverter) = @_;
  my $l = "\\converter $key \"$rConverter->{$key}->[0]\" \"$rConverter->{$key}->[1]\"\n";
  return($l);
}

sub applyChanges($$$$)
{
  my ($userdir, $rSubst, $rConverter, $add) = @_;

  if (open(FO, '>', "$userdir/preferences.tmp")) {
    if (open(FI, "$userdir/preferences")) {
      my $conv_section = -1;
      while (my $l = <FI>) {
	if ($conv_section-- == 0) {
	  &writeConverters(*FO, $rConverter);
	}
	if ($l =~ /^\# CONVERTERS SECTION/) {
	  $conv_section = 2;	# converters begin 2 lines later
	}
	if ($l =~ /^\\converter\s+(.*)$/) {
	  my $entry = &getConverter($1, $add);
	  if (defined($entry)) {
	    my $key = $entry->[0];
	    if (defined($rConverter->{$key})) {
	      if (defined($rConverter->{$key}->[2])) {
		$l = "";
	      }
	      else {
		$rConverter->{$key}->[2] = 1;
		$l = &createConvLine($key, $rConverter);
	      }
	    }
	    else {
	      # Converter defined only in preferences
	      $rConverter->{$key} = $entry->[1];
	      $rConverter->{$key}->[2] = 1;
	      $l = &createConvLine($key, $rConverter);
	    }
	  }
	}
	else {
	  for my $k (keys %{$rSubst}) {
	    if ($l =~ /^\\$k\b/) {
	      $l = "\\$k $rSubst->{$k}->[0]\n";
	      $rSubst->{$k}->[1] = 1;
	    }
	  }
	}
	print FO $l;
      }
      close(FI);
    }
    for my $k (keys %{$rSubst}) {
      if ($rSubst->{$k}->[1] == 0) {
	print FO "\\$k $rSubst->{$k}->[0]\n";
      }
    }
    &writeConverters(*FO, $rConverter);
    close(FO);
    rename("$userdir/preferences.tmp", "$userdir/preferences");
  }
}

return 1;
