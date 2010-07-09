/**
 * \file lyxfind.cpp
 * This file is part of LyX, the document processor.
 * License details can be found in the file COPYING.
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
#include "BufferList.h"
#include "BufferParams.h"
#include "BufferView.h"
#include "Changes.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "FuncRequest.h"
#include "LyX.h"
#include "output_latex.h"
#include "OutputParams.h"
#include "Paragraph.h"
#include "ParIterator.h"
#include "TexRow.h"
#include "Text.h"

#include "frontends/alert.h"

#include "mathed/InsetMath.h"
#include "mathed/InsetMathGrid.h"
#include "mathed/InsetMathHull.h"
#include "mathed/MathStream.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include "support/regex.h"
#include <boost/next_prior.hpp>

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


bool findChange(DocIterator & cur, bool next)
{
	if (!next)
		cur.backwardPos();
	for (; cur; next ? cur.forwardPos() : cur.backwardPos())
		if (cur.inTexted() && cur.paragraph().isChanged(cur.pos())) {
			if (!next)
				// if we search backwards, take a step forward
				// to correctly set the anchor
				cur.forwardPos();
			return true;
		}

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

	DocIterator cur_orig(bv->cursor());

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

	bv->putSelectionAt(doc_iterator_begin(&buf), 0, false);
	if (num)
		buf.markDirty();

	cur_orig.fixIfBroken();
	bv->setCursor(cur_orig);

	return num;
}


bool stringSelected(BufferView * bv, docstring & searchstr,
		    bool cs, bool mw, bool fw)
{
	// if nothing selected and searched string is empty, this
	// means that we want to search current word at cursor position,
	// but only if we are in texted() mode.
	if (!bv->cursor().selection() && searchstr.empty()
	    && bv->cursor().inTexted()) {
		bv->cursor().innerText()->selectWord(bv->cursor(), WHOLE_WORD);
		searchstr = bv->cursor().selectionAsString(false);
		return true;
	}

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


int replace(BufferView * bv, docstring & searchstr,
	    docstring const & replacestr, bool cs, bool mw, bool fw)
{
	if (!stringSelected(bv, searchstr, cs, mw, fw))
		return 0;

	if (!searchAllowed(bv, searchstr) || bv->buffer().isReadonly())
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


docstring const replace2string(docstring const & replace,
	docstring const & search, bool casesensitive, bool matchword,
	bool all, bool forward)
{
	odocstringstream ss;
	ss << replace << '\n'
	   << search << '\n'
	   << int(casesensitive) << ' '
	   << int(matchword) << ' '
	   << int(all) << ' '
	   << int(forward);
	return ss.str();
}


bool find(BufferView * bv, FuncRequest const & ev)
{
	if (!bv || ev.action() != LFUN_WORD_FIND)
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
	if (!bv || ev.action() != LFUN_WORD_REPLACE)
		return;

	// data is of the form
	// "<search>
	//  <replace>
	//  <casesensitive> <matchword> <all> <forward>"
	docstring search;
	docstring rplc;
	docstring howto = split(ev.argument(), rplc, '\n');
	howto = split(howto, search, '\n');

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
	return findChange(bv, true);
}


bool findPreviousChange(BufferView * bv)
{
	return findChange(bv, false);
}


bool findChange(BufferView * bv, bool next)
{
	if (bv->cursor().selection()) {
		// set the cursor at the beginning or at the end of the selection
		// before searching. Otherwise, the current change will be found.
		if (next != (bv->cursor().top() > bv->cursor().normalAnchor()))
			bv->cursor().setCursorToAnchor();
	}

	DocIterator cur = bv->cursor();

	// Are we within a change ? Then first search forward (backward),
	// clear the selection and search the other way around (see the end
	// of this function). This will avoid changes to be selected half.
	bool search_both_sides = false;
	DocIterator tmpcur = cur;
	// Leave math first
	while (tmpcur.inMathed())
		tmpcur.pop_back();
	Change change_next_pos
		= tmpcur.paragraph().lookupChange(tmpcur.pos());
	if (change_next_pos.changed() && cur.inMathed()) {
		cur = tmpcur;
		search_both_sides = true;
	} else if (tmpcur.pos() > 0 && tmpcur.inTexted()) {
		Change change_prev_pos
			= tmpcur.paragraph().lookupChange(tmpcur.pos() - 1);
		if (change_next_pos.isSimilarTo(change_prev_pos))
			search_both_sides = true;
	}

	if (!findChange(cur, next))
		return false;

	bv->cursor().setCursor(cur);
	bv->cursor().resetAnchor();

	if (!next)
		// take a step into the change
		cur.backwardPos();

	Change orig_change = cur.paragraph().lookupChange(cur.pos());

	CursorSlice & tip = cur.top();
	if (next) {
		for (; !tip.at_end(); tip.forwardPos()) {
			Change change = tip.paragraph().lookupChange(tip.pos());
			if (!change.isSimilarTo(orig_change))
				break;
		}
	} else {
		for (; !tip.at_begin();) {
			tip.backwardPos();
			Change change = tip.paragraph().lookupChange(tip.pos());
			if (!change.isSimilarTo(orig_change)) {
				// take a step forward to correctly set the selection
				tip.forwardPos();
				break;
			}
		}
	}

	// Now put cursor to end of selection:
	bv->cursor().setCursor(cur);
	bv->cursor().setSelection();

	if (search_both_sides) {
		bv->cursor().setSelection(false);
		findChange(bv, !next);
	}

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
	LYXERR(Debug::FIND, "Escaping: '" << s << "'");
	Escapes::const_iterator it;
	for (it = escape_map.begin(); it != escape_map.end(); ++it) {
//		LYXERR(Debug::FIND, "Escaping " << it->first << " as " << it->second);
		unsigned int pos = 0;
		while (pos < s.length() && (pos = s.find(it->first, pos)) < s.length()) {
			s.replace(pos, it->first.length(), it->second);
//			LYXERR(Debug::FIND, "After escape: " << s);
			pos += it->second.length();
//			LYXERR(Debug::FIND, "pos: " << pos);
		}
	}
	LYXERR(Debug::FIND, "Escaped : '" << s << "'");
	return s;
}

/** Return the position of the closing brace matching the open one at s[pos],
 ** or s.size() if not found.
 **/
