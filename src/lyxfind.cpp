/**
 * \file lyxfind.cpp
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

#include "Buffer.h"
#include "BufferParams.h"
#include "Cursor.h"
#include "CutAndPaste.h"
#include "buffer_funcs.h"
#include "BufferView.h"
#include "Changes.h"
#include "FuncRequest.h"
#include "Text.h"
#include "Paragraph.h"
#include "ParIterator.h"

#include "frontends/alert.h"

#include "support/convert.h"
#include "support/debug.h"
#include "support/docstream.h"
#include "support/gettext.h"
#include "support/lstrings.h"

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
	return (var == "1");
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
	cur.setCursor(doc_iterator_begin(buf.inset()));
	while (findForward(cur, match, false)) {
		pos_type pos = cur.pos();
		Font const & font
			= cur.paragraph().getFontSettings(buf.params(), pos);
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

	updateLabels(buf);
	bv->putSelectionAt(doc_iterator_begin(buf.inset()), 0, false);
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
	// avoid crash (assertion violation) if the imaginary end-of-par
	// character of the last paragraph of the document is marked as changed
	if (tip.at_end())
		tip.backwardPos();

	// Now put cursor to end of selection:
	bv->cursor().setCursor(cur);
	bv->cursor().setSelection();

	return true;
}

} // lyx namespace
