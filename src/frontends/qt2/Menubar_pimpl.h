// -*- C++ -*-
/**
 * \file qt2/Menubar_pimpl.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS
 */


#ifndef MENUBAR_PIMPL_H
#define MENUBAR_PIMPL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "frontends/Menubar.h"
#include "LString.h"
#include <map>

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
