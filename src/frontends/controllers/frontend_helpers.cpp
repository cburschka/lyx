/**
 * \file frontend_helpers.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "frontend_helpers.h"

#include "Buffer.h"
#include "BufferParams.h"
#include "Color.h"
#include "debug.h"
#include "gettext.h"
#include "Language.h"
#include "Length.h"

#include "frontends/FileDialog.h"
#include "frontends/alert.h"

#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/Package.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "support/lyxalgo.h"
#include "support/os.h"
#include "support/Package.h"
#include "support/Path.h"
#include "support/Systemcall.h"

#include <boost/cregex.hpp>
#include <boost/regex.hpp>

#include <algorithm>
#include <fstream>

using std::string;
using std::vector;
using std::pair;
using std::endl;

namespace lyx {

using support::ascii_lowercase;
using support::bformat;
using support::compare_ascii_no_case;
using support::contains;
using support::getVectorFromString;
using support::ltrim;
using support::prefixIs;
using support::rtrim;
using support::split;
using support::subst;
using support::token;
using support::trim;

namespace biblio {

namespace {

vector<string> const init_possible_cite_commands()
{
	char const * const pos[] = {
		"cite",
		"citet", "citep", "citealt", "citealp",
		"citeauthor", "citeyear", "citeyearpar",
		"citet*", "citep*", "citealt*", "citealp*", "citeauthor*",
		"Citet",  "Citep",  "Citealt",  "Citealp",  "Citeauthor",
		"Citet*", "Citep*", "Citealt*", "Citealp*", "Citeauthor*",
		"fullcite",
		"footcite", "footcitet", "footcitep", "footcitealt",
		"footcitealp", "footciteauthor", "footciteyear",
		"footciteyearpar",
		"citefield",
		"citetitle",
		"cite*"
	};
	size_t const size_pos = sizeof(pos) / sizeof(pos[0]);

	return vector<string>(pos, pos + size_pos);
}


vector<string> const & possible_cite_commands()
{
	static vector<string> const pos = init_possible_cite_commands();
	return pos;
}


bool is_possible_cite_command(string const & input)
{
	vector<string> const & possibles = possible_cite_commands();
	vector<string>::const_iterator const end = possibles.end();
	return std::find(possibles.begin(), end, input) != end;
}


string const default_cite_command(CiteEngine engine)
{
	string str;
	switch (engine) {
	case ENGINE_BASIC:
		str = "cite";
		break;
	case ENGINE_NATBIB_AUTHORYEAR:
		str = "citet";
		break;
	case ENGINE_NATBIB_NUMERICAL:
		str = "citep";
		break;
	case ENGINE_JURABIB:
		str = "cite";
		break;
	}
	return str;
}


static const docstring TheBibliographyRef(from_ascii("TheBibliographyRef"));

} // namespace anon


string const asValidLatexCommand(string const & input,
				 CiteEngine const engine)
{
	string const default_str = default_cite_command(engine);
	if (!is_possible_cite_command(input))
		return default_str;

	string output;
	switch (engine) {
	case ENGINE_BASIC:
		output = default_str;
		break;

	case ENGINE_NATBIB_AUTHORYEAR:
	case ENGINE_NATBIB_NUMERICAL:
		if (input == "cite" || input == "citefield" ||
		    input == "citetitle" || input == "cite*")
			output = default_str;
		else if (prefixIs(input, "foot"))
			output = input.substr(4);
		else
			output = input;
		break;

	case ENGINE_JURABIB: {
		// Jurabib does not support the 'uppercase' natbib style.
		if (input[0] == 'C')
			output = string(1, 'c') + input.substr(1);
		else
			output = input;

		// Jurabib does not support the 'full' natbib style.
		string::size_type const n = output.size() - 1;
		if (output != "cite*" && output[n] == '*')
			output = output.substr(0, n);

		break;
	}
	}

	return output;
}


docstring const familyName(docstring const & name)
{
	if (name.empty())
		return docstring();

	// Very simple parser
	docstring fname = name;

	// possible authorname combinations are:
	// "Surname, FirstName"
	// "Surname, F."
	// "FirstName Surname"
	// "F. Surname"
	docstring::size_type idx = fname.find(',');
	if (idx != docstring::npos)
		return ltrim(fname.substr(0, idx));
	idx = fname.rfind('.');
	if (idx != docstring::npos && idx + 1 < fname.size())
		fname = ltrim(fname.substr(idx + 1));
	// test if we have a LaTeX Space in front
	if (fname[0] == '\\')
		return fname.substr(2);

	return rtrim(fname);
}


docstring const getAbbreviatedAuthor(InfoMap const & map, string const & key)
{
	BOOST_ASSERT(!map.empty());

	InfoMap::const_iterator it = map.find(key);
	if (it == map.end())
		return docstring();
	docstring const & data = it->second;

	// Is the entry a BibTeX one or one from lyx-layout "bibliography"?
	docstring::size_type const pos = data.find(TheBibliographyRef);
	if (pos != docstring::npos) {
		if (pos <= 2) {
			return docstring();
		}

		docstring const opt = trim(data.substr(0, pos - 1));
		if (opt.empty())
			return docstring();

		docstring authors;
		split(opt, authors, '(');
		return authors;
	}

	docstring author = parseBibTeX(data, "author");

	if (author.empty())
		author = parseBibTeX(data, "editor");

	if (author.empty()) {
		author = parseBibTeX(data, "key");
		if (author.empty())
			// FIXME UNICODE
			return from_utf8(key);
		return author;
	}

	vector<docstring> const authors = getVectorFromString(author, from_ascii(" and "));
	if (authors.empty())
		return author;

	if (authors.size() == 2)
		return bformat(_("%1$s and %2$s"),
			familyName(authors[0]), familyName(authors[1]));

	if (authors.size() > 2)
		return bformat(_("%1$s et al."), familyName(authors[0]));

	return familyName(authors[0]);
}


docstring const getYear(InfoMap const & map, string const & key)
{
	BOOST_ASSERT(!map.empty());

	InfoMap::const_iterator it = map.find(key);
	if (it == map.end())
		return docstring();
	docstring const & data = it->second;

	// Is the entry a BibTeX one or one from lyx-layout "bibliography"?
	docstring::size_type const pos = data.find(TheBibliographyRef);
	if (pos != docstring::npos) {
		if (pos <= 2) {
			return docstring();
		}

		docstring const opt =
			trim(data.substr(0, pos - 1));
		if (opt.empty())
			return docstring();

		docstring authors;
		docstring const tmp = split(opt, authors, '(');
		docstring year;
		split(tmp, year, ')');
		return year;

	}

	docstring year = parseBibTeX(data, "year");
	if (year.empty())
		year = _("No year");

	return year;
}


namespace {

// A functor for use with std::sort, leading to case insensitive sorting
class compareNoCase: public std::binary_function<string, string, bool>
{
public:
	bool operator()(string const & s1, string const & s2) const {
		return compare_ascii_no_case(s1, s2) < 0;
	}
};

} // namespace anon


vector<string> const getKeys(InfoMap const & map)
{
	vector<string> bibkeys;
	InfoMap::const_iterator it  = map.begin();
	InfoMap::const_iterator end = map.end();
	for (; it != end; ++it) {
		bibkeys.push_back(it->first);
	}

	std::sort(bibkeys.begin(), bibkeys.end(), compareNoCase());
	return bibkeys;
}


docstring const getInfo(InfoMap const & map, string const & key)
{
	BOOST_ASSERT(!map.empty());

	InfoMap::const_iterator it = map.find(key);
	if (it == map.end())
		return docstring();
	docstring const & data = it->second;

	// is the entry a BibTeX one or one from lyx-layout "bibliography"?
	docstring::size_type const pos = data.find(TheBibliographyRef);
	if (pos != docstring::npos) {
		docstring::size_type const pos2 = pos + TheBibliographyRef.size();
		docstring const info = trim(data.substr(pos2));
		return info;
	}

	// Search for all possible "required" keys
	docstring author = parseBibTeX(data, "author");
	if (author.empty())
		author = parseBibTeX(data, "editor");

	docstring year      = parseBibTeX(data, "year");
	docstring title     = parseBibTeX(data, "title");
	docstring booktitle = parseBibTeX(data, "booktitle");
	docstring chapter   = parseBibTeX(data, "chapter");
	docstring number    = parseBibTeX(data, "number");
	docstring volume    = parseBibTeX(data, "volume");
	docstring pages     = parseBibTeX(data, "pages");
	docstring annote    = parseBibTeX(data, "annote");
	docstring media     = parseBibTeX(data, "journal");
	if (media.empty())
		media = parseBibTeX(data, "publisher");
	if (media.empty())
		media = parseBibTeX(data, "school");
	if (media.empty())
		media = parseBibTeX(data, "institution");

	odocstringstream result;
	if (!author.empty())
		result << author << ", ";
	if (!title.empty())
		result << title;
	if (!booktitle.empty())
		result << ", in " << booktitle;
	if (!chapter.empty())
		result << ", Ch. " << chapter;
	if (!media.empty())
		result << ", " << media;
	if (!volume.empty())
		result << ", vol. " << volume;
	if (!number.empty())
		result << ", no. " << number;
	if (!pages.empty())
		result << ", pp. " << pages;
	if (!year.empty())
		result << ", " << year;
	if (!annote.empty())
		result << "\n\n" << annote;

	docstring const result_str = rtrim(result.str());
	if (!result_str.empty())
		return result_str;

	// This should never happen (or at least be very unusual!)
	return data;
}


namespace {

// Escape special chars.
// All characters are literals except: '.|*?+(){}[]^$\'
// These characters are literals when preceded by a "\", which is done here
// @todo: This function should be moved to support, and then the test in tests
//        should be moved there as well.
string const escape_special_chars(string const & expr)
{
	// Search for all chars '.|*?+(){}[^$]\'
	// Note that '[' and '\' must be escaped.
	// This is a limitation of boost::regex, but all other chars in BREs
	// are assumed literal.
	boost::regex reg("[].|*?+(){}^$\\[\\\\]");

	// $& is a perl-like expression that expands to all
	// of the current match
	// The '$' must be prefixed with the escape character '\' for
	// boost to treat it as a literal.
	// Thus, to prefix a matched expression with '\', we use:
	return boost::regex_replace(expr, reg, "\\\\$&");
}


// A functor for use with std::find_if, used to ascertain whether a
// data entry matches the required regex_
// @throws: boost::regex_error if the supplied regex pattern is not valid
// @todo: This function should be moved to support.
class RegexMatch : public std::unary_function<string, bool>
{
public:
	// re and icase are used to construct an instance of boost::RegEx.
	// if icase is true, then matching is insensitive to case
	RegexMatch(InfoMap const & m, string const & re, bool icase)
		: map_(m), regex_(re, icase) {}

	bool operator()(string const & key) const {
		// the data searched is the key + its associated BibTeX/biblio
		// fields
		string data = key;
		InfoMap::const_iterator info = map_.find(key);
		if (info != map_.end())
			// FIXME UNICODE
			data += ' ' + to_utf8(info->second);

		// Attempts to find a match for the current RE
		// somewhere in data.
		return boost::regex_search(data, regex_);
	}
private:
	InfoMap const map_;
	mutable boost::regex regex_;
};

} // namespace anon


vector<string>::const_iterator
searchKeys(InfoMap const & theMap,
	   vector<string> const & keys,
	   string const & search_expr,
	   vector<string>::const_iterator start,
	   Search type,
	   Direction dir,
	   bool caseSensitive)
{
	// Preliminary checks
	if (start < keys.begin() || start >= keys.end())
		return keys.end();

	string expr = trim(search_expr);
	if (expr.empty())
		return keys.end();

	if (type == SIMPLE)
		// We must escape special chars in the search_expr so that
		// it is treated as a simple string by boost::regex.
		expr = escape_special_chars(expr);

	try {
		// Build the functor that will be passed to find_if.
		RegexMatch const match(theMap, expr, !caseSensitive);

		// Search the vector of 'keys' from 'start' for one
		// that matches the predicate 'match'. Searching can
		// be forward or backward from start.
		if (dir == FORWARD)
			return std::find_if(start, keys.end(), match);

		vector<string>::const_reverse_iterator rit(start);
		vector<string>::const_reverse_iterator rend = keys.rend();
		rit = std::find_if(rit, rend, match);

		if (rit == rend)
			return keys.end();
		// This is correct and always safe.
		// (See Meyer's Effective STL, Item 28.)
		return (++rit).base();
	}
	catch (boost::regex_error &) {
		return keys.end();
	}
}


docstring const parseBibTeX(docstring data, string const & findkey)
{
	// at first we delete all characters right of '%' and
	// replace tabs through a space and remove leading spaces
	// we read the data line by line so that the \n are
	// ignored, too.
	docstring data_;
	int Entries = 0;
	docstring dummy = token(data,'\n', Entries);
	while (!dummy.empty()) {
		// no tabs
		dummy = subst(dummy, '\t', ' ');
		// no leading spaces
		dummy = ltrim(dummy);
		// ignore lines with a beginning '%' or ignore all right of %
		docstring::size_type const idx =
			dummy.empty() ? docstring::npos : dummy.find('%');
		if (idx != docstring::npos)
			// Check if this is really a comment or just "\%"
			if (idx == 0 || dummy[idx - 1] != '\\')
				dummy.erase(idx, docstring::npos);
			else
				//  This is "\%", so just erase the '\'
				dummy.erase(idx - 1, 1);
		// do we have a new token or a new line of
		// the same one? In the first case we ignore
		// the \n and in the second we replace it
		// with a space
		if (!dummy.empty()) {
			if (!contains(dummy, '='))
				data_ += ' ' + dummy;
			else
				data_ += dummy;
		}
		dummy = token(data, '\n', ++Entries);
	}

	// replace double commas with "" for easy scanning
	data = subst(data_, from_ascii(",,"), from_ascii("\"\""));

	// unlikely!
	if (data.empty())
		return docstring();

	// now get only the important line of the bibtex entry.
	// all entries are devided by ',' except the last one.
	data += ',';
	// now we have same behaviour for all entries because the last one
	// is "blah ... }"
	Entries = 0;
	bool found = false;
	// parsing of title and booktitle is different from the
	// others, because booktitle contains title
	do {
		dummy = token(data, ',', Entries++);
		if (!dummy.empty()) {
			found = contains(ascii_lowercase(dummy), from_ascii(findkey));
			if (findkey == "title" &&
			    contains(ascii_lowercase(dummy), from_ascii("booktitle")))
				found = false;
		}
	} while (!found && !dummy.empty());
	if (dummy.empty())
		// no such keyword
		return docstring();

	// we are not sure, if we get all, because "key= "blah, blah" is
	// allowed.
	// Therefore we read all until the next "=" character, which follows a
	// new keyword
	docstring keyvalue = dummy;
	dummy = token(data, ',', Entries++);
	while (!contains(dummy, '=') && !dummy.empty()) {
		keyvalue += ',' + dummy;
		dummy = token(data, ',', Entries++);
	}

	// replace double "" with originals ,, (two commas)
	// leaving us with the all-important line
	data = subst(keyvalue, from_ascii("\"\""), from_ascii(",,"));

	// Clean-up.
	// 1. Spaces
	data = rtrim(data);
	// 2. if there is no opening '{' then a closing '{' is probably cruft.
	if (!contains(data, '{'))
		data = rtrim(data, "}");
	// happens, when last keyword
	docstring::size_type const idx =
		!data.empty() ? data.find('=') : docstring::npos;

	if (idx == docstring::npos)
		return docstring();

	data = trim(data.substr(idx));

	// a valid entry?
	if (data.length() < 2 || data[0] != '=')
		return docstring();
	else {
		// delete '=' and the following spaces
		data = ltrim(data, " =");
		if (data.length() < 2) {
			// not long enough to find delimiters
			return data;
		} else {
			docstring::size_type keypos = 1;
			char_type enclosing;
			if (data[0] == '{') {
				enclosing = '}';
			} else if (data[0] == '"') {
				enclosing = '"';
			} else {
				// no {} and no "", pure data but with a
				// possible ',' at the end
				return rtrim(data, ",");
			}
			docstring tmp = data.substr(keypos);
			while (tmp.find('{') != docstring::npos &&
			       tmp.find('}') != docstring::npos &&
			       tmp.find('{') < tmp.find('}') &&
			       tmp.find('{') < tmp.find(enclosing)) {

				keypos += tmp.find('{') + 1;
				tmp = data.substr(keypos);
				keypos += tmp.find('}') + 1;
				tmp = data.substr(keypos);
			}
			if (tmp.find(enclosing) == docstring::npos)
				return data;
			else {
				keypos += tmp.find(enclosing);
				return data.substr(1, keypos - 1);
			}
		}
	}
}


namespace {


char const * const citeCommands[] = {
	"cite", "citet", "citep", "citealt", "citealp", "citeauthor",
	"citeyear", "citeyearpar" };

unsigned int const nCiteCommands =
	sizeof(citeCommands) / sizeof(char *);

CiteStyle const citeStyles[] = {
	CITE, CITET, CITEP, CITEALT, CITEALP,
	CITEAUTHOR, CITEYEAR, CITEYEARPAR };

unsigned int const nCiteStyles =
	sizeof(citeStyles) / sizeof(CiteStyle);

CiteStyle const citeStylesFull[] = {
	CITET, CITEP, CITEALT, CITEALP, CITEAUTHOR };

unsigned int const nCiteStylesFull =
	sizeof(citeStylesFull) / sizeof(CiteStyle);

CiteStyle const citeStylesUCase[] = {
	CITET, CITEP, CITEALT, CITEALP, CITEAUTHOR };

unsigned int const nCiteStylesUCase =
	sizeof(citeStylesUCase) / sizeof(CiteStyle);

} // namespace anon


CitationStyle::CitationStyle(string const & command)
	: style(CITE), full(false), forceUCase(false)
{
	if (command.empty())
		return;

	string cmd = command;
	if (cmd[0] == 'C') {
		forceUCase = true;
		cmd[0] = 'c';
	}

	string::size_type const n = cmd.size() - 1;
	if (cmd != "cite" && cmd[n] == '*') {
		full = true;
		cmd = cmd.substr(0,n);
	}

	char const * const * const last = citeCommands + nCiteCommands;
	char const * const * const ptr = std::find(citeCommands, last, cmd);

	if (ptr != last) {
		size_t idx = ptr - citeCommands;
		style = citeStyles[idx];
	}
}


string const CitationStyle::asLatexStr() const
{
	string cite = citeCommands[style];
	if (full) {
		CiteStyle const * last = citeStylesFull + nCiteStylesFull;
		if (std::find(citeStylesFull, last, style) != last)
			cite += '*';
	}

	if (forceUCase) {
		CiteStyle const * last = citeStylesUCase + nCiteStylesUCase;
		if (std::find(citeStylesUCase, last, style) != last)
			cite[0] = 'C';
	}

	return cite;
}


vector<CiteStyle> const getCiteStyles(CiteEngine const engine)
{
	unsigned int nStyles = 0;
	unsigned int start = 0;

	switch (engine) {
	case ENGINE_BASIC:
		nStyles = 1;
		start = 0;
		break;
	case ENGINE_NATBIB_AUTHORYEAR:
	case ENGINE_NATBIB_NUMERICAL:
		nStyles = nCiteStyles - 1;
		start = 1;
		break;
	case ENGINE_JURABIB:
		nStyles = nCiteStyles;
		start = 0;
		break;
	}

	typedef vector<CiteStyle> cite_vec;

	cite_vec styles(nStyles);
	cite_vec::size_type i = 0;
	int j = start;
	for (; i != styles.size(); ++i, ++j)
		styles[i] = citeStyles[j];

	return styles;
}


vector<docstring> const
getNumericalStrings(string const & key,
		    InfoMap const & map, vector<CiteStyle> const & styles)
{
	if (map.empty())
		return vector<docstring>();

	docstring const author = getAbbreviatedAuthor(map, key);
	docstring const year   = getYear(map, key);
	if (author.empty() || year.empty())
		return vector<docstring>();

	vector<docstring> vec(styles.size());
	for (vector<docstring>::size_type i = 0; i != vec.size(); ++i) {
		docstring str;

		switch (styles[i]) {
		case CITE:
		case CITEP:
			str = from_ascii("[#ID]");
			break;

		case CITET:
			str = author + " [#ID]";
			break;

		case CITEALT:
			str = author + " #ID";
			break;

		case CITEALP:
			str = from_ascii("#ID");
			break;

		case CITEAUTHOR:
			str = author;
			break;

		case CITEYEAR:
			str = year;
			break;

		case CITEYEARPAR:
			str = '(' + year + ')';
			break;
		}

		vec[i] = str;
	}

	return vec;
}


vector<docstring> const
getAuthorYearStrings(string const & key,
		    InfoMap const & map, vector<CiteStyle> const & styles)
{
	if (map.empty())
		return vector<docstring>();

	docstring const author = getAbbreviatedAuthor(map, key);
	docstring const year   = getYear(map, key);
	if (author.empty() || year.empty())
		return vector<docstring>();

	vector<docstring> vec(styles.size());
	for (vector<docstring>::size_type i = 0; i != vec.size(); ++i) {
		docstring str;

		switch (styles[i]) {
		case CITE:
			// jurabib only: Author/Annotator
			// (i.e. the "before" field, 2nd opt arg)
			str = author + "/<" + _("before") + '>';
			break;

		case CITET:
			str = author + " (" + year + ')';
			break;

		case CITEP:
			str = '(' + author + ", " + year + ')';
			break;

		case CITEALT:
			str = author + ' ' + year ;
			break;

		case CITEALP:
			str = author + ", " + year ;
			break;

		case CITEAUTHOR:
			str = author;
			break;

		case CITEYEAR:
			str = year;
			break;

		case CITEYEARPAR:
			str = '(' + year + ')';
			break;
		}

		vec[i] = str;
	}

	return vec;
}

} // namespace biblio

namespace frontend {

vector<FamilyPair> const getFamilyData()
{
	vector<FamilyPair> family(5);

	FamilyPair pr;

	pr.first = _("No change");
	pr.second = Font::IGNORE_FAMILY;
	family[0] = pr;

	pr.first = _("Roman");
	pr.second = Font::ROMAN_FAMILY;
	family[1] = pr;

	pr.first = _("Sans Serif");
	pr.second = Font::SANS_FAMILY;
	family[2] = pr;

	pr.first = _("Typewriter");
	pr.second = Font::TYPEWRITER_FAMILY;
	family[3] = pr;

	pr.first = _("Reset");
	pr.second = Font::INHERIT_FAMILY;
	family[4] = pr;

	return family;
}


vector<SeriesPair> const getSeriesData()
{
	vector<SeriesPair> series(4);

	SeriesPair pr;

	pr.first = _("No change");
	pr.second = Font::IGNORE_SERIES;
	series[0] = pr;

	pr.first = _("Medium");
	pr.second = Font::MEDIUM_SERIES;
	series[1] = pr;

	pr.first = _("Bold");
	pr.second = Font::BOLD_SERIES;
	series[2] = pr;

	pr.first = _("Reset");
	pr.second = Font::INHERIT_SERIES;
	series[3] = pr;

	return series;
}


vector<ShapePair> const getShapeData()
{
	vector<ShapePair> shape(6);

	ShapePair pr;

	pr.first = _("No change");
	pr.second = Font::IGNORE_SHAPE;
	shape[0] = pr;

	pr.first = _("Upright");
	pr.second = Font::UP_SHAPE;
	shape[1] = pr;

	pr.first = _("Italic");
	pr.second = Font::ITALIC_SHAPE;
	shape[2] = pr;

	pr.first = _("Slanted");
	pr.second = Font::SLANTED_SHAPE;
	shape[3] = pr;

	pr.first = _("Small Caps");
	pr.second = Font::SMALLCAPS_SHAPE;
	shape[4] = pr;

	pr.first = _("Reset");
	pr.second = Font::INHERIT_SHAPE;
	shape[5] = pr;

	return shape;
}


vector<SizePair> const getSizeData()
{
	vector<SizePair> size(14);

	SizePair pr;

	pr.first = _("No change");
	pr.second = Font::IGNORE_SIZE;
	size[0] = pr;

	pr.first = _("Tiny");
	pr.second = Font::SIZE_TINY;
	size[1] = pr;

	pr.first = _("Smallest");
	pr.second = Font::SIZE_SCRIPT;
	size[2] = pr;

	pr.first = _("Smaller");
	pr.second = Font::SIZE_FOOTNOTE;
	size[3] = pr;

	pr.first = _("Small");
	pr.second = Font::SIZE_SMALL;
	size[4] = pr;

	pr.first = _("Normal");
	pr.second = Font::SIZE_NORMAL;
	size[5] = pr;

	pr.first = _("Large");
	pr.second = Font::SIZE_LARGE;
	size[6] = pr;

	pr.first = _("Larger");
	pr.second = Font::SIZE_LARGER;
	size[7] = pr;

	pr.first = _("Largest");
	pr.second = Font::SIZE_LARGEST;
	size[8] = pr;

	pr.first = _("Huge");
	pr.second = Font::SIZE_HUGE;
	size[9] = pr;

	pr.first = _("Huger");
	pr.second = Font::SIZE_HUGER;
	size[10] = pr;

	pr.first = _("Increase");
	pr.second = Font::INCREASE_SIZE;
	size[11] = pr;

	pr.first = _("Decrease");
	pr.second = Font::DECREASE_SIZE;
	size[12] = pr;

	pr.first = _("Reset");
	pr.second = Font::INHERIT_SIZE;
	size[13] = pr;

	return size;
}


vector<BarPair> const getBarData()
{
	vector<BarPair> bar(5);

	BarPair pr;

	pr.first = _("No change");
	pr.second = IGNORE;
	bar[0] = pr;

	pr.first = _("Emph");
	pr.second = EMPH_TOGGLE;
	bar[1] = pr;

	pr.first = _("Underbar");
	pr.second = UNDERBAR_TOGGLE;
	bar[2] = pr;

	pr.first = _("Noun");
	pr.second = NOUN_TOGGLE;
	bar[3] = pr;

	pr.first = _("Reset");
	pr.second = INHERIT;
	bar[4] = pr;

	return bar;
}


vector<ColorPair> const getColorData()
{
	vector<ColorPair> color(11);

	ColorPair pr;

	pr.first = _("No change");
	pr.second = Color::ignore;
	color[0] = pr;

	pr.first = _("No color");
	pr.second = Color::none;
	color[1] = pr;

	pr.first = _("Black");
	pr.second = Color::black;
	color[2] = pr;

	pr.first = _("White");
	pr.second = Color::white;
	color[3] = pr;

	pr.first = _("Red");
	pr.second = Color::red;
	color[4] = pr;

	pr.first = _("Green");
	pr.second = Color::green;
	color[5] = pr;

	pr.first = _("Blue");
	pr.second = Color::blue;
	color[6] = pr;

	pr.first = _("Cyan");
	pr.second = Color::cyan;
	color[7] = pr;

	pr.first = _("Magenta");
	pr.second = Color::magenta;
	color[8] = pr;

	pr.first = _("Yellow");
	pr.second = Color::yellow;
	color[9] = pr;

	pr.first = _("Reset");
	pr.second = Color::inherit;
	color[10] = pr;

	return color;
}



namespace {

class Sorter
	: public std::binary_function<LanguagePair,
				      LanguagePair, bool>
{
public:
	bool operator()(LanguagePair const & lhs,
			LanguagePair const & rhs) const {
		return lhs.first < rhs.first;
	}
};


class ColorSorter
{
public:
	bool operator()(Color::color const & lhs,
			Color::color const & rhs) const {
		return lcolor.getGUIName(lhs) < lcolor.getGUIName(rhs);
	}
};

} // namespace anon


vector<LanguagePair> const getLanguageData(bool character_dlg)
{
	vector<LanguagePair>::size_type const size = character_dlg ?
		languages.size() + 2 : languages.size();

	vector<LanguagePair> langs(size);

	if (character_dlg) {
		langs[0].first = _("No change");
		langs[0].second = "ignore";
		langs[1].first = _("Reset");
		langs[1].second = "reset";
	}

	vector<string>::size_type i = character_dlg ? 2 : 0;
	for (Languages::const_iterator cit = languages.begin();
	     cit != languages.end(); ++cit) {
		langs[i].first  = _(cit->second.display());
		langs[i].second = cit->second.lang();
		++i;
	}

	// Don't sort "ignore" and "reset"
	vector<LanguagePair>::iterator begin = character_dlg ?
		langs.begin() + 2 : langs.begin();

	std::sort(begin, langs.end(), Sorter());

	return langs;
}


vector<Color_color> const getSortedColors(vector<Color_color> colors)
{
	// sort the colors
	std::sort(colors.begin(), colors.end(), ColorSorter());
	return colors;
}

} // namespace frontend

using support::addName;
using support::FileFilterList;
using support::getExtension;
using support::libFileSearch;
using support::makeAbsPath;
using support::makeRelPath;
using support::onlyFilename;
using support::onlyPath;
using support::package;
using support::prefixIs;
using support::removeExtension;

namespace frontend {


docstring const browseFile(docstring const & filename,
			docstring const & title,
			FileFilterList const & filters,
			bool save,
			pair<docstring,docstring> const & dir1,
			pair<docstring,docstring> const & dir2)
{
	docstring lastPath = from_ascii(".");
	if (!filename.empty())
		lastPath = from_utf8(onlyPath(to_utf8(filename)));

	FileDialog fileDlg(title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result result;

	if (save)
		result = fileDlg.save(lastPath, filters,
				      from_utf8(onlyFilename(to_utf8(filename))));
	else
		result = fileDlg.open(lastPath, filters,
				      from_utf8(onlyFilename(to_utf8(filename))));

	return result.second;
}


docstring const browseRelFile(docstring const & filename,
			   docstring const & refpath,
			   docstring const & title,
			   FileFilterList const & filters,
			   bool save,
			   pair<docstring,docstring> const & dir1,
			   pair<docstring,docstring> const & dir2)
{
	docstring const fname = from_utf8(makeAbsPath(
		to_utf8(filename), to_utf8(refpath)).absFilename());

	docstring const outname = browseFile(fname, title, filters, save,
					  dir1, dir2);
	docstring const reloutname = makeRelPath(outname, refpath);
	if (prefixIs(reloutname, from_ascii("../")))
		return outname;
	else
		return reloutname;
}



docstring const browseLibFile(docstring const & dir,
			   docstring const & name,
			   docstring const & ext,
			   docstring const & title,
			   FileFilterList const & filters)
{
	// FIXME UNICODE
	pair<docstring, docstring> const dir1(_("System files|#S#s"),
				       from_utf8(addName(package().system_support().absFilename(), to_utf8(dir))));

	pair<docstring, docstring> const dir2(_("User files|#U#u"),
				       from_utf8(addName(package().user_support().absFilename(), to_utf8(dir))));

	docstring const result = browseFile(from_utf8(
		libFileSearch(to_utf8(dir), to_utf8(name), to_utf8(ext)).absFilename()),
		title, filters, false, dir1, dir2);

	// remove the extension if it is the default one
	docstring noextresult;
	if (from_utf8(getExtension(to_utf8(result))) == ext)
		noextresult = from_utf8(removeExtension(to_utf8(result)));
	else
		noextresult = result;

	// remove the directory, if it is the default one
	docstring const file = from_utf8(onlyFilename(to_utf8(noextresult)));
	if (from_utf8(libFileSearch(to_utf8(dir), to_utf8(file), to_utf8(ext)).absFilename()) == result)
		return file;
	else
		return noextresult;
}


docstring const browseDir(docstring const & pathname,
		       docstring const & title,
		       pair<docstring,docstring> const & dir1,
		       pair<docstring,docstring> const & dir2)
{
	docstring lastPath = from_ascii(".");
	if (!pathname.empty())
		lastPath = from_utf8(onlyPath(to_utf8(pathname)));

	FileDialog fileDlg(title, LFUN_SELECT_FILE_SYNC, dir1, dir2);

	FileDialog::Result const result =
		fileDlg.opendir(lastPath, from_utf8(onlyFilename(to_utf8(pathname))));

	return result.second;
}


vector<docstring> const getLatexUnits()
{
	vector<docstring> units;
	int i = 0;
	char const * str = stringFromUnit(i);
	for (; str != 0; ++i, str = stringFromUnit(i))
		units.push_back(from_ascii(str));

	return units;
}

} // namespace frontend


using support::bformat;
using support::contains;
using support::FileName;
using support::getExtension;
using support::getFileContents;
using support::getVectorFromString;
using support::libFileSearch;
using support::onlyFilename;
using support::package;
using support::quoteName;
using support::split;
using support::Systemcall;
using support::token;

namespace frontend {

void rescanTexStyles()
{
	// Run rescan in user lyx directory
	support::Path p(package().user_support());
	FileName const command = libFileSearch("scripts", "TeXFiles.py");
	Systemcall one;
	int const status = one.startscript(Systemcall::Wait,
			lyx::support::os::python() + ' ' +
			quoteName(command.toFilesystemEncoding()));
	if (status == 0)
		return;
	// FIXME UNICODE
	Alert::error(_("Could not update TeX information"),
		     bformat(_("The script `%s' failed."), lyx::from_utf8(command.absFilename())));
}


void texhash()
{
	// Run texhash in user lyx directory
	support::Path p(package().user_support());

	//path to texhash through system
	Systemcall one;
	one.startscript(Systemcall::Wait,"texhash");
}


void getTexFileList(string const & filename, std::vector<string> & list)
{
	list.clear();
	FileName const file = libFileSearch("", filename);
	if (file.empty())
		return;

	list = getVectorFromString(getFileContents(file), "\n");

	// Normalise paths like /foo//bar ==> /foo/bar
	boost::RegEx regex("/{2,}");
	std::vector<string>::iterator it  = list.begin();
	std::vector<string>::iterator end = list.end();
	for (; it != end; ++it) {
		*it = regex.Merge((*it), "/");
	}

	// remove empty items and duplicates
	list.erase(std::remove(list.begin(), list.end(), ""), list.end());
	eliminate_duplicates(list);
}


string const getListOfOptions(string const & classname, string const & type)
{
	FileName const filename(getTexFileFromList(classname, type));
	if (filename.empty())
		return string();
	string optionList = string();
	std::ifstream is(filename.toFilesystemEncoding().c_str());
	while (is) {
		string s;
		is >> s;
		if (contains(s,"DeclareOption")) {
			s = s.substr(s.find("DeclareOption"));
			s = split(s,'{');		// cut front
			s = token(s,'}',0);		// cut end
			optionList += (s + '\n');
		}
	}
	return optionList;
}


string const getTexFileFromList(string const & file,
			    string const & type)
{
	string file_ = file;
	// do we need to add the suffix?
	if (!(getExtension(file) == type))
		file_ += '.' + type;

	lyxerr << "Searching for file " << file_ << endl;

	string lstfile;
	if (type == "cls")
		lstfile = "clsFiles.lst";
	else if (type == "sty")
		lstfile = "styFiles.lst";
	else if (type == "bst")
		lstfile = "bstFiles.lst";
	else if (type == "bib")
		lstfile = "bibFiles.lst";
	FileName const abslstfile = libFileSearch(string(), lstfile);
	if (abslstfile.empty()) {
		lyxerr << "File `'" << lstfile << "' not found." << endl;
		return string();
	}
	string const allClasses = getFileContents(abslstfile);
	int entries = 0;
	string classfile = token(allClasses, '\n', entries);
	int count = 0;
	while ((!contains(classfile, file) ||
		(onlyFilename(classfile) != file)) &&
		(++count < 1000)) {
		classfile = token(allClasses, '\n', ++entries);
	}

	// now we have filename with full path
	lyxerr << "with full path: " << classfile << endl;

	return classfile;
}

} // namespace frontend
} // namespace lyx
