/**
 * \file biblio.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voss
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "LString.h"
#include "biblio.h"
#include "gettext.h" // for _()
#include "helper_funcs.h"
#include "support/lstrings.h"
#include "support/LAssert.h"

#include <boost/regex.hpp>
#include "BoostFormat.h"

#include <algorithm>

using std::vector;

namespace biblio {

string const familyName(string const & name)
{
	// Very simple parser
	string fname = name;

	// possible authorname combinations are:
	// "Surname, FirstName"
	// "Surname, F."
	// "FirstName Surname"
	// "F. Surname"
	string::size_type idx = fname.find(',');
	if (idx != string::npos)
		return ltrim(fname.substr(0, idx));
	idx = fname.rfind('.');
	if (idx != string::npos)
		fname = ltrim(fname.substr(idx + 1));
	// test if we have a LaTeX Space in front
	if (fname[0] == '\\')
		return fname.substr(2);

	return rtrim(fname);
}


string const getAbbreviatedAuthor(InfoMap const & map, string const & key)
{
	lyx::Assert(!map.empty());

	InfoMap::const_iterator it = map.find(key);
	if (it == map.end())
		return string();
	string const & data = it->second;

	// Is the entry a BibTeX one or one from lyx-layout "bibliography"?
	string::size_type const pos = data.find("TheBibliographyRef");
	if (pos != string::npos) {
		if (pos <= 2) {
			return string();
		}

		string const opt = trim(data.substr(0, pos - 1));
		if (opt.empty())
			return string();

		string authors;
		split(opt, authors, '(');
		return authors;
	}

	string author = parseBibTeX(data, "author");
	if (author.empty())
		author = parseBibTeX(data, "editor");

	if (author.empty()) {
		author = parseBibTeX(data, "key");
		if (author.empty())
			author = key;
		return author;
	}

	vector<string> const authors = getVectorFromString(author, " and ");
	if (authors.empty())
		return author;

#if USE_BOOST_FORMAT
	boost::format fmter("");
	if (authors.size() == 2)
		fmter = boost::format(_("%1$s and %2$s"))
			% familyName(authors[0]) % familyName(authors[1]);
	else if (authors.size() > 2)
		fmter = boost::format(_("%1$s et al.")) % familyName(authors[0]);
	else
		fmter = boost::format("%1$s") % familyName(authors[0]);
	return fmter.str();
#else
	string msg;
	if (authors.size() == 2)
		msg = familyName(authors[0]) + _(" and ") + familyName(authors[1]);
	else if (authors.size() > 2)
		msg = familyName(authors[0]) + _("et al.");
	else
		msg = familyName(authors[0]);
	return msg;
#endif
}


string const getYear(InfoMap const & map, string const & key)
{
	lyx::Assert(!map.empty());

	InfoMap::const_iterator it = map.find(key);
	if (it == map.end())
		return string();
	string const & data = it->second;

	// Is the entry a BibTeX one or one from lyx-layout "bibliography"?
	string::size_type const pos = data.find("TheBibliographyRef");
	if (pos != string::npos) {
		if (pos <= 2) {
			return string();
		}

		string const opt =
			trim(data.substr(0, pos - 1));
		if (opt.empty())
			return string();

		string authors;
		string const tmp = split(opt, authors, '(');
		string year;
		split(tmp, year, ')');
		return year;

	}

	string year = parseBibTeX(data, "year");
	if (year.empty())
		year = _("No year");

	return year;
}


namespace {

// A functor for use with std::sort, leading to case insensitive sorting
struct compareNoCase: public std::binary_function<string, string, bool>
{
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


string const getInfo(InfoMap const & map, string const & key)
{
	lyx::Assert(!map.empty());

	InfoMap::const_iterator it = map.find(key);
	if (it == map.end())
		return string();
	string const & data = it->second;

	// is the entry a BibTeX one or one from lyx-layout "bibliography"?
	string const separator("TheBibliographyRef");
	string::size_type const pos = data.find(separator);
	if (pos != string::npos) {
		string::size_type const pos2 = pos + separator.size();
		string const info = trim(data.substr(pos2));
		return info;
	}

	// Search for all possible "required" keys
	string author = parseBibTeX(data, "author");
	if (author.empty())
		author = parseBibTeX(data, "editor");

	string year       = parseBibTeX(data, "year");
	string title      = parseBibTeX(data, "title");
	string booktitle  = parseBibTeX(data, "booktitle");
	string chapter    = parseBibTeX(data, "chapter");
	string number     = parseBibTeX(data, "number");
	string volume     = parseBibTeX(data, "volume");
	string pages      = parseBibTeX(data, "pages");

	string media      = parseBibTeX(data, "journal");
	if (media.empty())
		media = parseBibTeX(data, "publisher");
	if (media.empty())
		media = parseBibTeX(data, "school");
	if (media.empty())
		media = parseBibTeX(data, "institution");

	ostringstream result;
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

	string const result_str = rtrim(STRCONV(result.str()));
	if (!result_str.empty())
		return result_str;

	// This should never happen (or at least be very unusual!)
	return data;
}


namespace {

// Escape special chars.
// All characters are literals except: '.|*?+(){}[]^$\'
// These characters are literals when preceded by a "\", which is done here
string const escape_special_chars(string const & expr)
{
	// Search for all chars '.|*?+(){}[^$]\'
	// Note that '[' and '\' must be escaped.
	// This is a limitation of boost::regex, but all other chars in BREs
	// are assumed literal.
	boost::RegEx reg("[].|*?+(){}^$\\[\\\\]");

	// $& is a perl-like expression that expands to all of the current match
	// The '$' must be prefixed with the escape character '\' for
	// boost to treat it as a literal.
	// Thus, to prefix a matched expression with '\', we use:
	string const fmt("\\\\$&");

	return reg.Merge(expr, fmt);
}


// A functor for use with std::find_if, used to ascertain whether a
// data entry matches the required regex_
struct RegexMatch
{
	// re and icase are used to construct an instance of boost::RegEx.
	// if icase is true, then matching is insensitive to case
	RegexMatch(InfoMap const & m, string const & re, bool icase)
		: map_(m), regex_(re, icase) {}

	bool operator()(string const & key) {
		if (!validRE())
			return false;

		// the data searched is the key + its associated BibTeX/biblio
		// fields
		string data = key;
		InfoMap::const_iterator info = map_.find(key);
		if (info != map_.end())
			data += ' ' + info->second;

		// Attempts to find a match for the current RE
		// somewhere in data.
		return regex_.Search(data);
	}

	bool validRE() const { return regex_.error_code() == 0; }

private:
	InfoMap const map_;
	boost::RegEx regex_;
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

	// Build the functor that will be passed to find_if.
	RegexMatch const match(theMap, expr, !caseSensitive);
	if (!match.validRE())
		return keys.end();

	// Search the vector of 'keys' from 'start' for one that matches the
	// predicate 'match'. Searching can be forward or backward from start.
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


string const parseBibTeX(string data, string const & findkey)
{
	string keyvalue;
	// at first we delete all characters right of '%' and
	// replace tabs through a space and remove leading spaces
	// we read the data line by line so that the \n are
	// ignored, too.
	string data_;
	int Entries = 0;
	string dummy = token(data,'\n', Entries);
	while (!dummy.empty()) {
		dummy = subst(dummy, '\t', ' ');	// no tabs
		dummy = ltrim(dummy);		// no leading spaces
		// ignore lines with a beginning '%' or ignore all right of %
		string::size_type const idx =
			dummy.empty() ? string::npos : dummy.find('%');
		if (idx != string::npos)
			dummy.erase(idx, string::npos);
		// do we have a new token or a new line of
		// the same one? In the first case we ignore
		// the \n and in the second we replace it
		// with a space
		if (!dummy.empty()) {
			if (!contains(dummy, "="))
				data_ += ' ' + dummy;
			else
				data_ += dummy;
		}
		dummy = token(data, '\n', ++Entries);
	}

	// replace double commas with "" for easy scanning
	data = subst(data_, ",,", "\"\"");

	// unlikely!
	if (data.empty())
		return string();

	// now get only the important line of the bibtex entry.
	// all entries are devided by ',' except the last one.
	data += ',';  // now we have same behaviour for all entries
		      // because the last one is "blah ... }"
	Entries = 0;
	bool found = false;
	// parsing of title and booktitle is different from the
	// others, because booktitle contains title
	do {
		dummy = token(data, ',', Entries++);
		if (!dummy.empty()) {
			found = contains(ascii_lowercase(dummy), findkey);
			if (findkey == "title" &&
				contains(ascii_lowercase(dummy), "booktitle"))
				found = false;
		}
	} while (!found && !dummy.empty());
	if (dummy.empty())
		// no such keyword
		return string();

	// we are not sure, if we get all, because "key= "blah, blah" is
	// allowed.
	// Therefore we read all until the next "=" character, which follows a
	// new keyword
	keyvalue = dummy;
	dummy = token(data, ',', Entries++);
	while (!contains(dummy, '=') && !dummy.empty()) {
		keyvalue += ',' + dummy;
		dummy = token(data, ',', Entries++);
	}

	// replace double "" with originals ,, (two commas)
	// leaving us with the all-important line
	data = subst(keyvalue, "\"\"", ",,");

	// Clean-up.
	// 1. Spaces
	data = rtrim(data);
	// 2. if there is no opening '{' then a closing '{' is probably cruft.
	if (!contains(data, '{'))
		data = rtrim(data, "}");
	// happens, when last keyword
	string::size_type const idx =
		!data.empty() ? data.find('=') : string::npos;

	if (idx == string::npos)
		return string();

	data = trim(data.substr(idx));

	if (data.length() < 2 || data[0] != '=') {	// a valid entry?
		return string();
	} else {
		// delete '=' and the following spaces
		data = ltrim(data, " =");
		if (data.length() < 2) {
			return data;	// not long enough to find delimiters
		} else {
			string::size_type keypos = 1;
			char enclosing;
			if (data[0] == '{') {
				enclosing = '}';
			} else if (data[0] == '"') {
				enclosing = '"';
			} else {
				// no {} and no "", pure data but with a
				// possible ',' at the end
				return rtrim(data, ",");
			}
			string tmp = data.substr(keypos);
			while (tmp.find('{') != string::npos &&
			       tmp.find('}') != string::npos &&
			       tmp.find('{') < tmp.find('}') &&
			       tmp.find('{') < tmp.find(enclosing)) {

				keypos += tmp.find('{') + 1;
				tmp = data.substr(keypos);
				keypos += tmp.find('}') + 1;
				tmp = data.substr(keypos);
			}
			if (tmp.find(enclosing) == string::npos)
				return data;
			else {
				keypos += tmp.find(enclosing);
				return data.substr(1, keypos - 1);
			}
		}
	}
}


namespace {

using namespace biblio;

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


CitationStyle const getCitationStyle(string const & command)
{
	if (command.empty()) return CitationStyle();

	CitationStyle cs;
	string cmd = command;

	if (cmd[0] == 'C') {
		cs.forceUCase = true;
		cmd[0] = 'c';
	}

	size_t n = cmd.size() - 1;
	if (cmd[n] == '*') {
		cs.full = true;
		cmd = cmd.substr(0,n);
	}

	char const * const * const last = citeCommands + nCiteCommands;
	char const * const * const ptr = std::find(citeCommands, last, cmd);

	if (ptr != last) {
		size_t idx = ptr - citeCommands;
		cs.style = citeStyles[idx];
	}

	return cs;
}


string const getCiteCommand(CiteStyle command, bool full, bool forceUCase)
{
	string cite = citeCommands[command];
	if (full) {
		CiteStyle const * last = citeStylesFull + nCiteStylesFull;
		if (std::find(citeStylesFull, last, command) != last)
			cite += '*';
	}

	if (forceUCase) {
		CiteStyle const * last = citeStylesUCase + nCiteStylesUCase;
		if (std::find(citeStylesUCase, last, command) != last)
			cite[0] = 'C';
	}

	return cite;
}


vector<CiteStyle> const getCiteStyles(bool usingNatbib)
{
	unsigned int nStyles = 1;
	unsigned int start = 0;
	if (usingNatbib) {
		nStyles = nCiteStyles - 1;
		start = 1;
	}

	vector<CiteStyle> styles(nStyles);

	vector<CiteStyle>::size_type i = 0;
	int j = start;
	for (; i != styles.size(); ++i, ++j) {
		styles[i] = citeStyles[j];
	}

	return styles;
}


vector<string> const
getNumericalStrings(string const & key,
		    InfoMap const & map, vector<CiteStyle> const & styles)
{
	if (map.empty()) {
		return vector<string>();
	}

	string const author = getAbbreviatedAuthor(map, key);
	string const year   = getYear(map, key);
	if (author.empty() || year.empty())
		return vector<string>();

	vector<string> vec(styles.size());
	for (vector<string>::size_type i = 0; i != vec.size(); ++i) {
		string str;

		switch (styles[i]) {
		case CITE:
		case CITEP:
			str = "[#ID]";
			break;

		case CITET:
			str = author + " [#ID]";
			break;

		case CITEALT:
			str = author + " #ID";
			break;

		case CITEALP:
			str = "#ID";
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


vector<string> const
getAuthorYearStrings(string const & key,
		    InfoMap const & map, vector<CiteStyle> const & styles)
{
	if (map.empty()) {
		return vector<string>();
	}

	string const author = getAbbreviatedAuthor(map, key);
	string const year   = getYear(map, key);
	if (author.empty() || year.empty())
		return vector<string>();

	vector<string> vec(styles.size());
	for (vector<string>::size_type i = 0; i != vec.size(); ++i) {
		string str;

		switch (styles[i]) {
		case CITE:
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
