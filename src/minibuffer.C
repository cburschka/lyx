/* ###########################################################################
 *
 *                 The MiniBuffer Class
 *                 read minibuffer.h for more
 *                 information.
 * 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2001 The LyX Team.  
 * 
 * ###########################################################################
 */

#include <config.h>

#include <iostream>

#ifdef __GNUG__
#pragma implementation
#endif

// FIXME: temporary 
#include "frontends/xforms/DropDown.h"
 
#include "minibuffer.h"

#include "support/lyxalgo.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "LyXView.h"
#include "XFormsView.h"
#include "gettext.h"
#include "LyXAction.h"
#include "BufferView.h"


using SigC::slot;
using std::vector;

extern LyXAction lyxaction;


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


MiniBuffer::MiniBuffer(LyXView * o, FL_Coord x, FL_Coord y,
		       FL_Coord h, FL_Coord w)
	: stored_(false), owner_(o), state_(spaces)
{
	add(FL_NORMAL_INPUT, x, y, h, w);
	timer.setTimeout(6000);
	timer.timeout.connect(slot(this, &MiniBuffer::init));
	stored_timer.setTimeout(1500);
	stored_timer.timeout.connect(slot(this, &MiniBuffer::stored_slot));
	deactivate();
}

 
// thanks for nothing, xforms (recursive creation not allowed) 
void MiniBuffer::dd_init()
{
	dropdown_ = new DropDown(owner_, the_buffer);
	dropdown_->result.connect(slot(this, &MiniBuffer::set_complete_input));
	dropdown_->keypress.connect(slot(this, &MiniBuffer::append_char));
}


MiniBuffer::~MiniBuffer()
{
	delete dropdown_;
}

 
void MiniBuffer::stored_slot() 
{
	if (stored_) {
		stored_ = false;
		set_input(stored_input);
	}
}


void MiniBuffer::stored_set(string const & str) 
{
	stored_input = str;
	stored_ = true;
	stored_timer.start();
}


int MiniBuffer::peek_event(FL_OBJECT * ob, int event, int key)
{
	switch (event) {
	case FL_UNFOCUS:
		deactivate();
		break;
	case FL_KEYBOARD:
	{
		char const * tmp = fl_get_input(ob);
		string input = tmp ? tmp : "";
		if (stored_) {
			stored_timer.stop();
			input = stored_input;
			set_input(input);
			stored_ = false;
		}
		
		switch (key) {
		case XK_Down:
			if (hist_iter != history_->end()) {
				++hist_iter;
			}
			if (hist_iter == history_->end()) {
				// no further history
				stored_set(input);
				set_input(_("[End of history]"));
			} else {
				set_input((*hist_iter));
			}
			return 1; 
		case XK_Up:
			if (hist_iter == history_->begin()) {
				// no further history
				stored_set(input);
				set_input(_("[Beginning of history]"));
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
				     std::back_inserter(comp), prefix(input));

			if (comp.empty()) {
				// No matches
				string const tmp = input + _(" [no match]");
				stored_set(input);
				set_input(tmp);
			} else if (comp.size() == 1) {
				// Perfect match
				string const tmp =
					comp[0] + _(" [sole completion]");
				stored_set(comp[0] + " ");
				set_input(tmp);
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
						     std::back_inserter(vtmp),
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
			owner_->view()->focus(true);
			init();
			deactivate();
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
				std::find(completion_.begin(),
					  completion_.end(),
					  input);
			if (cit == completion_.end()) {
				// no such func/item
				stored_set(input);
				string const tmp = input + _(" [no match]");
				set_input(tmp);
			} else {
#endif
				// Return the inputted string
				deactivate();
				owner_->view()->focus(true);
				if (!input.empty()) {
					history_->push_back(input);
				}
				stringReady.emit(input);
# if 0
			}
#endif
			return 1;
		}
		case XK_space:
		{
			// Depending on the input state spaces might not
			// be allowed.
			switch (state_) {
			case spaces:
				return 0;
			case nospaces:
			{
				stored_set(input);
				string const tmp = input + _(" [no match]");
				set_input(tmp);
				return 1;
			}
			}
			
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
	int C_MiniBuffer_peek_event(FL_OBJECT * ob, int event, 
				    FL_Coord, FL_Coord,
				    int key, void * /*xev*/)
	{
		MiniBuffer * mini = static_cast<MiniBuffer*>(ob->u_vdata);
		return mini->peek_event(ob, event, key);
	}
	
}


void MiniBuffer::prepare()
{
	text.erase();
	set_input("");
	activate();
	fl_set_focus_object(static_cast<XFormsView *>(owner_)->getForm(),
			    the_buffer);
}


FL_OBJECT * MiniBuffer::add(int type, FL_Coord x, FL_Coord y,
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
        fl_set_object_prehandler(obj, C_MiniBuffer_peek_event);
        obj->u_vdata = this;
        obj->wantkey = FL_KEY_TAB;

	set_input(text);
	
	return obj;
}


void MiniBuffer::message(string const & str) 
{
	timer.restart();
	string const ntext = strip(str);
	if (!the_buffer->focus) {
		set_input(ntext);
		text = ntext;
	}
}


void MiniBuffer::messagePush(string const & str) 
{
	text_stored = text;
	message(str);
}


void MiniBuffer::messagePop()
{
	if (!text_stored.empty()) {
		message(text_stored);
		text_stored.erase();
	}
}


void MiniBuffer::addSet(string const & s1, string const & s2)
{
	string const str = text + ' ' +  s1 + ' ' + s2;
	message(str);
}


void MiniBuffer::getString(State spaces,
			   vector<string> const & completion,
			   vector<string> & history)
{
	state_ = spaces;
	completion_ = completion;
	history_ = &history;
	hist_iter = history_->end();
	prepare();
}


void MiniBuffer::init()
{
	// If we have focus, we don't want to change anything.
	if (the_buffer->focus)
		return;

	timeout.emit();
	timer.stop();
}


void MiniBuffer::activate()
{
	fl_activate_object(the_buffer);
	redraw();
}


void MiniBuffer::deactivate()
{
	redraw();
	fl_deactivate_object(the_buffer);
	XFlush(fl_display);
}


void MiniBuffer::redraw() 
{
	fl_redraw_object(the_buffer);
	XFlush(fl_display);
}


void MiniBuffer::set_complete_input(string const & str)
{
	if (!str.empty()) {
		// add a space so the user can type
		// an argument immediately
		set_input(str + " ");
	}
}

 
void MiniBuffer::append_char(char c)
{
	if (!c || !isprint(c))
		return;

	char const * tmp = fl_get_input(the_buffer);
	string str = tmp ? tmp : "";

	str += c;

	fl_set_input(the_buffer, str.c_str());
}

 
void MiniBuffer::set_input(string const & str)
{
	fl_set_input(the_buffer, str.c_str());
	XFlush(fl_display);
}
