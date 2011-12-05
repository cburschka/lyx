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
#include "Encoding.h"
#include "InsetIterator.h"
#include "Language.h"
#include "Paragraph.h"
#include "TextClass.h"
#include "TocBackend.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/regex.h"
#include "support/textutils.h"

#include <set>

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

		// was the last character a \? If so, then this is something like:
		// \\ or \$, so we'll just output it. That's probably not always right...
		if (escaped) {
			// exception: output \, as THIN SPACE
			if (ch == ',')
				ret.push_back(0x2009);
			else
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
		static lyx::regex const reg("^\\\\\\W\\w");
		if (lyx::regex_search(to_utf8(val), reg)) {
			val.insert(3, from_ascii("}"));
			val.insert(2, from_ascii("{"));
		}
		docstring rem;
		docstring const cnvtd = Encodings::fromLaTeXCommand(val, rem,
							Encodings::TEXT_CMD);
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
	: is_bibtex_(true), bib_key_(key), entry_type_(type), info_(),
	  modifier_(0)
{}


docstring const BibTeXInfo::getAbbreviatedAuthor() const
{
	if (!is_bibtex_) {
		docstring const opt = label();
		if (opt.empty())
			return docstring();

		docstring authors;
		docstring const remainder = trim(split(opt, authors, '('));
		if (remainder.empty())
			// in this case, we didn't find a "(",
			// so we don't have author (year)
			return docstring();
		return authors;
	}

	docstring author = convertLaTeXCommands(operator[]("author"));
	if (author.empty()) {
		author = convertLaTeXCommands(operator[]("editor"));
		if (author.empty())
			return bib_key_;
	}

	// FIXME Move this to a separate routine that can
	// be called from elsewhere.
	//
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

	docstring const opt = label();
	if (opt.empty())
		return docstring();

	docstring authors;
	docstring tmp = split(opt, authors, '(');
	if (tmp.empty())
		// we don't have author (year)
		return docstring();
	docstring year;
	tmp = split(tmp, year, ')');
	return year;
}


docstring const BibTeXInfo::getXRef() const
{
	if (!is_bibtex_)
		return docstring();
	return operator[]("crossref");
}


namespace {
	string parseOptions(string const & format, string & optkey,
			string & ifpart, string & elsepart);

	// Calls parseOptions to deal with an embedded option, such as:
	//   {%number%[[, no.~%number%]]}
	// which must appear at the start of format. ifelsepart gets the
	// whole of the option, and we return what's left after the option.
	// we return format if there is an error.
	string parseEmbeddedOption(string const & format, string & ifelsepart)
	{
		LASSERT(format[0] == '{' && format[1] == '%', return format);
		string optkey;
		string ifpart;
		string elsepart;
		string const rest = parseOptions(format, optkey, ifpart, elsepart);
		if (format == rest) { // parse error
			LYXERR0("ERROR! Couldn't parse `" << format <<"'.");
			return format;
		}
		LASSERT(rest.size() <= format.size(), /* */);
		ifelsepart = format.substr(0, format.size() - rest.size());
		return rest;
	}


	// Gets a "clause" from a format string, where the clause is
	// delimited by '[[' and ']]'. Returns what is left after the
	// clause is removed, and returns format if there is an error.
	string getClause(string const & format, string & clause)
	{
		string fmt = format;
		// remove '[['
		fmt = fmt.substr(2);
		// we'll remove characters from the front of fmt as we
		// deal with them
		while (fmt.size()) {
			if (fmt[0] == ']' && fmt.size() > 1 && fmt[1] == ']') {
				// that's the end
				fmt = fmt.substr(2);
				break;
			}
			// check for an embedded option
			if (fmt[0] == '{' && fmt.size() > 1 && fmt[1] == '%') {
				string part;
				string const rest = parseEmbeddedOption(fmt, part);
				if (fmt == rest) {
					LYXERR0("ERROR! Couldn't parse embedded option in `" << format <<"'.");
					return format;
				}
				clause += part;
				fmt = rest;
			} else { // it's just a normal character
				clause += fmt[0];
				fmt = fmt.substr(1);
			}
		}
		return fmt;
	}


	// parse an options string, which must appear at the start of the
	// format parameter. puts the parsed bits in optkey, ifpart, and
	// elsepart and returns what's left after the option is removed.
	// if there's an error, it returns format itself.
	string parseOptions(string const & format, string & optkey,
			string & ifpart, string & elsepart)
	{
		LASSERT(format[0] == '{' && format[1] == '%', return format);
		// strip '{%'
		string fmt = format.substr(2);
		size_t pos = fmt.find('%'); // end of key
		if (pos == string::npos) {
			LYXERR0("Error parsing  `" << format <<"'. Can't find end of key.");
			return format;
		}
		optkey = fmt.substr(0,pos);
		fmt = fmt.substr(pos + 1);
		// [[format]] should be next
		if (fmt[0] != '[' || fmt[1] != '[') {
			LYXERR0("Error parsing  `" << format <<"'. Can't find '[[' after key.");
			return format;
		}

		string curfmt = fmt;
		fmt = getClause(curfmt, ifpart);
		if (fmt == curfmt) {
			LYXERR0("Error parsing  `" << format <<"'. Couldn't get if clause.");
			return format;
		}

		if (fmt[0] == '}') // we're done, no else clause
			return fmt.substr(1);

		// else part should follow
		if (fmt[0] != '[' || fmt[1] != '[') {
			LYXERR0("Error parsing  `" << format <<"'. Can't find else clause.");
			return format;
		}

		curfmt = fmt;
		fmt = getClause(curfmt, elsepart);
		// we should be done
		if (fmt == curfmt || fmt[0] != '}') {
			LYXERR0("Error parsing  `" << format <<"'. Can't find end of option.");
			return format;
		}
		return fmt.substr(1);
}

} // anon namespace


docstring BibTeXInfo::expandFormat(string const & format,
		BibTeXInfo const * const xref, int & counter, Buffer const & buf,
		bool richtext) const
{
	// incorrect use of macros could put us in an infinite loop
	static int max_passes = 5000;
	docstring ret; // return value
	string key;
	bool scanning_key = false;
	bool scanning_rich = false;

	string fmt = format;
	// we'll remove characters from the front of fmt as we
	// deal with them
	while (fmt.size()) {
		if (counter++ > max_passes) {
			LYXERR0("Recursion limit reached while parsing `"
			        << format << "'.");
			return _("ERROR!");
		}

		char_type thischar = fmt[0];
		if (thischar == '%') {
			// beginning or end of key
			if (scanning_key) {
				// end of key
				scanning_key = false;
				// so we replace the key with its value, which may be empty
				if (key[0] == '!') {
					// macro
					string const val =
						buf.params().documentClass().getCiteMacro(key);
					fmt = val + fmt.substr(1);
					continue;
				} else if (key[0] == '_') {
					// a translatable bit
					string const val =
						buf.params().documentClass().getCiteMacro(key);
					docstring const trans =
						translateIfPossible(from_utf8(val), buf.params().language->code());
					ret += trans;
				} else {
					docstring const val = getValueForKey(key, xref);
					ret += val;
				}
			} else {
				// beginning of key
				key.clear();
				scanning_key = true;
			}
		}
		else if (thischar == '{') {
			// beginning of option?
			if (scanning_key) {
				LYXERR0("ERROR: Found `{' when scanning key in `" << format << "'.");
				return _("ERROR!");
			}
			if (fmt.size() > 1) {
				if (fmt[1] == '%') {
					// it is the beginning of an optional format
					string optkey;
					string ifpart;
					string elsepart;
					string const newfmt =
						parseOptions(fmt, optkey, ifpart, elsepart);
					if (newfmt == fmt) // parse error
						return _("ERROR!");
					fmt = newfmt;
					docstring const val = getValueForKey(optkey, xref);
					if (!val.empty())
						ret += expandFormat(ifpart, xref, counter, buf, richtext);
					else if (!elsepart.empty())
						ret += expandFormat(elsepart, xref, counter, buf, richtext);
					// fmt will have been shortened for us already
					continue;
				}
				if (fmt[1] == '!') {
					// beginning of rich text
					scanning_rich = true;
					fmt = fmt.substr(2);
					continue;
				}
			}
			// we are here if '{' was not followed by % or !.
			// So it's just a character.
			ret += thischar;
		}
		else if (scanning_rich && thischar == '!'
		         && fmt.size() > 1 && fmt[1] == '}') {
			// end of rich text
			scanning_rich = false;
			fmt = fmt.substr(2);
			continue;
		}
		else if (scanning_key)
			key += char(thischar);
		else if (richtext) {
			if (scanning_rich)
				ret += thischar;
			else {
				// we need to escape '<' and '>'
				if (thischar == '<')
					ret += "&lt;";
				else if (thischar == '>')
					ret += "&gt;";
				else
					ret += thischar;
			}
		}	else if (!scanning_rich /* && !richtext */)
			ret += thischar;
		// else the character is discarded, which will happen only if
		// richtext == false and we are scanning rich text
		fmt = fmt.substr(1);
	} // for loop
	if (scanning_key) {
		LYXERR0("Never found end of key in `" << format << "'!");
		return _("ERROR!");
	}
	if (scanning_rich) {
		LYXERR0("Never found end of rich text in `" << format << "'!");
		return _("ERROR!");
	}
	return ret;
}


docstring const & BibTeXInfo::getInfo(BibTeXInfo const * const xref,
	Buffer const & buf, bool richtext) const
{
	if (!info_.empty())
		return info_;

	if (!is_bibtex_) {
		BibTeXInfo::const_iterator it = find(from_ascii("ref"));
		info_ = it->second;
		return info_;
	}

	DocumentClass const & dc = buf.params().documentClass();
	string const & format = dc.getCiteFormat(to_utf8(entry_type_));
	int counter = 0;
	info_ = expandFormat(format, xref, counter, buf, richtext);

	if (!info_.empty())
		info_ = convertLaTeXCommands(info_);
	return info_;
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
	if (!ret.empty() || !xref)
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


docstring const BiblioInfo::getCiteNumber(docstring const & key) const
{
	BiblioInfo::const_iterator it = find(key);
	if (it == end())
		return docstring();
	BibTeXInfo const & data = it->second;
	return data.citeNumber();
}


docstring const BiblioInfo::getYear(docstring const & key, bool use_modifier) const
{
	BiblioInfo::const_iterator it = find(key);
	if (it == end())
		return docstring();
	BibTeXInfo const & data = it->second;
	docstring year = data.getYear();
	if (year.empty()) {
		// let's try the crossref
		docstring const xref = data.getXRef();
		if (xref.empty())
			return _("No year"); // no luck
		BiblioInfo::const_iterator const xrefit = find(xref);
		if (xrefit == end())
			return _("No year"); // no luck again
		BibTeXInfo const & xref_data = xrefit->second;
		year = xref_data.getYear();
	}
	if (use_modifier && data.modifier() != 0)
		year += data.modifier();
	return year;
}


docstring const BiblioInfo::getInfo(docstring const & key,
	Buffer const & buf, bool richtext) const
{
	BiblioInfo::const_iterator it = find(key);
	if (it == end())
		return docstring(_("Bibliography entry not found!"));
	BibTeXInfo const & data = it->second;
	BibTeXInfo const * xrefptr = 0;
	docstring const xref = data.getXRef();
	if (!xref.empty()) {
		BiblioInfo::const_iterator const xrefit = find(xref);
		if (xrefit != end())
			xrefptr = &(xrefit->second);
	}
	return data.getInfo(xrefptr, buf, richtext);
}


bool BiblioInfo::isBibtex(docstring const & key) const
{
	BiblioInfo::const_iterator it = find(key);
	if (it == end())
		return false;
	return it->second.isBibTeX();
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
	field_names_.insert(info.field_names_.begin(), info.field_names_.end());
	entry_types_.insert(info.entry_types_.begin(), info.entry_types_.end());
}


namespace {
	// used in xhtml to sort a list of BibTeXInfo objects
	bool lSorter(BibTeXInfo const * lhs, BibTeXInfo const * rhs)
	{
		docstring const lauth = lhs->getAbbreviatedAuthor();
		docstring const rauth = rhs->getAbbreviatedAuthor();
		docstring const lyear = lhs->getYear();
		docstring const ryear = rhs->getYear();
		docstring const ltitl = lhs->operator[]("title");
		docstring const rtitl = rhs->operator[]("title");
		return  (lauth < rauth)
				|| (lauth == rauth && lyear < ryear)
				|| (lauth == rauth && lyear == ryear && ltitl < rtitl);
	}
}


void BiblioInfo::collectCitedEntries(Buffer const & buf)
{
	cited_entries_.clear();
	// We are going to collect all the citation keys used in the document,
	// getting them from the TOC.
	// FIXME We may want to collect these differently, in the first case,
	// so that we might have them in order of appearance.
	set<docstring> citekeys;
	Toc const & toc = buf.tocBackend().toc("citation");
	Toc::const_iterator it = toc.begin();
	Toc::const_iterator const en = toc.end();
	for (; it != en; ++it) {
		if (it->str().empty())
			continue;
		vector<docstring> const keys = getVectorFromString(it->str());
		citekeys.insert(keys.begin(), keys.end());
	}
	if (citekeys.empty())
		return;

	// We have a set of the keys used in this document.
	// We will now convert it to a list of the BibTeXInfo objects used in
	// this document...
	vector<BibTeXInfo const *> bi;
	set<docstring>::const_iterator cit = citekeys.begin();
	set<docstring>::const_iterator const cen = citekeys.end();
	for (; cit != cen; ++cit) {
		BiblioInfo::const_iterator const bt = find(*cit);
		if (bt == end() || !bt->second.isBibTeX())
			continue;
		bi.push_back(&(bt->second));
	}
	// ...and sort it.
	sort(bi.begin(), bi.end(), lSorter);

	// Now we can write the sorted keys
	vector<BibTeXInfo const *>::const_iterator bit = bi.begin();
	vector<BibTeXInfo const *>::const_iterator ben = bi.end();
	for (; bit != ben; ++bit)
		cited_entries_.push_back((*bit)->key());
}


void BiblioInfo::makeCitationLabels(Buffer const & buf)
{
	collectCitedEntries(buf);
	CiteEngine const engine = buf.params().citeEngine();
	bool const numbers =
		(engine == ENGINE_BASIC || engine == ENGINE_NATBIB_NUMERICAL);

	int keynumber = 0;
	char modifier = 0;
	// used to remember the last one we saw
	// we'll be comparing entries to see if we need to add
	// modifiers, like "1984a"
	map<docstring, BibTeXInfo>::iterator last;

	vector<docstring>::const_iterator it = cited_entries_.begin();
	vector<docstring>::const_iterator const en = cited_entries_.end();
	for (; it != en; ++it) {
		map<docstring, BibTeXInfo>::iterator const biit = bimap_.find(*it);
		// this shouldn't happen, but...
		if (biit == bimap_.end())
			// ...fail gracefully, anyway.
			continue;
		BibTeXInfo & entry = biit->second;
		if (numbers) {
			docstring const num = convert<docstring>(++keynumber);
			entry.setCiteNumber(num);
		} else {
			if (it != cited_entries_.begin()
			    && entry.getAbbreviatedAuthor() == last->second.getAbbreviatedAuthor()
			    // we access the year via getYear() so as to get it from the xref,
			    // if we need to do so
			    && getYear(entry.key()) == getYear(last->second.key())) {
				if (modifier == 0) {
					// so the last one should have been 'a'
					last->second.setModifier('a');
					modifier = 'b';
				} else if (modifier == 'z')
					modifier = 'A';
				else
					modifier++;
			} else {
				modifier = 0;
			}
			entry.setModifier(modifier);
			// remember the last one
			last = biit;
		}
	}
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
		if (std::find(citeStylesFull, last, s.style) != last)
			cite += '*';
	}

	if (s.forceUpperCase) {
		CiteStyle const * last = citeStylesUCase + nCiteStylesUCase;
		if (std::find(citeStylesUCase, last, s.style) != last)
			cite[0] = 'C';
	}

	return cite;
}

vector<CiteStyle> citeStyles(CiteEngine engine)
{
	vector<CiteStyle> styles(0);

	switch (engine) {
		case ENGINE_BASIC:
			styles.push_back(CITE);
			break;
		case ENGINE_JURABIB:
			styles.push_back(CITE);
		case ENGINE_NATBIB_AUTHORYEAR:
			styles.push_back(CITET);
			styles.push_back(CITEP);
			styles.push_back(CITEALT);
			styles.push_back(CITEALP);
			styles.push_back(CITEAUTHOR);
			styles.push_back(CITEYEAR);
			styles.push_back(CITEYEARPAR);
			break;
		case ENGINE_NATBIB_NUMERICAL:
			styles.push_back(CITET);
			styles.push_back(CITEALT);
			styles.push_back(CITEAUTHOR);
			styles.push_back(CITEP);
			styles.push_back(CITEALP);
			styles.push_back(CITEYEAR);
			styles.push_back(CITEYEARPAR);
			break;
	}

	styles.push_back(NOCITE);

	return styles;
}

} // namespace lyx

