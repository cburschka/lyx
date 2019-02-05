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
#include "mathed/MathData.h"
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
#include <map>

using namespace std;
using namespace lyx::support;

namespace lyx {


// Helper class for deciding what should be ignored
class IgnoreFormats {
 public:
	///
	IgnoreFormats()
		: ignoreFamily_(false), ignoreSeries_(false),
		  ignoreShape_(false), ignoreUnderline_(false),
		  ignoreMarkUp_(false), ignoreStrikeOut_(false),
		  ignoreSectioning_(false), ignoreFrontMatter_(true),
		  ignoreColor_(false), ignoreLanguage_(false) {}
	///
	bool getFamily() { return ignoreFamily_; };
	///
	bool getSeries() { return ignoreSeries_; };
	///
	bool getShape() { return ignoreShape_; };
	///
	bool getUnderline() { return ignoreUnderline_; };
	///
	bool getMarkUp() { return ignoreMarkUp_; };
	///
	bool getStrikeOut() { return ignoreStrikeOut_; };
	///
	bool getSectioning() { return ignoreSectioning_; };
	///
	bool getFrontMatter() { return ignoreFrontMatter_; };
	///
	bool getColor() { return ignoreColor_; };
	///
	bool getLanguage() { return ignoreLanguage_; };
	///
	void setIgnoreFormat(string type, bool value);

private:
	///
	bool ignoreFamily_;
	///
	bool ignoreSeries_;
	///
	bool ignoreShape_;
	///
	bool ignoreUnderline_;
	///
	bool ignoreMarkUp_;
	///
	bool ignoreStrikeOut_;
	///
	bool ignoreSectioning_;
	///
	bool ignoreFrontMatter_;
	///
	bool ignoreColor_;
	///
	bool ignoreLanguage_;
};


void IgnoreFormats::setIgnoreFormat(string type, bool value)
{
	if (type == "color") {
		ignoreColor_ = value;
	}
	else if (type == "language") {
		ignoreLanguage_ = value;
	}
	else if (type == "sectioning") {
		ignoreSectioning_ = value;
		ignoreFrontMatter_ = value;
	}
	else if (type == "font") {
		ignoreSeries_ = value;
		ignoreShape_ = value;
		ignoreFamily_ = value;
	}
	else if (type == "series") {
		ignoreSeries_ = value;
	}
	else if (type == "shape") {
		ignoreShape_ = value;
	}
	else if (type == "family") {
		ignoreFamily_ = value;
	}
	else if (type == "markup") {
		ignoreMarkUp_ = value;
	}
	else if (type == "underline") {
		ignoreUnderline_ = value;
	}
	else if (type == "strike") {
		ignoreStrikeOut_ = value;
	}
}

// The global variable that can be changed from outside
IgnoreFormats ignoreFormats;


void setIgnoreFormat(string type, bool value)
{
	ignoreFormats.setIgnoreFormat(type, value);
}


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

} // namespace


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


bool findNextChange(BufferView * bv, Cursor & cur, bool const check_wrap)
{
	for (; cur; cur.forwardPos())
		if (cur.inTexted() && cur.paragraph().isChanged(cur.pos()))
			return true;

	if (check_wrap) {
		DocIterator cur_orig(bv->cursor());
		docstring q = _("End of file reached while searching forward.\n"
			  "Continue searching from the beginning?");
		int wrap_answer = frontend::Alert::prompt(_("Wrap search?"),
			q, 0, 1, _("&Yes"), _("&No"));
		if (wrap_answer == 0) {
			bv->cursor().clear();
			bv->cursor().push_back(CursorSlice(bv->buffer().inset()));
			bv->clearSelection();
			cur.setCursor(bv->cursor().selectionBegin());
			if (findNextChange(bv, cur, false))
				return true;
		}
		bv->cursor().setCursor(cur_orig);
	}

	return false;
}


bool findPreviousChange(BufferView * bv, Cursor & cur, bool const check_wrap)
{
	for (cur.backwardPos(); cur; cur.backwardPos()) {
		if (cur.inTexted() && cur.paragraph().isChanged(cur.pos()))
			return true;
	}

	if (check_wrap) {
		DocIterator cur_orig(bv->cursor());
		docstring q = _("Beginning of file reached while searching backward.\n"
			  "Continue searching from the end?");
		int wrap_answer = frontend::Alert::prompt(_("Wrap search?"),
			q, 0, 1, _("&Yes"), _("&No"));
		if (wrap_answer == 0) {
			bv->cursor().setCursor(doc_iterator_end(&bv->buffer()));
			bv->cursor().backwardPos();
			bv->clearSelection();
			cur.setCursor(bv->cursor().selectionBegin());
			if (findPreviousChange(bv, cur, false))
				return true;
		}
		bv->cursor().setCursor(cur_orig);
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
	forward ? findNextChange(bv, cur, true) : findPreviousChange(bv, cur, true);
	return selectChange(cur, forward);
}

} // namespace

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
		escape_map.push_back(P("^", "(?:\\^|\\\\textasciicircum\\{\\}|\\\\textasciicircum|\\\\mathcircumflex)"));
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
		escape_map.push_back(P("\\\\", "(?:\\\\\\\\|\\\\backslash|\\\\textbackslash\\{\\}|\\\\textbackslash)"));
		escape_map.push_back(P("(<?!\\\\\\\\textbackslash)\\{", "\\\\\\{"));
		escape_map.push_back(P("(<?!\\\\\\\\textbackslash\\\\\\{)\\}", "\\\\\\}"));
		escape_map.push_back(P("\\[", "\\{\\[\\}"));
		escape_map.push_back(P("\\]", "\\{\\]\\}"));
		escape_map.push_back(P("\\^", "(?:\\^|\\\\textasciicircum\\{\\}|\\\\textasciicircum|\\\\mathcircumflex)"));
		escape_map.push_back(P("%", "\\\\\\%"));
		escape_map.push_back(P("#", "\\\\#"));
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
	lyx::regex e(searchstr, regex_constants::ECMAScript);
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
	otexstream os(ods);
	runparams.nice = true;
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = 100000; //lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;
	runparams.for_search = true;
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
	// Kornel: Added textsl, textsf, textit, texttt and noun
	// + allow to search for colored text too
	while (regex_replace(t, t, REGEX_BOS "\\\\(((footnotesize|tiny|scriptsize|small|large|Large|LARGE|huge|Huge|emph|noun|minisec|text(bf|md|sl|sf|it|tt))|((textcolor|foreignlanguage)\\{[a-z]+\\})|(u|uu)line|(s|x)out|uwave)|((sub)?(((sub)?section)|paragraph)|part|chapter)\\*?)\\{", "")
	       || regex_replace(t, t, REGEX_BOS "\\$", "")
	       || regex_replace(t, t, REGEX_BOS "\\\\\\[ ", "")
	       || regex_replace(t, t, REGEX_BOS " ?\\\\item\\{[a-z]+\\}", "")
	       || regex_replace(t, t, REGEX_BOS "\\\\begin\\{[a-zA-Z_]*\\*?\\} ", ""))
	       ;
	LYXERR(Debug::FIND, "  after removing leading $, \\[ , \\emph{, \\textbf{, etc.: '" << t << "'");
	return s.find(t);
}

/*
 * Given a latexified string, retrieve some handled features
 * The features of the regex will later be compared with the features
 * of the searched text. If the regex features are not a
 * subset of the analized, then, in not format ignoring search
 * we can early stop the search in the relevant inset.
 */
typedef map<string, bool> Features;

static Features identifyFeatures(string const & s)
{
	static regex const feature("\\\\(([a-z]+(\\{([a-z]+)\\}|\\*)?))\\{");
	static regex const valid("^(((footnotesize|tiny|scriptsize|small|large|Large|LARGE|huge|Huge|emph|noun|text(bf|md|sl|sf|it|tt)|(textcolor|foreignlanguage|item)\\{[a-z]+\\})|(u|uu)line|(s|x)out|uwave)|((sub)?(((sub)?section)|paragraph)|part|chapter)\\*?)$");
	smatch sub;
	bool displ = true;
	Features info;

	for (sregex_iterator it(s.begin(), s.end(), feature), end; it != end; ++it) {
		sub = *it;
		if (displ) {
			if (sub.str(1).compare("regexp") == 0) {
				displ = false;
				continue;
			}
			string token = sub.str(1);
			smatch sub2;
			if (regex_match(token, sub2, valid)) {
				info[token] = true;
			}
			else {
				// ignore
			}
		}
		else {
			if (sub.str(1).compare("endregexp") == 0) {
				displ = true;
				continue;
			}
		}
	}
	return(info);
}

