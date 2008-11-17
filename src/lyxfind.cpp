/**
 * \file lyxfind.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jürgen Vigna
 * \author Alfredo Braunstein
 * \author Tommaso Cucinotta
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxfind.h"

#include "Buffer.h"
#include "buffer_funcs.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Changes.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "FuncRequest.h"
#include "OutputParams.h"
#include "output_latex.h"
#include "Paragraph.h"
#include "ParIterator.h"
#include "TexRow.h"
#include "Text.h"

#include "mathed/InsetMath.h"
#include "mathed/InsetMathGrid.h"
#include "mathed/InsetMathHull.h"
#include "mathed/MathStream.h"

#include "frontends/alert.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

#include <boost/regex.hpp>

using namespace std;
using namespace lyx::support;

namespace lyx {

namespace {

bool parse_bool(docstring & howto)
{
	if (howto.empty())
		return false;
	docstring var;
	howto = split(howto, var, ' ');
	return var == "1";
}


class MatchString : public binary_function<Paragraph, pos_type, bool>
{
public:
	MatchString(docstring const & str, bool cs, bool mw)
		: str(str), cs(cs), mw(mw)
	{}

	// returns true if the specified string is at the specified position
	// del specifies whether deleted strings in ct mode will be considered
	bool operator()(Paragraph const & par, pos_type pos, bool del = true) const
	{
		return par.find(str, cs, mw, pos, del);
	}

private:
	// search string
	docstring str;
	// case sensitive
	bool cs;
	// match whole words only
	bool mw;
};


bool findForward(DocIterator & cur, MatchString const & match,
		 bool find_del = true)
{
	for (; cur; cur.forwardChar())
		if (cur.inTexted() &&
		    match(cur.paragraph(), cur.pos(), find_del))
			return true;
	return false;
}


bool findBackwards(DocIterator & cur, MatchString const & match,
		 bool find_del = true)
{
	while (cur) {
		cur.backwardChar();
		if (cur.inTexted() &&
		    match(cur.paragraph(), cur.pos(), find_del))
			return true;
	}
	return false;
}


bool findChange(DocIterator & cur)
{
	for (; cur; cur.forwardPos())
		if (cur.inTexted() && !cur.paragraph().isUnchanged(cur.pos()))
			return true;
	return false;
}


bool searchAllowed(BufferView * /*bv*/, docstring const & str)
{
	if (str.empty()) {
		frontend::Alert::error(_("Search error"), _("Search string is empty"));
		return false;
	}
	return true;
}


bool find(BufferView * bv, docstring const & searchstr,
	bool cs, bool mw, bool fw, bool find_del = true)
{
	if (!searchAllowed(bv, searchstr))
		return false;

	DocIterator cur = bv->cursor();

	MatchString const match(searchstr, cs, mw);

	bool found = fw ? findForward(cur, match, find_del) :
			  findBackwards(cur, match, find_del);

	if (found)
		bv->putSelectionAt(cur, searchstr.length(), !fw);

	return found;
}


int replaceAll(BufferView * bv,
	       docstring const & searchstr, docstring const & replacestr,
	       bool cs, bool mw)
{
	Buffer & buf = bv->buffer();

	if (!searchAllowed(bv, searchstr) || buf.isReadonly())
		return 0;

	MatchString const match(searchstr, cs, mw);
	int num = 0;

	int const rsize = replacestr.size();
	int const ssize = searchstr.size();

	Cursor cur(*bv);
	cur.setCursor(doc_iterator_begin(&buf));
	while (findForward(cur, match, false)) {
		// Backup current cursor position and font.
		pos_type const pos = cur.pos();
		Font const font = cur.paragraph().getFontSettings(buf.params(), pos);
		cur.recordUndo();
		int striked = ssize - cur.paragraph().eraseChars(pos, pos + ssize,
							    buf.params().trackChanges);
		cur.paragraph().insert(pos, replacestr, font,
				       Change(buf.params().trackChanges ?
					      Change::INSERTED : Change::UNCHANGED));
		for (int i = 0; i < rsize + striked; ++i)
			cur.forwardChar();
		++num;
	}

	buf.updateLabels();
	bv->putSelectionAt(doc_iterator_begin(&buf), 0, false);
	if (num)
		buf.markDirty();
	return num;
}


