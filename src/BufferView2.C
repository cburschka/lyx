// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include <fstream>

#include "BufferView.h"
#include "buffer.h"
#include "lyxcursor.h"
#include "lyxtext.h"
#include "insets/inseterror.h"
#include "insets/insetinfo.h"
#include "insets/insetspecialchar.h"
#include "LyXView.h"
#include "minibuffer.h"
#include "bufferlist.h"
#include "support/FileInfo.h"
#include "lyxscreen.h"
#include "support/filetools.h"
#include "lyx_gui_misc.h"
#include "LaTeX.h"
#include "BufferView_pimpl.h"

extern BufferList bufferlist;

using std::pair;
using std::endl;
using std::ifstream;

// Inserts a file into current document
bool BufferView::insertLyXFile(string const & filen)
	//
	// Copyright CHT Software Service GmbH
	// Uwe C. Schroeder
	//
	// Insert a Lyxformat - file into current buffer
	//
	// Moved from lyx_cb.C (Lgb)
{
	if (filen.empty()) return false;

	string fname = MakeAbsPath(filen);

	// check if file exist
	FileInfo fi(fname);

	if (!fi.readable()) {
		WriteAlert(_("Error!"),
			   _("Specified file is unreadable: "),
			   MakeDisplayPath(fname, 50));
		return false;
	}
	
	beforeChange();

	ifstream ifs(fname.c_str());
	if (!ifs) {
		WriteAlert(_("Error!"),
			   _("Cannot open specified file: "),
			   MakeDisplayPath(fname, 50));
		return false;
	}
	LyXLex lex(0, 0);
	lex.setStream(ifs);
	char c; ifs.get(c);
	ifs.putback(c);

	bool res = true;

	if (c == '#') {
		lyxerr.debug() << "Will insert file with header" << endl;
		res = buffer()->readFile(lex, text->cursor.par);
	} else {
		lyxerr.debug() << "Will insert file without header" << endl;
		res = buffer()->readLyXformat2(lex, text->cursor.par);
	}

	resize();
	return res;
}

bool BufferView::removeAutoInsets()
{
	LyXParagraph * par = buffer()->paragraph;

	LyXCursor cursor = text->cursor;
	LyXCursor tmpcursor = cursor;
	cursor.par = tmpcursor.par->ParFromPos(tmpcursor.pos);
	cursor.pos = tmpcursor.par->PositionInParFromPos(tmpcursor.pos);

	bool a = false;
	while (par) {
		if (par->AutoDeleteInsets()){
			a = true;
			if (par->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE){
				// this is possible now, since SetCursor takes
				// care about footnotes
				text->SetCursorIntern(par, 0);
				text->RedoParagraphs(text->cursor,
						     text->cursor.par->Next());
				text->FullRebreak();
			}
		}
		par = par->next;
	}
	// avoid forbidden cursor positions caused by error removing
	if (cursor.pos > cursor.par->Last())
		cursor.pos = cursor.par->Last();
	text->SetCursorIntern(cursor.par, cursor.pos);

	return a;
}


void BufferView::insertErrors(TeXErrors & terr)
{
	// Save the cursor position
	LyXCursor cursor = text->cursor;

	// This is drastic, but it's the only fix, I could find. (Asger)
	allFloats(1, 0);
	allFloats(1, 1);

	for (TeXErrors::Errors::const_iterator cit = terr.begin();
	     cit != terr.end();
	     ++cit) {
		string desctext((*cit).error_desc);
		string errortext((*cit).error_text);
		string msgtxt = desctext + '\n' + errortext;
		int errorrow = (*cit).error_in_line;

		// Insert error string for row number
		int tmpid = -1; 
		int tmppos = -1;

		buffer()->texrow.getIdFromRow(errorrow, tmpid, tmppos);

		LyXParagraph * texrowpar = 0;

		if (tmpid == -1) {
			texrowpar = text->FirstParagraph();
			tmppos = 0;
		} else {
			texrowpar = text->GetParFromID(tmpid);
		}

		if (texrowpar == 0)
			continue;

		InsetError * new_inset = new InsetError(msgtxt);
		text->SetCursorIntern(texrowpar, tmppos);
		text->InsertInset(new_inset);
		text->FullRebreak();
	}
	// Restore the cursor position
	text->SetCursorIntern(cursor.par, cursor.pos);
}


