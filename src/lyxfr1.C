/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich,
 *           Copyright 1995-1999 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include <cctype>
#include <cstring>
#include <cstdlib>

#ifdef __GNUG__
#pragma implementation
#endif

#include "LString.h"
#include "lyx_main.h"
#include FORMS_H_LOCATION
#include "form1.h"
#include "lyxfr0.h"
#include "lyxfr1.h"
#include "lyxfunc.h"
#include "debug.h"
#include "lyxtext.h"
#include "gettext.h"
#include "LyXView.h"
#include "lyx_gui_misc.h"
#include "minibuffer.h"
#include "support/lstrings.h"
#include "support/textutils.h"

extern BufferView * current_view; // called too many times in this file...

// Maximum length copied from the current selection to the search string
const int LYXSEARCH_MAXLEN =  128;

// function prototypes

// If nothing selected, select the word at the cursor.
// Returns the current selection
// Note: this function should be in LyXText!
string const GetSelectionOrWordAtCursor(LyXText * lt);

// Returns the current selection. If nothing is selected or if the selection
// spans 2 paragraphs, an empty string is returned.
string const GetCurrentSelectionAsString(LyXText * lt);

// This is a copy of SetSelectionOverString from text.C
// It does the same, but uses only the length as a parameter
void SetSelectionOverLenChars(LyXText * lt, int len);

//-------------------------------------------------------------



// Returns the current selection. If nothing is selected or if the selection
// spans 2 paragraphs, an empty string is returned.
string const GetCurrentSelectionAsString(LyXText * lt) 
{
	char sz[LYXSEARCH_MAXLEN];
	sz[0] = 0;
	
	LyXParagraph * par = lt->cursor.par;
	if (lt->selection && (lt->sel_cursor.par == par)) {
		// (selected) and (begin/end in same paragraph)
		LyXParagraph::size_type pos = 
			lt->sel_start_cursor.pos;
		LyXParagraph::size_type endpos = 
			lt->sel_end_cursor.pos;
		int i = 0;
		bool fPrevIsSpace = false;
		char ch;
		while ((i < LYXSEARCH_MAXLEN - 2) && 
		       (pos < par->Last()) && (pos < endpos)) {
			ch = par->GetChar(pos);

			//HB??: Maybe (ch <= ' ') 
			if ((ch == ' ') || (ch <= LyXParagraph::META_INSET)) {
				// consecutive spaces --> 1 space char
				if (fPrevIsSpace) {
					++pos;		// Next text pos
					continue;	// same search pos
				}
				sz[i] = ' ';
				fPrevIsSpace = true;
			} else {
				sz[i] = ch;
				fPrevIsSpace = false;
			}
			++pos;
			++i;
		} 
		sz[i] = 0;
	}
	return string(sz);
}


// If nothing selected, select the word at the cursor.
// Returns the current selection
string const GetSelectionOrWordAtCursor(LyXText * lt) 
{
	lt->SelectWordWhenUnderCursor();
	return GetCurrentSelectionAsString(lt);
}


// This is a copy of SetSelectionOverString from text.C
// It does the same, but uses only the length as a parameter
void SetSelectionOverLenChars(LyXText * lt, int len)
{
	lt->sel_cursor = lt->cursor;
	for (int i = 0; i < len; ++i)
		lt->CursorRight();
	lt->SetSelection();
}


//------------------------------

void LyXFindReplace1::StartSearch()
{
	LyXFindReplace0::StartSearch();
	SetReplaceEnabled(!current_view->buffer()->isReadonly());
	searchForward = true;
	if (lsSearch.empty()) 
		SetSearchString(GetSelectionOrWordAtCursor(current_view->text));
}	


// TODO?: the user can insert multiple spaces with this
// routine (1999-01-11, dnaber)
void LyXFindReplace1::SearchReplaceCB()
{
	if (!current_view->available()) return;
	if (current_view->buffer()->isReadonly()) return;
	
	// CutSelection cannot cut a single space, so we have to stop
	// in order to avoid endless loop :-(
	ReInitFromForm();
	if (SearchString().length() == 0 || (SearchString().length() == 1
					     && SearchString()[0] == ' ') ) {
		WriteAlert(_("Sorry!"), _("You cannot replace a single space, "
					  "nor an empty character."));
		return;
	}
	
	string const replacestring = ReplaceString();

	current_view->hideCursor();
	current_view->update(-2);

	LyXText * ltCur = current_view->text;	
	if (ltCur->selection) {
		// clear the selection (if there is any) 
		current_view->toggleSelection(false);
		current_view->text->
			ReplaceSelectionWithString(replacestring.c_str());
		current_view->text->
			SetSelectionOverString(replacestring.c_str());
		current_view->update(1);
	}
	
	// jump to next match:
	SearchCB( searchForward );
}


