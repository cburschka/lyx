/* This file is part of
 * ======================================================
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file biblio.C
 * \author Angus Leeming <a.leeming@ic.ac.uk>
 * \author Herbert Voss <voss@perce.de>
 */

#include <config.h>

#include <vector>
#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LString.h"
#include "biblio.h"
#include "gettext.h" // for _()
#include "helper_funcs.h"
#include "support/lstrings.h"
#include "support/LAssert.h"
#include "support/LRegex.h"

using std::find;
using std::min;
using std::vector;
using std::sort;

namespace biblio
{

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


// The functions doing the dirty work for the search.
vector<string>::const_iterator
simpleSearch(InfoMap const & theMap,
	     vector<string> const & keys,
	     string const & expr,
	     vector<string>::const_iterator start,
	     Direction dir,
	     bool caseSensitive)
{
	string tmp = expr;
	if (!caseSensitive)
		tmp = lowercase(tmp);

	vector<string> searchwords = getVectorFromString(tmp, " ");

	// Loop over all keys from start...
	for (vector<string>::const_iterator it = start;
	     // End condition is direction-dependent.
	     (dir == FORWARD) ? (it<keys.end()) : (it>=keys.begin());
	     // increment is direction-dependent.
	     (dir == FORWARD) ? (++it) : (--it)) {

		string data = (*it);
		InfoMap::const_iterator info = theMap.find(*it);
		if (info != theMap.end())
			data += " " + info->second;
		if (!caseSensitive)
			data = lowercase(data);

		bool found = true;

		// Loop over all search words...
		for (vector<string>::const_iterator sit = searchwords.begin();
		     sit != searchwords.end(); ++sit) {
			if (data.find(*sit) == string::npos) {
				found = false;
				break;
			}
		}

		if (found) return it;
	}

	return keys.end();
}


vector<string>::const_iterator
regexSearch(InfoMap const & theMap,
	    vector<string> const & keys,
	    string const & expr,
	    vector<string>::const_iterator start,
	    Direction dir)
{
	LRegex reg(expr);

	for (vector<string>::const_iterator it = start;
	     // End condition is direction-dependent.
	     (dir == FORWARD) ? (it<keys.end()) : (it>=keys.begin());
	     // increment is direction-dependent.
	     (dir == FORWARD) ? (++it) : (--it)) {

		string data = (*it);
		InfoMap::const_iterator info = theMap.find(*it);
		if (info != theMap.end())
			data += " " + info->second;

		if (reg.exec(data).size() > 0)
			return it;
	}

	return keys.end();
}


} // namespace anon


string const familyName(string const & name)
{
	// Very simple parser
	string fname = name;

	// possible authorname combinations are:
	// "Surname, FirstName"
	// "Surname, F."
	// "FirstName Surname"
	// "F. Surname"
	string::size_type idx = fname.find(",");
	if (idx != string::npos)
		return frontStrip(fname.substr(0,idx));
	idx = fname.rfind(".");
	if (idx != string::npos)
		fname = frontStrip(fname.substr(idx+1));
	// test if we have a LaTeX Space in front
	if (fname[0] == '\\')
		return fname.substr(2);

	return fname;
}


string const getAbbreviatedAuthor(InfoMap const & map, string const & key)
{
	lyx::Assert(!map.empty());

	InfoMap::const_iterator it = map.find(key);
	if (it == map.end())
		return string();

	string::size_type const pos = it->second.find("TheBibliographyRef");
	if (pos != string::npos) {
		if (pos <= 2) {
			return string();
		}

		string const opt =
			strip(frontStrip(it->second.substr(0, pos-1)));
		if (opt.empty())
			return string();

		string authors;
		split(opt, authors, '(');
		return authors;
	}

	string author = parseBibTeX(it->second, "author");
	if (author.empty())
		author = parseBibTeX(it->second, "editor");

	if (author.empty()) {
		author = parseBibTeX(it->second, "key");
		if (author.empty())
			author = key;
		return author;
	}

	vector<string> authors = getVectorFromString(author, " and");

	if (!authors.empty()) {
		author.erase();

		for (vector<string>::iterator it = authors.begin();
		     it != authors.end(); ++it) {
			*it = familyName(strip(*it));
		}

		author = authors[0];
		if (authors.size() == 2)
			author += _(" and ") + authors[1];
		else if (authors.size() > 2)
			author += _(" et al.");
	}

	return author;
}


string const getYear(InfoMap const & map, string const & key)
{
	lyx::Assert(!map.empty());

	InfoMap::const_iterator it = map.find(key);
	if (it == map.end())
		return string();

	string::size_type const pos = it->second.find("TheBibliographyRef");
	if (pos != string::npos) {
		if (pos <= 2) {
			return string();
		}

		string const opt =
			strip(frontStrip(it->second.substr(0, pos-1)));
		if (opt.empty())
			return string();

		string authors;
		string const tmp = split(opt, authors, '(');
		string year;
		split(tmp, year, ')');
		return year;

	}

	string year = parseBibTeX(it->second, "year");
	if (year.empty())
		year = _("No year");

	return year;
}


