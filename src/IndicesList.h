// -*- C++ -*-
/**
 * \file IndicesList.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 * \author Jürgen Spitzmüller
 *
 * Full author contact details are available in file CREDITS.
 *
 *
 * \class Index
 *
 * A class describing an Index type, such as "Index of Names".
 * Different Index types are used in splitted Indices
 *
 * An Index has a name and a shortcut notation. It uses a
 * user-specifyable GUI colour. All these can be set and
 * queried.
 *
 * \class IndicesList
 *
 * A class containing a vector of all defined indices within a
 * document. Has methods for outputting a '|'-separated string 
 * of all elements, and for adding, removing and renaming elements.
 */


#ifndef INDICESLIST_H
#define INDICESLIST_H

#include "ColorCode.h"

#include "support/docstring.h"

#include <list>


namespace lyx {

class Index {
public:
	///
	Index();
	///
	docstring const & index() const;
	///
	void setIndex(docstring const &);
	///
	docstring const & shortcut() const;
	///
	void setShortcut(docstring const &);
	///
	RGBColor const & color() const;
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
	docstring index_;
	///
	docstring shortcut_;
	///
	RGBColor color_;
};


class IndicesList {
	///
	typedef std::list<Index> List;
public:
	typedef List::const_iterator const_iterator;

	///
	IndicesList() : separator_(from_ascii("|")) {}

	///
	bool empty() const { return list.empty(); }
	///
	void clear() { list.clear(); }
	///
	const_iterator begin() const { return list.begin(); }
	const_iterator end() const { return list.end(); }

	/** \returns the Index with \c name. If not found, returns 0.
	 */
	Index * find(docstring const & name);
	Index const * find(docstring const & name) const;

	/** \returns the Index with the shortcut \c shortcut. 
	 *  If not found, returns 0.
	 */
	Index * findShortcut(docstring const & shortcut);
	Index const * findShortcut(docstring const & shortcut) const;

	/** Add (possibly multiple (separated by separator())) indices to list
	 *  \returns true if an index is added.
	 */
	bool add(docstring const & n, docstring const & s = docstring());
	/** Add the default index (if not already there)
	 *  \returns true if an index is added.
	 */
	bool addDefault(docstring const & n);
	/** remove an index from list by name
	 *  \returns true if an index is removed.
	 */
	bool remove(docstring const &);
	/** rename an index in list
	 *  \returns true if renaming succeeded.
	 */
	bool rename(docstring const &, docstring const &);

private:
	///
	List list;
	///
	docstring separator_;
};

} // namespace lyx

#endif // INDICESLIST_H
