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
#include "up.xpm"
#include "down.xpm"
#include "debug.h"
#include "support/lstrings.h"
#include "BufferView.h"

FL_OBJECT * figinset_canvas;

static inline
void waitForX()
{
	XSync(fl_get_display(), 0);
}

extern "C" {
// Just a bunch of C wrappers around static members of WorkArea
	void C_WorkArea_up_cb(FL_OBJECT * ob, long buf)
        {
		WorkArea::up_cb(ob, buf);
        }

	void C_WorkArea_down_cb(FL_OBJECT * ob, long buf)
        {
		WorkArea::down_cb(ob, buf);
        }

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
	: owner(o), workareapixmap(0), painter_(*this)
{
	fl_freeze_all_forms();

	figinset_canvas = 0;
	
	lyxerr << "Creating work area: +"
	       << xpos << '+' << ypos << ' '
	       << width << 'x' << height << endl;
	//
	FL_OBJECT * obj;
	const int bw = int(abs(float(fl_get_border_width())));

	// We really want to get rid of figinset_canvas.
	::figinset_canvas = figinset_canvas = obj =
		  fl_add_canvas(FL_NORMAL_CANVAS,
				xpos + 1, ypos + 1, 1, 1, "");
	fl_set_object_boxtype(obj, FL_NO_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, NorthWestGravity);
	
	// a box
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

	// up - scrollbar button
	fl_set_border_width(-1);

	lyxerr << "\tup button: +"
	       << xpos + width - 15 << '+' << ypos << ' '
	       << 15 << 'x' << 15 << endl;
	button_up = obj = fl_add_pixmapbutton(FL_TOUCH_BUTTON,
					      xpos + width - 15,
					      ypos,
					      15, 15, "");
	fl_set_object_boxtype(obj,FL_UP_BOX);
	fl_set_object_color(obj,FL_MCOL,FL_BLUE);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj,NorthEastGravity, NorthEastGravity);
	obj->u_vdata = this;
	fl_set_object_callback(obj,C_WorkArea_up_cb, 0);
	fl_set_pixmapbutton_data(obj, const_cast<char**>(up_xpm));

	// Remove the blue feedback rectangle
	fl_set_pixmapbutton_focus_outline(obj,0);

	// the scrollbar slider
	fl_set_border_width(-bw);
	lyxerr << "\tscrollbar slider: +"
	       << xpos + width - 15 << '+' << ypos + 15 << ' '
	       << 15 << 'x' << height - 30 << endl;
	scrollbar = obj = fl_add_slider(FL_VERT_SLIDER,
					xpos + width - 15,
					ypos + 15,
					15, height - 30, "");
	fl_set_object_color(obj,FL_COL1,FL_MCOL);
	fl_set_object_boxtype(obj, FL_UP_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthEastGravity, SouthEastGravity);
	obj->u_vdata = this;
	fl_set_object_callback(obj, C_WorkArea_scroll_cb, 0);
	fl_set_slider_precision(obj, 0);
	
	// down - scrollbar button
	fl_set_border_width(-1);

	lyxerr << "\tdown button: +"
	       << xpos + width - 15 << '+' << ypos + height - 15 << ' '
	       << 15 << 'x' << 15 << endl;
	button_down = obj = fl_add_pixmapbutton(FL_TOUCH_BUTTON,
						xpos + width - 15,
						ypos + height - 15,
						15, 15, "");
	fl_set_object_boxtype(obj,FL_UP_BOX);
	fl_set_object_color(obj,FL_MCOL,FL_BLUE);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, SouthEastGravity, SouthEastGravity);
	obj->u_vdata = this;
	fl_set_object_callback(obj, C_WorkArea_down_cb, 0);
	fl_set_pixmapbutton_data(obj, const_cast<char**>(down_xpm));

	fl_set_border_width(-bw);

	// Remove the blue feedback rectangle
	fl_set_pixmapbutton_focus_outline(obj,0);

	///
	/// The free object

	// Create the workarea pixmap
	createPixmap(width - 15 - 2 * bw, height - 2 * bw);

	// setup the painter
	painter_.setDisplay(fl_display);
	
	// We add this object as late as possible to avoit problems
	// with drawing.
	lyxerr << "\tfree object: +"
	       << xpos + bw << '+' << ypos + bw << ' '
	       << width - 15 - 2 * bw << 'x' << height - 2 * bw << endl;
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


void WorkArea::resize(int xpos, int ypos, int width, int height)
{
	fl_freeze_all_forms();
	
	const int bw = int(abs(float(fl_get_border_width())));

	// a box
	fl_set_object_geometry(backgroundbox, xpos, ypos, width - 15, height);
	
	//
	// THE SCROLLBAR
	//

	// up - scrollbar button
	fl_set_object_geometry(button_up, xpos + width - 15,
			       ypos,
			       15, 15);
	// the scrollbar slider
	fl_set_object_geometry(scrollbar, xpos + width - 15,
			       ypos + 15,
			       15, height - 30);
	// down - scrollbar button
	fl_set_object_geometry(button_down, xpos + width - 15,
			       ypos + height - 15,
			       15, 15);

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
	
	lyxerr << "Creating pixmap (" << width << 'x' << height << ")" << endl;
	
	workareapixmap = XCreatePixmap(fl_display,
				       RootWindow(fl_display, 0),
				       width,
				       height, 
				       fl_get_visual_depth());
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
	fl_set_slider_value(scrollbar, pos);
	fl_set_slider_size(scrollbar, scrollbar->h * length_fraction);
}


