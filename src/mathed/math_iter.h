// -*- C++ -*-
/*
 *  File:        math_iter.h
 *  Purpose:     Iterator for Math paragraphs
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1997
 *  Description: Using iterators is the only way to handle math paragraphs 
 *
 *  Dependencies: Xlib
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta, Mathed & Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 * 
 */

#ifndef MATH_ITER
#define MATH_ITER

#ifdef __GNUG__
#pragma interface
#endif

#include "mathed/support.h"
#include "math_defs.h"

class MathedInset;
class MathedArray;
class MathParInset;

///
enum mathIterFlags {
	/// Allow newlines
	MthIF_CR = 1,
	/// Allow tabs
	MthIF_Tabs = 2
};


/** Specialized array iterator for math paragraph.
    Used for storing and querying data operations
*/
class MathedIter {
public:
	///
	MathedIter();
	///
	explicit
	MathedIter(MathedArray *);
	///
	virtual ~MathedIter() {}
	///
	bool goNextCode(MathedTextCodes);
	///
	void goPosAbs(int);
	///
	int Empty() const;
	///
	int OK() const;
	///
	int IsFirst() const { return (pos == 0); }
	///
	byte GetChar() const;
	///
	string const GetString() const;
	///
	MathedInset * GetInset() const;
	///
	MathParInset * GetActiveInset() const;
	///
	bool IsInset() const;
	///
	bool IsActive() const;
	///
	bool IsFont() const;
	///
	bool IsScript() const;
	///
	bool IsTab() const;
	///
	bool IsCR() const;
	///
	virtual void Reset();
	///
	virtual void Insert(byte, MathedTextCodes c = LM_TC_CONST);
	///
	virtual void Insert(MathedInset *, int t = LM_TC_INSET);
	///
	virtual bool Delete();
	///
	virtual bool Next();
	/// Check consistency of tabs and newlines
	void checkTabs();
	/// Try to adjust tabs in the expected place, as in eqnarrays
	void adjustTabs();
	///
	short fcode() const;
	///
	void fcode(short) const;
	///
	int getPos() const { return pos; }
	///
	int getRow() const { return row; }
	///
	int getCol() const { return col; }
	///
	void setNumCols(int n) { ncols = n; }
	///
	void SetData(MathedArray * a);
	///
	MathedArray * GetData() const;
	/// Copy every object from position p1 to p2
	MathedArray * Copy(int p1 = 0, int p2 = 10000);
	/// Delete every object from position p1 to p2
	void Clear();
protected:
	///
	void split(int);
	///
	void join(int);
	///
	int flags;
	///
	mutable short fcode_;
	///
	mutable int pos;
	///
	int row;
	///
	int col;
	///
	int ncols;
	///
	MathedArray * array;
	// one element stack
	struct MIState {
		///
		short fcode;
		///
		int x;
		///
		int y;
		///
		int pos;
		///
		int row;
		///
		int col;
	};
	///
	MIState stck;
	/// Saves the current state of the iterator
	virtual void ipush();
	/// Recover previous state
	virtual void ipop();
};

///
//#define MX_WAS_SUB   1
///
//#define MX_WAS_SUPER 2

#endif
