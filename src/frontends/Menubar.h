// -*- C++ -*-
/* This file is part of
 * ======================================================
 * 
 *           LyX, The Document Processor
 *           Copyright 1995 Matthias Ettrich
 *           Copyright 1995-2000 The LyX Team.
 *
 *           This file is Copyright 1996-2000
 *           Lars Gullik Bjønnes
 *
 * ====================================================== */

#ifndef MENUBAR_H
#define MENUBAR_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"

class LyXView;
class MenuBackend;

/** The LyX GUI independent menubar class
  The GUI interface is implemented in the corresponding Menubar_pimpl class. 
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
#endif
