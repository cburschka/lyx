#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "BufferView_pimpl.h"
#include "WorkArea.h"
#include "lyxscreen.h"
#include "lyxtext.h"
#include "lyxrow.h"
#include "paragraph.h"
#include "LyXView.h"
#include "commandtags.h"
#include "lyxfunc.h"
#include "debug.h"
#include "font.h"
#include "bufferview_funcs.h"
#include "TextCache.h"
#include "bufferlist.h"
#include "lyxrc.h"
#include "intl.h"
// added for Dispatch functions
#include "lyx_cb.h"
#include "lyx_main.h"
#include "FloatList.h"
#include "gettext.h"
#include "ParagraphParameters.h"
#include "undo_funcs.h"
#include "lyxtextclasslist.h"

#include "frontends/Dialogs.h"
#include "frontends/Alert.h"
#include "frontends/FileDialog.h"

#include "insets/insetbib.h"
#include "insets/insettext.h"
#include "insets/inseturl.h"
#include "insets/insetlatexaccent.h"
#include "insets/insettoc.h"
#include "insets/insetref.h"
#include "insets/insetparent.h"
#include "insets/insetindex.h"
#include "insets/insetnote.h"
#include "insets/insetinclude.h"
#include "insets/insetcite.h"
#include "insets/insetert.h"
#include "insets/insetexternal.h"
#include "insets/insetgraphics.h"
#include "insets/insetfoot.h"
#include "insets/insetmarginal.h"
#include "insets/insetminipage.h"
#include "insets/insetfloat.h"
#include "insets/insettabular.h"
#if 0
#include "insets/insettheorem.h"
#include "insets/insetlist.h"
#endif
#include "insets/insetcaption.h"
#include "insets/insetfloatlist.h"
#include "insets/insetspecialchar.h"

#include "mathed/formulabase.h"

#include "support/LAssert.h"
#include "support/lstrings.h"
#include "support/filetools.h"
#include "support/lyxfunctional.h"

#include <ctime>
#include <unistd.h>
#include <sys/wait.h>
#include <clocale>


extern string current_layout;

using std::vector;
using std::find_if;
using std::find;
using std::pair;
using std::endl;
using std::make_pair;
using std::min;
using SigC::slot;

using lyx::pos_type;
using lyx::textclass_type;

/* the selection possible is needed, that only motion events are
 * used, where the bottom press event was on the drawing area too */
bool selection_possible = false;

extern BufferList bufferlist;
extern char ascii_type;

extern int bibitemMaxWidth(BufferView *, LyXFont const &);


namespace {

const unsigned int saved_positions_num = 20;

inline
void waitForX()
{
	XSync(fl_get_display(), 0);
}


void SetXtermCursor(Window win)
{
	static Cursor cursor;
	static bool cursor_undefined = true;
	if (cursor_undefined) {
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
	  workarea_(xpos, ypos, width, height), using_xterm_cursor(false),
	  inset_slept(false)
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
	workarea_.selectionRequested
		.connect(slot(this, &BufferView::Pimpl::selectionRequested));
	workarea_.selectionLost
		.connect(slot(this, &BufferView::Pimpl::selectionLost));

	cursor_timeout.timeout.connect(slot(this,
					    &BufferView::Pimpl::cursorToggle));
	cursor_timeout.start();
	workarea_.setFocus();
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
		insetSleep();
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
		bv_->text->first_y = screen_->topCursorVisible(bv_->text);
		owner_->updateMenubar();
		owner_->updateToolbar();
		// Similarly, buffer-dependent dialogs should be updated or
		// hidden. This should go here because some dialogs (eg ToC)
		// require bv_->text.
		owner_->getDialogs()->updateBufferDependent(true);
		redraw();
		insetWakeup();
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


bool BufferView::Pimpl::fitCursor()
{
	lyx::Assert(screen_.get());

	bool ret;

	if (bv_->theLockingInset()) {
		bv_->theLockingInset()->fitInsetCursor(bv_);
		ret = true;
	} else {
		ret = screen_->fitCursor(bv_->text, bv_);
	}

	bv_->owner()->getDialogs()->updateParagraph();
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

	Paragraph * par = 0;
	Paragraph * selstartpar = 0;
	Paragraph * selendpar = 0;
	UpdatableInset * the_locking_inset = 0;

	pos_type pos = 0;
	pos_type selstartpos = 0;
	pos_type selendpos = 0;
	bool selection = false;
	bool mark_set  = false;

	owner_->prohibitInput();

	owner_->message(_("Formatting document..."));

	if (bv_->text) {
		par = bv_->text->cursor.par();
		pos = bv_->text->cursor.pos();
		selstartpar = bv_->text->selection.start.par();
		selstartpos = bv_->text->selection.start.pos();
		selendpar = bv_->text->selection.end.par();
		selendpos = bv_->text->selection.end.pos();
		selection = bv_->text->selection.set();
		mark_set = bv_->text->selection.mark();
		the_locking_inset = bv_->theLockingInset();
		delete bv_->text;
		bv_->text = new LyXText(bv_);
		bv_->text->init(bv_);
		buffer_->resizeInsets(bv_);
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
			bv_->text->init(bv_);
			//buffer_->resizeInsets(bv_);
		}
	}

	updateScreen();

	if (par) {
		bv_->text->selection.set(true);
		// At this point just to avoid the Delete-Empty-Paragraph-
		// Mechanism when setting the cursor.
		bv_->text->selection.mark(mark_set);
		if (selection) {
			bv_->text->setCursor(bv_, selstartpar, selstartpos);
			bv_->text->selection.cursor = bv_->text->cursor;
			bv_->text->setCursor(bv_, selendpar, selendpos);
			bv_->text->setSelection(bv_);
			bv_->text->setCursor(bv_, par, pos);
		} else {
			bv_->text->setCursor(bv_, par, pos);
			bv_->text->selection.cursor = bv_->text->cursor;
			bv_->text->selection.set(false);
		}
		// remake the inset locking
		bv_->theLockingInset(the_locking_inset);
	}

	bv_->text->first_y = screen_->topCursorVisible(bv_->text);

	// this will scroll the screen such that the cursor becomes visible
	updateScrollbar();
	redraw();

	setState();
	owner_->allowInput();

	/// clear the "Formatting Document" message
	owner_->message("");

	return 0;
}


void BufferView::Pimpl::updateScreen()
{
	// Regenerate the screen.
	screen_.reset(new LyXScreen(workarea_));
}


void BufferView::Pimpl::updateScrollbar()
{
	if (!bv_->text) {
		lyxerr[Debug::GUI] << "no text in updateScrollbar" << endl;
		workarea_.setScrollbar(0, 1.0);
		return;
	}

	long const text_height = bv_->text->height;
	long const work_height = workarea_.height();

	double const lineh = bv_->text->defaultHeight();
	double const slider_size =
		(text_height == 0) ? 1.0 : 1.0 / double(text_height);

	lyxerr[Debug::GUI] << "text_height now " << text_height << endl;
	lyxerr[Debug::GUI] << "work_height " << work_height << endl;
 
	/* If the text is smaller than the working area, the scrollbar
	 * maximum must be the working area height. No scrolling will
	 * be possible */
	if (text_height <= work_height) {
		lyxerr[Debug::GUI] << "doc smaller than workarea !" << endl;
		workarea_.setScrollbarBounds(0.0, 0.0);
		current_scrollbar_value = bv_->text->first_y;
		workarea_.setScrollbar(current_scrollbar_value, 1.0);
		return;
	}

	workarea_.setScrollbarBounds(0.0, text_height - work_height);
	workarea_.setScrollbarIncrements(lineh);
	current_scrollbar_value = bv_->text->first_y;
	workarea_.setScrollbar(current_scrollbar_value, slider_size);
}


// Callback for scrollbar slider
void BufferView::Pimpl::scrollCB(double value)
{
	lyxerr[Debug::GUI] << "scrollCB of " << value << endl;
 
	if (!buffer_) return;

	current_scrollbar_value = long(value);

	if (current_scrollbar_value < 0)
		current_scrollbar_value = 0;

	if (!screen_.get())
		return;

	screen_->draw(bv_->text, bv_, current_scrollbar_value);

	if (!lyxrc.cursor_follows_scrollbar) {
		waitForX();
		return;
	}

	LyXText * vbt = bv_->text;

	int const height = vbt->defaultHeight();
	int const first = static_cast<int>((bv_->text->first_y + height));
	int const last = static_cast<int>((bv_->text->first_y + workarea_.height() - height));

	if (vbt->cursor.y() < first)
		vbt->setCursorFromCoordinates(bv_, 0, first);
	else if (vbt->cursor.y() > last)
		vbt->setCursorFromCoordinates(bv_, 0, last);

	waitForX();
}


int BufferView::Pimpl::scrollUp(long time)
{
	if (!buffer_) return 0;
	if (!screen_.get()) return 0;

	double value = workarea_.getScrollbarValue();

	if (value == 0) return 0;

	float add_value =  (bv_->text->defaultHeight()
			    + float(time) * float(time) * 0.125);

	if (add_value > workarea_.height())
		add_value = float(workarea_.height() -
				  bv_->text->defaultHeight());

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
	double const max = p.second;

	if (value == max) return 0;

	float add_value =  (bv_->text->defaultHeight()
			    + float(time) * float(time) * 0.125);

	if (add_value > workarea_.height())
		add_value = float(workarea_.height() -
				  bv_->text->defaultHeight());

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
		LyXFont font = bv_->text->getFont(buffer_,
						  cursor.par(), cursor.pos());
		int width = bv_->theLockingInset()->width(bv_, font);
		int inset_x = font.isVisibleRightToLeft()
			? cursor.x() - width : cursor.x();
		int start_x = inset_x + bv_->theLockingInset()->scroll();
		bv_->theLockingInset()->
			insetMotionNotify(bv_,
					  x - start_x,
					  y - cursor.y() + bv_->text->first_y,
					  state);
		return;
	}

	/* The test for not selection possible is needed, that only motion
	   events are used, where the bottom press event was on
	   the drawing area too */
	if (!selection_possible)
		return;

	screen_->hideCursor();

	bv_->text->setCursorFromCoordinates(bv_, x, y + bv_->text->first_y);

	if (!bv_->text->selection.set())
		update(bv_->text, BufferView::UPDATE); // Maybe an empty line was deleted

	bv_->text->setSelection(bv_);
	screen_->toggleToggle(bv_->text, bv_);
	fitCursor();
	showCursor();
}


// Single-click on work area
void BufferView::Pimpl::workAreaButtonPress(int xpos, int ypos,
					    unsigned int button)
{
	if (!buffer_ || !screen_.get())
		return;

	Inset * inset_hit = checkInsetHit(bv_->text, xpos, ypos);

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

	// Middle button press pastes if we have a selection
	// We do this here as if the selection was inside an inset
	// it could get cleared on the unlocking of the inset so
	// we have to check this first
	bool paste_internally = false;
	if (button == 2 && bv_->getLyXText()->selection.set()) {
		owner_->getLyXFunc()->dispatch(LFUN_COPY);
		paste_internally = true;
	}

	if (bv_->theLockingInset()) {
		// We are in inset locking mode

		/* Check whether the inset was hit. If not reset mode,
		   otherwise give the event to the inset */
		if (inset_hit == bv_->theLockingInset()) {
			bv_->theLockingInset()->
				insetButtonPress(bv_,xpos, ypos,button);
			return;
		} else {
			bv_->unlockInset(bv_->theLockingInset());
		}
	}

	if (!inset_hit)
		selection_possible = true;
	screen_->hideCursor();

	int const screen_first = bv_->text->first_y;

	// Clear the selection
	screen_->toggleSelection(bv_->text, bv_);
	bv_->text->clearSelection();
	bv_->text->fullRebreak(bv_);
	update();
	updateScrollbar();

	// Single left click in math inset?
	if (isHighlyEditableInset(inset_hit)) {
		// Highly editable inset, like math
		UpdatableInset * inset = static_cast<UpdatableInset *>(inset_hit);
		selection_possible = false;
		owner_->updateLayoutChoice();
		owner_->message(inset->editMessage());
		// IMO the inset has to be first in edit-mode and then we send the
		// button press. (Jug 20020222)
		inset->edit(bv_); //, xpos, ypos, button);
		inset->insetButtonPress(bv_, xpos, ypos, button);
		return;
	}

	// Right click on a footnote flag opens float menu
	if (button == 3) {
		selection_possible = false;
		return;
	}

	if (!inset_hit) // otherwise it was already set in checkInsetHit(...)
		bv_->text->setCursorFromCoordinates(bv_, xpos, ypos + screen_first);
	finishUndo();
	bv_->text->selection.cursor = bv_->text->cursor;
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
			owner_->getLyXFunc()->dispatch(LFUN_PASTE);
		else
			owner_->getLyXFunc()->dispatch(LFUN_PASTESELECTION,
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
			screen_->hideCursor();
			screen_->toggleSelection(text, bv_);
			text->selectWord(bv_, LyXText::WHOLE_WORD_STRICT);
			screen_->toggleSelection(text, bv_, false);
		} else {
			text->selectWord(bv_, LyXText::WHOLE_WORD_STRICT);
		}
		/* This will fit the cursor on the screen
		 * if necessary */
		update(text, BufferView::SELECT|BufferView::FITCUR);
	}
}


