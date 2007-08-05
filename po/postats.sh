#! /bin/sh

# file postats.sh
#
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author: Michael Gerz, michael.gerz@teststep.org
#
# This script extracts some information from the po file headers (last
# translator, revision date), generates the corresponding gmo files
# to retrieve the number of translated/fuzzy/untranslated messages,
# and generates a PHP web page.
#
# Invocation:
#    postats.sh po_files > "pathToWebPages"/i18n.php

# modifiy this when you change version
# Note that an empty lyx_branch variable (ie svn trunk)
# will "do the right thing".
lyx_version=1.5.2svn
lyx_branch=BRANCH_1_5_X


# GNU sed and grep have real problems dealing with 8-bit characters
# in UTF-8 encoded environments.
unset LANG
LANGUAGE=C

warning () {
	echo $* 1>&2
}


error () {
	warning $*
	exit 1
}


# $1 is a string like
# '588 translated messages, 1248 fuzzy translations, 2 untranslated messages.'
# Any one of these substrings may not appear if the associated number is 0.
#
# $2 is the word following the number to be extracted,
# ie, 'translated', 'fuzzy', or 'untranslated'.
#
# extract_number fills var $number with this number, or sets it to zero if the
# word is not found in the string.
extract_number () {
	test $# -eq 2 || error 'extract_number expects 2 args'

	number=0
	echo $1 | grep $2 >/dev/null || return
	# It /is/ safe to use 'Z' as a delimiter here.
	number=`echo $1 | sed "s/\([0-9]*\)[ ]*$2/Z\1Z/" | cut -d 'Z' -f 2`
}


# $template is used by run_msgfmt, below, to fill $output. The function extracts
# the appropriate values from the data.
template="array ( 'langcode' => 'LC',
\"msg_tr\" => TR, \"msg_fu\" => FU, \"msg_nt\" => NT,
\"translator\" => \"AUTHOR\", \"email\" => \"EMAIL\",
\"date\" => \"DATE\" )"
readonly template


# $1 is the name of the po file.
#
# The function runs msgfmt on it and fills var $output.
# All other variables created in the function are unset on exit.
run_msgfmt () {
	test $# -eq 1 || error 'run_msgfmt expects 1 arg'

	output=
	test -f $1 || {
		warning "File $1 does not exist"
		return
	}

	origdir=`pwd`
	dir=`dirname $1`
	pofile=`basename $1`
	gmofile=`echo $pofile | sed 's/po$/gmo/'`
	test $pofile != '' -a $pofile != $gmofile || {
		warning "File $1 is not a po file"
		unset origdir dir pofile gmofile
		return
	}

	cd $dir
	unset dir

	langcode=`echo $pofile | sed 's/\.po$//'`

	# Searching for a string of the form
	# '"PO-Revision-Date: 2003-01-18 03:00+0100\n"'
	date=`grep 'Revision-Date' $pofile | sed 's/  */ /g' | cut -d ' ' -f 2`

	# Searching for a string of the form
	# '"Last-Translator: Michael Gerz <Michael.Gerz@teststep.org>\n"'
	translator=
	email=
	input=`grep "Last-Translator" $pofile` && {
		input=`echo $input | sed 's/  */ /g' | cut -d ' ' -f 2-`

		translator=`echo $input | cut -d '<' -f 1 | sed 's/ *$//'`
		email=`echo $input | cut -d '<' -f 2 | cut -d '>' -f 1 | sed -e 's/@/ () /' -e 's/\./ ! /g'`
	}
	unset input

	# Does $translator contain 8-bit characters?
	TAB='	'
	echo $translator | grep "[^${TAB} -~]" >/dev/null && {
		# If so, grab the encoding from the po file.
		charset=`sed -n '/Content-Type/{s/.*charset=//;s/\\\\n" *$//p;q}' $pofile`
		# Use recode to generate HTML character codes for the 8-bit
		# characters.
		translator=`echo $translator | recode "${charset}..h4"` || exit 1
		# The ampersands in the $translator entries will mess things
		# up unless we escape 'em.
		translator=`echo $translator | sed 's/&/\\\&/g'`
	}

	# Run msgfmt on the pofile, filling $message with the raw info.
	message=`$msgfmt --statistics -o $gmofile $pofile 2>&1 | grep "^[1-9]"` || {
		warning "Unable to run msgfmt successfully on file $1"
		cg $origdir
		unset origdir pofile gmofile
		return
	}
	unset pofile gmofile

	extract_number "$message" 'translated'
	translated=$number

	extract_number "$message" 'fuzzy'
	fuzzy=$number

	extract_number "$message" 'untranslated'
	untranslated=$number
	unset message number

	output=`echo "$template" | sed "s/LC/$langcode/; \
		s/TR/$translated/; s/FU/$fuzzy/; s/NT/$untranslated/; \
		s/AUTHOR/$translator/; s/EMAIL/$email/; s/DATE/$date/"`

	unset langcode date translator email untranslated fuzzy translated
	cd $origdir
	unset origdir
}


