// -*- C++ -*-
/*
 *  Purpose:     A general purpose resizable array.
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx>
 *  Created:     January 1996
 *
 *  Dependencies: None (almost)
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
 *                 1997  The LyX Team!
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifndef MATH_DATA_H
#define MATH_DATA_H

#include <iosfwd>
#include <vector>

#include "math_atom.h"

class MathMacro;
class LaTeXFeatures;
class ReplaceData;


#ifdef __GNUG__
#pragma interface
#endif


/** \class MathArray
    \brief Low level container for math insets

    \author Alejandro Aguilar Sierra
    \author André Pönitz
    \author Lars Gullik Bjønnes
    \version February 2001
  */

class MathArray  {
public:
	///
	typedef std::vector<MathAtom>        buffer_type;
	///
	typedef buffer_type::const_iterator  const_iterator;
	///
	typedef buffer_type::iterator        iterator;
	///
	typedef buffer_type::size_type       size_type;
	///
	typedef buffer_type::difference_type difference_type;
	///
	typedef buffer_type::size_type       idx_type;
	///
	typedef buffer_type::size_type       pos_type;

public:
	///
	MathArray();
	///
	MathArray(MathArray const & ar, size_type from, size_type to);
	///
	MathArray(iterator from, iterator to);

	///
	size_type size() const;
	///
	bool empty() const;
	///
	void clear();
	///
	void swap(MathArray &);

	/// inserts single atom at position pos
	void insert(size_type pos, MathAtom const & at);
	/// inserts multiple atoms at position pos
	void insert(size_type pos, MathArray const & ar);

	/// erase range from pos1 to pos2
	void erase(iterator pos1, iterator pos2);
	/// erase single atom
	void erase(iterator pos);
	/// erase range from pos1 to pos2
	void erase(size_type pos1, size_type pos2);
	/// erase single atom
	void erase(size_type pos);
	/// erase everythng
	void erase();

	///
	void push_back(MathAtom const & at);
	///
	void push_back(MathArray const & ar);
	///
	void pop_back();
	///
	MathAtom & back();

	///
	MathAtom & front();
	///
	MathAtom const & front() const;

	///
	void dump() const;
	///
	void dump2() const;
	///
	void substitute(MathMacro const & macro);
	///
	void replace(ReplaceData &);

	/// looks for exact match
	bool match(MathArray const & ar) const;
	/// looks for inclusion match starting at pos
	bool matchpart(MathArray const & ar, pos_type pos) const;
	/// looks for containment, return == size mean not found
	size_type find(MathArray const & ar) const;
	/// looks for containment, return == size mean not found
	size_type find_last(MathArray const & ar) const;
	///
	bool contains(MathArray const & ar) const;

	/// write acccess to single atom
	MathAtom & operator[](size_type pos) { return at(pos); }
	/// read access o single atom
	MathAtom const & operator[](size_type pos) const { return at(pos); }
	///
	const_iterator begin() const;
	///
	const_iterator end() const;
	///
	iterator begin();
	///
	iterator end();

	///
	void validate(LaTeXFeatures &) const;

private:
	/// is this an exact match at this position?
	bool find1(MathArray const & ar, size_type pos) const;
	/// write acccess to single atom
	MathAtom & at(size_type pos);
	/// read access o single atom
	MathAtom const & at(size_type pos) const;

	/// Buffer
	buffer_type bf_;
};

///
std::ostream & operator<<(std::ostream & os, MathArray const & ar);


#endif
