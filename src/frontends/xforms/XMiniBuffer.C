// -*- C++ -*-
/**
 * \file XMiniBuffer.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars
 * \author Asger and Juergen
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
	info_suffix_shown_(false)
{
	input_obj_ = create_input_box(FL_NORMAL_INPUT, x, y, h, w);
	suffix_timer_.reset(new Timeout(1500));
	idle_timer_.reset(new Timeout(6000));
	suffix_timer_->timeout.connect(boost::bind(&XMiniBuffer::suffix_timeout, this));
	idle_timer_->timeout.connect(boost::bind(&XMiniBuffer::idle_timeout, this));
	idle_timer_->start(); 
	messageMode();
}


// thanks for nothing, xforms (recursive creation not allowed)
void XMiniBuffer::dd_init()
{
	dropdown_.reset(new DropDown(the_buffer_));
	dropdown_->result.connect(boost::bind(&XMiniBuffer::set_complete_input, this, _1));
	dropdown_->keypress.connect(boost::bind(&XMiniBuffer::append_char, this, _1));
}


XMiniBuffer::~XMiniBuffer()
{
}


int XMiniBuffer::peek_event(FL_OBJECT * ob, int event,
			    int key, XEvent * /*xev*/)
{
#if 0
	if (dropdown_->isVisible()) {
		return dropdown_->peek(xev);
	}
#endif

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
		if (info_suffix_shown_) {
			suffix_timer_->stop();
			suffix_timeout();
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
				show_info_suffix(_("[End of history]"), input);
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
				show_info_suffix(_("[Beginning of history]"), input);
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
				show_info_suffix(_("[no match]"), input);
				break;
			}

			if (comp.empty()) {
				set_input(new_input);
				show_info_suffix(("[only completion]"), new_input + " ");
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
#if 0
			// This will go in again in a little while
			// we need to be able to declare what types
			// of argumetns LFUN's should have first. (Lgb)
			// First check for match
			vector<string>::const_iterator cit =
				find(completion_.begin(),
				     completion_.end(),
				     input);
			if (cit == completion_.end()) {
				// no such func/item
				string const tmp = input + _(" [no match]");
				show_info_suffix(tmp, input);
			} else {
#endif
			messageMode();
			redraw();
			controller_.dispatch(input);
# if 0
			}
#endif
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

 
void XMiniBuffer::show_info_suffix(string const & suffix, string const & input)
{
	stored_input_ = input;
	info_suffix_shown_ = true;
	set_input(input + " " + suffix); 
	suffix_timer_->start();
}
 

void XMiniBuffer::idle_timeout()
{
	set_input(currentState(view_->view()));
}

 
void XMiniBuffer::suffix_timeout()
{
	info_suffix_shown_ = false;
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
