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
#include "BufferView.h"
#include "debug.h"
#include "iterators.h"
#include "gettext.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "PosIterator.h"
#include "undo.h"

#include "frontends/Alert.h"

#include "insets/insettext.h"

#include "support/textutils.h"

using lyx::support::lowercase;
using lyx::support::uppercase;
using bv_funcs::put_selection_at;

using std::string;


namespace lyx {
namespace find {

namespace {

class MatchString
{
public:
	MatchString(string const & str, bool cs, bool mw)
		: str(str), cs(cs), mw(mw) {};
// returns true if the specified string is at the specified position
	bool operator()(Paragraph const & par, pos_type pos) const
	{			
		string::size_type size = str.length();
		pos_type i = 0;
		pos_type parsize = par.size();
		while ((pos + i < parsize)
		       && (string::size_type(i) < size)
		       && (cs ? (str[i] == par.getChar(pos + i))
			   : (uppercase(str[i]) == uppercase(par.getChar(pos + i))))) {
			++i;
		}
		if (size == string::size_type(i)) {
			// if necessary, check whether string matches word
			if (!mw)
				return true;
			if ((pos <= 0 || !IsLetterCharOrDigit(par.getChar(pos - 1)))
			    && (pos + pos_type(size) >= parsize
				|| !IsLetterCharOrDigit(par.getChar(pos + size)))) {
				return true;
			}
		}
		return false;
	}
	
private:
	string str;
	bool cs;
	bool mw;
};


bool findForward(PosIterator & cur, PosIterator const & end,
		 MatchString & match)
{
	for (; cur != end && !match(*cur.pit(), cur.pos()); ++cur)
		;

	return cur != end;
}


bool findBackwards(PosIterator & cur, PosIterator const & beg,
		   MatchString & match)
{
	if (beg == cur)
		return false;
	do {
		--cur;
		if (match(*cur.pit(), cur.pos()))
			break;
	} while (cur != beg);

	return match(*cur.pit(), cur.pos());
}


bool findChange(PosIterator & cur, PosIterator const & end)
{
	for (; cur != end; ++cur) {
		if ((!cur.pit()->size() || !cur.at_end())
		    && cur.pit()->lookupChange(cur.pos()) != Change::UNCHANGED)
			break;
	}
	
	return cur != end;
}


bool searchAllowed(BufferView * bv, string const & str)
{
	if (str.empty()) {
		Alert::error(_("Search error"), _("Search string is empty"));
		return false;
	}
	return bv->available();
}

} // namespace anon



bool find(BufferView * bv, string const & searchstr,
	  bool cs, bool mw, bool fw)
{
	if (!searchAllowed(bv, searchstr))
		return false;

	PosIterator cur = PosIterator(*bv);

	MatchString match(searchstr, cs, mw);
	
	bool found;

	if (fw) {
		PosIterator const end = bv->buffer()->pos_iterator_end();
		found = findForward(cur, end, match);
	} else {
		PosIterator const beg = bv->buffer()->pos_iterator_begin();
		found = findBackwards(cur, beg, match);
	}
	
	if (found)
		put_selection_at(bv, cur, searchstr.length(), !fw);

	return found;
}


int replaceAll(BufferView * bv,
	       string const & searchstr, string const & replacestr,
	       bool cs, bool mw)
{
	Buffer & buf = *bv->buffer();

	if (!searchAllowed(bv, searchstr) || buf.isReadonly())
		return 0;
	
	recordUndo(Undo::ATOMIC, bv->text, 0,
		   buf.paragraphs().size() - 1);
	
	PosIterator cur = buf.pos_iterator_begin();
	PosIterator const end = buf.pos_iterator_end();
	MatchString match(searchstr, cs, mw);
	int num = 0;

	int const rsize = replacestr.size();
	int const ssize = searchstr.size();
	while (findForward(cur, end, match)) {
		pos_type pos = cur.pos();
		LyXFont const font
			= cur.pit()->getFontSettings(buf.params(), pos);
		int striked = ssize - cur.pit()->erase(pos, pos + ssize);
		cur.pit()->insert(pos, replacestr, font);
		advance(cur, rsize + striked);
		++num;
	}
	PosIterator beg = buf.pos_iterator_begin();
	bv->text->init(bv);
	put_selection_at(bv, beg, 0, false);
	if (num)
		buf.markDirty();
	return num;
}


int replace(BufferView * bv,
	    string const & searchstr, string const & replacestr,
	    bool cs, bool mw, bool fw)
{
	if (!searchAllowed(bv, searchstr) || bv->buffer()->isReadonly())
		return 0;
	
	{
		LyXText * text = bv->getLyXText();
		// if nothing selected or selection does not equal search
		// string search and select next occurance and return
		string const str1 = searchstr;
		string const str2 = text->selectionAsString(*bv->buffer(),
							    false);
		if ((cs && str1 != str2)
		    || lowercase(str1) != lowercase(str2)) {
			find(bv, searchstr, cs, mw, fw);
			return 0;
		}
	}

#ifdef LOCK
	LyXText * text = bv->getLyXText();
	// We have to do this check only because mathed insets don't
	// return their own LyXText but the LyXText of it's parent!
	if (!bv->innerInset() ||
	    ((text != bv->text) &&
	     (text->inset_owner == text->inset_owner->getLockingInset()))) {
		text->replaceSelectionWithString(replacestr);
		text->setSelectionRange(replacestr.length());
		text->cursor = fw ? text->selection.end
			: text->selection.start;
	}
#endif

	bv->buffer()->markDirty();
	find(bv, searchstr, cs, mw, fw);
	bv->update();
	
	return 1;
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
	bv->text->init(bv);
	put_selection_at(bv, cur, length, true);
	return true;
}

} // find namespace
} // lyx namespace
