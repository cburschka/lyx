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

#include "xarray.h"
#include "math_defs.h"
#include "math_mathmlstream.h"

/** Abstract base class for all math objects.
    A math insets is for use of the math editor only, it isn't a
    general LyX inset. It's used to represent all the math objects.
    The formulaInset (a LyX inset) encapsulates a math inset.
*/


class MathArrayInset;
class MathBoxInset;
class MathCharInset;
class MathGridInset;
class MathNestInset;
class MathMatrixInset;
class MathScriptInset;
class MathSpaceInset;
class MathMacroTemplate;

class LaTeXFeatures;
class Buffer;
class BufferView;
class UpdatableInset;
class MathOctaveInfo;


struct MathWriteInfo {
	///
	MathWriteInfo(Buffer const * buffer_, std::ostream & os_, bool fragile_)
		: buffer(buffer_), os(os_), fragile(fragile_)
	{}
	///
	explicit MathWriteInfo(std::ostream & os_)
		: buffer(0), os(os_), fragile(false)
	{}

	///
	template <class T>
	MathWriteInfo & operator<<(T const & t)
	{
		os << t;
		return *this;
	}
	///
	MathWriteInfo & operator<<(MathArray const & ar)
	{
		ar.write(*this);
		return *this;
	}


	///
	Buffer const * buffer;
	///
	std::ostream & os;
	///
	bool fragile;
};


class MathInset {
public: 
	/// short of anything else reasonable
	typedef MathArray::size_type     size_type;
	/// type for cursor positions within a cell
	typedef MathArray::size_type     pos_type;
	/// type for cell indices
	typedef size_type                idx_type;
	/// type for row numbers
	typedef size_type                row_type;
	/// type for column numbers
	typedef size_type                col_type;

	///
	MathInset();
	/// the virtual base destructor
	virtual ~MathInset(); 

	/// draw the object
	virtual void draw(Painter &, int x, int y) const;
	/// write LaTeX and Lyx code
	virtual void write(MathWriteInfo & os) const;
	/// write normalized content
	virtual void writeNormal(std::ostream &) const;
	/// reproduce itself
	virtual MathInset * clone() const = 0;
	///substitutes macro arguments if necessary
	virtual void substitute(MathMacro const & macro);
	/// compute the size of the object, sets ascend_, descend_ and width_
	virtual void metrics(MathMetricsInfo const & st) const;
	/// 
	virtual int ascent() const { return 1; }
	///
	virtual int descent() const { return 1; }
	///
	virtual int width() const { return 2; }
	///
	virtual int height() const;

	/// Where should we go when we press the up cursor key?
	virtual bool idxUp(idx_type & idx, pos_type & pos) const;
	/// The down key
	virtual bool idxDown(idx_type & idx, pos_type & pos) const;
	/// The left key
	virtual bool idxLeft(idx_type & idx, pos_type & pos) const;
	/// The right key
	virtual bool idxRight(idx_type & idx, pos_type & pos) const;

	/// Move one physical cell up
	virtual bool idxNext(idx_type & idx, pos_type & pos) const;
	/// Move one physical cell down
	virtual bool idxPrev(idx_type & idx, pos_type & pos) const;

	/// Target pos when we enter the inset from the left by pressing "Right"
	virtual bool idxFirst(idx_type & idx, pos_type & pos) const;
	/// Target pos when we enter the inset from the right by pressing "Left"
	virtual bool idxLast(idx_type & idx, pos_type & pos) const;

	/// Where should we go if we press home?
	virtual bool idxHome(idx_type & idx, pos_type & pos) const;
	/// Where should we go if we press end?
	virtual bool idxEnd(idx_type & idx, pos_type & pos) const;

	/// Delete a cell and move cursor
	// the return value indicates whether the cursor should leave the inset
	// and/or the whole inset should be deleted
	virtual void idxDelete(idx_type & idx, bool & popit, bool & deleteit);
	// deletes a cell range and moves the cursor 
	virtual void idxDeleteRange(idx_type from, idx_type to);
	// returns list of cell indices that are "between" from and to for
	// selection purposes
	virtual std::vector<idx_type> idxBetween(idx_type from, idx_type to) const;

	///
	virtual idx_type nargs() const;

	///
	virtual MathArray & cell(idx_type);
	///
	virtual MathArray const & cell(idx_type) const;
	///
	virtual MathXArray & xcell(idx_type);
	///
	virtual MathXArray const & xcell(idx_type) const;
			
	///
	virtual col_type ncols() const { return 1; }
	///
	virtual row_type nrows() const { return 1; }
	///
	virtual col_type col(row_type) const { return 0; }
	///
	virtual row_type row(row_type) const { return 0; }
	///
	virtual int cellXOffset(row_type) const { return 0; }
	///
	virtual int cellYOffset(row_type) const { return 0; }
	///
	virtual void addRow(row_type) {}
	///
	virtual void delRow(row_type) {}
	///
	virtual void addCol(col_type) {}
	///
	virtual void delCol(col_type) {}

	///
	virtual bool covers(int x, int y) const;

	/// identifies NestInsets
	virtual MathNestInset * asNestInset() { return 0; }
	/// identifies CharInsets
	virtual MathCharInset const * asCharInset() const { return 0; }
	/// identifies ScriptInsets
	virtual MathScriptInset const * asScriptInset() const { return 0; }
	/// identifies ScriptInsets
	virtual MathScriptInset * asScriptInset() { return 0; }
	/// identifies MatrixInsets
	virtual MathMatrixInset const * asMatrixInset() const { return 0; }
	/// identifies MatrixInsets
	virtual MathMatrixInset * asMatrixInset() { return 0; }
	/// identifies SpaceInset
	virtual MathSpaceInset * asSpaceInset() { return 0; }
	/// identifies GridInset
	virtual MathGridInset * asGridInset() { return 0; }
	/// identifies ArrayInsets
	virtual MathArrayInset * asArrayInset() { return 0; }
	/// identifies BoxInsets
	virtual MathBoxInset * asBoxInset() { return 0; }
	/// identifies macro templates
	virtual MathMacroTemplate * asMacroTemplate() { return 0; }
	/// identifies hyperactive insets
	virtual UpdatableInset * asHyperActiveInset() const { return 0; }

	/// identifies things that can get scripts
	virtual bool isScriptable() const { return false; }
	///
	virtual bool isActive() const { return nargs() > 0; }
	/// identifies insets from the outer world
	virtual bool isHyperActive() const { return 0; }
	///
	virtual bool isRelOp() const { return false; }
	///
	virtual bool isMacro() const { return false; }
	/// is this a matrix or matrix expression?
	virtual bool isMatrix() const { return false; }

	///
	virtual char getChar() const { return 0; }
	///
	virtual MathTextCodes code() const { return LM_TC_MIN; }
	/// identifies things that can get \limits or \nolimits
	virtual bool takesLimits() const { return false; }

	///
	virtual void dump() const;
	///
	virtual void edit(BufferView *, int, int, unsigned int) {}

	///
	virtual void validate(LaTeXFeatures & features) const;
	///
	virtual void handleFont(MathTextCodes) {}

	///
	virtual void octavize(OctaveStream &) const;
	///
	virtual void maplize(MapleStream &) const;
	///
	virtual void mathmlize(MathMLStream &) const;
};

std::ostream & operator<<(std::ostream &, MathInset const &);

#endif
