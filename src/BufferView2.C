/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>

#include "BufferView.h"
#include "buffer.h"
#include "lyxcursor.h"
#include "lyxtext.h"
#include "LyXView.h"
#include "bufferlist.h"
#include "lyxscreen.h"
#include "LaTeX.h"
#include "BufferView_pimpl.h"
#include "language.h"
#include "gettext.h"
#include "undo_funcs.h"
#include "debug.h"
#include "iterators.h"

#include "frontends/Alert.h"

#include "insets/insetcommand.h" //ChangeRefs
#include "insets/inseterror.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/lyxfunctional.h" //equal_1st_in_pair
#include "support/types.h"

#include <fstream>
#include <algorithm>

extern BufferList bufferlist;

using lyx::pos_type;

using std::pair;
using std::endl;
using std::ifstream;
using std::vector;
using std::find;
using std::count;
using std::count_if;


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

	string const fname = MakeAbsPath(filen);

	// check if file exist
	FileInfo const fi(fname);

	if (!fi.readable()) {
		Alert::alert(_("Error!"),
			   _("Specified file is unreadable: "),
			   MakeDisplayPath(fname, 50));
		return false;
	}
	
	beforeChange(text);

	ifstream ifs(fname.c_str());
	if (!ifs) {
		Alert::alert(_("Error!"),
			   _("Cannot open specified file: "),
			   MakeDisplayPath(fname, 50));
		return false;
	}
	
	int const c = ifs.peek();
       
	LyXLex lex(0, 0);
	lex.setStream(ifs);

	bool res = true;

	if (c == '#') {
		lyxerr[Debug::INFO] << "Will insert file with header" << endl;
		res = buffer()->readFile(lex, text->cursor.par());
	} else {
		lyxerr[Debug::INFO] << "Will insert file without header" 
				    << endl;
		res = buffer()->readLyXformat2(lex, text->cursor.par());
	}

	resize();
	return res;
}


bool BufferView::removeAutoInsets()
{
	LyXCursor tmpcursor = text->cursor;
	Paragraph * cur_par = tmpcursor.par();
	Paragraph * cur_par_prev = cur_par ? cur_par->previous() : 0;
	Paragraph * cur_par_next = cur_par ? cur_par->next() : 0;
	pos_type cur_pos = tmpcursor.pos();

	bool found = false;

	// Trap the deletion of the paragraph the cursor is in.
	// It should be almost impossible for the new cursor par to be
	// deleted later on in this function.
	// This is the way to segfault this now. Although you may have to do this
	// multiple times: Have an InsetERT with an unknown command in it.
	// View->DVI, move cursor between Error box and InsetERT and hit <Enter>,
	// <down-arrow>, <Enter> again, View->DVI, BANG!
	//
	while ((cur_par_prev || cur_par_next)
	       && text->setCursor(this,
				  cur_par_prev ? cur_par_prev : cur_par_next,
				  0)) {
		// we just removed cur_par so have to fix the "cursor"
		if (cur_par_prev) {
			cur_par = cur_par_prev;
			cur_pos = cur_par->size();
		} else {
			cur_par = cur_par_next;
			cur_pos = 0;
		}
		cur_par_prev = cur_par->previous();
		cur_par_next = cur_par->next();
	}

	ParIterator it = buffer()->par_iterator_begin();
	ParIterator end = buffer()->par_iterator_end();
	for (; it != end; ++it) {
		Paragraph * par = *it;
		Paragraph * par_prev = par ? par->previous() : 0;
		bool removed = false;

		if (text->setCursor(this, par, 0)
		    && cur_par == par_prev) {
			// The previous setCursor line was deleted and that
			// was the cur_par line.  This can only happen if an
			// error box was the sole item on cur_par.
			if (cur_par_prev) {
				// '|' = par, '.' = cur_par, 'E' = error box
				// First step below may occur before while{}
				//  a    |a      a     a    .a
				//  E -> .E -> |.E -> .  -> |b
				// .      b      b    |b
				//  b
				cur_par = cur_par_prev;
				cur_pos = cur_par_prev->size();
				cur_par_prev = cur_par->previous();
				// cur_par_next remains the same 
			} else if (cur_par_next) {
				// First step below may occur before while{}
				// .
				//  E -> |.E -> |.  -> . -> .|a
				//  a      a      a    |a
				cur_par = cur_par_next;
				cur_pos = 0;
				// cur_par_prev remains unset
				cur_par_next = cur_par->next();
			} else {
				// I can't find a way to trigger this
				// so it should be unreachable code
				// unless the buffer is corrupted.
				lyxerr << "BufferView::removeAutoInsets() is bad\n";
			}
		}

		Paragraph::inset_iterator pit = par->inset_iterator_begin();
		Paragraph::inset_iterator pend = par->inset_iterator_end();
		while (pit != pend) {
			if (pit->autoDelete()) {
				removed = true;
				pos_type const pos = pit.getPos();
				
				par->erase(pos);
				if (cur_par == par) {
					if (cur_pos > pos)
						--cur_pos;
				}
			} else {
				++pit;
			}
		}
		if (removed) {
			found = true;
			text->redoParagraph(this);
		}
	}

	text->setCursorIntern(this, cur_par, cur_pos);

	return found;
}


