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

#include "LString.h"
#include "frontends/mouse_state.h"
#include "math_data.h"

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
class MathAMSArrayInset;
class MathCharInset;
class MathDelimInset;
class MathGridInset;
class MathFracInset;
class MathHullInset;
class MathMatrixInset;
class MathNestInset;
class MathParboxInset;
class MathScriptInset;
class MathStringInset;
class MathSpaceInset;
class MathSymbolInset;
class MathUnknownInset;

class InsetRef;

class NormalStream;
class OctaveStream;
class MapleStream;
class MathematicaStream;
class MathMLStream;
class WriteStream;
class InfoStream;
class MathArray;

class LaTeXFeatures;
class BufferView;
class UpdatableInset;
class MathMacroTemplate;
class MathPosFinder;
class Dimension;


class MathInset {
public:
	/// short of anything else reasonable
	typedef MathArray::size_type        size_type;
	/// type for cursor positions differences within a cell
	typedef MathArray::difference_type  difference_type;
	/// type for cursor positions within a cell
	typedef MathArray::size_type        pos_type;
	/// type for cell indices
	typedef size_type                   idx_type;
	/// type for row numbers
	typedef size_type                   row_type;
	/// type for column numbers
	typedef size_type                   col_type;

	/// our members behave nicely...
	MathInset() {}
	/// the virtual base destructor
	virtual ~MathInset() {}

	/// reproduce itself
	virtual MathInset * clone() const = 0;
	/// substitutes macro arguments if necessary
	virtual void substitute(MathMacro const & macro);
	/// compute the size of the object, sets ascend_, descend_ and width_
	// updates the (xo,yo)-caches of all contained cells
	virtual void metrics(MathMetricsInfo & mi) const;
	/// draw the object
	virtual void draw(MathPainterInfo & pi, int x, int y) const;
	/// draw selection between two positions
	virtual void drawSelection(MathPainterInfo & pi,
		idx_type idx1, pos_type pos1, idx_type idx2, pos_type pos2) const;
	/// the ascent of the inset above the baseline
	/// compute the size of the object for text based drawing
	virtual void metricsT(TextMetricsInfo const & st) const;
	/// draw the object as text
	virtual void drawT(TextPainter &, int x, int y) const;
	/// the ascent of the inset above the baseline
	virtual int ascent() const { return 1; }
	/// the descent of the inset below the baseline
	virtual int descent() const { return 1; }
	/// total width
	virtual int width() const { return 2; }
	/// all in one batch
	virtual Dimension dimensions() const;
	/// total height (== ascent + descent)
	virtual int height() const;

	/// Where should we go when we press the up or down cursor key?
	virtual bool idxUpDown(idx_type & idx, pos_type & pos, bool up,
		int targetx) const;
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
	virtual bool idxDelete(idx_type &) { return false; }
	/// pulls cell after pressing erase
	virtual void idxGlue(idx_type) {}
	// returns list of cell indices that are "between" from and to for
	// selection purposes
	virtual bool idxBetween(idx_type idx, idx_type from, idx_type to) const;

	/// the number of nested cells this inset owns
	virtual idx_type nargs() const;

	/// return cell given its number
	virtual MathArray & cell(idx_type);
	/// return cell given its number
	virtual MathArray const & cell(idx_type) const;

	/// the number of columns of this inset if it is grid-like
	virtual col_type ncols() const { return 1; }
	/// the number of rows of this inset if it is grid-like
	virtual row_type nrows() const { return 1; }
	/// to which column belongs a cell with a given index?
	virtual col_type col(idx_type) const { return 0; }
	/// to which row belongs a cell with a given index?
	virtual row_type row(idx_type) const { return 0; }
	/// cell idex corresponding to row and column;
	virtual idx_type index(row_type row, col_type col) const;
	/// any additional x-offset when drawing a cell?
	virtual int cellXOffset(idx_type) const { return 0; }
	/// any additional y-offset when drawing a cell?
	virtual int cellYOffset(idx_type) const { return 0; }
	/// can we enter this cell?
	virtual bool validCell(idx_type) const { return true; }
	/// get coordinates
	virtual void getPos(idx_type idx, pos_type pos, int & x, int & y) const;

