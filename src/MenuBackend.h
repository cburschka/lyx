/* This file is part of              -*- C++ -*-
* ======================================================
* 
*           LyX, The Document Processor
*
*           Copyright (C) 1995 Matthias Ettrich
*           Copyright (C) 1995-1999 The LyX Team.
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
#include "support/LAssert.h"
#include <vector>

class LyXLex;
class MenuItem;

class MenuItem {
public:
	// The type of elements that can be in a menu
	enum Kind { 
		Command, 
		Submenu, 
		Separator, 
		Lastfiles, // This is the list of last opened file,
		           // typically for the File menu. 
		Documents  // This is the list of opened Documents,
		           // typically for the Documents menu.
	};
	// Create a copy of a given MenuItem
	MenuItem(MenuItem const &);
	// Create a Command type MenuItem
	MenuItem(Kind kind_, string const & label_ = string(), 
		 string const & command_ = string());
	//
	~MenuItem() {}

	// The label of a given menuitem
	string const & label() const { return label_; }
	// The kind of entry
	Kind kind() const { return kind_; } 
	// the action (if relevant)
	int action() const { return action_; }
	// the description of the  submenu (if relevant)
	string const & submenu() const { return submenu_; }
	
private:
	Kind kind_;
	string label_;
	int action_;
	string submenu_;
	MenuItem() {}
};


class Menu {
public:
	//
	typedef std::vector<MenuItem> ItemList;
	//
	typedef ItemList::const_iterator const_iterator;
	//
	explicit Menu(string const & name, bool mb = false) 
		: menubar_(mb), name_(name) {}
	//
	void add(MenuItem const &);
	//
	void read(LyXLex &);
	// 
	bool menubar() const { return menubar_; }
	// 
	string const & name() const { return name_; }
	//
	bool empty() const { return items_.empty(); }
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
	///
};


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

extern MenuBackend menubackend;

#endif /* MENUBACKEND_H */
