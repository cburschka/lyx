/* This file is part of
 * ====================================================== 
 * 
 *           LyX, The Document Processor
 *
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file is Copyright 1996-1998
 *           Lars Gullik Bjønnes
 *
 * ====================================================== */

//  Added pseudo-action handling, asierra 180296

#include <config.h>

#ifdef __GNUG__
#pragma implementation "toolbar.h"
#endif

#include "lyx_main.h"
#include "lyx_gui_misc.h"
#include "lyx.h"
#include "toolbar.h"
#include "lyxfunc.h"
#include "lyxlex.h"
#include "debug.h"
#include "combox.h"
#include "LyXView.h"
#include "LyXAction.h"
#include "support/lstrings.h"
#include "support/filetools.h"
#include "lyxrc.h"

using std::endl;

// this one is not "C" because combox callbacks are really C++ %-|
extern void LayoutsCB(int, void *);
extern char const ** get_pixmap_from_symbol(char const * arg, int, int);
extern LyXAction lyxaction;


Toolbar::Toolbar(LyXView * o, int x, int y)
	: owner(o), sxpos(x), sypos(y)
{
	combox = 0;
#if FL_REVISION < 89
	bubble_timer = 0;
#endif
	reset();

	// extracts the default toolbar actions from LyXRC
	for (ToolbarDefaults::const_iterator cit =
		     lyxrc.toolbardefaults.begin();
	     cit != lyxrc.toolbardefaults.end(); ++cit) {
		add((*cit));
		lyxerr[Debug::TOOLBAR] << "tool action: "
				       << (*cit) << endl;
	}
}


#if FL_REVISION < 89
// timer-cb for bubble-help (Matthias)
void Toolbar::BubbleTimerCB(FL_OBJECT *, long data)
{
	FL_OBJECT * ob = reinterpret_cast<FL_OBJECT*>(data);
	char * help = static_cast<char*>(ob->u_vdata);
	fl_show_oneliner(help, ob->form->x + ob->x,
			 ob->form->y + ob->y + ob->h);
}


extern "C" void C_Toolbar_BubbleTimerCB(FL_OBJECT * ob, long data)
{
	Toolbar::BubbleTimerCB(ob, data);
}


// post_handler for bubble-help (Matthias)
int Toolbar::BubblePost(FL_OBJECT *ob, int event,
			FL_Coord /*mx*/, FL_Coord /*my*/,
			int /*key*/, void */*xev*/)
{
	string help = static_cast<char *>(ob->u_vdata);
	Toolbar * t = reinterpret_cast<Toolbar*>(ob->u_ldata);
	
	if(event == FL_ENTER && !help.empty()){
		fl_set_object_callback(t->bubble_timer,
				       C_Toolbar_BubbleTimerCB,
				       reinterpret_cast<long>(ob));
		fl_set_timer(t->bubble_timer, 1);
	}
	else if(event != FL_MOTION){
		fl_set_timer(t->bubble_timer, 0);
		fl_hide_oneliner();
	}
	return 0;
}


extern "C" int C_Toolbar_BubblePost(FL_OBJECT * ob, int event,
				    FL_Coord /*mx*/, FL_Coord /*my*/, 
				    int key, void * xev)
{
	return Toolbar::BubblePost(ob, event, 0, 0, key, xev);
}
#endif


void Toolbar::activate()
{
	toolbarItem * tmp= 0;
	toolbarItem * item = toollist;
	while(item){
		tmp = item->next;
		if (item->icon) {
			fl_activate_object(item->icon);
		}
		item = tmp;
	}
}


void Toolbar::deactivate()
{
	toolbarItem * tmp= 0;
	toolbarItem * item = toollist;
	while(item){
		tmp = item->next;
		if (item->icon) {
			fl_deactivate_object(item->icon);
		}
		item = tmp;
	}
}


void Toolbar::ToolbarCB(FL_OBJECT * ob, long ac)
{
#if FL_REVISION >= 89
	Toolbar * t = static_cast<Toolbar*>(ob->u_vdata);
#else
	Toolbar * t = reinterpret_cast<Toolbar*>(ob->u_ldata);
#endif
	
	string res = t->owner->getLyXFunc()->Dispatch(int(ac));
	if(!res.empty())
		lyxerr[Debug::TOOLBAR] << res << endl;
}


