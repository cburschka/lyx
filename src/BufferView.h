// -*- C++ -*-
/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
*        
*           Copyright (C) 1995 Matthias Ettrich
*           Copyright (C) 1995-1998 The LyX Team.
*
*======================================================*/

#ifndef _BUFFER_VIEW_H
#define _BUFFER_VIEW_H

#ifdef __GNUG__
#pragma interface
#endif

#include FORMS_H_LOCATION

class LyXView;
class Buffer;
class LyXScreen;
class BackStack;
class Inset;

///
class BufferView {
public:
	///
	BufferView(LyXView *owner, int ,int ,int, int);
        ///
        ~BufferView();
	///
	Buffer *currentBuffer() { return _buffer; }
	///
	FL_OBJECT *getWorkArea() { return work_area; }
	///
	void setBuffer(Buffer *b);
	///
	void resize();
	///
	void redraw();
	///
	void fitCursor();
	///
	void update();
	///
	void updateScrollbar();
	///
	void redoCurrentBuffer();
	///
	int resizeCurrentBuffer();
	///
	void gotoError();
	///
	void cursorPrevious();
	///
	void cursorNext();
	/// 
	bool available() const;
	///
	LyXView *getOwner() { return _owner; }
	///
	LyXScreen *getScreen()
	{
		fl_set_timer(timer_cursor, 0.4);
		return screen;
	}
        ///
        void savePosition();
        ///
        void restorePosition();
private:
	/// Update pixmap of screen
	void updateScreen();
	///
	int workAreaExpose();
	///
	void create_view(int, int, int, int);
	///
	Inset * checkInsetHit(int &x, int &y);
	/// 
	int ScrollUp(long time);
	///
	int ScrollDown(long time);
	///
	void ScrollUpOnePage(long /*time*/);
	///
	void ScrollDownOnePage(long /*time*/);

public:
	/// A callback for the up arrow in the scrollbar.
	static void UpCB(FL_OBJECT *ob, long);

	/// A callback for the slider in the scrollbar.
	static void ScrollCB(FL_OBJECT *ob, long);

	/// A callback for the down arrow in the scrollbar.
	static void DownCB(FL_OBJECT *ob, long);

	///
	static void CursorToggleCB(FL_OBJECT *ob, long);
	/** Work area free object handler
	 */
	static int work_area_handler(FL_OBJECT *, int event,
				     FL_Coord, FL_Coord, int key, void *xev);
private:
	///
	int WorkAreaMotionNotify(FL_OBJECT *ob,
				 Window win,
				 int w, int h,
				 XEvent *ev, void *d);
	///
	int WorkAreaSelectionNotify(FL_OBJECT *, Window win,
				    int /*w*/, int /*h*/,
				    XEvent *event, void */*d*/);
	///
	int WorkAreaButtonPress(FL_OBJECT *ob,
				Window win,
				int w, int h,
				XEvent *ev, void *d);
	///
	int WorkAreaButtonRelease(FL_OBJECT *ob,
				  Window win,
				  int w, int h,
				  XEvent *ev, void *d);
	///
	LyXView *_owner;
	///
	Buffer *_buffer;
	///
	LyXScreen *screen;
	///
	long current_scrollbar_value;
	///
	int work_area_width;
	///
	bool lyx_focus;
	///
	bool work_area_focus;
	///
	FL_OBJECT *work_area;
	///
	FL_OBJECT *figinset_canvas;
	///
	FL_OBJECT *scrollbar;
	///
	FL_OBJECT *button_down;
	///
	FL_OBJECT *button_up;
	///
	FL_OBJECT *timer_cursor;
        ///
        BackStack *backstack;
	///
	int last_click_x, last_click_y;
};

#endif
