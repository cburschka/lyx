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
#include <boost/smart_ptr.hpp>

#include <config.h>
 
#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "frontends/Menubar.h"
#include "commandtags.h"
//#include "MenuBackend.h"

class LyXView;
class QtView;
class QMenuData;
class Menu;
class MenuItem;
class MenuBackend;

struct Menubar::Pimpl {
public:
	///
	Pimpl(LyXView *, MenuBackend const &);
	///
	void set(string const &);
	/// Opens a top-level submenu given its name
	void openByName(string const &);

        /// update the state of the menuitems
        void update() {}

private:
	void makeMenu(QMenuData * parent, MenuItem const & menu);
 
	QtView * owner_;

	MenuBackend const & menubackend_;
};
 
#endif
