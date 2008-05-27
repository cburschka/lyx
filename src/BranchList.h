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

#include "ColorCode.h"

#include "support/docstring.h"

#include <list>


namespace lyx {


class Branch {
public:
	///
	Branch();
	///
	docstring const & getBranch() const;
	///
	void setBranch(docstring const &);
	///
	bool getSelected() const;
	/** Select/deselect the branch.
	 *  \return true if the selection status changes.
	 */
	bool setSelected(bool);
	///
	RGBColor const & getColor() const;
	///
	void setColor(RGBColor const &);
	/**
	 * Set color from a string "#rrggbb".
	 * Use Color:background if the string is no valid color.
	 * This ensures compatibility with LyX 1.4.0 that had the symbolic
	 * color "none" that was displayed as Color:background.
	 */
	void setColor(std::string const &);

private:
	///
	docstring branch_;
	///
	bool selected_;
	///
	RGBColor color_;
};


class BranchList {
	///
	typedef std::list<Branch> List;
public:
	typedef List::const_iterator const_iterator;

	///
	BranchList() : separator_(from_ascii("|")) {}

	///
	bool empty() const { return list.empty(); }
	///
	void clear() { list.clear(); }
	///
	const_iterator begin() const { return list.begin(); }
	const_iterator end() const { return list.end(); }

	/** \returns the Branch with \c name. If not found, returns 0.
	 */
	Branch * find(docstring const & name);
	Branch const * find(docstring const & name) const;

	/** Add (possibly multiple (separated by separator())) branches to list
	 *  \returns true if a branch is added.
	 */
	bool add(docstring const &);
	/** remove a branch from list by name
	 *  \returns true if a branch is removed.
	 */
	bool remove(docstring const &);

private:
	///
	List list;
	///
	docstring separator_;
};




} // namespace lyx

#endif
