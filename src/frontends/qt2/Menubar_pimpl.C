/**
 * \file Menubar_pimpl.C
 * Copyright 1999-2001 The LyX Team.
 * See the file COPYING.
 *
 * \author  Lars Gullik Bjønnes, larsbj@lyx.org
 */

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
#include "lyxfunc.h"
#include "FloatList.h"
#include "support/lstrings.h"
#include "support/LAssert.h"
#include "gettext.h"
#include "debug.h"

#include "QtView.h"
 
#include <qmenubar.h>
#include <qpopupmenu.h>
 
using std::endl;
using std::vector;
using std::max;
using std::min;
using std::for_each;

typedef vector<int>::size_type size_type;

extern boost::scoped_ptr<kb_keymap> toplevel_keymap;
extern LyXAction lyxaction;


Menubar::Pimpl::Pimpl(LyXView * view, MenuBackend const & mbe) 
	: owner_(static_cast<QtView*>(view)), menubackend_(mbe)
{
	MenuBackend::const_iterator mb = menubackend_.begin(); 
	MenuBackend::const_iterator mbend = menubackend_.end();
	for (; mb != mbend; ++mb) {
		if (mb->menubar() && mb->name() == "main") {
			Menu::const_iterator m = mb->begin();
			Menu::const_iterator end = mb->end();
			for (; m != end; ++m) {
				makeMenu(owner_->menuBar(), *m);
			}
		}
	}
}


void Menubar::Pimpl::makeMenu(QMenuData * parent, MenuItem const & menu)
{
	// FIXME: leak
	QPopupMenu * pm = new QPopupMenu();
	parent->insertItem(menu.label().c_str(), pm);
 
	Menu md;
	menubackend_.getMenu(menu.submenu()).expand(md, 0);
	Menu::const_iterator m = md.begin();
	Menu::const_iterator end = md.end();
	for (; m != end; ++m) {
		if (m->kind() == MenuItem::Separator) {
			pm->insertSeparator();
		} else {
			pm->insertItem(m->label().c_str(), m->action());
		} 
	}
}

 
void Menubar::Pimpl::set(string const & menu_name) 
{
	lyxerr[Debug::GUI] << "Entering Menubar::Pimpl::set " 
			   << "for menu `" << menu_name << "'" << endl;

#if 0
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
#endif
} 


void Menubar::Pimpl::openByName(string const & name)
{
	lyxerr << "Menubar::Pimpl::openByName: menu " << name << endl;

#if 0
	if (menubackend_->getMenu(current_menu_name_).hasSubmenu(name)) {
		for (ButtonList::const_iterator cit = buttonlist_.begin();
		     cit != buttonlist_.end(); ++cit) {
			if ((*cit)->item_->submenu() == name) {
				MenuCallback((*cit)->obj_, 1);
				return;
			}
		}
	}
#endif
}
