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
#include "exporter.h"

using std::pair;
using std::endl;
using std::vector;
using std::max;
using std::min;

typedef vector<int>::size_type size_type;

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
				      str.c_str(), str.length());
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


void Menubar::Pimpl::add_lastfiles(int menu, string const & extra_label) 
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

		fl_addtopup(menu, label.c_str(), shortcut.c_str());
	}

}

void Menubar::Pimpl::add_documents(int menu, string const & extra_label)
{
	vector<string> names = bufferlist.getFileNames();

	if (names.empty()) {
		fl_addtopup(menu,_("No Documents Open!%i"));
		return;
	}

	for (vector<string>::const_iterator cit = names.begin();
	     cit != names.end() ; ++cit) {
		int action =
			lyxaction.getPseudoAction(LFUN_SWITCHBUFFER, *cit);
		string label = MakeDisplayPath(*cit, 30)
			+ "%x" + tostr(action);
		if ((cit + 1) == names.end())
			label += extra_label;
				
		fl_addtopup(menu, label.c_str());
	}

}


string limit_string_length(string const & str)
{
	string::size_type const max_item_length = 45;

	if (str.size() > max_item_length)
		return str.substr(0, max_item_length-3) + "...";
	else
		return str;
}

size_type const max_number_of_menus = 32;
size_type const max_number_of_items = 25;

void add_toc2(int menu, string const & extra_label,
	      vector<int> & smn, Window win,
	      vector<Buffer::TocItem> const & toc_list,
	      size_type from, size_type to, int depth)
{
	if (to - from <= max_number_of_items) {
		for (size_type i = from; i < to; ++i) {
			int action = lyxaction.
				getPseudoAction(LFUN_GOTO_PARAGRAPH,
						tostr(toc_list[i].par->id()));
			string label(4 * max(0, toc_list[i].depth - depth),' ');
			label += toc_list[i].str;
			label = limit_string_length(label);
			label += "%x" + tostr(action);
			if (i == to - 1 && depth == 0)
				label += extra_label;
			fl_addtopup(menu, label.c_str());
		}
	} else {
		size_type pos = from;
		size_type count = 0;
		while (pos < to) {
			++count;
			if (count > max_number_of_items) {
				fl_addtopup(menu, ". . .%d");
				break;
			}
			size_type new_pos = pos+1;
			while (new_pos < to &&
			       toc_list[new_pos].depth > depth)
				++new_pos;

			int action = lyxaction.
				getPseudoAction(LFUN_GOTO_PARAGRAPH,
						tostr(toc_list[pos].par->id()));
			string label(4 * max(0, toc_list[pos].depth - depth), ' ');
			label += toc_list[pos].str;
			label = limit_string_length(label);
			if (new_pos == to && depth == 0)
				label += extra_label;

			if (new_pos == pos + 1) {
				label += "%x" + tostr(action);
				fl_addtopup(menu, label.c_str());
			} else if (smn.size() < max_number_of_menus) {
				int menu2 = fl_newpup(win);
				smn.push_back(menu2);
				add_toc2(menu2, extra_label, smn, win,
					 toc_list, pos, new_pos, depth+1);
				label += "%m";
				fl_addtopup(menu, label.c_str(), menu2);
			} else {
				label += "%d";
				fl_addtopup(menu, label.c_str());
			}
			pos = new_pos;
		}
	}
}

void Menubar::Pimpl::add_toc(int menu, string const & extra_label,
			     vector<int> & smn, Window win)
{
	//xgettext:no-c-format
	static char const * MenuNames[3] = { N_("List of Figures%m"),
	//xgettext:no-c-format
					     N_("List of Tables%m"),
	//xgettext:no-c-format
					     N_("List of Algorithms%m") };

	vector<vector<Buffer::TocItem> > toc_list =
		owner_->buffer()->getTocList();

	// Handle LOF/LOT/LOA
	int max_nonempty = 0;
	for (int i = 1; i <= 3; ++i)
		if (!toc_list[i].empty())
			max_nonempty = i;

	for (int j = 1; j <= 3; ++j)
		if (!toc_list[j].empty()) {
			int menu2 = fl_newpup(win);
			smn.push_back(menu2);
			for (size_type i = 0; i < toc_list[j].size(); ++i) {
				if (i > max_number_of_items) {
					fl_addtopup(menu2, ". . .%d");
					break;
				}
				int action = lyxaction.
					getPseudoAction(LFUN_GOTO_PARAGRAPH,
							tostr(toc_list[j][i].par->id()));
				string label =
					limit_string_length(toc_list[j][i].str);
				label += "%x" + tostr(action);
				fl_addtopup(menu2, label.c_str());
			}
			if (j == max_nonempty) {
				string label = _(MenuNames[j-1]);
				label += "%l";
				fl_addtopup(menu, label.c_str(), menu2);
			} else
				fl_addtopup(menu, _(MenuNames[j-1]), menu2);
		}

	// Handle normal TOC
	if (max_nonempty == 0 && toc_list[0].empty()) {
		fl_addtopup(menu,_("No Table of Contents%i"));
		return;
	}

	add_toc2(menu, extra_label, smn, win,
		 toc_list[0], 0, toc_list[0].size(), 0);

}

