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
#include "BufferView.h"
#include "debug.h"
#include "iterators.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "PosIterator.h"
#include "undo.h"

#include "frontends/Alert.h"
#include "frontends/LyXView.h"

#include "support/textutils.h"
#include "support/tostr.h"

#include "support/std_sstream.h"

using lyx::support::lowercase;
using lyx::support::uppercase;
using lyx::support::split;

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


bool find(BufferView * bv,
	  string const & searchstr, bool cs, bool mw, bool fw);


int replace(BufferView * bv,
	    string const & searchstr, string const & replacestr,
	    bool cs, bool mw, bool fw);


int replaceAll(BufferView * bv,
	       string const & searchstr, string const & replacestr,
	       bool cs, bool mw);


bool findChange(PosIterator & cur, PosIterator const & end);

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

	// data is of the form
	// "<search>
	//  <casesensitive> <matchword> <forward>"
	string search;
	string howto = split(ev.argument, search, '\n');

	bool casesensitive = parse_bool(howto);
	bool matchword     = parse_bool(howto);
	bool forward       = parse_bool(howto);

	bool const found = ::find(bv, search,
				  forward, casesensitive, matchword);

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

	int const replace_count = all ?
		::replaceAll(bv, search, replace,
			     casesensitive, matchword) :
		::replace(bv, search, replace,
			  casesensitive, matchword, forward);

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

	PosIterator cur = PosIterator(*bv);
	PosIterator const endit = bv->buffer()->pos_iterator_end();

	if (!findChange(cur, endit))
		return false;

	ParagraphList::iterator pit = cur.pit();
	pos_type pos = cur.pos();

	Change orig_change = pit->lookupChangeFull(pos);
	pos_type parsize = pit->size();
	pos_type end = pos;

	for (; end != parsize; ++end) {
		Change change = pit->lookupChangeFull(end);
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


namespace {

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
		while ((pos + i < parsize)
		       && (string::size_type(i) < size)
		       && (cs ? (str[i] == par.getChar(pos + i))
			   : (uppercase(str[i]) == uppercase(par.getChar(pos + i))))) {
			++i;
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


bool findForward(PosIterator & cur, PosIterator const & end,
		 MatchString const & match)
{
	for (; cur != end; ++cur) {
		if (match(*cur.pit(), cur.pos()))
			return true;
	}
	return false;
}


bool findBackwards(PosIterator & cur, PosIterator const & beg,
		   MatchString const & match)
{
	while (beg != cur) {
		--cur;
		if (match(*cur.pit(), cur.pos()))
			return true;
	}
	return false;
}


bool findChange(PosIterator & cur, PosIterator const & end)
{
	for (; cur != end; ++cur) {
		if ((cur.pit()->empty() || !cur.at_end())
		    && cur.pit()->lookupChange(cur.pos()) != Change::UNCHANGED)
			return true;
	}
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

	PosIterator cur = PosIterator(*bv);

	MatchString const match(searchstr, cs, mw);

	PosIterator const end = bv->buffer()->pos_iterator_end();
	PosIterator const beg = bv->buffer()->pos_iterator_begin();

	bool found = fw ? findForward(cur, end, match)
		: findBackwards(cur, beg, match);

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

	recordUndo(Undo::ATOMIC, bv->text(), 0, buf.paragraphs().size() - 1);

	PosIterator cur = buf.pos_iterator_begin();
	PosIterator const end = buf.pos_iterator_end();
	MatchString const match(searchstr, cs, mw);
	int num = 0;

	int const rsize = replacestr.size();
	int const ssize = searchstr.size();

	while (findForward(cur, end, match)) {
		lyx::pos_type pos = cur.pos();
		LyXFont const font
			= cur.pit()->getFontSettings(buf.params(), pos);
		int striked = ssize - cur.pit()->erase(pos, pos + ssize);
		cur.pit()->insert(pos, replacestr, font);
		advance(cur, rsize + striked);
		++num;
	}

	PosIterator beg = buf.pos_iterator_begin();
	bv->text()->init(bv);
	bv->putSelectionAt(beg, 0, false);
	if (num)
		buf.markDirty();
	return num;
}


bool stringSelected(BufferView * bv,
		    string const & searchstr,
		    bool cs, bool mw, bool fw)
{
	LyXText * text = bv->getLyXText();
	// if nothing selected or selection does not equal search
	// string search and select next occurance and return
	string const & str1 = searchstr;
	string const str2 = text->selectionAsString(*bv->buffer(),
						    false);
	if ((cs && str1 != str2) || lowercase(str1) != lowercase(str2)) {
		find(bv, searchstr, cs, mw, fw);
		return false;
	}

	return true;
}


int replace(BufferView * bv,
	    string const & searchstr, string const & replacestr,
	    bool cs, bool mw, bool fw)
{
	if (!searchAllowed(bv, searchstr) || bv->buffer()->isReadonly())
		return 0;

	if (!stringSelected(bv, searchstr, cs, mw, fw))
		return 0;

	LyXText * text = bv->getLyXText();

	text->replaceSelectionWithString(replacestr);
	text->setSelectionRange(replacestr.length());
	bv->cursor().current() = fw ? bv->cursor().selEnd() : bv->cursor().selBegin();
	bv->buffer()->markDirty();
	find(bv, searchstr, cs, mw, fw);
	bv->update();

	return 1;
}

} //namespace anon
