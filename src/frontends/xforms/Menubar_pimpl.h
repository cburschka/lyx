// -*- C++ -*-
/**
 * \file Menubar_pimpl.h
 * Copyright 1995 Matthias Ettrich
 * Copyright 1996-2001 Lars Gullik Bjønnes
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS
 */

#ifndef MENUBAR_PIMPL_H
#define MENUBAR_PIMPL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "frontends/Menubar.h"

#include <boost/shared_ptr.hpp>

#include "forms_fwd.h"
#include <X11/X.h> // Window

#include <vector>
#include <map>

class LyXView;
class XFormsView;
class Menu;
class MenuItem;

/** The LyX GUI independent menubar class
  The GUI interface is implemented in the corresponding Menubar_pimpl class.
  */
struct Menubar::Pimpl {
public:
	///
	Pimpl(LyXView *, MenuBackend const &);
	///
	~Pimpl();

	/// update the state of the menuitems
	void update();

	/// Opens a top-level submenu given its name
	void openByName(string const &);

	///
	static void MenuCallback(FL_OBJECT *, long);

private:
	///
	void add_toc(int menu, string const & extra_label,
		     std::vector<int> & smn, Window win);
	///
	void add_references(int menu, string const & extra_label,
			    std::vector<int> & smn, Window win);
	///
	int create_submenu(Window win, XFormsView * view,
			   Menu const & menu,
			   std::vector<int> & smn, bool & all_disabled);

	//
	void makeMenubar(Menu const & menu);

	///
	XFormsView * owner_;
	///
	MenuBackend const * menubackend_;
	///
	struct ItemInfo {
		///
		ItemInfo(Menubar::Pimpl * p, MenuItem const * i,
			 FL_OBJECT * o);
		~ItemInfo();
		///
		Menubar::Pimpl * pimpl_;
		///
		boost::shared_ptr<MenuItem const> item_;
		///
		FL_OBJECT * obj_;
	};

	///
	typedef std::vector<boost::shared_ptr<ItemInfo> > ButtonList;
	///
	ButtonList buttonlist_;
};
#endif
