// -*- C++ -*-
/**
 * \file XMiniBuffer.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars
 * \author Asger and Juergen
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "XMiniBuffer.h"
#include "ControlCommandBuffer.h"
#include "freebrowser.h"
#include "xforms_helpers.h"

#include "gettext.h"

#include "frontends/Timeout.h"

#include "support/lstrings.h"

#include <boost/bind.hpp>

#include <vector>

#ifndef CXX_GLOBAL_CSTD
using std::isprint;
#endif

using std::endl;
using std::vector;


namespace {

/// This creates the input widget for the minibuffer
FL_OBJECT * create_input_box(void * parent, int type,
			     FL_Coord, FL_Coord, FL_Coord, FL_Coord);

FL_FREEBROWSER * create_freebrowser(void * parent);

} // namespace anon


XMiniBuffer::XMiniBuffer(ControlCommandBuffer & control,
			 FL_Coord x, FL_Coord y, FL_Coord h, FL_Coord w)
	: controller_(control),
	  info_shown_(false)
{
	input_ = create_input_box(this, FL_NORMAL_INPUT, x, y, h, w);
	freebrowser_.reset(create_freebrowser(this), fl_free_freebrowser);
	
	info_timer_.reset(new Timeout(1500));
	idle_timer_.reset(new Timeout(6000));
	info_con = info_timer_->timeout.connect(boost::bind(&XMiniBuffer::info_timeout, this));
	idle_con = idle_timer_->timeout.connect(boost::bind(&XMiniBuffer::idle_timeout, this));
	idle_timer_->start();
	messageMode();
}


// This is here so that scoped ptr will not require a complete type.
XMiniBuffer::~XMiniBuffer()
{}


void XMiniBuffer::freebrowserCB(int action)
{
	if (action < 0 || action > 1)
		// unrecognized action
		return;

	if (action == 0)
		// The freebrowser has been hidden
		return;

	if (freebrowser_->last_printable) {
		// Append this char to the current input contents
		string input = getString(input_);
		input += freebrowser_->last_printable;
		fl_set_input(input_, input.c_str());

	} else {
		// Fill the input widget with the selected
		// browser entry.
		FL_OBJECT * browser = freebrowser_->browser;
		string const str = getString(browser);

		if (!str.empty()) {
			// add a space so the user can type
			// an argument immediately
			set_input(str + ' ');
		}
	}
}


int XMiniBuffer::peek_event(FL_OBJECT * ob, int event,
			    int key, XEvent * /*xev*/)
{
	switch (event) {
	case FL_FOCUS:
		messageMode(false);
		break;
	case FL_UNFOCUS:
		messageMode();
		break;
	case FL_KEYBOARD:
	{
		string input;
		if (info_shown_) {
			info_timer_->stop();
			info_timeout();
		}

		char const * tmp = fl_get_input(ob);
		input = tmp ? tmp : "";

		switch (key) {
		case XK_Down:
#ifdef XK_KP_Down
		case XK_KP_Down:
#endif
		{
			string const h(controller_.historyDown());
			if (h.empty()) {
				show_info(_("[End of history]"), input, false);
			} else {
				set_input(h);
			}
			return 1;
		}

		case XK_Up:
#ifdef XK_KP_Up
		case XK_KP_Up:
#endif
		{
			string const h(controller_.historyUp());
			if (h.empty()) {
				show_info(_("[Beginning of history]"), input, false);
			} else {
				set_input(h);
			}
			return 1;
		}

		case 9:
		case XK_Tab:
		{
			string new_input;
			vector<string> comp = controller_.completions(input, new_input);

			if (comp.empty() && new_input == input) {
				show_info(_("[no match]"), input);
				break;
			}

			if (comp.empty()) {
				set_input(new_input);
				show_info(_("[only completion]"), new_input + ' ');
				break;
			}

			set_input(new_input);

			// Fill freebrowser_'s browser with the list of
			// available completions
			FL_OBJECT * browser = freebrowser_->browser;
			fl_clear_browser(browser);
			vector<string>::const_iterator cit = comp.begin();
			vector<string>::const_iterator end = comp.end();
			for (; cit != end; ++cit) {
				fl_add_browser_line(browser, cit->c_str());
			}
			fl_select_browser_line(browser, 1);

			// Set the position of the freebrowser and display it.
			int x,y,w,h;
			fl_get_wingeometry(fl_get_real_object_window(input_),
					   &x, &y, &w, &h);

			// asynchronous completion
			int const air = input_->x;
			x += air;
			y += h - (input_->h + air);
			w = input_->w;
			h = 100;

			fl_show_freebrowser(freebrowser_.get(), x, y-h, w, h);
			return 1;
		}
		case 27:
		case XK_Escape:
			messageMode();
			return 1;
		case 13:
		case XK_Return:
#ifdef XK_KP_Enter
		case XK_KP_Enter:
#endif
		{
			messageMode();
			redraw();
			controller_.dispatch(input);
			return 1;
		}
		default:
			return 0;
		}
	}
	default:
		break;
	}

	return 0;
}


