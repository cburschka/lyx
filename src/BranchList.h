// -*- C++ -*-
/**
 * \file BranchList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Martin Vermeer
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef BRANCHLIST_H
#define BRANCHLIST_H

#include "ColorCode.h"

#include "support/docstring.h"

#include <list>


namespace lyx {

/**
 * \class Branch
 *
 * A class describing a 'branch', i.e., a named alternative for
 * selectively outputting some parts of a document while suppressing
 * other parts.
 *
 * A branch has a name, can either be selected or not, and uses a
 * user-specifiable background colour. All these can be set and
 * queried.
 *
 * \class BranchList
 *
 * A class containing a vector of all defined branches within a
 * document. It has methods for selecting or deselecting branches by
 * name, for outputting a '|'-separated string of all elements or only
 * the selected ones, and for adding and removing elements.
 */

class Branch {
public:
	///
	Branch();
	///
	docstring const & branch() const;
	///
	void setBranch(docstring const &);
	///
	bool isSelected() const;
	/** Select/deselect the branch.
	 *  \return true if the selection status changes.
	 */
	bool setSelected(bool);
	/** If true, the branch name will be appended
	 *  to the output file name.
	 */
	bool hasFileNameSuffix() const;
	/// Select/deselect filename suffix property.
	void setFileNameSuffix(bool);
	///
	void setListID(int const id) { branch_list_id_ = id; }
	///
	std::string const & color() const;
	///
	std::string const & lightModeColor() const;
	///
	std::string const & darkModeColor() const;
	/**
	 * Set background color from a hexcolor string "#rrggbb" or a lyx color name.
	 * Use Color:background if the string is no valid color.
	 * This ensures compatibility with LyX 1.4.0 that had the symbolic
	 * color "none" that was displayed as Color:background.
	 * This sets the dark color if in dark mode, else the light color.
	 */
	void setColor(std::string const & color);
	/// Set dark and light background colors
	void setColors(std::string const & color,
		      std::string const & dmcolor = std::string());
	///
	int listID() const { return branch_list_id_; }

private:
	///
	docstring branch_;
	///
	bool selected_;
	///
	bool filenameSuffix_;
	/// light mode background color
	std::string lmcolor_;
	/// dark mode background color
	std::string dmcolor_;
	///
	int branch_list_id_;
};


class BranchList {
	///
	typedef std::list<Branch> List;
public:
	typedef List::const_iterator const_iterator;

	///
	BranchList() : separator_(from_ascii("|")), id_(rand()) {}

	///
	docstring separator() const { return separator_; }

	///
	int id() const { return id_; }

	///
	bool empty() const { return list_.empty(); }
	///
	void clear() { list_.clear(); }
	///
	const_iterator begin() const { return list_.begin(); }
	const_iterator end() const { return list_.end(); }

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
	/** rename an branch in list
	 *  \returns true if renaming succeeded.
	 * if \p merge is true, the branch will be removed
	 * if a branch with the newname already exists.
	 */
	bool rename(docstring const &, docstring const &, bool const merge = false);
	/// get the complete filename suffix
	docstring getFileNameSuffix() const;

private:
	///
	List list_;
	///
	docstring separator_;
	///
	int id_;
};

} // namespace lyx

#endif // BRANCHLIST_H
