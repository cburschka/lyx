/* ###########################################################################
 *
 *                 The MiniBuffer Class
 *                 read minibuffer.h for more
 *                 information.
 * 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.  
 * 
 * ###########################################################################
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation "minibuffer.h"
#endif

#include "support/filetools.h"
#include "lyx_main.h" 
#include "lyxfunc.h"
#include FORMS_H_LOCATION
#include "minibuffer.h"  
#include "LyXView.h"
#include "debug.h"
#include "gettext.h"
#include "LyXAction.h"
#include "BufferView.h"
#include "buffer.h"

using std::endl;

extern bool keyseqUncomplete();
extern string keyseqOptions(int l = 190);
extern string keyseqStr(int l = 190);
extern LyXAction lyxaction;

MiniBuffer::MiniBuffer(LyXView * o, FL_Coord x, FL_Coord y,
		       FL_Coord h, FL_Coord w)
	: owner(o)
{
	text = _("Welcome to LyX!");
	shows_no_match = true;
	history_idx = history_cnt = 0;
	add(FL_NORMAL_INPUT, x, y, h, w);
	timer.timeout.connect(slot(this, &MiniBuffer::Init));
}


void MiniBuffer::ExecutingCB(FL_OBJECT * ob, long)
{
	MiniBuffer * obj = static_cast<MiniBuffer*>(ob->u_vdata);
	lyxerr.debug() << "Getting ready to execute: " << obj->cur_cmd << endl;
	obj->owner->view()->focus(true);

	if (obj->cur_cmd.empty()) { 
		obj->Init();
		return ; 
	}
	obj->Set(_("Executing:"), obj->cur_cmd);
	obj->addHistory(obj->cur_cmd);
	
	// Dispatch only returns requested data for a few commands (ale)
	string res = obj->owner->getLyXFunc()->Dispatch(obj->cur_cmd);
	lyxerr.debug() << "Minibuffer Res: " << res << endl;
	obj->shows_no_match = false;

	return ;
}


extern "C" void C_MiniBuffer_ExecutingCB(FL_OBJECT * ob, long)
{
	MiniBuffer * obj = static_cast<MiniBuffer*>(ob->u_vdata);
	obj->Init();
}


// This is not as dirty as it seems, the hidden buttons removed by this
// function were just kludges for an uncomplete keyboard callback (ale)
int MiniBuffer::peek_event(FL_OBJECT * ob, int event, FL_Coord, FL_Coord,
			   int key, void */*xev*/)
{
	MiniBuffer * mini = static_cast<MiniBuffer*>(ob->u_vdata);

	switch (event) {
	case FL_KEYBOARD:
		switch (key) {
		case XK_Down:
			mini->history_idx++;
			if (!mini->getHistory().empty()) {
				fl_set_input(ob, mini->getHistory().c_str());
			} else
				mini->history_idx--;
			return 1; 
		case XK_Up:
			if (mini->history_idx > 0) mini->history_idx--;
			fl_set_input(ob, mini->getHistory().c_str());
			return 1; 
		case 9:
		case XK_Tab:
		{
			// complete or increment the command
			string s(lyxaction.getApproxFuncName(fl_get_input(ob)));
			if (!s.empty())
				fl_set_input(ob, s.c_str());
			return 1; 
		}
		case 27:
		case XK_Escape:
			// Abort
			mini->owner->view()->focus(true);
			mini->Init();
			return 1; 
		case 13:
		case XK_Return:
			// Execute a command. 
			mini->cur_cmd = string(fl_get_input(ob));
			ExecutingCB(ob, 0);
			return 1;
		default:
			return 0;
		}
	case FL_PUSH:
		// This actually clears the buffer.
		mini->PrepareForCommand();
		return 1;
	case FL_DRAW:
		lyxerr << "Minibuffer event: DRAW" << endl;
		break;
	default:
		lyxerr << "Unhandled minibuffer event!" << endl;
		break;
	}
	

	return 0;
}


extern "C" int C_MiniBuffer_peek_event(FL_OBJECT * ob, int event, 
				       FL_Coord, FL_Coord,
				       int key, void * xev)
{
	return MiniBuffer::peek_event(ob, event, 0, 0, key, xev);
}


void MiniBuffer::PrepareForCommand()
{
	text.erase();
	fl_set_input(the_buffer, "");
	fl_set_focus_object(owner->getForm(), the_buffer);
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
	fl_set_object_callback(obj, C_MiniBuffer_ExecutingCB, 0);

	// To intercept Up, Down, Table for history
        fl_set_object_prehandler(obj, C_MiniBuffer_peek_event);
        obj->u_vdata = this;
        obj->wantkey = FL_KEY_TAB;

	fl_set_input(the_buffer, text.c_str());
	
	return obj;
}


// Added optional arg `delay_secs', defaults to 4.
//When 0, no timeout is done. RVDK_PATCH_5 
void MiniBuffer::Set(string const& s1, string const& s2,
		     string const& s3, unsigned int delay_secs)
{
	if (delay_secs)
		timer.setTimeout(delay_secs * 1000).restart();
	else
		timer.stop();
	
	string ntext = strip(s1 + ' ' + s2 + ' ' + s3);

	if (!the_buffer->focus) {
		fl_set_input(the_buffer, ntext.c_str());
		XFlush(fl_get_display());
		text = ntext;
	}
}


void MiniBuffer::Init()
{
	// If we have focus, we don't want to change anything.
	if (the_buffer->focus)
		return;

	// When meta-fake key is pressed, show the key sequence so far + "M-".
	if (owner->getLyXFunc()->wasMetaKey()) {
		text = owner->getLyXFunc()->keyseqStr();
		text += " M-";
	}

	// Else, when a non-complete key sequence is pressed,
	// show the available options.
	else if (owner->getLyXFunc()->keyseqUncomplete()) 
		text = owner->getLyXFunc()->keyseqOptions();
   
	// Else, show the buffer state.
	else if (owner->view()->available()) {
			string nicename = 
				MakeDisplayPath(owner->buffer()->
						fileName());
			// Should we do this instead? (kindo like emacs)
			// leaves more room for other information
			text = "LyX: ";
			text += nicename;
			if (owner->buffer()->lyxvc.inUse()) {
				text += " [";
				text += owner->buffer()->lyxvc.version();
				text += ' ';
				text += owner->buffer()->lyxvc.locker();
				if (owner->buffer()->isReadonly())
					text += " (RO)";
				text += ']';
			} else if (owner->buffer()->isReadonly())
				text += " [RO]";
			if (!owner->buffer()->isLyxClean())
				text += _(" (Changed)");
	} else {
		if (text != _("Welcome to LyX!")) // this is a hack
			text = _("* No document open *");
	}
	

	fl_set_input(the_buffer, text.c_str());

	timer.stop();

	XFlush(fl_get_display());
}


// allows to store and reset the contents one time. Usefull for
// status messages like "load font" (Matthias)
void MiniBuffer::Store()
{
	text_stored = fl_get_input(the_buffer);
}


void MiniBuffer::Reset()
{
	if (!text_stored.empty()){
		Set(text_stored);
		text_stored.erase();
	}
}


void MiniBuffer::Activate()
{
	fl_activate_object(the_buffer);
	fl_redraw_object(the_buffer);
}


void MiniBuffer::Deactivate()
{
	fl_deactivate_object(the_buffer);
}
