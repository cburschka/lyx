/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 * ====================================================== */

#include <config.h>
#include <cmath>

#ifdef __GNUG__
#pragma implementation
#endif

#include "WorkArea.h"
#include "debug.h"
#include "support/lstrings.h"
#include "BufferView.h"

using std::endl;

FL_OBJECT * figinset_canvas;

// need to make the c++ compiler find the correct version of abs.
// This is at least true for g++.
using std::abs;

static inline
void waitForX()
{
	XSync(fl_get_display(), 0);
}

extern "C" {
// Just a bunch of C wrappers around static members of WorkArea
	void C_WorkArea_scroll_cb(FL_OBJECT * ob, long buf)
        {
		WorkArea::scroll_cb(ob, buf);
        }

	int C_WorkArea_work_area_handler(FL_OBJECT * ob, int event,
                                           FL_Coord, FL_Coord, 
                                           int key, void * xev)
        {
		return WorkArea::work_area_handler(ob, event,
						   0, 0, key, xev);
        }
}



WorkArea::WorkArea(BufferView * o, int xpos, int ypos, int width, int height)
	: owner_(o), workareapixmap(0), painter_(*this)
{
	fl_freeze_all_forms();

	figinset_canvas = 0;

	if (lyxerr.debugging())
		lyxerr << "Creating work area: +"
		       << xpos << '+' << ypos << ' '
		       << width << 'x' << height << endl;
	//
	FL_OBJECT * obj;
	const int bw = int(std::abs(float(fl_get_border_width())));

	// We really want to get rid of figinset_canvas.
	::figinset_canvas = figinset_canvas = obj =
		  fl_add_canvas(FL_NORMAL_CANVAS,
				xpos + 1, ypos + 1, 1, 1, "");
	fl_set_object_boxtype(obj, FL_NO_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, NorthWestGravity);
	
	// a box
	if (lyxerr.debugging())
		lyxerr << "\tbackground box: +"
		       << xpos << '+' << ypos << ' '
		       << width - 15 << 'x' << height << endl;
	backgroundbox = obj = fl_add_box(FL_BORDER_BOX,
					 xpos, ypos,
					 width - 15,
					 height,"");
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);

	//
	// THE SCROLLBAR
	//

	scrollbar = obj = fl_add_scrollbar(FL_VERT_SCROLLBAR,
					   xpos + width - 15,
					   ypos, 17, height, "");
	fl_set_object_boxtype(obj, FL_UP_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthEastGravity, SouthEastGravity);
	obj->u_vdata = this;
	fl_set_object_callback(obj, C_WorkArea_scroll_cb, 0);

	///
	/// The free object

	// Create the workarea pixmap
	createPixmap(width - 15 - 2 * bw, height - 2 * bw);

	// We add this object as late as possible to avoit problems
	// with drawing.
	if (lyxerr.debugging())
		lyxerr << "\tfree object: +"
		       << xpos + bw << '+' << ypos + bw << ' '
		       << width - 15 - 2 * bw << 'x'
		       << height - 2 * bw << endl;
	
	work_area = obj = fl_add_free(FL_INPUT_FREE,
				      xpos + bw, ypos + bw,
				      width - 15 - 2 * bw, // scrollbarwidth
				      height - 2 * bw, "",
				      C_WorkArea_work_area_handler);
	obj->wantkey = FL_KEY_TAB;
	obj->u_vdata = this; /* This is how we pass the WorkArea
				       to the work_area_handler. */
	fl_set_object_boxtype(obj,FL_DOWN_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);

	fl_unfreeze_all_forms();
}


WorkArea::~WorkArea()
{
	if (workareapixmap)
		XFreePixmap(fl_display, workareapixmap);
}


bool WorkArea::belowMouse() const
{
	FL_Coord x, y;
	unsigned int button;
	fl_get_mouse(&x, &y, &button);
	FL_Coord ulx = work_area->form->x + work_area->x;
	FL_Coord uly = work_area->form->y + work_area->y;
	FL_Coord w = work_area->w;
	FL_Coord h = work_area->h;
	if (x > ulx && y > uly && x < ulx + h && y < uly + w)
		return true;
	return false;
	
	
	//lyxerr << "Mouse: (" << x << ", " << y <<") button = " << button << endl;
	//lyxerr << "Workarea: (" << work_area->x + work_area->form->x << ", " << work_area->y + work_area->form->y << ", " << work_area->w << ", " << work_area->h << ")" << endl;
	//lyxerr << "Below mouse: " << work_area->belowmouse << endl;
	//return work_area->belowmouse;
}