size_t find_matching_brace(string const & s, size_t pos)
{
	LASSERT(s[pos] == '{', /* */);
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

/// Within \regexp{} apply get_regex_escapes(), while outside apply get_lyx_unescapes().
string escape_for_regex(string s)
{
	size_t pos = 0;
	while (pos < s.size()) {
		size_t new_pos = s.find("\\regexp{{{", pos);
		if (new_pos == string::npos)
			new_pos = s.size();
		LYXERR(Debug::FIND, "new_pos: " << new_pos);
		string t = apply_escapes(s.substr(pos, new_pos - pos), get_lyx_unescapes());
		LYXERR(Debug::FIND, "t      : " << t);
		t = apply_escapes(t, get_regexp_escapes());
		LYXERR(Debug::FIND, "t      : " << t);
		s.replace(pos, new_pos - pos, t);
		new_pos = pos + t.size();
		LYXERR(Debug::FIND, "Regexp after escaping: " << s);
		LYXERR(Debug::FIND, "new_pos: " << new_pos);
		if (new_pos == s.size())
			break;
		size_t end_pos = s.find("}}}", new_pos + 10); // find_matching_brace(s, new_pos + 7);
		LYXERR(Debug::FIND, "end_pos: " << end_pos);
		t = apply_escapes(s.substr(new_pos + 10, end_pos - (new_pos + 10)), get_lyx_unescapes());
		LYXERR(Debug::FIND, "t      : " << t);
		if (end_pos == s.size()) {
			s.replace(new_pos, end_pos - new_pos, t);
			pos = s.size();
			LYXERR(Debug::FIND, "Regexp after \\regexp{} removal: " << s);
			break;
		}
		s.replace(new_pos, end_pos + 3 - new_pos, t);
		LYXERR(Debug::FIND, "Regexp after \\regexp{} removal: " << s);
		pos = new_pos + t.size();
		LYXERR(Debug::FIND, "pos: " << pos);
	}
	return s;
}

/// Wrapper for lyx::regex_replace with simpler interface
bool regex_replace(string const & s, string & t, string const & searchstr,
	string const & replacestr)
{
	lyx::regex e(searchstr);
	ostringstream oss;
	ostream_iterator<char, char> it(oss);
	lyx::regex_replace(it, s.begin(), s.end(), e, replacestr);
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
		  string::const_iterator const & end,
		  int unmatched = 0)
{
	int open_pars = 0;
	string::const_iterator it = beg;
	LYXERR(Debug::FIND, "Checking " << unmatched << " unmatched braces in '" << string(beg, end) << "'");
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
				LYXERR(Debug::FIND, "Found unmatched closed brace");
				return false;
			} else
				--open_pars;
		}
	}
	if (open_pars != unmatched) {
	  LYXERR(Debug::FIND, "Found " << open_pars 
		 << " instead of " << unmatched 
		 << " unmatched open braces at the end of count");
			return false;
	}
	LYXERR(Debug::FIND, "Braces match as expected");
	return true;
}