void BufferView::Pimpl::tripleClick(int /*x*/, int /*y*/, unsigned int button)
{
	// select a line
	if (!buffer_)
		return;

	LyXText * text = bv_->getLyXText();

	if (text->bv_owner && bv_->theLockingInset())
	    return;

	if (screen_.get() && (button == 1)) {
		if (text->bv_owner) {
			screen_->hideCursor();
			screen_->toggleSelection(text, bv_);
		}
		text->cursorHome(bv_);
		text->selection.cursor = text->cursor;
		text->cursorEnd(bv_);
		text->setSelection(bv_);
		if (text->bv_owner) {
			screen_->toggleSelection(text, bv_, false);
		}
		/* This will fit the cursor on the screen
		 * if necessary */
		update(text, BufferView::SELECT|BufferView::FITCUR);
	}
}


void BufferView::Pimpl::selectionRequested()
{
	static string sel;

	if (!available())
		return;

	LyXText * text = bv_->getLyXText();

	if (text->selection.set() &&
		(!bv_->text->xsel_cache.set() ||
		 text->selection.start != bv_->text->xsel_cache.start ||
		 text->selection.end != bv_->text->xsel_cache.end))
	{
		bv_->text->xsel_cache = text->selection;
		sel = text->selectionAsString(bv_->buffer(), false);
	} else if (!text->selection.set()) {
		sel = string();
		bv_->text->xsel_cache.set(false);
	}
	if (!sel.empty()) {
		workarea_.putClipboard(sel);
	}
}


