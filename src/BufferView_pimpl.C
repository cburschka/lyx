#include <config.h>

#include <ctime>
#include <unistd.h>
#include <sys/wait.h>
#include <locale.h>

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
#include "lyx_gui_misc.h"
#include "lyxrc.h"
#include "intl.h"
#include "support/LAssert.h"
#include "frontends/Dialogs.h"
#include "insets/insetbib.h"
#include "insets/insettext.h"
/// added for Dispatch functions
#include "lyx_cb.h"
#include "frontends/FileDialog.h"
#include "lyx_main.h"
#include "FloatList.h"
#include "support/filetools.h"
#include "support/lyxfunctional.h"
#include "insets/inseturl.h"
#include "insets/insetlatexaccent.h"
#include "insets/insettoc.h"
#include "insets/insetref.h"
#include "insets/insetparent.h"
#include "insets/insetindex.h"
#include "insets/insetinclude.h"
#include "insets/insetcite.h"
#include "insets/insetert.h"
#include "insets/insetexternal.h"
#include "insets/insetgraphics.h"
#include "insets/insetfoot.h"
#include "insets/insetmarginal.h"
#include "insets/insetminipage.h"
#include "insets/insetfloat.h"
#include "insets/insetlist.h"
#include "insets/insettabular.h"
#include "insets/insettheorem.h"
#include "insets/insetcaption.h"
#include "mathed/formulamacro.h"
extern LyXTextClass::size_type current_layout;
extern int greek_kb_flag;

using std::vector;
using std::find_if;
using std::pair;
using std::endl;
using std::make_pair;
using std::min;
using SigC::slot;

/* the selection possible is needed, that only motion events are 
 * used, where the bottom press event was on the drawing area too */
bool selection_possible = false;

extern BufferList bufferlist;
extern char ascii_type;

extern bool math_insert_greek(BufferView *, char);
extern void sigchldhandler(pid_t pid, int * status);
extern int bibitemMaxWidth(BufferView *, LyXFont const &);

const unsigned int saved_positions_num = 20;

namespace {

inline
void waitForX()
{
	XSync(fl_get_display(), 0);
}


void SetXtermCursor(Window win)
{
	static Cursor cursor;
	static bool cursor_undefined = true;
	if (cursor_undefined){
		cursor = XCreateFontCursor(fl_get_display(), XC_xterm);
		XFlush(fl_get_display());
		cursor_undefined = false;
	}
	XDefineCursor(fl_get_display(), win, cursor);
	XFlush(fl_get_display());
}

} // anon namespace


BufferView::Pimpl::Pimpl(BufferView * b, LyXView * o,
	     int xpos, int ypos, int width, int height)
	: bv_(b), owner_(o), buffer_(0),
	  current_scrollbar_value(0), cursor_timeout(400),
	  workarea_(xpos, ypos, width, height), using_xterm_cursor(false)
{
	// Setup the signals
	workarea_.scrollCB.connect(slot(this, &BufferView::Pimpl::scrollCB));
	workarea_.workAreaExpose
		.connect(slot(this, &BufferView::Pimpl::workAreaExpose));
	workarea_.workAreaEnter
		.connect(slot(this, &BufferView::Pimpl::enterView));
	workarea_.workAreaLeave
		.connect(slot(this, &BufferView::Pimpl::leaveView));
	workarea_.workAreaButtonPress
		.connect(slot(this, &BufferView::Pimpl::workAreaButtonPress));
	workarea_.workAreaButtonRelease
		.connect(slot(this,
			      &BufferView::Pimpl::workAreaButtonRelease));
	workarea_.workAreaMotionNotify
		.connect(slot(this, &BufferView::Pimpl::workAreaMotionNotify));
	workarea_.workAreaDoubleClick
		.connect(slot(this, &BufferView::Pimpl::doubleClick));
	workarea_.workAreaTripleClick
		.connect(slot(this, &BufferView::Pimpl::tripleClick));
	workarea_.workAreaKeyPress
		.connect(slot(this, &BufferView::Pimpl::workAreaKeyPress));
	
	//screen_ = 0;

	cursor_timeout.timeout.connect(slot(this,
					    &BufferView::Pimpl::cursorToggle));
	//current_scrollbar_value = 0;
	cursor_timeout.start();
	workarea_.setFocus();
	//using_xterm_cursor = false;
	saved_positions.resize(saved_positions_num);
}


Painter & BufferView::Pimpl::painter() 
{
	return workarea_.getPainter();
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
		textcache.add(buffer_, workarea_.workWidth(), bv_->text);
		if (lyxerr.debugging())
			textcache.show(lyxerr, "BufferView::buffer");
		
		bv_->text = 0;
	}

	// Set current buffer
	buffer_ = b;

	if (bufferlist.getState() == BufferList::CLOSING) return;
	
	// Nuke old image
	// screen is always deleted when the buffer is changed.
	screen_.reset(0);
	//delete screen_;
	//screen_ = 0;

	// If we are closing the buffer, use the first buffer as current
	if (!buffer_) {
		buffer_ = bufferlist.first();
	}

	if (buffer_) {
		lyxerr[Debug::INFO] << "Buffer addr: " << buffer_ << endl;
		buffer_->addUser(bv_);
		// If we don't have a text object for this, we make one
		if (bv_->text == 0) {
			resizeCurrentBuffer();
		} else {
			updateScreen();
			updateScrollbar();
		}
		bv_->text->first = screen_->TopCursorVisible(bv_->text);
		owner_->updateMenubar();
		owner_->updateToolbar();
		// Similarly, buffer-dependent dialogs should be updated or
		// hidden. This should go here because some dialogs (eg ToC)
		// require bv_->text.
		owner_->getDialogs()->updateBufferDependent(true);
		redraw();
		bv_->insetWakeup();
	} else {
		lyxerr[Debug::INFO] << "  No Buffer!" << endl;
		owner_->updateMenubar();
		owner_->updateToolbar();
		owner_->getDialogs()->hideBufferDependent();
		updateScrollbar();
		workarea_.redraw();

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
	workarea_.resize(xpos, ypos, width, height);
	update(bv_->text, SELECT);
	redraw();
}


void BufferView::Pimpl::resize()
{
	if (buffer_)
		resizeCurrentBuffer();
}


void BufferView::Pimpl::redraw()
{
	lyxerr[Debug::INFO] << "BufferView::redraw()" << endl;
	workarea_.redraw();
}


bool BufferView::Pimpl::fitCursor(LyXText * text)
{
	Assert(screen_.get());
 
	bool ret = screen_->FitCursor(text, bv_);
	if (ret)
	    updateScrollbar();
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
	UpdatableInset * the_locking_inset = 0;
	
	LyXParagraph::size_type pos = 0;
	LyXParagraph::size_type selstartpos = 0;
	LyXParagraph::size_type selendpos = 0;
	bool selection = false;
	bool mark_set  = false;

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
		the_locking_inset = bv_->text->the_locking_inset;
		delete bv_->text;
		bv_->text = new LyXText(bv_);
	} else {
		// See if we have a text in TextCache that fits
		// the new buffer_ with the correct width.
		bv_->text = textcache.findFit(buffer_, workarea_.workWidth());
		if (bv_->text) {
			if (lyxerr.debugging()) {
				lyxerr << "Found a LyXText that fits:\n";
				textcache.show(lyxerr, make_pair(buffer_, make_pair(workarea_.workWidth(), bv_->text)));
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
			bv_->text->SetSelection(bv_);
			bv_->text->SetCursor(bv_, par, pos);
		} else {
			bv_->text->SetCursor(bv_, par, pos);
			bv_->text->sel_cursor = bv_->text->cursor;
			bv_->text->selection = false;
		}
		// remake the inset locking
		bv_->text->the_locking_inset = the_locking_inset;
	}
	bv_->text->first = screen_->TopCursorVisible(bv_->text);
	buffer_->resizeInsets(bv_);
	// this will scroll the screen such that the cursor becomes visible
	updateScrollbar();
	redraw();
	owner_->getMiniBuffer()->Init();
	bv_->setState();
	AllowInput(bv_);

	/// get rid of the splash screen if it's not gone already
	owner_->getDialogs()->destroySplash();
 
	return 0;
}


void BufferView::Pimpl::updateScreen()
{
	// Regenerate the screen.
	screen_.reset(new LyXScreen(workarea_));
	//delete screen_;
	//screen_ = new LyXScreen(workarea_);
}


void BufferView::Pimpl::updateScrollbar()
{
	/* If the text is smaller than the working area, the scrollbar
	 * maximum must be the working area height. No scrolling will 
	 * be possible */

	if (!buffer_) {
		workarea_.setScrollbar(0, 1.0);
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
	    height2 == workarea_.height() &&
	    current_scrollbar_value == cbsf)
		return; // no
	max2 = cbth;
	height2 = workarea_.height();
	current_scrollbar_value = cbsf;

	if (cbth <= height2) { // text is smaller than screen
		workarea_.setScrollbar(0, 1.0); // right?
		return;
	}

	long maximum_height = workarea_.height() * 3 / 4 + cbth;
	long value = cbsf;

	// set the scrollbar
	double hfloat = workarea_.height();
	double maxfloat = maximum_height;

	float slider_size = 0.0;
	int slider_value = value;

	workarea_.setScrollbarBounds(0, bv_->text->height - workarea_.height());
	double const lineh = bv_->text->DefaultHeight();
	workarea_.setScrollbarIncrements(lineh);
	if (maxfloat > 0.0) {
		if ((hfloat / maxfloat) * float(height2) < 3)
			slider_size = 3.0/float(height2);
		else
			slider_size = hfloat / maxfloat;
	} else
		slider_size = hfloat;

	workarea_.setScrollbar(slider_value, slider_size / workarea_.height());
}