/** The class performing a match between a position in the document and the FindAdvOptions.
 **/
class MatchStringAdv {
public:
	MatchStringAdv(lyx::Buffer & buf, FindAndReplaceOptions const & opt);

	/** Tests if text starting at the supplied position matches with the one provided to the MatchStringAdv
	 ** constructor as opt.search, under the opt.* options settings.
	 **
	 ** @param at_begin
	 ** 	If set, then match is searched only against beginning of text starting at cur.
	 ** 	If unset, then match is searched anywhere in text starting at cur.
	 **
	 ** @return
	 ** The length of the matching text, or zero if no match was found.
	 **/
	int operator()(DocIterator const & cur, int len = -1, bool at_begin = true) const;

public:
	/// buffer
	lyx::Buffer * p_buf;
	/// first buffer on which search was started
	lyx::Buffer * const p_first_buf;
	/// options
	FindAndReplaceOptions const & opt;

private:
	/// Auxiliary find method (does not account for opt.matchword)
	int findAux(DocIterator const & cur, int len = -1, bool at_begin = true) const;

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
	lyx::regex regexp;
	// same as regexp, but prefixed with a ".*"
	lyx::regex regexp2;
	// unmatched open braces in the search string/regexp
	int open_braces;
	// number of (.*?) subexpressions added at end of search regexp for closing
	// environments, math mode, styles, etc...
	int close_wildcards;
};


