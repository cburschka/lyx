/* This file is part of
* ======================================================
* 
*           LyX, The Document Processor
* 	 
*           Copyright 1999-2001 The LyX Team.
*
* ======================================================*/

#include <config.h>

#include <algorithm>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Menubar_pimpl.h"
#include "MenuBackend.h"
#include "LyXAction.h"
#include "kbmap.h"
#include "buffer.h"
#include "Dialogs.h"
#include "LyXView.h"
#include "lyxfunc.h"
#include "FloatList.h"
#include "support/lstrings.h"
#include "support/LAssert.h"
#include "gettext.h"
#include "debug.h"

using std::endl;
using std::vector;
using std::max;
using std::min;

typedef vector<int>::size_type size_type;

extern boost::scoped_ptr<kb_keymap> toplevel_keymap;
extern LyXAction lyxaction;

namespace {

// Some constants
int const MENU_LABEL_SIZE = FL_NORMAL_SIZE;
int const mheight = 30;
int const mbheight= 22;
// where to place the menubar?
int const yloc = (mheight - mbheight)/2; //air + bw;
int const mbadd = 20; // menu button add (to width)
// Some space between buttons on the menubar 
int const air = 2;
char const * menu_tabstop = "aa";
char const * default_tabstop = "aaaaaaaa";
// We do not want to mix position values in a menu (like the index of
// a submenu) with the action numbers which convey actual information.
// Therefore we offset all the action values by an arbitrary large
// constant. 
int const action_offset = 1000;

// This is used a few times below.
inline
int string_width(string const & str) 
{
	return fl_get_string_widthTAB(FL_NORMAL_STYLE, MENU_LABEL_SIZE,
				      str.c_str(),
				      static_cast<int>(str.length()));
}

} // namespace anon


//Defined later, used in makeMenubar().
extern "C"
void C_Menubar_Pimpl_MenuCallback(FL_OBJECT * ob, long button);


Menubar::Pimpl::Pimpl(LyXView * view, MenuBackend const & mb) 
	: owner_(view), menubackend_(&mb), current_group_(0)
{
	for (MenuBackend::const_iterator menu = menubackend_->begin();
	    menu != menubackend_->end() ; ++menu) {
		if (menu->menubar()) {
			FL_OBJECT * group = fl_bgn_group();
			makeMenubar(*menu);
			fl_end_group();
			fl_hide_object(group);
			lyxerr[Debug::GUI]
				<< "Menubar::Pimpl::Pimpl: "
				<< "creating and hiding group " << group
				<< " for menubar " << menu->name() << endl;
			menubarmap_[menu->name()] = group;
		}
	}
}


