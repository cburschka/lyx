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

#include <string>
#include <list>


class Branch {
public:
	///
	std::string const getBranch() const;
	///
	void setBranch(std::string const &);
	///
	bool getSelected() const;
	///
	void setSelected(bool);
	///
	std::string const getColor() const;
	///
	void setColor(std::string const &);


private:
	///
	std::string branch_;
	///
	bool selected_;
	///
	std::string color_;
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
	std::string getColor(std::string const &) const;
	///
	void setColor(std::string const &, std::string const &);
	/// Select/deselect multiple branches given in '|'-separated string
	void setSelected(std::string const &, bool);
	/// Add multiple branches to list
	void add(std::string const &);
	/// remove a branch from list by name
	void remove(std::string const &);
	/// return whether this branch is selected
	bool selected(std::string const &) const;
	/// return, as a '|'-separated string, all branch names
	std::string allBranches() const;
	///
	std::string allSelected() const;
	///
	std::string const separator() const;

private:
	///
	List list;
	///
	std::string separator_;
};

#endif
