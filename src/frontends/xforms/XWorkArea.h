// -*- C++ -*-
/**
 * \file XWorkArea.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

#ifndef XWORKAREA_H
#define XWORKAREA_H

#ifdef __GNUG__
#pragma interface
#endif

#include "frontends/WorkArea.h"
#include "XPainter.h"

#include FORMS_H_LOCATION

///
class XWorkArea : public WorkArea {
public:
	///
	XWorkArea(int xpos, int ypos, int width, int height);
	///
	~XWorkArea();
	///
	virtual Painter & getPainter() { return painter_; }
	///
	virtual int workWidth() const { return work_area->w; }
	///
	virtual int workHeight() const { return work_area->h; }
	///
	virtual void redraw() const {
		fl_redraw_object(work_area);
		fl_redraw_object(scrollbar);
	}
	///
	Window getWin() const { return work_area->form->window; }
        ///
	virtual void setScrollbarParams(int height, int pos, int line_height);
	///
	Pixmap getPixmap() const { return workareapixmap; }
	/// xforms callback
	static int work_area_handler(FL_OBJECT *, int event,
				     FL_Coord, FL_Coord,
				     int /*key*/, void * xev);
 
	/// return x position of window
	int xpos() const { return work_area->x; }
	/// return y position of window
	int ypos() const { return work_area->y; }
 
	/// xforms callback from scrollbar
	void scroll_cb();
	/// a selection exists
	virtual void haveSelection(bool) const;
	///
	virtual string const getClipboard() const;
	///
	virtual void putClipboard(string const &) const;

	/// handles SelectionRequest X Event, to fill the clipboard
	int event_cb(XEvent * xev);
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
	XPainter painter_;
	/// if we call redraw with true needed for locking-insets
	bool screen_cleared;
	/// the current document's height (for scrollbar)
	int doc_height_;
};
 
#endif // XWORKAREA_H