void BufferView::Pimpl::selectionLost()
{
	if (active() && available()) {
		hideCursor();
		toggleSelection();
		bv_->getLyXText()->clearSelection();
		showCursor();
		bv_->text->xsel_cache.set(false);
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
	Inset * inset_hit = checkInsetHit(bv_->text, x, y);

	if (bv_->theLockingInset()) {
		// We are in inset locking mode.

		/* LyX does a kind of work-area grabbing for insets.
		   Only a ButtonPress Event outside the inset will
		   force a insetUnlock. */
		bv_->theLockingInset()->
			insetButtonRelease(bv_, x, y, button);
		return;
	}

	selection_possible = false;

	if (button == 2)
		return;

	// finish selection
	if (button == 1) {
		workarea_.haveSelection(bv_->getLyXText()->selection.set());
	}

	setState();
	owner_->showState();
	owner_->updateMenubar();
	owner_->updateToolbar();

	// Did we hit an editable inset?
	if (inset_hit) {
		selection_possible = false;

		// if we reach this point with a selection, it
		// must mean we are currently selecting.
		// But we don't want to open the inset
		// because that is annoying for the user.
		// So just pretend we didn't hit it.
		// this is OK because a "kosher" ButtonRelease
		// will follow a ButtonPress that clears
		// the selection.
		// Note this also fixes selection drawing
		// problems if we end up opening an inset
		if (bv_->getLyXText()->selection.set())
			return;

		// CHECK fix this proper in 0.13
		// well, maybe 13.0 !!!!!!!!!

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
		if (inset_hit->lyxCode() == Inset::REF_CODE) {
			setCursorParUndo(bv_);
		}

		owner_->message(inset_hit->editMessage());

		if (isHighlyEditableInset(inset_hit)) {
			// Highly editable inset, like math
			UpdatableInset *inset = (UpdatableInset *)inset_hit;
			inset->insetButtonRelease(bv_, x, y, button);
		} else {
			inset_hit->insetButtonRelease(bv_, x, y, button);
			inset_hit->edit(bv_, x, y, button);
		}
		return;
	}

	// Maybe we want to edit a bibitem ale970302
	if (bv_->text->cursor.par()->bibkey && x < 20 +
	    bibitemMaxWidth(bv_, textclasslist[buffer_->params.textclass].defaultfont())) {
		bv_->text->cursor.par()->bibkey->edit(bv_, 0, 0, 0);
	}

	return;
}


Box BufferView::Pimpl::insetDimensions(LyXText const & text,
				       LyXCursor const & cursor) const
{
	Paragraph /*const*/ & par = *cursor.par();
	pos_type const pos = cursor.pos();

	lyx::Assert(par.getInset(pos));

	Inset const & inset(*par.getInset(pos));

	LyXFont const & font = text.getFont(buffer_, &par, pos);

	int const width = inset.width(bv_, font);
	int const inset_x = font.isVisibleRightToLeft()
		? (cursor.x() - width) : cursor.x();

	return Box(
		inset_x + inset.scroll(),
		inset_x + width,
		cursor.y() - inset.ascent(bv_, font),
		cursor.y() + inset.descent(bv_, font));
}


Inset * BufferView::Pimpl::checkInset(LyXText const & text,
				      LyXCursor const & cursor,
				      int & x, int & y) const
{
	pos_type const pos(cursor.pos());
	Paragraph /*const*/ & par(*cursor.par());

	if (pos >= par.size() || !par.isInset(pos)) {
		return 0;
	}

	Inset /*const*/ * inset = par.getInset(pos);

	if (!isEditableInset(inset)) {
		return 0;
	}

	Box b(insetDimensions(text, cursor));

	if (!b.contained(x, y)) {
		lyxerr[Debug::GUI] << "Missed inset at x,y " << x << "," << y
			<< " box " << b << endl;
		return 0;
	}

	text.setCursor(bv_, &par, pos, true);

	x -= b.x1;
	// The origin of an inset is on the baseline
	y -= (text.cursor.y());

	return inset;
}


Inset * BufferView::Pimpl::checkInsetHit(LyXText * text, int & x, int & y)
{
	if (!screen_.get())
		return 0;

	int y_tmp = y + text->first_y;

	LyXCursor cursor;
	text->setCursorFromCoordinates(bv_, cursor, x, y_tmp);

	Inset * inset(checkInset(*text, cursor, x, y_tmp));

	if (inset) {
		y = y_tmp;
		return inset;
	}

	// look at previous position

	if (cursor.pos() == 0) {
		return 0;
	}

	// move back one
	text->setCursor(bv_, cursor, cursor.par(), cursor.pos() - 1, true);

	inset = checkInset(*text, cursor, x, y_tmp);
	if (inset) {
		y = y_tmp;
	}
	return inset;
}


void BufferView::Pimpl::workAreaExpose()
{
	static int work_area_width;
	static unsigned int work_area_height;

	bool const widthChange = workarea_.workWidth() != work_area_width;
	bool const heightChange = workarea_.height() != work_area_height;

	// update from work area
	work_area_width = workarea_.workWidth();
	work_area_height = workarea_.height();
	if (buffer_ != 0) {
		if (widthChange) {
			// The visible LyXView need a resize
			owner_->resize();

			// Remove all texts from the textcache
			// This is not _really_ what we want to do. What
			// we really want to do is to delete in textcache
			// that does not have a BufferView with matching
			// width, but as long as we have only one BufferView
			// deleting all gives the same result.
			if (lyxerr.debugging())
				textcache.show(lyxerr, "Expose delete all");
			textcache.clear();
			buffer_->resizeInsets(bv_);
		} else if (heightChange) {
			// Rebuild image of current screen
			updateScreen();
			// fitCursor() ensures we don't jump back
			// to the start of the document on vertical
			// resize
			fitCursor();

			// The main window size has changed, repaint most stuff
			redraw();
		} else if (screen_.get())
		    screen_->redraw(bv_->text, bv_);
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
	if (screen_.get() &&
		(!bv_->theLockingInset() || !bv_->theLockingInset()->nodraw()))
	{
		LyXText::text_status st = bv_->text->status();
		screen_->update(bv_->text, bv_);
		bool fitc = false;
		while (bv_->text->status() == LyXText::CHANGED_IN_DRAW) {
			if (bv_->text->fullRebreak(bv_)) {
				st = LyXText::NEED_MORE_REFRESH;
				bv_->text->setCursor(bv_, bv_->text->cursor.par(),
									 bv_->text->cursor.pos());
				fitc = true;
			}
			bv_->text->status(bv_, st);
			screen_->update(bv_->text, bv_);
		}
		// do this here instead of in the screen::update because of
		// the above loop!
		bv_->text->status(bv_, LyXText::UNCHANGED);
		if (fitc)
			fitCursor();
	}
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

	if (!text->selection.set() && (f & SELECT)) {
		text->selection.cursor = text->cursor;
	}

	text->fullRebreak(bv_);

	if (text->inset_owner) {
		text->inset_owner->setUpdateStatus(bv_, InsetText::NONE);
	    updateInset(text->inset_owner, false);
	} else {
	    update();
	}

	if ((f & FITCUR)) {
		fitCursor();
	}

	if ((f & CHANGE)) {
		buffer_->markDirty();
	}
}


// Callback for cursor timer
void BufferView::Pimpl::cursorToggle()
{
	if (!buffer_) {
		cursor_timeout.restart();
		return;
	}

	if (!screen_.get()) {
		cursor_timeout.restart();
		return;
	}

	/* FIXME */
	extern void reapSpellchecker(void);
	reapSpellchecker();

	if (!bv_->theLockingInset()) {
		screen_->cursorToggle(bv_);
	} else {
		bv_->theLockingInset()->toggleInsetCursor(bv_);
	}

	cursor_timeout.restart();
}


void BufferView::Pimpl::cursorPrevious(LyXText * text)
{
	if (!text->cursor.row()->previous())
		return;

	int y = text->first_y;
	Row * cursorrow = text->cursor.row();

	text->setCursorFromCoordinates(bv_, bv_->text->cursor.x_fix(), y);
	finishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == text->cursor.row()))
		text->cursorUp(bv_);

	if (text->inset_owner ||
	    text->cursor.row()->height() < workarea_.height())
		screen_->draw(bv_->text, bv_,
			      text->cursor.y()
			      - text->cursor.row()->baseline()
			      + text->cursor.row()->height()
			      - workarea_.height() + 1);
	updateScrollbar();
}


void BufferView::Pimpl::cursorNext(LyXText * text)
{
	if (!text->cursor.row()->next())
		return;

	int y = text->first_y + workarea_.height();
//	if (text->inset_owner)
//		y += bv_->text->first;
	text->getRowNearY(y);

	Row * cursorrow = text->cursor.row();
	text->setCursorFromCoordinates(bv_, text->cursor.x_fix(), y); // + workarea_->height());
	finishUndo();
	// This is to allow jumping over large insets
	if ((cursorrow == bv_->text->cursor.row()))
		text->cursorDown(bv_);

	if (text->inset_owner ||
	    text->cursor.row()->height() < workarea_.height())
		screen_->draw(bv_->text, bv_, text->cursor.y() -
			      text->cursor.row()->baseline());
	updateScrollbar();
}


bool BufferView::Pimpl::available() const
{
	if (buffer_ && bv_->text)
		return true;
	return false;
}


void BufferView::Pimpl::beforeChange(LyXText * text)
{
	toggleSelection();
	text->clearSelection();
}


void BufferView::Pimpl::savePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;
	saved_positions[i] = Position(buffer_->fileName(),
				      bv_->text->cursor.par()->id(),
				      bv_->text->cursor.pos());
	if (i > 0) {
		ostringstream str;
		str << _("Saved bookmark") << ' ' << i;
		owner_->message(str.str().c_str());
	}
}


void BufferView::Pimpl::restorePosition(unsigned int i)
{
	if (i >= saved_positions_num)
		return;

	string const fname = saved_positions[i].filename;

	beforeChange(bv_->text);

	if (fname != buffer_->fileName()) {
		Buffer * b = bufferlist.exists(fname) ?
			bufferlist.getBuffer(fname) :
			bufferlist.loadLyXFile(fname); // don't ask, just load it
		if (b != 0) buffer(b);
	}

	Paragraph * par = buffer_->getParFromID(saved_positions[i].par_id);
	if (!par)
		return;

	bv_->text->setCursor(bv_, par,
			     min(par->size(), saved_positions[i].par_pos));

	update(bv_->text, BufferView::SELECT|BufferView::FITCUR);
	if (i > 0) {
		ostringstream str;
		str << _("Moved to bookmark") << ' ' << i;
		owner_->message(str.str().c_str());
	}
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
	if (text->real_current_font.isRightToLeft()) {
		if (owner_->getIntl()->keymap == Intl::PRIMARY)
			owner_->getIntl()->KeyMapSec();
	} else {
		if (owner_->getIntl()->keymap == Intl::SECONDARY)
			owner_->getIntl()->KeyMapPrim();
	}
}


void BufferView::Pimpl::insetSleep()
{
	if (bv_->theLockingInset() && !inset_slept) {
		bv_->theLockingInset()->getCursorPos(bv_, bv_->slx, bv_->sly);
		bv_->theLockingInset()->insetUnlock(bv_);
		inset_slept = true;
	}
}


void BufferView::Pimpl::insetWakeup()
{
	if (bv_->theLockingInset() && inset_slept) {
		bv_->theLockingInset()->edit(bv_, bv_->slx, bv_->sly, 0);
		inset_slept = false;
	}
}


