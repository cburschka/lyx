# -*- mode: perl; -*-
package CheckURL;
# file CheckURL.pm
#
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# authors: Kornel Benko <kornel@lyx.org>
#          Scott Kostyshak <skotysh@lyx.org>
#

# Check if given URL exists and is accessible
#
use strict;

our(@EXPORT, @ISA);
BEGIN {
  use Exporter   ();
  @ISA    = qw(Exporter);
  @EXPORT = qw(check_url);
}

# Prototypes
sub check_http_url($$$$);
sub check_ftp_dir_entry($$);
sub check_ftp_url($$$$);
sub check_unknown_url($$$$);
sub check_url($$);
################

sub check_http_url($$$$)
{
  require LWP::UserAgent;

  my ($protocol, $host, $path, $file) = @_;

  my $ua = LWP::UserAgent->new;
  my $getp = "/";
  if ($path ne "") {
    $getp .= $path;
  }
  if (defined($file)) {
    if ($getp =~ /\/$/) {
      $getp .= $file;
    }
    else {
      $getp .= "/$file";
    }
  }
  my $buf;
  $ua->agent("Firefox/43.0");
  my $response = $ua->get("$protocol://$host$getp");
  if ($response->is_success) {
    $buf = $response->decoded_content;
  }
  else {
    print " " . $response->status_line . ": ";
    return 3;
  }
  my @title = ();
  my $res = 0;
  while ($buf =~ s/\<title\>([^\<]*)\<\/title\>//i) {
    my $title = $1;
    $title =~ s/[\r\n]/ /g;
    $title =~ s/  +/ /g;
    $title =~ s/^ //;
    $title =~ s/ $//;
    push(@title, $title);
    print "title = \"$title\": ";
    if ($title =~ /Error 404|Not Found/) {
      print " Page reports 'Not Found' from \"$protocol://$host$getp\": ";
      $res = 3;
    }
  }
  return $res;
}

# Returns ($err, $isdir)
# returns 0, x if file does not match entry
#         1, x everything OK
#         2, x if not accesible (permission)
sub check_ftp_dir_entry($$)
{
  my ($file, $e) = @_;
  my $other = '---';
  my $isdir = 0;

  #print "Checking '$file' against '$e'\n";
  $file =~ s/^\///;
  $isdir = 1 if ($e =~ /^d/);
  return(0,$isdir) if ($e !~ /\s$file$/);
  if ($e =~ /^.[-rwx]{6}([-rwx]{3})\s/) {
    $other = $1;
  }
  else {
    #print "Invalid entry\n";
    # Invalid entry
    return(0,$isdir);
  }
  return(2,$isdir) if ($other !~ /^r/); # not readable
  if ($isdir) {
    #return(2,$isdir) if ($other !~ /x$/); # directory, but not executable
  }
  return(1,$isdir);
}

sub check_ftp2_url($$$$)
{
  my ($protocol, $host, $path, $file) = @_;

  my $checkentry = 1;
  print "\nhost $host\n";
  print "path $path\n";
  print "file $file\n";
  my $url = "$protocol://$host";
  $path =~ s/\/$//;
  if (defined($file)) {
    $url = "$url/$path/$file";
  }
  else {
    $url = "$url/$path/.";
  }
  print "curl $url, file = $file\n";
  my %listfiles = ();
  if (open(FFTP, "curl --anyauth -l $url|")) {
    while (my $l = <FFTP>) {
      chomp($l);
      $listfiles{$l} = 1;
    }
    close(FFTP);
  }
  if (%listfiles) {
    if (! defined($file)) {
      return(0, "OK");
    }
    elsif (defined($listfiles{$file})) {
      return(0, "OK");
    }
    elsif (defined($listfiles{"ftpinfo.txt"})) {
      return(0, "Probably a directory");
    }
    else {
      return(1, "Not found");
    }
  }
  else {
    return(1, "Error");
  }
}

sub check_ftp_url($$$$)
{
  use Net::FTP;

  my ($protocol, $host, $path, $file) = @_;
  my $res = 0;
  my $message = "";

  my $ftp = Net::FTP->new($host, Debug => 0, Timeout => 120);
  if(!$ftp) {
    return(3,"Cannot connect to $host");
  }
  if (! $ftp->login("anonymous",'-anonymous@')) {
    $message = $ftp->message;
    $res = 3;
  }
  else {
    my $rEntries;
    if ($path ne "") {
      #print "Path = $path\n";
      #if (!$ftp->cwd($path)) {
      # $message = $ftp->message;
      # $res = 3;
      #}
      $rEntries = $ftp->dir($path);
    }
    else {
      $rEntries = $ftp->dir();
    }
    if (! $rEntries) {
      $res = 3;
      $message = "Could not read directory \"$path\"";
    }
    elsif (defined($file)) {
      my $found = 0;
      my $found2 = 0;
      for my $f ( @{$rEntries}) {
	#print "Entry: $path $f\n";
	my ($res1,$isdir) = check_ftp_dir_entry($file,$f);
	if ($res1 == 1) {
	  $found = 1;
	  last;
	}
	elsif ($res1 == 2) {
	  # found, but not accessible
	  $found2 = 1;
	  $message = "Permission denied for '$file'";
	}
      }
      if (! $found) {
	$res = 4;
	if (! $found2) {
	  $message = "File or directory '$file' not found";
	}
      }
    }
  }
  $ftp->quit;
  #print "returning ($res,$message)\n";
  return($res, $message);
}

sub check_unknown_url($$$$)
{
  use LWP::Simple;

  my ($protocol, $host, $path, $file) = @_;
  my $res = 1;

  my $url = "$protocol://$host";
  if ($path ne "") {
    if ($path =~ /^\//) {
      $url .= $path;
    }
    else {
      $url .= "/$path";
    }
  }
  if(defined($file)) {
    #print "Trying $url$file\n";
    $res = head("$url/$file");
    if(! $res) {
      # try to check for directory '/';
      #print "Trying $url$file/\n";
      $res = head("$url/$file/");
    }
  }
  else {
    #print "Trying $url\n";
    $res = head($url);
  }
  return(! $res);
}

#
# Main entry
sub check_url($$)
{
  my($url,$use_curl) = @_;
  my $file = undef;
  my ($protocol,$host,$path);

  my $res = 0;

  # Split the url to protocol,host,path
  if ($url =~ /^([a-z]+):\/\/([^\/]+)(.*)$/) {
    $protocol = $1;
    $host = $2;
    $path = $3;
    $path =~ s/^\///;
    if($path =~ s/\/([^\/]+)$//) {
      $file = $1;
      if($file =~ / /) {
	# Filename contains ' ', maybe invalid. Don't check
	$file = undef;
      }
      $path .= "/";
    }
  }
  else {
    print " Invalid url '$url'";
    return 2;
  }
  if ($protocol =~ /^https?$/) {
    return check_http_url($protocol, $host, $path, $file);
  }
  elsif ($protocol eq "ftp") {
    my $message;
    if ($use_curl) {
      ($res, $message) = check_ftp2_url($protocol, $host, $path, $file);
    }
    else {
      ($res, $message) = check_ftp_url($protocol, $host, $path, $file);
    }
    return $res;
  }
  else {
    # it never should reach this point
    print " What protocol is '$protocol'?";
    $res = check_unknown_url($protocol, $host, $path, $file);
    return $res;
  }
}

1;
