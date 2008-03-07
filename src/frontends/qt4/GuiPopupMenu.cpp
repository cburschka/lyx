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

#include "GuiPopupMenu.h"

#include "Action.h"
#include "GuiApplication.h"
#include "GuiView.h"
#include "qt_helpers.h"

#include "LyXFunc.h"

#include "support/debug.h"

namespace lyx {
namespace frontend {

GuiPopupMenu::GuiPopupMenu(GuiView * owner, MenuItem const & mi,
		bool topLevelMenu)
	: QMenu(owner), owner_(owner)
{
	name_ = mi.submenuname();

	setTitle(label(mi));

	if (topLevelMenu)
		connect(this, SIGNAL(aboutToShow()), this, SLOT(updateView()));
}


void GuiPopupMenu::updateView()
{
	LYXERR(Debug::GUI, "GuiPopupMenu::updateView()"
		<< "\tTriggered menu: " << fromqstr(name_));

	clear();

	if (name_.isEmpty())
		return;

	// Here, We make sure that theLyXFunc points to the correct LyXView.
	theLyXFunc().setLyXView(owner_);

	Menus const & menus = guiApp->menus();
	Menu const & fromLyxMenu = menus.getMenu(name_);
	menus.expand(fromLyxMenu, topLevelMenu_, owner_->buffer());

	if (!menus.hasMenu(topLevelMenu_.name())) {
		LYXERR(Debug::GUI, "\tWARNING: menu seems empty"
			<< fromqstr(topLevelMenu_.name()));
	}
	populate(this, &topLevelMenu_);
}


void GuiPopupMenu::populate(QMenu * qMenu, Menu * menu)
{
	LYXERR(Debug::GUI, "populating menu " << fromqstr(menu->name()));
	if (menu->size() == 0) {
		LYXERR(Debug::GUI, "\tERROR: empty menu " << fromqstr(menu->name()));
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
				<< fromqstr(label(*m)));
			QMenu * subMenu = qMenu->addMenu(label(*m));
			populate(subMenu, m->submenu());

		} else { // we have a MenuItem::Command

			LYXERR(Debug::GUI, "creating Menu Item "
				<< fromqstr(m->label()));

			Action * action = new Action(*owner_,
				QIcon(), label(*m), m->func(), QString());
			qMenu->addAction(action);
		}
	}
}


QString GuiPopupMenu::label(MenuItem const & mi) const
{
	QString label = mi.label();
	label.replace("&", "&&");

	QString shortcut = mi.shortcut();
	if (!shortcut.isEmpty()) {
		int pos = label.indexOf(shortcut);
		if (pos != -1)
			//label.insert(pos, 1, char_type('&'));
			label.replace(pos, 0, "&");
	}

	QString const binding = mi.binding();
	if (!binding.isEmpty())
		label += '\t' + binding;

	return label;
}


} // namespace frontend
} // namespace lyx

#include "GuiPopupMenu_moc.cpp"