MatchStringAdv::MatchStringAdv(lyx::Buffer & buf, FindAndReplaceOptions const & opt)
	: p_buf(&buf), p_first_buf(&buf), opt(opt)
{
	par_as_string = normalize(opt.search);
	open_braces = 0;
	close_wildcards = 0;

	if (! opt.regexp) {
		// Remove trailing closure of math, macros and environments, so to catch parts of them.
		do {
			LYXERR(Debug::FIND, "par_as_string now is '" << par_as_string << "'");
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
		LYXERR(Debug::FIND, "Open braces: " << open_braces);
		LYXERR(Debug::FIND, "Built MatchStringAdv object: par_as_string = '" << par_as_string << "'");
	} else {
		par_as_string = escape_for_regex(par_as_string);
		// Insert (.*?) before trailing closure of math, macros and environments, so to catch parts of them.
		LYXERR(Debug::FIND, "par_as_string now is '" << par_as_string << "'");
		if (
			// Insert .* before trailing '\$' ('$' has been escaped by escape_for_regex)
			regex_replace(par_as_string, par_as_string, "(.*[^\\\\])(\\\\\\$)\\'", "$1(.*?)$2")
				// Insert .* before trailing '\\\]' ('\]' has been escaped by escape_for_regex)
				|| regex_replace(par_as_string, par_as_string, "(.*[^\\\\])(\\\\\\\\\\\\\\])\\'", "$1(.*?)$2")
				// Insert .* before trailing '\\end\{...}' ('\end{...}' has been escaped by escape_for_regex)
				|| regex_replace(par_as_string, par_as_string, 
					"(.*[^\\\\])(\\\\\\\\end\\\\\\{[a-zA-Z_]*\\\\\\})\\'", "$1(.*?)$2")
				// Insert .* before trailing '\}' ('}' has been escaped by escape_for_regex)
				|| regex_replace(par_as_string, par_as_string, "(.*[^\\\\])(\\\\\\})\\'", "$1(.*?)$2")
		) {
			++close_wildcards;
		}
		LYXERR(Debug::FIND, "par_as_string now is '" << par_as_string << "'");
		LYXERR(Debug::FIND, "Open braces: " << open_braces);
		LYXERR(Debug::FIND, "Close .*?  : " << close_wildcards);
		LYXERR(Debug::FIND, "Replaced text (to be used as regex): " << par_as_string);
		// If entered regexp must match at begin of searched string buffer
		regexp = lyx::regex(string("\\`") + par_as_string);
		// If entered regexp may match wherever in searched string buffer
		regexp2 = lyx::regex(string("\\`.*") + par_as_string);
	}
}


int MatchStringAdv::findAux(DocIterator const & cur, int len, bool at_begin) const
{
	docstring docstr = stringifyFromForSearch(opt, cur, len);
	LYXERR(Debug::FIND, "Matching against     '" << lyx::to_utf8(docstr) << "'");
	string str = normalize(docstr);
	LYXERR(Debug::FIND, "After normalization: '" << str << "'");
	if (! opt.regexp) {
		if (at_begin) {
			if (str.substr(0, par_as_string.size()) == par_as_string)
				return par_as_string.size();
		} else {
			size_t pos = str.find(par_as_string);
			if (pos != string::npos)
				return par_as_string.size();
		}
	} else {
		// Try all possible regexp matches, 
		//until one that verifies the braces match test is found
		regex const *p_regexp = at_begin ? &regexp : &regexp2;
		sregex_iterator re_it(str.begin(), str.end(), *p_regexp);
		sregex_iterator re_it_end;
		for (; re_it != re_it_end; ++re_it) {
			match_results<string::const_iterator> const & m = *re_it;
			// Check braces on the segment that matched the entire regexp expression,
			// plus the last subexpression, if a (.*?) was inserted in the constructor.
			if (! braces_match(m[0].first, m[0].second, open_braces))
				return 0;
			// Check braces on segments that matched all (.*?) subexpressions.
			for (size_t i = 1; i < m.size(); ++i)
				if (! braces_match(m[i].first, m[i].second))
					return false;
			// Exclude from the returned match length any length 
			// due to close wildcards added at end of regexp
			if (close_wildcards == 0)
				return m[0].second - m[0].first;
			else
				return m[m.size() - close_wildcards].first - m[0].first;
		}
	}
	return 0;
}


int MatchStringAdv::operator()(DocIterator const & cur, int len, bool at_begin) const
{
	int res = findAux(cur, len, at_begin);
	if (res == 0 || !at_begin || !opt.matchword || !cur.inTexted())
		return res;
	Paragraph const & par = cur.paragraph();
	bool ws_left = cur.pos() > 0 ?
		par.isWordSeparator(cur.pos() - 1) : true;
	bool ws_right = cur.pos() + res < par.size() ?
		par.isWordSeparator(cur.pos() + res) : true;
	LYXERR(Debug::FIND,
	       "cur.pos()=" << cur.pos() << ", res=" << res
	       << ", separ: " << ws_left << ", " << ws_right
	       << endl);
	if (ws_left && ws_right)
		return res;
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
	LYXERR(Debug::FIND, "Removing stale empty \\emph{}, \\textbf{}, \\*section{} macros from: " << t);
	while (regex_replace(t, t, "\\\\(emph|textbf|subsubsection|subsection|section|subparagraph|paragraph)(\\{\\})+", ""))
		LYXERR(Debug::FIND, "  further removing stale empty \\emph{}, \\textbf{} macros from: " << t);
	return t;
}


docstring stringifyFromCursor(DocIterator const & cur, int len)
{
	LYXERR(Debug::FIND, "Stringifying with len=" << len << " from cursor at pos: " << cur);
	if (cur.inTexted()) {
			Paragraph const & par = cur.paragraph();
			// TODO what about searching beyond/across paragraph breaks ?
			// TODO Try adding a AS_STR_INSERTS as last arg
			pos_type end = ( len == -1 || cur.pos() + len > int(par.size()) ) ?
				int(par.size()) : cur.pos() + len;
			OutputParams runparams(&cur.buffer()->params().encoding());
			odocstringstream os;
			runparams.nice = true;
			runparams.flavor = OutputParams::LATEX;
			runparams.linelen = 100000; //lyxrc.plaintext_linelen;
			// No side effect of file copying and image conversion
			runparams.dryrun = true;
			LYXERR(Debug::FIND, "Stringifying with cur: " 
				<< cur << ", from pos: " << cur.pos() << ", end: " << end);
			return par.stringify(cur.pos(), end, AS_STR_INSETS, runparams);
	} else if (cur.inMathed()) {
			odocstringstream os;
			CursorSlice cs = cur.top();
			MathData md = cs.cell();
			MathData::const_iterator it_end = 
				( ( len == -1 || cs.pos() + len > int(md.size()) )
					? md.end() : md.begin() + cs.pos() + len );
			for (MathData::const_iterator it = md.begin() + cs.pos(); it != it_end; ++it)
					os << *it;
			return os.str();
	}
	LYXERR(Debug::FIND, "Don't know how to stringify from here: " << cur);
	return docstring();
}


/** Computes the LaTeX export of buf starting from cur and ending len positions
 * after cur, if len is positive, or at the paragraph or innermost inset end
 * if len is -1.
 */
docstring latexifyFromCursor(DocIterator const & cur, int len)
{
	LYXERR(Debug::FIND, "Latexifying with len=" << len << " from cursor at pos: " << cur);
	LYXERR(Debug::FIND, "  with cur.lastpost=" << cur.lastpos() << ", cur.lastrow="
		<< cur.lastrow() << ", cur.lastcol=" << cur.lastcol());
	Buffer const & buf = *cur.buffer();
	LASSERT(buf.isLatex(), /* */);

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
		pos_type const endpos = (len == -1 || cur.pos() + len > int(pit->size()))
			? pit->size() : cur.pos() + len;
		TeXOnePar(buf, *cur.innerText(), pit, ods, texrow, runparams, string(),
			cur.pos(), endpos);
		LYXERR(Debug::FIND, "Latexified text: '" << lyx::to_utf8(ods.str()) << "'");
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
		LYXERR(Debug::FIND, "Latexified math: '" << lyx::to_utf8(ods.str()) << "'");
	} else {
		LYXERR(Debug::FIND, "Don't know how to stringify from here: " << cur);
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
		LYXERR(Debug::FIND, "Forwarding one step (searching for innermost match)");
		d = cur.depth();
		old_cur = cur;
		cur.forwardPos();
	} while (cur && cur.depth() > d && match(cur) > 0);
	cur = old_cur;
	LASSERT(match(cur) > 0, /* */);
	LYXERR(Debug::FIND, "Ok");

	// Compute the match length
	int len = 1;
	if (cur.pos() + len > cur.lastpos())
		return 0;
	LYXERR(Debug::FIND, "verifying unmatch with len = " << len);
	while (cur.pos() + len <= cur.lastpos() && match(cur, len) == 0) {
		++len;
		LYXERR(Debug::FIND, "verifying unmatch with len = " << len);
	}
	// Length of matched text (different from len param)
	int old_len = match(cur, len);
	int new_len;
	// Greedy behaviour while matching regexps
	while ((new_len = match(cur, len + 1)) > old_len) {
		++len;
		old_len = new_len;
		LYXERR(Debug::FIND, "verifying   match with len = " << len);
	}
	return len;
}


