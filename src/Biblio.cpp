/**
 * \file Biblio.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 * \author Richard Heck (re-write of BibTeX representation)
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "Biblio.h"

#include "gettext.h"
#include "InsetIterator.h"
#include "Paragraph.h"

#include "insets/Inset.h"
#include "insets/InsetBibitem.h"
#include "insets/InsetBibtex.h"
#include "insets/InsetInclude.h"

#include "support/lstrings.h"

#include "boost/regex.hpp"

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
			"cite", "citet", "citep", "citealt", "citealp",
			"citeauthor", "citeyear", "citeyearpar",
			"citet*", "citep*", "citealt*", "citealp*", "citeauthor*",
			"Citet",  "Citep",  "Citealt",  "Citealp",  "Citeauthor",
			"Citet*", "Citep*", "Citealt*", "Citealp*", "Citeauthor*",
			"fullcite",
			"footcite", "footcitet", "footcitep", "footcitealt",
			"footcitealp", "footciteauthor", "footciteyear", "footciteyearpar",
			"citefield", "citetitle", "cite*"
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


} // namespace anon


const docstring TheBibliographyRef(from_ascii("TheBibliographyRef"));

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


vector<string>::const_iterator searchKeys(InfoMap const & theMap,
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
	} //end while

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


void fillWithBibKeys(Buffer const * const buf, 
                     vector<pair<string, docstring> > & keys)
{	
	/// if this is a child document and the parent is already loaded
	/// use the parent's list instead  [ale990412]
	Buffer const * const tmp = buf->getMasterBuffer();
	BOOST_ASSERT(tmp);
	if (tmp != buf) {
		fillWithBibKeys(tmp, keys);
		return;
	}

	for (InsetIterator it = inset_iterator_begin(buf->inset()); it; ++it)
			it->fillWithBibKeys(*buf, keys, it);
}
} // namespace biblio
} // namespace lyx