void BufferView::setCursorFromRow(int row)
{
	int tmpid = -1; 
	int tmppos = -1;

	buffer()->texrow.getIdFromRow(row, tmpid, tmppos);

	LyXParagraph * texrowpar;

	if (tmpid == -1) {
		texrowpar = text->FirstParagraph();
		tmppos = 0;
	} else {
		texrowpar = text->GetParFromID(tmpid);
	}
	text->SetCursor(texrowpar, tmppos);
}

void BufferView::insertInset(Inset * inset, string const & lout,
			 bool no_table)
{
	// check for table/list in tables
	if (no_table && text->cursor.par->table){
		WriteAlert(_("Impossible Operation!"),
			   _("Cannot insert table/list in table."),
			   _("Sorry."));
		return;
	}
	// not quite sure if we want this...
	text->SetCursorParUndo();
	text->FreezeUndo();
	
	beforeChange();
	if (!lout.empty()) {
		update(-2);
		text->BreakParagraph();
		update(-1);
		
		if (text->cursor.par->Last()) {
			text->CursorLeft();
			
			text->BreakParagraph();
			update(-1);
		}

		pair<bool, LyXTextClass::size_type> lres =
			textclasslist.NumberOfLayout(buffer()->params
						     .textclass, lout);
		LyXTextClass::size_type lay;
		if (lres.first != false) {
			// layout found
			lay = lres.second;
		} else {
			// layout not fount using default "Standard" (0)
			lay = 0;
		}
		 
		text->SetLayout(lay);
		
		text->SetParagraph(0, 0,
				   0, 0,
				   VSpace(VSpace::NONE), VSpace(VSpace::NONE),
				   LYX_ALIGN_LAYOUT, 
				   string(),
				   0);
		update(-1);
		
		text->current_font.setLatex(LyXFont::OFF);
	}
	
	text->InsertInset(inset);
#if 1
	// if we enter a text-inset the cursor should be to the left side
	// of it! This couldn't happen before as Undo was not handled inside
	// inset now after the Undo LyX tries to call inset->Edit(...) again
	// and cannot do this as the cursor is behind the inset and GetInset
	// does not return the inset!
	if (inset->IsTextInset()) {
		if (text->cursor.par->getParDirection()==LYX_DIR_LEFT_TO_RIGHT)
			text->CursorLeft();
		else
			text->CursorRight();
	}
#endif
	update(-1);

	text->UnFreezeUndo();	
}


// Open and lock an updatable inset
void BufferView::open_new_inset(UpdatableInset * new_inset)
{
	beforeChange();
	text->FinishUndo();
	insertInset(new_inset);
	text->CursorLeft();
	update(1);
    	new_inset->Edit(this, 0, 0, 0);
}

/* This is also a buffer property (ale) */
// Not so sure about that. a goto Label function can not be buffer local, just
// think how this will work in a multiwindo/buffer environment, all the
// cursors in all the views showing this buffer will move. (Lgb)
// OK, then no cursor action should be allowed in buffer. (ale)
bool BufferView::gotoLabel(string const & label)

{
        LyXParagraph * par = buffer()->paragraph;
        LyXParagraph::size_type pos;
        Inset * inset;
        while (par) {
                pos = -1;
                while ((inset = par->ReturnNextInsetPointer(pos))){     
                        for (int i = 0; i < inset->GetNumberOfLabels(); ++i) {
				if (label == inset->getLabel(i)) {
					beforeChange();
					text->SetCursor(par, pos);
					text->sel_cursor = text->cursor;
					update(0);
					return true;
				}
			}
                        ++pos;
                } 
                par = par->next;
	}
	return false;
}