/*
 * defines values features of a key "\\[a-z]+{"
 */
class KeyInfo {
 public:
  enum KeyType {
    /* Char type with content discarded
     * like \hspace{1cm} */
    noContent,
    /* Char, like \backslash */
    isChar,
    /* \part, \section*, ... */
    isSectioning,
    /* \foreignlanguage{ngerman}, ... */
    isMain,
    /* inside \code{} or \footnote{}
     * to discard language in content */
    noMain,
    isRegex,
    /* \begin{eqnarray}...\end{eqnarray}, ... $...$ */
    isMath,
    /* fonts, colors, markups, ... */
    isStandard,
    /* footnotesize, ... large, ...
     * Ignore all of them */
    isSize,
    invalid,
    /* inputencoding, shortcut, ...
     * Discard also content, because they do not help in search */
    doRemove,
    /* item */
    isList,
    /* tex, latex, ... like isChar */
    isIgnored,
    /* like \lettrine[lines=5]{}{} */
    cleanToStart,
    endArguments
  };
 KeyInfo()
   : keytype(invalid),
    head(""),
    _tokensize(-1),
    _tokenstart(-1),
    _dataStart(-1),
    _dataEnd(-1),
    parenthesiscount(1),
    disabled(false),
    used(false)
  {};
 KeyInfo(KeyType type, int parcount, bool disable)
   : keytype(type),
    _tokensize(-1),
    _tokenstart(-1),
    _dataStart(-1),
    _dataEnd(-1),
    parenthesiscount(parcount),
    disabled(disable),
    used(false) {};
  KeyType keytype;
  string head;
  int _tokensize;
  int _tokenstart;
  int _dataStart;
  int _dataEnd;
  int parenthesiscount;
  bool disabled;
  bool used;                            /* by pattern */
};

class Border {
 public:
 Border(int l=0, int u=0) : low(l), upper(u) {};
  int low;
  int upper;
};

#define MAXOPENED 30
class Intervall {
  bool isPatternString;
 public:
 explicit Intervall(bool isPattern) :
  isPatternString(isPattern),
    ignoreidx(-1),
    actualdeptindex(0) { depts[0] = 0; closes[0] = 0;};
  string par;
  int ignoreidx;
  int depts[MAXOPENED];
  int closes[MAXOPENED];
  int actualdeptindex;
  Border borders[2*MAXOPENED];
  // int previousNotIgnored(int);
  int nextNotIgnored(int);
  void handleOpenP(int i);
  void handleCloseP(int i, bool closingAllowed);
  void resetOpenedP(int openPos);
  void addIntervall(int upper);
  void addIntervall(int low, int upper); /* if explicit */
  void setForDefaultLang(int upTo);
  int findclosing(int start, int end, char up, char down);
  void handleParentheses(int lastpos, bool closingAllowed);
  void output(ostringstream &os, int lastpos);
  // string show(int lastpos);
};

void Intervall::setForDefaultLang(int upTo)
{
  // Enable the use of first token again
  if (ignoreidx >= 0) {
    if (borders[0].low < upTo)
      borders[0].low = upTo;
    if (borders[0].upper < upTo)
      borders[0].upper = upTo;
  }
}

static void checkDepthIndex(int val)
{
  static int maxdepthidx = MAXOPENED-2;
  if (val > maxdepthidx) {
    maxdepthidx = val;
    LYXERR0("maxdepthidx now " << val);
  }
}

static void checkIgnoreIdx(int val)
{
  static int maxignoreidx = 2*MAXOPENED - 4;
  if (val > maxignoreidx) {
    maxignoreidx = val;
    LYXERR0("maxignoreidx now " << val);
  }
}

/*
 * Expand the region of ignored parts of the input latex string
 * The region is only relevant in output()
 */
void Intervall::addIntervall(int low, int upper)
{
  int idx;
  if (low == upper) return;
  for (idx = ignoreidx+1; idx > 0; --idx) {
    if (low > borders[idx-1].upper) {
      break;
    }
  }
  Border br(low, upper);
  if (idx > ignoreidx) {
    borders[idx] = br;
    ignoreidx = idx;
    checkIgnoreIdx(ignoreidx);
    return;
  }
  else {
    // Expand only if one of the new bound is inside the interwall
    // We know here that br.low > borders[idx-1].upper
    if (br.upper < borders[idx].low) {
      // We have to insert at this pos
      for (int i = ignoreidx+1; i > idx; --i) {
        borders[i] = borders[i-1];
      }
      borders[idx] = br;
      ignoreidx += 1;
      checkIgnoreIdx(ignoreidx);
      return;
    }
    // Here we know, that we are overlapping
    if (br.low > borders[idx].low)
      br.low = borders[idx].low;
    // check what has to be concatenated
    int count = 0;
    for (int i = idx; i <= ignoreidx; i++) {
      if (br.upper >= borders[i].low) {
        count++;
        if (br.upper < borders[i].upper)
          br.upper = borders[i].upper;
      }
      else {
        break;
      }
    }
    // count should be >= 1 here
    borders[idx] = br;
    if (count > 1) {
      for (int i = idx + count; i <= ignoreidx; i++) {
        borders[i-count+1] = borders[i];
      }
      ignoreidx -= count - 1;
      return;
    }
  }
}

void Intervall::handleOpenP(int i)
{
  actualdeptindex++;
  depts[actualdeptindex] = i+1;
  closes[actualdeptindex] = -1;
  checkDepthIndex(actualdeptindex);
}

void Intervall::handleCloseP(int i, bool closingAllowed)
{
  if (actualdeptindex <= 0) {
    if (! closingAllowed)
      LYXERR(Debug::FIND, "Bad closing parenthesis in latex");  /* should not happen, but the latex input may be wrong */
    // if we are at the very end
    addIntervall(i, i+1);
  }
  else {
    closes[actualdeptindex] = i+1;
    actualdeptindex--;
  }
}

void Intervall::resetOpenedP(int openPos)
{
  // Used as initializer for foreignlanguage entry
  actualdeptindex = 1;
  depts[1] = openPos+1;
  closes[1] = -1;
}

#if 0
int Intervall::previousNotIgnored(int start)
{
    int idx = 0;                          /* int intervalls */
    for (idx = ignoreidx; idx >= 0; --idx) {
      if (start > borders[idx].upper)
        return start;
      if (start >= borders[idx].low)
        start = borders[idx].low-1;
    }
    return start;
}
#endif

int Intervall::nextNotIgnored(int start)
{
    int idx = 0;                          /* int intervalls */
    for (idx = 0; idx <= ignoreidx; idx++) {
      if (start < borders[idx].low)
        return start;
      if (start < borders[idx].upper)
        start = borders[idx].upper;
    }
    return start;
}

typedef map<string, KeyInfo> KeysMap;
typedef vector< KeyInfo> Entries;
static KeysMap keys = map<string, KeyInfo>();

class LatexInfo {
 private:
  int entidx;
  Entries entries;
  Intervall interval;
  void buildKeys(bool);
  void buildEntries(bool);
  void makeKey(const string &, KeyInfo, bool isPatternString);
  void processRegion(int start, int region_end); /*  remove {} parts */
  void removeHead(KeyInfo&, int count=0);

 public:
 LatexInfo(string par, bool isPatternString) : entidx(-1), interval(isPatternString) {
    interval.par = par;
    buildKeys(isPatternString);
    entries = vector<KeyInfo>();
    buildEntries(isPatternString);
  };
  int getFirstKey() {
    entidx = 0;
    if (entries.empty()) {
      return (-1);
    }
    return 0;
  };
  int getNextKey() {
    entidx++;
    if (int(entries.size()) > entidx) {
      return entidx;
    }
    else {
      return (-1);
    }
  };
  bool setNextKey(int idx) {
    if ((idx == entidx) && (entidx >= 0)) {
      entidx--;
      return true;
    }
    else
      return false;
  };
  int process(ostringstream &os, KeyInfo &actual);
  int dispatch(ostringstream &os, int previousStart, KeyInfo &actual);
  // string show(int lastpos) { return interval.show(lastpos);};
  int nextNotIgnored(int start) { return interval.nextNotIgnored(start);};
  KeyInfo &getKeyInfo(int keyinfo) {
    static KeyInfo invalidInfo = KeyInfo();
    if ((keyinfo < 0) || ( keyinfo >= int(entries.size())))
      return invalidInfo;
    else
      return entries[keyinfo];
  };
  void setForDefaultLang(int upTo) {interval.setForDefaultLang(upTo);};
  void addIntervall(int low, int up) { interval.addIntervall(low, up); };
};


