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
class MathFuncInset;
class MathScriptInset;
class MathSpaceInset;
class InsetFormulaBase;
class MathArray;
class MathXArray;
class Painter;

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
	/// moves position on cell to the left
	bool idxLeft();
	/// moves position on cell to the right
	bool idxRight();
	/// moves position on cell up
	bool idxUp();
	/// moves position on cell up
	bool idxDown();
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
	void insert(char, MathTextCodes t = LM_TC_MIN);
	///
	void insert(MathInset *);
	///
	void insert(MathArray const &);
	///
	void erase();
	///
	void Home();
	///
	void End();
	///
	bool Right(bool sel = false);
	///
	bool Left(bool sel = false);
	///
	bool Up(bool sel = false);
	///
	bool Down(bool sel = false);
	/// Put the cursor in the first position
	void first();
	/// Put the cursor in the last position
	void last();
	///
	bool plainLeft();
	///
	bool plainRight();
	///
	void Delete();
	///
	void DelLine();
	/// This is in pixels from (maybe?) the top of inset
	void SetPos(int, int);
	///
	void GetPos(int & x, int & y);
	///
	MathInset * par() const;
	/// return the next enclosing par of the given type and the cursor's
	//index in it
	MathInset * enclosing(MathInsetTypes, int &) const;
	///
	InsetFormulaBase const * formula();
	///
	int pos() const;
	///
	void Interpret(string const &);
	///
	void SetSize(MathStyles);
	///
	bool toggleLimits();
	///
	// Macro mode methods
	void MacroModeOpen();
	///
	void MacroModeClose();
	///
	bool InMacroMode() const;
	
	// Local selection methods
	///
	bool Selection() const;
	///
	void SelCopy();
	///
	void SelCut();
	///
	void SelDel();
	///
	void SelPaste();
	///
	void SelHandle(bool);
	///
	void SelStart();
	///
	void SelClear();
	///
	void drawSelection(Painter & pain) const;
	///
	void clearLastCode();
	///
	void setLastCode(MathTextCodes t);
	///
	void handleFont(MathTextCodes t);
	///
	void handleAccent(string const & name, int code);
	///
	void handleDelim(int l, int r);
	/// Splits cells and shifts right part to the next cell
	void splitCell();
	/// Splits line and insert new row of cell 
	void breakLine();
	///
	MathTextCodes getLastCode() const;
	///
	int idx() const { return cursor().idx_; }
	///
	void idxNext();
	///
	void idxPrev();
	///
	void pullArg(bool goright);
	///
	bool isInside(MathInset *) const;
	///
	MathTextCodes nextCode() const;
	///
	MathTextCodes prevCode() const;
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
	
	/// Enter a new MathInset from the front or the back
	void push(MathInset * par, bool first);
	/// Leave current MathInset
	bool pop();

//private:
	///
	bool macro_mode;
	
	// Selection stuff
	/// do we currently select
	bool selection;

	///
	InsetFormulaBase * const formula_;
	///
	MathTextCodes lastcode;

	///
	MathArray & array() const;
	///
	MathXArray & xarray() const;

	/// returns the first position of the (normalized) selection
	MathCursorPos firstSelectionPos() const;
	/// returns the last position of the (normalized) selection
	MathCursorPos lastSelectionPos() const;
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
	void seldump(char const * str) const;
	///
	void dump(char const * str) const;

	///
	int xpos() const;
	///
	void gotoX(int x);

	///
	bool nextIsInset() const;
	///
	bool prevIsInset() const;
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

	///
	MathFuncInset * imacro;
};

extern MathCursor * mathcursor;

#endif