extern "C" void C_Toolbar_ToolbarCB(FL_OBJECT * ob, long data)
{
	Toolbar::ToolbarCB(ob, data);
}


int Toolbar::get_toolbar_func(string const & func)
{
	int action = lyxaction.LookupFunc(func.c_str());
	if (action == -1) {
		if (func == "separator"){
			action = TOOL_SEPARATOR;
		} else if (func == "layouts"){
                        action = TOOL_LAYOUTS;
                } else action = 0;
	}
	return action;
}

static
void setPixmap(FL_OBJECT * obj, int action, int buttonwidth, int height) {
	string name, arg, xpm_name;
	kb_action act;

	if (lyxaction.isPseudoAction(action)) {
		lyxerr[Debug::TOOLBAR] << "Pseudo action " << action << endl;

		act = lyxaction.retrieveActionArg(action, arg);
		name = lyxaction.getActionName(act);
		xpm_name = subst(name + ' ' + arg, ' ','_');
	} else {
		act = (kb_action)action;
		name = lyxaction.getActionName(action);
		xpm_name = name;
	}

	lyxerr[Debug::TOOLBAR] << "Icon name for action " << action
			       << " is `" << xpm_name << "'" << endl;
	
	string fullname = LibFileSearch("images", xpm_name, "xpm");

	if (!fullname.empty()) {
		lyxerr[Debug::TOOLBAR] << "Full icon name is `" 
				       << fullname << "'" << endl;
		fl_set_pixmapbutton_file(obj, fullname.c_str());
		return;
	}

	if (act == LFUN_INSERT_MATH && !arg.empty()) {
		lyxerr[Debug::TOOLBAR] << "Using mathed-provided icon" << endl;
		char const ** pixmap = get_pixmap_from_symbol(arg.c_str(),
							buttonwidth,
							height);
		fl_set_pixmapbutton_data(obj, const_cast<char **>(pixmap));
		return;
	}
	
	lyxerr << "Unable to find icon `" << xpm_name << "'" << endl;
	fullname = LibFileSearch("images", "unknown", "xpm");
	if (!fullname.empty()) {
		lyxerr[Debug::TOOLBAR] << "Using default `unknown' icon" 
				       << endl;
		fl_set_pixmapbutton_file(obj, fullname.c_str());
	}
}

void Toolbar::set(bool doingmain)
{
	// we shouldn't set if we have not cleaned
	if (!cleaned) return;
	
	FL_OBJECT * obj;
	toolbarItem * item = toollist;
	
	if (!doingmain) {
		fl_freeze_form(owner->getForm());
		fl_addto_form(owner->getForm());
	}

#if FL_REVISION < 89
	// add the time if it don't exist
	if (bubble_timer == 0)
		bubble_timer = fl_add_timer(FL_HIDDEN_TIMER,
					    xpos, ypos, 0, 0, "Timer");
#endif
	
	while(item != 0) {
		switch(item->action){
		case TOOL_SEPARATOR:
			xpos += sepspace;
			item = item->next;
			break;
		case TOOL_LAYOUTS:
			xpos += standardspacing;
			if (!combox)
				combox = new Combox(FL_COMBOX_DROPLIST);
			combox->add(xpos, ypos, 135, height, 400);
			combox->setcallback(LayoutsCB);
			combox->resize(FL_RESIZE_ALL);
			combox->gravity(NorthWestGravity, NorthWestGravity);
			item = item->next;
			xpos += 135;
			break;
		default:
			xpos += standardspacing;
			item->icon = obj = 
				fl_add_pixmapbutton(FL_NORMAL_BUTTON,
						    xpos, ypos,
						    buttonwidth,
						    height, "");
			fl_set_object_boxtype(obj, FL_UP_BOX);
			fl_set_object_color(obj, FL_MCOL, FL_BLUE);
			fl_set_object_resize(obj, FL_RESIZE_ALL);
			fl_set_object_gravity(obj,
					      NorthWestGravity,
					      NorthWestGravity);
			fl_set_object_callback(obj, C_Toolbar_ToolbarCB,
					       static_cast<long>(item->action));
			// Remove the blue feedback rectangle
			fl_set_pixmapbutton_focus_outline(obj, 0);

			// set the bubble-help (Matthias)
#if FL_REVISION >= 89
			// Set the tooltip
			fl_set_object_helper(obj, item->help.c_str());
			// The toolbar that this object belongs too.
			obj->u_vdata = this;
			
			
#else
#ifdef WITH_WARNINGS
#warning This is dangerous!
#endif
			obj->u_vdata = const_cast<char*>(item->help.c_str());
			// we need to know what toolbar this item
			// belongs too. (Lgb)
			obj->u_ldata = reinterpret_cast<long>(this);
			  
			fl_set_object_posthandler(obj, C_Toolbar_BubblePost);
#endif

			setPixmap(obj, item->action, buttonwidth, height);
			item = item->next;
			// we must remember to update the positions
			xpos += buttonwidth;
			// ypos is constant
			/* Here will come a check to see if the new
			 * pos is within the bounds of the main frame,
			 * and perhaps wrap the toolbar if not.
			 */
			break;
		}
	}

	if (!doingmain) {
		fl_end_form();
		fl_unfreeze_form(owner->getForm());
		// Should be safe to do this here.
		owner->updateLayoutChoice();
	}
	
	cleaned = false;
}


