// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
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

#ifdef SIGC_CXX_NAMESPACES
using SigC::Signal0;
using SigC::Signal2;
using SigC::Signal3;
#endif

class BufferView;

///
class WorkArea {
public:
	///
	WorkArea(BufferView *, int xpos, int ypos, int width, int height);
	///
	~WorkArea();
	///
	Painter & getPainter() { return painter_; }
	///
	int workWidth() const { return work_area->w; }
	///
	unsigned int width() const { return work_area->w + scrollbar->w; }
	///
	unsigned int height() const { return work_area->h; }
	///
	int xpos() const { return work_area->x; }
	///
	int ypos() const { return work_area->y; }
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
	///
	string const getClipboard() const;
	///
	void putClipboard(string const &) const;
	///
	BufferView * owner() const { return owner_; }

	// Signals
	///
	Signal0<void> workAreaExpose;
	///
	Signal2<void, KeySym, unsigned int> workAreaKeyPress;
	///
	Signal3<void, int, int, unsigned int> workAreaButtonPress;
	///
	Signal3<void, int, int, unsigned int> workAreaButtonRelease;
	///
	Signal3<void, int, int, unsigned int> workAreaMotionNotify;
	///
	Signal0<void> workAreaFocus;
	///
	Signal0<void> workAreaUnfocus;
	///
	Signal0<void> workAreaEnter;
	///
	Signal0<void> workAreaLeave;
	///
	Signal3<void, int, int, unsigned int> workAreaDoubleClick;
	///
	Signal3<void, int, int, unsigned int> workAreaTripleClick;
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
	BufferView * owner_;
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
