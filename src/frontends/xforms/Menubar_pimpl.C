/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
* 	 
*           Copyright (C) 1999 The LyX Team.
*
*======================================================*/

#ifdef __GNUG__
#pragma implementation
#endif

#include <config.h>

#include <algorithm>
#include <cctype>
#include "support/lstrings.h"
#include "support/filetools.h"
#include "support/StrPool.h"
#include "support/LAssert.h"
#include "debug.h"
#include "LyXAction.h"
#include "lyxfunc.h"
#include "kbmap.h"
#include "bufferlist.h"
#include "lastfiles.h"
#include "LyXView.h"
#include "MenuBackend.h"
#include "Menubar_pimpl.h"

using std::endl;

extern kb_keymap * toplevel_keymap;
extern LyXAction lyxaction;
extern BufferList bufferlist;
extern LastFiles * lastfiles; 

// Some constants
const int MENU_LABEL_SIZE = FL_NORMAL_SIZE;
const int mheight = 30;
const int mbheight= 22;
// where to place the menubar?
const int yloc = (mheight - mbheight)/2; //air + bw;
const int mbadd = 20; // menu button add (to width)
// Some space between buttons on the menubar 
const int air = 2;
char const * menu_tabstop = "aa";
char const * default_tabstop = "aaaaaaaa";


Menubar::Pimpl::Pimpl(LyXView * view, MenuBackend const & mb) 
	: frame_(0), owner_(view), menubackend_(&mb)
{
	// Should we do something here?
}

Menubar::Pimpl::~Pimpl() 
{
	// Should we do something here?
}

// This is used a few times below.
inline
int string_width(string const & str) 
{
	return fl_get_string_widthTAB(FL_NORMAL_STYLE, MENU_LABEL_SIZE,
				      str.c_str(),	str.length());
}

//Defined later, used in set().
extern "C"
void C_Menubar_Pimpl_MenuCallback(FL_OBJECT * ob, long button);

void Menubar::Pimpl::set(string const & menu_name) 
{
	lyxerr[Debug::GUI] << "Entering Menubar::Pimpl::set " 
			   << "for menu `" << menu_name << "'" << endl;

	if (menu_name == current_menu) {
		lyxerr[Debug::GUI] << "Nothing to do." << endl;
		return;
	}

	// If the backend has not been initialized yet, we use a
	// default instead.  
	if (menubackend_->empty()) {
		lyxerr << "Menubar::Pimpl::set: menubackend is empty! "
			"using default values." << endl;
		MenuBackend * mb = new MenuBackend();
		mb->defaults();
		menubackend_ = mb;
	}

	if (!menubackend_->hasMenu(menu_name)){ 
		lyxerr << "ERROR:set: Unknown menu `" << menu_name
		       << "'" << endl;
		return;
	}

	Menu menu = menubackend_->getMenu(menu_name);

	if (!menu.menubar()) {
		lyxerr << "Only a menubar-type object can go in a "
			"toplevel menu" << endl;
		return;
	}

	current_menu = menu_name;
	FL_FORM * form = owner_->getForm(); 
	int moffset = 0;
	bool form_was_open, form_was_frozen;

	if (fl_current_form == form)
		form_was_open = true;
	else if (fl_current_form == 0) {
		form_was_open = false;
		fl_addto_form(form);
	} 
  	else {
  		lyxerr << "Something is wrong: unknown form " 
		       << fl_current_form << " is already open" 
		       << "(main form is " << form << ")" << endl;
  		return;
  	}
	if (form->frozen)
		form_was_frozen = true;
	else {
		form_was_frozen = false;
		fl_freeze_form(form);
	}

	// Delete old buttons if there are some.
	for(ButtonList::const_iterator cit = buttonlist_.begin();
	    cit != buttonlist_.end(); ++cit) {
		if ((*cit)->obj_) {
			fl_delete_object((*cit)->obj_);
			fl_free_object((*cit)->obj_);
		}
		delete (*cit);
	}
	buttonlist_.clear();

	// Create menu frame if there is non yet.
	if (!frame_) {
		frame_ = fl_add_frame(FL_UP_FRAME, 0, 0, form->w, mheight, "");
		fl_set_object_resize(frame_, FL_RESIZE_ALL);
		fl_set_object_gravity(frame_, NorthWestGravity, 
				      NorthEastGravity);
	} 

	for (Menu::const_iterator i = menu.begin(); 
	     i != menu.end(); ++i) {
		FL_OBJECT * obj;
		if (i->kind() != MenuItem::Submenu) {
			lyxerr << "ERROR: Menubar::Pimpl::Pimpl:"
				" only submenus can appear in a menubar";
			break;
		}
		string label = i->label();
		string shortcut = '#' + i->shortcut();
		int width = string_width(label);
		obj = fl_add_button(FL_TOUCH_BUTTON,
				    air + moffset, yloc,
				    width + mbadd,
				    mbheight, 
				    label.c_str());
		fl_set_object_boxtype(obj, FL_FLAT_BOX);
		fl_set_object_color(obj, FL_MCOL, FL_MCOL);
		fl_set_object_lsize(obj, MENU_LABEL_SIZE);
		fl_set_object_lstyle(obj, FL_NORMAL_STYLE);
		fl_set_object_resize(obj, FL_RESIZE_ALL);
		fl_set_object_gravity(obj, NorthWestGravity, 
				      NorthWestGravity);
		moffset += obj->w + air;
		fl_set_object_shortcut(obj, shortcut.c_str(), 1);
		fl_set_object_callback(obj, C_Menubar_Pimpl_MenuCallback, 1);

		ItemInfo * iteminfo = new ItemInfo(this, 
						   new MenuItem(*i), obj);
		buttonlist_.push_back(iteminfo);
		obj->u_vdata = iteminfo;
//  		lyxerr << "MenuCallback: ItemInfo address=" << iteminfo
//  		       << " Val=(pimpl_=" << iteminfo->pimpl_
//  		       << ", item_=" << iteminfo->item_
//  		       << ", obj_=" << iteminfo->obj_ << ")" <<endl;
	}

	if (!form_was_frozen) {
		fl_unfreeze_form(form);
	}
	if (!form_was_open) 
		fl_end_form();

	// Force the redraw of the buttons (probably not the best
	// method, but...) 
	for(ButtonList::const_iterator cit = buttonlist_.begin();
	    cit != buttonlist_.end(); ++cit) {
		if ((*cit)->obj_) {
			fl_redraw_object((*cit)->obj_);
		}
	}

	lyxerr[Debug::GUI] << "Menubar set." << endl;
} 

