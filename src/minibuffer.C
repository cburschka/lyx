/* ###########################################################################
 *
 *                 The MiniBuffer Class
 *                 read minibuffer.h for more
 *                 information.
 * 
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-1999 The LyX Team.  
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

extern bool keyseqUncomplete();
extern string keyseqOptions(int l=190);
extern string keyseqStr(int l=190);
extern LyXAction lyxaction;

void MiniBuffer::TimerCB(FL_OBJECT *, long tmp)
{
	MiniBuffer *obj= (MiniBuffer*)tmp;
	obj->Init();
}


void MiniBuffer::ExecutingCB(FL_OBJECT *ob, long)
{
	MiniBuffer *obj = (MiniBuffer*)ob->u_vdata;
	lyxerr.debug() << "Getting ready to execute: " << obj->cur_cmd << endl;
	fl_set_focus_object(obj->owner->getForm(),
			    obj->owner->currentView()->getWorkArea());
	if (obj->cur_cmd.empty()) { 
		obj->Init();
		return ; 
	}
	obj->Set(_("Executing:"), obj->cur_cmd);
	obj->addHistory(obj->cur_cmd);
	
	// Split command into function and argument
	// This is done wrong Asger. Instead of <function argument>
	// it ends up as <argument function> Queer solution:
	string arg = obj->cur_cmd;
	string function;
	if (contains(arg, " ")) {
		arg = split(arg, function, ' ');
		function = strip(function);
	} else {
		function = arg;
		arg.erase();
	}
	lyxerr.debug() << "Function: " << function
		       << "\nArg     : " << arg << endl;
	// Check if the name is valid (ale)
	// No, let the dispatch functions handle that.
	//int action = lyxaction.LookupFunc(function.c_str());
	//lyxerr.debug(string("minibuffer action: ") + action);
	//if (action>=0) {
	    // Dispatch only returns requested data for a few commands (ale)
	string res=obj->owner->getLyXFunc()->Dispatch(function.c_str(),
						       arg.c_str());
	lyxerr.debug() << "Minibuffer Res: " << res << endl;
/*	if (!res.empty())
		if(obj->owner->getLyXFunc()->errorStat())
			obj->Set(_("Error:"), _(res.c_str()), string(), 4);
		else
			obj->Set(_("Result:"), _(res.c_str()), string(), 4);
	else
		obj->Init();
*/
	//} else {
#ifdef WITH_WARNINGS
#warning Look at this.
#endif
	//obj->Set(_("Cannot find function"), function, "!");
	obj->shows_no_match = false;
	//}

	return ;
}


void MiniBuffer::ExecCommand()
{
	text.erase();
	fl_set_input(the_buffer, "");
	fl_set_focus_object(owner->getForm(),the_buffer);
}


FL_OBJECT *MiniBuffer::add(int type, FL_Coord x, FL_Coord y,
			   FL_Coord w, FL_Coord h)
{
	FL_OBJECT *obj;
	
	the_buffer = obj = fl_add_input(type,x,y,w,h,text.c_str());
        fl_set_object_boxtype(obj,FL_DOWN_BOX);
        fl_set_object_resize(obj, FL_RESIZE_ALL);
        fl_set_object_gravity(obj, SouthWestGravity, SouthEastGravity);
        fl_set_object_color(obj,FL_MCOL,FL_MCOL);
        fl_set_object_lsize(obj,FL_NORMAL_SIZE);
	fl_set_object_callback(obj,ExecutingCB, 0);

	// To intercept Up, Down, Table for history
        fl_set_object_prehandler(obj, peek_event);
        obj->u_vdata = (void*)this;
        obj->wantkey = FL_KEY_TAB;
	
	// timer
	timer = fl_add_timer(FL_HIDDEN_TIMER, 0,0,0,0, "Timer");
	fl_set_object_callback(timer, TimerCB, (long)this);
	fl_set_input(the_buffer, text.c_str());

	return obj;
}


// Added optional arg `delay_secs', defaults to 4.
//When 0, no timeout is done. RVDK_PATCH_5 
void MiniBuffer::Set(string const& s1, string const& s2,
		     string const& s3, int delay_secs)
{
	setTimer(delay_secs);

	string ntext = strip(s1 + ' ' + s2 + ' ' + s3);

	if (!the_buffer->focus) {
		fl_set_input(the_buffer, ntext.c_str());
		XFlush(fl_display);
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
	else if (owner->currentView()->available()) {
			string nicename =
				MakeDisplayPath(owner->currentBuffer()->
						getFileName());
			// Should we do this instead? (kindo like emacs)
			// leaves more room for other information
			text = "LyX: ";
			text += nicename;
			if (owner->currentBuffer()->lyxvc.inUse()) {
				text += " [RCS:";
				text += owner->currentBuffer()->lyxvc.getVersion();
				text += ' ';
				text += owner->currentBuffer()->lyxvc.getLocker();
				if (owner->currentBuffer()->isReadonly())
					text += " (RO)";
				text += ']';
			} else if (owner->currentBuffer()->isReadonly())
				text += " [RO]";
			if (!owner->currentBuffer()->isLyxClean())
				text += _(" (Changed)");
	} else {
		if (text != _("Welcome to LyX!")) // this is a hack
			text = _("* No document open *");
	}
	

	fl_set_input(the_buffer, text.c_str());
	setTimer(0);
	XFlush(fl_display);
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


// This is not as dirty as it seems, the hidden buttons removed by this
// function were just kludges for an uncomplete keyboard callback (ale)
int MiniBuffer::peek_event(FL_OBJECT *ob, int event, FL_Coord, FL_Coord,
			   int key, void */*xev*/)
{
	MiniBuffer *mini = (MiniBuffer*)ob->u_vdata;
	
	if (event==FL_KEYBOARD){
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
			const char *s = lyxaction.getApproxFuncName(fl_get_input(ob));
			if (s && s[0])
				fl_set_input(ob, s);
			return 1; 
		}
		case 27:
		case XK_Escape:
			// Abort
			fl_set_focus_object(mini->owner->getForm(),
					    mini->owner->currentView()->getWorkArea());
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
	}
	return 0;
}

