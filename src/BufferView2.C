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
#include <algorithm>

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
#include "insets/insetcommand.h" //ChangeRefs

extern BufferList bufferlist;

using std::pair;
using std::endl;
using std::ifstream;
using std::vector;
using std::find;

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
		res = buffer()->readFile(lex, text->cursor.par());
	} else {
		lyxerr.debug() << "Will insert file without header" << endl;
		res = buffer()->readLyXformat2(lex, text->cursor.par());
	}

	resize();
	return res;
}

bool BufferView::removeAutoInsets()
{
	LyXParagraph * par = buffer()->paragraph;

	LyXCursor tmpcursor = text->cursor;
	LyXCursor cursor;

	bool a = false;
	while (par) {
		// this has to be done before the delete
		if (par->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE)
			text->SetCursor(this, cursor, par, 0);
		if (par->AutoDeleteInsets()){
			a = true;
			if (par->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE){
				text->RedoParagraphs(this, cursor,
						     cursor.par()->Next());
				text->FullRebreak(this);
			}
		}
		par = par->next;
	}
	// avoid forbidden cursor positions caused by error removing
	if (tmpcursor.pos() > tmpcursor.par()->Last())
		tmpcursor.pos(tmpcursor.par()->Last());
	text->SetCursorIntern(this, tmpcursor.par(), tmpcursor.pos());

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
		text->SetCursorIntern(this, texrowpar, tmppos);
		text->InsertInset(this, new_inset);
		text->FullRebreak(this);
	}
	// Restore the cursor position
	text->SetCursorIntern(this, cursor.par(), cursor.pos());
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
	text->SetCursor(this, texrowpar, tmppos);
}

bool BufferView::insertInset(Inset * inset, string const & lout,
			 bool no_table)
{
	// if we are in a locking inset we should try to insert the
	// inset there otherwise this is a illegal function now
	if (the_locking_inset) {
		if (the_locking_inset->InsertInsetAllowed(inset) &&
		    the_locking_inset->InsertInset(this, inset))
			return true;
		return false;
	}

#ifndef NEW_TABULAR
	// check for table/list in tables
	if (no_table && text->cursor.par()->table){
		WriteAlert(_("Impossible Operation!"),
			   _("Cannot insert table/list in table."),
			   _("Sorry."));
		return false;
	}
#endif

	// not quite sure if we want this...
	text->SetCursorParUndo(buffer());
	text->FreezeUndo();
	
	beforeChange();
	if (!lout.empty()) {
		update(BufferView::SELECT|BufferView::FITCUR);
		text->BreakParagraph(this);
		update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		
		if (text->cursor.par()->Last()) {
			text->CursorLeft(this);
			
			text->BreakParagraph(this);
			update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
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
		 
		text->SetLayout(this, lay);
		
		text->SetParagraph(this, 0, 0,
				   0, 0,
				   VSpace(VSpace::NONE), VSpace(VSpace::NONE),
				   LYX_ALIGN_LAYOUT, 
				   string(),
				   0);
		update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		
		text->current_font.setLatex(LyXFont::OFF);
	}
	
	text->InsertInset(this, inset);
#if 1
	// if we enter a text-inset the cursor should be to the left side
	// of it! This couldn't happen before as Undo was not handled inside
	// inset now after the Undo LyX tries to call inset->Edit(...) again
	// and cannot do this as the cursor is behind the inset and GetInset
	// does not return the inset!
	if (inset->IsTextInset()) {
		if (text->cursor.par()->isRightToLeftPar(buffer()->params))
			text->CursorRight(this);
		else
			text->CursorLeft(this);
	}
#endif
	update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);

	text->UnFreezeUndo();
	return true;
}


// Open and lock an updatable inset
void BufferView::open_new_inset(UpdatableInset * new_inset)
{
	beforeChange();
	text->FinishUndo();
	insertInset(new_inset);
	text->CursorLeft(this);
	update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
    	new_inset->Edit(this, 0, 0, 0);
}

/* This is also a buffer property (ale) */
// Not so sure about that. a goto Label function can not be buffer local, just
// think how this will work in a multiwindo/buffer environment, all the
// cursors in all the views showing this buffer will move. (Lgb)
// OK, then no cursor action should be allowed in buffer. (ale)
bool BufferView::gotoLabel(string const & label)

