/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 * 	 
 *           Copyright 1995 Matthias Ettrich,
 *           Copyright 1995-1999 The LyX Team.
 *
 * ======================================================*/

#include <config.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

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
#include "lyxscreen.h"
#include "debug.h"
#include "lyxtext.h"
#include "gettext.h"
#include "LyXView.h"
#include "lyx_gui_misc.h"
#include "minibuffer.h"

extern BufferView *current_view; // called too many times in this file...
extern MiniBuffer *minibuffer;

// Maximum length copied from the current selection to the search string
const int LYXSEARCH_MAXLEN =  128;

// function prototypes

bool IsLetterCharOrDigit(char ch);

// If nothing selected, select the word at the cursor.
// Returns the current selection
// Note: this function should be in LyXText!
string const GetSelectionOrWordAtCursor(LyXText *lt);

// Returns the current selection. If nothing is selected or if the selection
// spans 2 paragraphs, an empty string is returned.
string const GetCurrentSelectionAsString(LyXText *lt);

// This is a copy of SetSelectionOverString from text.C
// It does the same, but uses only the length as a parameter
void SetSelectionOverLenChars(LyXText *lt, int len);

//-------------------------------------------------------------

bool IsLetterCharOrDigit(char ch)
{
	return IsLetterChar(ch) || isdigit(ch);
}


// Returns the current selection. If nothing is selected or if the selection
// spans 2 paragraphs, an empty string is returned.
string const GetCurrentSelectionAsString(LyXText *lt) 
{
	LyXParagraph 	*par;
	int 		pos;
	int		endpos;
	int		i;
	char		sz[LYXSEARCH_MAXLEN];
	char		ch;
	bool		fPrevIsSpace;

	sz[0] = 0;
	par = lt->cursor.par;
	if (lt->selection && (lt->sel_cursor.par == par)) {
		// (selected) and (begin/end in same paragraph)
		pos = lt->sel_start_cursor.pos;
		endpos = lt->sel_end_cursor.pos;
		i = 0;
		fPrevIsSpace = false;
		while ((i < LYXSEARCH_MAXLEN-2) && 
			(pos < par->Last()) && (pos < endpos)) {
			ch = par->GetChar(pos);

			//HB??: Maybe (ch <= ' ') 
			if ((ch == ' ') || ((unsigned char)ch <= LYX_META_INSET)) {
				// consecutive spaces --> 1 space char
				if (fPrevIsSpace) {
					pos++;		// Next text pos
					continue;	// same search pos
				}
				sz[i] = ' ';
				fPrevIsSpace = true;
			} else {
				sz[i] = ch;
				fPrevIsSpace = false;
			}
			pos++;
			i++;
		} 
		sz[i] = 0;
	}
	return string(sz);
}


// If nothing selected, select the word at the cursor.
// Returns the current selection
string const GetSelectionOrWordAtCursor(LyXText *lt) 
{
	lt->SelectWordWhenUnderCursor();
	return GetCurrentSelectionAsString(lt);
}


// This is a copy of SetSelectionOverString from text.C
// It does the same, but uses only the length as a parameter
void SetSelectionOverLenChars(LyXText *lt, int len)
{
	lt->sel_cursor = lt->cursor;
	int i;
	for (i=0; i < len; i++)
		lt->CursorRight();
	lt->SetSelection();
}


//------------------------------

void LyXFindReplace1::StartSearch()
{
	LyXFindReplace0::StartSearch();
	SetReplaceEnabled(!current_view->currentBuffer()->isReadonly());
	searchForward = true;
	if (lsSearch.empty()) 
		SetSearchString(GetSelectionOrWordAtCursor(current_view->currentBuffer()->text));
}	


// TODO?: the user can insert multiple spaces with this routine (1999-01-11, dnaber)
void LyXFindReplace1::SearchReplaceCB()
{
	LyXText		*ltCur;

	if (!current_view->getScreen())
		return;
	if (current_view->currentBuffer()->isReadonly())
	        return;
	
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

	current_view->getScreen()->HideCursor();
	current_view->currentBuffer()->update(-2);

	ltCur = current_view->currentBuffer()->text;	
	if (ltCur->selection) {
		// clear the selection (if there is any) 
		current_view->getScreen()->ToggleSelection(false);
		current_view->currentBuffer()->text->
			ReplaceSelectionWithString(replacestring.c_str());
		current_view->currentBuffer()->text->
			SetSelectionOverString(replacestring.c_str());
		current_view->currentBuffer()->update(1);
	}
	
	// jump to next match:
	SearchCB( searchForward );
}


