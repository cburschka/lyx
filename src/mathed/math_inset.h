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

#include "math_xdata.h"
#include "math_defs.h"

/**

Abstract base class for all math objects.  A math insets is for use of the
math editor only, it isn't a general LyX inset. It's used to represent all
the math objects. 

Math insets do not know there parents, a cursor position or things
like that. The are dumb object that are contained in other math insets
(mathNestInsets, in fact) thus forming a tree. The root of this tree is
always a mathHullInset, which provides an interface to the Outer World by
inclusion in the "real LyX insets" FormulaInset and FormulaMacroInset.

*/


class MathArrayInset;
class MathBoxInset;
class MathCharInset;
class MathDelimInset;
class MathFuncInset;
class MathGridInset;
class MathFracInset;
class MathHullInset;
class MathMatrixInset;
class MathNestInset;
class MathScriptInset;
class MathStringInset;
class MathSpaceInset;
class MathSymbolInset;

class NormalStream;
class OctaveStream;
class MapleStream;
class MathMLStream;
class WriteStream;
class MathArray;

class LaTeXFeatures;
class BufferView;
class UpdatableInset;
class MathMacroTemplate;
class MathPosFinder;


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

	/// our members behave nicely...
	MathInset() {}
	/// the virtual base destructor
	virtual ~MathInset() {}

	/// draw the object
	virtual void draw(Painter &, int x, int y) const;
	/// write LaTeX and Lyx code
	virtual void write(WriteStream & os) const;
	/// reproduce itself
	virtual MathInset * clone() const = 0;
	/// substitutes macro arguments if necessary
	virtual void substitute(MathMacro const & macro);
	/// compute the size of the object, sets ascend_, descend_ and width_
	// updates the (xo,yo)-caches of all contained cells
	virtual void metrics(MathMetricsInfo const & st) const;
	/// the ascent of the inset above the baseline
	virtual int ascent() const { return 1; }
	/// the descent of the inset below the baseline
	virtual int descent() const { return 1; }
	/// total width
	virtual int width() const { return 2; }
	/// total height (== ascent + descent)
	virtual int height() const;

	/// Where should we go when we press the up cursor key?
	virtual bool idxUp(idx_type & idx) const;
	/// The down key
	virtual bool idxDown(idx_type & idx) const;
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
	/// Target pos when we enter the inset from the left by pressing "Up"
	virtual bool idxFirstUp(idx_type & idx, pos_type & pos) const;
	/// Target pos when we enter the inset from the left by pressing "Down"
	virtual bool idxFirstDown(idx_type & idx, pos_type & pos) const;
	/// Target pos when we enter the inset from the right by pressing "Left"
	virtual bool idxLast(idx_type & idx, pos_type & pos) const;
	/// Target pos when we enter the inset from the right by pressing "Up"
	virtual bool idxLastUp(idx_type & idx, pos_type & pos) const;
	/// Target pos when we enter the inset from the right by pressing "Down"
	virtual bool idxLastDown(idx_type & idx, pos_type & pos) const;

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

	/// the number of nested cells this inset owns
	virtual idx_type nargs() const;

	/// return cell given its number
	virtual MathArray & cell(idx_type);
	/// return cell given its number
	virtual MathArray const & cell(idx_type) const;
	/// return cell plus drawing cache given its number
	virtual MathXArray & xcell(idx_type);
	/// return cell plus drawing cache given its number
	virtual MathXArray const & xcell(idx_type) const;
			
	/// the number of columns of this inset if it is grid-like
	virtual col_type ncols() const { return 1; }
	/// the number of rows of this inset if it is grid-like
	virtual row_type nrows() const { return 1; }
	/// to which column belongs a cell with a given index?
	virtual col_type col(idx_type) const { return 0; }
	/// to which row belongs a cell with a given index?
	virtual row_type row(idx_type) const { return 0; }
	/// any additional x-offset when drawing a cell?
	virtual int cellXOffset(idx_type) const { return 0; }
	/// any additional y-offset when drawing a cell?
	virtual int cellYOffset(idx_type) const { return 0; }

	/// identifies certain types of insets
	virtual MathArrayInset        * asArrayInset()        { return 0; }
	virtual MathBoxInset          * asBoxInset()          { return 0; }
	virtual MathCharInset const   * asCharInset() const   { return 0; }
	virtual MathDelimInset        * asDelimInset()        { return 0; }
	virtual MathDelimInset const  * asDelimInset() const  { return 0; }
	virtual MathFuncInset         * asFuncInset()         { return 0; }
	virtual MathFracInset         * asFracInset()         { return 0; }
	virtual MathGridInset         * asGridInset()         { return 0; }
	virtual MathHullInset         * asHullInset()         { return 0; }
	virtual MathHullInset const   * asHullInset() const   { return 0; }
	virtual MathMacroTemplate     * asMacroTemplate()     { return 0; }
	virtual MathMatrixInset const * asMatrixInset() const { return 0; }
	virtual MathNestInset         * asNestInset()         { return 0; }
	virtual MathScriptInset       * asScriptInset()       { return 0; }
	virtual MathScriptInset const * asScriptInset() const { return 0; }
	virtual MathSpaceInset        * asSpaceInset()        { return 0; }
	virtual MathStringInset       * asStringInset()       { return 0; }
	virtual MathSymbolInset const * asSymbolInset() const { return 0; }
	virtual UpdatableInset   * asHyperActiveInset() const { return 0; }

	/// identifies things that can get scripts
	virtual bool isScriptable() const { return false; }
	/// thing that can be moved into
	virtual bool isActive() const { return nargs() > 0; }
	/// identifies insets from the outer world
	virtual bool isHyperActive() const { return 0; }
	/// is the a relational operator (used for splitting equations)
	virtual bool isRelOp() const { return false; }

	/// return the content as char if the inset is able to do so
	virtual char getChar() const { return 0; }
	/// return the content's char code if it has one
	virtual MathTextCodes code() const { return LM_TC_MIN; }
	/// identifies things that can get \limits or \nolimits
	virtual bool takesLimits() const { return false; }

	///
	virtual void edit(BufferView *, int, int, unsigned int) {}

	/// request "external features"
	virtual void validate(LaTeXFeatures & features) const;
	/// char char code if possible
	virtual void handleFont(MathTextCodes) {}
	/// is this inset equal to a given other inset?
	virtual bool match(MathInset *) const { return false; }
	/// replace things by other things
	virtual void replace(ReplaceData &) {}

	/// write normalized content
	virtual void normalize(NormalStream &) const;
	/// write content as something readable by Maple
	virtual void maplize(MapleStream &) const;
	/// write content as something resembling MathML
	virtual void mathmlize(MathMLStream &) const;
	/// write content as something readable by Octave
	virtual void octavize(OctaveStream &) const;

	/// dump content to stderr for debugging
	virtual void dump() const;
};

std::ostream & operator<<(std::ostream &, MathInset const &);

#endif
