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

class MathInset;
class MathScriptInset;
class MathMacro;
class Painter;

#ifdef __GNUG__
#pragma interface
#endif

/** \class MathArray
    \brief A resizable array.
    
    A general purpose resizable array.
    
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
	void insert(int pos, char, MathTextCodes);
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
	void push_back(char, MathTextCodes);
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
	MathInset * GetInset(int pos) const;
	///
	MathScriptInset * prevScriptInset(int pos) const;
	///
	char GetChar(int pos) const;
	///
	MathTextCodes GetCode(int pos) const;
	///
	bool isInset(int pos) const;
	///
	void Write(std::ostream &, bool) const;
	///
	void WriteNormal(std::ostream &) const;
private:
	///
	typedef std::vector<byte>           buffer_type;
	///
	typedef byte                        value_type;

	///
	int item_size(int pos) const;
	/// Buffer
	buffer_type bf_;
};


std::ostream & operator<<(std::ostream & os, MathArray const & ar);

#endif
