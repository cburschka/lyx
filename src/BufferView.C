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

#include <algorithm>

#include <cstdlib>
#include <csignal>

#include <unistd.h>
#include <sys/wait.h>

#include "support/lstrings.h"

#ifdef __GNUG__
#pragma implementation
#endif

#include "commandtags.h"
#include "BufferView.h"
#include "bufferlist.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "insets/lyxinset.h"
#include "insets/insetbib.h"
#include "minibuffer.h"
#include "lyxscreen.h"
#include "menus.h"

#include "debug.h"
#include "lyx_gui_misc.h"
#include "BackStack.h"
#include "lyxtext.h"
#include "lyx_cb.h"
#include "gettext.h"
#include "layout.h"
#include "TextCache.h"
#include "intl.h"
#include "lyxrc.h"
#include "lyxrow.h"
#include "WorkArea.h"
#include "font.h"
#include "BufferView_pimpl.h"

using std::pair;
using std::for_each;
using std::find_if;
using std::endl;

extern BufferList bufferlist;

void sigchldhandler(pid_t pid, int * status);

extern void SetXtermCursor(Window win);
extern bool input_prohibited;
extern bool selection_possible;
extern char ascii_type;
extern void MenuPasteSelection(char at);
extern void FreeUpdateTimer();

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
	return pimpl_->workarea->getPainter();
}


void BufferView::buffer(Buffer * b)
{
	lyxerr[Debug::INFO] << "Setting buffer in BufferView ("
			    << b << ")" << endl;
	if (pimpl_->buffer_) {
		insetSleep();
		pimpl_->buffer_->delUser(this);

		// Put the old text into the TextCache, but
		// only if the buffer is still loaded.
		// Also set the owner of the test to 0
		text->owner(0);
		textcache.add(text);
		if (lyxerr.debugging())
			textcache.show(lyxerr, "BufferView::buffer");
		
		text = 0;
	}

	// Set current buffer
	pimpl_->buffer_ = b;

	if (bufferlist.getState() == BufferList::CLOSING) return;
	
	// Nuke old image
	// screen is always deleted when the buffer is changed.
	delete pimpl_->screen;
	pimpl_->screen = 0;

	// If we are closing the buffer, use the first buffer as current
	if (!pimpl_->buffer_) {
		pimpl_->buffer_ = bufferlist.first();
	}

	if (pimpl_->buffer_) {
		lyxerr[Debug::INFO] << "Buffer addr: " << pimpl_->buffer_ << endl;
		pimpl_->buffer_->addUser(this);
		pimpl_->owner_->getMenus()->showMenus();
		// If we don't have a text object for this, we make one
		if (text == 0)
			resizeCurrentBuffer();
		else {
			pimpl_->updateScreen();
			updateScrollbar();
		}
		pimpl_->screen->first = pimpl_->screen->TopCursorVisible();
		redraw();
		updateAllVisibleBufferRelatedPopups();
		insetWakeup();
	} else {
		lyxerr[Debug::INFO] << "  No Buffer!" << endl;
		pimpl_->owner_->getMenus()->hideMenus();
		updateScrollbar();
		pimpl_->workarea->redraw();

		// Also remove all remaining text's from the testcache.
		// (there should not be any!) (if there is any it is a
		// bug!)
		if (lyxerr.debugging())
			textcache.show(lyxerr, "buffer delete all");
		textcache.clear();
	}
	// should update layoutchoice even if we don't have a buffer.
	pimpl_->owner_->updateLayoutChoice();
	pimpl_->owner_->getMiniBuffer()->Init();
	pimpl_->owner_->updateWindowTitle();
}


void BufferView::resize(int xpos, int ypos, int width, int height)
{
	pimpl_->workarea->resize(xpos, ypos, width, height);
	update(3);
	redraw();
}


void BufferView::resize()
{
	// This will resize the buffer. (Asger)
	if (pimpl_->buffer_)
		resizeCurrentBuffer();
}


void BufferView::redraw()
{
	lyxerr[Debug::INFO] << "BufferView::redraw()" << endl;
	pimpl_->workarea->redraw();
}