void WorkArea::resize(int xpos, int ypos, int width, int height)
{
	fl_freeze_all_forms();
	
	const int bw = int(std::abs(float(fl_get_border_width())));

	// a box
	fl_set_object_geometry(backgroundbox, xpos, ypos, width - 15, height);
	
	//
	// THE SCROLLBAR
	//
	fl_set_object_geometry(scrollbar, xpos + width - 15,
			       ypos, 17, height);

	// Create the workarea pixmap
	createPixmap(width - 15 - 2 * bw, height - 2 * bw);

	// the free object
	fl_set_object_geometry(work_area, xpos + bw, ypos + bw,
			       width - 15 - 2 * bw,
			       height - 2 * bw);

	fl_unfreeze_all_forms();

}


void WorkArea::createPixmap(int width, int height)
{
	static int cur_width = -1;
	static int cur_height = -1;

	if (cur_width == width && cur_height == height && workareapixmap)
		return;
	
	cur_width = width;
	cur_height = height;

	if (workareapixmap)
		XFreePixmap(fl_display, workareapixmap);

	if (lyxerr.debugging())
		lyxerr << "Creating pixmap ("
		       << width << 'x' << height << ")" << endl;
	
	workareapixmap = XCreatePixmap(fl_display,
				       RootWindow(fl_display, 0),
				       width,
				       height, 
				       fl_get_visual_depth());
	if (lyxerr.debugging())
		lyxerr << "\tpixmap=" << workareapixmap << endl;
}


void WorkArea::greyOut() const
{
	fl_winset(FL_ObjWin(work_area));
	fl_rectangle(1, work_area->x, work_area->y,
		     work_area->w, work_area->h, FL_GRAY63);
}


void WorkArea::setFocus() const
{
	fl_set_focus_object(work_area->form, work_area);
}


void WorkArea::setScrollbar(double pos, double length_fraction) const
{
	fl_set_scrollbar_value(scrollbar, pos);
	fl_set_scrollbar_size(scrollbar, scrollbar->h * length_fraction);
}


void WorkArea::setScrollbarBounds(double l1, double l2) const
{
	fl_set_scrollbar_bounds(scrollbar, l1, l2);
}


void WorkArea::setScrollbarIncrements(double inc) const
{
	fl_set_scrollbar_increment(scrollbar, work_area->h - inc, inc);
}


// Callback for scrollbar slider
void WorkArea::scroll_cb(FL_OBJECT * ob, long)
{
	WorkArea * area = static_cast<WorkArea*>(ob->u_vdata);
	// If we really want the accellerating scroll we can do that
	// from here. IMHO that is a waste of effort since we already
	// have other ways to move fast around in the document. (Lgb)
	area->owner_->scrollCB(fl_get_scrollbar_value(ob));
	waitForX();
}

bool Lgb_bug_find_hack = false;

