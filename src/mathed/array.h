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
#include "LString.h"

class MathScriptInset;
class MathMacro;
class MathWriteInfo;
class MathMetricsInfo;
class MathMLStream;
class MapleStream;
class OctaveStream;
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
	///
	typedef buffer_type::size_type       size_type;

public:
	///
	MathArray();
	///
	MathArray(MathArray const &, size_type from, size_type to);

	///
	size_type size() const;
	///
	bool empty() const;
	///
	void clear();
	///
	void swap(MathArray &);
	
	///
	void insert(size_type pos, MathAtom const &);
	///
	void insert(size_type pos, MathArray const &);

	///
	void erase(size_type pos1, size_type pos2);
	///
	void erase(size_type pos);
	///
	void erase();

	///
	void push_back(MathAtom const &);
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
	MathAtom & at(size_type pos);
	///
	MathAtom const & at(size_type pos) const;
	/// glue chars if necessary
	void write(MathWriteInfo & os) const;
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
	///
	MathScriptInset const * asScript(const_iterator it) const;
	/// glues chars with the same attributes into MathStringInsets
	MathArray glueChars() const;
	/// insert asterisks in "suitable" places
	MathArray guessAsterisks() const;

	/// interface to Octave
	void octavize(OctaveStream &) const;
	/// interface to Maple
	void maplize(MapleStream &) const;
	/// interface to MathML
	void mathmlize(MathMLStream &) const;

	///
	bool isMatrix() const;

private:
	/// Buffer
	buffer_type bf_;
};


std::ostream & operator<<(std::ostream & os, MathArray const & ar);


#endif
