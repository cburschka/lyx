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
	// The trick we use to get the help text is to read the
	// argument of the callback that has been registered for
	// ToolBarCB.  (JMarc)
	string help = lyxaction.helpText(ob->argument);
	fl_show_oneliner(help.c_str(), ob->form->x + ob->x,
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
	Toolbar * t = reinterpret_cast<Toolbar*>(ob->u_vdata);
	
	// We do not test for empty help here, since this can never happen
	if(event == FL_ENTER){
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
	ToolbarList::const_iterator p = toollist.begin();
	for (; p != toollist.end(); ++p) {
		if (p->icon) {
			fl_activate_object(p->icon);
		}
	}
}


void Toolbar::deactivate()
{
	ToolbarList::const_iterator p = toollist.begin();
	for (; p != toollist.end(); ++p) {
		if (p->icon) {
			fl_deactivate_object(p->icon);
		}
	}
}

void Toolbar::update()
{
	ToolbarList::const_iterator p = toollist.begin();
	for (; p != toollist.end(); ++p) {
		if (p->icon) {
			int status = owner->getLyXFunc()->getStatus(p->action);
			if (status & LyXFunc::ToggleOn) {
				// I'd like to use a different color
				// here, but then the problem is to
				// know how to use transparency with
				// Xpm library. It seems pretty
				// complicated to me (JMarc)
				fl_set_object_boxtype(p->icon, FL_DOWN_BOX);
			} else {
				fl_set_object_boxtype(p->icon, FL_UP_BOX);
			}

			if (status & LyXFunc::Disabled) {
				// Is there a way here to specify a
				// mask in order to show that the
				// button is disabled? (JMarc)
				fl_deactivate_object(p->icon);
			}
			else
				fl_activate_object(p->icon);
		}
	}
}


void Toolbar::ToolbarCB(FL_OBJECT * ob, long ac)
{
	Toolbar * t = static_cast<Toolbar*>(ob->u_vdata);
	
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
	
	ToolbarList::iterator item = toollist.begin();
	for (; item != toollist.end(); ++item) {
		switch(item->action){
		case TOOL_SEPARATOR:
			xpos += sepspace;
			break;
		case TOOL_LAYOUTS:
			xpos += standardspacing;
			if (!combox)
				combox = new Combox(FL_COMBOX_DROPLIST);
			combox->add(xpos, ypos, 135, height, 400);
			combox->setcallback(LayoutsCB);
			combox->resize(FL_RESIZE_ALL);
			combox->gravity(NorthWestGravity, NorthWestGravity);
			xpos += 135;
			break;
		default:
			xpos += standardspacing;
			item->icon = obj = 
				fl_add_pixmapbutton(FL_NORMAL_BUTTON,
						    xpos, ypos,
						    buttonwidth,
						    height, "");
			fl_set_object_resize(obj, FL_RESIZE_ALL);
			fl_set_object_gravity(obj,
					      NorthWestGravity,
					      NorthWestGravity);
			fl_set_object_callback(obj, C_Toolbar_ToolbarCB,
					       static_cast<long>(item->action));
			fl_set_object_color(obj, FL_MCOL, FL_BLUE);
			// Remove the blue feedback rectangle
			fl_set_pixmapbutton_focus_outline(obj, 0);

			// Set the tooltip
#if FL_REVISION >= 89
			string help = lyxaction.helpText(action);
			fl_set_object_helper(obj, help.c_str());	
#else
			fl_set_object_posthandler(obj, C_Toolbar_BubblePost);
#endif

			// The toolbar that this object belongs too.
			obj->u_vdata = this;

			setPixmap(obj, item->action, buttonwidth, height);
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

	// set the state of the icons
	//update();

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
		lyxerr << "Toolbar::add: \"hide\" the toolbar buttons." 
		       << endl;

		lightReset();
		
		fl_freeze_form(owner->getForm());

		ToolbarList::iterator p = toollist.begin();
		for (; p != toollist.end(); ++p) {
			p->clean();
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

	toolbarItem newItem;
	newItem.action = action;
	toollist.push_back(newItem);
}


void Toolbar::clean()
{
	//reset(); // I do not understand what this reset() is, anyway

	//now delete all the objects..
	if (owner)
		fl_freeze_form(owner->getForm());

	// G++ vector does not have clear defined
	//toollist.clear();
	toollist.erase(toollist.begin(), toollist.end());

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
	
	if (nth <= 0 || nth >= int(toollist.size())) {
		// item nth not found...
		LyXBell();
		return;
	}

	fl_trigger_object(toollist[nth - 1].icon);
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
	//toollist = 0; // what is this supposed to do?
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
