/**
 * \file QLPopupMenu.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author John Levon <levon@movementarian.org>
 */

#ifndef QLPOPUPMENU_H
#define QLPOPUPMENU_H
 
#include "Menubar_pimpl.h"
 
#include <qpopupmenu.h> 

#include "LString.h"
 
class MenuBackend;
class MenuItem;
class Menu;
class QMenuData;
class QLPopupMenu;

/// create a sub-menu
std::pair<int, QLPopupMenu *>
	createMenu(QMenuData * parent, MenuItem const * item, Menubar::Pimpl * owner, bool is_toplevel = false);
 
/// a submenu
class QLPopupMenu : public QPopupMenu {
	Q_OBJECT
public:
	QLPopupMenu(Menubar::Pimpl * owner, string const & name, bool toplevel);

	/// populate the menu 
	void populate(Menu * menu);

public slots:
	/// populate the toplevel menu and all children
	void showing();

private:
	/// return true if the given submenu is disabled
	bool disabled(Menu * menu);
 
	/// our owning menubar
	Menubar::Pimpl * owner_;

	/// the name of this menu
	string name_;
};

#endif // QLPOPUPMENU_H
