/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich,
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "lyxfr1.h"

#include "lyxtext.h"
#include "LyXView.h"
#include "minibuffer.h"
#include "lyx_gui_misc.h"
#include "support/textutils.h"
#include "support/lstrings.h"
#include "BufferView.h"
#include "buffer.h"

// Returns the current selection. If nothing is selected or if the selection
// spans 2 paragraphs, an empty string is returned.
static
string GetCurrentSelectionAsString(LyXText * lt) 
{
	string sz;
	
	LyXParagraph * par = lt->cursor.par();
	if (lt->selection && lt->sel_cursor.par() == par) {
		// (selected) and (begin/end in same paragraph)
		LyXParagraph::size_type pos = 
			lt->sel_start_cursor.pos();
		LyXParagraph::size_type endpos = 
			lt->sel_end_cursor.pos();
		bool fPrevIsSpace = false;
		char ch;
		while (pos < par->Last() && pos < endpos) {
			ch = par->GetChar(pos);
			
			//HB??: Maybe (ch <= ' ') 
			if ((ch == ' ') || (ch <= LyXParagraph::META_INSET)) {
				// consecutive spaces --> 1 space char
				if (fPrevIsSpace) {
					++pos;		// Next text pos
					continue;	// same search pos
				}
				sz += ' ';
				fPrevIsSpace = true;
			} else {
				sz += ch;
				fPrevIsSpace = false;
			}
			++pos;
		}
	}
	return sz;
}


// If nothing selected, select the word at the cursor.
// Returns the current selection
static inline
string GetSelectionOrWordAtCursor(BufferView * bv) 
{
	bv->text->SelectWordWhenUnderCursor(bv);
	return GetCurrentSelectionAsString(bv->text);
}


// This is a copy of SetSelectionOverString from text.C
// It does the same, but uses only the length as a parameter
static inline
void SetSelectionOverLenChars(BufferView * bv, int len)
{
	bv->text->sel_cursor = bv->text->cursor;
	for (int i = 0; i < len; ++i)
		bv->text->CursorRight(bv);
	bv->text->SetSelection();
}


//------------------------------


LyXFindReplace::LyXFindReplace()
	: bv(0)
{}


LyXFindReplace::~LyXFindReplace()
{}


void LyXFindReplace::StartSearch(BufferView * b)
{
	bv = b;
	SF.StartSearch(this);
	SF.replaceEnabled(!bv->buffer()->isReadonly());
	searchForward = true;
	if (SF.SearchString().empty()) 
		SF.SetSearchString(GetSelectionOrWordAtCursor(bv));
}	


// TODO?: the user can insert multiple spaces with this
// routine (1999-01-11, dnaber)
void LyXFindReplace::SearchReplaceCB()
{
	if (!bv->available()) return;
	if (bv->buffer()->isReadonly()) return;
	
	// CutSelection cannot cut a single space, so we have to stop
	// in order to avoid endless loop :-(
	if (SF.SearchString().length() == 0
	    || (SF.SearchString().length() == 1
		&& SF.SearchString()[0] == ' ')) {
		WriteAlert(_("Sorry!"), _("You cannot replace a single space, "
					  "nor an empty character."));
		return;
	}
	
	string const replacestring = SF.ReplaceString();

	bv->hideCursor();
	bv->update(BufferView::SELECT|BufferView::FITCUR);

	LyXText * ltCur = bv->text;	
	if (ltCur->selection) {
		// clear the selection (if there is any) 
		bv->toggleSelection(false);
		bv->text->
			ReplaceSelectionWithString(bv, replacestring);
		bv->text->
			SetSelectionOverString(bv, replacestring);
		bv->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	}
	
	// jump to next match:
	SearchCB(searchForward);
}


// replaces all occurences of a string (1999-01-15, dnaber@mini.gt.owl.de)
void LyXFindReplace::SearchReplaceAllCB()
{
	if (!bv->available()) return;
	if (bv->buffer()->isReadonly()) return;

	// CutSelection cannot cut a single space, so we have to stop
	// in order to avoid endless loop :-(
	if (SF.SearchString().length() == 0
	    || (SF.SearchString().length() == 1
		&& SF.SearchString()[0] == ' ')) {
		WriteAlert(_("Sorry!"), _("You cannot replace a single space, "
					  "nor an empty character."));
		return;
	}

	string const replacestring = SF.ReplaceString();

	bv->hideCursor();

	// start at top
	bv->text->ClearSelection();
	bv->text->CursorTop(bv);

	int replace_count = 0;
	LyXText	* ltCur;
	do {
		ltCur = bv->text;	
		if (ltCur->selection) {
			bv->update(BufferView::SELECT|BufferView::FITCUR);
			bv->toggleSelection(false);
			bv->text->
				ReplaceSelectionWithString(bv, replacestring);
			bv->text->
				SetSelectionOverString(bv, replacestring);
			bv->update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE); 
			++replace_count;
		}
	} while (SearchCB(true));
	if (replace_count == 0) {
		bv->owner()->getMiniBuffer()->Set(
			_("String not found!"));
	} else {
		if (replace_count == 1) {
			bv->owner()->getMiniBuffer()->Set(
				_("1 string has been replaced."));
		} else {
			string str = tostr(replace_count);
			str += _(" strings have been replaced.");
			bv->owner()->getMiniBuffer()->Set(str);
		}
	}
}


