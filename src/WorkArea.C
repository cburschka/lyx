/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *        
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.
 *
 * ====================================================== */

#include <config.h>
#include <cmath>
#include <cctype>
#ifdef __GNUG__
#pragma implementation
#endif

#include "WorkArea.h"
#include "debug.h"
#include "support/lstrings.h"
#include "LyXView.h"
#include "support/filetools.h" // LibFileSearch
#include "lyxrc.h" // lyxrc.show_banner
#include "version.h" // LYX_VERSION
#include "support/LAssert.h"

#if FL_REVISION < 89 || (FL_REVISION == 89 && FL_FIXLEVEL < 5)
#include "lyxlookup.h"
#endif

using std::endl;

FL_OBJECT * figinset_canvas;

// needed to make the c++ compiler find the correct version of abs.
// This is at least true for g++.
//using std::abs;

namespace {

inline
void waitForX()
{
	XSync(fl_get_display(), 0);
}

} // anon namespace


extern "C" {
	// Just a bunch of C wrappers around static members of WorkArea
	static
	void C_WorkArea_scroll_cb(FL_OBJECT * ob, long buf)
        {
		WorkArea::scroll_cb(ob, buf);
        }

	
	static
	int C_WorkArea_work_area_handler(FL_OBJECT * ob, int event,
					 FL_Coord, FL_Coord, 
					 int key, void * xev)
        {
		return WorkArea::work_area_handler(ob, event,
						   0, 0, key, xev);
        }


	// Resizing the display causes the version string to move relative to
	// the splash pixmap because the parameters xforms uses to control
	// resizing are not very sophisticated.
	// I found it easier, therefore, to just remove the splash screen.
	// (Angus, 25 September 2001)
        static
        int C_WorkAreaSplashPH(FL_OBJECT * ob, int event,
			       FL_Coord, FL_Coord, int, void *)
        {
		static int counter = 0;
                if (event != FL_DRAW || ++counter > 3) {
                        return 0;
		}

		lyx::Assert(ob && ob->u_vdata);
                WorkArea * pre = static_cast<WorkArea *>(ob->u_vdata);

		if (counter == 3) {
			pre->destroySplash();
		}

		return 0;
        }
}