// replaces all occurences of a string (1999-01-15, dnaber@mini.gt.owl.de)
void LyXFindReplace1::SearchReplaceAllCB()
{
	if (!current_view->available()) return;
	if (current_view->buffer()->isReadonly()) return;

	// CutSelection cannot cut a single space, so we have to stop
	// in order to avoid endless loop :-(
	ReInitFromForm();
	if (SearchString().length() == 0 || (SearchString().length() == 1
					     && SearchString()[0] == ' ') ) {
		WriteAlert(_("Sorry!"), _("You cannot replace a single space, "
					  "nor an empty character."));
		return;
	}

	string const replacestring = ReplaceString();

	current_view->hideCursor();

	// start at top
	current_view->text->ClearSelection();
	current_view->text->CursorTop();

	int replace_count = 0;
	LyXText	* ltCur;
	do {
		ltCur = current_view->text;	
		if (ltCur->selection) {
			current_view->update(-2);
			current_view->toggleSelection(false);
			current_view->text->
				ReplaceSelectionWithString(replacestring.c_str());
			current_view->text->
				SetSelectionOverString(replacestring.c_str());
			current_view->update(1); 
			++replace_count;
		}
	} while( SearchCB(true) );
	if( replace_count == 0 ) {
		LyXBell();	
		current_view->owner()->getMiniBuffer()->Set(
			_("String not found!"));
	} else {
		if (replace_count == 1) {
			current_view->owner()->getMiniBuffer()->Set(
				_("1 string has been replaced."));
		} else {
			string str = tostr(replace_count);
			str += _(" strings have been replaced.");
			current_view->owner()->getMiniBuffer()->Set(str);
		}
	}
}


bool LyXFindReplace1::SearchCB(bool fForward)
{
	// store search direction
	searchForward = fForward;
	
	if (!current_view->available())
		return false;
   
	current_view->hideCursor();
	current_view->update(-2);
	LyXText * ltCur = current_view->text;
	if (ltCur->selection) 
		ltCur->cursor = fForward ? ltCur->sel_end_cursor :
		ltCur->sel_start_cursor;

	ReInitFromForm();
	iLenSelected = SearchString().length();
	bool result;
   
	if (!ValidSearchData() ||
	    (fForward ? SearchForward(ltCur) : SearchBackward(ltCur))) {
		current_view->update(-2);

		// clear the selection (if there is any) 
		current_view->toggleSelection();
		current_view->text->ClearSelection();

		// set the new selection 
		SetSelectionOverLenChars(current_view->text, iLenSelected);
		current_view->toggleSelection(false);
		current_view->owner()->getMiniBuffer()->Set(_("Found."));
		result = true;
	} else {
		LyXBell();
		current_view->owner()->getMiniBuffer()->Set(_("String not found!"));
		result = false;
	}

	if (current_view->focus())
		current_view->showCursor();

	return result;
}


// if the string can be found: return true and set the cursor to
// the new position 
// (was: LyXText::SearchForward(char const* string) in text2.C )
bool LyXFindReplace1::SearchForward(LyXText * lt)
{
	LyXParagraph * par = lt->cursor.par;
	LyXParagraph::size_type pos = lt->cursor.pos;

	while (par && !IsSearchStringInText(par, pos)) {
		if (pos < par->Last() - 1)
			++pos;
		else {
			pos = 0;
			par = par->Next();
		}
	}
	if (par) {
		lt->SetCursor(par, pos);
		return true;
	} else
		return false;
}


// if the string can be found: return true and set the cursor to
// the new position 
// (was: LyXText::SearchBackward(char const* string) in text2.C )
bool LyXFindReplace1::SearchBackward(LyXText * lt)
{
	LyXParagraph * par = lt->cursor.par;
	int pos = lt->cursor.pos;

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
		lt->SetCursor(par, pos);
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
int LyXFindReplace1::CompareChars(char chSearch, char chText)
{
	if (CaseSensitive())
		return (chSearch - chText);
	return (toupper(chSearch) - toupper(chText));
}


// returns true if the search string is at the specified position 
// (Copied from the original "LyXText::IsStringInText" in text2.C )
bool LyXFindReplace1::IsSearchStringInText(LyXParagraph * par,
					   LyXParagraph::size_type pos)
{
	if (!par) return false;

	char chSrch = 0;
	char chText;
	bool fPrevIsSpace = false;
	int iText = 0;
	string::size_type iSrch = 0;
	while (pos + iText < par->Last() && 
	       iSrch < SearchString().length()) {
		chSrch = SearchString()[iSrch];
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

	if (iSrch < SearchString().length())
		return false;

	if (!MatchWord() 
	    || ((pos <= 0 || !IsLetterCharOrDigit(par->GetChar(pos - 1)))
		&& (pos + iText >= par->Last() 
		    || !IsLetterCharOrDigit(par->GetChar(pos + iText))))) {
		iLenSelected = iText;
		return true;
	}

	return false;
}
