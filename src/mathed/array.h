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

#ifndef MATHEDARRAY_H
#define MATHEDARRAY_H

#include <vector>
#include <iosfwd>
#include "math_atom.h"

class MathInset;
class MathMacro;
class LaTeXFeatures;

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

public:
	///
	MathArray();
	///
	MathArray(MathArray const &, int from, int to);

	///
	int size() const;
	///
	bool empty() const;
	///
	void clear();
	///
	void swap(MathArray &);
	
	///
	void insert(int pos, MathInset * inset);
	///
	void insert(int pos, MathArray const &);

	///
	void erase(int pos1, int pos2);
	///
	void erase(int pos);
	///
	void erase();
	///
	int last() const;


	///
	void push_back(MathInset * inset);
	///
	void push_back(MathArray const &);
	///
	void pop_back();
	///
	MathAtom & back();

	///
	void dump(std::ostream &) const;
	///
	void dump2(std::ostream &) const;
	///
	void substitute(MathMacro const &);

	///
	MathAtom * at(int pos);
	///
	MathAtom const * at(int pos) const;
	///
	void write(std::ostream &, bool) const;
	///
	void writeNormal(std::ostream &) const;
	///
	void validate(LaTeXFeatures &) const;
	///
	const_iterator begin() const;	
	///
	const_iterator end() const;	
	///
	iterator begin();
	///
	iterator end();
private:
	/// Buffer
	buffer_type bf_;
};


std::ostream & operator<<(std::ostream & os, MathArray const & ar);

#endif
