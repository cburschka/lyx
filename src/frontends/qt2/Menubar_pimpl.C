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

namespace {
 
string const getLabel(MenuItem const & mi)
{
	string const shortcut = mi.shortcut();
	string label = mi.label();

	label = subst(label, "&", "&&");
 
	if (shortcut.empty())
		return label;
 
	string::size_type pos = label.find(shortcut);
	if (pos == string::npos)
		return label;
	label.insert(pos, "&");
 
	return label;
}

};
 
typedef vector<int>::size_type size_type;

extern boost::scoped_ptr<kb_keymap> toplevel_keymap;
extern LyXAction lyxaction;


Menubar::Pimpl::Pimpl(LyXView * view, MenuBackend const & mbe) 
	: owner_(static_cast<QtView*>(view)), menubackend_(mbe)
{
	Menu::const_iterator m = mbe.getMenubar().begin();
	Menu::const_iterator end = mbe.getMenubar().end();
	for (; m != end; ++m) {
		makeMenu(owner_->menuBar(), *m);
	}
}


void Menubar::Pimpl::makeMenu(QMenuData * parent, MenuItem const & menu)
{
	// FIXME: does this leak or not ?
	QPopupMenu * pm = new QPopupMenu();
	int const parentid = parent->insertItem(getLabel(menu).c_str(), pm);
 
	Menu md;
	menubackend_.getMenu(menu.submenu()).expand(md, 0);
	Menu::const_iterator m = md.begin();
	Menu::const_iterator end = md.end();
	for (; m != end; ++m) {
		// FIXME: handle the special stuff here
		if (m->kind() == MenuItem::Separator) {
			pm->insertSeparator();
		} else if (m->kind() == MenuItem::Submenu) {
			makeMenu(pm, *m);
		} else {
			pm->insertItem(getLabel(*m).c_str(), m->action());
			MenuItemInfo const info(pm, m->action(), m);
			items_[m->label()] = info;
			updateItem(info);
		}
	}

	MenuItemInfo const info(parent, parentid, &menu);
	items_[menu.label()] = info;
	updateSubmenu(info);
}

 
// FIXME: this is probably buggy with respect to enabling
// two-level submenus
void Menubar::Pimpl::updateSubmenu(MenuItemInfo const & i)
{
	bool enable = true;
#if 0
	bool enable = false;
	Menu md;
	// FIXME FIXME SEGFAULTS
	menubackend_.getMenu(i.item_->submenu()).expand(md, 0);
	Menu::const_iterator m = md.begin();
	Menu::const_iterator end = md.end();
	for (; m != end; ++m) {
	 	if (m->action() > 0) {
			FuncStatus const status =
				owner_->getLyXFunc()->getStatus(m->action());
			if (!status.disabled())
				enable = true;
		}
	}
#endif 
	i.parent_->setItemEnabled(i.id_, enable);
}
 
 
void Menubar::Pimpl::updateItem(MenuItemInfo const & i)
{
	if (i.item_->kind() == MenuItem::Submenu) {
		updateSubmenu(i);
		return;
	}
 
	// FIXME
	if (i.id_ < 0)
		return;
 
	FuncStatus const status = owner_->getLyXFunc()->getStatus(i.id_);
	i.parent_->setItemEnabled(i.id_, !status.disabled());
	i.parent_->setItemChecked(i.id_, status.onoff(true));
}

 
void Menubar::Pimpl::update()
{
	// FIXME: handle special stuff to be updated.
 
	ItemMap::const_iterator cit = items_.begin();
	ItemMap::const_iterator end = items_.end();

	for (; cit != end; ++cit)
		updateItem(cit->second);
}


void Menubar::Pimpl::openByName(string const & name)
{
	lyxerr << "Menubar::Pimpl::openByName: menu " << name << endl;

	ItemMap::iterator it = items_.find(name);

	if (it == items_.end())
		lyxerr << "NOT FOUND " << name << endl;

	// FIXME 
}
