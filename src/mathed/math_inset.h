// -*- C++ -*-
/*
 *  File:        math_inset.h
 *  Purpose:     Declaration of insets for mathed 
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: Math paragraph and objects for a WYSIWYG math editor.
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, 1997 Alejandro Aguilar Sierra
 *
 *   Version: 0.8beta, Math & Lyx project.
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

//  Note: These math insets are internal to Math and are not derived
//        from lyx inset.

#ifndef MATH_INSET_H
#define MATH_INSET_H

#ifdef __GNUG__
#pragma interface
#endif

#include "LString.h"
#include "symbol_def.h"
#include "xarray.h"

/** Abstract base class for all math objects.
    A math insets is for use of the math editor only, it isn't a
    general LyX inset. It's used to represent all the math objects.
    The formulaInset (a LyX inset) encapsulates a math inset.
*/


class MathInset {
public: 
	/** A math inset has a name (usually its LaTeX name),
	    type and font-size
	*/
	///
	explicit
	MathInset (string const & nm = string(), short ot = LM_OT_SIMPLE, int na = 0);
	/// The virtual base destructor
	virtual ~MathInset() {}

	/// Draw the object
	virtual void draw(Painter &, int x, int baseline) = 0;	
	/// Write LaTeX and Lyx code
	virtual void Write(std::ostream &, bool fragile) const = 0;
	/// Write normalized content
	virtual void WriteNormal(std::ostream &) const;
	/// Reproduces itself
	virtual MathInset * Clone() const = 0;
	/// Appends itself with macro arguments substituted
	virtual void substitute(MathArray & array, MathMacro const & macro) const;
	/// Compute the size of the object
	virtual void Metrics(MathStyles st) = 0; 
	/// 
	virtual int ascent() const;
	///
	virtual int descent() const;
	///
	virtual int width() const;
	///
	virtual int height() const;
	///
	virtual bool GetLimits() const;
	///
	virtual void SetLimits(bool);
	///
	string const & name() const;
	///
	short GetType() const;
	//Man:  Avoid to use these functions if it's not strictly necessary 
	///
	virtual void SetType(short t);
	///
	virtual void SetName(string const & n);
	///
	MathStyles size() const;

	/// Where should we go when we press the up cursor key?
	virtual bool idxUp(int & idx, int & pos) const;
	/// The down key
	virtual bool idxDown(int & idx, int & pos) const;
	/// The left key
	virtual bool idxLeft(int & idx, int & pos) const;
	/// The right key
	virtual bool idxRight(int & idx, int & pos) const;
	/// Where should we go when we enter the inset from the left?
	virtual bool idxFirst(int & idx, int & pos) const;
	/// Where should we go when we enter the inset from the right?
	virtual bool idxLast(int & idx, int & pos) const;

	/// Where should we go if we press home?
	virtual bool idxHome(int & idx, int & pos) const;
	/// Where should we go if we press end?
	virtual bool idxEnd(int & idx, int & pos) const;

	///
	int nargs() const;

	///
	MathArray & cell(int);
	///
	MathArray const & cell(int) const;
	///
	MathXArray & xcell(int);
	///
	MathXArray const & xcell(int) const;
	///
	void setData(MathArray const &, int);
			
	///
	int xo() const;
	///
	int yo() const;
	///
	void xo(int tx);
	///
	void yo(int ty);
	///

	///
	virtual int ncols() const { return 1; }
	///
	virtual int nrows() const { return 1; }
	///
	virtual int col(int) const { return 0; }
	///
	virtual int row(int) const { return 0; }
	///
	virtual void addRow(int) {}
	///
	virtual void delRow(int) {}
	///
	virtual void addCol(int) {}
	///
	virtual void delCol(int) {}

	///
	virtual void UserSetSize(MathStyles &) {}

	///
	void GetXY(int & x, int & y) const;
	///
	bool covers(int x, int y) const;


	///
	void push_back(MathInset *);
	///
	void push_back(byte ch, MathTextCodes fcode);
	///
	void dump() const;

	///
	///
	static int workwidth;
protected:
	///
	string name_;
	///
	short objtype;
	///
	int width_;
	///
	int ascent_;
	///
	int descent_;
	///
	void size(MathStyles s);
	///
	MathStyles size_;

protected:
	///
	typedef std::vector<MathXArray> cells_type;
	/**
	 * The contents of the inset are contained here.
	 * Each inset is build from a number of insets.
	 * For instance, a
	 */
	cells_type cells_;

private:
	/// Cursor start position in pixels from the document top
	int xo_;
	///
	int yo_;
};

std::ostream & operator<<(std::ostream &, MathInset const &);

#endif
