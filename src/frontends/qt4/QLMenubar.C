/**
 * \file qt4/QLMenubar.C
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

#include <QMenuBar>
#include <QCursor>

using std::pair;
using std::string;
using std::endl;

namespace {

} // namespace anon

namespace lyx {


namespace frontend {

// MacOSX specific stuff is at the end.

QLMenubar::QLMenubar(LyXView * view, MenuBackend & mbe)
	: owner_(static_cast<GuiView*>(view)), menubackend_(mbe)
{
	macxMenuBarInit();

	lyxerr[Debug::GUI] << "populating menu bar" << lyx::to_utf8(menubackend_.getMenubar().name()) << endl;

	if (menubackend_.getMenubar().size() == 0) {
		lyxerr[Debug::GUI] << "\tERROR: empty menu bar" << lyx::to_utf8(menubackend_.getMenubar().name()) << endl;
		return;
		//			continue;
	}
	else {
		lyxerr[Debug::GUI] << "menu bar entries " << menubackend_.getMenubar().size();
	}
	//	for (; m != end; ++m) {

	Menu menu;
	menubackend_.expand(menubackend_.getMenubar(), menu, owner_->buffer());

	Menu::const_iterator m = menu.begin();
	Menu::const_iterator end = menu.end();

	for (; m != end; ++m) {

		if (m->kind() != MenuItem::Submenu) {
			lyxerr[Debug::GUI] << "\tERROR: not a submenu " << lyx::to_utf8(m->label()) << endl;
			continue;
		}

		lyxerr[Debug::GUI] << "menu bar item " << lyx::to_utf8(m->label()) << " is a submenu named " << lyx::to_utf8(m->submenuname()) << endl;

		docstring name = m->submenuname();
		if (!menubackend_.hasMenu(name)) {
			lyxerr[Debug::GUI] << "\tERROR: " << lyx::to_utf8(name) << " submenu has no menu!" << endl;
			continue;
		}

		Menu menu;
		menubackend_.expand(menubackend_.getMenubar(), menu, owner_->buffer());

		QLPopupMenu * qMenu = new QLPopupMenu(this, *m, true);
		owner_->menuBar()->addMenu(qMenu);

		pair<NameMap::iterator, bool> I = name_map_.insert(make_pair(name, qMenu));
		if (!I.second) {
			lyxerr[Debug::GUI] << "\tERROR: " << lyx::to_utf8(name) << " submenu is already there!" << endl;
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


void QLMenubar::update()
{ }

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
# define MERGE_MAC_MENUS
# ifndef MERGE_MAC_MENUS
extern void qt_mac_set_menubar_merge(bool b);
# endif
#endif

void QLMenubar::macxMenuBarInit()
{
#ifdef Q_WS_MACX
	mac_menubar_.reset(new QMenuBar);

# ifdef MERGE_MAC_MENUS
	/* The qt/mac menu code has a very silly hack that moves some
	   menu entries that it recognizes by name (e.g.
	   "Preferences...") to the "LyX" menu. This feature can only
	   work if the menu entries are always available. Since we
	   build menus on demand, we add the entries to a dummy menu
	   (JMarc)
	*/
	
	Menu special;
	special.add(MenuItem(MenuItem::Command, 
			     qstring_to_ucs4(QMenuBar::tr("About")), 
			     FuncRequest(LFUN_DIALOG_SHOW, "aboutlyx")));
	special.add(MenuItem(MenuItem::Command, 
			     qstring_to_ucs4(QMenuBar::tr("Preferences")),
			     FuncRequest(LFUN_DIALOG_SHOW, "prefs")));
	special.add(MenuItem(MenuItem::Command, 
			     qstring_to_ucs4(QMenuBar::tr("Quit")),
			     FuncRequest(LFUN_LYX_QUIT)));
	menubackend_.specialMenu(special);
	
	QMenu * qMenu = owner_->menuBar()->addMenu("special");	
	Menu::const_iterator end = menubackend_.specialMenu().end();
	for (Menu::const_iterator cit = menubackend_.specialMenu().begin();
	     cit != end ; ++cit) 
		qMenu->addAction(new Action(*owner_, cit->label(), cit->func()));
# else
	qt_mac_set_menubar_merge(false);
# endif // MERGE_MAC_MENUS
#endif // Q_WS_MACX
}

} // namespace frontend
} // namespace lyx

#include "QLMenubar_moc.cpp"
