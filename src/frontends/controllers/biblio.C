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

string const familyName(string const & name)
{
	// Very simple parser
	string fname = name;

	string::size_type idx = fname.rfind(".");
	if (idx != string::npos)
		fname = frontStrip(fname.substr(idx+1));

	return fname;
}


string const getAbbreviatedAuthor(InfoMap const & map, string const & key)
{
	lyx::Assert(!map.empty());

       	InfoMap::const_iterator it = map.find(key);

	string author;
	if (it != map.end()) {
		author = parseBibTeX(it->second, "author");
		if (author.empty())
			author = parseBibTeX(it->second, "editor");

		vector<string> authors = getVectorFromString(author, "and");

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
	}

	if (author.empty())
		author = _("Caesar et al.");

	return author;
}


string const getYear(InfoMap const & map, string const & key)
{
	lyx::Assert(!map.empty());

       	InfoMap::const_iterator it = map.find(key);

	string year;

	if (it != map.end())
		year = parseBibTeX(it->second, "year");

	if (year.empty())
		year = "50BC";

	return year;
}

} // namespace anon 







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
	if (it == map.end()) return string();

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

	if (result.str().empty()) // not a BibTeX record
		result << it->second;

	return result.str().c_str();
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
	if(start < keys.begin() || start >= keys.end())
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
	for (string::iterator it = data.begin(); it < data.end(); ++it) {
		if ((*it) == '\n' || (*it) == '\t')
			(*it)= ' ';
	}
	data = frontStrip(data);
	
	// now get only the important line of the bibtex entry.
	// all entries are devided by ',' except the last one.	
	data += ',';  // now we have same behaviour for all entries
		      // because the last one is "blah ... }"
	int Entries = 0;			
	string dummy = token(data, ',', Entries);
	while (!contains(lowercase(dummy), findkey) && !dummy.empty())
		dummy = token(data, ',', ++Entries);
	if (dummy.empty())
		return string();			// no such keyword
	// we are not sure, if we get all, because "key= "blah, blah" is allowed.
	// therefore we read all until the next "=" character, which follows a
	// new keyword
	keyvalue = dummy;
	dummy = token(data, ',', ++Entries);
	while (!contains(dummy, '=') && !dummy.empty()) {
		keyvalue += (',' + dummy);
		dummy = token(data, ',', ++Entries);
	}
	data = keyvalue;		// now we have the important line    	
	data = strip(data, ' ');		// all spaces
	if (!contains(data, '{'))	// no opening '{'
		data = strip(data, '}');// maybe there is a main closing '}'
	// happens, when last keyword
	string key = lowercase(data.substr(0, data.find('=')));
	data = data.substr(data.find('='), data.length() - 1);
	data = frontStrip(strip(data));
	if (data.length() < 2 || data[0] != '=') {	// a valid entry?
		return string();
	} else {
		data = frontStrip(data.substr(1, data.length() - 1));
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
				return data;	// no {} and no "", pure data
			}
			string tmp = data.substr(keypos, data.length()-1);
			while (tmp.find('{') != string::npos &&
			       tmp.find('}') != string::npos &&
			       tmp.find('{') < tmp.find('}') &&
			       tmp.find('{') < tmp.find(enclosing)) {
				
				keypos += tmp.find('{') + 1;
				tmp = data.substr(keypos, data.length() - 1);
				keypos += tmp.find('}') + 1;
				tmp = data.substr(keypos, data.length() - 1);
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
		vector<string> vec(1);
		vec[0] = _("No database");
		return vec;
	}
	
	vector<string> vec(styles.size());

	string const author = getAbbreviatedAuthor(map, key);
	string const year   = getYear(map, key);
	
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
		vector<string> vec(1);
		vec[0] = _("No database");
		return vec;
	}
	
	vector<string> vec(styles.size());

	string const author = getAbbreviatedAuthor(map, key);
	string const year   = getYear(map, key);
	
	for (vector<string>::size_type i = 0; i != vec.size(); ++i) {
		string str;

		switch (styles[i]) {
		case CITET:
			str = author + " (" + year + ")";
			break;
			
		case CITE:
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
