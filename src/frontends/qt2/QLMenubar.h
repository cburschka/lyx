// -*- C++ -*-
/**
 * \file qt2/QLMenubar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */


#ifndef QLMENUBAR_H
#define QLMENUBAR_H

#include "frontends/Menubar.h"
#include "support/std_string.h"
#include <map>

class LyXView;
class QtView;
class MenuBackend;
class QLPopupMenu;

class QLMenubar : public Menubar {
public:
	QLMenubar(LyXView *, MenuBackend const &);

	/// opens a top-level submenu given its name
	void openByName(string const &);

	/// update the state of the menuitems - not needed
	void update();

	/// return the owning view
	QtView * view();

	/// return the menu controller
	MenuBackend const & backend();
private:
	/// owning view
	QtView * owner_;

	/// menu controller
	MenuBackend const & menubackend_;

	typedef std::map<string, QLPopupMenu *> NameMap;

	/// name to menu for openByName
	NameMap name_map_;
};

#endif // QLMENUBAR_H
