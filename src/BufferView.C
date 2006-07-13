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
#include "coordcache.h"
#include "debug.h"
#include "funcrequest.h"
#include "FuncStatus.h"
#include "gettext.h"
#include "insetiterator.h"
#include "language.h"
#include "lyxlayout.h"
#include "lyxtext.h"
#include "lyxtextclass.h"
#include "paragraph.h"
#include "paragraph_funcs.h"
#include "pariterator.h"
#include "texrow.h"
#include "undo.h"
#include "WordLangTuple.h"
#include "metricsinfo.h"

#include "frontends/Alert.h"
#include "frontends/Clipboard.h"
#include "frontends/Dialogs.h"
#include "frontends/LyXView.h"

#include "insets/insetcommand.h" // ChangeRefs
#include "insets/insettext.h"


using lyx::support::bformat;

using lyx::cap::setSelectionRange;

using std::distance;
using std::find;
using std::string;
using std::vector;


extern BufferList bufferlist;


BufferView::BufferView(LyXView * owner)
	: pimpl_(new Pimpl(*this, owner))
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


LyXView * BufferView::owner() const
{
	return pimpl_->owner_;
}


void BufferView::setBuffer(Buffer * b)
{
	pimpl_->setBuffer(b);
}


string BufferView::firstLayout()
{
	return pimpl_->firstLayout();
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


void BufferView::update(Update::flags flags)
{
	pimpl_->update(flags);
}


void BufferView::updateScrollbar()
{
	pimpl_->updateScrollbar();
}


ScrollbarParameters const & BufferView::scrollbarParameters() const
{
	return pimpl_->scrollbarParameters();
}


void BufferView::scrollDocView(int value)
{
	pimpl_->scrollDocView(value);
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

void BufferView::saveSavedPositions()
{
	return pimpl_->saveSavedPositions();
}

void BufferView::switchKeyMap()
{
	pimpl_->switchKeyMap();
}


int BufferView::workWidth() const
{
	return pimpl_->width();
}


void BufferView::center()
{
	pimpl_->center();
}


FuncStatus BufferView::getStatus(FuncRequest const & cmd)
{
	return pimpl_->getStatus(cmd);
}


bool BufferView::dispatch(FuncRequest const & ev)
{
	return pimpl_->dispatch(ev);
}


void BufferView::selectionRequested()
{
	pimpl_->selectionRequested();
}


void BufferView::selectionLost()
{
	pimpl_->selectionLost();
}


void BufferView::workAreaResize(int width, int height)
{
	pimpl_->workAreaResize(width, height);
}


void BufferView::workAreaKeyPress(LyXKeySymPtr key, key_modifier::state state)
{
	pimpl_->workAreaKeyPress(key, state);
}


bool BufferView::workAreaDispatch(FuncRequest const & ev)
{
	return pimpl_->workAreaDispatch(ev);
}


void BufferView::scroll(int lines)
{
	pimpl_->scroll(lines);
}


void BufferView::showErrorList(string const & action) const
{
	if (getErrorList().size()) {
		string const title = bformat(_("%1$s Errors (%2$s)"),
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
		text()->setCursor(cursor(), buffer()->getParFromID(tmpid).pit(), tmppos);
}


void BufferView::gotoLabel(string const & label)
{
	for (InsetIterator it = inset_iterator_begin(buffer()->inset()); it; ++it) {
		vector<string> labels;
		it->getLabelList(*buffer(), labels);
		if (find(labels.begin(),labels.end(),label) != labels.end()) {
			setCursor(it);
			update();
			return;
		}
	}
}


LyXText * BufferView::getLyXText()
{
	LyXText * text = cursor().innerText();
	BOOST_ASSERT(text);
	return text;
}


LyXText const * BufferView::getLyXText() const
{
	LyXText const * text = cursor().innerText();
	BOOST_ASSERT(text);
	return text;
}


int BufferView::workHeight() const
{
	return pimpl_->height();
}


LyXText * BufferView::text() const
{
	return buffer() ? &buffer()->text() : 0;
}


void BufferView::setCursor(DocIterator const & dit)
{
	size_t const n = dit.depth();
	for (size_t i = 0; i < n; ++i)
		dit[i].inset().edit(cursor(), true);

	cursor().setCursor(dit);
	cursor().selection() = false;
}


void BufferView::mouseSetCursor(LCursor & cur)
{
	BOOST_ASSERT(&cur.bv() == this);

	// Has the cursor just left the inset?
	if (&cursor().inset() != &cur.inset())
		cursor().inset().notifyCursorLeaves(cursor());

	// do the dEPM magic if needed
	if (cursor().inTexted())
		cursor().text()->deleteEmptyParagraphMechanism(cur, cursor());

	cursor() = cur;
	cursor().clearSelection();
	cursor().setTargetX();
	finishUndo();

}


void BufferView::putSelectionAt(DocIterator const & cur,
				int length, bool backwards)
{
	cursor().clearSelection();

	setCursor(cur);

	if (length) {
		if (backwards) {
			cursor().pos() += length;
			cursor().setSelection(cursor(), -length);
		} else
			cursor().setSelection(cursor(), length);
	}
}


LCursor & BufferView::cursor()
{
	return pimpl_->cursor_;
}


LCursor const & BufferView::cursor() const
{
	return pimpl_->cursor_;
}


lyx::pit_type BufferView::anchor_ref() const
{
	return pimpl_->anchor_ref_;
}


int BufferView::offset_ref() const
{
	return pimpl_->offset_ref_;
}


ViewMetricsInfo const & BufferView::viewMetricsInfo()
{
	return pimpl_->viewMetricsInfo();
}


bool BufferView::needsRedraw() const
{
	return pimpl_->needsRedraw();
}


void BufferView::needsRedraw(bool redraw_needed)
{
	pimpl_->needsRedraw(redraw_needed);
}
