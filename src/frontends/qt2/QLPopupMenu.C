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

using std::pair;
using std::make_pair;
 
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


pair<int, QLPopupMenu *> createMenu(QMenuData * parent, MenuItem const * item, Menubar::Pimpl * owner, bool is_toplevel)
{
	// FIXME: leaks ??
	QLPopupMenu * pm = new QLPopupMenu(owner, item->submenuname(), is_toplevel);
	int id = parent->insertItem(getLabel(*item).c_str(), pm);
	return make_pair(id, pm);
}
 
 
QLPopupMenu::QLPopupMenu(Menubar::Pimpl * owner, string const & name, bool toplevel)
	: owner_(owner), name_(name)
{
	if (toplevel)
		connect(this, SIGNAL(aboutToShow()), this, SLOT(showing()));
}
 

bool QLPopupMenu::disabled(Menu * menu)
{
	bool disable = true;
 
	Menu::const_iterator m = menu->begin();
	Menu::const_iterator end = menu->end();
	for (; m != end; ++m) {
		if (m->kind() == MenuItem::Submenu && !disabled(m->submenu())) {
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
 

void QLPopupMenu::populate(Menu * menu)
{
	Menu::const_iterator m = menu->begin();
	Menu::const_iterator end = menu->end();
	for (; m != end; ++m) {
		if (m->kind() == MenuItem::Separator) {
			insertSeparator();
		} else if (m->kind() == MenuItem::Submenu) {
			pair<int, QLPopupMenu *> res = createMenu(this, m, owner_);
			setItemEnabled(res.first, !disabled(m->submenu()));
			res.second->populate(m->submenu());
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

 
void QLPopupMenu::showing()
{
	clear();
	Menu tomenu;
	Menu const frommenu = owner_->backend().getMenu(name_);
	owner_->backend().expand(frommenu, tomenu, owner_->view()->buffer());
	populate(&tomenu); 
}
