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
#include "DropDown.h"
#include "ControlCommandBuffer.h"

#include "gettext.h"

#include "frontends/Timeout.h"

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

} // namespace anon


XMiniBuffer::XMiniBuffer(ControlCommandBuffer & control,
			 FL_Coord x, FL_Coord y, FL_Coord h, FL_Coord w)
	: controller_(control),
	  info_shown_(false)
{
	input_ = create_input_box(this, FL_NORMAL_INPUT, x, y, h, w);
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


// thanks for nothing, xforms (recursive creation not allowed)
void XMiniBuffer::dd_init()
{
	dropdown_.reset(new DropDown(input_));
	result_con = dropdown_->result.connect(boost::bind(&XMiniBuffer::set_complete_input, this, _1));
	keypress_con = dropdown_->keypress.connect(boost::bind(&XMiniBuffer::append_char, this, _1));
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

			int x,y,w,h;
			fl_get_wingeometry(fl_get_real_object_window(input_),
					   &x, &y, &w, &h);

			// asynchronous completion
			int const air = input_->x;
			x += air;
			y += h - (input_->h + air);
			w = input_->w;
			dropdown_->select(comp, x, y, w);
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


void XMiniBuffer::append_char(char c)
{
	if (!c || !isprint(c))
		return;

	char const * tmp = fl_get_input(input_);
	string str = tmp ? tmp : "";

	str += c;

	fl_set_input(input_, str.c_str());
}


void XMiniBuffer::set_complete_input(string const & str)
{
	if (!str.empty()) {
		// add a space so the user can type
		// an argument immediately
		set_input(str + ' ');
	}
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

} // namespace anon
