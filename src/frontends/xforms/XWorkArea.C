/**
 * \file XWorkArea.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "XWorkArea.h"

#include "Color.h"
#include "XFormsView.h"
#include "XLyXKeySym.h"

#include "debug.h"
#include "funcrequest.h"
#include "LColor.h"
#include "Timeout.h"

#include <boost/bind.hpp>

using std::abs;
using std::dec;
using std::endl;
using std::hex;
using std::string;

namespace lyx {
namespace frontend {

namespace {

inline void waitForX(bool discard)
{
	XSync(fl_get_display(), discard);
}


mouse_button::state x_button_state(unsigned int button)
{
	mouse_button::state b = mouse_button::none;
	switch (button) {
	case FL_MBUTTON1:
		b = mouse_button::button1;
		break;
	case FL_MBUTTON2:
		b = mouse_button::button2;
		break;
	case FL_MBUTTON3:
		b = mouse_button::button3;
		break;
	case FL_MBUTTON4:
		b = mouse_button::button4;
		break;
	case FL_MBUTTON5:
		b = mouse_button::button5;
		break;
	}
	return b;
}


key_modifier::state x_key_state(unsigned int state)
{
	key_modifier::state k = key_modifier::none;
	if (state & ControlMask)
		k |= key_modifier::ctrl;
	if (state & ShiftMask)
		k |= key_modifier::shift;
	if (state & Mod1Mask)
		k |= key_modifier::alt;
	return k;
}


extern "C" {

void C_scroll_cb(FL_OBJECT * ob, long)
{
	XWorkArea * area = static_cast<XWorkArea *>(ob->u_vdata);
	area->scroll_cb();
}


int C_work_area_handler(FL_OBJECT * ob, int event, FL_Coord, FL_Coord,
			int key, void * xev)
{
	return XWorkArea::work_area_handler(ob, event, 0, 0, key, xev);
}


int C_event_cb(FL_FORM * form, void * xev)
{
	XWorkArea * area = static_cast<XWorkArea *>(form->u_vdata);
	return area->event_cb(static_cast<XEvent *>(xev));
}

} // extern "C"
} // namespace anon


XWorkArea::XWorkArea(LyXView & owner, int w, int h)
	: workareapixmap(0), painter_(*this)
{
	fl_freeze_all_forms();

	FL_OBJECT * obj;
	FL_OBJECT * frame;

	// A frame around the work area.
	frame = obj = fl_add_box(FL_BORDER_BOX, 0, 0, w, h, "");
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);

	unsigned int r, g, b;
	if (getRGBColor(LColor::background, r, g, b)) {
		fl_mapcolor(FL_FREE_COL12, r, g, b);
		fl_set_object_color(obj, FL_FREE_COL12, FL_MCOL);
	}

	// The scrollbar.
	scrollbar = obj = fl_add_scrollbar(FL_VERT_SCROLLBAR, 0, 0, w, h, "");
	fl_set_object_boxtype(obj, FL_UP_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthEastGravity, SouthEastGravity);
	obj->u_vdata = this;
	fl_set_object_callback(obj, C_scroll_cb, 0);
	fl_set_scrollbar_bounds(scrollbar, 0.0, 0.0);
	fl_set_scrollbar_value(scrollbar, 0.0);
	fl_set_scrollbar_size(scrollbar, scrollbar->h);

	// The work area itself
	work_area = obj = fl_add_free(FL_ALL_FREE, 0, 0, w, h, "",
				      C_work_area_handler);
	obj->wantkey = FL_KEY_ALL;
	obj->u_vdata = this;

	fl_set_object_boxtype(obj,FL_DOWN_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);

	// Hand control of the layout of these widgets to the
	// Layout Engine.
	XFormsView & xview = dynamic_cast<XFormsView &>(owner);
	BoxList & boxlist = xview.getBox(XFormsView::Center).children();

	wa_box_ = &boxlist.push_back(Box(0,0));
	wa_box_->set(Box::Horizontal);

	Box & frame_box = widgets_.add(frame, wa_box_->children(), 0, 0);
	frame_box.set(Box::Expand);

	int const bw = int(abs(fl_get_border_width()));
	Box & wa_box = embed(work_area, frame_box.children(), widgets_, bw);
	wa_box.set(Box::Expand);

	widgets_.add(scrollbar, wa_box_->children(), 17, 0);

	xview.metricsUpdated.connect(boost::bind(&WidgetMap::updateMetrics,
						 &widgets_));

	/// X selection hook - xforms gets it wrong
	fl_current_form->u_vdata = this;
	fl_register_raw_callback(fl_current_form, FL_ALL_EVENT, C_event_cb);

	fl_unfreeze_all_forms();

	XGCValues val;

	val.function = GXcopy;
	val.graphics_exposures = false;
	copy_gc = XCreateGC(fl_get_display(), RootWindow(fl_get_display(), 0),
			    GCFunction | GCGraphicsExposures, &val);
}


XWorkArea::~XWorkArea()
{
	XFreeGC(fl_get_display(), copy_gc);
	if (workareapixmap)
		XFreePixmap(fl_get_display(), workareapixmap);
}


void XWorkArea::updateGeometry(int width, int height)
{
	static int cur_width = -1;
	static int cur_height = -1;

	if (cur_width == width && cur_height == height && workareapixmap)
		return;

	cur_width = width;
	cur_height = height;

	if (lyxerr.debugging(Debug::WORKAREA)) {
		lyxerr << "(Re)creating pixmap ("
		       << width << 'x' << height << ')' << endl;
	}

	if (workareapixmap) {
		XFreePixmap(fl_get_display(), workareapixmap);
	}

	workareapixmap = XCreatePixmap(fl_get_display(),
				       RootWindow(fl_get_display(), 0),
				       width,
				       height,
				       fl_get_visual_depth());

	workAreaResize();
}


void XWorkArea::paint(int x, int y, int w, int h)
{
	lyxerr[Debug::WORKAREA]
		<< "XWorkarea::paint " << w << 'x' << h
		<< '+' << x << '+' << y << endl;

	updateGeometry(workWidth(), workHeight());
	XCopyArea(fl_get_display(),
		  getPixmap(), getWin(),
		  copy_gc, x, y, w, h,
		  work_area->x + x, work_area->y + y);
}


void XWorkArea::setScrollbarParams(int height, int pos, int line_height)
{
	// we need to cache this for scroll_cb
	doc_height_ = height;

	if (height == 0) {
		fl_set_scrollbar_value(scrollbar, 0.0);
		fl_set_scrollbar_size(scrollbar, scrollbar->h);
		return;
	}

	long const work_height = workHeight();

	if (lyxerr.debugging(Debug::GUI)) {
		lyxerr << "scroll: height now " << height << '\n'
		       << "scroll: work_height " << work_height << endl;
	}

	/* If the text is smaller than the working area, the scrollbar
	 * maximum must be the working area height. No scrolling will
	 * be possible */
	if (height <= work_height) {
		lyxerr[Debug::GUI] << "scroll: doc smaller than workarea !"
				   << endl;
		fl_set_scrollbar_bounds(scrollbar, 0.0, 0.0);
		fl_set_scrollbar_value(scrollbar, pos);
		fl_set_scrollbar_size(scrollbar, scrollbar->h);
		return;
	}

	fl_set_scrollbar_bounds(scrollbar, 0.0, height - work_height);
	fl_set_scrollbar_increment(scrollbar, work_area->h - line_height, line_height);

	fl_set_scrollbar_value(scrollbar, pos);

	double const slider_size =
		(height == 0) ? 1.0 : 1.0 / double(height);

	fl_set_scrollbar_size(scrollbar, scrollbar->h * slider_size);
}


