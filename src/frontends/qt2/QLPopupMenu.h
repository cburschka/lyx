// -*- C++ -*-
/**
 * \file QLPopupMenu.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QLPOPUPMENU_H
#define QLPOPUPMENU_H


#include <qpopupmenu.h>

#include "LString.h"

#include <map>

class MenuBackend;
class MenuItem;
class Menu;
class QMenuData;
class QLMenubar;
class QLPopupMenu;

/// create a sub-menu
std::pair<int, QLPopupMenu *>
createMenu(QMenuData * parent, MenuItem const * item,
	   QLMenubar * owner, bool is_toplevel = false);

/// a submenu
class QLPopupMenu : public QPopupMenu {
	Q_OBJECT
public:
	QLPopupMenu(QLMenubar * owner,
		    string const & name, bool toplevel);

	/// populate the menu
	void populate(Menu * menu);
public slots:
	/// populate the toplevel menu and all children
	void showing();
private:
	/// our owning menubar
	QLMenubar * owner_;

	/// the name of this menu
	string name_;
};

#endif // QLPOPUPMENU_H
