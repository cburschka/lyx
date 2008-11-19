/**
 * \file BiblioInfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voß
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BiblioInfo.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "buffer_funcs.h"
#include "InsetIterator.h"
#include "Paragraph.h"

#include "insets/Inset.h"
#include "insets/InsetBibitem.h"
#include "insets/InsetBibtex.h"
#include "insets/InsetInclude.h"

#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include "boost/regex.hpp"

using namespace std;
using namespace lyx::support;


namespace lyx {

//////////////////////////////////////////////////////////////////////
//
// BibTeXInfo
//
//////////////////////////////////////////////////////////////////////

BibTeXInfo::BibTeXInfo(docstring const & key, docstring const & type)
	: is_bibtex_(true), bib_key_(key), entry_type_(type)
{}


bool BibTeXInfo::hasField(docstring const & field) const
{
	return count(field) == 1;
}


docstring const & BibTeXInfo::getValueForField(docstring const & field) const
{
	BibTeXInfo::const_iterator it = find(field);
	if (it != end())
		return it->second;
	static docstring const empty_value = docstring();
	return empty_value;
}
	
	
docstring const & BibTeXInfo::getValueForField(string const & field) const
{
	return getValueForField(from_ascii(field));
}


docstring familyName(docstring const & name)
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

docstring const BibTeXInfo::getAbbreviatedAuthor() const
{
	if (!is_bibtex_) {
		docstring const opt = trim(getValueForField("label"));
		if (opt.empty())
			return docstring();

		docstring authors;
		split(opt, authors, '(');
		return authors;
	}

	docstring author = getValueForField("author");
	if (author.empty()) {
		author = getValueForField("editor");
		if (author.empty())
			return bib_key_;
	}

	// OK, we've got some names. Let's format them.
	// Try to split the author list on " and "
	vector<docstring> const authors =
		getVectorFromString(author, from_ascii(" and "));

	if (authors.size() == 2)
		return bformat(_("%1$s and %2$s"),
			familyName(authors[0]), familyName(authors[1]));

	if (authors.size() > 2)
		return bformat(_("%1$s et al."), familyName(authors[0]));

	return familyName(authors[0]);
}


docstring const BibTeXInfo::getYear() const
{
	if (!is_bibtex_) {
		docstring const opt = trim(getValueForField("label"));
		if (opt.empty())
			return docstring();

		docstring authors;
		docstring const tmp = split(opt, authors, '(');
		docstring year;
		split(tmp, year, ')');
		return year;
	}

	docstring year = getValueForField("year");
	if (year.empty())
		year = _("No year");
	return year;
}


docstring const BibTeXInfo::getInfo() const
{
	if (!is_bibtex_) {
		BibTeXInfo::const_iterator it = find(from_ascii("ref"));
		return it->second;
	}
 
	// FIXME
	// This could be made a lot better using the entryType
	// field to customize the output based upon entry type.
	
	// Search for all possible "required" fields
	docstring author = getValueForField("author");
	if (author.empty())
		author = getValueForField("editor");
 
	docstring year      = getValueForField("year");
	docstring title     = getValueForField("title");
	docstring docLoc    = getValueForField("pages");
	if (docLoc.empty()) {
		docLoc = getValueForField("chapter");
		if (!docLoc.empty())
			docLoc = from_ascii("Ch. ") + docLoc;
	}	else {
		docLoc = from_ascii("pp. ") + docLoc;
	}

	docstring media = getValueForField("journal");
	if (media.empty()) {
		media = getValueForField("publisher");
		if (media.empty()) {
			media = getValueForField("school");
			if (media.empty())
				media = getValueForField("institution");
		}
	}
	docstring volume = getValueForField("volume");

	odocstringstream result;
	if (!author.empty())
		result << author << ", ";
	if (!title.empty())
		result << title;
	if (!media.empty())
		result << ", " << media;
	if (!year.empty())
		result << ", " << year;
	if (!docLoc.empty())
		result << ", " << docLoc;

	docstring const result_str = rtrim(result.str());
	if (!result_str.empty())
		return result_str;

	// This should never happen (or at least be very unusual!)
	return docstring();
}


//////////////////////////////////////////////////////////////////////
//
// BiblioInfo
//
//////////////////////////////////////////////////////////////////////

namespace {
// A functor for use with sort, leading to case insensitive sorting
	class compareNoCase: public binary_function<docstring, docstring, bool>
	{
		public:
			bool operator()(docstring const & s1, docstring const & s2) const {
				return compare_no_case(s1, s2) < 0;
			}
	};
} // namespace anon


vector<docstring> const BiblioInfo::getKeys() const
{
	vector<docstring> bibkeys;
	BiblioInfo::const_iterator it  = begin();
	for (; it != end(); ++it)
		bibkeys.push_back(it->first);
	sort(bibkeys.begin(), bibkeys.end(), compareNoCase());
	return bibkeys;
}


vector<docstring> const BiblioInfo::getFields() const
{
	vector<docstring> bibfields;
	set<docstring>::const_iterator it = field_names_.begin();
	set<docstring>::const_iterator end = field_names_.end();
	for (; it != end; ++it)
		bibfields.push_back(*it);
	sort(bibfields.begin(), bibfields.end());
	return bibfields;
}


vector<docstring> const BiblioInfo::getEntries() const
{
	vector<docstring> bibentries;
	set<docstring>::const_iterator it = entry_types_.begin();
	set<docstring>::const_iterator end = entry_types_.end();
	for (; it != end; ++it)
		bibentries.push_back(*it);
	sort(bibentries.begin(), bibentries.end());
	return bibentries;
}


docstring const BiblioInfo::getAbbreviatedAuthor(docstring const & key) const
{
	BiblioInfo::const_iterator it = find(key);
	if (it == end())
		return docstring();
	BibTeXInfo const & data = it->second;
	return data.getAbbreviatedAuthor();
}


docstring const BiblioInfo::getYear(docstring const & key) const
{
	BiblioInfo::const_iterator it = find(key);
	if (it == end())
		return docstring();
	BibTeXInfo const & data = it->second;
	return data.getYear();
}


docstring const BiblioInfo::getInfo(docstring const & key) const
{
	BiblioInfo::const_iterator it = find(key);
	if (it == end())
		return docstring();
	BibTeXInfo const & data = it->second;
	return data.getInfo();
}


vector<docstring> const BiblioInfo::getCiteStrings(
	docstring const & key, Buffer const & buf) const
{
	CiteEngine const engine = buf.params().citeEngine();
	if (engine == ENGINE_BASIC || engine == ENGINE_NATBIB_NUMERICAL)
		return getNumericalStrings(key, buf);
	else
		return getAuthorYearStrings(key, buf);
}


vector<docstring> const BiblioInfo::getNumericalStrings(
	docstring const & key, Buffer const & buf) const
{
	if (empty())
		return vector<docstring>();

	docstring const author = getAbbreviatedAuthor(key);
	docstring const year   = getYear(key);
	if (author.empty() || year.empty())
		return vector<docstring>();

	vector<CiteStyle> const & styles = citeStyles(buf.params().citeEngine());
	
	vector<docstring> vec(styles.size());
	for (size_t i = 0; i != vec.size(); ++i) {
		docstring str;

		switch (styles[i]) {
			case CITE:
			case CITEP:
				str = from_ascii("[#ID]");
				break;

			case NOCITE:
				str = _("Add to bibliography only.");
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


vector<docstring> const BiblioInfo::getAuthorYearStrings(
	docstring const & key, Buffer const & buf) const
{
	if (empty())
		return vector<docstring>();

	docstring const author = getAbbreviatedAuthor(key);
	docstring const year   = getYear(key);
	if (author.empty() || year.empty())
		return vector<docstring>();

	vector<CiteStyle> const & styles = citeStyles(buf.params().citeEngine());
	
	vector<docstring> vec(styles.size());
	for (size_t i = 0; i != vec.size(); ++i) {
		docstring str;

		switch (styles[i]) {
			case CITE:
		// jurabib only: Author/Annotator
		// (i.e. the "before" field, 2nd opt arg)
				str = author + "/<" + _("before") + '>';
				break;

			case NOCITE:
				str = _("Add to bibliography only.");
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


void BiblioInfo::mergeBiblioInfo(BiblioInfo const & info)
{
	bimap_.insert(info.begin(), info.end());
}


//////////////////////////////////////////////////////////////////////
//
// CitationStyle
//
//////////////////////////////////////////////////////////////////////

namespace {


char const * const citeCommands[] = {
	"cite", "citet", "citep", "citealt", "citealp",
	"citeauthor", "citeyear", "citeyearpar", "nocite" };

unsigned int const nCiteCommands =
		sizeof(citeCommands) / sizeof(char *);

CiteStyle const citeStylesArray[] = {
	CITE, CITET, CITEP, CITEALT, CITEALP, 
	CITEAUTHOR, CITEYEAR, CITEYEARPAR, NOCITE };

unsigned int const nCiteStyles =
		sizeof(citeStylesArray) / sizeof(CiteStyle);

CiteStyle const citeStylesFull[] = {
	CITET, CITEP, CITEALT, CITEALP, CITEAUTHOR };

unsigned int const nCiteStylesFull =
		sizeof(citeStylesFull) / sizeof(CiteStyle);

CiteStyle const citeStylesUCase[] = {
	CITET, CITEP, CITEALT, CITEALP, CITEAUTHOR };

unsigned int const nCiteStylesUCase =
	sizeof(citeStylesUCase) / sizeof(CiteStyle);

} // namespace anon


CitationStyle citationStyleFromString(string const & command)
{
	CitationStyle s;
	if (command.empty())
		return s;

	string cmd = command;
	if (cmd[0] == 'C') {
		s.forceUpperCase = true;
		cmd[0] = 'c';
	}

	size_t const n = cmd.size() - 1;
	if (cmd != "cite" && cmd[n] == '*') {
		s.full = true;
		cmd = cmd.substr(0, n);
	}

	char const * const * const last = citeCommands + nCiteCommands;
	char const * const * const ptr = find(citeCommands, last, cmd);

	if (ptr != last) {
		size_t idx = ptr - citeCommands;
		s.style = citeStylesArray[idx];
	}
	return s;
}


string citationStyleToString(const CitationStyle & s)
{
	string cite = citeCommands[s.style];
	if (s.full) {
		CiteStyle const * last = citeStylesFull + nCiteStylesFull;
		if (find(citeStylesFull, last, s.style) != last)
			cite += '*';
	}

	if (s.forceUpperCase) {
		CiteStyle const * last = citeStylesUCase + nCiteStylesUCase;
		if (find(citeStylesUCase, last, s.style) != last)
			cite[0] = 'C';
	}

	return cite;
}

vector<CiteStyle> citeStyles(CiteEngine engine)
{
	unsigned int nStyles = 0;
	unsigned int start = 0;

	switch (engine) {
		case ENGINE_BASIC:
			nStyles = 2;
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

	vector<CiteStyle> styles(nStyles);
	size_t i = 0;
	int j = start;
	for (; i != styles.size(); ++i, ++j)
		styles[i] = citeStylesArray[j];

	return styles;
}

} // namespace lyx

