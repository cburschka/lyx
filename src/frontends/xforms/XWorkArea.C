/**
 * \file XWorkArea.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author unknown
 * \author John Levon 
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "XWorkArea.h"
#include "debug.h"
#include "LyXView.h"
#include "XLyXKeySym.h"
#include "ColorHandler.h"
#include "funcrequest.h"

#if FL_VERSION < 1 && (FL_REVISION < 89 || (FL_REVISION == 89 && FL_FIXLEVEL < 5))
#include "lyxlookup.h"
#endif

#include "support/filetools.h" // LibFileSearch
#include "support/lstrings.h"
#include "support/LAssert.h"

#include <cmath>
#include <cctype>

// xforms doesn't define this (but it should be in <forms.h>).
extern "C"
FL_APPEVENT_CB fl_set_preemptive_callback(Window, FL_APPEVENT_CB, void *);

using std::endl;
using std::abs;
using std::hex;
using std::dec;

namespace {

inline
void waitForX()
{
	XSync(fl_get_display(), 0);
}


void setXtermCursor(Window win)
{
	static Cursor cursor;
	static bool cursor_undefined = true;
	if (cursor_undefined) {
		cursor = XCreateFontCursor(fl_get_display(), XC_xterm);
		XFlush(fl_get_display());
		cursor_undefined = false;
	}
	XDefineCursor(fl_get_display(), win, cursor);
	XFlush(fl_get_display());
}


mouse_button::state x_button_state(unsigned int button)
{
	mouse_button::state b = mouse_button::none;
	switch (button) {
		case Button1:
			b = mouse_button::button1;
			break;
		case Button2:
			b = mouse_button::button2;
			break;
		case Button3:
			b = mouse_button::button3;
			break;
		case Button4:
			b = mouse_button::button4;
			break;
		case Button5:
			b = mouse_button::button5;
			break;
		default: // FIXME
			break;
	}
	return b;
}


mouse_button::state x_motion_state(unsigned int state)
{
	mouse_button::state b = mouse_button::none;
	if (state & Button1MotionMask)
		b |= mouse_button::button1;
	if (state & Button2MotionMask)
		b |= mouse_button::button2;
	if (state & Button3MotionMask)
		b |= mouse_button::button3;
	if (state & Button4MotionMask)
		b |= mouse_button::button4;
	if (state & Button5MotionMask)
		b |= mouse_button::button5;
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


} // anon namespace


extern "C" {
	// Just a bunch of C wrappers around static members of XWorkArea
	static
	void C_XWorkArea_scroll_cb(FL_OBJECT * ob, long)
	{
		XWorkArea * area = static_cast<XWorkArea*>(ob->u_vdata);
		area->scroll_cb();
	}


	static
	int C_XWorkArea_work_area_handler(FL_OBJECT * ob, int event,
					 FL_Coord, FL_Coord,
					 int key, void * xev)
	{
		return XWorkArea::work_area_handler(ob, event,
						   0, 0, key, xev);
	}

	static
	int C_XWorkAreaEventCB(FL_FORM * form, void * xev) {
		XWorkArea * wa = static_cast<XWorkArea*>(form->u_vdata);
		return wa->event_cb(static_cast<XEvent*>(xev));
	}
}


XWorkArea::XWorkArea(int x, int y, int w, int h)
	: workareapixmap(0), painter_(*this)
{
	fl_freeze_all_forms();

	FL_OBJECT * obj;

	if (lyxerr.debugging(Debug::WORKAREA))
		lyxerr << "\tbackground box: +"
		       << x << '+' << y << ' '
		       << w - 15 << 'x' << h << endl;
	backgroundbox = obj = fl_add_box(FL_BORDER_BOX,
					 x, y,
					 w - 15,
					 h, "");
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);

	scrollbar = obj = fl_add_scrollbar(FL_VERT_SCROLLBAR,
					   x + w - 15,
					   y, 17, h, "");
	fl_set_object_boxtype(obj, FL_UP_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthEastGravity, SouthEastGravity);
	obj->u_vdata = this;
	fl_set_object_callback(obj, C_XWorkArea_scroll_cb, 0);
	fl_set_scrollbar_bounds(scrollbar, 0.0, 0.0);
	fl_set_scrollbar_value(scrollbar, 0.0);
	fl_set_scrollbar_size(scrollbar, scrollbar->h);

	int const bw = int(abs(fl_get_border_width()));

	// Create the workarea pixmap
	// FIXME remove redraw(w - 15 - 2 * bw, h - 2 * bw);

	if (lyxerr.debugging(Debug::WORKAREA))
		lyxerr << "\tfree object: +"
		       << x + bw << '+' << y + bw << ' '
		       << w - 15 - 2 * bw << 'x'
		       << h - 2 * bw << endl;

	// We add this object as late as possible to avoid problems
	// with drawing.
	// FIXME: like ??
	work_area = obj = fl_add_free(FL_ALL_FREE,
				      x + bw, y + bw,
				      w - 15 - 2 * bw,
				      h - 2 * bw, "",
				      C_XWorkArea_work_area_handler);
	obj->wantkey = FL_KEY_ALL;
	obj->u_vdata = this;

	fl_set_object_boxtype(obj,FL_DOWN_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, NorthWestGravity, SouthEastGravity);

	/// X selection hook - xforms gets it wrong
	fl_current_form->u_vdata = this;
	fl_register_raw_callback(fl_current_form, FL_ALL_EVENT, C_XWorkAreaEventCB);

	fl_unfreeze_all_forms();

	XGCValues val;

	val.function = GXcopy;
	copy_gc = XCreateGC(fl_get_display(), RootWindow(fl_get_display(), 0),
		GCFunction, &val);
}


XWorkArea::~XWorkArea()
{
	XFreeGC(fl_get_display(), copy_gc);
	if (workareapixmap)
		XFreePixmap(fl_get_display(), workareapixmap);
}


namespace {
void destroy_object(FL_OBJECT * obj)
{
	if (!obj)
		return;

	if (obj->visible) {
		fl_hide_object(obj);
	}
	fl_delete_object(obj);
	fl_free_object(obj);
}
} // namespace anon


void XWorkArea::redraw(int width, int height)
{
	static int cur_width = -1;
	static int cur_height = -1;

	if (cur_width == width && cur_height == height && workareapixmap) {
		XCopyArea(fl_get_display(),
			getPixmap(), getWin(), copy_gc,
			0, 0, width, height, xpos(), ypos());
		return;
	}

	cur_width = width;
	cur_height = height;

	if (lyxerr.debugging(Debug::WORKAREA)) {
		lyxerr << "(Re)creating pixmap ("
		       << width << 'x' << height << ")" << endl;
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

	lyxerr[Debug::GUI] << "scroll: height now " << height << endl;
	lyxerr[Debug::GUI] << "scroll: work_height " << work_height << endl;

	/* If the text is smaller than the working area, the scrollbar
	 * maximum must be the working area height. No scrolling will
	 * be possible */
	if (height <= work_height) {
		lyxerr[Debug::GUI] << "scroll: doc smaller than workarea !" << endl;
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
	lyxerr[Debug::GUI] << "scroll: val: " << val << endl;
	lyxerr[Debug::GUI] << "scroll: height: " << scrollbar->h << endl;
	lyxerr[Debug::GUI] << "scroll: docheight: " << doc_height_ << endl;
	scrollDocView(int(val));
	waitForX();
}