int Intervall::findclosing(int start, int end, char up = '{', char down = '}')
{
  int skip = 0;
  int depth = 0;
  for (int i = start; i < end; i += 1 + skip) {
    char c;
    c = par[i];
    skip = 0;
    if (c == '\\') skip = 1;
    else if (c == up) {
      depth++;
    }
    else if (c == down) {
      if (depth == 0) return i;
      --depth;
    }
  }
  return end;
}

class MathInfo {
  class MathEntry {
  public:
    string wait;
    size_t mathEnd;
    size_t mathStart;
    size_t mathSize;
  };
  size_t actualIdx;
  vector<MathEntry> entries;
 public:
  MathInfo() {
    actualIdx = 0;
  }
  void insert(string wait, size_t start, size_t end) {
    MathEntry m = MathEntry();
    m.wait = wait;
    m.mathStart = start;
    m.mathEnd = end;
    m.mathSize = end - start;
    entries.push_back(m);
  }
  bool empty() { return entries.empty(); };
  size_t getEndPos() {
    if (entries.empty() || (actualIdx >= entries.size())) {
      return 0;
    }
    return entries[actualIdx].mathEnd;
  }
  size_t getStartPos() {
    if (entries.empty() || (actualIdx >= entries.size())) {
      return 100000;                    /*  definitely enough? */
    }
    return entries[actualIdx].mathStart;
  }
  size_t getFirstPos() {
    actualIdx = 0;
    return getStartPos();
  }
  size_t getSize() {
    if (entries.empty() || (actualIdx >= entries.size())) {
      return size_t(0);
    }
    return entries[actualIdx].mathSize;
  }
  void incrEntry() { actualIdx++; };
};

void LatexInfo::buildEntries(bool isPatternString)
{
  static regex const rmath("\\$|\\\\\\[|\\\\\\]|\\\\(begin|end)\\{((eqnarray|equation|flalign|gather|multline|align|alignat)\\*?)\\}");
  static regex const rkeys("\\$|\\\\\\[|\\\\\\]|\\\\((([a-zA-Z]+\\*?)(\\{([a-z]+\\*?)\\}|=[0-9]+[a-z]+)?))");
  static bool disableLanguageOverride = false;
  smatch sub, submath;
  bool evaluatingRegexp = false;
  MathInfo mi;
  bool evaluatingMath = false;
  bool evaluatingCode = false;
  size_t codeEnd = 0;
  int codeStart = -1;
  KeyInfo found;
  bool math_end_waiting = false;
  size_t math_pos = 10000;
  string math_end;

  for (sregex_iterator itmath(interval.par.begin(), interval.par.end(), rmath), end; itmath != end; ++itmath) {
    submath = *itmath;
    if (math_end_waiting) {
      size_t pos = submath.position(size_t(0));
      if ((math_end == "$") &&
          (submath.str(0) == "$") &&
          (interval.par[pos-1] != '\\')) {
        mi.insert("$", math_pos, pos + 1);
        math_end_waiting = false;
      }
      else if ((math_end == "\\]") &&
               (submath.str(0) == "\\]")) {
        mi.insert("\\]", math_pos, pos + 2);
        math_end_waiting = false;
      }
      else if ((submath.str(1).compare("end") == 0) &&
          (submath.str(2).compare(math_end) == 0)) {
        mi.insert(math_end, math_pos, pos + submath.str(0).length());
        math_end_waiting = false;
      }
      else
        continue;
    }
    else {
      if (submath.str(1).compare("begin") == 0) {
        math_end_waiting = true;
        math_end = submath.str(2);
        math_pos = submath.position(size_t(0));
      }
      else if (submath.str(0).compare("\\[") == 0) {
        math_end_waiting = true;
        math_end = "\\]";
        math_pos = submath.position(size_t(0));
      }
      else if (submath.str(0) == "$") {
        size_t pos = submath.position(size_t(0));
        if ((pos == 0) || (interval.par[pos-1] != '\\')) {
          math_end_waiting = true;
          math_end = "$";
          math_pos = pos;
        }
      }
    }
  }
  // Ignore language if there is math somewhere in pattern-string
  if (isPatternString) {
    if (! mi.empty()) {
      // Disable language
      keys["foreignlanguage"].disabled = true;
      disableLanguageOverride = true;
    }
    else
      disableLanguageOverride = false;
  }
  else {
    if (disableLanguageOverride) {
      keys["foreignlanguage"].disabled = true;
    }
  }
  math_pos = mi.getFirstPos();
  for (sregex_iterator it(interval.par.begin(), interval.par.end(), rkeys), end; it != end; ++it) {
    sub = *it;
    string key = sub.str(3);
    if (key == "") {
      if (sub.str(0)[0] == '\\')
        key = sub.str(0)[1];
      else {
        key = sub.str(0);
        if (key == "$") {
          size_t k_pos = sub.position(size_t(0));
          if ((k_pos > 0) && (interval.par[k_pos - 1] == '\\')) {
            // Escaped '$', ignoring
            continue;
          }
        }
      }
    };
    if (evaluatingRegexp) {
      if (sub.str(1).compare("endregexp") == 0) {
        evaluatingRegexp = false;
        // found._tokenstart already set
        found._dataEnd = sub.position(size_t(0)) + 13;
        found._dataStart = found._dataEnd;
        found._tokensize = found._dataEnd - found._tokenstart;
        found.parenthesiscount = 0;
      }
    }
    else {
      if (evaluatingMath) {
        if (size_t(sub.position(size_t(0))) < mi.getEndPos())
          continue;
        evaluatingMath = false;
        mi.incrEntry();
        math_pos = mi.getStartPos();
      }
      if (keys.find(key) == keys.end()) {
        LYXERR(Debug::FIND, "Found unknown key " << sub.str(0));
        continue;
      }
      found = keys[key];
      if (key.compare("regexp") == 0) {
        evaluatingRegexp = true;
        found._tokenstart = sub.position(size_t(0));
        found._tokensize = 0;
        continue;
      }
    }
    // Handle the other params of key
    if (found.keytype == KeyInfo::isIgnored)
      continue;
    else if (found.keytype == KeyInfo::isMath) {
      if (size_t(sub.position(size_t(0))) == math_pos) {
        found = keys[key];
        found._tokenstart = sub.position(size_t(0));
        found._tokensize = mi.getSize();
        found._dataEnd = found._tokenstart + found._tokensize;
        found._dataStart = found._dataEnd;
        found.parenthesiscount = 0;
        evaluatingMath = true;
      }
      else {
        // begin|end of unknown env, discard
        // First handle tables
        // longtable|tabular
        bool discardComment;
        if ((sub.str(5).compare("longtable") == 0) ||
            (sub.str(5).compare("tabular") == 0)) {
          discardComment = true;        /* '%' */
        }
        else
          discardComment = false;
        found = keys[key];
        // discard spaces before pos(0)
        int pos = sub.position(size_t(0));
        int count;
        for (count = 0; pos - count > 0; count++) {
          char c = interval.par[pos-count-1];
          if (discardComment) {
            if ((c != ' ') && (c != '%'))
              break;
          }
          else if (c != ' ')
            break;
        }
        found.keytype = KeyInfo::doRemove;
        found._tokenstart = pos - count;
        if (sub.str(1).compare(0, 5, "begin") == 0) {
          size_t pos1 = pos + sub.str(0).length();
          if (sub.str(5).compare("cjk") == 0) {
            pos1 = interval.findclosing(pos1+1, interval.par.length()) + 1;
            if ((interval.par[pos1] == '{') && (interval.par[pos1+1] == '}'))
              pos1 += 2;
            found.keytype = KeyInfo::isMain;
            found._dataStart = pos1;
            found._dataEnd = interval.par.length();
            found.disabled = keys["foreignlanguage"].disabled;
            found.used = keys["foreignlanguage"].used;
            found._tokensize = pos1 - found._tokenstart;
            found.head = interval.par.substr(found._tokenstart, found._tokensize);
          }
          else {
            if (interval.par[pos1] == '[') {
              pos1 = interval.findclosing(pos1+1, interval.par.length(), '[', ']')+1;
            }
            if (interval.par[pos1] == '{') {
              found._dataEnd = interval.findclosing(pos1+1, interval.par.length()) + 1;
            }
            else {
              found._dataEnd = pos1;
            }
            found._dataStart = found._dataEnd;
            found._tokensize = count + found._dataEnd - pos;
            found.parenthesiscount = 0;
            found.disabled = true;
          }
        }
        else {
          // Handle "\end{...}"
          found._dataStart = pos + sub.str(0).length();
          found._dataEnd = found._dataStart;
          found._tokensize = count + found._dataEnd - pos;
          found.parenthesiscount = 0;
          found.disabled = true;
        }
      }
    }
    else if (found.keytype != KeyInfo::isRegex) {
      found._tokenstart = sub.position(size_t(0));
      if (found.parenthesiscount == 0) {
        // Probably to be discarded
        size_t following_pos = sub.position(size_t(0)) + sub.str(3).length() + 1;
        char following = interval.par[following_pos];
        if (following == ' ')
          found.head = "\\" + sub.str(3) + " ";
        else if (following == '=') {
          // like \uldepth=1000pt
          found.head = sub.str(0);
        }
        else
          found.head = "\\" + key;
        found._tokensize = found.head.length();
        found._dataEnd = found._tokenstart + found._tokensize;
        found._dataStart = found._dataEnd;
      }
      else {
        if (found.parenthesiscount == 1) {
          found.head = "\\" + key + "{";
        }
        else if (found.parenthesiscount == 2) {
          found.head = sub.str(0) + "{";
        }
        found._tokensize = found.head.length();
        found._dataStart = found._tokenstart + found.head.length();
        size_t endpos = interval.findclosing(found._dataStart, interval.par.length());
        if (found.keytype == KeyInfo::noMain) {
          evaluatingCode = true;
          codeEnd = endpos;
          codeStart = found._dataStart;
        }
        else if (evaluatingCode) {
          if (size_t(found._dataStart) > codeEnd)
            evaluatingCode = false;
          else if (found.keytype == KeyInfo::isMain) {
            // Disable this key, treate it as standard
            found.keytype = KeyInfo::isStandard;
            found.disabled = true;
            if ((codeEnd == interval.par.length()) &&
                (found._tokenstart == codeStart)) {
              // trickery, because the code inset starts
              // with \selectlanguage ...
              codeEnd = endpos;
              if (entries.size() > 1) {
                entries[entries.size()-1]._dataEnd = codeEnd;
              }
            }
          }
        }
        if ((endpos == interval.par.length()) &&
            (found.keytype == KeyInfo::doRemove)) {
          // Missing closing => error in latex-input?
          // therefore do not delete remaining data
          found._dataStart -= 1;
          found._dataEnd = found._dataStart;
        }
        else
          found._dataEnd = endpos;
      }
      if (isPatternString) {
        keys[key].used = true;
      }
    }
    entries.push_back(found);
  }
}

