#include <config.h>

#include <unistd.h>
#include <sys/wait.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "BufferView_pimpl.h"
#include "WorkArea.h"
#include "lyxscreen.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "LyXView.h"
#include "commandtags.h"
#include "lyxfunc.h"
#include "minibuffer.h"
#include "font.h"
#include "bufferview_funcs.h"
#include "TextCache.h"
#include "bufferlist.h"
#include "insets/insetbib.h"
#include "menus.h"
#include "lyx_gui_misc.h"
#include "lyxrc.h"
#include "intl.h"

using std::pair;
using std::endl;

/* the selection possible is needed, that only motion events are 
 * used, where the bottom press event was on the drawing area too */
bool selection_possible = false;

extern BufferList bufferlist;
extern char ascii_type;

extern void sigchldhandler(pid_t pid, int * status);
extern int bibitemMaxWidth(Painter &, LyXFont const &);
extern void FreeUpdateTimer();

extern "C"
void C_BufferView_CursorToggleCB(FL_OBJECT * ob, long buf);


static inline
void waitForX()
{
	XSync(fl_get_display(), 0);
}


BufferView::Pimpl::Pimpl(BufferView * b, LyXView * o,
	     int xpos, int ypos, int width, int height)
	: bv_(b), owner_(o)
{
	buffer_ = 0;
	workarea = new WorkArea(bv_, xpos, ypos, width, height);
	screen = 0;
	timer_cursor = 0;
	create_view();
	current_scrollbar_value = 0;
	fl_set_timer(timer_cursor, 0.4);
	workarea->setFocus();
	work_area_focus = true;
	lyx_focus = false;
}


Painter & BufferView::Pimpl::painter() 
{
	return workarea->getPainter();
}