bool stringSelected(BufferView * bv, docstring const & searchstr,
		    bool cs, bool mw, bool fw)
{
	// if nothing selected or selection does not equal search
	// string search and select next occurance and return
	docstring const & str1 = searchstr;
	docstring const str2 = bv->cursor().selectionAsString(false);
	if ((cs && str1 != str2) || compare_no_case(str1, str2) != 0) {
		find(bv, searchstr, cs, mw, fw);
		return false;
	}

	return true;
}


int replace(BufferView * bv, docstring const & searchstr,
	    docstring const & replacestr, bool cs, bool mw, bool fw)
{
	if (!searchAllowed(bv, searchstr) || bv->buffer().isReadonly())
		return 0;

	if (!stringSelected(bv, searchstr, cs, mw, fw))
		return 0;

	Cursor & cur = bv->cursor();
	cap::replaceSelectionWithString(cur, replacestr, fw);
	bv->buffer().markDirty();
	find(bv, searchstr, cs, mw, fw, false);
	bv->buffer().updateMacros();
	bv->processUpdateFlags(Update::Force | Update::FitCursor);

	return 1;
}

} // namespace anon


docstring const find2string(docstring const & search,
			 bool casesensitive, bool matchword, bool forward)
{
	odocstringstream ss;
	ss << search << '\n'
	   << int(casesensitive) << ' '
	   << int(matchword) << ' '
	   << int(forward);
	return ss.str();
}


docstring const replace2string(docstring const & search, docstring const & replace,
			    bool casesensitive, bool matchword,
			    bool all, bool forward)
{
	odocstringstream ss;
	ss << search << '\n'
	   << replace << '\n'
	   << int(casesensitive) << ' '
	   << int(matchword) << ' '
	   << int(all) << ' '
	   << int(forward);
	return ss.str();
}


bool find(BufferView * bv, FuncRequest const & ev)
{
	if (!bv || ev.action != LFUN_WORD_FIND)
		return false;

	//lyxerr << "find called, cmd: " << ev << endl;

	// data is of the form
	// "<search>
	//  <casesensitive> <matchword> <forward>"
	docstring search;
	docstring howto = split(ev.argument(), search, '\n');

	bool casesensitive = parse_bool(howto);
	bool matchword     = parse_bool(howto);
	bool forward       = parse_bool(howto);

	return find(bv, search, casesensitive, matchword, forward);
}


void replace(BufferView * bv, FuncRequest const & ev, bool has_deleted)
{
	if (!bv || ev.action != LFUN_WORD_REPLACE)
		return;

	// data is of the form
	// "<search>
	//  <replace>
	//  <casesensitive> <matchword> <all> <forward>"
	docstring search;
	docstring rplc;
	docstring howto = split(ev.argument(), search, '\n');
	howto = split(howto, rplc, '\n');

	bool casesensitive = parse_bool(howto);
	bool matchword     = parse_bool(howto);
	bool all           = parse_bool(howto);
	bool forward       = parse_bool(howto);

	if (!has_deleted) {
		int const replace_count = all
			? replaceAll(bv, search, rplc, casesensitive, matchword)
			: replace(bv, search, rplc, casesensitive, matchword, forward);
	
		Buffer & buf = bv->buffer();
		if (replace_count == 0) {
			// emit message signal.
			buf.message(_("String not found!"));
		} else {
			if (replace_count == 1) {
				// emit message signal.
				buf.message(_("String has been replaced."));
			} else {
				docstring str = convert<docstring>(replace_count);
				str += _(" strings have been replaced.");
				// emit message signal.
				buf.message(str);
			}
		}
	} else {
		// if we have deleted characters, we do not replace at all, but
		// rather search for the next occurence
		if (find(bv, search, casesensitive, matchword, forward))
			bv->showCursor();
		else
			bv->message(_("String not found!"));
	}
}


bool findNextChange(BufferView * bv)
{
	DocIterator cur = bv->cursor();

	if (!findChange(cur))
		return false;

	bv->cursor().setCursor(cur);
	bv->cursor().resetAnchor();

	Change orig_change = cur.paragraph().lookupChange(cur.pos());

	CursorSlice & tip = cur.top();
	for (; !tip.at_end(); tip.forwardPos()) {
		Change change = tip.paragraph().lookupChange(tip.pos());
		if (change != orig_change)
			break;
	}

	// Now put cursor to end of selection:
	bv->cursor().setCursor(cur);
	bv->cursor().setSelection();

	return true;
}

