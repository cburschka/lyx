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
using std::for_each;

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

using std::find_if;

extern BufferList bufferlist;
extern LyXRC * lyxrc;

void sigchldhandler(pid_t pid, int * status);

extern void SetXtermCursor(Window win);
extern bool input_prohibited;
extern bool selection_possible;
extern char ascii_type;
extern void MenuPasteSelection(char at);
extern void FreeUpdateTimer();

BufferView::BufferView(LyXView * o, int xpos, int ypos,
		       int width, int height)
	: owner_(o)
{
	buffer_ = 0;
	text = 0;
	workarea = new WorkArea(this, xpos, ypos, width, height);
	screen = 0;
	timer_cursor = 0;
	create_view();
	current_scrollbar_value = 0;
	// Activate the timer for the cursor 
	fl_set_timer(timer_cursor, 0.4);
	workarea->setFocus();
	work_area_focus = true;
	lyx_focus = false;
	the_locking_inset = 0;
	inset_slept = false;
}


BufferView::~BufferView()
{
	delete text;
}


Painter & BufferView::painter() 
{
	return workarea->getPainter();
}


void BufferView::buffer(Buffer * b)
{
	lyxerr[Debug::INFO] << "Setting buffer in BufferView ("
			    << b << ")" << endl;
	if (buffer_) {
		insetSleep();
		buffer_->delUser(this);

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
	buffer_ = b;

	if (bufferlist.getState() == BufferList::CLOSING) return;
	
	// Nuke old image
	// screen is always deleted when the buffer is changed.
	delete screen;
	screen = 0;

	// If we are closing the buffer, use the first buffer as current
	if (!buffer_) {
		buffer_ = bufferlist.first();
	}

	if (buffer_) {
		lyxerr[Debug::INFO] << "Buffer addr: " << buffer_ << endl;
		buffer_->addUser(this);
		owner_->getMenus()->showMenus();
		// If we don't have a text object for this, we make one
		if (text == 0)
			resizeCurrentBuffer();
		else {
			updateScreen();
			updateScrollbar();
		}
		screen->first = screen->TopCursorVisible();
		redraw();
		updateAllVisibleBufferRelatedPopups();
		insetWakeup();
	} else {
		lyxerr[Debug::INFO] << "  No Buffer!" << endl;
		owner_->getMenus()->hideMenus();
		updateScrollbar();
		workarea->redraw();

		// Also remove all remaining text's from the testcache.
		// (there should not be any!) (if there is any it is a
		// bug!)
		if (lyxerr.debugging())
			textcache.show(lyxerr, "buffer delete all");
		textcache.clear();
	}
	// should update layoutchoice even if we don't have a buffer.
	owner_->updateLayoutChoice();
	owner_->getMiniBuffer()->Init();
	owner_->updateWindowTitle();
}


void BufferView::updateScreen()
{
	// Regenerate the screen.
	delete screen;
	screen = new LyXScreen(*workarea, text);
}


void BufferView::resize(int xpos, int ypos, int width, int height)
{
	workarea->resize(xpos, ypos, width, height);
	update(3);
	redraw();
}


void BufferView::resize()
{
	// This will resize the buffer. (Asger)
	if (buffer_)
		resizeCurrentBuffer();
}


void BufferView::redraw()
{
	lyxerr[Debug::INFO] << "BufferView::redraw()" << endl;
	workarea->redraw();
}


void BufferView::fitCursor()
{
	if (screen) screen->FitCursor();
	updateScrollbar();
}


void BufferView::update()
{
	if (screen) screen->Update();
}


void BufferView::updateScrollbar()
{
	/* If the text is smaller than the working area, the scrollbar
	 * maximum must be the working area height. No scrolling will 
	 * be possible */

	if (!buffer_) {
		workarea->setScrollbar(0, 1.0);
		return;
	}
	
	static long max2 = 0;
	static long height2 = 0;

	long cbth = 0;
	long cbsf = 0;

	if (text)
		cbth = text->height;
	if (screen)
		cbsf = screen->first;

	// check if anything has changed.
	if (max2 == cbth &&
	    height2 == workarea->height() &&
	    current_scrollbar_value == cbsf)
		return; // no
	max2 = cbth;
	height2 = workarea->height();
	current_scrollbar_value = cbsf;

	if (cbth <= height2) { // text is smaller than screen
		workarea->setScrollbar(0, 1.0); // right?
		return;
	}

	long maximum_height = workarea->height() * 3 / 4 + cbth;
	long value = cbsf;

	// set the scrollbar
	double hfloat = workarea->height();
	double maxfloat = maximum_height;

	float slider_size = 0.0;
	int slider_value = value;

	workarea->setScrollbarBounds(0, text->height - workarea->height());
	double lineh = text->DefaultHeight();
	workarea->setScrollbarIncrements(lineh);
	if (maxfloat > 0.0) {
		if ((hfloat / maxfloat) * float(height2) < 3)
			slider_size = 3.0/float(height2);
		else
			slider_size = hfloat / maxfloat;
	} else
		slider_size = hfloat;

	workarea->setScrollbar(slider_value, slider_size / workarea->height());
}


void BufferView::redoCurrentBuffer()
{
	lyxerr[Debug::INFO] << "BufferView::redoCurrentBuffer" << endl;
	if (buffer_ && text) {
		resize();
		owner_->updateLayoutChoice();
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

	owner_->getMiniBuffer()->Set(_("Formatting document..."));   

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
		text = new LyXText(this, workarea->workWidth(), buffer_);
	} else {
		// See if we have a text in TextCache that fits
		// the new buffer_ with the correct width.
		text = textcache.findFit(buffer_, workarea->workWidth());
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
			text = new LyXText(this, workarea->workWidth(), buffer_);
		}
	}
	updateScreen();

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
	screen->first = screen->TopCursorVisible(); /* this will scroll the
						     * screen such that the
						     * cursor becomes
						     * visible */ 
	updateScrollbar();
	redraw();
	owner_->getMiniBuffer()->Init();
	setState();
	AllowInput();

	// Now if the title form still exist kill it
	TimerCB(0, 0);

	return 0;
}


