/**
 * \file BufferView.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "BufferView.h"
#include "BufferView_pimpl.h"
#include "lyxtext.h"
#include "frontends/screen.h"
#include "frontends/WorkArea.h"


BufferView::BufferView(LyXView * o, int xpos, int ypos,
		       int width, int height)
	: pimpl_(new Pimpl(this, o, xpos, ypos, width, height))
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


void BufferView::resize()
{
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


Inset * BufferView::checkInsetHit(LyXText * text, int & x, int & y)
{
	return pimpl_->checkInsetHit(text, x, y);
}


void BufferView::redoCurrentBuffer()
{
	pimpl_->redoCurrentBuffer();
}


void BufferView::cursorPrevious(LyXText * text)
{
	pimpl_->cursorPrevious(text);
}


void BufferView::cursorNext(LyXText * text)
{
	pimpl_->cursorNext(text);
}


bool BufferView::available() const
{
	return pimpl_->available();
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


void BufferView::setState()
{
	pimpl_->setState();
}


void BufferView::insetUnlock()
{
	pimpl_->insetUnlock();
}


int BufferView::workWidth() const
{
	return pimpl_->workarea().workWidth();
}


void BufferView::showCursor()
{
	pimpl_->showCursor();
}


void BufferView::hideCursor()
{
	pimpl_->hideCursor();
}


void BufferView::toggleSelection(bool b)
{
	pimpl_->toggleSelection(b);
}


void BufferView::toggleToggle()
{
	pimpl_->toggleToggle();
}


void BufferView::center()
{
	pimpl_->center();
}


void BufferView::pasteClipboard(bool asPara)
{
	pimpl_->pasteClipboard(asPara);
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

bool BufferView::Dispatch(kb_action action, string const & argument)
{
	return pimpl_->Dispatch(action, argument);
}
