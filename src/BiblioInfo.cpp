/**
 * \file BiblioInfo.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 * \author Herbert Voﬂ
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BiblioInfo.h"
#include "Buffer.h"
#include "BufferParams.h"
#include "buffer_funcs.h"
#include "Encoding.h"
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
#include "support/textutils.h"

#include "boost/regex.hpp"

using namespace std;
using namespace lyx::support;


namespace lyx {

namespace {

// gets the "family name" from an author-type string
docstring familyName(docstring const & name)
{
	if (name.empty())
		return docstring();

	// first we look for a comma, and take the last name to be everything
	// preceding the right-most one, so that we also get the "jr" part.
	docstring::size_type idx = name.rfind(',');
	if (idx != docstring::npos)
		return ltrim(name.substr(0, idx));

	// OK, so now we want to look for the last name. We're going to
	// include the "von" part. This isn't perfect.
	// Split on spaces, to get various tokens.
	vector<docstring> pieces = getVectorFromString(name, from_ascii(" "));
	// If we only get two, assume the last one is the last name
	if (pieces.size() <= 2)
		return pieces.back();

	// Now we look for the first token that begins with a lower case letter.
	vector<docstring>::const_iterator it = pieces.begin();
	vector<docstring>::const_iterator en = pieces.end();
	for (; it != en; ++it) {
		if ((*it).size() == 0)
			continue;
		char_type const c = (*it)[0];
		if (isLower(c))
			break;
	}

	if (it == en) // we never found a "von"
		return pieces.back();

	// reconstruct what we need to return
	docstring retval;
	bool first = true;
	for (; it != en; ++it) {
		if (!first)
			retval += " ";
		else 
			first = false;
		retval += *it;
	}
	return retval;
}


// converts a string containing LaTeX commands into unicode
// for display.
docstring convertLaTeXCommands(docstring const & str)
	{
		docstring val = str;
		docstring ret;
	
		bool scanning_cmd = false;
		bool scanning_math = false;
		bool escaped = false; // used to catch \$, etc.
		while (val.size()) {
			char_type const ch = val[0];

			// if we're scanning math, we output everything until we
			// find an unescaped $, at which point we break out.
			if (scanning_math) {
				if (escaped)
					escaped = false;
				else if (ch == '\\')
					escaped = true;
				else if (ch == '$') 
					scanning_math = false;
				ret += ch;
				val = val.substr(1);
				continue;
			}

			// if we're scanning a command name, then we just
			// discard characters until we hit something that
			// isn't alpha.
			if (scanning_cmd) {
				if (isAlphaASCII(ch)) {
					val = val.substr(1);
					escaped = false;
					continue;
				}
				// so we're done with this command.
				// now we fall through and check this character.
				scanning_cmd = false;
			}

			// was the last character a \? If so, then this is something like: \\,
			// or \$, so we'll just output it. That's probably not always right...
			if (escaped) {
				ret += ch;
				val = val.substr(1);
				escaped = false;
				continue;
			}

			if (ch == '$') {
				ret += ch;
				val = val.substr(1);
				scanning_math = true;
				continue;
			}

			// we just ignore braces
			if (ch == '{' || ch == '}') {
				val = val.substr(1);
				continue;
			}

			// we're going to check things that look like commands, so if
			// this doesn't, just output it.
			if (ch != '\\') {
				ret += ch;
				val = val.substr(1);
				continue;
			}

			// ok, could be a command of some sort
			// let's see if it corresponds to some unicode
			// unicodesymbols has things in the form: \"{u},
			// whereas we may see things like: \"u. So we'll
			// look for that and change it, if necessary.
			static boost::regex const reg("^\\\\\\W\\w");
			if (boost::regex_search(to_utf8(val), reg)) {
				val.insert(3, from_ascii("}"));
				val.insert(2, from_ascii("{"));
			}
			docstring rem;
			docstring const cnvtd = Encodings::fromLaTeXCommand(val, rem);
			if (!cnvtd.empty()) {
				// it did, so we'll take that bit and proceed with what's left
				ret += cnvtd;
				val = rem;
				continue;
			}
			// it's a command of some sort
			scanning_cmd = true;
			escaped = true;
			val = val.substr(1);
		}
		return ret;
	}

} // anon namespace


//////////////////////////////////////////////////////////////////////
//
// BibTeXInfo
//
//////////////////////////////////////////////////////////////////////

BibTeXInfo::BibTeXInfo(docstring const & key, docstring const & type)
	: is_bibtex_(true), bib_key_(key), entry_type_(type), info_()
{}


bool BibTeXInfo::hasField(docstring const & field) const
{
	return count(field) == 1;
}


docstring const BibTeXInfo::getAbbreviatedAuthor() const
{
	if (!is_bibtex_) {
		docstring const opt = trim(operator[]("label"));
		if (opt.empty())
			return docstring();

		docstring authors;
		split(opt, authors, '(');
		return authors;
	}

	docstring author = operator[]("author");
	if (author.empty()) {
		author = operator[]("editor");
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
	if (is_bibtex_) 
		return operator[]("year");

	docstring const opt = trim(operator[]("label"));
	if (opt.empty())
		return docstring();

	docstring authors;
	docstring const tmp = split(opt, authors, '(');
	docstring year;
	split(tmp, year, ')');
	return year;
}


docstring const BibTeXInfo::getXRef() const
{
	if (!is_bibtex_)
		return docstring();
	return operator[]("crossref");
}


docstring const & BibTeXInfo::getInfo(BibTeXInfo const * const xref) const
{
	if (!info_.empty())
		return info_;

	if (!is_bibtex_) {
		BibTeXInfo::const_iterator it = find(from_ascii("ref"));
		info_ = it->second;
		return info_;
	}
 
	// FIXME
	// This could be made a lot better using the entry_type_
	// field to customize the output based upon entry type.
	
	// Search for all possible "required" fields
	docstring author = getValueForKey("author", xref);
	if (author.empty())
		author = getValueForKey("editor", xref);
 
	docstring year   = getValueForKey("year", xref);
	docstring title  = getValueForKey("title", xref);
	docstring docLoc = getValueForKey("pages", xref);
	if (docLoc.empty()) {
		docLoc = getValueForKey("chapter", xref);
		if (!docLoc.empty())
			docLoc = _("Ch. ") + docLoc;
	}	else {
		docLoc = _("pp. ") + docLoc;
	}

	docstring media = getValueForKey("journal", xref);
	if (media.empty()) {
		media = getValueForKey("publisher", xref);
		if (media.empty()) {
			media = getValueForKey("school", xref);
			if (media.empty())
				media = getValueForKey("institution");
		}
	}
	docstring volume = getValueForKey("volume", xref);

	odocstringstream result;
	if (!author.empty())
		result << author << ", ";
	if (!title.empty())
		result << title;
	if (!media.empty())
		result << ", " << media;
	if (!year.empty())
		result << " (" << year << ")";
	if (!docLoc.empty())
		result << ", " << docLoc;

	docstring const result_str = rtrim(result.str());
	if (!result_str.empty()) {
		info_ = convertLaTeXCommands(result_str);
		return info_;
	}

	// This should never happen (or at least be very unusual!)
	static docstring e = docstring();
	return e;
}


docstring const & BibTeXInfo::operator[](docstring const & field) const
{
	BibTeXInfo::const_iterator it = find(field);
	if (it != end())
		return it->second;
	static docstring const empty_value = docstring();
	return empty_value;
}
	
	
docstring const & BibTeXInfo::operator[](string const & field) const
{
	return operator[](from_ascii(field));
}


docstring BibTeXInfo::getValueForKey(string const & key, 
		BibTeXInfo const * const xref) const
{
	docstring const ret = operator[](key);
	if (!ret.empty() or !xref)
		return ret;
	return (*xref)[key];
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
	docstring year = data.getYear();
	if (!year.empty())
		return year;
	// let's try the crossref
	docstring const xref = data.getXRef();
	if (xref.empty())
		return _("No year"); // no luck
	BiblioInfo::const_iterator const xrefit = find(xref);
	if (xrefit == end())
		return _("No year"); // no luck again
	BibTeXInfo const & xref_data = xrefit->second;
	return xref_data.getYear();
	return data.getYear();
}


docstring const BiblioInfo::getInfo(docstring const & key) const
{
	BiblioInfo::const_iterator it = find(key);
	if (it == end())
		return docstring();
	BibTeXInfo const & data = it->second;
	BibTeXInfo const * xrefptr = 0;
	docstring const xref = data.getXRef();
	if (!xref.empty()) {
		BiblioInfo::const_iterator const xrefit = find(xref);
		if (xrefit != end())
			xrefptr = &(xrefit->second);
	}
	return data.getInfo(xrefptr);
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

