/**
 * \file BufferView.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braustein
 * \author Lars Gullik Bjønnes
 * \author John Levon
 * \author André Pönitz
 * \author Jürgen Vigna
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "BufferView.h"

#include "buffer.h"
#include "bufferlist.h"
#include "BufferView_pimpl.h"
#include "funcrequest.h"
#include "gettext.h"
#include "iterators.h"
#include "language.h"
#include "lyxlayout.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "texrow.h"
#include "undo.h"
#include "WordLangTuple.h"

#include "frontends/Alert.h"
#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"
#include "frontends/screen.h"
#include "frontends/WorkArea.h"

#include "insets/insetcommand.h" // ChangeRefs
#include "insets/updatableinset.h"

#include "support/filetools.h"
#include "support/lyxalgo.h" // lyx_count

using lyx::support::bformat;
using lyx::support::MakeAbsPath;

using std::find;
using std::string;
using std::vector;


extern BufferList bufferlist;


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


void BufferView::center()
{
	pimpl_->center();
}


int BufferView::top_y() const
{
	return pimpl_->top_y();
}


void BufferView::top_y(int y)
{
	pimpl_->top_y(y);
}


string const BufferView::getClipboard() const
{
	return pimpl_->workarea().getClipboard();
}


void BufferView::stuffClipboard(string const & stuff) const
{
	pimpl_->stuffClipboard(stuff);
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
	BOOST_ASSERT(!filen.empty());

	string const fname = MakeAbsPath(filen);

	beforeChange(text);

	text->breakParagraph(buffer()->paragraphs());

	bool res = buffer()->readFile(fname, text->cursorPar());

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

	buffer()->texrow().getIdFromRow(row, tmpid, tmppos);

	if (tmpid == -1)
		text->setCursor(0, 0);
	else
		text->setCursor(buffer()->getParFromID(tmpid).pit(), tmppos);
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
		it->getLabelList(*buffer(), labels);
		if (find(labels.begin(),labels.end(),label) != labels.end()) {
			beforeChange(text);
			text->setCursor(
				std::distance(text->ownerParagraphs().begin(), it.getPar()),
				it.getPos());
			text->selection.cursor = text->cursor;
			update();
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
	if (!textUndo(this))
		owner()->message(_("No further undo information"));
	update();
	switchKeyMap();
}


void BufferView::redo()
{
	if (!available())
		return;

	owner()->message(_("Redo"));
	beforeChange(text);
	if (!textRedo(this))
		owner()->message(_("No further redo information"));
	update();
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
	update();
}


void BufferView::endOfSpellCheck()
{
	if (!available()) return;

	beforeChange(text);
	text->selectSelectedWord();
	text->clearSelection();
	update();
}


void BufferView::replaceWord(string const & replacestring)
{
	if (!available())
		return;

	LyXText * tt = getLyXText();

	tt->replaceSelectionWithString(replacestring);
	tt->setSelectionRange(replacestring.length());

	// Go back so that replacement string is also spellchecked
	for (string::size_type i = 0; i < replacestring.length() + 1; ++i)
		tt->cursorLeft(this);

	// FIXME: should be done through LFUN
	buffer()->markDirty();
	update();
}


bool BufferView::lockInset(UpdatableInset * inset)
{
	if (!inset)
		return false;

	// don't relock if we're already locked
	if (theLockingInset() == inset)
		return true;

	if (theLockingInset())
		return theLockingInset()->lockInsetInInset(this, inset);

	// first check if it's the inset under the cursor we want lock
	// should be most of the time
	if (text->cursor.pos() < text->cursorPar()->size()
			&& text->cursorPar()->getChar(text->cursor.pos()) ==
			Paragraph::META_INSET) {
		if (inset == text->cursorPar()->getInset(text->cursor.pos())) {
			theLockingInset(inset);
			return true;
		}
	}

	// then do a deep look at the inset and lock the right one
	ParagraphList::iterator pit = buffer()->paragraphs().begin();
	ParagraphList::iterator pend = buffer()->paragraphs().end();
	for (int par = 0; pit != pend; ++pit, ++par) {
		InsetList::iterator it = pit->insetlist.begin();
		InsetList::iterator end = pit->insetlist.end();
		for (; it != end; ++it) {
			if (it->inset == inset) {
				text->setCursorIntern(par, it->pos);
				theLockingInset(inset);
				return true;
			}
		}
	}
	return false;
}


bool BufferView::fitLockedInsetCursor(int x, int y, int asc, int desc)
{
	if (theLockingInset() && available()) {
		y += text->cursor.y() + theLockingInset()->insetInInsetY();
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
		owner()->setLayout(getLyXText()->cursorPar()->layout()->name());
		// Tell the paragraph dialog that we changed paragraph
		dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
		finishUndo();
		return 0;
	}
	if (inset && theLockingInset() &&
		   theLockingInset()->unlockInsetInInset(this, inset)) {
		// Tell the paragraph dialog that we changed paragraph
		dispatch(FuncRequest(LFUN_PARAGRAPH_UPDATE));
		// owner inset has updated the layout combo
		finishUndo();
		return 0;
	}
	return 1;
}


void BufferView::updateInset(InsetOld const * inset)
{
	pimpl_->updateInset(inset);
}


bool BufferView::ChangeRefsIfUnique(string const & from, string const & to)
{
	// Check if the label 'from' appears more than once
	vector<string> labels;
	buffer()->getLabelList(labels);

	if (lyx::count(labels.begin(), labels.end(), from) > 1)
		return false;

	return pimpl_->ChangeInsets(InsetOld::REF_CODE, from, to);
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


Language const * BufferView::getParentLanguage(InsetOld * inset) const
{
	Paragraph const & par = ownerPar(*buffer(), inset);
	return par.getFontSettings(buffer()->params(),
	                           par.getPositionOfInset(inset)).language();
}


Encoding const * BufferView::getEncoding() const
{
	LyXText * text = getLyXText();
	if (!text)
		return 0;

	return text->cursorPar()->getFont(
		buffer()->params(),
		text->cursor.pos(),
		outerFont(text->cursorPar(), text->ownerParagraphs())
	).language()->encoding();
}


void BufferView::haveSelection(bool sel)
{
	pimpl_->workarea().haveSelection(sel);
}


int BufferView::workHeight() const
{
	return pimpl_->workarea().workHeight();
}
