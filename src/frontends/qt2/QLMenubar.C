/**
 * \file qt2/QLMenubar.C
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

#include "MenuBackend.h"

#include <qmenubar.h>
#include <qcursor.h>


using std::pair;
using std::string;

namespace lyx {
namespace frontend {

QLMenubar::QLMenubar(LyXView * view, MenuBackend const & mbe)
	: owner_(static_cast<QtView*>(view)), menubackend_(mbe)
#ifdef Q_WS_MACX
	, menubar_(new QMenuBar)
#endif
{
	Menu::const_iterator m = mbe.getMenubar().begin();
	Menu::const_iterator end = mbe.getMenubar().end();
	for (; m != end; ++m) {
		pair<int, QLPopupMenu *> menu =
			createMenu(menuBar(), &(*m), this, true);
		name_map_[m->submenuname()] = menu.second;
#ifdef Q_WS_MACX
		/* The qt/mac menu code has a very silly hack that
		   moves some menu entries that it recognizes by name
		   (ex: "Preferences...") to the "LyX" menu. This
		   feature can only work if the menu entries are
		   always available. Since we build menus on demand,
		   we have to have a reasonable default value before
		   the menus have been explicitely opened. (JMarc)
		*/
		menu.second->showing();
#endif
	}
}

void QLMenubar::openByName(string const & name)
{
	NameMap::const_iterator const cit = name_map_.find(name);
	if (cit == name_map_.end())
		return;

	// this will have to do I'm afraid.
	cit->second->exec(QCursor::pos());
}


void QLMenubar::update()
{}


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
#ifdef Q_WS_MAC
	return menubar_.get();
#else
	return owner_->menuBar();
#endif
}

} // namespace frontend
} // namespace lyx
