#include <config.h>

#include "lyxtext.h"
#include "lyxfind.h"
#include "paragraph.h"
#include "frontends/LyXView.h"
#include "frontends/Alert.h"
#include "support/textutils.h"
#include "support/lstrings.h"
#include "BufferView.h"
#include "buffer.h"
#include "debug.h"
#include "gettext.h"
#include "insets/insettext.h"
#include "changes.h"

using namespace lyx::support;

using lyx::pos_type;
using std::endl;

namespace lyx {
namespace find {

namespace {

// returns true if the specified string is at the specified position
bool isStringInText(Paragraph const & par, pos_type pos,
		    string const & str, bool const & cs,
		    bool const & mw)
{
	string::size_type size = str.length();
	pos_type i = 0;
	pos_type parsize = par.size();
	while (((pos + i) < parsize)
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

// forward search:
// if the string can be found: return true and set the cursor to
// the new position, cs = casesensitive, mw = matchword
SearchResult searchForward(BufferView * bv, LyXText * text, string const & str,
			   bool const & cs, bool const & mw)
{
	ParagraphList::iterator pit = text->cursor.par();
	ParagraphList::iterator pend = text->ownerParagraphs().end();
	pos_type pos = text->cursor.pos();
	UpdatableInset * inset;

	while (pit != pend && !isStringInText(*pit, pos, str, cs, mw)) {
		if (pos < pit->size()
		    && pit->isInset(pos)
		    && (inset = (UpdatableInset *)pit->getInset(pos))
		    && inset->isTextInset()
		    && inset->searchForward(bv, str, cs, mw))
			return SR_FOUND_NOUPDATE;

		if (++pos >= pit->size()) {
			++pit;
			pos = 0;
		}
	}

	if (pit != pend) {
		text->setCursor(pit, pos);
		return SR_FOUND;
	} else
		return SR_NOT_FOUND;
}


// backward search:
// if the string can be found: return true and set the cursor to
// the new position, cs = casesensitive, mw = matchword
SearchResult searchBackward(BufferView * bv, LyXText * text,
			    string const & str,
			    bool const & cs, bool const & mw)
{
	ParagraphList::iterator pit = text->cursor.par();
	ParagraphList::iterator pbegin = text->ownerParagraphs().begin();
	pos_type pos = text->cursor.pos();

	// skip past a match at the current cursor pos
	if (pos > 0) {
		--pos;
	} else if (pit != pbegin) {
		--pit;
		pos = pit->size();
	} else {
		return SR_NOT_FOUND;
	}

	while (true) {
		if (pos < pit->size()) {
			if (pit->isInset(pos) && pit->getInset(pos)->isTextInset()) {
				UpdatableInset * inset = (UpdatableInset *)pit->getInset(pos);
				if (inset->searchBackward(bv, str, cs, mw))
					return SR_FOUND_NOUPDATE;
			}

			if (isStringInText(*pit, pos, str, cs, mw)) {
				text->setCursor(pit, pos);
				return SR_FOUND;
			}
		}

		if (pos == 0 && pit == pbegin)
			break;

		if (pos > 0) {
			--pos;
		} else if (pit != pbegin) {
			--pit;
			pos = pit->size();
		}
	}

	return SR_NOT_FOUND;
}

} // anon namespace


int replace(BufferView * bv,
	       string const & searchstr, string const & replacestr,
	       bool forward, bool casesens, bool matchwrd, bool replaceall,
	       bool once)
{
	if (!bv->available() || bv->buffer()->isReadonly())
		return 0;

	// CutSelection cannot cut a single space, so we have to stop
	// in order to avoid endless loop :-(
	if (searchstr.length() == 0
		|| (searchstr.length() == 1 && searchstr[0] == ' ')) {
#ifdef WITH_WARNINGS
#warning BLECH. If we have an LFUN for replace, we can sort of fix this bogosity
#endif
		Alert::error(_("Cannot replace"),
			_("You cannot replace a single space or "
			  "an empty character."));
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
		text->cursorTop();
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
		if (!find(bv, searchstr, fw, casesens, matchwrd) ||
			!replaceall) {
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
			 (text->inset_owner == text->inset_owner->getLockingInset()))) {
			text->replaceSelectionWithString(replacestr);
			text->setSelectionRange(replacestr.length());
			++replace_count;
		}
		if (!once)
			found = find(bv, searchstr, fw, casesens, matchwrd);
	} while (!once && replaceall && found);

	// FIXME: should be called via an LFUN
	bv->buffer()->markDirty();
	bv->update();

	return replace_count;
}


bool find(BufferView * bv,
	     string const & searchstr, bool forward,
	     bool casesens, bool matchwrd)
{
	if (!bv->available() || searchstr.empty())
		return false;

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
			bv->text->cursorRight(true);
		}
	}
	// If we arrive here we are in the main text again so we
	// just start searching from the root LyXText at the position
	// we are!
	LyXText * text = bv->text;