void BufferView::insertErrors(TeXErrors & terr)
{
	// Save the cursor position
	LyXCursor cursor = text->cursor;

	TeXErrors::Errors::const_iterator cit = terr.begin();
	TeXErrors::Errors::const_iterator end = terr.end();
	for (; cit != end; ++cit) {
		string const desctext(cit->error_desc);
		string const errortext(cit->error_text);
		string const msgtxt = desctext + '\n' + errortext;
		int const errorrow = cit->error_in_line;

		// Insert error string for row number
		int tmpid = -1; 
		int tmppos = -1;

		if (buffer()->texrow.getIdFromRow(errorrow, tmpid, tmppos)) {
			buffer()->texrow.increasePos(tmpid, tmppos);
		}
		
		Paragraph * texrowpar = 0;

		if (tmpid == -1) {
			texrowpar = text->ownerParagraph();
			tmppos = 0;
		} else {
			texrowpar = buffer()->getParFromID(tmpid);
		}

		if (texrowpar == 0)
			continue;

		InsetError * new_inset = new InsetError(msgtxt);
		text->setCursorIntern(this, texrowpar, tmppos);
		text->insertInset(this, new_inset);
		text->fullRebreak(this);
	}
	// Restore the cursor position
	text->setCursorIntern(this, cursor.par(), cursor.pos());
}


void BufferView::setCursorFromRow(int row)
{
	int tmpid = -1; 
	int tmppos = -1;

	buffer()->texrow.getIdFromRow(row, tmpid, tmppos);

	Paragraph * texrowpar;

	if (tmpid == -1) {
		texrowpar = text->ownerParagraph();
		tmppos = 0;
	} else {
		texrowpar = buffer()->getParFromID(tmpid);
	}
	text->setCursor(this, texrowpar, tmppos);
}


bool BufferView::insertInset(Inset * inset, string const & lout)
{
	return pimpl_->insertInset(inset, lout);
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
		if (find(labels.begin(),labels.end(),label)
		     != labels.end()) {
			beforeChange(text);
			text->setCursor(this, it.getPar(), it.getPos());
			text->selection.cursor = text->cursor;
			update(text, BufferView::SELECT|BufferView::FITCUR);
			return true;
		}
	}
	return false;
}