# The head of the generated php file.
dump_head () {
cat <<EOF
<?
	// What's the title of the page?
	\$title = "LyX i18n";
	// What's the short name of the page in the navigation bar?
	\$item="i18n";
	// Who is the author?
	\$author="Michael Gerz";
	// Full name of the file (relative path from LyX home page -- i.e., it should
	// be "foo.php" or "bar/foo.php")
	\$file_full="devel/i18n.php";

	include("start.php");

	error_reporting(E_ALL);
?>

<?
\$podata = array (
EOF
}


# The foot of the generated php file.
dump_tail () {

test "$lyx_branch" = "" && {
	branch_tag="trunk"
} || {
	branch_tag="branches/$lyx_branch"
}

cat <<EOF
<?
\$lang = array(
	'bg' => 'Bulgarian',
	'ca' => 'Catalan',
	'cs' => 'Czech',
	'da' => 'Danish',
	'de' => 'German',
	'es' => 'Spanish',
	'eu' => 'Basque',
	'fi' => 'Finnish',
	'fr' => 'French',
	'gl' => 'Galician',
	'he' => 'Hebrew',
	'hu' => 'Hungarian',
	'it' => 'Italian',
	'ja' => 'Japanese',
	'ko' => 'Korean',
	'nl' => 'Dutch',
	'nn' => 'Nynorsk',
	'nb' => 'Norwegian',
	'pl' => 'Polish',
	'pt' => 'Portuguese',
	'ro' => 'Romanian',
	'ru' => 'Russian',
	'sk' => 'Slovak',
	'sl' => 'Slovenian',
	'sv' => 'Swedish',
	'tr' => 'Turkish',
	'wa' => 'Walloon',
	'zh_CN' => 'Simplified Chinese',
	'zh_TW' => 'Traditional Chinese'
);

\$noOfMsg = \$podata[0]['msg_tr'] + \$podata[0]['msg_fu'] + \$podata[0]['msg_nt'];

function cmp (\$a, \$b) {
	if (\$a['msg_tr'] == \$b['msg_tr']) {
		return 0;
	}
	return (\$a['msg_tr'] > \$b['msg_tr']) ? -1 : 1;
}

usort (\$podata, "cmp");
?>

<p>
	The following table details the current state of the translations of the
	LyX GUI for the LyX development branch (currently $lyx_version).
	Unfortunately, only a few languages are well-supported. The LyX team may,
	therefore, decide to exclude some of the translations from a formal
	release in order not to confuse the user with a strongly mixed-language
	interface.
</p>
<p>
	Explanation:
</p>
<ul>
	<li><i>Translated:</i> The number of translated messages</li>
	<li><i>Fuzzy:</i> The number of fuzzy messages; these are not considered
	    for LyX output but solely serve as a hint for the translators</li>
	<li><i>Untranslated:</i> The number of untranslated messages; the
	    default language (i.e., English) will be used in the LyX outputs</li>
</ul>
<table class="center" frame="box" rules="all" border="2" cellpadding="5">
<thead>
	<tr>
		<td>Language</td>
		<td>Translated</td>
		<td>Fuzzy</td>
		<td>Untranslated</td>
		<td>Revision Date</td>
		<td>Translator</td>
	</tr>
</thead>
<tbody>
<?
while (list(\$foo,\$info) = each(\$podata)) {
	print "<tr>";

	if ( \$info['msg_tr'] > \$noOfMsg * 2 / 3 ) {
		\$style="style='background:#009900'";
	} else if ( \$info['msg_tr'] > \$noOfMsg / 2 ) {
		\$style="style='background:#AAAA00'";
	} else {
		\$style="style='background:#AA3333'";
	}
	print "<td \$style>" ;

	print "<a href=\"http://www.lyx.org/trac/browser/lyx-devel/$branch_tag/po/" . \$info['langcode'] . ".po?format=raw\">" . \$lang[\$info['langcode']] . "</a></td>";

	print "<td \$style align=\"right\">" . \$info['msg_tr'] . "</td>";

	print "<td \$style align=\"right\">";
	if (isset(\$info['msg_fu'])) {
		print \$info['msg_fu'];
	} else {
		print "0";
	}
	print "</td>";

	print "<td \$style align=\"right\">";
	if (isset(\$info['msg_nt'])) {
		print \$info['msg_nt'];
	} else {
		print "0";
	}
	print "</td>";

	print "<td \$style align=\"center\">" . \$info['date'] . "</td>";

	print "<td \$style>";
	if (\$info['email'] == "") {
		print \$info['translator'];
	} else {
		print "<a href=\"mailto:" . \$info['email'] . "\">" .
			\$info['translator'] . "</a>";
	}
	print "</td>";

	print "</tr>\n";
}
?>
</tbody>
</table>
<?
include("end.php");
?>
EOF
}


# The main body of the script
msgfmt=`which msgfmt`
test $msgfmt != '' || error "Unable to find 'msgfmt'. Cannot proceed."

dump_head

while [ $# -ne 0 ]
do
	run_msgfmt $1
	shift
	if [ $# -eq 0 ]; then
		echo "${output});"
		echo '?>'
	else
		echo "${output},"
		echo
	fi
done

dump_tail
# The end