void Menubar::Pimpl::makeMenubar(Menu const & menu)
{
	FL_FORM * form = owner_->getForm(); 
	int moffset = 0;

	// Create menu frame if there is non yet.
	FL_OBJECT * frame = fl_add_frame(FL_UP_FRAME, 0, 0,
					 form->w, mheight, "");
	fl_set_object_resize(frame, FL_RESIZE_ALL);
	fl_set_object_gravity(frame, NorthWestGravity, 
			      NorthEastGravity);

	Menu::const_iterator i = menu.begin();
	Menu::const_iterator end = menu.end();
	for (; i != end; ++i) {
		FL_OBJECT * obj;
		if (i->kind() != MenuItem::Submenu) {
			lyxerr << "ERROR: Menubar::Pimpl::createMenubar:"
				" only submenus can appear in a menubar";
			break;
		}
		string const label = i->label();
		string const shortcut = "#" + i->shortcut();
		int const width = string_width(label);
		obj = fl_add_button(FL_MENU_BUTTON,
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

		boost::shared_ptr<ItemInfo>
			iteminfo(new ItemInfo(this, new MenuItem(*i), obj));
		buttonlist_.push_back(iteminfo);
		obj->u_vdata = iteminfo.get();
	}

}


void Menubar::Pimpl::set(string const & menu_name) 
{
	lyxerr[Debug::GUI] << "Entering Menubar::Pimpl::set " 
			   << "for menu `" << menu_name << "'" << endl;

	if (menu_name != current_menu_name_) {
		MenubarMap::iterator mbit = menubarmap_.find(menu_name);

		if (mbit == menubarmap_.end()) {
			lyxerr << "ERROR:set: Unknown menu `" << menu_name
			       << "'" << endl;
			return;
		}

		if (current_group_) {
			lyxerr[Debug::GUI] << "  hiding group "
					   << current_group_ << endl;
			fl_hide_object(current_group_);
		}
		
		lyxerr[Debug::GUI] << "  showing group "
				   << mbit->second << endl;
		fl_show_object(mbit->second);
		current_menu_name_ = menu_name;
		current_group_ = mbit->second;
		lyxerr[Debug::GUI] << "Menubar::Pimpl::set: Menubar set."
				   << endl;
	} else
		lyxerr [Debug::GUI] << "Menubar::Pimpl::set: Nothing to do."
				    << endl;
} 


void Menubar::Pimpl::openByName(string const & name)
{
	if (menubackend_->getMenu(current_menu_name_).hasSubmenu(name)) {
		for (ButtonList::const_iterator cit = buttonlist_.begin();
		     cit != buttonlist_.end(); ++cit) {
			if ((*cit)->item_->submenu() == name) {
				MenuCallback((*cit)->obj_, 1);
				return;
			}
		}
	}
	lyxerr << "Menubar::Pimpl::openByName: menu "
	       << name << " not found" << endl;
}


namespace {

inline
string const limit_string_length(string const & str)
{
	string::size_type const max_item_length = 45;

	if (str.size() > max_item_length)
		return str.substr(0, max_item_length - 3) + "...";
	else
		return str;
}


int get_new_submenu(vector<int> & smn, Window win)
{
	static size_type max_number_of_menus = 32;
	if (smn.size() >= max_number_of_menus)
		max_number_of_menus =
		    fl_setpup_maxpup(static_cast<int>(2*smn.size()));
	int menu = fl_newpup(win);
	smn.push_back(menu);
	return menu;
}


size_type const max_number_of_items = 25;

inline
string const fixlabel(string const & str)
{
#if FL_REVISION < 89
	return subst(str, '%', '?');
#else
	return subst(str, "%", "%%");
#endif
}

} // namespace anon


void add_toc2(int menu, string const & extra_label,
	      vector<int> & smn, Window win,
	      vector<Buffer::TocItem> const & toc_list,
	      size_type from, size_type to, int depth)
{
	int shortcut_count = 0;
	if (to - from <= max_number_of_items) {
		for (size_type i = from; i < to; ++i) {
			int const action = lyxaction.
				getPseudoAction(LFUN_GOTO_PARAGRAPH,
						tostr(toc_list[i].par->id()));
			string label(4 * max(0, toc_list[i].depth - depth),' ');
			label += fixlabel(toc_list[i].str);
			label = limit_string_length(label);
			label += "%x" + tostr(action + action_offset);
			if (i == to - 1 && depth == 0)
				label += extra_label;
			if (toc_list[i].depth == depth
			    && ++shortcut_count <= 9) {
				label += "%h";
				fl_addtopup(menu, label.c_str(),
					    tostr(shortcut_count).c_str());
			} else
				fl_addtopup(menu, label.c_str());
		}
	} else {
		size_type pos = from;
		size_type count = 0;
		while (pos < to) {
			++count;
			if (count > max_number_of_items) {
				int menu2 = get_new_submenu(smn, win);
				add_toc2(menu2, extra_label, smn, win,
					 toc_list, pos, to, depth);
				string label = _("More");
				label += "...%m";
				if (depth == 0)
					label += extra_label;
				fl_addtopup(menu, label.c_str(), menu2);
				break;
			}
			size_type new_pos = pos+1;
			while (new_pos < to &&
			       toc_list[new_pos].depth > depth)
				++new_pos;

			int const action = lyxaction.
				getPseudoAction(LFUN_GOTO_PARAGRAPH,
						tostr(toc_list[pos].par->id()));
			string label(4 * max(0, toc_list[pos].depth - depth), ' ');
			label += fixlabel(toc_list[pos].str);
			label = limit_string_length(label);
			if (new_pos == to && depth == 0)
				label += extra_label;
			string shortcut;
			if (toc_list[pos].depth == depth &&
			    ++shortcut_count <= 9)
				shortcut = tostr(shortcut_count);

			if (new_pos == pos + 1) {
				label += "%x" + tostr(action + action_offset);
				if (!shortcut.empty()) {
					label += "%h";
					fl_addtopup(menu, label.c_str(),
						    shortcut.c_str());
				} else
					fl_addtopup(menu, label.c_str());
			} else {
				int menu2 = get_new_submenu(smn, win);
				add_toc2(menu2, extra_label, smn, win,
					 toc_list, pos, new_pos, depth+1);
				label += "%m";
				if (!shortcut.empty()) {
					label += "%h";
					fl_addtopup(menu, label.c_str(), menu2,
						    shortcut.c_str());
				} else
					fl_addtopup(menu, label.c_str(), menu2);
			}
			pos = new_pos;
		}
	}
}


void Menubar::Pimpl::add_toc(int menu, string const & extra_label,
			     vector<int> & smn, Window win)
{
	Buffer::Lists toc_list = owner_->buffer()->getLists();
	Buffer::Lists::const_iterator cit = toc_list.begin();
	Buffer::Lists::const_iterator end = toc_list.end();
	for (; cit != end; ++cit) {
		// Handle this elsewhere
		if (cit->first == "TOC") continue;

		// All the rest is for floats
		int menu2 = get_new_submenu(smn, win);
		Buffer::SingleList::const_iterator ccit = cit->second.begin();
		Buffer::SingleList::const_iterator eend = cit->second.end();
		for (; ccit != eend; ++ccit) {
			int const action =
				lyxaction
				.getPseudoAction(LFUN_GOTO_PARAGRAPH,
						 tostr(ccit->par->id()));
			string label = fixlabel(ccit->str);
			label = limit_string_length(label);
			label += "%x" + tostr(action + action_offset);
			fl_addtopup(menu2, label.c_str());
		}
		string const m = floatList[cit->first]->second.name() + "%m";
		fl_addtopup(menu, m.c_str(), menu2);
	}
	
	
	// Handle normal TOC
	cit = toc_list.find("TOC");
	if (cit == end) {
		string const tmp = _("No Table of contents%i") + extra_label;
		fl_addtopup(menu, tmp.c_str());
		return;
	} else {
		add_toc2(menu, extra_label, smn, win,
			 cit->second, 0, cit->second.size(), 0);
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
	Menu md;
	menubackend_->getMenu(menu_name).expand(md, owner_->buffer());

	int const menu = get_new_submenu(smn, win);
	fl_setpup_softedge(menu, true);
	fl_setpup_bw(menu, -1);
	lyxerr[Debug::GUI] << "Adding menu " << menu 
			   << " in deletion list" << endl;

	// Compute the size of the largest label (because xforms is
	// not able to support shortcuts correctly...)
	int max_width = 0;
	string widest_label;
	Menu::const_iterator end = md.end();
	for (Menu::const_iterator i = md.begin(); i != end; ++i) {
		MenuItem const & item = (*i);
		if (item.kind() == MenuItem::Command) {
			string const label = item.label() + '\t';
			int const width = string_width(label);
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

		switch (item.kind()) {
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
			string const accel = toplevel_keymap->findbinding(item.action());
			// Build the menu label from all the info
			string label = item.label();

			if (!accel.empty()) {
				// Try to be clever and add  just enough
				// tabs to align shortcuts.
				do 
					label += '\t';
				while (string_width(label) < max_width + 5);
				label += accel.substr(1,accel.find(']') - 1);
			}
			label += "%x" + tostr(item.action() + action_offset)
				+ extra_label;
			
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
					   << "\", binding \"" << accel
					   << "\", shortcut \"" << shortcut 
					   << "\"" << endl;
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
				label += "%h";
				fl_addtopup(menu, label.c_str(),
					    submenu, shortcut.c_str());
			} else {
				fl_addtopup(menu, label.c_str(), submenu);
			}
			break;
		}

		case MenuItem::Separator:
			// already done, and if it was the first one,
			// we just ignore it.
			break;

		case MenuItem::Toc:
			add_toc(menu, extra_label, smn, win);
			break;

		case MenuItem::Documents: 
		case MenuItem::Lastfiles: 
		case MenuItem::ViewFormats:
		case MenuItem::UpdateFormats:
		case MenuItem::ExportFormats:
		case MenuItem::ImportFormats:
		case MenuItem::FloatListInsert:
		case MenuItem::FloatInsert:
			lyxerr << "Menubar::Pimpl::create_submenu: "
				"this should not happen" << endl;
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
	MenuItem const * item = iteminfo->item_.get();

	/* get the splash out of the way. It would be nicer
	 * to only have this code at the start, but xforms
	 * makes it too ugly to do
	 */
	view->getDialogs()->destroySplash();
 
	if (button == 1) {
		// set the pseudo menu-button
		fl_set_object_boxtype(ob, FL_DOWN_BOX);
		fl_set_button(ob, 0);
		fl_redraw_object(ob);
	}

	// Paranoia check
	lyx::Assert(item->kind() == MenuItem::Submenu);
	
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

		// If the action value is too low, then it is not a
		// valid action, but something else.
		if (choice >= action_offset + 1) {
			view->getLyXFunc()->Dispatch(choice - action_offset);
		}
		else {
			lyxerr[Debug::GUI]
				<< "MenuCallback: ignoring bogus action "
				<< choice << endl;
		}
	}
	else 
		lyxerr << "Error in MenuCallback" << endl;
	
	std::for_each(submenus.begin(), submenus.end(), fl_freepup);
	// restore tabstop length
	fl_set_tabstop(default_tabstop);

}
