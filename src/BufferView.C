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


extern "C" {
	void C_BufferView_CursorToggleCB(FL_OBJECT * ob, long buf)
	{
		BufferView::cursorToggleCB(ob, buf);
	}
}


// Callback for scrollbar up button
void BufferView::upCB(long time, int button)
{
	if (pimpl_->buffer_ == 0) return;

	switch (button) {
	case 3:
		pimpl_->scrollUpOnePage();
		break;
	case 2:
		pimpl_->scrollDownOnePage();
		break;
	default:
		pimpl_->scrollUp(time);
		break;
	}
}


// Callback for scrollbar slider
void BufferView::scrollCB(double value)
{
	pimpl_->scrollCB(value);
}


// Callback for scrollbar down button
void BufferView::downCB(long time, int button)
{
	pimpl_->downCB(time, button);
}


void BufferView::workAreaMotionNotify(int x, int y, unsigned int state)
{
	pimpl_->workAreaMotionNotify(x, y, state);
}


extern int bibitemMaxWidth(Painter &, LyXFont const &);


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


//  // Callback for cursor timer
void BufferView::cursorToggleCB(FL_OBJECT * ob, long )
{
	BufferView * view = static_cast<BufferView*>(ob->u_vdata);
	view->pimpl_->cursorToggle();
}


void BufferView::workAreaSelectionNotify(Window win, XEvent * event)
{
	pimpl_->workAreaSelectionNotify(win, event);
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


void BufferView::update(signed char f)
{
	pimpl_->update(f);
}


void BufferView::smallUpdate(signed char f)
{
	pimpl_->smallUpdate(f);
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


unsigned short BufferView::paperWidth() const
{
    return text->paperWidth();
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