namespace {

typedef vector<pair<string, string> > Escapes;

/// A map of symbols and their escaped equivalent needed within a regex.
Escapes const & get_regexp_escapes()
{
	static Escapes escape_map;
	if (escape_map.empty()) {
		escape_map.push_back(pair<string, string>("\\", "\\\\"));
		escape_map.push_back(pair<string, string>("^", "\\^"));
		escape_map.push_back(pair<string, string>("$", "\\$"));
		escape_map.push_back(pair<string, string>("{", "\\{"));
		escape_map.push_back(pair<string, string>("}", "\\}"));
		escape_map.push_back(pair<string, string>("[", "\\["));
		escape_map.push_back(pair<string, string>("]", "\\]"));
		escape_map.push_back(pair<string, string>("(", "\\("));
		escape_map.push_back(pair<string, string>(")", "\\)"));
		escape_map.push_back(pair<string, string>("+", "\\+"));
		escape_map.push_back(pair<string, string>("*", "\\*"));
		escape_map.push_back(pair<string, string>(".", "\\."));
	}
	return escape_map;
}

/// A map of lyx escaped strings and their unescaped equivalent.
Escapes const & get_lyx_unescapes() {
	static Escapes escape_map;
	if (escape_map.empty()) {
		escape_map.push_back(pair<string, string>("{*}", "*"));
		escape_map.push_back(pair<string, string>("{[}", "["));
		escape_map.push_back(pair<string, string>("\\$", "$"));
		escape_map.push_back(pair<string, string>("\\backslash{}", "\\"));
		escape_map.push_back(pair<string, string>("\\backslash", "\\"));
		escape_map.push_back(pair<string, string>("\\sim ", "~"));
		escape_map.push_back(pair<string, string>("\\^", "^"));
	}
	return escape_map;
}

/** @todo Probably the maps need to be migrated to regexps, in order to distinguish if
 ** the found occurrence were escaped.
 **/
string apply_escapes(string s, Escapes const & escape_map)
{
	LYXERR(Debug::DEBUG, "Escaping: '" << s << "'");
	Escapes::const_iterator it;
	for (it = escape_map.begin(); it != escape_map.end(); ++it) {
//		LYXERR(Debug::DEBUG, "Escaping " << it->first << " as " << it->second);
		unsigned int pos = 0;
		while (pos < s.length() && (pos = s.find(it->first, pos)) < s.length()) {
			s.replace(pos, it->first.length(), it->second);
//			LYXERR(Debug::DEBUG, "After escape: " << s);
			pos += it->second.length();
//			LYXERR(Debug::DEBUG, "pos: " << pos);
		}
	}
	LYXERR(Debug::DEBUG, "Escaped : '" << s << "'");
	return s;
}

/** Return the position of the closing brace matching the open one at s[pos],
 ** or s.size() if not found.
 **/
size_t find_matching_brace(string const & s, size_t pos)
{
	BOOST_ASSERT(s[pos] == '{');
	int open_braces = 1;
	for (++pos; pos < s.size(); ++pos) {
		if (s[pos] == '\\')
			++pos;
		else if (s[pos] == '{')
			++open_braces;
		else if (s[pos] == '}') {
			--open_braces;
			if (open_braces == 0)
				return pos;
		}
	}
	return s.size();
}

/// Within \regex{} apply get_regex_escapes(), while outside apply get_lyx_unescapes().
string escape_for_regex(string s)
{
	size_t pos = 0;
	while (pos < s.size()) {
			size_t new_pos = s.find("\\regexp{", pos);
			if (new_pos == string::npos)
					new_pos = s.size();
			LYXERR(Debug::DEBUG, "new_pos: " << new_pos);
			string t = apply_escapes(s.substr(pos, new_pos - pos), get_lyx_unescapes());
			LYXERR(Debug::DEBUG, "t      : " << t);
			t = apply_escapes(t, get_regexp_escapes());
			s.replace(pos, new_pos - pos, t);
			new_pos = pos + t.size();
			LYXERR(Debug::DEBUG, "Regexp after escaping: " << s);
			LYXERR(Debug::DEBUG, "new_pos: " << new_pos);
			if (new_pos == s.size())
					break;
			size_t end_pos = find_matching_brace(s, new_pos + 7);
			LYXERR(Debug::DEBUG, "end_pos: " << end_pos);
			t = apply_escapes(s.substr(new_pos + 8, end_pos - (new_pos + 8)), get_lyx_unescapes());
			LYXERR(Debug::DEBUG, "t      : " << t);
			if (end_pos == s.size()) {
					s.replace(new_pos, end_pos - new_pos, t);
					pos = s.size();
					LYXERR(Debug::DEBUG, "Regexp after \\regexp{} removal: " << s);
					break;
			}
			s.replace(new_pos, end_pos + 1 - new_pos, t);
			LYXERR(Debug::DEBUG, "Regexp after \\regexp{} removal: " << s);
			pos = new_pos + t.size();
			LYXERR(Debug::DEBUG, "pos: " << pos);
	}
	return s;
}

/// Wrapper for boost::regex_replace with simpler interface
bool regex_replace(string const & s, string & t, string const & searchstr,
	string const & replacestr)
{
	boost::regex e(searchstr);
	ostringstream oss;
	ostream_iterator<char, char> it(oss);
	boost::regex_replace(it, s.begin(), s.end(), e, replacestr);
	// tolerate t and s be references to the same variable
	bool rv = (s != oss.str());
	t = oss.str();
	return rv;
}

/** Checks if supplied string segment is well-formed from the standpoint of matching open-closed braces.
 **
 ** Verify that closed braces exactly match open braces. This avoids that, for example,
 ** \frac{.*}{x} matches \frac{x+\frac{y}{x}}{z} with .* being 'x+\frac{y'.
 **
 ** @param unmatched
 ** Number of open braces that must remain open at the end for the verification to succeed.
 **/
bool braces_match(string::const_iterator const & beg,
	string::const_iterator const & end, int unmatched = 0)
{
	int open_pars = 0;
	string::const_iterator it = beg;
	LYXERR(Debug::DEBUG, "Checking " << unmatched << " unmatched braces in '" << string(beg, end) << "'");
	for (; it != end; ++it) {
		// Skip escaped braces in the count
		if (*it == '\\') {
			++it;
			if (it == end)
				break;
		} else if (*it == '{') {
			++open_pars;
		} else if (*it == '}') {
			if (open_pars == 0) {
				LYXERR(Debug::DEBUG, "Found unmatched closed brace");
				return false;
			} else
				--open_pars;
		}
	}
	if (open_pars != unmatched) {
	  LYXERR(Debug::DEBUG, "Found " << open_pars << " instead of " << unmatched << " unmatched open braces at the end of count");
			return false;
	}
	LYXERR(Debug::DEBUG, "Braces match as expected");
	return true;
}

/** The class performing a match between a position in the document and the FindAdvOptions.
 **
 ** @todo The user-entered regexp expression(s) should be enclosed within something like \regexp{},
 **       to be written by a dedicated Inset, so to avoid escaping it in escape_for_regex().
 **/
class MatchStringAdv {
public:
	MatchStringAdv(lyx::Buffer const & buf, FindAdvOptions const & opt);

