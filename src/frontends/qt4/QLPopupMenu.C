/**
 * \file QLPopupMenu.C
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include <boost/current_function.hpp>

// Qt defines a macro 'signals' that clashes with a boost namespace.
// All is well if the namespace is visible first.
#include "QtView.h"

#include "QLAction.h"
#include "QLPopupMenu.h"
#include "QLMenubar.h"
#include "qt_helpers.h"
#include "MenuBackend.h"

#include "frontends/lyx_gui.h"
#include "support/lstrings.h"
#include "debug.h"


#ifdef Q_WS_MACX
#include "kbmap.h"
#include "QLyXKeySym.h"
extern boost::scoped_ptr<kb_keymap> toplevel_keymap;
#endif

using std::make_pair;
using std::string;
using std::pair;
using std::endl;

namespace {

} // namespace anon

namespace lyx {

namespace frontend {


// MacOSX specific stuff is at the end.

QLPopupMenu::QLPopupMenu(QLMenubar * owner, 
						 MenuItem const & mi, bool topLevelMenu)
	: owner_(owner), topLevelMenu_(topLevelMenu)
{
	name_ = mi.submenuname();

	setTitle(toqstr(getLabel(mi)));

	if (topLevelMenu)
		connect(this, SIGNAL(aboutToShow()), this, SLOT(update()));
}



void QLPopupMenu::update()
{
	lyxerr[Debug::GUI] << BOOST_CURRENT_FUNCTION << endl;
	lyxerr[Debug::GUI] << "\tTriggered menu: " << name_ << endl;

	clear();
	topLevelMenu.clear();

	if (name_.empty())
		return;

	Menu const & fromLyxMenu = owner_->backend().getMenu(name_);
	owner_->backend().expand(fromLyxMenu, topLevelMenu, owner_->view());
	
	if (!owner_->backend().hasMenu(topLevelMenu.name())) {
		lyxerr[Debug::GUI] << "\tWARNING: menu seems empty" << topLevelMenu.name() << endl;
	}
	populate(this, &topLevelMenu);

	specialMacXmenuHack();
}

void QLPopupMenu::populate(QMenu* qMenu, Menu * menu)
{
	lyxerr[Debug::GUI] << "populating menu " << menu->name() ;
	if (menu->size() == 0) {
		lyxerr[Debug::GUI] << "\tERROR: empty menu " << menu->name() << endl;
		return;
	}
	else {
		lyxerr[Debug::GUI] << " *****  menu entries " << menu->size() << endl;
	}

	Menu::const_iterator m = menu->begin();
	Menu::const_iterator end = menu->end();
	
	for (; m != end; ++m) {

		if (m->kind() == MenuItem::Separator) {
		
			qMenu->addSeparator();
			lyxerr[Debug::GUI] << "adding Menubar Separator" << endl;

		} else if (m->kind() == MenuItem::Submenu) {
			
			lyxerr[Debug::GUI] << "** creating New Sub-Menu " << getLabel(*m) << endl;
			QMenu * subMenu = qMenu->addMenu(toqstr(getLabel(*m)));
			populate(subMenu, m->submenu());
			
		} else { // we have a MenuItem::Command

			FuncStatus status = m->status();
			lyxerr[Debug::GUI] << "creating Menu Item " << m->label() << endl;
			
			string label = getLabel(*m);
			addBinding(label, *m);
			
			QLAction * action = new QLAction(*(owner_->view()), label, m->func());
			action->setEnabled(m->status().enabled());
			action->setChecked(m->status().onoff(true));
			// Actually insert the menu item
			qMenu->addAction(action);
		}
	}
}

string const QLPopupMenu::getLabel(MenuItem const & mi)
{
	string const shortcut = mi.shortcut();
	string label = support::subst(mi.label(), "&", "&&");

	if (!shortcut.empty()) {
		string::size_type pos = label.find(shortcut);
		if (pos != string::npos)
			label.insert(pos, 1, '&');
	}

	return label;
}

/// \todo Mac specific binding handling.
void QLPopupMenu::addBinding(string & label, MenuItem const & mi)
{
#ifndef Q_WS_MACX

		string const binding(mi.binding());
		if (!binding.empty()) {
			label += '\t' + binding;
		}

#else
			/* There are two constraints on Qt/Mac: (1)
			   the bindings require a unicode string to be
			   represented meaningfully and std::string
			   does not work (2) only 1-key bindings can
			   be represented in menus.

			   This is why the unpleasant hack bellow is
			   needed (JMarc)
			*/
/*			pair<LyXKeySym const *, key_modifier::state>
				binding = toplevel_keymap->find1keybinding(mi.func());
			if (binding.first) {
				QLyXKeySym const *key = static_cast<QLyXKeySym const *>(binding.first);
				label += '\t' + key->qprint(binding.second);
			}
*/
#endif
}

/// \todo Fix Mac specific menu hack
void QLPopupMenu::specialMacXmenuHack()
{
#ifdef Q_WS_MACX
	/* The qt/mac menu code has a very silly hack that
	   moves some menu entries that it recognizes by name
	   (e.g. "Preferences...") to the "LyX" menu. This
	   feature can only work if the menu entries are
	   always available. Since we build menus on demand,
	   we add some dummy contents to one of the menus (JMarc)
	*/
/*
	static QLPopupMenu * themenu = this;
	if (themenu == this && owner_->backend().hasMenu("LyX")) {
		Menu special = owner_->backend().getMenu("LyX");
		Menu::const_iterator end = special.end();
		Menu::size_type i = 0;
		for (Menu::const_iterator cit = special.begin();
		     cit != end ; ++cit, ++i)
			insertItem(toqstr(cit->label()), indexOffset + i);
	}
*/
#endif
}

} // namespace frontend
} // namespace lyx