/// Finds forward
int findForwardAdv(DocIterator & cur, MatchStringAdv & match)
{
	if (!cur)
		return 0;
	while (cur && !match(cur, -1, false)) {
		if (cur.pit() < cur.lastpit())
			cur.forwardPar();
		else {
			cur.forwardPos();
		}
	}
	for (; cur; cur.forwardPos()) {
		if (match(cur))
			return findAdvFinalize(cur, match);
	}
	return 0;
}


/// Find the most backward consecutive match within same paragraph while searching backwards.
int findMostBackwards(DocIterator & cur, MatchStringAdv const & match)
{
	DocIterator cur_begin = doc_iterator_begin(cur.buffer());
	DocIterator tmp_cur = cur;
	int len = findAdvFinalize(tmp_cur, match);
	Inset & inset = cur.inset();
	for (; cur != cur_begin; cur.backwardPos()) {
		LYXERR(Debug::FIND, "findMostBackwards(): cur=" << cur);
		DocIterator new_cur = cur;
		new_cur.backwardPos();
		if (new_cur == cur || &new_cur.inset() != &inset || !match(new_cur))
			break;
		int new_len = findAdvFinalize(new_cur, match);
		if (new_len == len)
			break;
		len = new_len;
	}
	LYXERR(Debug::FIND, "findMostBackwards(): exiting with cur=" << cur);
	return len;
}