	/** Tests if text starting at the supplied position matches with the one provided to the MatchStringAdv
	 ** constructor as opt.search, under the opt.* options settings.
	 **
	 ** @return
	 ** The length of the matching text, or zero if no match was found.
	 **/
	int operator()(DocIterator const & cur, int len = -1) const;

public:
	/// buffer
	lyx::Buffer const & buf;
	/// options
	FindAdvOptions const & opt;

private:
	/** Normalize a stringified or latexified LyX paragraph.
	 **
	 ** Normalize means:
	 ** <ul>
	 **   <li>if search is not casesensitive, then lowercase the string;
	 **   <li>remove any newline at begin or end of the string;
	 **   <li>replace any newline in the middle of the string with a simple space;
	 **   <li>remove stale empty styles and environments, like \emph{} and \textbf{}.
	 ** </ul>
	 **
	 ** @todo Normalization should also expand macros, if the corresponding
	 ** search option was checked.
	 **/
	string normalize(docstring const & s) const;
	// normalized string to search
	string par_as_string;
	// regular expression to use for searching
	boost::regex regexp;
	// unmatched open braces in the search string/regexp
	int open_braces;
	// number of (.*?) subexpressions added at end of search regexp for closing
	// environments, math mode, styles, etc...
	int close_wildcards;
};


MatchStringAdv::MatchStringAdv(lyx::Buffer const & buf, FindAdvOptions const & opt)
  : buf(buf), opt(opt)
{
	par_as_string = normalize(opt.search);
	open_braces = 0;
	close_wildcards = 0;

	if (! opt.regexp) {
		// Remove trailing closure of math, macros and environments, so to catch parts of them.
		do {
			LYXERR(Debug::DEBUG, "par_as_string now is '" << par_as_string << "'");
			if (regex_replace(par_as_string, par_as_string, "(.*)[[:blank:]]\\'", "$1"))
					continue;
			if (regex_replace(par_as_string, par_as_string, "(.*[^\\\\]) ?\\$\\'", "$1"))
					continue;
			// @todo need to account for open square braces as well ?
			if (regex_replace(par_as_string, par_as_string, "(.*[^\\\\]) ?\\\\\\]\\'", "$1"))
					continue;
			if (regex_replace(par_as_string, par_as_string, "(.*[^\\\\]) ?\\\\end\\{[a-zA-Z_]*\\}\\'", "$1"))
					continue;
			if (regex_replace(par_as_string, par_as_string, "(.*[^\\\\]) ?\\}\\'", "$1")) {
				++open_braces;
				continue;
			}
			break;
		} while (true);
		LYXERR(Debug::DEBUG, "Open braces: " << open_braces);
		BOOST_ASSERT(braces_match(par_as_string.begin(), par_as_string.end(), open_braces));
		LYXERR(Debug::DEBUG, "Built MatchStringAdv object: par_as_string = '" << par_as_string << "'");
	} else {
		par_as_string = escape_for_regex(par_as_string);
		// Insert (.*?) before trailing closure of math, macros and environments, so to catch parts of them.
		LYXERR(Debug::DEBUG, "par_as_string now is '" << par_as_string << "'");
		if (
			// Insert .* before trailing '\$' ('$' has been escaped by escape_for_regex)
			regex_replace(par_as_string, par_as_string, "(.*[^\\\\])(\\\\\\$)\\'", "$1(.*?)$2")
				// Insert .* before trailing '\\\]' ('\]' has been escaped by escape_for_regex)
				|| regex_replace(par_as_string, par_as_string, "(.*[^\\\\])(\\\\\\\\\\\\\\])\\'", "$1(.*?)$2")
				// Insert .* before trailing '\\end\{...}' ('\end{...}' has been escaped by escape_for_regex)
				|| regex_replace(par_as_string, par_as_string, "(.*[^\\\\])(\\\\\\\\end\\\\\\{[a-zA-Z_]*\\\\\\})\\'", "$1(.*?)$2")
				// Insert .* before trailing '\}' ('}' has been escaped by escape_for_regex)
				|| regex_replace(par_as_string, par_as_string, "(.*[^\\\\])(\\\\\\})\\'", "$1(.*?)$2")
		) {
			++close_wildcards;
		}
		LYXERR(Debug::DEBUG, "par_as_string now is '" << par_as_string << "'");
		LYXERR(Debug::DEBUG, "Open braces: " << open_braces);
		LYXERR(Debug::DEBUG, "Close .*?  : " << close_wildcards);
		BOOST_ASSERT(braces_match(par_as_string.begin(), par_as_string.end(), open_braces));
		// Entered regexp must match at begin of searched string buffer
		par_as_string = string("\\`") + par_as_string;
		LYXERR(Debug::DEBUG, "Replaced text (to be used as regex): " << par_as_string);
		regexp = boost::regex(par_as_string);
	}
}

int MatchStringAdv::operator()(DocIterator const & cur, int len) const
{
	docstring docstr = stringifyFromForSearch(opt, buf, cur, len);
	LYXERR(Debug::DEBUG, "Matching against     '" << lyx::to_utf8(docstr) << "'");
	string str = normalize(docstr);
	LYXERR(Debug::DEBUG, "After normalization: '" << str << "'");
	if (! opt.regexp) {
		if (str.substr(0, par_as_string.size()) == par_as_string)
			return par_as_string.size();
	} else {
		// Try all possible regexp matches, until one that verifies the braces match test is found
		boost::sregex_iterator re_it(str.begin(), str.end(), regexp);
		boost::sregex_iterator re_it_end;
		for (; re_it != re_it_end; ++re_it) {
			boost::match_results<string::const_iterator> const & m = *re_it;
			// Check braces on the segment that matched the entire regexp expression,
			// plus the last subexpression, if a (.*?) was inserted in the constructor.
			if (! braces_match(m[0].first, m[0].second, open_braces))
				return 0;
			// Check braces on segments that matched all (.*?) subexpressions.
			for (size_t i = 1; i < m.size(); ++i)
				if (! braces_match(m[i].first, m[i].second))
					return false;
			// Exclude from the returned match length any length due to close wildcards added at end of regexp
			if (close_wildcards == 0)
				return m[0].second - m[0].first;
			else
				return m[m.size() - close_wildcards].first - m[0].first;
		}
	}
	return 0;
}


string MatchStringAdv::normalize(docstring const & s) const
{
	string t;
	if (! opt.casesensitive)
		t = lyx::to_utf8(lowercase(s));
	else
		t = lyx::to_utf8(s);
	// Remove \n at begin
	while (t.size() > 0 && t[0] == '\n')
		t = t.substr(1);
	// Remove \n at end
	while (t.size() > 0 && t[t.size() - 1] == '\n')
		t = t.substr(0, t.size() - 1);
	size_t pos;
	// Replace all other \n with spaces
	while ((pos = t.find("\n")) != string::npos)
		t.replace(pos, 1, " ");
	// Remove stale empty \emph{}, \textbf{} and similar blocks from latexify
	while (regex_replace(t, t, "\\\\[a-zA-Z_]+(\\{\\})+", ""))
		;
	return t;
}


docstring stringifyFromCursor(DocIterator const & cur, int len)
{
	LYXERR(Debug::DEBUG, "Stringifying with len=" << len << " from cursor at pos: " << cur);
	if (cur.inTexted()) {
			Paragraph const & par = cur.paragraph();
			// TODO what about searching beyond/across paragraph breaks ?
			// TODO Try adding a AS_STR_INSERTS as last arg
			return par.asString(cur.pos(), ( len == -1 || cur.pos() + len > int(par.size()) ) ? int(par.size()) : cur.pos() + len, AS_STR_INSETS);
	} else if (cur.inMathed()) {
			odocstringstream os;
			CursorSlice cs = cur.top();
			MathData md = cs.cell();
			MathData::const_iterator it_end = ( ( len == -1 || cs.pos() + len > int(md.size()) ) ? md.end() : md.begin() + cs.pos() + len );
			for (MathData::const_iterator it = md.begin() + cs.pos(); it != it_end; ++it)
					os << *it;
			return os.str();
	}
	LYXERR(Debug::DEBUG, "Don't know how to stringify from here: " << cur);
	return docstring();
}

/** Computes the LaTeX export of buf starting from cur and ending len positions
 * after cur, if len is positive, or at the paragraph or innermost inset end
 * if len is -1.
 */

docstring latexifyFromCursor(Buffer const & buf, DocIterator const & cur, int len)
{
	LYXERR(Debug::DEBUG, "Latexifying with len=" << len << " from cursor at pos: " << cur);
	LYXERR(Debug::DEBUG, "  with cur.lastpost=" << cur.lastpos() << ", cur.lastrow="
		<< cur.lastrow() << ", cur.lastcol=" << cur.lastcol());
	BOOST_ASSERT(buf.isLatex());

	TexRow texrow;
	odocstringstream ods;
	OutputParams runparams(&buf.params().encoding());
	runparams.nice = false;
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = 8000; //lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;

	if (cur.inTexted()) {
			// @TODO what about searching beyond/across paragraph breaks ?
			ParagraphList::const_iterator pit = cur.innerText()->paragraphs().begin();
			for (int i = 0; i < cur.pit(); ++i)
					++pit;
//		ParagraphList::const_iterator pit_end = pit;
//		++pit_end;
//		lyx::latexParagraphs(buf, cur.innerText()->paragraphs(), ods, texrow, runparams, string(), pit, pit_end);
		pos_type const endpos = (len == -1 || cur.pos() + len > int(pit->size()))
			? pit->size() : cur.pos() + len;
		TeXOnePar(buf, *cur.innerText(), pit, ods, texrow, runparams, string(),
			cur.pos(), endpos);
		LYXERR(Debug::DEBUG, "Latexified text: '" << lyx::to_utf8(ods.str()) << "'");
	} else if (cur.inMathed()) {
		// Retrieve the math environment type, and add '$' or '$[' or others (\begin{equation}) accordingly
		for (int s = cur.depth() - 1; s >= 0; --s) {
				CursorSlice const & cs = cur[s];
				if (cs.asInsetMath() && cs.asInsetMath() && cs.asInsetMath()->asHullInset()) {
						WriteStream ws(ods);
						cs.asInsetMath()->asHullInset()->header_write(ws);
						break;
				}
		}

		CursorSlice const & cs = cur.top();
		MathData md = cs.cell();
		MathData::const_iterator it_end = ( ( len == -1 || cs.pos() + len > int(md.size()) )
			? md.end() : md.begin() + cs.pos() + len );
		for (MathData::const_iterator it = md.begin() + cs.pos(); it != it_end; ++it)
				ods << *it;

		// MathData md = cur.cell();
		// MathData::const_iterator it_end = ( ( len == -1 || cur.pos() + len > int(md.size()) ) ? md.end() : md.begin() + cur.pos() + len );
		// for (MathData::const_iterator it = md.begin() + cur.pos(); it != it_end; ++it) {
		//	MathAtom const & ma = *it;
		//	ma.nucleus()->latex(buf, ods, runparams);
		// }

		// Retrieve the math environment type, and add '$' or '$]'
		// or others (\end{equation}) accordingly
		for (int s = cur.depth() - 1; s >= 0; --s) {
			CursorSlice const & cs = cur[s];
			InsetMath * inset = cs.asInsetMath();
			if (inset && inset->asHullInset()) {
				WriteStream ws(ods);
				inset->asHullInset()->footer_write(ws);
				break;
			}
		}
		LYXERR(Debug::DEBUG, "Latexified math: '" << lyx::to_utf8(ods.str()) << "'");
	} else {
		LYXERR(Debug::DEBUG, "Don't know how to stringify from here: " << cur);
	}
	return ods.str();
}

/** Finalize an advanced find operation, advancing the cursor to the innermost
 ** position that matches, plus computing the length of the matching text to
 ** be selected
 **/
int findAdvFinalize(DocIterator & cur, MatchStringAdv const & match)
{
	// Search the foremost position that matches (avoids find of entire math
	// inset when match at start of it)
	size_t d;
	DocIterator old_cur(cur.buffer());
	do {
	  LYXERR(Debug::DEBUG, "Forwarding one step (searching for innermost match)");
		d = cur.depth();
		old_cur = cur;
		cur.forwardPos();
	} while (cur && cur.depth() > d && match(cur) > 0);
	cur = old_cur;
	BOOST_ASSERT(match(cur) > 0);
	LYXERR(Debug::DEBUG, "Ok");

	// Compute the match length
	int len = 1;
	LYXERR(Debug::DEBUG, "verifying unmatch with len = " << len);
	while (cur.pos() + len <= cur.lastpos() && match(cur, len) == 0) {
		++len;
		LYXERR(Debug::DEBUG, "verifying unmatch with len = " << len);
	}
	// Length of matched text (different from len param)
	int old_len = match(cur, len);
	int new_len;
	// Greedy behaviour while matching regexps
	while ((new_len = match(cur, len + 1)) > old_len) {
		++len;
		old_len = new_len;
		LYXERR(Debug::DEBUG, "verifying   match with len = " << len);
	}
	return len;
}

/// Finds forward
int findForwardAdv(DocIterator & cur, MatchStringAdv const & match)
{
	if (!cur)
		return 0;
	for (; cur; cur.forwardPos()) {
		// odocstringstream ods;
		// ods << _("Searching ... ")
		//     << (cur.bottom().lastpit() - cur.bottom().pit()) * 100 / total;
		// cur.message(ods.str());
		if (match(cur))
			return findAdvFinalize(cur, match);
	}
	return 0;
}

/// Finds backwards
int findBackwardsAdv(DocIterator & cur, MatchStringAdv const & match)
{
	//	if (cur.pos() > 0 || cur.depth() > 0)
	//		cur.backwardPos();
	DocIterator cur_orig(cur);
	if (match(cur_orig))
		findAdvFinalize(cur_orig, match);
	// int total = cur.bottom().pit() + 1;
	for (; cur; cur.backwardPos()) {
		// odocstringstream ods;
		// ods << _("Searching ... ") << (total - cur.bottom().pit()) * 100 / total;
		// cur.message(ods.str());
		if (match(cur)) {
			// Find the most backward consecutive match within same
			// paragraph while searching backwards.
			int pit = cur.pit();
			int old_len;
			DocIterator old_cur;
			int len = findAdvFinalize(cur, match);
			do {
				old_cur = cur;
				old_len = len;
				cur.backwardPos();
				LYXERR(Debug::DEBUG, "old_cur: " << old_cur
						<< ", old_len: " << len << ", cur: " << cur);
			} while (cur && cur.pit() == pit && match(cur)
				 && (len = findAdvFinalize(cur, match)) > old_len);
			cur = old_cur;
			len = old_len;
			LYXERR(Debug::DEBUG, "cur_orig    : " << cur_orig);
			LYXERR(Debug::DEBUG, "cur         : " << cur);
			if (cur != cur_orig)
				return len;
		}
	}
	return 0;
}

} // anonym namespace