void BufferView::allFloats(char flag, char figmar)
{
	if (!available()) return;

	LyXCursor cursor = text->cursor;

	if (!flag && cursor.par->footnoteflag != LyXParagraph::NO_FOOTNOTE
	    && ((figmar 
		 && cursor.par->footnotekind != LyXParagraph::FOOTNOTE 
		 && cursor.par->footnotekind != LyXParagraph::MARGIN)
		|| (!figmar
		    && cursor.par->footnotekind != LyXParagraph::FIG 
		    && cursor.par->footnotekind != LyXParagraph::TAB
 		    && cursor.par->footnotekind != LyXParagraph::WIDE_FIG 
 		    && cursor.par->footnotekind != LyXParagraph::WIDE_TAB
		    && cursor.par->footnotekind != LyXParagraph::ALGORITHM)))
		toggleFloat();
	else
		beforeChange();

	LyXCursor tmpcursor = cursor;
	cursor.par = tmpcursor.par->ParFromPos(tmpcursor.pos);
	cursor.pos = tmpcursor.par->PositionInParFromPos(tmpcursor.pos);

	LyXParagraph *par = buffer()->paragraph;
	while (par) {
		if (flag) {
			if (par->footnoteflag == LyXParagraph::CLOSED_FOOTNOTE
			    && ((figmar 
				 && par->footnotekind != LyXParagraph::FOOTNOTE 
				 && par->footnotekind !=  LyXParagraph::MARGIN)
				|| (!figmar
				    && par->footnotekind != LyXParagraph::FIG 
				    && par->footnotekind != LyXParagraph::TAB
				    && par->footnotekind != LyXParagraph::WIDE_FIG 
				    && par->footnotekind != LyXParagraph::WIDE_TAB
				    && par->footnotekind != LyXParagraph::ALGORITHM
					)
				    )
				) {
				if (par->previous
				    && par->previous->footnoteflag != 
				    LyXParagraph::CLOSED_FOOTNOTE){ /* should be */ 
					text->SetCursorIntern(par->previous,
							      0);
					text->OpenFootnote();
				}
			}
		} else {
			if (par->footnoteflag == LyXParagraph::OPEN_FOOTNOTE
			    && (
				    (figmar 
				     &&
				     par->footnotekind != LyXParagraph::FOOTNOTE 
				     &&
				     par->footnotekind !=  LyXParagraph::MARGIN
					    )
				    ||
				    (!figmar
				     &&
				     par->footnotekind != LyXParagraph::FIG 
				     &&
				     par->footnotekind != LyXParagraph::TAB
				     &&
 				     par->footnotekind != LyXParagraph::WIDE_FIG 
 				     &&
 				     par->footnotekind != LyXParagraph::WIDE_TAB
 				     &&
				     par->footnotekind != LyXParagraph::ALGORITHM
					    )
				    )
				) {
				text->SetCursorIntern(par, 0);
				text->CloseFootnote();
			}
		}
		par = par->next;
	}

	text->SetCursorIntern(cursor.par, cursor.pos);
	redraw();
	fitCursor();
	updateScrollbar();
}


void BufferView::insertNote()
{
	InsetInfo * new_inset = new InsetInfo();
	insertInset(new_inset);
	new_inset->Edit(this, 0, 0, 0);
}


void BufferView::openStuff()
{
	if (available()) {
		owner()->getMiniBuffer()->Set(_("Open/Close..."));
		hideCursor();
		beforeChange();
		update(-2);
		text->OpenStuff();
		update(0);
		setState();
	}
}


void BufferView::toggleFloat()
{
	if (available()) {
		owner()->getMiniBuffer()->Set(_("Open/Close..."));
		hideCursor();
		beforeChange();
		update(-2);
		text->ToggleFootnote();
		update(0);
		setState();
	}
}

void BufferView::menuUndo()
{
	if (available()) {
		owner()->getMiniBuffer()->Set(_("Undo"));
		hideCursor();
		beforeChange();
		update(-2);
		if (!text->TextUndo())
			owner()->getMiniBuffer()->Set(_("No further undo information"));
		else
			update(-1);
		setState();
	}
}


void BufferView::menuRedo()
{
	if (the_locking_inset) {
		owner()->getMiniBuffer()->Set(_("Redo not yet supported in math mode"));
		return;
	}    
   
	if (available()) {
		owner()->getMiniBuffer()->Set(_("Redo"));
		hideCursor();
		beforeChange();
		update(-2);
		if (!text->TextRedo())
			owner()->getMiniBuffer()->Set(_("No further redo information"));
		else
			update(-1);
		setState();
	}
}


void BufferView::hyphenationPoint()
{
	if (available()) {
		hideCursor();
		update(-2);
		InsetSpecialChar * new_inset = 
			new InsetSpecialChar(InsetSpecialChar::HYPHENATION);
		insertInset(new_inset);
	}
}