/// Finds backwards
int findBackwardsAdv(DocIterator & cur, MatchStringAdv & match) {
	if (! cur)
		return 0;
	// Backup of original position
	DocIterator cur_begin = doc_iterator_begin(cur.buffer());
	if (cur == cur_begin)
		return 0;
	cur.backwardPos();
	DocIterator cur_orig(cur);
	bool found_match;
	bool pit_changed = false;
	found_match = false;
	do {
		cur.pos() = 0;
		found_match = match(cur, -1, false);

		if (found_match) {
			if (pit_changed)
				cur.pos() = cur.lastpos();
			else
				cur.pos() = cur_orig.pos();
			LYXERR(Debug::FIND, "findBackAdv2: cur: " << cur);
			DocIterator cur_prev_iter;
			do {
				found_match = match(cur);
				LYXERR(Debug::FIND, "findBackAdv3: found_match=" 
				       << found_match << ", cur: " << cur);
				if (found_match)
					return findMostBackwards(cur, match);

				// Stop if begin of document reached
				if (cur == cur_begin)
					break;
				cur_prev_iter = cur;
				cur.backwardPos();
			} while (true);
		}
		if (cur == cur_begin)
			break;
		if (cur.pit() > 0)
			--cur.pit();
		else
			cur.backwardPos();
		pit_changed = true;
	} while (true);
	return 0;
}


} // anonym namespace


docstring stringifyFromForSearch(FindAndReplaceOptions const & opt,
	DocIterator const & cur, int len)
{
	if (!opt.ignoreformat)
		return latexifyFromCursor(cur, len);
	else
		return stringifyFromCursor(cur, len);
}


FindAndReplaceOptions::FindAndReplaceOptions(docstring const & search, bool casesensitive,
	bool matchword, bool forward, bool expandmacros, bool ignoreformat,
	bool regexp, docstring const & replace, bool keep_case,
	SearchScope scope)
	: search(search), casesensitive(casesensitive), matchword(matchword),
	forward(forward), expandmacros(expandmacros), ignoreformat(ignoreformat),
	regexp(regexp), replace(replace), keep_case(keep_case), scope(scope)
{
}


/** Checks if the supplied character is lower-case */
static bool isLowerCase(char_type ch) {
	return lowercase(ch) == ch;
}


/** Checks if the supplied character is upper-case */
static bool isUpperCase(char_type ch) {
	return uppercase(ch) == ch;
}