void Menubar::Pimpl::openByName(string const & name)
{
	for(ButtonList::const_iterator cit = buttonlist_.begin();
	    cit != buttonlist_.end(); ++cit) {
		if ((*cit)->item_->submenu() == name) {
			MenuCallback((*cit)->obj_, 1);
			return;
		}
	}
	lyxerr << "Menubar::Pimpl::openByName: menu "
	       << name << " not found" << endl;
}


void Menubar::Pimpl::add_lastfiles(int menu, string const & extra_label,
				   std::vector<int> & /*smn*/, 
				   StrPool & strpool) 
{
	int ii = 1;
	for (LastFiles::const_iterator cit = lastfiles->begin();
	     cit != lastfiles->end() && ii < 10; ++cit, ++ii) {

		int action =
			lyxaction.getPseudoAction(LFUN_FILE_OPEN, (*cit));
		string label = tostr(ii) + ". "
			+ MakeDisplayPath((*cit),30)
			+ "%x" + tostr(action) + "%h";
		if ((cit + 1) == lastfiles->end())
			label += extra_label;
		string shortcut = tostr(ii) + "#" + tostr(ii); 
		lyxerr[Debug::GUI] << "shortcut is " << shortcut <<
			endl;

		fl_addtopup(menu, strpool.add(label), strpool.add(shortcut));
	}

}

void Menubar::Pimpl::add_documents(int menu, string const & extra_label,
				   std::vector<int> & /*smn*/, 
				   StrPool & strpool) 
{
	std::vector<string> names = bufferlist.getFileNames();

	if (names.empty()) {
		fl_addtopup(menu,_("No Documents Open!%i"));
		return;
	}

	for (std::vector<string>::const_iterator cit = names.begin();
	     cit != names.end() ; ++cit) {
		int action =
			lyxaction.getPseudoAction(LFUN_SWITCHBUFFER, *cit);
		string label = MakeDisplayPath(*cit, 30)
			+ "%x" + tostr(action);
		if ((cit + 1) == names.end())
			label += extra_label;
				
		fl_addtopup(menu, strpool.add(label));
	}

}


