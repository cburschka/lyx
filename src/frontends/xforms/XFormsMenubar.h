// -*- C++ -*-
/**
 * \file XFormsMenubar.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef XFORMSMENUBAR_H
#define XFORMSMENUBAR_H

#include "support/std_string.h"
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
class MenuBackend;

class XFormsMenubar : public Menubar {
public:
	///
	XFormsMenubar(LyXView *, MenuBackend const &);
	///
	~XFormsMenubar();

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
			   Menu const & menu, std::vector<int> & smn);

	//
	void makeMenubar(Menu const & menu);

	///
	XFormsView * owner_;
	///
	MenuBackend const * menubackend_;
	///
	struct ItemInfo {
		///
		ItemInfo(XFormsMenubar * p, MenuItem const * i,
			 FL_OBJECT * o);
		~ItemInfo();
		///
		XFormsMenubar * menubar_;
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
