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
#include "support/LAssert.h"
#include "frontends/Dialogs.h"

#ifdef SIGC_CXX_NAMESPACES
using SigC::slot;
#endif

using std::pair;
using std::endl;
using std::vector;
using std::make_pair;

/* the selection possible is needed, that only motion events are 
 * used, where the bottom press event was on the drawing area too */
bool selection_possible = false;

extern BufferList bufferlist;
extern char ascii_type;

extern void sigchldhandler(pid_t pid, int * status);
extern int bibitemMaxWidth(BufferView *, LyXFont const &);


static inline
void waitForX()
{
	XSync(fl_get_display(), 0);
}


static
void SetXtermCursor(Window win)
{
	static Cursor cursor;
	static bool cursor_undefined = true;
	if (cursor_undefined){
		cursor = XCreateFontCursor(fl_display, XC_xterm);
		XFlush(fl_display);
		cursor_undefined = false;
	}
	XDefineCursor(fl_display, win, cursor);
	XFlush(fl_display);
}


BufferView::Pimpl::Pimpl(BufferView * b, LyXView * o,
	     int xpos, int ypos, int width, int height)
	: bv_(b), owner_(o), cursor_timeout(400)
{
	buffer_ = 0;
	workarea_ = new WorkArea(bv_, xpos, ypos, width, height);
	screen_ = 0;

	cursor_timeout.timeout.connect(slot(this,
					    &BufferView::Pimpl::cursorToggle));
	current_scrollbar_value = 0;
	cursor_timeout.start();
	workarea_->setFocus();
	using_xterm_cursor = false;
}


Painter & BufferView::Pimpl::painter() 
{
	return workarea_->getPainter();
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
		//		bv_->text->owner(0);
		textcache.add(buffer_, workarea_->workWidth(), bv_->text);
		if (lyxerr.debugging())
			textcache.show(lyxerr, "BufferView::buffer");
		
		bv_->text = 0;
	}

	// Set current buffer
	buffer_ = b;

	if (bufferlist.getState() == BufferList::CLOSING) return;
	
	// Nuke old image
	// screen is always deleted when the buffer is changed.
	delete screen_;
	screen_ = 0;

	// If we are closing the buffer, use the first buffer as current
	if (!buffer_) {
		buffer_ = bufferlist.first();
	}

	if (buffer_) {
		lyxerr[Debug::INFO] << "Buffer addr: " << buffer_ << endl;
		buffer_->addUser(bv_);
		owner_->getMenus()->showMenus();
		// If we don't have a text object for this, we make one
		if (bv_->text == 0) {
			resizeCurrentBuffer();
		} else {
			updateScreen();
			updateScrollbar();
		}
		bv_->text->first = screen_->TopCursorVisible(bv_->text);
		redraw();
		owner_->getDialogs()->updateBufferDependent();
		bv_->insetWakeup();
	} else {
		lyxerr[Debug::INFO] << "  No Buffer!" << endl;
		owner_->getMenus()->hideMenus();
		updateScrollbar();
		workarea_->redraw();

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
	workarea_->resize(xpos, ypos, width, height);
	update(SELECT);
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
	workarea_->redraw();
}


