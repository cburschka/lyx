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

// Qt defines a macro 'signals' that clashes with a boost namespace.
// All is well if the namespace is visible first.
#include "QtView.h"

#include "QLPopupMenu.h"
#include "QLMenubar.h"
#include "qt_helpers.h"

#include "MenuBackend.h"

#include "support/lstrings.h"

#ifdef Q_WS_MACX
#include "kbmap.h"
#include "QLyXKeySym.h"
extern boost::scoped_ptr<kb_keymap> toplevel_keymap;
#endif

#include <qapplication.h>

using std::distance;
using std::make_pair;
using std::string;
using std::pair;

namespace lyx {

using support::subst;

namespace frontend {

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

	return label;
}

} // namespace anon


pair<int, QLPopupMenu *>
createMenu(QMenuData * parent, MenuItem const * item, QLMenubar * owner,
	   bool is_toplevel)
{
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
	qApp->processEvents();
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
			setItemEnabled(res.first, m->status().enabled());
			res.second->populate(m->submenu());
		} else {
			FuncStatus const status = m->status();

			Funcs::iterator fit =
				funcs_.insert(funcs_.end(), m->func());
			int const index = distance(funcs_.begin(), fit);

			QString label = toqstr(getLabel(*m));
#ifdef Q_WS_MACX
			/* There are two constraints on Qt/Mac: (1)
			   the bindings require a unicode string to be
			   represented meaningfully and std::string
			   does not work (2) only 1-key bindings can
			   be represented in menus.
			   
			   This is why the unpleasant hack bellow is
			   needed (JMarc)
			*/
			pair<LyXKeySym const *, key_modifier::state>
				binding = toplevel_keymap->find1keybinding(m->func());
			if (binding.first) {
				QLyXKeySym const *key = static_cast<QLyXKeySym const *>(binding.first);
				label += '\t' + key->qprint(binding.second);
			}
#else
			string const binding(m->binding());
			if (!binding.empty()) {
				label += '\t' + toqstr(binding);
			}
#endif
			insertItem(label, index);
			setItemEnabled(index, status.enabled());
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

} // namespace frontend
} // namespace lyx
