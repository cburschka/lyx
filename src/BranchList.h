// -*- C++ -*- 
/**
 * \file BranchList.h 
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 * \author Martin Vermeer
 * 
 * Full author contact details are available in file CREDITS.
 *
 *
 * \class Branch
 *
 * A class describing a 'branch', i.e., a named alternative for
 * selectively outputting some parts of a document while suppressing
 * other parts.
 *
 * A branch has a name, can either be selected or not, and uses a
 * user-specifyable background colour. All these can be set and
 * queried.
 * 
 * \class BranchList
 *
 * A class containing a vector of all defined branches within a
 * document. Has methods for selecting or deselecting branches by
 * name, for outputting a '|'-separated string of all elements or only
 * the selected ones, and for adding and removing elements.
 */


#ifndef BRANCHES_H
#define BRANCHES_H

#include "support/std_string.h"
#include <list>

class Branch {
public:
	///
	string const getBranch() const;
	///
	void setBranch(string const &);
	///
	bool getSelected() const;
	///
	void setSelected(bool);
	/// 
	string const getColor() const;
	///
	void setColor(string const &);


private:
	///
	string branch_;
	///
	bool selected_;
	///
	string color_;
};


class BranchList {
public:
	///
	BranchList() : separator_("|") {}
	
	///
	typedef std::list<Branch> List;

	///
	void clear();
	///
	bool empty() { return list.empty(); }
	///
	bool size() const { return list.size(); }
	///
	List::const_iterator begin() const { return list.begin(); }
	///
	List::const_iterator end() const { return list.end(); }
	///
	string getColor(string const &) const;
	///	
	void setColor(string const &, string const &);
	/// Select/deselect multiple branches given in '|'-separated string
	void setSelected(string const &, bool);
	/// Add multiple branches to list
	void add(string const &);
	/// remove a branch from list by name
	void remove(string const &);
	/// return whether this branch is selected
	bool selected(string const &) const;
	/// return, as a '|'-separated string, all branch names
	string allBranches() const;
	/// 
	string allSelected() const;
	///
	string const separator() const;
	
private:
	///
	List list;
	///
	string separator_;
};

#endif