/** Check if 'len' letters following cursor are all non-lowercase */
static bool allNonLowercase(DocIterator const & cur, int len) {
	pos_type end_pos = cur.pos() + len;
	for (pos_type pos = cur.pos(); pos != end_pos; ++pos)
		if (isLowerCase(cur.paragraph().getChar(pos)))
			return false;
	return true;
}


/** Check if first letter is upper case and second one is lower case */
static bool firstUppercase(DocIterator const & cur) {
	char_type ch1, ch2;
	if (cur.pos() >= cur.lastpos() - 1) {
		LYXERR(Debug::FIND, "No upper-case at cur: " << cur);
		return false;
	}
	ch1 = cur.paragraph().getChar(cur.pos());
	ch2 = cur.paragraph().getChar(cur.pos()+1);
	bool result = isUpperCase(ch1) && isLowerCase(ch2);
	LYXERR(Debug::FIND, "firstUppercase(): "
	       << "ch1=" << ch1 << "(" << char(ch1) << "), ch2=" 
	       << ch2 << "(" << char(ch2) << ")"
	       << ", result=" << result << ", cur=" << cur);
	return result;
}


/** Make first letter of supplied buffer upper-case, and the rest lower-case.
 **
 ** \fixme What to do with possible further paragraphs in replace buffer ?
 **/
static void changeFirstCase(Buffer & buffer, TextCase first_case, TextCase others_case) {
	ParagraphList::iterator pit = buffer.paragraphs().begin();
	pos_type right = pos_type(1);
	pit->changeCase(buffer.params(), pos_type(0), right, first_case);
	right = pit->size() + 1;
	pit->changeCase(buffer.params(), right, right, others_case);
}


///
static void findAdvReplace(BufferView * bv, FindAndReplaceOptions const & opt, MatchStringAdv & matchAdv)
{
	Cursor & cur = bv->cursor();
	if (opt.replace == docstring(from_utf8(LYX_FR_NULL_STRING)))
		return;
	DocIterator sel_beg = cur.selectionBegin();
	DocIterator sel_end = cur.selectionEnd();
	if (&sel_beg.inset() != &sel_end.inset()
	    || sel_beg.pit() != sel_end.pit())
		return;
	int sel_len = sel_end.pos() - sel_beg.pos();
	LYXERR(Debug::FIND, "sel_beg: " << sel_beg << ", sel_end: " << sel_end
	       << ", sel_len: " << sel_len << endl);
	if (sel_len == 0)
		return;
	LASSERT(sel_len > 0, /**/);

	if (!matchAdv(sel_beg, sel_len))
		return;

	string lyx = to_utf8(opt.replace);
	// FIXME: Seems so stupid to me to rebuild a buffer here,
	// when we already have one (replace_work_area_.buffer())
	Buffer repl_buffer("", false);
	repl_buffer.setUnnamed(true);
	LASSERT(repl_buffer.readString(lyx), /**/);
	repl_buffer.changeLanguage(
		repl_buffer.language(),
		cur.getFont().language());
	if (opt.keep_case && sel_len >= 2) {
		if (cur.inTexted()) {
			if (firstUppercase(cur))
				changeFirstCase(repl_buffer, text_uppercase, text_lowercase);
			else if (allNonLowercase(cur, sel_len))
				changeFirstCase(repl_buffer, text_uppercase, text_uppercase);
		}
	}
	cap::cutSelection(cur, false, false);
	if (!cur.inMathed()) {
		LYXERR(Debug::FIND, "Replacing by pasteParagraphList()ing repl_buffer");
		cap::pasteParagraphList(cur, repl_buffer.paragraphs(),
					repl_buffer.params().documentClassPtr(),
					bv->buffer().errorList("Paste"));
	} else {
		odocstringstream ods;
		OutputParams runparams(&repl_buffer.params().encoding());
		runparams.nice = false;
		runparams.flavor = OutputParams::LATEX;
		runparams.linelen = 8000; //lyxrc.plaintext_linelen;
		runparams.dryrun = true;
		TexRow texrow;
		TeXOnePar(repl_buffer, repl_buffer.text(), 
			  repl_buffer.paragraphs().begin(), ods, texrow, runparams);
		//repl_buffer.getSourceCode(ods, 0, repl_buffer.paragraphs().size(), false);
		docstring repl_latex = ods.str();
		LYXERR(Debug::FIND, "Latexified replace_buffer: '" << repl_latex << "'");
		string s;
		regex_replace(to_utf8(repl_latex), s, "\\$(.*)\\$", "$1");
		regex_replace(s, s, "\\\\\\[(.*)\\\\\\]", "$1");
		repl_latex = from_utf8(s);
		LYXERR(Debug::FIND, "Replacing by niceInsert()ing latex: '" << repl_latex << "'");
		cur.niceInsert(repl_latex);
	}
	bv->buffer().markDirty();
	cur.pos() -= repl_buffer.paragraphs().begin()->size();
	bv->putSelectionAt(DocIterator(cur), repl_buffer.paragraphs().begin()->size(), !opt.forward);
}