// A functor for use with std::sort, leading to case insensitive sorting
struct compareNoCase: public std::binary_function<string, string, bool>
{
	bool operator()(string const & s1, string const & s2) const {
		return compare_no_case(s1, s2) < 0;
	}
};

vector<string> const getKeys(InfoMap const & map)
{
	vector<string> bibkeys;

	for (InfoMap::const_iterator it = map.begin(); it != map.end(); ++it) {
		bibkeys.push_back(it->first);
	}

	sort(bibkeys.begin(), bibkeys.end(), compareNoCase());
	return bibkeys;
}


string const getInfo(InfoMap const & map, string const & key)
{
	lyx::Assert(!map.empty());

	InfoMap::const_iterator it = map.find(key);
	if (it == map.end())
		return string();

	// is the entry a BibTeX one or one from lyx-layout "bibliography"?
	string const separator("TheBibliographyRef");
	string::size_type const pos = it->second.find(separator);
	if (pos != string::npos) {
		string::size_type const pos2 = pos + separator.size();
		string const info = strip(frontStrip(it->second.substr(pos2)));
		return info;
	}

	// Search for all possible "required" keys
	string author = parseBibTeX(it->second, "author");
	if (author.empty())
		author = parseBibTeX(it->second, "editor");

	string year       = parseBibTeX(it->second, "year");
	string title      = parseBibTeX(it->second, "title");
	string booktitle  = parseBibTeX(it->second, "booktitle");
	string chapter    = parseBibTeX(it->second, "chapter");
	string number     = parseBibTeX(it->second, "number");
	string volume     = parseBibTeX(it->second, "volume");
	string pages      = parseBibTeX(it->second, "pages");

	string media      = parseBibTeX(it->second, "journal");
	if (media.empty())
		media = parseBibTeX(it->second, "publisher");
	if (media.empty())
		media = parseBibTeX(it->second, "school");
	if (media.empty())
		media = parseBibTeX(it->second, "institution");

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

	string const result_str = strip(result.str().c_str());
	if (!result_str.empty())
		return result_str;

	// This should never happen (or at least be very unusual!)
	return it->second;
}


vector<string>::const_iterator
searchKeys(InfoMap const & theMap,
	   vector<string> const & keys,
	   string const & expr,
	   vector<string>::const_iterator start,
	   Search type,
	   Direction dir,
	   bool caseSensitive)
{
	// Preliminary checks
	if (start < keys.begin() || start >= keys.end())
		return keys.end();

	string search_expr = frontStrip(strip(expr));
	if (search_expr.empty())
		return keys.end();

	if (type == SIMPLE)
		return simpleSearch(theMap, keys, search_expr, start, dir,
				    caseSensitive);

	return regexSearch(theMap, keys, search_expr, start, dir);
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
		dummy = frontStrip(dummy);		// no leading spaces
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
		    		data_ += (' ' + dummy); 
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
			found = contains(lowercase(dummy), findkey);
			if (findkey == "title" && 
				contains(lowercase(dummy), "booktitle"))
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
		keyvalue += (',' + dummy);
		dummy = token(data, ',', Entries++);
	}

	// replace double "" with originals ,, (two commas)
	// leaving us with the all-important line
	data = subst(keyvalue, "\"\"", ",,");

	// Clean-up.
	// 1. Spaces
	data = strip(data, ' ');
	// 2. if there is no opening '{' then a closing '{' is probably cruft.
	if (!contains(data, '{'))
		data = strip(data, '}');
	// happens, when last keyword
	string::size_type const idx =
		!data.empty() ? data.find('=') : string::npos;

	if (idx == string::npos)
		return string();

	data = data.substr(idx);
	data = frontStrip(strip(data));

	if (data.length() < 2 || data[0] != '=') {	// a valid entry?
		return string();
	} else {
		// delete '=' and the following spaces
		data = frontStrip(frontStrip(data,'='));
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
				return strip(data,',');
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


CitationStyle const getCitationStyle(string const & command)
{
	if (command.empty()) return CitationStyle();

	CitationStyle cs;
	string cmd = command;

	if (cmd[0] == 'C') {
		cs.forceUCase = true;
		cmd[0] = 'c';
	}

	size_t n = cmd.size()-1;
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
			cite += "*";
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
			str = "(" + year + ")";
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
			str = author + " (" + year + ")";
			break;

		case CITEP:
			str = "(" + author + ", " + year + ")";
			break;

		case CITEALT:
			str = author + " " + year ;
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
			str = "(" + year + ")";
			break;
		}

		vec[i] = str;
	}

	return vec;
}

} // namespace biblio
