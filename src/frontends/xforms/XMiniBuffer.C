// -*- C++ -*-
/**
 * \file XMiniBuffer.C
 * Read the file COPYING
 *
 * \author Lars
 * \author Asger and Juergen
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "frontends/xforms/DropDown.h"
#include "frontends/xforms/XFormsView.h"
#include "frontends/controllers/ControlCommandBuffer.h"
#include "frontends/Timeout.h"

#include "XMiniBuffer.h"
#include "gettext.h"
#include "debug.h"
#include "bufferview_funcs.h"

#include <boost/bind.hpp>

#include <vector>

#ifndef CXX_GLOBAL_CSTD
using std::isprint;
#endif

using std::endl;
using std::vector;


XMiniBuffer::XMiniBuffer(XFormsView * v, ControlCommandBuffer & control,
	FL_Coord x, FL_Coord y, FL_Coord h, FL_Coord w)
	: controller_(control), view_(v),
	info_shown_(false)
{
	input_obj_ = create_input_box(FL_NORMAL_INPUT, x, y, h, w);
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
	dropdown_.reset(new DropDown(the_buffer_));
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
				show_info(("[only completion]"), new_input + " ");
				break;
			}

			set_input(new_input);

			int x,y,w,h;
			fl_get_wingeometry(fl_get_real_object_window(the_buffer_),
					   &x, &y, &w, &h);

			// asynchronous completion
			int const air = the_buffer_->x;
			x += air;
			y += h - (the_buffer_->h + air);
			w = the_buffer_->w;
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


extern "C" {

	static
	int C_XMiniBuffer_peek_event(FL_OBJECT * ob, int event,
				     FL_Coord, FL_Coord,
				     int key, void * xev)
	{
		XMiniBuffer * mini = static_cast<XMiniBuffer*>(ob->u_vdata);
		return mini->peek_event(ob, event, key,
					static_cast<XEvent *>(xev));
	}
}


FL_OBJECT * XMiniBuffer::create_input_box(int type, FL_Coord x, FL_Coord y,
					  FL_Coord w, FL_Coord h)
{
	FL_OBJECT * obj;

	the_buffer_ = obj = fl_add_input(type, x, y, w, h, "");
	fl_set_object_boxtype(obj, FL_DOWN_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, SouthWestGravity, SouthEastGravity);
	fl_set_object_color(obj, FL_MCOL, FL_MCOL);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);

	// To intercept Up, Down, Table for history
	fl_set_object_prehandler(obj, C_XMiniBuffer_peek_event);
	obj->u_vdata = this;
	obj->wantkey = FL_KEY_TAB;

	return obj;
}


void XMiniBuffer::freeze()
{
	// we must prevent peek_event, or we get an unfocus() when the
	// containing form gets destroyed
	fl_set_object_prehandler(input_obj_, 0);
}


void XMiniBuffer::show_info(string const & info, string const & input, bool append)
{
	stored_input_ = input;
	info_shown_ = true;
	if (append)
		set_input(input + " " + info);
	else
		set_input(info);
	info_timer_->start();
}


void XMiniBuffer::idle_timeout()
{
	set_input(currentState(view_->view().get()));
}


void XMiniBuffer::info_timeout()
{
	info_shown_ = false;
	set_input(stored_input_);
}


bool XMiniBuffer::isEditingMode() const
{
	return the_buffer_->focus;
}


void XMiniBuffer::messageMode(bool on)
{
	set_input("");
	if (!on) {
		fl_activate_object(the_buffer_);
		fl_set_focus_object(view_->getForm(), the_buffer_);
		redraw();
		idle_timer_->stop();
	} else {
		if (isEditingMode()) {
			// focus back to the workarea
			fl_set_focus_object(view_->getForm(), 0);
			idle_timer_->start();
		}
	}
}


void XMiniBuffer::redraw()
{
	fl_redraw_object(the_buffer_);
	XFlush(fl_display);
}


void XMiniBuffer::append_char(char c)
{
	if (!c || !isprint(c))
		return;

	char const * tmp = fl_get_input(the_buffer_);
	string str = tmp ? tmp : "";

	str += c;

	fl_set_input(the_buffer_, str.c_str());
}


void XMiniBuffer::set_complete_input(string const & str)
{
	if (!str.empty()) {
		// add a space so the user can type
		// an argument immediately
		set_input(str + " ");
	}
}


void XMiniBuffer::message(string const & str)
{
	if (!isEditingMode())
		set_input(str);
}


void XMiniBuffer::set_input(string const & str)
{
	fl_set_input(the_buffer_, str.c_str());
}
