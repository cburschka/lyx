// -*- C++ -*-
/*
 *  File:        math_root.h
 *  Purpose:     Declaration of the root object 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1999
 *  Description: Root math object
 *
 *  Copyright: 1999 Alejandro Aguilar Sierra
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifndef MATH_ROOT
#define MATH_ROOT

#ifdef __GNUG__
#pragma interface
#endif

#include "math_sqrtinset.h"
#include "symbol_def.h"


/** The general n-th root inset.
    \author Alejandro Aguilar Sierra
    \version January 1999
 */
class MathRootInset : public MathSqrtInset {
public:
	///
	explicit
	MathRootInset(short st = LM_ST_TEXT);
	///
	~MathRootInset();
	///
	MathedInset * Clone();
	///
	void draw(Painter &, int x, int baseline);
	///
	void Write(std::ostream &, bool fragile);
	///
	void Metrics();
	///
	bool Inside(int, int);
	///
	void SetFocus(int, int);
	///
	void setData(MathedArray const &);
	///
	void GetXY(int & x, int & y) const;
	///
	MathedArray & GetData();
	///
	bool setArgumentIdx(int i);
	///
	int getArgumentIdx() const;
	///
	int getMaxArgumentIdx() const;
	///
	void SetStyle(short);
private:
	///
	int idx_;
	///
	MathParInset * uroot_;
	///
	int wroot_;
	///
	int dh_;
};


inline
int MathRootInset::getArgumentIdx() const
{
	return idx_;
}


inline
int MathRootInset::getMaxArgumentIdx() const
{
	return 1;
}

#endif