	if (text->selection.set())
		text->cursor = forward ?
			text->selection.end : text->selection.start;

	text->clearSelection();

	SearchResult result = forward ?
		searchForward(bv, text, searchstr, casesens, matchwrd) :
		searchBackward(bv, text, searchstr, casesens, matchwrd);

	bool found = true;
	// If we found the cursor inside an inset we will get back
	// SR_FOUND_NOUPDATE and we don't have to do anything as the
	// inset did it already.
	if (result == SR_FOUND) {
		bv->unlockInset(bv->theLockingInset());
		text->setSelectionRange(searchstr.length());
	} else if (result == SR_NOT_FOUND) {
		bv->unlockInset(bv->theLockingInset());
		found = false;
	}
	bv->update();

	return found;
}


SearchResult find(BufferView * bv, LyXText * text,
		     string const & searchstr, bool forward,
		     bool casesens, bool matchwrd)
{
	if (text->selection.set())
		text->cursor = forward ?
			text->selection.end : text->selection.start;

	text->clearSelection();

	SearchResult result = forward ?
		searchForward(bv, text, searchstr, casesens, matchwrd) :
		searchBackward(bv, text, searchstr, casesens, matchwrd);

	return result;
}




SearchResult nextChange(BufferView * bv, LyXText * text, pos_type & length)
{
	ParagraphList::iterator pit = text->cursor.par();
	ParagraphList::iterator pend = text->ownerParagraphs().end();
	pos_type pos = text->cursor.pos();

	while (pit != pend) {
		pos_type parsize = pit->size();

		if (pos < parsize) {
			if ((!parsize || pos != parsize)
				&& pit->lookupChange(pos) != Change::UNCHANGED)
				break;

			if (pit->isInset(pos) && pit->getInset(pos)->isTextInset()) {
				UpdatableInset * inset = (UpdatableInset *)pit->getInset(pos);
				if (inset->nextChange(bv, length))
					return SR_FOUND_NOUPDATE;
			}
		}

		++pos;

		if (pos >= parsize) {
			++pit;
			pos = 0;
		}
	}

	if (pit == pend)
		return SR_NOT_FOUND;

	text->setCursor(pit, pos);
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
	length = end - pos;
	return SR_FOUND;
}


SearchResult findNextChange(BufferView * bv, LyXText * text, pos_type & length)
{
	if (text->selection.set())
		text->cursor = text->selection.end;

	text->clearSelection();

	return nextChange(bv, text, length);
}


bool findNextChange(BufferView * bv)
{
	if (!bv->available())
		return false;

	pos_type length;

	if (bv->theLockingInset()) {
		bool found = bv->theLockingInset()->nextChange(bv, length);

		// We found the stuff inside the inset so we don't have to
		// do anything as the inset did all the update for us!
		if (found)
			return true;

		// We now are in the main text but if we did a forward
		// search we have to put the cursor behind the inset.
		bv->text->cursorRight(true);
	}
	// If we arrive here we are in the main text again so we
	// just start searching from the root LyXText at the position
	// we are!
	LyXText * text = bv->text;

	if (text->selection.set())
		text->cursor = text->selection.end;

	text->clearSelection();

	SearchResult result = nextChange(bv, text, length);

	bool found = true;

	// If we found the cursor inside an inset we will get back
	// SR_FOUND_NOUPDATE and we don't have to do anything as the
	// inset did it already.
	if (result == SR_FOUND) {
		bv->unlockInset(bv->theLockingInset());
		text->setSelectionRange(length);
	} else if (result == SR_NOT_FOUND) {
		bv->unlockInset(bv->theLockingInset());
		found = false;
	}

	bv->update();

	return found;
}

} // find namespace
} // lyx namespace
