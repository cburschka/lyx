// -*- C++ -*-

#ifndef BUFFERVIEW_PIMPL_H
#define BUFFERVIEW_PIMPL_H

#include "BufferView.h"
#include "UpdateInset.h"
#include "BackStack.h"
#include "Timeout.h"

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;
class WorkArea;
class LyXScreen;

#ifdef SIGC_CXX_NAMESPACES
using SigC::Object;
#endif

///
struct BufferView::Pimpl : public Object {
	///
	Pimpl(BufferView * i, LyXView * o,
	      int xpos, int ypos, int width, int height);
	///
	Painter & painter();
	///
	void buffer(Buffer *);
	///
	void resize(int xpos, int ypos, int width, int height);
	///
	void resize();
	///
	void redraw();
	/// Return true if the cursor was fitted.
	bool fitCursor();
	///
	void redoCurrentBuffer();
	///
	int resizeCurrentBuffer();
	///
	void update();
	//
	void update(BufferView::UpdateCodes);
	///
	void gotoError();
  	/// Update pixmap of screen
	void updateScreen();
	///
	void workAreaExpose();
	///
	void updateScrollbar();
	///
	void scrollCB(double value);
	///
	Inset * checkInsetHit(LyXText *, int & x, int & y,
			      unsigned int button);
	/// 
	int scrollUp(long time);
	///
	int scrollDown(long time);
	///
	void workAreaKeyPress(KeySym, unsigned int state);
	///
	void workAreaMotionNotify(int x, int y, unsigned int state);
	///
	void workAreaButtonPress(int x, int y, unsigned int button);
	///
	void workAreaButtonRelease(int x, int y, unsigned int button);
	///
	void doubleClick(int x, int y, unsigned int button);
	///
	void tripleClick(int x, int y, unsigned int button);
	///
	void enterView();
	///
	void leaveView();
	///
	void cursorToggle();
	///
	void cursorPrevious();
	///
	void cursorNext();
	///
	bool available() const;
	///
	void beforeChange();
	///
	void savePosition();
	///
	void restorePosition();
	///
	bool NoSavedPositions();
	///
	void setState();
	///
	void insetSleep();
	///
	void insetWakeup();
	///
	void insetUnlock();
	///
	bool focus() const;
	///
	void focus(bool);
	///
	bool active() const;
	///
	bool belowMouse() const;
	///
	void showCursor();
	///
	void hideCursor();
	///
	void toggleSelection(bool = true);
	///
	void toggleToggle();
	///
	void center();
	///
	BufferView * bv_;
	///
	LyXView * owner_;
	///
	Buffer * buffer_;
	///
	LyXScreen * screen_;
	///
	long current_scrollbar_value;
	///
	FL_OBJECT * figinset_canvas;
	///
	Timeout cursor_timeout;
        ///
        BackStack backstack;
	///
	int last_click_x, last_click_y;
	///
	WorkArea * workarea_;
	///
	UpdateInset updatelist;
	///
	void pasteClipboard(bool asPara);
	///
	void stuffClipboard(string const &) const;
private:
	///
	bool using_xterm_cursor;
};
#endif
