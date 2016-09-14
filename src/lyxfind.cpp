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

#include "frontends/Application.h"
#include "frontends/alert.h"

#include "mathed/InsetMath.h"
#include "mathed/InsetMathGrid.h"
#include "mathed/InsetMathHull.h"
#include "mathed/MathStream.h"
#include "mathed/MathSupport.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"

#include "support/regex.h"

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


class MatchString : public binary_function<Paragraph, pos_type, int>
{
public:
	MatchString(docstring const & str, bool cs, bool mw)
		: str(str), case_sens(cs), whole_words(mw)
	{}

	// returns true if the specified string is at the specified position
	// del specifies whether deleted strings in ct mode will be considered
	int operator()(Paragraph const & par, pos_type pos, bool del = true) const
	{
		return par.find(str, case_sens, whole_words, pos, del);
	}

private:
	// search string
	docstring str;
	// case sensitive
	bool case_sens;
	// match whole words only
	bool whole_words;
};


int findForward(DocIterator & cur, MatchString const & match,
		bool find_del = true)
{
	for (; cur; cur.forwardChar())
		if (cur.inTexted()) {
			int len = match(cur.paragraph(), cur.pos(), find_del);
			if (len > 0)
				return len;
		}
	return 0;
}


int findBackwards(DocIterator & cur, MatchString const & match,
		  bool find_del = true)
{
	while (cur) {
		cur.backwardChar();
		if (cur.inTexted()) {
			int len = match(cur.paragraph(), cur.pos(), find_del);
			if (len > 0)
				return len;
		}
	}
	return 0;
}


bool searchAllowed(docstring const & str)
{
	if (str.empty()) {
		frontend::Alert::error(_("Search error"), _("Search string is empty"));
		return false;
	}
	return true;
}


bool findOne(BufferView * bv, docstring const & searchstr,
	     bool case_sens, bool whole, bool forward,
	     bool find_del = true, bool check_wrap = false)
{
	if (!searchAllowed(searchstr))
		return false;

	DocIterator cur = forward
		? bv->cursor().selectionEnd()
		: bv->cursor().selectionBegin();

	MatchString const match(searchstr, case_sens, whole);

	int match_len = forward
		? findForward(cur, match, find_del)
		: findBackwards(cur, match, find_del);

	if (match_len > 0)
		bv->putSelectionAt(cur, match_len, !forward);
	else if (check_wrap) {
		DocIterator cur_orig(bv->cursor());
		docstring q;
		if (forward)
			q = _("End of file reached while searching forward.\n"
			  "Continue searching from the beginning?");
		else
			q = _("Beginning of file reached while searching backward.\n"
			  "Continue searching from the end?");
		int wrap_answer = frontend::Alert::prompt(_("Wrap search?"),
			q, 0, 1, _("&Yes"), _("&No"));
		if (wrap_answer == 0) {
			if (forward) {
				bv->cursor().clear();
				bv->cursor().push_back(CursorSlice(bv->buffer().inset()));
			} else {
				bv->cursor().setCursor(doc_iterator_end(&bv->buffer()));
				bv->cursor().backwardPos();
			}
			bv->clearSelection();
			if (findOne(bv, searchstr, case_sens, whole, forward, find_del, false))
				return true;
		}
		bv->cursor().setCursor(cur_orig);
		return false;
	}

	return match_len > 0;
}


int replaceAll(BufferView * bv,
	       docstring const & searchstr, docstring const & replacestr,
	       bool case_sens, bool whole)
{
	Buffer & buf = bv->buffer();

	if (!searchAllowed(searchstr) || buf.isReadonly())
		return 0;

	DocIterator cur_orig(bv->cursor());

	MatchString const match(searchstr, case_sens, whole);
	int num = 0;

	int const rsize = replacestr.size();
	int const ssize = searchstr.size();

	Cursor cur(*bv);
	cur.setCursor(doc_iterator_begin(&buf));
	int match_len = findForward(cur, match, false);
	while (match_len > 0) {
		// Backup current cursor position and font.
		pos_type const pos = cur.pos();
		Font const font = cur.paragraph().getFontSettings(buf.params(), pos);
		cur.recordUndo();
		int striked = ssize -
			cur.paragraph().eraseChars(pos, pos + match_len,
						   buf.params().track_changes);
		cur.paragraph().insert(pos, replacestr, font,
				       Change(buf.params().track_changes
					      ? Change::INSERTED
					      : Change::UNCHANGED));
		for (int i = 0; i < rsize + striked; ++i)
			cur.forwardChar();
		++num;
		match_len = findForward(cur, match, false);
	}

	bv->putSelectionAt(doc_iterator_begin(&buf), 0, false);

	cur_orig.fixIfBroken();
	bv->setCursor(cur_orig);

	return num;
}