WorkArea::WorkArea(int xpos, int ypos, int width, int height)
	: splash_(0), splash_text_(0), workareapixmap(0), painter_(*this)
{
	fl_freeze_all_forms();

	figinset_canvas = 0;

	if (lyxerr.debugging(Debug::GUI))
		lyxerr << "Creating work area: +"
		       << xpos << '+' << ypos << ' '
		       << width << 'x' << height << endl;
	//
	FL_OBJECT * obj;
	int const bw = int(std::abs(float(fl_get_border_width())));

	// We really want to get rid of figinset_canvas.
	::figinset_canvas = figinset_canvas = obj =
		  fl_add_canvas(FL_NORMAL_CANVAS,
				xpos + 1, ypos + 1, 1, 1, "");
	fl_set_object_boxtype(obj, FL_NO_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, NorthWestGravity);

	// a box
	if (lyxerr.debugging(Debug::GUI))
		lyxerr << "\tbackground box: +"
		       << xpos << '+' << ypos << ' '
		       << width - 15 << 'x' << height << endl;
	backgroundbox = obj = fl_add_box(FL_BORDER_BOX,
					 xpos, ypos,
					 width - 15,
					 height,"");
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);

	// Add a splash screen to the centre of the work area
	string const splash_file = (lyxrc.show_banner) ?
		LibFileSearch("images", "banner", "xpm") : string();

	if (!splash_file.empty()) {
		int const splash_w = 425;
		int const splash_h = 290;
		int const splash_x = xpos + (width - 15 - splash_w) / 2;
		int const splash_y = ypos + (height - splash_h) / 2;
		splash_ = obj =
			fl_add_pixmapbutton(FL_NORMAL_BUTTON,
					    splash_x, splash_y, 
					    splash_w, splash_h, "");
		obj->u_vdata = this;
		fl_set_object_prehandler(obj, C_WorkAreaSplashPH);

		fl_set_pixmapbutton_file(obj, splash_file.c_str());
		fl_set_pixmapbutton_focus_outline(obj, 3);
		fl_set_object_boxtype(obj, FL_NO_BOX);

		int const text_x = splash_x + 248;
		int const text_y = splash_y + 265;
		splash_text_ = obj =
			fl_add_text(FL_NORMAL_TEXT, text_x, text_y, 170, 16,
				    LYX_VERSION);
		fl_set_object_lsize(obj, FL_NORMAL_SIZE);
		fl_mapcolor(FL_FREE_COL2, 0x2b, 0x47, 0x82);
		fl_mapcolor(FL_FREE_COL3, 0xe1, 0xd2, 0x9b);
		fl_set_object_color(obj, FL_FREE_COL2, FL_FREE_COL2);
		fl_set_object_lcol(obj, FL_FREE_COL3);
		fl_set_object_lalign(obj, FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
		fl_set_object_lstyle(obj, FL_BOLD_STYLE);

		fl_hide_object(splash_);
		fl_hide_object(splash_text_);
	}

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
	if (lyxerr.debugging(Debug::GUI))
		lyxerr << "\tfree object: +"
		       << xpos + bw << '+' << ypos + bw << ' '
		       << width - 15 - 2 * bw << 'x'
		       << height - 2 * bw << endl;
	
	work_area = obj = fl_add_free(FL_ALL_FREE,
				      xpos + bw, ypos + bw,
				      width - 15 - 2 * bw, // scrollbarwidth
				      height - 2 * bw, "",
				      C_WorkArea_work_area_handler);
	obj->wantkey = FL_KEY_ALL;
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
		XFreePixmap(fl_get_display(), workareapixmap);
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
	
	int const bw = int(std::abs(float(fl_get_border_width())));

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

	destroySplash();

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
		XFreePixmap(fl_get_display(), workareapixmap);

	if (lyxerr.debugging(Debug::GUI))
		lyxerr << "Creating pixmap ("
		       << width << 'x' << height << ")" << endl;
	
	workareapixmap = XCreatePixmap(fl_get_display(),
				       RootWindow(fl_get_display(), 0),
				       width,
				       height, 
				       fl_get_visual_depth());
	if (lyxerr.debugging(Debug::GUI))
		lyxerr << "\tpixmap=" << workareapixmap << endl;
}


void WorkArea::show() const
{
	if (!work_area->visible) {
		fl_show_object(work_area);
	}
	
	destroySplash();
}


void WorkArea::greyOut() const
{
	if (work_area->visible) {
		fl_hide_object(work_area);
	}

	if (splash_ && !splash_->visible) {
		fl_show_object(splash_);
		fl_show_object(splash_text_);
	}
}


void WorkArea::destroySplash() const
{
	if (splash_) {
		if (splash_->visible) {
			fl_hide_object(splash_);
		}
		fl_set_object_prehandler(splash_, 0);
		// Causes a segmentation fault!
		// fl_delete_object(splash_);
		// fl_free_object(splash_);
		splash_ = 0;
	}

	if (splash_text_) {
		if (splash_text_->visible) {
			fl_hide_object(splash_text_);
		}
		fl_delete_object(splash_text_);
		fl_free_object(splash_text_);
		splash_text_ = 0;
	}
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
	area->scrollCB(fl_get_scrollbar_value(ob));
	waitForX();
}


bool Lgb_bug_find_hack = false;