	/// identifies certain types of insets
	virtual MathAMSArrayInset      * asAMSArrayInset()      { return 0; }
	virtual MathArrayInset         * asArrayInset()         { return 0; }
	virtual MathCharInset const    * asCharInset() const    { return 0; }
	virtual MathDelimInset         * asDelimInset()         { return 0; }
	virtual MathDelimInset const   * asDelimInset() const   { return 0; }
	virtual MathFracInset          * asFracInset()          { return 0; }
	virtual MathGridInset          * asGridInset()          { return 0; }
	virtual MathHullInset          * asHullInset()          { return 0; }
	virtual MathHullInset const    * asHullInset() const    { return 0; }
	virtual MathMacroTemplate      * asMacroTemplate()      { return 0; }
	virtual MathMatrixInset const  * asMatrixInset() const  { return 0; }
	virtual MathNestInset          * asNestInset()          { return 0; }
	virtual MathParboxInset        * asParboxInset()        { return 0; }
	virtual MathScriptInset        * asScriptInset()        { return 0; }
	virtual MathScriptInset const  * asScriptInset() const  { return 0; }
	virtual MathSpaceInset         * asSpaceInset()         { return 0; }
	virtual MathStringInset        * asStringInset()        { return 0; }
	virtual MathSymbolInset const  * asSymbolInset() const  { return 0; }
	virtual MathUnknownInset       * asUnknownInset()       { return 0; }
	virtual MathUnknownInset const * asUnknownInset() const { return 0; }
	virtual InsetRef               * asInsetRef()           { return 0; }

	/// identifies things that can get scripts
	virtual bool isScriptable() const { return false; }
	/// thing that can be moved into
	virtual bool isActive() const { return nargs() > 0; }
	/// is the a relational operator (used for splitting equations)
	virtual bool isRelOp() const { return false; }
	/// -1: text mode, 1: math mode, 0 undecided
	enum mode_type {UNDECIDED_MODE, TEXT_MODE, MATH_MODE};
	virtual mode_type currentMode() const { return UNDECIDED_MODE; }
	/// will this get written as a single block in {..}
	virtual bool extraBraces() const { return false; }

	/// return the content as char if the inset is able to do so
	virtual char getChar() const { return 0; }
	/// identifies things that can get \limits or \nolimits
	virtual bool takesLimits() const { return false; }

	///
	virtual void edit(BufferView *, int, int, mouse_button::state) {}

	/// request "external features"
	virtual void validate(LaTeXFeatures &) const {}
	/// char char code if possible
	virtual void handleFont(string const &) {}
	/// is this inset equal to a given other inset?
	virtual bool match(MathInset *) const { return false; }
	/// replace things by other things
	virtual void replace(ReplaceData &) {}
	/// do we contain a given subsequence?
	virtual bool contains(MathArray const &) { return false; }
	/// access to the lock (only nest array have one)
	virtual bool lock() const { return false; }
	/// access to the lock (only nest array have one)
	virtual void lock(bool) {}
	/// get notification when the cursor leaves this inset
	virtual void notifyCursorLeaves() {}

	/// write LaTeX and Lyx code
	virtual void write(WriteStream & os) const;
	/// write normalized content
	virtual void normalize(NormalStream &) const;
	/// write content as something readable by Maple
	virtual void maplize(MapleStream &) const;
	/// write content as something readable by Mathematica
	virtual void mathematicize(MathematicaStream &) const;
	/// write content as something resembling MathML
	virtual void mathmlize(MathMLStream &) const;
	/// write content as something readable by Octave
	virtual void octavize(OctaveStream &) const;
	/// describe content
	virtual void infoize(std::ostream &) const {}
	/// plain ascii output
	virtual int ascii(std::ostream & os, int) const;
	/// linuxdoc output
	virtual int linuxdoc(std::ostream & os) const;
	/// docbook output
	virtual int docbook(std::ostream & os, bool) const;

	/// dump content to stderr for debugging
	virtual void dump() const;
	/// local dispatcher
	virtual int dispatch(string const & cmd, idx_type idx, pos_type pos);

	/// LyXInset stuff
	/// write labels into a list
	virtual void getLabelList(std::vector<string> &) const {}
	/// LyXInset stuff
	virtual bool numberedType() const { return false; }
	/// hull type
	virtual string const & getType() const;
	/// change type
	virtual void mutate(string const &) {}
	/// how is the inset called in the .lyx file?
	virtual string fileInsetLabel() const { return "Formula"; }
	/// usually the latex name
	virtual string name() const;
};

std::ostream & operator<<(std::ostream &, MathInset const &);
std::ostream & operator<<(std::ostream &, MathAtom const &);

string asString(MathArray const & ar);
MathArray asArray(string const & str);
void initMath();

/// here to ssave a few includes in the insets
class Dialogs;
class LyXFunc;

Dialogs * getDialogs();
LyXFunc * getLyXFunc();

#endif
