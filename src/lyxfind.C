#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxtext.h"
#include "lyxfind.h"
#include "LyXView.h"
#include "lyx_gui_misc.h"
#include "support/textutils.h"
#include "support/lstrings.h"
#include "BufferView.h"
#include "buffer.h"
#include "gettext.h"

///
// locally used enum
///
enum SearchResult {
	//
	SR_NOT_FOUND = 0,
	//
	SR_FOUND,
	//
	SR_FOUND_NOUPDATE
};


/// returns true if the specified string is at the specified  position
bool IsStringInText(Paragraph * par, Paragraph::size_type pos,
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
		WriteAlert(_("Sorry!"), _("You cannot replace a single space, "
		                          "nor an empty character."));
		return 0;
	}
	
	LyXText * text = bv->getLyXText();

	// now we can start searching for the first 
	// start at top if replaceall
	bool fw = forward;
	if (replaceall) {
		text->clearSelection();
		if (text->inset_owner) {
			bv->unlockInset(bv->theLockingInset());
			text = bv->text;
		}
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
		if (!LyXFind(bv, searchstr, fw, false, casesens, matchwrd) ||
			!replaceall)
		{
			return 0;
		}
	}

	bool found = false;
	int replace_count = 0;
	do {
		bv->hideCursor();
		bv->update(bv->getLyXText(), BufferView::SELECT|BufferView::FITCUR);
		bv->toggleSelection(false);
		bv->getLyXText()->replaceSelectionWithString(bv, replacestr);
		bv->getLyXText()->setSelectionOverString(bv, replacestr);
		bv->update(bv->getLyXText(), BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		++replace_count;
		if (!once)
			found = LyXFind(bv, searchstr, fw, false, casesens, matchwrd);
	} while (!once && replaceall && found);
   
	if (bv->focus())
		bv->showCursor();
	
	return replace_count;
}

bool LyXFind(BufferView * bv,
             string const & searchstr, bool forward,
             bool frominset, bool casesens, bool matchwrd)
{
	if (!bv->available() || searchstr.empty())
		return false;
	
	LyXText * text = bv->getLyXText();

	bv->hideCursor();
	bv->update(text, BufferView::SELECT|BufferView::FITCUR);
	
	if (text->selection.set())
		text->cursor = forward ?
			text->selection.end : text->selection.start;

	SearchResult result = SR_NOT_FOUND;

	if (!frominset && bv->theLockingInset()) {
		bool found = forward ?
			bv->theLockingInset()->searchForward(bv, searchstr, casesens, matchwrd) :
			bv->theLockingInset()->searchBackward(bv, searchstr, casesens, matchwrd);
		if (found)
			result = SR_FOUND_NOUPDATE;
		else {
			text = bv->getLyXText();
			Paragraph * par = text->cursor.par();
			Paragraph::size_type pos = text->cursor.pos();
			if (forward) {
				if (pos < par->size() - 1)
					++pos;
				else {
					pos = 0;
					par = par->next();
				}
				if (par)
					text->setCursor(bv, par, pos);
			}
			if (par) {
				result = forward ? 
					SearchForward(bv, text, searchstr, casesens, matchwrd) :
					SearchBackward(bv, text, searchstr, casesens, matchwrd);
			}
		}
	} else {
		result = forward ? 
			SearchForward(bv, text, searchstr, casesens, matchwrd) :
			SearchBackward(bv, text, searchstr, casesens, matchwrd);
	}

	bool found = true;
	if (result == SR_FOUND) {
		// the actual text pointer could have changed!
		bv->update(bv->getLyXText(), BufferView::SELECT|BufferView::FITCUR);
		bv->toggleSelection();
		bv->getLyXText()->clearSelection();
		bv->getLyXText()->setSelectionOverString(bv, searchstr);
		bv->toggleSelection(false);
		bv->update(bv->getLyXText(), BufferView::SELECT|BufferView::FITCUR);
	} else if (result == SR_NOT_FOUND)
		found = false;
   
	if (bv->focus())
		bv->showCursor();
   
	return found;
}


// returns true if the specified string is at the specified position
bool IsStringInText(Paragraph * par, Paragraph::size_type pos,
		    string const & str, bool const & cs,
		    bool const & mw)
{
	if (!par)
		return false;
   
	string::size_type size = str.length();
	Paragraph::size_type i = 0;
	while (((pos + i) < par->size())
	       && (string::size_type(i) < size)
	       && (cs ? (str[i] == par->getChar(pos + i))
	           : (toupper(str[i]) == toupper(par->getChar(pos + i)))))
	{
		++i;
	}
	if (size == string::size_type(i)) {
		// if necessary, check whether string matches word
		if (!mw || 
		    (mw && ((pos <= 0 || !IsLetterCharOrDigit(par->getChar(pos - 1)))
		             && (pos + Paragraph::size_type(size) >= par->size()
		                 || !IsLetterCharOrDigit(par->getChar(pos + size))))
		     ))
		{
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
	Paragraph::size_type pos = text->cursor.pos();
	UpdatableInset * inset;

	while (par && !IsStringInText(par, pos, str, cs, mw)) {
		if (par->isInset(pos) &&
			(inset = (UpdatableInset *)par->getInset(pos)) &&
			(inset->isTextInset()))
		{
			// lock the inset!
			text->setCursor(bv, par, pos);
			inset->edit(bv);
			if (inset->searchForward(bv, str, cs, mw))
				return SR_FOUND_NOUPDATE;
			text = bv->getLyXText();
		}
		if (pos < par->size() - 1)
			++pos;
		else {
			pos = 0;
			par = par->next();
		}
	}
	if (par) {
		text->setCursor(bv, par, pos);
		return SR_FOUND;
#if 0
	} else if (text->inset_owner) {
		// test if we're inside an inset if yes unlock the inset
		// and recall us with the outside LyXText!
		bv->unlockInset((UpdatableInset *)text->inset_owner);
		if (!bv->theLockingInset()) {
			text = bv->getLyXText();
			par = text->cursor.par();
			pos = text->cursor.pos();
			if (pos < par->size() - 1)
				++pos;
			else {
				pos = 0;
				par = par->next();
			}
			if (!par)
				return SR_NOT_FOUND;
			text->setCursor(bv, par, pos);
			return SearchForward(bv, text, str, cs, mw);
		} else {
			return SR_NOT_FOUND;
		}
#endif
	} else
		return SR_NOT_FOUND;
}


// backward search:
// if the string can be found: return true and set the cursor to
// the new position, cs = casesensitive, mw = matchword
SearchResult SearchBackward(BufferView * bv, LyXText * text,
                            string const & str,
                            bool const & cs, bool const & mw)
{
	Paragraph * par = text->cursor.par();
	Paragraph::size_type pos = text->cursor.pos();

	do {
		if (pos > 0)
			--pos;
		else {
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
			// lock the inset!
			text->setCursor(bv, par, pos);
			inset->edit(bv, false);
			if (inset->searchBackward(bv, str, cs, mw))
				return SR_FOUND_NOUPDATE;
			text = bv->getLyXText();
		}		
	} while (par && !IsStringInText(par, pos, str, cs, mw));
  
	if (par) {
		text->setCursor(bv, par, pos);
		return SR_FOUND;
	}
#if 0
	else if (text->inset_owner) {
		// test if we're inside an inset if yes unlock the inset
		// and recall us with the outside LyXText!
		bv->unlockInset((UpdatableInset *)text->inset_owner);
		if (!bv->theLockingInset()) {
			return SearchBackward(bv, bv->getLyXText(), str, cs, mw);
		}
	}
#endif
	return SR_NOT_FOUND;
}

