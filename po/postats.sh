#! /bin/sh

# file postats.sh
#
# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# author: Michael Schmitt, michael.schmitt@teststep.org
#
# This script extracts some information from the po file headers (last
# translator, revision date), generates the corresponding gmo files
# to retrieve the number of translated/fuzzy/untranslated messages,
# and generates a PHP web page.
#
# Invocation:
#    postats.sh po_files > "pathToWebPages"/i18n.php3

error () {
	while [ $# -ne 0 ]
	do
		echo $1
		shift
	done
	exit 1
}

# $1 is a string like
# '588 translated messages, 1248 fuzzy translations, 2 untranslated messages.'
# $2 is the word following the number to be extracted,
# ie, 'translated', 'fuzzy', or 'untranslated'.
# Fills var $number with this number, or sets it to zero if the
# word is not found in the string.
extract_number () {
	test $# -eq 2 || error 'extract_number expects 2 args'

	number=0
	echo $1 | grep $2 >/dev/null || return
	number=`echo $1 | sed "s/\([0-9]*\)[ ]*$2.*/Z\1/" | cut -d 'Z' -f 2`
}


# Takes the name of the .po file and the .gmo file as input
# Regenerates the .gmo file and manipulates the output to stderr to 
# fill var $output
run_msgfmt () {
	test $# -eq 2 || error 'run_msgfmt expects 2 args'

	rm -f $gmofile
	message=`$msgfmt --statistics -o $gmofile $1 2>&1 | grep "^[1-9]"`
	#message=`make $gmofile 2>&1 | grep "^[1-9]"`
	
	extract_number "$message" translated
	output='"msg_tr" => '$number
	extract_number "$message" fuzzy
	output=$output', "msg_fu" => '$number
	extract_number "$message" untranslated
	output=$output', "msg_nt" => '$number
}


# Passed the name of the .po file
dump_stats () {
	test $# -eq 1 || error 'dump_stats expects 1 arg'

	file=$1
	test -f $file || {
		echo "File $file does not exist"
		return
	}

	dir=`dirname $file`
	pofile=`basename $file`
	gmofile=`echo $pofile | sed 's/po$/gmo/'`
	test $pofile != '' -a $pofile != $gmofile || {
		echo "File $file is not a po file"
		return
	}

	(
	cd $dir
	# $output is a string of the form
	# '"msg_tr" => A, "msg_fu" => B, "msg_nt" => C'
	# where A, B, C are extracted from the process of generating the .gmo
	# file
	run_msgfmt $pofile $gmofile

	# earching for a string of the form
	# '"Last-Translator: Michael Schmitt <Michael.Schmitt@teststep.org>\n"'
	translator='"translator" => ""'
	email='"email" => ""'

	input=`grep "Last-Translator" $pofile` && {
		input=`echo $input | sed 's/  */ /g' | cut -d ' ' -f 2-`

		translator=`echo $input | cut -d '<' -f 1 | sed 's/ *$//'`
		translator='"translator" => "'$translator'"'

		email=`echo $input | cut -d '<' -f 2 | cut -d '>' -f 1`
		email='"email" => "'$email'"'
	}
	
	# Searching for a string of the form
	# '"PO-Revision-Date: 2003-01-18 03:00+0100\n"'
	date=`grep 'Revision-Date' $pofile | sed 's/  */ /g' | cut -d ' ' -f 2`
	date='"date" => "'$date'"'

	langcode=`echo $pofile | sed 's/\.po//'`
	echo "array ( 'langcode' => '"$langcode"',"
	echo ${output},
	echo ${translator}, ${email},
	echo "${date} )"
	)
}

dump_head () {
cat <<EOF
<?
	// What's the title of the page?
	\$title = "LyX i18n";
	// What's the short name of the page in the navigation bar?
	\$item="i18n";
	// Who is the author?
	\$author="Michael Schmitt";
	// Full name of the file (relative path from LyX home page -- i.e., it should
	// be "foo.php3" or "bar/foo.php3")
	\$file_full="i18n.php3";

	include("start.php3");

	error_reporting(E_ALL);
?>

<?
\$podata = array (
EOF
}


dump_tail () {
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
				'he' => 'Hebrew',
				'hu' => 'Hungarian',
				'it' => 'Italian',
				'nl' => 'Dutch',
				'no' => 'Norwegian',
				'pl' => 'Polish',
				'pt' => 'Portuguese',
				'ro' => 'Romanian',
				'ru' => 'Russian',
				'sk' => 'Slovak',
				'sl' => 'Slovenian',
				'sv' => 'Swedish',
				'tr' => 'Turkish',
				'wa' => 'Wallon'
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
		The following table lists all translations available with the number of messages
		given for the LyX main development branch (currently 1.3.0cvs).
		Unfortunately, only a few languages are well-supported.
		For every release, the LyX development team may decide to exclude some of the
		translations from the distribution in order not to confuse the user by a strongly
		mixed-language interface.
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

					print "<td \$style>" . \$lang[\$info['langcode']] . "</td>";

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
		include("end.php3");
	?>
EOF
}


# The main body of the script
msgfmt=`type msgfmt | sed 's/msgfmt is *//'`
test msgfmt != '' || error "Unable to find 'msgfmt'"

dump_head

while [ $# -ne 0 ]
do
	dump_stats $1
	shift
	if [ $# -eq 0 ]; then
	    echo ');'
	    echo '?>'
	else
	    echo ','
	fi
done

dump_tail