{
	for (Buffer::inset_iterator it = buffer()->inset_iterator_begin();
	     it != buffer()->inset_iterator_end(); ++it) {
		vector<string> labels = (*it)->getLabelList();
		if ( find(labels.begin(),labels.end(),label)
		     != labels.end()) {
			beforeChange();
			text->SetCursor(this, it.getPar(), it.getPos());
			text->sel_cursor = text->cursor;
			update(BufferView::SELECT|BufferView::FITCUR);
			return true;
		}
	}
	return false;
}

void BufferView::allFloats(char flag, char figmar)
{
	if (!available()) return;

	LyXCursor cursor = text->cursor;

	if (!flag && cursor.par()->footnoteflag != LyXParagraph::NO_FOOTNOTE
	    && ((figmar 
		 && cursor.par()->footnotekind != LyXParagraph::FOOTNOTE 
		 && cursor.par()->footnotekind != LyXParagraph::MARGIN)
		|| (!figmar
		    && cursor.par()->footnotekind != LyXParagraph::FIG 
		    && cursor.par()->footnotekind != LyXParagraph::TAB
 		    && cursor.par()->footnotekind != LyXParagraph::WIDE_FIG 
 		    && cursor.par()->footnotekind != LyXParagraph::WIDE_TAB
		    && cursor.par()->footnotekind != LyXParagraph::ALGORITHM)))
		toggleFloat();
	else
		beforeChange();

	LyXCursor tmpcursor = cursor;
	cursor.par(tmpcursor.par()->ParFromPos(tmpcursor.pos()));
	cursor.pos(tmpcursor.par()->PositionInParFromPos(tmpcursor.pos()));

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
					text->SetCursorIntern(this, 
							      par->previous,
							      0);
					text->OpenFootnote(this);
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
				text->SetCursorIntern(this, par, 0);
				text->CloseFootnote(this);
			}
		}
		par = par->next;
	}

	text->SetCursorIntern(this, cursor.par(), cursor.pos());
	redraw();
	fitCursor();
	//updateScrollbar();
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
		update(BufferView::SELECT|BufferView::FITCUR);
		text->OpenStuff(this);
		update(BufferView::SELECT|BufferView::FITCUR);
		setState();
	}
}


void BufferView::toggleFloat()
{
	if (available()) {
		owner()->getMiniBuffer()->Set(_("Open/Close..."));
		hideCursor();
		beforeChange();
		update(BufferView::SELECT|BufferView::FITCUR);
		text->ToggleFootnote(this);
		update(BufferView::SELECT|BufferView::FITCUR);
		setState();
	}
}

void BufferView::menuUndo()
{
	if (available()) {
		owner()->getMiniBuffer()->Set(_("Undo"));
		hideCursor();
		beforeChange();
		update(BufferView::SELECT|BufferView::FITCUR);
		if (!text->TextUndo(this))
			owner()->getMiniBuffer()->Set(_("No further undo information"));
		else
			update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
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
		update(BufferView::SELECT|BufferView::FITCUR);
		if (!text->TextRedo(this))
			owner()->getMiniBuffer()->Set(_("No further redo information"));
		else
			update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		setState();
	}
}


void BufferView::hyphenationPoint()
{
	if (available()) {
		hideCursor();
		update(BufferView::SELECT|BufferView::FITCUR);
		InsetSpecialChar * new_inset = 
			new InsetSpecialChar(InsetSpecialChar::HYPHENATION);
		insertInset(new_inset);
	}
}


void BufferView::ldots()
{
	if (available())  {
		hideCursor();
		update(BufferView::SELECT|BufferView::FITCUR);
		InsetSpecialChar * new_inset = 
			new InsetSpecialChar(InsetSpecialChar::LDOTS);
		insertInset(new_inset);
	}
}


void BufferView::endOfSentenceDot()
{
	if (available()) {
		hideCursor();
		update(BufferView::SELECT|BufferView::FITCUR);
		InsetSpecialChar * new_inset = 
			new InsetSpecialChar(InsetSpecialChar::END_OF_SENTENCE);
		insertInset(new_inset);
	}
}