// replaces all occurences of a string (1999-01-15, dnaber@mini.gt.owl.de)
void LyXFindReplace1::SearchReplaceAllCB()
{
	LyXText		*ltCur;

	if (!current_view->getScreen())
		return;
	if (current_view->currentBuffer()->isReadonly())
	        return;

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

	current_view->getScreen()->HideCursor();

	// start at top
	current_view->currentBuffer()->text->ClearSelection();
	current_view->currentBuffer()->text->CursorTop();

	int replace_count = 0;
	do {
		ltCur = current_view->currentBuffer()->text;	
		if (ltCur->selection) {
			current_view->currentBuffer()->update(-2);
			current_view->getScreen()->ToggleSelection(false);
			current_view->currentBuffer()->text->
				ReplaceSelectionWithString(replacestring.c_str());
			current_view->currentBuffer()->text->
				SetSelectionOverString(replacestring.c_str());
			current_view->currentBuffer()->update(1); 
			replace_count++;
		}
	} while( SearchCB(true) );

	if( replace_count == 0 ) {
		LyXBell();	
		minibuffer->Set(_("String not found!"));
	} else {
		if( replace_count == 1 ) {
			minibuffer->Set(_("1 string has been replaced."));
		} else {
			string str;
			str += replace_count;
			str += _(" strings have been replaced.");
			minibuffer->Set(str);
		}
	}
}


bool LyXFindReplace1::SearchCB(bool fForward)
{
	LyXText		*ltCur;
	bool result;

	// store search direction
	searchForward = fForward;
	
	if (!current_view->getScreen())
		return(false);
   
	current_view->getScreen()->HideCursor();
	current_view->currentBuffer()->update(-2);
	ltCur = current_view->currentBuffer()->text;
	if (ltCur->selection) 
		ltCur->cursor = fForward ? ltCur->sel_end_cursor :
                                                 ltCur->sel_start_cursor;

	ReInitFromForm();
	iLenSelected = SearchString().length();
   
	if (!ValidSearchData() ||
	    (fForward ? SearchForward(ltCur) : SearchBackward(ltCur))) {
		current_view->currentBuffer()->update(-2);

		// clear the selection (if there is any) 
		current_view->getScreen()->ToggleSelection();
		current_view->currentBuffer()->text->ClearSelection();

		// set the new selection 
		SetSelectionOverLenChars(current_view->currentBuffer()->text, iLenSelected);
		current_view->getScreen()->ToggleSelection(false);
		minibuffer->Set(_("Found."));
		result = true;
	} else {
		LyXBell();
		minibuffer->Set(_("String not found!"));
		result = false;
	}
   
	if (current_view->getWorkArea()->focus)
		current_view->getScreen()->ShowCursor();

	return(result);
}


// if the string can be found: return true and set the cursor to
// the new position 
// (was: LyXText::SearchForward(char const* string) in text2.C )
bool LyXFindReplace1::SearchForward(LyXText *lt)
{
	LyXParagraph *par;
	int pos;

	par = lt->cursor.par;
	pos = lt->cursor.pos;

	while (par && !IsSearchStringInText(par,pos)) {
		if (pos<par->Last()-1)
			pos++;
		else {
			pos = 0;
			par = par->Next();
		}
	}
	if (par) {
		lt->SetCursor(par,pos);
		return true;
	} else
		return false;
}


// if the string can be found: return true and set the cursor to
// the new position 
// (was: LyXText::SearchBackward(char const* string) in text2.C )
bool LyXFindReplace1::SearchBackward(LyXText *lt)
{
	LyXParagraph *par = lt->cursor.par;
	int pos = lt->cursor.pos;

	do {
		if (pos>0)
			pos--;
		else {
			// We skip empty paragraphs (Asger)
			do {
				par = par->Previous();
				if (par)
					pos = par->Last()-1;
			} while (par && pos<0);
		}
	} while (par && !IsSearchStringInText(par,pos));
  
	if (par) {
		lt->SetCursor(par,pos);
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
	return (toupper((unsigned char) chSearch) - toupper((unsigned char) chText));
}


// returns true if the search string is at the specified position 
// (Copied from the original "LyXText::IsStringInText" in text2.C )
bool LyXFindReplace1::IsSearchStringInText(LyXParagraph *par, int pos)
{
	char		chSrch = 0;
	char		chText;
	bool		fPrevIsSpace;
	int		iText;
	string::size_type iSrch;

	if (!par) 
		return false;

	fPrevIsSpace = false;
	iText = 0; iSrch = 0;
	while (pos+iText < par->Last() && 
	       iSrch < SearchString().length()) {
		chSrch = SearchString()[iSrch];
		chText = par->GetChar(pos+iText);
// Why was this code there ??? Insets are *not* spaces!
// It seems that there is some code here to handle multiple spaces. I
// wonder why this is useful...  (JMarc)
// 		if ((chText == ' ') || 
// 		    ((unsigned char)chText <= LYX_META_INSET)) 
 		if (chText == ' ') {
			if (fPrevIsSpace) {
				iText++;  // next Text pos
				continue; // same search pos
			}
// 			chText = ' ';				
			fPrevIsSpace = true;
		} else
			fPrevIsSpace = false;
		if (CompareChars(chSrch, chText) != 0)
			break;
		
		iSrch++;
		iText++;
	}

	if (iSrch < SearchString().length())
		return false;

	if (!MatchWord() 
	    || ((pos <= 0 || !IsLetterCharOrDigit(par->GetChar(pos-1)))
		&& (pos+iText >= par->Last() 
		    || !IsLetterCharOrDigit(par->GetChar(pos + iText))))) {
		iLenSelected = iText;
		return true;
	}

	return false;
}
