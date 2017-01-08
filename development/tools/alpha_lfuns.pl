#!/usr/bin/perl

@lines;

while (<>) { push @lines, $_; }

$l = 0;
$found = 0;
while (1) {
    if ($lines[$l] =~ m{^/\*!}) {
        last if $found == 2;
        $found++;
    }
    print $lines[$l];
    $l++;
}

$cmdlines = [$line];
%cmds;
$cmdname = "";

while (1) {
    $line = $lines[$l];
    if ($line =~ /{ LFUN_NOACTION, "", Noop, Hidden }/) {
        push @$cmdlines, "\n";
        $cmds{$cmdname} = $cmdlines;
        $cmdlines = [$line];
        last;
    }
    if ($line =~ m/^\s*{\s*LFUN_([^,]+)/) {
        $cmdname = $1;
        push @$cmdlines, $line;
    } 
    elsif ($line =~ m{^/\*!}) {
        push @$cmdlines, "\n";
        $cmds{$cmdname} = $cmdlines;
        $cmdlines = [$line];
    }
    else {
        push @$cmdlines, $line;
    }
    $l++;
}

# Now output those, in order

foreach $c (sort keys %cmds) {
    print @{$cmds{$c}};
}

while ($l < scalar @lines) {
    print $lines[$l]; $l++;
}
