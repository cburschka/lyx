#! /usr/bin/env perl
# -*- mode: perl; -*-

# file listFontWithLang.pl
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING
# or at http://www.lyx.org/about/licence.php
#
# author Kornel Benko
# Full author contact details are available in the file CREDITS
# or at https://www.lyx.org/Credits
#
# Usage: listFontWithLang.pl <options>
#   Displays installed system font names selected by <options>
#   Option-strings with more that 1 char need be prefixed by '--'
#
# Option to get list of options: -h
#
# Some equivalencies for instance with option -n
#	-n arial
#	-N=arial
#	--nAme=Arial
#	--name arial
# Options and option-parameter are case insensitive

BEGIN {
    use File::Spec;
    my $p = File::Spec->rel2abs( __FILE__ );
    $p =~ s/[\/\\]?[^\/\\]+$//;
    unshift(@INC, $p);
}

my $lyxfontsdir = $INC[0];
$lyxfontsdir =~ s/[\/\\]?[^\/\\]+$//;
$lyxfontsdir =~ s/[\/\\]?[^\/\\]+$//;
$lyxfontsdir .= "/lib/fonts";

use strict;
use warnings;
use Encode;
use GetOptions;
use constant {
  # for ftype
  SERIF => 1,
  SANS => 2,
  SCRIPT => 4,
  FRAKTUR => 8,
  DOUBLESTROKE => 16,
  FANCY => 32,
  INITIALS => 64,
  SYMBOL => 128,
  SMALLCAP => 256,

  # for UseProperty (UP)
  UPPROPERTY => 1,         # select
  UPNPROPERTY => 2,        # deselect
  UPPPROPERTIES => 4,      # print
  UPWPROPERTIES => 8,      # write to a file

  # for UseCharsets (UC)
  UCCONTAINS => 1,         # select
  UCNCONTAINS => 2,        # deselect
  UCPCONTAINS => 4,        # print

  # for UseScripts (US)
  USSCRIPT => 1,           # select
  USNSCRIPT => 2,          # deselect
  USMSCRIPT => 4,          # math
  USPSCRIPT => 8,          # print
  USWSCRIPT => 16,         # write to a file
};

# These will be set according to options
my $iproperty = 0;         # info for Use Properties
my $icontains = 0;         # info for Use Charsets
my $iscript = 0;           # info for Use Scripts

sub convertlang($);
sub extractlist($$$);	# my ($l, $islang, $txt, $rres) = @_;
sub getIndexes($$);
sub getVal($$$$);	# my ($l, $txtval, $txtlang, $combine) = @_;
sub getproperties($$$$);
sub ismathfont($$);
sub correctstyle($);
sub decimalUnicode($);
sub contains($$);
sub sprintIntervalls($);
sub buildFontName($$);
sub splitatlU($);          # split text at change from lower case to upper case
sub splitStyle($);

# Following fields for a parameter can be defined:
# fieldname:         Name of entry in %options
# type:              [:=][sif], ':' = optional, '=' = required, 's' = string, 'i' = integer, 'f' = float
# alias:             reference to a list of aliases e.g. ["alias1", "alias2", ... ]
# listsep:           Separator for multiple data
# comment:           Parameter description
my @optionsDef = (
  # help + verbose already handled in 'GetOptions'
  ["n",
   {fieldname => "FontName", listsep => ',',
    type => "=s", alias => ["name"],
    comment => "Select font-names matching these (comma separated) regexes"},],
  ["nn",
   {fieldname => "NFontName",
    type => "=s", listsep => ',',
    comment => "Select font-names NOT matching these (comma separated) regexes"},],
  ["p",
   {fieldname => "Property",
    type => "=s", listsep => ',',
    comment => "Select fonts with properties matching these (comma separated) regexes"},],
  ["np",
   {fieldname => "NProperty",
    type => "=s", listsep => ',',
    comment => "Select fonts with properties NOT matching these (comma separated) regexes"},],
  ["s",
   {fieldname => "Scripts",
    type => "=s", listsep => ',',
    comment => "Select fonts with scripts matching these (comma separated) regexes"},],
  ["ns",
   {fieldname => "NScripts",
    type => "=s", listsep => ',',
    comment => "Select fonts with scripts NOT matching these (comma separated) regexes"},],
  ["math",
   {fieldname => "Math",
    comment => "Select fonts probably containing math glyphs"},],
  ["c",
   {fieldname => "Contains",  alias => ["contains"],
    type => "=s", listsep => ',',
    comment => "Select fonts containing all these (possibly comma separated) glyphs",
    comment2 => "____example: -c=\"0-9,u+32-u+x7f\"",}],
  ["nc",
   {fieldname => "NContains",
    type => "=s", listsep => ',',
    comment => "Select fonts NOT containing any of these (possibly comma separated) glyphs",
    comment2 => "____example: --nc=\"0-9,u+32-u+x7f\"",}],
  ["l",
   {fieldname => "Lang",
    type => "=s", alias=>["lang"],
    comment => "Comma separated list of desired languages"},],
  ["pc",
   {fieldname => "PrintCharset", alias => ["printcharset"],
    comment => "Print intervals of supported unicode character values"},],
  ["pl",
   {fieldname => "PrintLangs", alias => ["printlangs"],
    comment => "Print supported languages"},],
  ["pp",
   {fieldname => "PrintProperties", alias => ["printproperties"],
    comment => "Print properties from weight, slant and width"},],
  ["ps",
   {fieldname => "PrintScripts", alias => ["printscripts"],
    comment => "Print supported scripts"},],
  ["pf",
   {fieldname => "PrintFiles", alias => ["printfiles"],
    comment => "Print font file names"},],
  ["pw",
   {fieldname => "PrintWarnings",
    comment => "Print warnings about discarded/overwritten fonts, conflicting styles"},],
  ["wf",
   {fieldname => "Write",
    type => "=s", alias => ["writefile"],
    comment => "Write to a file for later use"},],
);
my %options = %{&handleOptions(\@optionsDef)};

$options{Lang} = "" if (! defined($options{Lang}));

