/**
 * \file BufferView.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#include "BufferView.h"
#include "BufferView_pimpl.h"

#include "LaTeX.h"
#include "ParagraphParameters.h"
#include "WordLangTuple.h"
#include "buffer.h"
#include "bufferlist.h"
#include "debug.h"
#include "gettext.h"
#include "errorlist.h"
#include "iterators.h"
#include "language.h"
#include "lyxcursor.h"
#include "lyxlex.h"
#include "lyxtext.h"
#include "undo_funcs.h"
#include "changes.h"
#include "paragraph_funcs.h"

#include "frontends/Alert.h"
#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"
#include "frontends/WorkArea.h"
#include "frontends/screen.h"

#include "insets/insetcommand.h" // ChangeRefs
#include "insets/updatableinset.h"

#include "support/FileInfo.h"
#include "support/filetools.h"
#include "support/lyxfunctional.h" // equal_1st_in_pair
#include "support/types.h"
#include "support/lyxalgo.h" // lyx_count

#include <fstream>

extern BufferList bufferlist;

using lyx::pos_type;
using namespace lyx::support;

using std::pair;
using std::endl;
using std::ifstream;
using std::vector;
using std::find;
using std::count_if;


BufferView::BufferView(LyXView * owner, int xpos, int ypos,
		       int width, int height)
	: pimpl_(new Pimpl(this, owner, xpos, ypos, width, height))
{
	text = 0;
}


BufferView::~BufferView()
{
	delete text;
	delete pimpl_;
}


Buffer * BufferView::buffer() const
{
	return pimpl_->buffer_;
}


LyXScreen & BufferView::screen() const
{
	return pimpl_->screen();
}


LyXView * BufferView::owner() const
{
	return pimpl_->owner_;
}


Painter & BufferView::painter() const
{
	return pimpl_->painter();
}


void BufferView::buffer(Buffer * b)
{
	pimpl_->buffer(b);
}


bool BufferView::newFile(string const & fn, string const & tn, bool named)
{
	return pimpl_->newFile(fn, tn, named);
}


bool BufferView::loadLyXFile(string const & fn, bool tl)
{
	return pimpl_->loadLyXFile(fn, tl);
}


void BufferView::reload()
{
	string const fn = buffer()->fileName();
	if (bufferlist.close(buffer(), false))
		loadLyXFile(fn);
}


void BufferView::resize()
{
	if (pimpl_->buffer_)
		pimpl_->resizeCurrentBuffer();
}


void BufferView::repaint()
{
	pimpl_->repaint();
}


bool BufferView::fitCursor()
{
	return pimpl_->fitCursor();
}


void BufferView::update()
{
	pimpl_->update();
}


void BufferView::updateScrollbar()
{
	pimpl_->updateScrollbar();
}


void BufferView::scrollDocView(int value)
{
	pimpl_->scrollDocView(value);
}


void BufferView::redoCurrentBuffer()
{
	pimpl_->redoCurrentBuffer();
}


bool BufferView::available() const
{
	return pimpl_->available();
}


Change const BufferView::getCurrentChange()
{
	return pimpl_->getCurrentChange();
}


void BufferView::beforeChange(LyXText * text)
{
	pimpl_->beforeChange(text);
}


void BufferView::savePosition(unsigned int i)
{
	pimpl_->savePosition(i);
}


void BufferView::restorePosition(unsigned int i)
{
	pimpl_->restorePosition(i);
}


bool BufferView::isSavedPosition(unsigned int i)
{
	return pimpl_->isSavedPosition(i);
}


void BufferView::update(LyXText * text, UpdateCodes f)
{
	pimpl_->update(text, f);
}


void BufferView::update(UpdateCodes f)
{
	pimpl_->update(f);
}


void BufferView::switchKeyMap()
{
	pimpl_->switchKeyMap();
}


void BufferView::insetUnlock()
{
	pimpl_->insetUnlock();
}


int BufferView::workWidth() const
{
	return pimpl_->workarea().workWidth();
}


void BufferView::toggleSelection(bool b)
{
	pimpl_->toggleSelection(b);
}


void BufferView::center()
{
	pimpl_->center();
}


string const BufferView::getClipboard() const
{
	return pimpl_->workarea().getClipboard();
}


void BufferView::stuffClipboard(string const & stuff) const
{
	pimpl_->stuffClipboard(stuff);
}


BufferView::UpdateCodes operator|(BufferView::UpdateCodes uc1,
				  BufferView::UpdateCodes uc2)
{
	return static_cast<BufferView::UpdateCodes>
		(static_cast<int>(uc1) | static_cast<int>(uc2));
}


bool BufferView::dispatch(FuncRequest const & ev)
{
	return pimpl_->dispatch(ev);
}


void BufferView::scroll(int lines)
{
	pimpl_->scroll(lines);
}


// Inserts a file into current document
bool BufferView::insertLyXFile(string const & filen)
	//
	// Copyright CHT Software Service GmbH
	// Uwe C. Schroeder
	//
	// Insert a LyXformat - file into current buffer
	//
	// Moved from lyx_cb.C (Lgb)
{
	if (filen.empty())
		return false;

	string const fname = MakeAbsPath(filen);

	// check if file exist
	FileInfo const fi(fname);

	if (!fi.readable()) {
		string const file = MakeDisplayPath(fname, 50);
		string const text =
			bformat(_("The specified document\n%1$s\ncould not be read."), file);
		Alert::error(_("Could not read document"), text);
		return false;
	}

	beforeChange(text);

	ifstream ifs(fname.c_str());
	if (!ifs) {
		string const file = MakeDisplayPath(fname, 50);
		string const text =
			bformat(_("Could not open the specified document %1$s\n"), file);
		Alert::error(_("Could not open file"), text);
		return false;
	}

	int const c = ifs.peek();

	LyXLex lex(0, 0);
	lex.setStream(ifs);

	bool res = true;

	text->breakParagraph(buffer()->paragraphs);

	if (c == '#') {
		// FIXME: huh ? No we won't !
		lyxerr[Debug::INFO] << "Will insert file with header" << endl;
		res = buffer()->readFile(lex, fname, ParagraphList::iterator(text->cursor.par()));
	} else {
		lyxerr[Debug::INFO] << "Will insert file without header"
				    << endl;
		res = buffer()->readBody(lex, ParagraphList::iterator(text->cursor.par()));
	}

	resize();
	return res;
}


void BufferView::showErrorList(string const & action) const
{
	if (getErrorList().size()) {
		string const title = bformat(_("LyX: %1$s errors (%2$s)"), action, buffer()->fileName());
		owner()->getDialogs().show("errorlist", title);
		pimpl_->errorlist_.clear();
	}
}


ErrorList const &
BufferView::getErrorList() const
{
	return pimpl_->errorlist_;
}


void BufferView::setCursorFromRow(int row)
{
	int tmpid = -1;
	int tmppos = -1;

	buffer()->texrow.getIdFromRow(row, tmpid, tmppos);

	ParagraphList::iterator texrowpar;

	if (tmpid == -1) {
		texrowpar = text->ownerParagraphs().begin();
		tmppos = 0;
	} else {
		texrowpar = buffer()->getParFromID(tmpid).pit();
	}
	text->setCursor(texrowpar, tmppos);
}


bool BufferView::insertInset(InsetOld * inset, string const & lout)
{
	return pimpl_->insertInset(inset, lout);
}


void BufferView::gotoLabel(string const & label)
{
	for (Buffer::inset_iterator it = buffer()->inset_iterator_begin();
	     it != buffer()->inset_iterator_end(); ++it) {
		vector<string> labels;
		it->getLabelList(labels);
		if (find(labels.begin(),labels.end(),label)
		     != labels.end()) {
			beforeChange(text);
			text->setCursor(it.getPar(), it.getPos());
			text->selection.cursor = text->cursor;
			update(text, BufferView::SELECT);
			return;
		}
	}
}


void BufferView::undo()
{
	if (!available())
		return;

	owner()->message(_("Undo"));
	beforeChange(text);
	update(text, BufferView::SELECT);
	if (!textUndo(this))
		owner()->message(_("No further undo information"));
	else
		update(text, BufferView::SELECT);
	switchKeyMap();
}


void BufferView::redo()
{
	if (!available())
		return;

	owner()->message(_("Redo"));
	beforeChange(text);
	update(text, BufferView::SELECT);
	if (!textRedo(this))
		owner()->message(_("No further redo information"));
	else
		update(text, BufferView::SELECT);
	switchKeyMap();
}


// these functions are for the spellchecker
WordLangTuple const BufferView::nextWord(float & value)
{
	if (!available()) {
		value = 1;
		return WordLangTuple();
	}

	return text->selectNextWordToSpellcheck(value);
}


void BufferView::selectLastWord()
{
	if (!available())
		return;

	LyXCursor cur = text->selection.cursor;
	beforeChange(text);
	text->selection.cursor = cur;
	text->selectSelectedWord();
	toggleSelection(false);
	update(text, BufferView::SELECT);
}


void BufferView::endOfSpellCheck()
{
	if (!available()) return;

	beforeChange(text);
	text->selectSelectedWord();
	text->clearSelection();
	update(text, BufferView::SELECT);
}


void BufferView::replaceWord(string const & replacestring)
{
	if (!available())
		return;

	LyXText * tt = getLyXText();
	update(tt, BufferView::SELECT);

	// clear the selection (if there is any)
	toggleSelection(false);
	update(tt, BufferView::SELECT);

	// clear the selection (if there is any)
	toggleSelection(false);
	tt->replaceSelectionWithString(replacestring);

	tt->setSelectionRange(replacestring.length());

	// Go back so that replacement string is also spellchecked
	for (string::size_type i = 0; i < replacestring.length() + 1; ++i) {
		tt->cursorLeft(this);
	}
	update(tt, BufferView::SELECT);

	// FIXME: should be done through LFUN
	buffer()->markDirty();
	fitCursor();
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
		if (text->cursor.pos() < text->cursor.par()->size()
		    && text->cursor.par()->getChar(text->cursor.pos()) ==
		    Paragraph::META_INSET) {
			InsetOld * in = text->cursor.par()->getInset(text->cursor.pos());
			if (inset == in) {
				theLockingInset(inset);
				return true;
			}
		}
		// Then do a deep look of the inset and lock the right one
		int const id = inset->id();
		ParagraphList::iterator pit = buffer()->paragraphs.begin();
		ParagraphList::iterator pend = buffer()->paragraphs.end();
		for (; pit != pend; ++pit) {
			InsetList::iterator it = pit->insetlist.begin();
			InsetList::iterator end = pit->insetlist.end();
			for (; it != end; ++it) {
				if (it->inset == inset) {
					text->setCursorIntern(pit, it->pos);
					theLockingInset(inset);
					return true;
				}
				if (it->inset->getInsetFromID(id)) {
					text->setCursorIntern(pit, it->pos);
					FuncRequest cmd(this, LFUN_INSET_EDIT, "left");
					it->inset->localDispatch(cmd);
					return theLockingInset()->lockInsetInInset(this, inset);
				}
			}
		}
		return false;
	}
	return theLockingInset()->lockInsetInInset(this, inset);
}


bool BufferView::fitLockedInsetCursor(int x, int y, int asc, int desc)
{
	if (theLockingInset() && available()) {
		y += text->cursor.iy() + theLockingInset()->insetInInsetY();
		if (screen().fitManualCursor(this, text, x, y, asc, desc)) {
			updateScrollbar();
			return true;
		}
	}
	return false;
}


void BufferView::hideCursor()
{
	screen().hideCursor();
}


int BufferView::unlockInset(UpdatableInset * inset)
{
	if (!inset)
		return 0;
	if (inset && theLockingInset() == inset) {
		inset->insetUnlock(this);
		theLockingInset(0);
		// make sure we update the combo !
		owner()->setLayout(getLyXText()->cursor.par()->layout()->name());
		// Tell the paragraph dialog that we changed paragraph
		dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
		finishUndo();
		return 0;
	} else if (inset && theLockingInset() &&
		   theLockingInset()->unlockInsetInInset(this, inset)) {
		// Tell the paragraph dialog that we changed paragraph
		dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
		// owner inset has updated the layout combo
		finishUndo();
		return 0;
	}
	return 1;
}


void BufferView::updateInset(InsetOld * inset)
{
	pimpl_->updateInset(inset);
}


bool BufferView::ChangeInsets(InsetOld::Code code,
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
		bool changed_inset = false;
		for (InsetList::iterator it2 = it->insetlist.begin();
		     it2 != it->insetlist.end(); ++it2) {
			if (it2->inset->lyxCode() == code) {
				InsetCommand * inset = static_cast<InsetCommand *>(it2->inset);
				if (inset->getContents() == from) {
					inset->setContents(to);
					changed_inset = true;
				}
			}
		}
		if (changed_inset) {
			need_update = true;

			// FIXME

			// The test it.size()==1 was needed to prevent crashes.
			// How to set the cursor corretly when it.size()>1 ??
			if (it.size() == 1) {
				text->setCursorIntern(it.pit(), 0);
				text->redoParagraphs(text->cursor,
						     boost::next(text->cursor.par()));
				text->partialRebreak();
			}
		}
	}
	text->setCursorIntern(cursor.par(), cursor.pos());
	return need_update;
}


bool BufferView::ChangeRefsIfUnique(string const & from, string const & to)
{
	// Check if the label 'from' appears more than once
	vector<string> labels;
	buffer()->getLabelList(labels);

	if (lyx::count(labels.begin(), labels.end(), from) > 1)
		return false;

	return ChangeInsets(InsetOld::REF_CODE, from, to);
}


bool BufferView::ChangeCitationsIfUnique(string const & from, string const & to)
{
	typedef pair<string, string> StringPair;

	vector<StringPair> keys;
	buffer()->fillWithBibKeys(keys);
	if (count_if(keys.begin(), keys.end(),
		     lyx::equal_1st_in_pair<StringPair>(from))
	    > 1)
		return false;

	return ChangeInsets(InsetOld::CITE_CODE, from, to);
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


LyXText * BufferView::getParentText(InsetOld * inset) const
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


Language const * BufferView::getParentLanguage(InsetOld * inset) const
{
	LyXText * text = getParentText(inset);
	return text->cursor.par()->getFontSettings(buffer()->params,
						   text->cursor.pos()).language();
}


Encoding const * BufferView::getEncoding() const
{
	LyXText * t = getLyXText();
	if (!t)
		return 0;

	LyXCursor const & c = t->cursor;
	LyXFont const font = c.par()->getFont(buffer()->params, c.pos(),
					      outerFont(c.par(), t->ownerParagraphs()));
	return font.language()->encoding();
}


void BufferView::haveSelection(bool sel)
{
	pimpl_->workarea().haveSelection(sel);
}


int BufferView::workHeight() const
{
	return pimpl_->workarea().workHeight();
}