void BufferView::gotoError()
{
	if (!screen)
		return;
   
	screen->HideCursor();
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
				owner_->getMiniBuffer()
					->Set(_("No more errors"));
				LyXBell();
			}
		} else {
			owner_->getMiniBuffer()->Set(_("No more errors"));
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


void BufferView::create_view()
{
	FL_OBJECT * obj;

	//
	// TIMERS
	//
	
	// timer_cursor
	timer_cursor = obj = fl_add_timer(FL_HIDDEN_TIMER,
					  0, 0, 0, 0, "Timer");
	fl_set_object_callback(obj, C_BufferView_CursorToggleCB, 0);
	obj->u_vdata = this;
}


// Callback for scrollbar up button
void BufferView::upCB(long time, int button)
{
	if (buffer_ == 0) return;

	switch (button) {
	case 3:
		scrollUpOnePage();
		break;
	case 2:
		scrollDownOnePage();
		break;
	default:
		scrollUp(time);
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
	
	if (buffer_ == 0) return;

	current_scrollbar_value = long(value);
	if (current_scrollbar_value < 0)
		current_scrollbar_value = 0;
   
	if (!screen)
		return;

	screen->Draw(current_scrollbar_value);

	if (cursor_follows_scrollbar) {
		LyXText * vbt = text;
		int height = vbt->DefaultHeight();
		
		if (vbt->cursor.y < screen->first + height) {
			vbt->SetCursorFromCoordinates(0,
						      screen->first +
						      height);
		} else if (vbt->cursor.y >
			   screen->first + workarea->height() - height) {
			vbt->SetCursorFromCoordinates(0,
						      screen->first +
						      workarea->height()  -
						      height);
		}
	}
	waitForX();
}


// Callback for scrollbar down button
void BufferView::downCB(long time, int button)
{
	if (buffer_ == 0) return;
	
	switch (button) {
	case 2:
		scrollUpOnePage();
		break;
	case 3:
		scrollDownOnePage();
		break;
	default:
		scrollDown(time);
		break;
	}
}


int BufferView::scrollUp(long time)
{
	if (buffer_ == 0) return 0;
	if (!screen) return 0;
   
	double value = workarea->getScrollbarValue();
   
	if (value == 0) return 0;

	float add_value =  (text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > workarea->height())
		add_value = float(workarea->height() -
				  text->DefaultHeight());
   
	value -= add_value;

	if (value < 0)
		value = 0;
   
	workarea->setScrollbarValue(value);
   
	scrollCB(value); 
	return 0;
}


int BufferView::scrollDown(long time)
{
	if (buffer_ == 0) return 0;
	if (!screen) return 0;
   
	double value= workarea->getScrollbarValue();
	pair<double, double> p = workarea->getScrollbarBounds();
	double max = p.second;
	
	if (value == max) return 0;

	float add_value =  (text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > workarea->height())
		add_value = float(workarea->height() -
				  text->DefaultHeight());
   
	value += add_value;
   
	if (value > max)
		value = max;

	workarea->setScrollbarValue(value);
	
	scrollCB(value); 
	return 0;
}


void BufferView::scrollUpOnePage()
{
	if (buffer_ == 0) return;
	if (!screen) return;
   
	long y = screen->first;

	if (!y) return;

	Row * row = text->GetRowNearY(y);

	y = y - workarea->height() + row->height;

	workarea->setScrollbarValue(y);
	
	scrollCB(y); 
}


void BufferView::scrollDownOnePage()
{
	if (buffer_ == 0) return;
	if (!screen) return;
   
	long y = screen->first;

	if (y > text->height - workarea->height())
		return;
   
	y += workarea->height();
	text->GetRowNearY(y);

	workarea->setScrollbarValue(y);
	
	scrollCB(y); 
}


void BufferView::workAreaMotionNotify(int x, int y, unsigned int state)
{
	if (buffer_ == 0 || !screen) return;

	// Check for inset locking
	if (the_locking_inset) {
		LyXCursor cursor = text->cursor;
		the_locking_inset->
			InsetMotionNotify(this,
					  x - cursor.x,
					  y - cursor.y,
					  state);
		return;
	}

	// Only use motion with button 1
	if (!state & Button1MotionMask)
		return; 
   
	/* The selection possible is needed, that only motion events are 
	 * used, where the bottom press event was on the drawing area too */
	if (selection_possible) {
		screen->HideCursor();

		text->SetCursorFromCoordinates(x, y + screen->first);
      
		if (!text->selection)
			update(-3); // Maybe an empty line was deleted
      
		text->SetSelection();
		screen->ToggleToggle();
		if (screen->FitCursor())
			updateScrollbar(); 
		screen->ShowCursor();
	}
	return;
}


extern int bibitemMaxWidth(Painter &, LyXFont const &);


// Single-click on work area
void BufferView::workAreaButtonPress(int xpos, int ypos, unsigned int button)
{
	last_click_x = -1;
	last_click_y = -1;

	if (buffer_ == 0 || !screen) return;

	Inset * inset_hit = checkInsetHit(xpos, ypos);

	// ok ok, this is a hack.
	if (button == 4 || button == 5) {
		switch (button) {
		case 4:
			scrollUp(100); // This number is only temporary
			break;
		case 5:
			scrollDown(100);
			break;
		}
	}
	
	if (the_locking_inset) {
		// We are in inset locking mode
		
		/* Check whether the inset was hit. If not reset mode,
		   otherwise give the event to the inset */
		if (inset_hit) {
			the_locking_inset->
				InsetButtonPress(this,
						 xpos, ypos,
						 button);
			return;
		} else {
			unlockInset(the_locking_inset);
		}
	}
	
	selection_possible = true;
	screen->HideCursor();
	
	// Right button mouse click on a table
	if (button == 3 &&
	    (text->cursor.par->table ||
	     text->MouseHitInTable(xpos, ypos + screen->first))) {
		// Set the cursor to the press-position
		text->SetCursorFromCoordinates(xpos, ypos + screen->first);
		bool doit = true;
		
		// Only show the table popup if the hit is in
		// the table, too
		if (!text->HitInTable(text->cursor.row, xpos))
			doit = false;
		
		// Hit above or below the table?
		if (doit) {
			if (!text->selection) {
				screen->ToggleSelection();
				text->ClearSelection();
				text->FullRebreak();
				screen->Update();
				updateScrollbar();
			}
			// Popup table popup when on a table.
			// This is obviously temporary, since we
			// should be able to popup various
			// context-sensitive-menus with the
			// the right mouse. So this should be done more
			// general in the future. Matthias.
			selection_possible = false;
			owner_->getLyXFunc()
				->Dispatch(LFUN_LAYOUT_TABLE,
					   "true");
			return;
		}
	}
	
	int screen_first = screen->first;
	
	// Middle button press pastes if we have a selection
	bool paste_internally = false;
	if (button == 2
	    && text->selection) {
		owner_->getLyXFunc()->Dispatch(LFUN_COPY);
		paste_internally = true;
	}
	
	// Clear the selection
	screen->ToggleSelection();
	text->ClearSelection();
	text->FullRebreak();
	screen->Update();
	updateScrollbar();
	
	// Single left click in math inset?
	if (inset_hit != 0 && inset_hit->Editable() == 2) {
		// Highly editable inset, like math
		selection_possible = false;
		owner_->updateLayoutChoice();
		owner_->getMiniBuffer()->Set(inset_hit->EditMessage());
		inset_hit->Edit(this, xpos, ypos, button);
		return;
	} 
	
	// Right click on a footnote flag opens float menu
	if (button == 3) { 
		selection_possible = false;
		return;
	}
	
	text->SetCursorFromCoordinates(xpos, ypos + screen_first);
	text->FinishUndo();
	text->sel_cursor = text->cursor;
	text->cursor.x_fix = text->cursor.x;
	
	owner_->updateLayoutChoice();
	if (screen->FitCursor()){
		updateScrollbar();
		selection_possible = false;
	}
	
	// Insert primary selection with middle mouse
	// if there is a local selection in the current buffer,
	// insert this
	if (button == 2) {
		if (paste_internally)
			owner_->getLyXFunc()->Dispatch(LFUN_PASTE);
		else
			owner_->getLyXFunc()->Dispatch(LFUN_PASTESELECTION,
						       "paragraph");
		selection_possible = false;
		return;
	}
}


void BufferView::doubleClick(int /*x*/, int /*y*/, unsigned int button) 
{
	// select a word
	if (buffer_ && !the_locking_inset) {
		if (screen && button == 1) {
			screen->HideCursor();
			screen->ToggleSelection();
			text->SelectWord();
			screen->ToggleSelection(false);
			/* This will fit the cursor on the screen
			 * if necessary */
			update(0);
		}
	}            
}


void BufferView::trippleClick(int /*x*/, int /*y*/, unsigned int button)
{
	// select a line
	if (buffer_ && screen && button == 1) {
		screen->HideCursor();
		screen->ToggleSelection();
		text->CursorHome();
		text->sel_cursor = text->cursor;
		text->CursorEnd();
		text->SetSelection();
		screen->ToggleSelection(false);
		/* This will fit the cursor on the screen
		 * if necessary */
		update(0);
	}
}


void BufferView::workAreaButtonRelease(int x, int y, unsigned int button)
{
	if (buffer_ == 0 || screen == 0) return;

	// If we hit an inset, we have the inset coordinates in these
	// and inset_hit points to the inset.  If we do not hit an
	// inset, inset_hit is 0, and inset_x == x, inset_y == y.
	Inset * inset_hit = checkInsetHit(x, y);

	if (the_locking_inset) {
		// We are in inset locking mode.

		/* LyX does a kind of work-area grabbing for insets.
		   Only a ButtonPress Event outside the inset will 
		   force a InsetUnlock. */
		the_locking_inset->
			InsetButtonRelease(this, x, y, button);
		return;
	}
	
	selection_possible = false;
        if (text->cursor.par->table) {
                int cell = text->
                        NumberOfCell(text->cursor.par,
                                     text->cursor.pos);
                if (text->cursor.par->table->IsContRow(cell) &&
                    text->cursor.par->table->
                    CellHasContRow(text->cursor.par->table->
                                   GetCellAbove(cell))<0) {
                        text->CursorUp();
                }
        }
	
	if (button >= 2) return;

	setState();
	owner_->getMiniBuffer()->Set(CurrentState());

	// Did we hit an editable inset?
	if (inset_hit != 0) {
		// Inset like error, notes and figures
		selection_possible = false;
#ifdef WITH_WARNINGS
#warning fix this proper in 0.13
#endif
		// Following a ref shouldn't issue
		// a push on the undo-stack
		// anylonger, now that we have
		// keybindings for following
		// references and returning from
		// references.  IMHO though, it
		// should be the inset's own business
		// to push or not push on the undo
		// stack. They don't *have* to
		// alter the document...
		// (Joacim)
		// ...or maybe the SetCursorParUndo()
		// below isn't necessary at all anylonger?
		if (inset_hit->LyxCode() == Inset::REF_CODE) {
			text->SetCursorParUndo();
		}

		owner_->getMiniBuffer()->Set(inset_hit->EditMessage());
		inset_hit->Edit(this, x, y, button);
		return;
	}

	// check whether we want to open a float
	if (text) {
		bool hit = false;
		char c = ' ';
		if (text->cursor.pos <
		    text->cursor.par->Last()) {
			c = text->cursor.par->
				GetChar(text->cursor.pos);
		}
		if (c == LyXParagraph::META_FOOTNOTE
		    || c == LyXParagraph::META_MARGIN
		    || c == LyXParagraph::META_FIG
		    || c == LyXParagraph::META_TAB
		    || c == LyXParagraph::META_WIDE_FIG
		    || c == LyXParagraph::META_WIDE_TAB
                    || c == LyXParagraph::META_ALGORITHM){
			hit = true;
		} else if (text->cursor.pos - 1 >= 0) {
			c = text->cursor.par->
				GetChar(text->cursor.pos - 1);
			if (c == LyXParagraph::META_FOOTNOTE
			    || c == LyXParagraph::META_MARGIN
			    || c == LyXParagraph::META_FIG
			    || c == LyXParagraph::META_TAB
			    || c == LyXParagraph::META_WIDE_FIG 
			    || c == LyXParagraph::META_WIDE_TAB
			    || c == LyXParagraph::META_ALGORITHM){
				// We are one step too far to the right
				text->CursorLeft();
				hit = true;
			}
		}
		if (hit == true) {
			toggleFloat();
			selection_possible = false;
			return;
		}
	}

	// Do we want to close a float? (click on the float-label)
	if (text->cursor.row->par->footnoteflag == 
	    LyXParagraph::OPEN_FOOTNOTE
	    //&& text->cursor.pos == 0
	    && text->cursor.row->previous &&
	    text->cursor.row->previous->par->
	    footnoteflag != LyXParagraph::OPEN_FOOTNOTE){
		LyXFont font(LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_FOOTNOTE);

		int box_x = 20; // LYX_PAPER_MARGIN;
		box_x += font.textWidth(" wide-tab ", 10);

		int screen_first = screen->first;

		if (x < box_x
		    && y + screen_first > text->cursor.y -
		    text->cursor.row->baseline
		    && y + screen_first < text->cursor.y -
		    text->cursor.row->baseline
		    + font.maxAscent() * 1.2 + font.maxDescent() * 1.2) {
			toggleFloat();
			selection_possible = false;
			return;
		}
	}

	// Maybe we want to edit a bibitem ale970302
	if (text->cursor.par->bibkey && x < 20 + 
	    bibitemMaxWidth(painter(),
			    textclasslist
			    .TextClass(buffer_->
				       params.textclass).defaultfont())) {
		text->cursor.par->bibkey->Edit(this, 0, 0, 0);
	}

	return;
}


/* 
 * Returns an inset if inset was hit. 0 otherwise.
 * If hit, the coordinates are changed relative to the inset. 
 * Otherwise coordinates are not changed, and false is returned.
 */
Inset * BufferView::checkInsetHit(int & x, int & y)
{
	if (!screen)
		return 0;
  
	int y_tmp = y + screen->first;
  
	LyXCursor & cursor = text->cursor;
	LyXDirection direction = text->real_current_font.getFontDirection();

	if (cursor.pos < cursor.par->Last()
	    && cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET
	    && cursor.par->GetInset(cursor.pos)
	    && cursor.par->GetInset(cursor.pos)->Editable()) {

		// Check whether the inset really was hit
		Inset * tmpinset = cursor.par->GetInset(cursor.pos);
		LyXFont font = text->GetFont(cursor.par, cursor.pos);
		int start_x, end_x;
		if (direction == LYX_DIR_LEFT_TO_RIGHT) {
			start_x = cursor.x;
			end_x = cursor.x + tmpinset->width(painter(), font);
		} else {
			start_x = cursor.x - tmpinset->width(painter(), font);
			end_x = cursor.x;
		}

		if (x > start_x && x < end_x
		    && y_tmp > cursor.y - tmpinset->ascent(painter(), font)
		    && y_tmp < cursor.y + tmpinset->descent(painter(), font)) {
			x = x - start_x;
			// The origin of an inset is on the baseline
			y = y_tmp - (cursor.y); 
			return tmpinset;
		}
	}

	if (cursor.pos - 1 >= 0
		   && cursor.par->GetChar(cursor.pos - 1) == LyXParagraph::META_INSET
		   && cursor.par->GetInset(cursor.pos - 1)
		   && cursor.par->GetInset(cursor.pos - 1)->Editable()) {
		text->CursorLeft();
		Inset * tmpinset = cursor.par->GetInset(cursor.pos);
		LyXFont font = text->GetFont(cursor.par, cursor.pos);
		int start_x, end_x;
		if (direction == LYX_DIR_LEFT_TO_RIGHT) {
			start_x = cursor.x;
			end_x = cursor.x + tmpinset->width(painter(), font);
		} else {
			start_x = cursor.x - tmpinset->width(painter(), font);
			end_x = cursor.x;
		}
		if (x > start_x && x < end_x
		    && y_tmp > cursor.y - tmpinset->ascent(painter(), font)
		    && y_tmp < cursor.y + tmpinset->descent(painter(), font)) {
			x = x - start_x;
			// The origin of an inset is on the baseline
			y = y_tmp - (cursor.y); 
			return tmpinset;
		} else {
			text->CursorRight();
			return 0;
		}
	}
	return 0;
}

void BufferView::workAreaExpose()
{
	// this is a hack to ensure that we only call this through
	// BufferView::redraw().
	//if (!lgb_hack) {
	//	redraw();
	//}
	
	static int work_area_width = -1;
	static int work_area_height = -1;

	bool widthChange = workarea->workWidth() != work_area_width;
	bool heightChange = workarea->height() != work_area_height;

	// update from work area
	work_area_width = workarea->workWidth();
	work_area_height = workarea->height();
	if (buffer_ != 0) {
		if (widthChange) {
			// All buffers need a resize
			bufferlist.resize();

			// Remove all texts from the textcache
			// This is not _really_ what we want to do. What
			// we really want to do is to delete in textcache
			// that does not have a BufferView with matching
			// width, but as long as we have only one BufferView
			// deleting all gives the same result.
			if (lyxerr.debugging())
				textcache.show(lyxerr, "Expose delete all");
			textcache.clear();
		} else if (heightChange) {
			// Rebuild image of current screen
			updateScreen();
			// fitCursor() ensures we don't jump back
			// to the start of the document on vertical
			// resize
			fitCursor();

			// The main window size has changed, repaint most stuff
			redraw();
			// ...including the minibuffer
			owner_->getMiniBuffer()->Init();

		} else if (screen) screen->Redraw();
	} else {
		// Grey box when we don't have a buffer
		workarea->greyOut();
	}

	// always make sure that the scrollbar is sane.
	updateScrollbar();
	owner_->updateLayoutChoice();
	return;
}


// Callback for cursor timer
void BufferView::cursorToggleCB(FL_OBJECT * ob, long)
{
	BufferView * view = static_cast<BufferView*>(ob->u_vdata);
	
	// Quite a nice place for asyncron Inset updating, isn't it?
	// Actually no! This is run even if no buffer exist... so (Lgb)
	if (view && !view->buffer_) {
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

	view->updatelist.update(view);
	
	if (view && !view->screen){
		goto set_timer_and_return;
	}

	if (view->lyx_focus && view->work_area_focus) {
		if (!view->the_locking_inset) {
			view->screen->CursorToggle();
		} else {
			view->the_locking_inset->
				ToggleInsetCursor(view);
		}
		goto set_timer_and_return;
	} else {
		// Make sure that the cursor is visible.
		if (!view->the_locking_inset) {
			view->screen->ShowCursor();
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
		if (tmpwin != view->owner_->getForm()->window) {
			view->lyx_focus = false;
			goto skip_timer;
		} else {
			view->lyx_focus = true;
			if (!view->work_area_focus)
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


static
string fromClipboard(Window win, XEvent * event)
{
	string strret;
	if (event->xselection.type == XA_STRING
	    && event->xselection.property) {
		Atom tmpatom;
		unsigned long ul1;
		unsigned long ul2;
		unsigned char * uc = 0;
		int tmpint;
		if (XGetWindowProperty(
			event->xselection.display,  // display
			win,                        // w
			event->xselection.property, // property
			0,                          // long_offset	
			0,                          // logn_length	
			False,                      // delete	
			XA_STRING,                  // req_type	
			&tmpatom,                   // actual_type_return
			&tmpint,                    // actual_format_return
			&ul1,
			&ul2,
			&uc                         // prop_return	
			) != Success) {
			return strret;
		}
		if (uc) {
			free(uc);
			uc = 0;
		}
		if (XGetWindowProperty(
			event->xselection.display,             // display
			win,                        // w
			event->xselection.property, // property
			0,                          // long_offset
			ul2/4+1,                    // long_length
			True,                       // delete
			XA_STRING,                  // req_type
			&tmpatom,                   // actual_type_return
			&tmpint,                    // actual_format_return
			&ul1,                       // nitems_return
			&ul2,                       // bytes_after_return
			&uc                         // prop_return */
			) != Success) {
			return strret;
		}
		if (uc) {
			strret = reinterpret_cast<char*>(uc);
			free(uc); // yes free!
			uc = 0;
		}
	}
	return strret;
}


void BufferView::workAreaSelectionNotify(Window win, XEvent * event)
{
	if (buffer_ == 0) return;

	screen->HideCursor();
	beforeChange();
	string clb = fromClipboard(win, event);
	if (!clb.empty()) {
		if (!ascii_type)
			text->InsertStringA(clb.c_str());
		else
			text->InsertStringB(clb.c_str());

		update(1);
		
	}
}


void BufferView::cursorPrevious()
{
	if (!text->cursor.row->previous) return;
	
	long y = screen->first;
	Row * cursorrow = text->cursor.row;
	text->SetCursorFromCoordinates(text->cursor.x_fix, y);
	text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == text->cursor.row))
		text->CursorUp();
	
  	if (text->cursor.row->height < workarea->height())
		screen->Draw(text->cursor.y
				  - text->cursor.row->baseline
				  + text->cursor.row->height
				  - workarea->height() + 1 );
}


void BufferView::cursorNext()
{
	if (!text->cursor.row->next) return;
	
	long y = screen->first;
	text->GetRowNearY(y);
	Row * cursorrow = text->cursor.row;
	text->SetCursorFromCoordinates(text->cursor.x_fix, y
				       + workarea->height());
	text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == text->cursor.row))
		text->CursorDown();
	
 	if (text->cursor.row->height < workarea->height())
		screen->Draw(text->cursor.y
				  - text->cursor.row->baseline);
}


bool BufferView::available() const
{
	if (buffer_ && text) return true;
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
	backstack.push(buffer()->fileName(),
		       text->cursor.x,
		       text->cursor.y);
}


void BufferView::restorePosition()
{
	if (backstack.empty()) return;
	
	int  x, y;
	string fname = backstack.pop(&x, &y);
	
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
	screen->SmallUpdate();
	if (screen->TopCursorVisible()
	    != screen->first) {
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
	if (!lyxrc->rtl_support)
		return;
	
	if (text->real_current_font.getFontDirection()
	    == LYX_DIR_LEFT_TO_RIGHT) {
		if (!owner_->getIntl()->primarykeymap)
			owner_->getIntl()->KeyMapPrim();
	} else {
		if (owner_->getIntl()->primarykeymap)
			owner_->getIntl()->KeyMapSec();
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
	return workarea->hasFocus();
}


void BufferView::focus(bool f)
{
	if (f) workarea->setFocus();
}


bool BufferView::active() const
{
	return workarea->active();
}


Painter & BufferView::getPainter() const
{
    return workarea->getPainter();
}


unsigned short BufferView::paperWidth() const
{
    return text->paperWidth();
}


bool BufferView::belowMouse() const 
{
	return workarea->belowMouse();
}


void BufferView::showCursor()
{
	if (screen)
		screen->ShowCursor();
}


void BufferView::hideCursor()
{
	if (screen)
		screen->HideCursor();
}


void BufferView::toggleSelection(bool b)
{
	if (screen)
		screen->ToggleSelection(b);
}


void BufferView::toggleToggle()
{
	if (screen)
		screen->ToggleToggle();
}


void BufferView::center() 
{
	beforeChange();
	if (text->cursor.y > workarea->height() / 2) {
		screen->Draw(text->cursor.y - workarea->height() / 2);
	} else {
		screen->Draw(0);
	}
	update(0);
	redraw();
}