#############################################################
my %mapShortcuts = (
  "Cond" => "Condensed",
  "Expd" => "Expanded",
  "Lt"   => "Light",
  "Med"  => "Medium",
  "med"  => "Medium",
  "bol"  => "Bold",
);

my @langs = split(',', $options{Lang});
for my $lg (@langs) {
  $lg = &convertlang($lg);
}

$iproperty |= UPPROPERTY if (defined($options{Property}));
$iproperty |= UPNPROPERTY if (defined($options{NProperty}));
$iproperty |= UPPPROPERTIES if (exists($options{PrintProperties}));

$icontains |= UCCONTAINS if (defined($options{Contains}));
$icontains |= UCNCONTAINS if (defined($options{NContains}));
$icontains |= UCPCONTAINS if (exists($options{PrintCharset}));

$iscript |= USSCRIPT if (defined($options{Scripts}));
$iscript |= USNSCRIPT if (defined($options{NScripts}));
$iscript |= USMSCRIPT if (exists($options{Math}));
$iscript |= USPSCRIPT if (exists($options{PrintScripts}));

if (defined($options{Write})) {
  if (! open(INFOS, '>', $options{Write})) {
    die("Could not open file $options{Write} for write");
  }
  $iproperty |= UPWPROPERTIES;
  # not yet ...
  # $iscript |= USWSCRIPT;
}

for my $charFld ("Contains", "NContains") {
  if (defined($options{$charFld})) {
    my %glyphs = ();         # To ignore duplicates
    for my $a1 (@{$options{$charFld}}) {
      for my $e (decimalUnicode($a1)) {
        $glyphs{$e} = 1;
      }
    }
    # create intervalls
    my @glyphs = sort {$a <=> $b;} keys %glyphs;

    # $options{$charFld} no longer needed, so use it for unicode-point intervalls
    $options{$charFld} = [];
    my ($first, $last) = (undef, undef);
    for my $i (@glyphs) {
      if (! defined($last)) {
        $first = $i;
        $last = $i;
        next;
      }
      if ($i == $last+1) {
        $last = $i;
        next;
      }
      push(@{$options{$charFld}}, [$first, $last]);
      $first = $i;
      $last = $i;
    }
    if (defined($last)) {
      push(@{$options{$charFld}}, [$first, $last]);
    }
    if (exists($options{verbose})) {
      if ($charFld eq "Contains") {
        print "Checking for unicode-points: " . &sprintIntervalls($options{$charFld}) . "\n";
      }
      else {
        print "Ignore if matching unicode-points: " . &sprintIntervalls($options{$charFld}) . "\n";
      }
    }
  }
}