// the idea here is that we are going to replace the string that
// is selected IF it is the search string.
// if there is a selection, but it is not the search string, then
// we basically ignore it. (FIXME We ought to replace only within
// the selection.)
// if there is no selection, then:
//  (i) if some search string has been provided, then we find it.
//      (think of how the dialog works when you hit "replace" the
//      first time.)
// (ii) if no search string has been provided, then we treat the
//      word the cursor is in as the search string. (why? i have no
//      idea.) but this only works in text?
//
// returns the number of replacements made (one, if any) and
// whether anything at all was done.
pair<bool, int> replaceOne(BufferView * bv, docstring searchstr,
			   docstring const & replacestr, bool case_sens,
			   bool whole, bool forward, bool findnext)
{
	Cursor & cur = bv->cursor();
	bool found = false;
	if (!cur.selection()) {
		// no selection, non-empty search string: find it
		if (!searchstr.empty()) {
			found = findOne(bv, searchstr, case_sens, whole, forward, true, findnext);
			return make_pair(found, 0);
		}
		// empty search string
		if (!cur.inTexted())
			// bail in math
			return make_pair(false, 0);
		// select current word and treat it as the search string.
		// This causes a minor bug as undo will restore this selection,
		// which the user did not create (#8986).
		cur.innerText()->selectWord(cur, WHOLE_WORD);
		searchstr = cur.selectionAsString(false);
	}

	// if we still don't have a search string, report the error
	// and abort.
	if (!searchAllowed(searchstr))
		return make_pair(false, 0);

	bool have_selection = cur.selection();
	docstring const selected = cur.selectionAsString(false);
	bool match =
		case_sens
		? searchstr == selected
		: compare_no_case(searchstr, selected) == 0;

	// no selection or current selection is not search word:
	// just find the search word
	if (!have_selection || !match) {
		found = findOne(bv, searchstr, case_sens, whole, forward, true, findnext);
		return make_pair(found, 0);
	}

	// we're now actually ready to replace. if the buffer is
	// read-only, we can't, though.
	if (bv->buffer().isReadonly())
		return make_pair(false, 0);

	cap::replaceSelectionWithString(cur, replacestr);
	if (forward) {
		cur.pos() += replacestr.length();
		LASSERT(cur.pos() <= cur.lastpos(),
		        cur.pos() = cur.lastpos());
	}
	if (findnext)
		findOne(bv, searchstr, case_sens, whole, forward, false, findnext);

	return make_pair(true, 1);
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
			       docstring const & search,
			       bool casesensitive, bool matchword,
			       bool all, bool forward, bool findnext)
{
	odocstringstream ss;
	ss << replace << '\n'
	   << search << '\n'
	   << int(casesensitive) << ' '
	   << int(matchword) << ' '
	   << int(all) << ' '
	   << int(forward) << ' '
	   << int(findnext);
	return ss.str();
}


bool lyxfind(BufferView * bv, FuncRequest const & ev)
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

	return findOne(bv, search, casesensitive, matchword, forward, true, true);
}


bool lyxreplace(BufferView * bv,
		FuncRequest const & ev, bool has_deleted)
{
	if (!bv || ev.action() != LFUN_WORD_REPLACE)
		return false;

	// data is of the form
	// "<search>
	//  <replace>
	//  <casesensitive> <matchword> <all> <forward> <findnext>"
	docstring search;
	docstring rplc;
	docstring howto = split(ev.argument(), rplc, '\n');
	howto = split(howto, search, '\n');

	bool casesensitive = parse_bool(howto);
	bool matchword     = parse_bool(howto);
	bool all           = parse_bool(howto);
	bool forward       = parse_bool(howto);
	bool findnext      = howto.empty() ? true : parse_bool(howto);

	bool update = false;

	if (!has_deleted) {
		int replace_count = 0;
		if (all) {
			replace_count = replaceAll(bv, search, rplc, casesensitive, matchword);
			update = replace_count > 0;
		} else {
			pair<bool, int> rv =
				replaceOne(bv, search, rplc, casesensitive, matchword, forward, findnext);
			update = rv.first;
			replace_count = rv.second;
		}

		Buffer const & buf = bv->buffer();
		if (!update) {
			// emit message signal.
			buf.message(_("String not found."));
		} else {
			if (replace_count == 0) {
				buf.message(_("String found."));
			} else if (replace_count == 1) {
				buf.message(_("String has been replaced."));
			} else {
				docstring const str =
					bformat(_("%1$d strings have been replaced."), replace_count);
				buf.message(str);
			}
		}
	} else if (findnext) {
		// if we have deleted characters, we do not replace at all, but
		// rather search for the next occurence
		if (findOne(bv, search, casesensitive, matchword, forward, true, findnext))
			update = true;
		else
			bv->message(_("String not found."));
	}
	return update;
}


bool findNextChange(DocIterator & cur)
{
	for (; cur; cur.forwardPos())
		if (cur.inTexted() && cur.paragraph().isChanged(cur.pos()))
			return true;
	return false;
}


bool findPreviousChange(DocIterator & cur)
{
	for (cur.backwardPos(); cur; cur.backwardPos()) {
		if (cur.inTexted() && cur.paragraph().isChanged(cur.pos()))
			return true;
	}
	return false;
}


bool selectChange(Cursor & cur, bool forward)
{
	if (!cur.inTexted() || !cur.paragraph().isChanged(cur.pos()))
		return false;
	Change ch = cur.paragraph().lookupChange(cur.pos());

	CursorSlice tip1 = cur.top();
	for (; tip1.pit() < tip1.lastpit() || tip1.pos() < tip1.lastpos(); tip1.forwardPos()) {
		Change ch2 = tip1.paragraph().lookupChange(tip1.pos());
		if (!ch2.isSimilarTo(ch))
			break;
	}
	CursorSlice tip2 = cur.top();
	for (; tip2.pit() > 0 || tip2.pos() > 0;) {
		tip2.backwardPos();
		Change ch2 = tip2.paragraph().lookupChange(tip2.pos());
		if (!ch2.isSimilarTo(ch)) {
			// take a step forward to correctly set the selection
			tip2.forwardPos();
			break;
		}
	}
	if (forward)
		swap(tip1, tip2);
	cur.top() = tip1;
	cur.bv().mouseSetCursor(cur, false);
	cur.top() = tip2;
	cur.bv().mouseSetCursor(cur, true);
	return true;
}