void BufferView::fitCursor()
{
	if (pimpl_->screen) pimpl_->screen->FitCursor();
	updateScrollbar();
}


void BufferView::update()
{
	if (pimpl_->screen) pimpl_->screen->Update();
}


void BufferView::updateScrollbar()
{
	/* If the text is smaller than the working area, the scrollbar
	 * maximum must be the working area height. No scrolling will 
	 * be possible */

	if (!pimpl_->buffer_) {
		pimpl_->workarea->setScrollbar(0, 1.0);
		return;
	}
	
	static long max2 = 0;
	static long height2 = 0;

	long cbth = 0;
	long cbsf = 0;

	if (text)
		cbth = text->height;
	if (pimpl_->screen)
		cbsf = pimpl_->screen->first;

	// check if anything has changed.
	if (max2 == cbth &&
	    height2 == pimpl_->workarea->height() &&
	    pimpl_->current_scrollbar_value == cbsf)
		return; // no
	max2 = cbth;
	height2 = pimpl_->workarea->height();
	pimpl_->current_scrollbar_value = cbsf;

	if (cbth <= height2) { // text is smaller than screen
		pimpl_->workarea->setScrollbar(0, 1.0); // right?
		return;
	}

	long maximum_height = pimpl_->workarea->height() * 3 / 4 + cbth;
	long value = cbsf;

	// set the scrollbar
	double hfloat = pimpl_->workarea->height();
	double maxfloat = maximum_height;

	float slider_size = 0.0;
	int slider_value = value;

	pimpl_->workarea->setScrollbarBounds(0, text->height - pimpl_->workarea->height());
	double lineh = text->DefaultHeight();
	pimpl_->workarea->setScrollbarIncrements(lineh);
	if (maxfloat > 0.0) {
		if ((hfloat / maxfloat) * float(height2) < 3)
			slider_size = 3.0/float(height2);
		else
			slider_size = hfloat / maxfloat;
	} else
		slider_size = hfloat;

	pimpl_->workarea->setScrollbar(slider_value, slider_size / pimpl_->workarea->height());
}


void BufferView::redoCurrentBuffer()
{
	lyxerr[Debug::INFO] << "BufferView::redoCurrentBuffer" << endl;
	if (pimpl_->buffer_ && text) {
		resize();
		pimpl_->owner_->updateLayoutChoice();
	}
}


int BufferView::resizeCurrentBuffer()
{
	lyxerr[Debug::INFO] << "resizeCurrentBuffer" << endl;
	
	LyXParagraph * par = 0;
	LyXParagraph * selstartpar = 0;
	LyXParagraph * selendpar = 0;
	int pos = 0;
	int selstartpos = 0;
	int selendpos = 0;
	int selection = 0;
	int mark_set = 0;

	ProhibitInput();

	pimpl_->owner_->getMiniBuffer()->Set(_("Formatting document..."));   

	if (text) {
		par = text->cursor.par;
		pos = text->cursor.pos;
		selstartpar = text->sel_start_cursor.par;
		selstartpos = text->sel_start_cursor.pos;
		selendpar = text->sel_end_cursor.par;
		selendpos = text->sel_end_cursor.pos;
		selection = text->selection;
		mark_set = text->mark_set;
		delete text;
		text = new LyXText(this, pimpl_->workarea->workWidth(), pimpl_->buffer_);
	} else {
		// See if we have a text in TextCache that fits
		// the new buffer_ with the correct width.
		text = textcache.findFit(pimpl_->buffer_, pimpl_->workarea->workWidth());
		if (text) {
			if (lyxerr.debugging()) {
				lyxerr << "Found a LyXText that fits:\n";
				textcache.show(lyxerr, text);
			}
			// Set the owner of the newly found text
			text->owner(this);
			if (lyxerr.debugging())
				textcache.show(lyxerr, "resizeCurrentBuffer");
		} else {
			text = new LyXText(this, pimpl_->workarea->workWidth(), pimpl_->buffer_);
		}
	}
	pimpl_->updateScreen();

	if (par) {
		text->selection = true;
		/* at this point just to avoid the Delete-Empty-Paragraph
		 * Mechanism when setting the cursor */
		text->mark_set = mark_set;
		if (selection) {
			text->SetCursor(selstartpar, selstartpos);
			text->sel_cursor = text->cursor;
			text->SetCursor(selendpar, selendpos);
			text->SetSelection();
			text->SetCursor(par, pos);
		} else {
			text->SetCursor(par, pos);
			text->sel_cursor = text->cursor;
			text->selection = false;
		}
	}
	pimpl_->screen->first = pimpl_->screen->TopCursorVisible(); /* this will scroll the
						     * screen such that the
						     * cursor becomes
						     * visible */ 
	updateScrollbar();
	redraw();
	pimpl_->owner_->getMiniBuffer()->Init();
	setState();
	AllowInput();

	// Now if the title form still exist kill it
	TimerCB(0, 0);

	return 0;
}