// callback for scrollbar slider
void XWorkArea::scroll_cb()
{
	double const val = fl_get_scrollbar_value(scrollbar);

	if (lyxerr.debugging(Debug::GUI)) {
		lyxerr << "scroll: val: " << val << '\n'
		       << "scroll: height: " << scrollbar->h << '\n'
		       << "scroll: docheight: " << doc_height_ << endl;
	}

	scrollDocView(int(val));
	waitForX(false);
}


int XWorkArea::work_area_handler(FL_OBJECT * ob, int event,
				 FL_Coord, FL_Coord,
				 int key, void * xev)
{
	if (event != 11)
		lyxerr[Debug::WORKAREA] << "Workarea event: EVENT: " << event << endl;

	XEvent * ev = static_cast<XEvent*>(xev);
	XWorkArea * area = static_cast<XWorkArea*>(ob->u_vdata);

	if (!area)
		return 1;

	switch (event) {

	case FL_DRAW: {
		if (!area->work_area || !area->work_area->form->visible)
			return 1;

		if (ev) {
			lyxerr[Debug::WORKAREA]
				<< "work_area_handler, handling X11 "
				"expose event "
				<< ev->xexpose.width << 'x'
				<< ev->xexpose.height << '+'
				<< ev->xexpose.x << '+'
				<< ev->xexpose.y << endl;

			// X11 generates XEvents with x, y relative to the
			// top left corner of the window.
			// XScreen::expose emulates this behaviour.
			// We therefore need to remove this offset before
			// generating the pixmap.
			 int const x = ev->xexpose.x - ob->x;
			 int const y = ev->xexpose.y - ob->y;

			area->paint(x, y,
				    ev->xexpose.width, ev->xexpose.height);
		} else
			area->paint(0, 0,
				    area->workWidth(), area->workHeight());

		break;
	}

	case FL_PUSH:
		if (!ev || ev->xbutton.button == 0) break;

		if (ev->xbutton.button == 4 || ev->xbutton.button == 5) {
			static long last_wheel;

			long cur_wheel = ev->xbutton.time;
			if (last_wheel == cur_wheel)
				break;

			last_wheel = cur_wheel;

			float l, r;
			fl_get_scrollbar_increment(area->scrollbar, &l, &r);

			if (ev->xbutton.button == 4)
				l *= -1.0;

			fl_set_scrollbar_value(
				area->scrollbar,
				fl_get_scrollbar_value(area->scrollbar) + l);

			area->scroll_cb();
			break;
		}

		// Should really have used xbutton.state
		lyxerr[Debug::WORKAREA] << "Workarea event: PUSH" << endl;
		area->dispatch(
			FuncRequest(LFUN_MOUSE_PRESS,
				    ev->xbutton.x - ob->x,
				    ev->xbutton.y - ob->y,
				    x_button_state(key)));
		break;

	case FL_RELEASE:
		if (!ev || ev->xbutton.button == 0) break;
		// Should really have used xbutton.state

		if (ev->xbutton.button == 4 || ev->xbutton.button == 5) {
			// We ingnore wheel event here
			break;
		}

		lyxerr[Debug::WORKAREA] << "Workarea event: RELEASE" << endl;

		area->dispatch(
			FuncRequest(LFUN_MOUSE_RELEASE,
				    ev->xbutton.x - ob->x,
				    ev->xbutton.y - ob->y,
				    x_button_state(key)));
		break;

	case FL_DRAG: {
		lyxerr[Debug::WORKAREA] << "Workarea event: DRAG 0" << endl;

		if (!ev || !area->scrollbar)
			break;

		int const drag_x = ev->xmotion.x;
		int const drag_y = ev->xmotion.y;
		int const area_y = ob->y;
		int const area_h = ob->h;

		// Check if the mouse is above or below the workarea
		if (drag_y <= area_y || drag_y >= area_y + area_h) {
			// The mouse button is depressed and we are outside the
			// workarea. That means we are simultaneously selecting
			// text and scrolling the view.
			// Use a Timeout to react to a drag events only every
			// 200ms. All intervening events are discarded,
			// allowing the user to control position easily.
			static int const discard_interval = 200;
			static Timeout timeout(discard_interval);

			if (timeout.running())
				break;
			// The timeout is not running, so process the
			// event, first starting the timeout to discard future
			// events.
			timeout.start();
		}

		static int x_old = -1;
		static int y_old = -1;
		static double scrollbar_value_old = -1.0;

		double const scrollbar_value =
			fl_get_scrollbar_value(area->scrollbar);

		if (drag_x != x_old || drag_y != y_old ||
		    scrollbar_value != scrollbar_value_old) {
			x_old = drag_x;
			y_old = drag_y;
			scrollbar_value_old = scrollbar_value;

			lyxerr[Debug::WORKAREA] << "Workarea event: DRAG"
						<< endl;

			// It transpires that ev->xbutton.button == 0 when
			// the mouse is dragged, so it cannot be used to
			// initialise x_button_state and hence FuncRequest.

			// The 'key' that is passed into the function does
			// contain the necessary info, however.

			// It is for this reason that x_button_state has
			// been modified to work with key
			// rather than ev->xbutton.button.

			// Angus 15 Oct 2002.
			FuncRequest cmd(LFUN_MOUSE_MOTION,
					ev->xbutton.x - ob->x,
					ev->xbutton.y - ob->y,
					x_button_state(key));
			area->dispatch(cmd);
		}
		break;
	}

	case FL_KEYPRESS: {
		lyxerr[Debug::WORKAREA] << "Workarea event: KEYPRESS" << endl;

		KeySym keysym = 0;
		char dummy[1];
		XKeyEvent * xke = reinterpret_cast<XKeyEvent *>(ev);
		XLookupString(xke, dummy, 1, &keysym, 0);

		if (lyxerr.debugging(Debug::KEY)) {
			char const * const tmp  = XKeysymToString(key);
			char const * const tmp2 = XKeysymToString(keysym);
			string const stm  = (tmp ? tmp : string());
			string const stm2 = (tmp2 ? tmp2 : string());

			lyxerr << "XWorkArea: Key is `" << stm
			       << "' [" << key << "]\n"
			       << "XWorkArea: Keysym is `" << stm2
			       << "' [" << keysym << ']' << endl;
		}

		// Note that we need this handling because of a bug
		// in XForms 0.89, if this bug is resolved in the way I hope
		// we can just use the keysym directly without looking
		// at key at all. (Lgb)
		KeySym ret_key = 0;
		if (!key) {
			// We might have to add more keysyms here also,
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
				waitForX(true);
			// This purge make f.ex. scrolling stop immediately when
			// releasing the PageDown button. The question is if
			// this purging of XEvents can cause any harm...
			// after some testing I can see no problems, but
			// I'd like other reports too.
			break;
		}
		last_time_pressed = xke->time;
		last_key_pressed = xke->keycode;
		last_state_pressed = ret_state;

		XLyXKeySym * xlk = new XLyXKeySym;
		xlk->initFromKeySym(ret_key);

		area->workAreaKeyPress(LyXKeySymPtr(xlk),
				       x_key_state(ret_state));
		break;
	}

	case FL_KEYRELEASE:
		lyxerr[Debug::WORKAREA] << "Workarea event: KEYRELEASE" << endl;
		break;

	case FL_ENTER:
		lyxerr[Debug::WORKAREA] << "Workarea event: ENTER" << endl;
		fl_set_cursor(FL_ObjWin(area->work_area), XC_xterm);
		break;

	case FL_LEAVE:
		lyxerr[Debug::WORKAREA] << "Workarea event: LEAVE" << endl;
		// There should be no need for this. But there is.
		fl_set_cursor(FL_ObjWin(area->work_area), FL_DEFAULT_CURSOR);
		break;

	case FL_DBLCLICK:
		if (ev) {
			if (ev->xbutton.button == 4 || ev->xbutton.button == 5) {
				// Ignore wheel events
				break;
			}


			lyxerr[Debug::WORKAREA] << "Workarea event: DBLCLICK" << endl;
			FuncRequest cmd(LFUN_MOUSE_DOUBLE,
					ev->xbutton.x - ob->x,
					ev->xbutton.y - ob->y,
					x_button_state(key));
			area->dispatch(cmd);
		}
		break;

	case FL_TRPLCLICK:
		if (ev) {
			if (ev->xbutton.button == 4 || ev->xbutton.button == 5) {
				// Ignore wheel events
				break;
			}

			lyxerr[Debug::WORKAREA] << "Workarea event: TRPLCLICK" << endl;
			FuncRequest cmd(LFUN_MOUSE_TRIPLE,
					ev->xbutton.x - ob->x,
					ev->xbutton.y - ob->y,
					x_button_state(key));
			area->dispatch(cmd);
		}
		break;

	case FL_OTHER:
		if (ev)
			lyxerr[Debug::WORKAREA] << "Workarea event: OTHER" << endl;
		break;
	}

	return 1;
}