// Callback for scrollbar slider
void BufferView::Pimpl::scrollCB(double value)
{
	if (!buffer_) return;

	current_scrollbar_value = long(value);
	if (current_scrollbar_value < 0)
		current_scrollbar_value = 0;
   
	if (!screen_.get())
		return;

	screen_->Draw(bv_->text, bv_, current_scrollbar_value);

	if (!lyxrc.cursor_follows_scrollbar) {
		waitForX();
		return;
	}
 
	LyXText * vbt = bv_->text;
 
	int const height = vbt->DefaultHeight();
	int const first = static_cast<int>((bv_->text->first + height));
	int const last = static_cast<int>((bv_->text->first + workarea_.height() - height));

	if (vbt->cursor.y() < first)
		vbt->SetCursorFromCoordinates(bv_, 0, first);
	else if (vbt->cursor.y() > last)
		vbt->SetCursorFromCoordinates(bv_, 0, last);

	waitForX();
}


int BufferView::Pimpl::scrollUp(long time)
{
	if (!buffer_) return 0;
	if (!screen_.get()) return 0;
   
	double value = workarea_.getScrollbarValue();
   
	if (value == 0) return 0;

	float add_value =  (bv_->text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > workarea_.height())
		add_value = float(workarea_.height() -
				  bv_->text->DefaultHeight());
   
	value -= add_value;

	if (value < 0)
		value = 0;
   
	workarea_.setScrollbarValue(value);
   
	scrollCB(value); 
	return 0;
}


int BufferView::Pimpl::scrollDown(long time)
{
	if (!buffer_) return 0;
	if (!screen_.get()) return 0;
   
	double value = workarea_.getScrollbarValue();
	pair<float, float> p = workarea_.getScrollbarBounds();
	double max = p.second;
	
	if (value == max) return 0;

	float add_value =  (bv_->text->DefaultHeight()
			    + float(time) * float(time) * 0.125);
   
	if (add_value > workarea_.height())
		add_value = float(workarea_.height() -
				  bv_->text->DefaultHeight());
   
	value += add_value;
   
	if (value > max)
		value = max;

	workarea_.setScrollbarValue(value);
	
	scrollCB(value); 
	return 0;
}


void BufferView::Pimpl::workAreaKeyPress(KeySym keysym, unsigned int state)
{
	bv_->owner()->getLyXFunc()->processKeySym(keysym, state);
}


void BufferView::Pimpl::workAreaMotionNotify(int x, int y, unsigned int state)
{
	// Only use motion with button 1
	if (!(state & Button1MotionMask))
		return;

	if (!buffer_ || !screen_.get()) return;

	// Check for inset locking
	if (bv_->theLockingInset()) {
		LyXCursor cursor = bv_->text->cursor;
		LyXFont font = bv_->text->GetFont(bv_->buffer(),
						  cursor.par(), cursor.pos());
		int width = bv_->theLockingInset()->width(bv_, font);
		int inset_x = font.isVisibleRightToLeft()
			? cursor.x() - width : cursor.x();
		int start_x = inset_x + bv_->theLockingInset()->scroll();
		bv_->theLockingInset()->
			InsetMotionNotify(bv_,
					  x - start_x,
					  y - cursor.y() + bv_->text->first,
					  state);
		return;
	}
   
	/* The test for not selection possible is needed, that only motion events are 
	 * used, where the bottom press event was on the drawing area too */
	if (!selection_possible)
		return;
 
	screen_->HideCursor();

	bv_->text->SetCursorFromCoordinates(bv_, x, y + bv_->text->first);
      
	if (!bv_->text->selection)
		update(bv_->text, BufferView::UPDATE); // Maybe an empty line was deleted
      
	bv_->text->SetSelection(bv_);
	screen_->ToggleToggle(bv_->text, bv_);
	fitCursor(bv_->text);
	screen_->ShowCursor(bv_->text, bv_);
}