int XWorkArea::work_area_handler(FL_OBJECT * ob, int event,
				FL_Coord, FL_Coord,
				int key, void * xev)
{
	static int x_old = -1;
	static int y_old = -1;
	static double scrollbar_value_old = -1.0;

	XEvent * ev = static_cast<XEvent*>(xev);
	XWorkArea * area = static_cast<XWorkArea*>(ob->u_vdata);

	if (!area) return 1;

	switch (event) {
	case FL_DRAW:
		if (!area->work_area ||
		    !area->work_area->form->visible)
			return 1;
		lyxerr[Debug::WORKAREA] << "Workarea event: DRAW" << endl;
		area->redraw(area->workWidth(), area->workHeight());
		break;
	case FL_PUSH:
		if (!ev || ev->xbutton.button == 0) break;
		// Should really have used xbutton.state
		lyxerr[Debug::WORKAREA] << "Workarea event: PUSH" << endl;
		area->dispatch(
			FuncRequest(LFUN_MOUSE_PRESS, ev->xbutton.x - ob->x,
							ev->xbutton.y - ob->y,
							x_button_state(ev->xbutton.button)));
		break;
	case FL_RELEASE:
		if (!ev || ev->xbutton.button == 0) break;
		// Should really have used xbutton.state
		lyxerr[Debug::WORKAREA] << "Workarea event: RELEASE" << endl;
		area->dispatch(
			FuncRequest(LFUN_MOUSE_RELEASE, ev->xbutton.x - ob->x,
							ev->xbutton.y - ob->y,
							x_button_state(ev->xbutton.button)));
		break;
#if FL_VERSION < 1 && FL_REVISION < 89
	case FL_MOUSE:
#else
	case FL_DRAG:
#endif
		if (!ev || !area->scrollbar)
			break;
		if (ev->xmotion.x != x_old ||
		    ev->xmotion.y != y_old ||
		    fl_get_scrollbar_value(area->scrollbar) != scrollbar_value_old
			) {
			x_old = ev->xmotion.x;
			y_old = ev->xmotion.y;
			scrollbar_value_old = fl_get_scrollbar_value(area->scrollbar);
			lyxerr[Debug::WORKAREA] << "Workarea event: MOUSE" << endl;
			area->dispatch(
				FuncRequest(LFUN_MOUSE_MOTION, ev->xbutton.x - ob->x,
								ev->xbutton.y - ob->y,
								x_button_state(ev->xbutton.button)));
		}
		break;
#if FL_VERSION < 1 && FL_REVISION < 89
	case FL_KEYBOARD:
#else
	case FL_KEYPRESS:
#endif
	{
		lyxerr[Debug::WORKAREA] << "Workarea event: KEYBOARD" << endl;

		KeySym keysym = 0;
		char dummy[1];
		XKeyEvent * xke = reinterpret_cast<XKeyEvent *>(ev);
#if FL_VERSION < 1 && (FL_REVISION < 89 || (FL_REVISION == 89 && FL_FIXLEVEL < 5))
		// XForms < 0.89.5 does not have compose support
		// so we are using our own compose support
		LyXLookupString(ev, dummy, 1, &keysym);
#else
		XLookupString(xke, dummy, 1, &keysym, 0);
//		int num_keys = XLookupString(xke, dummy, 10, &keysym, &xcs);
//		lyxerr << "We have " << num_keys << " keys in the returned buffer" << endl;
//		lyxerr << "Our dummy string is " << dummy << endl;
#endif

		if (lyxerr.debugging(Debug::KEY)) {
			char const * tmp = XKeysymToString(key);
			char const * tmp2 = XKeysymToString(keysym);
			string const stm = (tmp ? tmp : "");
			string const stm2 = (tmp2 ? tmp2 : "");

			lyxerr[Debug::KEY] << "XWorkArea: Key is `" << stm << "' ["
			       << key << "]" << endl;
			lyxerr[Debug::KEY] << "XWorkArea: Keysym is `" << stm2 << "' ["
			       << keysym << "]" << endl;
		}

#if FL_VERSION < 1 && (FL_REVISION < 89 || (FL_REVISION == 89 && FL_FIXLEVEL < 5))
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

		XLyXKeySym * xlk = new XLyXKeySym;
		xlk->initFromKeySym(ret_key);

		area->workAreaKeyPress(LyXKeySymPtr(xlk),
				       x_key_state(ret_state));
	}
	break;

#if FL_VERSION > 0 || FL_REVISION >= 89
	case FL_KEYRELEASE:
		lyxerr[Debug::WORKAREA] << "Workarea event: KEYRELEASE" << endl;
		break;
#endif

	case FL_ENTER:
		lyxerr[Debug::WORKAREA] << "Workarea event: ENTER" << endl;
		break;
	case FL_LEAVE:
		lyxerr[Debug::WORKAREA] << "Workarea event: LEAVE" << endl;
		break;
	case FL_DBLCLICK:
		if (ev) {
			lyxerr[Debug::WORKAREA] << "Workarea event: DBLCLICK" << endl;
			FuncRequest cmd(LFUN_MOUSE_DOUBLE, ev->xbutton.x - ob->x,
							ev->xbutton.y - ob->y,
							x_button_state(ev->xbutton.button));
			area->dispatch(cmd);
		}
		break;
	case FL_TRPLCLICK:
		if (ev) {
			lyxerr[Debug::WORKAREA] << "Workarea event: TRPLCLICK" << endl;
			FuncRequest cmd(LFUN_MOUSE_TRIPLE, ev->xbutton.x - ob->x,
							ev->xbutton.y - ob->y,
							x_button_state(ev->xbutton.button));
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


int XWorkArea::event_cb(XEvent * xev)
{
	int ret = 0;
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
	return ret;
}


void XWorkArea::haveSelection(bool yes) const
{
	if (!yes) {
		XSetSelectionOwner(fl_get_display(), XA_PRIMARY, None, CurrentTime);
		return;
	}

	XSetSelectionOwner(fl_get_display(), XA_PRIMARY, FL_ObjWin(work_area), CurrentTime);
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
			lyxerr << "Received unhandled X11 event" << endl;
			lyxerr << "Type: 0x" << hex << ev.xany.type <<
				" Target: 0x" << hex << ev.xany.window << dec << endl;
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