int WorkArea::work_area_handler(FL_OBJECT * ob, int event,
				FL_Coord, FL_Coord ,
				int /*key*/, void * xev)
{
	static int x_old = -1;
	static int y_old = -1;
	static long scrollbar_value_old = -1;
	
	XEvent * ev = static_cast<XEvent*>(xev);
	WorkArea * area = static_cast<WorkArea*>(ob->u_vdata);

	if (!area) return 1;
	
	switch (event){
	case FL_DRAW:
		if (!area->work_area ||
		    !area->work_area->form->visible)
			return 1;
		lyxerr.debug() << "Workarea event: DRAW" << endl;
		area->createPixmap(area->workWidth(), area->height());
		Lgb_bug_find_hack = true;
		//workAreaExpose();
		area->owner_->workAreaExpose();
		Lgb_bug_find_hack = false;
		break;
	case FL_PUSH:
		if (!ev) break;
		// Should really have used xbutton.state
		lyxerr.debug() << "Workarea event: PUSH" << endl;
		//workAreaButtonPress(ev->xbutton.x - ob->x,
		//		    ev->xbutton.y - ob->y,
		//		    ev->xbutton.button);
		
		area->owner_->workAreaButtonPress(ev->xbutton.x - ob->x,
					   ev->xbutton.y - ob->y,
					   ev->xbutton.button);
		break; 
	case FL_RELEASE:
		if (!ev) break;
		// Should really have used xbutton.state
		lyxerr.debug() << "Workarea event: RELEASE" << endl;
		//workAreaButtonRelease(ev->xbutton.x - ob->x,
		//		      ev->xbutton.y - ob->y,
		//		      ev->xbutton.button);
		area->owner_->workAreaButtonRelease(ev->xbutton.x - ob->x,
					     ev->xbutton.y - ob->y,
					     ev->xbutton.button);
		break;
	case FL_MOUSE:
		if (!ev || ! area->scrollbar) break;
		if (ev->xmotion.x != x_old ||
		    ev->xmotion.y != y_old ||
		    fl_get_scrollbar_value(area->scrollbar) != scrollbar_value_old
			) {
			lyxerr.debug() << "Workarea event: MOUSE" << endl;
			//workAreaMotionNotify(ev->xmotion.x - ob->x,
			//		     ev->xmotion.y - ob->y,
			//		     ev->xbutton.state);
			
			area->owner_->workAreaMotionNotify(ev->xmotion.x - ob->x,
						    ev->xmotion.y - ob->y,
						    ev->xbutton.state);
		}
		break;
	// Done by the raw callback:
	//  case FL_KEYBOARD: WorkAreaKeyPress(ob, 0,0,0,ev,0); break;
	case FL_FOCUS:
		lyxerr.debug() << "Workarea event: FOCUS" << endl;
		//workAreaFocus();
		break;
	case FL_UNFOCUS:
		lyxerr.debug() << "Workarea event: UNFOCUS" << endl;
		//workAreaUnfocus();
		break;
	case FL_ENTER:
		lyxerr.debug() << "Workarea event: ENTER" << endl;
		//workAreaEnter();
		area->owner_->enterView();
		break;
	case FL_LEAVE:
		lyxerr.debug() << "Workarea event: LEAVE" << endl;
		//workAreaLeave();
		area->owner_->leaveView();
		break;
	case FL_DBLCLICK:
		if (!ev) break;
		lyxerr.debug() << "Workarea event: DBLCLICK" << endl;
		//workAreaDoubleClick(ev->xbutton.x - ob->x,
		//		    ev->xbutton.y - ob->y,
		//		    ev->xbutton.button);
		area->owner_->doubleClick(ev->xbutton.x - ob->x,
					 ev->xbutton.y - ob->y,
					 ev->xbutton.button);
		break;
	case FL_TRPLCLICK:
		if (!ev) break;
		lyxerr.debug() << "Workarea event: TRPLCLICK" << endl;
		//workAreaTripleClick(ev->xbutton.x - ob->x,
		//		    ev->xbutton.y - ob->y,
		//		    ev->xbutton.button);
		area->owner_->tripleClick(ev->xbutton.x - ob->x,
					 ev->xbutton.y - ob->y,
					 ev->xbutton.button);
		break;
	case FL_OTHER:
		if (!ev) break;
			lyxerr.debug() << "Workarea event: OTHER" << endl;

		break;
	}
  
	return 1;
}


static string clipboard_selection;
static bool clipboard_read = false;

extern "C" {
	static
int request_clipboard_cb(FL_OBJECT * /*ob*/, long /*type*/,
			void const * data, long size) 
{
	clipboard_selection.erase();
	if (size == 0) return 0; // no selection
	 
	clipboard_selection.reserve(size);
	for (int i = 0; i < size; ++i) {
		clipboard_selection += static_cast<char const *>(data)[i];
	}
	clipboard_read = true;
	return 0;
}
}

string WorkArea::getClipboard() const 
{
	clipboard_read = false;
	
	if (fl_request_clipboard(work_area, 0, request_clipboard_cb) == -1)
		return string();

	XEvent ev;
	
	while (!clipboard_read) {
		if (fl_check_forms() == FL_EVENT) {
			lyxerr << "LyX: This shouldn't happen..." << endl;
			fl_XNextEvent(&ev);
		}
	}
	return clipboard_selection;
}

	
void WorkArea::putClipboard(string const & s) const
{
	static string hold;
	hold = s;
	
	fl_stuff_clipboard(work_area, 0, hold.c_str(), hold.size(), 0);
}
