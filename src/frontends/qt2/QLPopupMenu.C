/**
 * \file QLPopupMenu.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>


#include "support/lstrings.h"
#include "MenuBackend.h"

#include "QtView.h"

#include "QLMenubar.h"
#include "QLPopupMenu.h"
#include "qt_helpers.h"

using lyx::support::subst;

using std::make_pair;

using std::pair;


namespace {

string const getLabel(MenuItem const & mi)
{
	string const shortcut = mi.shortcut();
	string label = subst(mi.label(), "&", "&&");

	if (!shortcut.empty()) {
		string::size_type pos = label.find(shortcut);
		if (pos != string::npos)
			label.insert(pos, 1, '&');
	}

	if (mi.kind() == MenuItem::Command) {
		string const binding(mi.binding());
		if (!binding.empty()) {
			label += '\t' + binding;
		}
	}

	return label;
}

} // namespace anon


pair<int, QLPopupMenu *>
createMenu(QMenuData * parent, MenuItem const * item, QLMenubar * owner,
	   bool is_toplevel)
{
	// FIXME: leaks ??
	QLPopupMenu * pm = new QLPopupMenu(owner, item->submenuname(), is_toplevel);
	int const id = parent->insertItem(toqstr(getLabel(*item)), pm);
	return make_pair(id, pm);
}


QLPopupMenu::QLPopupMenu(QLMenubar * owner,
			 string const & name, bool toplevel)
	: owner_(owner), name_(name)
{
	if (toplevel)
		connect(this, SIGNAL(aboutToShow()), this, SLOT(showing()));
	connect(this, SIGNAL(activated(int)),
		this, SLOT(fire(int)));
}


void QLPopupMenu::fire(int index)
{
	owner_->view()->activated(funcs_[index]);
}


void QLPopupMenu::populate(Menu * menu)
{
	funcs_.clear();

	Menu::const_iterator m = menu->begin();
	Menu::const_iterator end = menu->end();
	for (; m != end; ++m) {
		if (m->kind() == MenuItem::Separator) {
			insertSeparator();
		} else if (m->kind() == MenuItem::Submenu) {
			pair<int, QLPopupMenu *> res = createMenu(this, &(*m), owner_);
			setItemEnabled(res.first,
				       !m->status().disabled());
			res.second->populate(m->submenu());
		} else {
			FuncStatus const status = m->status();

			Funcs::iterator fit =
				funcs_.insert(funcs_.end(), m->func());
			int const index = std::distance(funcs_.begin(), fit);

			insertItem(toqstr(getLabel(*m)), index);
			setItemEnabled(index, !status.disabled());
			setItemChecked(index, status.onoff(true));
		}
	}
}


void QLPopupMenu::showing()
{
	clear();
	Menu tomenu;
	Menu const frommenu = owner_->backend().getMenu(name_);
	owner_->backend().expand(frommenu, tomenu, owner_->view());
	populate(&tomenu);
}