bool BufferView::Pimpl::fitCursor()
{
	Assert(screen_); // it is a programming error to call fitCursor
	// without a valid screen.
	bool ret = screen_->FitCursor(bv_->text);
	if (ret) updateScrollbar();
	return ret;
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
		par = bv_->text->cursor.par();
		pos = bv_->text->cursor.pos();
		selstartpar = bv_->text->sel_start_cursor.par();
		selstartpos = bv_->text->sel_start_cursor.pos();
		selendpar = bv_->text->sel_end_cursor.par();
		selendpos = bv_->text->sel_end_cursor.pos();
		selection = bv_->text->selection;
		mark_set = bv_->text->mark_set;
		delete bv_->text;
		bv_->text = new LyXText(bv_);
	} else {
		// See if we have a text in TextCache that fits
		// the new buffer_ with the correct width.
		bv_->text = textcache.findFit(buffer_, workarea_->workWidth());
		if (bv_->text) {
			if (lyxerr.debugging()) {
				lyxerr << "Found a LyXText that fits:\n";
				textcache.show(lyxerr, make_pair(buffer_, make_pair(workarea_->workWidth(), bv_->text)));
			}
			// Set the owner of the newly found text
			//	bv_->text->owner(bv_);
			if (lyxerr.debugging())
				textcache.show(lyxerr, "resizeCurrentBuffer");
		} else {
			bv_->text = new LyXText(bv_);
		}
	}
	updateScreen();

	if (par) {
		bv_->text->selection = true;
		/* at this point just to avoid the Delete-Empty-Paragraph
		 * Mechanism when setting the cursor */
		bv_->text->mark_set = mark_set;
		if (selection) {
			bv_->text->SetCursor(bv_, selstartpar, selstartpos);
			bv_->text->sel_cursor = bv_->text->cursor;
			bv_->text->SetCursor(bv_, selendpar, selendpos);
			bv_->text->SetSelection();
			bv_->text->SetCursor(bv_, par, pos);
		} else {
			bv_->text->SetCursor(bv_, par, pos);
			bv_->text->sel_cursor = bv_->text->cursor;
			bv_->text->selection = false;
		}
	}
	bv_->text->first = screen_->TopCursorVisible(bv_->text);
	/* this will scroll the
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
	if (!screen_)
		return;
   
	screen_->HideCursor();
	bv_->beforeChange();
	update(BufferView::SELECT|BufferView::FITCUR);
	LyXCursor tmp;

	if (!bv_->text->GotoNextError(bv_)) {
		if (bv_->text->cursor.pos() 
		    || bv_->text->cursor.par() != bv_->text->FirstParagraph()) {
			tmp = bv_->text->cursor;
			bv_->text->cursor.par(bv_->text->FirstParagraph());
			bv_->text->cursor.pos(0);
			if (!bv_->text->GotoNextError(bv_)) {
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
	update(BufferView::SELECT|BufferView::FITCUR);
	bv_->text->sel_cursor = bv_->text->cursor;
}


void BufferView::Pimpl::updateScreen()
{
	// Regenerate the screen.
	delete screen_;
	screen_ = new LyXScreen(*workarea_);
}


void BufferView::Pimpl::updateScrollbar()
{
	/* If the text is smaller than the working area, the scrollbar
	 * maximum must be the working area height. No scrolling will 
	 * be possible */

	if (!buffer_) {
		workarea_->setScrollbar(0, 1.0);
		return;
	}
	
	static unsigned long max2 = 0;
	static unsigned long height2 = 0;

	unsigned long cbth = 0;
	long cbsf = 0;

	if (bv_->text) {
		cbth = bv_->text->height;
		cbsf = bv_->text->first;
	}

	// check if anything has changed.
	if (max2 == cbth &&
	    height2 == workarea_->height() &&
	    current_scrollbar_value == cbsf)
		return; // no
	max2 = cbth;
	height2 = workarea_->height();
	current_scrollbar_value = cbsf;

	if (cbth <= height2) { // text is smaller than screen
		workarea_->setScrollbar(0, 1.0); // right?
		return;
	}

	long maximum_height = workarea_->height() * 3 / 4 + cbth;
	long value = cbsf;

	// set the scrollbar
	double hfloat = workarea_->height();
	double maxfloat = maximum_height;

	float slider_size = 0.0;
	int slider_value = value;

	workarea_->setScrollbarBounds(0, bv_->text->height - workarea_->height());
	double lineh = bv_->text->DefaultHeight();
	workarea_->setScrollbarIncrements(lineh);
	if (maxfloat > 0.0) {
		if ((hfloat / maxfloat) * float(height2) < 3)
			slider_size = 3.0/float(height2);
		else
			slider_size = hfloat / maxfloat;
	} else
		slider_size = hfloat;

	workarea_->setScrollbar(slider_value, slider_size / workarea_->height());
}


// Callback for scrollbar slider
void BufferView::Pimpl::scrollCB(double value)
{
	extern bool cursor_follows_scrollbar;
	
	if (buffer_ == 0) return;

	current_scrollbar_value = long(value);
	if (current_scrollbar_value < 0)
		current_scrollbar_value = 0;
   
	if (!screen_)
		return;

	screen_->Draw(bv_->text, current_scrollbar_value);

	if (cursor_follows_scrollbar) {
		LyXText * vbt = bv_->text;
		unsigned int height = vbt->DefaultHeight();
		
		if (vbt->cursor.y() < bv_->text->first + height) {
			vbt->SetCursorFromCoordinates(bv_, 0,
						      bv_->text->first +
						      height);
		} else if (vbt->cursor.y() >
			   bv_->text->first + workarea_->height() - height) {
			vbt->SetCursorFromCoordinates(bv_, 0,
						      bv_->text->first +
						      workarea_->height()  -
						      height);
		}
	}
	waitForX();
}