void add_references2(int menu, vector<int> & smn, Window win,
		     vector<string> const & label_list, string const & type)
{
	size_type const max_number_of_items = 25;
	size_type const max_number_of_items2 = 20;
	string::size_type const max_item_length = 40;
	string::size_type const max_item_length2 = 20;

	if (label_list.size() <= max_number_of_items)
		for (size_type i = 0; i < label_list.size(); ++i) {
			int action = (type == "goto")
				? lyxaction.getPseudoAction(LFUN_REF_GOTO, 
							    label_list[i])
				: lyxaction.getPseudoAction(LFUN_REF_INSERT,
							    type + "|++||++|"
							    + label_list[i]);
			string label = label_list[i];
			if (label.size() > max_item_length)
				label = label.substr(0, max_item_length-1) + "$";
			label += "%x" + tostr(action);
			fl_addtopup(menu, label.c_str());
		}
	else {
		size_type count = 0;
		for (size_type i = 0; i < label_list.size();
		     i += max_number_of_items2) {
			++count;
			if (count > max_number_of_items) {
				fl_addtopup(menu, ". . .%d");
				break;
			}
			size_type j = min(label_list.size(),
					  i+max_number_of_items2);

			string label;
			label += (label_list[i].size() > max_item_length2)
				? label_list[i].substr(0, max_item_length2-1) + "$"
				: label_list[i];
			label += "..";
			label += (label_list[j-1].size() > max_item_length2)
				? label_list[j-1].substr(0, max_item_length2-1) + "$"
				: label += label_list[j-1];

			if (smn.size() < max_number_of_menus) {
				int menu2 = fl_newpup(win);
				smn.push_back(menu2);
				for (size_type k = i;  k < j; ++k) {
					int action = (type == "goto")
						? lyxaction.getPseudoAction(LFUN_REF_GOTO, 
									    label_list[k])
						: lyxaction.getPseudoAction(LFUN_REF_INSERT,
									    type + "|++||++|"
									    + label_list[k]);
					string label2 = label_list[k];
					if (label2.size() > max_item_length)
						label2 = label2.substr(0, max_item_length-1) + "$";
					label2 += "%x" + tostr(action);
					fl_addtopup(menu2, label2.c_str());
				}
				label += "%m";
				fl_addtopup(menu, label.c_str(), menu2);
			} else {
				label += "%d";
				fl_addtopup(menu, label.c_str());
			}
		}
	}
}


void Menubar::Pimpl::add_references(int menu, string const & extra_label,
				    vector<int> & smn, Window win)
{
	//xgettext:no-c-format
	static char const * MenuNames[6] = { N_("Insert Reference%m"),
	//xgettext:no-c-format
					     N_("Insert Page Number%m"),
	//xgettext:no-c-format
					     N_("Insert vref%m"),
	//xgettext:no-c-format
					     N_("Insert vpageref%m"),
	//xgettext:no-c-format
					     N_("Insert Pretty Ref%m"),
	//xgettext:no-c-format
					     N_("Goto Reference%m") };

	int const EMPTY = 1;
	int const SGML = 2;
	int const READONLY = 4;

	static int MenuFlags[6] = {
		EMPTY | READONLY,
		EMPTY | READONLY,
		EMPTY | READONLY | SGML,
		EMPTY | READONLY | SGML,
		EMPTY | READONLY | SGML,
		EMPTY };

	static string const MenuTypes[6] = {
		"ref", "pageref", "vref", "vpageref", "prettyref", "goto" };

	vector<string> label_list = owner_->buffer()->getLabelList();

	int flag = 0;
	if (label_list.empty())
		flag += EMPTY;
	if (owner_->buffer()->isSGML())
		flag += SGML;
	if (owner_->buffer()->isReadonly())
		flag += READONLY;

	int max_nonempty = -1;
	for (int i = 0; i < 6; ++i)
		if ((MenuFlags[i] & flag) == 0)
			max_nonempty = i;

	for (int i = 0; i < 6; ++i) {
		if ((MenuFlags[i] & flag) == 0) {
			string label = _(MenuNames[i]);
			if (i == max_nonempty)
				label += extra_label;
			if (smn.size() < max_number_of_menus) {
				int menu2 = fl_newpup(win);
				smn.push_back(menu2);
				add_references2(menu2, smn, win, label_list,
						MenuTypes[i]);
				fl_addtopup(menu, label.c_str(), menu2);
			} else {
				label += "%d";
				fl_addtopup(menu, label.c_str());	
			}
		}
	}
}


