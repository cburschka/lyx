// -*- C++ -*-
/**
 * \file XWorkArea.h
 * Read the file COPYING
 *
 * \author unknown
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
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
	/// generate the pixmap, and copy backing pixmap to it,
	/// and send resize event if needed
	void redraw(int, int);

	/// GC used for copying to the screen
	GC copy_gc;
 
	///
	FL_OBJECT * backgroundbox;
	/// the workarea free object
	FL_OBJECT * work_area;
	/// the scrollbar objcet
	FL_OBJECT * scrollbar;
	/// The pixmap overlay on the workarea
	Pixmap workareapixmap;
	/// the xforms-specific painter
	XPainter painter_;
	/// if we call redraw with true needed for locking-insets
	bool screen_cleared;
	/// the current document's height (for scrollbar)
	int doc_height_;
};

#endif // XWORKAREA_H
