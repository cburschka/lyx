// -*- C++ -*-
/**
 * \file MenuBackend.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Jean-Marc Lasgouttes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MENUBACKEND_H
#define MENUBACKEND_H

#include "LString.h"

#include <boost/shared_ptr.hpp>

#include <vector>

#include "FuncStatus.h"

class LyXLex;
class LyXView;
class Menu;

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
		/** This is a list of viewable formats
		    typically for the File->View menu. */
		ViewFormats,
		/** This is a list of updatable formats
		    typically for the File->Update menu. */
		UpdateFormats,
		/** This is a list of exportable formats
		    typically for the File->Export menu. */
		ExportFormats,
		/** This is a list of importable formats
		    typically for the File->Export menu. */
		ImportFormats,
		/** This is the list of floats that we can
		    insert a list for. */
		FloatListInsert,
		/** This is the list of floats that we can
		    insert. */
		FloatInsert,
		/** This is the list of selections that can
		    be pasted. */
		PasteRecent,
		/** Available branches in document */
		Branches
	};
	/// Create a Command type MenuItem
	MenuItem(Kind kind,
		 string const & label = string(),
		 string const & command = string(),
		 bool optional = false);
	MenuItem(Kind kind,
		 string const & label,
		 int action,
		 bool optional = false);

	/// This one is just to please boost::shared_ptr<>
	~MenuItem();
	/// The label of a given menuitem
	string const label() const;
	/// The keyboard shortcut (usually underlined in the entry)
	string const shortcut() const;
	/// The complete label, with label and shortcut separated by a '|'
	string const fulllabel() const { return label_;}
	/// The kind of entry
	Kind kind() const { return kind_; }
	/// the action (if relevant)
	int action() const { return action_; }
	/// returns true if the entry should be ommited when disabled
	bool optional() const { return optional_; }
	/// returns the status of the lfun associated with this entry
	FuncStatus const & status() const { return status_; }
	/// returns the status of the lfun associated with this entry
	FuncStatus & status() { return status_; }
	/// returns the status of the lfun associated with this entry
	void status(FuncStatus const & status) { status_ = status; }
	/// returns the binding associated to this action
	string const binding() const;
	/// the description of the  submenu (if relevant)
	string const & submenuname() const { return submenuname_; }
	/// set the description of the  submenu
	void submenuname(string const & name) { submenuname_ = name; }
	///
	Menu * submenu() const { return submenu_.get(); }
	///
	void submenu(Menu * menu);

private:
	//friend class MenuBackend;
	///
	Kind kind_;
	///
	string label_;
	///
	int action_;
	///
	string submenuname_;
	///
	bool optional_;
	///
	FuncStatus status_;
	///
	boost::shared_ptr<Menu> submenu_;
};


///
class Menu {
public:
	///
	typedef std::vector<MenuItem> ItemList;
	///
	typedef ItemList::const_iterator const_iterator;
	///
	typedef ItemList::size_type size_type;
	///
	explicit Menu(string const & name = string())
		: name_(name) {}
	///
	Menu & add(MenuItem const &, LyXView const * view = 0);
	///
	Menu & read(LyXLex &);
	///
	string const & name() const { return name_; }
	///
	bool empty() const { return items_.empty(); }
	///
	ItemList::size_type size() const { return items_.size(); }
	///
	bool hasSubmenu(string const &) const;
	///
	const_iterator begin() const {
		return items_.begin();
	}
	///
	const_iterator end() const {
		return items_.end();
	}

	// Check whether the menu shortcuts are unique
	void checkShortcuts() const;

private:
	friend class MenuBackend;
	///
	ItemList items_;
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
	///
	void add(Menu const &);
	///
	bool hasMenu(string const &) const;
	///
	Menu & getMenu(string const &);
	///
	Menu const & getMenu(string const &) const;
	///
	Menu const & getMenubar() const;
	///
	bool empty() const { return menulist_.empty(); }
	/// Expands some special entries of the menu
	/** The entries with the following kind are expanded to a
	    sequence of Command MenuItems: Lastfiles, Documents,
	    ViewFormats, ExportFormats, UpdateFormats, Branches
	*/
	void expand(Menu const & frommenu, Menu & tomenu,
		    LyXView const *) const;
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
	///
	Menu menubar_;
};

///
extern MenuBackend menubackend;

#endif /* MENUBACKEND_H */