void XMiniBuffer::freeze()
{
	// we must prevent peek_event, or we get an unfocus() when the
	// containing form gets destroyed
	fl_set_object_prehandler(input_, 0);
}


void XMiniBuffer::show_info(string const & info, string const & input, bool append)
{
	stored_input_ = input;
	info_shown_ = true;
	if (append)
		set_input(input + ' ' + info);
	else
		set_input(info);
	info_timer_->start();
}


void XMiniBuffer::idle_timeout()
{
	set_input(controller_.getCurrentState());
}


void XMiniBuffer::info_timeout()
{
	info_shown_ = false;
	set_input(stored_input_);
}


bool XMiniBuffer::isEditingMode() const
{
	return input_->focus;
}


void XMiniBuffer::messageMode(bool on)
{
	set_input("");
	if (!on) {
		fl_activate_object(input_);
		fl_set_focus_object(input_->form, input_);
		redraw();
		idle_timer_->stop();
	} else {
		if (isEditingMode()) {
			// focus back to the workarea
			fl_set_focus_object(input_->form, 0);
			idle_timer_->start();
		}
	}
}


void XMiniBuffer::redraw()
{
	fl_redraw_object(input_);
	XFlush(fl_display);
}


void XMiniBuffer::message(string const & str)
{
	if (!isEditingMode())
		set_input(str);
}


void XMiniBuffer::set_input(string const & str)
{
	fl_set_input(input_, str.c_str());
}


namespace {

extern "C"
int C_XMiniBuffer_peek_event(FL_OBJECT * ob, int event,
			     FL_Coord, FL_Coord,
			     int key, void * xev)
{
	XMiniBuffer * mini = static_cast<XMiniBuffer*>(ob->u_vdata);
	return mini->peek_event(ob, event, key, static_cast<XEvent *>(xev));
}


extern "C"
void C_freebrowserCB(FL_FREEBROWSER * fb, int action)
{
	if (!fb || !fb->parent)
		return;

	XMiniBuffer * ptr = static_cast<XMiniBuffer *>(fb->parent);
	ptr->freebrowserCB(action);
}


FL_OBJECT * create_input_box(void * parent, int type,
			     FL_Coord x, FL_Coord y, FL_Coord w, FL_Coord h)
{
	FL_OBJECT * obj = fl_add_input(type, x, y, w, h, "");
	fl_set_object_boxtype(obj, FL_DOWN_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, SouthWestGravity, SouthEastGravity);
	fl_set_object_color(obj, FL_MCOL, FL_MCOL);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);

	// To intercept Up, Down, Table for history
	fl_set_object_prehandler(obj, C_XMiniBuffer_peek_event);
	obj->u_vdata = parent;
	obj->wantkey = FL_KEY_TAB;

	return obj;
}


FL_FREEBROWSER * create_freebrowser(void * parent)
{
	FL_FREEBROWSER * fb = fl_create_freebrowser(parent);
	fb->want_printable = 1;
	fb->callback = C_freebrowserCB;
	return fb;
}

} // namespace anon