void BufferView::gotoError()
{
	if (!pimpl_->screen)
		return;
   
	pimpl_->screen->HideCursor();
	beforeChange();
	update(-2);
	LyXCursor tmp;

	if (!text->GotoNextError()) {
		if (text->cursor.pos 
		    || text->cursor.par != text->FirstParagraph()) {
			tmp = text->cursor;
			text->cursor.par = text->FirstParagraph();
			text->cursor.pos = 0;
			if (!text->GotoNextError()) {
				text->cursor = tmp;
				pimpl_->owner_->getMiniBuffer()
					->Set(_("No more errors"));
				LyXBell();
			}
		} else {
			pimpl_->owner_->getMiniBuffer()->Set(_("No more errors"));
			LyXBell();
		}
	}
	update(0);
	text->sel_cursor = text->cursor;
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


static inline
void waitForX()
{
	XSync(fl_get_display(), 0);
}


// Callback for scrollbar slider
void BufferView::scrollCB(double value)
{
	extern bool cursor_follows_scrollbar;
	
	if (pimpl_->buffer_ == 0) return;

	pimpl_->current_scrollbar_value = long(value);
	if (pimpl_->current_scrollbar_value < 0)
		pimpl_->current_scrollbar_value = 0;
   
	if (!pimpl_->screen)
		return;

	pimpl_->screen->Draw(pimpl_->current_scrollbar_value);

	if (cursor_follows_scrollbar) {
		LyXText * vbt = text;
		int height = vbt->DefaultHeight();
		
		if (vbt->cursor.y < pimpl_->screen->first + height) {
			vbt->SetCursorFromCoordinates(0,
						      pimpl_->screen->first +
						      height);
		} else if (vbt->cursor.y >
			   pimpl_->screen->first + pimpl_->workarea->height() - height) {
			vbt->SetCursorFromCoordinates(0,
						      pimpl_->screen->first +
						      pimpl_->workarea->height()  -
						      height);
		}
	}
	waitForX();
}


// Callback for scrollbar down button
void BufferView::downCB(long time, int button)
{
	if (pimpl_->buffer_ == 0) return;
	
	switch (button) {
	case 2:
		pimpl_->scrollUpOnePage();
		break;
	case 3:
		pimpl_->scrollDownOnePage();
		break;
	default:
		pimpl_->scrollDown(time);
		break;
	}
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


// Callback for cursor timer
void BufferView::cursorToggleCB(FL_OBJECT * ob, long)
{
	BufferView * view = static_cast<BufferView*>(ob->u_vdata);
	
	// Quite a nice place for asyncron Inset updating, isn't it?
	// Actually no! This is run even if no buffer exist... so (Lgb)
	if (view && !view->pimpl_->buffer_) {
		goto set_timer_and_return;
	}

	// NOTE:
	// On my quest to solve the gs render hangups I am now
	// disabling the SIGHUP completely, and will do a wait
	// now and then instead. If the guess that xforms somehow
	// destroys something is true, this is likely (hopefully)
	// to solve the problem...at least I hope so. Lgb

	// ...Ok this seems to work...at least it does not make things
	// worse so far. However I still see gs processes that hangs.
	// I would really like to know _why_ they are hanging. Anyway
	// the solution without the SIGCHLD handler seems to be easier
	// to debug.

	// When attaching gdb to a a running gs that hangs it shows
	// that it is waiting for input(?) Is it possible for us to
	// provide that input somehow? Or figure what it is expecing
	// to read?

	// One solution is to, after some time, look if there are some
	// old gs processes still running and if there are: kill them
	// and re render.

	// Another solution is to provide the user an option to rerender
	// a picture. This would, for the picture in question, check if
	// there is a gs running for it, if so kill it, and start a new
	// rendering process.

	// these comments posted to lyx@via
	{
		int status = 1;
		int pid = waitpid(static_cast<pid_t>(0), &status, WNOHANG);
		if (pid == -1) // error find out what is wrong
			; // ignore it for now.
		else if (pid > 0)
			sigchldhandler(pid, &status);
	}

	view->pimpl_->updatelist.update(view);
	
	if (view && !view->pimpl_->screen) {
		goto set_timer_and_return;
	}

	if (view->pimpl_->lyx_focus && view->pimpl_->work_area_focus) {
		if (!view->the_locking_inset) {
			view->pimpl_->screen->CursorToggle();
		} else {
			view->the_locking_inset->
				ToggleInsetCursor(view);
		}
		goto set_timer_and_return;
	} else {
		// Make sure that the cursor is visible.
		if (!view->the_locking_inset) {
			view->pimpl_->screen->ShowCursor();
		} else {
			if (!view->the_locking_inset->isCursorVisible())
				view->the_locking_inset->
					ToggleInsetCursor(view);
		}
		// This is only run when work_area_focus or lyx_focus is false.
		Window tmpwin;
		int tmp;
		XGetInputFocus(fl_display, &tmpwin, &tmp);
		// Commenting this out, we have not had problems with this
		// for a long time. We will probably work on this code later
		// and we can reenable this debug code then. Now it only
		// anoying when debugging. (Lgb)
		//if (lyxerr.debugging(Debug::INFO)) {
		//	lyxerr << "tmpwin: " << tmpwin
		//	       << "\nwindow: " << view->owner_->getForm()->window
		//	       << "\nwork_area_focus: " << view->work_area_focus
		//	       << "\nlyx_focus      : " << view->lyx_focus
		//	       << endl;
		//}
		if (tmpwin != view->pimpl_->owner_->getForm()->window) {
			view->pimpl_->lyx_focus = false;
			goto skip_timer;
		} else {
			view->pimpl_->lyx_focus = true;
			if (!view->pimpl_->work_area_focus)
				goto skip_timer;
			else
				goto set_timer_and_return;
		}
	}

  set_timer_and_return:
	fl_set_timer(ob, 0.4);
  skip_timer:
	return;
}


void BufferView::workAreaSelectionNotify(Window win, XEvent * event)
{
	pimpl_->workAreaSelectionNotify(win, event);
}


void BufferView::cursorPrevious()
{
	if (!text->cursor.row->previous) return;
	
	long y = pimpl_->screen->first;
	Row * cursorrow = text->cursor.row;
	text->SetCursorFromCoordinates(text->cursor.x_fix, y);
	text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == text->cursor.row))
		text->CursorUp();
	
  	if (text->cursor.row->height < pimpl_->workarea->height())
		pimpl_->screen->Draw(text->cursor.y
				  - text->cursor.row->baseline
				  + text->cursor.row->height
				  - pimpl_->workarea->height() + 1 );
}