void BufferView::menuSeparator()
{
	if (available()) {
		hideCursor();
		update(BufferView::SELECT|BufferView::FITCUR);
		InsetSpecialChar * new_inset = 
			new InsetSpecialChar(InsetSpecialChar::MENU_SEPARATOR);
		insertInset(new_inset);
	}
}


void BufferView::newline()
{
	if (available()) {
		hideCursor();
		update(BufferView::SELECT|BufferView::FITCUR);
		text->InsertChar(this, LyXParagraph::META_NEWLINE);
		update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	}
}


void BufferView::protectedBlank()
{
	if (available()) {
		hideCursor();
		update(BufferView::SELECT|BufferView::FITCUR);
		InsetSpecialChar * new_inset =
			new InsetSpecialChar(InsetSpecialChar::PROTECTED_SEPARATOR);
		insertInset(new_inset);
	}
}


void BufferView::hfill()
{
	if (available()) {
		hideCursor();
		update(BufferView::SELECT|BufferView::FITCUR);
		text->InsertChar(this, LyXParagraph::META_HFILL);
		update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	}
}

void BufferView::copyEnvironment()
{
	if (available()) {
		text->copyEnvironmentType();
		// clear the selection, even if mark_set
		toggleSelection();
		text->ClearSelection();
		update(BufferView::SELECT|BufferView::FITCUR);
		owner()->getMiniBuffer()->Set(_("Paragraph environment type copied"));
	}
}


void BufferView::pasteEnvironment()
{
	if (available()) {
		text->pasteEnvironmentType(this);
		owner()->getMiniBuffer()->Set(_("Paragraph environment type set"));
		update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	}
}


void BufferView::copy()
{
	if (available()) {
		text->CopySelection(this);
		// clear the selection, even if mark_set
		toggleSelection();
		text->ClearSelection();
		update(BufferView::SELECT|BufferView::FITCUR);
		owner()->getMiniBuffer()->Set(_("Copy"));
	}
}