void BufferView::ldots()
{
	if (available())  {
		hideCursor();
		update(-2);
		InsetSpecialChar * new_inset = 
			new InsetSpecialChar(InsetSpecialChar::LDOTS);
		insertInset(new_inset);
	}
}


void BufferView::endOfSentenceDot()
{
	if (available()) {
		hideCursor();
		update(-2);
		InsetSpecialChar * new_inset = 
			new InsetSpecialChar(InsetSpecialChar::END_OF_SENTENCE);
		insertInset(new_inset);
	}
}


void BufferView::menuSeparator()
{
	if (available()) {
		hideCursor();
		update(-2);
		InsetSpecialChar * new_inset = 
			new InsetSpecialChar(InsetSpecialChar::MENU_SEPARATOR);
		insertInset(new_inset);
	}
}


void BufferView::newline()
{
	if (available()) {
		hideCursor();
		update(-2);
#if 0
		InsetSpecialChar * new_inset =
			new InsetSpecialChar(InsetSpecialChar::NEWLINE);
		insertInset(new_inset);
#else
		text->InsertChar(LyXParagraph::META_NEWLINE);
		update(-1);
#endif
	}
}


void BufferView::protectedBlank()
{
	if (available()) {
		hideCursor();
		update(-2);
#if 1
		InsetSpecialChar * new_inset =
			new InsetSpecialChar(InsetSpecialChar::PROTECTED_SEPARATOR);
		insertInset(new_inset);
#else
		text->InsertChar(LyXParagraph::META_PROTECTED_SEPARATOR);
		update(-1);
#endif
	}
}


void BufferView::hfill()
{
	if (available()) {
		hideCursor();
		update(-2);
		text->InsertChar(LyXParagraph::META_HFILL);
		update(-1);
	}
}

void BufferView::copyEnvironment()
{
	if (available()) {
		text->copyEnvironmentType();
		// clear the selection, even if mark_set
		toggleSelection();
		text->ClearSelection();
		update(-2);
		owner()->getMiniBuffer()->Set(_("Paragraph environment type copied"));
	}
}


void BufferView::pasteEnvironment()
{
	if (available()) {
		text->pasteEnvironmentType();
		owner()->getMiniBuffer()->Set(_("Paragraph environment type set"));
		update(1);
	}
}


void BufferView::copy()
{
	if (available()) {
		text->CopySelection();
		// clear the selection, even if mark_set
		toggleSelection();
		text->ClearSelection();
		update(-2);
		owner()->getMiniBuffer()->Set(_("Copy"));
	}
}

void BufferView::cut()
{
	if (available()) {
		hideCursor();
		update(-2);
		text->CutSelection();
		update(1);
		owner()->getMiniBuffer()->Set(_("Cut"));
	}
}


void BufferView::paste()
{
	if (!available()) return;
	
	owner()->getMiniBuffer()->Set(_("Paste"));
	hideCursor();
	// clear the selection
	toggleSelection();
	text->ClearSelection();
	update(-2);
	
	// paste
	text->PasteSelection();
	update(1);
	
	// clear the selection 
	toggleSelection();
	text->ClearSelection();
	update(-2);
}


void BufferView::gotoNote()
{
	if (!available()) return;
   
	hideCursor();
	beforeChange();
	update(-2);
	LyXCursor tmp;
   
	if (!text->GotoNextNote()) {
		if (text->cursor.pos 
		    || text->cursor.par != text->FirstParagraph()) {
				tmp = text->cursor;
				text->cursor.par = text->FirstParagraph();
				text->cursor.pos = 0;
				if (!text->GotoNextNote()) {
					text->cursor = tmp;
					owner()->getMiniBuffer()->Set(_("No more notes"));
					LyXBell();
				}
			} else {
				owner()->getMiniBuffer()->Set(_("No more notes"));
				LyXBell();
			}
	}
	update(0);
	text->sel_cursor = text->cursor;
}


void BufferView::insertCorrectQuote()
{
	char c;

	if (text->cursor.pos)
		c = text->cursor.par->GetChar(text->cursor.pos - 1);
	else 
		c = ' ';

	insertInset(new InsetQuotes(c, buffer()->params));
}


