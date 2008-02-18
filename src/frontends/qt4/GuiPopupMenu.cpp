/**
 * \file GuiPopupMenu.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiView.h"

#include "Action.h"
#include "GuiApplication.h"
#include "GuiPopupMenu.h"
#include "qt_helpers.h"

#include "LyXFunc.h"
#include "MenuBackend.h"

#include "support/debug.h"
#include "support/lstrings.h"

namespace lyx {
namespace frontend {

GuiPopupMenu::GuiPopupMenu(GuiView * owner, MenuItem const & mi,
		bool topLevelMenu)
	: QMenu(owner), owner_(owner)
{
	name_ = mi.submenuname();

	setTitle(toqstr(getLabel(mi)));

	if (topLevelMenu)
		connect(this, SIGNAL(aboutToShow()), this, SLOT(updateView()));
}


void GuiPopupMenu::updateView()
{
	LYXERR(Debug::GUI, "GuiPopupMenu::updateView()"
		<< "\tTriggered menu: " << to_utf8(name_));

	clear();

	if (name_.empty())
		return;

	// Here, We make sure that theLyXFunc points to the correct LyXView.
	theLyXFunc().setLyXView(owner_);

	MenuBackend const & menubackend = guiApp->menuBackend();
	Menu const & fromLyxMenu = menubackend.getMenu(name_);
	menubackend.expand(fromLyxMenu, topLevelMenu_, owner_->buffer());

	if (!menubackend.hasMenu(topLevelMenu_.name())) {
		LYXERR(Debug::GUI, "\tWARNING: menu seems empty"
			<< to_utf8(topLevelMenu_.name()));
	}
	populate(this, &topLevelMenu_);
}


void GuiPopupMenu::populate(QMenu * qMenu, Menu * menu)
{
	LYXERR(Debug::GUI, "populating menu " << to_utf8(menu->name()));
	if (menu->size() == 0) {
		LYXERR(Debug::GUI, "\tERROR: empty menu " << to_utf8(menu->name()));
		return;
	}
	LYXERR(Debug::GUI, " *****  menu entries " << menu->size());

	Menu::const_iterator m = menu->begin();
	Menu::const_iterator end = menu->end();

	for (; m != end; ++m) {

		if (m->kind() == MenuItem::Separator) {

			qMenu->addSeparator();
			LYXERR(Debug::GUI, "adding Menubar Separator");

		} else if (m->kind() == MenuItem::Submenu) {

			LYXERR(Debug::GUI, "** creating New Sub-Menu "
				<< to_utf8(getLabel(*m)));
			QMenu * subMenu = qMenu->addMenu(toqstr(getLabel(*m)));
			populate(subMenu, m->submenu());

		} else { // we have a MenuItem::Command

			LYXERR(Debug::GUI, "creating Menu Item "
				<< to_utf8(m->label()));

			docstring const label = getLabel(*m);

			Action * action = new Action(*(owner_),
				QIcon(), toqstr(label), m->func(), QString());
			qMenu->addAction(action);
		}
	}
}


docstring const GuiPopupMenu::getLabel(MenuItem const & mi)
{
	docstring label = support::subst(mi.label(), 
					 from_ascii("&"), from_ascii("&&"));

	docstring const shortcut = mi.shortcut();
	if (!shortcut.empty()) {
		size_t pos = label.find(shortcut);
		if (pos != docstring::npos)
			label.insert(pos, 1, char_type('&'));
	}

	docstring const binding = mi.binding();
	if (!binding.empty())
		label += '\t' + binding;

	return label;
}


} // namespace frontend
} // namespace lyx

#include "GuiPopupMenu_moc.cpp"