/// Perform a FindAdv operation.
bool findAdv(BufferView * bv, FindAndReplaceOptions const & opt)
{
	DocIterator cur;
	int match_len;

	if (opt.search.empty()) {
		bv->message(_("Search text is empty!"));
		return false;
	}

	try {
		MatchStringAdv matchAdv(bv->buffer(), opt);
		findAdvReplace(bv, opt, matchAdv);
		cur = bv->cursor();
		if (opt.forward)
				match_len = findForwardAdv(cur, matchAdv);
		else
				match_len = findBackwardsAdv(cur, matchAdv);
	} catch (...) {
		// This may only be raised by lyx::regex()
		bv->message(_("Invalid regular expression!"));
		return false;
	}

	if (match_len == 0) {
		bv->message(_("Match not found!"));
		return false;
	}

	bv->message(_("Match found!"));

	LYXERR(Debug::FIND, "Putting selection at cur=" << cur << " with len: " << match_len);
	bv->putSelectionAt(cur, match_len, !opt.forward);

	return true;
}


ostringstream & operator<<(ostringstream & os, FindAndReplaceOptions const & opt)
{
	os << to_utf8(opt.search) << "\nEOSS\n"
	   << opt.casesensitive << ' '
	   << opt.matchword << ' '
	   << opt.forward << ' '
	   << opt.expandmacros << ' '
	   << opt.ignoreformat << ' '
	   << opt.regexp << ' '
	   << to_utf8(opt.replace) << "\nEOSS\n"
	   << opt.keep_case << ' '
	   << int(opt.scope);

	LYXERR(Debug::FIND, "built: " << os.str());

	return os;
}

istringstream & operator>>(istringstream & is, FindAndReplaceOptions & opt)
{
	LYXERR(Debug::FIND, "parsing");
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
	LYXERR(Debug::FIND, "searching for: '" << s << "'");
	opt.search = from_utf8(s);
	is >> opt.casesensitive >> opt.matchword >> opt.forward >> opt.expandmacros >> opt.ignoreformat >> opt.regexp;
	is.get();	// Waste space before replace string
	s = "";
	getline(is, line);
	while (line != "EOSS") {
		if (! s.empty())
				s = s + "\n";
		s = s + line;
		if (is.eof())	// Tolerate malformed request
				break;
		getline(is, line);
	}
	is >> opt.keep_case;
	int i;
	is >> i;
	opt.scope = FindAndReplaceOptions::SearchScope(i);
	LYXERR(Debug::FIND, "parsed: " << opt.casesensitive << ' ' << opt.matchword << ' ' << opt.forward << ' '
		   << opt.expandmacros << ' ' << opt.ignoreformat << ' ' << opt.regexp << ' ' << opt.keep_case);
	LYXERR(Debug::FIND, "replacing with: '" << s << "'");
	opt.replace = from_utf8(s);
	return is;
}

} // lyx namespace
