// -*- C++ -*-
/**
 * \file Menubar.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes
 */

#ifndef MENUBAR_H
#define MENUBAR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class LyXView;
class MenuBackend;

/**
 * The LyX GUI independent menubar class
 * The GUI interface is implemented in the corresponding Menubar_pimpl class.
 */
class Menubar {
public:
	///
	Menubar(LyXView * o, MenuBackend const &);
	///
	~Menubar();
	///
	void set(string const &);
	/// Opens a top-level submenu given its name
	void openByName(string const &);
	/// update the state of the menuitems
	void update();


	//I disable this temporarily until I find a nice way to make it work
	//with compaq cxx. (Jean-Marc)
	// Is this a new comment? (Lgb)
	struct Pimpl;
	friend struct Pimpl;
private:
	///
	Pimpl * pimpl_;
};
#endif // MENUBAR_H
