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

#include "minibuffer.h"

#include "support/lyxalgo.h"
#include "support/filetools.h"
#include "LyXView.h"
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


void MiniBuffer::stored_slot() 
{
	if (stored_) {
		stored_ = false;
		fl_set_input(the_buffer, stored_input.c_str());
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
	case FL_KEYBOARD:
	{
		char const * tmp = fl_get_input(ob);
		string input = tmp ? tmp : "";
		if (stored_) {
			stored_timer.stop();
			input = stored_input;
			fl_set_input(ob, input.c_str());
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
				fl_set_input(ob, _("[End of history]"));
			} else {
				fl_set_input(ob, (*hist_iter).c_str());
			}
			return 1; 
		case XK_Up:
			if (hist_iter == history_->begin()) {
				// no further history
				stored_set(input);
				fl_set_input(ob, _("[Beginning of history]"));
			} else {
				--hist_iter;
				fl_set_input(ob, (*hist_iter).c_str());
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
				fl_set_input(ob, tmp.c_str());
			} else if (comp.size() == 1) {
				// Perfect match
				string const tmp =
					comp[0] + _(" [sole completion]");
				stored_set(comp[0]);
				fl_set_input(ob, tmp.c_str());
			} else {
				// More that one match
				// Find maximal avaliable prefix
				string const tmp = comp[0];
				string test(input);
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
				fl_set_input(ob, test.c_str());
				
				// How should the possible matches
				// be visualized?
				std::copy(comp.begin(), comp.end(),
					  std::ostream_iterator<string>(std::cerr, "\n"));
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
				fl_set_input(ob, tmp.c_str());
			} else {
#endif
				// Return the inputted string
				deactivate();
				owner_->view()->focus(true);
				history_->push_back(input);
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
				fl_set_input(ob, tmp.c_str());
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


extern "C"
int C_MiniBuffer_peek_event(FL_OBJECT * ob, int event, 
			    FL_Coord, FL_Coord,
			    int key, void * /*xev*/)
{
	MiniBuffer * mini = static_cast<MiniBuffer*>(ob->u_vdata);
	return mini->peek_event(ob, event, key);
}


void MiniBuffer::prepare()
{
	text.erase();
	fl_set_input(the_buffer, "");
	activate();
	fl_set_focus_object(owner_->getForm(), the_buffer);
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

	fl_set_input(the_buffer, text.c_str());
	
	return obj;
}


void MiniBuffer::message(string const & str) 
{
	timer.restart();
	string const ntext = strip(str);
	if (!the_buffer->focus) {
		fl_set_input(the_buffer, ntext.c_str());
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
	fl_redraw_object(the_buffer);
}


void MiniBuffer::deactivate()
{
	fl_redraw_object(the_buffer);
	fl_deactivate_object(the_buffer);
}


void MiniBuffer::redraw() 
{
	fl_redraw_object(the_buffer);
}
