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

#ifdef __GNUG__
#pragma implementation
#endif

#include "BufferView.h"
#include "BufferView_pimpl.h"
#include "lyxtext.h"
#include "WorkArea.h"
#include "lyxscreen.h"


BufferView::BufferView(LyXView * o, int xpos, int ypos,
		       int width, int height)
	: pimpl_(new Pimpl(this, o, xpos, ypos, width, height))
{
	text = 0;
	inset_slept = false;
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


LyXScreen * BufferView::screen() const
{
	return pimpl_->screen_.get();
}


WorkArea * BufferView::workarea() const
{
	return &pimpl_->workarea_;
}


LyXView * BufferView::owner() const
{
	return pimpl_->owner_;
}


void BufferView::pushIntoUpdateList(Inset * i)
{
	pimpl_->updatelist.push(i);
}


Painter & BufferView::painter() 
{
	return pimpl_->painter();
}


void BufferView::buffer(Buffer * b)
{
	pimpl_->buffer(b);
}


void BufferView::resize(int xpos, int ypos, int width, int height)
{
	pimpl_->resize(xpos, ypos, width, height);
}


void BufferView::resize()
{
	pimpl_->resize();
}


void BufferView::redraw()
{
	pimpl_->redraw();
}


void BufferView::fitCursor(LyXText * text)
{
	pimpl_->fitCursor(text);
}


void BufferView::update()
{
	pimpl_->update();
}


void BufferView::updateScrollbar()
{
	pimpl_->updateScrollbar();
}


void BufferView::scrollCB(double value)
{
	pimpl_->scrollCB(value);
}


Inset * BufferView::checkInsetHit(LyXText * text, int & x, int & y,
				  unsigned int button)
{
	return pimpl_->checkInsetHit(text, x, y, button);
}


void BufferView::redoCurrentBuffer()
{
	pimpl_->redoCurrentBuffer();
}


int BufferView::resizeCurrentBuffer()
{
	return pimpl_->resizeCurrentBuffer();
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


void BufferView::insetSleep()
{
	pimpl_->insetSleep();
}


void BufferView::insetWakeup()
{
	pimpl_->insetWakeup();
}


void BufferView::insetUnlock()
{
	pimpl_->insetUnlock();
}


bool BufferView::focus() const
{
  	return pimpl_->focus();
}


void BufferView::focus(bool f)
{
  	pimpl_->focus(f);
}


bool BufferView::active() const
{
  	return pimpl_->active();
}


int BufferView::workWidth() const
{
    return pimpl_->workarea_.workWidth();
}


bool BufferView::belowMouse() const 
{
	return pimpl_->belowMouse();
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
