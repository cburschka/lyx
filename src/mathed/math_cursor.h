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

class MathInset;
class MathArrayInset;
class MathFuncInset;
class MathMatrixInset;
class MathScriptInset;
class MathSpaceInset;
class InsetFormulaBase;
class MathArray;
class MathXArray;
class Painter;
class latexkeys;

/// Description of a position 
struct MathCursorPos {
	/// inset
	MathInset * par_;
	/// cell index
	int idx_;
	/// cell position
	int pos_;
	/// returns cell corresponding to this position
	MathArray & cell() const;
	/// returns cell corresponding to this position
	MathArray & cell(int idx) const;
	/// returns xcell corresponding to this position
	MathXArray & xcell() const;
	/// returns xcell corresponding to this position
	MathXArray & xcell(int idx) const;
};

/// 
bool operator==(MathCursorPos const &, MathCursorPos const &);
/// 
bool operator<(MathCursorPos const &, MathCursorPos const &);


/// This is the external interface of Math's subkernel
class MathCursor {
public:
	///
	explicit MathCursor(InsetFormulaBase *);
	///
	~MathCursor();
	///
	void insert(char, MathTextCodes t = LM_TC_MIN);
	///
	void insert(MathInset *);
	///
	void insert(MathArray const &);
	///
	void erase();
	///
	void backspace();
	///
	void home();
	///
	void end();
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
	void idxNext();
	///
	void idxPrev();
	///
	void plainErase();
	///
	void plainInsert(MathInset * p);
	///
	void niceInsert(MathInset * p);
	///
	void delLine();
	/// This is in pixels from (maybe?) the top of inset
	void setPos(int, int);
	///
	void getPos(int & x, int & y);
	///
	MathInset * par() const;
	/// return the next enclosing grid inset and the cursor's index in it
	MathArrayInset * enclosingArray(int &) const;
	///
	InsetFormulaBase const * formula();
	///
	int pos() const;
	///
	int idx() const;
	///
	int size() const;
	///
	void interpret(string const &);
	///
	void setSize(MathStyles);
	///
	bool toggleLimits();
	///
	// Macro mode methods
	void macroModeOpen();
	///
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
	void drawSelection(Painter & pain) const;
	///
	void handleFont(MathTextCodes t);
	///
	void handleAccent(string const & name);
	///
	void handleDelim(latexkeys const * l, latexkeys const * r);
	///
	void handleNest(MathInset * p);
	/// Splits cells and shifts right part to the next cell
	void splitCell();
	/// Splits line and insert new row of cell 
	void breakLine();
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
	int col() const;
	///
	int row() const;

	///
	MathStyles style() const;
	/// Make sure cursor position is valid
	void normalize() const;
	
	/// enter a MathInset from the front
	void pushLeft(MathInset * par);
	/// enter a MathInset from the back
	void pushRight(MathInset * par);
	/// leave current MathInset to the left
	bool popLeft();
	/// leave current MathInset to the left
	bool popRight();

	///
	MathArray & array() const;
	///
	MathXArray & xarray() const;

	/// returns the selection
	void getSelection(MathCursorPos &, MathCursorPos &) const;
	/// returns the normalized anchor of the selection
	MathCursorPos normalAnchor() const;
	/// returns the normalized anchor of the selection
	bool openable(MathInset *, bool selection, bool useupdown) const;

	/// path of positions the cursor had to go if it were leving each inset
	std::vector<MathCursorPos> Cursor_;
	/// path of positions the anchor had to go if it were leving each inset
	std::vector<MathCursorPos> Anchor_;

	/// reference to the last item of the path
	MathCursorPos & cursor();
	///
	MathCursorPos const & cursor() const;


	///  
	int last() const;
	///
	MathInset * parInset(int i) const;
	///
	MathMatrixInset * outerPar() const;
	///
	void seldump(char const * str) const;
	///
	void dump(char const * str) const;

	///
	void merge(MathArray const & arr);
	///
	MathInset * nextInset() const;
	///
	MathInset * prevInset() const;
	///
	MathScriptInset * prevScriptInset() const;
	///
	MathSpaceInset * prevSpaceInset() const;
private:
	///
	int & pos();
	///
	int & idx();
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
	///
	MathFuncInset * imacro_;
	// Selection stuff
	/// do we currently select
	bool selection_;
};

extern MathCursor * mathcursor;

#endif