/* these functions are for the spellchecker */ 
char * BufferView::nextWord(float & value)
{
	if (!available()) {
		value = 1;
		return 0;
	}

	char * string = text->SelectNextWord(value);

	return string;
}

  
void BufferView::selectLastWord()
{
	if (!available()) return;
   
	hideCursor();
	beforeChange();
	text->SelectSelectedWord();
	toggleSelection(false);
	update(0);
}


void BufferView::endOfSpellCheck()
{
	if (!available()) return;
   
	hideCursor();
	beforeChange();
	text->SelectSelectedWord();
	text->ClearSelection();
	update(0);
}


void BufferView::replaceWord(string const & replacestring)
{
	if (!available()) return;

	hideCursor();
	update(-2);
   
	/* clear the selection (if there is any) */ 
	toggleSelection(false);
	update(-2);
   
	/* clear the selection (if there is any) */ 
	toggleSelection(false);
	text->ReplaceSelectionWithString(replacestring.c_str());
   
	text->SetSelectionOverString(replacestring.c_str());

	// Go back so that replacement string is also spellchecked
	for (string::size_type i = 0; i < replacestring.length() + 1; ++i) {
		text->CursorLeftIntern();
	}
	update(1);
}
// End of spellchecker stuff



/* these functions return 1 if an error occured, 
   otherwise 0 */
int BufferView::lockInset(UpdatableInset * inset)
{
	if (!the_locking_inset && inset){
		the_locking_inset = inset;
		return 0;
	}
	return 1;
}


void BufferView::showLockedInsetCursor(long x, long y, int asc, int desc)
{
	if (the_locking_inset && available()) {
		y += text->cursor.y;
		pimpl_->screen->ShowManualCursor(x, y, asc, desc,
					 LyXScreen::BAR_SHAPE);
	}
}


void BufferView::hideLockedInsetCursor()
{
	if (the_locking_inset && available()) {
		pimpl_->screen->HideCursor();
	}
}


void BufferView::fitLockedInsetCursor(long x, long y, int asc, int desc)
{
	if (the_locking_inset && available()){
		y += text->cursor.y;
		if (pimpl_->screen->FitManualCursor(x, y, asc, desc))
			updateScrollbar();
	}
}


int BufferView::unlockInset(UpdatableInset * inset)
{
	if (inset && the_locking_inset == inset) {
		inset->InsetUnlock(this);
		the_locking_inset = 0;
		text->FinishUndo();
		return 0;
	} else if (inset && the_locking_inset &&
		   the_locking_inset->UnlockInsetInInset(this, inset)) {
		text->FinishUndo();
		return 0;
	}
	return bufferlist.unlockInset(inset);
}


void BufferView::lockedInsetStoreUndo(Undo::undo_kind kind)
{
	if (!the_locking_inset)
		return; // shouldn't happen
	if (kind == Undo::EDIT) // in this case insets would not be stored!
		kind = Undo::FINISH;
	text->SetUndo(kind,
		      text->cursor.par->
		      ParFromPos(text->cursor.pos)->previous, 
		      text->cursor.par->
		      ParFromPos(text->cursor.pos)->next);
}


void BufferView::updateInset(Inset * inset, bool mark_dirty)
{
	if (!inset)
		return;

	// first check for locking insets
	if (the_locking_inset) {
		if (the_locking_inset == inset) {
			if (text->UpdateInset(inset)){
				update();
				if (mark_dirty){
					if (buffer()->isLyxClean())
						owner()->getMiniBuffer()->
							setTimer(4);
					buffer()->markDirty();
				}
				updateScrollbar();
				return;
			}
		} else if (the_locking_inset->UpdateInsetInInset(this,inset)) {
			if (text->UpdateInset(the_locking_inset)) {
				update();
				if (mark_dirty){
					if (buffer()->isLyxClean())
						owner()->getMiniBuffer()->
							setTimer(4);
					buffer()->markDirty();
				}
				updateScrollbar();
				return;
			}
		}
	}
  
	// then check the current buffer
	if (available()) {
		hideCursor();
		update(-3);
		if (text->UpdateInset(inset)){
			if (mark_dirty)
				update(1);
			else 
				update(3);
			return;
		}
	}
}
