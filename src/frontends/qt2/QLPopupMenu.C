/**
 * \file QLPopupMenu.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "support/lstrings.h"
#include "MenuBackend.h"
#include "lyxfunc.h"
#include "kbmap.h"
#include "debug.h"

#include "QtView.h"

#include "QLPopupMenu.h"

#include <boost/scoped_ptr.hpp>

using std::pair;
using std::make_pair;

extern boost::scoped_ptr<kb_keymap> toplevel_keymap;

namespace {

string const getLabel(MenuItem const & mi)
{
	string const shortcut = mi.shortcut();
	string label = subst(mi.label(), "&", "&&");

	if (shortcut.empty())
		return label;

	string::size_type pos = label.find(shortcut);
	if (pos == string::npos)
		return label;
	label.insert(pos, 1, '&');

	if (mi.kind() == MenuItem::Command) {
		// FIXME: backend should do this
		string const accel(toplevel_keymap->findbinding(mi.action()));

		if (!accel.empty()) {
			label += '\t' + accel.substr(1, accel.find(']') - 1);
		}

		lyxerr[Debug::GUI] << "Label: " << mi.label()
				   << " Shortcut: " << mi.shortcut()
				   << " Accel: " << accel << endl;
	} else
		lyxerr[Debug::GUI] << "Label: " << mi.label()
				   << " Shortcut: " << mi.shortcut() << endl;

	return label;
}

} // namespace anon


pair<int, QLPopupMenu *>
createMenu(QMenuData * parent, MenuItem const * item, Menubar::Pimpl * owner, bool is_toplevel)
{
	// FIXME: leaks ??
	QLPopupMenu * pm = new QLPopupMenu(owner, item->submenuname(), is_toplevel);
	int id = parent->insertItem(getLabel(*item).c_str(), pm);
	return make_pair(id, pm);
}


QLPopupMenu::QLPopupMenu(Menubar::Pimpl * owner,
			 string const & name, bool toplevel)
	: owner_(owner), name_(name)
{
	if (toplevel)
		connect(this, SIGNAL(aboutToShow()), this, SLOT(showing()));
	connect(this, SIGNAL(activated(int)),
		owner_->view(), SLOT(activated(int)));
}


// FIXME: should all be in backend
bool QLPopupMenu::disabled(Menu * menu)
{
	bool disable = true;

	Menu::const_iterator m = menu->begin();
	Menu::const_iterator end = menu->end();
	for (; m != end; ++m) {
		if (m->kind() == MenuItem::Submenu) {
			if (!disabled(m->submenu()))
				disable = false;
		} else if (m->kind() != MenuItem::Separator) {
			FuncStatus const status =
				owner_->view()->getLyXFunc()
				.getStatus(m->action());
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
			pair<int, QLPopupMenu *> res = createMenu(this, &(*m), owner_);
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
