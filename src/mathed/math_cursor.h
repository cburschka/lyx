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
class InsetFormulaBase;
class MathArray;
class MathXArray;

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
	/// Set accent: if argument = 0 it's considered consumed 
	void setAccent(int ac = 0);
	/// Returns last accent
	int getAccent() const;
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
	void SelGetArea(int * xp, int * yp, int & n);
	///
	void clearLastCode();
	///
	void setLastCode(MathTextCodes t);
	///
	void handleFont(MathTextCodes t);
	///
	MathTextCodes getLastCode() const;
	///
	int idx() const { return idx_; }
	///
	void idxRight();
	///
	void pullArg();
	///
	bool isInside(MathInset *) const;
	///
	MathTextCodes nextCode() const;
	///
	MathTextCodes prevCode() const;
	///
	void selArray(MathArray &) const;
	///
	char valign() const;
	///
	char halign() const;
	///
	int col() const;
	///
	int row() const;

//protected:
	///
	bool macro_mode;
	
	// Selection stuff
	///
	bool selection;
	///
	int anchor_;
	///
	int cursor_;
	///
	int idx_;
	///
	MathInset	* par_;
	///
	InsetFormulaBase * const formula_;
	///
	void doAccent(char c, MathTextCodes t);
	///
	void doAccent(MathInset * p);
	///
	int accent;
	///
	int nestaccent[8];
	///
	MathTextCodes lastcode;

	///
	MathArray & array() const;
	///
	MathXArray & xarray() const;

	///
	MathStyles style() const;
	/// Make sure cursor position is valid
	void normalize() const;
	
	/// Enter a new MathInset from the front or the back
	void push(MathInset * par, bool first);
	/// Leave current MathInset
	bool pop();

private:
	/// Description of a position 
	struct MathIter {
		MathInset * par_;
		int idx_;
		int cursor_;
	};

	/// MathPath
	std::vector<MathIter> path_;

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
	bool nextIsActive() const;
	///
	bool prevIsInset() const;
	///
	bool prevIsActive() const;
	///
	bool IsFont() const;
	///
	bool IsScript() const;
	///
	void merge(MathArray const & arr);
	///
	MathInset * nextInset() const;
	///
	MathInset * nextActiveInset() const;
	///
	MathInset * prevInset() const;
	///
	MathInset * prevActiveInset() const;
	///
	MathScriptInset * nearbyScriptInset() const;

	///
	MathFuncInset * imacro;
};

extern MathCursor * mathcursor;

#endif
