// -*- C++ -*-

#ifndef BUFFERVIEW_PIMPL_H
#define BUFFERVIEW_PIMPL_H

#include "BufferView.h"
#include "UpdateInset.h"
#include "BackStack.h"

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;
class WorkArea;
class LyXScreen;

struct BufferView::Pimpl {
	Pimpl(BufferView * i, LyXView * o,
	      int xpos, int ypos, int width, int height);
	
  	/// Update pixmap of screen
	void updateScreen();
	///
	void workAreaExpose();
	///
	void scrollUpOnePage();
	///
	void scrollDownOnePage();
	///
	void create_view();
	///
	Inset * checkInsetHit(int & x, int & y, unsigned int button);
	/// 
	int scrollUp(long time);
	///
	int scrollDown(long time);
	///
	void workAreaMotionNotify(int x, int y, unsigned int state);
	///
	void workAreaButtonPress(int x, int y, unsigned int button);
	///
	void workAreaButtonRelease(int x, int y, unsigned int button);
	///
	void workAreaSelectionNotify(Window win, XEvent * event);
	///
	void doubleClick(int x, int y, unsigned int button);
	///
	void tripleClick(int x, int y, unsigned int button);
	///
	BufferView * bv_;
	///
	LyXView * owner_;
	///
	Buffer * buffer_;
	///
	LyXScreen * screen;
	///
	long current_scrollbar_value;
	///
	bool lyx_focus;
	///
	bool work_area_focus;
	///
	FL_OBJECT * figinset_canvas;
	///
	FL_OBJECT * timer_cursor;
        ///
        BackStack backstack;
	///
	int last_click_x, last_click_y;
	///
	WorkArea * workarea;
	///
	UpdateInset updatelist;
};
#endif
