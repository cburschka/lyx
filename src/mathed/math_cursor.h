// -*- C++ -*-
/*
 *  File:        math_cursor.h
 *  Purpose:     Declaration of interaction classes for mathed
 *  Author:      Alejandro Aguilar Sierra <asierra@servidor.unam.mx> 
 *  Created:     January 1996
 *  Description: MathCursor control all user interaction
 *
 *  Dependencies: Xlib, XForms
 *
 *  Copyright: 1996, Alejandro Aguilar Sierra
 *
 *   You are free to use and modify this code under the terms of
 *   the GNU General Public Licence version 2 or later.
 */

#ifndef MATH_CURSOR
#define MATH_CURSOR

#ifdef __GNUG__
#pragma interface
#endif

#include "math_defs.h"
#include "math_inset.h"
#include "math_pos.h"
#include "LString.h"

class InsetFormulaBase;
class Painter;
class Selection;

/**

[Have a look at math_inset.h first]

The MathCursor is different from the kind of cursor used in the Outer
World. It contains a stack of MathCursorPositions, each of which is made
up of a inset pointer, an index and a position offset, marking a path from
this formula's mathHullInset to the current position.

*/


class MathCursor {
public:
	/// short of anything else reasonable
	typedef MathInset::size_type       size_type;
	/// type for cursor positions within a cell
	typedef MathInset::pos_type        pos_type;
	/// type for cell indices
	typedef MathInset::idx_type        idx_type;
	/// type for row numbers
	typedef MathInset::row_type        row_type;
	/// type for column numbers
	typedef MathInset::col_type        col_type;
	/// how to store a cursor
	typedef std::vector<MathCursorPos> cursor_type;

	///
	explicit MathCursor(InsetFormulaBase *, bool left);
	///
	void insert(MathAtom const &);
	///
	void insert(MathArray const &);
	///
	void paste(MathArray const &);
	///
	void erase();
	///
	void backspace();
	/// called for LFUN_HOME etc
	void home(bool sel = false);
	/// called for LFUN_END etc
	void end(bool sel = false);
	/// called for LFUN_RIGHT and LFUN_RIGHTSEL
	bool right(bool sel = false);
	/// called for LFUN_LEFT etc
	bool left(bool sel = false);
	/// called for LFUN_UP etc
	bool up(bool sel = false);
	/// called for LFUN_DOWN etc
	bool down(bool sel = false);
	/// Put the cursor in the first position
	void first();
	/// Put the cursor in the last position
	void last();
	/// move to next cell in current inset
	void idxNext();
	/// move to previous cell in current inset
	void idxPrev();
	///
	void plainErase();
	///
	void plainInsert(MathAtom const &);
	///
	void niceInsert(MathAtom const &);

	///
	void delLine();
	/// in pixels from top of screen
	void setPos(int x, int y);
	/// in pixels from top of screen
	void getPos(int & x, int & y);
	/// 
	MathAtom & par() const;
	/// return the next enclosing grid inset and the cursor's index in it
	MathGridInset * enclosingGrid(idx_type &) const;
	///
	InsetFormulaBase * formula();
	/// current offset in the current cell
	pos_type pos() const;
	/// current cell
	idx_type idx() const;
	/// size of current cell
	size_type size() const;
	///
	bool interpret(string const &);
	///
	bool interpret(char);
	///
	bool toggleLimits();
	/// interpret name a name of a macro
	void macroModeClose();
	///
	bool inMacroMode() const;
	
	// Local selection methods
	///
	bool selection() const;
	///
	void selCopy();
	///
	void selCut();
	///
	void selDel();
	///
	void selPaste();
	///
	void selHandle(bool);
	///
	void selStart();
	///
	void selClear();
	///
	void selGet(MathArray & ar);
	///
	void drawSelection(Painter & pain) const;
	///
	void handleFont(MathTextCodes t);
	///
	void handleDelim(string const & l, string const & r);
	///
	void handleNest(MathInset * p);
	/// splits cells and shifts right part to the next cell
	void splitCell();
	/// splits line and insert new row of cell 
	void breakLine();
	/// read contents of line into an array
	void readLine(MathArray & ar) const;
	///
	MathTextCodes getLastCode() const;
	///
	void pullArg(bool goright);
	///
	bool isInside(MathInset const *) const;
	///
	MathTextCodes nextCode() const;
	///
	char valign() const;
	///
	char halign() const;
	///
	col_type hullCol() const;
	///
	row_type hullRow() const;

	/// make sure cursor position is valid
	void normalize() const;
	///
	UpdatableInset * asHyperActiveInset() const;

	/// enter a MathInset 
	void push(MathAtom & par);
	/// enter a MathInset from the front
	void pushLeft(MathAtom & par);
	/// enter a MathInset from the back
	void pushRight(MathAtom & par);
	/// leave current MathInset to the left
	bool popLeft();
	/// leave current MathInset to the left
	bool popRight();

	///
	MathArray & array() const;
	///
	MathXArray & xarray() const;
	///
	bool hasPrevAtom() const;
	///
	bool hasNextAtom() const;
	///
	MathAtom const & prevAtom() const;
	///
	MathAtom & prevAtom();
	///
	MathAtom const & nextAtom() const;
	///
	MathAtom & nextAtom();

	/// returns the selection
	void getSelection(MathCursorPos &, MathCursorPos &) const;
	/// returns the normalized anchor of the selection
	MathCursorPos normalAnchor() const;

	/// path of positions the cursor had to go if it were leving each inset
	cursor_type Cursor_;
	/// path of positions the anchor had to go if it were leving each inset
	cursor_type Anchor_;

	/// reference to the last item of the path, i.e. "The Cursor"
	MathCursorPos & cursor();
	/// reference to the last item of the path, i.e. "The Cursor"
	MathCursorPos const & cursor() const;

	/// dump selection information for debugging
	void seldump(char const * str) const;
	/// dump selection information for debugging
	void dump(char const * str) const;
	///
	void stripFromLastEqualSign();

	///
	friend class Selection;

private:
	/// moves cursor position one cell to the left
	bool posLeft();
	/// moves cursor position one cell to the right
	bool posRight();
	/// moves cursor index one cell to the left
	bool idxLeft();
	/// moves cursor index one cell to the right
	bool idxRight();
	/// moves position somehow up
	bool goUp();
	/// moves position somehow down
	bool goDown();
	/// moves position into box
	bool bruteFind(int xlow, int xhigh, int ylow, int yhigh);

	///
	string macroName() const;
	///
	int macroNamePos() const;
	///
	void insert(char, MathTextCodes t);
	/// can we enter the inset? 
	bool openable(MathAtom const &, bool selection) const;
	/// write access to cursor cell position
	pos_type & pos();
	/// write access to cursor cell index
	idx_type & idx();

	///
	InsetFormulaBase * formula_;
	///
	MathTextCodes lastcode_;
	// Selection stuff
	/// do we currently select
	bool selection_;
};

extern MathCursor * mathcursor;

#endif