void BufferView::cursorNext()
{
	if (!text->cursor.row->next) return;
	
	long y = pimpl_->screen->first;
	text->GetRowNearY(y);
	Row * cursorrow = text->cursor.row;
	text->SetCursorFromCoordinates(text->cursor.x_fix, y
				       + pimpl_->workarea->height());
	text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == text->cursor.row))
		text->CursorDown();
	
 	if (text->cursor.row->height < pimpl_->workarea->height())
		pimpl_->screen->Draw(text->cursor.y
				  - text->cursor.row->baseline);
}


bool BufferView::available() const
{
	if (pimpl_->buffer_ && text) return true;
	return false;
}


void BufferView::beforeChange()
{
	toggleSelection();
	text->ClearSelection();
	FreeUpdateTimer();
}


void BufferView::savePosition()
{
	pimpl_->backstack.push(buffer()->fileName(),
		       text->cursor.x,
		       text->cursor.y);
}


void BufferView::restorePosition()
{
	if (pimpl_->backstack.empty()) return;
	
	int  x, y;
	string fname = pimpl_->backstack.pop(&x, &y);
	
	beforeChange();
	Buffer * b = bufferlist.exists(fname) ?
		bufferlist.getBuffer(fname) :
		bufferlist.loadLyXFile(fname); // don't ask, just load it
	buffer(b);
	text->SetCursorFromCoordinates(x, y);
	update(0);
} 