namespace {

string clipboard_selection;
bool clipboard_read = false;

extern "C" {

int request_clipboard_cb(FL_OBJECT * /*ob*/, long /*type*/,
			 void const * data, long size)
{
	clipboard_selection.erase();

	if (size > 0)
		clipboard_selection.reserve(size);
	for (int i = 0; i < size; ++i)
		clipboard_selection += static_cast<char const *>(data)[i];
	clipboard_read = true;
	return 0;
}

} // extern "C"
} // namespace anon


int XWorkArea::event_cb(XEvent * xev)
{
	switch (xev->type) {
	case SelectionRequest:
		lyxerr[Debug::GUI] << "X requested selection." << endl;
		selectionRequested();
		break;
	case SelectionClear:
		lyxerr[Debug::GUI] << "Lost selection." << endl;
		selectionLost();
		break;
	}
	return 0;
}


void XWorkArea::haveSelection(bool yes) const
{
	Window const owner = yes ? FL_ObjWin(work_area) : None;
	XSetSelectionOwner(fl_get_display(), XA_PRIMARY, owner, CurrentTime);
}


string const XWorkArea::getClipboard() const
{
	clipboard_read = false;

	if (fl_request_clipboard(work_area, 0, request_clipboard_cb) == -1)
		return string();

	XEvent ev;

	while (!clipboard_read) {
		if (fl_check_forms() == FL_EVENT) {
			fl_XNextEvent(&ev);
			lyxerr << "Received unhandled X11 event\n"
			       << "Type: 0x" << hex << ev.xany.type
			       << " Target: 0x" << hex << ev.xany.window
			       << dec << endl;
		}
	}
	return clipboard_selection;
}


void XWorkArea::putClipboard(string const & s) const
{
	static string hold;
	hold = s;

	fl_stuff_clipboard(work_area, 0, hold.data(), hold.size(), 0);
}

} // namespace frontend
} // namespace lyx