docstring stringifyFromForSearch(FindAdvOptions const & opt, Buffer const & buf,
	DocIterator const & cur, int len)
{
	if (!opt.ignoreformat)
		return latexifyFromCursor(buf, cur, len);
	else
		return stringifyFromCursor(cur, len);
}


lyx::FindAdvOptions::FindAdvOptions(docstring const & search, bool casesensitive,
	bool matchword, bool forward, bool expandmacros, bool ignoreformat,
	bool regexp)
	: search(search), casesensitive(casesensitive), matchword(matchword),
	forward(forward), expandmacros(expandmacros), ignoreformat(ignoreformat),
	regexp(regexp)
{
}

/// Perform a FindAdv operation.
bool findAdv(BufferView * bv, FindAdvOptions const & opt)
{
	DocIterator cur = bv->cursor();
	int match_len = 0;

	if (opt.search.empty()) {
			bv->message(_("Search text is empty !"));
			return false;
	}
// 	if (! bv->buffer()) {
// 		bv->message(_("No open document !"));
// 		return false;
// 	}

	try {
		MatchStringAdv const matchAdv(bv->buffer(), opt);
		if (opt.forward)
				match_len = findForwardAdv(cur, matchAdv);
		else
				match_len = findBackwardsAdv(cur, matchAdv);
	} catch (...) {
		// This may only be raised by boost::regex()
		bv->message(_("Invalid regular expression !"));
		return false;
	}

	if (match_len == 0) {
		bv->message(_("Match not found !"));
		return false;
	}

	LYXERR(Debug::DEBUG, "Putting selection at " << cur << " with len: " << match_len);
	bv->putSelectionAt(cur, match_len, ! opt.forward);
	bv->message(_("Match found !"));
	//bv->update();

	return true;
}


