/**
 * \file qt4/GuiMenubar.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "GuiMenubar.h"

#include "Action.h"
#include "GuiApplication.h"
#include "GuiPopupMenu.h"
#include "GuiView.h"

#include "qt_helpers.h"

#include "MenuBackend.h"

#include "support/debug.h"

#include <QCursor>
#include <QMenuBar>


namespace lyx {
namespace frontend {

// MacOSX specific stuff is at the end.

GuiMenubar::GuiMenubar(GuiView * view)
	: owner_(view)
{
	init();
}


void GuiMenubar::init()
{
	// Clear all menubar contents before filling it.
	owner_->menuBar()->clear();
	
#ifdef Q_WS_MACX
	// setup special mac specific menu item
	macxMenuBarInit();
#endif

	/// menu controller
	MenuBackend & menu_backend = guiApp->menuBackend();
	LYXERR(Debug::GUI, "populating menu bar" << to_utf8(menu_backend.getMenubar().name()));

	if (menu_backend.getMenubar().size() == 0) {
		LYXERR(Debug::GUI, "\tERROR: empty menu bar"
			<< to_utf8(menu_backend.getMenubar().name()));
		return;
		//			continue;
	}
	else {
		LYXERR(Debug::GUI, "menu bar entries "
			<< menu_backend.getMenubar().size());
	}

	Menu menu;
	menu_backend.expand(menu_backend.getMenubar(), menu, owner_->buffer());

	Menu::const_iterator m = menu.begin();
	Menu::const_iterator end = menu.end();

	for (; m != end; ++m) {

		if (m->kind() != MenuItem::Submenu) {
			LYXERR(Debug::GUI, "\tERROR: not a submenu " << to_utf8(m->label()));
			continue;
		}

		LYXERR(Debug::GUI, "menu bar item " << to_utf8(m->label())
			<< " is a submenu named " << to_utf8(m->submenuname()));

		docstring name = m->submenuname();
		if (!menu_backend.hasMenu(name)) {
			LYXERR(Debug::GUI, "\tERROR: " << to_utf8(name)
				<< " submenu has no menu!");
			continue;
		}

		Menu menu;
		menu_backend.expand(menu_backend.getMenubar(), menu, owner_->buffer());

		GuiPopupMenu * qMenu = new GuiPopupMenu(owner_, *m, true);
		owner_->menuBar()->addMenu(qMenu);

		name_map_[toqstr(name)] = qMenu;
	}
}


GuiMenubar::~GuiMenubar() {}

void GuiMenubar::openByName(QString const & name)
{
	if (QMenu * menu = name_map_.value(name))
		menu->exec(QCursor::pos());
}


/// Some special Qt/Mac support hacks

/*
  Here is what the Qt documentation says about how a menubar is chosen:

     1) If the window has a QMenuBar then it is used. 2) If the window
     is a modal then its menubar is used. If no menubar is specified
     then a default menubar is used (as documented below) 3) If the
     window has no parent then the default menubar is used (as
     documented below).

     The above 3 steps are applied all the way up the parent window
     chain until one of the above are satisifed. If all else fails a
     default menubar will be created, the default menubar on Qt/Mac is
     an empty menubar, however you can create a different default
     menubar by creating a parentless QMenuBar, the first one created
     will thus be designated the default menubar, and will be used
     whenever a default menubar is needed.

  Thus, for Qt/Mac, we add the menus to a free standing menubar, so
  that this menubar will be used also when one of LyX' dialogs has
  focus. (JMarc)
*/

void GuiMenubar::macxMenuBarInit()
{
	/* Since Qt 4.2, the qt/mac menu code has special code for
	   specifying the role of a menu entry. However, it does not
	   work very well with our scheme of creating menus on demand,
	   and therefore we need to put these entries in a special
	   invisible menu. (JMarc)
	*/

	/* The entries of our special mac menu. If we add support for
	 * special entries in MenuBackend, we could imagine something
	 * like
	 *    SpecialItem About " "About LyX" "dialog-show aboutlyx"
	 * and therefore avoid hardcoding. I am not sure it is worth
	 * the hassle, though. (JMarc)
	 */
	struct MacMenuEntry {
		kb_action action;
		char const * arg;
		char const * label;
		QAction::MenuRole role;
	};

	MacMenuEntry entries[] = {
		{LFUN_DIALOG_SHOW, "aboutlyx", "About LyX",
		 QAction::AboutRole},
		{LFUN_DIALOG_SHOW, "prefs", "Preferences",
		 QAction::PreferencesRole},
		{LFUN_RECONFIGURE, "", "Reconfigure",
		 QAction::ApplicationSpecificRole},
		{LFUN_LYX_QUIT, "", "Quit LyX", QAction::QuitRole}
	};
	const size_t num_entries = sizeof(entries) / sizeof(MacMenuEntry);

	// the special menu for MenuBackend.
	Menu special;
	for (size_t i = 0 ; i < num_entries ; ++i) {
		FuncRequest const func(entries[i].action,
				       from_utf8(entries[i].arg));
		special.add(MenuItem(MenuItem::Command,
				     from_utf8(entries[i].label),
				     func));
	}
	MenuBackend & menu_backend = guiApp->menuBackend();
	menu_backend.specialMenu(special);

	// add the entries to a QMenu that will eventually be empty
	// and therefore invisible.
	QMenu * qMenu = owner_->menuBar()->addMenu("special");

	// we do not use 'special' because it is a temporary variable,
	// whereas MenuBackend::specialMenu points to a persistent
	// copy.
	Menu::const_iterator cit = menu_backend.specialMenu().begin();
	Menu::const_iterator end = menu_backend.specialMenu().end();
	for (size_t i = 0 ; cit != end ; ++cit, ++i) {
		Action * action = new Action(*owner_, QIcon(), 
					     toqstr(cit->label()),
					     cit->func(), QString());
		action->setMenuRole(entries[i].role);
		qMenu->addAction(action);

	}
}

} // namespace frontend
} // namespace lyx

#include "GuiMenubar_moc.cpp"
