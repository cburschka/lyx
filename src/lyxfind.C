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
#include "buffer_funcs.h"
#include "BufferView.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "pariterator.h"
#include "undo.h"

#include "frontends/Alert.h"

#include "support/convert.h"
#include "support/docstream.h"

namespace lyx {

using support::lowercase;
using support::uppercase;
using support::split;

using std::advance;


namespace {

bool parse_bool(docstring & howto)
{
	if (howto.empty())
		return false;
	docstring var;
	howto = split(howto, var, ' ');
	return (var == "1");
}


class MatchString : public std::binary_function<Paragraph, pos_type, bool>
{
public:
	MatchString(docstring const & str, bool cs, bool mw)
		: str(str), cs(cs), mw(mw)
	{}

	// returns true if the specified string is at the specified position
	bool operator()(Paragraph const & par, pos_type pos) const
	{
		docstring::size_type const size = str.length();
		pos_type i = 0;
		pos_type const parsize = par.size();
		for (i = 0; pos + i < parsize; ++i) {
			if (docstring::size_type(i) >= size)
				break;
			if (cs && str[i] != par.getChar(pos + i))
				break;
			if (!cs && uppercase(str[i]) != uppercase(par.getChar(pos + i)))
				break;
		}

		if (size != docstring::size_type(i))
			return false;

		// if necessary, check whether string matches word
		if (mw) {
			if (pos > 0 && par.isLetter(pos - 1))
				return false;
			if (pos + pos_type(size) < parsize
			    && par.isLetter(pos + size))
				return false;
		}

		return true;
	}

private:
	// search string
	docstring str;
	// case sensitive
	bool cs;
	// match whole words only
	bool mw;
};


bool findForward(DocIterator & cur, MatchString const & match)
{
	for (; cur; cur.forwardChar())
		if (cur.inTexted() && match(cur.paragraph(), cur.pos()))
			return true;
	return false;
}


bool findBackwards(DocIterator & cur, MatchString const & match)
{
	while (cur) {
		cur.backwardChar();
		if (cur.inTexted() && match(cur.paragraph(), cur.pos()))
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


bool searchAllowed(BufferView * bv, docstring const & str)
{
	if (str.empty()) {
		frontend::Alert::error(_("Search error"),
		                            _("Search string is empty"));
		return false;
	}
	return bv->buffer();
}


bool find(BufferView * bv, docstring const & searchstr, bool cs, bool mw, bool fw)
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
	       docstring const & searchstr, docstring const & replacestr,
	       bool cs, bool mw)
{
	Buffer & buf = *bv->buffer();

	if (!searchAllowed(bv, searchstr) || buf.isReadonly())
		return 0;

	recordUndoFullDocument(bv);

	MatchString const match(searchstr, cs, mw);
	int num = 0;

	int const rsize = replacestr.size();
	int const ssize = searchstr.size();

	DocIterator cur = doc_iterator_begin(buf.inset());
	while (findForward(cur, match)) {
		pos_type pos = cur.pos();
		LyXFont const font
			= cur.paragraph().getFontSettings(buf.params(), pos);
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
	if ((cs && str1 != str2) || lowercase(str1) != lowercase(str2)) {
		find(bv, searchstr, cs, mw, fw);
		return false;
	}

	return true;
}


int replace(BufferView * bv, docstring const & searchstr,
	    docstring const & replacestr, bool cs, bool mw, bool fw)
{
	if (!searchAllowed(bv, searchstr) || bv->buffer()->isReadonly())
		return 0;

	if (!stringSelected(bv, searchstr, cs, mw, fw))
		return 0;

	LCursor & cur = bv->cursor();
	cap::replaceSelectionWithString(cur, replacestr, fw);
	bv->buffer()->markDirty();
	find(bv, searchstr, cs, mw, fw);
	bv->update();

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


void find(BufferView * bv, FuncRequest const & ev)
{
	if (!bv || ev.action != LFUN_WORD_FIND)
		return;

	//lyxerr << "find called, cmd: " << ev << std::endl;

	// data is of the form
	// "<search>
	//  <casesensitive> <matchword> <forward>"
	docstring search;
	docstring howto = split(ev.argument(), search, '\n');

	bool casesensitive = parse_bool(howto);
	bool matchword     = parse_bool(howto);
	bool forward       = parse_bool(howto);

	bool const found = find(bv, search,
				  casesensitive, matchword, forward);

	if (!found)
		// emit message signal.
		bv->message(_("String not found!"));
}


void replace(BufferView * bv, FuncRequest const & ev)
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

	Buffer * buf = bv->buffer();

	int const replace_count = all
		? replaceAll(bv, search, rplc, casesensitive, matchword)
		: replace(bv, search, rplc, casesensitive, matchword, forward);

	if (replace_count == 0) {
		// emit message signal.
		buf->message(_("String not found!"));
	} else {
		if (replace_count == 1) {
			// emit message signal.
			buf->message(_("String has been replaced."));
		} else {
			docstring str = convert<docstring>(replace_count);
			str += _(" strings have been replaced.");
			// emit message signal.
			buf->message(str);
		}
	}
}


bool findNextChange(BufferView * bv)
{
	if (!bv->buffer())
		return false;

	DocIterator cur = bv->cursor();

	if (!findChange(cur))
		return false;

	bv->cursor().setCursor(cur);
	bv->cursor().resetAnchor();

	Change orig_change = cur.paragraph().lookupChange(cur.pos());

	DocIterator et = doc_iterator_end(cur.inset());
	for (; cur != et; cur.forwardPosNoDescend()) {
		Change change = cur.paragraph().lookupChange(cur.pos());
		if (change != orig_change) {
			break;
		}
	}
	// Now put cursor to end of selection:
	bv->cursor().setCursor(cur);
	bv->cursor().setSelection();
	// if we used a lfun like in find/replace, dispatch would do
	// that for us
	bv->update();

	return true;
}

} // lyx namespace