void findAdv(BufferView * bv, FuncRequest const & ev)
{
	if (!bv || ev.action != LFUN_WORD_FINDADV)
		return;

	FindAdvOptions opt;
	istringstream iss(to_utf8(ev.argument()));
	iss >> opt;
	findAdv(bv, opt);
}


ostringstream & operator<<(ostringstream & os, lyx::FindAdvOptions const & opt)
{
	os << to_utf8(opt.search) << "\nEOSS\n"
	   << opt.casesensitive << ' '
	   << opt.matchword << ' '
	   << opt.forward << ' '
	   << opt.expandmacros << ' '
	   << opt.ignoreformat << ' '
	   << opt.regexp;

	LYXERR(Debug::DEBUG, "built: " << os.str());

	return os;
}

istringstream & operator>>(istringstream & is, lyx::FindAdvOptions & opt)
{
	LYXERR(Debug::DEBUG, "parsing");
	string s;
	string line;
	getline(is, line);
	while (line != "EOSS") {
		if (! s.empty())
				s = s + "\n";
		s = s + line;
		if (is.eof())	// Tolerate malformed request
				break;
		getline(is, line);
	}
	LYXERR(Debug::DEBUG, "searching for: '" << s << "'");
	opt.search = from_utf8(s);
	is >> opt.casesensitive >> opt.matchword >> opt.forward >> opt.expandmacros >> opt.ignoreformat >> opt.regexp;
	LYXERR(Debug::DEBUG, "parsed: " << opt.casesensitive << ' ' << opt.matchword << ' ' << opt.forward << ' '
		   << opt.expandmacros << ' ' << opt.ignoreformat << ' ' << opt.regexp);
	return is;
}

} // lyx namespace