void Menubar::Pimpl::add_formats(int menu, string const & extra_label,
				 kb_action action, bool viewable)
{
	vector<pair<string,string> > names = 
		viewable
		? Exporter::GetViewableFormats(owner_->buffer())
		: Exporter::GetExportableFormats(owner_->buffer());

	for (vector<pair<string,string> >::const_iterator cit = names.begin();
	     cit != names.end() ; ++cit) {
		int action2 =
			lyxaction.getPseudoAction(action, (*cit).first);
		string label = (*cit).second
			+ "%x" + tostr(action2);
		if ((cit + 1) == names.end())
			label += extra_label;
				
		fl_addtopup(menu, label.c_str());
	}
}


int Menubar::Pimpl::create_submenu(Window win, LyXView * view, 
				   string const & menu_name, 
				   vector<int> & smn) 
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
	Menu::const_iterator end = md.end();
	for (Menu::const_iterator i = md.begin(); i != end; ++i) {
		MenuItem const & item = (*i);
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

	// Compute where to put separators
	vector<string> extra_labels(md.size());
	vector<string>::iterator it = extra_labels.begin();
	vector<string>::iterator last = it;
	for (Menu::const_iterator i = md.begin(); i != end; ++i, ++it)
		if (i->kind() == MenuItem::Separator)
			*last = "%l";
		else if (!i->optional() ||
			 !(view->getLyXFunc()->getStatus(i->action()) & LyXFunc::Disabled))
			last = it;

	it = extra_labels.begin();
	for (Menu::const_iterator i = md.begin(); i != end; ++i, ++it) {
		MenuItem const & item = (*i);
		string & extra_label = *it;

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
				fl_addtopup(menu, label.c_str(), 
					    shortcut.c_str());
			} else
				fl_addtopup(menu, label.c_str());
			
			lyxerr[Debug::GUI] << "Command: \""  
					   << lyxaction.getActionName(item.action())
					   << "\", Binding " << accel 
					   << ", shortcut " << shortcut 
					   << endl;


			break;
		}

		case MenuItem::Submenu: {
			int submenu = create_submenu(win, view, 
						     item.submenu(), smn);
			if (submenu == -1)
				return -1;
			string label = item.label();
			label += extra_label + "%m";
			string shortcut = item.shortcut();
			if (!shortcut.empty()) {
				shortcut += lowercase(shortcut[0]);
				fl_addtopup(menu, (label + "%h").c_str(),
					    submenu, shortcut.c_str());
			}
			else {
				fl_addtopup(menu, label.c_str(), submenu);
			}
			break;
		}

		case MenuItem::Separator:
			// already done, and if it was the first one,
			// we just ignore it.
			break;

		case MenuItem::Documents: 
			add_documents(menu, extra_label);
			break;

		case MenuItem::Lastfiles: 
			add_lastfiles(menu, extra_label);
			break;

		case MenuItem::Toc:
			add_toc(menu, extra_label, smn, win);
			break;

		case MenuItem::References:
			add_references(menu, extra_label, smn, win);
			break;

		case MenuItem::ViewFormats:
			add_formats(menu, extra_label, LFUN_PREVIEW, true);
			break;

		case MenuItem::UpdateFormats:
			add_formats(menu, extra_label, LFUN_UPDATE, true);
			break;  

		case MenuItem::ExportFormats:
			add_formats(menu, extra_label, LFUN_EXPORT, false);
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
	vector<int> submenus;
	int menu = iteminfo->pimpl_->
		create_submenu(FL_ObjWin(ob), view, 
			       item->submenu(), submenus);
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
