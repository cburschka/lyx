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

#include <map>

#include <config.h>
 
#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "frontends/Menubar.h"

class LyXView;
class QtView;
class MenuBackend;
class QLPopupMenu;

struct Menubar::Pimpl {
public:
	Pimpl(LyXView *, MenuBackend const &);
 
	/// opens a top-level submenu given its name
	void openByName(string const &);

	/// update the state of the menuitems - not needed
	void update() {};

	/// return the owning view
	QtView * view() { return owner_; } 

	/// return the menu controller
	MenuBackend const & backend() { return menubackend_; }
private:
	/// owning view
	QtView * owner_;

	/// menu controller
	MenuBackend const & menubackend_;

	typedef std::map<string, QLPopupMenu *> NameMap;

	/// name to menu for openByName 
	NameMap name_map_;
};
 
#endif // MENUBAR_PIMPL_H
