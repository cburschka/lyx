// -*- C++ -*-
/**
 * \file QLPopupMenu.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef QLPOPUPMENU_H
#define QLPOPUPMENU_H

#ifdef __GNUG__
#pragma interface
#endif

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
