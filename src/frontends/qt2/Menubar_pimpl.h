// -*- C++ -*-
/**
 * \file Menubar_pimpl.h
 * Copyright 2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes <larsbj@lyx.org>
 * \author John Levon <moz@compsoc.man.ac.uk>
 */

 
#ifndef MENUBAR_PIMPL_H
#define MENUBAR_PIMPL_H

#include <vector>
#include <map>

#include <config.h>
 
#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "frontends/Menubar.h"
#include "commandtags.h"

class LyXView;
class QtView;
class QMenuData;
class Menu;
class MenuItem;
class MenuBackend;

/// stored state for menu items
struct MenuItemInfo {
	// I REALLY hate this stupid requirement of std::map
	MenuItemInfo::MenuItemInfo()
		: parent_(0), id_(0), item_(0) {};
 
	MenuItemInfo::MenuItemInfo(QMenuData * p, int id, MenuItem const * item)
		: parent_(p), id_(id), item_(item) {};
 
	/// menu containing item
	QMenuData * parent_;
 
	/// id in containing menu
	int id_;
 
	/// LyX info for item
	MenuItem const * item_;
};
 

struct Menubar::Pimpl {
public:
	Pimpl(LyXView *, MenuBackend const &);
 
	/// opens a top-level submenu given its name
	void openByName(string const &);

	/// update the state of the menuitems
	void update();

private:
	/// create a menu
	void makeMenu(QMenuData * parent, MenuItem const * item, Menu const & menu);
 
	/// special handling updating a submenu label
	void updateSubmenu(MenuItemInfo const & i);
 
	/// update an individual item, returns true if enabled
	void updateItem(MenuItemInfo const & i);

	/// owning view
	QtView * owner_;

	/// menu controller
	MenuBackend const & menubackend_;

	typedef std::map<string, MenuItemInfo> ItemMap;

	/// menu items
	ItemMap items_;
};
 
#endif // MENUBAR_PIMPL_H
