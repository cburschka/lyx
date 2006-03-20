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
#include "QtView.h"

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
	: owner_(static_cast<QtView*>(view)), menubackend_(mbe)
{
	macxMenuBarInit();

	lyxerr[Debug::GUI] << "populating menu bar" << menubackend_.getMenubar().name() << endl;

	if (menubackend_.getMenubar().size() == 0) {
		lyxerr[Debug::GUI] << "\tERROR: empty menu bar" << menubackend_.getMenubar().name() << endl;
		return;
		//			continue;
	}
	else {
		lyxerr[Debug::GUI] << "menu bar entries " << menubackend_.getMenubar().size();
	}
	//	for (; m != end; ++m) {

	Menu menu;
	menubackend_.expand(menubackend_.getMenubar(), menu, owner_);

	Menu::const_iterator m = menu.begin();
	Menu::const_iterator end = menu.end();

	for (; m != end; ++m) {

		if (m->kind() != MenuItem::Submenu) {
			lyxerr[Debug::GUI] << "\tERROR: not a submenu " << m->label() << endl;
			continue;
		}

		lyxerr[Debug::GUI] << "menu bar item " << m->label() << " is a submenu named " << m->submenuname() << endl;

		string name = m->submenuname();
		if (!menubackend_.hasMenu(name)) {
			lyxerr[Debug::GUI] << "\tERROR: " << name << " submenu has no menu!" << endl;
			continue;
		}

		Menu menu;
		menubackend_.expand(menubackend_.getMenubar(), menu, owner_);

		QLPopupMenu * qMenu = new QLPopupMenu(this, *m, true);
		owner_->menuBar()->addMenu(qMenu);

		pair<NameMap::iterator, bool> I = name_map_.insert(make_pair(name, qMenu));
		if (!I.second) {
			lyxerr[Debug::GUI] << "\tERROR: " << name << " submenu is already there!" << endl;
		}
/*
		QObject::connect(qMenu, SIGNAL(aboutToShow()), this, SLOT(update()));
		QObject::connect(qMenu, SIGNAL(triggered(QAction *)), this, SLOT(update()));
		QObject::connect(qMenu->menuAction(), SIGNAL(triggered()), this, SLOT(update()));
*/
	}
	//QObject::connect(owner_->menuBar(), SIGNAL(triggered()), this, SLOT(update()));
}

void QLMenubar::openByName(string const & name)
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

QtView * QLMenubar::view()
{
	return owner_;
}


MenuBackend const & QLMenubar::backend()
{
	return menubackend_;
}


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

void QLMenubar::macxMenuBarInit()
{
#ifdef Q_WS_MACX
	mac_menubar_.reset(new QMenuBar);

	// this is the name of the menu that contains our special entries
	menubackend_.specialMenu("LyX");
	// make sure that the special entries are added to the first
	// menu even before this menu has been opened.
	//name_map_[menubackend_.getMenubar().begin()->submenuname()]->update();
#endif
}

} // namespace frontend
} // namespace lyx