int WorkArea::work_area_handler(FL_OBJECT * ob, int event,
				FL_Coord, FL_Coord ,
				int key, void * xev)
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
		lyxerr[Debug::GUI] << "Workarea event: DRAW" << endl;
		area->createPixmap(area->workWidth(), area->height());
		Lgb_bug_find_hack = true;
		area->workAreaExpose();
		Lgb_bug_find_hack = false;
		break;
	case FL_PUSH:
		if (!ev || ev->xbutton.button == 0) break;
		// Should really have used xbutton.state
		lyxerr[Debug::GUI] << "Workarea event: PUSH" << endl;
		area->workAreaButtonPress(ev->xbutton.x - ob->x,
					  ev->xbutton.y - ob->y,
					  ev->xbutton.button);
		//area->workAreaKeyPress(XK_Pointer_Button1, ev->xbutton.state);
		break; 
	case FL_RELEASE:
		if (!ev || ev->xbutton.button == 0) break;
		// Should really have used xbutton.state
		lyxerr[Debug::GUI] << "Workarea event: RELEASE" << endl;
		area->workAreaButtonRelease(ev->xbutton.x - ob->x,
				      ev->xbutton.y - ob->y,
				      ev->xbutton.button);
		break;
#if FL_REVISION < 89
	case FL_MOUSE:
#else
	case FL_DRAG:
#endif
		if (!ev || ! area->scrollbar) break;
		if (ev->xmotion.x != x_old ||
		    ev->xmotion.y != y_old ||
		    fl_get_scrollbar_value(area->scrollbar) != scrollbar_value_old
			) {
			lyxerr[Debug::GUI] << "Workarea event: MOUSE" << endl;
			area->workAreaMotionNotify(ev->xmotion.x - ob->x,
					     ev->xmotion.y - ob->y,
					     ev->xbutton.state);
		}
		break;
#if FL_REVISION < 89
	case FL_KEYBOARD:
#else
	case FL_KEYPRESS:
#endif
	{
		lyxerr[Debug::KEY] << "Workarea event: KEYBOARD" << endl;
		
		KeySym keysym = 0;
		char dummy[1];
		XKeyEvent * xke = reinterpret_cast<XKeyEvent *>(ev);
#if FL_REVISION < 89 || (FL_REVISION == 89 && FL_FIXLEVEL < 5)
		// XForms < 0.89.5 does not have compose support
		// so we are using our own compose support
		LyXLookupString(ev, dummy, 1, &keysym);
#else
		XLookupString(xke, dummy, 1, &keysym, 0);
#endif
		if (lyxerr.debugging(Debug::KEY)) {
			char const * tmp = XKeysymToString(key);
			char const * tmp2 = XKeysymToString(keysym);
			string const stm = (tmp ? tmp : "");
			string const stm2 = (tmp2 ? tmp2 : "");
			
			lyxerr << "WorkArea: Key is `" << stm << "' ["
			       << key << "]" << endl;
			lyxerr << "WorkArea: Keysym is `" << stm2 << "' ["
			       << keysym << "]" << endl;
		}

#if FL_REVISION < 89 || (FL_REVISION == 89 && FL_FIXLEVEL < 5)
		if (keysym == NoSymbol) {
			lyxerr[Debug::KEY]
				<< "Empty kdb action (probably composing)"
				<< endl;
			break;
		}
		KeySym ret_key = keysym;
#else
		// Note that we need this handling because of a bug
		// in XForms 0.89, if this bug is resolved in the way I hope
		// we can just use the keysym directly with out looking
		// at key at all. (Lgb)
		KeySym ret_key = 0;
		if (!key) {
			// We migth have to add more keysyms here also,
			// we will do that as the issues arise. (Lgb)
			if (keysym == XK_space) {
				ret_key = keysym;
				lyxerr[Debug::KEY] << "Using keysym [A]"
						   << endl;
			} else
				break;
		} else {
			// It seems that this was a bit optimistic...
			// With this hacking things seems to be better (Lgb)
			//if (!iscntrl(key)) {
			//	ret_key = key;
			//	lyxerr[Debug::KEY]
			//		<< "Using key [B]\n"
			//		<< "Uchar["
			//		<< static_cast<unsigned char>(key)
			//		<< endl;
			//} else {
				ret_key = (keysym ? keysym : key);
				lyxerr[Debug::KEY] << "Using keysym [B]"
						   << endl;
				//}
		}
		
#endif
		unsigned int const ret_state = xke->state;

		// If you have a better way to handle "wild-output" of
		// characters after the key has been released than the one
		// below, please contact me. (Lgb)
		static Time last_time_pressed;
		static unsigned int last_key_pressed;
		static unsigned int last_state_pressed;
		lyxerr[Debug::KEY] << "Workarea Diff: "
				   << xke->time - last_time_pressed
				   << endl;
		if (xke->time - last_time_pressed < 25 // should perhaps be tunable
		    && ret_state == last_state_pressed
		    && xke->keycode == last_key_pressed) {
			lyxerr[Debug::KEY]
				<< "Workarea: Purging X events." << endl;
			//lyxerr << "Workarea Events: "
			//       << XEventsQueued(fl_get_display(), QueuedAlready)
			//       << endl;
			if (XEventsQueued(fl_get_display(), QueuedAlready) > 0)
				XSync(fl_get_display(), 1);
			// This purge make f.ex. scrolling stop immidiatly when
			// releasing the PageDown button. The question is if
			// this purging of XEvents can cause any harm...
			// after some testing I can see no problems, but
			// I'd like other reports too.
			break;
		}
		last_time_pressed = xke->time;
		last_key_pressed = xke->keycode;
		last_state_pressed = ret_state;
		
		area->workAreaKeyPress(ret_key, ret_state);
	}
	break;

