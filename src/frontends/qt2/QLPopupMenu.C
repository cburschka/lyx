/**
 * \file QLPopupMenu.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <levon@movementarian.org>
 */

#include "MenuBackend.h"
#include "lyxfunc.h"
#include "debug.h"
 
#include "QtView.h"
 
#include "QLPopupMenu.h"

#include "support/lstrings.h"

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

} 


int createMenu(QMenuData * parent, MenuItem const * item, Menubar::Pimpl * owner)
{
	// FIXME: leaks ??
	QLPopupMenu * pm = new QLPopupMenu(owner, item->submenuname());
	return parent->insertItem(getLabel(*item).c_str(), pm);
}
 
 
QLPopupMenu::QLPopupMenu(Menubar::Pimpl * owner, string const & name)
	: owner_(owner), name_(name)
{
	connect(this, SIGNAL(aboutToShow()), this, SLOT(showing()));
}
 

bool QLPopupMenu::disabled(string const & name)
{
	bool disable = true;
 
	Menu tomenu;
	Menu const frommenu = owner_->backend().getMenu(name);
	owner_->backend().expand(frommenu, tomenu, owner_->view()->buffer());
	Menu::const_iterator m = tomenu.begin();
	Menu::const_iterator end = tomenu.end();
	for (; m != end; ++m) {
		if (m->kind() == MenuItem::Submenu && !disabled(m->submenuname())) {
			disable = false;
		} else {
			FuncStatus const status =
				owner_->view()->getLyXFunc().getStatus(m->action());
			if (!status.disabled())
				disable = false;
		}
	}
	return disable;
}
 

void QLPopupMenu::showing()
{
	clear();
	Menu tomenu;
	Menu const frommenu = owner_->backend().getMenu(name_);
	owner_->backend().expand(frommenu, tomenu, owner_->view()->buffer());
	Menu::const_iterator m = tomenu.begin();
	Menu::const_iterator end = tomenu.end();
	for (; m != end; ++m) {
		if (m->kind() == MenuItem::Separator) {
			insertSeparator();
		} else if (m->kind() == MenuItem::Submenu) {
			int id(createMenu(this, m, owner_));
			setItemEnabled(id, !disabled(m->submenuname()));
		} else {
			FuncStatus const status =
				owner_->view()->getLyXFunc().getStatus(m->action());
			if (status.disabled() && m->optional())
				continue;
			insertItem(getLabel(*m).c_str(), m->action());
			setItemEnabled(m->action(), !status.disabled());
			setItemChecked(m->action(), status.onoff(true));
		}
	}
}
