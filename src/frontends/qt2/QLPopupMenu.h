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
class QtView;
class MenuItem;
class QMenuData;

/// create a sub-menu
int createMenu(QMenuData * parent, MenuItem const * item, Menubar::Pimpl * owner);
 
/// a submenu
class QLPopupMenu : public QPopupMenu {
	Q_OBJECT
public:
	QLPopupMenu(Menubar::Pimpl * owner, string const & name);

public slots:
	/// populate the menu
	void showing();

private:
	/// return true if the given submenu is disabled
	bool disabled(string const & name);
 
	/// our owning menubar
	Menubar::Pimpl * owner_;

	/// the name of this menu
	string name_;
};

#endif // QLPOPUPMENU_H