void Toolbar::add(int action, bool doclean)
{
	if (doclean && !cleaned) clean();

	// this is what we do if we want to add to an existing
	// toolbar.
	if (!doclean && owner) {
		// first "hide" the toolbar buttons. This is not a real hide
		// actually it deletes and frees the button altogether.
		lyxerr << "Toolbar::add: \"hide\" the toolbar buttons." << endl;
		toolbarItem * tmp= 0;
		toolbarItem * item = toollist;

		lightReset();
		
		fl_freeze_form(owner->getForm());
		while(item){
			tmp = item->next;
			if (item->icon) {
				fl_delete_object(item->icon);
				fl_free_object(item->icon);
			}
			item = tmp;
		}
		if (combox) {
			delete combox;
			combox = 0;
		}
		fl_unfreeze_form(owner->getForm());
		cleaned = true; // this is not completely true, but OK anyway
	}
	
	// there exist some special actions not part of
	// kb_action: SEPARATOR, LAYOUTS

	toolbarItem * newItem, * tmp;

	// adds an item to the list
	newItem = new toolbarItem;
	newItem->action = action;
	newItem->help = lyxaction.helpText(action);
	// the new item is placed at the end of the list
	tmp = toollist;
	if (tmp != 0){
		while(tmp->next != 0)
			tmp = tmp->next;
				// here is tmp->next == 0
		tmp->next = newItem;
		} else
			toollist = newItem;
}


void Toolbar::clean()
{
	toolbarItem * tmp = 0;
	toolbarItem * item = toollist;

	reset();

	//now delete all the objects..
	if (owner)
		fl_freeze_form(owner->getForm());
	while (item) {
		tmp = item->next;
		delete item;
		item = tmp;
	}
	lyxerr[Debug::TOOLBAR] << "Combox: " << combox << endl;
	if (combox) {
		delete combox;
		combox = 0;
	}
	if (owner)
		fl_unfreeze_form(owner->getForm());
	lyxerr[Debug::TOOLBAR] << "toolbar cleaned" << endl;
	cleaned = true;
}


void Toolbar::push(int nth)
{
	lyxerr[Debug::TOOLBAR] << "Toolbar::push: trying to trigger no `"
			       << nth << '\'' << endl;
	
	if (nth == 0) return;

	int count = 0;
	toolbarItem * tmp = toollist;
	while (tmp) {
		++count;
		if (count == nth) {
			fl_trigger_object(tmp->icon);
			return;
		}
		tmp = tmp->next;
	}
	// item nth not found...
	LyXBell();
}


void Toolbar::add(string const & func, bool doclean)
{
	int tf = lyxaction.LookupFunc(func);

	if (tf == -1) {
		lyxerr << "Toolbar::add: no LyX command called`"
		       << func << "'exists!" << endl; 
	} else {
		add(tf, doclean);
	}
}


void Toolbar::reset() 
{
	toollist = 0;
	cleaned = false;
	lightReset();
}

//  void Toolbar::lightReset()
//  {
//  	standardspacing = 2; // the usual space between items
//  	sepspace = 6; // extra space
//  	xpos = sxpos - standardspacing;
//  	ypos = sypos;
//  	buttonwidth = 30; // the standard button width
//  	height = 30; // the height of all items in the toolbar
//  }
