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
#include "LString.h"

class MathInset;
class MathAtom;
class MathGridInset;
class MathFuncInset;
class MathHullInset;
class MathScriptInset;
class MathSpaceInset;
class InsetFormulaBase;
class MathArray;
class MathXArray;
class Painter;
class Selection;
class latexkeys;

/// Description of a position 
struct MathCursorPos {
	/// inset
	MathAtom * par_;
	/// cell index
	MathInset::idx_type idx_;
	/// cell position
	MathInset::pos_type pos_;

	/// returns cell corresponding to this position
	MathArray & cell() const;
	/// returns cell corresponding to this position
	MathArray & cell(MathInset::idx_type idx) const;
	/// returns xcell corresponding to this position
	MathXArray & xcell() const;
	/// returns xcell corresponding to this position
	MathXArray & xcell(MathInset::idx_type idx) const;
};

/// 
bool operator==(MathCursorPos const &, MathCursorPos const &);
/// 
bool operator<(MathCursorPos const &, MathCursorPos const &);


/// This is the external interface of Math's subkernel
class MathCursor {
public:
	/// short of anything else reasonable
	typedef MathInset::size_type    size_type;
	/// type for cursor positions within a cell
	typedef MathInset::pos_type     pos_type;
	/// type for cell indices
	typedef MathInset::idx_type     idx_type;
	/// type for row numbers
	typedef MathInset::row_type     row_type;
	/// type for column numbers
	typedef MathInset::col_type     col_type;

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
	///
	void home(bool sel = false);
	///
	void end(bool sel = false);
	///
	bool right(bool sel = false);
	///
	bool left(bool sel = false);
	///
	bool up(bool sel = false);
	///
	bool down(bool sel = false);
	/// Put the cursor in the first position
	void first();
	/// Put the cursor in the last position
	void last();
	///
	void idxNext();
	///
	void idxPrev();
	///
	void plainErase();
	///
	void plainInsert(MathAtom const &);
	///
	void niceInsert(MathAtom const &);

	///
	void delLine();
	/// This is in pixels from (maybe?) the top of inset
	void setPos(int, int);
	///
	void getPos(int & x, int & y);
	///
	MathAtom & par() const;
	/// return the next enclosing grid inset and the cursor's index in it
	MathGridInset * enclosingGrid(idx_type &) const;
	///
	InsetFormulaBase const * formula();
	///
	pos_type pos() const;
	///
	idx_type idx() const;
	///
	size_type size() const;
	///
	bool interpret(string const &);
	///
	bool interpret(char);
	///
	bool toggleLimits();
	///
	// Macro mode methods
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
	/// Splits cells and shifts right part to the next cell
	void splitCell();
	/// Splits line and insert new row of cell 
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
	col_type ncols() const;
	///
	col_type col() const;
	///
	row_type row() const;

	/// Make sure cursor position is valid
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
	std::vector<MathCursorPos> Cursor_;
	/// path of positions the anchor had to go if it were leving each inset
	std::vector<MathCursorPos> Anchor_;

	/// reference to the last item of the path
	MathCursorPos & cursor();
	///
	MathCursorPos const & cursor() const;

	///
	void seldump(char const * str) const;
	///
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

	///
	string macroName() const;
	///
	int macroNamePos() const;
	///
	void insert(char, MathTextCodes t);
	/// can we enter the inset? 
	bool openable(MathAtom const &, bool selection) const;
	/// can the setPos routine enter that inset?
	bool positionable(MathAtom const &, int x, int y) const;
	/// write access to cursor cell position
	pos_type & pos();
	/// write access to cursor cell index
	idx_type & idx();
	/// x-offset of current cell relative to par xo
	int cellXOffset() const;
	/// y-offset of current cell relative to par yo
	int cellYOffset() const;
	/// current x position relative to par xo
	int xpos() const;
	/// current y position relative to par yo
	int ypos() const;
	/// adjust position in current cell according to x. idx is not changed.
	void gotoX(int x);

	///
	InsetFormulaBase * const formula_;
	///
	MathTextCodes lastcode_;
	// Selection stuff
	/// do we currently select
	bool selection_;
};

extern MathCursor * mathcursor;

#endif