int Menubar::Pimpl::create_submenu(Window win, LyXView * view, 
				   string const & menu_name, 
				   std::vector<int> & smn, StrPool & strpool) 
{
	if (!menubackend_->hasMenu(menu_name)){ 
		lyxerr << "ERROR:create_submenu: Unknown menu `" 
		       << menu_name << "'" << endl;
		return -1;
	}
	Menu md = menubackend_->getMenu(menu_name);

	int menu = fl_newpup(win);
	fl_setpup_softedge(menu, true);
	fl_setpup_bw(menu, -1);
	lyxerr[Debug::GUI] << "Adding menu " << menu 
			   << " in deletion list" << endl;
	smn.push_back(menu);

	// Compute the size of the largest label (because xforms is
	// not able to support shortcuts correctly...)
	int max_width = 0;
	string widest_label;
	for (Menu::const_iterator i = md.begin(); i != md.end(); ++i) {
		MenuItem item = (*i);
		if (item.kind() == MenuItem::Command) {
			string label = item.label() + '\t';
			int width = string_width(label);
			if (width > max_width) {
				max_width = width;
				widest_label = label;
			}
		}
	}
	lyxerr[Debug::GUI] << "max_width=" << max_width 
			   << ", widest_label=`" << widest_label 
			   << "'" << endl;

	for (Menu::const_iterator i = md.begin(); i != md.end(); ++i) {
		MenuItem item = (*i);
		// Is there a separator after this item?
		string extra_label;
		if ((i+1) != md.end()  
		    && (i+1)->kind() == MenuItem::Separator)
			extra_label = "%l";

		switch(item.kind()) {
		case MenuItem::Command: {
			LyXFunc::func_status flag = 
				view->getLyXFunc()->getStatus(item.action()); 

			// handle optional entries.
			if (item.optional() && (flag & LyXFunc::Disabled)) {
				lyxerr[Debug::GUI] 
					<< "Skipping optional item " 
					<< item.label() << endl; 
				break;
			}

			// Get the keys bound to this action, but keep only the
			// first one later
			string accel = toplevel_keymap->findbinding(item.action());
			// Build the menu label from all the info
			string label = item.label();

			if (!accel.empty()) {
				// Try to be clever and add  just enough
				// tabs to align shortcuts.
				do 
					label += '\t';
				while (string_width(label) < max_width);
				label += accel.substr(1,accel.find(']') - 1);
			}
			label += "%x" + tostr(item.action()) + extra_label;
			
			// Modify the entry using the function status
			string pupmode;
			if (flag & (LyXFunc::Disabled | LyXFunc::Unknown))
				pupmode += "%i";
			if (flag & LyXFunc::ToggleOn)
				pupmode += "%B";
			if (flag & LyXFunc::ToggleOff)
				pupmode += "%b";
			label += pupmode;

			// Finally the menu shortcut
			string shortcut = item.shortcut();

			if (!shortcut.empty()) {
				shortcut += lowercase(shortcut[0]);
				label += "%h";
				fl_addtopup(menu, strpool.add(label), 
					    strpool.add(shortcut));
			} else
				fl_addtopup(menu, strpool.add(label));
			
			lyxerr[Debug::GUI] << "Command: \""  
					   << lyxaction.getActionName(item.action())
					   << "\", Binding " << accel 
					   << ", shortcut " << shortcut 
					   << endl;


			break;
		}

		case MenuItem::Submenu: {
			int submenu = create_submenu(win, view, 
						     item.submenu(),
						     smn, strpool);
			if (submenu == -1)
				return -1;
			string label = item.label();
			label += extra_label + "%m";
			string shortcut = item.shortcut();
			if (!shortcut.empty()) {
				shortcut += lowercase(shortcut[0]);
				fl_addtopup(menu, strpool.add(label + "%h"),
					    submenu, strpool.add(shortcut));
			}
			else {
				fl_addtopup(menu, strpool.add(label), submenu);
			}
			break;
		}

		case MenuItem::Separator:
			// already done, and if it was the first one,
			// we just ignore it.
			break;

		case MenuItem::Documents: 
			add_documents(menu, extra_label, smn, strpool);
			break;


		case MenuItem::Lastfiles: 
			add_lastfiles(menu, extra_label, smn, strpool);
			break;

		}
	}
	return menu;
}

extern "C"
void C_Menubar_Pimpl_MenuCallback(FL_OBJECT * ob, long button)
{
	Menubar::Pimpl::MenuCallback(ob, button);
}


void Menubar::Pimpl::MenuCallback(FL_OBJECT * ob, long button)
{
	ItemInfo * iteminfo = static_cast<ItemInfo *>(ob->u_vdata);
//  	lyxerr << "MenuCallback: ItemInfo address=" << iteminfo
//  	       << "Val=(pimpl_=" << iteminfo->pimpl_
//  	       << ", item_=" << iteminfo->item_
//  	       << ", obj_=" << iteminfo->obj_ << ")" <<endl;

	LyXView * view = iteminfo->pimpl_->owner_;
	MenuItem const * item = iteminfo->item_;

	if (button == 1) {
		// set the pseudo menu-button
		fl_set_object_boxtype(ob, FL_DOWN_BOX);
		fl_set_button(ob, 0);
		fl_redraw_object(ob);
	}

	// Paranoia check
	Assert(item->kind() == MenuItem::Submenu);
	
	// set tabstop length
	fl_set_tabstop(menu_tabstop);
	std::vector<int> submenus;
	StrPool strpool;
	int menu = iteminfo->pimpl_->
		create_submenu(FL_ObjWin(ob), view, 
			       item->submenu(), 
			       submenus, strpool);
	if (menu != -1) {
		// place popup
		fl_setpup_position(view->getForm()->x + ob->x,
				   view->getForm()->y + ob->y + ob->h + 10);   
		int choice = fl_dopup(menu);
		
		if (button == 1) {
				// set the pseudo menu-button back
			fl_set_object_boxtype(ob, FL_FLAT_BOX);
			fl_redraw_object(ob);
		}
		
		if (choice >= 1) {
			view->getLyXFunc()->Dispatch(choice);
		}
	}
	else 
		lyxerr << "Error in MenuCallback" << endl;
	
	std::for_each(submenus.begin(), submenus.end(), fl_freepup);
	// restore tabstop length
	fl_set_tabstop(default_tabstop);

}
