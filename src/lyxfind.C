#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxtext.h"
#include "lyxfind.h"
#include "frontends/LyXView.h"
#include "frontends/Alert.h"
#include "support/textutils.h"
#include "support/lstrings.h"
#include "BufferView.h"
#include "buffer.h"
#include "debug.h"
#include "gettext.h"
#include "insets/insettext.h"

using lyx::pos_type;

namespace lyxfind {

/// returns true if the specified string is at the specified  position
bool IsStringInText(Paragraph * par, pos_type pos,
		    string const & str, bool const & = true,
		    bool const & = false);

/// if the string is found: return true and set the cursor to the new position
SearchResult SearchForward(BufferView *, LyXText * text, string const & str,
			   bool const & = true, bool const & = false);
///
SearchResult SearchBackward(BufferView *, LyXText * text, string const & str,
			    bool const & = true, bool const & = false);

int LyXReplace(BufferView * bv,
	       string const & searchstr, string const & replacestr,
	       bool forward, bool casesens, bool matchwrd, bool replaceall,
	       bool once)
{
	if (!bv->available() || bv->buffer()->isReadonly())
		return 0;

	// CutSelection cannot cut a single space, so we have to stop
	// in order to avoid endless loop :-(
	if (searchstr.length() == 0
		|| (searchstr.length() == 1 && searchstr[0] == ' '))
	{
		Alert::alert(_("Sorry!"), _("You cannot replace a single space, "
					  "nor an empty character."));
		return 0;
	}

	// now we can start searching for the first
	// start at top if replaceall
	LyXText * text = bv->getLyXText();
	bool fw = forward;
	if (replaceall) {
		text->clearSelection();
		bv->unlockInset(bv->theLockingInset());
		text = bv->text;
		text->cursorTop(bv);
		// override search direction because we search top to bottom
		fw = true;
	}

	// if nothing selected or selection does not equal search string
	// search and select next occurance and return if no replaceall
	string str1;
	string str2;
	if (casesens) {
		str1 = searchstr;
		str2 = text->selectionAsString(bv->buffer(), false);
	} else {
		str1 = lowercase(searchstr);
		str2 = lowercase(text->selectionAsString(bv->buffer(), false));
	}
	if (str1 != str2) {
		if (!LyXFind(bv, searchstr, fw, casesens, matchwrd) ||
			!replaceall)
		{
			return 0;
		}
	}

	bool found = false;
	int replace_count = 0;
	do {
		text = bv->getLyXText();
		// We have to do this check only because mathed insets don't
		// return their own LyXText but the LyXText of it's parent!
		if (!bv->theLockingInset() ||
			((text != bv->text) &&
			 (text->inset_owner == text->inset_owner->getLockingInset())))
		{
			bv->hideCursor();
			bv->update(text, BufferView::SELECT|BufferView::FITCUR);
			bv->toggleSelection(false);
			text->replaceSelectionWithString(bv, replacestr);
			text->setSelectionOverString(bv, replacestr);
			bv->update(text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			++replace_count;
		}
		if (!once)
			found = LyXFind(bv, searchstr, fw, casesens, matchwrd);
	} while (!once && replaceall && found);

	return replace_count;
}


bool LyXFind(BufferView * bv,
	     string const & searchstr, bool forward,
	     bool casesens, bool matchwrd)
{
	if (!bv->available() || searchstr.empty())
		return false;

	bv->hideCursor();
	bv->update(bv->getLyXText(), BufferView::SELECT|BufferView::FITCUR);

	if (bv->theLockingInset()) {
		bool found = forward ?
			bv->theLockingInset()->searchForward(bv, searchstr, casesens, matchwrd) :
			bv->theLockingInset()->searchBackward(bv, searchstr, casesens, matchwrd);
		// We found the stuff inside the inset so we don't have to
		// do anything as the inset did all the update for us!
		if (found)
			return true;
		// We now are in the main text but if we did a forward
		// search we have to put the cursor behind the inset.
		if (forward) {
			bv->text->cursorRight(bv, true);
		}
	}
	// If we arrive here we are in the main text again so we
	// just start searching from the root LyXText at the position
	// we are!
	LyXText * text = bv->text;

	if (text->selection.set())
		text->cursor = forward ?
			text->selection.end : text->selection.start;

	bv->toggleSelection();
	text->clearSelection();

	SearchResult result = forward ?
		SearchForward(bv, text, searchstr, casesens, matchwrd) :
		SearchBackward(bv, text, searchstr, casesens, matchwrd);

	bool found = true;
	// If we found the cursor inside an inset we will get back
	// SR_FOUND_NOUPDATE and we don't have to do anything as the
	// inset did it already.
	if (result == SR_FOUND) {
		bv->unlockInset(bv->theLockingInset());
		bv->update(text, BufferView::SELECT|BufferView::FITCUR);
		text->setSelectionOverString(bv, searchstr);
		bv->toggleSelection(false);
		bv->update(text, BufferView::SELECT|BufferView::FITCUR);
	} else if (result == SR_NOT_FOUND) {
		bv->unlockInset(bv->theLockingInset());
		bv->update(text, BufferView::SELECT|BufferView::FITCUR);
		found = false;
	}

	return found;
}


SearchResult LyXFind(BufferView * bv, LyXText * text,
		     string const & searchstr, bool forward,
		     bool casesens, bool matchwrd)
{
	if (text->selection.set())
		text->cursor = forward ?
			text->selection.end : text->selection.start;

	bv->toggleSelection();
	text->clearSelection();

	SearchResult result = forward ?
		SearchForward(bv, text, searchstr, casesens, matchwrd) :
		SearchBackward(bv, text, searchstr, casesens, matchwrd);

	return result;
}


// returns true if the specified string is at the specified position
bool IsStringInText(Paragraph * par, pos_type pos,
		    string const & str, bool const & cs,
		    bool const & mw)
{
	if (!par)
		return false;

	string::size_type size = str.length();
	pos_type i = 0;
	while (((pos + i) < par->size())
	       && (string::size_type(i) < size)
	       && (cs ? (str[i] == par->getChar(pos + i))
		   : (uppercase(str[i]) == uppercase(par->getChar(pos + i)))))
	{
		++i;
	}
	if (size == string::size_type(i)) {
		// if necessary, check whether string matches word
		if (!mw)
			return true;
		if ((pos <= 0 || !IsLetterCharOrDigit(par->getChar(pos - 1)))
			&& (pos + pos_type(size) >= par->size()
			|| !IsLetterCharOrDigit(par->getChar(pos + size)))) {
			return true;
		}
	}
	return false;
}

// forward search:
// if the string can be found: return true and set the cursor to
// the new position, cs = casesensitive, mw = matchword
SearchResult SearchForward(BufferView * bv, LyXText * text, string const & str,
			   bool const & cs, bool const & mw)
{
	Paragraph * par = text->cursor.par();
	pos_type pos = text->cursor.pos();
	Paragraph * prev_par = par;
	UpdatableInset * inset;

	while (par && !IsStringInText(par, pos, str, cs, mw)) {
		if (par->isInset(pos) &&
			(inset = (UpdatableInset *)par->getInset(pos)) &&
			(inset->isTextInset()))
		{
#if 0
			// lock the inset!
			text->setCursor(bv, par, pos);
			inset->edit(bv);
#endif
			if (inset->searchForward(bv, str, cs, mw))
				return SR_FOUND_NOUPDATE;
		}

		++pos;

		if (pos >= par->size()) {
			prev_par = par;
			par = par->next();
			pos = 0;
		}
	}

	if (par) {
		text->setCursor(bv, par, pos);
		return SR_FOUND;
	} else {
		// make sure we end up at the end of the text,
		// not the start point of the last search
		text->setCursor(bv, prev_par, prev_par->size());
		return SR_NOT_FOUND;
	}
}


// backward search:
// if the string can be found: return true and set the cursor to
// the new position, cs = casesensitive, mw = matchword
SearchResult SearchBackward(BufferView * bv, LyXText * text,
			    string const & str,
			    bool const & cs, bool const & mw)
{
	Paragraph * par = text->cursor.par();
	pos_type pos = text->cursor.pos();
	Paragraph * prev_par = par;

	do {
		if (pos > 0)
			--pos;
		else {
			prev_par = par;
			// We skip empty paragraphs (Asger)
			do {
				par = par->previous();
				if (par)
					pos = par->size() - 1;
			} while (par && pos < 0);
		}
		UpdatableInset * inset;
		if (par && par->isInset(pos) &&
			(inset = (UpdatableInset *)par->getInset(pos)) &&
			(inset->isTextInset()))
		{
#if 0
			// lock the inset!
			text->setCursor(bv, par, pos);
			inset->edit(bv, false);
#endif
			if (inset->searchBackward(bv, str, cs, mw))
				return SR_FOUND_NOUPDATE;
		}
	} while (par && !IsStringInText(par, pos, str, cs, mw));

	if (par) {
		text->setCursor(bv, par, pos);
		return SR_FOUND;
	} else {
		// go to the last part of the unsuccessful search
		text->setCursor(bv, prev_par, 0);
		return SR_NOT_FOUND;
	}
}

} // end lyxfind namespace
