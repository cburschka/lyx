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

#ifndef MENUBAR_PIMPL_H
#define MENUBAR_PIMPL_H

#ifdef __GNUG__
#pragma interface
#endif

#include <vector>
#include <map>
#include "LString.h"
#include "frontends/Menubar.h"
#include "commandtags.h"
#include FORMS_H_LOCATION
class LyXView;
class MenuBackend;
class MenuItem;
class Menu;

#include "debug.h"

/** The LyX GUI independent menubar class
  The GUI interface is implemented in the corresponding Menubar_pimpl class. 
  */
struct Menubar::Pimpl {
public:
	///
	Pimpl(LyXView *, MenuBackend const &);
	///
	~Pimpl();
	///
	void set(string const &);
	/// Opens a top-level submenu given its name
	void openByName(string const &);

	///
	static void MenuCallback(FL_OBJECT *, long);

	/** Add to "menu" the list of last opened files
	    (add "extra_label" to the last entry)
	*/
	void add_lastfiles(int menu, string const & extra_label);
	/** Add to "menu" the list of opened documents
	    (add "extra_label" to the last entry)
	*/
	void add_documents(int menu, string const & extra_label);
	/// Add to "menu" the list of exportable/viewable formats
	/// (add "extra_label" to the last entry)
	void add_formats(int menu, string const & extra_label,
			 kb_action action, bool viewable);
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
			: pimpl_(p), item_(i), obj_(o) {}
		///
		Menubar::Pimpl * pimpl_;
		///
		MenuItem const * item_;
		///
		FL_OBJECT * obj_;
	};

	///
	typedef std::vector<ItemInfo *> ButtonList;
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