// Single-click on work area
void BufferView::Pimpl::workAreaButtonPress(int xpos, int ypos,
					    unsigned int button)
{
	last_click_x = -1;
	last_click_y = -1;

	if (!buffer_ || !screen_.get()) return;

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
	
	if (bv_->theLockingInset()) {
		// We are in inset locking mode
		
		/* Check whether the inset was hit. If not reset mode,
		   otherwise give the event to the inset */
		if (inset_hit == bv_->theLockingInset()) {
			bv_->theLockingInset()->
				InsetButtonPress(bv_,
						 xpos, ypos,
						 button);
			return;
		} else {
			bv_->unlockInset(bv_->theLockingInset());
		}
	}
	
	if (!inset_hit)
		selection_possible = true;
	screen_->HideCursor();

	int const screen_first = bv_->text->first;
	
	// Middle button press pastes if we have a selection
	bool paste_internally = false;
	if (button == 2
	    && bv_->text->selection) {
		owner_->getLyXFunc()->Dispatch(LFUN_COPY);
		paste_internally = true;
	}
	
	// Clear the selection
	screen_->ToggleSelection(bv_->text, bv_);
	bv_->text->ClearSelection(bv_);
	bv_->text->FullRebreak(bv_);
	screen_->Update(bv_->text, bv_);
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
	if (fitCursor(bv_->text)) {
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
	if (!buffer_)
	    return;

	LyXText * text = bv_->getLyXText();

	if (text->bv_owner && bv_->theLockingInset())
	    return;

	if (screen_.get() && button == 1) {
	    if (text->bv_owner) {
		screen_->HideCursor();
		screen_->ToggleSelection(text, bv_);
		text->SelectWord(bv_);
		screen_->ToggleSelection(text, bv_, false);
	    } else {
		text->SelectWord(bv_);
	    }
	    /* This will fit the cursor on the screen
	     * if necessary */
	    update(text, BufferView::SELECT|BufferView::FITCUR);
	}
}


void BufferView::Pimpl::tripleClick(int /*x*/, int /*y*/, unsigned int button)
{
	// select a line
	if (buffer_)
		return;

	LyXText * text = bv_->getLyXText();

	if (text->bv_owner && bv_->theLockingInset())
	    return;

	if (screen_.get() && (button == 1)) {
		screen_->HideCursor();
		screen_->ToggleSelection(text, bv_);
		text->CursorHome(bv_);
		text->sel_cursor = text->cursor;
		text->CursorEnd(bv_);
		text->SetSelection(bv_);
		screen_->ToggleSelection(text, bv_, false);
		/* This will fit the cursor on the screen
		 * if necessary */
		update(text, BufferView::SELECT|BufferView::FITCUR);
	}
}


void BufferView::Pimpl::enterView()
{
	if (active() && available()) {
		SetXtermCursor(workarea_.getWin());
		using_xterm_cursor = true;
	}
}


void BufferView::Pimpl::leaveView()
{
	if (using_xterm_cursor) {
		XUndefineCursor(fl_get_display(), workarea_.getWin());
		using_xterm_cursor = false;
	}
}


void BufferView::Pimpl::workAreaButtonRelease(int x, int y,
					      unsigned int button)
{
	if (!buffer_ || !screen_.get()) return;

	// If we hit an inset, we have the inset coordinates in these
	// and inset_hit points to the inset.  If we do not hit an
	// inset, inset_hit is 0, and inset_x == x, inset_y == y.
	Inset * inset_hit = checkInsetHit(bv_->text, x, y, button);

	if (bv_->theLockingInset()) {
		// We are in inset locking mode.

		/* LyX does a kind of work-area grabbing for insets.
		   Only a ButtonPress Event outside the inset will 
		   force a InsetUnlock. */
		bv_->theLockingInset()->
			InsetButtonRelease(bv_, x, y, button);
		return;
	}
	
	selection_possible = false;
	
	if (button == 2)
		return;

	bv_->setState();
	owner_->showState();

	// Did we hit an editable inset?
	if (inset_hit) {
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

#ifndef NEW_INSETS
	// check whether we want to open a float
	if (bv_->text) {
		bool hit = false;
		char c = ' ';
		if (bv_->text->cursor.pos() <
		    bv_->text->cursor.par()->Last()) {
			c = bv_->text->cursor.par()->
				GetChar(bv_->text->cursor.pos());
		}
	       if(!bv_->text->selection)
		if (c == LyXParagraph::META_FOOTNOTE
		    || c == LyXParagraph::META_MARGIN
		    || c == LyXParagraph::META_FIG
		    || c == LyXParagraph::META_TAB
		    || c == LyXParagraph::META_WIDE_FIG
		    || c == LyXParagraph::META_WIDE_TAB
                    || c == LyXParagraph::META_ALGORITHM){
			hit = true;
		} else
			if (bv_->text->cursor.pos() - 1 >= 0) {
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
#else
	// check whether we want to open a float
	if (bv_->text) {
		bool hit = false;
		char c = ' ';
		if (bv_->text->cursor.pos() <
		    bv_->text->cursor.par()->size()) {
			c = bv_->text->cursor.par()->
				GetChar(bv_->text->cursor.pos());
		}
			if (bv_->text->cursor.pos() - 1 >= 0) {
			c = bv_->text->cursor.par()->
				GetChar(bv_->text->cursor.pos() - 1);
		}
		if (hit == true) {
			selection_possible = false;
			return;
		}
	}
#endif
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
	if (!screen_.get())
		return 0;
  
	int y_tmp = y + text->first;
  
	LyXCursor cursor;
	text->SetCursorFromCoordinates(bv_, cursor, x, y_tmp);
	text->SetCursor(bv_, cursor, cursor.par(),cursor.pos(),true);


#ifndef NEW_INSETS
	if (cursor.pos() < cursor.par()->Last()
#else
	if (cursor.pos() < cursor.par()->size()
#endif
	    && cursor.par()->GetChar(cursor.pos()) == LyXParagraph::META_INSET
	    && cursor.par()->GetInset(cursor.pos())
	    && cursor.par()->GetInset(cursor.pos())->Editable()) {

		// Check whether the inset really was hit
		Inset * tmpinset = cursor.par()->GetInset(cursor.pos());
		LyXFont font = text->GetFont(bv_->buffer(),
						  cursor.par(), cursor.pos());
		int const width = tmpinset->width(bv_, font);
		int const inset_x = font.isVisibleRightToLeft()
			? cursor.x() - width : cursor.x();
		int const start_x = inset_x + tmpinset->scroll();
		int const end_x = inset_x + width;

		if (x > start_x && x < end_x
		    && y_tmp > cursor.y() - tmpinset->ascent(bv_, font)
		    && y_tmp < cursor.y() + tmpinset->descent(bv_, font)) {
			text->SetCursor(bv_, cursor.par(),cursor.pos(), true);
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
		int const width = tmpinset->width(bv_, font);
		int const inset_x = font.isVisibleRightToLeft()
			? cursor.x() : cursor.x() - width;
		int const start_x = inset_x + tmpinset->scroll();
		int const end_x = inset_x + width;

		if (x > start_x && x < end_x
		    && y_tmp > cursor.y() - tmpinset->ascent(bv_, font)
		    && y_tmp < cursor.y() + tmpinset->descent(bv_, font)) {
#if 0
			if (move_cursor && (tmpinset != bv_->theLockingInset()))
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
	static int work_area_width = 0;
	static unsigned int work_area_height = 0;

	bool const widthChange = workarea_.workWidth() != work_area_width;
	bool const heightChange = workarea_.height() != work_area_height;

	// update from work area
	work_area_width = workarea_.workWidth();
	work_area_height = workarea_.height();
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
			fitCursor(bv_->text);

			// The main window size has changed, repaint most stuff
			redraw();
			// ...including the minibuffer
			owner_->getMiniBuffer()->Init();

		} else if (screen_.get())
		    screen_->Redraw(bv_->text, bv_);
	} else {
		// Grey box when we don't have a buffer
		workarea_.greyOut();
	}

	// always make sure that the scrollbar is sane.
	updateScrollbar();
	owner_->updateLayoutChoice();
	return;
}


void BufferView::Pimpl::update()
{
	if (screen_.get()) screen_->Update(bv_->text, bv_);
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

void BufferView::Pimpl::update(LyXText * text, BufferView::UpdateCodes f)
{
	owner_->updateLayoutChoice();

	if (!text->selection && (f & SELECT)) {
		text->sel_cursor = text->cursor;
	}

	text->FullRebreak(bv_);

	if (text->inset_owner) {
	    text->inset_owner->SetUpdateStatus(bv_, InsetText::NONE);
	    bv_->updateInset(text->inset_owner, true);
	} else
	    update();

	if ((f & FITCUR)) {
		fitCursor(text);
      	}

	if ((f & CHANGE)) {
		if (buffer_->isLyxClean()) {
			buffer_->markDirty();
#if 0
			owner_->getMiniBuffer()->setTimer(4);
#endif
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
		cursor_timeout.restart();
		return;
	}
 
	int status = 1;
	int pid = waitpid(static_cast<pid_t>(0), &status, WNOHANG);
	if (pid == -1) // error find out what is wrong
		; // ignore it for now.
	else if (pid > 0)
		sigchldhandler(pid, &status);

	updatelist.update(bv_);
	
	if (!screen_.get()) {
		cursor_timeout.restart();
		return;
	}

	if (!bv_->theLockingInset()) {
		screen_->CursorToggle(bv_->text, bv_);
	} else {
		bv_->theLockingInset()->ToggleInsetCursor(bv_);
	}
	
	cursor_timeout.restart();
}


void BufferView::Pimpl::cursorPrevious(LyXText * text)
{
	if (!text->cursor.row()->previous())
		return;
	
	int y = text->first;
	if (text->inset_owner)
		y += bv_->text->first;
	Row * cursorrow = text->cursor.row();
	text->SetCursorFromCoordinates(bv_, bv_->text->cursor.x_fix(), y);
	bv_->text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == text->cursor.row()))
		text->CursorUp(bv_);
	
  	if (text->inset_owner ||
	    text->cursor.row()->height() < workarea_.height())
		screen_->Draw(bv_->text, bv_,
			      text->cursor.y()
			      - text->cursor.row()->baseline()
			      + text->cursor.row()->height()
			      - workarea_.height() + 1 );
	updateScrollbar();
}


void BufferView::Pimpl::cursorNext(LyXText * text)
{
	if (!text->cursor.row()->next())
		return;
	
	int y = text->first + workarea_.height();
//	if (text->inset_owner)
//		y += bv_->text->first;
	text->GetRowNearY(y);
    
	Row * cursorrow = text->cursor.row();
	text->SetCursorFromCoordinates(bv_, text->cursor.x_fix(), y); // + workarea_->height());
	bv_->text->FinishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == bv_->text->cursor.row()))
		text->CursorDown(bv_);
	
 	if (text->inset_owner ||
	    text->cursor.row()->height() < workarea_.height())
		screen_->Draw(bv_->text, bv_, text->cursor.y() -
			      text->cursor.row()->baseline());
	updateScrollbar();
}


bool BufferView::Pimpl::available() const
{
	if (buffer_ && bv_->text) return true;
	return false;
}


void BufferView::Pimpl::beforeChange(LyXText * text)
{
	toggleSelection();
	text->ClearSelection(bv_);
}


void BufferView::Pimpl::savePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;
	saved_positions[i] = Position(buffer_->fileName(),
				      bv_->text->cursor.par()->id(),
				      bv_->text->cursor.pos());
	if (i > 0)
		owner_->getMiniBuffer()->Set(_("Saved bookmark"), tostr(i));
}


void BufferView::Pimpl::restorePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;

	string fname = saved_positions[i].filename;

	beforeChange(bv_->text);

	if (fname != buffer_->fileName()) {
		Buffer * b = bufferlist.exists(fname) ?
	 		bufferlist.getBuffer(fname) :
			bufferlist.loadLyXFile(fname); // don't ask, just load it
		if (b != 0 ) buffer(b);
	}

	LyXParagraph * par = bv_->text->GetParFromID(saved_positions[i].par_id);
	if (!par)
		return;

#ifndef NEW_INSETS
	bv_->text->SetCursor(bv_, par,
			     min(par->Last(), saved_positions[i].par_pos));
#else
	bv_->text->SetCursor(bv_, par,
			     min(par->size(), saved_positions[i].par_pos));
#endif
	update(bv_->text, BufferView::SELECT|BufferView::FITCUR);
	if (i > 0)
		owner_->getMiniBuffer()->Set(_("Moved to bookmark"), tostr(i));
}


bool BufferView::Pimpl::isSavedPosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return false;

	return !saved_positions[i].filename.empty();
}


void BufferView::Pimpl::setState()
{
	if (!lyxrc.rtl_support)
		return;

	LyXText * text = bv_->getLyXText();
	if (text->real_current_font.isRightToLeft() &&
	    text->real_current_font.latex() != LyXFont::ON) {
		if (owner_->getIntl()->keymap == Intl::PRIMARY)
			owner_->getIntl()->KeyMapSec();
	} else {
		if (owner_->getIntl()->keymap == Intl::SECONDARY)
			owner_->getIntl()->KeyMapPrim();
	}
}


void BufferView::Pimpl::insetSleep()
{
	if (bv_->theLockingInset() && !bv_->inset_slept) {
		bv_->theLockingInset()->GetCursorPos(bv_, bv_->slx, bv_->sly);
		bv_->theLockingInset()->InsetUnlock(bv_);
		bv_->inset_slept = true;
	}
}


void BufferView::Pimpl::insetWakeup()
{
	if (bv_->theLockingInset() && bv_->inset_slept) {
		bv_->theLockingInset()->Edit(bv_, bv_->slx, bv_->sly, 0);
		bv_->inset_slept = false;
	}
}


void BufferView::Pimpl::insetUnlock()
{
	if (bv_->theLockingInset()) {
		if (!bv_->inset_slept)
			bv_->theLockingInset()->InsetUnlock(bv_);
		bv_->theLockingInset(0);
		bv_->text->FinishUndo();
		bv_->inset_slept = false;
	}
}


bool BufferView::Pimpl::focus() const
{
	return workarea_.hasFocus();
}


void BufferView::Pimpl::focus(bool f)
{
	if (f) workarea_.setFocus();
}


bool BufferView::Pimpl::active() const
{
	return workarea_.active();
}


bool BufferView::Pimpl::belowMouse() const 
{
	return workarea_.belowMouse();
}


void BufferView::Pimpl::showCursor()
{
	if (screen_.get())
		screen_->ShowCursor(bv_->text, bv_);
}


void BufferView::Pimpl::hideCursor()
{
	if (screen_.get())
		screen_->HideCursor();
}


void BufferView::Pimpl::toggleSelection(bool b)
{
	if (screen_.get())
		screen_->ToggleSelection(bv_->text, bv_, b);
}


void BufferView::Pimpl::toggleToggle()
{
	if (screen_.get())
		screen_->ToggleToggle(bv_->text, bv_);
}


void BufferView::Pimpl::center() 
{
	beforeChange(bv_->text);
	if (bv_->text->cursor.y() > static_cast<int>((workarea_.height() / 2))) {
		screen_->Draw(bv_->text, bv_, bv_->text->cursor.y() - workarea_.height() / 2);
	} else {
		screen_->Draw(bv_->text, bv_, 0);
	}
	update(bv_->text, BufferView::SELECT|BufferView::FITCUR);
	redraw();
}


void BufferView::Pimpl::pasteClipboard(bool asPara) 
{
	if (!buffer_) return;

	screen_->HideCursor();
	beforeChange(bv_->text);
	
	string const clip(workarea_.getClipboard());
	
	if (clip.empty()) return;

	if (asPara) {
		bv_->text->InsertStringB(bv_, clip);
	} else {
		bv_->text->InsertStringA(bv_, clip);
	}
	update(bv_->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
}


void BufferView::Pimpl::stuffClipboard(string const & stuff) const
{
	workarea_.putClipboard(stuff);
}


/*
 * Dispatch functions for actions which can be valid for BufferView->text
 * and/or InsetText->text!!!
 */
static LyXText * TEXT(BufferView * bv) { return bv->getLyXText(); }

inline
void BufferView::Pimpl::moveCursorUpdate(bool selecting)
{
	if (selecting || TEXT(bv_)->mark_set) {
		TEXT(bv_)->SetSelection(bv_);
		if (TEXT(bv_)->bv_owner)
			bv_->toggleToggle();
	}
	update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
	showCursor();
	
	/* ---> Everytime the cursor is moved, show the current font state. */
	// should this too me moved out of this func?
	//owner->showState();
	bv_->setState();
}


Inset * BufferView::Pimpl::getInsetByCode(Inset::Code code)
{
	LyXCursor cursor = TEXT(bv_)->cursor;
	Buffer::inset_iterator it =
		find_if(Buffer::inset_iterator(
			cursor.par(), cursor.pos()),
			buffer_->inset_iterator_end(),
			compare_memfun(&Inset::LyxCode, code)
			);
	return it != buffer_->inset_iterator_end() ? (*it) : 0;
}


void BufferView::Pimpl::MenuInsertLyXFile(string const & filen)
{
	string filename = filen;

	if (filename.empty()) {
		// Launch a file browser
		string initpath = lyxrc.document_path;

		if (available()) {
			string const trypath = owner_->buffer()->filepath;
			// If directory is writeable, use this as default.
			if (IsDirWriteable(trypath) == 1)
				initpath = trypath;
		}

		FileDialog fileDlg(bv_->owner(), _("Select LyX document to insert"),
			LFUN_FILE_INSERT,
			make_pair(string(_("Documents")), string(lyxrc.document_path)),
			make_pair(string(_("Examples")), string(AddPath(system_lyxdir, "examples"))));

		FileDialog::Result result = fileDlg.Select(initpath, _("*.lyx| LyX Documents (*.lyx)"));
 
		if (result.first == FileDialog::Later)
			return;

		filename = result.second;

		// check selected filename
		if (filename.empty()) {
			owner_->getMiniBuffer()->Set(_("Canceled."));
			return;
		}
	}

	// get absolute path of file and make sure the filename ends
	// with .lyx
	filename = MakeAbsPath(filename);
	if (!IsLyXFilename(filename))
		filename += ".lyx";

	// Inserts document
	owner_->getMiniBuffer()->Set(_("Inserting document"),
				     MakeDisplayPath(filename), "...");
	bool const res = bv_->insertLyXFile(filename);
	if (res) {
		owner_->getMiniBuffer()->Set(_("Document"),
					     MakeDisplayPath(filename),
					     _("inserted."));
	} else {
		owner_->getMiniBuffer()->Set(_("Could not insert document"),
					     MakeDisplayPath(filename));
	}
}


bool BufferView::Pimpl::Dispatch(kb_action action, string const & argument)
{
	switch (action) {
		// --- Misc -------------------------------------------
	case LFUN_APPENDIX:
		if (available()) {
			TEXT(bv_)->toggleAppendix(bv_);
			update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}
		break;

	case LFUN_TOC_INSERT:
	case LFUN_LOA_INSERT:
	case LFUN_LOF_INSERT:
	case LFUN_LOT_INSERT:
	{
		InsetCommandParams p;
		
		if (action == LFUN_TOC_INSERT )
			p.setCmdName( "tableofcontents" );
		else if (action == LFUN_LOA_INSERT )
			p.setCmdName( "listofalgorithms" );
		else if (action == LFUN_LOF_INSERT )
			p.setCmdName( "listoffigures" );
		else
			p.setCmdName( "listoftables" );

		Inset * inset = new InsetTOC( p );
		if (!bv_->insertInset( inset, "Standard", true ) )
			delete inset;
		break;
	}
		
	case LFUN_TABULAR_FEATURE:
	case LFUN_SCROLL_INSET:
		// this is not handled here as this funktion is only aktive
		// if we have a locking_inset and that one is (or contains)
		// a tabular-inset
		break;

	case LFUN_INSET_GRAPHICS:
	{
		Inset * new_inset = new InsetGraphics;
		if (!bv_->insertInset(new_inset)) {
			delete new_inset;
		} else {
			// this is need because you don't use a inset->Edit()
			bv_->updateInset(new_inset, true);
			new_inset->Edit(bv_, 0, 0, 0);
		}
		break;
	}
		
	case LFUN_PASTE:
		bv_->paste();
		setState();
		break;
		
	case LFUN_PASTESELECTION:
	{
	        bool asPara = false;
		if (argument == "paragraph") asPara = true;
		pasteClipboard(asPara);
	}
	break;
	
	case LFUN_CUT:
		bv_->cut();
		break;
		
	case LFUN_COPY:
		bv_->copy();
		break;
		
	case LFUN_LAYOUT_COPY:
		bv_->copyEnvironment();
		break;
		
	case LFUN_LAYOUT_PASTE:
		bv_->pasteEnvironment();
		setState();
		break;
		
	case LFUN_GOTOERROR:
		bv_->gotoInset(Inset::ERROR_CODE, false);
		break;
		
	case LFUN_GOTONOTE:
		bv_->gotoInset(Inset::IGNORE_CODE, false);
		break;

	case LFUN_REFERENCE_GOTO:
	{
		vector<Inset::Code> tmp;
		tmp.push_back(Inset::LABEL_CODE);
		tmp.push_back(Inset::REF_CODE);
		bv_->gotoInset(tmp, true);
		break;
	}

	case LFUN_HYPHENATION:
		bv_->hyphenationPoint();
		break;
		
	case LFUN_LDOTS:
		bv_->ldots();
		break;
		
	case LFUN_END_OF_SENTENCE:
		bv_->endOfSentenceDot();
		break;

	case LFUN_MENU_SEPARATOR:
		bv_->menuSeparator();
		break;
		
	case LFUN_HFILL:
		bv_->hfill();
		break;
		
	case LFUN_DEPTH:
		changeDepth(bv_, TEXT(bv_), 0);
		break;
		
	case LFUN_DEPTH_MIN:
		changeDepth(bv_, TEXT(bv_), -1);
		break;
		
	case LFUN_DEPTH_PLUS:
		changeDepth(bv_, TEXT(bv_), 1);
		break;
		
	case LFUN_FREE:
		owner_->getDialogs()->setUserFreeFont();
		break;
		
	case LFUN_TEX:
		Tex(bv_);
		setState();
		owner_->showState();
		break;

	case LFUN_FILE_INSERT:
	{
		MenuInsertLyXFile(argument);
	}
	break;
	
	case LFUN_FILE_INSERT_ASCII_PARA:
		InsertAsciiFile(bv_, argument, true);
		break;

	case LFUN_FILE_INSERT_ASCII:
		InsertAsciiFile(bv_, argument, false);
		break;
		
	case LFUN_LAYOUT:
	{
		lyxerr.debug() << "LFUN_LAYOUT: (arg) "
			       << argument << endl;
		
		// Derive layout number from given argument (string)
		// and current buffer's textclass (number). */    
		LyXTextClassList::ClassList::size_type tclass =
			buffer_->params.textclass;
		pair <bool, LyXTextClass::size_type> layout = 
			textclasslist.NumberOfLayout(tclass, argument);

		// If the entry is obsolete, use the new one instead.
		if (layout.first) {
			string obs = textclasslist.Style(tclass,layout.second)
				.obsoleted_by();
			if (!obs.empty()) 
				layout = 
					textclasslist.NumberOfLayout(tclass, obs);
		}

		// see if we found the layout number:
		if (!layout.first) {
			owner_->getLyXFunc()->setErrorMessage(
				string(N_("Layout ")) + argument +
				N_(" not known"));
			break;
		}

		if (current_layout != layout.second) {
			hideCursor();
			current_layout = layout.second;
			update(TEXT(bv_),
			       BufferView::SELECT|BufferView::FITCUR);
			TEXT(bv_)->SetLayout(bv_, layout.second);
			owner_->setLayout(layout.second);
			update(TEXT(bv_),
			       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			setState();
		}
	}
	break;

	case LFUN_LANGUAGE:
		Lang(bv_, argument);
		setState();
		owner_->showState();
		break;

	case LFUN_EMPH:
		Emph(bv_);
		owner_->showState();
		break;

	case LFUN_BOLD:
		Bold(bv_);
		owner_->showState();
		break;
		
	case LFUN_NOUN:
		Noun(bv_);
		owner_->showState();
		break;
		
	case LFUN_CODE:
		Code(bv_);
		owner_->showState();
		break;
		
	case LFUN_SANS:
		Sans(bv_);
		owner_->showState();
		break;
		
	case LFUN_ROMAN:
		Roman(bv_);
		owner_->showState();
		break;
		
	case LFUN_DEFAULT:
		StyleReset(bv_);
		owner_->showState();
		break;
		
	case LFUN_UNDERLINE:
		Underline(bv_);
		owner_->showState();
		break;
		
	case LFUN_FONT_SIZE:
		FontSize(bv_, argument);
		owner_->showState();
		break;
		
	case LFUN_FONT_STATE:
		owner_->getLyXFunc()->setMessage(CurrentState(bv_));
		break;
		
	case LFUN_UPCASE_WORD:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->ChangeWordCase(bv_, LyXText::text_uppercase);
		if (TEXT(bv_)->inset_owner)
			bv_->updateInset(TEXT(bv_)->inset_owner, true);
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		break;
		
	case LFUN_LOWCASE_WORD:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->ChangeWordCase(bv_, LyXText::text_lowercase);
		if (TEXT(bv_)->inset_owner)
			bv_->updateInset(TEXT(bv_)->inset_owner, true);
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		break;
		
	case LFUN_CAPITALIZE_WORD:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->ChangeWordCase(bv_,
					     LyXText::text_capitalization);
		if (TEXT(bv_)->inset_owner)
			bv_->updateInset(TEXT(bv_)->inset_owner, true);
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		break;
		
	case LFUN_INSERT_LABEL:
		MenuInsertLabel(bv_, argument);
		break;
		
	case LFUN_REF_INSERT:
		if (argument.empty()) {
			InsetCommandParams p("ref");
			owner_->getDialogs()->createRef(p.getAsString());
		} else {
			InsetCommandParams p;
			p.setFromString(argument);

			InsetRef * inset = new InsetRef(p, *buffer_);
			if (!bv_->insertInset(inset))
				delete inset;
			else
				bv_->updateInset(inset, true);
		}
		break;

	case LFUN_BOOKMARK_SAVE:
		savePosition(strToUnsignedInt(argument));
		break;

	case LFUN_BOOKMARK_GOTO:
		restorePosition(strToUnsignedInt(argument));
		break;

	case LFUN_REF_GOTO:
	{
		string label(argument);
		if (label.empty()) {
			InsetRef * inset = 
				static_cast<InsetRef*>(getInsetByCode(Inset::REF_CODE));
			if (inset) {
                                label = inset->getContents();
				savePosition(0);
			}
		}
		
		if (!label.empty()) {
			//bv_->savePosition(0);
			if (!bv_->gotoLabel(label))
				WriteAlert(_("Error"), 
					   _("Couldn't find this label"), 
					   _("in current document."));
		}
	}
	break;
		
		// --- Cursor Movements -----------------------------
	case LFUN_RIGHT:
	{
		bool is_rtl = TEXT(bv_)->cursor.par()->isRightToLeftPar(buffer_->params);
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		if (is_rtl)
			TEXT(bv_)->CursorLeft(bv_, false);
#ifndef NEW_INSETS
		if (TEXT(bv_)->cursor.pos() < TEXT(bv_)->cursor.par()->Last()
#else
		if (TEXT(bv_)->cursor.pos() < TEXT(bv_)->cursor.par()->size()
#endif
		    && TEXT(bv_)->cursor.par()->GetChar(TEXT(bv_)->cursor.pos())
		    == LyXParagraph::META_INSET
		    && TEXT(bv_)->cursor.par()->GetInset(TEXT(bv_)->cursor.pos())
		    && TEXT(bv_)->cursor.par()->GetInset(TEXT(bv_)->cursor.pos())->Editable() == Inset::HIGHLY_EDITABLE){
			Inset * tmpinset = TEXT(bv_)->cursor.par()->GetInset(TEXT(bv_)->cursor.pos());
			owner_->getLyXFunc()->setMessage(tmpinset->EditMessage());
			int y = 0;
			if (is_rtl) {
				LyXFont font = 
					TEXT(bv_)->GetFont(buffer_,
							      TEXT(bv_)->cursor.par(),
							      TEXT(bv_)->cursor.pos());	
				y = tmpinset->descent(bv_,font);
			}
			tmpinset->Edit(bv_, 0, y, 0);
			break;
		}
		if (!is_rtl)
			TEXT(bv_)->CursorRight(bv_, false);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;
		
	case LFUN_LEFT:
	{
		// This is soooo ugly. Isn`t it possible to make
		// it simpler? (Lgb)
		bool is_rtl = TEXT(bv_)->cursor.par()->isRightToLeftPar(buffer_->params);
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		LyXCursor cur = TEXT(bv_)->cursor;
		if (!is_rtl)
			TEXT(bv_)->CursorLeft(bv_, false);
		if ((is_rtl || cur != TEXT(bv_)->cursor) && // only if really moved!
#ifndef NEW_INSETS
		    TEXT(bv_)->cursor.pos() < TEXT(bv_)->cursor.par()->Last() &&
#else
		    TEXT(bv_)->cursor.pos() < TEXT(bv_)->cursor.par()->size() &&
#endif
		    (TEXT(bv_)->cursor.par()->GetChar(TEXT(bv_)->cursor.pos()) ==
		     LyXParagraph::META_INSET) &&
		    TEXT(bv_)->cursor.par()->GetInset(TEXT(bv_)->cursor.pos()) &&
		    (TEXT(bv_)->cursor.par()->GetInset(TEXT(bv_)->cursor.pos())->Editable()
		     == Inset::HIGHLY_EDITABLE))
		{
			Inset * tmpinset = TEXT(bv_)->cursor.par()->GetInset(TEXT(bv_)->cursor.pos());
			owner_->getLyXFunc()->setMessage(tmpinset->EditMessage());
			LyXFont font = TEXT(bv_)->GetFont(buffer_,
							     TEXT(bv_)->cursor.par(),
							     TEXT(bv_)->cursor.pos());
			int y = is_rtl ? 0 
				: tmpinset->descent(bv_,font);
			tmpinset->Edit(bv_,
				       tmpinset->x() +
				       tmpinset->width(bv_,font),
				       y, 0);
			break;
		}
		if  (is_rtl)
			TEXT(bv_)->CursorRight(bv_, false);

		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;
		
	case LFUN_UP:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::UPDATE);
		TEXT(bv_)->CursorUp(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
	case LFUN_DOWN:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::UPDATE);
		TEXT(bv_)->CursorDown(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;

	case LFUN_UP_PARAGRAPH:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::UPDATE);
		TEXT(bv_)->CursorUpParagraph(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
	case LFUN_DOWN_PARAGRAPH:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::UPDATE);
		TEXT(bv_)->CursorDownParagraph(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
	case LFUN_PRIOR:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::UPDATE);
		cursorPrevious(TEXT(bv_));
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
	case LFUN_NEXT:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::UPDATE);
		cursorNext(TEXT(bv_));
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
	case LFUN_HOME:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorHome(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
	case LFUN_END:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorEnd(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
	case LFUN_SHIFT_TAB:
	case LFUN_TAB:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorTab(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
	case LFUN_WORDRIGHT:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		if (TEXT(bv_)->cursor.par()->isRightToLeftPar(buffer_->params))
			TEXT(bv_)->CursorLeftOneWord(bv_);
		else
			TEXT(bv_)->CursorRightOneWord(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
	case LFUN_WORDLEFT:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		if (TEXT(bv_)->cursor.par()->isRightToLeftPar(buffer_->params))
			TEXT(bv_)->CursorRightOneWord(bv_);
		else
			TEXT(bv_)->CursorLeftOneWord(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
	case LFUN_BEGINNINGBUF:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorTop(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
	case LFUN_ENDBUF:
		if (!TEXT(bv_)->mark_set)
			beforeChange(TEXT(bv_));
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorBottom(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(false);
		owner_->showState();
		break;

      
		/* cursor selection ---------------------------- */
	case LFUN_RIGHTSEL:
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR);
		if (TEXT(bv_)->cursor.par()->isRightToLeftPar(buffer_->params))
			TEXT(bv_)->CursorLeft(bv_);
		else
			TEXT(bv_)->CursorRight(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_LEFTSEL:
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR);
		if (TEXT(bv_)->cursor.par()->isRightToLeftPar(buffer_->params))
			TEXT(bv_)->CursorRight(bv_);
		else
			TEXT(bv_)->CursorLeft(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_UPSEL:
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorUp(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_DOWNSEL:
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorDown(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;

	case LFUN_UP_PARAGRAPHSEL:
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorUpParagraph(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_DOWN_PARAGRAPHSEL:
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorDownParagraph(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_PRIORSEL:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		cursorPrevious(TEXT(bv_));
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_NEXTSEL:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		cursorNext(TEXT(bv_));
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_HOMESEL:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorHome(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_ENDSEL:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorEnd(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_WORDRIGHTSEL:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		if (TEXT(bv_)->cursor.par()->isRightToLeftPar(buffer_->params))
			TEXT(bv_)->CursorLeftOneWord(bv_);
		else
			TEXT(bv_)->CursorRightOneWord(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_WORDLEFTSEL:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		if (TEXT(bv_)->cursor.par()->isRightToLeftPar(buffer_->params))
			TEXT(bv_)->CursorRightOneWord(bv_);
		else
			TEXT(bv_)->CursorLeftOneWord(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_BEGINNINGBUFSEL:
		if (TEXT(bv_)->inset_owner)
			break;
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorTop(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;
		
	case LFUN_ENDBUFSEL:
		if (TEXT(bv_)->inset_owner)
			break;
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->CursorBottom(bv_);
		TEXT(bv_)->FinishUndo();
		moveCursorUpdate(true);
		owner_->showState();
		break;

		// --- text changing commands ------------------------
	case LFUN_BREAKLINE:
		beforeChange(TEXT(bv_));
		TEXT(bv_)->InsertChar(bv_, LyXParagraph::META_NEWLINE);
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		moveCursorUpdate(false);
		break;
		
	case LFUN_PROTECTEDSPACE:
	{
		LyXLayout const & style =
			textclasslist.Style(buffer_->params.textclass,
					    TEXT(bv_)->cursor.par()->GetLayout());

		if (style.free_spacing) {
			TEXT(bv_)->InsertChar(bv_, ' ');
			update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		} else {
			bv_->protectedBlank(TEXT(bv_));
		}
		moveCursorUpdate(false);
	}
	break;
		
	case LFUN_SETMARK:
		if (TEXT(bv_)->mark_set) {
			beforeChange(TEXT(bv_));
			update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
			owner_->getLyXFunc()->setMessage(N_("Mark removed"));
		} else {
			beforeChange(TEXT(bv_));
			TEXT(bv_)->mark_set = 1;
			update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
			owner_->getLyXFunc()->setMessage(N_("Mark set"));
		}
		TEXT(bv_)->sel_cursor = TEXT(bv_)->cursor;
		break;
		
	case LFUN_DELETE:
		if (!TEXT(bv_)->selection) {
			TEXT(bv_)->Delete(bv_);
			TEXT(bv_)->sel_cursor = 
				TEXT(bv_)->cursor;
			update(TEXT(bv_),
			       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			// It is possible to make it a lot faster still
			// just comment out the line below...
			showCursor();
		} else {
			bv_->cut();
		}
		moveCursorUpdate(false);
		owner_->showState();
		setState();
		break;

	case LFUN_DELETE_SKIP:
	{
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		
		LyXCursor cursor = TEXT(bv_)->cursor;

		if (!TEXT(bv_)->selection) {
#ifndef NEW_INSETS
			if (cursor.pos() == cursor.par()->Last()) {
#else
			if (cursor.pos() == cursor.par()->size()) {
#endif
				TEXT(bv_)->CursorRight(bv_);
				cursor = TEXT(bv_)->cursor;
				if (cursor.pos() == 0
				    && !(cursor.par()->params.spaceTop()
					 == VSpace (VSpace::NONE))) {
					TEXT(bv_)->SetParagraph
						(bv_,
						 cursor.par()->params.lineTop(),
						 cursor.par()->params.lineBottom(),
						 cursor.par()->params.pagebreakTop(), 
						 cursor.par()->params.pagebreakBottom(),
						 VSpace(VSpace::NONE), 
						 cursor.par()->params.spaceBottom(),
						 cursor.par()->params.align(), 
						 cursor.par()->params.labelWidthString(), 0);
					TEXT(bv_)->CursorLeft(bv_);
					update(TEXT(bv_), 
					       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
				} else {
					TEXT(bv_)->CursorLeft(bv_);
					TEXT(bv_)->Delete(bv_);
					TEXT(bv_)->sel_cursor = 
						TEXT(bv_)->cursor;
					update(TEXT(bv_),
					       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
				}
			} else {
				TEXT(bv_)->Delete(bv_);
				TEXT(bv_)->sel_cursor = 
					TEXT(bv_)->cursor;
				update(TEXT(bv_),
				       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			}
		} else {
			bv_->cut();
		}
	}
	break;

	/* -------> Delete word forward. */
	case LFUN_DELETE_WORD_FORWARD:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->DeleteWordForward(bv_);
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		moveCursorUpdate(false);
		owner_->showState();
		break;

		/* -------> Delete word backward. */
	case LFUN_DELETE_WORD_BACKWARD:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->DeleteWordBackward(bv_);
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		moveCursorUpdate(false);
		owner_->showState();
		break;
		
		/* -------> Kill to end of line. */
	case LFUN_DELETE_LINE_FORWARD:
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->DeleteLineForward(bv_);
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		moveCursorUpdate(false);
		break;
		
		/* -------> Set mark off. */
	case LFUN_MARK_OFF:
		beforeChange(TEXT(bv_));
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->sel_cursor = TEXT(bv_)->cursor;
		owner_->getLyXFunc()->setMessage(N_("Mark off"));
		break;

		/* -------> Set mark on. */
	case LFUN_MARK_ON:
		beforeChange(TEXT(bv_));
		TEXT(bv_)->mark_set = 1;
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR);
		TEXT(bv_)->sel_cursor = TEXT(bv_)->cursor;
		owner_->getLyXFunc()->setMessage(N_("Mark on"));
		break;
		
	case LFUN_BACKSPACE:
	{
		if (!TEXT(bv_)->selection) {
			if (owner_->getIntl()->getTrans().backspace()) {
				TEXT(bv_)->Backspace(bv_);
				TEXT(bv_)->sel_cursor = 
					TEXT(bv_)->cursor;
				update(TEXT(bv_),
				       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
				// It is possible to make it a lot faster still
				// just comment out the line below...
				showCursor();
			}
		} else {
			bv_->cut();
		}
		owner_->showState();
		setState();
	}
	break;

	case LFUN_BACKSPACE_SKIP:
	{
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		
		LyXCursor cursor = TEXT(bv_)->cursor;
		
		if (!TEXT(bv_)->selection) {
			if (cursor.pos() == 0 
			    && !(cursor.par()->params.spaceTop() 
				 == VSpace (VSpace::NONE))) {
				TEXT(bv_)->SetParagraph 
					(bv_,
					 cursor.par()->params.lineTop(),      
					 cursor.par()->params.lineBottom(),
					 cursor.par()->params.pagebreakTop(), 
					 cursor.par()->params.pagebreakBottom(),
					 VSpace(VSpace::NONE), cursor.par()->params.spaceBottom(),
					 cursor.par()->params.align(), 
					 cursor.par()->params.labelWidthString(), 0);
				update(TEXT(bv_),
				       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			} else {
				TEXT(bv_)->Backspace(bv_);
				TEXT(bv_)->sel_cursor 
					= cursor;
				update(TEXT(bv_),
				       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			}
		} else
			bv_->cut();
	}
	break;

	case LFUN_BREAKPARAGRAPH:
	{
		beforeChange(TEXT(bv_));
		TEXT(bv_)->BreakParagraph(bv_, 0);
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		TEXT(bv_)->sel_cursor = TEXT(bv_)->cursor;
		setState();
		owner_->showState();
		break;
	}

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	{
		beforeChange(TEXT(bv_));
		TEXT(bv_)->BreakParagraph(bv_, 1);
		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		TEXT(bv_)->sel_cursor = TEXT(bv_)->cursor;
		setState();
		owner_->showState();
		break;
	}
	
	case LFUN_BREAKPARAGRAPH_SKIP:
	{
		// When at the beginning of a paragraph, remove
		// indentation and add a "defskip" at the top.
		// Otherwise, do the same as LFUN_BREAKPARAGRAPH.
		
		LyXCursor cursor = TEXT(bv_)->cursor;
		
		beforeChange(TEXT(bv_));
		if (cursor.pos() == 0) {
			if (cursor.par()->params.spaceTop() == VSpace(VSpace::NONE)) {
				TEXT(bv_)->SetParagraph
					(bv_,
					 cursor.par()->params.lineTop(),      
					 cursor.par()->params.lineBottom(),
					 cursor.par()->params.pagebreakTop(), 
					 cursor.par()->params.pagebreakBottom(),
					 VSpace(VSpace::DEFSKIP), cursor.par()->params.spaceBottom(),
					 cursor.par()->params.align(), 
					 cursor.par()->params.labelWidthString(), 1);
				//update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			} 
		}
		else {
			TEXT(bv_)->BreakParagraph(bv_, 0);
			//update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}

		update(TEXT(bv_),
		       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		TEXT(bv_)->sel_cursor = cursor;
		setState();
		owner_->showState();
	}
	break;

	case LFUN_PARAGRAPH_SPACING:
	{
		LyXParagraph * par = TEXT(bv_)->cursor.par();
		Spacing::Space cur_spacing = par->params.spacing().getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other) {
			cur_value = par->params.spacing().getValue();
		}
		
		istringstream istr(argument.c_str());

		string tmp;
		istr >> tmp;
		Spacing::Space new_spacing = cur_spacing;
		float new_value = cur_value;
		if (tmp.empty()) {
			lyxerr << "Missing argument to `paragraph-spacing'"
			       << endl;
		} else if (tmp == "single") {
			new_spacing = Spacing::Single;
		} else if (tmp == "onehalf") {
			new_spacing = Spacing::Onehalf;
		} else if (tmp == "double") {
			new_spacing = Spacing::Double;
		} else if (tmp == "other") {
			new_spacing = Spacing::Other;
			float tmpval = 0.0;
			istr >> tmpval;
			lyxerr << "new_value = " << tmpval << endl;
			if (tmpval != 0.0)
				new_value = tmpval;
		} else if (tmp == "default") {
			new_spacing = Spacing::Default;
		} else {
			lyxerr << _("Unknown spacing argument: ")
			       << argument << endl;
		}
		if (cur_spacing != new_spacing || cur_value != new_value) {
			par->params.spacing(Spacing(new_spacing, new_value));
			TEXT(bv_)->RedoParagraph(bv_);
			update(TEXT(bv_),
			       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}
	}
	break;
	
	case LFUN_QUOTE:
		beforeChange(TEXT(bv_));
		TEXT(bv_)->InsertChar(bv_, '\"');  // This " matches the single quote in the code
		update(TEXT(bv_), BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
                moveCursorUpdate(false);
		break;

	case LFUN_HTMLURL:
	case LFUN_URL:
	{
		InsetCommandParams p;
		if (action == LFUN_HTMLURL)
			p.setCmdName("htmlurl");
		else
			p.setCmdName("url");
		owner_->getDialogs()->createUrl( p.getAsString() );
	}
	break;
		    
	case LFUN_INSERT_URL:
	{
		InsetCommandParams p;
		p.setFromString( argument );

		InsetUrl * inset = new InsetUrl( p );
		if (!bv_->insertInset(inset))
			delete inset;
		else
			bv_->updateInset( inset, true );
	}
	break;
		    
	case LFUN_INSET_TEXT:
	{
		InsetText * new_inset = new InsetText;
		if (bv_->insertInset(new_inset))
			new_inset->Edit(bv_, 0, 0, 0);
		else
			delete new_inset;
	}
	break;
	
	case LFUN_INSET_ERT:
	{
		InsetERT * new_inset = new InsetERT;
		if (bv_->insertInset(new_inset))
			new_inset->Edit(bv_, 0, 0, 0);
		else
			delete new_inset;
	}
	break;
	
	case LFUN_INSET_EXTERNAL:
	{
		InsetExternal * new_inset = new InsetExternal;
		if (bv_->insertInset(new_inset))
			new_inset->Edit(bv_, 0, 0, 0);
		else
			delete new_inset;
	}
	break;
	
	case LFUN_INSET_FOOTNOTE:
	{
		InsetFoot * new_inset = new InsetFoot;
		if (bv_->insertInset(new_inset))
			new_inset->Edit(bv_, 0, 0, 0);
		else
			delete new_inset;
	}
	break;

	case LFUN_INSET_MARGINAL:
	{
		InsetMarginal * new_inset = new InsetMarginal;
		if (bv_->insertInset(new_inset))
			new_inset->Edit(bv_, 0, 0, 0);
		else
			delete new_inset;
	}
	break;

	case LFUN_INSET_MINIPAGE:
	{
		InsetMinipage * new_inset = new InsetMinipage;
		if (bv_->insertInset(new_inset))
			new_inset->Edit(bv_, 0, 0, 0);
		else
			delete new_inset;
	}
	break;

	case LFUN_INSET_FLOAT:
	{
		// check if the float type exist
		if (floatList.typeExist(argument)) {
			InsetFloat * new_inset = new InsetFloat(argument);
			if (bv_->insertInset(new_inset))
				new_inset->Edit(bv_, 0, 0, 0);
			else
				delete new_inset;
		} else {
			lyxerr << "Non-existant float type: "
			       << argument << endl;
		}
		
	}
	break;

	case LFUN_INSET_LIST:
	{
		InsetList * new_inset = new InsetList;
		if (bv_->insertInset(new_inset))
			new_inset->Edit(bv_, 0, 0, 0);
		else
			delete new_inset;
	}
	break;

	case LFUN_INSET_THEOREM:
	{
		InsetTheorem * new_inset = new InsetTheorem;
		if (bv_->insertInset(new_inset))
			new_inset->Edit(bv_, 0, 0, 0);
		else
			delete new_inset;
	}
	break;

	case LFUN_INSET_CAPTION:
	{
		// Do we have a locking inset...
		if (bv_->theLockingInset()) {
			lyxerr << "Locking inset code: "
			       << static_cast<int>(bv_->theLockingInset()->LyxCode());
			InsetCaption * new_inset = new InsetCaption;
			new_inset->setOwner(bv_->theLockingInset());
			new_inset->SetAutoBreakRows(true);
			new_inset->SetDrawFrame(0, InsetText::LOCKED);
			new_inset->SetFrameColor(0, LColor::footnoteframe);
			if (bv_->insertInset(new_inset))
				new_inset->Edit(bv_, 0, 0, 0);
			else
				delete new_inset;
		}
	}
	break;
	
	case LFUN_INSET_TABULAR:
	{
		int r = 2, c = 2;
		if (!argument.empty())
			::sscanf(argument.c_str(),"%d%d", &r, &c);
		InsetTabular * new_inset =
			new InsetTabular(*buffer_, r, c);
		bool rtl =
			TEXT(bv_)->real_current_font.isRightToLeft();
		if (!bv_->open_new_inset(new_inset, rtl))
			delete new_inset;
	}
	break;

	// --- lyxserver commands ----------------------------

	case LFUN_CHARATCURSOR:
	{
		LyXParagraph::size_type pos = TEXT(bv_)->cursor.pos();
		if (pos < TEXT(bv_)->cursor.par()->size())
			owner_->getLyXFunc()->setMessage(
				tostr(TEXT(bv_)->cursor.par()->GetChar(pos)));
		else
			owner_->getLyXFunc()->setMessage("EOF");
	}
	break;
	
	case LFUN_GETXY:
		owner_->getLyXFunc()->setMessage(tostr(TEXT(bv_)->cursor.x())
						 + ' '
						 + tostr(TEXT(bv_)->cursor.y()));
		break;
		
	case LFUN_SETXY:
	{
		int x;
		int y;
		::sscanf(argument.c_str(), " %d %d", &x, &y);
		TEXT(bv_)->SetCursorFromCoordinates(bv_, x, y);
	}
	break;
	
	case LFUN_GETLAYOUT:
		owner_->getLyXFunc()->setMessage(tostr(TEXT(bv_)->cursor.par()->layout));
		break;
			
	case LFUN_GETFONT:
	{
		LyXFont & font = TEXT(bv_)->current_font;
                if (font.shape() == LyXFont::ITALIC_SHAPE)
			owner_->getLyXFunc()->setMessage("E");
                else if (font.shape() == LyXFont::SMALLCAPS_SHAPE)
			owner_->getLyXFunc()->setMessage("N");
                else
			owner_->getLyXFunc()->setMessage("0");

	}
	break;

	case LFUN_GETLATEX:
	{
		LyXFont & font = TEXT(bv_)->current_font;
                if (font.latex() == LyXFont::ON)
			owner_->getLyXFunc()->setMessage("L");
                else
			owner_->getLyXFunc()->setMessage("0");
	}
	break;

	// --- accented characters ---------------------------
		
	case LFUN_UMLAUT:
	case LFUN_CIRCUMFLEX:
	case LFUN_GRAVE:
	case LFUN_ACUTE:
	case LFUN_TILDE:
	case LFUN_CEDILLA:
	case LFUN_MACRON:
	case LFUN_DOT:
	case LFUN_UNDERDOT:
	case LFUN_UNDERBAR:
	case LFUN_CARON:
	case LFUN_SPECIAL_CARON:
	case LFUN_BREVE:
	case LFUN_TIE:
	case LFUN_HUNG_UMLAUT:
	case LFUN_CIRCLE:
	case LFUN_OGONEK:
		owner_->getLyXFunc()->handleKeyFunc(action);
		break;
	
	// --- insert characters ----------------------------------------
	
	case LFUN_MATH_DELIM:     
	case LFUN_INSERT_MATRIX:
	{ 	   
		if (available()) { 
			if (bv_->open_new_inset(new InsetFormula(false)))
			{
				bv_->theLockingInset()
					->LocalDispatch(bv_, action, argument);
			}
		}
	}	   
	break;
	       
	case LFUN_INSERT_MATH:
	{
		if (!available())
			break;
 
		InsetFormula * f = new InsetFormula(true);
		bv_->open_new_inset(f);
		f->LocalDispatch(bv_, LFUN_INSERT_MATH, argument);
	}
	break;
	
	case LFUN_MATH_DISPLAY:
	{
		if (available())
			bv_->open_new_inset(new InsetFormula(true));
		break;
	}
		    
	case LFUN_MATH_MACRO:
	{
		if (available()) {
			string s(argument);
		        if (s.empty())
				owner_->getLyXFunc()->setErrorMessage(N_("Missing argument"));
		        else {
				string const s1 = token(s, ' ', 1);
				int const na = s1.empty() ? 0 : lyx::atoi(s1);
				bv_->open_new_inset(new InsetFormulaMacro(token(s, ' ', 0), na));
			}
		}
	}
	break;

	case LFUN_MATH_MODE:   // Open or create a math inset
	{		
		if (available())
			bv_->open_new_inset(new InsetFormula);
		owner_->getLyXFunc()->setMessage(N_("Math editor mode"));
	}
	break;
	  
	case LFUN_CITATION_INSERT:
	{
		InsetCommandParams p;
		p.setFromString( argument );

		InsetCitation * inset = new InsetCitation( p );
		if (!bv_->insertInset(inset))
			delete inset;
		else
			bv_->updateInset( inset, true );
	}
	break;
		    
	case LFUN_INSERT_BIBTEX:
	{   
		// ale970405+lasgoutt970425
		// The argument can be up to two tokens separated 
		// by a space. The first one is the bibstyle.
		string const db       = token(argument, ' ', 0);
		string bibstyle = token(argument, ' ', 1);
		if (bibstyle.empty())
			bibstyle = "plain";

		InsetCommandParams p( "BibTeX", db, bibstyle );
		InsetBibtex * inset = new InsetBibtex(p);
		
		if (bv_->insertInset(inset)) {
			if (argument.empty())
				inset->Edit(bv_, 0, 0, 0);
		} else
			delete inset;
	}
	break;
		
	// BibTeX data bases
	case LFUN_BIBDB_ADD:
	{
		InsetBibtex * inset = 
			static_cast<InsetBibtex*>(getInsetByCode(Inset::BIBTEX_CODE));
		if (inset) {
			inset->addDatabase(argument);
		}
	}
	break;
		    
	case LFUN_BIBDB_DEL:
	{
		InsetBibtex * inset = 
			static_cast<InsetBibtex*>(getInsetByCode(Inset::BIBTEX_CODE));
		if (inset) {
			inset->delDatabase(argument);
		}
	}
	break;
	
	case LFUN_BIBTEX_STYLE:
	{
		InsetBibtex * inset = 
			static_cast<InsetBibtex*>(getInsetByCode(Inset::BIBTEX_CODE));
		if (inset) {
			inset->setOptions(argument);
		}
	}
	break;
		
	case LFUN_INDEX_CREATE:
	{
		InsetCommandParams p( "index" );
		
		if (argument.empty()) {
			// Get the word immediately preceding the cursor
			LyXParagraph::size_type curpos = 
				TEXT(bv_)->cursor.pos() - 1;

			string curstring;
			if (curpos >= 0 )
				curstring = TEXT(bv_)
					->cursor.par()->GetWord(curpos);

			p.setContents( curstring );
		} else {
			p.setContents( argument );
		}

		owner_->getDialogs()->createIndex( p.getAsString() );
	}
	break;
		    
	case LFUN_INDEX_INSERT:
	{
		InsetCommandParams p;
		p.setFromString(argument);
		InsetIndex * inset = new InsetIndex(p);

		if (!bv_->insertInset(inset))
			delete inset;
		else
			bv_->updateInset(inset, true);
	}
	break;
		    
	case LFUN_INDEX_INSERT_LAST:
	{
		// Get word immediately preceding the cursor
		LyXParagraph::size_type curpos = 
			TEXT(bv_)->cursor.pos() - 1;
	  	// Can't do that at the beginning of a paragraph
	  	if (curpos < 0) break;

		string const curstring(TEXT(bv_)
				       ->cursor.par()->GetWord(curpos));

		InsetCommandParams p("index", curstring);
		InsetIndex * inset = new InsetIndex(p);

		if (!bv_->insertInset(inset))
			delete inset;
		else
			bv_->updateInset(inset, true);
	}
	break;

	case LFUN_INDEX_PRINT:
	{
		InsetCommandParams p("printindex");
		Inset * inset = new InsetPrintIndex(p);
		if (!bv_->insertInset(inset, "Standard", true))
			delete inset;
	}
	break;

	case LFUN_PARENTINSERT:
	{
		lyxerr << "arg " << argument << endl;
		InsetCommandParams p( "lyxparent", argument );
		Inset * inset = new InsetParent(p, *buffer_);
		if (!bv_->insertInset(inset, "Standard", true))
			delete inset;
	}
		 
	break;

	case LFUN_CHILD_INSERT:
	{
		InsetInclude::Params p;
		p.cparams.setFromString(argument);
		p.masterFilename_ = buffer_->fileName();

		InsetInclude * inset = new InsetInclude(p);
		if (!bv_->insertInset(inset))
			delete inset;
		else {
			bv_->updateInset(inset, true);
			bv_->owner()->getDialogs()->showInclude(inset);
		}
	}
	break; 

	case LFUN_INSERT_NOTE:
		bv_->insertNote();
		break;

	case LFUN_SELFINSERT:
	{
		LyXFont const old_font(TEXT(bv_)->real_current_font);
		for (string::size_type i = 0; i < argument.length(); ++i) {
			TEXT(bv_)->InsertChar(bv_, argument[i]);
			// This needs to be in the loop, or else we
			// won't break lines correctly. (Asger)
			update(TEXT(bv_),
			       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}
		TEXT(bv_)->sel_cursor = TEXT(bv_)->cursor;
		moveCursorUpdate(false);

		// real_current_font.number can change so we need to 
		// update the minibuffer
		if (old_font != TEXT(bv_)->real_current_font)
			owner_->showState();
	}
	break;

	case LFUN_DATE_INSERT:  // jdblair: date-insert cmd
	{
		struct tm * now_tm;
		
		time_t now_time_t = time(NULL);
		now_tm = localtime(&now_time_t);
		setlocale(LC_TIME, "");
		string arg;
		if (!argument.empty())
			arg = argument;
		else 
			arg = lyxrc.date_insert_format;
		char datetmp[32];
		int const datetmp_len =
			::strftime(datetmp, 32, arg.c_str(), now_tm);
		for (int i = 0; i < datetmp_len; i++) {
			TEXT(bv_)->InsertChar(bv_, datetmp[i]);
			update(TEXT(bv_),
			       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}

		TEXT(bv_)->sel_cursor = TEXT(bv_)->cursor;
		moveCursorUpdate(false);
	}
	break;

	case LFUN_UNKNOWN_ACTION:
	{
		if (!buffer_) {
			owner_->getLyXFunc()->setErrorMessage(N_("No document open"));
			break;
		}

		if (buffer_->isReadonly()) {
			owner_->getLyXFunc()->setErrorMessage(N_("Document is read only"));
			break;
		}
		if (!argument.empty()) {
			/* Automatically delete the currently selected
			 * text and replace it with what is being
			 * typed in now. Depends on lyxrc settings
			 * "auto_region_delete", which defaults to
			 * true (on). */
		
			if (lyxrc.auto_region_delete) {
				if (TEXT(bv_)->selection){
					TEXT(bv_)->CutSelection(bv_, false);
					update(TEXT(bv_),
					       BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
				}
			}
			
			beforeChange(TEXT(bv_));
			LyXFont const old_font(TEXT(bv_)->real_current_font);
			for (string::size_type i = 0;
			     i < argument.length(); ++i) {
				if (greek_kb_flag) {
					if (!math_insert_greek(bv_, argument[i]))
						owner_->getIntl()->getTrans().TranslateAndInsert(argument[i], TEXT(bv_));
				} else
					owner_->getIntl()->getTrans().TranslateAndInsert(argument[i], TEXT(bv_));
			}

			update(TEXT(bv_),BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);

			TEXT(bv_)->sel_cursor = TEXT(bv_)->cursor;
			moveCursorUpdate(false);

			// real_current_font.number can change so we need to
			// update the minibuffer
			if (old_font != TEXT(bv_)->real_current_font)
				owner_->showState();
		} else {
			// if we don't have an argument there was something
			// strange going on so we just tell this to someone!
			owner_->getLyXFunc()->setErrorMessage(N_("No argument given"));
		}
		break;
	}
	default:
		return false;
	} // end of switch

	return true;
}
