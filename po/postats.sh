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


# *** The PHP header ***

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
EOF

# *** The po file analysis ***

echo "<?"
echo "\$podata = array ( "
first=true
for x
do
	if [ $first = true ] ; then
		first=false ;
	else
		echo ", " ;
	fi
	y=`basename $x .po`
	echo "array ( 'langcode' => '$y', "
	touch $x
	make 2>&1 $y.gmo | grep "^[1-9]" |
		sed -e 's/\([0-9]*\) translated m[a-z]*[.,]/"msg_tr" => \1,/' |
		sed -e 's/\([0-9]*\) fuzzy t[a-z]*[.,]/"msg_fu" => \1,/' |
		sed -e 's/\([0-9]*\) untranslated m[a-z]*./"msg_nt" => \1,/'
	# Format: "Last-Translator: Michael Schmitt <Michael.Schmitt@teststep.org>\n"
	grep "Last-Translator" $x |
		sed -e 's/"Last-Translator: \(.*\)\( *\)<\(.*\)>\\n"/"translator" => "\1", "email" => "\3", /'
	# Format: "PO-Revision-Date: 2003-01-18 03:00+0100\n"
	grep "PO-Revision-Date" $x |
		sed -e 's/"PO-Revision-Date: \(.*\) .*/"date" => "\1" )/'
done
echo ");"
echo "?>"

# *** The PHP core part ***

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
		<ul>
		<li><i>Translated:</i> The number of translated messages</li>
		<li><i>Fuzzy:</i> The number of fuzzy messages; these are not considered
		    for LyX output but solely serve as a hint for the translators</li>
		<li><i>Untranslated:</i> The number of untranslated messages; the
		    default language (i.e., English) will be used in the LyX outputs</li>
		</ul>
	</p>
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
EOF

cat <<EOF
	<?
		include("end.php3");
	?>
EOF