void LatexInfo::makeKey(const string &keysstring, KeyInfo keyI, bool isPatternString)
{
  stringstream s(keysstring);
  string key;
  const char delim = '|';
  while (getline(s, key, delim)) {
    KeyInfo keyII(keyI);
    if (isPatternString) {
      keyII.used = false;
    }
    else if ( !keys[key].used)
      keyII.disabled = true;
    keys[key] = keyII;
  }
}

void LatexInfo::buildKeys(bool isPatternString)
{

  static bool keysBuilt = false;
  if (keysBuilt && !isPatternString) return;

  // Known standard keys with 1 parameter.
  // Split is done, if not at start of region
  makeKey("textsf|textss|texttt", KeyInfo(KeyInfo::isStandard, 1, ignoreFormats.getFamily()), isPatternString);
  makeKey("textbf",               KeyInfo(KeyInfo::isStandard, 1, ignoreFormats.getSeries()), isPatternString);
  makeKey("textit|textsc|textsl", KeyInfo(KeyInfo::isStandard, 1, ignoreFormats.getShape()), isPatternString);
  makeKey("uuline|uline|uwave",   KeyInfo(KeyInfo::isStandard, 1, ignoreFormats.getUnderline()), isPatternString);
  makeKey("emph|noun",            KeyInfo(KeyInfo::isStandard, 1, ignoreFormats.getMarkUp()), isPatternString);
  makeKey("sout|xout",            KeyInfo(KeyInfo::isStandard, 1, ignoreFormats.getStrikeOut()), isPatternString);

  makeKey("section|subsection|subsubsection|paragraph|subparagraph|minisec",
          KeyInfo(KeyInfo::isSectioning, 1, ignoreFormats.getSectioning()), isPatternString);
  makeKey("section*|subsection*|subsubsection*|paragraph*",
          KeyInfo(KeyInfo::isSectioning, 1, ignoreFormats.getSectioning()), isPatternString);
  makeKey("part|part*|chapter|chapter*", KeyInfo(KeyInfo::isSectioning, 1, ignoreFormats.getSectioning()), isPatternString);
  makeKey("title|subtitle|author|subject|publishers|dedication|uppertitleback|lowertitleback|extratitle|lyxaddress|lyxrightaddress", KeyInfo(KeyInfo::isSectioning, 1, ignoreFormats.getFrontMatter()), isPatternString);
  // Regex
  makeKey("regexp", KeyInfo(KeyInfo::isRegex, 1, false), isPatternString);

  // Split is done, if not at start of region
  makeKey("textcolor", KeyInfo(KeyInfo::isStandard, 2, ignoreFormats.getColor()), isPatternString);

  // Split is done always.
  makeKey("foreignlanguage", KeyInfo(KeyInfo::isMain, 2, ignoreFormats.getLanguage()), isPatternString);

  // Know charaters
  // No split
  makeKey("backslash|textbackslash|slash",  KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("textasciicircum|textasciitilde", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("textasciiacute",                 KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("dots|ldots",                     KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  // Spaces
  makeKey("quad|qquad|hfill|dotfill",               KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("textvisiblespace|nobreakspace",          KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("negthickspace|negmedspace|negthinspace", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  // Skip
  makeKey("enskip|smallskip|medskip|bigskip|vfill", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  // Custom space/skip, remove the content (== length value)
  makeKey("vspace|hspace|mspace", KeyInfo(KeyInfo::noContent, 1, false), isPatternString);
  // Found in fr/UserGuide.lyx
  makeKey("og|fg", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  // quotes
  makeKey("textquotedbl|quotesinglbase|lyxarrow", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("textquotedblleft|textquotedblright", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  // Known macros to remove (including their parameter)
  // No split
  makeKey("inputencoding|shortcut|label|ref|index", KeyInfo(KeyInfo::doRemove, 1, false), isPatternString);

  // handle like standard keys with 1 parameter.
  makeKey("url|href|vref|thanks", KeyInfo(KeyInfo::isStandard, 1, false), isPatternString);

  // Macros to remove, but let the parameter survive
  // No split
  makeKey("menuitem|textmd|textrm", KeyInfo(KeyInfo::isStandard, 1, true), isPatternString);

  // Remove language spec from content of these insets
  makeKey("code|footnote", KeyInfo(KeyInfo::noMain, 1, false), isPatternString);

  // Same effect as previous, parameter will survive (because there is no one anyway)
  // No split
  makeKey("noindent|textcompwordmark", KeyInfo(KeyInfo::isStandard, 0, true), isPatternString);
  // Remove table decorations
  makeKey("hline|tabularnewline|toprule|bottomrule|midrule", KeyInfo(KeyInfo::doRemove, 0, true), isPatternString);
  // Discard shape-header
  makeKey("circlepar|diamondpar|heartpar|nutpar",  KeyInfo(KeyInfo::isStandard, 1, true), isPatternString);
  makeKey("trianglerightpar|hexagonpar|starpar",   KeyInfo(KeyInfo::isStandard, 1, true), isPatternString);
  makeKey("triangleuppar|triangledownpar|droppar", KeyInfo(KeyInfo::isStandard, 1, true), isPatternString);
  makeKey("triangleleftpar|shapepar|dropuppar",    KeyInfo(KeyInfo::isStandard, 1, true), isPatternString);
  // like ('tiny{}' or '\tiny ' ... )
  makeKey("footnotesize|tiny|scriptsize|small|large|Large|LARGE|huge|Huge", KeyInfo(KeyInfo::isSize, 0, false), isPatternString);

  // Survives, like known character
  makeKey("lyx|latex|latexe|tex", KeyInfo(KeyInfo::isIgnored, 0, false), isPatternString);
  makeKey("item", KeyInfo(KeyInfo::isList, 1, false), isPatternString);

  makeKey("begin|end", KeyInfo(KeyInfo::isMath, 1, false), isPatternString);
  makeKey("[|]", KeyInfo(KeyInfo::isMath, 1, false), isPatternString);
  makeKey("$", KeyInfo(KeyInfo::isMath, 1, false), isPatternString);

  makeKey("par|uldepth|ULdepth|protect|nobreakdash", KeyInfo(KeyInfo::isStandard, 0, true), isPatternString);
  // Remove RTL/LTR marker
  makeKey("l|r|textlr|textfr|textar|beginl|endl", KeyInfo(KeyInfo::isStandard, 0, true), isPatternString);
  makeKey("lettrine", KeyInfo(KeyInfo::cleanToStart, 0, true), isPatternString);
  makeKey("endarguments", KeyInfo(KeyInfo::endArguments, 0, true), isPatternString);
  if (isPatternString) {
    // Allow the first searched string to rebuild the keys too
    keysBuilt = false;
  }
  else {
    // no need to rebuild again
    keysBuilt = true;
  }
}

/*
 * Keep the list of actual opened parentheses actual
 * (e.g. depth == 4 means there are 4 '{' not processed yet)
 */
void Intervall::handleParentheses(int lastpos, bool closingAllowed)
{
  int skip = 0;
  for (int i = depts[actualdeptindex]; i < lastpos; i+= 1 + skip) {
    char c;
    c = par[i];
    skip = 0;
    if (c == '\\') skip = 1;
    else if (c == '{') {
      handleOpenP(i);
    }
    else if (c == '}') {
      handleCloseP(i, closingAllowed);
    }
  }
}

#if (0)
string Intervall::show(int lastpos)
{
  int idx = 0;                          /* int intervalls */
  int count = 0;
  string s;
  int i = 0;
  for (idx = 0; idx <= ignoreidx; idx++) {
    while (i < lastpos) {
      int printsize;
      if (i <= borders[idx].low) {
        if (borders[idx].low > lastpos)
          printsize = lastpos - i;
        else
          printsize = borders[idx].low - i;
        s += par.substr(i, printsize);
        i += printsize;
        if (i >= borders[idx].low)
          i = borders[idx].upper;
      }
      else {
        i = borders[idx].upper;
        break;
      }
    }
  }
  if (lastpos > i) {
    s += par.substr(i, lastpos-i);
  }
  return (s);
}
#endif

void Intervall::output(ostringstream &os, int lastpos)
{
  // get number of chars to output
  int idx = 0;                          /* int intervalls */
  int i = 0;
  for (idx = 0; idx <= ignoreidx; idx++) {
    if (i < lastpos) {
      if (i <= borders[idx].low) {
        int printsize;
        if (borders[idx].low > lastpos)
          printsize = lastpos - i;
        else
          printsize = borders[idx].low - i;
        os << par.substr(i, printsize);
        i += printsize;
        handleParentheses(i, false);
        if (i >= borders[idx].low)
          i = borders[idx].upper;
      }
      else {
        i = borders[idx].upper;
      }
    }
    else
      break;
  }
  if (lastpos > i) {
    os << par.substr(i, lastpos-i);
  }
  handleParentheses(lastpos, false);
  for (int i = actualdeptindex; i > 0; --i) {
    os << "}";
  }
  if (! isPatternString)
    os << "\n";
  handleParentheses(lastpos, true); /* extra closings '}' allowed here */
}

void LatexInfo::processRegion(int start, int region_end)
{
  while (start < region_end) {          /* Let {[} and {]} survive */
    if ((interval.par[start] == '{') &&
        (interval.par[start+1] != ']') &&
        (interval.par[start+1] != '[')) {
      // Closing is allowed past the region
      int closing = interval.findclosing(start+1, interval.par.length());
      interval.addIntervall(start, start+1);
      interval.addIntervall(closing, closing+1);
    }
    start = interval.nextNotIgnored(start+1);
  }
}

void LatexInfo::removeHead(KeyInfo &actual, int count)
{
  if (actual.parenthesiscount == 0) {
    // "{\tiny{} ...}" ==> "{{} ...}"
    interval.addIntervall(actual._tokenstart-count, actual._tokenstart + actual._tokensize);
  }
  else {
    // Remove header hull, that is "\url{abcd}" ==> "abcd"
    interval.addIntervall(actual._tokenstart, actual._dataStart);
    interval.addIntervall(actual._dataEnd, actual._dataEnd+1);
  }
}

int LatexInfo::dispatch(ostringstream &os, int previousStart, KeyInfo &actual)
{
  int nextKeyIdx = 0;
  switch (actual.keytype)
  {
    case KeyInfo::cleanToStart: {
      actual._dataEnd = actual._dataStart;
      if (interval.par[actual._dataStart] == '[') {
        // Discard optional params
        actual._dataStart = interval.findclosing(actual._dataStart+1, interval.par.length(), '[', ']') + 1;
      }
      actual._dataEnd = actual._dataStart;
      nextKeyIdx = getNextKey();
      // Search for end of arguments
      int tmpIdx = nextKeyIdx;
      while (tmpIdx > 0) {
        KeyInfo &nextk = entries[tmpIdx];
        if (nextk.keytype == KeyInfo::endArguments) {
          actual._dataEnd = nextk._dataEnd;
          break;
        }
        nextk.disabled = true;
        tmpIdx++;
        if (tmpIdx >= int(entries.size()))
          break;
      }
      while (interval.par[actual._dataEnd] == ' ')
        actual._dataEnd++;
      interval.addIntervall(0, actual._dataEnd+1);
      interval.actualdeptindex = 0;
      interval.depts[0] = actual._dataEnd+1;
      interval.closes[0] = -1;
      break;
    }
    case KeyInfo::noContent: {          /* char like "\hspace{2cm}" */
      interval.addIntervall(actual._dataStart, actual._dataEnd);
    }
      // fall through
    case KeyInfo::isChar: {
      nextKeyIdx = getNextKey();
      break;
    }
    case KeyInfo::isSize: {
      if (actual.disabled || (interval.par[actual._dataStart] != '{') || (interval.par[actual._dataStart-1] == ' ')) {
        processRegion(actual._dataEnd, actual._dataEnd+1); /* remove possibly following {} */
        interval.addIntervall(actual._tokenstart, actual._dataEnd+1);
        nextKeyIdx = getNextKey();
      } else {
        // Here _dataStart points to '{', so correct it
        actual._dataStart += 1;
        actual._tokensize += 1;
        actual.parenthesiscount = 1;
        if (interval.par[actual._dataStart] == '}') {
          // Determine the end if used like '{\tiny{}...}'
          actual._dataEnd = interval.findclosing(actual._dataStart+1, interval.par.length()) + 1;
          interval.addIntervall(actual._dataStart, actual._dataStart+1);
        }
        else {
          // Determine the end if used like '\tiny{...}'
          actual._dataEnd = interval.findclosing(actual._dataStart, interval.par.length()) + 1;
        }
        // Split on this key if not at start
        int start = interval.nextNotIgnored(previousStart);
        if (start < actual._tokenstart) {
          interval.output(os, actual._tokenstart);
          interval.addIntervall(start, actual._tokenstart);
        }
        // discard entry if at end of actual
        nextKeyIdx = process(os, actual);
      }
      break;
    }
    case KeyInfo::endArguments:
      removeHead(actual);
      processRegion(actual._dataStart, actual._dataStart+1);
      nextKeyIdx = getNextKey();
      break;
    case KeyInfo::noMain:
      // fall through
    case KeyInfo::isStandard: {
      if (actual.disabled) {
        removeHead(actual);
        processRegion(actual._dataStart, actual._dataStart+1);
        nextKeyIdx = getNextKey();
      } else {
        // Split on this key if not at start
        int start = interval.nextNotIgnored(previousStart);
        if (start < actual._tokenstart) {
          interval.output(os, actual._tokenstart);
          interval.addIntervall(start, actual._tokenstart);
        }
        // discard entry if at end of actual
        nextKeyIdx = process(os, actual);
      }
      break;
    }
    case KeyInfo::doRemove: {
      // Remove the key with all parameters and following spaces
      size_t pos;
      for (pos = actual._dataEnd+1; pos < interval.par.length(); pos++) {
        if ((interval.par[pos] != ' ') && (interval.par[pos] != '%'))
          break;
      }
      interval.addIntervall(actual._tokenstart, pos);
      nextKeyIdx = getNextKey();
      break;
    }
    case KeyInfo::isList: {
      // Discard space before _tokenstart
      int count;
      for (count = 0; count < actual._tokenstart; count++) {
        if (interval.par[actual._tokenstart-count-1] != ' ')
          break;
      }
      if (actual.disabled) {
        interval.addIntervall(actual._tokenstart-count, actual._dataEnd+1);
      }
      else {
        interval.addIntervall(actual._tokenstart-count, actual._tokenstart);
      }
      // Discard extra parentheses '[]'
      if (interval.par[actual._dataEnd+1] == '[') {
        int posdown = interval.findclosing(actual._dataEnd+2, interval.par.length(), '[', ']');
        if ((interval.par[actual._dataEnd+2] == '{') &&
            (interval.par[posdown-1] == '}')) {
          interval.addIntervall(actual._dataEnd+1,actual._dataEnd+3);
          interval.addIntervall(posdown-1, posdown+1);
        }
        else {
          interval.addIntervall(actual._dataEnd+1, actual._dataEnd+2);
          interval.addIntervall(posdown, posdown+1);
        }
        int blk = interval.nextNotIgnored(actual._dataEnd+1);
        if (blk > posdown) {
          // Discard at most 1 space after empty item
          int count;
          for (count = 0; count < 1; count++) {
            if (interval.par[blk+count] != ' ')
              break;
          }
          if (count > 0)
            interval.addIntervall(blk, blk+count);
        }
      }
      nextKeyIdx = getNextKey();
      break;
    }
    case KeyInfo::isSectioning: {
      // Discard spaces before _tokenstart
      int count;
      for (count = 0; count < actual._tokenstart; count++) {
        if (interval.par[actual._tokenstart-count-1] != ' ')
          break;
      }
      if (actual.disabled) {
        removeHead(actual, count);
        nextKeyIdx = getNextKey();
      } else {
        interval.addIntervall(actual._tokenstart-count, actual._tokenstart);
        nextKeyIdx = process(os, actual);
      }
      break;
    }
    case KeyInfo::isMath: {
      // Same as regex, use the content unchanged
      nextKeyIdx = getNextKey();
      break;
    }
    case KeyInfo::isRegex: {
      // DO NOT SPLIT ON REGEX
      // Do not disable
      nextKeyIdx = getNextKey();
      break;
    }
    case KeyInfo::isIgnored: {
      // Treat like a character for now
      nextKeyIdx = getNextKey();
      break;
    }
    case KeyInfo::isMain: {
      if (interval.par.substr(actual._dataStart, 2) == "% ")
        interval.addIntervall(actual._dataStart, actual._dataStart+2);
      if (actual.disabled) {
        removeHead(actual);
        if ((interval.par.substr(actual._dataStart, 3) == " \\[") ||
            (interval.par.substr(actual._dataStart, 8) == " \\begin{")) {
          // Discard also the space before math-equation
          interval.addIntervall(actual._dataStart, actual._dataStart+1);
        }
        interval.resetOpenedP(actual._dataStart-1);
      }
      else {
        if (actual._tokenstart == 0) {
          // for the first (and maybe dummy) language
          interval.setForDefaultLang(actual._tokenstart + actual._tokensize);
        }
        interval.resetOpenedP(actual._dataStart-1);
      }
      break;
    }
    case KeyInfo::invalid:
      // This cannot happen, already handled
      // fall through
    default: {
      // LYXERR0("Unhandled keytype");
      nextKeyIdx = getNextKey();
      break;
    }
  }
  return nextKeyIdx;
}

int LatexInfo::process(ostringstream &os, KeyInfo &actual )
{
  int end = interval.nextNotIgnored(actual._dataEnd);
  int oldStart = actual._dataStart;
  int nextKeyIdx = getNextKey();
  while (true) {
    if ((nextKeyIdx < 0) ||
        (entries[nextKeyIdx]._tokenstart >= actual._dataEnd) ||
        (entries[nextKeyIdx].keytype == KeyInfo::invalid)) {
      if (oldStart <= end) {
        processRegion(oldStart, end);
        oldStart = end+1;
      }
      break;
    }
    KeyInfo &nextKey = getKeyInfo(nextKeyIdx);

    if (nextKey.keytype == KeyInfo::isMain) {
      (void) dispatch(os, actual._dataStart, nextKey);
      end = nextKey._tokenstart;
      break;
    }
    processRegion(oldStart, nextKey._tokenstart);
    nextKeyIdx = dispatch(os, actual._dataStart, nextKey);

    oldStart = nextKey._dataEnd+1;
  }
  // now nextKey is either invalid or is outside of actual._dataEnd
  // output the remaining and discard myself
  if (oldStart <= end) {
    processRegion(oldStart, end);
  }
  if (interval.par[end] == '}') {
    end += 1;
    // This is the normal case.
    // But if using the firstlanguage, the closing may be missing
  }
  // get minimum of 'end' and  'actual._dataEnd' in case that the nextKey.keytype was 'KeyInfo::isMain'
  int output_end;
  if (actual._dataEnd < end)
    output_end = interval.nextNotIgnored(actual._dataEnd);
  else
    output_end = interval.nextNotIgnored(end);
  if ((actual.keytype == KeyInfo::isMain) && actual.disabled) {
    interval.addIntervall(actual._tokenstart, actual._tokenstart+actual._tokensize);
  }
  // Remove possible empty data
  int dstart = interval.nextNotIgnored(actual._dataStart);
  while ((dstart < output_end) && (interval.par[dstart] == '{')) {
    interval.addIntervall(dstart, dstart+1);
    int dend = interval.findclosing(dstart+1, output_end);
    interval.addIntervall(dend, dend+1);
    dstart = interval.nextNotIgnored(dstart+1);
  }
  if (dstart < output_end)
    interval.output(os, output_end);
  interval.addIntervall(actual._tokenstart, end);
  return nextKeyIdx;
}

string splitOnKnownMacros(string par, bool isPatternString) {
  ostringstream os;
  LatexInfo li(par, isPatternString);
  KeyInfo DummyKey = KeyInfo(KeyInfo::KeyType::isMain, 2, true);
  DummyKey.head = "";
  DummyKey._tokensize = 0;
  DummyKey._tokenstart = 0;
  DummyKey._dataStart = 0;
  DummyKey._dataEnd = par.length();
  DummyKey.disabled = true;
  int firstkeyIdx = li.getFirstKey();
  string s;
  if (firstkeyIdx >= 0) {
    KeyInfo firstKey = li.getKeyInfo(firstkeyIdx);
    int nextkeyIdx;
    if ((firstKey.keytype != KeyInfo::isMain) || firstKey.disabled) {
      // Use dummy firstKey
      firstKey = DummyKey;
      (void) li.setNextKey(firstkeyIdx);
    }
    else {
      if (par.substr(firstKey._dataStart, 2) == "% ")
        li.addIntervall(firstKey._dataStart, firstKey._dataStart+2);
    }
    nextkeyIdx = li.process(os, firstKey);
    while (nextkeyIdx >= 0) {
      // Check for a possible gap between the last
      // entry and this one
      int datastart = li.nextNotIgnored(firstKey._dataStart);
      KeyInfo &nextKey = li.getKeyInfo(nextkeyIdx);
      if ((nextKey._tokenstart > datastart)) {
        // Handle the gap
        firstKey._dataStart = datastart;
        firstKey._dataEnd = par.length();
        (void) li.setNextKey(nextkeyIdx);
        if (firstKey._tokensize > 0) {
          // Fake the last opened parenthesis
          li.setForDefaultLang(firstKey._tokensize);
        }
        nextkeyIdx = li.process(os, firstKey);
      }
      else {
        if (nextKey.keytype != KeyInfo::isMain) {
          firstKey._dataStart = datastart;
          firstKey._dataEnd = nextKey._dataEnd+1;
          (void) li.setNextKey(nextkeyIdx);
          if (firstKey._tokensize > 0)
            li.setForDefaultLang(firstKey._tokensize);
          nextkeyIdx = li.process(os, firstKey);
        }
        else {
          nextkeyIdx = li.process(os, nextKey);
        }
      }
    }
    // Handle the remaining
    firstKey._dataStart = li.nextNotIgnored(firstKey._dataStart);
    firstKey._dataEnd = par.length();
    // Check if ! empty
    if ((firstKey._dataStart < firstKey._dataEnd) &&
        (par[firstKey._dataStart] != '}')) {
      if (firstKey._tokensize > 0)
        li.setForDefaultLang(firstKey._tokensize);
      (void) li.process(os, firstKey);
    }
    s = os.str();
    if (s.empty()) {
      // return string definitelly impossible to match
      s = "\\foreignlanguage{ignore}{ }";
    }
  }
  else
    s = par;                            /* no known macros found */
  return s;
}

/*
 * Try to unify the language specs in the latexified text.
 * Resulting modified string is set to "", if
 * the searched tex does not contain all the features in the search pattern
 */
static string correctlanguagesetting(string par, bool isPatternString, bool withformat)
{
	static Features regex_f;
	static int missed = 0;
	static bool regex_with_format = false;

	int parlen = par.length();

	while ((parlen > 0) && (par[parlen-1] == '\n')) {
		parlen--;
	}
	string result;
	if (withformat) {
		// Split the latex input into pieces which
		// can be digested by our search engine
		LYXERR(Debug::FIND, "input: \"" << par << "\"");
		result = splitOnKnownMacros(par, isPatternString);
		LYXERR(Debug::FIND, "After split: \"" << result << "\"");
	}
	else
		result = par.substr(0, parlen);
	if (isPatternString) {
		missed = 0;
		if (withformat) {
			regex_f = identifyFeatures(result);
                        string features = "";
			for (auto it = regex_f.cbegin(); it != regex_f.cend(); ++it) {
				string a = it->first;
				regex_with_format = true;
                                features += " " + a;
				// LYXERR0("Identified regex format:" << a);
			}
                        LYXERR(Debug::FIND, "Identified Features" << features);

		}
	} else if (regex_with_format) {
		Features info = identifyFeatures(result);
		for (auto it = regex_f.cbegin(); it != regex_f.cend(); ++it) {
			string a = it->first;
			bool b = it->second;
			if (b && ! info[a]) {
				missed++;
				LYXERR(Debug::FIND, "Missed(" << missed << " " << a <<", srclen = " << parlen );
				return("");
			}
		}
	}
	else {
		// LYXERR0("No regex formats");
	}
	return(result);
}


// Remove trailing closure of math, macros and environments, so to catch parts of them.
static int identifyClosing(string & t)
{
	int open_braces = 0;
	do {
		LYXERR(Debug::FIND, "identifyClosing(): t now is '" << t << "'");
		if (regex_replace(t, t, "(.*[^\\\\])\\$" REGEX_EOS, "$1"))
			continue;
		if (regex_replace(t, t, "(.*[^\\\\]) \\\\\\]" REGEX_EOS, "$1"))
			continue;
		if (regex_replace(t, t, "(.*[^\\\\]) \\\\end\\{[a-zA-Z_]*\\*?\\}" REGEX_EOS, "$1"))
			continue;
		if (regex_replace(t, t, "(.*[^\\\\])\\}" REGEX_EOS, "$1")) {
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
	// correct the language settings
	par_as_string = correctlanguagesetting(par_as_string, true, !opt.ignoreformat);
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
		LYXERR(Debug::FIND, "Lead_size: " << lead_size);
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
		if (!opt.ignoreformat) {
			// Remove extra '\}' at end
			while ( regex_replace(par_as_string, par_as_string, "(.*)\\\\}$", "$1")) {
				open_braces++;
			}
			/*
			// save '\.'
			regex_replace(par_as_string, par_as_string, "\\\\\\.", "_xxbdotxx_");
			// handle '.' -> '[^]', replace later as '[^\}\{\\]'
			regex_replace(par_as_string, par_as_string, "\\.", "[^]");
			// replace '[^...]' with '[^...\}\{\\]'
			regex_replace(par_as_string, par_as_string, "\\[\\^([^\\\\\\]]*)\\]", "_xxbrlxx_$1\\}\\{\\\\_xxbrrxx_");
			regex_replace(par_as_string, par_as_string, "_xxbrlxx_", "[^");
			regex_replace(par_as_string, par_as_string, "_xxbrrxx_", "]");
			// restore '\.'
			regex_replace(par_as_string, par_as_string, "_xxbdotxx_", "\\.");
			*/
		}
		LYXERR(Debug::FIND, "par_as_string now is '" << par_as_string << "'");
		LYXERR(Debug::FIND, "Open braces: " << open_braces);
		LYXERR(Debug::FIND, "Close .*?  : " << close_wildcards);
		LYXERR(Debug::FIND, "Replaced text (to be used as regex): " << par_as_string);

		// If entered regexp must match at begin of searched string buffer
		// Kornel: Added parentheses to use $1 for size of the leading string
		string regexp_str;
		string regexp2_str;
		{
			// TODO: Adapt '\[12345678]' in par_as_string to acount for the first '()
			// Unfortunately is '\1', '\2', etc not working for strings with extra format
			// so the convert has no effect in that case
			for (int i = 8; i > 0; --i) {
				string orig = "\\\\" + std::to_string(i);
				string dest = "\\" + std::to_string(i+1);
				while (regex_replace(par_as_string, par_as_string, orig, dest));
			}
			regexp_str = "(" + lead_as_regexp + ")" + par_as_string;
			regexp2_str = "(" + lead_as_regexp + ").*" + par_as_string;
		}
		LYXERR(Debug::FIND, "Setting regexp to : '" << regexp_str << "'");
		regexp = lyx::regex(regexp_str);

		LYXERR(Debug::FIND, "Setting regexp2 to: '" << regexp2_str << "'");
		regexp2 = lyx::regex(regexp2_str);
	}
}


// Count number of characters in string
// {]} ==> 1
// \&  ==> 1
// --- ==> 1
// \\[a-zA-Z]+ ==> 1
static int computeSize(string s, int len)
{
	if (len == 0)
		return 0;
	int skip = 1;
	int count = 0;
	for (int i = 0; i < len; i += skip, count++) {
		if (s[i] == '\\') {
			skip = 2;
			if (isalpha(s[i+1])) {
				for (int j = 2;  i+j < len; j++) {
					if (! isalpha(s[i+j])) {
						if (s[i+j] == ' ')
							skip++;
						else if ((s[i+j] == '{') && s[i+j+1] == '}')
							skip += 2;
						else if ((s[i+j] == '{') && (i + j + 1 >= len))
							skip++;
						break;
					}
					skip++;
				}
			}
		}
		else if (s[i] == '{') {
			if (s[i+1] == '}')
				skip = 2;
			else
				skip = 3;
		}
		else if (s[i] == '-') {
			if (s[i+1] == '-') {
				if (s[i+2] == '-')
					skip = 3;
				else
					skip = 2;
			}
			else
				skip = 1;
		}
		else {
			skip = 1;
		}
	}
	return count;
}

int MatchStringAdv::findAux(DocIterator const & cur, int len, bool at_begin) const
{
	if (at_begin &&
		(opt.restr == FindAndReplaceOptions::R_ONLY_MATHS && !cur.inMathed()) )
		return 0;

	docstring docstr = stringifyFromForSearch(opt, cur, len);
	string str = normalize(docstr, true);
	if (!opt.ignoreformat) {
		str = correctlanguagesetting(str, false, !opt.ignoreformat);
	}
	if (str.empty()) return(-1);
	LYXERR(Debug::FIND, "Matching against     '" << lyx::to_utf8(docstr) << "'");
	LYXERR(Debug::FIND, "After normalization: '" << str << "'");

	if (use_regexp) {
		LYXERR(Debug::FIND, "Searching in regexp mode: at_begin=" << at_begin);
		regex const *p_regexp;
		regex_constants::match_flag_type flags;
		if (at_begin) {
			flags = regex_constants::match_continuous;
			p_regexp = &regexp;
		} else {
			flags = regex_constants::match_default;
			p_regexp = &regexp2;
		}
		sregex_iterator re_it(str.begin(), str.end(), *p_regexp, flags);
		if (re_it == sregex_iterator())
			return 0;
		match_results<string::const_iterator> const & m = *re_it;

		if (0) { // Kornel Benko: DO NOT CHECKK
			// Check braces on the segment that matched the entire regexp expression,
			// plus the last subexpression, if a (.*?) was inserted in the constructor.
			if (!braces_match(m[0].first, m[0].second, open_braces))
				return 0;
		}

		// Check braces on segments that matched all (.*?) subexpressions,
		// except the last "padding" one inserted by lyx.
		for (size_t i = 1; i < m.size() - 1; ++i)
			if (!braces_match(m[i].first, m[i].second, open_braces))
				return 0;

		// Exclude from the returned match length any length
		// due to close wildcards added at end of regexp
		// and also the length of the leading (e.g. '\emph{')
		//
		// Whole found string, including the leading: m[0].second - m[0].first
		// Size of the leading string: m[1].second - m[1].first
		int leadingsize = 0;
		if (m.size() > 1)
			leadingsize = m[1].second - m[1].first;
		int result;
                for (size_t i = 0; i < m.size(); i++) {
                  LYXERR(Debug::FIND, "Match " << i << " is " << m[i].second - m[i].first << " long");
                }
		if (close_wildcards == 0)
			result = m[0].second - m[0].first;

		else
			result =  m[m.size() - close_wildcards].first - m[0].first;

		size_t pos = m.position(size_t(0));
		// Ignore last closing characters
		while (result > 0) {
			if (str[pos+result-1] == '}')
				--result;
			else
				break;
		}
		if (result > leadingsize)
			result -= leadingsize;
		else
			result = 0;
		return computeSize(str.substr(pos+leadingsize,result), result);
	}

	// else !use_regexp: but all code paths above return
	LYXERR(Debug::FIND, "Searching in normal mode: par_as_string='"
				 << par_as_string << "', str='" << str << "'");
	LYXERR(Debug::FIND, "Searching in normal mode: lead_as_string='"
				 << lead_as_string << "', par_as_string_nolead='"
				 << par_as_string_nolead << "'");

	if (at_begin) {
		LYXERR(Debug::FIND, "size=" << par_as_string.size()
					 << ", substr='" << str.substr(0, par_as_string.size()) << "'");
		if (str.substr(0, par_as_string.size()) == par_as_string)
			return par_as_string.size();
	} else {
		size_t pos = str.find(par_as_string_nolead);
		if (pos != string::npos)
			return par_as_string.size();
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
        if ((len > 0) && (res < len))
          return 0;
	Paragraph const & par = cur.paragraph();
	bool ws_left = (cur.pos() > 0)
		? par.isWordSeparator(cur.pos() - 1)
		: true;
	bool ws_right = (cur.pos() + len < par.size())
		? par.isWordSeparator(cur.pos() + len)
		: true;
	LYXERR(Debug::FIND,
	       "cur.pos()=" << cur.pos() << ", res=" << res
	       << ", separ: " << ws_left << ", " << ws_right
               << ", len: " << len
	       << endl);
	if (ws_left && ws_right) {
          // Check for word separators inside the found 'word'
          for (int i = 0; i < len; i++) {
            if (par.isWordSeparator(cur.pos() + i))
              return 0;
          }
          return res;
        }
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
	// Kornel: Added textsl, textsf, textit, texttt and noun
	// + allow to seach for colored text too
	LYXERR(Debug::FIND, "Removing stale empty \\emph{}, \\textbf{}, \\*section{} macros from: " << t);
	while (regex_replace(t, t, "\\\\(emph|noun|text(bf|sl|sf|it|tt)|(u|uu)line|(s|x)out|uwave)(\\{(\\{\\})?\\})+", ""))
		LYXERR(Debug::FIND, "  further removing stale empty \\emph{}, \\textbf{} macros from: " << t);
	while (regex_replace(t, t, "\\\\((sub)?(((sub)?section)|paragraph)|part)\\*?(\\{(\\{\\})?\\})+", ""))
		LYXERR(Debug::FIND, "  further removing stale empty \\emph{}, \\textbf{} macros from: " << t);

	while (regex_replace(t, t, "\\\\(foreignlanguage|textcolor|item)\\{[a-z]+\\}(\\{(\\{\\})?\\})+", ""));
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
		CursorSlice cs = cur.top();
		MathData md = cs.cell();
		MathData::const_iterator it_end =
			(( len == -1 || cs.pos() + len > int(md.size()))
			 ? md.end()
			 : md.begin() + cs.pos() + len );
		MathData md2;
		for (MathData::const_iterator it = md.begin() + cs.pos();
		     it != it_end; ++it)
			md2.push_back(*it);
		docstring s = asString(md2);
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

	odocstringstream ods;
	otexstream os(ods);
	OutputParams runparams(&buf.params().encoding());
	runparams.nice = false;
	runparams.flavor = OutputParams::LATEX;
	runparams.linelen = 8000; //lyxrc.plaintext_linelen;
	// No side effect of file copying and image conversion
	runparams.dryrun = true;
	runparams.for_search = true;

	if (cur.inTexted()) {
		// @TODO what about searching beyond/across paragraph breaks ?
		pos_type endpos = cur.paragraph().size();
		if (len != -1 && endpos > cur.pos() + len)
			endpos = cur.pos() + len;
		TeXOnePar(buf, *cur.innerText(), cur.pit(), os, runparams,
			  string(), cur.pos(), endpos);
		string s = lyx::to_utf8(ods.str());
		LYXERR(Debug::FIND, "Latexified +modified text: '" << s << "'");
		return(lyx::from_utf8(s));
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
		MathData md2;
		for (MathData::const_iterator it = md.begin() + cs.pos();
		     it != it_end; ++it)
			md2.push_back(*it);

		ods << asString(md2);
		// Retrieve the math environment type, and add '$' or '$]'
		// or others (\end{equation}) accordingly
		for (int s = cur.depth() - 1; s >= 0; --s) {
			CursorSlice const & cs2 = cur[s];
			InsetMath * inset = cs2.asInsetMath();
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
	int max_match = match(cur);     /* match valid only if not searching whole words */
	if (max_match <= 0) return 0;
	LYXERR(Debug::FIND, "Ok");

	// Compute the match length
        int len = 1;
	if (cur.pos() + len > cur.lastpos())
	  return 0;
	if (match.opt.matchword) {
          LYXERR(Debug::FIND, "verifying unmatch with len = " << len);
          while (cur.pos() + len <= cur.lastpos() && match(cur, len) <= 0) {
            ++len;
            LYXERR(Debug::FIND, "verifying unmatch with len = " << len);
          }
          // Length of matched text (different from len param)
          int old_match = match(cur, len);
          if (old_match < 0)
            old_match = 0;
          int new_match;
          // Greedy behaviour while matching regexps
          while ((new_match = match(cur, len + 1)) > old_match) {
            ++len;
            old_match = new_match;
            LYXERR(Debug::FIND, "verifying   match with len = " << len);
          }
          if (old_match == 0)
            len = 0;
        }
	else {
	  int minl = 1;
	  int maxl = cur.lastpos() - cur.pos();
	  // Greedy behaviour while matching regexps
	  while (maxl > minl) {
	    int actual_match = match(cur, len);
	    if (actual_match >= max_match) {
	      // actual_match > max_match _can_ happen,
	      // if the search area splits
	      // some following word so that the regex
	      // (e.g. 'r.*r\b' matches 'r' from the middle of the
	      // splitted word)
	      // This means, the len value is too big
	      maxl = len;
	      len = (int)((maxl + minl)/2);
	    }
	    else {
	      // (actual_match < max_match)
	      minl = len + 1;
	      len = (int)((maxl + minl)/2);
	    }
	  }
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
		if (match_len > 0) {
			int match_len_zero_count = 0;
			for (; !theApp()->longOperationCancelled() && cur; cur.forwardPos()) {
				LYXERR(Debug::FIND, "Advancing cur: " << cur);
				int match_len3 = match(cur, 1);
				if (match_len3 < 0)
					continue;
				int match_len2 = match(cur);
				LYXERR(Debug::FIND, "match_len2: " << match_len2);
				if (match_len2 > 0) {
					// Sometimes in finalize we understand it wasn't a match
					// and we need to continue the outest loop
					int len = findAdvFinalize(cur, match);
					if (len > 0) {
						return len;
					}
				}
				if (match_len2 >= 0) {
					if (match_len2 == 0)
						match_len_zero_count++;
					else
						match_len_zero_count = 0;
				}
				else {
                                        if (++match_len_zero_count > 3) {
                                                LYXERR(Debug::FIND, "match_len2_zero_count: " << match_len_zero_count << ", match_len was " << match_len);
                                                match_len_zero_count = 0;
                                        }
					break;
				}
			}
			if (!cur)
				return 0;
		}
		if (match_len >= 0 && cur.pit() < cur.lastpit()) {
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


} // namespace


docstring stringifyFromForSearch(FindAndReplaceOptions const & opt,
				 DocIterator const & cur, int len)
{
	if (cur.pos() < 0 || cur.pos() > cur.lastpos())
	        return docstring();
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

} // namespace

///
static void findAdvReplace(BufferView * bv, FindAndReplaceOptions const & opt, MatchStringAdv & matchAdv)
{
	Cursor & cur = bv->cursor();
	if (opt.repl_buf_name == docstring()
	    || theBufferList().getBuffer(FileName(to_utf8(opt.repl_buf_name)), true) == 0
	    || theBufferList().getBuffer(FileName(to_utf8(opt.find_buf_name)), true) == 0)
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
	cap::cutSelection(cur, false);
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
		otexstream os(ods);
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
		(void)regex_replace(to_utf8(repl_latex), s, "\\$(.*)\\$", "$1");
		(void)regex_replace(s, s, "\\\\\\[(.*)\\\\\\]", "$1");
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

	// e.g., when invoking word-findadv from mini-buffer wither with
	//       wrong options syntax or before ever opening advanced F&R pane
	if (theBufferList().getBuffer(FileName(to_utf8(opt.find_buf_name)), true) == 0)
		return false;

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

} // namespace lyx
