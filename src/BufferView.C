/**
 * \file BufferView.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alfredo Braunstein
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
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "iterators.h"
#include "language.h"
#include "lyxlayout.h"
#include "lyxtext.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "PosIterator.h"
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

using std::distance;
using std::find;
using std::string;
using std::swap;
using std::vector;


extern BufferList bufferlist;


BufferView::BufferView(LyXView * owner, int xpos, int ypos,
		       int width, int height)
	: pimpl_(new Pimpl(*this, owner, xpos, ypos, width, height))
{}


BufferView::~BufferView()
{
	delete pimpl_;
}


void BufferView::unsetXSel()
{
	pimpl_->xsel_cache_.set = false;
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

	cursor().clearSelection();
	text()->breakParagraph(buffer()->paragraphs());

	bool res = buffer()->readFile(fname, text()->cursorPar());
	resize();
	return res;
}


void BufferView::showErrorList(string const & action) const
{
	if (getErrorList().size()) {
		string const title = bformat(_("LyX: %1$s errors (%2$s)"),
			action, buffer()->fileName());
		owner()->getDialogs().show("errorlist", title);
		pimpl_->errorlist_.clear();
	}
}


ErrorList const & BufferView::getErrorList() const
{
	return pimpl_->errorlist_;
}


void BufferView::setCursorFromRow(int row)
{
	int tmpid = -1;
	int tmppos = -1;

	buffer()->texrow().getIdFromRow(row, tmpid, tmppos);

	if (tmpid == -1)
		text()->setCursor(0, 0);
	else
		text()->setCursor(buffer()->getParFromID(tmpid).pit(), tmppos);
}


bool BufferView::insertInset(InsetBase * inset, string const & lout)
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
			cursor().clearSelection();
			text()->setCursor(
				distance(text()->paragraphs().begin(), it.getPar()),
				it.getPos());
			cursor().resetAnchor();
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
	cursor().clearSelection();
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
	cursor().clearSelection();
	if (!textRedo(this))
		owner()->message(_("No further redo information"));
	update();
	switchKeyMap();
}


void BufferView::replaceWord(string const & replacestring)
{
	if (!available())
		return;

	LyXText * t = getLyXText();

	t->replaceSelectionWithString(replacestring);
	t->setSelectionRange(replacestring.length());

	// Go back so that replacement string is also spellchecked
	for (string::size_type i = 0; i < replacestring.length() + 1; ++i)
		t->cursorLeft(this);

	// FIXME: should be done through LFUN
	buffer()->markDirty();
	update();
}


void BufferView::hideCursor()
{
	screen().hideCursor();
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


LyXText * BufferView::getLyXText() const
{
	return cursor().innerText();
}


Language const * BufferView::getParentLanguage(InsetOld * inset) const
{
	Paragraph const & par = ownerPar(*buffer(), inset);
	return par.getFontSettings(buffer()->params(),
	                           par.getPositionOfInset(inset)).language();
}


Encoding const * BufferView::getEncoding() const
{
	LyXText * t = getLyXText();
	if (!t)
		return 0;
	CursorSlice const & cur = cursor().innerTextSlice();
	return t->getPar(cur.par())->getFont(
		buffer()->params(), cur.pos(),
		outerFont(t->getPar(cur.par()), t->paragraphs())
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


void BufferView::updateParagraphDialog()
{
	pimpl_->updateParagraphDialog();
}


LyXText * BufferView::text() const
{
	return pimpl_->buffer_ ? &pimpl_->buffer_->text() : 0;
}




/*
if the fitCursor call refers to some point in never-explored-land, then we
don't have y information in insets there, then we cannot even do an update
to get it (because we need the y infomation for setting top_y first). So
this is solved in putSelectionAt with:

- setting top_y to the y of the outerPar (that has good info)
- calling update
- calling cursor().updatePos()
- then call fitCursor()

Ab.
*/

void BufferView::putSelectionAt(PosIterator const & cur,
		      int length, bool backwards)
{
	ParIterator par(cur);

	cursor().clearSelection();

	LyXText * text = par.text(*buffer());
	par.lockPath(this);
	// hack for the chicken and egg problem
	if (par.inset())
		top_y(par.outerPar()->y);
	update();
	text->setCursor(cur.pit(), cur.pos());
	cursor().updatePos();

	if (length) {
		text->setSelectionRange(length);
		cursor().setSelection();
		if (backwards)
			swap(cursor().cursor_, cursor().anchor_);
	}

	fitCursor();
	update();
}


LCursor & BufferView::cursor()
{
	return pimpl_->cursor_;
}


LCursor const & BufferView::cursor() const
{
	return pimpl_->cursor_;
}