int BufferView::Pimpl::scrollUp(long time)
{
	if (buffer_ == 0) return 0;
	if (!screen_) return 0;
   
	double value = workarea_->getScrollbarValue();
   
	if (value == 0) return 0;

	float add_value =  (bv_->text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > workarea_->height())
		add_value = float(workarea_->height() -
				  bv_->text->DefaultHeight());
   
	value -= add_value;

	if (value < 0)
		value = 0;
   
	workarea_->setScrollbarValue(value);
   
	bv_->scrollCB(value); 
	return 0;
}


int BufferView::Pimpl::scrollDown(long time)
{
	if (buffer_ == 0) return 0;
	if (!screen_) return 0;
   
	double value= workarea_->getScrollbarValue();
	pair<float, float> p = workarea_->getScrollbarBounds();
	double max = p.second;
	
	if (value == max) return 0;

	float add_value =  (bv_->text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > workarea_->height())
		add_value = float(workarea_->height() -
				  bv_->text->DefaultHeight());
   
	value += add_value;
   
	if (value > max)
		value = max;

	workarea_->setScrollbarValue(value);
	
	bv_->scrollCB(value); 
	return 0;
}


void BufferView::Pimpl::workAreaMotionNotify(int x, int y, unsigned int state)
{
	// Only use motion with button 1
	if (!(state & Button1MotionMask))
		return;

	if (buffer_ == 0 || !screen_) return;

	// Check for inset locking
	if (bv_->the_locking_inset) {
		LyXCursor cursor = bv_->text->cursor;
		bv_->the_locking_inset->
			InsetMotionNotify(bv_,
					  x - cursor.x(),
					  y - cursor.y() + bv_->text->first,
					  state);
		return;
	}
   
	/* The selection possible is needed, that only motion events are 
	 * used, where the bottom press event was on the drawing area too */
	if (selection_possible) {
		screen_->HideCursor();

		bv_->text->SetCursorFromCoordinates(bv_, x, y + bv_->text->first);
      
		if (!bv_->text->selection)
			update(BufferView::UPDATE); // Maybe an empty line was deleted
      
		bv_->text->SetSelection();
		screen_->ToggleToggle(bv_->text);
		fitCursor();
		screen_->ShowCursor(bv_->text);
	}
	return;
}