void BufferView::cut()
{
	if (available()) {
		hideCursor();
		update(BufferView::SELECT|BufferView::FITCUR);
		text->CutSelection(this);
		update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
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
	update(BufferView::SELECT|BufferView::FITCUR);
	
	// paste
	text->PasteSelection(this);
	update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	
	// clear the selection 
	toggleSelection();
	text->ClearSelection();
	update(BufferView::SELECT|BufferView::FITCUR);
}


void BufferView::gotoNote()
{
	if (!available()) return;
   
	hideCursor();
	beforeChange();
	update(BufferView::SELECT|BufferView::FITCUR);
	LyXCursor tmp;
   
	if (!text->GotoNextNote(this)) {
		if (text->cursor.pos() 
		    || text->cursor.par() != text->FirstParagraph()) {
				tmp = text->cursor;
				text->cursor.par(text->FirstParagraph());
				text->cursor.pos(0);
				if (!text->GotoNextNote(this)) {
					text->cursor = tmp;
					owner()->getMiniBuffer()->Set(_("No more notes"));
					LyXBell();
				}
			} else {
				owner()->getMiniBuffer()->Set(_("No more notes"));
				LyXBell();
			}
	}
	update(BufferView::SELECT|BufferView::FITCUR);
	text->sel_cursor = text->cursor;
}


void BufferView::insertCorrectQuote()
{
	char c;

	if (text->cursor.pos())
		c = text->cursor.par()->GetChar(text->cursor.pos() - 1);
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

	char * string = text->SelectNextWord(this, value);

	return string;
}

  
void BufferView::selectLastWord()
{
	if (!available()) return;
   
	hideCursor();
	beforeChange();
	text->SelectSelectedWord(this);
	toggleSelection(false);
	update(BufferView::SELECT|BufferView::FITCUR);
}


void BufferView::endOfSpellCheck()
{
	if (!available()) return;
   
	hideCursor();
	beforeChange();
	text->SelectSelectedWord(this);
	text->ClearSelection();
	update(BufferView::SELECT|BufferView::FITCUR);
}


void BufferView::replaceWord(string const & replacestring)
{
	if (!available()) return;

	hideCursor();
	update(BufferView::SELECT|BufferView::FITCUR);
   
	/* clear the selection (if there is any) */ 
	toggleSelection(false);
	update(BufferView::SELECT|BufferView::FITCUR);
   
	/* clear the selection (if there is any) */ 
	toggleSelection(false);
	text->ReplaceSelectionWithString(this, replacestring.c_str());
   
	text->SetSelectionOverString(this, replacestring.c_str());

	// Go back so that replacement string is also spellchecked
	for (string::size_type i = 0; i < replacestring.length() + 1; ++i) {
		text->CursorLeftIntern(this);
	}
	update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
}
// End of spellchecker stuff


bool BufferView::lockInset(UpdatableInset * inset)
{
	if (!the_locking_inset && inset) {
		the_locking_inset = inset;
		return true;
	} else if (inset) {
	    return the_locking_inset->LockInsetInInset(this, inset);
	}
	return false;
}


void BufferView::showLockedInsetCursor(long x, long y, int asc, int desc)
{
	if (the_locking_inset && available()) {
		LyXCursor cursor = text->cursor;
		if ((cursor.pos() - 1 >= 0) &&
		    (cursor.par()->GetChar(cursor.pos() - 1) ==
		     LyXParagraph::META_INSET) &&
		    (cursor.par()->GetInset(cursor.pos() - 1) ==
		     the_locking_inset->GetLockingInset()))
			text->SetCursor(this, cursor,
					cursor.par(), cursor.pos() - 1);
		y += cursor.y() + the_locking_inset->InsetInInsetY();
		pimpl_->screen_->ShowManualCursor(x, y, asc, desc,
					 LyXScreen::BAR_SHAPE);
	}
}


void BufferView::hideLockedInsetCursor()
{
	if (the_locking_inset && available()) {
		pimpl_->screen_->HideCursor();
	}
}


void BufferView::fitLockedInsetCursor(long x, long y, int asc, int desc)
{
	if (the_locking_inset && available()){
		y += text->cursor.y() + the_locking_inset->InsetInInsetY();
		if (pimpl_->screen_->FitManualCursor(text, x, y, asc, desc))
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
	text->SetUndo(buffer(), kind,
		      text->cursor.par()->
		      ParFromPos(text->cursor.pos())->previous, 
		      text->cursor.par()->
		      ParFromPos(text->cursor.pos())->next);
}


void BufferView::updateInset(Inset * inset, bool mark_dirty)
{
	if (!inset)
		return;

	// first check for locking insets
	if (the_locking_inset) {
		if (the_locking_inset == inset) {
			if (text->UpdateInset(this, inset)){
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
			if (text->UpdateInset(this, the_locking_inset)) {
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
		update(BufferView::UPDATE);
		if (text->UpdateInset(this, inset)){
			if (mark_dirty)
				update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			else 
				update(SELECT);
			return;
		}
	}
}

bool BufferView::ChangeRefs(string const & from, string const & to)
{
	bool flag = false;
	LyXParagraph * par = buffer()->paragraph;
	LyXCursor cursor = text->cursor;
	LyXCursor tmpcursor = cursor;
	cursor.par(tmpcursor.par()->ParFromPos(tmpcursor.pos()));
	cursor.pos(tmpcursor.par()->PositionInParFromPos(tmpcursor.pos()));

	while (par) {
		bool flag2 = false;
		for (LyXParagraph::inset_iterator it = par->inset_iterator_begin();
		     it != par->inset_iterator_end(); ++it) {
			if ((*it)->LyxCode() == Inset::REF_CODE) {
				InsetCommand * inset = static_cast<InsetCommand *>(*it);
				if (inset->getContents() == from) {
					inset->setContents(to);
					flag2 = true;
				}
			}
		}
		if (flag2) {
			flag = true;
			if (par->footnoteflag != LyXParagraph::CLOSED_FOOTNOTE){
				// this is possible now, since SetCursor takes
				// care about footnotes
				text->SetCursorIntern(this, par, 0);
				text->RedoParagraphs(this, text->cursor,
						     text->cursor.par()->Next());
				text->FullRebreak(this);
			}
		}
		par = par->next;
	}
	text->SetCursorIntern(this, cursor.par(), cursor.pos());
	return flag;
}
