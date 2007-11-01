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

#include "FuncStatus.h"
#include "FuncRequest.h"

#include <boost/shared_ptr.hpp>

#include <vector>
#include <stack>


namespace lyx {

class Lexer;
class Buffer;
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
		/** This is the bookmarks */
		Bookmarks,
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
		/** This is the list of elements available
		 * for insertion into document. */
		CharStyles,
		/** This is the list of user-configurable
		insets to insert into document */
		Custom,
		/** This is the list of XML elements to
		insert into the document */
		Elements,
		/** This is the list of floats that we can
		    insert a list for. */
		FloatListInsert,
		/** This is the list of floats that we can
		    insert. */
		FloatInsert,
		/** This is the list of selections that can
		    be pasted. */
		PasteRecent,
		/** toolbars */
		Toolbars,
		/** Available branches in document */
		Branches
	};

	explicit MenuItem(Kind kind);

	MenuItem(Kind kind,
		 docstring const & label,
		 docstring const & submenu = docstring(),
		 bool optional = false);

	MenuItem(Kind kind,
		 docstring const & label,
		 FuncRequest const & func,
		 bool optional = false);

	/// This one is just to please boost::shared_ptr<>
	~MenuItem();
	/// The label of a given menuitem
	docstring const label() const;
	/// The keyboard shortcut (usually underlined in the entry)
	docstring const shortcut() const;
	/// The complete label, with label and shortcut separated by a '|'
	docstring const fulllabel() const { return label_;}
	/// The kind of entry
	Kind kind() const { return kind_; }
	/// the action (if relevant)
	FuncRequest const & func() const { return func_; }
	/// returns true if the entry should be ommited when disabled
	bool optional() const { return optional_; }
	/// returns the status of the lfun associated with this entry
	FuncStatus const & status() const { return status_; }
	/// returns the status of the lfun associated with this entry
	FuncStatus & status() { return status_; }
	/// returns the status of the lfun associated with this entry
	void status(FuncStatus const & status) { status_ = status; }
	/**
	 * returns the binding associated to this action.
	 * Use the native UI format when \c forgui is true.
	 */
	docstring const binding(bool forgui) const;
	/// the description of the  submenu (if relevant)
	docstring const & submenuname() const { return submenuname_; }
	/// set the description of the  submenu
	void submenuname(docstring const & name) { submenuname_ = name; }
	///
	Menu * submenu() const { return submenu_.get(); }
	///
	void submenu(Menu * menu);

private:
	//friend class MenuBackend;
	///
	Kind kind_;
	///
	docstring label_;
	///
	FuncRequest func_;
	///
	docstring submenuname_;
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
	explicit Menu(docstring const & name = docstring()) : name_(name) {}
	/// Add the menu item unconditionally
	Menu & add(MenuItem const &);
	/// Checks the associated FuncRequest status before adding the
	/// menu item.
	Menu & addWithStatusCheck(MenuItem const &);
	///
	Menu & read(Lexer &);
	///
	docstring const & name() const { return name_; }
	///
	bool empty() const { return items_.empty(); }
	/// Clear the menu content.
	void clear() { items_.clear(); }
	///
	ItemList::size_type size() const { return items_.size(); }
	///
	MenuItem const & operator[](size_type) const;
	///
	bool hasFunc(FuncRequest const &) const;
	///
	const_iterator begin() const { return items_.begin(); }
	///
	const_iterator end() const { return items_.end(); }

	// Check whether the menu shortcuts are unique
	void checkShortcuts() const;
	
	// search for func in this menu iteratively, and put menu
	// names in a stack.
	bool searchFunc(FuncRequest & func, std::stack<docstring> & names);

private:
	friend class MenuBackend;
	///
	ItemList items_;
	///
	docstring name_;
};


///
class MenuBackend {
public:
	///
	typedef std::vector<Menu> MenuList;
	///
	typedef MenuList::const_iterator const_iterator;
	///
	typedef MenuList::iterator iterator;
	///
	MenuBackend() {}
	///
	void read(Lexer &);
	///
	void add(Menu const &);
	///
	bool hasMenu(docstring const &) const;
	///
	Menu & getMenu(docstring const &);
	///
	Menu const & getMenu(docstring const &) const;
	///
	Menu const & getMenubar() const;
	///
	bool empty() const { return menulist_.empty(); }
	/** This defines a menu whose entries list the FuncRequests
	    that will be removed by expand() in other menus. This is
	    used by the Qt/Mac code
	*/
	void specialMenu(Menu const &);
	///
	Menu const & specialMenu() { return specialmenu_; }

	/// Expands some special entries of the menu
	/** The entries with the following kind are expanded to a
	    sequence of Command MenuItems: Lastfiles, Documents,
	    ViewFormats, ExportFormats, UpdateFormats, Branches
	*/
	void expand(Menu const & frommenu, Menu & tomenu,
		    Buffer const *) const;
	///
	const_iterator begin() const { return menulist_.begin(); }
	///
	iterator begin() { return menulist_.begin(); }
	///
	const_iterator end() const { return menulist_.end(); }
	///
	iterator end() { return menulist_.end(); }

private:
	///
	MenuList menulist_;
	///
	Menu menubar_;
	///
	Menu specialmenu_;
};

///
extern MenuBackend menubackend;


} // namespace lyx

#endif // MENUBACKEND_H