// Single-click on work area
void BufferView::Pimpl::workAreaButtonPress(int xpos, int ypos,
					    unsigned int button)
{
	last_click_x = -1;
	last_click_y = -1;

	if (buffer_ == 0 || !screen_) return;

	Inset * inset_hit = checkInsetHit(bv_->text, xpos, ypos, button);

	// ok ok, this is a hack.
	if (button == 4 || button == 5) {
		switch (button) {
		case 4:
			scrollUp(lyxrc.wheel_jump); // default 100, set in lyxrc
			break;
		case 5:
			scrollDown(lyxrc.wheel_jump);
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
	screen_->HideCursor();

#ifndef NEW_TABULAR
	// Right button mouse click on a table
	if (button == 3 &&
	    (bv_->text->cursor.par()->table ||
	     bv_->text->MouseHitInTable(bv_, xpos, ypos + bv_->text->first))) {
		// Set the cursor to the press-position
		bv_->text->SetCursorFromCoordinates(bv_, xpos, ypos + bv_->text->first);
		bool doit = true;
		
		// Only show the table popup if the hit is in
		// the table, too
		if (!bv_->text->HitInTable(bv_,
					   bv_->text->cursor.row(), xpos))
			doit = false;
		
		// Hit above or below the table?
		if (doit) {
			if (!bv_->text->selection) {
				screen_->ToggleSelection(bv_->text);
				bv_->text->ClearSelection();
				bv_->text->FullRebreak(bv_);
				screen_->Update(bv_->text);
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
#endif
	
	int screen_first = bv_->text->first;
	
	// Middle button press pastes if we have a selection
	bool paste_internally = false;
	if (button == 2
	    && bv_->text->selection) {
		owner_->getLyXFunc()->Dispatch(LFUN_COPY);
		paste_internally = true;
	}
	
	// Clear the selection
	screen_->ToggleSelection(bv_->text);
	bv_->text->ClearSelection();
	bv_->text->FullRebreak(bv_);
	screen_->Update(bv_->text);
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
		bv_->text->SetCursorFromCoordinates(bv_, xpos, ypos + screen_first);
	bv_->text->FinishUndo();
	bv_->text->sel_cursor = bv_->text->cursor;
	bv_->text->cursor.x_fix(bv_->text->cursor.x());
	
	owner_->updateLayoutChoice();
	if (fitCursor()) {
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
		if (screen_ && button == 1) {
			screen_->HideCursor();
			screen_->ToggleSelection(bv_->text);
			bv_->text->SelectWord(bv_);
			screen_->ToggleSelection(bv_->text, false);
			/* This will fit the cursor on the screen
			 * if necessary */
			update(BufferView::SELECT|BufferView::FITCUR);
		}
	}            
}


void BufferView::Pimpl::tripleClick(int /*x*/, int /*y*/, unsigned int button)
{
	// select a line
	if (buffer_ && screen_ && button == 1) {
		screen_->HideCursor();
		screen_->ToggleSelection(bv_->text);
		bv_->text->CursorHome(bv_);
		bv_->text->sel_cursor = bv_->text->cursor;
		bv_->text->CursorEnd(bv_);
		bv_->text->SetSelection();
		screen_->ToggleSelection(bv_->text, false);
		/* This will fit the cursor on the screen
		 * if necessary */
		update(BufferView::SELECT|BufferView::FITCUR);
	}
}


void BufferView::Pimpl::enterView()
{
	if (active() && available()) {
		SetXtermCursor(workarea_->getWin());
		using_xterm_cursor = true;
	}
}


void BufferView::Pimpl::leaveView()
{
	if (using_xterm_cursor) {
		XUndefineCursor(fl_display, workarea_->getWin());
		using_xterm_cursor = false;
	}
}


void BufferView::Pimpl::workAreaButtonRelease(int x, int y,
					      unsigned int button)
{
	if (buffer_ == 0 || screen_ == 0) return;

	// If we hit an inset, we have the inset coordinates in these
	// and inset_hit points to the inset.  If we do not hit an
	// inset, inset_hit is 0, and inset_x == x, inset_y == y.
	Inset * inset_hit = checkInsetHit(bv_->text, x, y, button);

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
#ifndef NEW_TABULAR
	if (bv_->text->cursor.par()->table) {
                int cell = bv_->text->
                        NumberOfCell(bv_->text->cursor.par(),
                                     bv_->text->cursor.pos());
                if (bv_->text->cursor.par()->table->IsContRow(cell) &&
                    bv_->text->cursor.par()->table->
                    CellHasContRow(bv_->text->cursor.par()->table->
                                   GetCellAbove(cell))<0) {
                        bv_->text->CursorUp(bv_);
                }
        }
#endif
	
	if (button >= 2) return;

	bv_->setState();
	owner_->getMiniBuffer()->Set(CurrentState(bv_));

	// Did we hit an editable inset?
	if (inset_hit != 0) {
		// Inset like error, notes and figures
		selection_possible = false;

		// CHECK fix this proper in 0.13

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
			bv_->text->SetCursorParUndo(bv_->buffer());
		}

		owner_->getMiniBuffer()->Set(inset_hit->EditMessage());
		if (inset_hit->Editable()==Inset::HIGHLY_EDITABLE) {
			// Highly editable inset, like math
			UpdatableInset *inset = (UpdatableInset *)inset_hit;
			inset->InsetButtonRelease(bv_, x, y, button);
		} else {
			inset_hit->InsetButtonRelease(bv_, x, y, button);
			inset_hit->Edit(bv_, x, y, button);
		}
		return;
	}

	// check whether we want to open a float
	if (bv_->text) {
		bool hit = false;
		char c = ' ';
		if (bv_->text->cursor.pos() <
		    bv_->text->cursor.par()->Last()) {
			c = bv_->text->cursor.par()->
				GetChar(bv_->text->cursor.pos());
		}
		if (c == LyXParagraph::META_FOOTNOTE
		    || c == LyXParagraph::META_MARGIN
		    || c == LyXParagraph::META_FIG
		    || c == LyXParagraph::META_TAB
		    || c == LyXParagraph::META_WIDE_FIG
		    || c == LyXParagraph::META_WIDE_TAB
                    || c == LyXParagraph::META_ALGORITHM){
			hit = true;
		} else if (bv_->text->cursor.pos() - 1 >= 0) {
			c = bv_->text->cursor.par()->
				GetChar(bv_->text->cursor.pos() - 1);
			if (c == LyXParagraph::META_FOOTNOTE
			    || c == LyXParagraph::META_MARGIN
			    || c == LyXParagraph::META_FIG
			    || c == LyXParagraph::META_TAB
			    || c == LyXParagraph::META_WIDE_FIG 
			    || c == LyXParagraph::META_WIDE_TAB
			    || c == LyXParagraph::META_ALGORITHM){
				// We are one step too far to the right
				bv_->text->CursorLeft(bv_);
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
	if (bv_->text->cursor.row()->par()->footnoteflag == 
	    LyXParagraph::OPEN_FOOTNOTE
	    && bv_->text->cursor.row()->previous() &&
	    bv_->text->cursor.row()->previous()->par()->
	    footnoteflag != LyXParagraph::OPEN_FOOTNOTE){
		LyXFont font(LyXFont::ALL_SANE);
		font.setSize(LyXFont::SIZE_FOOTNOTE);

		int box_x = 20; // LYX_PAPER_MARGIN;
		box_x += lyxfont::width(" wide-tab ", font);

		unsigned int screen_first = bv_->text->first;

		if (x < box_x
		    && y + screen_first > bv_->text->cursor.y() -
		    bv_->text->cursor.row()->baseline()
		    && y + screen_first < bv_->text->cursor.y() -
		    bv_->text->cursor.row()->baseline()
		    + lyxfont::maxAscent(font) * 1.2 + lyxfont::maxDescent(font) * 1.2) {
			bv_->toggleFloat();
			selection_possible = false;
			return;
		}
	}

	// Maybe we want to edit a bibitem ale970302
	if (bv_->text->cursor.par()->bibkey && x < 20 + 
	    bibitemMaxWidth(bv_, textclasslist.
			    TextClass(buffer_->
				      params.textclass).defaultfont())) {
		bv_->text->cursor.par()->bibkey->Edit(bv_, 0, 0, 0);
	}

	return;
}


/* 
 * Returns an inset if inset was hit. 0 otherwise.
 * If hit, the coordinates are changed relative to the inset. 
 * Otherwise coordinates are not changed, and false is returned.
 */
Inset * BufferView::Pimpl::checkInsetHit(LyXText * text, int & x, int & y,
					 unsigned int /* button */)
{
	if (!screen_)
		return 0;
  
	unsigned int y_tmp = y + text->first;
  
	LyXCursor cursor;
	text->SetCursorFromCoordinates(bv_, cursor, x, y_tmp);
#if 0 // Are you planning to use this Jürgen? (Lgb)
	bool move_cursor = ((cursor.par != text->cursor.par) ||
			    (cursor.pos != text->cursor.pos()));
#endif
	if (cursor.pos() < cursor.par()->Last()
	    && cursor.par()->GetChar(cursor.pos()) == LyXParagraph::META_INSET
	    && cursor.par()->GetInset(cursor.pos())
	    && cursor.par()->GetInset(cursor.pos())->Editable()) {

		// Check whether the inset really was hit
		Inset * tmpinset = cursor.par()->GetInset(cursor.pos());
		LyXFont font = text->GetFont(bv_->buffer(),
						  cursor.par(), cursor.pos());
		bool is_rtl = font.isVisibleRightToLeft();
		int start_x, end_x;

		if (is_rtl) {
			start_x = cursor.x() - tmpinset->width(bv_, font);
			end_x = cursor.x();
		} else {
			start_x = cursor.x();
			end_x = cursor.x() + tmpinset->width(bv_, font);
		}

		if (x > start_x && x < end_x
		    && y_tmp > cursor.y() - tmpinset->ascent(bv_, font)
		    && y_tmp < cursor.y() + tmpinset->descent(bv_, font)) {
#if 0
			if (move_cursor && (tmpinset != bv_->the_locking_inset))
#endif
				text->SetCursor(bv_, cursor.par(),cursor.pos(),true);
			x = x - start_x;
			// The origin of an inset is on the baseline
			y = y_tmp - (text->cursor.y()); 
			return tmpinset;
		}
	}

	if ((cursor.pos() - 1 >= 0) &&
	    (cursor.par()->GetChar(cursor.pos()-1) == LyXParagraph::META_INSET) &&
	    (cursor.par()->GetInset(cursor.pos() - 1)) &&
	    (cursor.par()->GetInset(cursor.pos() - 1)->Editable())) {
		Inset * tmpinset = cursor.par()->GetInset(cursor.pos()-1);
		LyXFont font = text->GetFont(bv_->buffer(), cursor.par(),
						  cursor.pos()-1);
		bool is_rtl = font.isVisibleRightToLeft();
		int start_x, end_x;

		if (!is_rtl) {
			start_x = cursor.x() - tmpinset->width(bv_, font);
			end_x = cursor.x();
		} else {
			start_x = cursor.x();
			end_x = cursor.x() + tmpinset->width(bv_, font);
		}
		if (x > start_x && x < end_x
		    && y_tmp > cursor.y() - tmpinset->ascent(bv_, font)
		    && y_tmp < cursor.y() + tmpinset->descent(bv_, font)) {
#if 0
			if (move_cursor && (tmpinset != bv_->the_locking_inset))
#endif
				text->SetCursor(bv_, cursor.par(),cursor.pos()-1,true);
			x = x - start_x;
			// The origin of an inset is on the baseline
			y = y_tmp - (text->cursor.y()); 
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
	
	static int work_area_width = 0;
	static unsigned int work_area_height = 0;

	bool widthChange = workarea_->workWidth() != work_area_width;
	bool heightChange = workarea_->height() != work_area_height;

	// update from work area
	work_area_width = workarea_->workWidth();
	work_area_height = workarea_->height();
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

		} else if (screen_) screen_->Redraw(bv_->text);
	} else {
		// Grey box when we don't have a buffer
		workarea_->greyOut();
	}

	// always make sure that the scrollbar is sane.
	updateScrollbar();
	owner_->updateLayoutChoice();
	return;
}


void BufferView::Pimpl::update()
{
	if (screen_) screen_->Update(bv_->text);
}

// Values used when calling update:
// -3 - update
// -2 - update, move sel_cursor if selection, fitcursor
// -1 - update, move sel_cursor if selection, fitcursor, mark dirty
//  0 - update, move sel_cursor if selection, fitcursor 
//  1 - update, move sel_cursor if selection, fitcursor, mark dirty
//  3 - update, move sel_cursor if selection
//
// update -
// a simple redraw of the parts that need refresh
//
// move sel_cursor if selection -
// the text's sel_cursor is moved if there is selection is progress
//
// fitcursor -
// fitCursor() is called and the scrollbar updated
//
// mark dirty -
// the buffer is marked dirty.
//
// enum {
//       UPDATE = 0,
//       SELECT = 1,
//       FITCUR = 2,
//       CHANGE = 4 
// };
//
// UPDATE_ONLY = UPDATE;
// UPDATE_SELECT = UPDATE | SELECT;
// UPDATE_SELECT_MOVE = UPDATE | SELECT | FITCUR;
// UPDATE_SELECT_MOVE_AFTER_CHANGE = UPDATE | SELECT | FITCUR | CHANGE;
//
// update(-3) -> update(0)         -> update(0) -> update(UPDATE)
// update(-2) -> update(1 + 2)     -> update(3) -> update(SELECT|FITCUR)
// update(-1) -> update(1 + 2 + 4) -> update(7) -> update(SELECT|FITCUR|CHANGE)
// update(1)  -> update(1 + 2 + 4) -> update(7) -> update(SELECT|FITCUR|CHANGE)
// update(3)  -> update(1)         -> update(1) -> update(SELECT)

void BufferView::Pimpl::update(BufferView::UpdateCodes f)
{
	owner_->updateLayoutChoice();

	if (!bv_->text->selection && (f & SELECT)) {
		bv_->text->sel_cursor = bv_->text->cursor;
	}

	bv_->text->FullRebreak(bv_);

	update();

	if ((f & FITCUR)) {
		fitCursor();
      	}

	if ((f & CHANGE)) {
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
	
	if (!screen_) {
		goto set_timer_and_return;
	}

	if (!bv_->the_locking_inset) {
		screen_->CursorToggle(bv_->text);
	} else {
		bv_->the_locking_inset->
			ToggleInsetCursor(bv_);
	}
	
  set_timer_and_return:
	cursor_timeout.restart();
	return;
}


void BufferView::Pimpl::cursorPrevious()
{
	if (!bv_->text->cursor.row()->previous()) return;
	
	long y = bv_->text->first;
	Row * cursorrow = bv_->text->cursor.row();
	bv_->text->SetCursorFromCoordinates(bv_, bv_->text->cursor.x_fix(), y);
	bv_->text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == bv_->text->cursor.row()))
		bv_->text->CursorUp(bv_);
	
  	if (bv_->text->cursor.row()->height() < workarea_->height())
		screen_->Draw(bv_->text,
			      bv_->text->cursor.y()
			      - bv_->text->cursor.row()->baseline()
			      + bv_->text->cursor.row()->height()
			      - workarea_->height() + 1 );
	updateScrollbar();
}


void BufferView::Pimpl::cursorNext()
{
	if (!bv_->text->cursor.row()->next()) return;
	
	long y = bv_->text->first;
	bv_->text->GetRowNearY(y);
	Row * cursorrow = bv_->text->cursor.row();
	bv_->text->SetCursorFromCoordinates(bv_, bv_->text->cursor.x_fix(), y
				       + workarea_->height());
	bv_->text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == bv_->text->cursor.row()))
		bv_->text->CursorDown(bv_);
	
 	if (bv_->text->cursor.row()->height() < workarea_->height())
		screen_->Draw(bv_->text, bv_->text->cursor.y()
			     - bv_->text->cursor.row()->baseline());
	updateScrollbar();
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

	// CHECK
	//owner_->update_timeout.stop();
}


void BufferView::Pimpl::savePosition()
{
	backstack.push(buffer_->fileName(),
		       bv_->text->cursor.x(),
		       bv_->text->cursor.y());
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
	bv_->text->SetCursorFromCoordinates(bv_, x, y);
	update(BufferView::SELECT|BufferView::FITCUR);
} 


bool BufferView::Pimpl::NoSavedPositions()
{
	return backstack.empty();
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
		bv_->the_locking_inset->GetCursorPos(bv_, bv_->slx, bv_->sly);
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
	return workarea_->hasFocus();
}


void BufferView::Pimpl::focus(bool f)
{
	if (f) workarea_->setFocus();
}


bool BufferView::Pimpl::active() const
{
	return workarea_->active();
}


bool BufferView::Pimpl::belowMouse() const 
{
	return workarea_->belowMouse();
}


void BufferView::Pimpl::showCursor()
{
	if (screen_)
		screen_->ShowCursor(bv_->text);
}


void BufferView::Pimpl::hideCursor()
{
	if (screen_)
		screen_->HideCursor();
}


void BufferView::Pimpl::toggleSelection(bool b)
{
	if (screen_)
		screen_->ToggleSelection(bv_->text, b);
}


void BufferView::Pimpl::toggleToggle()
{
	if (screen_)
		screen_->ToggleToggle(bv_->text);
}


void BufferView::Pimpl::center() 
{
	beforeChange();
	if (bv_->text->cursor.y() > workarea_->height() / 2) {
		screen_->Draw(bv_->text, bv_->text->cursor.y() - workarea_->height() / 2);
	} else {
		screen_->Draw(bv_->text, 0);
	}
	update(BufferView::SELECT|BufferView::FITCUR);
	redraw();
}


void BufferView::Pimpl::pasteClipboard(bool asPara) 
{
	if (buffer_ == 0) return;

	screen_->HideCursor();
	bv_->beforeChange();
	
	string clip(workarea_->getClipboard());
	
	if (clip.empty()) return;

	if (asPara) {
		bv_->text->InsertStringB(bv_, clip);
	} else {
		bv_->text->InsertStringA(bv_, clip);
	}
	update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
}


void BufferView::Pimpl::stuffClipboard(string const & stuff) const
{
	workarea_->putClipboard(stuff);
}
