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
#include "bufferparams.h"
#include "BufferView_pimpl.h"
#include "CutAndPaste.h"
#include "debug.h"
#include "funcrequest.h"
#include "gettext.h"
#include "iterators.h"
#include "language.h"
#include "lyxlayout.h"
#include "lyxtext.h"
#include "lyxtextclass.h"
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
#include "insets/insettext.h"

#include "support/filetools.h"
#include "support/lyxalgo.h" // lyx_count

using lyx::support::bformat;
using lyx::support::MakeAbsPath;

using lyx::cap::setSelectionRange;

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


void BufferView::setBuffer(Buffer * b)
{
	pimpl_->setBuffer(b);
}


void BufferView::newFile(string const & fn, string const & tn, bool named)
{
	pimpl_->newFile(fn, tn, named);
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
	text()->breakParagraph(cursor());

	BOOST_ASSERT(cursor().inTexted());
	bool res = buffer()->readFile(fname, cursor().par());
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
		text()->setCursor(cursor(), 0, 0);
	else
		text()->setCursor(cursor(), buffer()->getParFromID(tmpid).pit(),
			tmppos);
}


void BufferView::gotoLabel(string const & label)
{
	for (Buffer::inset_iterator it = buffer()->inset_iterator_begin();
	     it != buffer()->inset_iterator_end(); ++it) {
		vector<string> labels;
		it->getLabelList(*buffer(), labels);
		if (find(labels.begin(),labels.end(),label) != labels.end()) {
			cursor().clearSelection();
			text()->setCursor(cursor(), it.getPar(), it.getPos());
			cursor().resetAnchor();
			update();
			return;
		}
	}
}


void BufferView::hideCursor()
{
	screen().hideCursor();
}


LyXText * BufferView::getLyXText() const
{
	LyXText * text = cursor().innerText();
	BOOST_ASSERT(text);
	return text;
}


Language const * BufferView::getParentLanguage(InsetOld * inset) const
{
	Paragraph const & par = ownerPar(*buffer(), inset);
	return par.getFontSettings(buffer()->params(),
	                           par.getPositionOfInset(inset)).language();
}


void BufferView::haveSelection(bool sel)
{
	pimpl_->workarea().haveSelection(sel);
}


int BufferView::workHeight() const
{
	return pimpl_->workarea().workHeight();
}


LyXText * BufferView::text() const
{
	return buffer() ? &buffer()->text() : 0;
}


void BufferView::setCursor(ParIterator const & par, lyx::pos_type pos)
{
	LCursor & cur = cursor();
	cur.reset(buffer()->inset());
	ParIterator::PosHolder const & positions = par.positions();
	int const last = par.size() - 1;
	for (int i = 0; i < last; ++i)
		(*positions[i].it)->inset->edit(cur, true);
	cur.resetAnchor();
	par.text(*buffer())->setCursor(cur, par.pit(), pos);
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

void BufferView::putSelectionAt(DocumentIterator const & cur,
		      int length, bool backwards)
{
	ParIterator par(cur);

	cursor().clearSelection();

	LyXText * text = par.text(*buffer());
	setCursor(par, cur.pos());
	
	// hack for the chicken and egg problem
	if (par.inset())
		top_y(text->getPar(par.outerPar()).y);
	update();
	text->setCursor(cursor(), cur.par(), cur.pos());
	cursor().updatePos();

	if (length) {
		setSelectionRange(cursor(), length);
		cursor().setSelection();
		if (backwards) {
			DocumentIterator const it = cursor();
			cursor().setCursor(cursor().anchor_, false);
			cursor().anchor_ = it;
		}
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