void BufferView::Pimpl::insetUnlock()
{
	if (bv_->theLockingInset()) {
		if (!inset_slept)
			bv_->theLockingInset()->insetUnlock(bv_);
		bv_->theLockingInset(0);
		finishUndo();
		inset_slept = false;
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
	if (screen_.get()) {
		if (bv_->theLockingInset())
			bv_->theLockingInset()->showInsetCursor(bv_);
		else
			screen_->showCursor(bv_->text, bv_);
	}
}


void BufferView::Pimpl::hideCursor()
{
	if (screen_.get()) {
		if (!bv_->theLockingInset())
//			bv_->theLockingInset()->hideInsetCursor(bv_);
//		else
			screen_->hideCursor();
	}
}


void BufferView::Pimpl::toggleSelection(bool b)
{
	if (screen_.get()) {
		if (bv_->theLockingInset())
			bv_->theLockingInset()->toggleSelection(bv_, b);
		screen_->toggleSelection(bv_->text, bv_, b);
	}
}


void BufferView::Pimpl::toggleToggle()
{
	if (screen_.get())
		screen_->toggleToggle(bv_->text, bv_);
}


void BufferView::Pimpl::center()
{
	beforeChange(bv_->text);
	if (bv_->text->cursor.y() > static_cast<int>((workarea_.height() / 2))) {
		screen_->draw(bv_->text, bv_, bv_->text->cursor.y() - workarea_.height() / 2);
	} else {
		screen_->draw(bv_->text, bv_, 0);
	}
	update(bv_->text, BufferView::SELECT|BufferView::FITCUR);
	redraw();
}


void BufferView::Pimpl::pasteClipboard(bool asPara)
{
	if (!buffer_)
		return;

	screen_->hideCursor();
	beforeChange(bv_->text);

	string const clip(workarea_.getClipboard());

	if (clip.empty())
		return;

	if (asPara) {
		bv_->getLyXText()->insertStringAsParagraphs(bv_, clip);
	} else {
		bv_->getLyXText()->insertStringAsLines(bv_, clip);
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


inline
void BufferView::Pimpl::moveCursorUpdate(bool selecting)
{
	LyXText * lt = bv_->getLyXText();

	if (selecting || lt->selection.mark()) {
		lt->setSelection(bv_);
		if (lt->bv_owner)
			toggleToggle();
		else
			updateInset(lt->inset_owner, false);
	}
	if (lt->bv_owner) {
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		showCursor();
	}

	if (!lt->selection.set())
		workarea_.haveSelection(false);

	/* ---> Everytime the cursor is moved, show the current font state. */
	// should this too me moved out of this func?
	//owner->showState();
	setState();
}


Inset * BufferView::Pimpl::getInsetByCode(Inset::Code code)
{
	LyXCursor cursor = bv_->getLyXText()->cursor;
	Buffer::inset_iterator it =
		find_if(Buffer::inset_iterator(
			cursor.par(), cursor.pos()),
			buffer_->inset_iterator_end(),
			lyx::compare_memfun(&Inset::lyxCode, code));
	return it != buffer_->inset_iterator_end() ? (*it) : 0;
}


void BufferView::Pimpl::MenuInsertLyXFile(string const & filen)
{
	string filename = filen;

	if (filename.empty()) {
		// Launch a file browser
		string initpath = lyxrc.document_path;

		if (available()) {
			string const trypath = owner_->buffer()->filePath();
			// If directory is writeable, use this as default.
			if (IsDirWriteable(trypath))
				initpath = trypath;
		}

		FileDialog fileDlg(bv_->owner(),
				   _("Select LyX document to insert"),
			LFUN_FILE_INSERT,
			make_pair(string(_("Documents|#o#O")),
				  string(lyxrc.document_path)),
			make_pair(string(_("Examples|#E#e")),
				  string(AddPath(system_lyxdir, "examples"))));

		FileDialog::Result result =
			fileDlg.Select(initpath,
				       _("*.lyx| LyX Documents (*.lyx)"));

		if (result.first == FileDialog::Later)
			return;

		filename = result.second;

		// check selected filename
		if (filename.empty()) {
			owner_->message(_("Canceled."));
			return;
		}
	}

	// get absolute path of file and add ".lyx" to the filename if
	// necessary
	filename = FileSearch(string(), filename, "lyx");

	string const disp_fn(MakeDisplayPath(filename));

	ostringstream s1;
	s1 << _("Inserting document") << ' '
	   << disp_fn << " ...";
	owner_->message(s1.str().c_str());
	bool const res = bv_->insertLyXFile(filename);
	if (res) {
		ostringstream str;
		str << _("Document") << ' ' << disp_fn
		    << ' ' << _("inserted.");
		owner_->message(str.str().c_str());
	} else {
		ostringstream str;
		str << _("Could not insert document") << ' '
		    << disp_fn;
		owner_->message(str.str().c_str());
	}
}


bool BufferView::Pimpl::Dispatch(kb_action action, string const & argument)
{
	lyxerr[Debug::ACTION] << "BufferView::Pimpl::Dispatch: action["
			      << action <<"] arg[" << argument << "]" << endl;

	switch (action) {
		// --- Misc -------------------------------------------
	case LFUN_APPENDIX:
	{
		if (available()) {
			LyXText * lt = bv_->getLyXText();
			lt->toggleAppendix(bv_);
			update(lt,
			       BufferView::SELECT
			       | BufferView::FITCUR
			       | BufferView::CHANGE);
		}
	}
	break;

	case LFUN_TOC_INSERT:
	{
		InsetCommandParams p;
		p.setCmdName("tableofcontents");
		Inset * inset = new InsetTOC(p);
		if (!insertInset(inset, "Standard"))
			delete inset;
		break;
	}

	case LFUN_SCROLL_INSET:
		// this is not handled here as this funktion is only aktive
		// if we have a locking_inset and that one is (or contains)
		// a tabular-inset
		break;

	case LFUN_INSET_GRAPHICS:
	{
		Inset * new_inset = new InsetGraphics;
		if (!insertInset(new_inset)) {
			delete new_inset;
		} else {
			// this is need because you don't use a inset->Edit()
			updateInset(new_inset, true);
			new_inset->edit(bv_);
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
		if (argument == "paragraph")
			asPara = true;
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
		gotoInset(Inset::ERROR_CODE, false);
		break;

	case LFUN_GOTONOTE:
		gotoInset(Inset::IGNORE_CODE, false);
		break;

	case LFUN_REFERENCE_GOTO:
	{
		vector<Inset::Code> tmp;
		tmp.push_back(Inset::LABEL_CODE);
		tmp.push_back(Inset::REF_CODE);
		gotoInset(tmp, true);
		break;
	}

	case LFUN_HYPHENATION:
		specialChar(InsetSpecialChar::HYPHENATION);
		break;

	case LFUN_LIGATURE_BREAK:
		specialChar(InsetSpecialChar::LIGATURE_BREAK);
		break;

	case LFUN_LDOTS:
		specialChar(InsetSpecialChar::LDOTS);
		break;

	case LFUN_END_OF_SENTENCE:
		specialChar(InsetSpecialChar::END_OF_SENTENCE);
		break;

	case LFUN_MENU_SEPARATOR:
		specialChar(InsetSpecialChar::MENU_SEPARATOR);
		break;

	case LFUN_HFILL:
		hfill();
		break;

	case LFUN_DEPTH_MIN:
		changeDepth(bv_, bv_->getLyXText(), -1);
		break;

	case LFUN_DEPTH_PLUS:
		changeDepth(bv_, bv_->getLyXText(), 1);
		break;

	case LFUN_FREE:
		owner_->getDialogs()->setUserFreeFont();
		break;

	case LFUN_FILE_INSERT:
		MenuInsertLyXFile(argument);
		break;

	case LFUN_FILE_INSERT_ASCII_PARA:
		InsertAsciiFile(bv_, argument, true);
		break;

	case LFUN_FILE_INSERT_ASCII:
		InsertAsciiFile(bv_, argument, false);
		break;

	case LFUN_LAYOUT:
	{
		lyxerr[Debug::INFO] << "LFUN_LAYOUT: (arg) "
				    << argument << endl;

		// Derive layout number from given argument (string)
		// and current buffer's textclass (number). */
		textclass_type tclass = buffer_->params.textclass;
		bool hasLayout =
			textclasslist[tclass].hasLayout(argument);
		string layout = argument;

		// If the entry is obsolete, use the new one instead.
		if (hasLayout) {
			string const & obs = textclasslist[tclass][layout]
				.obsoleted_by();
			if (!obs.empty())
				layout = obs;
		}

		if (!hasLayout) {
			owner_->getLyXFunc()->setErrorMessage(
				string(N_("Layout ")) + argument +
				N_(" not known"));
			break;
		}

		if (current_layout != layout) {
			LyXText * lt = bv_->getLyXText();
			hideCursor();
			current_layout = layout;
			update(lt,
			       BufferView::SELECT
			       | BufferView::FITCUR);
			lt->setLayout(bv_, layout);
			owner_->setLayout(layout);
			update(lt,
			       BufferView::SELECT
			       | BufferView::FITCUR
			       | BufferView::CHANGE);
			setState();
		}
	}
	break;

	case LFUN_LANGUAGE:
		lang(bv_, argument);
		setState();
		owner_->showState();
		break;

	case LFUN_EMPH:
		emph(bv_);
		owner_->showState();
		break;

	case LFUN_BOLD:
		bold(bv_);
		owner_->showState();
		break;

	case LFUN_NOUN:
		noun(bv_);
		owner_->showState();
		break;

	case LFUN_CODE:
		code(bv_);
		owner_->showState();
		break;

	case LFUN_SANS:
		sans(bv_);
		owner_->showState();
		break;

	case LFUN_ROMAN:
		roman(bv_);
		owner_->showState();
		break;

	case LFUN_DEFAULT:
		styleReset(bv_);
		owner_->showState();
		break;

	case LFUN_UNDERLINE:
		underline(bv_);
		owner_->showState();
		break;

	case LFUN_FONT_SIZE:
		fontSize(bv_, argument);
		owner_->showState();
		break;

	case LFUN_FONT_STATE:
		owner_->getLyXFunc()->setMessage(currentState(bv_));
		break;

	case LFUN_UPCASE_WORD:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR);
		lt->changeCase(bv_, LyXText::text_uppercase);
		if (lt->inset_owner)
			updateInset(lt->inset_owner, true);
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
	}
	break;

	case LFUN_LOWCASE_WORD:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->changeCase(bv_, LyXText::text_lowercase);
		if (lt->inset_owner)
			updateInset(lt->inset_owner, true);
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
	}
	break;

	case LFUN_CAPITALIZE_WORD:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->changeCase(bv_, LyXText::text_capitalization);
		if (lt->inset_owner)
			updateInset(lt->inset_owner, true);
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
	}
	break;

	case LFUN_TRANSPOSE_CHARS:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->transposeChars(*bv_);
		if (lt->inset_owner)
			updateInset(lt->inset_owner, true);
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
	}
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
			if (!insertInset(inset))
				delete inset;
			else
				updateInset(inset, true);
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
				Alert::alert(_("Error"),
					   _("Couldn't find this label"),
					   _("in current document."));
		}
	}
	break;

		// --- Cursor Movements -----------------------------
	case LFUN_RIGHT:
	{
		LyXText * lt = bv_->getLyXText();

		bool is_rtl = lt->cursor.par()->isRightToLeftPar(buffer_->params);
		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		if (is_rtl)
			lt->cursorLeft(bv_, false);
		if (lt->cursor.pos() < lt->cursor.par()->size()
		    && lt->cursor.par()->isInset(lt->cursor.pos())
		    && isHighlyEditableInset(lt->cursor.par()->getInset(lt->cursor.pos()))) {
			Inset * tmpinset = lt->cursor.par()->getInset(lt->cursor.pos());
			owner_->getLyXFunc()->setMessage(tmpinset->editMessage());
			if (is_rtl)
				tmpinset->edit(bv_, false);
			else
				tmpinset->edit(bv_);
			break;
		}
		if (!is_rtl)
			lt->cursorRight(bv_, false);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_LEFT:
	{
		// This is soooo ugly. Isn`t it possible to make
		// it simpler? (Lgb)
		LyXText * lt = bv_->getLyXText();
		bool is_rtl = lt->cursor.par()->isRightToLeftPar(buffer_->params);
		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		LyXCursor const cur = lt->cursor;
		if (!is_rtl)
			lt->cursorLeft(bv_, false);
		if ((is_rtl || cur != lt->cursor) && // only if really moved!
		    lt->cursor.pos() < lt->cursor.par()->size() &&
		    lt->cursor.par()->isInset(lt->cursor.pos()) &&
		    isHighlyEditableInset(lt->cursor.par()->getInset(lt->cursor.pos()))) {
			Inset * tmpinset = lt->cursor.par()->getInset(lt->cursor.pos());
			owner_->getLyXFunc()->setMessage(tmpinset->editMessage());
			if (is_rtl)
				tmpinset->edit(bv_);
			else
				tmpinset->edit(bv_, false);
			break;
		}
		if  (is_rtl)
			lt->cursorRight(bv_, false);

		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_UP:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt, BufferView::UPDATE);
		lt->cursorUp(bv_);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_DOWN:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt, BufferView::UPDATE);
		lt->cursorDown(bv_);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_UP_PARAGRAPH:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt, BufferView::UPDATE);
		lt->cursorUpParagraph(bv_);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_DOWN_PARAGRAPH:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt, BufferView::UPDATE);
		lt->cursorDownParagraph(bv_);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_PRIOR:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt, BufferView::UPDATE);
		cursorPrevious(lt);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_NEXT:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt, BufferView::UPDATE);
		cursorNext(lt);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_HOME:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->cursorHome(bv_);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_END:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt,
		       BufferView::SELECT|BufferView::FITCUR);
		lt->cursorEnd(bv_);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_SHIFT_TAB:
	case LFUN_TAB:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt,
		       BufferView::SELECT|BufferView::FITCUR);
		lt->cursorTab(bv_);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_WORDRIGHT:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		if (lt->cursor.par()->isRightToLeftPar(buffer_->params))
			lt->cursorLeftOneWord(bv_);
		else
			lt->cursorRightOneWord(bv_);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_WORDLEFT:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		if (lt->cursor.par()->isRightToLeftPar(buffer_->params))
			lt->cursorRightOneWord(bv_);
		else
			lt->cursorLeftOneWord(bv_);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_BEGINNINGBUF:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt,
		       BufferView::SELECT|BufferView::FITCUR);
		lt->cursorTop(bv_);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

	case LFUN_ENDBUF:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.mark())
			beforeChange(lt);
		update(lt,
		       BufferView::SELECT|BufferView::FITCUR);
		lt->cursorBottom(bv_);
		finishUndo();
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

		/* cursor selection ---------------------------- */
	case LFUN_RIGHTSEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt,
		       BufferView::SELECT|BufferView::FITCUR);
		if (lt->cursor.par()->isRightToLeftPar(buffer_->params))
			lt->cursorLeft(bv_);
		else
			lt->cursorRight(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_LEFTSEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt,
		       BufferView::SELECT|BufferView::FITCUR);
		if (lt->cursor.par()->isRightToLeftPar(buffer_->params))
			lt->cursorRight(bv_);
		else
			lt->cursorLeft(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_UPSEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt,
		       BufferView::SELECT|BufferView::FITCUR);
		lt->cursorUp(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_DOWNSEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt,
		       BufferView::SELECT|BufferView::FITCUR);
		lt->cursorDown(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_UP_PARAGRAPHSEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt,
		       BufferView::SELECT|BufferView::FITCUR);
		lt->cursorUpParagraph(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_DOWN_PARAGRAPHSEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt,
		       BufferView::SELECT|BufferView::FITCUR);
		lt->cursorDownParagraph(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_PRIORSEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt, BufferView::SELECT|BufferView::FITCUR);
		cursorPrevious(lt);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_NEXTSEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt, BufferView::SELECT|BufferView::FITCUR);
		cursorNext(lt);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_HOMESEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->cursorHome(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_ENDSEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->cursorEnd(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_WORDRIGHTSEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt, BufferView::SELECT|BufferView::FITCUR);
		if (lt->cursor.par()->isRightToLeftPar(buffer_->params))
			lt->cursorLeftOneWord(bv_);
		else
			lt->cursorRightOneWord(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_WORDLEFTSEL:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt, BufferView::SELECT|BufferView::FITCUR);
		if (lt->cursor.par()->isRightToLeftPar(buffer_->params))
			lt->cursorRightOneWord(bv_);
		else
			lt->cursorLeftOneWord(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_BEGINNINGBUFSEL:
	{
		LyXText * lt = bv_->getLyXText();

		if (lt->inset_owner)
			break;
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->cursorTop(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

	case LFUN_ENDBUFSEL:
	{
		LyXText * lt = bv_->getLyXText();

		if (lt->inset_owner)
			break;
		update(lt,
		       BufferView::SELECT|BufferView::FITCUR);
		lt->cursorBottom(bv_);
		finishUndo();
		moveCursorUpdate(true);
		owner_->showState();
	}
	break;

		// --- text changing commands ------------------------
	case LFUN_BREAKLINE:
	{
		LyXText * lt = bv_->getLyXText();

		beforeChange(lt);
		lt->insertChar(bv_, Paragraph::META_NEWLINE);
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
		moveCursorUpdate(false);
	}
	break;

	case LFUN_PROTECTEDSPACE:
	{
		LyXText * lt = bv_->getLyXText();

		LyXLayout const & style = textclasslist[buffer_->params.textclass][lt->cursor.par()->layout()];

		if (style.free_spacing) {
			lt->insertChar(bv_, ' ');
			update(lt,
			       BufferView::SELECT
			       | BufferView::FITCUR
			       | BufferView::CHANGE);
		} else {
			specialChar(InsetSpecialChar::PROTECTED_SEPARATOR);
		}
		moveCursorUpdate(false);
	}
	break;

	case LFUN_SETMARK:
	{
		LyXText * lt = bv_->getLyXText();

		if (lt->selection.mark()) {
			beforeChange(lt);
			update(lt,
			       BufferView::SELECT
			       | BufferView::FITCUR);
			owner_->getLyXFunc()->setMessage(N_("Mark removed"));
		} else {
			beforeChange(lt);
			lt->selection.mark(true);
			update(lt,
			       BufferView::SELECT
			       | BufferView::FITCUR);
			owner_->getLyXFunc()->setMessage(N_("Mark set"));
		}
		lt->selection.cursor = lt->cursor;
	}
	break;

	case LFUN_DELETE:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.set()) {
			lt->Delete(bv_);
			lt->selection.cursor = lt->cursor;
			update(lt,
			       BufferView::SELECT
			       | BufferView::FITCUR
			       | BufferView::CHANGE);
			// It is possible to make it a lot faster still
			// just comment out the line below...
			showCursor();
		} else {
			bv_->cut(false);
		}
		moveCursorUpdate(false);
		owner_->showState();
		setState();
	}
	break;

	case LFUN_DELETE_SKIP:
	{
		LyXText * lt = bv_->getLyXText();

		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.

		LyXCursor cursor = lt->cursor;

		if (!lt->selection.set()) {
			if (cursor.pos() == cursor.par()->size()) {
				lt->cursorRight(bv_);
				cursor = lt->cursor;
				if (cursor.pos() == 0
				    && !(cursor.par()->params().spaceTop()
					 == VSpace (VSpace::NONE))) {
					lt->setParagraph
						(bv_,
						 cursor.par()->params().lineTop(),
						 cursor.par()->params().lineBottom(),
						 cursor.par()->params().pagebreakTop(),
						 cursor.par()->params().pagebreakBottom(),
						 VSpace(VSpace::NONE),
						 cursor.par()->params().spaceBottom(),
						 cursor.par()->params().spacing(),
						 cursor.par()->params().align(),
						 cursor.par()->params().labelWidthString(), 0);
					lt->cursorLeft(bv_);
					update(lt,
					       BufferView::SELECT
					       | BufferView::FITCUR
					       | BufferView::CHANGE);
				} else {
					lt->cursorLeft(bv_);
					lt->Delete(bv_);
					lt->selection.cursor = lt->cursor;
					update(lt,
					       BufferView::SELECT
					       | BufferView::FITCUR
					       | BufferView::CHANGE);
				}
			} else {
				lt->Delete(bv_);
				lt->selection.cursor = lt->cursor;
				update(lt,
				       BufferView::SELECT
				       | BufferView::FITCUR
				       | BufferView::CHANGE);
			}
		} else {
			bv_->cut(false);
		}
	}
	break;

	/* -------> Delete word forward. */
	case LFUN_DELETE_WORD_FORWARD:
		update(bv_->getLyXText(), BufferView::SELECT|BufferView::FITCUR);
		bv_->getLyXText()->deleteWordForward(bv_);
		update(bv_->getLyXText(), BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		moveCursorUpdate(false);
		owner_->showState();
		break;

		/* -------> Delete word backward. */
	case LFUN_DELETE_WORD_BACKWARD:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->deleteWordBackward(bv_);
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
		moveCursorUpdate(false);
		owner_->showState();
	}
	break;

		/* -------> Kill to end of line. */
	case LFUN_DELETE_LINE_FORWARD:
	{
		LyXText * lt = bv_->getLyXText();

		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->deleteLineForward(bv_);
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
		moveCursorUpdate(false);
	}
	break;

		/* -------> Set mark off. */
	case LFUN_MARK_OFF:
	{
		LyXText * lt = bv_->getLyXText();

		beforeChange(lt);
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->selection.cursor = lt->cursor;
		owner_->getLyXFunc()->setMessage(N_("Mark off"));
	}
	break;

		/* -------> Set mark on. */
	case LFUN_MARK_ON:
	{
		LyXText * lt = bv_->getLyXText();

		beforeChange(lt);
		lt->selection.mark(true);
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->selection.cursor = lt->cursor;
		owner_->getLyXFunc()->setMessage(N_("Mark on"));
	}
	break;

	case LFUN_BACKSPACE:
	{
		LyXText * lt = bv_->getLyXText();

		if (!lt->selection.set()) {
			if (owner_->getIntl()->getTrans().backspace()) {
				lt->backspace(bv_);
				lt->selection.cursor = lt->cursor;
				update(lt,
				       BufferView::SELECT
				       | BufferView::FITCUR
				       | BufferView::CHANGE);
				// It is possible to make it a lot faster still
				// just comment out the line below...
				showCursor();
			}
		} else {
			bv_->cut(false);
		}
		owner_->showState();
		setState();
	}
	break;

	case LFUN_BACKSPACE_SKIP:
	{
		// Reverse the effect of LFUN_BREAKPARAGRAPH_SKIP.
		LyXText * lt = bv_->getLyXText();

		LyXCursor cursor = lt->cursor;

		if (!lt->selection.set()) {
			if (cursor.pos() == 0
			    && !(cursor.par()->params().spaceTop()
				 == VSpace (VSpace::NONE))) {
				lt->setParagraph
					(bv_,
					 cursor.par()->params().lineTop(),
					 cursor.par()->params().lineBottom(),
					 cursor.par()->params().pagebreakTop(),
					 cursor.par()->params().pagebreakBottom(),
					 VSpace(VSpace::NONE), cursor.par()->params().spaceBottom(),
					 cursor.par()->params().spacing(),
					 cursor.par()->params().align(),
					 cursor.par()->params().labelWidthString(), 0);
				update(lt,
				       BufferView::SELECT
				       | BufferView::FITCUR
				       | BufferView::CHANGE);
			} else {
				lt->backspace(bv_);
				lt->selection.cursor = cursor;
				update(lt,
				       BufferView::SELECT
				       | BufferView::FITCUR
				       | BufferView::CHANGE);
			}
		} else
			bv_->cut(false);
	}
	break;

	case LFUN_BREAKPARAGRAPH:
	{
		LyXText * lt = bv_->getLyXText();

		beforeChange(lt);
		lt->breakParagraph(bv_, 0);
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
		lt->selection.cursor = lt->cursor;
		setState();
		owner_->showState();
		break;
	}

	case LFUN_BREAKPARAGRAPHKEEPLAYOUT:
	{
		LyXText * lt = bv_->getLyXText();

		beforeChange(lt);
		lt->breakParagraph(bv_, 1);
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
		lt->selection.cursor = lt->cursor;
		setState();
		owner_->showState();
		break;
	}

	case LFUN_BREAKPARAGRAPH_SKIP:
	{
		// When at the beginning of a paragraph, remove
		// indentation and add a "defskip" at the top.
		// Otherwise, do the same as LFUN_BREAKPARAGRAPH.
		LyXText * lt = bv_->getLyXText();

		LyXCursor cursor = lt->cursor;

		beforeChange(lt);
		if (cursor.pos() == 0) {
			if (cursor.par()->params().spaceTop() == VSpace(VSpace::NONE)) {
				lt->setParagraph
					(bv_,
					 cursor.par()->params().lineTop(),
					 cursor.par()->params().lineBottom(),
					 cursor.par()->params().pagebreakTop(),
					 cursor.par()->params().pagebreakBottom(),
					 VSpace(VSpace::DEFSKIP), cursor.par()->params().spaceBottom(),
					 cursor.par()->params().spacing(),
					 cursor.par()->params().align(),
					 cursor.par()->params().labelWidthString(), 1);
				//update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
			}
		}
		else {
			lt->breakParagraph(bv_, 0);
			//update(BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}

		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
		lt->selection.cursor = cursor;
		setState();
		owner_->showState();
	}
	break;

	case LFUN_PARAGRAPH_SPACING:
	{
		LyXText * lt = bv_->getLyXText();

		Paragraph * par = lt->cursor.par();
		Spacing::Space cur_spacing = par->params().spacing().getSpace();
		float cur_value = 1.0;
		if (cur_spacing == Spacing::Other) {
			cur_value = par->params().spacing().getValue();
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
			par->params().spacing(Spacing(new_spacing, new_value));
			lt->redoParagraph(bv_);
			update(lt,
			       BufferView::SELECT
			       | BufferView::FITCUR
			       | BufferView::CHANGE);
		}
	}
	break;

	case LFUN_INSET_TOGGLE:
	{
		LyXText * lt = bv_->getLyXText();
		hideCursor();
		beforeChange(lt);
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		lt->toggleInset(bv_);
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		setState();
	}
		break;

	case LFUN_QUOTE:
		smartQuote();
		break;

	case LFUN_HTMLURL:
	case LFUN_URL:
	{
		InsetCommandParams p;
		if (action == LFUN_HTMLURL)
			p.setCmdName("htmlurl");
		else
			p.setCmdName("url");
		owner_->getDialogs()->createUrl(p.getAsString());
	}
	break;

	case LFUN_INSERT_URL:
	{
		InsetCommandParams p;
		p.setFromString(argument);

		InsetUrl * inset = new InsetUrl(p);
		if (!insertInset(inset))
			delete inset;
		else
			updateInset(inset, true);
	}
	break;

	case LFUN_INSET_ERT:
		insertAndEditInset(new InsetERT(buffer_->params));
		break;

	case LFUN_INSET_EXTERNAL:
		insertAndEditInset(new InsetExternal);
		break;

	case LFUN_INSET_FOOTNOTE:
		insertAndEditInset(new InsetFoot(buffer_->params));
		break;

	case LFUN_INSET_MARGINAL:
		insertAndEditInset(new InsetMarginal(buffer_->params));
		break;

	case LFUN_INSET_MINIPAGE:
		insertAndEditInset(new InsetMinipage(buffer_->params));
		break;

	case LFUN_INSERT_NOTE:
		insertAndEditInset(new InsetNote(buffer_->params));
		break;

	case LFUN_INSET_FLOAT:
		// check if the float type exist
		if (floatList.typeExist(argument)) {
			insertAndEditInset(new InsetFloat(buffer_->params,
							  argument));
		} else {
			lyxerr << "Non-existent float type: "
			       << argument << endl;
		}
		break;

	case LFUN_INSET_WIDE_FLOAT:
	{
		// check if the float type exist
		if (floatList.typeExist(argument)) {
			InsetFloat * new_inset =
				new InsetFloat(buffer_->params, argument);
			new_inset->wide(true);
			if (insertInset(new_inset))
				new_inset->edit(bv_);
			else
				delete new_inset;
		} else {
			lyxerr << "Non-existent float type: "
			       << argument << endl;
		}

	}
	break;

#if 0
	case LFUN_INSET_LIST:
		insertAndEditInset(new InsetList);
		break;

	case LFUN_INSET_THEOREM:
		insertAndEditInset(new InsetTheorem);
		break;
#endif

	case LFUN_INSET_CAPTION:
	{
		// Do we have a locking inset...
		if (bv_->theLockingInset()) {
			lyxerr << "Locking inset code: "
			       << static_cast<int>(bv_->theLockingInset()->lyxCode());
			InsetCaption * new_inset =
				new InsetCaption(buffer_->params);
			new_inset->setOwner(bv_->theLockingInset());
			new_inset->setAutoBreakRows(true);
			new_inset->setDrawFrame(0, InsetText::LOCKED);
			new_inset->setFrameColor(0, LColor::captionframe);
			if (insertInset(new_inset))
				new_inset->edit(bv_);
			else
				delete new_inset;
		}
	}
	break;

	case LFUN_INSET_TABULAR:
	{
		int r = 2;
		int c = 2;
		if (!argument.empty())
			::sscanf(argument.c_str(),"%d%d", &r, &c);
		InsetTabular * new_inset =
			new InsetTabular(*buffer_, r, c);
		bool const rtl =
			bv_->getLyXText()->real_current_font.isRightToLeft();
		if (!open_new_inset(new_inset, rtl))
			delete new_inset;
	}
	break;

	// --- lyxserver commands ----------------------------

	case LFUN_CHARATCURSOR:
	{
		pos_type pos = bv_->getLyXText()->cursor.pos();
		if (pos < bv_->getLyXText()->cursor.par()->size())
			owner_->getLyXFunc()->setMessage(
				tostr(bv_->getLyXText()->cursor.par()->getChar(pos)));
		else
			owner_->getLyXFunc()->setMessage("EOF");
	}
	break;

	case LFUN_GETXY:
		owner_->getLyXFunc()->setMessage(tostr(bv_->getLyXText()->cursor.x())
						 + ' '
						 + tostr(bv_->getLyXText()->cursor.y()));
		break;

	case LFUN_SETXY:
	{
		int x = 0;
		int y = 0;
		if (::sscanf(argument.c_str(), " %d %d", &x, &y) != 2) {
			lyxerr << "SETXY: Could not parse coordinates in '"
			       << argument << std::endl;
		}
		bv_->getLyXText()->setCursorFromCoordinates(bv_, x, y);
	}
	break;

	case LFUN_GETLAYOUT:
		owner_->getLyXFunc()->setMessage(tostr(bv_->getLyXText()->cursor.par()->layout()));
		break;

	case LFUN_GETFONT:
	{
		LyXFont & font = bv_->getLyXText()->current_font;
		if (font.shape() == LyXFont::ITALIC_SHAPE)
			owner_->getLyXFunc()->setMessage("E");
		else if (font.shape() == LyXFont::SMALLCAPS_SHAPE)
			owner_->getLyXFunc()->setMessage("N");
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
		if (argument.empty()) {
			// As always...
			owner_->getLyXFunc()->handleKeyFunc(action);
		} else {
			owner_->getLyXFunc()->handleKeyFunc(action);
			owner_->getIntl()->getTrans()
				.TranslateAndInsert(argument[0], bv_->getLyXText());
			update(bv_->getLyXText(),
			       BufferView::SELECT
			       | BufferView::FITCUR
			       | BufferView::CHANGE);
		}
		break;

	case LFUN_MATH_MACRO:
		mathDispatchMathMacro(bv_, argument);
		break;

	case LFUN_MATH_DELIM:
		mathDispatchMathDelim(bv_, argument);
		break;

	case LFUN_INSERT_MATRIX:
		mathDispatchInsertMatrix(bv_, argument);
		break;

	case LFUN_INSERT_MATH:
		mathDispatchInsertMath(bv_, argument);
		break;

	case LFUN_MATH_IMPORT_SELECTION: // Imports LaTeX from the X selection
		mathDispatchMathImportSelection(bv_, argument);
		break;

	case LFUN_MATH_DISPLAY:          // Open or create a displayed math inset
		mathDispatchMathDisplay(bv_, argument);
		break;

	case LFUN_MATH_MODE:             // Open or create an inlined math inset
		mathDispatchMathMode(bv_, argument);
		break;

	case LFUN_GREEK:                 // Insert a single greek letter
		mathDispatchGreek(bv_, argument);
		break;

	case LFUN_CITATION_INSERT:
	{
		InsetCommandParams p;
		p.setFromString(argument);

		InsetCitation * inset = new InsetCitation(p);
		if (!insertInset(inset))
			delete inset;
		else
			updateInset(inset, true);
	}
	break;

	case LFUN_INSERT_BIBTEX:
	{
		// ale970405+lasgoutt970425
		// The argument can be up to two tokens separated
		// by a space. The first one is the bibstyle.
		string const db       = token(argument, ' ', 0);
		string const bibstyle = token(argument, ' ', 1);

		InsetCommandParams p("BibTeX", db, bibstyle);
		InsetBibtex * inset = new InsetBibtex(p);

		if (insertInset(inset)) {
			if (argument.empty())
				inset->edit(bv_);
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
		InsetCommandParams p("index");
		if (argument.empty()) {
			string const idxstring(bv_->getLyXText()->getStringToIndex(bv_));
			p.setContents(idxstring);
		} else {
			p.setContents(argument);
		}

		owner_->getDialogs()->createIndex(p.getAsString());
	}
	break;

	case LFUN_INDEX_INSERT:
	{
		InsetCommandParams p;
		p.setFromString(argument);
		InsetIndex * inset = new InsetIndex(p);

		if (!insertInset(inset))
			delete inset;
		else
			updateInset(inset, true);
	}
	break;

	case LFUN_INDEX_INSERT_LAST:
	{
		string const idxstring(bv_->getLyXText()->getStringToIndex(bv_));
		if (!idxstring.empty()) {
			owner_->message(_("Word `")
					+ idxstring + _(("' indexed.")));
			InsetCommandParams p("index", idxstring);
			InsetIndex * inset = new InsetIndex(p);

			if (!insertInset(inset))
				delete inset;
			else
				updateInset(inset, true);
		}
	}
	break;

	case LFUN_INDEX_PRINT:
	{
		InsetCommandParams p("printindex");
		Inset * inset = new InsetPrintIndex(p);
		if (!insertInset(inset, "Standard"))
			delete inset;
	}
	break;

	case LFUN_PARENTINSERT:
	{
		InsetCommandParams p("lyxparent", argument);
		Inset * inset = new InsetParent(p, *buffer_);
		if (!insertInset(inset, "Standard"))
			delete inset;
	}

	break;

	case LFUN_CHILD_INSERT:
	{
		InsetInclude::Params p;
		p.cparams.setFromString(argument);
		p.masterFilename_ = buffer_->fileName();

		InsetInclude * inset = new InsetInclude(p);
		if (!insertInset(inset))
			delete inset;
		else {
			updateInset(inset, true);
			bv_->owner()->getDialogs()->showInclude(inset);
		}
	}
	break;

	case LFUN_FLOAT_LIST:
	{
		// We should check the argument for validity. (Lgb)
		Inset * inset = new InsetFloatList(argument);
		if (!insertInset(inset, "Standard"))
			delete inset;
	}
	break;

	case LFUN_THESAURUS_ENTRY:
	{
		string arg = argument;

		if (arg.empty()) {
			arg = bv_->getLyXText()->selectionAsString(buffer_,
								   false);

			// FIXME
			if (arg.size() > 100 || arg.empty()) {
				// Get word or selection
				bv_->getLyXText()->selectWordWhenUnderCursor(bv_, LyXText::WHOLE_WORD);
				arg = bv_->getLyXText()->selectionAsString(buffer_, false);
				// FIXME: where is getLyXText()->unselect(bv_) ?
			}
		}

		bv_->owner()->getDialogs()->showThesaurus(arg);
	}
		break;

	case LFUN_SELFINSERT:
	{
		if (argument.empty()) break;

		/* Automatically delete the currently selected
		 * text and replace it with what is being
		 * typed in now. Depends on lyxrc settings
		 * "auto_region_delete", which defaults to
		 * true (on). */

		LyXText * lt = bv_->getLyXText();

		if (lyxrc.auto_region_delete) {
			if (lt->selection.set()) {
				lt->cutSelection(bv_, false, false);
				bv_->update(lt,
					    BufferView::SELECT
					    | BufferView::FITCUR
					    | BufferView::CHANGE);
			}
			workarea_.haveSelection(false);
		}

		beforeChange(lt);
		LyXFont const old_font(lt->real_current_font);

		string::const_iterator cit = argument.begin();
		string::const_iterator end = argument.end();
		for (; cit != end; ++cit) {
			owner_->getIntl()->getTrans().TranslateAndInsert(*cit, lt);
		}

		bv_->update(lt,
			    BufferView::SELECT
			    | BufferView::FITCUR
			    | BufferView::CHANGE);

		lt->selection.cursor = lt->cursor;
		moveCursorUpdate(false);

		// real_current_font.number can change so we need to
		// update the minibuffer
		if (old_font != lt->real_current_font)
			owner_->showState();
		//return string();
	}
	break;

	case LFUN_DATE_INSERT:  // jdblair: date-insert cmd
	{
		time_t now_time_t = time(NULL);
		struct tm * now_tm = localtime(&now_time_t);
		setlocale(LC_TIME, "");
		string arg;
		if (!argument.empty())
			arg = argument;
		else
			arg = lyxrc.date_insert_format;
		char datetmp[32];
		int const datetmp_len =
			::strftime(datetmp, 32, arg.c_str(), now_tm);

		LyXText * lt = bv_->getLyXText();

		for (int i = 0; i < datetmp_len; i++) {
			lt->insertChar(bv_, datetmp[i]);
			update(lt,
			       BufferView::SELECT
			       | BufferView::FITCUR
			       | BufferView::CHANGE);
		}

		lt->selection.cursor = lt->cursor;
		moveCursorUpdate(false);
	}
	break;

	case LFUN_UNKNOWN_ACTION:
		owner_->getLyXFunc()->setErrorMessage(N_("Unknown function!"));
		break;

	default:
		return false;
	} // end of switch

	return true;
}


void BufferView::Pimpl::newline()
{
	if (available()) {
		LyXText * lt = bv_->getLyXText();
		hideCursor();
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR);
		lt->insertChar(bv_, Paragraph::META_NEWLINE);
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
	}
}


void BufferView::Pimpl::hfill()
{
	if (available()) {
		LyXText * lt = bv_->getLyXText();
		hideCursor();
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR);
		lt->insertChar(bv_, Paragraph::META_HFILL);
		update(lt,
		       BufferView::SELECT
		       | BufferView::FITCUR
		       | BufferView::CHANGE);
	}
}


void BufferView::Pimpl::specialChar(InsetSpecialChar::Kind kind)
{
	if (available()) {
		LyXText * lt = bv_->getLyXText();

		hideCursor();
		update(lt, BufferView::SELECT|BufferView::FITCUR);
		InsetSpecialChar * new_inset =
			new InsetSpecialChar(kind);
		if (!insertInset(new_inset))
			delete new_inset;
		else
			updateInset(new_inset, true);
	}
}


void BufferView::Pimpl::smartQuote()
{
	LyXText const * lt = bv_->getLyXText();
	Paragraph const * par = lt->cursor.par();
	pos_type pos = lt->cursor.pos();
	char c;

	if (!pos
	    || (par->isInset(pos - 1)
		&& par->getInset(pos - 1)->isSpace()))
		c = ' ';
	else
		c = par->getChar(pos - 1);


	hideCursor();

	LyXLayout const & style =
		textclasslist[bv_->buffer()->params.textclass][par->layout()];

	if (style.pass_thru ||
		(!insertInset(new InsetQuotes(c, bv_->buffer()->params))))
		bv_->owner()->getLyXFunc()->dispatch(LFUN_SELFINSERT, "\"");
}


void BufferView::Pimpl::insertAndEditInset(Inset * inset)
{
	if (insertInset(inset))
		inset->edit(bv_);
	else
		delete inset;
}


// Open and lock an updatable inset
bool BufferView::Pimpl::open_new_inset(UpdatableInset * new_inset, bool behind)
{
	LyXText * lt = bv_->getLyXText();

	beforeChange(lt);
	finishUndo();
	if (!insertInset(new_inset)) {
		delete new_inset;
		return false;
	}
	new_inset->edit(bv_, !behind);
	return true;
}


bool BufferView::Pimpl::insertInset(Inset * inset, string const & lout)
{
	// if we are in a locking inset we should try to insert the
	// inset there otherwise this is a illegal function now
	if (bv_->theLockingInset()) {
		if (bv_->theLockingInset()->insetAllowed(inset))
		    return bv_->theLockingInset()->insertInset(bv_, inset);
		return false;
	}

	// not quite sure if we want this...
	setCursorParUndo(bv_);
	freezeUndo();

	beforeChange(bv_->text);
	if (!lout.empty()) {
		update(bv_->text, BufferView::SELECT|BufferView::FITCUR);
		bv_->text->breakParagraph(bv_);
		update(bv_->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);

		if (bv_->text->cursor.par()->size()) {
			bv_->text->cursorLeft(bv_);

			bv_->text->breakParagraph(bv_);
			update(bv_->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
		}

		string lres = lout;
		LyXTextClass const & tclass =
			textclasslist[buffer_->params.textclass];
		bool hasLayout = tclass.hasLayout(lres);
		string lay = tclass.defaultLayoutName();

		if (hasLayout != false) {
			// layout found
			lay = lres;
		} else {
			// layout not fount using default
			lay = tclass.defaultLayoutName();
		}

		bv_->text->setLayout(bv_, lay);

		bv_->text->setParagraph(bv_, 0, 0,
				   0, 0,
				   VSpace(VSpace::NONE), VSpace(VSpace::NONE),
				   Spacing(),
				   LYX_ALIGN_LAYOUT,
				   string(),
				   0);
		update(bv_->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);
	}

	bv_->text->insertInset(bv_, inset);
	update(bv_->text, BufferView::SELECT|BufferView::FITCUR|BufferView::CHANGE);

	unFreezeUndo();
	return true;
}


void BufferView::Pimpl::updateInset(Inset * inset, bool mark_dirty)
{
	if (!inset || !available())
		return;

	// first check for locking insets
	if (bv_->theLockingInset()) {
		if (bv_->theLockingInset() == inset) {
			if (bv_->text->updateInset(bv_, inset)) {
				update();
				if (mark_dirty) {
					buffer_->markDirty();
				}
				updateScrollbar();
				return;
			}
		} else if (bv_->theLockingInset()->updateInsetInInset(bv_, inset)) {
			if (bv_->text->updateInset(bv_,  bv_->theLockingInset())) {
				update();
				if (mark_dirty) {
					buffer_->markDirty();
				}
				updateScrollbar();
				return;
			}
		}
	}

	// then check if the inset is a top_level inset (has no owner)
	// if yes do the update as always otherwise we have to update the
	// toplevel inset where this inset is inside
	Inset * tl_inset = inset;
	while(tl_inset->owner())
		tl_inset = tl_inset->owner();
	hideCursor();
	if (tl_inset == inset) {
		update(bv_->text, BufferView::UPDATE);
		if (bv_->text->updateInset(bv_, inset)) {
			if (mark_dirty) {
				update(bv_->text,
				       BufferView::SELECT
				       | BufferView::FITCUR
				       | BufferView::CHANGE);
			} else {
				update(bv_->text, SELECT);
			}
			return;
		}
	} else if (static_cast<UpdatableInset *>(tl_inset)
			   ->updateInsetInInset(bv_, inset))
	{
			if (bv_->text->updateInset(bv_,  tl_inset)) {
				update();
				updateScrollbar();
			}
	}
}


void BufferView::Pimpl::gotoInset(vector<Inset::Code> const & codes,
				  bool same_content)
{
	if (!available()) return;

	hideCursor();
	beforeChange(bv_->text);
	update(bv_->text, BufferView::SELECT|BufferView::FITCUR);

	LyXCursor const & cursor = bv_->text->cursor;

	string contents;
	if (same_content &&
	    cursor.par()->isInset(cursor.pos())) {
		Inset const * inset = cursor.par()->getInset(cursor.pos());
		if (find(codes.begin(), codes.end(), inset->lyxCode())
		    != codes.end())
			contents =
				static_cast<InsetCommand const *>(inset)->getContents();
	}


	if (!bv_->text->gotoNextInset(bv_, codes, contents)) {
		if (bv_->text->cursor.pos()
		    || bv_->text->cursor.par() != bv_->text->ownerParagraph()) {
			LyXCursor tmp = bv_->text->cursor;
			bv_->text->cursor.par(bv_->text->ownerParagraph());
			bv_->text->cursor.pos(0);
			if (!bv_->text->gotoNextInset(bv_, codes, contents)) {
				bv_->text->cursor = tmp;
				bv_->owner()->message(_("No more insets"));
			}
		} else {
			bv_->owner()->message(_("No more insets"));
		}
	}
	update(bv_->text, BufferView::SELECT|BufferView::FITCUR);
	bv_->text->selection.cursor = bv_->text->cursor;
}


void BufferView::Pimpl::gotoInset(Inset::Code code, bool same_content)
{
	gotoInset(vector<Inset::Code>(1, code), same_content);
}
