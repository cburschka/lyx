#include <config.h>

//#include <utility>

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
#include "lyx_cb.h"
#include "TextCache.h"
#include "bufferlist.h"
#include "insets/insetbib.h"

using std::pair;

extern BufferList bufferlist;
extern bool selection_possible;
extern char ascii_type;

extern int bibitemMaxWidth(Painter &, LyXFont const &);

extern "C"
void C_BufferView_CursorToggleCB(FL_OBJECT * ob, long buf);


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
			bv_->update(-3); // Maybe an empty line was deleted
      
		bv_->text->SetSelection();
		screen->ToggleToggle();
		if (screen->FitCursor())
			bv_->updateScrollbar(); 
		screen->ShowCursor();
	}
	return;
}


// Single-click on work area
void BufferView::Pimpl::workAreaButtonPress(int xpos, int ypos, unsigned int button)
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
				bv_->updateScrollbar();
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
	bv_->updateScrollbar();
	
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
		bv_->updateScrollbar();
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
			bv_->update(0);
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
		bv_->update(0);
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
			bv_->fitCursor();

			// The main window size has changed, repaint most stuff
			bv_->redraw();
			// ...including the minibuffer
			owner_->getMiniBuffer()->Init();

		} else if (screen) screen->Redraw();
	} else {
		// Grey box when we don't have a buffer
		workarea->greyOut();
	}

	// always make sure that the scrollbar is sane.
	bv_->updateScrollbar();
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

		bv_->update(1);
	}
}

