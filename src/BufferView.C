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
	the_locking_inset = 0;
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
	return pimpl_->screen_;
}


WorkArea * BufferView::workarea() const
{
	return pimpl_->workarea_;
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


void BufferView::fitCursor()
{
	pimpl_->fitCursor();
}


void BufferView::update()
{
	pimpl_->update();
}


void BufferView::updateScrollbar()
{
	pimpl_->updateScrollbar();
}


void BufferView::redoCurrentBuffer()
{
	pimpl_->redoCurrentBuffer();
}


int BufferView::resizeCurrentBuffer()
{
	return pimpl_->resizeCurrentBuffer();
}


void BufferView::gotoError()
{
	pimpl_->gotoError();
}


void BufferView::enterView()
{
	pimpl_->enterView();
}


void BufferView::leaveView()
{
	pimpl_->leaveView();
}


// Callback for scrollbar slider
void BufferView::scrollCB(double value)
{
	pimpl_->scrollCB(value);
}


void BufferView::workAreaMotionNotify(int x, int y, unsigned int state)
{
	pimpl_->workAreaMotionNotify(x, y, state);
}


///  Single-click on work area
void BufferView::workAreaButtonPress(int xpos, int ypos, unsigned int button)
{
	pimpl_->workAreaButtonPress(xpos, ypos, button);
}


void BufferView::doubleClick(int x, int y, unsigned int button) 
{
	pimpl_->doubleClick(x, y, button);
}


void BufferView::tripleClick(int x, int y, unsigned int button)
{
	pimpl_->tripleClick(x, y, button);
}


void BufferView::workAreaButtonRelease(int x, int y, unsigned int button)
{
	pimpl_->workAreaButtonRelease(x, y, button);
}


void BufferView::workAreaExpose()
{
	pimpl_->workAreaExpose();
}


void BufferView::cursorPrevious()
{
	pimpl_->cursorPrevious();
}


void BufferView::cursorNext()
{
	pimpl_->cursorNext();
}


bool BufferView::available() const
{
	return pimpl_->available();
}


void BufferView::beforeChange()
{
	pimpl_->beforeChange();
}


void BufferView::savePosition()
{
	pimpl_->savePosition();
}


void BufferView::restorePosition()
{
	pimpl_->restorePosition();
}


bool BufferView::NoSavedPositions()
{
	return pimpl_->NoSavedPositions();
}


void BufferView::update(UpdateCodes f)
{
	pimpl_->update(f);
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
    return pimpl_->workarea_->workWidth();
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
