/**
 * \file XFormsMenubar.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author  Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#include "XFormsMenubar.h"
#include "MenuBackend.h"
#include "XFormsView.h"
#include "lyxfunc.h"
#include "support/lstrings.h"
#include "support/tostr.h"
#include "support/LAssert.h"
#include "gettext.h"
#include "debug.h"
#include "lyx_forms.h"

//#include <boost/scoped_ptr.hpp>

#include <algorithm>

using namespace lyx::support;

using std::endl;
using std::vector;
using std::max;
using std::min;
using std::for_each;

typedef vector<int>::size_type size_type;

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
	void C_XFormsMenubar_MenuCallback(FL_OBJECT * ob, long button)
	{
		XFormsMenubar::MenuCallback(ob, button);
	}

}


XFormsMenubar::XFormsMenubar(LyXView * view, MenuBackend const & mb)
	: owner_(static_cast<XFormsView*>(view)), menubackend_(&mb)
{
	makeMenubar(menubackend_->getMenubar());
}


XFormsMenubar::~XFormsMenubar()
{}


void XFormsMenubar::makeMenubar(Menu const & menu)
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
			lyxerr << "ERROR: XFormsMenubar::createMenubar:"
				" only submenus can appear in a menubar"
			       << endl;
			continue;
		}
		string const label = i->label();
		string const shortcut = '#' + i->shortcut();
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
		fl_set_object_callback(obj, C_XFormsMenubar_MenuCallback, 1);

		boost::shared_ptr<ItemInfo>
			iteminfo(new ItemInfo(this, new MenuItem(*i), obj));
		buttonlist_.push_back(iteminfo);
		obj->u_vdata = iteminfo.get();
	}

}


void XFormsMenubar::update()
{
	// nothing yet
}


void XFormsMenubar::openByName(string const & name)
{
	for (ButtonList::const_iterator cit = buttonlist_.begin();
	     cit != buttonlist_.end(); ++cit) {
		if ((*cit)->item_->submenuname() == name) {
			MenuCallback((*cit)->obj_, 1);
			return;
		}
	}

	lyxerr << "XFormsMenubar::openByName: menu "
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
	smn.push_back(menu);
	return menu;
}


string const fixlabel(string const & str)
{
	return subst(str, "%", "%%");
}



} // namespace anon



int XFormsMenubar::create_submenu(Window win, XFormsView * view,
				   Menu const & menu, vector<int> & smn)
{
	const int menuid = get_new_submenu(smn, win);
	lyxerr[Debug::GUI] << "XFormsMenubar::create_submenu: creating "
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
			   << ", widest_label=\"" << widest_label
			   << '"' << endl;

	size_type count = 0;
	int curmenuid = menuid;
	for (Menu::const_iterator i = menu.begin(); i != end; ++i) {
		MenuItem const & item = (*i);

		++count;
		// add a More... submenu if the menu is too long (but
		// not just for one extra entry!)
		if (count > max_number_of_items && (i+1) != end) {
			int tmpmenuid = get_new_submenu(smn, win);
			lyxerr[Debug::GUI] << "Too many items, creating "
					   << "new menu " << tmpmenuid << endl;
			string label = _("More");
			label += "...%m";
			fl_addtopup(curmenuid, label.c_str(), tmpmenuid);
			count = 0;
			curmenuid = tmpmenuid;
		}

		switch (item.kind()) {
		case MenuItem::Command:
		case MenuItem::Submenu:
		{
			// Build the menu label from all the info
			string label = fixlabel(item.label());
			FuncStatus const flag = item.status();
			int submenuid = 0;

			// Is there a key binding?
			string const binding = item.binding();
			if (!binding.empty()) {
				// Try to be clever and add  just enough
				// tabs to align shortcuts.
				do
					label += '\t';
				while (string_width(label) < max_width + 5);
				label += binding;
			}

			// Is there a separator after the item?
			if ((i+1) != end
			    && (i + 1)->kind() == MenuItem::Separator)
				label += "%l";

			// Modify the entry using the function status
			if (flag.onoff(true))
				label += "%B";
			if (flag.onoff(false))
				label += "%b";
			if (flag.disabled())
				label += "%i";

			// Add the shortcut
			string shortcut = item.shortcut();
			if (!shortcut.empty()) {
				shortcut += lowercase(shortcut[0]);
				label += "%h";
			}

			// Finally add the action/submenu
			if (item.kind() == MenuItem::Submenu) {
				// create the submenu
				submenuid =
					create_submenu(win, view,
						       *item.submenu(), smn);
				if (submenuid == -1)
					return -1;
				label += "%x" + tostr(smn.size());
				lyxerr[Debug::GUI]
					<< "Menu: " << submenuid
					<< " (at index " << smn.size()
					<< "), ";
			} else {
				// Add the action
				label += "%x" + tostr(item.action()
						      + action_offset);
				lyxerr[Debug::GUI] << "Action: \""
						   << item.action() << "\", ";
			}

			// Add everything to the menu
			fl_addtopup(curmenuid, label.c_str(),
				    shortcut.c_str());
			if (item.kind() == MenuItem::Submenu)
				fl_setpup_submenu(curmenuid, smn.size(),
						  submenuid);

			lyxerr[Debug::GUI] << "label \"" << label
					   << "\", binding \"" << binding
					   << "\", shortcut \"" << shortcut
					   << "\" (added to menu "
					   << curmenuid << ')' << endl;
			break;
		}

		case MenuItem::Separator:
			// already done, and if it was the first one,
			// we just ignore it.
			--count;
			break;


		default:
			lyxerr << "XFormsMenubar::create_submenu: "
				"this should not happen" << endl;
			break;
		}
	}
	return menuid;
}


void XFormsMenubar::MenuCallback(FL_OBJECT * ob, long button)
{
	ItemInfo * iteminfo = static_cast<ItemInfo *>(ob->u_vdata);
	XFormsView * view = iteminfo->menubar_->owner_;
	MenuItem const * item = iteminfo->item_.get();

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

	MenuBackend const * menubackend_ = iteminfo->menubar_->menubackend_;
	Menu tomenu;
	Menu const frommenu = menubackend_->getMenu(item->submenuname());
	menubackend_->expand(frommenu, tomenu, view);
	vector<int> submenus;
	int menu = iteminfo->menubar_->create_submenu(FL_ObjWin(ob), view,
						    tomenu, submenus);
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


XFormsMenubar::ItemInfo::ItemInfo
	(XFormsMenubar * p, MenuItem const * i, FL_OBJECT * o)
	: menubar_(p), obj_(o)
{
	item_.reset(i);
}


XFormsMenubar::ItemInfo::~ItemInfo()
{}
