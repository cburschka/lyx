// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ======================================================*/

#ifndef WORKAREA_H
#define WORKAREA_H

#ifdef __GNUG__
#pragma interface
#endif

#include <utility>

#include <sigc++/signal_system.h>

#include FORMS_H_LOCATION
#include "Painter.h"

///
class WorkArea {
public:
	///
	WorkArea(int xpos, int ypos, int width, int height);
	///
	~WorkArea();
	///
	Painter & getPainter() { return painter_; }
	///
	int workWidth() const { return work_area->w; }
	///
	unsigned int width() const { return work_area->w + scrollbar->w; }
	//unsigned int width() const { return backgroundbox->w + 15; }
	///
	unsigned int height() const { return work_area->h; }
	//unsigned int height() const { return backgroundbox->h; }
	///
	int xpos() const { return work_area->x; }
	//int xpos() const { return backgroundbox->x; }
	///
	int ypos() const { return work_area->y; }
	//int ypos() const { return backgroundbox->y; }
	///
	void resize(int xpos, int ypos, int width, int height);
	///
	void redraw() const {
		fl_redraw_object(work_area);
		fl_redraw_object(scrollbar);
	}
	///
	void setFocus() const;
	///
	Window getWin() const { return work_area->form->window; }
	///
	bool hasFocus() const { return work_area->focus; }
	///
	bool active() const { return work_area->active; }
	///
	bool belowMouse() const;
	///
	bool visible() const { return work_area->form->visible; }
	///
	void greyOut() const;
	///
	void setScrollbar(double pos, double length_fraction) const;
	///
	void setScrollbarValue(double y) const {
		fl_set_scrollbar_value(scrollbar, y);
	}
	///
	void setScrollbarBounds(double, double) const;
	///
	void setScrollbarIncrements(double inc) const;
	///
	double getScrollbarValue() const {
		return fl_get_scrollbar_value(scrollbar);
	}
	///
	std::pair<float, float> const getScrollbarBounds() const {
		std::pair<float, float> p;
		fl_get_scrollbar_bounds(scrollbar, &p.first, &p.second);
		return p;
	}
	///
	Pixmap getPixmap() const { return workareapixmap; }
	/// xforms callback
	static int work_area_handler(FL_OBJECT *, int event,
				     FL_Coord, FL_Coord,
				     int /*key*/, void * xev);
	/// xforms callback
	static void scroll_cb(FL_OBJECT *, long);
	/// a selection exists
	void haveSelection(bool) const;
	///
	string const getClipboard() const;
	///
	void putClipboard(string const &) const;
	// Signals
	///
	SigC::Signal0<void> workAreaExpose;
	///
	SigC::Signal1<void, double> scrollCB;
	///
	SigC::Signal2<void, KeySym, unsigned int> workAreaKeyPress;
	///
	SigC::Signal3<void, int, int, unsigned int> workAreaButtonPress;
	///
	SigC::Signal3<void, int, int, unsigned int> workAreaButtonRelease;
	///
	SigC::Signal3<void, int, int, unsigned int> workAreaMotionNotify;
	///
	SigC::Signal0<void> workAreaFocus;
	///
	SigC::Signal0<void> workAreaUnfocus;
	///
	SigC::Signal0<void> workAreaEnter;
	///
	SigC::Signal0<void> workAreaLeave;
	///
	SigC::Signal3<void, int, int, unsigned int> workAreaDoubleClick;
	///
	SigC::Signal3<void, int, int, unsigned int> workAreaTripleClick;
	/// emitted when an X client has requested our selection
	SigC::Signal0<void> selectionRequested;
 
	/// handles SelectionRequest X Event, to fill the clipboard
	void event_cb(XEvent * xev);
private:
	///
	void createPixmap(int, int);
 
	///
	FL_OBJECT * backgroundbox;
	///	
	FL_OBJECT * work_area;
	///
	FL_OBJECT * scrollbar;
	///
	mutable FL_OBJECT * splash_;
	///
	mutable FL_OBJECT * splash_text_;
	/// The pixmap overlay on the workarea
	Pixmap workareapixmap;
	///
	Painter painter_;
	///
	FL_OBJECT * figinset_canvas;
	/// if we call redraw with true needed for locking-insets
	bool screen_cleared;
};
#endif
