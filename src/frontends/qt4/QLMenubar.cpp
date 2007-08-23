/**
 * \file qt4/QLMenubar.cpp
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
#include "GuiView.h"

#include "Action.h"
#include "QLMenubar.h"
#include "QLPopupMenu.h"

#include "qt_helpers.h"
#include "support/lstrings.h"

#include "MenuBackend.h"

#include "debug.h"

#include <QCursor>
#include <QMenuBar>

using std::pair;
using std::string;
using std::endl;

namespace lyx {
namespace frontend {

// MacOSX specific stuff is at the end.

QLMenubar::QLMenubar(LyXView * view, MenuBackend & mbe)
	: owner_(static_cast<GuiView*>(view)), menubackend_(mbe)
{
	macxMenuBarInit();

	LYXERR(Debug::GUI) << "populating menu bar" << to_utf8(menubackend_.getMenubar().name()) << endl;

	if (menubackend_.getMenubar().size() == 0) {
		LYXERR(Debug::GUI) << "\tERROR: empty menu bar"
			<< to_utf8(menubackend_.getMenubar().name()) << endl;
		return;
		//			continue;
	}
	else {
		LYXERR(Debug::GUI) << "menu bar entries "
			<< menubackend_.getMenubar().size();
	}

	Menu menu;
	menubackend_.expand(menubackend_.getMenubar(), menu, owner_->buffer());

	Menu::const_iterator m = menu.begin();
	Menu::const_iterator end = menu.end();

	for (; m != end; ++m) {

		if (m->kind() != MenuItem::Submenu) {
			LYXERR(Debug::GUI) << "\tERROR: not a submenu "
				<< to_utf8(m->label()) << endl;
			continue;
		}

		LYXERR(Debug::GUI) << "menu bar item " << to_utf8(m->label())
			<< " is a submenu named " << to_utf8(m->submenuname()) << endl;

		docstring name = m->submenuname();
		if (!menubackend_.hasMenu(name)) {
			LYXERR(Debug::GUI) << "\tERROR: " << to_utf8(name)
				<< " submenu has no menu!" << endl;
			continue;
		}

		Menu menu;
		menubackend_.expand(menubackend_.getMenubar(), menu, owner_->buffer());

		QLPopupMenu * qMenu = new QLPopupMenu(this, *m, true);
		owner_->menuBar()->addMenu(qMenu);

		pair<NameMap::iterator, bool> I = name_map_.insert(make_pair(name, qMenu));
		if (!I.second) {
			LYXERR(Debug::GUI) << "\tERROR: " << to_utf8(name)
				<< " submenu is already there!" << endl;
		}
/*
		QObject::connect(qMenu, SIGNAL(aboutToShow()), this, SLOT(update()));
		QObject::connect(qMenu, SIGNAL(triggered(QAction *)), this, SLOT(update()));
		QObject::connect(qMenu->menuAction(), SIGNAL(triggered()), this, SLOT(update()));
*/
	}
	//QObject::connect(owner_->menuBar(), SIGNAL(triggered()), this, SLOT(update()));
}

void QLMenubar::openByName(docstring const & name)
{
	NameMap::const_iterator const cit = name_map_.find(name);
	if (cit == name_map_.end())
		return;

	// I (Abdel) don't understand this comment:
	// this will have to do I'm afraid.
	cit->second->exec(QCursor::pos());
}


GuiView * QLMenubar::view()
{
	return owner_;
}


MenuBackend const & QLMenubar::backend()
{
	return menubackend_;
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
QMenuBar * QLMenubar::menuBar() const
{
#ifdef Q_WS_MACX
	return mac_menubar_.get();
#else
	return owner_->menuBar();
#endif
}

#ifdef Q_WS_MACX
extern void qt_mac_set_menubar_merge(bool b);
#endif

void QLMenubar::macxMenuBarInit()
{
#ifdef Q_WS_MACX
	mac_menubar_.reset(new QMenuBar);

# if QT_VERSION >= 0x040200
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
	menubackend_.specialMenu(special);

	// add the entries to a QMenu that will eventually be empty
	// and therefore invisible.
	QMenu * qMenu = owner_->menuBar()->addMenu("special");

	// we do not use 'special' because it is a temporary variable,
	// whereas MenuBackend::specialMenu points to a persistent
	// copy.
	Menu::const_iterator cit = menubackend_.specialMenu().begin();
	Menu::const_iterator end = menubackend_.specialMenu().end();
	for (size_t i = 0 ; cit != end ; ++cit, ++i) {
		Action * action = new Action(*owner_, cit->label(),
					     cit->func());
		action->setMenuRole(entries[i].role);
		qMenu->addAction(action);

	}
# else
	qt_mac_set_menubar_merge(false);
# endif // QT_VERSION >= 0x040200
#endif // Q_WS_MACX
}

} // namespace frontend
} // namespace lyx

#include "QLMenubar_moc.cpp"
