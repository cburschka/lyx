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

#include <config.h>

#ifdef __GNUG__
#pragma interface
#endif

#include "symbol_def.h"
#include "xarray.h"

/** Abstract base class for all math objects.
    A math insets is for use of the math editor only, it isn't a
    general LyX inset. It's used to represent all the math objects.
    The formulaInset (a LyX inset) encapsulates a math inset.
*/


class LaTeXFeatures;

class MathInset {
public: 
	///
	MathInset();

	/// the virtual base destructor
	virtual ~MathInset() {}

	/// draw the object, sets xo_ and yo_ cached values 
	virtual void draw(Painter &, int x, int y) const;
	/// write LaTeX and Lyx code
	virtual void write(std::ostream &, bool fragile) const;
	/// write normalized content
	virtual void writeNormal(std::ostream &) const;
	/// reproduce itself
	virtual MathInset * clone() const = 0;
	/// appends itself with macro arguments substituted
	virtual void substitute(MathArray & array, MathMacro const & macro) const;
	/// compute the size of the object, sets ascend_, descend_ and width_
	virtual void metrics(MathStyles st) const;
	/// 
	virtual int ascent() const { return 1; }
	///
	virtual int descent() const { return 1; }
	///
	virtual int width() const { return 2; }
	///
	virtual int height() const;
	///
	virtual MathStyles size() const;

	/// Where should we go when we press the up cursor key?
	virtual bool idxUp(int & idx, int & pos) const;
	/// The down key
	virtual bool idxDown(int & idx, int & pos) const;
	/// The left key
	virtual bool idxLeft(int & idx, int & pos) const;
	/// The right key
	virtual bool idxRight(int & idx, int & pos) const;

	/// Move one physical cell up
	virtual bool idxNext(int & idx, int & pos) const;
	/// Move one physical cell down
	virtual bool idxPrev(int & idx, int & pos) const;

	/// Target pos when we enter the inset from the left by pressing "Right"
	virtual bool idxFirst(int & idx, int & pos) const;
	/// Target pos when we enter the inset from the left by pressing "Up"
	virtual bool idxFirstUp(int & idx, int & pos) const;
	/// Target pos when we enter the inset from the left by pressing "Down"
	virtual bool idxFirstDown(int & idx, int & pos) const;

	/// Target pos when we enter the inset from the right by pressing "Left"
	virtual bool idxLast(int & idx, int & pos) const;
	/// Target pos when we enter the inset from the right by pressing "Up"
	virtual bool idxLastUp(int & idx, int & pos) const;
	/// Target pos when we enter the inset from the right by pressing "Down"
	virtual bool idxLastDown(int & idx, int & pos) const;

	/// Where should we go if we press home?
	virtual bool idxHome(int & idx, int & pos) const;
	/// Where should we go if we press end?
	virtual bool idxEnd(int & idx, int & pos) const;

	/// Delete a cell and move cursor
	// the return value indicates whether the cursor should leave the inset
	// and/or the whole inset should be deleted
	virtual void idxDelete(int & idx, bool & popit, bool & deleteit);
	// deletes a cell range and moves the cursor 
	virtual void idxDeleteRange(int from, int to);
	// returns list of cell indices that are "between" from and to for
	// selection purposes
	virtual std::vector<int> idxBetween(int from, int to) const;

	///
	virtual int nargs() const;

	///
	virtual MathArray & cell(int);
	///
	virtual MathArray const & cell(int) const;
	///
	virtual MathXArray & xcell(int);
	///
	virtual MathXArray const & xcell(int) const;
			
	///
	virtual int xo() const;
	///
	virtual int yo() const;
	///
	virtual void xo(int tx) const;
	///
	virtual void yo(int ty) const;
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
	virtual int cellXOffset(int) const { return 0; }
	///
	virtual int cellYOffset(int) const { return 0; }
	///
	virtual void addRow(int) {}
	///
	virtual void delRow(int) {}
	///
	virtual void addCol(int) {}
	///
	virtual void delCol(int) {}

	///
	virtual void userSetSize(MathStyles &) {}

	///
	virtual void getXY(int & x, int & y) const;
	///
	virtual bool covers(int x, int y) const;
	/// identifies things that can get scripts
	virtual bool isScriptable() const { return false; }
	/// identifies ScriptInsets
	virtual bool isScriptInset() const { return false; }
	/// identifies SpaceInsets
	virtual bool isSpaceInset() const { return false; }
	/// identifies GridInsets
	virtual bool isGrid() const { return false; }
	/// identifies ArrayInsets
	virtual bool isArray() const { return false; }
	/// identifies Charinsets
	virtual bool isCharInset() const { return false; }
	///
	virtual bool isActive() const { return nargs() > 0; }
	///
	virtual bool isRelOp() const { return false; }
	///
	virtual char getChar() const { return 0; }
	///
	virtual MathTextCodes code() const { return LM_TC_MIN; }

	///
	virtual void push_back(MathInset *);
	///
	virtual void push_back(unsigned char c, MathTextCodes code);
	///
	virtual void dump() const;

	///
	virtual void validate(LaTeXFeatures & features) const;
	///
	virtual void handleFont(MathTextCodes) {}

	///
	static int workwidth;

protected:
	/// _sets_ style
	void size(MathStyles s) const;
	/// the used font size
	mutable MathStyles size_;

private:
	/// the following are used for positioning the cursor with the mouse
	/// cached cursor start position in pixels from the document left
	mutable int xo_;
	/// cached cursor start position in pixels from the document top
	mutable int yo_;
};

std::ostream & operator<<(std::ostream &, MathInset const &);

#endif
