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

class MathArray : private std::vector<MathAtom> {
public:
	/// re-use inhertited stuff
	typedef std::vector<MathAtom> base_type;
	using base_type::const_iterator;
	using base_type::iterator;
	using base_type::size_type;
	using base_type::difference_type;
	using base_type::size;
	using base_type::empty;
	using base_type::clear;
	using base_type::begin;
	using base_type::end;
	using base_type::push_back;
	using base_type::pop_back;
	using base_type::back;
	using base_type::front;
	using base_type::swap;
	///
	typedef size_type idx_type;
	typedef size_type pos_type;
	
public:
	///
	MathArray() {}
	///
	MathArray(const_iterator from, const_iterator to);
	///
	void append(MathArray const & ar);

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
	///
	void validate(LaTeXFeatures &) const;

	/// checked write access
	MathAtom & operator[](pos_type);
	/// checked read access
	MathAtom const & operator[](pos_type) const;
private:
	/// is this an exact match at this position?
	bool find1(MathArray const & ar, size_type pos) const;
};

///
std::ostream & operator<<(std::ostream & os, MathArray const & ar);


#endif
