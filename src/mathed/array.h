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

class MathedInset;

#ifdef __GNUG__
#pragma interface
#endif

/** \class MathedArray
    \brief A resizable array.
    
    A general purpose resizable array.
    
    \author Alejandro Aguilar Sierra
    \author André Pönitz
    \author Lars Gullik Bjønnes
    \version February 2001
  */
class MathedArray  {
public:
	///
	typedef std::vector<byte>           buffer_type;
	typedef byte                        value_type;
	typedef buffer_type::size_type      size_type;
	typedef buffer_type::iterator       iterator;
	typedef buffer_type::const_iterator const_iterator;
	
	///
	MathedArray();
	///
	MathedArray(MathedArray const &);
	///
	MathedArray & operator=(MathedArray const &);
	///
	~MathedArray();

	///
	iterator begin();
	///
	iterator end();
	///
	const_iterator begin() const;
	///
	const_iterator end() const;
	
	///
	int empty() const;
	///
	void clear();
   
	///
	int last() const;
	///
	void last(int l);

	///
	void swap(MathedArray &);
	///
	void shrink(int pos1, int pos2);

#if 0
	///
	void insert(iterator pos, const_iterator beg, const_iterator end);
#else
	/// Merge \a dx elements from array \a a at \apos.
	/// This doesn't changes the size (dangerous)
	void merge(MathedArray const & a, int pos); 
#endif
	///
	void raw_pointer_copy(MathedInset ** p, int pos) const;
#if 0
	///
	void insertInset(int pos, MathedInset * p, int type);
	///
	MathedInset * getInset(int pos);
#else
	///
	void raw_pointer_insert(void * p, int pos, int len);
#endif
	///
	void strange_copy(MathedArray * dest, int dpos, int spos, int len);
	///
	byte operator[](int) const;
	///
	byte & operator[](int i);
	
	///
	void move(int p, int shift);
	///
	void resize(int newsize);
	/// Make sure we can access at least \a needed elements
	void need_size(int needed);
	///
	void dump(ostream &) const;
private:
	/// Buffer
	buffer_type bf_;
#if 0
	///
	struct InsetTable {
		///
		int pos;
		///
		MathedInset * inset;
		///
		InsetTable(int p, MathedInset * i)
			: pos(p), inset(i) {}
		
	};
	/// 
	typedef std::vector<InsetTable> InsetList;
	/// The list of insets in this array.
	InsetList insetList_;
#endif
	/// Last position inserted.
	int last_;
};
#endif
