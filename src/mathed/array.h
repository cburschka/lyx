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

#include "mathed/support.h"
#include "math_defs.h"
#include "LString.h"

class MathInset;
class MathMacro;
class Painter;
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
	MathArray();
	///
	MathArray(MathArray const &);
	///
	MathArray & operator=(MathArray const &);
	///
	~MathArray();

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
	void insert(int pos, unsigned char, MathTextCodes);
	///
	void insert(int pos, MathArray const &);

	///
	void erase(int pos1, int pos2);
	///
	void erase(int pos);
	///
	void replace(int pos, MathInset * inset);
	///
	bool prev(int & pos) const;
	///
	bool next(int & pos) const;
	///
	bool last(int & pos) const;


	///
	void push_back(MathInset * inset);
	///
	void push_back(unsigned char, MathTextCodes);
	///
	void push_back(MathArray const &);
	///
	MathInset * back_inset() const;

	///
	void dump(std::ostream &) const;
	///
	void dump2(std::ostream &) const;
	///
	void substitute(MathMacro const &);
	///

	///
	MathInset * nextInset(int pos) const;
	///
	MathInset * prevInset(int pos) const;
	///
	unsigned char GetChar(int pos) const;
	/// read subsequent chars of the same kind.
	// pos is afterwards one behind the last char belonging to the string
	string GetString(int & pos) const;
	///
	MathTextCodes GetCode(int pos) const;
	///
	void setCode(int pos, MathTextCodes t);
	///
	bool isInset(int pos) const;
	///
	void Write(std::ostream &, bool) const;
	///
	void WriteNormal(std::ostream &) const;
	///
	void Validate(LaTeXFeatures &) const;
private:
	///
	typedef std::vector<unsigned char>           buffer_type;
	///
	typedef unsigned char                        value_type;

	///
	int item_size(int pos) const;
	/// Buffer
	buffer_type bf_;
};


std::ostream & operator<<(std::ostream & os, MathArray const & ar);

#endif
