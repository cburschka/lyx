// -*- C++ -*-
/**
 * \file Menubar_pimpl.h
 * Copyright 1995 Matthias Ettrich
 * Copyright 1995-2002 the LyX Team
 * Copyright 1996-2001 Lars Gullik Bjønnes
 * Read the file COPYING
 *
 * \author Lars Gullik Bjønnes, larsbj@lyx.org
 */

#ifndef MENUBAR_PIMPL_H
#define MENUBAR_PIMPL_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "frontends/Menubar.h"
#include "commandtags.h"
#include "MenuBackend.h"

#include <boost/shared_ptr.hpp>

#include FORMS_H_LOCATION

#include <vector>
#include <map>

class LyXView;

/** The LyX GUI independent menubar class
  The GUI interface is implemented in the corresponding Menubar_pimpl class.
  */
struct Menubar::Pimpl {
public:
	///
	Pimpl(LyXView *, MenuBackend const &);
	///
	void set(string const &);
	/// Opens a top-level submenu given its name
	void openByName(string const &);

	///
	static void MenuCallback(FL_OBJECT *, long);

	///
	void add_toc(int menu, string const & extra_label,
		     std::vector<int> & smn, Window win);
	///
	void add_references(int menu, string const & extra_label,
			    std::vector<int> & smn, Window win);
	///
	int create_submenu(Window win, LyXView * view,
			   string const & menuname,
			   std::vector<int> & smn);

	/// update the state of the menuitems
	void update() {}
private:
	//
	void makeMenubar(Menu const &menu);

	///
	LyXView * owner_;
	///
	MenuBackend const * menubackend_;
	///
	struct ItemInfo {
		///
		ItemInfo(Menubar::Pimpl * p, MenuItem const * i,
			 FL_OBJECT * o)

			: pimpl_(p), obj_(o) { item_.reset(i); }
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

	///
	typedef std::map<string, FL_OBJECT *> MenubarMap;
	///
	MenubarMap menubarmap_;
	///
	string current_menu_name_;
	///
	FL_OBJECT * current_group_;
};
#endif
