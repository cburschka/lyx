/**
 * \file lyxfind.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author Jürgen Vigna
 * \author Alfredo Braunstein
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "lyxfind.h"

#include "buffer.h"
#include "cursor.h"
#include "CutAndPaste.h"
#include "BufferView.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "pariterator.h"
#include "undo.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"

#include "support/textutils.h"
#include "support/tostr.h"

#include "support/std_sstream.h"

using lyx::support::lowercase;
using lyx::support::uppercase;
using lyx::support::split;

using lyx::par_type;
using lyx::pos_type;

using std::advance;
using std::ostringstream;
using std::string;


namespace {

bool parse_bool(string & howto)
{
	if (howto.empty())
		return false;
	string var;
	howto = split(howto, var, ' ');
	return (var == "1");
}


class MatchString : public std::binary_function<Paragraph, lyx::pos_type, bool>
{
public:
	MatchString(string const & str, bool cs, bool mw)
		: str(str), cs(cs), mw(mw)
	{}

	// returns true if the specified string is at the specified position
	bool operator()(Paragraph const & par, lyx::pos_type pos) const
	{
		string::size_type const size = str.length();
		lyx::pos_type i = 0;
		lyx::pos_type const parsize = par.size();
		for (i = 0; pos + i < parsize; ++i) {
			if (string::size_type(i) >= size)
				break;
		  if (cs && str[i] != par.getChar(pos + i))
				break;
			if (!cs && uppercase(str[i]) != uppercase(par.getChar(pos + i)))
				break;
		}

		if (size != string::size_type(i))
			return false;

		// if necessary, check whether string matches word
		if (mw) {
			if (pos > 0	&& IsLetterCharOrDigit(par.getChar(pos - 1)))
				return false;
			if (pos + lyx::pos_type(size) < parsize
					&& IsLetterCharOrDigit(par.getChar(pos + size)));
				return false;
		}

		return true;
	}

private:
	// search string
	string str;
	// case sensitive
	bool cs;
	// match whole words only
	bool mw;
};


bool findForward(DocIterator & cur, MatchString const & match)
{
	for (; cur.size(); cur.forwardChar())
		if (match(cur.paragraph(), cur.pos()))
			return true;
	return false;
}


bool findBackwards(DocIterator & cur, MatchString const & match)
{
	for (; cur.size(); cur.backwardChar())
		if (match(cur.paragraph(), cur.pos()))
			return true;
	return false;
}


bool findChange(DocIterator & cur)
{
	for (; cur.size(); cur.forwardChar())
		if ((cur.paragraph().empty() || !cur.empty())
		    && cur.paragraph().lookupChange(cur.pos()) != Change::UNCHANGED)
			return true;
	return false;
}


bool searchAllowed(BufferView * bv, string const & str)
{
	if (str.empty()) {
		Alert::error(_("Search error"), _("Search string is empty"));
		return false;
	}
	return bv->available();
}


bool find(BufferView * bv, string const & searchstr, bool cs, bool mw, bool fw)
{
	if (!searchAllowed(bv, searchstr))
		return false;

	DocIterator cur = bv->cursor();

	MatchString const match(searchstr, cs, mw);

	bool found = fw ? findForward(cur, match) : findBackwards(cur, match);

	if (found)
		bv->putSelectionAt(cur, searchstr.length(), !fw);

	return found;
}


int replaceAll(BufferView * bv,
	       string const & searchstr, string const & replacestr,
	       bool cs, bool mw)
{
	Buffer & buf = *bv->buffer();

	if (!searchAllowed(bv, searchstr) || buf.isReadonly())
		return 0;

	recordUndoFullDocument(bv->cursor());

	MatchString const match(searchstr, cs, mw);
	int num = 0;

	int const rsize = replacestr.size();
	int const ssize = searchstr.size();

	DocIterator cur = DocIterator(buf.inset());
	while (findForward(cur, match)) {
		lyx::pos_type pos = cur.pos();
		LyXFont const font
			= cur.paragraph().getFontSettings(buf.params(), pos);
		int striked = ssize - cur.paragraph().erase(pos, pos + ssize);
		cur.paragraph().insert(pos, replacestr, font);
		for (int i = 0; i < rsize + striked; ++i)
			cur.forwardChar();
		++num;
	}

	bv->text()->init(bv);
	bv->putSelectionAt(DocIterator(buf.inset()), 0, false);
	if (num)
		buf.markDirty();
	return num;
}


bool stringSelected(BufferView * bv, string const & searchstr,
		    bool cs, bool mw, bool fw)
{
	// if nothing selected or selection does not equal search
	// string search and select next occurance and return
	string const & str1 = searchstr;
	string const str2 = bv->cursor().selectionAsString(false);
	if ((cs && str1 != str2) || lowercase(str1) != lowercase(str2)) {
		find(bv, searchstr, cs, mw, fw);
		return false;
	}

	return true;
}


int replace(BufferView * bv, string const & searchstr,
      string const & replacestr, bool cs, bool mw, bool fw)
{
	if (!searchAllowed(bv, searchstr) || bv->buffer()->isReadonly())
		return 0;

	if (!stringSelected(bv, searchstr, cs, mw, fw))
		return 0;

	LCursor & cur = bv->cursor();
	lyx::cap::replaceSelectionWithString(cur, replacestr);
	lyx::cap::setSelectionRange(cur, replacestr.length());
	cur.top() = fw ? cur.selEnd() : cur.selBegin();
	bv->buffer()->markDirty();
	find(bv, searchstr, cs, mw, fw);
	bv->update();

	return 1;
}

} // namespace anon


namespace lyx {
namespace find {

string const find2string(string const & search,
			 bool casesensitive, bool matchword, bool forward)
{
	ostringstream ss;
	ss << search << '\n'
	   << int(casesensitive) << ' '
	   << int(matchword) << ' '
	   << int(forward);
	return ss.str();
}


string const replace2string(string const & search, string const & replace,
			    bool casesensitive, bool matchword,
			    bool all, bool forward)
{
	ostringstream ss;
	ss << search << '\n'
	   << replace << '\n'
	   << int(casesensitive) << ' '
	   << int(matchword) << ' '
	   << int(all) << ' '
	   << int(forward);
	return ss.str();
}


void find(BufferView * bv, FuncRequest const & ev)
{
	if (!bv || ev.action != LFUN_WORD_FIND)
		return;

	lyxerr << "find called, cmd: " << ev << std::endl;

	// data is of the form
	// "<search>
	//  <casesensitive> <matchword> <forward>"
	string search;
	string howto = split(ev.argument, search, '\n');

	bool casesensitive = parse_bool(howto);
	bool matchword     = parse_bool(howto);
	bool forward       = parse_bool(howto);

	bool const found = ::find(bv, search,
				  casesensitive, matchword, forward);

	if (!found)
		bv->owner()->message(_("String not found!"));
}


void replace(BufferView * bv, FuncRequest const & ev)
{
	if (!bv || ev.action != LFUN_WORD_REPLACE)
		return;

	// data is of the form
	// "<search>
	//  <replace>
	//  <casesensitive> <matchword> <all> <forward>"
	string search;
	string replace;
	string howto = split(ev.argument, search, '\n');
	howto = split(howto, replace, '\n');

	bool casesensitive = parse_bool(howto);
	bool matchword     = parse_bool(howto);
	bool all           = parse_bool(howto);
	bool forward       = parse_bool(howto);

	LyXView * lv = bv->owner();

	int const replace_count = all
		? ::replaceAll(bv, search, replace, casesensitive, matchword)
		: ::replace(bv, search, replace, casesensitive, matchword, forward);

	if (replace_count == 0) {
		lv->message(_("String not found!"));
	} else {
		if (replace_count == 1) {
			lv->message(_("String has been replaced."));
		} else {
			string str = tostr(replace_count);
			str += _(" strings have been replaced.");
			lv->message(str);
		}
	}
}


bool findNextChange(BufferView * bv)
{
	if (!bv->available())
		return false;

	DocIterator cur = DocIterator(bv->buffer()->inset());

	if (!findChange(cur))
		return false;

	Paragraph const & par = cur.paragraph();
	pos_type pos = cur.pos();

	Change orig_change = par.lookupChangeFull(pos);
	pos_type parsize = par.size();
	pos_type end = pos;

	for (; end != parsize; ++end) {
		Change change = par.lookupChangeFull(end);
		if (change != orig_change) {
			// slight UI optimisation: for replacements, we get
			// text like : _old_new. Consider that as one change.
			if (!(orig_change.type == Change::DELETED &&
				change.type == Change::INSERTED))
				break;
		}
	}
	pos_type length = end - pos;
	bv->putSelectionAt(cur, length, true);
	return true;
}

} // find namespace
} // lyx namespace