void BufferView::Pimpl::buffer(Buffer * b)
{
	lyxerr[Debug::INFO] << "Setting buffer in BufferView ("
			    << b << ")" << endl;
	if (buffer_) {
		bv_->insetSleep();
		buffer_->delUser(bv_);

		// Put the old text into the TextCache, but
		// only if the buffer is still loaded.
		// Also set the owner of the test to 0
		bv_->text->owner(0);
		textcache.add(bv_->text);
		if (lyxerr.debugging())
			textcache.show(lyxerr, "BufferView::buffer");
		
		bv_->text = 0;
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
		buffer_->addUser(bv_);
		owner_->getMenus()->showMenus();
		// If we don't have a text object for this, we make one
		if (bv_->text == 0)
			resizeCurrentBuffer();
		else {
			updateScreen();
			updateScrollbar();
		}
		screen->first = screen->TopCursorVisible();
		redraw();
		updateAllVisibleBufferRelatedPopups();
		bv_->insetWakeup();
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

void BufferView::Pimpl::resize(int xpos, int ypos, int width, int height)
{
	workarea->resize(xpos, ypos, width, height);
	update(3);
	redraw();
}


void BufferView::Pimpl::resize()
{
	// This will resize the buffer. (Asger)
	if (buffer_)
		resizeCurrentBuffer();
}


void BufferView::Pimpl::redraw()
{
	lyxerr[Debug::INFO] << "BufferView::redraw()" << endl;
	workarea->redraw();
}


void BufferView::Pimpl::fitCursor()
{
	if (screen) screen->FitCursor();
	updateScrollbar();
}


void BufferView::Pimpl::redoCurrentBuffer()
{
	lyxerr[Debug::INFO] << "BufferView::redoCurrentBuffer" << endl;
	if (buffer_ && bv_->text) {
		resize();
		owner_->updateLayoutChoice();
	}
}


int BufferView::Pimpl::resizeCurrentBuffer()
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

	ProhibitInput(bv_);

	owner_->getMiniBuffer()->Set(_("Formatting document..."));   

	if (bv_->text) {
		par = bv_->text->cursor.par;
		pos = bv_->text->cursor.pos;
		selstartpar = bv_->text->sel_start_cursor.par;
		selstartpos = bv_->text->sel_start_cursor.pos;
		selendpar = bv_->text->sel_end_cursor.par;
		selendpos = bv_->text->sel_end_cursor.pos;
		selection = bv_->text->selection;
		mark_set = bv_->text->mark_set;
		delete bv_->text;
		bv_->text = new LyXText(bv_, workarea->workWidth(), buffer_);
	} else {
		// See if we have a text in TextCache that fits
		// the new buffer_ with the correct width.
		bv_->text = textcache.findFit(buffer_, workarea->workWidth());
		if (bv_->text) {
			if (lyxerr.debugging()) {
				lyxerr << "Found a LyXText that fits:\n";
				textcache.show(lyxerr, bv_->text);
			}
			// Set the owner of the newly found text
			bv_->text->owner(bv_);
			if (lyxerr.debugging())
				textcache.show(lyxerr, "resizeCurrentBuffer");
		} else {
			bv_->text = new LyXText(bv_, workarea->workWidth(), buffer_);
		}
	}
	updateScreen();

	if (par) {
		bv_->text->selection = true;
		/* at this point just to avoid the Delete-Empty-Paragraph
		 * Mechanism when setting the cursor */
		bv_->text->mark_set = mark_set;
		if (selection) {
			bv_->text->SetCursor(selstartpar, selstartpos);
			bv_->text->sel_cursor = bv_->text->cursor;
			bv_->text->SetCursor(selendpar, selendpos);
			bv_->text->SetSelection();
			bv_->text->SetCursor(par, pos);
		} else {
			bv_->text->SetCursor(par, pos);
			bv_->text->sel_cursor = bv_->text->cursor;
			bv_->text->selection = false;
		}
	}
	screen->first = screen->TopCursorVisible(); /* this will scroll the
						     * screen such that the
						     * cursor becomes
						     * visible */ 
	updateScrollbar();
	redraw();
	owner_->getMiniBuffer()->Init();
	bv_->setState();
	AllowInput(bv_);

	// Now if the title form still exist kill it
	TimerCB(0, 0);

	return 0;
}


void BufferView::Pimpl::gotoError()
{
	if (!screen)
		return;
   
	screen->HideCursor();
	bv_->beforeChange();
	update(-2);
	LyXCursor tmp;

	if (!bv_->text->GotoNextError()) {
		if (bv_->text->cursor.pos 
		    || bv_->text->cursor.par != bv_->text->FirstParagraph()) {
			tmp = bv_->text->cursor;
			bv_->text->cursor.par = bv_->text->FirstParagraph();
			bv_->text->cursor.pos = 0;
			if (!bv_->text->GotoNextError()) {
				bv_->text->cursor = tmp;
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
	bv_->text->sel_cursor = bv_->text->cursor;
}


void BufferView::Pimpl::updateScreen()
{
	// Regenerate the screen.
	delete screen;
	screen = new LyXScreen(*workarea, bv_->text);
}


void BufferView::Pimpl::create_view()
{
	FL_OBJECT * obj;

	//
	// TIMERS
	//
	
	// timer_cursor
	timer_cursor = obj = fl_add_timer(FL_HIDDEN_TIMER,
					  0, 0, 0, 0, "Timer");
	fl_set_object_callback(obj, C_BufferView_CursorToggleCB, 0);
	obj->u_vdata = bv_;
}


void BufferView::Pimpl::updateScrollbar()
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

	if (bv_->text)
		cbth = bv_->text->height;
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

	workarea->setScrollbarBounds(0, bv_->text->height - workarea->height());
	double lineh = bv_->text->DefaultHeight();
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


// Callback for scrollbar slider
void BufferView::Pimpl::scrollCB(double value)
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
		LyXText * vbt = bv_->text;
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
void BufferView::Pimpl::downCB(long time, int button)
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


int BufferView::Pimpl::scrollUp(long time)
{
	if (buffer_ == 0) return 0;
	if (!screen) return 0;
   
	double value = workarea->getScrollbarValue();
   
	if (value == 0) return 0;

	float add_value =  (bv_->text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > workarea->height())
		add_value = float(workarea->height() -
				  bv_->text->DefaultHeight());
   
	value -= add_value;

	if (value < 0)
		value = 0;
   
	workarea->setScrollbarValue(value);
   
	bv_->scrollCB(value); 
	return 0;
}


int BufferView::Pimpl::scrollDown(long time)
{
	if (buffer_ == 0) return 0;
	if (!screen) return 0;
   
	double value= workarea->getScrollbarValue();
	pair<double, double> p = workarea->getScrollbarBounds();
	double max = p.second;
	
	if (value == max) return 0;

	float add_value =  (bv_->text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > workarea->height())
		add_value = float(workarea->height() -
				  bv_->text->DefaultHeight());
   
	value += add_value;
   
	if (value > max)
		value = max;

	workarea->setScrollbarValue(value);
	
	bv_->scrollCB(value); 
	return 0;
}


void BufferView::Pimpl::scrollUpOnePage()
{
	if (buffer_ == 0) return;
	if (!screen) return;
   
	long y = screen->first;

	if (!y) return;

	Row * row = bv_->text->GetRowNearY(y);

	y = y - workarea->height() + row->height;

	workarea->setScrollbarValue(y);
	
	bv_->scrollCB(y); 
}


void BufferView::Pimpl::scrollDownOnePage()
{
	if (buffer_ == 0) return;
	if (!screen) return;
   
	long y = screen->first;

	if (y > bv_->text->height - workarea->height())
		return;
   
	y += workarea->height();
	bv_->text->GetRowNearY(y);

	workarea->setScrollbarValue(y);
	
	bv_->scrollCB(y); 
}


void BufferView::Pimpl::workAreaMotionNotify(int x, int y, unsigned int state)
{
	if (buffer_ == 0 || !screen) return;

	// Check for inset locking
	if (bv_->the_locking_inset) {
		LyXCursor cursor = bv_->text->cursor;
		bv_->the_locking_inset->
			InsetMotionNotify(bv_,
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

		bv_->text->SetCursorFromCoordinates(x, y + screen->first);
      
		if (!bv_->text->selection)
			update(-3); // Maybe an empty line was deleted
      
		bv_->text->SetSelection();
		screen->ToggleToggle();
		if (screen->FitCursor())
			updateScrollbar(); 
		screen->ShowCursor();
	}
	return;
}


// Single-click on work area
void BufferView::Pimpl::workAreaButtonPress(int xpos, int ypos,
					    unsigned int button)
{
	last_click_x = -1;
	last_click_y = -1;

	if (buffer_ == 0 || !screen) return;

	Inset * inset_hit = checkInsetHit(xpos, ypos, button);

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
	
	if (bv_->the_locking_inset) {
		// We are in inset locking mode
		
		/* Check whether the inset was hit. If not reset mode,
		   otherwise give the event to the inset */
		if (inset_hit == bv_->the_locking_inset) {
			bv_->the_locking_inset->
				InsetButtonPress(bv_,
						 xpos, ypos,
						 button);
			return;
		} else {
			bv_->unlockInset(bv_->the_locking_inset);
		}
	}
	
	if (!inset_hit)
		selection_possible = true;
	screen->HideCursor();
	
	// Right button mouse click on a table
	if (button == 3 &&
	    (bv_->text->cursor.par->table ||
	     bv_->text->MouseHitInTable(xpos, ypos + screen->first))) {
		// Set the cursor to the press-position
		bv_->text->SetCursorFromCoordinates(xpos, ypos + screen->first);
		bool doit = true;
		
		// Only show the table popup if the hit is in
		// the table, too
		if (!bv_->text->HitInTable(bv_->text->cursor.row, xpos))
			doit = false;
		
		// Hit above or below the table?
		if (doit) {
			if (!bv_->text->selection) {
				screen->ToggleSelection();
				bv_->text->ClearSelection();
				bv_->text->FullRebreak();
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
	    && bv_->text->selection) {
		owner_->getLyXFunc()->Dispatch(LFUN_COPY);
		paste_internally = true;
	}
	
	// Clear the selection
	screen->ToggleSelection();
	bv_->text->ClearSelection();
	bv_->text->FullRebreak();
	screen->Update();
	updateScrollbar();
	
	// Single left click in math inset?
	if ((inset_hit != 0) &&
	    (inset_hit->Editable()==Inset::HIGHLY_EDITABLE)) {
		// Highly editable inset, like math
		UpdatableInset * inset = static_cast<UpdatableInset *>(inset_hit);
		selection_possible = false;
		owner_->updateLayoutChoice();
		owner_->getMiniBuffer()->Set(inset->EditMessage());
		inset->InsetButtonPress(bv_, xpos, ypos, button);
		inset->Edit(bv_, xpos, ypos, button);
		return;
	} 
	
	// Right click on a footnote flag opens float menu
	if (button == 3) { 
		selection_possible = false;
		return;
	}
	
	if (!inset_hit) // otherwise it was already set in checkInsetHit(...)
		bv_->text->SetCursorFromCoordinates(xpos, ypos + screen_first);
	bv_->text->FinishUndo();
	bv_->text->sel_cursor = bv_->text->cursor;
	bv_->text->cursor.x_fix = bv_->text->cursor.x;
	
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


void BufferView::Pimpl::doubleClick(int /*x*/, int /*y*/, unsigned int button) 
{
	// select a word
	if (buffer_ && !bv_->the_locking_inset) {
		if (screen && button == 1) {
			screen->HideCursor();
			screen->ToggleSelection();
			bv_->text->SelectWord();
			screen->ToggleSelection(false);
			/* This will fit the cursor on the screen
			 * if necessary */
			update(0);
		}
	}            
}


void BufferView::Pimpl::tripleClick(int /*x*/, int /*y*/, unsigned int button)
{
	// select a line
	if (buffer_ && screen && button == 1) {
		screen->HideCursor();
		screen->ToggleSelection();
		bv_->text->CursorHome();
		bv_->text->sel_cursor = bv_->text->cursor;
		bv_->text->CursorEnd();
		bv_->text->SetSelection();
		screen->ToggleSelection(false);
		/* This will fit the cursor on the screen
		 * if necessary */
		update(0);
	}
}


void BufferView::Pimpl::workAreaButtonRelease(int x, int y, unsigned int button)
{
	if (buffer_ == 0 || screen == 0) return;

	// If we hit an inset, we have the inset coordinates in these
	// and inset_hit points to the inset.  If we do not hit an
	// inset, inset_hit is 0, and inset_x == x, inset_y == y.
	Inset * inset_hit = checkInsetHit(x, y, button);

	if (bv_->the_locking_inset) {
		// We are in inset locking mode.

		/* LyX does a kind of work-area grabbing for insets.
		   Only a ButtonPress Event outside the inset will 
		   force a InsetUnlock. */
		bv_->the_locking_inset->
			InsetButtonRelease(bv_, x, y, button);
		return;
	}
	
	selection_possible = false;
        if (bv_->text->cursor.par->table) {
                int cell = bv_->text->
                        NumberOfCell(bv_->text->cursor.par,
                                     bv_->text->cursor.pos);
                if (bv_->text->cursor.par->table->IsContRow(cell) &&
                    bv_->text->cursor.par->table->
                    CellHasContRow(bv_->text->cursor.par->table->
                                   GetCellAbove(cell))<0) {
                        bv_->text->CursorUp();
                }
        }
	
	if (button >= 2) return;

	bv_->setState();
	owner_->getMiniBuffer()->Set(CurrentState(bv_));

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
			bv_->text->SetCursorParUndo();
		}

		owner_->getMiniBuffer()->Set(inset_hit->EditMessage());
		if (inset_hit->Editable()==Inset::HIGHLY_EDITABLE) {
			// Highly editable inset, like math
			UpdatableInset *inset = (UpdatableInset *)inset_hit;
			inset->InsetButtonRelease(bv_, x, y, button);
		} else {
			inset_hit->Edit(bv_, x, y, button);
		}
		return;
	}

	// check whether we want to open a float
	if (bv_->text) {
		bool hit = false;
		char c = ' ';
		if (bv_->text->cursor.pos <
		    bv_->text->cursor.par->Last()) {
			c = bv_->text->cursor.par->
				GetChar(bv_->text->cursor.pos);
		}
		if (c == LyXParagraph::META_FOOTNOTE
		    || c == LyXParagraph::META_MARGIN
		    || c == LyXParagraph::META_FIG
		    || c == LyXParagraph::META_TAB
		    || c == LyXParagraph::META_WIDE_FIG
		    || c == LyXParagraph::META_WIDE_TAB
                    || c == LyXParagraph::META_ALGORITHM){
			hit = true;
		} else if (bv_->text->cursor.pos - 1 >= 0) {
			c = bv_->text->cursor.par->
				GetChar(bv_->text->cursor.pos - 1);
			if (c == LyXParagraph::META_FOOTNOTE
			    || c == LyXParagraph::META_MARGIN
			    || c == LyXParagraph::META_FIG
			    || c == LyXParagraph::META_TAB
			    || c == LyXParagraph::META_WIDE_FIG 
			    || c == LyXParagraph::META_WIDE_TAB
			    || c == LyXParagraph::META_ALGORITHM){
				// We are one step too far to the right
				bv_->text->CursorLeft();
				hit = true;
			}
		}
		if (hit == true) {
			bv_->toggleFloat();
			selection_possible = false;
			return;
		}
	}

	// Do we want to close a float? (click on the float-label)
	if (bv_->text->cursor.row->par->footnoteflag == 
	    LyXParagraph::OPEN_FOOTNOTE
	    //&& text->cursor.pos == 0
	    && bv_->text->cursor.row->previous &&
	    bv_->text->cursor.row->previous->par->
	    footnoteflag != LyXParagraph::OPEN_FOOTNOTE){
		LyXFont font(LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_FOOTNOTE);

		int box_x = 20; // LYX_PAPER_MARGIN;
		box_x += lyxfont::width(" wide-tab ", font);

		int screen_first = screen->first;

		if (x < box_x
		    && y + screen_first > bv_->text->cursor.y -
		    bv_->text->cursor.row->baseline
		    && y + screen_first < bv_->text->cursor.y -
		    bv_->text->cursor.row->baseline
		    + lyxfont::maxAscent(font) * 1.2 + lyxfont::maxDescent(font) * 1.2) {
			bv_->toggleFloat();
			selection_possible = false;
			return;
		}
	}

	// Maybe we want to edit a bibitem ale970302
	if (bv_->text->cursor.par->bibkey && x < 20 + 
	    bibitemMaxWidth(bv_->painter(),
			    textclasslist
			    .TextClass(buffer_->
				       params.textclass).defaultfont())) {
		bv_->text->cursor.par->bibkey->Edit(bv_, 0, 0, 0);
	}

	return;
}


/* 
 * Returns an inset if inset was hit. 0 otherwise.
 * If hit, the coordinates are changed relative to the inset. 
 * Otherwise coordinates are not changed, and false is returned.
 */
Inset * BufferView::Pimpl::checkInsetHit(int & x, int & y, unsigned int button)
{
	if (!screen)
		return 0;
  
	int y_tmp = y + screen->first;
  
	LyXCursor cursor;
	bv_->text->SetCursorFromCoordinates(cursor, x, y_tmp);
#if 1
	bool move_cursor = true;
#else
	bool move_cursor = ((cursor.par != bv_->text->cursor.par) ||
			    (cursor.pos != bv_->text->cursor.pos)) && (button < 2);
#endif

	if (cursor.pos < cursor.par->Last()
	    && cursor.par->GetChar(cursor.pos) == LyXParagraph::META_INSET
	    && cursor.par->GetInset(cursor.pos)
	    && cursor.par->GetInset(cursor.pos)->Editable()) {

		// Check whether the inset really was hit
		Inset * tmpinset = cursor.par->GetInset(cursor.pos);
		LyXFont font = bv_->text->GetFont(cursor.par, cursor.pos);
		bool is_rtl = font.isVisibleRightToLeft();
		int start_x, end_x;

		if (is_rtl) {
			start_x = cursor.x - tmpinset->width(bv_->painter(), font);
			end_x = cursor.x;
		} else {
			start_x = cursor.x;
			end_x = cursor.x + tmpinset->width(bv_->painter(), font);
		}

		if (x > start_x && x < end_x
		    && y_tmp > cursor.y - tmpinset->ascent(bv_->painter(), font)
		    && y_tmp < cursor.y + tmpinset->descent(bv_->painter(), font)) {
			if (move_cursor)
				bv_->text->SetCursorFromCoordinates(x, y_tmp);
			x = x - start_x;
			// The origin of an inset is on the baseline
			y = y_tmp - (bv_->text->cursor.y); 
			return tmpinset;
		}
	}

	if ((cursor.pos - 1 >= 0) &&
	    (cursor.par->GetChar(cursor.pos-1) == LyXParagraph::META_INSET) &&
	    (cursor.par->GetInset(cursor.pos - 1)) &&
	    (cursor.par->GetInset(cursor.pos - 1)->Editable())) {
		Inset * tmpinset = cursor.par->GetInset(cursor.pos-1);
		LyXFont font = bv_->text->GetFont(cursor.par, cursor.pos-1);
		bool is_rtl = font.isVisibleRightToLeft();
		int start_x, end_x;

		if (!is_rtl) {
			start_x = cursor.x - tmpinset->width(bv_->painter(), font);
			end_x = cursor.x;
		} else {
			start_x = cursor.x;
			end_x = cursor.x + tmpinset->width(bv_->painter(), font);
		}
		if (x > start_x && x < end_x
		    && y_tmp > cursor.y - tmpinset->ascent(bv_->painter(), font)
		    && y_tmp < cursor.y + tmpinset->descent(bv_->painter(), font)) {
			if (move_cursor)
				bv_->text->SetCursorFromCoordinates(x, y_tmp);
			x = x - start_x;
			// The origin of an inset is on the baseline
			y = y_tmp - (bv_->text->cursor.y); 
			return tmpinset;
		}
	}
	return 0;
}


void BufferView::Pimpl::workAreaExpose()
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


void BufferView::Pimpl::workAreaSelectionNotify(Window win, XEvent * event)
{
	if (buffer_ == 0) return;

	screen->HideCursor();
	bv_->beforeChange();
	string clb = fromClipboard(win, event);
	if (!clb.empty()) {
		if (!ascii_type)
			bv_->text->InsertStringA(clb);
		else
			bv_->text->InsertStringB(clb);

		update(1);
	}
}


void BufferView::Pimpl::update()
{
	if (screen) screen->Update();
}


void BufferView::Pimpl::update(signed char f)
{
	owner_->updateLayoutChoice();

	if (!bv_->text->selection && f > -3)
		bv_->text->sel_cursor = bv_->text->cursor;
	
	FreeUpdateTimer();
	bv_->text->FullRebreak();

	update();

	if (f != 3 && f != -3) {
		fitCursor();
		updateScrollbar();
      	}

	if (f == 1 || f == -1) {
		if (buffer_->isLyxClean()) {
			buffer_->markDirty();
			owner_->getMiniBuffer()->setTimer(4);
		} else {
			buffer_->markDirty();
		}
	}
}


void BufferView::Pimpl::smallUpdate(signed char f)
{
	screen->SmallUpdate();
	if (screen->TopCursorVisible()
	    != screen->first) {
		update(f);
		return;
	}

	fitCursor();
	updateScrollbar();

	if (!bv_->text->selection)
		bv_->text->sel_cursor = bv_->text->cursor;

	if (f == 1 || f == -1) {
		if (buffer_->isLyxClean()) {
			buffer_->markDirty();
			owner_->getMiniBuffer()->setTimer(4);
		} else {
			buffer_->markDirty();
		}
	}
}


// Callback for cursor timer
void BufferView::Pimpl::cursorToggle()
{
	// Quite a nice place for asyncron Inset updating, isn't it?
	// Actually no! This is run even if no buffer exist... so (Lgb)
	if (!buffer_) {
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

	updatelist.update(bv_);
	
	if (!screen) {
		goto set_timer_and_return;
	}

	if (lyx_focus && work_area_focus) {
		if (!bv_->the_locking_inset) {
			screen->CursorToggle();
		} else {
			bv_->the_locking_inset->
				ToggleInsetCursor(bv_);
		}
		goto set_timer_and_return;
	} else {
		// Make sure that the cursor is visible.
		if (!bv_->the_locking_inset) {
			screen->ShowCursor();
		} else {
			if (!bv_->the_locking_inset->isCursorVisible())
				bv_->the_locking_inset->
					ToggleInsetCursor(bv_);
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
		if (tmpwin != owner_->getForm()->window) {
			lyx_focus = false;
			goto skip_timer;
		} else {
			lyx_focus = true;
			if (!work_area_focus)
				goto skip_timer;
			else
				goto set_timer_and_return;
		}
	}

  set_timer_and_return:
	fl_set_timer(timer_cursor, 0.4);
  skip_timer:
	return;
}


void BufferView::Pimpl::cursorPrevious()
{
	if (!bv_->text->cursor.row->previous) return;
	
	long y = screen->first;
	Row * cursorrow = bv_->text->cursor.row;
	bv_->text->SetCursorFromCoordinates(bv_->text->cursor.x_fix, y);
	bv_->text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == bv_->text->cursor.row))
		bv_->text->CursorUp();
	
  	if (bv_->text->cursor.row->height < workarea->height())
		screen->Draw(bv_->text->cursor.y
				  - bv_->text->cursor.row->baseline
				  + bv_->text->cursor.row->height
				  - workarea->height() + 1 );
}


void BufferView::Pimpl::cursorNext()
{
	if (!bv_->text->cursor.row->next) return;
	
	long y = screen->first;
	bv_->text->GetRowNearY(y);
	Row * cursorrow = bv_->text->cursor.row;
	bv_->text->SetCursorFromCoordinates(bv_->text->cursor.x_fix, y
				       + workarea->height());
	bv_->text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == bv_->text->cursor.row))
		bv_->text->CursorDown();
	
 	if (bv_->text->cursor.row->height < workarea->height())
		screen->Draw(bv_->text->cursor.y
			     - bv_->text->cursor.row->baseline);
}


bool BufferView::Pimpl::available() const
{
	if (buffer_ && bv_->text) return true;
	return false;
}


void BufferView::Pimpl::beforeChange()
{
	toggleSelection();
	bv_->text->ClearSelection();
	FreeUpdateTimer();
}


void BufferView::Pimpl::savePosition()
{
	backstack.push(buffer_->fileName(),
		       bv_->text->cursor.x,
		       bv_->text->cursor.y);
}


void BufferView::Pimpl::restorePosition()
{
	if (backstack.empty()) return;
	
	int  x, y;
	string fname = backstack.pop(&x, &y);
	
	beforeChange();
	Buffer * b = bufferlist.exists(fname) ?
		bufferlist.getBuffer(fname) :
		bufferlist.loadLyXFile(fname); // don't ask, just load it
	buffer(b);
	bv_->text->SetCursorFromCoordinates(x, y);
	update(0);
} 


void BufferView::Pimpl::setState()
{
	if (!lyxrc.rtl_support)
		return;

	if (bv_->text->real_current_font.isRightToLeft() &&
	    bv_->text->real_current_font.latex() != LyXFont::ON) {
		if (owner_->getIntl()->primarykeymap)
			owner_->getIntl()->KeyMapSec();
	} else {
		if (!owner_->getIntl()->primarykeymap)
			owner_->getIntl()->KeyMapPrim();
	}
}


void BufferView::Pimpl::insetSleep()
{
	if (bv_->the_locking_inset && !bv_->inset_slept) {
		bv_->the_locking_inset->GetCursorPos(bv_->slx, bv_->sly);
		bv_->the_locking_inset->InsetUnlock(bv_);
		bv_->inset_slept = true;
	}
}


void BufferView::Pimpl::insetWakeup()
{
	if (bv_->the_locking_inset && bv_->inset_slept) {
		bv_->the_locking_inset->Edit(bv_, bv_->slx, bv_->sly, 0);
		bv_->inset_slept = false;
	}
}


void BufferView::Pimpl::insetUnlock()
{
	if (bv_->the_locking_inset) {
		if (!bv_->inset_slept) bv_->the_locking_inset->InsetUnlock(bv_);
		bv_->the_locking_inset = 0;
		bv_->text->FinishUndo();
		bv_->inset_slept = false;
	}
}


bool BufferView::Pimpl::focus() const
{
	return workarea->hasFocus();
}


void BufferView::Pimpl::focus(bool f)
{
	if (f) workarea->setFocus();
}


bool BufferView::Pimpl::active() const
{
	return workarea->active();
}


bool BufferView::Pimpl::belowMouse() const 
{
	return workarea->belowMouse();
}


void BufferView::Pimpl::showCursor()
{
	if (screen)
		screen->ShowCursor();
}


void BufferView::Pimpl::hideCursor()
{
	if (screen)
		screen->HideCursor();
}


void BufferView::Pimpl::toggleSelection(bool b)
{
	if (screen)
		screen->ToggleSelection(b);
}


void BufferView::Pimpl::toggleToggle()
{
	if (screen)
		screen->ToggleToggle();
}


void BufferView::Pimpl::center() 
{
	beforeChange();
	if (bv_->text->cursor.y > workarea->height() / 2) {
		screen->Draw(bv_->text->cursor.y - workarea->height() / 2);
	} else {
		screen->Draw(0);
	}
	update(0);
	redraw();
}
