/**
 * \file Menubar_pimpl.C
 * See the file COPYING.
 *
 * \author  Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "Menubar_pimpl.h"
#include "MenuBackend.h"
#include "LyXAction.h"
#include "kbmap.h"
#include "Dialogs.h"
#include "XFormsView.h"
#include "lyxfunc.h"
#include "support/lstrings.h"
#include "support/LAssert.h"
#include "gettext.h"
#include "debug.h"
#include FORMS_H_LOCATION

#include <boost/scoped_ptr.hpp>

#include <algorithm>

using std::endl;
using std::vector;
using std::max;
using std::min;
using std::for_each;

typedef vector<int>::size_type size_type;

extern boost::scoped_ptr<kb_keymap> toplevel_keymap;

namespace {

// Some constants
int const MENU_LABEL_SIZE = FL_NORMAL_SIZE;
int const MENU_LABEL_STYLE = FL_NORMAL_STYLE;
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
	return fl_get_string_widthTAB(MENU_LABEL_STYLE, MENU_LABEL_SIZE,
				      str.c_str(),
				      static_cast<int>(str.length()));
}

} // namespace anon


extern "C" {

	//Defined later, used in makeMenubar().
	static
	void C_Menubar_Pimpl_MenuCallback(FL_OBJECT * ob, long button)
	{
		Menubar::Pimpl::MenuCallback(ob, button);
	}

}


Menubar::Pimpl::Pimpl(LyXView * view, MenuBackend const & mb)
	: owner_(static_cast<XFormsView*>(view)), menubackend_(&mb)
{
	makeMenubar(menubackend_->getMenubar());
}


Menubar::Pimpl::~Pimpl()
{}


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
				" only submenus can appear in a menubar"
			       << endl;
			continue;
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
		fl_set_object_lstyle(obj, MENU_LABEL_STYLE);
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


void Menubar::Pimpl::update()
{
	// nothing yet
}


void Menubar::Pimpl::openByName(string const & name)
{
	for (ButtonList::const_iterator cit = buttonlist_.begin();
	     cit != buttonlist_.end(); ++cit) {
		if ((*cit)->item_->submenuname() == name) {
			MenuCallback((*cit)->obj_, 1);
			return;
		}
	}

	lyxerr << "Menubar::Pimpl::openByName: menu "
	       << name << " not found" << endl;
}


namespace {

Menu::size_type const max_number_of_items = 25;

int get_new_submenu(vector<int> & smn, Window win)
{
	static size_type max_number_of_menus = 32;
	if (smn.size() >= max_number_of_menus)
		max_number_of_menus =
		    fl_setpup_maxpup(static_cast<int>(2*smn.size()));
	int menu = fl_newpup(win);
	fl_setpup_softedge(menu, true);
	fl_setpup_bw(menu, -1);
	lyxerr[Debug::GUI] << "Adding menu " << menu
			   << " in deletion list" << endl;
	smn.push_back(menu);
	return menu;
}


string const fixlabel(string const & str)
{
#if FL_VERSION < 1 && FL_REVISION < 89
	return subst(str, '%', '?');
#else
	return subst(str, "%", "%%");
#endif
}



} // namespace anon



int Menubar::Pimpl::create_submenu(Window win, XFormsView * view,
				   Menu const & menu, vector<int> & smn,
				   bool & all_disabled)
{
	const int menuid = get_new_submenu(smn, win);
	lyxerr[Debug::GUI] << "Menubar::Pimpl::create_submenu: creating "
			   << menu.name() << " as menuid=" << menuid << endl;

	// Compute the size of the largest label (because xforms is
	// not able to support shortcuts correctly...)
	int max_width = 0;
	string widest_label;
	Menu::const_iterator end = menu.end();
	for (Menu::const_iterator i = menu.begin(); i != end; ++i) {
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
	vector<string> extra_labels(menu.size());
	vector<string>::iterator it = extra_labels.begin();
	vector<string>::iterator last = it;
	for (Menu::const_iterator i = menu.begin(); i != end; ++i, ++it)
		if (i->kind() == MenuItem::Separator)
			*last = "%l";
		else if (!i->optional() ||
			 !(view->getLyXFunc().getStatus(i->action()).disabled()))
			last = it;

	it = extra_labels.begin();
	size_type count = 0;
	all_disabled = true;
	int curmenuid = menuid;
	for (Menu::const_iterator i = menu.begin(); i != end; ++i, ++it) {
		MenuItem const & item = (*i);
		string & extra_label = *it;

		++count;
		if (count > max_number_of_items) {
			int tmpmenuid = get_new_submenu(smn, win);
			lyxerr[Debug::GUI] << "Too many items, creating "
					   << "new menu " << tmpmenuid << endl;
			string label = _("More");
			label += "...%m";
			fl_addtopup(curmenuid, label.c_str(), tmpmenuid);
			count = 1;
			curmenuid = tmpmenuid;
		}

		switch (item.kind()) {
		case MenuItem::Command: {
			FuncStatus const flag =
				view->getLyXFunc().getStatus(item.action());
			// handle optional entries.
			if (item.optional()
			    && (flag.disabled())) {
				lyxerr[Debug::GUI]
					<< "Skipping optional item "
					<< item.label() << endl;
				break;
			}

			// Get the keys bound to this action, but keep only the
			// first one later
			string const accel =
				toplevel_keymap->findbinding(item.action());
			// Build the menu label from all the info
			string label = fixlabel(item.label());

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
			if (flag.onoff(true))
				pupmode += "%B";
			if (flag.onoff(false))
				pupmode += "%b";
			if (flag.disabled() || flag.unknown())
				pupmode += "%i";
			else
				all_disabled = false;
			label += pupmode;

			// Finally the menu shortcut
			string shortcut = item.shortcut();

			if (!shortcut.empty()) {
				shortcut += lowercase(shortcut[0]);
				label += "%h";
				fl_addtopup(curmenuid, label.c_str(),
					    shortcut.c_str());
			} else
				fl_addtopup(curmenuid, label.c_str());

			lyxerr[Debug::GUI] << "Command: \""
					   << lyxaction.getActionName(item.action())
					   << "\", binding \"" << accel
					   << "\", shortcut \"" << shortcut
					   << "\" (added to menu"
					   << curmenuid << ")" << endl;
			break;
		}

		case MenuItem::Submenu: {
			bool sub_all_disabled;
			int submenuid = create_submenu(win, view,
						       *item.submenu(), smn,
						       sub_all_disabled);
			all_disabled &= sub_all_disabled;
			if (submenuid == -1)
				return -1;
			string label = fixlabel(item.label());
			label += extra_label + "%m";
			if (sub_all_disabled)
				label += "%i";
			string shortcut = item.shortcut();
			if (!shortcut.empty()) {
				shortcut += lowercase(shortcut[0]);
				label += "%h";
				fl_addtopup(curmenuid, label.c_str(),
					    submenuid, shortcut.c_str());
			} else {
				fl_addtopup(curmenuid, label.c_str(),
					    submenuid);
			}
			break;
		}

		case MenuItem::Separator:
			// already done, and if it was the first one,
			// we just ignore it.
			break;


		default:
			lyxerr << "Menubar::Pimpl::create_submenu: "
				"this should not happen" << endl;
			break;
		}
	}
	return menuid;
}


void Menubar::Pimpl::MenuCallback(FL_OBJECT * ob, long button)
{
	ItemInfo * iteminfo = static_cast<ItemInfo *>(ob->u_vdata);
	XFormsView * view = iteminfo->pimpl_->owner_;
	MenuItem const * item = iteminfo->item_.get();

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

	MenuBackend const * menubackend_ = iteminfo->pimpl_->menubackend_;
	Menu tomenu;
	Menu const frommenu = menubackend_->getMenu(item->submenuname());
	menubackend_->expand(frommenu, tomenu, view->buffer());
	vector<int> submenus;
	bool all_disabled = true;
	int menu = iteminfo->pimpl_->
		create_submenu(FL_ObjWin(ob), view, tomenu,
			       submenus, all_disabled);
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
			view->getLyXFunc().dispatch(choice - action_offset, true);
		} else {
			lyxerr[Debug::GUI]
				<< "MenuCallback: ignoring bogus action "
				<< choice << endl;
		}
	} else {
		lyxerr << "Error in MenuCallback" << endl;
	}

	for_each(submenus.begin(), submenus.end(), fl_freepup);
	// restore tabstop length
	fl_set_tabstop(default_tabstop);

}


Menubar::Pimpl::ItemInfo::ItemInfo
	(Menubar::Pimpl * p, MenuItem const * i, FL_OBJECT * o)
	: pimpl_(p), obj_(o)
{
	item_.reset(i);
}


Menubar::Pimpl::ItemInfo::~ItemInfo()
{}