for my $fn ("FontName", "NFontName") {
  if (defined($options{$fn})) {
    # split each entry and make a compiled regex
    # Allow space between all characters
    for my $e (@{$options{$fn}}) {
      if ($e =~ /(\^|\\|\||\[|\]|\(|\)|\*|\+|\?)/) {
        # already regex, don't manipulate
        $e = qr/$e/i;
      }
      else {
        my $u = decode('utf-8', $e);
        my $fill = decode('utf-8', "\\s?");
        my @u = split(//, $u);
        my $ud = join($fill, @u);
        my $e1 = encode('utf-8', $ud);
        $e = qr/$e1/i;
      }
    }
  }
}

my @cmds = ();
{
  my $cmd = "fc-list";
  my $langs = "";
  if (defined($langs[0])) {
    $langs = " :lang=" . join(',', @langs) . " ";
  }

  my $format = "foundry=\"%{foundry}\"" .
      " postscriptname=\"%{postscriptname}\"" .
      " fn=\"%{fullname}\" fnl=\"%{fullnamelang}\"" .
      " family=\"%{family}\" flang=\"%{familylang}\" " .
      " style=\"%{style}\" stylelang=\"%{stylelang}\"";

  if ($iscript) {
    $format .= " script=\"%{capability}\"";
  }
  if (exists($options{PrintLangs}) || defined($langs[0])) {
    $format .= " lang=\"%{lang}\"";
  }
  if ($iproperty) {
    $format .= " weight=%{weight} slant=%{slant} width=%{width} spacing=%{spacing}";
  }
  if ($icontains) {
    $format .= " charset=\"%{charset}\"";
  }
  $format .= " file=\"%{file}\" abcd\\n";
  $cmd .= $langs . " -f '$format'";
  push(@cmds, $cmd);

  # Now Add also lyx fonts to be examinated
  if (opendir(DI, "$lyxfontsdir")) {
    while (my $l = readdir(DI)) {
      chomp($l);
      if ($l =~ /\.ttf$/) {
	my $file = "$lyxfontsdir/$l";
	push(@cmds, "fc-query$langs -f '$format' '$file'");
      }
    }
    closedir(DI);
  }
}


my %ftypes = (
  # Dummy internal map
  # using '()' to prevent the initializer to take
  #    the key as a string. (Constants in perl _are_ functions without argument)
  SERIF() => "Serif",
  SANS() => "Sans",
  SCRIPT() => "Script",
  FRAKTUR() => "Fraktur",
  DOUBLESTROKE() => "DoubleStroke",
  FANCY() => "Fancy",
  INITIALS() => "Initials",
  SYMBOL() => "Symbol",
  SMALLCAP() => "SmallCap",
  "default" => 1,
);

my %weights = (
  0 => "Thin",
  40 => "Extralight",
  50 => "Light",
  55 => "Semilight",
  75 => "Book",
  80 => "Regular",
  100 => "Medium",
  180 => "Semibold",
  200 => "Bold",
  205 => "Extrabold",
  210 => "Black",
  215 => "ExtraBlack",
);

my %slants = (
  0 => "Roman",
  100 => "Italic",
  110 => "Oblique",
);

my %widths = (
  50 => "Ultracondensed",
  63 => "Extracondensed",
  75 => "Condensed",
  87 => "Semicondensed",
  100 => "Normal",
  113 => "Semiexpanded",
  125 => "Expanded",
  150 => "Extraexpanded",
  200 => "Ultraexpanded",
);

my %spacings = (
  0 => "Proportional",
  90 => "Dual",
  100 => "Mono",
  110 => "Charcell",
  "default" => "Proportional",
);

# Build reverse mappings, (not needed yet)
for my $txt (qw(ftypes weights slants widths spacings)) {
  my $map;
  eval "\$map = \\%$txt";
  for my $key (keys %{$map}) {
    next if ($key !~ /^\d+$/);
    my $value = lc($map->{$key});
    $map->{"r"}->{$value} = $key;
  }
}

# key:= fontname
#     subkey foundry
#            subfoundry
my %collectedfonts = ();
my %fontpriority = (
  otf => 0,                # type 2, opentype CFF (Compact Font Format)
  ttc => 1.1,              # type 1 (True Type font Collection)
  ttf => 1.2,              # type 1 (True Type Font)
  woff=> 1.3,              # type 1 (Web Open Font Format)
  t1  => 1.4,              # type 1 (postscript)
  pfb => 1.5,              # type 1 (Printer Font Binary)
  pfa => 1.6,              # type 1 (Printer Font Ascii)
  pfm => 2,                # requires associated .pfb file
  pcf => 5,                # Bitmap (Packaged Collaboration Files)?
);
my $nexttype = 6;

my %serifFonts = (
  # some extra serif fonts
  "value" => SERIF,
  "g" => qr/^gfs ?(didot)/i,
);
# list of regexes for known sans serif fonts
my %sansFonts = (
  "value" => SANS,          # Sans serif
  "a" => qr/^a(030|bydos|haroni|e?rial|ndika|ngostura|nonymous|rab|roania|rimo|rundina|sap|e almothnna|egean|egyptus|l (arabiya|battar|hor|manzomah|yarmook)|lmonte|natolian|ndale|nglepoise|njali|xaxa)/i,
  "b" => qr/^b(abel ?stone ?modern|aekmuk|alker|altar|andal|angwool|arbatrick|aveuse|old|dxsfm|ebas|erenika|eteckna|euron|iometric|iting|lue|m ?hanna)/i,
  "c" => qr/^c(abin|aliban|antarell|arbon|arlito|handas|harles|hilanka|hinese ?rocks|hivo|mu bright|omfortaa|omi[cx]|omputer ?(bold|(modern ?(bright ?((semi)?bold|math|oblique|roman|italic))))|oolvetica|ortoba|ountries|ousine|uprum|wtex(hei|yen)|yklop|ypro)/i,
  "d" => qr/^(d2coding|dimnah|dosis|dyuthi)/i,
  "e" => qr/^(electron|engebrechtre)/i,
  "f" => qr/^(fandolhei|fetamont|fira|font awesome 5|forgotten)/i,
  "g" => qr/^g(ardiner|aruda|fs ?neo|illius|ood ?times|ranada|raph|uanine|unplay)/i,
  "h" => qr/^(hack|hani|haramain|harano|harmattan|hor\b)/i,
  "i" => qr/^(ibm ?(plex ?mono|3270)|ikarius|inconsolata|induni.?h|iwona)/i,
  "j" => qr/^j(ara|ura|s ?math.?bbold)/i,
  "k" => qr/^(kalimati|kanji|karla|karma|kayrawan|kenyan|keraleeyam|khalid|khmer [or]|kiloji|klaudia|ko[mn]atu|kp ?mono|kurier|kustom)/i,
  "l" => qr/^l(aksaman|arabie|ato|eague|exend|exigulim|ibel|iberation ?mono|ibre franklin|ibris|inux biolinum|obster|ogix|ohit ?(devana|telugu)|oma)/i,
  "m" => qr/^m(\+ |anchu|anjari|arcellus|ashq|eera|etal|igmix|igu|ikachan|intspirit|iriam ?clm|isaki|itra ?mono|ona|onlam|ono(fonto|id|isome|noki)|ontserrat|otoyal|ukti|usica)/i,
  "n" => qr/^(nachlieli|nada|nafees|nagham|nanum(barunpen|square)|nice|noto ?mono)/i,
  "o" => qr/^(ocr|okolaks|open ?dyslexic|ostorah|ouhud|over|oxygen)/i,
  "p" => qr/^(padauk|pagul|paktype|pakenham|palladio|petra|phetsarath|play\b|poiret|port\b|primer\b|prociono|pt\b|purisa)/i,
  "q" => qr/^(qt(ancient|helvet|avanti|doghaus|eratype|eurotype|floraline|frank|fritz|future|greece|howard|letter|optimum)|quercus)/i,
  "r" => qr/^(rachana|radio\b|raleway|ricty|roboto|rosario)/i,
  "s" => qr/^(salem|samanata|sawasdee|shado|sharja|simple|sophia|soul|source|switzera)/i,
  "t" => qr/^(tarablus|teen|tex ?gyre ?(adventor|heros)|tiresias|trebuchet|tscu|tuffy)/i,
  "u" => qr/^u(buntu|kij (bom|chechek|cjk|diwani|ekran|elipbe|inchike|jelliy|kufi|mejnuntal|qara|qolyazma|teng|title|tor|tuz ?(neqish|tom))|mpush|n ?(dinaru|jamo|graphic|taza|vada|yetgul)|ni(kurd|space|versalis)|roob|rw ?classico)/i,
  "v" => qr/^(veranda|vn ?urwclassico)/i,
  "w" => qr/^(waree)/i,
  "y" => qr/^(yanone)/i,
  "z" => qr/^(zekton|zero)/i,
);
my %scriptFonts = (
  "value" => SCRIPT,          # Script
  "a" => qr/^a(becedario|ir ?cut|ugie|uriocus ?kalligraph)/i,
  "b" => qr/^b(reip|rush ?script)/i,
  "c" => qr/^(chancery)/i,
  "d" => qr/^(dancing)/i,
  "e" => qr/^(elegante)/i,
  "f" => qr/^femkeklaver/i,
  "j" => qr/^jsmath.?(rsfs)/i,
  "k" => qr/^(kaushan|karumbi|kristi)/i,
  "m" => qr/^(math ?jax.?script|miama)/i,
  "n" => qr/^(nanum (brush|pen) script)/i,
  "p" => qr/^pecita/i,
  "q" => qr/^qt( ?black ?forest|arabian|boulevard|brush ?stroke|chancery|coronation|florencia|handwriting|linostroke|merry|pandora|slogan)/i,
  "r" => qr/^((romande.*|ruf)script|rsfs)/i,
  "t" => qr/^typo ?script/i,
  "u" => qr/^u(n ?pilgi|rw ?chancery|kij ?(jelliy|moy|qolyazma ?(tez|yantu)))/i,
);

my %fraktFonts = (
  "value" => FRAKTUR,          # Fraktur
  "b" => qr/^boondox ?fraktur/i,
  "e" => qr/^eufm/i,
  "j" => qr/^(jsmath.?euf)/i,
  "m" => qr/^(missaali)/i,
  "o" => qr/^(oldania)/i,
  "q" => qr/^qt(blackforest|cloisteredmonk|dublinirish|fraktur|heidelbergtype|(lino|london) ?scroll)/i,
  "u" => qr/^ukij ?(kufi ?tar|mejnun ?reg)/i,
);

my %fancyFonts = (
  "value" => FANCY,          # Fancy
  "a" => qr/^a(bandoned|bberancy|driator|irmole|lmendra ?display|lmonte (snow|woodgrain)|nalecta|ni|nklepants|nn ?stone|oyagi|rt ?nouveau ?caps|stron|xaxa)/i,
  "b" => qr/^b(aileys|alcony|altar|andal|arbatrick|aveuse|eat ?my|etsy|iometric|iting|lankenburg|oondox ?callig|org|oron|raeside|ramalea|udmo|urnstown|utterbelly)/i,
  "c" => qr/^c(retino|msy|abin ?sketch|arbon|arolingan|harles|hicken|hilanka|hr\d)/i,
  "d" => qr/^dseg/i,
  "e" => qr/^electorate/i,
  "f" => qr/^frederika/i,
  "g" => qr/^(gfs.?theo)/i,
  "j" => qr/^jsmath.cmsy/i,
  "k" => qr/^keter|kicking|kredit|kouzan/i,
  "l" => qr/^lcmsy/i,
  "q" => qr/^qtcaslan ?open/i,
  "u" => qr/^u(kij ?(saet|tiken)|nion ?city)/i,
  "v" => qr/^vectroid/i,
);

my %initialFonts = (
  "value" => INITIALS,          # Initials
  "c" => qr/^carrick/i,
  "e" => qr/^(eb.?garamond.?init)/i,
  "t" => qr/^typographer/i,
  "y" => qr/^(yinit)/i,
);

my %symbolFonts = (
  "value" => SYMBOL,          # Symbol
  "a" => qr/^a(cademicons|lblant|lianna|mar|nka|rb?\d|rchaic|rrow|rs|rt[mt]|ssy(rb\d+)?\b|miri ?quran|mit\b)/i,
  "b" => qr/^b(aby ?jeepers|bding|euron|guq|lex|lsy|oondox ?upr|ullets|urma)/i,
  "c" => qr/^c(aladings|cicons|hess|msy|mex|apacitor|ounterscraps)/i,
  "d" => qr/^(dingbats|drmsym|d05)/i,
  "e" => qr/^e(lusiveicons|mmentaler|moji|sint|uterpe)/i,
  "f" => qr/^(fandol.?brail|fdsymbol|fourierorns|font(awesome|ello|.?mfizz))/i,
  "g" => qr/^(gan.?clm|gfs.?(baskerville|gazis|olga|porson|solomos|(bodoni|didot).?classic|complutum))/i,
  "h" => qr/^h(ots|ershey)/i,
  "j" => qr/^jsmath.?(msam|cmsy|masm|msbm|wasy|cmex|stmary)/i,
  "l" => qr/^l(cmsy|msam|ohit ?(Assamese|Bengali|Gujarati|Gurmukhi|Kannada|Malayalam|Marathi|Nepali|Odia|Tamil))/i,
  "m" => qr/^(marvosym|material|msam|msbm)/i,
  "n" => qr/^(noto.*(emoji|brahmi))/i,
  "o" => qr/^(octicons)/i,
  "p" => qr/^patch/i,
  "q" => qr/^(qtding ?bits)/i,
  "s" => qr/^s(emafor|kak|tmary|s?msam|tix ?math)/i,
  "t" => qr/^(te ?xxslh?[du]|typicons|twemoji)/i,
  "u" => qr/^ukij ?(imaret|orxun|tughra)/i,
  "w" => qr/^w(ebdings|asy|elfare ?brat)/i,
);

my %smallcapFonts = (
  "value" => SMALLCAP | SERIF,
  "c" => qr/^c(s[ct]sc\d|inzel|omputer ?modern ?sans ?italic ?regular ?\d)/i,
  "d" => qr/^drm(it)?sc\d/i,
  "f" => qr/^fetamont.?script/i,
  "n" => qr/^newtxb?ttsc/i,
  "s" => qr/^(screengem|sf.?kp.?sc)/i,
  "t" => qr/^t1?xb?ttsc/i,
  "v" => qr/^vn ?cccsc\d/i,
);

for my $cmd (@cmds) {
  if (open(FI,  "$cmd |")) {
   NXTLINE: while (my $l = <FI>) {
      chomp($l);
      while ($l !~ /abcd$/) {
	$l .= <FI>;
	chomp($l);
      }
      my $file = "";
      my $fonttype;
      if ($l =~ /file=\"([^\"]+)\"/) {
	$file = $1;
	#next if ($file !~ /\.(otf|ttf|pfa|pfb|pcf|ttc)$/i);
	if ($file !~ /\.([a-z0-9]{2,5})$/i) {
	  print "Unhandled extension for file $file\n";
	  next;
	}
	$fonttype = lc($1);
	if (! defined($fontpriority{$fonttype})) {
	  print "Added extension $fonttype for file $file\n";
	  $fontpriority{$fonttype} = $nexttype;
	  $nexttype++;
	}
      }
      my %usedlangs = ();
      if ($l =~ / lang=\"([^\"]+)\"/) {
	my @ll = split(/\|/, $1);
	for my $lx (@ll) {
	  $usedlangs{&convertlang($lx)} = 1;
	}
      }

      for my $lang (@langs) {
	next NXTLINE if (! defined($usedlangs{$lang}));
      }
      my ($fullname, $fuidx) = &getVal($l, "fn", "fnl", -1);
      my ($style, $fsidx) = &getVal($l, "style", "stylelang", $fuidx);
      $style =~ s/^\\040//;
      my ($family, $faidx)  = &getVal($l, "family", "flang", $fsidx);

      my $postscriptname = "";
      if ($l =~ /postscriptname=\"([^\"]+)\"/) {
	$postscriptname = $1;
      }
      my $fontname;
      ($fontname, $style) = &buildFontName($family, $style);
      if (defined($options{NFontName})) {
	for my $fn (@{$options{NFontName}}) {
	  next NXTLINE if ($fontname =~ $fn);
	}
      }
      if (defined($options{FontName})) {
	for my $fn (@{$options{FontName}}) {
	  next NXTLINE if ($fontname !~ $fn);
	}
      }
      my @charlist = ();
      if ($icontains) {
	if ($l =~ / charset=\"([^\"]+)\"/) {
	  my @list = split(/\s+/, $1);
	  for my $e (@list) {
	    my ($l, $h) = split('-', $e);
	    $h = $l if (! defined($h));
	    push(@charlist, [hex($l), hex($h)]);
	  }
	}
	if ($icontains & UCCONTAINS) {
	  for my $g (@{$options{Contains}}) {
	    next NXTLINE if (! contains($g, \@charlist));
	  }
	}
	if ($icontains & UCNCONTAINS) {
	  for my $g (@{$options{NContains}}) {
	    # Ignore if ANY char exist in @charlist
	    for (my $i = $g->[0]; $i <= $g->[1]; $i++) {
	      next NXTLINE if (contains([$i,$i], \@charlist));
	    }
	  }
	}
      }
      my $props = "";
      my $wprops = "";
      my @errors = ();
      if ($iproperty) {
	my $properties = getproperties($l, $fontname, $style, \@errors);
	if ($iproperty & UPPROPERTY) {
	  for my $pn (@{$options{Property}}) {
	    next NXTLINE if ($properties !~ /$pn/i);
	  }
	}
	if ($iproperty & UPNPROPERTY) {
	  for my $pn (@{$options{NProperty}}) {
	    next NXTLINE if ($properties =~ /$pn/i);
	  }
	}
	if ($iproperty & UPPPROPERTIES) {
	  $props .= " ($properties)";
	}
	if ($iproperty & UPWPROPERTIES) {
	  $wprops .= " ($properties)";
	}
      }
      if (exists($options{PrintLangs})) {
	$props .= '(' . join(',', sort keys %usedlangs) . ')';
      }
      if (exists($options{PrintCharset})) {
	$props .= '(' . &sprintIntervalls(\@charlist) . ')';
      }
      if ($iscript) {
	my @scripts = ();
	my $scripts = "";
	if ($l =~ / script=\"([^\"]+)\"/) {
	  @scripts = split(/\s+/, $1);
	  for my $ent (@scripts) {
	    $ent =~ s/^\s*otlayout://;
	    $ent = lc($ent);
	  }
	  $scripts = join(',', @scripts);
	}
	if ($iscript & USMSCRIPT) {
	  next NXTLINE if (! &ismathfont($fontname,\@scripts));
	}
	if ($iscript & USPSCRIPT) {
	  $props .= "($scripts)";
	}
	if (!defined($scripts[0])) {
	  # No script defined in font, so check only $options{Scripts}
	  next NXTLINE if ($iscript & USSCRIPT);
	}
	else {
	  if ($iscript & USSCRIPT) {
	    for my $s (@{$options{Scripts}}) {
	      next NXTLINE if ($scripts !~ /$s/i);
	    }
	  }
	  if ($iscript & USNSCRIPT) {
	    for my $s (@{$options{NScripts}}) {
	      next NXTLINE if ($scripts =~ /$s/i);
	    }
	  }
	}
      }
      my $foundry = "";
      if ($l =~ /foundry=\"([^\"]+)\"/) {
	$foundry = $1;
	$foundry =~ s/^\s+//;
	$foundry =~ s/\s+$//;
      }
      if (defined($collectedfonts{$fontname}->{$foundry}->{errors})) {
	# Apparently not the first one, so add some info
	my $oldfonttype = $collectedfonts{$fontname}->{$foundry}->{fonttype};
	if (defined($errors[0])) {
	  push(@{$collectedfonts{$fontname}->{$foundry}->{errors}}, @errors);
	}
	if ($fontpriority{$oldfonttype} > $fontpriority{$fonttype}) {
	  push(@{$collectedfonts{$fontname}->{$foundry}->{errors}}, "Warning: overwriting old info of file: " . $collectedfonts{$fontname}->{$foundry}->{file});
	}
	else {
	  push(@{$collectedfonts{$fontname}->{$foundry}->{errors}}, "Warning: discarding new info from file: $file");
	  next;
	}
      }
      else {
	$collectedfonts{$fontname}->{$foundry}->{errors} = \@errors;
      }
      $collectedfonts{$fontname}->{$foundry}->{props} = $props;
      $collectedfonts{$fontname}->{$foundry}->{wprops} = $wprops;
      $collectedfonts{$fontname}->{$foundry}->{file} = $file;
      $collectedfonts{$fontname}->{$foundry}->{fonttype} = $fonttype;
    }
    close(FI);
  }
}

for my $fontname (sort keys %collectedfonts) {
  my @foundries = sort keys %{$collectedfonts{$fontname}};
  my $printfoundries = 1;
  if (defined($foundries[1])) {
    $printfoundries = 1;
  }
  for my $foundry (@foundries) {
    if (exists($options{PrintWarnings})) {
      for my $err (@{$collectedfonts{$fontname}->{$foundry}->{errors}}) {
        print "$err\n";
      }
    }
    my $fn = "Font : $fontname";
    if ($printfoundries && ($foundry ne "")) {
      $fn .= " \[$foundry\]";
    }
    print $fn;
    print INFOS $fn if (defined($options{Write}));
    print $collectedfonts{$fontname}->{$foundry}->{props};
    print INFOS $collectedfonts{$fontname}->{$foundry}->{wprops} if (defined($options{Write}));
    print INFOS ": " . $collectedfonts{$fontname}->{$foundry}->{file} . "\n" if (defined($options{Write}));
    if (exists($options{PrintFiles})) {
      print ": " . $collectedfonts{$fontname}->{$foundry}->{file} . "\n";
    }
    else {
      print "\n";
    }
  }
}
close(INFOS) if (defined($options{Write}));

exit(0);
#################################################################################
sub convertlang($)
{
  my ($ilang) = @_;
  if ($ilang =~ /^\s*([a-z]+)([\-_]([a-z]+))?\s*$/i) {
    my ($x, $y) = ($1, $3);
    if (defined($y)) {
      $ilang = lc($x) . '-' . lc($y);
    }
    else {
      $ilang = lc($x);
    }
  }
  return($ilang);
}

sub extractlist($$$)
{
  my ($l, $islang, $txt, $rres) = @_;
  my @res = ();
  if ($l =~ /$txt=\"([^\"]+)\"/) {
    @res = split(',', $1);
    if ($islang) {
      for my $lg (@res) {
	$lg = &convertlang($lg);
      }
    }
  }
  @{$rres} = @res;
}

sub getIndexes($$)
{
  my ($lang, $rlangs) = @_;
  my @res = ();

  for (my $i = 0; defined($rlangs->[$i]); $i++) {
    if ($rlangs->[$i] eq $lang) {
      push(@res, $i);
    }
  }
  return(\@res);
}

sub getVal($$$$)
{
  my ($l, $txtval, $txtlang, $startentry) = @_;
  my @values = ();
  my @langs = ();
  &extractlist($l, 0, $txtval, \@values);
  return("", -1) if (! defined($values[0]));
  &extractlist($l, 1, $txtlang, \@langs);
  my $i = &getIndexes("en", \@langs);
  my $usedentry = -1;
  my $actualentry = -1;
  my $res = "";
  for my $k (@{$i}) {
    if (defined($values[$k])) {
      $actualentry++;
      if ($startentry < 0) {
        if (length($values[$k]) > length($res)) {
          $res = $values[$k];
          $usedentry = $actualentry;
        }
      }
      elsif ($actualentry == $startentry) {
        $res = $values[$k];
        $usedentry = $actualentry;
        last;
      }
      else {
        # select the longest entry if possible
        if (length($values[$k]) > length($res)) {
          $res = $values[$k];
          $usedentry = $actualentry;
        }
      }
    }
  }
  return($values[0], -1) if ($res eq "");
  return($res, $usedentry);
}

sub getsinglevalue($$$)
{
  my ($l, $txt, $rMap, $rget) = @_;
  my $val;
  if ($l =~ / $txt=(\d+)/) {
    $val = $1;
    # Search for nearest value to $val
    if (defined($rMap->{$val})) {
      return($rMap->{$val});
    }
    my $maxv = -1;
    my $minv = 1000;
    for my $key (keys %{$rMap}) {
      next if ($key !~ /^\d+$/);
      my $diff = abs($key - $val);
      if ($diff < $minv) {
        $maxv = $key;
        $minv = $diff;
      }
      elsif ($diff == $minv) {
        if ($key < $maxv) {
          $maxv = $key;
        }
      }
    }
    if (! defined($rMap->{$maxv})) {
      print "ERROR2: txt=$txt, val=$val\n";
      exit(-2);
    }
    if ($val > $maxv) {
      return($rMap->{$maxv} . "+$minv");
    }
    else {
      return($rMap->{$maxv} . "-$minv");
    }
  }
  else {
    return(undef);
  }
}

sub addTxt($$)
{
  my ($txt, $val) = @_;
  return("$txt($val)");
}

sub getftype($$)
{
  my ($fontname, $style) = @_;
  my $resftype = 0;
  if ($fontname =~ /(sans)[-_ ]?(serif)?/i) {
    $resftype |= SANS;
  }
  elsif ($fontname =~ /gothic|dotum|gulim/i) {
    if ($fontname =~ /bisrat gothic/i) {
      $resftype |= SERIF;
    }
    elsif ($fontname !~ /hershey/i) {
      $resftype |= SANS;
    }
  }
  elsif ($fontname =~ /^(js ?math.?)?bbold|msbm|^(ds(rom|serif|ss))|DoubleStruck/i) {
    $resftype |= DOUBLESTROKE;  # Double stroke (math font)
  }
  if ($fontname =~ /serif|times|mincho|batang/i) {
    $resftype |= SERIF; # Serif
  }
  if ($fontname =~ /initial(s|en)/i) {
    $resftype |= INITIALS;
    if ($fontname =~ /^linux ?libertine/i) {
      $resftype |= SERIF;
    }
  }
  if ($fontname =~ /participants/i) {
    $resftype |= SANS|FANCY;
  }
  if ($fontname =~ /symbol|cherokee/i) {
    if ($fontname !~ /^(symbola|asap)/i) {
      $resftype |= SYMBOL;
      if ($fontname =~ /^(ams ?math|computer modern bright msb)/i) {
        $resftype |= DOUBLESTROKE | SERIF;
      }
    }
  }
  if ($fontname =~ /callig/i) {
    $resftype |= FANCY;
  }
  if ($fontname =~ /\bS(mall)?C(aps)\b/i) {
    if ($fontname !~ /^noto/i) {
      $resftype |= SMALLCAP|SERIF;
    }
  }
  # Now check for fonts without a hint in font name
  if ($fontname =~ /^([a-z])/i) {
    my $key = lc($1);
    # check the mutual exclusive first
    for my $rFonts (\%fraktFonts, \%scriptFonts, \%sansFonts, \%serifFonts) {
      if (defined($rFonts->{$key})) {
        if ($fontname =~ $rFonts->{$key}) {
          $resftype |= $rFonts->{"value"};
          last;
        }
      }
    }
    for my $rFonts (\%fancyFonts, \%initialFonts, \%symbolFonts, \%smallcapFonts) {
      if (defined($rFonts->{$key})) {
        if ($fontname =~ $rFonts->{$key}) {
          $resftype |= $rFonts->{"value"};
        }
      }
    }
  }
  if ("$fontname" =~ /^bpg/i) {
    if ("$fontname" =~ /bpg (courier gpl|elite|serif)/i) {
      $resftype |= SERIF;    # Serif
    }
    else {
      $resftype |= SANS; # Sans Serif
    }
  }
  elsif ("$fontname" =~ /^dustismo/i) {
    if ("$fontname" =~ /^dustismo roman/i) {
      $resftype |= SERIF;    # Serif
    }
    else {
      $resftype |= SANS; # Sans Serif
    }
  }
  elsif ("$fontname" =~ /^go\b/i) {
    if ("$fontname" =~ /^go mono/i) {
      $resftype |= SERIF;    # Serif
    }
    else {
      $resftype |= SANS; # Sans Serif
    }
  }
  # Create the string
  my @ft = ();
  if ($resftype == 0) {
    $resftype = $ftypes{default};
  }
  else {
    # fonts SANS, SERIF, SCRIPT and FRAKTUR are mutualy exclusive
    if ($resftype & FRAKTUR) {
      $resftype &= ~(SANS|SERIF|SCRIPT);
    }
    elsif ($resftype & SCRIPT) {
      $resftype &= ~(SANS|SERIF);
    }
    elsif ($resftype & SANS) {
      $resftype &= ~SERIF;
    }
  }
  for (my $i = 1; $i < 513; $i *= 2) {
    if ($resftype & $i) {
      push(@ft, $ftypes{$i});
    }
  }
  return(join(',', @ft));
}

sub getweight($$)
{
  my ($fontname, $style) = @_;
  my $result = undef;
  for my $info ($style, $fontname) {
    for my $key (keys %weights) {
      next if ($key !~ /^\d+$/);
      my $val = $weights{$key};
      if ($info =~ /\b$val\b/i) {
        return($val);
      }
    }
  }
  return($result);
}

sub getwidth($$)
{
  my ($fontname, $style) = @_;
  my $result = undef;
  for my $key (keys %widths) {
    next if ($key !~ /^\d+$/);
    for my $info ($style, $fontname) {
      if ($info =~ /\b$widths{$key}\b/i) {
        return($widths{$key});
      }
      if ($info =~ /\bRegular\b/) {
        if (!defined($result)) {
          $result = $widths{100};
        }
      }
    }
  }
  return($result);
}

sub getslant($$)
{
  my ($fontname, $style) = @_;
  for my $key (keys %slants) {
    next if ($key !~ /^\d+$/);
    if ($style =~ /\b$slants{$key}\b/i) {
      return($slants{$key});
    }
  }
  return(undef);
}

sub getspacing($$)
{
  my ($fontname, $style) = @_;
  for my $key (keys %spacings) {
    next if ($key !~ /^\d+$/);
    if ($style =~ /\b$spacings{$key}\b/i) {
      return($spacings{$key});
    }
  }
  if ("$fontname $style" =~ /(\bmono\b|luximono|typewriter|cursor|fixed)\b/i) {
    return($spacings{100}); # Mono
  }
  else {
    return(undef);
  }
}

sub ismathfont($$)
{
  my ($fontname, $rCapability) = @_;

  return 1 if ($fontname =~ /math/i);
  for my $cap (@{$rCapability}) {
    return 1 if ($cap eq "math");
  }
  return 0;
}

sub getproperties($$$$)
{
  my ($l, $fontname, $style, $rerrors) = @_;
  my $newstyle = &correctstyle($style);
  my $newfam = &correctstyle($fontname);
  my @properties = ();

  for my $txt (qw(ftype weight width slant spacing)) {
    my ($map, $rget);
    eval("\$map = " . '\%' . $txt . 's');
    eval('$rget = \&' . "get$txt");
    my $val2 = getsinglevalue($l, $txt, $map);
    my $val1 = $rget->($newfam, $newstyle);
    my $val;
    if (defined($val2) && defined($val1) && ($val2 ne $val1)) {
      if (($txt =~/^(weight|slant)$/) && ($newstyle =~ /$val1/i)){
        # style overrides weight and slant
        push(@{$rerrors}, "Fontname($fontname),Style($style): Values for $txt ($val1 != $val2) differ, pick $val1 from style");
        $val = $val1;
      }
      elsif ($newfam =~ /$val1/) {
        push(@{$rerrors}, "Fontname($fontname),Style($style): Values for $txt ($val1 != $val2) differ, pick $val1 from fontname");
        $val = $val1;
      }
      else {
        push(@{$rerrors}, "Fontname($fontname),Style($style): Values for $txt ($val1 != $val2) differ, pick $val2 from $txt-property");
        $val = $val2;
      }
    }
    elsif (! defined($val2)) {
      $val = $val1;
    }
    else {
      $val = $val2;
    }
    if (defined($val)) {
      push(@properties, &addTxt($txt,$val));
    }
    else {
      if (defined($map->{"default"})) {
        push(@properties, &addTxt($txt,$map->{"default"}));
      }
      else {
        push(@{$rerrors}, "Undefined value for $txt");
      }
    }
  }
  return(join(' ', @properties));
}

sub correctstyle($)
{
  my ($style) = @_;
  $style =~ s/^\s*\d+\s*//;
  $style =~ s/\s*\d+$//;
  $style =~ s/italic/ Italic/i;
  $style =~ s/oblique/ Oblique/i;
  $style =~ s/[\-_]/ /g;
  $style =~ s/\breg\b/Regular/i;
  $style =~ s/\bregita(lic)?\b/Regular Italic/i;
  $style =~ s/\bregobl(ique)?\b/Regular Oblique/i;
  $style =~ s/medium/Medium /i;
  $style =~ s/\bmedita(lic)?\b/Medium Italic/i;
  $style =~ s/\bmedobl(ique)?\b/Medium Oblique/i;
  $style =~ s/\bmed\b/Medium /i;
  $style =~ s/\bex(pd|t)\b/Expanded/i;
  $style =~ s/semi ?cond(ensed)?/SemiCondensed/i;
  $style =~ s/[sd]emi ?(bold|bd|bol)/SemiBold/i;
  $style =~ s/semi ?(expanded|extended|expd)/SemiExpanded/i;
  $style =~ s/[sd]emi ?light/SemiLight/i;
  $style =~ s/\b[sd]emi\b/SemiBold/i;
  $style =~ s/ultra ?(expanded|extended|expd)/UltraExpanded/i;
  $style =~ s/light/Light /i;
  $style =~ s/\blt\b/Light /i;
  $style =~ s/(ultra|extra) ?(light|lt)/ExtraLight /i;
  $style =~ s/\bheavy\b/Extrabold/i;
  $style =~ s/\bhairline\b/Extralight/i;
  $style =~ s/\bcond\b/Condensed/i;
  $style =~ s/(roman)?slanted/ Italic/i;
  $style =~ s/\bslant\b/Italic/i;
  $style =~ s/\b(SC|Small(caps(alt)?)?)\b/SmallCaps/i;
  $style =~ s/w3 mono/Dual/i;
  $style =~ s/Regul[ea]r/Regular/i;
  $style =~ s/Megablack/ExtraBlack/i;
  $style =~ s/  +/ /g;
  $style =~ s/ +$//;
  return($style);
}

# return list of unicode values of the input string
#Allow input of intervals (e.g. 'a-z')
sub decimalUnicode($)
{
  my ($a) = @_;
  my @res = ();
  # Convert to unicode chars first
  while ($a =~ /^(.*)u\+(0?x[\da-f]+|\d+)(.*)$/i) {
    my ($prev, $d, $post) = ($1, $2, $3);
    if ($d =~ /^0?x(.+)$/) {
      $d = hex($1);
    }
    my $chr = encode('utf-8', chr($d));
    $a = $prev . $chr . $post;
  }
  # $a is now a string of unicode chars
  my $u = decode('utf-8', $a);
  my @a = split(//, $u);
  my $interval = 0;
  my $start = undef;
  for my $x (@a) {
    if ($x eq '-') {    # Interval
      $interval = 1;
      next;
    }
    if ($interval && defined($start)) {
      if (ord($x) < $start) {
        for (my $i = $start - 1; $i >= ord($x); $i--) {
          push(@res, $i);
        }
      }
      else {
        for (my $i = $start + 1; $i <= ord($x); $i++) {
          push(@res, $i);
        }
      }
      $start = undef;
    }
    else {
      $start = ord($x);
      push(@res, $start);
    }
    $interval = 0;
  }
  return(@res);
}


# check if the glyph-values in interval @{$ri} are contained
# in one of the (sorted) intervals
sub contains($$)
{
  # ok if
  # ...re0..........re1...
  # ......start..end......
  my ($ri, $rList) = @_;
  my $start = $ri->[0];
  my $end = $ri->[1];

  for my $re (@{$rList}) {
    next if ($re->[1] < $start);
    # now we found a possible matching interval
    return 1 if (($start >= $re->[0]) && ($end <= $re->[1]));
    return 0;
  }
  return 0;
}

sub sprintIntervalls($)
{
  my ($rList) = @_;
  my @out = ();
  for my $rE (@{$rList}) {
    if ($rE->[0] != $rE->[1]) {
      push(@out, $rE->[0] . '-' . $rE->[1]);
    }
    else {
      push(@out, $rE->[0]);
    }
  }
  return join(',', @out);
}

sub buildFontName($$)
{
  my ($family, $style) = @_;

  my $result = "";
  $style =~ s/\\040//;
  $family =~ s/\\040/\-/;
  $family =~ s/\bcond\b/Condensed/i;
  $family =~ s/\bblk\b/Black/i;
  $family =~ s/\bsembd\b/SemiBold/i;
  $family =~ s/\bsemcond\b/SemiCondensed/i;
  $family =~ s/\bextcond\b/ExtraCondensed/i;
  $family =~ s/\bextbd\b/ExtraBold/i;
  $family =~ s/\bextlt\b/ExtraLight/i;
  $family =~ s/\bmed\b/Medium/i;
  my $prefix = "";
  # Split the family name at the border lowercase <-> uppercase characters
  # except if family starts with "TeX" or "DejaVu", etc
  if ($family =~ s/^((La|cw|PL)?TeX|LyX|DejaVu|MarVoSym|PL)//) {
    $prefix = $1;
    if ($family =~ /^[A-Z]/) {
      $prefix .= " ";
    }
  }

  if ($family =~ /^([A-Z]*[a-z]+)([A-Z]\w+)\b(.*)$/) {
    $family = $1 . splitatlU($2) . $3;
  }
  $family =~ s/^Ant Polt\b/Antykwa Poltawskiego/;
  $family =~ s/\b(Semi|Extra) (Bold|Condensed|Expanded|Light)\b/$1$2/;
  $family = $prefix . $family;
  my @style = ();
  if ($style =~ s/^((La|cw|PL)?TeX|LyX|DejaVu|MarVoSym|PL)//) {
    push(@style, $1);
  }

  push(@style, &splitStyle($style));
  for my $st (@style) {
    $st = ucfirst($st);
    if ($family !~ s/$st/$st/i) {
      $family .= " $st";
    }
    else {
      # check if $st in $family starts with ' '
      $family =~ s/(\w)$st/$1 $st/i;
    }
  }
  $result = $family;
  return($result, join(' ', @style));
}

# split text at change from lower case to upper case
sub splitatlU($)
{
  my ($txt) = @_;
  if ($txt =~ /^([A-Z]+[a-z]*)(.*)$/) {
    if (defined($mapShortcuts{$1})) {
      return(" " . $mapShortcuts{$1} . splitatlU($2));
    }
    else {
      return(" $1" . splitatlU($2));
    }
  }
  return($txt);
}

sub splitStyle($)
{
  my @in = split(/[- ]/, $_[0]);
  my @result = ();
  my $prefix = "";
  for my $en (@in) {
    while ($en =~ s/^([A-Z][a-z]+)//) {
      my $found = $1;
      if ($found =~ /^(Semi|Extra)$/) {
        $prefix = $found;
        next;
      }
      elsif (defined($mapShortcuts{$found})) {
        $found = $mapShortcuts{$found};
      }
      push(@result, "$prefix$found");
      $prefix = "";
    }
    if ($en ne "") {
      push(@result, "$prefix$en");
      $prefix = "";
    }
  }
  return(@result);
}