void BufferView::update(signed char f)
{
	owner()->updateLayoutChoice();

	if (!text->selection && f > -3)
		text->sel_cursor = text->cursor;
	
	FreeUpdateTimer();
	text->FullRebreak();

	update();

	if (f != 3 && f != -3) {
		fitCursor();
		updateScrollbar();
      	}

	if (f == 1 || f == -1) {
		if (buffer()->isLyxClean()) {
			buffer()->markDirty();
			owner()->getMiniBuffer()->setTimer(4);
		} else {
			buffer()->markDirty();
		}
	}
}


void BufferView::smallUpdate(signed char f)
{
	pimpl_->screen->SmallUpdate();
	if (pimpl_->screen->TopCursorVisible()
	    != pimpl_->screen->first) {
		update(f);
		return;
	}

	fitCursor();
	updateScrollbar();

	if (!text->selection)
		text->sel_cursor = text->cursor;

	if (f == 1 || f == -1) {
		if (buffer()->isLyxClean()) {
			buffer()->markDirty();
			owner()->getMiniBuffer()->setTimer(4);
		} else {
			buffer()->markDirty();
		}
	}
}


void BufferView::setState()
{
	if (!lyxrc.rtl_support)
		return;

	if (text->real_current_font.isVisibleRightToLeft()) {
		if (pimpl_->owner_->getIntl()->primarykeymap)
			pimpl_->owner_->getIntl()->KeyMapSec();
	} else {
		if (!pimpl_->owner_->getIntl()->primarykeymap)
			pimpl_->owner_->getIntl()->KeyMapPrim();
	}
}


void BufferView::insetSleep()
{
	if (the_locking_inset && !inset_slept) {
		the_locking_inset->GetCursorPos(slx, sly);
		the_locking_inset->InsetUnlock(this);
		inset_slept = true;
	}
}


void BufferView::insetWakeup()
{
	if (the_locking_inset && inset_slept) {
		the_locking_inset->Edit(this, slx, sly, 0);
		inset_slept = false;
	}
}


void BufferView::insetUnlock()
{
	if (the_locking_inset) {
		if (!inset_slept) the_locking_inset->InsetUnlock(this);
		the_locking_inset = 0;
		text->FinishUndo();
		inset_slept = false;
	}
}


bool BufferView::focus() const
{
	return pimpl_->workarea->hasFocus();
}


void BufferView::focus(bool f)
{
	if (f) pimpl_->workarea->setFocus();
}


bool BufferView::active() const
{
	return pimpl_->workarea->active();
}


Painter & BufferView::getPainter() const
{
    return pimpl_->workarea->getPainter();
}


unsigned short BufferView::paperWidth() const
{
    return text->paperWidth();
}


bool BufferView::belowMouse() const 
{
	return pimpl_->workarea->belowMouse();
}


void BufferView::showCursor()
{
	if (pimpl_->screen)
		pimpl_->screen->ShowCursor();
}


void BufferView::hideCursor()
{
	if (pimpl_->screen)
		pimpl_->screen->HideCursor();
}


void BufferView::toggleSelection(bool b)
{
	if (pimpl_->screen)
		pimpl_->screen->ToggleSelection(b);
}


void BufferView::toggleToggle()
{
	if (pimpl_->screen)
		pimpl_->screen->ToggleToggle();
}


void BufferView::center() 
{
	beforeChange();
	if (text->cursor.y > pimpl_->workarea->height() / 2) {
		pimpl_->screen->Draw(text->cursor.y - pimpl_->workarea->height() / 2);
	} else {
		pimpl_->screen->Draw(0);
	}
	update(0);
	redraw();
}