bool LyXFindReplace::SearchCB(bool fForward)
{
	// store search direction
	searchForward = fForward;
	
	if (!bv->available())
		return false;
   
	bv->hideCursor();
	bv->update(BufferView::SELECT|BufferView::FITCUR);
	LyXText * ltCur = bv->text;
	if (ltCur->selection) 
		ltCur->cursor = fForward ? ltCur->sel_end_cursor :
		ltCur->sel_start_cursor;

	iLenSelected = SF.SearchString().length();
	bool result;
   
	if (!SF.ValidSearchData() ||
	    (fForward ? SearchForward(bv):SearchBackward(bv))) {
		bv->update(BufferView::SELECT|BufferView::FITCUR);

		// clear the selection (if there is any) 
		bv->toggleSelection();
		bv->text->ClearSelection();

		// set the new selection 
		SetSelectionOverLenChars(bv, iLenSelected);
		bv->toggleSelection(false);
		bv->owner()->getMiniBuffer()->Set(_("Found."));
		result = true;
	} else {
		bv->owner()->getMiniBuffer()->Set(_("String not found!"));
		result = false;
	}

	if (bv->focus())
		bv->showCursor();

	return result;
}


// if the string can be found: return true and set the cursor to
// the new position 
// (was: LyXText::SearchForward(char const* string) in text2.C )
bool LyXFindReplace::SearchForward(BufferView * bv)
{
	LyXParagraph * par = bv->text->cursor.par();
	LyXParagraph::size_type pos = bv->text->cursor.pos();

	while (par && !IsSearchStringInText(par, pos)) {
		if (pos < par->Last() - 1)
			++pos;
		else {
			pos = 0;
			par = par->Next();
		}
	}
	if (par) {
		bv->text->SetCursor(bv, par, pos);
		return true;
	} else
		return false;
}


// if the string can be found: return true and set the cursor to
// the new position 
// (was: LyXText::SearchBackward(char const* string) in text2.C )
bool LyXFindReplace::SearchBackward(BufferView * bv)
{
	LyXParagraph * par = bv->text->cursor.par();
	int pos = bv->text->cursor.pos();

	do {
		if (pos > 0)
			--pos;
		else {
			// We skip empty paragraphs (Asger)
			do {
				par = par->Previous();
				if (par)
					pos = par->Last() - 1;
			} while (par && pos < 0);
		}
	} while (par && !IsSearchStringInText(par, pos));
  
	if (par) {
		bv->text->SetCursor(bv, par, pos);
		return true;
	} else
		return false;
}


/* Compares 2 char values. 
return value is
    > 0 if chSearch > ch2
    = 0 if chSearch == ch2
    < 0 if chSearch < ch2
*/
int LyXFindReplace::CompareChars(char chSearch, char chText) const
{
	if (SF.CaseSensitive())
		return (chSearch - chText);
	return (toupper(chSearch) - toupper(chText));
}


// returns true if the search string is at the specified position 
// (Copied from the original "LyXText::IsStringInText" in text2.C )
bool LyXFindReplace::IsSearchStringInText(LyXParagraph * par,
					  LyXParagraph::size_type pos) const
{
	if (!par) return false;

	char chSrch = 0;
	char chText;
	bool fPrevIsSpace = false;
	int iText = 0;
	string::size_type iSrch = 0;
	while (pos + iText < par->Last() && 
	       iSrch < SF.SearchString().length()) {
		chSrch = SF.SearchString()[iSrch];
		chText = par->GetChar(pos+iText);
 		if (chText == ' ') {
			if (fPrevIsSpace) {
				++iText;  // next Text pos
				continue; // same search pos
			}
			fPrevIsSpace = true;
		} else
			fPrevIsSpace = false;
		if (CompareChars(chSrch, chText) != 0)
			break;
		
		++iSrch;
		++iText;
	}

	if (iSrch < SF.SearchString().length())
		return false;

	if (!SF.MatchWord() 
	    || ((pos <= 0 || !IsLetterCharOrDigit(par->GetChar(pos - 1)))
		&& (pos + iText >= par->Last() 
		    || !IsLetterCharOrDigit(par->GetChar(pos + iText))))) {
		iLenSelected = iText;
		return true;
	}

	return false;
}
