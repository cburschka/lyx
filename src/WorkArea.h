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
using std::pair;

#include FORMS_H_LOCATION
#include "Painter.h"

class BufferView;

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
	int width() const { return work_area->w + scrollbar->w; }
	///
	int height() const { return work_area->h; }
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
		fl_redraw_object(button_down);
		fl_redraw_object(button_up);
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
	void setScrollbarValue(double y) const { fl_set_slider_value(scrollbar, y); }
	///
	void setScrollbarBounds(double, double) const;
	///
	void setScrollbarIncrements(float inc) const;
	///
	double getScrollbarValue() const {
		return fl_get_slider_value(scrollbar);
	}
	///
	pair<double, double> getScrollbarBounds() const {
		pair<double, double> p;
		fl_get_slider_bounds(scrollbar, &p.first, &p.second);
		return p;
	}
	///
	Pixmap getPixmap() const { return workareapixmap; }
	/// xforms callback
	static int work_area_handler(FL_OBJECT *, int event,
				     FL_Coord, FL_Coord,
				     int /*key*/, void * xev);
	/// xforms callback
	static void up_cb(FL_OBJECT *, long);
	/// xforms callback
	static void down_cb(FL_OBJECT *, long);
	/// xforms callback
	static void scroll_cb(FL_OBJECT *, long);
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
	FL_OBJECT * button_down;
	///
	FL_OBJECT * button_up;
	///
	BufferView * owner;
	/// The pixmap overlay on the workarea
	Pixmap workareapixmap;
	///
	Painter painter_;
	///
	FL_OBJECT * figinset_canvas;
};
#endif
