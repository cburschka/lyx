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
 * \author Kornel Benko
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxfind.h"

#include "Buffer.h"
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
#include "Text.h"
#include "Encoding.h"

#include "frontends/Application.h"
#include "frontends/alert.h"

#include "mathed/InsetMath.h"
#include "mathed/InsetMathHull.h"
#include "mathed/MathData.h"
#include "mathed/MathStream.h"
#include "mathed/MathSupport.h"

#include "support/debug.h"
#include "support/docstream.h"
#include "support/FileName.h"
#include "support/gettext.h"
#include "support/lassert.h"
#include "support/lstrings.h"
#include "support/textutils.h"

#include <map>
#include <regex>

//#define ResultsDebug
#define USE_QT_FOR_SEARCH
#if defined(USE_QT_FOR_SEARCH)
	#include <QtCore>	// sets QT_VERSION
	#if (QT_VERSION >= 0x050000)
		#include <QRegularExpression>
		#define QTSEARCH 1
	#else
		#define QTSEARCH 0
	#endif
#else
	#define QTSEARCH 0
#endif

using namespace std;
using namespace lyx::support;

namespace lyx {

typedef map<string, string> AccentsMap;
static AccentsMap accents = map<string, string>();

// Helper class for deciding what should be ignored
class IgnoreFormats {
 public:
	///
	IgnoreFormats() = default;
	///
	bool getFamily() const { return ignoreFamily_; }
	///
	bool getSeries() const { return ignoreSeries_; }
	///
	bool getShape() const { return ignoreShape_; }
	///
	bool getUnderline() const { return ignoreUnderline_; }
	///
	bool getMarkUp() const { return ignoreMarkUp_; }
	///
	bool getStrikeOut() const { return ignoreStrikeOut_; }
	///
	bool getSectioning() const { return ignoreSectioning_; }
	///
	bool getFrontMatter() const { return ignoreFrontMatter_; }
	///
	bool getColor() const { return ignoreColor_; }
	///
	bool getLanguage() const { return ignoreLanguage_; }
	///
	void setIgnoreFormat(string const & type, bool value);

private:
	///
	bool ignoreFamily_ = false;
	///
	bool ignoreSeries_ = false;
	///
	bool ignoreShape_ = false;
	///
	bool ignoreUnderline_ = false;
	///
	bool ignoreMarkUp_ = false;
	///
	bool ignoreStrikeOut_ = false;
	///
	bool ignoreSectioning_ = false;
	///
	bool ignoreFrontMatter_ = false;
	///
	bool ignoreColor_ = false;
	///
	bool ignoreLanguage_ = false;
};


void IgnoreFormats::setIgnoreFormat(string const & type, bool value)
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


void setIgnoreFormat(string const & type, bool value)
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


class MatchString
{
public:
	MatchString(docstring const & s, bool cs, bool mw)
		: str(s), case_sens(cs), whole_words(mw)
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
	if (!cur.selection()) {
		// no selection, non-empty search string: find it
		if (!searchstr.empty()) {
			bool const found = findOne(bv, searchstr, case_sens, whole, forward, true, findnext);
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
		searchstr = cur.selectionAsString(false, true);
	}

	// if we still don't have a search string, report the error
	// and abort.
	if (!searchAllowed(searchstr))
		return make_pair(false, 0);

	bool have_selection = cur.selection();
	docstring const selected = cur.selectionAsString(false, true);
	bool match =
		case_sens
		? searchstr == selected
		: compare_no_case(searchstr, selected) == 0;

	// no selection or current selection is not search word:
	// just find the search word
	if (!have_selection || !match) {
		bool const found = findOne(bv, searchstr, case_sens, whole, forward, true, findnext);
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

	return findOne(bv, search, casesensitive, matchword, forward, false, true);
}


bool lyxreplace(BufferView * bv, FuncRequest const & ev)
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
/*
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
*/

/// A map of lyx escaped strings and their unescaped equivalent.
/*
Escapes const & get_lyx_unescapes()
{
	typedef std::pair<std::string, std::string> P;

	static Escapes escape_map;
	if (escape_map.empty()) {
		escape_map.push_back(P("\\%", "%"));
		escape_map.push_back(P("\\{", "{"));
		escape_map.push_back(P("\\}", "}"));
		escape_map.push_back(P("\\mathcircumflex ", "^"));
		escape_map.push_back(P("\\mathcircumflex", "^"));
		escape_map.push_back(P("\\backslash ", "\\"));
		escape_map.push_back(P("\\backslash", "\\"));
		escape_map.push_back(P("\\sim ", "~"));
		escape_map.push_back(P("\\sim", "~"));
	}
	return escape_map;
}
*/

/// A map of escapes turning a regexp matching text to one matching latex.
/*
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
*/

/** @todo Probably the maps need to be migrated to regexps, in order to distinguish if
 ** the found occurrence were escaped.
 **/
/*
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
*/


string string2regex(string in)
{
	static std::regex specialChars { R"([-[\]{}()*+?.,\^$|#\s\$\\])" };
	string temp = std::regex_replace(in, specialChars,  R"(\$&)" );
	string temp2("");
	size_t lastpos = 0;
	size_t fl_pos = 0;
	int offset = 1;
	while (fl_pos < temp.size()) {
		fl_pos = temp.find("\\\\foreignlanguage", lastpos + offset);
		if (fl_pos == string::npos)
			break;
		offset = 16;
		temp2 += temp.substr(lastpos, fl_pos - lastpos);
		temp2 += "\\n";
		lastpos = fl_pos;
	}
	if (lastpos == 0)
		return(temp);
	if (lastpos < temp.size()) {
		temp2 += temp.substr(lastpos, temp.size() - lastpos);
	}
	return temp2;
}

string correctRegex(string t, bool withformat)
{
	/* Convert \backslash => \
	 * and \{, \}, \[, \] => {, }, [, ]
	 */
	string s("");
	regex wordre("(\\\\)*(\\\\((backslash|mathcircumflex) ?|[\\[\\]\\{\\}]))");
	size_t lastpos = 0;
	smatch sub;
	for (sregex_iterator it(t.begin(), t.end(), wordre), end; it != end; ++it) {
		sub = *it;
		string replace;
		if ((sub.position(2) - sub.position(0)) % 2 == 1) {
			continue;
		}
		else {
			if (sub.str(4) == "backslash") {
				replace = "\\";
				if (withformat) {
					sregex_iterator it2 = it;
					++it2;
					smatch sub2 = *it2;
					if (sub2.str(3) == "{")
						replace = "";
					else if (sub2.str(3) == "}")
						replace = "";
				}
			}
			else if (sub.str(4) == "mathcircumflex")
				replace = "^";
			else if (withformat && (sub.str(3) == "{"))
				replace = accents["braceleft"];
			else if (withformat && (sub.str(3) == "}"))
				replace = accents["braceright"];
			else
				replace = sub.str(3);
		}
		if (lastpos < (size_t) sub.position(2))
			s += t.substr(lastpos, sub.position(2) - lastpos);
		s += replace;
		lastpos = sub.position(2) + sub.length(2);
	}
	if (lastpos == 0)
		return t;
	else if (lastpos < t.length())
		s += t.substr(lastpos, t.length() - lastpos);
	return s;
}

/// Within \regexp{} apply get_lyx_unescapes() only (i.e., preserve regexp semantics of the string),
/// while outside apply get_lyx_unescapes()+get_regexp_escapes().
/// If match_latex is true, then apply regexp_latex_escapes() to \regexp{} contents as well.
string escape_for_regex(string s, bool withformat)
{
	size_t lastpos = 0;
	string result = "";
	while (lastpos < s.size()) {
		size_t regex_pos = s.find("\\regexp{", lastpos);
		if (regex_pos == string::npos) {
			regex_pos = s.size();
		}
		if (regex_pos > lastpos) {
			result += string2regex(s.substr(lastpos, regex_pos-lastpos));
			lastpos = regex_pos;
			if (lastpos == s.size())
				break;
		}
		size_t end_pos = s.find("\\endregexp{}}", regex_pos + 8);
		result += correctRegex(s.substr(regex_pos + 8, end_pos -(regex_pos + 8)), withformat);
		lastpos = end_pos + 13;
	}
	return result;
}


/// Wrapper for lyx::regex_replace with simpler interface
bool regex_replace(string const & s, string & t, string const & searchstr,
		   string const & replacestr)
{
	regex e(searchstr, regex_constants::ECMAScript);
	ostringstream oss;
	ostream_iterator<char, char> it(oss);
	regex_replace(it, s.begin(), s.end(), e, replacestr);
	// tolerate t and s be references to the same variable
	bool rv = (s != oss.str());
	t = oss.str();
	return rv;
}

#if 0
/** Checks if supplied string segment is well-formed from the standpoint of matching open-closed braces.
 **
 ** Verify that closed braces exactly match open braces. This avoids that, for example,
 ** \frac{.*}{x} matches \frac{x+\frac{y}{x}}{z} with .* being 'x+\frac{y'.
 **
 ** @param unmatched
 ** Number of open braces that must remain open at the end for the verification to succeed.
 **/
#if QTSEARCH
bool braces_match(QString const & beg,
		  int unmatched = 0)
#else
bool braces_match(string const & beg,
		int unmatched = 0)
#endif
{
	int open_pars = 0;
#if QTSEARCH
	LYXERR(Debug::FIND, "Checking " << unmatched << " unmatched braces in '" << beg.toStdString() << "'");
#else
	LYXERR(Debug::FIND, "Checking " << unmatched << " unmatched braces in '" << beg << "'");
#endif
	int lastidx = beg.size();
	for (int i=0; i < lastidx; ++i) {
		// Skip escaped braces in the count
#if QTSEARCH
		QChar c = beg.at(i);
#else
		char c = beg.at(i);
#endif
		if (c == '\\') {
			++i;
			if (i >= lastidx)
				break;
		} else if (c == '{') {
			++open_pars;
		} else if (c == '}') {
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
#endif

class MatchResult {
public:
	enum range {
		newIsTooFar,
		newIsBetter,
                newIsInvalid
        };
	int match_len;
	int match_prefix;
	int match2end;
	int pos;
	int leadsize;
	int pos_len;
	int searched_size;
	vector <string> result = vector <string>();
	MatchResult(int len = 0): match_len(len),match_prefix(0),match2end(0), pos(0),leadsize(0),pos_len(-1),searched_size(0) {};
};

static MatchResult::range interpretMatch(MatchResult &oldres, MatchResult &newres)
{
  int range = oldres.match_len;
  if (range > 0) range--;
  if (newres.match2end < oldres.match2end - oldres.match_len)
    return MatchResult::newIsTooFar;
  if (newres.match_len < oldres.match_len)
    return MatchResult::newIsTooFar;
  if ((newres.match_len == oldres.match_len) &&
      (newres.match2end < oldres.match2end + range) &&
      (newres.match2end > oldres.match2end - range)) {
    return MatchResult::newIsBetter;
  }
  return MatchResult::newIsInvalid;
}

/** The class performing a match between a position in the document and the FindAdvOptions.
 **/

class MatchStringAdv {
public:
	MatchStringAdv(lyx::Buffer & buf, FindAndReplaceOptions & opt);

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
	MatchResult operator()(DocIterator const & cur, int len = -1, bool at_begin = true) const;
#if QTSEARCH
	bool regexIsValid;
	string regexError;
#endif

public:
	/// buffer
	lyx::Buffer * p_buf;
	/// first buffer on which search was started
	lyx::Buffer * const p_first_buf;
	/// options
	FindAndReplaceOptions const & opt;

private:
	/// Auxiliary find method (does not account for opt.matchword)
	MatchResult findAux(DocIterator const & cur, int len = -1, bool at_begin = true) const;
	void CreateRegexp(FindAndReplaceOptions const & opt, string regexp_str, string regexp2_str, string par_as_string = "");

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
	// regexp2 is same as regexp, but prefixed with a ".*?"
#if QTSEARCH
	QRegularExpression regexp;
	QRegularExpression regexp2;
#else
	regex regexp;
	regex regexp2;
#endif
	// leading format material as string
	string lead_as_string;
	// par_as_string after removal of lead_as_string
	string par_as_string_nolead;
	// unmatched open braces in the search string/regexp
	int open_braces;
	// number of (.*?) subexpressions added at end of search regexp for closing
	// environments, math mode, styles, etc...
	int close_wildcards;
public:
	// Are we searching with regular expressions ?
	bool use_regexp;
	static int valid_matches;
	static vector <string> matches;
	void FillResults(MatchResult &found_mr);
};

int MatchStringAdv::valid_matches = 0;
vector <string> MatchStringAdv::matches = vector <string>(10);

void MatchStringAdv::FillResults(MatchResult &found_mr)
{
  if (found_mr.match_len > 0) {
    valid_matches = found_mr.result.size();
    for (size_t i = 0; i < found_mr.result.size(); i++)
      matches[i] = found_mr.result[i];
  }
  else
    valid_matches = 0;
}

static docstring buffer_to_latex(Buffer & buffer)
{
	//OutputParams runparams(&buffer.params().encoding());
	OutputParams runparams(encodings.fromLyXName("utf8"));
	odocstringstream ods;
	otexstream os(ods);
	runparams.nice = true;
	runparams.flavor = Flavor::XeTeX;
	runparams.linelen = 10000; //lyxrc.plaintext_linelen;
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
		// OutputParams runparams(&buffer.params().encoding());
		OutputParams runparams(encodings.fromLyXName("utf8"));
		runparams.nice = true;
		runparams.flavor = Flavor::XeTeX;
		runparams.linelen = 10000; //lyxrc.plaintext_linelen;
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
		// Even in ignore-format we have to remove "\text{}, \lyxmathsym{}" parts
		string t = to_utf8(str);
		while (regex_replace(t, t, "\\\\(text|lyxmathsym)\\{([^\\}]*)\\}", "$2"));
		str = from_utf8(t);
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
	while (regex_replace(t, t, "^\\\\(("
	                     "(author|title|subtitle|subject|publishers|dedication|uppertitleback|lowertitleback|extratitle|"
	                       "lyxaddress|lyxrightaddress|"
	                       "footnotesize|tiny|scriptsize|small|large|Large|LARGE|huge|Huge|"
	                       "emph|noun|minisec|text(bf|md|sl|sf|it|tt))|"
	                     "((textcolor|foreignlanguage|latexenvironment)\\{[a-z]+\\*?\\})|"
	                     "(u|uu)line|(s|x)out|uwave)|((sub)?(((sub)?section)|paragraph)|part|chapter)\\*?)\\{", "")
	       || regex_replace(t, t, "^\\$", "")
	       || regex_replace(t, t, "^\\\\\\[", "")
	       || regex_replace(t, t, "^ ?\\\\item\\{[a-z]+\\}", "")
	       || regex_replace(t, t, "^\\\\begin\\{[a-zA-Z_]*\\*?\\}", ""))
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
	static regex const feature("\\\\(([a-zA-Z]+(\\{([a-z]+\\*?)\\}|\\*)?))\\{");
	static regex const valid("^("
		"("
			"(footnotesize|tiny|scriptsize|small|large|Large|LARGE|huge|Huge|"
				"emph|noun|text(bf|md|sl|sf|it|tt)|"
				"(textcolor|foreignlanguage|item|listitem|latexenvironment)\\{[a-z]+\\*?\\})|"
			"(u|uu)line|(s|x)out|uwave|"
			"(sub|extra)?title|author|subject|publishers|dedication|(upper|lower)titleback|lyx(right)?address)|"
		"((sub)?(((sub)?section)|paragraph)|part|chapter|lyxslide)\\*?)$");
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
	return info;
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
    /* replace starting backslash with '#' */
    isText,
    /* \part, \section*, ... */
    isSectioning,
    /* title, author etc */
    isTitle,
    /* \foreignlanguage{ngerman}, ... */
    isMain,
    /* inside \code{}
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
    /* inputencoding, ...
     * Discard also content, because they do not help in search */
    doRemove,
    /* twocolumns, ...
     * like remove, but also all arguments */
    removeWithArg,
    /* item, listitem */
    isList,
    /* tex, latex, ... like isChar */
    isIgnored,
    /* like \lettrine[lines=5]{}{} */
    cleanToStart,
    // like isStandard, but always remove head
    headRemove,
    /* End of arguments marker for lettrine,
     * so that they can be ignored */
    endArguments
  };
 KeyInfo() = default;
 KeyInfo(KeyType type, int parcount, bool disable)
   : keytype(type),
    parenthesiscount(parcount),
    disabled(disable) {}
  KeyType keytype = invalid;
  string head;
  int _tokensize = -1;
  int _tokenstart = -1;
  int _dataStart = -1;
  int _dataEnd = -1;
  int parenthesiscount = 1;
  bool disabled = false;
  bool used = false;                    /* by pattern */
};

class Border {
 public:
 Border(int l=0, int u=0) : low(l), upper(u) {};
  int low;
  int upper;
};

#define MAXOPENED 30
class Intervall {
  bool isPatternString_;
public:
  explicit Intervall(bool isPattern, string const & p) :
        isPatternString_(isPattern), par(p), ignoreidx(-1), actualdeptindex(0),
        hasTitle(false), langcount(0)
  {
    depts[0] = 0;
    closes[0] = 0;
  }

  string par;
  int ignoreidx;
  static vector<Border> borders;
  int depts[MAXOPENED];
  int closes[MAXOPENED];
  int actualdeptindex;
  int previousNotIgnored(int) const;
  int nextNotIgnored(int) const;
  void handleOpenP(int i);
  void handleCloseP(int i, bool closingAllowed);
  void resetOpenedP(int openPos);
  void addIntervall(int upper);
  void addIntervall(int low, int upper); /* if explicit */
  void removeAccents();
  void setForDefaultLang(KeyInfo const & defLang) const;
  int findclosing(int start, int end, char up, char down, int repeat);
  void handleParentheses(int lastpos, bool closingAllowed);
  bool hasTitle;
  int langcount;	// Number of disabled language specs up to current position in actual interval
  int isOpeningPar(int pos) const;
  string titleValue;
  void output(ostringstream &os, int lastpos);
  // string show(int lastpos);
};

vector<Border> Intervall::borders = vector<Border>(30);

int Intervall::isOpeningPar(int pos) const
{
  if ((pos < 0) || (size_t(pos) >= par.size()))
    return 0;
  if (par[pos] != '{')
    return 0;
  if (size_t(pos) + 2 >= par.size())
    return 1;
  if (par[pos+2] != '}')
    return 1;
  if (par[pos+1] == '[' || par[pos+1] == ']')
    return 3;
  return 1;
}

void Intervall::setForDefaultLang(KeyInfo const & defLang) const
{
  // Enable the use of first token again
  if (ignoreidx >= 0) {
    int value = defLang._tokenstart + defLang._tokensize;
    int borderidx = 0;
    if (hasTitle) {
      borderidx = 1;
    }
    if (value > 0) {
      if (borders[borderidx].low < value)
        borders[borderidx].low = value;
      if (borders[borderidx].upper < value)
        borders[borderidx].upper = value;
    }
  }
}

static void checkDepthIndex(int val)
{
  static int maxdepthidx = MAXOPENED-2;
  static int lastmaxdepth = 0;
  if (val > lastmaxdepth) {
    LYXERR(Debug::INFO, "Depth reached " << val);
    lastmaxdepth = val;
  }
  if (val > maxdepthidx) {
    maxdepthidx = val;
    LYXERR(Debug::INFO, "maxdepthidx now " << val);
  }
}

#if 0
// Not needed, because borders are now dynamically expanded
static void checkIgnoreIdx(int val)
{
  static int lastmaxignore = -1;
  if ((lastmaxignore < val) && (size_t(val+1) >= borders.size())) {
    LYXERR(Debug::INFO, "IgnoreIdx reached " << val);
    lastmaxignore = val;
  }
}
#endif

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
    if (borders.size() <= size_t(idx)) {
      borders.push_back(br);
    }
    else {
      borders[idx] = br;
    }
    ignoreidx = idx;
    // checkIgnoreIdx(ignoreidx);
    return;
  }
  else {
    // Expand only if one of the new bound is inside the interwall
    // We know here that br.low > borders[idx-1].upper
    if (br.upper < borders[idx].low) {
      // We have to insert at this pos
      if (size_t(ignoreidx+1) >= borders.size()) {
        borders.push_back(borders[ignoreidx]);
      }
      else {
        borders[ignoreidx+1] = borders[ignoreidx];
      }
      for (int i = ignoreidx; i > idx; --i) {
        borders[i] = borders[i-1];
      }
      borders[idx] = br;
      ignoreidx += 1;
      // checkIgnoreIdx(ignoreidx);
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

static void buildaccent(string n, string param, string values)
{
  stringstream s(n);
  string name;
  const char delim = '|';
  while (getline(s, name, delim)) {
    size_t start = 0;
    for (char c : param) {
      string key = name + "{" + c + "}";
      // get the corresponding utf8-value
      if ((values[start] & 0xc0) != 0xc0) {
        // should not happen, utf8 encoding starts at least with 11xxxxxx
        // but value for '\dot{i}' is 'i', which is ascii
        if ((values[start] & 0x80) == 0) {
          // is ascii
          accents[key] = values.substr(start, 1);
          // LYXERR(Debug::INFO, "" << key << "=" << accents[key]);
        }
        start++;
        continue;
      }
      for (int j = 1; ;j++) {
        if (start + j >= values.size()) {
          accents[key] = values.substr(start, j);
          start = values.size() - 1;
          break;
        }
        else if ((values[start+j] & 0xc0) != 0x80) {
          // This is the first byte of following utf8 char
          accents[key] = values.substr(start, j);
          start += j;
          // LYXERR(Debug::INFO, "" << key << "=" << accents[key]);
          break;
        }
      }
    }
  }
}

// Helper function
static string getutf8(unsigned uchar)
{
	#define maxc 5
	string ret = string();
	char c[maxc] = {0};
	if (uchar <= 0x7f) {
		c[maxc-1] = uchar & 0x7f;
	}
	else {
		unsigned char rest = 0x40;
		unsigned char first = 0x80;
		int start = maxc-1;
		for (int i = start; i >=0; --i) {
			if (uchar < rest) {
				c[i] = first + uchar;
				break;
			}
			c[i] = 0x80 | (uchar &  0x3f);
			uchar >>= 6;
			rest >>= 1;
			first >>= 1;
			first |= 0x80;
		}
	}
	for (int i = 0; i < maxc; i++) {
		if (c[i] == 0) continue;
		ret += c[i];
	}
	return(ret);
}

static void buildAccentsMap()
{
  accents["imath"] = "ı";
  accents["i"] = "ı";
  accents["jmath"] = "ȷ";
  accents["cdot"] = "·";
  accents["textasciicircum"] = "^";
  accents["mathcircumflex"] = "^";
  accents["sim"] = "~";
  accents["guillemotright"] = "»";
  accents["guillemotleft"] = "«";
  accents["hairspace"]     = getutf8(0xf0000);	// select from free unicode plane 15
  accents["thinspace"]     = getutf8(0xf0002);	// and used _only_ by findadv
  accents["negthinspace"]  = getutf8(0xf0003);	// to omit backslashed latex macros
  accents["medspace"]      = getutf8(0xf0004);	// See https://en.wikipedia.org/wiki/Private_Use_Areas
  accents["negmedspace"]   = getutf8(0xf0005);
  accents["thickspace"]    = getutf8(0xf0006);
  accents["negthickspace"] = getutf8(0xf0007);
  accents["lyx"]           = getutf8(0xf0010);	// Used logos
  accents["LyX"]           = getutf8(0xf0010);
  accents["tex"]           = getutf8(0xf0011);
  accents["TeX"]           = getutf8(0xf0011);
  accents["latex"]         = getutf8(0xf0012);
  accents["LaTeX"]         = getutf8(0xf0012);
  accents["latexe"]        = getutf8(0xf0013);
  accents["LaTeXe"]        = getutf8(0xf0013);
  accents["lyxarrow"]      = getutf8(0xf0020);
  accents["braceleft"]     = getutf8(0xf0030);
  accents["braceright"]    = getutf8(0xf0031);
  accents["backslash lyx"]           = getutf8(0xf0010);	// Used logos inserted with starting \backslash
  accents["backslash LyX"]           = getutf8(0xf0010);
  accents["backslash tex"]           = getutf8(0xf0011);
  accents["backslash TeX"]           = getutf8(0xf0011);
  accents["backslash latex"]         = getutf8(0xf0012);
  accents["backslash LaTeX"]         = getutf8(0xf0012);
  accents["backslash latexe"]        = getutf8(0xf0013);
  accents["backslash LaTeXe"]        = getutf8(0xf0013);
  accents["backslash lyxarrow"]      = getutf8(0xf0020);
  accents["ddot{\\imath}"] = "ï";
  buildaccent("ddot", "aAeEhHiIioOtuUwWxXyY",
                      "äÄëËḧḦïÏïöÖẗüÜẅẄẍẌÿŸ");	// umlaut
  buildaccent("dot|.", "aAbBcCdDeEfFGghHIimMnNoOpPrRsStTwWxXyYzZ",
                       "ȧȦḃḂċĊḋḊėĖḟḞĠġḣḢİİṁṀṅṄȯȮṗṖṙṘṡṠṫṪẇẆẋẊẏẎżŻ");	// dot{i} can only happen if ignoring case, but there is no lowercase of 'İ'
  accents["acute{\\imath}"] = "í";
  buildaccent("acute", "aAcCeEgGkKlLmMoOnNpPrRsSuUwWyYzZiI",
                       "áÁćĆéÉǵǴḱḰĺĹḿḾóÓńŃṕṔŕŔśŚúÚẃẂýÝźŹíÍ");
  buildaccent("dacute|H|h", "oOuU", "őŐűŰ");	// double acute
  buildaccent("mathring|r", "aAuUwy",
                            "åÅůŮẘẙ");	// ring
  accents["check{\\imath}"] = "ǐ";
  accents["check{\\jmath}"] = "ǰ";
  buildaccent("check|v", "cCdDaAeEiIoOuUgGkKhHlLnNrRsSTtzZ",
                         "čČďĎǎǍěĚǐǏǒǑǔǓǧǦǩǨȟȞľĽňŇřŘšŠŤťžŽ");	// caron
  accents["hat{\\imath}"] = "î";
  accents["hat{\\jmath}"] = "ĵ";
  buildaccent("hat|^", "aAcCeEgGhHiIjJoOsSuUwWyYzZ",
                       "âÂĉĈêÊĝĜĥĤîÎĵĴôÔŝŜûÛŵŴŷŶẑẐ");	// circ
  accents["bar{\\imath}"] = "ī";
  buildaccent("bar|=", "aAeEiIoOuUyY",
                       "āĀēĒīĪōŌūŪȳȲ");	// macron
  accents["tilde{\\imath}"] = "ĩ";
  buildaccent("tilde", "aAeEiInNoOuUvVyY",
                       "ãÃẽẼĩĨñÑõÕũŨṽṼỹỸ");	// tilde
  accents["breve{\\imath}"] = "ĭ";
  buildaccent("breve|u", "aAeEgGiIoOuU",
                         "ăĂĕĔğĞĭĬŏŎŭŬ");	// breve
  accents["grave{\\imath}"] = "ì";
  buildaccent("grave|`", "aAeEiIoOuUnNwWyY",
                         "àÀèÈìÌòÒùÙǹǸẁẀỳỲ");	// grave
  buildaccent("subdot|d", "BbDdHhKkLlMmNnRrSsTtVvWwZzAaEeIiOoUuYy",
                          "ḄḅḌḍḤḥḲḳḶḷṂṃṆṇṚṛṢṣṬṭṾṿẈẉẒẓẠạẸẹỊịỌọỤụỴỵ");	// dot below
  buildaccent("ogonek|k", "AaEeIiUuOo",
                          "ĄąĘęĮįŲųǪǫ");	// ogonek
  buildaccent("cedilla|c", "CcGgKkLlNnRrSsTtEeDdHh",
                           "ÇçĢģĶķĻļŅņŖŗŞşŢţȨȩḐḑḨḩ");	// cedilla
  buildaccent("subring|textsubring", "Aa",
                                     "Ḁḁ");	// subring
  buildaccent("subhat|textsubcircum", "DdEeLlNnTtUu",
                                      "ḒḓḘḙḼḽṊṋṰṱṶṷ");	// subcircum
  buildaccent("subtilde|textsubtilde", "EeIiUu",
                                       "ḚḛḬḭṴṵ");	// subtilde
  accents["dgrave{\\imath}"] = "ȉ";
  accents["textdoublegrave{\\i}"] = "ȉ";
  buildaccent("dgrave|textdoublegrave", "AaEeIiOoRrUu",
                                        "ȀȁȄȅȈȉȌȍȐȑȔȕ"); // double grave
  accents["rcap{\\imath}"] = "ȉ";
  accents["textroundcap{\\i}"] = "ȉ";
  buildaccent("rcap|textroundcap", "AaEeIiOoRrUu",
                                   "ȂȃȆȇȊȋȎȏȒȓȖȗ"); // inverted breve
  buildaccent("slashed", "oO",
                         "øØ"); // slashed
}

/*
 * Created accents in math or regexp environment
 * are macros, but we need the utf8 equivalent
 */
void Intervall::removeAccents()
{
  if (accents.empty())
    buildAccentsMap();
  static regex const accre("\\\\(([\\S]|grave|breve|ddot|dot|acute|dacute|mathring|check|hat|bar|tilde|subdot|ogonek|"
         "cedilla|subring|textsubring|subhat|textsubcircum|subtilde|textsubtilde|dgrave|textdoublegrave|rcap|textroundcap|slashed)\\{[^\\{\\}]+\\}"
      "|((i|imath|jmath|cdot|[a-z]+space)|((backslash )?([lL]y[xX]|[tT]e[xX]|[lL]a[tT]e[xX]e?|lyxarrow))|(brace|guillemot)(left|right)|textasciicircum|mathcircumflex|sim)(?![a-zA-Z]))");
  smatch sub;
  for (sregex_iterator itacc(par.begin(), par.end(), accre), end; itacc != end; ++itacc) {
    sub = *itacc;
    string key = sub.str(1);
    if (accents.find(key) != accents.end()) {
      string val = accents[key];
      size_t pos = sub.position(size_t(0));
      for (size_t i = 0; i < val.size(); i++) {
        par[pos+i] = val[i];
      }
      // Remove possibly following space too
      if (par[pos+sub.str(0).size()] == ' ')
        addIntervall(pos+val.size(), pos + sub.str(0).size()+1);
      else
        addIntervall(pos+val.size(), pos + sub.str(0).size());
      for (size_t i = pos+val.size(); i < pos + sub.str(0).size(); i++) {
        // remove traces of any remaining chars
        par[i] = ' ';
      }
    }
    else {
      LYXERR(Debug::INFO, "Not added accent for \"" << key << "\"");
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

int Intervall::previousNotIgnored(int start) const
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

int Intervall::nextNotIgnored(int start) const
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
  int entidx_;
  Entries entries_;
  Intervall interval_;
  void buildKeys(bool);
  void buildEntries(bool);
  void makeKey(const string &, KeyInfo, bool isPatternString);
  void processRegion(int start, int region_end); /*  remove {} parts */
  void removeHead(KeyInfo const &, int count=0);

 public:
 LatexInfo(string const & par, bool isPatternString)
         : entidx_(-1), interval_(isPatternString, par)
  {
    buildKeys(isPatternString);
    entries_ = vector<KeyInfo>();
    buildEntries(isPatternString);
  };
  int getFirstKey() {
    entidx_ = 0;
    if (entries_.empty()) {
      return -1;
    }
    if (entries_[0].keytype == KeyInfo::isTitle) {
      interval_.hasTitle = true;
      if (! entries_[0].disabled) {
        interval_.titleValue = entries_[0].head;
      }
      else {
        interval_.titleValue = "";
      }
      removeHead(entries_[0]);
      if (entries_.size() > 1)
        return 1;
      else
        return -1;
    }
    return 0;
  };
  int getNextKey() {
    entidx_++;
    if (int(entries_.size()) > entidx_) {
      return entidx_;
    }
    else {
      return -1;
    }
  };
  bool setNextKey(int idx) {
    if ((idx == entidx_) && (entidx_ >= 0)) {
      entidx_--;
      return true;
    }
    else
      return false;
  };
  int find(int start, KeyInfo::KeyType keytype) const {
    if (start < 0)
      return -1;
    int tmpIdx = start;
    while (tmpIdx < int(entries_.size())) {
      if (entries_[tmpIdx].keytype == keytype)
        return tmpIdx;
      tmpIdx++;
    }
    return -1;
  };
  int process(ostringstream & os, KeyInfo const & actual);
  int dispatch(ostringstream & os, int previousStart, KeyInfo & actual);
  // string show(int lastpos) { return interval.show(lastpos);};
  int nextNotIgnored(int start) { return interval_.nextNotIgnored(start);};
  KeyInfo &getKeyInfo(int keyinfo) {
    static KeyInfo invalidInfo = KeyInfo();
    if ((keyinfo < 0) || ( keyinfo >= int(entries_.size())))
      return invalidInfo;
    else
      return entries_[keyinfo];
  };
  void setForDefaultLang(KeyInfo const & defLang) {interval_.setForDefaultLang(defLang);};
  void addIntervall(int low, int up) { interval_.addIntervall(low, up); };
};


int Intervall::findclosing(int start, int end, char up = '{', char down = '}', int repeat = 1)
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
      if (depth == 0) {
        repeat--;
        if ((repeat <= 0) || (par[i+1] != up))
          return i;
      }
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
  size_t actualIdx_;
  vector<MathEntry> entries_;
 public:
  MathInfo() {
    actualIdx_ = 0;
  }
  void insert(string const & wait, size_t start, size_t end) {
    MathEntry m = MathEntry();
    m.wait = wait;
    m.mathStart = start;
    m.mathEnd = end;
    m.mathSize = end - start;
    entries_.push_back(m);
  }
  bool empty() const { return entries_.empty(); };
  size_t getEndPos() const {
    if (entries_.empty() || (actualIdx_ >= entries_.size())) {
      return 0;
    }
    return entries_[actualIdx_].mathEnd;
  }
  size_t getStartPos() const {
    if (entries_.empty() || (actualIdx_ >= entries_.size())) {
      return 100000;                    /*  definitely enough? */
    }
    return entries_[actualIdx_].mathStart;
  }
  size_t getFirstPos() {
    actualIdx_ = 0;
    return getStartPos();
  }
  size_t getSize() const {
    if (entries_.empty() || (actualIdx_ >= entries_.size())) {
      return size_t(0);
    }
    return entries_[actualIdx_].mathSize;
  }
  void incrEntry() { actualIdx_++; };
};

void LatexInfo::buildEntries(bool isPatternString)
{
  static regex const rmath("(\\\\)*(\\$|\\\\\\[|\\\\\\]|\\\\(begin|end)\\{((eqnarray|equation|flalign|gather|multline|align|alignat)\\*?)\\})");
  static regex const rkeys("(\\\\)*(\\$|\\\\\\[|\\\\\\]|\\\\((([a-zA-Z]+\\*?)(\\{([a-z]+\\*?)\\}|=[0-9]+[a-z]+)?)))");
  static bool disableLanguageOverride = false;
  smatch sub, submath;
  bool evaluatingRegexp = false;
  MathInfo mi;
  bool evaluatingMath = false;
  bool evaluatingCode = false;
  size_t codeEnd = 0;
  bool evaluatingOptional = false;
  size_t optionalEnd = 0;
  int codeStart = -1;
  KeyInfo found;
  bool math_end_waiting = false;
  size_t math_pos = 10000;
  string math_end;
  static vector<string> usedText = vector<string>();

  interval_.removeAccents();

  for (sregex_iterator itmath(interval_.par.begin(), interval_.par.end(), rmath), end; itmath != end; ++itmath) {
    submath = *itmath;
    if ((submath.position(2) - submath.position(0)) %2 == 1) {
      // prefixed by odd count of '\\'
      continue;
    }
    if (math_end_waiting) {
      size_t pos = submath.position(size_t(2));
      if ((math_end == "$") &&
          (submath.str(2) == "$")) {
        mi.insert("$", math_pos, pos + 1);
        math_end_waiting = false;
      }
      else if ((math_end == "\\]") &&
               (submath.str(2) == "\\]")) {
        mi.insert("\\]", math_pos, pos + 2);
        math_end_waiting = false;
      }
      else if ((submath.str(3).compare("end") == 0) &&
          (submath.str(4).compare(math_end) == 0)) {
        mi.insert(math_end, math_pos, pos + submath.str(2).length());
        math_end_waiting = false;
      }
      else
        continue;
    }
    else {
      if (submath.str(3).compare("begin") == 0) {
        math_end_waiting = true;
        math_end = submath.str(4);
        math_pos = submath.position(size_t(2));
      }
      else if (submath.str(2).compare("\\[") == 0) {
        math_end_waiting = true;
        math_end = "\\]";
        math_pos = submath.position(size_t(2));
      }
      else if (submath.str(2) == "$") {
        size_t pos = submath.position(size_t(2));
        math_end_waiting = true;
        math_end = "$";
        math_pos = pos;
      }
    }
  }
  // Ignore language if there is math somewhere in pattern-string
  if (isPatternString) {
    for (auto s: usedText) {
      // Remove entries created in previous search runs
      keys.erase(s);
    }
    usedText = vector<string>();
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
  for (sregex_iterator it(interval_.par.begin(), interval_.par.end(), rkeys), end; it != end; ++it) {
    sub = *it;
    if ((sub.position(2) - sub.position(0)) %2 == 1) {
      // prefixed by odd count of '\\'
      continue;
    }
    string key = sub.str(5);
    if (key == "") {
      if (sub.str(2)[0] == '\\')
        key = sub.str(2)[1];
      else {
        key = sub.str(2);
      }
    }
    if (keys.find(key) != keys.end()) {
      if (keys[key].keytype == KeyInfo::headRemove) {
        KeyInfo found1 = keys[key];
        found1.disabled = true;
        found1.head = "\\" + key + "{";
        found1._tokenstart = sub.position(size_t(2));
        found1._tokensize = found1.head.length();
        found1._dataStart = found1._tokenstart + found1.head.length();
        int endpos = interval_.findclosing(found1._dataStart, interval_.par.length(), '{', '}', 1);
        found1._dataEnd = endpos;
        removeHead(found1);
        continue;
      }
    }
    if (evaluatingRegexp) {
      if (sub.str(3).compare("endregexp") == 0) {
        evaluatingRegexp = false;
        // found._tokenstart already set
        found._dataEnd = sub.position(size_t(2)) + 13;
        found._dataStart = found._dataEnd;
        found._tokensize = found._dataEnd - found._tokenstart;
        found.parenthesiscount = 0;
        found.head = interval_.par.substr(found._tokenstart, found._tokensize);
      }
      else {
        continue;
      }
    }
    else {
      if (evaluatingMath) {
        if (size_t(sub.position(size_t(2))) < mi.getEndPos())
          continue;
        evaluatingMath = false;
        mi.incrEntry();
        math_pos = mi.getStartPos();
      }
      if (keys.find(key) == keys.end()) {
        found = KeyInfo(KeyInfo::isStandard, 0, true);
        LYXERR(Debug::INFO, "Undefined key " << key << " ==> will be used as text");
        found = KeyInfo(KeyInfo::isText, 0, false);
        if (isPatternString) {
          found.keytype = KeyInfo::isChar;
          found.disabled = false;
          found.used = true;
        }
        keys[key] = found;
        usedText.push_back(key);
      }
      else
        found = keys[key];
      if (key.compare("regexp") == 0) {
        evaluatingRegexp = true;
        found._tokenstart = sub.position(size_t(2));
        found._tokensize = 0;
        continue;
      }
    }
    // Handle the other params of key
    if (found.keytype == KeyInfo::isIgnored)
      continue;
    else if (found.keytype == KeyInfo::isMath) {
      if (size_t(sub.position(size_t(2))) == math_pos) {
        found = keys[key];
        found._tokenstart = sub.position(size_t(2));
        found._tokensize = mi.getSize();
        found._dataEnd = found._tokenstart + found._tokensize;
        found._dataStart = found._dataEnd;
        found.parenthesiscount = 0;
        found.head = interval_.par.substr(found._tokenstart, found._tokensize);
        evaluatingMath = true;
      }
      else {
        // begin|end of unknown env, discard
        // First handle tables
        // longtable|tabular
        bool discardComment;
        found = keys[key];
        found.keytype = KeyInfo::doRemove;
        if ((sub.str(7).compare("longtable") == 0) ||
            (sub.str(7).compare("tabular") == 0)) {
          discardComment = true;        /* '%' */
        }
        else {
          discardComment = false;
          static regex const removeArgs("^(multicols|multipar|sectionbox|subsectionbox|tcolorbox)$");
          smatch sub2;
          string token = sub.str(7);
          if (regex_match(token, sub2, removeArgs)) {
            found.keytype = KeyInfo::removeWithArg;
          }
        }
        // discard spaces before pos(2)
        int pos = sub.position(size_t(2));
        int count;
        for (count = 0; pos - count > 0; count++) {
          char c = interval_.par[pos-count-1];
          if (discardComment) {
            if ((c != ' ') && (c != '%'))
              break;
          }
          else if (c != ' ')
            break;
        }
        found._tokenstart = pos - count;
        if (sub.str(3).compare(0, 5, "begin") == 0) {
          size_t pos1 = pos + sub.str(2).length();
          if (sub.str(7).compare("cjk") == 0) {
            pos1 = interval_.findclosing(pos1+1, interval_.par.length()) + 1;
            if ((interval_.par[pos1] == '{') && (interval_.par[pos1+1] == '}'))
              pos1 += 2;
            found.keytype = KeyInfo::isMain;
            found._dataStart = pos1;
            found._dataEnd = interval_.par.length();
            found.disabled = keys["foreignlanguage"].disabled;
            found.used = keys["foreignlanguage"].used;
            found._tokensize = pos1 - found._tokenstart;
            found.head = interval_.par.substr(found._tokenstart, found._tokensize);
          }
          else {
            // Swallow possible optional params
            while (interval_.par[pos1] == '[') {
              pos1 = interval_.findclosing(pos1+1, interval_.par.length(), '[', ']')+1;
            }
            // Swallow also the eventual parameter
            if (interval_.par[pos1] == '{') {
              found._dataEnd = interval_.findclosing(pos1+1, interval_.par.length()) + 1;
            }
            else {
              found._dataEnd = pos1;
            }
            found._dataStart = found._dataEnd;
            found._tokensize = count + found._dataEnd - pos;
            found.parenthesiscount = 0;
            found.head = interval_.par.substr(found._tokenstart, found._tokensize);
            found.disabled = true;
          }
        }
        else {
          // Handle "\end{...}"
          found._dataStart = pos + sub.str(2).length();
          found._dataEnd = found._dataStart;
          found._tokensize = count + found._dataEnd - pos;
          found.parenthesiscount = 0;
          found.head = interval_.par.substr(found._tokenstart, found._tokensize);
          found.disabled = true;
        }
      }
    }
    else if (found.keytype != KeyInfo::isRegex) {
      found._tokenstart = sub.position(size_t(2));
      if (found.parenthesiscount == 0) {
        // Probably to be discarded
        size_t following_pos = sub.position(size_t(2)) + sub.str(5).length() + 1;
        char following = interval_.par[following_pos];
        if (following == ' ')
          found.head = "\\" + sub.str(5) + " ";
        else if (following == '=') {
          // like \uldepth=1000pt
          found.head = sub.str(2);
        }
        else
          found.head = "\\" + key;
        found._tokensize = found.head.length();
        found._dataEnd = found._tokenstart + found._tokensize;
        found._dataStart = found._dataEnd;
      }
      else {
        int params = found._tokenstart + key.length() + 1;
        if (evaluatingOptional) {
          if (size_t(found._tokenstart) > optionalEnd) {
            evaluatingOptional = false;
          }
          else {
            found.disabled = true;
          }
        }
        int optend = params;
        while (interval_.par[optend] == '[') {
          // discard optional parameters
          optend = interval_.findclosing(optend+1, interval_.par.length(), '[', ']') + 1;
        }
        if (optend > params) {
          key += interval_.par.substr(params, optend-params);
          evaluatingOptional = true;
          optionalEnd = optend;
          if (found.keytype == KeyInfo::isSectioning) {
            // Remove optional values (but still keep in header)
            interval_.addIntervall(params, optend);
          }
        }
        string token = sub.str(7);
        int closings;
        if (interval_.par[optend] != '{') {
          closings = 0;
          found.parenthesiscount = 0;
          found.head = "\\" + key;
        }
        else
          closings = found.parenthesiscount;
        if (found.parenthesiscount == 1) {
          found.head = "\\" + key + "{";
        }
        else if (found.parenthesiscount > 1) {
          if (token != "") {
            found.head = sub.str(2) + "{";
            closings = found.parenthesiscount - 1;
          }
          else {
            found.head = "\\" + key + "{";
          }
        }
        found._tokensize = found.head.length();
        found._dataStart = found._tokenstart + found.head.length();
        if (found.keytype == KeyInfo::doRemove) {
          if (closings > 0) {
            size_t endpar = 2 + interval_.findclosing(found._dataStart, interval_.par.length(), '{', '}', closings);
            if (endpar >= interval_.par.length())
              found._dataStart = interval_.par.length();
            else
              found._dataStart = endpar;
            found._tokensize = found._dataStart - found._tokenstart;
          }
          else {
            found._dataStart = found._tokenstart + found._tokensize;
          }
          closings = 0;
        }
        if (interval_.par.substr(found._dataStart, 15).compare("\\endarguments{}") == 0) {
          found._dataStart += 15;
        }
        size_t endpos;
        if (closings < 1)
          endpos = found._dataStart - 1;
        else
          endpos = interval_.findclosing(found._dataStart, interval_.par.length(), '{', '}', closings);
        if (found.keytype == KeyInfo::isList) {
          // Check if it really is list env
          static regex const listre("^([a-z]+)$");
          smatch sub2;
          if (!regex_match(token, sub2, listre)) {
            // Change the key of this entry. It is not in a list/item environment
            found.keytype = KeyInfo::endArguments;
          }
        }
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
            if ((codeEnd +1 >= interval_.par.length()) &&
                (found._tokenstart == codeStart)) {
              // trickery, because the code inset starts
              // with \selectlanguage ...
              codeEnd = endpos;
              if (entries_.size() > 1) {
                entries_[entries_.size()-1]._dataEnd = codeEnd;
              }
            }
          }
        }
        if ((endpos == interval_.par.length()) &&
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
    entries_.push_back(found);
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

  // Keys to ignore in any case
  makeKey("text|textcyrillic|lyxmathsym", KeyInfo(KeyInfo::headRemove, 1, true), true);
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
  makeKey("title|subtitle|author|subject|publishers|dedication|uppertitleback|lowertitleback|extratitle|lyxaddress|lyxrightaddress", KeyInfo(KeyInfo::isTitle, 1, ignoreFormats.getFrontMatter()), isPatternString);
  // Regex
  makeKey("regexp", KeyInfo(KeyInfo::isRegex, 1, false), isPatternString);

  // Split is done, if not at start of region
  makeKey("textcolor", KeyInfo(KeyInfo::isStandard, 2, ignoreFormats.getColor()), isPatternString);
  makeKey("latexenvironment", KeyInfo(KeyInfo::isStandard, 2, false), isPatternString);

  // Split is done always.
  makeKey("foreignlanguage", KeyInfo(KeyInfo::isMain, 2, ignoreFormats.getLanguage()), isPatternString);

  // Known charaters
  // No split
  makeKey("backslash|textbackslash|slash",  KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("textasciicircum|textasciitilde", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("textasciiacute|texemdash",       KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("dots|ldots",                     KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  // Spaces
  makeKey("quad|qquad|hfill|dotfill",               KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("textvisiblespace|nobreakspace",          KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("negthickspace|negmedspace|negthinspace", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("thickspace|medspace|thinspace",          KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  // Skip
  // makeKey("enskip|smallskip|medskip|bigskip|vfill", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  // Custom space/skip, remove the content (== length value)
  makeKey("vspace|vspace*|hspace|hspace*|mspace", KeyInfo(KeyInfo::noContent, 1, false), isPatternString);
  // Found in fr/UserGuide.lyx
  makeKey("og|fg", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  // quotes
  makeKey("textquotedbl|quotesinglbase|lyxarrow", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("textquotedblleft|textquotedblright", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  // Known macros to remove (including their parameter)
  // No split
  makeKey("input|inputencoding|label|ref|index|bibitem", KeyInfo(KeyInfo::doRemove, 1, false), isPatternString);
  makeKey("addtocounter|setlength",                 KeyInfo(KeyInfo::noContent, 2, true), isPatternString);
  // handle like standard keys with 1 parameter.
  makeKey("url|href|vref|thanks", KeyInfo(KeyInfo::isStandard, 1, false), isPatternString);

  // Ignore deleted text
  makeKey("lyxdeleted", KeyInfo(KeyInfo::doRemove, 3, false), isPatternString);
  // but preserve added text
  makeKey("lyxadded", KeyInfo(KeyInfo::doRemove, 2, false), isPatternString);

  // Macros to remove, but let the parameter survive
  // No split
  makeKey("menuitem|textmd|textrm", KeyInfo(KeyInfo::isStandard, 1, true), isPatternString);

  // Remove language spec from content of these insets
  makeKey("code", KeyInfo(KeyInfo::noMain, 1, false), isPatternString);

  // Same effect as previous, parameter will survive (because there is no one anyway)
  // No split
  makeKey("noindent|textcompwordmark|maketitle", KeyInfo(KeyInfo::isStandard, 0, true), isPatternString);
  // Remove table decorations
  makeKey("hline|tabularnewline|toprule|bottomrule|midrule", KeyInfo(KeyInfo::doRemove, 0, true), isPatternString);
  // Discard shape-header.
  // For footnote or shortcut too, because of lang settings
  // and wrong handling if used 'KeyInfo::noMain'
  makeKey("circlepar|diamondpar|heartpar|nutpar",  KeyInfo(KeyInfo::isStandard, 1, true), isPatternString);
  makeKey("trianglerightpar|hexagonpar|starpar",   KeyInfo(KeyInfo::isStandard, 1, true), isPatternString);
  makeKey("triangleuppar|triangledownpar|droppar", KeyInfo(KeyInfo::isStandard, 1, true), isPatternString);
  makeKey("triangleleftpar|shapepar|dropuppar",    KeyInfo(KeyInfo::isStandard, 1, true), isPatternString);
  makeKey("hphantom|vphantom|footnote|shortcut|include|includegraphics",     KeyInfo(KeyInfo::isStandard, 1, true), isPatternString);
  makeKey("parbox", KeyInfo(KeyInfo::doRemove, 1, true), isPatternString);
  // like ('tiny{}' or '\tiny ' ... )
  makeKey("footnotesize|tiny|scriptsize|small|large|Large|LARGE|huge|Huge", KeyInfo(KeyInfo::isSize, 0, false), isPatternString);

  // Survives, like known character
  // makeKey("lyx|LyX|latex|LaTeX|latexe|LaTeXe|tex|TeX", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("tableofcontents", KeyInfo(KeyInfo::isChar, 0, false), isPatternString);
  makeKey("item|listitem", KeyInfo(KeyInfo::isList, 1, false), isPatternString);

  makeKey("begin|end", KeyInfo(KeyInfo::isMath, 1, false), isPatternString);
  makeKey("[|]", KeyInfo(KeyInfo::isMath, 1, false), isPatternString);
  makeKey("$", KeyInfo(KeyInfo::isMath, 1, false), isPatternString);

  makeKey("par|uldepth|ULdepth|protect|nobreakdash|medskip|relax", KeyInfo(KeyInfo::isStandard, 0, true), isPatternString);
  // Remove RTL/LTR marker
  makeKey("l|r|textlr|textfr|textar|beginl|endl", KeyInfo(KeyInfo::isStandard, 0, true), isPatternString);
  makeKey("lettrine", KeyInfo(KeyInfo::cleanToStart, 0, true), isPatternString);
  makeKey("lyxslide", KeyInfo(KeyInfo::isSectioning, 1, true), isPatternString);
  makeKey("endarguments", KeyInfo(KeyInfo::endArguments, 0, true), isPatternString);
  makeKey("twocolumn", KeyInfo(KeyInfo::removeWithArg, 2, true), isPatternString);
  makeKey("tnotetext|ead|fntext|cortext|address", KeyInfo(KeyInfo::removeWithArg, 0, true), isPatternString);
  makeKey("lyxend", KeyInfo(KeyInfo::isStandard, 0, true), isPatternString);
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
  return s;
}
#endif

void Intervall::output(ostringstream &os, int lastpos)
{
  // get number of chars to output
  int idx = 0;                          /* int intervalls */
  int i = 0;
  int printed = 0;
  string startTitle = titleValue;
  for (idx = 0; idx <= ignoreidx; idx++) {
    if (i < lastpos) {
      if (i <= borders[idx].low) {
        int printsize;
        if (borders[idx].low > lastpos)
          printsize = lastpos - i;
        else
          printsize = borders[idx].low - i;
        if (printsize > 0) {
          os << startTitle << par.substr(i, printsize);
          i += printsize;
          printed += printsize;
          startTitle = "";
        }
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
    os << startTitle << par.substr(i, lastpos-i);
    printed += lastpos-i;
  }
  handleParentheses(lastpos, false);
  int startindex;
  if (keys["foreignlanguage"].disabled)
    startindex = actualdeptindex-langcount;
  else
    startindex = actualdeptindex;
  for (int i = startindex; i > 0; --i) {
    os << "}";
  }
  if (hasTitle && (printed > 0))
    os << "}";
  if (! isPatternString_)
    os << "\n";
  handleParentheses(lastpos, true); /* extra closings '}' allowed here */
}

void LatexInfo::processRegion(int start, int region_end)
{
  while (start < region_end) {          /* Let {[} and {]} survive */
    int cnt = interval_.isOpeningPar(start);
    if (cnt == 1) {
      // Closing is allowed past the region
      int closing = interval_.findclosing(start+1, interval_.par.length());
      interval_.addIntervall(start, start+1);
      interval_.addIntervall(closing, closing+1);
    }
    else if (cnt == 3)
      start += 2;
    start = interval_.nextNotIgnored(start+1);
  }
}

void LatexInfo::removeHead(KeyInfo const & actual, int count)
{
  if (actual.parenthesiscount == 0) {
    // "{\tiny{} ...}" ==> "{{} ...}"
    interval_.addIntervall(actual._tokenstart-count, actual._tokenstart + actual._tokensize);
  }
  else {
    // Remove header hull, that is "\url{abcd}" ==> "abcd"
    interval_.addIntervall(actual._tokenstart - count, actual._dataStart);
    interval_.addIntervall(actual._dataEnd, actual._dataEnd+1);
  }
}

int LatexInfo::dispatch(ostringstream &os, int previousStart, KeyInfo &actual)
{
  int nextKeyIdx = 0;
  switch (actual.keytype)
  {
    case KeyInfo::isTitle: {
      removeHead(actual);
      nextKeyIdx = getNextKey();
      break;
    }
    case KeyInfo::cleanToStart: {
      actual._dataEnd = actual._dataStart;
      nextKeyIdx = getNextKey();
      // Search for end of arguments
      int tmpIdx = find(nextKeyIdx, KeyInfo::endArguments);
      if (tmpIdx > 0) {
        for (int i = nextKeyIdx; i <= tmpIdx; i++) {
          entries_[i].disabled = true;
        }
        actual._dataEnd = entries_[tmpIdx]._dataEnd;
      }
      while (interval_.par[actual._dataEnd] == ' ')
        actual._dataEnd++;
      interval_.addIntervall(0, actual._dataEnd+1);
      interval_.actualdeptindex = 0;
      interval_.depts[0] = actual._dataEnd+1;
      interval_.closes[0] = -1;
      break;
    }
    case KeyInfo::isText:
      interval_.par[actual._tokenstart] = '#';
      //interval_.addIntervall(actual._tokenstart, actual._tokenstart+1);
      nextKeyIdx = getNextKey();
      break;
    case KeyInfo::noContent: {          /* char like "\hspace{2cm}" */
      if (actual.disabled)
        interval_.addIntervall(actual._tokenstart, actual._dataEnd);
      else
        interval_.addIntervall(actual._dataStart, actual._dataEnd);
    }
      // fall through
    case KeyInfo::isChar: {
      nextKeyIdx = getNextKey();
      break;
    }
    case KeyInfo::isSize: {
      if (actual.disabled || (interval_.par[actual._dataStart] != '{') || (interval_.par[actual._dataStart-1] == ' ')) {
        if (actual.parenthesiscount == 0)
          interval_.addIntervall(actual._tokenstart, actual._dataEnd);
        else {
          interval_.addIntervall(actual._tokenstart, actual._dataEnd+1);
        }
        nextKeyIdx = getNextKey();
      } else {
        // Here _dataStart points to '{', so correct it
        actual._dataStart += 1;
        actual._tokensize += 1;
        actual.parenthesiscount = 1;
        if (interval_.par[actual._dataStart] == '}') {
          // Determine the end if used like '{\tiny{}...}'
          actual._dataEnd = interval_.findclosing(actual._dataStart+1, interval_.par.length()) + 1;
          interval_.addIntervall(actual._dataStart, actual._dataStart+1);
        }
        else {
          // Determine the end if used like '\tiny{...}'
          actual._dataEnd = interval_.findclosing(actual._dataStart, interval_.par.length()) + 1;
        }
        // Split on this key if not at start
        int start = interval_.nextNotIgnored(previousStart);
        if (start < actual._tokenstart) {
          interval_.output(os, actual._tokenstart);
          interval_.addIntervall(start, actual._tokenstart);
        }
        // discard entry if at end of actual
        nextKeyIdx = process(os, actual);
      }
      break;
    }
    case KeyInfo::endArguments: {
      // Remove trailing '{}' too
      actual._dataStart += 1;
      actual._dataEnd += 1;
      interval_.addIntervall(actual._tokenstart, actual._dataEnd+1);
      nextKeyIdx = getNextKey();
      break;
    }
    case KeyInfo::noMain:
      // fall through
    case KeyInfo::isStandard: {
      if (actual.disabled) {
        removeHead(actual);
        processRegion(actual._dataStart, actual._dataStart+1);
        nextKeyIdx = getNextKey();
      } else {
        // Split on this key if not at datastart of calling entry
        int start = interval_.nextNotIgnored(previousStart);
        if (start < actual._tokenstart) {
          interval_.output(os, actual._tokenstart);
          interval_.addIntervall(start, actual._tokenstart);
        }
        // discard entry if at end of actual
        nextKeyIdx = process(os, actual);
      }
      break;
    }
    case KeyInfo::removeWithArg: {
      nextKeyIdx = getNextKey();
      // Search for end of arguments
      int tmpIdx = find(nextKeyIdx, KeyInfo::endArguments);
      if (tmpIdx > 0) {
        for (int i = nextKeyIdx; i <= tmpIdx; i++) {
          entries_[i].disabled = true;
        }
        actual._dataEnd = entries_[tmpIdx]._dataEnd;
      }
      interval_.addIntervall(actual._tokenstart, actual._dataEnd+1);
      break;
    }
    case KeyInfo::doRemove: {
      // Remove the key with all parameters and following spaces
      size_t pos;
      size_t start;
      if (interval_.par[actual._dataEnd-1] == ' ')
        start = actual._dataEnd;
      else
        start = actual._dataEnd+1;
      for (pos = start; pos < interval_.par.length(); pos++) {
        if ((interval_.par[pos] != ' ') && (interval_.par[pos] != '%'))
          break;
      }
      // Remove also enclosing parentheses [] and {}
      int numpars = 0;
      int spaces = 0;
      while (actual._tokenstart > numpars) {
        if (pos+numpars >= interval_.par.size())
          break;
        else if (interval_.par[pos+numpars] == ']' && interval_.par[actual._tokenstart-numpars-1] == '[')
          numpars++;
        else if (interval_.par[pos+numpars] == '}' && interval_.par[actual._tokenstart-numpars-1] == '{')
          numpars++;
        else
          break;
      }
      if (numpars > 0) {
        if (interval_.par[pos+numpars] == ' ')
          spaces++;
      }

      interval_.addIntervall(actual._tokenstart-numpars, pos+numpars+spaces);
      nextKeyIdx = getNextKey();
      break;
    }
    case KeyInfo::isList: {
      // Discard space before _tokenstart
      int count;
      for (count = 0; count < actual._tokenstart; count++) {
        if (interval_.par[actual._tokenstart-count-1] != ' ')
          break;
      }
      nextKeyIdx = getNextKey();
      int tmpIdx = find(nextKeyIdx, KeyInfo::endArguments);
      if (tmpIdx > 0) {
        // Special case: \item is not a list, but a command (like in Style Author_Biography in maa-monthly.layout)
        // with arguments
        // How else can we catch this one?
        for (int i = nextKeyIdx; i <= tmpIdx; i++) {
          entries_[i].disabled = true;
        }
        actual._dataEnd = entries_[tmpIdx]._dataEnd;
      }
      else if (nextKeyIdx > 0) {
        // Ignore any lang entries inside data region
        for (int i = nextKeyIdx; i < int(entries_.size()) && entries_[i]._tokenstart < actual._dataEnd; i++) {
          if (entries_[i].keytype == KeyInfo::isMain)
            entries_[i].disabled = true;
        }
      }
      if (actual.disabled) {
        interval_.addIntervall(actual._tokenstart-count, actual._dataEnd+1);
      }
      else {
        interval_.addIntervall(actual._tokenstart-count, actual._tokenstart);
      }
      if (interval_.par[actual._dataEnd+1] == '[') {
        int posdown = interval_.findclosing(actual._dataEnd+2, interval_.par.length(), '[', ']');
        if ((interval_.par[actual._dataEnd+2] == '{') &&
            (interval_.par[posdown-1] == '}')) {
          interval_.addIntervall(actual._dataEnd+1,actual._dataEnd+3);
          interval_.addIntervall(posdown-1, posdown+1);
        }
        else {
          interval_.addIntervall(actual._dataEnd+1, actual._dataEnd+2);
          interval_.addIntervall(posdown, posdown+1);
        }
        int blk = interval_.nextNotIgnored(actual._dataEnd+1);
        if (blk > posdown) {
          // Discard at most 1 space after empty item
          int count;
          for (count = 0; count < 1; count++) {
            if (interval_.par[blk+count] != ' ')
              break;
          }
          if (count > 0)
            interval_.addIntervall(blk, blk+count);
        }
      }
      break;
    }
    case KeyInfo::isSectioning: {
      // Discard spaces before _tokenstart
      int count;
      int val = actual._tokenstart;
      for (count = 0; count < actual._tokenstart;) {
        val = interval_.previousNotIgnored(val-1);
        if (val < 0 || interval_.par[val] != ' ')
          break;
        else {
          count = actual._tokenstart - val;
        }
      }
      if (actual.disabled) {
        removeHead(actual, count);
        nextKeyIdx = getNextKey();
      } else {
        interval_.addIntervall(actual._tokenstart-count, actual._tokenstart);
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
      if (interval_.par.substr(actual._dataStart, 2) == "% ")
        interval_.addIntervall(actual._dataStart, actual._dataStart+2);
      if (actual._tokenstart > 0) {
        int prev = interval_.previousNotIgnored(actual._tokenstart - 1);
        if ((prev >= 0) && interval_.par[prev] == '%')
          interval_.addIntervall(prev, prev+1);
      }
      if (actual.disabled) {
        removeHead(actual);
        interval_.langcount++;
        if ((interval_.par.substr(actual._dataStart, 3) == " \\[") ||
            (interval_.par.substr(actual._dataStart, 8) == " \\begin{")) {
          // Discard also the space before math-equation
          interval_.addIntervall(actual._dataStart, actual._dataStart+1);
        }
        nextKeyIdx = getNextKey();
        // interval.resetOpenedP(actual._dataStart-1);
      }
      else {
        if (actual._tokenstart < 26) {
          // for the first (and maybe dummy) language
          interval_.setForDefaultLang(actual);
        }
        interval_.resetOpenedP(actual._dataStart-1);
      }
      break;
    }
    case KeyInfo::invalid:
    case KeyInfo::headRemove:
      // These two cases cannot happen, already handled
      // fall through
    default: {
      // LYXERR(Debug::INFO, "Unhandled keytype");
      nextKeyIdx = getNextKey();
      break;
    }
  }
  return nextKeyIdx;
}

int LatexInfo::process(ostringstream & os, KeyInfo const & actual )
{
  int end = interval_.nextNotIgnored(actual._dataEnd);
  int oldStart = actual._dataStart;
  int nextKeyIdx = getNextKey();
  while (true) {
    if ((nextKeyIdx < 0) ||
        (entries_[nextKeyIdx]._tokenstart >= actual._dataEnd) ||
        (entries_[nextKeyIdx].keytype == KeyInfo::invalid)) {
      if (oldStart <= end) {
        processRegion(oldStart, end);
        oldStart = end+1;
      }
      break;
    }
    KeyInfo &nextKey = getKeyInfo(nextKeyIdx);

    if ((nextKey.keytype == KeyInfo::isMain) && !nextKey.disabled) {
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
  if (interval_.par.size() > (size_t) end && interval_.par[end] == '}') {
    end += 1;
    // This is the normal case.
    // But if using the firstlanguage, the closing may be missing
  }
  // get minimum of 'end' and  'actual._dataEnd' in case that the nextKey.keytype was 'KeyInfo::isMain'
  int output_end;
  if (actual._dataEnd < end)
    output_end = interval_.nextNotIgnored(actual._dataEnd);
  else if (interval_.par.size() > (size_t) end)
    output_end = interval_.nextNotIgnored(end);
  else
    output_end = interval_.par.size();
  if ((actual.keytype == KeyInfo::isMain) && actual.disabled) {
    interval_.addIntervall(actual._tokenstart, actual._tokenstart+actual._tokensize);
  }
  // Remove possible empty data
  int dstart = interval_.nextNotIgnored(actual._dataStart);
  while (interval_.isOpeningPar(dstart) == 1) {
    interval_.addIntervall(dstart, dstart+1);
    int dend = interval_.findclosing(dstart+1, output_end);
    interval_.addIntervall(dend, dend+1);
    dstart = interval_.nextNotIgnored(dstart+1);
  }
  if (dstart < output_end)
    interval_.output(os, output_end);
  interval_.addIntervall(actual._tokenstart, end);
  return nextKeyIdx;
}

string splitOnKnownMacros(string par, bool isPatternString)
{
  ostringstream os;
  LatexInfo li(par, isPatternString);
  // LYXERR(Debug::INFO, "Berfore split: " << par);
  KeyInfo DummyKey = KeyInfo(KeyInfo::KeyType::isMain, 2, true);
  DummyKey.head = "";
  DummyKey._tokensize = 0;
  DummyKey._dataStart = 0;
  DummyKey._dataEnd = par.length();
  DummyKey.disabled = true;
  int firstkeyIdx = li.getFirstKey();
  string s;
  if (firstkeyIdx >= 0) {
    KeyInfo firstKey = li.getKeyInfo(firstkeyIdx);
    DummyKey._tokenstart = firstKey._tokenstart;
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
        // Fake the last opened parenthesis
        li.setForDefaultLang(firstKey);
        nextkeyIdx = li.process(os, firstKey);
      }
      else {
        if (nextKey.keytype != KeyInfo::isMain) {
          firstKey._dataStart = datastart;
          firstKey._dataEnd = nextKey._dataEnd+1;
          (void) li.setNextKey(nextkeyIdx);
          li.setForDefaultLang(firstKey);
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
      li.setForDefaultLang(firstKey);
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
  // LYXERR(Debug::INFO, "After split: " << s);
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
	if (isPatternString && (parlen > 0) && (par[parlen-1] == '~')) {
		// Happens to be there in case of description or labeling environment
		parlen--;
	}
	string result;
	if (withformat) {
		// Split the latex input into pieces which
		// can be digested by our search engine
		LYXERR(Debug::FIND, "input: \"" << par << "\"");
		result = splitOnKnownMacros(par.substr(0,parlen), isPatternString);
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
				// LYXERR(Debug::INFO, "Identified regex format:" << a);
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
				return "";
			}
		}

	}
	else {
		// LYXERR(Debug::INFO, "No regex formats");
	}
	return result;
}


// Remove trailing closure of math, macros and environments, so to catch parts of them.
static int identifyClosing(string & t)
{
	int open_braces = 0;
	do {
		LYXERR(Debug::FIND, "identifyClosing(): t now is '" << t << "'");
		if (regex_replace(t, t, "(.*[^\\\\])\\$$", "$1"))
			continue;
		if (regex_replace(t, t, "(.*[^\\\\])\\\\\\]$", "$1"))
			continue;
		if (regex_replace(t, t, "(.*[^\\\\])\\\\end\\{[a-zA-Z_]*\\*?\\}$", "$1"))
			continue;
		if (regex_replace(t, t, "(.*[^\\\\])\\}$", "$1")) {
			++open_braces;
			continue;
		}
		break;
	} while (true);
	return open_braces;
}

static int num_replaced = 0;
static bool previous_single_replace = true;

void MatchStringAdv::CreateRegexp(FindAndReplaceOptions const & opt, string regexp_str, string regexp2_str, string par_as_string)
{
#if QTSEARCH
	// Handle \w properly
	QRegularExpression::PatternOptions popts = QRegularExpression::UseUnicodePropertiesOption | QRegularExpression::MultilineOption;
	if (! opt.casesensitive) {
		popts |= QRegularExpression::CaseInsensitiveOption;
	}
	regexp = QRegularExpression(QString::fromStdString(regexp_str), popts);
	regexp2 = QRegularExpression(QString::fromStdString(regexp2_str), popts);
	regexError = "";
	if (regexp.isValid() && regexp2.isValid()) {
		regexIsValid = true;
		// Check '{', '}' pairs inside the regex
		int balanced = 0;
		int skip = 1;
		for (unsigned i = 0; i < par_as_string.size(); i+= skip) {
			char c = par_as_string[i];
			if (c == '\\') {
				skip = 2;
				continue;
			}
			if (c == '{')
				balanced++;
			else if (c == '}') {
				balanced--;
				if (balanced < 0)
					break;
				}
				skip = 1;
			}
		if (balanced != 0) {
			regexIsValid = false;
			regexError = "Unbalanced curly brackets in regexp \"" + regexp_str + "\"";
		}
	}
	else {
		regexIsValid = false;
		if (!regexp.isValid())
			regexError += "Invalid regexp \"" + regexp_str + "\", error = " + regexp.errorString().toStdString();
		else
			regexError += "Invalid regexp2 \"" + regexp2_str + "\", error = " + regexp2.errorString().toStdString();
	}
#else
	if (opt.casesensitive) {
		regexp = regex(regexp_str);
		regexp2 = regex(regexp2_str);
	}
	else {
		regexp = regex(regexp_str, std::regex_constants::icase);
		regexp2 = regex(regexp2_str, std::regex_constants::icase);
	}
#endif
}

static void modifyRegexForMatchWord(string &t)
{
	string s("");
	regex wordre("(\\\\)*((\\.|\\\\b))");
	size_t lastpos = 0;
	smatch sub;
	for (sregex_iterator it(t.begin(), t.end(), wordre), end; it != end; ++it) {
		sub = *it;
		if ((sub.position(2) - sub.position(0)) % 2 == 1) {
			continue;
		}
		else if (sub.str(2) == "\\\\b")
			return;
		if (lastpos < (size_t) sub.position(2))
			s += t.substr(lastpos, sub.position(2) - lastpos);
		s += "\\S";
		lastpos = sub.position(2) + sub.length(2);
	}
	if (lastpos == 0) {
		s = "\\b" + t + "\\b";
		t = s;
		return;
	}
	else if (lastpos < t.length())
		s += t.substr(lastpos, t.length() - lastpos);
      t = "\\b" + s + "\\b";
}

MatchStringAdv::MatchStringAdv(lyx::Buffer & buf, FindAndReplaceOptions & opt)
	: p_buf(&buf), p_first_buf(&buf), opt(opt)
{
	Buffer & find_buf = *theBufferList().getBuffer(FileName(to_utf8(opt.find_buf_name)), true);
	docstring const & ds = stringifySearchBuffer(find_buf, opt);
	use_regexp = lyx::to_utf8(ds).find("\\regexp{") != std::string::npos;
	if (opt.replace_all && previous_single_replace) {
		previous_single_replace = false;
		num_replaced = 0;
	}
	else if (!opt.replace_all) {
		num_replaced = 0;	// count number of replaced strings
		previous_single_replace = true;
	}
	// When using regexp, braces are hacked already by escape_for_regex()
	par_as_string = normalize(ds);
	open_braces = 0;
	close_wildcards = 0;

	size_t lead_size = 0;
	// correct the language settings
	par_as_string = correctlanguagesetting(par_as_string, true, !opt.ignoreformat);
	opt.matchstart = false;
	if (!use_regexp) {
		identifyClosing(par_as_string); // Removes math closings ($, ], ...) at end of string
		if (opt.ignoreformat) {
			lead_size = 0;
		}
		else {
			lead_size = identifyLeading(par_as_string);
		}
		lead_as_string = par_as_string.substr(0, lead_size);
		string lead_as_regex_string = string2regex(lead_as_string);
		par_as_string_nolead = par_as_string.substr(lead_size, par_as_string.size() - lead_size);
		string par_as_regex_string_nolead = string2regex(par_as_string_nolead);
		/* Handle whole words too in this case
		*/
		if (opt.matchword) {
			par_as_regex_string_nolead = "\\b" + par_as_regex_string_nolead + "\\b";
			opt.matchword = false;
		}
		string regexp_str = "(" + lead_as_regex_string + ")()" + par_as_regex_string_nolead;
		string regexp2_str = "(" + lead_as_regex_string + ")(.*?)" + par_as_regex_string_nolead;
		CreateRegexp(opt, regexp_str, regexp2_str);
		use_regexp = true;
		LYXERR(Debug::FIND, "Setting regexp to : '" << regexp_str << "'");
		LYXERR(Debug::FIND, "Setting regexp2 to: '" << regexp2_str << "'");
		return;
	}

	if (!opt.ignoreformat) {
		lead_size = identifyLeading(par_as_string);
		LYXERR(Debug::FIND, "Lead_size: " << lead_size);
		lead_as_string = par_as_string.substr(0, lead_size);
		par_as_string_nolead = par_as_string.substr(lead_size, par_as_string.size() - lead_size);
	}

	// Here we are using regexp
	LASSERT(use_regexp, /**/);
	{
		string lead_as_regexp;
		if (lead_size > 0) {
			lead_as_regexp = string2regex(par_as_string.substr(0, lead_size));
			regex_replace(par_as_string_nolead, par_as_string_nolead, "}$", "");
			par_as_string = par_as_string_nolead;
			LYXERR(Debug::FIND, "lead_as_regexp is '" << lead_as_regexp << "'");
			LYXERR(Debug::FIND, "par_as_string now is '" << par_as_string << "'");
		}
		LYXERR(Debug::FIND, "par_as_string before escape_for_regex() is '" << par_as_string << "'");
		par_as_string = escape_for_regex(par_as_string, !opt.ignoreformat);
		// Insert (.*?) before trailing closure of math, macros and environments, so to catch parts of them.
		LYXERR(Debug::FIND, "par_as_string now is '" << par_as_string << "'");
		LYXERR(Debug::FIND, "par_as_string after correctRegex is '" << par_as_string << "'");
		++close_wildcards;
		size_t lng = par_as_string.size();
		if (!opt.ignoreformat) {
			// Remove extra '\}' at end if not part of \{\.\}
			while(lng > 2) {
				if (par_as_string.substr(lng-2, 2).compare("\\}") == 0) {
					if (lng >= 6) {
						if (par_as_string.substr(lng-6,3).compare("\\{\\") == 0)
							break;
					}
					lng -= 2;
					open_braces++;
				}
				else
					break;
			}
			if (lng < par_as_string.size())
				par_as_string = par_as_string.substr(0,lng);
		}
		if ((lng > 0) && (par_as_string[0] == '^')) {
			par_as_string = par_as_string.substr(1);
			--lng;
			opt.matchstart = true;
		}
		LYXERR(Debug::FIND, "par_as_string now is '" << par_as_string << "'");
		LYXERR(Debug::FIND, "Open braces: " << open_braces);
		LYXERR(Debug::FIND, "Replaced text (to be used as regex): " << par_as_string);

		// If entered regexp must match at begin of searched string buffer
		// Kornel: Added parentheses to use $1 for size of the leading string
		string regexp_str;
		string regexp2_str;
		{
			// TODO: Adapt '\[12345678]' in par_as_string to acount for the first '()
			// Unfortunately is '\1', '\2', etc not working for strings with extra format
			// so the convert has no effect in that case
			for (int i = 7; i > 0; --i) {
				string orig = "\\\\" + std::to_string(i);
				string dest = "\\" + std::to_string(i+2);
				while (regex_replace(par_as_string, par_as_string, orig, dest));
			}
			if (opt.matchword) {
				modifyRegexForMatchWord(par_as_string);
				opt.matchword = false;
			}
			regexp_str = "(" + lead_as_regexp + ")()" + par_as_string;
			regexp2_str = "(" + lead_as_regexp + ")(.*?)" + par_as_string;
		}
		LYXERR(Debug::FIND, "Setting regexp to : '" << regexp_str << "'");
		LYXERR(Debug::FIND, "Setting regexp2 to: '" << regexp2_str << "'");
		CreateRegexp(opt, regexp_str, regexp2_str, par_as_string);
	}
}

#if 0
// Count number of characters in string
// {]} ==> 1
// \&  ==> 1
// --- ==> 1
// \\[a-zA-Z]+ ==> 1
#if QTSEARCH
static int computeSize(QStringRef s, int len)
#define isLyxAlpha(arg) arg.isLetter()
#else
static int computeSize(string s, int len)
#define isLyxAlpha(arg) isalpha(arg)
#endif
{
	if (len == 0)
		return 0;
	int skip = 1;
	int count = 0;
	for (int i = 0; i < len; i += skip, count++) {
		if (s.at(i) == '\\') {
			skip = 2;
			if (i + 1 < len && isLyxAlpha(s.at(i+1))) {
				for (int j = 2;  i+j < len; j++) {
					if (! isLyxAlpha(s.at(i+j))) {
						if (s.at(i+j) == ' ')
							skip++;
						else if (s.at(i+j) == '{') {
							if (i+j+1 < len && s.at(i+j+1) == '}')
								skip += 2;
							else if (i + j + 1 >= len)
								skip++;
						}
						break;
					}
					skip++;
				}
			}
		}
		else if (s.at(i) == '{') {
			if (i + 1 < len && s.at(i+1) == '}')
				skip = 2;
			else
				skip = 3;
		}
		else if (s.at(i) == '-') {
			if (i+1 < len && s.at(i+1) == '-') {
				if (i + 2 < len && s.at(i+2) == '-')
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
#endif

MatchResult MatchStringAdv::findAux(DocIterator const & cur, int len, bool at_begin) const
{
	MatchResult mres;

	mres.searched_size = len;
	if (at_begin &&
		(opt.restr == FindAndReplaceOptions::R_ONLY_MATHS && !cur.inMathed()) )
		return mres;

	docstring docstr = stringifyFromForSearch(opt, cur, len);
	string str;
	if (use_regexp || opt.casesensitive)
		str = normalize(docstr);
	else
		str = normalize(lowercase(docstr));
	if (!opt.ignoreformat) {
		str = correctlanguagesetting(str, false, !opt.ignoreformat);
		// remove closing '}' and '\n' to allow for use of '$' in regex
		size_t lng = str.size();
		while ((lng > 1) && ((str[lng -1] == '}') || (str[lng -1] == '\n')))
			lng--;
		if (lng != str.size()) {
			str = str.substr(0, lng);
		}
	}
	if (str.empty()) {
		mres.match_len = -1;
		return mres;
	}
	LYXERR(Debug::FIND, "Matching against     '" << lyx::to_utf8(docstr) << "'");
	LYXERR(Debug::FIND, "After normalization: '" << str << "'");

	if (use_regexp) {
		LYXERR(Debug::FIND, "Searching in regexp mode: at_begin=" << at_begin);
#if QTSEARCH
		QString qstr = QString::fromStdString(str);
		QRegularExpression const *p_regexp;
		QRegularExpression::MatchType flags = QRegularExpression::NormalMatch;
		if (at_begin) {
			p_regexp = &regexp;
		} else {
			p_regexp = &regexp2;
		}
		QRegularExpressionMatch match = p_regexp->match(qstr, 0, flags);
		if (!match.hasMatch())
			return mres;
#else
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
			return mres;
		match_results<string::const_iterator> const & m = *re_it;
#endif
		// Whole found string, including the leading
		// std: m[0].second - m[0].first
		// Qt: match.capturedEnd(0) - match.capturedStart(0)
		//
		// Size of the leading string
		// std: m[1].second - m[1].first
		// Qt: match.capturedEnd(1) - match.capturedStart(1)
		int leadingsize = 0;
#if QTSEARCH
		if (match.lastCapturedIndex() > 0) {
			leadingsize = match.capturedEnd(1) - match.capturedStart(1);
		}

#else
		if (m.size() > 2) {
			leadingsize = m[1].second - m[1].first;
		}
#endif
#if QTSEARCH
		mres.match_prefix = match.capturedEnd(2) - match.capturedStart(2);
		mres.match_len = match.capturedEnd(0) - match.capturedEnd(2);
		// because of different number of closing at end of string
		// we have to 'unify' the length of the post-match.
		// Done by ignoring closing parenthesis and linefeeds at string end
		int matchend = match.capturedEnd(0);
		while (mres.match_len > 0) {
		  QChar c = qstr.at(matchend - 1);
		  if ((c == '\n') || (c == '}') || (c == '{')) {
		    mres.match_len--;
		    matchend--;
		  }
		  else
		    break;
		}
		size_t strsize = qstr.size();
		while (strsize > (size_t) match.capturedEnd(0)) {
			QChar c = qstr.at(strsize-1);
			if ((c == '\n') || (c == '}')) {
				--strsize;
			}
			else
				break;
		}
		// LYXERR0(qstr.toStdString());
		mres.match2end = strsize - matchend;
		mres.pos = match.capturedStart(2);
#else
		mres.match_prefix = m[2].second - m[2].first;
		mres.match_len = m[0].second - m[2].second;
		// ignore closing parenthesis and linefeeds at string end
		size_t strend = m[0].second - m[0].first;
		int matchend = strend;
		while (mres.match_len > 0) {
		  char c = str.at(matchend - 1);
		  if ((c == '\n') || (c == '}') || (c == '{')) {
		    mres.match_len--;
		    matchend--;
		  }
		  else
		    break;
		}
		size_t strsize = str.size();
		while (strsize > strend) {
			if ((str.at(strsize-1) == '}') || (str.at(strsize-1) == '\n')) {
				--strsize;
			}
			else
				break;
		}
		// LYXERR0(str);
		mres.match2end = strsize - matchend;
		mres.pos = m[2].first - m[0].first;;
#endif
		if (mres.match2end < 0)
		  mres.match_len = 0;
		mres.leadsize = leadingsize;
#if QTSEARCH
		if (mres.match_len > 0) {
		  string a0 = match.captured(0).mid(mres.pos + mres.match_prefix, mres.match_len).toStdString();
		  mres.result.push_back(a0);
		  for (int i = 3; i <= match.lastCapturedIndex(); i++) {
		    mres.result.push_back(match.captured(i).toStdString());
		  }
		}
#else
		if (mres.match_len > 0) {
		  string a0 = m[0].str().substr(mres.pos + mres.match_prefix, mres.match_len);
		  mres.result.push_back(a0);
		  for (size_t i = 3; i < m.size(); i++) {
		    mres.result.push_back(m[i]);
		  }
		}
#endif
		return mres;
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
		if (str.substr(0, par_as_string.size()) == par_as_string) {
			mres.match_len = par_as_string.size();
			mres.match2end = str.size();
			mres.pos = 0;
			return mres;
		}
	} else {
		// Start the search _after_ the leading part
		size_t pos = str.find(par_as_string_nolead, lead_as_string.size());
		if (pos != string::npos) {
			mres.match_len = par_as_string.size();
			mres.match2end = str.size() - pos;
			mres.pos = pos;
			return mres;
		}
	}
	return mres;
}


MatchResult MatchStringAdv::operator()(DocIterator const & cur, int len, bool at_begin) const
{
	MatchResult mres = findAux(cur, len, at_begin);
	int res = mres.match_len;
	LYXERR(Debug::FIND,
	       "res=" << res << ", at_begin=" << at_begin
	       << ", matchstart=" << opt.matchstart
	       << ", inTexted=" << cur.inTexted());
	if (opt.matchstart) {
		if (cur.pos() != 0)
			mres.match_len = 0;
		else if (mres.match_prefix > 0)
			mres.match_len = 0;
		return mres;
	}
	else
		return mres;
}

#if 0
static bool simple_replace(string &t, string from, string to)
{
  regex repl("(\\\\)*(" + from + ")");
  string s("");
  size_t lastpos = 0;
  smatch sub;
  for (sregex_iterator it(t.begin(), t.end(), repl), end; it != end; ++it) {
    sub = *it;
    if ((sub.position(2) - sub.position(0)) % 2 == 1)
      continue;
    if (lastpos < (size_t) sub.position(2))
      s += t.substr(lastpos, sub.position(2) - lastpos);
    s += to;
    lastpos = sub.position(2) + sub.length(2);
  }
  if (lastpos == 0)
    return false;
  else if (lastpos < t.length())
    s += t.substr(lastpos, t.length() - lastpos);
  t = s;
  return true;
}
#endif

string MatchStringAdv::normalize(docstring const & s) const
{
	string t;
	t = lyx::to_utf8(s);
	// Remove \n at begin
	while (!t.empty() && t[0] == '\n')
		t = t.substr(1);
	// Remove \n at end
	while (!t.empty() && t[t.size() - 1] == '\n')
		t = t.substr(0, t.size() - 1);
	size_t pos;
	// Handle all other '\n'
	while ((pos = t.find("\n")) != string::npos) {
		if (pos > 1 && t[pos-1] == '\\' && t[pos-2] == '\\' ) {
			// Handle '\\\n'
			if (isAlnumASCII(t[pos+1])) {
				t.replace(pos-2, 3, " ");
			}
			else {
				t.replace(pos-2, 3, "");
			}
		}
		else if (!isAlnumASCII(t[pos+1]) || !isAlnumASCII(t[pos-1])) {
			// '\n' adjacent to non-alpha-numerics, discard
			t.replace(pos, 1, "");
		}
		else {
			// Replace all other \n with spaces
			t.replace(pos, 1, " ");
		}
	}
	// Remove stale empty \emph{}, \textbf{} and similar blocks from latexify
	// Kornel: Added textsl, textsf, textit, texttt and noun
	// + allow to seach for colored text too
	LYXERR(Debug::FIND, "Removing stale empty \\emph{}, \\textbf{}, \\*section{} macros from: " << t);
	while (regex_replace(t, t, "\\\\(emph|noun|text(bf|sl|sf|it|tt)|(u|uu)line|(s|x)out|uwave)(\\{(\\{\\})?\\})+", ""))
		LYXERR(Debug::FIND, "  further removing stale empty \\emph{}, \\textbf{} macros from: " << t);
	while (regex_replace(t, t, "\\\\((sub)?(((sub)?section)|paragraph)|part)\\*?(\\{(\\{\\})?\\})+", ""))
		LYXERR(Debug::FIND, "  further removing stale empty \\emph{}, \\textbf{} macros from: " << t);
	while (regex_replace(t, t, "\\\\(foreignlanguage|textcolor|item)\\{[a-z]+\\}(\\{(\\{\\})?\\})+", ""));

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
		// OutputParams runparams(&cur.buffer()->params().encoding());
		OutputParams runparams(encodings.fromLyXName("utf8"));
		runparams.nice = true;
		runparams.flavor = Flavor::XeTeX;
		runparams.linelen = 10000; //lyxrc.plaintext_linelen;
		// No side effect of file copying and image conversion
		runparams.dryrun = true;
		runparams.for_search = true;
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
	//OutputParams runparams(&buf.params().encoding());
	OutputParams runparams(encodings.fromLyXName("utf8"));
	runparams.nice = false;
	runparams.flavor = Flavor::XeTeX;
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
				TeXMathStream ws(os);
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
				TeXMathStream ws(os);
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

#if defined(ResultsDebug)
// Debugging output
static void displayMResult(MatchResult &mres, string from, DocIterator & cur)
{
	LYXERR0( "from:\t\t\t" << from);
	string status;
	if (mres.pos_len > 0) {
		// Set in finalize
		status = "FINALSEARCH";
	}
	else {
		if (mres.match_len > 0) {
			if ((mres.match_prefix == 0) && (mres.pos == mres.leadsize))
				status = "Good Match";
			else
				status = "Matched in";
		}
		else
			status = "MissedSearch";
	}

	LYXERR0( status << "(" << cur.pos() << " ... " << mres.searched_size + cur.pos() << ") cur.lastpos(" << cur.lastpos() << ")");
	if ((mres.leadsize > 0) || (mres.match_len > 0) || (mres.match2end > 0))
		LYXERR0( "leadsize(" << mres.leadsize << ") match_len(" << mres.match_len << ") match2end(" << mres.match2end << ")");
	if ((mres.pos > 0) || (mres.match_prefix > 0))
		LYXERR0( "pos(" << mres.pos << ") match_prefix(" << mres.match_prefix << ")");
	for (size_t i = 0; i < mres.result.size(); i++)
		LYXERR0( "Match " << i << " = \"" << mres.result[i] << "\"");
}
	#define displayMres(s, txt, cur) displayMResult(s, txt, cur);
#else
	#define displayMres(s, txt, cur)
#endif

/** Finalize an advanced find operation, advancing the cursor to the innermost
 ** position that matches, plus computing the length of the matching text to
 ** be selected
 ** Return the cur.pos() difference between start and end of found match
 **/
MatchResult &findAdvFinalize(DocIterator & cur, MatchStringAdv const & match, MatchResult const & expected = MatchResult(-1))
{
	// Search the foremost position that matches (avoids find of entire math
	// inset when match at start of it)
	DocIterator old_cur(cur.buffer());
	MatchResult mres;
	static MatchResult fail = MatchResult();
	static MatchResult max_match;
	// If (prefix_len > 0) means that forwarding 1 position will remove the complete entry
	// Happens with e.g. hyperlinks
	// either one sees "http://www.bla.bla" or nothing
	// so the search for "www" gives prefix_len = 7 (== sizeof("http://")
	// and although we search for only 3 chars, we find the whole hyperlink inset
	bool at_begin = (expected.match_prefix == 0);
	LASSERT(at_begin, /**/);
	if (expected.match_len > 0 && at_begin) {
		// Search for deepest match
		old_cur = cur;
		max_match = expected;
		do {
			size_t d = cur.depth();
			cur.forwardPos();
			if (!cur)
				break;
			if (cur.depth() < d)
				break;
			if (cur.depth() == d)
				break;
			size_t lastd = d;
			while (cur && cur.depth() > lastd) {
				lastd = cur.depth();
				mres = match(cur, -1, at_begin);
				displayMres(mres, "Checking innermost", cur);
				if (mres.match_len > 0)
					break;
				// maybe deeper?
				cur.forwardPos();
			}
			if (mres.match_len < expected.match_len)
				break;
			max_match = mres;
			old_cur = cur;;
		} while(1);
		cur = old_cur;
	}
	else {
		// (expected.match_len <= 0)
		mres = match(cur);      /* match valid only if not searching whole words */
		displayMres(mres, "Start with negative match", cur);
		max_match = mres;
	}
	if (max_match.match_len <= 0) return fail;
	LYXERR(Debug::FIND, "Ok");

	// Compute the match length
	int len = 1;
	if (cur.pos() + len > cur.lastpos())
	  return fail;
	// regexp should use \w+, \S+, or \b(some string)\b
	// to search for whole words
	if (match.opt.matchword && !match.use_regexp) {
	  LYXERR(Debug::FIND, "verifying unmatch with len = " << len);
	  while (cur.pos() + len <= cur.lastpos() && match(cur, len).match_len <= 0) {
	    ++len;
	    LYXERR(Debug::FIND, "verifying unmatch with len = " << len);
	  }
	  // Length of matched text (different from len param)
	  static MatchResult old_match = match(cur, len, at_begin);
	  if (old_match.match_len < 0)
	    old_match = fail;
	  MatchResult new_match;
	  // Greedy behaviour while matching regexps
	  while ((new_match = match(cur, len + 1, at_begin)).match_len > old_match.match_len) {
	    ++len;
	    old_match = new_match;
	    LYXERR(Debug::FIND, "verifying   match with len = " << len);
	  }
	  displayMres(old_match, "SEARCH RESULT", cur)
	  return old_match;
	}
	else {
          int minl = 1;
          int maxl = cur.lastpos() - cur.pos();
          // Greedy behaviour while matching regexps
          while (maxl > minl) {
            MatchResult mres2;
            mres2 = match(cur, len, at_begin);
            displayMres(mres2, "Finalize loop", cur);
            int actual_match_len = mres2.match_len;
            if (actual_match_len >= max_match.match_len) {
              // actual_match_len > max_match _can_ happen,
              // if the search area splits
              // some following word so that the regex
              // (e.g. 'r.*r\b' matches 'r' from the middle of the
              // splitted word)
              // This means, the len value is too big
	      actual_match_len = max_match.match_len;
	      max_match = mres2;
	      max_match.match_len = actual_match_len;
              maxl = len;
              if (maxl - minl < 4)
                len = (int)((maxl + minl)/2);
              else
                len = (int)(minl + (maxl - minl + 3)/4);
            }
            else {
              // (actual_match_len < max_match.match_len)
              minl = len + 1;
              len = (int)((maxl + minl)/2);
            }
          }
	  len = minl;
          old_cur = cur;
          // Search for real start of matched characters
          while (len > 1) {
            MatchResult actual_match;
            do {
              cur.forwardPos();
            } while (cur.depth() > old_cur.depth()); /* Skip inner insets */
            if (cur.depth() < old_cur.depth()) {
              // Outer inset?
              LYXERR(Debug::INFO, "cur.depth() < old_cur.depth(), this should never happen");
              break;
            }
            if (cur.pos() != old_cur.pos()) {
              // OK, forwarded 1 pos in actual inset
              actual_match = match(cur, len-1, at_begin);
              if (actual_match.match_len == max_match.match_len) {
                // Ha, got it! The shorter selection has the same match length
                len--;
                old_cur = cur;
		max_match = actual_match;
              }
              else {
                // OK, the shorter selection matches less chars, revert to previous value
                cur = old_cur;
                break;
              }
            }
            else {
              LYXERR(Debug::INFO, "cur.pos() == old_cur.pos(), this should never happen");
              actual_match = match(cur, len, at_begin);
              if (actual_match.match_len == max_match.match_len) {
                old_cur = cur;
                max_match = actual_match;
              }
            }
          }
          if (len == 0)
            return fail;
          else {
            max_match.pos_len = len;
	    displayMres(max_match, "SEARCH RESULT", cur)
            return max_match;
          }
        }
}

/// Finds forward
int findForwardAdv(DocIterator & cur, MatchStringAdv & match)
{
	if (!cur)
		return 0;
	bool repeat = false;
	while (!theApp()->longOperationCancelled() && cur) {
		//(void) findAdvForwardInnermost(cur);
		LYXERR(Debug::FIND, "findForwardAdv() cur: " << cur);
		MatchResult mres = match(cur, -1, false);
		string msg = "Starting";
		if (repeat)
			msg = "Repeated";
		displayMres(mres, msg + " findForwardAdv", cur)
		int match_len = mres.match_len;
		if ((mres.pos > 100000) || (mres.match2end > 100000) || (match_len > 100000)) {
			LYXERR(Debug::INFO, "BIG LENGTHS: " << mres.pos << ", " << match_len << ", " << mres.match2end);
			match_len = 0;
		}
		if (match_len <= 0) {
			// This should exit nested insets, if any, or otherwise undefine the currsor.
			cur.pos() = cur.lastpos();
			LYXERR(Debug::FIND, "Advancing pos: cur=" << cur);
			cur.forwardPos();
		}
		else {	// match_len > 0
			// Try to find the begin of searched string
			int increment;
			int firstInvalid = 100000;
			{
				int incrmatch = (mres.match_prefix + mres.pos - mres.leadsize + 1)*3/4;
				int incrcur = (cur.lastpos() - cur.pos() + 1 )*3/4;
				if (incrcur < incrmatch)
					increment = incrcur;
				else
					increment = incrmatch;
				if (increment < 1)
					increment = 1;
			}
			LYXERR(Debug::FIND, "Set increment to " << increment);
			while (increment > 0) {
				DocIterator old_cur = cur;
				size_t skipping = cur.depth();
				for (int i = 0; i < increment && cur; i++) {
					cur.forwardPos();
					while (cur && cur.depth() > skipping) {
						cur.pos() = cur.lastpos();
						cur.forwardPos();
					}
				}
				if (! cur || (cur.pit() > old_cur.pit())) {
					// Are we outside of the paragraph?
					// This can happen if moving past some UTF8-encoded chars
					cur = old_cur;
					increment /= 2;
				}
				else {
					MatchResult mres2 = match(cur, -1, false);
					displayMres(mres2, "findForwardAdv loop", cur)
					switch (interpretMatch(mres, mres2)) {
					case MatchResult::newIsTooFar:
					  // behind the expected match
					  firstInvalid = increment;
					  cur = old_cur;
					  increment /= 2;
					  break;
					case MatchResult::newIsBetter:
					  // not reached yet, but cur.pos()+increment is bettert
					  mres = mres2;
					  firstInvalid -= increment;
					  if (increment > firstInvalid*3/4)
					    increment = firstInvalid*3/4;
					  if ((mres2.pos == mres2.leadsize) && (increment >= mres2.match_prefix)) {
					    if (increment >= mres2.match_prefix)
					      increment = (mres2.match_prefix+1)*3/4;
					  }
					  break;
					default:
					  // Todo@
					  // Handle not like MatchResult::newIsTooFar
					  LYXERR0( "Something is wrong: Increment = " << increment << " match_prefix = " << mres.match_prefix);
					  firstInvalid--;
					  increment = increment*3/4;
					  cur = old_cur;
					  break;
					}
				}
			}
			if (mres.match_len > 0 && mres.match_prefix + mres.pos - mres.leadsize > 0) {
				repeat = true;
				cur.forwardPos();
				continue;
			}
			// LYXERR0("Leaving first loop");
			LYXERR(Debug::FIND, "Finalizing 1");
			MatchResult found_match = findAdvFinalize(cur, match, mres);
			if (found_match.match_len > 0) {
			  LASSERT(found_match.pos_len > 0, /**/);
			  match.FillResults(found_match);
			  return found_match.pos_len;
			}
			else {
			  // try next possible match
			  cur.forwardPos();
			  repeat = false;
			  continue;
			}
		}
	}
	return 0;
}


/// Find the most backward consecutive match within same paragraph while searching backwards.
MatchResult &findMostBackwards(DocIterator & cur, MatchStringAdv const & match)
{
	DocIterator cur_begin = doc_iterator_begin(cur.buffer());
	DocIterator tmp_cur = cur;
	static MatchResult mr = findAdvFinalize(tmp_cur, match, MatchResult(-1));
	Inset & inset = cur.inset();
	for (; cur != cur_begin; cur.backwardPos()) {
		LYXERR(Debug::FIND, "findMostBackwards(): cur=" << cur);
		DocIterator new_cur = cur;
		new_cur.backwardPos();
		if (new_cur == cur || &new_cur.inset() != &inset || !match(new_cur).match_len)
			break;
		MatchResult new_mr = findAdvFinalize(new_cur, match, MatchResult(-1));
		if (new_mr.match_len == mr.match_len)
			break;
		mr = new_mr;
	}
	LYXERR(Debug::FIND, "findMostBackwards(): exiting with cur=" << cur);
	return mr;
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
		bool found_match = (match(cur, -1, false).match_len > 0);

		if (found_match) {
			if (pit_changed)
				cur.pos() = cur.lastpos();
			else
				cur.pos() = cur_orig.pos();
			LYXERR(Debug::FIND, "findBackAdv2: cur: " << cur);
			DocIterator cur_prev_iter;
			do {
				found_match = (match(cur).match_len > 0);
				LYXERR(Debug::FIND, "findBackAdv3: found_match="
				       << found_match << ", cur: " << cur);
				if (found_match) {
					MatchResult found_mr = findMostBackwards(cur, match);
					match.FillResults(found_mr);
					LASSERT(found_mr.pos_len > 0, /**/);
					return found_mr.pos_len;
				}

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
	docstring const & _find_buf_name, bool _casesensitive,
	bool _matchword, bool _forward, bool _expandmacros, bool _ignoreformat,
	docstring const & _repl_buf_name, bool _keep_case,
	SearchScope _scope, SearchRestriction _restr, bool _replace_all)
	: find_buf_name(_find_buf_name), casesensitive(_casesensitive), matchword(_matchword),
	  forward(_forward), expandmacros(_expandmacros), ignoreformat(_ignoreformat),
	  repl_buf_name(_repl_buf_name), keep_case(_keep_case), scope(_scope), restr(_restr), replace_all(_replace_all)
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
	LASSERT(!pit->empty(), /**/);
	pos_type right = pos_type(1);
	pit->changeCase(buffer.params(), pos_type(0), right, first_case);
	right = pit->size();
	pit->changeCase(buffer.params(), pos_type(1), right, others_case);
}
} // namespace

#if 1
static bool replaceMatches(string &t, int maxmatchnum, vector <string> const & replacements)
{
  // Should replace the string "$" + std::to_string(matchnum) with replacement
  // if the char '$' is not prefixed with odd number of char '\\'
  static regex const rematch("(\\\\)*(\\$\\$([0-9]))");
  string s;
  size_t lastpos = 0;
  smatch sub;
  for (sregex_iterator it(t.begin(), t.end(), rematch), end; it != end; ++it) {
    sub = *it;
    if ((sub.position(2) - sub.position(0)) % 2 == 1)
      continue;
    int num = stoi(sub.str(3), nullptr, 10);
    if (num >= maxmatchnum)
      continue;
    if (lastpos < (size_t) sub.position(2))
      s += t.substr(lastpos, sub.position(2) - lastpos);
    s += replacements[num];
    lastpos = sub.position(2) + sub.length(2);
  }
  if (lastpos == 0)
    return false;
  else if (lastpos < t.length())
    s += t.substr(lastpos, t.length() - lastpos);
  t = s;
  return true;
}
#endif

///
static int findAdvReplace(BufferView * bv, FindAndReplaceOptions const & opt, MatchStringAdv & matchAdv)
{
	Cursor & cur = bv->cursor();
	if (opt.repl_buf_name.empty()
	    || theBufferList().getBuffer(FileName(to_utf8(opt.repl_buf_name)), true) == 0
	    || theBufferList().getBuffer(FileName(to_utf8(opt.find_buf_name)), true) == 0)
		return 0;

	DocIterator sel_beg = cur.selectionBegin();
	DocIterator sel_end = cur.selectionEnd();
	if (&sel_beg.inset() != &sel_end.inset()
	    || sel_beg.pit() != sel_end.pit()
	    || sel_beg.idx() != sel_end.idx())
		return 0;
	int sel_len = sel_end.pos() - sel_beg.pos();
	LYXERR(Debug::FIND, "sel_beg: " << sel_beg << ", sel_end: " << sel_end
	       << ", sel_len: " << sel_len << endl);
	if (sel_len == 0)
		return 0;
	LASSERT(sel_len > 0, return 0);

	if (!matchAdv(sel_beg, sel_len).match_len)
		return 0;

	// Build a copy of the replace buffer, adapted to the KeepCase option
	Buffer const & repl_buffer_orig = *theBufferList().getBuffer(FileName(to_utf8(opt.repl_buf_name)), true);
	ostringstream oss;
	repl_buffer_orig.write(oss);
	string lyx = oss.str();
	if (matchAdv.valid_matches > 0) {
	  replaceMatches(lyx, matchAdv.valid_matches, matchAdv.matches);
	}
	Buffer repl_buffer("", false);
	repl_buffer.setUnnamed(true);
	LASSERT(repl_buffer.readString(lyx), return 0);
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
		// OutputParams runparams(&repl_buffer.params().encoding());
		OutputParams runparams(encodings.fromLyXName("utf8"));
		runparams.nice = false;
		runparams.flavor = Flavor::XeTeX;
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
	return 1;
}


/// Perform a FindAdv operation.
bool findAdv(BufferView * bv, FindAndReplaceOptions & opt)
{
	DocIterator cur;
	int pos_len = 0;

	// e.g., when invoking word-findadv from mini-buffer wither with
	//       wrong options syntax or before ever opening advanced F&R pane
	if (theBufferList().getBuffer(FileName(to_utf8(opt.find_buf_name)), true) == 0)
		return false;

	try {
		MatchStringAdv matchAdv(bv->buffer(), opt);
#if QTSEARCH
		if (!matchAdv.regexIsValid) {
			bv->message(lyx::from_utf8(matchAdv.regexError));
			return(false);
		}
#endif
		int length = bv->cursor().selectionEnd().pos() - bv->cursor().selectionBegin().pos();
		if (length > 0)
			bv->putSelectionAt(bv->cursor().selectionBegin(), length, !opt.forward);
		num_replaced += findAdvReplace(bv, opt, matchAdv);
		cur = bv->cursor();
		if (opt.forward)
			pos_len = findForwardAdv(cur, matchAdv);
		else
			pos_len = findBackwardsAdv(cur, matchAdv);
	} catch (exception & ex) {
		bv->message(from_utf8(ex.what()));
		return false;
	}

	if (pos_len == 0) {
		if (num_replaced > 0) {
			switch (num_replaced)
			{
				case 1:
					bv->message(_("One match has been replaced."));
					break;
				case 2:
					bv->message(_("Two matches have been replaced."));
					break;
				default:
					bv->message(bformat(_("%1$d matches have been replaced."), num_replaced));
					break;
			}
			num_replaced = 0;
		}
		else {
			bv->message(_("Match not found."));
		}
		return false;
	}

	if (num_replaced > 0)
		bv->message(_("Match has been replaced."));
	else
		bv->message(_("Match found."));

	if (cur.pos() + pos_len > cur.lastpos()) {
		// Prevent crash in bv->putSelectionAt()
		// Should never happen, maybe LASSERT() here?
		pos_len = cur.lastpos() - cur.pos();
	}
	LYXERR(Debug::FIND, "Putting selection at cur=" << cur << " with len: " << pos_len);
	bv->putSelectionAt(cur, pos_len, !opt.forward);

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
	   << opt.replace_all << ' '
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
	is >> opt.casesensitive >> opt.matchword >> opt.forward >> opt.expandmacros >> opt.ignoreformat >> opt.replace_all;
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
