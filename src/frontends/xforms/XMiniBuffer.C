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
#include "frontends/Timeout.h"

#include "XMiniBuffer.h"
#include "BufferView.h"
#include "LyXAction.h"
#include "gettext.h"
#include "support/lyxalgo.h"
#include "support/lstrings.h"

#include <boost/bind.hpp>

#include <vector>
#include <cctype>

#ifndef CXX_GLOBAL_CSTD
using std::isprint;
#endif

using std::vector;


namespace {

struct prefix {
	string p;
	prefix(string const & s)
		: p(s) {}
	bool operator()(string const & s) const {
		return prefixIs(s, p);
	}
};

} // end of anon namespace


XMiniBuffer::XMiniBuffer(LyXView * o, FL_Coord x, FL_Coord y,
			 FL_Coord h, FL_Coord w)
	: MiniBuffer(o)
{
	create_input_box(FL_NORMAL_INPUT, x, y, h, w);
	messageMode();
}


// thanks for nothing, xforms (recursive creation not allowed)
void XMiniBuffer::dd_init()
{
	dropdown_ = new DropDown(owner_, the_buffer);
	dropdown_->result.connect(boost::bind(&XMiniBuffer::set_complete_input, this, _1));
	dropdown_->keypress.connect(boost::bind(&XMiniBuffer::append_char, this, _1));
}


XMiniBuffer::~XMiniBuffer()
{
	delete dropdown_;
}


int XMiniBuffer::peek_event(FL_OBJECT * ob, int event, int key, XEvent * xev)
{
#if 0
	if (dropdown_->isVisible()) {
		return dropdown_->peek(xev);
	}
#endif
	
	switch (event) {
	case FL_UNFOCUS:
		messageMode();
		break;
	case FL_KEYBOARD:
	{
		string input;
		if (information_displayed_) {
			information_timer_->stop();
			input = stored_input;
			restore_input();
		} else {
			char const * tmp = fl_get_input(ob);
			input = tmp ? tmp : "";
		}
		  

		switch (key) {
		case XK_Down:
			if (hist_iter != history_->end()) {
				++hist_iter;
			}
			if (hist_iter == history_->end()) {
				// no further history
				show_information(_("[End of history]"), input);
			} else {
				set_input((*hist_iter));
			}
			return 1;
		case XK_Up:
			if (hist_iter == history_->begin()) {
				// no further history
				show_information(_("[Beginning of history]"),
						 input);
			} else {
				--hist_iter;
				set_input((*hist_iter));
			}
			return 1;
		case 9:
		case XK_Tab:
		{
			// Completion handling.

			vector<string> comp;
			lyx::copy_if(completion_.begin(),
				     completion_.end(),
				     back_inserter(comp), prefix(input));

			if (comp.empty()) {
				// No matches
				string const tmp = input + _(" [no match]");
				show_information(tmp, input);
			} else if (comp.size() == 1) {
				// Perfect match
				string const tmp =
					comp[0] + _(" [sole completion]");
				show_information(tmp, comp[0] + " ");
			} else {
				// More that one match
				// Find maximal avaliable prefix
				string const tmp = comp[0];
				string test(input);
				if (tmp.length() > test.length())
					test += tmp[test.length()];
				while (test.length() < tmp.length()) {
					vector<string> vtmp;
					lyx::copy_if(comp.begin(),
						     comp.end(),
						     back_inserter(vtmp),
						     prefix(test));
					if (vtmp.size() != comp.size()) {
						test.erase(test.length() - 1);
						break;
					}
					test += tmp[test.length()];
				}
				set_input(test);

				int x,y,w,h;
				fl_get_wingeometry(fl_get_real_object_window(the_buffer),
						   &x, &y, &w, &h);

				// asynchronous completion
				int const air = the_buffer->x;
				x += air;
				y += h - (the_buffer->h + air);
				w = the_buffer->w;
				dropdown_->select(comp, x, y, w);
			}
			return 1;
		}
		case 27:
		case XK_Escape:
			// Abort
#if 0
			owner_->view()->focus(true);
#endif
			message_timeout();
			messageMode();
			//escape.emit();
			return 1;
		case 13:
		case XK_Return:
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
				show_information(tmp, input);
			} else {
#endif
				// Return the inputted string
				messageMode();
#if 0
				owner_->view()->focus(true);
#endif
				if (!input.empty()) {
					history_->push_back(input);
				}
				inputReady(input);
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
		//lyxerr << "Unhandled minibuffer event!" << endl;
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

	the_buffer = obj = fl_add_input(type, x, y, w, h, text.c_str());
	fl_set_object_boxtype(obj, FL_DOWN_BOX);
	fl_set_object_resize(obj, FL_RESIZE_ALL);
	fl_set_object_gravity(obj, SouthWestGravity, SouthEastGravity);
	fl_set_object_color(obj, FL_MCOL, FL_MCOL);
	fl_set_object_lsize(obj, FL_NORMAL_SIZE);

	// To intercept Up, Down, Table for history
	fl_set_object_prehandler(obj, C_XMiniBuffer_peek_event);
	obj->u_vdata = this;
	obj->wantkey = FL_KEY_TAB;

	set_input(text);

	return obj;
}


bool XMiniBuffer::isEditingMode() const
{
	return the_buffer->focus;
}


void XMiniBuffer::editingMode()
{
	fl_activate_object(the_buffer);
	fl_set_focus_object(static_cast<XFormsView *>(owner_)->getForm(),
			    the_buffer);
	redraw();
}


void XMiniBuffer::messageMode()
{
	fl_deactivate_object(the_buffer);
	redraw();
}


void XMiniBuffer::redraw()
{
	fl_redraw_object(the_buffer);
	XFlush(fl_display);
}


void XMiniBuffer::append_char(char c)
{
	if (!c || !isprint(c))
		return;

	char const * tmp = fl_get_input(the_buffer);
	string str = tmp ? tmp : "";

	str += c;

	fl_set_input(the_buffer, str.c_str());
}


void XMiniBuffer::set_input(string const & str)
{
	fl_set_input(the_buffer, str.c_str());
}