void WorkArea::setScrollbarBounds(double l1, double l2) const
{
	fl_set_slider_bounds(scrollbar, l1, l2);
}


void WorkArea::setScrollbarIncrements(float inc) const
{
	fl_set_slider_increment(scrollbar, work_area->h - inc, inc);
}


void WorkArea::up_cb(FL_OBJECT * ob, long)
{
	WorkArea * area = static_cast<WorkArea*>(ob->u_vdata);
	XEvent const * ev2 = fl_last_event();
	static long time = 0;
	ev2 = fl_last_event();
	if (ev2->type == ButtonPress || ev2->type == ButtonRelease) 
		time = 0;
	//area->up(time++, fl_get_button_numb(ob));
	area->owner->upCB(time++, fl_get_button_numb(ob));
}


void WorkArea::down_cb(FL_OBJECT * ob, long)
{
	WorkArea * area = static_cast<WorkArea*>(ob->u_vdata);
	XEvent const * ev2 = fl_last_event();
	static long time = 0;
	if (ev2->type == ButtonPress || ev2->type == ButtonRelease)
		time = 0;
	//area->down(time++, fl_get_button_numb(ob));
	area->owner->downCB(time++, fl_get_button_numb(ob));
}


// Callback for scrollbar slider
void WorkArea::scroll_cb(FL_OBJECT * ob, long)
{
	WorkArea * area = static_cast<WorkArea*>(ob->u_vdata);

	//area->scroll(fl_get_slider_value(ob));
	area->owner->scrollCB(fl_get_slider_value(ob));
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
		area->owner->workAreaExpose();
		Lgb_bug_find_hack = false;
		break;
	case FL_PUSH:
		if (!ev) break;
		// Should really have used xbutton.state
		lyxerr.debug() << "Workarea event: PUSH" << endl;
		area->owner->workAreaButtonPress(ev->xbutton.x - ob->x,
					   ev->xbutton.y - ob->y,
					   ev->xbutton.button);
		break; 
	case FL_RELEASE:
		if (!ev) break;
		// Should really have used xbutton.state
		lyxerr.debug() << "Workarea event: RELEASE" << endl;
		area->owner->workAreaButtonRelease(ev->xbutton.x - ob->x,
					     ev->xbutton.y - ob->y,
					     ev->xbutton.button);
		break;
	case FL_MOUSE:
		if (!ev || ! area->scrollbar) break;
		if (ev->xmotion.x != x_old ||
		    ev->xmotion.y != y_old ||
		    fl_get_slider_value(area->scrollbar) != scrollbar_value_old) {
			lyxerr.debug() << "Workarea event: MOUSE" << endl;
			area->owner->workAreaMotionNotify(ev->xmotion.x - ob->x,
						    ev->xmotion.y - ob->y,
						    ev->xbutton.state);
		}
		break;
	// Done by the raw callback:
	//  case FL_KEYBOARD: WorkAreaKeyPress(ob, 0,0,0,ev,0); break;
	case FL_FOCUS:
		lyxerr.debug() << "Workarea event: FOCUS" << endl;
		break;
	case FL_UNFOCUS:
		lyxerr.debug() << "Workarea event: UNFOCUS" << endl;
		break;
	case FL_ENTER:
		lyxerr.debug() << "Workarea event: ENTER" << endl;
		break;
	case FL_LEAVE:
		lyxerr.debug() << "Workarea event: LEAVE" << endl;
		break;
	case FL_DBLCLICK:
		if (!ev) break;
		lyxerr.debug() << "Workarea event: DBLCLICK" << endl;
		area->owner->doubleClick(ev->xbutton.x - ob->x,
					 ev->xbutton.y - ob->y,
					 ev->xbutton.button);
		break;
	case FL_TRPLCLICK:
		if (!ev) break;
		lyxerr.debug() << "Workarea event: TRPLCLICK" << endl;
		area->owner->trippleClick(ev->xbutton.x - ob->x,
					 ev->xbutton.y - ob->y,
					 ev->xbutton.button);
		break;
	case FL_OTHER:
		if (!ev) break;
		if (ev->type == SelectionNotify) {
			lyxerr.debug() << "Workarea event: SELECTION" << endl;
			area->owner->workAreaSelectionNotify(area->work_area->form->window, ev);
		} else
			lyxerr.debug() << "Workarea event: OTHER" << endl;

		break;
	}
  
	return 1;
}