namespace {


bool findChange(BufferView * bv, bool forward)
{
	Cursor cur(*bv);
	cur.setCursor(forward ? bv->cursor().selectionEnd()
		      : bv->cursor().selectionBegin());
	forward ? findNextChange(cur) : findPreviousChange(cur);
	return selectChange(cur, forward);
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



namespace {

typedef vector<pair<string, string> > Escapes;

/// A map of symbols and their escaped equivalent needed within a regex.
/// @note Beware of order
Escapes const & get_regexp_escapes()
{
	typedef std::pair<std::string, std::string> P;

	static Escapes escape_map;
	if (escape_map.empty()) {
		escape_map.push_back(P("$", "_x_$"));
		escape_map.push_back(P("{", "_x_{"));
		escape_map.push_back(P("}", "_x_}"));
		escape_map.push_back(P("[", "_x_["));
		escape_map.push_back(P("]", "_x_]"));
		escape_map.push_back(P("(", "_x_("));
		escape_map.push_back(P(")", "_x_)"));
		escape_map.push_back(P("+", "_x_+"));
		escape_map.push_back(P("*", "_x_*"));
		escape_map.push_back(P(".", "_x_."));
		escape_map.push_back(P("\\", "(?:\\\\|\\\\backslash)"));
		escape_map.push_back(P("~", "(?:\\\\textasciitilde|\\\\sim)"));
		escape_map.push_back(P("^", "(?:\\^|\\\\textasciicircum\\{\\}|\\\\mathcircumflex)"));
		escape_map.push_back(P("_x_", "\\"));
	}
	return escape_map;
}

/// A map of lyx escaped strings and their unescaped equivalent.
Escapes const & get_lyx_unescapes()
{
	typedef std::pair<std::string, std::string> P;

	static Escapes escape_map;
	if (escape_map.empty()) {
		escape_map.push_back(P("\\%", "%"));
		escape_map.push_back(P("\\mathcircumflex ", "^"));
		escape_map.push_back(P("\\mathcircumflex", "^"));
		escape_map.push_back(P("\\backslash ", "\\"));
		escape_map.push_back(P("\\backslash", "\\"));
		escape_map.push_back(P("\\\\{", "_x_<"));
		escape_map.push_back(P("\\\\}", "_x_>"));
		escape_map.push_back(P("\\sim ", "~"));
		escape_map.push_back(P("\\sim", "~"));
	}
	return escape_map;
}

/// A map of escapes turning a regexp matching text to one matching latex.
Escapes const & get_regexp_latex_escapes()
{
	typedef std::pair<std::string, std::string> P;

	static Escapes escape_map;
	if (escape_map.empty()) {
		escape_map.push_back(P("\\\\", "(?:\\\\\\\\|\\\\backslash|\\\\textbackslash\\{\\})"));
		escape_map.push_back(P("(<?!\\\\\\\\textbackslash)\\{", "\\\\\\{"));
		escape_map.push_back(P("(<?!\\\\\\\\textbackslash\\\\\\{)\\}", "\\\\\\}"));
		escape_map.push_back(P("\\[", "\\{\\[\\}"));
		escape_map.push_back(P("\\]", "\\{\\]\\}"));
		escape_map.push_back(P("\\^", "(?:\\^|\\\\textasciicircum\\{\\}|\\\\mathcircumflex)"));
		escape_map.push_back(P("%", "\\\\\\%"));
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
			LYXERR(Debug::FIND, "After escape: " << s);
			pos += it->second.length();
//			LYXERR(Debug::FIND, "pos: " << pos);
		}
	}
	LYXERR(Debug::FIND, "Escaped : '" << s << "'");
	return s;
}


/// Within \regexp{} apply get_lyx_unescapes() only (i.e., preserve regexp semantics of the string),
/// while outside apply get_lyx_unescapes()+get_regexp_escapes().
/// If match_latex is true, then apply regexp_latex_escapes() to \regexp{} contents as well.
string escape_for_regex(string s, bool match_latex)
{
	size_t pos = 0;
	while (pos < s.size()) {
		size_t new_pos = s.find("\\regexp{", pos);
		if (new_pos == string::npos)
			new_pos = s.size();
		LYXERR(Debug::FIND, "new_pos: " << new_pos);
		string t = apply_escapes(s.substr(pos, new_pos - pos), get_lyx_unescapes());
		LYXERR(Debug::FIND, "t [lyx]: " << t);
		t = apply_escapes(t, get_regexp_escapes());
		LYXERR(Debug::FIND, "t [rxp]: " << t);
		s.replace(pos, new_pos - pos, t);
		new_pos = pos + t.size();
		LYXERR(Debug::FIND, "Regexp after escaping: " << s);
		LYXERR(Debug::FIND, "new_pos: " << new_pos);
		if (new_pos == s.size())
			break;
		// Might fail if \\endregexp{} is preceeded by unexpected stuff (weird escapes)
		size_t end_pos = s.find("\\endregexp{}}", new_pos + 8);
		LYXERR(Debug::FIND, "end_pos: " << end_pos);
		t = s.substr(new_pos + 8, end_pos - (new_pos + 8));
		LYXERR(Debug::FIND, "t in regexp      : " << t);
		t = apply_escapes(t, get_lyx_unescapes());
		LYXERR(Debug::FIND, "t in regexp [lyx]: " << t);
		if (match_latex) {
			t = apply_escapes(t, get_regexp_latex_escapes());
			LYXERR(Debug::FIND, "t in regexp [ltx]: " << t);
		}
		if (end_pos == s.size()) {
			s.replace(new_pos, end_pos - new_pos, t);
			LYXERR(Debug::FIND, "Regexp after \\regexp{} removal: " << s);
			break;
		}
		s.replace(new_pos, end_pos + 13 - new_pos, t);
		LYXERR(Debug::FIND, "Regexp after \\regexp{...\\endregexp{}} removal: " << s);
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
	string normalize(docstring const & s, bool hack_braces) const;
	// normalized string to search
	string par_as_string;
	// regular expression to use for searching
	lyx::regex regexp;
	// same as regexp, but prefixed with a ".*"
	lyx::regex regexp2;
	// leading format material as string
	string lead_as_string;
	// par_as_string after removal of lead_as_string
	string par_as_string_nolead;
	// unmatched open braces in the search string/regexp
	int open_braces;
	// number of (.*?) subexpressions added at end of search regexp for closing
	// environments, math mode, styles, etc...
	int close_wildcards;
	// Are we searching with regular expressions ?
	bool use_regexp;
};


static docstring buffer_to_latex(Buffer & buffer)
{
	OutputParams runparams(&buffer.params().encoding());
	odocstringstream ods;
	otexstream os(ods, false);
	runparams.nice = true;
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = 80; //lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;
	pit_type const endpit = buffer.paragraphs().size();
	for (pit_type pit = 0; pit != endpit; ++pit) {
		TeXOnePar(buffer, buffer.text(), pit, os, runparams);
		LYXERR(Debug::FIND, "searchString up to here: " << ods.str());
	}
	return ods.str();
}


static docstring stringifySearchBuffer(Buffer & buffer, FindAndReplaceOptions const & opt)
{
	docstring str;
	if (!opt.ignoreformat) {
		str = buffer_to_latex(buffer);
	} else {
		OutputParams runparams(&buffer.params().encoding());
		runparams.nice = true;
		runparams.flavor = OutputParams::LATEX;
		runparams.linelen = 100000; //lyxrc.plaintext_linelen;
		runparams.dryrun = true;
		runparams.for_search = true;
		for (pos_type pit = pos_type(0); pit < (pos_type)buffer.paragraphs().size(); ++pit) {
			Paragraph const & par = buffer.paragraphs().at(pit);
			LYXERR(Debug::FIND, "Adding to search string: '"
			       << par.asString(pos_type(0), par.size(),
					       AS_STR_INSETS | AS_STR_SKIPDELETE | AS_STR_PLAINTEXT,
					       &runparams)
			       << "'");
			str += par.asString(pos_type(0), par.size(),
					    AS_STR_INSETS | AS_STR_SKIPDELETE | AS_STR_PLAINTEXT,
					    &runparams);
		}
	}
	return str;
}


/// Return separation pos between the leading material and the rest
static size_t identifyLeading(string const & s)
{
	string t = s;
	// @TODO Support \item[text]
	while (regex_replace(t, t, "^\\\\(emph|textbf|subsubsection|subsection|section|subparagraph|paragraph|part)\\*?\\{", "")
	       || regex_replace(t, t, "^\\$", "")
	       || regex_replace(t, t, "^\\\\\\[ ", "")
	       || regex_replace(t, t, "^\\\\item ", "")
	       || regex_replace(t, t, "^\\\\begin\\{[a-zA-Z_]*\\*?\\} ", ""))
		LYXERR(Debug::FIND, "  after removing leading $, \\[ , \\emph{, \\textbf{, etc.: '" << t << "'");
	return s.find(t);
}


// Remove trailing closure of math, macros and environments, so to catch parts of them.
static int identifyClosing(string & t)
{
	int open_braces = 0;
	do {
		LYXERR(Debug::FIND, "identifyClosing(): t now is '" << t << "'");
		if (regex_replace(t, t, "(.*[^\\\\])\\$\\'", "$1"))
			continue;
		if (regex_replace(t, t, "(.*[^\\\\]) \\\\\\]\\'", "$1"))
			continue;
		if (regex_replace(t, t, "(.*[^\\\\]) \\\\end\\{[a-zA-Z_]*\\*?\\}\\'", "$1"))
			continue;
		if (regex_replace(t, t, "(.*[^\\\\])\\}\\'", "$1")) {
			++open_braces;
			continue;
		}
		break;
	} while (true);
	return open_braces;
}


MatchStringAdv::MatchStringAdv(lyx::Buffer & buf, FindAndReplaceOptions const & opt)
	: p_buf(&buf), p_first_buf(&buf), opt(opt)
{
	Buffer & find_buf = *theBufferList().getBuffer(FileName(to_utf8(opt.find_buf_name)), true);
	docstring const & ds = stringifySearchBuffer(find_buf, opt);
	use_regexp = lyx::to_utf8(ds).find("\\regexp{") != std::string::npos;
	// When using regexp, braces are hacked already by escape_for_regex()
	par_as_string = normalize(ds, !use_regexp);
	open_braces = 0;
	close_wildcards = 0;

	size_t lead_size = 0;
	if (opt.ignoreformat) {
		if (!use_regexp) {
			// if par_as_string_nolead were emty,
			// the following call to findAux will always *find* the string
			// in the checked data, and thus always using the slow
			// examining of the current text part.
			par_as_string_nolead = par_as_string;
		}
	} else {
		lead_size = identifyLeading(par_as_string);
		lead_as_string = par_as_string.substr(0, lead_size);
		par_as_string_nolead = par_as_string.substr(lead_size, par_as_string.size() - lead_size);
	}

	if (!use_regexp) {
		open_braces = identifyClosing(par_as_string);
		identifyClosing(par_as_string_nolead);
		LYXERR(Debug::FIND, "Open braces: " << open_braces);
		LYXERR(Debug::FIND, "Built MatchStringAdv object: par_as_string = '" << par_as_string << "'");
	} else {
		string lead_as_regexp;
		if (lead_size > 0) {
			// @todo No need to search for \regexp{} insets in leading material
			lead_as_regexp = escape_for_regex(par_as_string.substr(0, lead_size), !opt.ignoreformat);
			par_as_string = par_as_string_nolead;
			LYXERR(Debug::FIND, "lead_as_regexp is '" << lead_as_regexp << "'");
			LYXERR(Debug::FIND, "par_as_string now is '" << par_as_string << "'");
		}
		par_as_string = escape_for_regex(par_as_string, !opt.ignoreformat);
		// Insert (.*?) before trailing closure of math, macros and environments, so to catch parts of them.
		LYXERR(Debug::FIND, "par_as_string now is '" << par_as_string << "'");
		if (
			// Insert .* before trailing '\$' ('$' has been escaped by escape_for_regex)
			regex_replace(par_as_string, par_as_string, "(.*[^\\\\])(\\\\\\$)\\'", "$1(.*?)$2")
			// Insert .* before trailing '\\\]' ('\]' has been escaped by escape_for_regex)
			|| regex_replace(par_as_string, par_as_string, "(.*[^\\\\])( \\\\\\\\\\\\\\])\\'", "$1(.*?)$2")
			// Insert .* before trailing '\\end\{...}' ('\end{...}' has been escaped by escape_for_regex)
			|| regex_replace(par_as_string, par_as_string,
					 "(.*[^\\\\])( \\\\\\\\end\\\\\\{[a-zA-Z_]*)(\\\\\\*)?(\\\\\\})\\'", "$1(.*?)$2$3$4")
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
		string regexp_str = string("\\`") + lead_as_regexp + par_as_string;
		LYXERR(Debug::FIND, "Setting regexp to : '" << regexp_str << "'");
		regexp = lyx::regex(regexp_str);

		// If entered regexp may match wherever in searched string buffer
		string regexp2_str = string("\\`.*") + lead_as_regexp + ".*" + par_as_string;
		LYXERR(Debug::FIND, "Setting regexp2 to: '" << regexp2_str << "'");
		regexp2 = lyx::regex(regexp2_str);
	}
}


int MatchStringAdv::findAux(DocIterator const & cur, int len, bool at_begin) const
{
	if (at_begin &&
		(opt.restr == FindAndReplaceOptions::R_ONLY_MATHS && !cur.inMathed()) )
		return 0;
	docstring docstr = stringifyFromForSearch(opt, cur, len);
	LYXERR(Debug::FIND, "Matching against     '" << lyx::to_utf8(docstr) << "'");
	string str = normalize(docstr, true);
	LYXERR(Debug::FIND, "After normalization: '" << str << "'");
	if (! use_regexp) {
		LYXERR(Debug::FIND, "Searching in normal mode: par_as_string='" << par_as_string << "', str='" << str << "'");
		LYXERR(Debug::FIND, "Searching in normal mode: lead_as_string='" << lead_as_string << "', par_as_string_nolead='" << par_as_string_nolead << "'");
		if (at_begin) {
			LYXERR(Debug::FIND, "size=" << par_as_string.size() << ", substr='" << str.substr(0, par_as_string.size()) << "'");
			if (str.substr(0, par_as_string.size()) == par_as_string)
				return par_as_string.size();
		} else {
			size_t pos = str.find(par_as_string_nolead);
			if (pos != string::npos)
				return par_as_string.size();
		}
	} else {
		LYXERR(Debug::FIND, "Searching in regexp mode: at_begin=" << at_begin);
		// Try all possible regexp matches,
		//until one that verifies the braces match test is found
		regex const *p_regexp = at_begin ? &regexp : &regexp2;
		sregex_iterator re_it(str.begin(), str.end(), *p_regexp);
		sregex_iterator re_it_end;
		for (; re_it != re_it_end; ++re_it) {
			match_results<string::const_iterator> const & m = *re_it;
			// Check braces on the segment that matched the entire regexp expression,
			// plus the last subexpression, if a (.*?) was inserted in the constructor.
			if (!braces_match(m[0].first, m[0].second, open_braces))
				return 0;
			// Check braces on segments that matched all (.*?) subexpressions,
			// except the last "padding" one inserted by lyx.
			for (size_t i = 1; i < m.size() - 1; ++i)
				if (!braces_match(m[i].first, m[i].second))
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
	LYXERR(Debug::FIND,
	       "res=" << res << ", at_begin=" << at_begin
	       << ", matchword=" << opt.matchword
	       << ", inTexted=" << cur.inTexted());
	if (res == 0 || !at_begin || !opt.matchword || !cur.inTexted())
		return res;
	Paragraph const & par = cur.paragraph();
	bool ws_left = (cur.pos() > 0)
		? par.isWordSeparator(cur.pos() - 1)
		: true;
	bool ws_right = (cur.pos() + res < par.size())
		? par.isWordSeparator(cur.pos() + res)
		: true;
	LYXERR(Debug::FIND,
	       "cur.pos()=" << cur.pos() << ", res=" << res
	       << ", separ: " << ws_left << ", " << ws_right
	       << endl);
	if (ws_left && ws_right)
		return res;
	return 0;
}


string MatchStringAdv::normalize(docstring const & s, bool hack_braces) const
{
	string t;
	if (! opt.casesensitive)
		t = lyx::to_utf8(lowercase(s));
	else
		t = lyx::to_utf8(s);
	// Remove \n at begin
	while (!t.empty() && t[0] == '\n')
		t = t.substr(1);
	// Remove \n at end
	while (!t.empty() && t[t.size() - 1] == '\n')
		t = t.substr(0, t.size() - 1);
	size_t pos;
	// Replace all other \n with spaces
	while ((pos = t.find("\n")) != string::npos)
		t.replace(pos, 1, " ");
	// Remove stale empty \emph{}, \textbf{} and similar blocks from latexify
	LYXERR(Debug::FIND, "Removing stale empty \\emph{}, \\textbf{}, \\*section{} macros from: " << t);
	while (regex_replace(t, t, "\\\\(emph|textbf|subsubsection|subsection|section|subparagraph|paragraph|part)(\\{\\})+", ""))
		LYXERR(Debug::FIND, "  further removing stale empty \\emph{}, \\textbf{} macros from: " << t);

	// FIXME - check what preceeds the brace
	if (hack_braces) {
		if (opt.ignoreformat)
			while (regex_replace(t, t, "\\{", "_x_<")
			       || regex_replace(t, t, "\\}", "_x_>"))
				LYXERR(Debug::FIND, "After {} replacement: '" << t << "'");
		else
			while (regex_replace(t, t, "\\\\\\{", "_x_<")
			       || regex_replace(t, t, "\\\\\\}", "_x_>"))
				LYXERR(Debug::FIND, "After {} replacement: '" << t << "'");
	}

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
		runparams.nice = true;
		runparams.flavor = OutputParams::LATEX;
		runparams.linelen = 100000; //lyxrc.plaintext_linelen;
		// No side effect of file copying and image conversion
		runparams.dryrun = true;
		LYXERR(Debug::FIND, "Stringifying with cur: "
		       << cur << ", from pos: " << cur.pos() << ", end: " << end);
		return par.asString(cur.pos(), end,
			AS_STR_INSETS | AS_STR_SKIPDELETE | AS_STR_PLAINTEXT,
			&runparams);
	} else if (cur.inMathed()) {
		docstring s;
		CursorSlice cs = cur.top();
		MathData md = cs.cell();
		MathData::const_iterator it_end =
			(( len == -1 || cs.pos() + len > int(md.size()))
			 ? md.end()
			 : md.begin() + cs.pos() + len );
		for (MathData::const_iterator it = md.begin() + cs.pos();
		     it != it_end; ++it)
			s = s + asString(*it);
		LYXERR(Debug::FIND, "Stringified math: '" << s << "'");
		return s;
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
	LBUFERR(buf.params().isLatex());

	odocstringstream ods;
	otexstream os(ods, false);
	OutputParams runparams(&buf.params().encoding());
	runparams.nice = false;
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = 8000; //lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;

	if (cur.inTexted()) {
		// @TODO what about searching beyond/across paragraph breaks ?
		pos_type endpos = cur.paragraph().size();
		if (len != -1 && endpos > cur.pos() + len)
			endpos = cur.pos() + len;
		TeXOnePar(buf, *cur.innerText(), cur.pit(), os, runparams,
			  string(), cur.pos(), endpos);
		LYXERR(Debug::FIND, "Latexified text: '" << lyx::to_utf8(ods.str()) << "'");
	} else if (cur.inMathed()) {
		// Retrieve the math environment type, and add '$' or '$[' or others (\begin{equation}) accordingly
		for (int s = cur.depth() - 1; s >= 0; --s) {
			CursorSlice const & cs = cur[s];
			if (cs.asInsetMath() && cs.asInsetMath()->asHullInset()) {
				WriteStream ws(os);
				cs.asInsetMath()->asHullInset()->header_write(ws);
				break;
			}
		}

		CursorSlice const & cs = cur.top();
		MathData md = cs.cell();
		MathData::const_iterator it_end =
			((len == -1 || cs.pos() + len > int(md.size()))
			 ? md.end()
			 : md.begin() + cs.pos() + len);
		for (MathData::const_iterator it = md.begin() + cs.pos();
		     it != it_end; ++it)
			ods << asString(*it);

		// Retrieve the math environment type, and add '$' or '$]'
		// or others (\end{equation}) accordingly
		for (int s = cur.depth() - 1; s >= 0; --s) {
			CursorSlice const & cs = cur[s];
			InsetMath * inset = cs.asInsetMath();
			if (inset && inset->asHullInset()) {
				WriteStream ws(os);
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
	LASSERT(match(cur) > 0, return 0);
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
	while (!theApp()->longOperationCancelled() && cur) {
		LYXERR(Debug::FIND, "findForwardAdv() cur: " << cur);
		int match_len = match(cur, -1, false);
		LYXERR(Debug::FIND, "match_len: " << match_len);
		if (match_len) {
			for (; !theApp()->longOperationCancelled() && cur; cur.forwardPos()) {
				LYXERR(Debug::FIND, "Advancing cur: " << cur);
				int match_len = match(cur);
				LYXERR(Debug::FIND, "match_len: " << match_len);
				if (match_len) {
					// Sometimes in finalize we understand it wasn't a match
					// and we need to continue the outest loop
					int len = findAdvFinalize(cur, match);
					if (len > 0)
						return len;
				}
			}
			if (!cur)
				return 0;
		}
		if (cur.pit() < cur.lastpit()) {
			LYXERR(Debug::FIND, "Advancing par: cur=" << cur);
			cur.forwardPar();
		} else {
			// This should exit nested insets, if any, or otherwise undefine the currsor.
			cur.pos() = cur.lastpos();
			LYXERR(Debug::FIND, "Advancing pos: cur=" << cur);
			cur.forwardPos();
		}
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
int findBackwardsAdv(DocIterator & cur, MatchStringAdv & match)
{
	if (! cur)
		return 0;
	// Backup of original position
	DocIterator cur_begin = doc_iterator_begin(cur.buffer());
	if (cur == cur_begin)
		return 0;
	cur.backwardPos();
	DocIterator cur_orig(cur);
	bool pit_changed = false;
	do {
		cur.pos() = 0;
		bool found_match = match(cur, -1, false);

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
	} while (!theApp()->longOperationCancelled());
	return 0;
}


} // anonym namespace


docstring stringifyFromForSearch(FindAndReplaceOptions const & opt,
				 DocIterator const & cur, int len)
{
	LASSERT(cur.pos() >= 0 && cur.pos() <= cur.lastpos(),
	        return docstring());
	if (!opt.ignoreformat)
		return latexifyFromCursor(cur, len);
	else
		return stringifyFromCursor(cur, len);
}


FindAndReplaceOptions::FindAndReplaceOptions(
	docstring const & find_buf_name, bool casesensitive,
	bool matchword, bool forward, bool expandmacros, bool ignoreformat,
	docstring const & repl_buf_name, bool keep_case,
	SearchScope scope, SearchRestriction restr)
	: find_buf_name(find_buf_name), casesensitive(casesensitive), matchword(matchword),
	  forward(forward), expandmacros(expandmacros), ignoreformat(ignoreformat),
	  repl_buf_name(repl_buf_name), keep_case(keep_case), scope(scope), restr(restr)
{
}


namespace {


/** Check if 'len' letters following cursor are all non-lowercase */
static bool allNonLowercase(Cursor const & cur, int len)
{
	pos_type beg_pos = cur.selectionBegin().pos();
	pos_type end_pos = cur.selectionBegin().pos() + len;
	if (len > cur.lastpos() + 1 - beg_pos) {
		LYXERR(Debug::FIND, "This should not happen, more debug needed");
		len = cur.lastpos() + 1 - beg_pos;
		end_pos = beg_pos + len;
	}
	for (pos_type pos = beg_pos; pos != end_pos; ++pos)
		if (isLowerCase(cur.paragraph().getChar(pos)))
			return false;
	return true;
}


/** Check if first letter is upper case and second one is lower case */
static bool firstUppercase(Cursor const & cur)
{
	char_type ch1, ch2;
	pos_type pos = cur.selectionBegin().pos();
	if (pos >= cur.lastpos() - 1) {
		LYXERR(Debug::FIND, "No upper-case at cur: " << cur);
		return false;
	}
	ch1 = cur.paragraph().getChar(pos);
	ch2 = cur.paragraph().getChar(pos + 1);
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
static void changeFirstCase(Buffer & buffer, TextCase first_case, TextCase others_case)
{
	ParagraphList::iterator pit = buffer.paragraphs().begin();
	LASSERT(pit->size() >= 1, /**/);
	pos_type right = pos_type(1);
	pit->changeCase(buffer.params(), pos_type(0), right, first_case);
	right = pit->size();
	pit->changeCase(buffer.params(), pos_type(1), right, others_case);
}

} // anon namespace

///
static void findAdvReplace(BufferView * bv, FindAndReplaceOptions const & opt, MatchStringAdv & matchAdv)
{
	Cursor & cur = bv->cursor();
	if (opt.repl_buf_name == docstring())
		return;

	DocIterator sel_beg = cur.selectionBegin();
	DocIterator sel_end = cur.selectionEnd();
	if (&sel_beg.inset() != &sel_end.inset()
	    || sel_beg.pit() != sel_end.pit()
	    || sel_beg.idx() != sel_end.idx())
		return;
	int sel_len = sel_end.pos() - sel_beg.pos();
	LYXERR(Debug::FIND, "sel_beg: " << sel_beg << ", sel_end: " << sel_end
	       << ", sel_len: " << sel_len << endl);
	if (sel_len == 0)
		return;
	LASSERT(sel_len > 0, return);

	if (!matchAdv(sel_beg, sel_len))
		return;

	// Build a copy of the replace buffer, adapted to the KeepCase option
	Buffer & repl_buffer_orig = *theBufferList().getBuffer(FileName(to_utf8(opt.repl_buf_name)), true);
	ostringstream oss;
	repl_buffer_orig.write(oss);
	string lyx = oss.str();
	Buffer repl_buffer("", false);
	repl_buffer.setUnnamed(true);
	LASSERT(repl_buffer.readString(lyx), return);
	if (opt.keep_case && sel_len >= 2) {
		LYXERR(Debug::FIND, "keep_case true: cur.pos()=" << cur.pos() << ", sel_len=" << sel_len);
		if (cur.inTexted()) {
			if (firstUppercase(cur))
				changeFirstCase(repl_buffer, text_uppercase, text_lowercase);
			else if (allNonLowercase(cur, sel_len))
				changeFirstCase(repl_buffer, text_uppercase, text_uppercase);
		}
	}
	cap::cutSelection(cur, false, false);
	if (cur.inTexted()) {
		repl_buffer.changeLanguage(
			repl_buffer.language(),
			cur.getFont().language());
		LYXERR(Debug::FIND, "Replacing by pasteParagraphList()ing repl_buffer");
		LYXERR(Debug::FIND, "Before pasteParagraphList() cur=" << cur << endl);
		cap::pasteParagraphList(cur, repl_buffer.paragraphs(),
					repl_buffer.params().documentClassPtr(),
					bv->buffer().errorList("Paste"));
		LYXERR(Debug::FIND, "After pasteParagraphList() cur=" << cur << endl);
		sel_len = repl_buffer.paragraphs().begin()->size();
	} else if (cur.inMathed()) {
		odocstringstream ods;
		otexstream os(ods, false);
		OutputParams runparams(&repl_buffer.params().encoding());
		runparams.nice = false;
		runparams.flavor = OutputParams::LATEX;
		runparams.linelen = 8000; //lyxrc.plaintext_linelen;
		runparams.dryrun = true;
		TeXOnePar(repl_buffer, repl_buffer.text(), 0, os, runparams);
		//repl_buffer.getSourceCode(ods, 0, repl_buffer.paragraphs().size(), false);
		docstring repl_latex = ods.str();
		LYXERR(Debug::FIND, "Latexified replace_buffer: '" << repl_latex << "'");
		string s;
		// false positive from coverity
		// coverity[CHECKED_RETURN]
		regex_replace(to_utf8(repl_latex), s, "\\$(.*)\\$", "$1");
		regex_replace(s, s, "\\\\\\[(.*)\\\\\\]", "$1");
		repl_latex = from_utf8(s);
		LYXERR(Debug::FIND, "Replacing by insert()ing latex: '" << repl_latex << "' cur=" << cur << " with depth=" << cur.depth());
		MathData ar(cur.buffer());
		asArray(repl_latex, ar, Parse::NORMAL);
		cur.insert(ar);
		sel_len = ar.size();
		LYXERR(Debug::FIND, "After insert() cur=" << cur << " with depth: " << cur.depth() << " and len: " << sel_len);
	}
	if (cur.pos() >= sel_len)
		cur.pos() -= sel_len;
	else
		cur.pos() = 0;
	LYXERR(Debug::FIND, "After pos adj cur=" << cur << " with depth: " << cur.depth() << " and len: " << sel_len);
	bv->putSelectionAt(DocIterator(cur), sel_len, !opt.forward);
	bv->processUpdateFlags(Update::Force);
}


/// Perform a FindAdv operation.
bool findAdv(BufferView * bv, FindAndReplaceOptions const & opt)
{
	DocIterator cur;
	int match_len = 0;

	try {
		MatchStringAdv matchAdv(bv->buffer(), opt);
		int length = bv->cursor().selectionEnd().pos() - bv->cursor().selectionBegin().pos();
		if (length > 0)
			bv->putSelectionAt(bv->cursor().selectionBegin(), length, !opt.forward);
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
	os << to_utf8(opt.find_buf_name) << "\nEOSS\n"
	   << opt.casesensitive << ' '
	   << opt.matchword << ' '
	   << opt.forward << ' '
	   << opt.expandmacros << ' '
	   << opt.ignoreformat << ' '
	   << to_utf8(opt.repl_buf_name) << "\nEOSS\n"
	   << opt.keep_case << ' '
	   << int(opt.scope) << ' '
	   << int(opt.restr);

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
	LYXERR(Debug::FIND, "file_buf_name: '" << s << "'");
	opt.find_buf_name = from_utf8(s);
	is >> opt.casesensitive >> opt.matchword >> opt.forward >> opt.expandmacros >> opt.ignoreformat;
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
	LYXERR(Debug::FIND, "repl_buf_name: '" << s << "'");
	opt.repl_buf_name = from_utf8(s);
	is >> opt.keep_case;
	int i;
	is >> i;
	opt.scope = FindAndReplaceOptions::SearchScope(i);
	is >> i;
	opt.restr = FindAndReplaceOptions::SearchRestriction(i);

	LYXERR(Debug::FIND, "parsed: " << opt.casesensitive << ' ' << opt.matchword << ' ' << opt.forward << ' '
	       << opt.expandmacros << ' ' << opt.ignoreformat << ' ' << opt.keep_case << ' '
	       << opt.scope << ' ' << opt.restr);
	return is;
}

} // lyx namespace