#if FL_REVISION >= 89
	case FL_KEYRELEASE:
		lyxerr << "Workarea event: KEYRELEASE" << endl;
		break;
#endif

	case FL_FOCUS:
		lyxerr[Debug::GUI] << "Workarea event: FOCUS" << endl;
		area->workAreaFocus();
		break;
	case FL_UNFOCUS:
		lyxerr[Debug::GUI] << "Workarea event: UNFOCUS" << endl;
		area->workAreaUnfocus();
		break;
	case FL_ENTER:
		lyxerr[Debug::GUI] << "Workarea event: ENTER" << endl;
		area->workAreaEnter();
		break;
	case FL_LEAVE:
		lyxerr[Debug::GUI] << "Workarea event: LEAVE" << endl;
		area->workAreaLeave();
		break;
	case FL_DBLCLICK:
		if (!ev) break;
		lyxerr[Debug::GUI] << "Workarea event: DBLCLICK" << endl;
		area->workAreaDoubleClick(ev->xbutton.x - ob->x,
					  ev->xbutton.y - ob->y,
					  ev->xbutton.button);
		break;
	case FL_TRPLCLICK:
		if (!ev) break;
		lyxerr[Debug::GUI] << "Workarea event: TRPLCLICK" << endl;
		area->workAreaTripleClick(ev->xbutton.x - ob->x,
					  ev->xbutton.y - ob->y,
					  ev->xbutton.button);
		break;
	case FL_OTHER:
		if (!ev) break;
			lyxerr[Debug::GUI] << "Workarea event: OTHER" << endl;

		break;
	}
  
	return 1;
}


namespace {

string clipboard_selection;
bool clipboard_read = false;

extern "C" {
	
	static
	int request_clipboard_cb(FL_OBJECT * /*ob*/, long /*type*/,
				 void const * data, long size) 
	{
		clipboard_selection.erase();
		
		if (size > 0)
			clipboard_selection.reserve(size);
		for (int i = 0; i < size; ++i)
			clipboard_selection +=
				static_cast<char const *>(data)[i];
		clipboard_read = true;
		return 0;
	}

}

} // namespace anon

string const WorkArea::getClipboard() const 
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
	
	fl_stuff_clipboard(work_area, 0, hold.data(), hold.size(), 0);
}