void BufferView::menuUndo()
{
	if (available()) {
		owner()->message(_("Undo"));
		hideCursor();
		beforeChange(text);
		update(text, BufferView::SELECT|BufferView::FITCUR);
		if (!textUndo(this))
			owner()->message(_("No further undo information"));
		else
			update(text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		setState();
	}
}


void BufferView::menuRedo()
{
#if 0 // this should not be here (Jug 20011206)
	if (theLockingInset()) {
		owner()->message(_("Redo not yet supported in math mode"));
		return;
	}
#endif
   
	if (available()) {
		owner()->message(_("Redo"));
		hideCursor();
		beforeChange(text);
		update(text, BufferView::SELECT|BufferView::FITCUR);
		if (!textRedo(this))
			owner()->message(_("No further redo information"));
		else
			update(text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		setState();
	}
}


void BufferView::copyEnvironment()
{
	if (available()) {
		text->copyEnvironmentType();
		owner()->message(_("Paragraph environment type copied"));
	}
}


void BufferView::pasteEnvironment()
{
	if (available()) {
		text->pasteEnvironmentType(this);
		owner()->message(_("Paragraph environment type set"));
		update(text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	}
}


void BufferView::copy()
{
	if (available()) {
		text->copySelection(this);
		owner()->message(_("Copy"));
	}
}


void BufferView::cut(bool realcut)
{
	if (available()) {
		hideCursor();
		update(text, BufferView::SELECT|BufferView::FITCUR);
		text->cutSelection(this, true, realcut);
		update(text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		owner()->message(_("Cut"));
	}
}


void BufferView::paste()
{
	if (!available()) return;

	owner()->message(_("Paste"));

	hideCursor();
	// clear the selection
	toggleSelection();
	text->clearSelection();
	update(text, BufferView::SELECT|BufferView::FITCUR);
	
	// paste
	text->pasteSelection(this);
	update(text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	
	// clear the selection 
	toggleSelection();
	text->clearSelection();
	update(text, BufferView::SELECT|BufferView::FITCUR);
}


/* these functions are for the spellchecker */ 
string const BufferView::nextWord(float & value)
{
	if (!available()) {
		value = 1;
		return string();
	}

	return text->selectNextWordToSpellcheck(this, value);
}

  
void BufferView::selectLastWord()
{
	if (!available()) return;
   
	LyXCursor cur = text->selection.cursor;
	hideCursor();
	beforeChange(text);
	text->selection.cursor = cur;
	text->selectSelectedWord(this);
	toggleSelection(false);
	update(text, BufferView::SELECT|BufferView::FITCUR);
}


void BufferView::endOfSpellCheck()
{
	if (!available()) return;
   
	hideCursor();
	beforeChange(text);
	text->selectSelectedWord(this);
	text->clearSelection();
	update(text, BufferView::SELECT|BufferView::FITCUR);
}


void BufferView::replaceWord(string const & replacestring)
{
	if (!available()) return;

	LyXText * tt = getLyXText();
	hideCursor();
	update(tt, BufferView::SELECT|BufferView::FITCUR);
   
	/* clear the selection (if there is any) */ 
	toggleSelection(false);
	update(tt, BufferView::SELECT|BufferView::FITCUR);
   
	/* clear the selection (if there is any) */ 
	toggleSelection(false);
	tt->replaceSelectionWithString(this, replacestring);
   
	tt->setSelectionOverString(this, replacestring);

	// Go back so that replacement string is also spellchecked
	for (string::size_type i = 0; i < replacestring.length() + 1; ++i) {
		tt->cursorLeft(this);
	}
	update(tt, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
}
// End of spellchecker stuff


bool BufferView::lockInset(UpdatableInset * inset)
{
	if (!inset)
		return false;
	// don't relock if we're already locked
	if (theLockingInset() == inset)
		return true;
	if (!theLockingInset()) {
		// first check if it's the inset under the cursor we want lock
		// should be most of the time
		char const c = text->cursor.par()->getChar(text->cursor.pos());
		if (c == Paragraph::META_INSET) {
			Inset * in = text->cursor.par()->getInset(text->cursor.pos());
			if (inset == in) {
				theLockingInset(inset);
				return true;
			}
		}
		// Then do a deep look of the inset and lock the right one
		Paragraph * par = buffer()->paragraph;
		int const id = inset->id();
		while(par) {
			Paragraph::inset_iterator it =
				par->inset_iterator_begin();
			Paragraph::inset_iterator const end =
				par->inset_iterator_end();
			for (; it != end; ++it) {
				if ((*it) == inset) {
					text->setCursorIntern(this, par, it.getPos());
					theLockingInset(inset);
					return true;
				}
				if ((*it)->getInsetFromID(id)) {
					text->setCursorIntern(this, par, it.getPos());
					theLockingInset(static_cast<UpdatableInset *>(*it));
					return theLockingInset()->lockInsetInInset(this, inset);
				}
			}
			par = par->next();
		}
		return false;
	}
	return theLockingInset()->lockInsetInInset(this, inset);
}


void BufferView::showLockedInsetCursor(int x, int y, int asc, int desc)
{
	if (available() && theLockingInset() && !theLockingInset()->nodraw()) {
		LyXCursor cursor = text->cursor;
		Inset * locking_inset = theLockingInset()->getLockingInset();

		if ((cursor.pos() - 1 >= 0) &&
		    cursor.par()->isInset(cursor.pos() - 1) &&
		    (cursor.par()->getInset(cursor.pos() - 1) ==
		     locking_inset))
			text->setCursor(this, cursor,
			                cursor.par(), cursor.pos() - 1);
		LyXScreen::Cursor_Shape shape = LyXScreen::BAR_SHAPE;
		LyXText * txt = getLyXText();
		if (locking_inset->isTextInset() &&
		    locking_inset->lyxCode() != Inset::ERT_CODE &&
		    (txt->real_current_font.language() !=
		     buffer()->params.language
		     || txt->real_current_font.isVisibleRightToLeft()
		     != buffer()->params.language->RightToLeft()))
			shape = (txt->real_current_font.isVisibleRightToLeft())
				? LyXScreen::REVERSED_L_SHAPE
				: LyXScreen::L_SHAPE;
		y += cursor.y() + theLockingInset()->insetInInsetY();
		pimpl_->screen_->showManualCursor(text, x, y, asc, desc,
						  shape);
	}
}


void BufferView::hideLockedInsetCursor()
{
	if (theLockingInset() && available()) {
		pimpl_->screen_->hideCursor();
	}
}


void BufferView::fitLockedInsetCursor(int x, int y, int asc, int desc)
{
	if (theLockingInset() && available()) {
		y += text->cursor.y() + theLockingInset()->insetInInsetY();
		if (pimpl_->screen_->fitManualCursor(text, this, x, y, asc, desc))
			updateScrollbar();
	}
}


int BufferView::unlockInset(UpdatableInset * inset)
{
	if (!inset)
		return 0;
	if (inset && theLockingInset() == inset) {
		inset->insetUnlock(this);
		theLockingInset(0);
		// make sure we update the combo !
		owner()->setLayout(getLyXText()->cursor.par()->getLayout());
		finishUndo();
		return 0;
	} else if (inset && theLockingInset() &&
		   theLockingInset()->unlockInsetInInset(this, inset)) {
		// owner inset has updated the layout combo 
		finishUndo();
		return 0;
	}
	return bufferlist.unlockInset(inset);
}


void BufferView::lockedInsetStoreUndo(Undo::undo_kind kind)
{
	if (!theLockingInset())
		return; // shouldn't happen
	if (kind == Undo::EDIT) // in this case insets would not be stored!
		kind = Undo::FINISH;
	setUndo(this, kind,
	        text->cursor.par(),
	        text->cursor.par()->next());
}


void BufferView::updateInset(Inset * inset, bool mark_dirty)
{
	pimpl_->updateInset(inset, mark_dirty);
}


bool BufferView::ChangeInsets(Inset::Code code,
			      string const & from, string const & to)
{
	bool need_update = false;
	LyXCursor cursor = text->cursor;
	LyXCursor tmpcursor = cursor;
	cursor.par(tmpcursor.par());
	cursor.pos(tmpcursor.pos());

	ParIterator end = buffer()->par_iterator_end();
	for (ParIterator it = buffer()->par_iterator_begin();
	     it != end; ++it) {
		Paragraph * par = *it;
		bool changed_inset = false;
		for (Paragraph::inset_iterator it2 = par->inset_iterator_begin();
		     it2 != par->inset_iterator_end(); ++it2) {
			if ((*it2)->lyxCode() == code) {
				InsetCommand * inset = static_cast<InsetCommand *>(*it2);
				if (inset->getContents() == from) {
					inset->setContents(to);
					changed_inset = true;
				}
			}
		}
		if (changed_inset) {
			need_update = true;
#ifdef WITH_WARNINGS
#warning FIXME
#endif
			// The test it.size()==1 was needed to prevent crashes.
			// How to set the cursor corretly when it.size()>1 ??
			if (it.size() == 1) {
				text->setCursorIntern(this, par, 0);
				text->redoParagraphs(this, text->cursor,
						     text->cursor.par()->next());
				text->fullRebreak(this);
			}
		}
	}
	text->setCursorIntern(this, cursor.par(), cursor.pos());
	return need_update;
}


bool BufferView::ChangeRefsIfUnique(string const & from, string const & to)
{
	// Check if the label 'from' appears more than once
	vector<string> labels = buffer()->getLabelList();
	// count is broken on some systems, so use the HP version
	int res;
	count(labels.begin(), labels.end(), from, res);
	if (res > 1)
		return false;

	return ChangeInsets(Inset::REF_CODE, from, to);
}


bool BufferView::ChangeCitationsIfUnique(string const & from, string const & to)
{

	vector<pair<string,string> > keys = buffer()->getBibkeyList();	
	if (count_if(keys.begin(), keys.end(), 
		     lyx::equal_1st_in_pair<string,string>(from)) 
	    > 1)
		return false;

	return ChangeInsets(Inset::CITE_CODE, from, to);
}


UpdatableInset * BufferView::theLockingInset() const
{
	// If NULL is not allowed we should put an Assert here. (Lgb)
	if (text)
		return text->the_locking_inset;
	return 0;
}


void BufferView::theLockingInset(UpdatableInset * inset)
{
	text->the_locking_inset = inset;
}


LyXText * BufferView::getLyXText() const
{
	if (theLockingInset()) {
		LyXText * txt = theLockingInset()->getLyXText(this, true);
		if (txt)
			return txt;
	}
	return text;
}


LyXText * BufferView::getParentText(Inset * inset) const
{
	if (inset->owner()) {
		LyXText * txt = inset->getLyXText(this);
		inset = inset->owner();
		while (inset && inset->getLyXText(this) == txt)
			inset = inset->owner();
		if (inset)
			return inset->getLyXText(this);
	}
	return text;
}


Language const * BufferView::getParentLanguage(Inset * inset) const
{
	LyXText * text = getParentText(inset);
	return text->cursor.par()->getFontSettings(buffer()->params,
						   text->cursor.pos()).language();
}
