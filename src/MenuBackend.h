/* This file is part of              -*- C++ -*-
* ======================================================
* 
*           LyX, The Document Processor
*
*           Copyright 1995 Matthias Ettrich
*           Copyright 1995-2000 The LyX Team.
*
*           This file is Copyright 1999
*           Jean-Marc Lasgouttes
*
*======================================================*/

#ifndef MENUBACKEND_H
#define MENUBACKEND_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "support/lstrings.h"
#include <vector>

class LyXLex;

///
class MenuItem {
public:
	/// The type of elements that can be in a menu
	enum Kind {
		///
		Command,
		///
		Submenu,
		///
		Separator,
		/** This is the list of last opened file,
		    typically for the File menu. */
		Lastfiles,
		/** This is the list of opened Documents,
		    typically for the Documents menu. */
		Documents,
		///
		Toc,
		///
		References,
		/** This is a list of viewable formats
		    typically for the Documents menu. */
		ViewFormats,
		/** This is a list of updatable formats
		    typically for the Documents menu. */
		UpdateFormats,
		/** This is a list of exportable formats
		    typically for the Documents menu. */
		ExportFormats
	};
	/// Create a Command type MenuItem
	MenuItem(Kind kind, 
		 string const & label = string(), 
		 string const & command = string(), 
		 bool optional = false);
	/// The label of a given menuitem
	string label() const { return token(label_, '|', 0); }
	///
	string shortcut() const { return token(label_, '|', 1); }
	/// The kind of entry
	Kind kind() const { return kind_; } 
	/// the action (if relevant)
	int action() const { return action_; }
	/// the description of the  submenu (if relevant)
	string const & submenu() const { return submenu_; }
	/// returns true if the entry should be ommited when disabled
	bool optional() const { return optional_; }
private:
	///
	Kind kind_;
	///
	string label_;
	///
	int action_;
	///
	string submenu_;
	///
	bool optional_;
};


///
class Menu {
public:
	///
	typedef std::vector<MenuItem> ItemList;
	///
	typedef ItemList::const_iterator const_iterator;
	///
	explicit Menu(string const & name, bool mb = false) 
		: menubar_(mb), name_(name) {}
	///
	Menu & add(MenuItem const &);
	///
	Menu & read(LyXLex &);
	/// 
	bool menubar() const { return menubar_; }
	/// 
	string const & name() const { return name_; }
	///
	bool empty() const { return items_.empty(); }
	///
	ItemList::size_type size() const { return items_.size(); }
        ///
        const_iterator begin() const {
                return items_.begin();
        }
        ///
        const_iterator end() const {
                return items_.end();
        }
private:
	///
	ItemList items_;
	///
	bool menubar_;
	///
	string name_;
};


///
class MenuBackend {
public:
	///
	typedef std::vector<Menu> MenuList;
	///
	typedef MenuList::const_iterator const_iterator;
	///
	void read(LyXLex &);
	/// Set default values for menu structure.
	void defaults();
	///
	void add(Menu const &);
	///
	bool hasMenu (string const &) const;
	///
	Menu & getMenu (string const &);
	///
	Menu const & getMenu (string const &) const;
	//
	bool empty() const { return menulist_.empty(); }
        ///
        const_iterator begin() const {
                return menulist_.begin();
        }
        ///
        const_iterator end() const {
                return menulist_.end();
        }
private:
	///
	MenuList menulist_;
};

///
extern MenuBackend menubackend;

#endif /* MENUBACKEND_H */
