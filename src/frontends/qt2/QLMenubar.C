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

#include "MenuBackend.h"

#include "QtView.h"
#include "QLMenubar.h"
#include "QLPopupMenu.h"

#include <qmenubar.h>
#include <qcursor.h>


using std::endl;
using std::vector;
using std::max;
using std::min;
using std::for_each;
using std::pair;


QLMenubar::QLMenubar(LyXView * view, MenuBackend const & mbe)
	: owner_(static_cast<QtView*>(view)), menubackend_(mbe)
{
	Menu::const_iterator m = mbe.getMenubar().begin();
	Menu::const_iterator end = mbe.getMenubar().end();
	for (; m != end; ++m) {
		pair<int, QLPopupMenu *> menu =
			createMenu(owner_->menuBar(), &(*m), this, true);
		name_map_[m->submenuname()] = menu.second;
#ifdef Q_WS_MAC
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
